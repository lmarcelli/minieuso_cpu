#include "RunInstrument.h"


/* default constructor */
RunInstrument::RunInstrument(CmdLineInputs * CmdLine) {
  this->CmdLine = CmdLine;

  {
    std::unique_lock<std::mutex> lock(this->m_inst_mode);
    this->current_inst_mode = RunInstrument::INST_UNDEF;
  } /* relase mutex */
  this->current_acq_mode = RunInstrument::ACQ_UNDEF;

  {
    std::unique_lock<std::mutex> lock(this->_m_stop);
    this->_stop = false;
  } /* relase mutex */

}


/* handle SIGINT */
void RunInstrument::SignalHandler(int signum) {

  std::cout << "Interrupt signal (" << signum << ") received" << std::endl;  

  /* signal to main program */
  signal_shutdown.store(true);

  sleep(10);
  
  /* terminate the program */
  exit(signum);  
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

  /* run through main subsystems for easy debugging */
  
  std::cout << "-----------------------------" << std::endl; 
  std::cout << "Mini-EUSO software debug mode" << std::endl;
  std::cout << "-----------------------------" << std::endl; 
  std::cout << "https://github.com/cescalara/minieuso_cpu" << std::endl;
  std::cout << std::endl;
  std::cout << "running checks of all subsystems..." <<std::endl; 
  std::cout << std::endl;

  std::cout << "USB" << std::endl;
  std::cout << "there are " << (int)this->Usb.LookupUsbStorage() << "USB storage devices connected" << std::endl;
  std::cout << std::endl;

  std::cout << "LVPS" << std::endl;
  std::cout << "switching on/off all subsystems... ";  
  this->Lvps.SwitchOn(LvpsManager::CAMERAS);
  sleep(1);
  this->Lvps.SwitchOn(LvpsManager::HK);
  sleep(1);
  this->Lvps.SwitchOn(LvpsManager::ZYNQ);
  sleep(1);
  this->Lvps.SwitchOff(LvpsManager::CAMERAS);
  sleep(1);
  this->Lvps.SwitchOff(LvpsManager::HK);
  sleep(1);
  this->Lvps.SwitchOff(LvpsManager::ZYNQ);
  std::cout << "done!" << std::endl;
  std::cout << std::endl;

  std::cout << "ANALOG" << std::endl;
  std::cout << "running an acquisition..." << std::endl;  
  this->Daq.Analog->GetLightLevel();
  auto light_level = this->Daq.Analog->ReadLightLevel();
  int i = 0;
  for (i = 0; i < N_CHANNELS_PHOTODIODE; i++) {
    std::cout << "photodiode channel " << i << ": " << light_level->photodiode_data[i] << std::endl;
  }
  float avg_sipm = 0;
  for (i = 0; i < N_CHANNELS_SIPM; i++) {
    avg_sipm += light_level->sipm_data[i];
  }
  avg_sipm = avg_sipm/N_CHANNELS_SIPM;
  std::cout << "SIPM 64 channel average: " << avg_sipm << std::endl;
  std::cout << "SIPM single channel: " << light_level->sipm_single << std::endl;
  std::cout << std::endl;

  std::cout << "THERMISTORS" << std::endl;
  std::cout << "running an acquisition (takes ~10 s)..." << std::endl;  
  this->Daq.ThManager->PrintTemperature();
  std::cout << std::endl;

  /* uncomment when ST has given updates */
  /*
  this->Lvps.SwitchOn(LvpsManager::CAMERAS);
  std::cout << "CAMERAS" << std::endl;
  std::cout << "running an acquisition..." << std::endl;  
  this->CmdLine->cam_on = true;
  this->CmdLine->cam_verbose = true;
  this->LaunchCam();
  std::cout << "stopping acquisition... ";
  this->Cam.KillCamAcq();
  std::cout << "done!" << std::endl;
  std::cout << std::endl;
  this->Lvps.SwitchOff(LvpsManager::CAMERAS);
  */
  
  this->Lvps.SwitchOn(LvpsManager::ZYNQ);
  std::cout << "ZYNQ" << std::endl;
  this->Zynq.CheckTelnet();
  if (this->Zynq.telnet_connected) {
    this->Zynq.GetInstStatus();
    this->Zynq.GetHvpsStatus();
  }
  else {
    std::cout << "ERROR: Zynq cannot reach Mini-EUSO over telnet" << std::endl;
    std::cout << "first try to ping 192.168.7.10 then try again" << std::endl;
  }
  std::cout << std::endl;
  this->Lvps.SwitchOff(LvpsManager::ZYNQ);

  
  std::cout << "debug tests completed, exiting the program" << std::endl;
      
  return 0;
}


/* set the instrument mode with mutex protection */
int RunInstrument::SetInstMode(InstrumentMode mode_to_set) {

  {
    std::unique_lock<std::mutex> lock(this->m_inst_mode);
    this->current_inst_mode = mode_to_set;
  } /* release mutex */

  return 0;
}


/* read the instrument mode with mutex protection */
RunInstrument::InstrumentMode RunInstrument::GetInstMode() {
  InstrumentMode current_inst_mode;

  {
    std::unique_lock<std::mutex> lock(this->m_inst_mode);
    current_inst_mode = this->current_inst_mode;
  } /* release mutex */
  
  return current_inst_mode;
}


/* initialise instrument mode using the current light level */
int RunInstrument::InitInstMode() {

  clog << "info: " << logstream::info << "setting the instrument mode" << std::endl;

  /* get the current light level */
  this->Daq.Analog->GetLightLevel();
  bool above_light_threshold = this->Daq.Analog->CompareLightLevel();

  /* make a decision */
  if (above_light_threshold) {
    /* set to day mode */
    this->SetInstMode(RunInstrument::DAY);
  }
  else {
    /* set to night mode */
   this->SetInstMode(RunInstrument::NIGHT);
  }
  
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
  else {
    /* remove the log file */
    std::string cmd = "rm " + log_name;
    system(cmd.c_str());
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


/* check the systems */
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

  if (this->Zynq.telnet_connected) {
    /* check the instrument and HV status */
    this->Zynq.GetInstStatus();
    this->Zynq.GetHvpsStatus();
  }
  else {
    std::cout << "ERROR: Zynq cannot reach Mini-EUSO over telnet" << std::endl;
    std::cout << "first try to ping 192.168.7.10 then try again" << std::endl;
  }
  
  /* check the number storage Usbs connected */
  std::cout << "there are " << (int)this->Usb.LookupUsbStorage() << " USB storage devices connected " << std::endl;
  this->Daq.usb_num_storage_dev = this->Usb.num_storage_dev;
  this->Cam.usb_num_storage_dev = this->Usb.num_storage_dev;

  /* initialise the instrument mode */
  InitInstMode();
  
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

/* launch the cameras and handle errors */
int RunInstrument::LaunchCam() {
  size_t check = 0;

  this->Cam.n_relaunch_attempt = 0;
  
  /* launch cameras, if required */
  if (this->CmdLine->cam_on) {
    
    /* check verbosity */
    if (this->CmdLine->cam_verbose) {
      this->Cam.SetVerbose();
    }
  
    check = this->Cam.CollectData();
  
    /* react if launched with errors */
    while ((check != 0) &&
	(this->Cam.n_relaunch_attempt < N_TRY_RELAUNCH)) {

      std::cout << "Camera relaunch attempt " << this->Cam.n_relaunch_attempt << std::endl;
      clog << "info: " << logstream::info << "camera relaunch attempt no. " << this->Cam.n_relaunch_attempt << std::endl;

      std::cout << "Rebooting the cameras" << std::endl;
      clog << "info: " << logstream::info << "rebooting the cameras" << std::endl;

      /* reboot the cameras */
      this->Lvps.SwitchOff(LvpsManager::CAMERAS);
      sleep(1);
      this->Lvps.SwitchOff(LvpsManager::CAMERAS);
      sleep(1);

      std::cout << "Relaunching the cameras" << std::endl;
      clog << "info: " << logstream::info << "relaunching the cameras" << std::endl;
      
      /* relaunch */
      check = this->Cam.CollectData();
      this->Cam.n_relaunch_attempt++;
    }
    if (check != 0) {

      std::cout << "ERROR: cameras failed to relaunch" << std::endl;
      clog << "error: " << logstream::error << "cameras failed to relaunch" << std::endl;

    }
  }
  
  return 0;
}


/* monitor the photodiode data to determine the instrument mode */
int RunInstrument::MonitorLightLevel() {

  /* launch a thread to watch the photodiode measurements */
  std::thread light_monitor (&RunInstrument::PollLightLevel, this);

  /* detach */
  light_monitor.detach();
  
  return 0;
}


/* set the stop signal in a thread-safe way */
int RunInstrument::SetStop() {

  {
    std::unique_lock<std::mutex> lock(this->_m_stop);
    this->_stop = true;
  } /* release mutex */
  
  return 0;
}


/* check if stop signal sent in a thread-safe way */
bool RunInstrument::CheckStop() {
  bool stop_status;

  {
    std::unique_lock<std::mutex> lock(this->_m_stop);
    stop_status = this->_stop;
  } /* release mutex */

  return stop_status;
}


int RunInstrument::PollLightLevel() {

  /* different procedure for day and night */
  while (!this->CheckStop()) {
    
    switch(GetInstMode()) {
    case NIGHT:
      /* check the output of the analog acquisition is below threshold */
      sleep(LIGHT_POLL_TIME);
      if (this->Daq.Analog->CompareLightLevel()) {
	/* switch mode to DAY */
	this->Daq.Notify();
	this->SetInstMode(DAY);
      }
      break;
      
    case DAY:
      /* check the output of analog acquisition is above threshold */
      sleep(LIGHT_POLL_TIME);
      if (!this->Data.Analog->CompareLightLevel()) {
	/* switch mode to NIGHT */
	this->Data.Notify();
	this->SetInstMode(NIGHT);
      }
      break;
      
    case INST_UNDEF:
      std::cout << "ERROR: instrument mode is undefined" << std::endl;
      break;
    }
    
  }

  /* reached only when stop signal sent */
  std::cout << "exiting light monitoring thread..." << std::endl;
 
  return 0;
}

/* interface to the whole data acquisition */
int RunInstrument::Acquisition() {

  std::cout << "starting acqusition run..." <<std::endl; 
  clog << "info: " << logstream::info << "starting acquisition run" << std::endl;

  /* clear the FTP server */
  CpuTools::ClearFolder(DATA_DIR);
  
  /* add acquisition with cameras if required */
  this->LaunchCam();

  /* check for light level in the background */
  this->MonitorLightLevel();
  
  /* select SCURVE or STANDARD acquisition */
  if (this->Zynq.telnet_connected) {
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
  }

  
  /* reached for SCURVE acq and instrument mode change */
  if (this->CmdLine->cam_on) {
    this->Cam.KillCamAcq();
  }
  return 0;
}


/* night time operational procedure */
int RunInstrument::NightOperations() {

  clog << "info: " << logstream::info << "entering NIGHT mode" << std::endl;
  std::cout << "entering NIGHT mode..." << std::endl;

  /* reset mode switching */
  this->Daq.Reset();
    
  /* set the HV as required */
  if (this->CmdLine->hvps_on) {
    HvpsSwitch();
  }

  /* start data acquisition */
  /* acquisition runs until signal to switch mode */
  Acquisition();
  
  /* turn off HV */
  if (this->Zynq.telnet_connected) {
    this->CmdLine->hvps_status = ZynqManager::OFF;
    HvpsSwitch();
  }
  
  return 0;
}


/* day time operational procedure */
int RunInstrument::DayOperations() {

  clog << "info: " << logstream::info << "entering DAY mode" << std::endl;
  std::cout << "entering DAY mode..." << std::endl;

  /* reset mode switching */
  this->Data.Reset();
  
  /* data reduction runs until signal to switch mode */
  this->Data.Start();
  
  return 0;
}


/* shut down upon SIGINT */
int RunInstrument::Stop() {

  /* stop running threads */
  clog << "info: " << logstream::info << "stopping joinable threads..." << std::endl;
  std::cout << "stopping joinable threads..." << std::endl;
  
  switch(GetInstMode()) {
  case NIGHT:

    this->Daq.Notify();
    break;
    
  case DAY:

    this->Data.Notify();
    break;
    
  case INST_UNDEF:
    
    clog << "info: " << logstream::info << "instrument mode undefined, no threads to stop" << std::endl;
    std::cout << "instrument mode undefined, no threads to stop" << std::endl;
    break;
    
  }
  
  /* kill detached threads */
  clog << "info: " << logstream::info << "stopping deatached threads..." << std::endl;
  std::cout << "stopping deatached threads..." << std::endl;
  this->Cam.KillCamAcq();
  this->Usb.KillDataBackup();

  /* turn off all subsystems */
  this->Lvps.SwitchOff(LvpsManager::ZYNQ);
  this->Lvps.SwitchOff(LvpsManager::CAMERAS);
  this->Lvps.SwitchOff(LvpsManager::HK);    

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
  this->StartUp();

  /* check for execute-and-exit commands which require config */
  if (this->CmdLine->hvps_switch) {
    HvpsSwitch();
    return 0;
  }
  else if (this->CmdLine->debug_mode) {
    DebugMode();
    return 0;
  }
 
  /* check systems and operational mode */
  this->CheckSystems();

  if (!this->Zynq.telnet_connected) {
    std::cout << "no Zynq connection, exiting the program" << std::endl;
    return 1;
  }

  /* launch data backup in background */
  this->Usb.RunDataBackup();
  
  /* launch background process to monitor the light level */
  this->MonitorLightLevel();

  /* enable signal handling */
  signal(SIGINT, SignalHandler);  
  
  /* enter instrument mode */
  while (!signal_shutdown.load()) {
    switch(GetInstMode()) {
      
    
      /* NIGHT OPERATIONS */
      /*------------------*/
    case NIGHT:
      this->NightOperations();
      break;

      
      /* DAY OPERATIONS */
      /*----------------*/
    case DAY:
      this->DayOperations();
      break;

      /* UNDEFINED */
      /*-----------*/
    case INST_UNDEF:
      std::cout << "ERROR: instrument mode undefined, cannot start acquisition" << std::endl;
      
      break;
    } /* end switch statement */
   
  } /* end loop when stop signal sent */

  /* program shutdown */
  SetStop();
  this->Stop();

  std::cout << "exiting the program..." << std::endl;
 
  return 0;
}

