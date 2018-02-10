#ifndef _CAM_MANAGER_H
#define _CAM_MANAGER_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <thread>
#include <future>
#include <chrono>

#include "log.h"
#include "CpuTools.h"

#define CAMERA_DIR "/home/software/CPU/cameras/multiplecam"
#define CAMERA_EXEC "(cd /home/software/CPU/cameras/multiplecam && ./multiplecam.sh)"
#define CAMERA_EXEC_USB "(cd /home/software/CPU/cameras/multiplecam && ./multiplecam.sh)"
/* to be updated when ST updates multiplecam */
//#define CAMERA_EXEC_USB "(cd /home/software/CPU/cameras/multiplecam && ./multiplecam.sh /home/software/CPU/cameras/multiplecam /media/usb0)"
#define N_TRY_RELAUNCH 2
#define WAIT_TIME 120

/**
 * interface to the multiplecam camera software to allow for parallel acquisition with the two 
 * cameras (VIS and NIR)
 */
class CamManager {
public:
  /**
   * stores the camera thread handle
   */
  std::thread::native_handle_type cam_thread_handle;
  /**
   * stores the number of available usb storage devices
   */
  int usb_num_storage_dev;
  /**
   * stores the number of attempts to launch the cameras
   */
  int n_relaunch_attempt;
  /*
   * set to true when launch failes
   */
  std::promise<bool> launch_failed;
  /*
   * set to true when laucnh succeeds
   */
  bool launch_running;
  
  
  CamManager();
  int SetVerbose();
  int KillCamAcq();
  int CollectData();
  
private:
  bool verbose = false;
  int StartAcquisition();

};

#endif
/* _CAM_MANAGER_H */
