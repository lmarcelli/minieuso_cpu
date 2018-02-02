#include "OperationMode.h"

/* default constructor */
OperationMode::OperationMode() {
  this->_switch = false;
  this->_stop =false;
}



/* notify the object of a mode switch */
void OperationMode::Notify() {

  {
    std::unique_lock<std::mutex> lock(this->_m_switch);   
    this->_switch = true;
  } /* release mutex */
  this->_cv_switch.notify_all();

  /* also notify the analog acquisition */
  this->Analog->NotifySwitch();

}

/* reset the object's mode switch */
void OperationMode::Reset() {

  {
    std::unique_lock<std::mutex> lock(this->_m_switch);   
    this->_switch = false;
  } /* release mutex */

  /* also reset the analog switch */
  this->Analog->ResetSwitch();

  
}

/* start the operational mode - launch all processes */
void OperationMode::Start() {

}

/* stop the operational mode - exit all processes */
void OperationMode::Stop() {

  {
    std::unique_lock<std::mutex> lock(this->_m_stop);   
    this->_stop = true;
  } /* release mutex */
  this->_cv_stop.notify_all();
    
}
