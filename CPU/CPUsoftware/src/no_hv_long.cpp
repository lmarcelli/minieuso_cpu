/*-------------------------------
                                 
TEST CPU PROGRAM: LONG RUN                 
V1.9: August 2017                 
                                 
Full PDM data acquisition chain	 
Acquires continuously CPU files of 25 packets 
each and is stopped by interrupt
                                  
Francesca Capel                  
capel.francesca@gmail.com         
                                 
--------------------------------*/
#include "globals.h"

int main(void) {

  /* definitions */
  std::string config_dir(CONFIG_DIR);

  /* start-up */
  /*----------*/
  printf("TEST CPU SOFTWARE Version: %.2f Date: %s\n", VERSION, VERSION_DATE_STRING);

  /* create the log file */
  std::ofstream log_file(log_name,std::ios::out);
  logstream clog(log_file, logstream::all);
  clog << std::endl;
  clog << "info: " << logstream::info << "log created" << std::endl;

  /* reload and parse the configuration file */
  std::string config_file = config_dir + "/dummy.conf";
  std::string config_file_local = config_dir + "/dummy_local.conf";
  
  Config * ConfigOut = Configure(config_file, config_file_local);

  /* test the connection to the zynq board */
  CheckTelnet(ZYNQ_IP, TELNET_PORT);
  
  /* check the instrument and HV status */
  InstStatus();
  HvpsStatus();

  /* start infinite loop over acquisition */
  while(1) {
    /* one file run */
    /*-------------*/
    printf("Starting acquisition run\n");
    clog << "info: " << logstream::info << "starting acquisition run" << std::endl;

    /* clear the FTP directory */
    std::string data_dir(DATA_DIR);
    DIR * theFolder = opendir(data_dir);
    struct dirent * next_file;
    char filepath[256];

    while ((next_file = readdir(theFolder)) != NULL) {
        sprintf(filepath, "%s/%s", data_dir, next_file->d_name);
        remove(filepath);
    }
    closedir(theFolder);
    
    /* enable signal handling */
    signal(SIGINT, SignalHandler);  

    /* define data backup */
    uint8_t num_storage_dev = LookupUsb();
    std::thread run_backup (DefDataBackup, num_storage_dev);
    
    /* create the run file */ 
    std::string current_run_file = CreateCpuRunName(num_storage_dev);
    CreateCpuRun(current_run_file);

    /* turn on the HV */
    //HvpsTurnOn(ConfigOut.cathode_voltage, ConfigOut.dynode_voltage);

    /* take an scurve */
    std::thread check_sc (ProcessIncomingData, current_run_file, ConfigOut);

    Scurve(ConfigOut->scurve_start, ConfigOut->scurve_step, ConfigOut->scurve_stop, ConfigOut->scurve_acc);

    check_sc.join();
  
    /* set the DAC level */
    SetDac(DAC_LEVEL); 

    /* start checking for new files and appending */
    std::thread check_data (ProcessIncomingData, current_run_file, ConfigOut);
  
    /* start the data acquisition */
    DataAcquisitionStart();

    /* wait for data acquisition to complete */
    check_data.join();

    /* stop the data acquisition */
    DataAcquisitionStop();
  
    /* close the run file */
    CloseCpuRun(current_run_file);
  
    /* wait for backup to complete */
    run_backup.join();
  }

  /* never reached, clean up on interrupt */
  /* clean up */
  delete ConfigOut;
  return 0; 
}


  
