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
  /* for debug just print */
  std::cout << output << std::endl;

  /* make the results struct */
  TemperatureAcq * temperature_result = NULL;
  
  return temperature_result;
}
