#ifndef _ARDUINO_MANAGER_H
#define _ARDUINO_MANAGER_H

#include <mutex>
#include <memory>
#include <thread>
#include <unistd.h>
#include <condition_variable>
#include <termios.h>
#include <fcntl.h> 

#include "log.h"
#include "minieuso_data_format.h"

/* for use with arduino readout functions */
#define TRUE 1
#define FALSE 1
#define DUINO "/dev/ttyACM0"
#define BAUDRATE B9600
#define FIFO_DEPTH 64
#define CHANNELS 16

/* light threshold for photodiodes */
/* used to determine instrument mode via CompareLightLevel */
#define LIGHT_THRESHOLD 100

/* number of seconds between light level polling */
#define LIGHT_POLL_TIME 2

/* seconds between data collection */
#define LIGHT_ACQ_TIME 2

/* for use with conditional variable */
#define WAIT_PERIOD 1 /* milliseconds */


/**
 * acquisition structure to store analog readout 
 */
typedef struct {
  float val [FIFO_DEPTH][CHANNELS];
} AnalogAcq;

/**
 * struct to store light levels for polling 
 */
typedef struct {
  float photodiode_data[N_CHANNELS_PHOTODIODE];  
  float sipm_data[N_CHANNELS_SIPM]; 
  float sipm_single; 
} LightLevel;

/**
 * class to handle the analog data acquisition (photodiodes and SiPMs) 
 * uses a simple Arduino to replace the AnalogManager module due to 
 * issues with power comsumption.
 */
class ArduinoManager {
public:

  ArduinoManager();
  std::shared_ptr<LightLevel> ReadLightLevel();
  bool CompareLightLevel();
  int ProcessAnalogData();  
  int GetLightLevel();
  int AnalogDataCollect();

  /* handle instrument mode switching */
  int Notify();
  int Reset();
  
private:

  /*
   * for thread-safe access to the light_level 
   */
  std::mutex m_light_level;
  /*
   * light level stored here and accessed only with mutex protection
   */
  std::shared_ptr<LightLevel> light_level;
  /*
   * analog acquisition stored here
   */
  std::shared_ptr<AnalogAcq> analog_acq;

  /*
   * to notify the object of a mode switch
   */
  bool inst_mode_switch;
  /*
   * to handle mode switching in a thread-safe way
   */
  std::mutex m_mode_switch;
  /*
   * to wait for a mode switch
   */
  std::condition_variable cv_mode_switch;

  
  int SetInterfaceAttribs(int fd, int speed);
  void SerialReadOut(int fd);
  
};

#endif
/* _ARDUINO_MANAGER_H */
