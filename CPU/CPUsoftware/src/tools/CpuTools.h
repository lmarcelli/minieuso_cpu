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
#include <vector>

#include <dirent.h>
#include <unistd.h>

#include "ZynqManager.h"

/*
 * define maximum output length for use with CpuTools::CommandToStr()
 */
#define MAX_STR_LENGTH 2000

/**
 * class to provide useful funcions to other parts of the software 
 */
class CpuTools {

public:
  
  CpuTools();
  static std::string CommandToStr(const char * cmd);
  static std::string IntToFixedLenStr(const int input, const int length);
  static void ClearFolder(const char * data_dir);
  static std::string SpaceToUnderscore(std::string);
  static bool PingConnect(std::string ip_address);
  static bool CheckFtp();
  static std::vector<int> DelimStrToVec(std::string input_string, char delim, int size, bool check_01);
  static std::string BuildStr(std::string stem, std::string sep, int val, int rep);
  static std::string BuildStrFromVec(std::string stem, std::string sep, std::vector<int> values);

};

#endif
/* _CPU_TOOLS_H */
