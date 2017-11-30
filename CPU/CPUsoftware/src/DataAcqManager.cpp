#include "DataAcqManager.h"

/* default constructor */
DataAcqManager::DataAcqManager() { 
  /* filename initialisation */
  this->cpu_main_file_name = "";
  this->cpu_sc_file_name = "";    
}
  
/* create cpu run file name */
std::string DataAcqManager::CreateCpuRunName(RunType run_type, Config * ConfigOut) {
  struct timeval tv;
  char cpu_file_name[80];
  std::string done_str(DONE_DIR);
  std::string usb_str(USB_MOUNTPOINT_0);
  std::string time_str;
  switch (run_type) {
  case CPU:
    time_str = "/CPU_RUN_MAIN__%Y_%m_%d__%H_%M_%S.dat";
    break;
  case SC:
    time_str = "/CPU_RUN_SC__%Y_%m_%d__%H_%M_%S__" + std::to_string(ConfigOut->dynode_voltage) + ".dat";
    break;
  }
  std::string cpu_str;

  /* get the number of devices */
  UsbManager UManager;
  uint8_t num_storage_dev = UManager.num_storage_dev;
  
  /* write on USB if possible */
  if (num_storage_dev == 1 || num_storage_dev == 2) {
    //cpu_str = usb_str + time_str;
    /* update when camera bug fixed */
    cpu_str = done_str + time_str;
  }
  else {
    cpu_str = done_str + time_str;
  }
  const char * kCpuCh = cpu_str.c_str();

  gettimeofday(&tv ,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  
  strftime(cpu_file_name, sizeof(cpu_file_name), kCpuCh, now_tm);
  return cpu_file_name;
}

/* build the cpu file header */
uint32_t DataAcqManager::BuildCpuFileHeader(uint32_t type, uint32_t ver) {

  uint32_t header;
  header =  (('C'<<24) | (INSTRUMENT_ME_PDM<<16) | ((type)<<8) | (ver));
 
  return header;
}

/* build the cpu packet header */
uint32_t DataAcqManager::BuildCpuPktHeader(uint32_t type, uint32_t ver) {

  uint32_t header;
  header =  (('P'<<24) | (INSTRUMENT_ME_PDM<<16) | ((type)<<8) | (ver));
 
  return header;
}

/* build the cpu timestamp */
uint32_t DataAcqManager::BuildCpuTimeStamp() {

  uint32_t timestamp;
  struct timeval tv; 
  gettimeofday(&tv, 0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  
  timestamp = ( ((now_tm->tm_year + 1900 - 2017) << 26) | ((now_tm->tm_mon) << 22) | ((now_tm->tm_mday) << 17) | ((now_tm->tm_hour) << 12) | ((now_tm->tm_min) << 6) | (now_tm->tm_sec));

  return timestamp;
}

/* make a cpu data file for a new run */
int DataAcqManager::CreateCpuRun(RunType run_type, Config * ConfigOut) {

  CpuFileHeader * cpu_file_header = new CpuFileHeader();
  
  /* set the cpu file name */
  switch (run_type) {
  case CPU: 
    this->cpu_main_file_name = CreateCpuRunName(CPU, ConfigOut);
    clog << "info: " << logstream::info << "Set cpu_main_file_name to: " << cpu_main_file_name << std::endl;
    this->CpuFile = std::make_shared<SynchronisedFile>(this->cpu_main_file_name);
    break;
  case SC: 
    this->cpu_sc_file_name = CreateCpuRunName(SC, ConfigOut);
    clog << "info: " << logstream::info << "Set cpu_sc_file_name to: " << cpu_sc_file_name << std::endl;
    this->CpuFile = std::make_shared<SynchronisedFile>(this->cpu_sc_file_name);
    break;
  }
  this->RunAccess = new Access(this->CpuFile); 
   
  /* set up the cpu file structure */
  cpu_file_header->header = BuildCpuFileHeader(CPU_FILE_TYPE, CPU_FILE_VER);
  cpu_file_header->run_size = RUN_SIZE;

  /* write to file */
  this->RunAccess->WriteToSynchFile<CpuFileHeader *>(cpu_file_header);
  delete cpu_file_header;
  
  
  return 0;
}

/* close the CPU file run and append CRC */
int DataAcqManager::CloseCpuRun(RunType run_type) {

  CpuFileTrailer * cpu_file_trailer = new CpuFileTrailer();
  
  clog << "info: " << logstream::info << "closing the cpu run file called " << this->CpuFile->path << std::endl;
  
  /* set up the cpu file trailer */
  cpu_file_trailer->run_size = RUN_SIZE;
  cpu_file_trailer->crc = this->RunAccess->GetChecksum(); 

  /* write to file */
  this->RunAccess->WriteToSynchFile<CpuFileTrailer *>(cpu_file_trailer);
  delete cpu_file_trailer;

  /* close the current SynchronisedFile */
  this->RunAccess->CloseSynchFile();
  return 0;
}


/* read out an scurve file into an scurve packet */
SC_PACKET * DataAcqManager::ScPktReadOut(std::string sc_file_name, Config * ConfigOut) {

  FILE * ptr_scfile;
  SC_PACKET * sc_packet = new SC_PACKET();
  const char * kScFileName = sc_file_name.c_str();
  size_t check;

  clog << "info: " << logstream::info << "reading out the file " << sc_file_name << std::endl;
  
  ptr_scfile = fopen(kScFileName, "rb");
  if (!ptr_scfile) {
    clog << "error: " << logstream::error << "cannot open the file " << sc_file_name << std::endl;
    return NULL;
  }
  
  /* prepare the scurve packet */
  sc_packet->sc_packet_header.header = BuildCpuPktHeader(SC_PACKET_TYPE, SC_PACKET_VER);
  sc_packet->sc_packet_header.pkt_size = sizeof(SC_PACKET);
  sc_packet->sc_time.cpu_time_stamp = BuildCpuTimeStamp();
  sc_packet->sc_start = ConfigOut->scurve_start;
  sc_packet->sc_step = ConfigOut->scurve_step;
  sc_packet->sc_stop = ConfigOut->scurve_stop;
  sc_packet->sc_acc = ConfigOut->scurve_acc;

  ptr_scfile = fopen(kScFileName, "rb");
  if (!ptr_scfile) {
    clog << "error: " << logstream::error << "cannot open the file " << sc_file_name << std::endl;
    return NULL;
  }
  
  /* read out the scurve data from the file */
  check = fread(&sc_packet->sc_data, sizeof(sc_packet->sc_data), 1, ptr_scfile);
  if (check != 1) {
    clog << "error: " << logstream::error << "fread from " << sc_file_name << " failed" << std::endl;
    return NULL;   
  }
  
  /* close the scurve file */
  fclose(ptr_scfile);
  
  return sc_packet;
}


/* read out a zynq data file into a zynq packet */
ZYNQ_PACKET * DataAcqManager::ZynqPktReadOut(std::string zynq_file_name) {

  FILE * ptr_zfile;
  ZYNQ_PACKET * zynq_packet = new ZYNQ_PACKET();
  const char * kZynqFileName = zynq_file_name.c_str();
  size_t check;
  int fsize;

  clog << "info: " << logstream::info << "reading out the file " << zynq_file_name << std::endl;

  ptr_zfile = fopen(kZynqFileName, "rb");
  if (!ptr_zfile) {
    clog << "error: " << logstream::error << "cannot open the file " << zynq_file_name << std::endl;
    return NULL;
  }

  /* find the length of the file */
  fseek(ptr_zfile, 0L, SEEK_END);
  fsize = ftell(ptr_zfile);
  rewind(ptr_zfile);
  
  /* DEBUG */
  std::cout << "file size: " << fsize << std::endl;
  std::cout << "sizeof(*zynq_packet): " << sizeof(*zynq_packet) << std::endl;
  
  std::cout << "zynq file name: " << zynq_file_name << std::endl;
  std::cout << "ptr_zfile: " << ptr_zfile << std::endl;
  std::cout << "zynq_packet: " << zynq_packet << std::endl;
  
  /* read out the zynq structure, defined in "pdmdata.h" */
  check = fread(zynq_packet, sizeof(*zynq_packet), 1, ptr_zfile);

  /* DEBUG */
  std::cout << "Check: " << check << std::endl;
  std::cout << "feof: " << feof(ptr_zfile) << std::endl;
  std::cout << "ferror: " << ferror(ptr_zfile) << std::endl;
  
  if (check != 1) {
    clog << "error: " << logstream::error << "fread from " << zynq_file_name << " failed" << std::endl;
    return NULL;
  }
  
  /* DEBUG: print records to check */
  std::cout << "header L1 = " << zynq_packet->level1_data[0].zbh.header << std::endl;
  std::cout << "payload_size L1 = " << zynq_packet->level1_data[0].zbh.payload_size << std::endl;
  std::cout << "hv_status L1 = " << zynq_packet->level1_data[0].payload.hv_status << std::endl;
  std::cout << "n_gtu L1 = " << zynq_packet->level1_data[0].payload.ts.n_gtu << std::endl; 

  /* close the zynq file */
  fclose(ptr_zfile);
  
  return zynq_packet;
}

/* analog board read out */
AnalogAcq * DataAcqManager::AnalogDataCollect() {
  AnalogAcq * acq_output = new AnalogAcq();
#ifndef __APPLE__  
  DM75xx_Board_Descriptor * brd;
  DM75xx_Error dm75xx_status;
  dm75xx_cgt_entry_t cgt[CHANNELS];
  int i, j;
  float actR;
  uint16_t data = 0x0000;  
  unsigned long int minor_number = 0;

  clog << "info: " << logstream::info << "starting analog acquistion" << std::endl;

  /* Device initialisation */
  dm75xx_status = DM75xx_Board_Open(minor_number, &brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Open");
  dm75xx_status = DM75xx_Board_Init(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Init");
  
  /* Clear the FIFO */
  dm75xx_status = DM75xx_ADC_Clear(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Clear_AD_FIFO");
  dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_FIFO_Get_Status");
  
  /* enable the channel gain table */
  dm75xx_status = DM75xx_CGT_Enable(brd, 0xFF);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_CGT_Enable");
  
  /* set the channel gain table for all channels */
  for (i = 0; i < CHANNELS; i++) {
    cgt[i].channel = i;
    cgt[i].gain = 0;
    cgt[i].nrse = 0;
    cgt[i].range = 0;
    cgt[i].ground = 0;
    cgt[i].pause = 0;
    cgt[i].dac1 = 0;
    cgt[i].dac2 = 0;
    cgt[i].skip = 0;
    dm75xx_status = DM75xx_CGT_Write(brd, cgt[i]);
    DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_CGT_Write");
  }
  
  /* set up clocks */
  dm75xx_status = DM75xx_BCLK_Setup(brd,
				    DM75xx_BCLK_START_PACER,
				    DM75xx_BCLK_FREQ_8_MHZ,
				    BURST_RATE, &actR);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_PCLK_Setup");
  dm75xx_status = DM75xx_PCLK_Setup(brd,
				    DM75xx_PCLK_INTERNAL,
				    DM75xx_PCLK_FREQ_8_MHZ,
				    DM75xx_PCLK_NO_REPEAT,
				    DM75xx_PCLK_START_SOFTWARE,
				    DM75xx_PCLK_STOP_SOFTWARE,
				    PACER_RATE, &actR);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_PCLK_Setup");
  
  /* Set ADC Conversion Signal Select */
  dm75xx_status =
    DM75xx_ADC_Conv_Signal(brd, DM75xx_ADC_CONV_SIGNAL_BCLK);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_ADC_Conv_Signal");
  
  /* Start the pacer clock */
  dm75xx_status = DM75xx_PCLK_Start(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_PCLK_Start");
  
  /* Read data into the FIFO */
  do {
    dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
    DM75xx_Exit_On_Error(brd, dm75xx_status,
			 (char *)"DM75xx_FIFO_Get_Status");
  }
  while (data & DM75xx_FIFO_ADC_NOT_FULL);
  
  /* Stop the pacer clock */
  dm75xx_status = DM75xx_PCLK_Stop(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_PCLK_Stop");
  
  /* Read out data from the FIFO */
  do {
    
    /* Reading the FIFO */
    for (i = 0; i < FIFO_DEPTH; i++) {
      for (j = 0; j < CHANNELS; j++) {
	dm75xx_status = DM75xx_ADC_FIFO_Read(brd, &data);
	DM75xx_Exit_On_Error(brd, dm75xx_status,
			     (char *)"DM75xx_ADC_FIFO_Read");
	acq_output->val[i][j] = ((DM75xx_ADC_ANALOG_DATA(data) / 4096.) * 10);

	/* Check the FIFO status each time */
	dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
	DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_FIFO_Get_Status");
      }
    }
  }
  while (data & DM75xx_FIFO_ADC_NOT_EMPTY);
  
  /* Print how many samples were received */
  clog << "info: " << logstream::info << "received " << (unsigned)(i * j) << " analog samples" << std::endl;

  /* Reset the board and close the device */
  dm75xx_status = DM75xx_Board_Reset(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Reset");
  dm75xx_status = DM75xx_Board_Close(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Close");
  
  return acq_output;
#endif
  return acq_output;
}

/* read out a hk packet */
HK_PACKET * DataAcqManager::AnalogPktReadOut(AnalogAcq * acq_output) {

  int i, k;
  float sum_ph[PH_CHANNELS];
  float sum_sipm1 = 0;
  HK_PACKET * hk_packet = new HK_PACKET();
  
  /* make the header of the hk packet and timestamp */
  hk_packet->hk_packet_header.header = BuildCpuPktHeader(HK_PACKET_TYPE, HK_PACKET_VER);
  hk_packet->hk_packet_header.pkt_size = sizeof(hk_packet);
  hk_packet->hk_time.cpu_time_stamp = BuildCpuTimeStamp();
  
  
  /* initialise */
  for(k = 0; k < PH_CHANNELS; k++) {
    sum_ph[k] = 0;
  }

  /* read out multiplexed sipm 64 values and averages of sipm 1 and photodiodes */
  for(i = 0; i < FIFO_DEPTH; i++) {
    sum_ph[0] += acq_output->val[i][0];
    sum_ph[1] += acq_output->val[i][1];
    sum_ph[2] += acq_output->val[i][2];
    sum_ph[3] += acq_output->val[i][3];
    sum_sipm1 += acq_output->val[i][4];
    hk_packet->sipm_data[i] = acq_output->val[i][5];
  }

  for (k = 0; k < PH_CHANNELS; k++) {
    hk_packet->photodiode_data[k] = sum_ph[k]/FIFO_DEPTH;
  }
  hk_packet->sipm_single = sum_sipm1/FIFO_DEPTH;

  return hk_packet;
}


/* write the cpu packet to the cpu file */
int DataAcqManager::WriteCpuPkt(ZYNQ_PACKET * zynq_packet, HK_PACKET * hk_packet) {

  CPU_PACKET * cpu_packet = new CPU_PACKET();
  static unsigned int pkt_counter = 0;

  clog << "info: " << logstream::info << "writing new packet to " << cpu_main_file_name << std::endl;
  /* create the cpu packet header */
  cpu_packet->cpu_packet_header.header = BuildCpuPktHeader(CPU_PACKET_TYPE, CPU_PACKET_VER);
  cpu_packet->cpu_packet_header.pkt_size = sizeof(*cpu_packet);
  cpu_packet->cpu_packet_header.pkt_num = pkt_counter; 
  cpu_packet->cpu_time.cpu_time_stamp = BuildCpuTimeStamp();
  hk_packet->hk_packet_header.pkt_num = pkt_counter;

  /* add the zynq and hk packets */
  cpu_packet->zynq_packet = *zynq_packet;
  delete zynq_packet;
  cpu_packet->hk_packet = *hk_packet;
  delete hk_packet;

  /* write the CPU packet */
  this->RunAccess->WriteToSynchFile<CPU_PACKET *>(cpu_packet);
  delete cpu_packet; 
  pkt_counter++;
  
  return 0;
}


/* write the sc packet to the cpu file */
int DataAcqManager::WriteScPkt(SC_PACKET * sc_packet) {

  static unsigned int pkt_counter = 0;

  clog << "info: " << logstream::info << "writing new packet to " << cpu_sc_file_name << std::endl;

  /* write the SC packet */
  this->RunAccess->WriteToSynchFile<SC_PACKET *>(sc_packet);
  delete sc_packet;
  pkt_counter++;
  
  return 0;
}

/* Look for new files in the data directory and process them */
int DataAcqManager::ProcessIncomingData(Config * ConfigOut, bool single_run) {
#ifndef __APPLE__
  int length, i = 0;
  int fd, wd;
  char buffer[BUF_LEN];

  std::string zynq_file_name;
  std::string sc_file_name;
  std::string data_str(DATA_DIR);
  std::string event_name;

  clog << "info: " << logstream::info << "starting background process of processing incoming data" << std::endl;
  
  /* watch the data directory for incoming files */
  fd = inotify_init();
  if (fd < 0) {
    clog << "error: " << logstream::error << "unable to start inotify service" << std::endl;
  }
  
  clog << "info: " << logstream::info << "start watching " << DONE_DIR << std::endl;
  wd = inotify_add_watch(fd, DATA_DIR, IN_CREATE);

  int packet_counter = 0;
  int bad_packet_counter = 0;
    
  while(1) {
    
    struct inotify_event * event;
    
    length = read(fd, buffer, BUF_LEN); 
    if (length < 0) {
      clog << "error: " << logstream::error << "unable to read from inotify file descriptor" << std::endl;
    } 
    
    event = (struct inotify_event *) &buffer[i];
    
    if (event->len) {
      if (event->mask & IN_CREATE) {
	if (event->mask & IN_ISDIR) {

	  /* process new directory creation */
	  printf("The directory %s was created\n", event->name);
	  clog << "info: " << logstream::info << "new directory created" << std::endl;

	}
	else {

	  /* process new file */
	  printf("The file %s was created\n", event->name);
	  clog << "info: " << logstream::info << "new file created with name " << event->name << std::endl;
	  event_name = event->name;
	  
	  /* for CPU run files */
	  if (event_name.compare(0, 3, "frm") == 0) {

	    /* new run file every RUN_SIZE packets*/
	    if (packet_counter == RUN_SIZE) {
	      CloseCpuRun(CPU);
	      packet_counter = 0;
	    }
	    if (packet_counter == 0) {
	      CreateCpuRun(CPU, ConfigOut);
	    }
	    
	    zynq_file_name = data_str + "/" + event->name;
	    sleep(2);
	      
	    /* generate sub packets */
	    ZYNQ_PACKET * zynq_packet = ZynqPktReadOut(zynq_file_name);
	    AnalogAcq * acq = AnalogDataCollect();
	    HK_PACKET * hk_packet = AnalogPktReadOut(acq);

	    /* check for NULL packets */
	    if (zynq_packet != NULL && hk_packet != NULL) {
	    
	      /* generate cpu packet and append to file */
	      WriteCpuPkt(zynq_packet, hk_packet);
	      
	      /* delete upon completion */
	      std::remove(zynq_file_name.c_str());
	      
	      /* increment the packet counter */
	      packet_counter++;
	      /* leave loop for a single run file */
	      if (packet_counter == 25 && single_run == true) {
		break;
	      }
	    }
	    else {
	      /* skip this packet */
	      bad_packet_counter++;
	    }

	  }
	  else if (event_name.compare(0, 2, "sc") == 0) {
	    
	    sc_file_name = data_str + "/" + event->name;
	    sleep(27);

	    CreateCpuRun(SC, ConfigOut);
	    
	    /* generate sc packet and append to file */
	    SC_PACKET * sc_packet = ScPktReadOut(sc_file_name, ConfigOut);
	    WriteScPkt(sc_packet);
	    
	    CloseCpuRun(SC);
	    
	    /* delete upon completion */
	    std::remove(sc_file_name.c_str());
	    
	    /* exit without waiting for more files */
	    return 0;
	    
	  }
	  else {
	    
	    /* do nothing and exit */
	    return 0;
	    
	  }
	  
	}
      }
    }
  }

  /* stop watching the directory */
  inotify_rm_watch(fd, wd);
  close(fd);
  return 0;
#endif /* #ifndef __APPLE__ */
  return 0;
}

/* spawn thread to collect an S-curve */
int DataAcqManager::CollectSc(Config * ConfigOut) {

  ZynqManager ZqManager;


  /* collect the data */
  std::thread collect_data (&DataAcqManager::ProcessIncomingData, this, ConfigOut, false);
  ZqManager.Scurve(ConfigOut->scurve_start, ConfigOut->scurve_step, ConfigOut->scurve_stop, ConfigOut->scurve_acc);
  collect_data.join();

  return 0;
}

/* spawn thread to collect data */
int DataAcqManager::CollectData(Config * ConfigOut, uint8_t instrument_mode, bool single_run) {

  ZynqManager ZqManager;

  /* collect the data */
  std::thread collect_data (&DataAcqManager::ProcessIncomingData, this, ConfigOut, single_run);

  switch(instrument_mode) {
  case ZynqManager::MODE0:
    ZqManager.SetInstrumentMode(ZynqManager::MODE0);
    break;
  case ZynqManager::MODE1:
    ZqManager.SetInstrumentMode(ZynqManager::MODE1);
    break;
  case ZynqManager::MODE2:
    ZqManager.SetInstrumentMode(ZynqManager::MODE2);
    break;
  case ZynqManager::MODE3:
    ZqManager.SetInstrumentMode(ZynqManager::MODE3);
    break;
  }
  collect_data.join();         

  /* never reached for infinite acquisition right now */
  ZqManager.SetInstrumentMode(ZynqManager::MODE0);
  CloseCpuRun(CPU);
  
  return 0;
}
