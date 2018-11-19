#include "ArduinoManager.h"

/**
 * constructor 
 */
ArduinoManager::ArduinoManager() {
  this->light_level = std::make_shared<LightLevel>();
  this->analog_acq = std::make_shared<AnalogAcq>();
  int i = 0, j = 0;
  for (i = 0; i < FIFO_DEPTH; i++) {
    for (j = 0; j < CHANNELS; j++) {
      this->analog_acq->val[i][j] = 0;
    }
  }
  this->inst_mode_switch = false;
}


/**
 * analog board read out
 * uses the Arduino to collect data on the analog ports, 
 * as defined in ArduinoManager.h
 */
int ArduinoManager::AnalogDataCollect() {

  /* test implementation for now, just prints output to screen */

  int fd;
  int i;
  
  fd = open(DUINO, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    printf("Error opening %s: %s\n", DUINO, strerror(errno));
    return -1;
  }
  else {
    printf("Device has been opened and ready for operation! \n");
  }

  /*baudrate 9600, 8 bits, no parity, 1 stop bit */
  set_interface_attribs(fd, BAUDRATE);

  printf("Will now run ArduinoManager::SerialReadOut() 10 times...\n")
  for (i = 0; i < 10; i++) {

    print("%i: \n", i);
    SerialReadOut(fd);
    
  }
  
  return 0;
}

/**
 * Read serial output from a file descriptor.
 */
void ArduinoManager::SerialReadOut(int fd) {

  unsigned char buf[14] = "";
  int rdlen;
  bool continue = true;

  /* repeat read to get full message */
  while(continue) {
   
    rdlen = read(fd, buf, sizeof(buf) - 1);

    /* print output */
    if (rdlen > 0) {
      buf[rdlen] = 0;
      printf("%s", buf);
    }

    /* catch error */
    else if (rdlen < 0) {
      printf("Error from read: %d: %s\n", rdlen, strerror(errno));
    }

    /* stop reading */
    else {
      continue = false;
    }
    
  }
  
}

/**
 * get the current light level. 
 * preforms an analog acquisition using ArduinoManager::AnalogDataCollect()
 * and converts the output to the easily readable LightLevel format
 */
int ArduinoManager::GetLightLevel() {

  int i, k;
  float sum_ph[N_CHANNELS_PHOTODIODE];
  float sum_sipm1 = 0;
 
  /* read out the data */
  AnalogDataCollect();
  
  /* interpret the analog acquisition struct */
  /* initialise */
  for(k = 0; k < N_CHANNELS_PHOTODIODE; k++) {
    sum_ph[k] = 0;
  }

  /* read out multiplexed sipm 64 values and averages of sipm 1 and photodiodes */
  for(i = 0; i < FIFO_DEPTH; i++) {

    /* sum the four photodiode channels */
    sum_ph[0] += this->analog_acq->val[i][0];
    sum_ph[1] += this->analog_acq->val[i][1];
    sum_ph[2] += this->analog_acq->val[i][2];
    sum_ph[3] += this->analog_acq->val[i][3];

    /* sum the one channel SiPM values */
    sum_sipm1 += this->analog_acq->val[i][4];
    
    /* read out the multiplexed 64 channel SiPM values */
    {
      std::unique_lock<std::mutex> lock(this->m_light_level);
      this->light_level->sipm_data[i] = this->analog_acq->val[i][5];
    } /* release mutex */
 }

  /* average the photodiode values */
  for (k = 0; k < N_CHANNELS_PHOTODIODE; k++) {
     {
       std::unique_lock<std::mutex> lock(this->m_light_level);
       this->light_level->photodiode_data[k] = sum_ph[k]/FIFO_DEPTH;
     } /* release mutex */
  }
  /* average the one channel SiPM values */
   {
     std::unique_lock<std::mutex> lock(this->m_light_level);
     this->light_level->sipm_single = sum_sipm1/FIFO_DEPTH;
   } /* release mutex */

   return 0;
}

/* 
 * read the light_level from object in a thread-safe way, 
 * without making an acquisition 
 */
std::shared_ptr<LightLevel> ArduinoManager::ReadLightLevel() {
  
  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    auto light_level = this->light_level;
  } /* release mutex */
  
 
  return light_level; 
}

/**
 * compare light level to threshold value 
 * LIGHT_THRESHOLD which is define in ArduinoManager.h
 * @TODO check if more sophisticated tests needed in lab
 */
bool ArduinoManager::CompareLightLevel() {

  bool above_light_threshold = false;
  float ph_avg = 0;
  int i;
  
  clog << "info: " << logstream::info << "comparing light level to threshold" << std::endl;
  
  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    auto light_level = this->light_level;
  } /* release mutex */
  
  
  /* read the light level */
  /* average the 4 photodiode values */
  {
    std::unique_lock<std::mutex> lock(this->m_light_level);
    for (i = 0; i < N_CHANNELS_PHOTODIODE; i++) {
      ph_avg += light_level->photodiode_data[i];
    }
  } /* release mutex */
  ph_avg = ph_avg/(float)N_CHANNELS_PHOTODIODE;

  /* debug */
  // std::cout << "photodiode average = " << ph_avg << std::endl;
  clog << "info: " << logstream::info << "average photodiode reading is: " << ph_avg << std::endl;
     
  /* compare the result to threshold */
  if (ph_avg > LIGHT_THRESHOLD) {
    above_light_threshold = true;
    clog << "info: " << logstream::info << "light level is ABOVE threshold" << std::endl;
  }
  else { 
    clog << "info: " << logstream::info << "light level is BELOW threshold" << std::endl;
  }
  
  return above_light_threshold;
}

int ArduinoManager::ProcessAnalogData() {


  std::unique_lock<std::mutex> lock(this->m_mode_switch);
  /* enter loop while instrument mode switching not requested */
  while(!this->cv_mode_switch.wait_for(lock,
				       std::chrono::milliseconds(WAIT_PERIOD),
				       [this] { return this->inst_mode_switch; })) { 

    this->GetLightLevel();

    sleep(LIGHT_ACQ_TIME);
  }
  return 0;
}


/**
 * reset the mode switching after an instrument mode change
 * used by OperationMode::Reset() 
 */
int ArduinoManager::Reset() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = false;
  } /* release mutex */

  /* update measurement */
  this->GetLightLevel();
  
  return 0;
}

/**
 * notify the object of an instrument mode switch 
 * used by OperationMode::Notify
 */
int ArduinoManager::Notify() {

  {
    std::unique_lock<std::mutex> lock(this->m_mode_switch);   
    this->inst_mode_switch = true;
  } /* release mutex */
  
  return 0;
}

/**
 * Set up interface attributes for the interface with the Arduino device.
 */
int ArduinoManager::SetInterfaceAttribs(int fd, int speed) {

  struct termios tty;

  if (tcgetattr(fd, &tty) < 0) {
    printf("Error from tcgetattr: %s\n", strerror(errno));
    return -1;
  }

  cfsetospeed(&tty, (speed_t)speed);
  cfsetispeed(&tty, (speed_t)speed);

  tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;         /* 8-bit characters */
  tty.c_cflag &= ~PARENB;     /* no parity bit */
  tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
  tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

  /* setup for non-canonical mode */
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  /* fetch bytes as they become available */
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 5;
  
  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    printf("Error from tcsetattr: %s\n", strerror(errno));
    return -1;
  }

  return 0;
}













