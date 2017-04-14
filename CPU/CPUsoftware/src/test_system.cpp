/*-------------------------------\
                                 |
TEST CPU PROGRAM                 |
V1.1: April 2017                 |
                                 |
Full PDM data acquisition chain	 |
                                 | 
Francesca Capel                  |
capel.francesca@gmail.com        | 
                                 |
--------------------------------*/
#include "globals.h"

int main(void) {
  /* start-up */
  /*----------*/
  
  /* create the log file */
  std::ofstream log_file(log_name,std::ios::out);
  logstream clog(log_file, logstream::all);
  //clog << "warning!" << setlevel(logstream::warning) << std::endl;
  //clog << "testing:" << std::setw(30) << "error!"  << setlevel(logstream::error) << std::endl;
  clog << std::endl;
  clog << "info: " << logstream::info << "log created" << std::endl;
  
  /* reload and parse the configuration file */
  configure(CONFIG_FILE_USB);
  
  /* check communication with systems */
  //check_IP_com(ZYNQ_IP_ADDRESS);
  //printf("checked IP\n");
  
  /* launch different systems */
  
  return 0; 
}


  
