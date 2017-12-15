#include "LvpsManager.h"
#include "mecontrol.h"

/* code to switch the LVPS from command line */
int main() {

  //InputParser input(argc, argv);

  /* initialise */
  LvpsManager::SubSystem subsystem = LvpsManager::ZYNQ;
  LvpsManager Lvps;
  
  Lvps.SwitchOn(subsystem);
  return 0;
}
