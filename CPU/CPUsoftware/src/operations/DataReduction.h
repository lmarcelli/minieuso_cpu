#ifndef _DATA_REDUCTION_H
#define _DATA_REDUCTION_H

#include <thread>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

#include "log.h"
#include "OperationMode.h"
#include "AnalogManager.h"
#include "ThermManager.h"

/* for use with conditional variable */
#define WAIT_PERIOD 1 /* milliseconds */


/* DAY operational mode: data reduction */
/* class to handle data reduction */
class DataReduction : public OperationMode {
public:
  
  DataReduction();
  void Start();
  
private:

  int RunDataReduction();
  
};

#endif
/* DATA_REDUCTION_H */
