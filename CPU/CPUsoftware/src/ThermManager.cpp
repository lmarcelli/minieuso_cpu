#include "ThermManager.h"

/* default constructor */
ThermManager::ThermManager() { 
}

/* build the cpu packet header */
uint32_t ThermManager::BuildCpuPktHeader(uint32_t type, uint32_t ver) {

  uint32_t header;
  header =  (('P'<<24) | (INSTRUMENT_ME_PDM<<16) | ((type)<<8) | (ver));
 
  return header;
}

/* build the cpu timestamp */
uint32_t ThermManager::BuildCpuTimeStamp() {

  uint32_t timestamp;
  struct timeval tv; 
  gettimeofday(&tv, 0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  
  timestamp = ( ((now_tm->tm_year + 1900 - 2017) << 26) | ((now_tm->tm_mon) << 22) | ((now_tm->tm_mday) << 17) | ((now_tm->tm_hour) << 12) | ((now_tm->tm_min) << 6) | (now_tm->tm_sec));

  return timestamp;
}


/* get the temperature */
TemperatureAcq * ThermManager::GetTemperature() {

  /* define command to read temperature from all thermistors on COM port 1 */
  const char * cmd = "digitemp -s /dev/ttyS0 -a";

  /* run the digitemp command */
  std::string output = CpuTools::CommandToStr(cmd);
  
  /* parse the output */
  TemperatureAcq * temperature_result = ParseDigitempOutput(output);
    
  return temperature_result;
}


/* parse the digitemp output */
TemperatureAcq * ThermManager::ParseDigitempOutput(std::string input_string) {

  std::regex num_with_two_dp("([0-9]+\\.[0-9]{2})");
  std::smatch match;
  TemperatureAcq * temperature_results = new TemperatureAcq();

  /* search for numbers with 2 decimal places */ 
  std::string::const_iterator searchStart(input_string.cbegin());
  int i = 0;
  int j = 0;
  while (std::regex_search(searchStart, input_string.cend(), match, num_with_two_dp)) {

    /* fill the results for even values only (ignore Fahrenheit results) */
    if (i % 2 == 0) {
      temperature_results->val[j] = std::stof(match[0]);
      std::cout << std::stof(match[0]) << std::endl;
      /* ADD: throw exception if no readout/sensors not connected */
      j++;
    }
    
    i++;
    searchStart += match.position() + match.length();
  }
    
  return temperature_results;
}

/* write the temperature packet to file */
int ThermManager::WriteThermPkt(TemperatureAcq * temperature_results) {

  THERM_PACKET * therm_packet = new THERM_PACKET();
  static unsigned int pkt_counter = 0;
  
  clog << "info: " << logstream::info << "writing new packet to " << this->RunAccess->path << std::endl;
  /* create the therm packet header */
  therm_packet->therm_packet_header.header = BuildCpuPktHeader(THERM_PACKET_TYPE, THERM_PACKET_VER);
  therm_packet->therm_packet_header.pkt_size = sizeof(*therm_packet);
  therm_packet->therm_packet_header.pkt_num = pkt_counter; 
  therm_packet->therm_time.cpu_time_stamp = BuildCpuTimeStamp();

  /* get the temperature data */
  for (int i = 0; i < N_CHANNELS_THERM; i++) {
    therm_packet->therm_data[i] = temperature_results->val[i];
  }
  delete temperature_results;
  
  /* DEBUG: print to check */
  std::cout <<  "therm_data[0]: " << therm_packet->therm_data[0] << std::endl;

  /* write the therm packet */
  //this->RunAccess->WriteToSynchFile<THERM_PACKET *>(therm_packet);
  delete therm_packet; 
  pkt_counter++;

  return 0;
}


int ThermManager::ProcessThermData() {

  /* start infinite loop */
  while(1) {
    
    /* collect data */
    TemperatureAcq * temperature_results = GetTemperature();
    
    /* write to file */
    if (temperature_results != NULL) {
      WriteThermPkt(temperature_results);
    }
    
    /* sleep */
    sleep(THERM_ACQ_SLEEP);
    
  }
  
  /* never reached */
  return 0;
}
