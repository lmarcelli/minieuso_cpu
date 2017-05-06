#include "globals.h"

/* acquisition structure for analog readout */
typedef struct
{
  float val[1024];
} acq;


/* create log file name */
std::string CreateLogname(void) {
  struct timeval tv;
  char logname[80];
  std::string log_dir(LOG_DIR);
  std::string time_str("/CPU_MAIN__%Y_%m_%d__%H_%M_%S.log");
  std::string log_str = log_dir + time_str;
  const char * kLogCh = log_str.c_str();
  
  gettimeofday(&tv,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);

  strftime(logname, sizeof(logname), kLogCh, now_tm);
  std::cout << "Logname: " << logname << std::endl;
  return logname;
}

/* copy a file */
bool CopyFile(const char *SRC, const char* DEST) {
  std::ifstream src(SRC, std::ios::binary);
  std::ofstream dest(DEST, std::ios::binary);
  dest << src.rdbuf();
  return src && dest;
}

/* handle SIGINT */
void SignalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received" << std::endl;
  std::cout << "Stopping the acquisition" << std::endl;
  
  /* handle the signal*/
  DataAcquisitionStop();
  std::cout << "Acquisition stopped" << std::endl;  
  
  /* terminate the program */
  exit(signum);  
}

/* create cpu run file name */
std::string CreateCpuRunName(void) {
  struct timeval tv;
  char cpu_file_name[80];
  std::string done_str(DONE_DIR);
  std::string time_str("/CPU_RUN__%Y_%m_%d__%H_%M_%S.dat");
  std::string cpu_str = done_str + time_str;
  const char * kCpuCh = cpu_str.c_str();

  gettimeofday(&tv,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  
  strftime(cpu_file_name, sizeof(cpu_file_name), kCpuCh, now_tm);
  return cpu_file_name;
}


/* make a cpu data file for a new run */
int CreateCpuRun(std::string cpu_file_name) {

  FILE * ptr_cpufile;
  const char * kCpuFileName = cpu_file_name.c_str();
  CpuFileHeader cpu_file_header;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "creating a new cpu run file called " << cpu_file_name << std::endl;


  /* set up the cpu file structure */
  cpu_file_header.header = 111;
  cpu_file_header.run_size = RUN_SIZE;

  /* open the cpu run file */
  ptr_cpufile = fopen(kCpuFileName, "wb");
  if (!ptr_cpufile) {
    clog << "error: " << logstream::error << "cannot open the file " << cpu_file_name << std::endl;
    return 1;
  }

  /* write to the cpu run file */
  fwrite(&cpu_file_header, sizeof(cpu_file_header), 1, ptr_cpufile);

  /* close the cpu run file */
  fclose(ptr_cpufile);
  
  return 0;
}

/* Look for new files in the data directory and process them */
void ProcessIncomingData(std::string cpu_file_name) {

  int length, i = 0;
  int fd;
  int wd;
  char buffer[BUF_LEN];

  std::string zynq_file_name;
  std::string data_str(DATA_DIR);
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "starting background process of processing incoming data" << std::endl;

  /* watch the data directory for incoming files */
  fd = inotify_init();
  if (fd < 0) {
    clog << "error: " << logstream::error << "unable to start inotify service" << std::endl;
  }
  
  clog << "info: " << logstream::info << "start watching " << DONE_DIR << std::endl;
  wd = inotify_add_watch(fd, DATA_DIR, IN_CREATE);
  
  while(1) {
    
    struct inotify_event * event;
    
    length = read(fd, buffer, BUF_LEN); 
    if (length < 0) {
      clog << "error: " << logstream::error << "unable to read from inotify file descriptor" << std::endl;
    } 
    
    event = (struct inotify_event *) &buffer[i];
    
    if (event->len) {
      if ( event->mask & IN_CREATE ) {
	if ( event->mask & IN_ISDIR ) {
	  /* process new directory creation */
	  printf( "The directory %s was created\n", event->name);
	  clog << "info: " << logstream::info << "new directory created" << std::endl;

	}
	else {

	  /* process new file */
	  printf( "The file %s was created\n", event->name);
	  clog << "info: " << logstream::info << "new file created with name" << event->name << std::endl;
	  zynq_file_name = data_str + "/" + event->name;
    	  clog << "info: " << logstream::info << "path of file " << event->name << std::endl;

	  usleep(100000);
	  ZynqFileReadOut(zynq_file_name, cpu_file_name);

	  /* delete upon completion */
	  std::remove(zynq_file_name.c_str());
	  
	}
      }
    }
  }
  
  inotify_rm_watch(fd, wd);
  close(fd);

}

/* read out a zynq data file and append to a cpu data file */
int ZynqFileReadOut(std::string zynq_file_name, std::string cpu_file_name) {

  FILE * ptr_zfile;
  FILE * ptr_cpufile;
  Z_DATA_TYPE_SCI_POLY_V5 zynq_data_file;
  CPU_PACKET cpu_packet;
  const char * kZynqFileName = zynq_file_name.c_str();
  const char * kCpuFileName = cpu_file_name.c_str();
  size_t res;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "reading out the file " << zynq_file_name << " and appending to " << cpu_file_name << std::endl;

  ptr_zfile = fopen(kZynqFileName, "rb");
  if (!ptr_zfile) {
    clog << "error: " << logstream::error << "cannot open the file " << zynq_file_name << std::endl;
    return 1;
  }
  
  /* read out the zynq structure, defined in "pdmdata.h" */
  res = fread(&zynq_data_file, sizeof(zynq_data_file), 1, ptr_zfile);
  if (res != 0) {
    clog << "error: " << logstream::error << "fread from " << zynq_file_name << " failed" << std::endl;
    return 1;   
  }
  
  /* DEBUG: print records to check */
  printf("header = %u\n", zynq_data_file.zbh.header);
  printf("payload_size = %u\n", zynq_data_file.zbh.payload_size);
  printf("hv_status = %u\n", zynq_data_file.payload.hv_status);
  printf("n_gtu = %lu\n", zynq_data_file.payload.ts.n_gtu);

  /* close the zynq file */
  fclose(ptr_zfile);

  /* create the cpu packet from the zynq file */
  cpu_packet.cpu_packet_header.header = 888;
  cpu_packet.cpu_packet_header.pkt_size = 777;
  cpu_packet.cpu_time.cpu_time_stamp = 666;
  cpu_packet.zynq_packet = zynq_data_file;
  
  /* open the cpu file to append */
  ptr_cpufile = fopen(kCpuFileName, "a+b");
  if (!ptr_cpufile) {
    clog << "error: " << logstream::error << "cannot open the file " << zynq_file_name << std::endl;
    return 1;
  }

  /* write the cpu packet */
  fwrite(&cpu_packet, sizeof(cpu_packet), 1, ptr_cpufile);
  
  /* close the cpu file */
  fclose(ptr_cpufile);
  
  return 0;
}

/* analog board read out */
int AnalogReadOut() {

  return 0;
}

/* photodiode test code */
int PhotodiodeTest() {
  DM75xx_Board_Descriptor *brd;
  DM75xx_Error dm75xx_status;
  dm75xx_cgt_entry_t cgt[CHANNELS];
  int i, k;
  float actR;
  uint16_t data = 0x0000;  
  unsigned long int minor_number = 0;
 
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "running the photodiode test code" << std::endl;

  /* Device initialisation */
  clog << "info: " << logstream::info << "initialising the DM75xx device" << std::endl;
  dm75xx_status = DM75xx_Board_Open(minor_number, &brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Open");
  dm75xx_status = DM75xx_Board_Init(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Init");
  
  /* Main acquisition code */
  k = 0;
  for( ; ; ) { 
    acq acq_output;
    char fname[64];
    snprintf(fname, sizeof(char) * 64, "/home/minieusouser/DATA/output%i.dat", k);  
    
    /* Clear the FIFO */
    clog << "info: " << logstream::info << "clearing the DM75xx FIFO" << std::endl;
    dm75xx_status = DM75xx_ADC_Clear(brd);
    DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Clear_AD_FIFO");
    dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
    DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_FIFO_Get_Status");
    printf("FIFO Status: 0x%4x\n", data);
    
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
    printf("Collecting data until FIFO is full\n");
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
    i = 0;
    do {
      
      /* Reading the FIFO */
      dm75xx_status = DM75xx_ADC_FIFO_Read(brd, &data);
      DM75xx_Exit_On_Error(brd, dm75xx_status,
			   (char *)"DM75xx_ADC_FIFO_Read");
      printf("%2.2f\n",
      	((DM75xx_ADC_ANALOG_DATA(data) / 4096.) * 10));
      acq_output.val[i]=((DM75xx_ADC_ANALOG_DATA(data) / 4096.) * 10);
      
      i++;
      
      /* Check the FIFO status each time */
      dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
      DM75xx_Exit_On_Error(brd, dm75xx_status,
			   (char *)"DM75xx_FIFO_Get_Status");
    }
    while (data & DM75xx_FIFO_ADC_NOT_EMPTY);

    /* Print how many samples were received */
    clog << "info: " << logstream::info << "received " << i << "analog samples" << std::endl;

    /* Save FIFO output to a file */	
    FILE * file = fopen(fname,"wb");
    if (file != NULL){
      fwrite(&acq_output, sizeof(acq_output), 1, file);
      fclose(file);
    }

    k++;
    sleep(5);
  }
  
  /* Program clean up */
  clog << "info: " << logstream::info << "analog acquisition completed" << std::endl;

  /* Reset the brd */
  dm75xx_status = DM75xx_Board_Reset(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Reset");
	
  /* Close the device */
  dm75xx_status = DM75xx_Board_Close(brd);
  DM75xx_Exit_On_Error(brd, dm75xx_status, (char *)"DM75xx_Board_Close");
  exit(EXIT_SUCCESS);
}
