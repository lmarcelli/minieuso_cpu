#include "InputParser.h"

/* default constructor */
InputParser::InputParser(int &argc, char **argv) {

  /* initialise the struct to handle input */
  this->CmdLine->hv_on = false;
  this->CmdLine->debug_mode = false;
  this->CmdLine->log_on = false;
  this->CmdLine->trig_on = false;
  this->CmdLine->cam_on = false;
  this->CmdLine->lvps_on = false;
  this->CmdLine->sc_on = false;
  this->CmdLine->single_run = false;
  this->CmdLine->test_zynq_on = false;
  this->CmdLine->keep_zynq_pkt = false;
  this->CmdLine->dv = -1;
  this->CmdLine->hvdac = -1;
  this->CmdLine->test_mode_num = -1;
  this->CmdLine->lvps_subsystem = LvpsManager::ZYNQ;
  
  /* get command line input */
  for (int i = 1; i < argc; i++) {
    this->tokens.push_back(std::string(argv[i]));
  }
}


/* parse the command line options */
CmdLineInputs * InputParser::ParseCmdLineInputs() {

  /* check what comand line options exist */
  if(cmdOptionExists("-hv")){
    this->CmdLine->hv_on = true;
  }
  if(cmdOptionExists("-short")){
    this->CmdLine->single_run = true;
  }
  if(cmdOptionExists("-db")){
    this->CmdLine->debug_mode = true;
  }
  if(cmdOptionExists("-log")){
    this->CmdLine->log_on = true;
  }
  if(cmdOptionExists("-trig")){
    this->CmdLine->trig_on = true;
  }
  if(cmdOptionExists("-cam")){
    this->CmdLine->cam_on = true;
  }
  if(cmdOptionExists("-lvps")){
    this->CmdLine->lvps_on = true;  
  }
  if(cmdOptionExists("-scurve")){
    this->CmdLine->sc_on = true;
  }
  if(cmdOptionExists("-test_zynq")){
    this->CmdLine->test_zynq_on = true;
  }
  if(cmdOptionExists("-keep_zynq_pkt")){
    this->CmdLine->keep_zynq_pkt = true;
  }
  
  /* get the arguments */
  /* dynode voltage */
  const std::string &dynode_voltage = getCmdOption("-dv");
  if (!dynode_voltage.empty()){
    this->CmdLine->dv = std::stoi(dynode_voltage);
  }
  
  /* high voltage dac */
  const std::string &hv_dac = getCmdOption("-hvdac");
  if (!hv_dac.empty()){
    this->CmdLine->hvdac = std::stoi(hv_dac);
  }
  
  /* zynq test mode number */
  const std::string &test_mode = getCmdOption("-test_zynq");
  if (!test_mode.empty()){
    this->CmdLine->test_mode_num = std::stoi(test_mode);
  }

  /* LVPS subsystem */    
  const std::string & subsystem_str = getCmdOption("-lvps");
  if (!subsystem_str.empty()) {
    if (subsystem_str == "zynq") {
      this->CmdLine->lvps_subsystem = LvpsManager::ZYNQ;
	}
	else if (subsystem_str == "cam") {
	  this->CmdLine->lvps_subsystem = LvpsManager::CAMERAS;
	}
	else if (subsystem_str == "hk") {
	  this->CmdLine->lvps_subsystem = LvpsManager::HK;
	}
      }

  return this->CmdLine;
}

