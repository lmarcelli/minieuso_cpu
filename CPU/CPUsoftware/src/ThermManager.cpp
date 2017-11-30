#include "ThermManager.h"

/* default constructor */
ThermManager::ThermManager(std::shared_ptr<SynchronisedFile> CpuFile) { 

  /* get the cpu run file shared pointer */
  this->CpuFile = CpuFile;
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
    //std::cout << (searchStart == input_string.cbegin() ? "" : " " ) << match[0];
    
    /* fill the results for even values only (ignore Fahrenheit results) */
    if (i % 2 == 0) {
      temperature_results->val[j] = std::stof(match[0]);
      std::cout << std::stof(match[0]) << std::endl;
      j++;
    }
    
    i++;
    searchStart += match.position() + match.length();
  }
    
  return temperature_results;
}
