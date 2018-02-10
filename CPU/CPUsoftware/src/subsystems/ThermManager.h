#ifndef _THERM_MANAGER_H
#define _THERM_MANAGER_H

#include <regex>
#include <condition_variable>

#include "log.h"
#include "data_format.h"
#include "CpuTools.h"
#include "SynchronisedFile.h"


/* number of seconds between temperature acquisitions */
#define THERM_ACQ_SLEEP 60

/**
 * acquisition structure for temperature readout 
 */
typedef struct
{
  float val [N_CHANNELS_THERM];
} TemperatureAcq;


/**
 * controls the thermistor acquisition 
 */
class ThermManager {
public:
  /*
   * SynchronisedFile access
   */
  Access * RunAccess;
  /*
   * to wait for CPU file to be created by DataAcquisition::CreateCpuRun
   */
  std::condition_variable cond_var;
  /*
  * to notify that the CPU file is set by DataAcquisition::CreateCpuRun
  */
  bool cpu_file_is_set;

  ThermManager();
  void Init();
  int ProcessThermData();
  TemperatureAcq * GetTemperature();
  int WriteThermPkt(TemperatureAcq * temperature_results);
  void PrintTemperature();
  
private:
  TemperatureAcq * ParseDigitempOutput(std::string input_string);
  uint32_t BuildCpuPktHeader(uint32_t type, uint32_t ver);
  uint32_t BuildCpuTimeStamp();
  
};

#endif /* _THERM_MANAGER_H */
