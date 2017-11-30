#include "ThermManager.h"

/* default constructor */
ThermManager::ThermManager(std::shared_ptr<SynchronisedFile> CpuFile) { 
  /* get the cpu run file shared pointer */
  this->CpuFile = CpuFile;
}

/* get the temperature */
TemperatureAcq * ThermManager::ThermManager() {
  /* run the digitemp command */
  std::string output = CpuTools::
  
}
