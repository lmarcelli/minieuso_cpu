#include "ThermManager.h"

/* default constructor */
ThermManager::ThermManager() { 

  this->cpu_file_is_set = false;
  
}

/* initialise the thermistors */
void ThermManager::Init() {

  const char * cmd = "digitemp -s /dev/ttyS0 -i";
  std::string output = CpuTools::CommandToStr(cmd);

}

/* build the cpu packet header */
uint32_t ThermManager::BuildCpuPktHeader(uint32_t type, uint32_t ver) {

  uint32_t header;
  header =  (('P'<<24) | (INSTRUMENT_ME_PDM<<16) | ((type)<<8) | (ver));
 
  return header;
}

/* build the cpu timestamp */
uint32_t ThermManager::BuildCpuTimeStamp() {

  uint32_t timestamp = time(NULL);

  return timestamp;
}


/* get the temperature */
TemperatureAcq * ThermManager::GetTemperature() {
 
  /* define command to read temperature from all thermistors on COM port 1 */
  const char * cmd = "digitemp -s /dev/ttyS0 -a";

  /* run the digitemp command */
  std::string output = CpuTools::CommandToStr(cmd);
  
    
  size_t found = output.find("Error 10:");
  if (found != std::string::npos) {
    clog << "error: " << logstream::error << "cannot connect to temprature sensors" << std::endl;
    TemperatureAcq * temperature_result = NULL;
  }
  else {
    /* parse the output */
    TemperatureAcq * temperature_result = ParseDigitempOutput(output);
  }
  
  return temperature_result;
}

/* print the temperature */
void ThermManager::PrintTemperature() {
 
  /* define command to read temperature from all thermistors on COM port 1 */
  const char * cmd = "digitemp -s /dev/ttyS0 -a";

  /* run the digitemp command */
  std::string output = CpuTools::CommandToStr(cmd);
  
 size_t found = output.find("Error 10:");
 if (found != std::string::npos) {
   clog << "error: " << logstream::error << "cannot connect to temprature sensors" << std::endl;
 }
 else {
   /* print the output */
   std::cout << output << std::endl;
 }
 
}


/* parse the digitemp output */
TemperatureAcq * ThermManager::ParseDigitempOutput(std::string input_string) {

  std::regex num_with_two_dp("([0-9]+\\.[0-9]{2})");
  std::smatch match;
  TemperatureAcq * temperature_result = new TemperatureAcq();

  /* search for numbers with 2 decimal places */ 
  std::string::const_iterator searchStart(input_string.cbegin());
  int i = 0;
  int j = 0;
  while (std::regex_search(searchStart, input_string.cend(), match, num_with_two_dp)) {

    /* fill the results for even values only (ignore Fahrenheit results) */
    if (i % 2 == 0) {
      temperature_result->val[j] = std::stof(match[0]);
      j++;
    }
    
    i++;
    searchStart += match.position() + match.length();
  }
    
  return temperature_result;
}

/* write the temperature packet to file */
int ThermManager::WriteThermPkt(TemperatureAcq * temperature_result) {

  THERM_PACKET * therm_packet = new THERM_PACKET();
  static unsigned int pkt_counter = 0;
  
  clog << "info: " << logstream::info << "writing new packet to " << this->RunAccess->path << std::endl;
  /* create the therm packet header */
  therm_packet->therm_packet_header.header = BuildCpuPktHeader(THERM_PACKET_TYPE, THERM_PACKET_VER);
  therm_packet->therm_packet_header.pkt_size = sizeof(*therm_packet);
  therm_packet->therm_packet_header.pkt_num = pkt_counter; 
  therm_packet->therm_time.cpu_time_stamp = BuildCpuTimeStamp();

  if (temperature_result != NULL) {
    /* get the temperature data */
    for (int i = 0; i < N_CHANNELS_THERM; i++) {
      therm_packet->therm_data[i] = temperature_result->val[i];
    }
  }
  delete temperature_result;
  
  /* write the therm packet */
  this->RunAccess->WriteToSynchFile<THERM_PACKET *>(therm_packet, SynchronisedFile::CONSTANT);
  delete therm_packet; 
  pkt_counter++;

  return 0;
}


int ThermManager::ProcessThermData() {

  std::mutex m;
  
  /* start infinite loop */
  while(1) {
    
    /* collect data */
    TemperatureAcq * temperature_result = GetTemperature();

    
    /* wait for CPU file to be set by DataAcqManager::ProcessIncomingData() */
    std::unique_lock<std::mutex> lock(m);
    this->cond_var.wait(lock, [this]{return cpu_file_is_set == true;});
    
    
    /* write to file */
    if (temperature_result != NULL) {
      WriteThermPkt(temperature_result);
    }
    
    /* sleep */
    sleep(THERM_ACQ_SLEEP);
    
  }
  
  /* never reached */
  return 0;
}
