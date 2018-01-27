#include "CamManager.h"

/* default constructor */
CamManager::CamManager() {
  this->n_relaunch_attempt = 0;
}

int CamManager::SetVerbose() {
  this->verbose = true;

  return 0;
}

/* start acquisition */
int CamManager::StartAcquisition() {

  /* launch and check output */
  std::string output;
  if (this->verbose) {
    output = CpuTools::CommandToStr(CAMERA_EXEC);
    std::cout << output << std::endl;
  }
  else {
    output = CpuTools::CommandToStr(CAMERA_EXEC);   
  }
  
  size_t found = output.find("Error Trace:");
  if (found != std::string::npos) {
    clog << "error: " << logstream::error << "camera launch failed" << std::endl;

   
    std::cout << "ERROR: camera launch failed" << std::endl;

    found = output.find("*** BUS RESET ***");
    if (found != std::string::npos) {

      std::cout << "ERROR: cameras BUS RESET" << std::endl;

      /* signal launch failure */
      this->launch_failed.set_value(true);
      return 1;	
    } 
  }

 return 0;
}

/* spawn thread to launch the camera software */
int CamManager::CollectData() {

 /* launch the camera software */
  std::cout << "starting camera acquisition in the background..." << std::endl;
  clog << "info: " << logstream::info << "starting camera acquisition" << std::endl; 

  auto future = this->launch_failed.get_future();
  std::thread collect_cam_data (&CamManager::StartAcquisition, this);

  /* wait for launch to be marked as failed by CamManager::StartAcquisition() */
  auto status = future.wait_for(std::chrono::seconds(5));   

  /* check if cameras failed to launch */
  if (status == std::future_status::ready) {

    /* wait for thread to join */
    collect_cam_data.join();
    
    /* clear the promise */
    this->launch_failed = std::promise<bool>();

    return 1;
  }
  else {

    /* if launch OK, detach thread */
    collect_cam_data.detach();
    this->cam_thread_handle = collect_cam_data.native_handle();
  }
  
  
  return 0;
}

/* kill the data collection thread */
int CamManager::KillCamAcq() {

  clog << "info: " << logstream::info << "killing the camera acquisition" << std::endl;

  /* kill the thread */
  /* justifiable as no locked resources */
  pthread_cancel(this->cam_thread_handle);
  
  return 0;
}
