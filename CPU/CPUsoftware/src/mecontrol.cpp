/*-------------------------------
                                 
Mini-EUSO CPU software                 
https://github.com/cescalara
                                  
Francesca Capel                  
capel.francesca@gmail.com         
                                 
--------------------------------*/
#include "mecontrol.h"

/* main program */
/*--------------*/
int main(int argc, char ** argv) {

  /* definitions */
  std::string config_dir(CONFIG_DIR);
  InputParser input(argc, argv);

  /* parse command line options */
  CmdLineInputs * CmdLine = input.ParseCmdLineInputs();
  if (CmdLine->help) {
    /* exit when help message called */
    return 0;
  }

  /* run instrument according to specifications */
  RunInstrument MiniEuso(CmdLine);
  MiniEuso.Start();

  /* clean up */
  delete Input, MiniEuso;
  return 0; 
}

  
