#ifndef _RUN_INSTRUMENT_H
#define _RUN_INSTRUMENT_H

#include "DataAcqManager.h"
#include "InputParser.h"

/* class to handle different instrument operation modes */
class RunInstrument {
public:
  CmdLineInputs * CmdLine;
  RunInstrument(CmdLineInputs * CmdLine);
private:
  
};

#endif
/* _RUN_INSTRUMENT_H */
