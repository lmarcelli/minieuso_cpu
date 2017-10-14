/*-------------------------------
                                 
Mini-EUSO CPU software                 
V2.0: September 2017                 
                                  
Francesca Capel                  
capel.francesca@gmail.com         
                                 
--------------------------------*/
#include "mecontrol.h"

class InputParser{
public:
  InputParser(int &argc, char **argv) {
    for (int i=1; i < argc; i++)
      this->tokens.push_back(std::string(argv[i]));
  }
  
  const std::string& getCmdOption(const std::string &option) const {
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && itr++ != this->tokens.end()){
      return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
  }
  
  bool cmdOptionExists(const std::string &option) const{
    return std::find(this->tokens.begin(), this->tokens.end(), option)
      != this->tokens.end();
  }
private:
  std::vector <std::string> tokens;
};

/* handle SIGINT */
void SignalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received" << std::endl;  
  /* handle the signal */
#ifdef SINGLE_EVENT
  ZynqManager::DataAcquisitionStop();
#else
  ZynqManager::StopAcquisition();
#endif /* SINGLE_EVENT */
  std::cout << "Acquisition stopped" << std::endl;  
  
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

/* a single 2.5 min acquisition run */
int single_acq_run(UsbManager * UManager, Config * ConfigOut, ZynqManager * ZqManager, DataAcqManager * DaqManager,
		   CamManager * CManager, bool hv_on, bool trig_on, bool cam_on, bool sc_on,
		   int hvdac, int dv) {
  
  std::cout << "starting acqusition run..." <<std::endl; 
  clog << "info: " << logstream::info << "starting acquisition run" << std::endl;
  
  /* clear the FTP server */
  ClearFTP();
  
  /* enable signal handling */
  signal(SIGINT, SignalHandler);  
  
  /* define data backup */
  UManager->DataBackup();
  
  if(hv_on == true) {
    
    /* check for command line override */
    if (dv != -1){
        ZqManager->HvpsTurnOn(ConfigOut->cathode_voltage, dv);
    }
    else {
      ZqManager->HvpsTurnOn(ConfigOut->cathode_voltage, ConfigOut->dynode_voltage);
    }

    ZqManager->HvpsStatus();
    
    /* set the DAC  */
    /* check for command line override */
    if (hvdac != -1){
       ZqManager->SetDac(hvdac); 
    }
    else {
      ZqManager->SetDac(ConfigOut->dac_level); 
    }
    
  }
  else {
    /* set the DAC to the pedestal */
    ZqManager->SetDac(750); 
  }
  
#ifdef SINGLE_EVENT    
  /* create the run file */ 
  DaqManager->CreateCpuRun();

  /* take data */
  DaqManager->CollectSc(ConfigOut);
  DaqManager->CollectData(ConfigOut);
  
  /* close the run file */
  DaqManager->CloseCpuRun();
  
#else

  /* take data */
  if (trig_on == true) {
    DaqManager->CollectData(ConfigOut, ZynqManager::MODE3);
  }
  else {
    DaqManager->CollectData(ConfigOut, ZynqManager::MODE2);
    /* collext camera data if required */
    if (cam_on == true) {
      CManager->CollectData();
    }
  }
#endif /* SINGLE_EVENT */
  
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
#ifdef SINGLE_EVENT
  DataAcqManagerSe DaqManager;
#else
  DataAcqManager DaqManager;
#endif /* SINGLE_EVENT */
  
  /* parse command line options */
  bool hv_on = false;
  bool long_acq = false;
  bool debug_mode = false;
  bool log_on = false;
  bool trig_on = false;
  bool cam_on = false;
  bool lvps_on = false;
  bool sc_on = false;
  
  if(input.cmdOptionExists("-hv")){
    hv_on = true;
  }
  if(input.cmdOptionExists("-long")){
    long_acq = true;
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
      LvpsManager::Status camera_status;
      camera_status = Lvps.GetStatus(LvpsManager::CAMERAS);
      std::cout << "camera status: " << camera_status << std::endl;
      Lvps.SwitchOn(LvpsManager::CAMERAS);
      camera_status = Lvps.GetStatus(LvpsManager::CAMERAS);
      std::cout << "camera status on: " << camera_status << std::endl;
    }

    /* testing the passing of voltage via command line */
    const std::string &dynode_voltage = input.getCmdOption("-dv");
    if (!dynode_voltage.empty()){
      std::cout << "dynode voltage: " << dynode_voltage << std::endl;
      int dv = std::stoi(dynode_voltage);
      std::cout << dv << std::endl;
    }
    else {
      std::cout << "no dynode voltage found" << std::endl;
    }
    
    
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

  if (lvps_on == true) {
    /* turn on all systems */
    std::cout << "switching on all systems..." << std::endl;
    Lvps.SwitchOn(LvpsManager::CAMERAS);
    Lvps.SwitchOn(LvpsManager::HK);

    /* wait for boot */
    sleep(5);
  }
  
  /* test the connection to the zynq board */
  ZqManager.CheckTelnet();
  
  /* check the instrument and HV status */
  ZqManager.InstStatus();
  ZqManager.HvpsStatus();

  if (sc_on == true) {
    /* take an scurve */
    DaqManager.CollectSc(ConfigOut);

    /* then exit */
    return 0;
  }

  
  if(long_acq == true){
    /* loop over single acquisition */
    while(1) {
      single_acq_run(&UManager, ConfigOut, &ZqManager, &DaqManager,
		     &CManager, hv_on, trig_on, cam_on, sc_on, hvdac, dv);
    }
    /* never reached */
  }
  else {
    /* single acquisition run */
    single_acq_run(&UManager, ConfigOut, &ZqManager, &DaqManager,
		   &CManager, hv_on, trig_on, cam_on, sc_on, hvdac, dv);
  }

  if (lvps_on == true) {
    /* turn off all systems */
    std::cout << "switching off all systems..." << std::endl;
    Lvps.SwitchOff(LvpsManager::CAMERAS);
    Lvps.SwitchOff(LvpsManager::HK);

    /* wait for switch off */
    sleep(5);
  }
  /* clean up */
  delete ConfigOut;
  return 0; 
}


  
