#include "LvpsManager.h"

/* default constructor */
LvpsManager::LvpsManager() {
  this->cam_status = UNDEF;
  this->hk_status = UNDEF;
}

#ifndef __APPLE__
/* get the status of a subsystem */
Status LvpsManager::GetStatus(SubSystem sub_system) {
  Status sub_system_status = UNDEF;

  switch (sub_system) {
  case CAMERAS:
    sub_system_status = this->cam_status;
    break;
  case HK:
    sub_system_status = this->HK_status;
    break;   
  }

  return sub_system_status;
}

/* switch on a subsystem */
int LvpsManager::SwitchOn(SubSystem sub_system) {
  Status sub_system_status = UNDEF;
  int exec_check = 0;
  
  switch (sub_system) {
  case CAMERAS:
    SetPulseP0(CAMERA_PORT_ON); 
    this->cam_status = ON;
    break;
  case HK:
    SetPulseP0(HK_PORT_ON);
    this->hk_status = ON;
    break;
  }
  
  return 0;
}

/* switch off a subsystem */
int LvpsManager::SwitchOff(SubSystem sub_system) {
  Status sub_system_status = UNDEF;
  int exec_check = 0;
  
  switch (sub_system) {
  case CAMERAS:
    SetPulseP0(CAMERA_PORT_OFF); 
    this->cam_status = OFF;
    break;
  case HK:
    SetPulseP0(HK_PORT_OFF);
    this->hk_status = OFF;
    break;
  }
  
  return 0;
}


/* initialise the aDIO ports */
int LvpsManager::InitPorts() {
  int aDIO_ReturnVal;
 
  aDIO_ReturnVal = OpenDIO_aDIO(&aDIO_Device, minor_number);
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  OpenDIO_aDIO(%u) FAILED: MinorNumber(= %u) maybe incorrect",
	  minor_number, minor_number);
    clog << "error: " << logstream::error << "could not initialise CPU aDIO ports" << std::endl;
    return 1;
  }
  
  return 0;
}
/* write the direction of P0 */
int LvpdManager::SetDirP0(uint8_t port_config) {
  int aDIO_ReturnVal;
  uint8_t P0Bits[8];
  int Bit = 0;

  /* write the direction of port 0 */
  for (Bit = 0; Bit < 8; Bit++) {
    P0Bits[Bit] = (port_config >> Bit) & 0x01;
  }
  
  /* set the bits of port 0 */
  aDIO_ReturnVal =
    LoadPort0BitDir_aDIO(aDIO_Device, P0Bits[7],
			 P0Bits[6], P0Bits[5],
			 P0Bits[4], P0Bits[3],
			 P0Bits[2], P0Bits[1],
			 P0Bits[0]);
  /* sleep 1 ms */
  usleep(ONE_MILLISEC);
  
  /* check the return value */
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  LoadPort0bitDir_aDIO() FAILED");
    clog << "error: " << logstream::error << "could not set direction of CPU aDIO ports to " << port_config << std::endl;
    return 1;
  }
  
  return 0;
}

int LvpsManager::SetValP0(PortValue port_value) {
  int aDIO_ReturnVal;
  
  switch (port_value) {
  case HIGH:
    aDIO_ReturnVal =
      WritePort_aDIO(aDIO_Device, 0, HIGH);
    break;
  case LOW:
    aDIO_ReturnVal =
      WritePort_aDIO(aDIO_Device, 0, LOW);
    break;
  }
 
  usleep(ONE_MILLISEC); 

  /* check the return */
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  WritePort_aDIO() FAILED");
    clog << "error: " << logstream::error << "could not set value of CPU aDIO P0 " << port_value << std::endl; 
    return 1;
  }

  return 0;
}

int LvpsManager::CloseDev() {
 int aDIO_ReturnVal;
  
  aDIO_ReturnVal = CloseDIO_aDIO(aDIO_Device);
  if (aDIO_ReturnVal) {
    clog << "error: " << logstream::error << "could not close CPU aDIO " << aDIO_ReturnVal << std::endl;
  }

  return 0;
}

/* deliver a 5V, 10 ms pulse to a certain pin of P0 */
int LvpsManager::SetPulseP0(uint8_t port_config) {
  int exec_ret = 0;

  /* initialise */
  InitPorts();
  
  /* set specified output */
  SetDirP0(port_config);
 
  /* write high */
  SetValP0(HIGH);
   
  /* sleep for 9 ms */
  usleep(9 * ONE_MILLISEC);

  /* write low */
  SetValP0(LOW);

  /* clean up and exit */
  CloseDev();
    
  return 0;
}

#endif /* __APPLE__ */
