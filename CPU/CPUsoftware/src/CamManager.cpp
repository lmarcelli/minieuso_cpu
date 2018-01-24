#include "CamManager.h"

/* default constructor */
CamManager::CamManager() {

}

int CamManager::SetVerbose() {
  this->quiet = false;

  return 0;
}

/* start acquisition */
int CamManager::StartAcquisition() {

  /* launch the camera software */
  
  /* move multiplecam directory to save images in correct place */
  std::string camera_dir = std::string(CAMERA_DIR);
  std::string cmd = "cd " +  camera_dir;
  int status = system("cd /home/software/CPU/cameras/multiplecam");
  if (status != 0) {
    clog << "error: " << logstream::error << "could not cd into " << CAMERA_DIR << std::endl;
  }

  /* launch and check output */
  std::string output;
  if (this->quiet) {
    output = CpuTools::CommandToStr(CAMERA_EXEC_QUIET);
  }
  else {
    output = CpuTools::CommandToStr(CAMERA_EXEC);   
  }
  
  size_t found = output.find("Error Trace:");
  if (found != std::string::npos) {
    clog << "error: " << logstream::error << "camera launch failed" << std::endl;
  }

 return 0;
}

/* launch a thread to collect camera data */
int CamManager::CollectData() {

  std::cout << "starting camera acquisition in the background..." << std::endl;
  clog << "info: " << logstream::info << "starting camera acquisition" << std::endl;
  
  /* spawn a thread to run camera software */
  std::thread collect_data (&CamManager::StartAcquisition, this);

  /* store the handle */
  this->cam_thread_handle = collect_data.native_handle();

  /* detach */
  collect_data.detach();
  
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
