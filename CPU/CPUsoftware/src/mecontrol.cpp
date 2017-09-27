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

/* main program */
/*--------------*/
int main(int argc, char ** argv) {

  /* definitions */
  std::string config_dir(CONFIG_DIR);
  InputParser input(argc, argv);
  ZynqManager ZqManager;
  UsbManager UManager;

  /* parse command line options */
  bool hv_on = false;
  bool long_acq = false;
  bool debug_mode = false;
  bool log_on = false;
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

    /* perform configuration */
    std::string config_dir_mac = "../config";
    std::string config_file = config_dir_mac + "/dummy.conf";
    std::string config_file_local = config_dir_mac + "/dummy_local.conf";
    ConfigManager CfManager(config_file, config_file_local);
    Config * ConfigOut = CfManager.Configure();
   
    if (ConfigOut != NULL) {
      std::cout << "configured parameters: " << std::endl;
      std::cout << ConfigOut->scurve_start << std::endl;
      std::cout << ConfigOut->cathode_voltage << std::endl;
    }
    else {
      std::cout << "NULL" << std::endl;
    }
    
    delete ConfigOut;
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
  
  /* test the connection to the zynq board */
  ZqManager.CheckTelnet();
  
  /* check the instrument and HV status */
  ZqManager.InstStatus();
  ZqManager.HvpsStatus();

  if(long_acq == true){
    /* start infinite loop over acquisition */
    while(1) {
      /* one file run */
      /*-------------*/
      printf("Starting infinite acquisition run\n");
      clog << "info: " << logstream::info << "starting acquisition run" << std::endl;
      
      /* clear the FTP directory */
      DIR * theFolder = opendir(DATA_DIR);
      struct dirent * next_file;
      char filepath[256];
      
      while ((next_file = readdir(theFolder)) != NULL) {
        sprintf(filepath, "%s/%s", DATA_DIR, next_file->d_name);
        remove(filepath);
      }
      closedir(theFolder);
    
      /* enable signal handling */
      signal(SIGINT, SignalHandler);  
      
      /* define data backup */
      uint8_t num_storage_dev = UManager.LookupUsb();
      //std::thread run_backup (UManager.DefDataBackup, num_storage_dev);
      uint8_t backup_ret = UManager.DataBackup(num_storage_dev);
    
      /* create the run file */ 
      std::string current_run_file = CreateCpuRunName(num_storage_dev);
      CreateCpuRun(current_run_file);
      
      /* turn on the HV */
      //HvpsTurnOn(ConfigOut.cathode_voltage, ConfigOut.dynode_voltage);
      
      /* take an scurve */
      std::thread check_sc (ProcessIncomingData, current_run_file, ConfigOut);
      
      ZqManager.Scurve(ConfigOut->scurve_start, ConfigOut->scurve_step, ConfigOut->scurve_stop, ConfigOut->scurve_acc);
      
      check_sc.join();
      
      /* set the DAC level */
      ZqManager.SetDac(ConfigOut->dac_level); 
      
      /* start checking for new files and appending */
      std::thread check_data (ProcessIncomingData, current_run_file, ConfigOut);
      
      /* start the data acquisition */
      ZqManager.DataAcquisitionStart();
      
      /* wait for data acquisition to complete */
      check_data.join();
      
      /* stop the data acquisition */
      ZqManager.DataAcquisitionStop();
      
      /* close the run file */
      CloseCpuRun(current_run_file);
      
    /* wait for backup to complete */
      // run_backup.join();
    }
    
    /* never reached, clean up on interrupt */
    return 0;
  }
  else{
    /* typical run */
    /*-------------*/
    printf("Starting acquisition run\n");
    clog << "info: " << logstream::info << "starting acquisition run" << std::endl;

    /* enable signal handling */
    signal(SIGINT, SignalHandler);  
    
    /* define data backup */
    uint8_t num_storage_dev = UManager.LookupUsb();
    //std::thread run_backup (DefDataBackup, num_storage_dev);
    uint8_t backup_ret = UManager.DataBackup(num_storage_dev);
    
    /* create the run file */ 
    std::string current_run_file = CreateCpuRunName(num_storage_dev);
    CreateCpuRun(current_run_file);
    
    if(hv_on == true) {
      std::cout << "hv on test!" << std::endl;
      /* turn on the HV */
      //HvpsTurnOn(ConfigOut.cathode_voltage, ConfigOut.dynode_voltage);
    }
    
    /* take an scurve */
    std::thread check_sc (ProcessIncomingData, current_run_file, ConfigOut);
    
    ZqManager.Scurve(ConfigOut->scurve_start, ConfigOut->scurve_step, ConfigOut->scurve_stop, ConfigOut->scurve_acc);
    
    check_sc.join();
    
    /* set the DAC level */
    ZqManager.SetDac(500); // in pedestal to give non-zero values with no HV
    
    /* start checking for new files and appending */
    std::thread check_data (ProcessIncomingData, current_run_file, ConfigOut);
    
    /* start the data acquisition */
    ZqManager.DataAcquisitionStart();
    
    /* wait for data acquisition to complete */
    check_data.join();
    
    /* stop the data acquisition */
    ZqManager.DataAcquisitionStop();
    
    /* close the run file */
    CloseCpuRun(current_run_file);

    /* wait for backup to complete */
    //run_backup.join();
  }

  /* clean up */
  delete ConfigOut;
  return 0; 
}


  
