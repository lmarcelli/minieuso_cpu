#include "RunInstrument.h"

/* default constructor */
RunInstrument::RunInstrument(CmdLineInputs * CmdLine) {
  this->CmdLine = CmdLine;
  this->current_inst_mode = RunInstrument::INST_UNDEF;
  this->current_acq_mode = RunInstrument::ACQ_UNDEF;
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
    this->ZqManager.SetDac(this->ConfigOut->dac_level);
    break;
  case ZynqManager::OFF:
    std::cout << "Switching OFF the HVPS" << std::endl;
    this->ZqManager.HvpsTurnOff();   
    this->ZqManager.SetDac(PEDESTAL); 
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
  std::string config_dir(CONFIG_DIR);
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

/* determine acquisition mode from program inputs */
int RunInstrument::SelectAcqOption() {
  
  /* select standard or scurve */
  if (this->CmdLine->sc_on) {
    this->current_acq_mode = SCURVE;
  }
  else {
    this->current_acq_mode = STANDARD;
  }

  /* select Zynq acquisition mode */
  this->ZqManager.instrument_mode = this->CmdLine->zynq_mode;
  this->ZqManager.test_mode = this->CmdLine->zynq_test_mode;    
 

  return 0;
}

/* interface to the whole data acquisition */
int RunInstrument::Acquisition() {

  std::cout << "starting acqusition run..." <<std::endl; 
  clog << "info: " << logstream::info << "starting acquisition run" << std::endl;
  
  /* clear the FTP server */
  CpuTools::ClearFolder(DATA_DIR);
  
  /* enable signal handling */
  signal(SIGINT, CpuTools::SignalHandler);  
  
  /* define data backup */
  this->UManager.DataBackup();
  
  
  /* select SCURVE or STANDARD acquisition */
  SelectAcqOption();
  switch (this->current_acq_mode) {
  case SCURVE:
    
      /* take an scurve */
    DaqManager.CollectSc(&this->ZqManager, this->ConfigOut, this->CmdLine);
    
    break;
  case STANDARD:

    /* start data acquisition */
    this->DaqManager.CollectData(&this->ZqManager, this->ConfigOut, this->CmdLine);
    
    break;
  case ACQ_UNDEF:
    clog << "error: " << logstream::error << "RunInstrument AcquisitionMode is undefined" << std::endl;
    std::cout << "Error: RunInstrument AcquisitionMode is undefined" << std::endl;
  }

  /* never reached for infinite acquisition */

  /* wait for backup to complete */
  //run_backup.join();
  
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
  /* switch (DAY/NIGHT) */

  /* case: NIGHT */
  /* set the HV as required */
  if (this->CmdLine->hvps_on) {
    HvpsSwitch();
  }
  
  /* start data acquisition */
  Acquisition();
  
  /* only reached for SCURVE and SHORT acquisitions */
  /* turn off HV */
  this->CmdLine->hvps_status = ZynqManager::OFF;
  HvpsSwitch();

  /* turn off all subsystems */
  this->CmdLine->lvps_status = LvpsManager::OFF;
  this->CmdLine->lvps_subsystem = LvpsManager::HK;
  LvpsSwitch();
  this->CmdLine->lvps_subsystem = LvpsManager::CAMERAS;
  LvpsSwitch();
  this->CmdLine->lvps_subsystem = LvpsManager::ZYNQ;
  LvpsSwitch();
   
  return 0;
}
