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
  ZynqManager::HvpsTurnOff();
  
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
	    CamManager * CManager, bool hv_on, bool trig_on, bool cam_on, bool sc_on, bool single_run, bool test_zynq_on,  uint8_t test_mode_num, bool keep_zynq_pkt) {

  std::cout << "starting acqusition run..." <<std::endl; 
  clog << "info: " << logstream::info << "starting acquisition run" << std::endl;
  
  /* clear the FTP server */
  ClearFTP();
  
  /* enable signal handling */
  signal(SIGINT, SignalHandler);  
  
  /* define data backup */
  UManager->DataBackup();
  
  if(hv_on == true) {
    
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
  if (trig_on == true) {
    DaqManager->CollectData(ConfigOut, ZynqManager::MODE3, single_run, test_zynq_on);
  }
  else {
    DaqManager->CollectData(ConfigOut, ZynqManager::MODE2, test_mode_num, single_run, test_zynq_on, keep_zynq_pkt);
  }

  /* turn off the HV */
  if (hv_on == true) {
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
  bool hv_on = false;
  bool debug_mode = false;
  bool log_on = false;
  bool trig_on = false;
  bool cam_on = false;
  bool lvps_on = false;
  bool sc_on = false;
  bool single_run = false; 
  bool test_zynq_on = false;
  bool keep_zynq_pkt = false;
  
  if(input.cmdOptionExists("-hv")){
    hv_on = true;
  }
  if(input.cmdOptionExists("-short")){
    single_run = true;
  }
  if(input.cmdOptionExists("-db")){
    debug_mode = true;
  }
  if(input.cmdOptionExists("-log")){
    log_on = true;
  }
  if(input.cmdOptionExists("-trig")){
    trig_on = true;
  }
  if(input.cmdOptionExists("-cam")){
    cam_on = true;
  }
  if(input.cmdOptionExists("-lvps")){
    lvps_on = true;
  }
  if(input.cmdOptionExists("-scurve")){
    sc_on = true;
  }
  if(input.cmdOptionExists("-test_zynq")){
    test_zynq_on = true;
  }
  if(input.cmdOptionExists("-keep_zynq_pkt")){
    keep_zynq_pkt = true;
  }
  
  
  int dv = -1;
  const std::string &dynode_voltage = input.getCmdOption("-dv");
  if (!dynode_voltage.empty()){
    dv = std::stoi(dynode_voltage);
  }
  int hvdac = -1;
  const std::string &hv_dac = input.getCmdOption("-hvdac");
  if (!hv_dac.empty()){
    hvdac = std::stoi(hv_dac);
  }
  uint8_t test_mode_num = -1;
  const std::string &test_mode = input.getCmdOption("-test_zynq");
  if (!test_mode.empty()){
    test_mode_num = std::stoi(test_mode);
  }


  /* debug/test mode */
  /*-----------------*/
  if(debug_mode == true) {
    std::cout << "Mini-EUSO software debug mode" << std::endl;
    
    /* check the log level */
    if (log_on == true) {
      clog.change_log_level(logstream::all);     
    }
    clog << std::endl;
    clog << "info: " << logstream::info << "log created" << std::endl;

    if (lvps_on == true) {

      /* testing the LVPS switching */
      std::cout << "Testing the LVPS switching" << std::endl;

      std::cout << "switch OFF HK" << std::endl;
      Lvps.SwitchOff(LvpsManager::HK);
      sleep(2);
      std::cout << "switch ON HK" << std::endl;
      Lvps.SwitchOn(LvpsManager::HK);
      sleep(2);
      std::cout << "switch OFF Zynq" << std::endl;
      Lvps.SwitchOff(LvpsManager::ZYNQ);
      sleep(2);
      std::cout << "switch ON Zynq" << std::endl;
      Lvps.SwitchOn(LvpsManager::ZYNQ);
      sleep(2);
      std::cout << "switch OFF cameras" << std::endl;
      Lvps.SwitchOff(LvpsManager::CAMERAS);
      sleep(2);
      std::cout << "switch ON cameras" << std::endl;
      Lvps.SwitchOn(LvpsManager::CAMERAS);
      sleep(2);
      
    }
    
    /* make a test Zynq packet */
    //DataAcqManager::WriteFakeZynqPkt();
    //DataAcqManager::ReadFakeZynqPkt();
    
    return 0;
  }

  /* start-up */
  /*----------*/
  
  printf("TEST CPU SOFTWARE Version: %.2f Date: %s\n", VERSION, VERSION_DATE_STRING);

  /* check the log level */
  if (log_on == true) {
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
  if (dv != -1) {
    ConfigOut->dynode_voltage = dv;
  }
  if (hvdac != -1) {
    ConfigOut->dac_level = hvdac;
  }
  
  if (lvps_on == true) {
    /* turn on all systems */
    std::cout << "switching on all systems..." << std::endl;
    if (cam_on ==true) {
      Lvps.SwitchOn(LvpsManager::CAMERAS);
    }
    Lvps.SwitchOn(LvpsManager::HK);
    Lvps.SwitchOn(LvpsManager::ZYNQ);

    /* wait for boot */
    sleep(BOOT_TIME);
  }
  
  /* test the connection to the zynq board */
  ZqManager.CheckTelnet();
  
  /* check the instrument and HV status */
  ZqManager.InstStatus();
  ZqManager.HvpsStatus();

  if (sc_on == true) {

    if (hv_on == true) {

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
  if (cam_on == true) {
    std::thread collect_cam_data (&CamManager::CollectData, CManager);
    
    /* take data */
    if (trig_on == true) {
      DaqManager.CollectData(ConfigOut, ZynqManager::MODE3, single_run);
    }
    else {
      DaqManager.CollectData(ConfigOut, ZynqManager::MODE2, single_run);
    }
    collect_cam_data.join();
  }
  
  /* data acquisition */
  acq_run(&UManager, ConfigOut, &ZqManager, &DaqManager,
	  &CManager, hv_on, trig_on, cam_on, sc_on,
	  single_run, test_zynq_on, test_mode_num, keep_zynq_pkt);

  if (lvps_on == true) {
    /* turn off all systems */
    std::cout << "switching off all systems..." << std::endl;
    Lvps.SwitchOff(LvpsManager::CAMERAS);
    Lvps.SwitchOff(LvpsManager::HK);
    Lvps.SwitchOff(LvpsManager::ZYNQ);

    /* wait for switch off */
    sleep(5);
  }
  /* clean up */
  delete ConfigOut;
  return 0; 
}


  
