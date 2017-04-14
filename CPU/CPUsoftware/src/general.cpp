#include "globals.h"

/* create log file name */
std::string create_logname(void) {
  struct timeval tv;
  gettimeofday(&tv,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  char logname[40];
  strftime(logname, sizeof(logname), "../log/CPU_MAIN_%Y-%m-%d_%H:%M:%S.log", now_tm);
  return logname;
}

/* copy a file */
bool copy_file(const char *SRC, const char* DEST)
{
    std::ifstream src(SRC, std::ios::binary);
    std::ofstream dest(DEST, std::ios::binary);
    dest << src.rdbuf();
    return src && dest;
}
