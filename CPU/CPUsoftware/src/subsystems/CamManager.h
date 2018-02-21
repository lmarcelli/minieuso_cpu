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

/*
 * Define the commands needed to launch the cameras, used by CamManager::DefineLaunchCmd()
 */
#define CAMERA_SOFTWARE_DIR "/home/software/CPU/cameras/multiplecam"
#define CAMERA_EXEC "./multiplecam.sh"
#define USB_WRITE_DIR "/media/usb0"
#define OTHER_WRITE_DIR "/home/software/CPU/cameras/multiplecam"
#define N_TRY_RELAUNCH 2
#define WAIT_TIME 120

/**
 * interface to the multiplecam camera software to allow for parallel acquisition with the two 
 * cameras (VIS and NIR)
 */
class CamManager {
public:
  /**
   * used to describe the desired camera status to use with CamManager::DefineLaunchcmd()
   */
  enum CamStatus : int {
    OFF = 0,
    ON = 1,
    UNDEF = 2,
  };
  /**
   * stores the status of the NIR camera
   */
  CamStatus nir_status;
  /**
   * stores the status of the VIS camera
   */
  CamStatus vis_status;
  /**
   * stores the NIR serial number
   */
  int nir_serial;
  /**
   * stores the VIS serial number
   */
  int vis_serial;
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
  const char * DefineLaunchCmd();
  void SetCamStatus(CamStatus nir_status, CamStatus vis_status);
  void ParseSerialNumbers();
  
};

#endif
/* _CAM_MANAGER_H */
