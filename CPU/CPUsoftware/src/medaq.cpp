/*-------------------------------
                                 
Mini-EUSO CPU software                 
V2.0: September 2017                 
                                  
Francesca Capel                  
capel.francesca@gmail.com         
                                 
--------------------------------*/
#include "globals.h"
#include <algorithm>
#include <vector>

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

int main(int argc, char ** argv) {

  /* definitions */
  std::string config_dir(CONFIG_DIR);
  InputParser input(argc, argv);
  
  /* parse command line options */
  if(input.cmdOptionExists("-hv")){
    bool hv_on = true;
  }
  
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

  /* typical run */
  /*-------------*/
  printf("Starting acquisition run\n");
  clog << "info: " << logstream::info << "starting acquisition run" << std::endl;

  /* enable signal handling */
  signal(SIGINT, SignalHandler);  

  /* define data backup */
  uint8_t num_storage_dev = LookupUsb();
  std::thread run_backup (DefDataBackup, num_storage_dev);
  
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

  Scurve(ConfigOut->scurve_start, ConfigOut->scurve_step, ConfigOut->scurve_stop, ConfigOut->scurve_acc);

  check_sc.join();
  
  /* set the DAC level */
  SetDac(500); // in pedestal to give non-zero values with no HV

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
  
  /* clean up */
  delete ConfigOut;
  return 0; 
}


  
