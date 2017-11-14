#ifndef _MECONTROL_H
#define _MECONTROL_H

#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <algorithm>
#include <vector>

#include "log.h"
#include "ConfigManager.h"
#include "ZynqManager.h"
#include "UsbManager.h"
#include "CamManager.h"
#include "LvpsManager.h"
#include "DataAcqManager.h"

#define VERSION 3.1
#define VERSION_DATE_STRING "26/10/2017"

#define HOME_DIR "/home/software/CPU"
#define DONE_DIR "/home/minieusouser/DONE"
#define DATA_DIR "/home/minieusouser/DATA"

class InputParser{
public:
  InputParser(int &argc, char **argv) {
    for (int i = 1; i < argc; i++)
      this->tokens.push_back(std::string(argv[i]));
  }
  
  const std::string& getCmdOption(const std::string &option) const {
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && itr++ != this->tokens.end()){
      return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
  }
  
  bool cmdOptionExists(const std::string &option) const{
    return std::find(this->tokens.begin(), this->tokens.end(), option)
      != this->tokens.end();
  }
private:
  std::vector <std::string> tokens;
};


void SignalHandler(int signum);
void ClearFTP();
int single_acq_run(UsbManager * UManager, Config * ConfigOut,
		   ZynqManager * ZqManager, DataAcqManager * DaqManager,
		   CamManager * CManager, bool hv_on, bool trig_on, bool cam_on, bool sc_on, bool single_run);

#endif
/* _MECONTROL_H */
