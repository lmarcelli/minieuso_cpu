#ifndef _DATA_REDUCTION_H
#define _DATA_REDUCTION_H

#include <thread>
#include <unistd.h>

#include "log.h"

/* for use with conditional variable */
#define LONG_PERIOD 36000 /* 10 hours */

/* class to handle data reduction in daytime */
class DataReduction {
public:
  /* handle mode switching signal from RunInstrument::MonitorLightLevel */
  bool inst_mode_switch;
  std::condition_variable cv_mode_switch;
  std::mutex m_mode_switch;
  
  DataReduction();
  int Start();
  
private:
  int RunDataReduction();
  
};

#endif
/* DATA_REDUCTION_H */
