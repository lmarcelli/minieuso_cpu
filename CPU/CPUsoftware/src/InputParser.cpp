#include "InputParser.h"

/* default constructor */
InputParser::InputParser(int &argc, char **argv) {

  /* initialise the struct to handle input */
  this->CmdLine->help = false;
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

  /* check for help option */
  if(cmdOptionExists("-help")){
    this->CmdLine->help = true;
    PrintHelpMsg();
  }
  
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

/* print the help message */
int InputParser::PrintHelpMsg() {
  std::cout << "Mini-EUSO command line interface" << std::endl;
  std::cout << "--------------------------------" << std::endl;
  std::cout << std::endl;
  std::cout << "COMMAND: mecontrol" << std::endl;
  std::cout << "USAGE: mecontrol -option argument" << std::endl;
  std::cout << std::endl;
  std::cout << "DESCRIPTION" << std::endl;
  std::cout << "The default behaviour of the software is to run an infinite acquisition," << std::endl;
  std::cout << "without HV and with the DAC in the pedestal (DAC = 750)." << std::endl;
  std::cout << "For further information on the default configuration see the online documentation." << std::endl;
  std::cout << "https://github.com/cescalara/minieuso_cpu" <<std::endl;
  std::cout << "capel.francesca@gmail.com" <<std::endl;
  std::cout << std::endl;
  std::cout << "GENERAL" << std::endl;
  std::cout << "-db: enter software test/debug mode" << std::endl;
  std::cout << "-log: turn on logging (off by default)" << std::endl;
  std::cout << std::endl;
  std::cout << "SUBSYSTEMS" << std::endl;
  std::cout << "-lvps: use the CPU to switch on/off the LVPS" << std::endl;
  std::cout << "-cam: make a simultaneous acquisition with the cameras" << std::endl;
  std::cout << std::endl;
  std::cout << "HIGH VOLTAGE" << std::endl;
  std::cout << "-hv: turn on the high voltage" << std::endl;
  std::cout << "-dv X: provide the dynode voltage (0 - 4096)" << std::endl;
  std::cout << "-hvdac X: provide the HV DAC (0 - 1000)" << std::endl;
  std::cout >> "Example use case: ./mecontrol -log -hv -dv 3200 -hvdac 500" << std::endl;
  std::cout << std::endl;
  std::cout << "ACQUISITION" << std::endl;
  std::cout << "-scurve: take a single S-curve and exit" << std::endl;
  std::cout << "-short: take a single file (~ 2min) acquisition and exit "<< std::endl;
  std::cout << "-test_zynq MODE: use the Zynq test mode (0 - 6)" << std::endl;
  std::cout << "-keep_zynq_pkt: keep the Zynq packets on FTP" << std::endl;
  std::cout << "Example use case: ./mecontrol -log -test_zynq 3 -keep_zynq_pkt" << std::endl;
  std::cout << std::endl;
 
  return 0;
}
