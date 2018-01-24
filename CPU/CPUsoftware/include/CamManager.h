#ifndef _CAM_MANAGER_H
#define _CAM_MANAGER_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <thread>

#include "log.h"

#define CAMERA_DIR "/home/software/CPU/cameras/multiplecam"
#define CAMERA_EXEC "/home/software/CPU/cameras/multiplecam/multiplecam.sh"
#define CAMERA_EXEC_QUIET "/home/software/CPU/cameras/multiplecam/multiplecam.sh 2>&1"

#define WAIT_TIME 120

class CamManager {
public:
  bool quiet = true;
  
  CamManager();
  int SetVerbose();
  int StartAcquisition();
  int CollectData();

private:
  bool global_stop_exec = false;
  std::thread::native_handle_type cam_thread_handle;
};

#endif
/* _CAM_MANAGER_H */
