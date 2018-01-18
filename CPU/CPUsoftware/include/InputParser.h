#ifndef _INPUT_PARSER_H
#define _INPUT_PARSER_H

#include <vector>
#include <algorithm>
#include "LvpsManager.h"

/* struct to handle the command line inputs */
struct CmdLineInputs {

  /* command line options */
  bool help;
  bool hv_on;
  bool debug_mode;
  bool log_on;
  bool trig_on;
  bool cam_on;
  bool lvps_on;
  bool sc_on;
  bool single_run; 
  bool test_zynq_on;
  bool keep_zynq_pkt;

  /* command line arguments */
  int dv;
  int hvdac;
  uint8_t test_mode_num;
  LvpsManager::SubSystem lvps_subsystem;
       
};


/* class to parse command line input to program */
class InputParser{
public:
  CmdLineInputs * CmdLine;
  
  InputParser(int &argc, char **argv); 
  CmdLineInputs * ParseCmdLineInputs();
  /* get the command line options */
  const std::string& getCmdOption(const std::string &option) const {
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && itr++ != this->tokens.end()){
      return *itr;
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
