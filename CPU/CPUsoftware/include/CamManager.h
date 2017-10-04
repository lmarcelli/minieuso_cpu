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
#define WAIT_TIME 120

class CamManager {
public:
  bool global_stop_exec = false;
  
  CamManager();
  int StartAcquisition();
  int CollectData();
};

#endif
/* _CAM_MANAGER_H */
