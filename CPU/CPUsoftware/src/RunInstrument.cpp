#include "RunInstrument.h"

/* default constructor */
RunInstrument::RunInstrument(CmdLineInputs * CmdLine) {
  this->CmdLine = CmdLine;
  this->current_mode = RunInstrument::UNDEF;
}

/* switching of LVPS then exit */
int RunInstrument::LvpsSwitch() {

  switch (this->CmdLine->lvps_status) {
    case LvpsManager::ON:
      
      switch (this->CmdLine->lvps_subsystem) {
      case LvpsManager::ZYNQ:
	std::cout << "Switching ON the ZYNQ" << std::endl;
	break;
      case LvpsManager::CAMERAS:
	std::cout << "Switching ON the CAMERAS" << std::endl;
	break;
      case LvpsManager::HK:
	std::cout << "Switching ON the HK" << std::endl;
	break;
      }
      this->Lvps.SwitchOn(this->CmdLine->lvps_subsystem);
      break;

    case LvpsManager::OFF:
      switch (this->CmdLine->lvps_subsystem) {
      case LvpsManager::ZYNQ:
	std::cout << "Switching OFF the ZYNQ" << std::endl;
	break;
      case LvpsManager::CAMERAS:
	std::cout << "Switching OFF the CAMERAS" << std::endl;
	break;
      case LvpsManager::HK:
	std::cout << "Switching OFF the HK" << std::endl;
	break;
      }      
      this->Lvps.SwitchOff(this->CmdLine->lvps_subsystem);
      break;
      
    case LvpsManager::UNDEF:
      std::cout << "Error: Cannot switch subsystem, on/off undefined" << std::endl;
      break;
    }
   
  return 0;
}

/* switching of HVPS then exit */
int RunInstrument::HvpsSwitch() {

  switch (this->CmdLine->hvps_status) {
  case ZynqManager::ON:
    std::cout << "Switching ON the HVPS" << std::endl;
    this->ZqManager.HvpsTurnOn(this->ConfigOut->cathode_voltage, this->ConfigOut->dynode_voltage);
    break;
  case ZynqManager::OFF:
    std::cout << "Switching OFF the HVPS" << std::endl;
    this->ZqManager.HvpsTurnOff();   
    break;
  case ZynqManager::UNDEF:
    std::cout << "Error: Cannot switch subsystem, on/off undefined" << std::endl;
    break;
  }
  
  return 0;
}


/* enter the debug mode then exit */
int RunInstrument::DebugMode() {

    std::cout << "Mini-EUSO software debug mode" << std::endl;
     
    /* make a test Zynq packet */
    DataAcqManager::WriteFakeZynqPkt();
    DataAcqManager::ReadFakeZynqPkt();

    /* add any quick tests here */
    
  return 0;
}


/* define start-up procedure upon switch-on */
int RunInstrument::StartUp() {

  printf("Mini-EUSO CPU SOFTWARE Version: %.2f Date: %s\n", VERSION, VERSION_DATE_STRING);

  /* check the log level */
  if (this->CmdLine->log_on) {
    clog.change_log_level(logstream::all);
  }
  clog << std::endl;
  clog << "info: " << logstream::info << "log created" << std::endl;

  /* reload and parse the configuration file */
  std::string config_file = config_dir + "/dummy.conf";
  std::string config_file_local = config_dir + "/dummy_local.conf";
  ConfigManager CfManager(config_file, config_file_local);
  this->ConfigOut = CfManager.Configure();

  /* check for command line override to config */
  if (this->CmdLine->dv != -1) {
    this->ConfigOut->dynode_voltage = this->CmdLine->dv;
  }
  if (this->CmdLine->hvdac != -1) {
    this->ConfigOut->dac_level = this->CmdLine->hvdac;
  }

  /* move to separate function ... */  

  return 0;
}

int RunInstrument::CheckSystems() {

  /* turn on all systems */
  std::cout << "switching on all systems..." << std::endl;
  if (this->CmdLine->cam_on ==true) {
    this->Lvps.SwitchOn(LvpsManager::CAMERAS);
  }
  this->Lvps.SwitchOn(LvpsManager::HK);
  this->Lvps.SwitchOn(LvpsManager::ZYNQ);

  /* wait for boot */
  std::cout << "waiting for boot..." << std::endl;
  sleep(BOOT_TIME);
  
  /* test the connection to the zynq board */
  this->ZqManager.CheckTelnet();
  
  /* check the instrument and HV status */
  this->ZqManager.InstStatus();
  this->ZqManager.HvpsStatus();

  return 0;
}

/* start running the instrument according to specifications */
int RunInstrument::Start() {

  /* run start-up  */
  StartUp();

  /* check for execute-and-exit commands */
  /* these commands can only be used one at a time */
  if (this->CmdLine->lvps_on) {
    LvpsSwitch();
    return 0;
  }
  else if (this->CmdLine->hvps_on) {
    HvpsSwitch();
    return 0;
  }
  else if (this->CmdLine->debug_mode) {
    DebugMode();
    return 0;
  } 
  
  /* check systems and operational mode */
  CheckSystems();
  
  /* add mode switching DAY/NIGHT here */

  /* start data acquisition */
  /* build on acq_run from mecontrol */
  
  return 0;
}
