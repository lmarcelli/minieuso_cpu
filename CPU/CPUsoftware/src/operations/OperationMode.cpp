#include "OperationMode.h"

/**
 * constructor
 */
OperationMode::OperationMode() {
  this->_switch = false;
}



/**
 * notify the object of a mode switch
 */
void OperationMode::Notify() {

  {
    std::unique_lock<std::mutex> lock(this->_m_switch);   
    this->_switch = true;
  } /* release mutex */
  this->_cv_switch.notify_all();

  /* also notify the analog and thermal acquisition */
  this->Analog->Notify();
  this->Thermistors->Notify();

}

/**
 * reset the objects mode switch
 */
void OperationMode::Reset() {

  {
    std::unique_lock<std::mutex> lock(this->_m_switch);   
    this->_switch = false;
  } /* release mutex */

  /* also reset the analog switch */
  this->Analog->Reset();
  this->Thermistors->Notify();

  
}

/**
 * start the operational mode, 
 * virtual function for overloading
 */
void OperationMode::Start() {

}

