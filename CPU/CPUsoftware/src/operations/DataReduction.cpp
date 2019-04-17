#include "DataReduction.h"

/** 
 * constructor 
 */
DataReduction::DataReduction() {
  
}

/**
 * launch thread to do data reduction 
 */
void DataReduction::Start() {

  clog << "info: " << logstream::info << "starting data reduction" << std::endl;
  std::cout << "starting data reduction" << std::endl;

  this->ConfigOut = std::make_shared<Config>();
  
  /* launch thread */
  std::thread data_reduction (&DataReduction::RunDataReduction, this);

  /* launch the analog acquisition */
  std::thread analog(&ArduinoManager::ProcessAnalogData, this->Analog, ConfigOut); 
  analog.join();

  
  /* wait for thread to exit, when instrument mode switches */
  data_reduction.join();
  
  return;
}

/**
 * data reduction procedure 
 * @TODO currently just sleeps! add main procedure
 */
int DataReduction::RunDataReduction() {

  {
    std::unique_lock<std::mutex> lock(this->_m_switch);
  }
  
  std::unique_lock<std::mutex> lock(this->_m_switch); 

  /* enter loop while instrument mode switching not requested */
  while(!this->_cv_switch.wait_for(lock,
				   std::chrono::milliseconds(WAIT_PERIOD),
				   [this] { return this->_switch; } )) {   
    
    /* add data reduction procedure here */
    std::cout << "daytime work..." << std::endl; 
    
    /* for now just sleep */
    sleep(5);
  }
  
  return 0;
}

