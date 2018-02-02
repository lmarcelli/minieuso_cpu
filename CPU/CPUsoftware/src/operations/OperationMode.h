#ifndef _OPERATION_MODE_H
#define _OPERATION_MODE_H

#include <mutex>
#include <condition_variable>

#include "AnalogManager.h"

/* base class for an operational mode */
class OperationMode {
public:
  /* controlled subsystem */
  AnalogManager * Analog = new AnalogManager();

  OperationMode();

  /* operation control functions */
  virtual void Start();
  void Stop();
  
  void Notify();
  void Reset();

protected:
  /* to handle switching */
  std::mutex _m_switch;
  std::condition_variable _cv_switch;
  bool _switch;

  /* to handle stopping */
  std::mutex _m_stop;
  std::condition_variable _cv_stop;
  bool _stop;
  
};


#endif
/* _OPERATION_MODE_H */
