#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string>
#include <fstream>

#include "config.h"
#include "log.h"

#define CONFIG_FILE_USB "/media/usb/main_cpu.conf"
#define CONFIG_FILE_LOCAL "/home/software/CPU/CPUsoftware/config/main_cpu.conf"
#define CONFIG_DIR "/home/software/CPU/CPUsoftware/config"

class ConfigManager {
private:
  bool CopyFile(const char * SRC, const char * DEST);
  Config * Parse();
  
public:
  std::string config_file_local;
  std::string config_file;
  
  ConfigManager();
  ConfigManager(std::string, std::string);
  Config * Configure();
  
};

#endif
