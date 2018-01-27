#ifndef _INPUT_PARSER_H
#define _INPUT_PARSER_H

#include <vector>
#include <algorithm>

#include "LvpsManager.h"
#include "ZynqManager.h"


/* struct to handle the command line inputs */
struct CmdLineInputs {

  /* command line options */
  bool help;
  bool hvps_on;
  bool debug_mode;
  bool log_on;
  bool trig_on;
  bool cam_on;
  bool therm_on;
  bool lvps_on;
  bool sc_on;
  bool single_run; 
  bool test_zynq_on;
  bool keep_zynq_pkt;

  /* command line arguments */
  int dv;
  int hvdac;
  LvpsManager::Status lvps_status;
  LvpsManager::SubSystem lvps_subsystem;
  ZynqManager::HvpsStatus hvps_status;
  ZynqManager::InstrumentMode zynq_mode;
  ZynqManager::TestMode zynq_test_mode;
  bool cam_verbose;

};


/* class to parse command line input to program */
class InputParser{
public:
  CmdLineInputs * CmdLine = new CmdLineInputs();
  
  InputParser(int &argc, char **argv); 
  CmdLineInputs * ParseCmdLineInputs();
  /* get the command line options */
  const std::string getCmdOption(const std::string &option) const {
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && itr++ != this->tokens.end()) {
      /* prevent segfault until better fix */
      if (option != "-cam") {
	return * itr;
      }
    }
    static const std::string empty_string("");
    return empty_string;
  }
  
  /* check if the command line options exists */
  bool cmdOptionExists(const std::string &option) const {
    return std::find(this->tokens.begin(), this->tokens.end(), option)
      != this->tokens.end();
  }
  

private:
  std::vector <std::string> tokens;

  int PrintHelpMsg();
};


#endif
/* _INPUT_PARSER_H */
