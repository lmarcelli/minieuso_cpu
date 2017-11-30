#ifndef _THERM_MANAGER_H
#define _THERM_MANAGER_H

#include <regex>

#include "log.h"
#include "data_format.h"
#include "CpuTools.h"
#include "SynchronisedFile.h"

/* number of seconds between temperature acquisitions */
#define THERM_ACQ_SLEEP 60

/* acquisition structure for temperature readout */
typedef struct
{
  float val [N_CHANNELS_THERM];
} TemperatureAcq;


/* class to control the thermistor acquisition */
class ThermManager {
public:
  std::shared_ptr<SynchronisedFile> CpuFile;
  Access * RunAccess;

  ThermManager();
  int ProcessThermData();
  
private:
  TemperatureAcq * ParseDigitempOutput(std::string input_string);
  TemperatureAcq * GetTemperature();
  int WriteThermPkt(TemperatureAcq * temperature_results);
  uint32_t BuildCpuPktHeader(uint32_t type, uint32_t ver);
  uint32_t BuildCpuTimeStamp();


  
};

#endif/* _THERM_MANAGER_H */
