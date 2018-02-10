#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string>
#include <fstream>

#include "log.h"

#define CONFIG_FILE_USB "/media/usb/main_cpu.conf"
#define CONFIG_FILE_LOCAL "/home/software/CPU/CPUsoftware/config/main_cpu.conf"
#define CONFIG_DIR "/home/software/CPU/CPUsoftware/config"

/**
 * struct for output of the configuration file 
 */
struct Config {
  int cathode_voltage;
  int dynode_voltage;
  int scurve_start;
  int scurve_step;
  int scurve_stop;
  int scurve_acc;
  int dac_level;
  int N1;
  int N2;
};

/**
 * class for configuring the instrument based on the configuration file provided 
 */
class ConfigManager {  
public:
  /**
   * path to the local configuration file
   */
  std::string config_file_local;
  /**
   * path to configuration file to be copied
   */
  std::string config_file;
  
  ConfigManager();
  ConfigManager(std::string, std::string);
  Config * Configure();

private:
  bool CopyFile(const char * SRC, const char * DEST);
  Config * Parse();

};

#endif
/* _CONFIGURATION_H */
