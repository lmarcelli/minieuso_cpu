#ifndef _RUN_INSTRUMENT_H
#define _RUN_INSTRUMENT_H

#include "log.h"
#include "ConfigManager.h"
#include "LvpsManager.h"
#include "DataAcqManager.h"
#include "InputParser.h"

/* number of seconds CPU waits for other systems to boot */
#define BOOT_TIME 4

/* class to handle different instrument operation modes */
class RunInstrument {
public:
  CmdLineInputs * CmdLine;
  enum InstrumentMode : uint8_t {
    NIGHT = 0,
    DAY = 1,
    UNDEF = 2,
  };
  InstrumentMode current_mode;

  Config * ConfigOut;
  ZynqManager ZqManager;
  UsbManager UManager;
  CamManager CManager;
  LvpsManager Lvps;
  DataAcqManager DaqManager;

  RunInstrument(CmdLineInputs * CmdLine);
  int Start();
  //int Stop();
  
private:
  int StartUp();
  //int RunAcquisition();
  //int SwitchMode();
};

#endif
/* _RUN_INSTRUMENT_H */
