/*-------------------------------
                                 
Mini-EUSO CPU software                 
https://github.com/cescalara
                                  
Francesca Capel                  
capel.francesca@gmail.com         
                                 
--------------------------------*/
#include "mecontrol.h"

/* handle SIGINT */
void SignalHandler(int signum) {

  std::cout << "Interrupt signal (" << signum << ") received" << std::endl;  

  /* stop the data acquisition */
  ZynqManager::StopAcquisition();
  std::cout << "Acquisition stopped" << std::endl;  

  /* turn off the HV */
  //ZynqManager::HvpsTurnOff();
  
  /* terminate the program */
  exit(signum);  
}

/* clear the FTP directory */
void ClearFTP() {
  DIR * theFolder = opendir(DATA_DIR);
  struct dirent * next_file;
  char filepath[256];   
  while ((next_file = readdir(theFolder)) != NULL) {
    sprintf(filepath, "%s/%s", DATA_DIR, next_file->d_name);
    remove(filepath);
  }
  closedir(theFolder);    
}

/* an acquisition run */
int acq_run(UsbManager * UManager, Config * ConfigOut, ZynqManager * ZqManager, DataAcqManager * DaqManager,
	    CamManager * CManager, CmdLineInputs * CmdLine) {

  std::cout << "starting acqusition run..." <<std::endl; 
  clog << "info: " << logstream::info << "starting acquisition run" << std::endl;
  
  /* clear the FTP server */
  ClearFTP();
  
  /* enable signal handling */
  signal(SIGINT, SignalHandler);  
  
  /* define data backup */
  UManager->DataBackup();
  
  if(CmdLine->hv_on == true) {
    
    ZqManager->HvpsTurnOn(ConfigOut->cathode_voltage, ConfigOut->dynode_voltage);

    ZqManager->HvpsStatus();
    
    /* set the DAC */
    ZqManager->SetDac(ConfigOut->dac_level); 
    
  }
  else {
    /* set the DAC to the pedestal */
    ZqManager->SetDac(750); 
  }
  
  /* take data */
  if (CmdLine->trig_on == true) {
    DaqManager->CollectData(ConfigOut, ZynqManager::MODE3, CmdLine->single_run, CmdLine->test_zynq_on);
  }
  else {
    DaqManager->CollectData(ConfigOut, ZynqManager::MODE2, CmdLine->test_mode_num, CmdLine->single_run,
			    CmdLine->test_zynq_on, CmdLine->keep_zynq_pkt);
  }

  /* turn off the HV */
  if (CmdLine->hv_on == true) {
    ZqManager->HvpsTurnOff();
  }
  
/* wait for backup to complete */
  //run_backup.join();
  return 0;
}

/* main program */
/*--------------*/
int main(int argc, char ** argv) {

  /* definitions */
  std::string config_dir(CONFIG_DIR);
  InputParser input(argc, argv);
  ZynqManager ZqManager;
  UsbManager UManager;
  CamManager CManager;
  LvpsManager Lvps;
  DataAcqManager DaqManager;

  /* parse command line options */
  CmdLineInputs * CmdLine = input.ParseCmdLineInputs();
  if (CmdLine->help) {
    /* exit when help message called */
    return 0;
  }

  /* debug/test mode */
  /*-----------------*/
  if(CmdLine->debug_mode == true) {
    std::cout << "Mini-EUSO software debug mode" << std::endl;
    
    /* check the log level */
    if (CmdLine->log_on == true) {
      clog.change_log_level(logstream::all);     
    }
    clog << std::endl;
    clog << "info: " << logstream::info << "log created" << std::endl;

    if (CmdLine->lvps_on == true) {
      /* testing the LVPS switching */
      std::cout << "Testing the LVPS switching" << std::endl;

      Lvps.SwitchOn(CmdLine->lvps_subsystem);
      sleep(2);
      Lvps.SwitchOff(CmdLine->lvps_subsystem);
      sleep(2);
    }
    
    /* make a test Zynq packet */
    //DataAcqManager::WriteFakeZynqPkt();
    //DataAcqManager::ReadFakeZynqPkt();
    
    return 0;
  }

  /* LVPS switching mode */
  /*---------------------*/
  if (CmdLine->lvps_on) {
    switch (CmdLine->lvps_status) {
    case LvpsManager::ON:
      
      switch (CmdLine->lvps_subsystem) {
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
      Lvps.SwitchOn(CmdLine->lvps_subsystem);
      break;

    case LvpsManager::OFF:
      switch (CmdLine->lvps_subsystem) {
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
      Lvps.SwitchOff(CmdLine->lvps_subsystem);
      break;
      
    case LvpsManager::UNDEF:
      std::cout << "Cannot switch subsystem, on/off undefined" << std::endl;
      break;
    }
    
    return 0;
  }
  
  /* start-up */
  /*----------*/
  
  printf("TEST CPU SOFTWARE Version: %.2f Date: %s\n", VERSION, VERSION_DATE_STRING);

  /* check the log level */
  if (CmdLine->log_on == true) {
    clog.change_log_level(logstream::all);
  }
  clog << std::endl;
  clog << "info: " << logstream::info << "log created" << std::endl;

 
  /* reload and parse the configuration file */
  std::string config_file = config_dir + "/dummy.conf";
  std::string config_file_local = config_dir + "/dummy_local.conf";
  ConfigManager CfManager(config_file, config_file_local);
  Config * ConfigOut = CfManager.Configure();

  /* check for command line override to config */
  if (CmdLine->dv != -1) {
    ConfigOut->dynode_voltage = CmdLine->dv;
  }
  if (CmdLine->hvdac != -1) {
    ConfigOut->dac_level = CmdLine->hvdac;
  }
  
  /* turn on all systems */
  std::cout << "switching on all systems..." << std::endl;
  if (CmdLine->cam_on ==true) {
    Lvps.SwitchOn(LvpsManager::CAMERAS);
  }
  Lvps.SwitchOn(LvpsManager::HK);
  Lvps.SwitchOn(LvpsManager::ZYNQ);

  /* wait for boot */
  std::cout << "waiting for boot" << std::endl;
  sleep(BOOT_TIME);
  
  /* test the connection to the zynq board */
  ZqManager.CheckTelnet();
  
  /* check the instrument and HV status */
  ZqManager.InstStatus();
  ZqManager.HvpsStatus();

  /*-----*/
  if (CmdLine->sc_on == true) {

    if (CmdLine->hv_on == true) {

      /* turn on the HV */
      /* check for command line override */
      ZqManager.HvpsTurnOn(ConfigOut->cathode_voltage, ConfigOut->dynode_voltage);	

      /* check the status */
      ZqManager.HvpsStatus();
    }
    
      /* take an scurve */
      DaqManager.CollectSc(ConfigOut);

      /* turn off the HV */  
      ZqManager.HvpsTurnOff();
      
      /* check the status */
      ZqManager.HvpsStatus();
      
    /* then exit */
    return 0;
  }

  /* collect camera data if required */
  if (CmdLine->cam_on == true) {
    std::thread collect_cam_data (&CamManager::CollectData, CManager);
    
    /* take data */
    if (CmdLine->trig_on == true) {
      DaqManager.CollectData(ConfigOut, ZynqManager::MODE3, CmdLine->single_run);
    }
    else {
      DaqManager.CollectData(ConfigOut, ZynqManager::MODE2, CmdLine->single_run);
    }
    collect_cam_data.join();
  }
  
  /* data acquisition */
  acq_run(&UManager, ConfigOut, &ZqManager, &DaqManager,
	  &CManager, CmdLine);

  /* turn off all systems */
  std::cout << "switching off all systems..." << std::endl;
  Lvps.SwitchOff(LvpsManager::CAMERAS);
  Lvps.SwitchOff(LvpsManager::HK);
  Lvps.SwitchOff(LvpsManager::ZYNQ);

  /* wait for switch off */
  sleep(5);

  /* clean up */
  delete ConfigOut;
  return 0; 
}

  
