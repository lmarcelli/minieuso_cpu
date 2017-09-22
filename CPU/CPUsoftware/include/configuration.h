#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <copyfile.h>
#include <dirent.h>

#include <string>
#include <fstream>

#include "config.h"
#include "log.h"

/* functions for configuration */
/*-----------------------------*/
bool CopyFile(const char * SRC, const char * DEST);
Config * Parse(std::string config_file_local);
Config * Configure(std::string config_file, std::string config_file_local);

#endif
