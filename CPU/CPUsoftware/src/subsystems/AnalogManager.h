#ifndef _ANALOG_MANAGER_H
#define _ANALOG_MANAGER_H

#ifndef __APPLE__
#include "dm75xx_library.h"
#endif /* __APPLE__ */

#include <mutex>
#include <memory>
#include <thread>
#include <unistd.h>
#include <condition_variable>

#include "log.h"
#include "data_format.h"

/* for use with analog readout functions */
#define CHANNELS 16
#define FIFO_DEPTH 64
#define BURST_RATE 1000000
#define PACER_RATE 100000

/* light threshold for photodiodes */
/* used to determine instrument mode via CompareLightLevel */
#define LIGHT_THRESHOLD 0

/* number of seconds between light level polling */
#define LIGHT_POLL_TIME 2

/* seconds between data collection */
#define LIGHT_ACQ_TIME 2

/* for use with conditional variable */
#define WAIT_PERIOD 1 /* milliseconds */


/* acquisition structure for analog readout */
typedef struct {
  float val [FIFO_DEPTH][CHANNELS];
} AnalogAcq;

/* struct to hold average light levels for polling */
typedef struct {
  float photodiode_data[N_CHANNELS_PHOTODIODE];  
  float sipm_data[N_CHANNELS_SIPM]; 
  float sipm_single; 
} LightLevel;

/* class to handle the analog data acquisition (photodiodes and SiPMs) */
class AnalogManager {
public:
  AnalogManager();
  std::shared_ptr<LightLevel> ReadLightLevel();
  bool CompareLightLevel();
  int ProcessAnalogData();  
  int GetLightLevel();

  /* handle instrument mode switching */
  int Notify();
  int Reset();
  
private:
  std::mutex m_light_level;
  std::shared_ptr<LightLevel> light_level;
  std::shared_ptr<AnalogAcq> analog_acq;

  bool inst_mode_switch;
  std::mutex m_mode_switch;
  std::condition_variable cv_mode_switch;

  int AnalogDataCollect();
};

#endif
/* _ANALOG_MANAGER_H */
