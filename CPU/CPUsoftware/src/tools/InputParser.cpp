#include "InputParser.h"

/** 
 * constructor.
 * @param argc command line inputs
 * @param argv command line inputs
 * initalise the CmdLine struct
 */
InputParser::InputParser(int &argc, char **argv) {
  
  /* initialise the struct to handle input */
  this->CmdLine->help = false;
  this->CmdLine->hvps_on = false;
  this->CmdLine->hvps_switch = false;
  this->CmdLine->debug_mode = false;
  this->CmdLine->log_on = false;
  this->CmdLine->trig_on = false;
  this->CmdLine->cam_on = false;
  this->CmdLine->cam_verbose = false;
  this->CmdLine->therm_on = false;
  this->CmdLine->lvps_on = false;
  this->CmdLine->sc_on = false;
  this->CmdLine->single_run = false;
  this->CmdLine->test_zynq_on = false;
  this->CmdLine->keep_zynq_pkt = false;
  this->CmdLine->check_status = false;
  
  this->CmdLine->dv = -1;
  this->CmdLine->asic_dac = -1;
  this->CmdLine->lvps_status = LvpsManager::UNDEF;
  this->CmdLine->lvps_subsystem = LvpsManager::ZYNQ;
  this->CmdLine->hvps_status = ZynqManager::UNDEF;
  this->CmdLine->zynq_mode = ZynqManager::NONE;
  this->CmdLine->zynq_test_mode = ZynqManager::T_NONE;
  
  /* get command line input */
  for (int i = 1; i < argc; i++) {
    this->tokens.push_back(std::string(argv[i]));
  }
}


/**
 * parse the command line options
 * and store the result in CmdLine 
 */
CmdLineInputs * InputParser::ParseCmdLineInputs() {

  /* check for help option */
  if(cmdOptionExists("-help")){
    this->CmdLine->help = true;
    PrintHelpMsg();
  }
  
  /* check what comand line options exist */
  if(cmdOptionExists("-hv")){
    this->CmdLine->hvps_on = true;
    this->CmdLine->hvps_status = ZynqManager::ON;

  }
  if(cmdOptionExists("-hvps")){
    this->CmdLine->hvps_switch = true;

    /* HVPS on/off */
    const std::string & hv_status_str = getCmdOption("-hvps");
    if (!hv_status_str.empty()) {
      if (hv_status_str == "on") {
	this->CmdLine->hvps_status = ZynqManager::ON;
      }
      else if (hv_status_str == "off") {
	this->CmdLine->hvps_status = ZynqManager::OFF;   
      }
    }   

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
  if(cmdOptionExists("-v")){
    this->CmdLine->cam_verbose = true;
  }
  if(cmdOptionExists("-therm")){
    this->CmdLine->therm_on = true;
  }
  if(cmdOptionExists("-lvps")){
    this->CmdLine->lvps_on = true;  

    /* LVPS on/off */
    const std::string & lvps_status_str = getCmdOption("-lvps");
    if (!lvps_status_str.empty()) {
      if (lvps_status_str == "on") {
	this->CmdLine->lvps_status = LvpsManager::ON;
      }
      else if (lvps_status_str == "off") {
	this->CmdLine->lvps_status = LvpsManager::OFF;   
      }
    }
    
    /* LVPS subsystem */    
    const std::string & subsystem_str = getCmdOption("-subsystem");
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
    else {
      std::cout << "WARNING: no subsystem specified, using default: zynq" << std::endl;
    }
    
  }
  if(cmdOptionExists("-scurve")){
    this->CmdLine->sc_on = true;
  }
  if(cmdOptionExists("-zynq")){

    /* zynq instrument mode */
    const std::string &mode = getCmdOption("-zynq");
    if (!mode.empty()){

      /* basic modes */
      if (mode == "none") {
	this->CmdLine->zynq_mode = ZynqManager::NONE;
      }
      else if (mode == "periodic") {
	this->CmdLine->zynq_mode = ZynqManager::PERIODIC;
      }
      else if (mode == "self") {
	this->CmdLine->zynq_mode = ZynqManager::SELF;
      }
      else if (mode == "immediate") {
	this->CmdLine->zynq_mode = ZynqManager::IMMEDIATE;
      }
      else if (mode == "external") {
	this->CmdLine->zynq_mode = ZynqManager::EXTERNAL;
      }
      else if (mode == "trigger") {
	this->CmdLine->zynq_mode = ZynqManager::TRIGGER;
      }

      /* compound modes */
      uint8_t mode_to_set = 0;
      size_t found = mode.find(",");
      if (found != std::string::npos) {
	found = mode.find("periodic");
	if (found != std::string::npos) {
	  mode_to_set += ZynqManager::PERIODIC;
	}
	found = mode.find("self");
	if (found != std::string::npos) {
	  mode_to_set += ZynqManager::SELF;
	}
	found = mode.find("immediate");
	if (found != std::string::npos) {
	 mode_to_set += ZynqManager::IMMEDIATE;
	}
	found = mode.find("external");
	if (found != std::string::npos) {
	  mode_to_set += ZynqManager::EXTERNAL;
	}
	this->CmdLine->zynq_mode = mode_to_set;
      }
	      
    }
    else {
      this->CmdLine->zynq_mode = ZynqManager::PERIODIC;
      std::cout << "WARNING: could not identify required zynq mode, using default: periodic" << std::endl;
    }

  }
  if(cmdOptionExists("-test_zynq")){
    this->CmdLine->test_zynq_on = true;

    /* zynq test mode */
    const std::string &test_mode = getCmdOption("-test_zynq");
    if (!test_mode.empty()){
      if (test_mode == "none") {
	this->CmdLine->zynq_test_mode = ZynqManager::T_NONE;
      }
      else if (test_mode == "ecasic") {
	this->CmdLine->zynq_test_mode = ZynqManager::ECASIC;
      }
      else if (test_mode == "pmt") {
	this->CmdLine->zynq_test_mode = ZynqManager::PMT;
      }
      else if (test_mode == "pdm") {
	this->CmdLine->zynq_test_mode = ZynqManager::PDM;
      }
      else if (test_mode == "l1") {
	this->CmdLine->zynq_test_mode = ZynqManager::L1;
      }
      else if (test_mode == "l2") {
	this->CmdLine->zynq_test_mode = ZynqManager::L2;
      }
      else if (test_mode == "L3") {
	this->CmdLine->zynq_test_mode = ZynqManager::L3;
      }
    }
    else {
      this->CmdLine->zynq_test_mode = ZynqManager::PDM;
      std::cout << "WARNING: cannot identify required zynq test mode, using default: pdm" << std::endl;
    }

    /* also set Zynq mode to PERIODIC to enable data collecting */
    this->CmdLine->zynq_mode = ZynqManager::PERIODIC;
    
  }
  if(cmdOptionExists("-keep_zynq_pkt")){
    this->CmdLine->keep_zynq_pkt = true;
  }
  if(cmdOptionExists("-check_status")){
    this->CmdLine->check_status = true;
  }
  
  
  /* get the arguments */
  /* dynode voltage */
  const std::string &dynode_voltage = getCmdOption("-dv");
  if (!dynode_voltage.empty()){
    this->CmdLine->dv = std::stoi(dynode_voltage);
  }
  
  /* high voltage dac */
  const std::string &asic_dac = getCmdOption("-asicdac");
  if (!asic_dac.empty()){
    this->CmdLine->asic_dac = std::stoi(asic_dac);
  }  

  /* high voltage dac (kept for compatibility) */
  const std::string &hv_dac = getCmdOption("-hvdac");
  if (!hv_dac.empty()){
    this->CmdLine->asic_dac = std::stoi(hv_dac);
  }  

  

  return this->CmdLine;
}

/**
 * print the help message detailing all accepted command line options
 * (for use with mecontrol -help) 
 */
int InputParser::PrintHelpMsg() {
  
  std::cout << "--------------------------------" << std::endl;
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
  std::cout << "Contact: capel.francesca@gmail.com" <<std::endl;
  std::cout << std::endl;
  std::cout << "GENERAL" << std::endl;
  std::cout << "-db: enter software test/debug mode" << std::endl;
  std::cout << "-log: turn on logging (off by default)" << std::endl;
  std::cout << std::endl;
  std::cout << "EXECUTE-AND-EXIT" << std::endl;
  std::cout << "These commands execute and exit without running an automated acquisition" << std::endl;
  std::cout << "Switching the LVPS manually" << std::endl;
  std::cout << "-lvps <MODE>: switch a subsystem using the LVPS (<MODE> = \"on\" or \"off\") then exit the program" << std::endl;
  std::cout << "-subsystem <SUBSYS>: select subsystem to switch (<SUBSYS> = \"zynq\", \"cam\" or \"hk\"), \"zynq\" by default" << std::endl;
  std::cout << "Example use case: mecontrol -lvps on -subsystem zynq" << std::endl;
  std::cout << "Note: the automated acquisition program switches subsystems on/off automatically as required" << std::endl;
  std::cout << "Switching the HVPS manually" << std::endl;
  std::cout << "-hvps <MODE>: switch the high voltage (<MODE> = \"on\" or \"off\") then exit the program" << std::endl;
  std::cout << "-dv <X>: provide the dynode voltage (<X> = 0 - 4096)" << std::endl;
  std::cout << "-hvdac <X>: provide the HV DAC (<X> = 0 - 1000)" << std::endl;
  std::cout << "Example use case: mecontrol -hvps on -dv 3200 -hvdac 500" << std::endl;
  std::cout << "Example use case: mecontrol -hvps off" << std::endl;
  std::cout << "Note: the automated acquisition program switches the HV on automatically as required," << std::endl;
  std::cout << "and switches it off automatically if the program is interrupted with CTRL-C" << std::endl;
  std::cout << "Checking the instrument status" << std::endl;
  std::cout << "-check_status: check the Zynq telnet connection, instrument status and HV status" << std::endl;
  std::cout << std::endl;
  std::cout << "SUBSYSTEMS" << std::endl;
  std::cout << "-cam: make an independent or simultaneous acquisition with the cameras" << std::endl;
  std::cout << "-cam -v: make an independent or simultaneous acquisition with the cameras with verbose output" << std::endl;
  std::cout << "-therm: make a simultaneous acquisition with the thermistors" << std::endl;
  std::cout << "Example use case: mecontrol -log -cam -therm" << std::endl;
  std::cout << std::endl;
  std::cout << "HIGH VOLTAGE" << std::endl;
  std::cout << "-hv: run an automated acquisition with the HV on" << std::endl;
  std::cout << "-dv <X>: provide the dynode voltage (<X> = 0 - 4096), default in ../config/dummy.conf" << std::endl;
  std::cout << "-hvdac <X>: provide the HV DAC (<X> = 0 - 1000), default in ../config/dummy.conf" << std::endl;
  std::cout << "Example use case: mecontrol -log -hv -dv 3200 -hvdac 500" << std::endl;
  std::cout << "Note: high voltage should switch off automatically if the program is interrupted with CTRL-C" << std::endl;
  std::cout << std::endl;
  std::cout << "ACQUISITION" << std::endl;
  std::cout << "-scurve: take a single S-curve and exit" << std::endl;
  std::cout << "-short: take a single file (~ 2min) acquisition and exit "<< std::endl;
  std::cout << "-zynq <MODE>: use the Zynq acquisition mode (<MODE> = none, periodic, self, immediate, external trigger, default = none)" << std::endl;
  std::cout << "-test_zynq <MODE>: use the Zynq test mode (<MODE> = none, ecasic, pmt, pdm, l1, l2, l3, default = pdm)" << std::endl;
  std::cout << "-keep_zynq_pkt: keep the Zynq packets on FTP" << std::endl;
  std::cout << "Example use case: mecontrol -log -test_zynq pdm -keep_zynq_pkt" << std::endl;
  std::cout << "Example use case: mecontrol -log -hv on -zynq trigger" << std::endl;
  std::cout << "NB: different zynq modes can be used together! E.g.: mecontrol -log -hv -zynq periodic,external" << std::endl;
  std::cout << "NB: different zynq test modes CANNOT be used together" << std::endl;
  std::cout << std::endl;
  std::cout << "NOTES" << std::endl;
  std::cout << "Execute-and-exit flags such as -db, -hv on/off and -lvps on/off can only be used one at a time" << std::endl;
  std::cout << "*ALWAYS CONFIRM THE HV IS SWITCHED OFF BEFORE ALLOWING LIGHT ON THE PDM*" << std::endl;
  std::cout << "to safely stop the program's exectution use CTRL-C" << std::endl;
 
  return 0;
}
