#ifndef _CPU_TOOLS_H
#define _CPU_TOOLS_H

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

/* class to provide useful funcions to other parts of the software */
class CpuTools {

public:
  CpuTools();
  static std::string CommandToStr(const char * cmd);
 
};

#endif /* _CPU_TOOLS_H */
