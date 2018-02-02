#include "DataReduction.h"

/* default constructor */
DataReduction::DataReduction() {
  
}

/* launch thread to do data reduction */
void DataReduction::Start() {

  clog << "info: " << logstream::info << "starting data reduction" << std::endl;
  std::cout << "starting data reduction" << std::endl;
  
  /* launch thread */
  std::thread data_reduction (&DataReduction::RunDataReduction, this);

  /* launch the analog acquisition */
  std::thread analog (&AnalogManager::ProcessAnalogData, this->Analog);
  analog.join();
  
  /* wait for thread to exit, when instrument mode switches */
  data_reduction.join();
  
  return;
}

/* data reduction procedure */
int DataReduction::RunDataReduction() {

  {
    std::unique_lock<std::mutex> lock(this->_m_switch);
    if(!this->_switch) {
      std::cout << "_switch is false, as expected" << std::endl;
    }
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

