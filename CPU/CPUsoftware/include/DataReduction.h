#ifndef _DATA_REDUCTION_H
#define _DATA_REDUCTION_H

#include <thread>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

#include "log.h"
#include "AnalogManager.h"
#include "ThermManager.h"

/* for use with conditional variable */
#define WAIT_PERIOD 1 /* milliseconds */

/* class to handle data reduction in DAY mode */
class DataReduction {
public:
  /* subsystems controlled */
  ThermManager * ThManager = new ThermManager();
  AnalogManager * Analog = new AnalogManager();
  
  DataReduction();
  int Start();

  /* handle instrument mode switching */
  int NotifySwitch();
  int ResetSwitch();
  
private:
  bool inst_mode_switch;
  std::condition_variable cv_mode_switch;
  std::mutex m_mode_switch;

  int RunDataReduction();
  
};

#endif
/* DATA_REDUCTION_H */
