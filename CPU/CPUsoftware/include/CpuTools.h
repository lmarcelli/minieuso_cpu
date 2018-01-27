#ifndef _CPU_TOOLS_H
#define _CPU_TOOLS_H

#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <dirent.h>

#include "ZynqManager.h"

#define MAX_STR_LENGTH 1000

/* class to provide useful funcions to other parts of the software */
class CpuTools {

public:
  CpuTools();
  static std::string CommandToStr(const char * cmd);
  static std::string IntToFixedLenStr(const int input, const int length);
  static void ClearFolder(const char * data_dir);
  static void SignalHandler(int signum);

};

#endif
/* _CPU_TOOLS_H */
