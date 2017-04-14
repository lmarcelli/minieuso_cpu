/*-------------------------------
                                 
TEST CPU PROGRAM                 
V1.1: April 2017                 
                                 
Full PDM data acquisition chain	 
                                  
Francesca Capel                  
capel.francesca@gmail.com         
                                 
--------------------------------*/
#include "globals.h"

int main(void) {
  /* start-up */
  printf("TEST CPU SOFTWARE Version: %.2f Date: %s\n", VERSION, VERSION_DATE_STRING);
  /* create the log file */
  std::ofstream log_file(log_name,std::ios::out);
  logstream clog(log_file, logstream::all);
  //clog << "warning!" << setlevel(logstream::warning) << std::endl;
  //clog << "testing:" << std::setw(30) << "error!"  << setlevel(logstream::error) << std::endl;
  clog << std::endl;
  clog << "info: " << logstream::info << "log created" << std::endl;
  
  /* reload and parse the configuration file */
  std::string config_file = "../config/dummy.conf";
  std::string config_file_local = "../config/dummy_local.conf";
  configure(config_file, config_file_local);

  /* test the connection to the zynq board */
  std::string ip_address = "130.237.34.97";
  if (check_telnet(ip_address, 12345)) {
    printf()
  }

  //
  
  return 0; 
}


  
