#ifndef _THERM_MANAGER_H
#define _THERM_MANAGER_H

#include <regex>

#include "CpuTools.h"
#include "SynchronisedFile.h"

/* deinfe the number of channels */
#define N_THERMISTOR 10

/* acquisition structure for temperature readout */
typedef struct
{
  float val [N_THERMISTOR];
} TemperatureAcq;


/* class to control the thermistor acquisition */
class ThermManager {
public:
  std::shared_ptr<SynchronisedFile> CpuFile;
  Access * RunAccess;

  ThermManager(std::shared_ptr<SynchronisedFile> CpuFile);
  int WriteThermPkt();
  TemperatureAcq * GetTemperature();
  
private:
  float ParseDigitempOutput(std::string input_string);

};

#endif/* _THERM_MANAGER_H */
