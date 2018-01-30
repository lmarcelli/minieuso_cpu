#include "DataReduction.h"

/* default constructor */
DataReduction::DataReduction() {
  this->inst_mode_switch = false;
  
}

/* launch thread to do data reduction */
int DataReduction::Start() {

  clog << "info: " << logstream::info << "starting data reduction" << std::endl;
  std::cout << "starting data reduction" << std::endl;
  
  /* launch thread */
  std::thread data_reduction (&DataReduction::RunDataReduction, this);
  
  std::cout << "try to join thread" << std::endl;
  /* wait for thread to exit, when instrument mode switches */
  if (data_reduction.joinable){
    data_reduction.join();
  }
  
  return 0;
}

/* data reduction procedure */
int DataReduction::RunDataReduction() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);
    if(!this->inst_mode_switch) {
      std::cout << "inst_mode_switch is false, as expected" << std::endl;
    }
  }
  
  std::unique_lock<std::mutex> lock(this->m_mode_switch); 
  /* enter loop while instrument mode switching not requested */
  while(!this->cv_mode_switch.wait_for(lock,
				       std::chrono::milliseconds(WAIT_PERIOD),
				       [this] { return this->inst_mode_switch; } )) { 

    
    /* add data reduction procedure here */
    std::cout << "daytime work..." << std::endl; 

    /* for now just sleep */
    sleep(5);
  }
  
  
  return 0;
}
