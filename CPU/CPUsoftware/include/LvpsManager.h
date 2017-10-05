#ifndef _LVPS_MANAGER_H
#define _LVPS_MANAGER_H

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#ifndef __APPLE__
#include "aDIO_library.h"
#endif /* __APPLE__ */
#include "log.h"

#define ONE_MILLISEC 1000

#define CAMERA_PORT_ON 0x01
#define CAMERA_PORT_OFF 0x02
#define HK_PORT_ON 0x04
#define HK_PORT_OFF 0x08
#define CC_LVPS_HK 0x10
#define RET_CC_LVPS_HK 0x20

#define HIGH_VAL 0xFF
#define LOW_VAL 0x00

class LvpsManager {
public:
  enum Status : uint8_t {
    OFF = 0,
    ON = 1,
    UNDEF = 2,
  };
  enum SubSystem : uint8_t {
    CAMERAS = 0,
    HK = 1,    
  };

  Status cam_status;
  Status hk_status;
  
  Status GetStatus(SubSystem sub_system);
  int SwitchOn(SubSystem sub_system);
  int SwitchOff(SubSystem sub_system);
  static int Check(SubSystem sub_system);

private:
  static uint32_t minor_number = 0;
  enum PortValue : uint8_t {
    HIGH = 0xFF,
    LOW = 0x00,
  };

  int SetDirP0(uint8_t port_config);
  int SetValP0(PortValue port_value);
  int SetPulseP0(uint8_t port_config);

};


#endif /* LVPS_MANAGER */
