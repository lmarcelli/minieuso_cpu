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

  /* tidying up... */
  /* run data acquisition */
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

  
