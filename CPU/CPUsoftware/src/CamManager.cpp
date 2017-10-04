#include "CamManager.h"

/* default constructor */
CamManager::CamManager() {

}

/* start acquisition */
int CamManager::StartAcquisition() {
  /* launch the camera software */
  int status = system("cd /home/software/CPU/cameras/multiplecam");
  if (status != 0) {
    clog << "error: " << logstream::error << "could not cd into " << CAMERA_DIR << std::endl;
  }
  
  /* fork a process */
  pid_t pid = fork();

  if (pid == 0) {
    /* child process */
    execl(CAMERA_EXEC, NULL);
  }
  else if (pid > 0) {
    /* parent process */
    while (global_stop_exec == false) {

    }
    /* kill the process */
    kill(pid, SIGKILL);
  }

 return 0;
}

/* collect data */
int CamManager::CollectData() {

  clog << "info: " << logstream::info << "starting camera acquisition" << std::endl;
  
  /* spawn a thread to run camera software */
  std::thread collect_data (&CamManager::StartAcquisition, this);

  /* wait a set period */
  sleep(WAIT_TIME);

  /* set global var to kill execution */
  clog << "info: " << logstream::info << "stopping camera acquisition" << std::endl;
  global_stop_exec = true;
  collect_data.join();
  return 0;
}
