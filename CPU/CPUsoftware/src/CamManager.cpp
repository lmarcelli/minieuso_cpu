#include "CamManager.h"

/* default constructor */
CamManager::CamManager() {
  this->n_relaunch_attempt = 0;
  this->launch_failed = false;
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
    output = CpuTools::CommandToStr(CAMERA_EXEC_QUIET);   
  }
  
  size_t found = output.find("Error Trace:");
  if (found != std::string::npos) {
    clog << "error: " << logstream::error << "camera launch failed" << std::endl;

   
    std::cout << "ERROR: camera launch failed" << std::endl;

    found = output.find("*** BUS RESET ***");
    if (found != std::string::npos) {

      std::cout << "ERROR: cameras BUS RESET" << std::endl;

      /* signal launch failure */
      this->launch_failed = true;
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

  /* check camera verbosity */
  if (CmdLine->cam_verbose) {
    this->CManager->SetVerbose();
  }
  
  std::thread collect_cam_data (&CamManager::StartAcquisition, this->CManager);

  /* wait for launch to be checked / mutex to be release? */
  sleep(1);

  /* check if cameras failed to launch */
  if (this->launch_failed) {

    /* wait for thread to join */
    collect_cam_data.join();
    
    /* reset */
    this->launch_failed = false;
    
    /* exit */
    return 1;
  }
  
  /* store the handle */
  this->cam_thread_handle = collect_cam_data.native_handle();

  /* if launch OK, detach thread */
  collect_cam_data.detach();
  
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
