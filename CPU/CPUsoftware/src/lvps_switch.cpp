#include "LvpsManager.h"
#include "mecontrol.h"

/* code to switch the LVPS from command line */
int main(int argc, char ** argv) {

  InputParser input(argc, argv);

  /* initialise */
  LvpsManager::SubSystem subsystem = LvpsManager::ZYNQ;
  LvpsManager Lvps;
  
  /* parse command line options */
  /* set desired status */
  if(input.cmdOptionExists("-on")){

    const std::string & subsystem_str = input.getCmdOption("-on");
    if (!subsystem_str.empty()) {
      if (subsystem_str.compare("zynq")) {
	subsystem = LvpsManager::ZYNQ;
      }
      else if (subsystem_str.compare("cam")) {
	subsystem = LvpsManager::CAMERAS;
      }
      else if (subsystem_str.compare("hk")) {
	subsystem = LvpsManager::HK;
      }
      else {

	std::cout << "Usage: lvps_switch -on subsystem" << std::endl;
	std::cout << "or: lvps_switch -off subsystem" << std::endl;
	std::cout << std::endl;
	std::cout << "subsytems: zynq, cam or hk" << std::endl;
	
      }
    }
    
    Lvps.SwitchOn(subsystem);
     
  }
  else if (input.cmdOptionExists("-off")) {

    const std::string & subsystem_str = input.getCmdOption("-off");

    if (!subsystem_str.empty()) {
      if (subsystem_str.compare("zynq")) {
	subsystem = LvpsManager::ZYNQ;
      }
      else if (subsystem_str.compare("cam")) {
	subsystem = LvpsManager::CAMERAS;
      }
      else if (subsystem_str.compare("hk")) {
	subsystem = LvpsManager::HK;
      }
      else {

	std::cout << "Usage: lvps_switch -on swubsystem" << std::endl;
	std::cout << "or: lvps_switch -off subsystem" << std::endl;
	std::cout << std::endl;
	std::cout << "subsytems: zynq, cam or hk" << std::endl;
    
      }
    }

    Lvps.SwitchOff(subsystem);
      
  }
  else {

    std::cout << "Usage: lvps_switch -on swubsystem" << std::endl;
    std::cout << "or: lvps_switch -off subsystem" << std::endl;
    std::cout << std::endl;
    std::cout << "subsytems: zynq, cam or hk" << std::endl;
    
  }
  
  return 0;
}
