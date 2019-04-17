#include "OperationMode.h"

/**
 * constructor
 */
OperationMode::OperationMode() {
  this->_switch = false;
  this->_ftp = false;
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

  {
    std::unique_lock<std::mutex> lock(this->_m_ftp);   
    this->_ftp = true;
  } /* release mutex */
  this->_cv_ftp.notify_all();

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

  {
    std::unique_lock<std::mutex> lock(this->_m_ftp);   
    this->_ftp = false;
  } /* release mutex */
  
  /* also reset the analog and thermal switch */
  this->Analog->Reset();
  this->Thermistors->Reset();

  
}

/**
 * start the operational mode, 
 * virtual function for overloading
 */
void OperationMode::Start() {

}

