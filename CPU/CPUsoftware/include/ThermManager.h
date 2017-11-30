#ifndef _THERM_MANAGER_H
#define _THERM_MANAGER_H

#include "SynchronisedFile.h"

/* deinfe the number of channels */
#deinfe N_THERMISTOR 10

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

  ThermManager();
  int WriteThermPkt();
  
private:
  TemperatureAcq * GetTemperature();
};

#endif/* _THERM_MANAGER_H */
