#ifndef _CAM_MANAGER_H
#define _CAM_MANAGER_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <thread>

#include "log.h"
#include "CpuTools.h"

#define CAMERA_DIR "/home/software/CPU/cameras/multiplecam"
#define CAMERA_EXEC "(cd /home/software/CPU/cameras/multiplecam && ./multiplecam.sh)"
#define CAMERA_EXEC_QUIET "(cd /home/software/CPU/cameras/multiplecam && ./multiplecam.sh 2>&1)"

#define N_TRY_RELAUNCH 2
#define WAIT_TIME 120

class CamManager {
public:
  std::thread::native_handle_type cam_thread_handle;
  static int n_relaunch_attempt;
  bool launch_failed;
  
  CamManager();
  int SetVerbose();
  int StartAcquisition();
  int KillCamAcq();

private:
  bool verbose = false;

};

#endif
/* _CAM_MANAGER_H */
