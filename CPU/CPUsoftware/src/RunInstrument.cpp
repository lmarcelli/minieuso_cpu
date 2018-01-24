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
    this->Zynq.HvpsTurnOn(this->ConfigOut->cathode_voltage, this->ConfigOut->dynode_voltage);
    this->Zynq.SetDac(this->ConfigOut->dac_level);
    break;
  case ZynqManager::OFF:
    std::cout << "Switching OFF the HVPS" << std::endl;
    this->Zynq.HvpsTurnOff();   
    this->Zynq.SetDac(PEDESTAL); 
    break;
  case ZynqManager::UNDEF:
    std::cout << "Error: Cannot switch subsystem, on/off undefined" << std::endl;
    break;
  }
  
  return 0;
}


/* enter the debug mode then exit */
int RunInstrument::DebugMode() {

  std::cout << "-----------------------------" <<std::endl; 
  std::cout << "Mini-EUSO software debug mode" << std::endl;
  std::cout << "-----------------------------" <<std::endl; 
  
  /* add any quick tests here */
  
  /* print the USB devices connected */
  this->Daq.Usb->LookupUsbStorage();
  
  /* make a test Zynq packet */
  //DataAcqManager::WriteFakeZynqPkt();
  //DataAcqManager::ReadFakeZynqPkt();  
    
  return 0;
}


/* define start-up procedure upon switch-on */
int RunInstrument::StartUp() {

  std::cout << "-----------------------------------------------------" << std::endl;
  std::cout << "Mini-EUSO CPU SOFTWARE Version: " << VERSION << " Date: " << VERSION_DATE_STRING << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl;
  std::cout << "https://github.com/cescalara/minieuso_cpu" << std::endl;
  std::cout << std::endl;

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

  std::cout << "SUBSYSTEMS TO BE USED IN ACQUISITION" << std::endl;
  std::cout << "Zynq board" << std::endl;
  std::cout << "Analog board" << std::endl;

  if (this->CmdLine->hvps_on) {
    std::cout << "HVPS" << std::endl;
  }
  if (this->CmdLine->cam_on) {
    std::cout << "Cameras" << std::endl;
  }
  if (this->CmdLine->therm_on) {
    std::cout << "Thermistors" << std::endl;
  }
  std::cout << std::endl;
  

  std::cout << "STARTING INSTRUMENT" << std::endl;
  
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
  this->Zynq.CheckTelnet();
  
  /* check the instrument and HV status */
  this->Zynq.GetInstStatus();
  this->Zynq.GetHvpsStatus();

  /* check the number storage Usbs connected */
  this->Daq.Usb->LookupUsbStorage();
  std::cout << "there are " <<
    (int)this->Daq.Usb->num_storage_dev <<
    " USB storage devices connected " << std::endl;

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
  this->Zynq.instrument_mode = this->CmdLine->zynq_mode;
  this->Zynq.test_mode = this->CmdLine->zynq_test_mode;    
 

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
  
  /* launch data backup in background */
  this->Daq.Usb->RunDataBackup();
  
  
  /* select SCURVE or STANDARD acquisition */
  SelectAcqOption();
  switch (this->current_acq_mode) {
  case SCURVE:
    
    /* take an scurve */
    Daq.CollectSc(&this->Zynq, this->ConfigOut, this->CmdLine);
    
    break;
  case STANDARD:

    /* start data acquisition */
    this->Daq.CollectData(&this->Zynq, this->ConfigOut, this->CmdLine);
    
    break;
  case ACQ_UNDEF:
    clog << "error: " << logstream::error << "RunInstrument AcquisitionMode is undefined" << std::endl;
    std::cout << "Error: RunInstrument AcquisitionMode is undefined" << std::endl;
  }

  /* never reached for infinite acquisition */

  return 0;
}

/* start running the instrument according to specifications */
int RunInstrument::Start() {

  /* check for execute-and-exit commands */
  if (this->CmdLine->lvps_on) {
    LvpsSwitch();
    return 0;
  }
  
  /* run start-up  */
  StartUp();

  /* check for execute-and-exit commands which require config */
  if (this->CmdLine->hvps_on) {
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
