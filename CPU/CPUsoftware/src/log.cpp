#include "log.h"

std::string log_name = CreateLogname(); 

/* create log file name */
std::string CreateLogname(void) {
  struct timeval tv;
  char logname[80];
  std::string log_dir(LOG_DIR);
  std::string time_str("/CPU_MAIN__%Y_%m_%d__%H_%M_%S.log");
  std::string log_str = log_dir + time_str;
  const char * kLogCh = log_str.c_str();
  
  gettimeofday(&tv,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);

  strftime(logname, sizeof(logname), kLogCh, now_tm);
  std::cout << "Logname: " << logname << std::endl;
  return logname;
}
