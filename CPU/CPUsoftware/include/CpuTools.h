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

/* class to provide useful funcions to other parts of the software */
class CpuTools {

public:
  CpuTools();
  static std::string CommandToStr(const char * cmd);
  static std::string IntToFixedLenStr(int input, int length);
 
};

#endif /* _CPU_TOOLS_H */
