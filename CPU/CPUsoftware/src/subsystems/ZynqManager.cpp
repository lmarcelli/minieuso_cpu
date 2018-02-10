
#include "ZynqManager.h"

/**
 * constructor 
 * initialises public members
 * hvps_status, instrument_mode, test_mode and telnet_connected
 */
ZynqManager::ZynqManager () {   
  this->hvps_status = ZynqManager::UNDEF;
  this->instrument_mode = ZynqManager::MODE0;
  this->test_mode = ZynqManager::T_MODE0;
  this->telnet_connected = false;
}

/**
 * check telnet connection on ZYNQ_IP (defined in ZynqManager.h)
 * and close the telnet connection after.
 * has a timeout implemented of length CONNECT_TIMEOUT_SEC (defined in ZynqManager.h)
 */
int ZynqManager::CheckTelnet() {

  /* definitions */
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent * server;
  const char * ip = ZYNQ_IP;
  struct timeval tv;
  fd_set fdset;
  
  clog << "info: " << logstream::info << "checking connection to IP " << ZYNQ_IP  << std::endl;
 
  /* set up the telnet connection */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) { 
    clog << "error: " << logstream::error << "error opening socket" << std::endl;
    return 1;
  }
 
  server = gethostbyname(ip);
  if (server == NULL) {
    clog << "error: " << logstream::error << "no host found for " << ZYNQ_IP << std::endl;  
    return 1;
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(TELNET_PORT);

  /* set non-blocking */
  int opts = fcntl(sockfd, F_SETFL, O_NONBLOCK);
  connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

  FD_ZERO(&fdset);
  FD_SET(sockfd, &fdset);

  /* add timeout */
  tv.tv_sec = CONNECT_TIMEOUT_SEC; 
  tv.tv_usec = 0;
  
  if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
      int so_error;
      socklen_t len = sizeof so_error;
      
      getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
      
      if (so_error == 0) {
	clog << "info: " << logstream::info << "connected to " << ZYNQ_IP << " on port " << TELNET_PORT  << std::endl;

	/* clear non-blocking */
	opts = opts & (~O_NONBLOCK);
	fcntl(sockfd, F_SETFL, opts);   
       
      }
      else {

	clog << "error: " << logstream::error << "error connecting to " << ZYNQ_IP << " on port " << TELNET_PORT << std::endl;
	this->telnet_connected = false;
	return 1;
      }
        
  }
  else {
    std::cout << "ERROR: Connection timeout to the Zynq board" << std::endl;
    clog << "error: " << logstream::error << "connection timeout to " << ZYNQ_IP << " on port " << TELNET_PORT << std::endl;
    this->telnet_connected = false;
    return 1;
  }

  
  close(sockfd);
  this->telnet_connected = true;
  return 0;  
}

/**
 * send and recieve commands over the telnet connection
 * to be used inside a function which opens the telnet connection 
 * @param send_msg the message to be sent
 * @param sockfd the socket field descriptor
 */
std::string ZynqManager::SendRecvTelnet(std::string send_msg, int sockfd) {

  const char * kSendMsg = send_msg.c_str();
  char buffer[256];
  std::string recv_msg;
  std::string err_msg = "error";
  int n;
  
  /* prepare the message to send */
  bzero(buffer, 256);
  strncpy(buffer, kSendMsg, sizeof(buffer));
  send_msg.erase(std::remove(send_msg.begin(), send_msg.end(), '\n'), send_msg.end());
  clog << "info: " << logstream::info << "sending via telnet: " << send_msg << std::endl;
 
  n = write(sockfd, buffer, strlen(buffer));
  if (n < 0) {
    clog << "error: " << logstream::error << "error writing to socket" << std::endl;
    return err_msg;
  }
  bzero(buffer, 256);
  n = read(sockfd, buffer, 255);
  if (n < 0) {
    clog << "error: " << logstream::error << "error reading to socket" << std::endl;
    return err_msg;
  }
  recv_msg = buffer;
  recv_msg.erase(std::remove(recv_msg.begin(), recv_msg.end(), '\r'), recv_msg.end());
  recv_msg.erase(std::remove(recv_msg.begin(), recv_msg.end(), '\n'), recv_msg.end());
  clog << "info: " << logstream::info << "receiving via telnet: " << recv_msg << std::endl;
  return recv_msg;
 }

/**
 * connect via telnet to ZYNQ_IP.
 * NB: leaves telnet open to be closed with a separate function 
 */
int ZynqManager::ConnectTelnet() {

  /* definitions */
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent * server;
  const char * ip = ZYNQ_IP;
  struct timeval tv;
  fd_set fdset;
  
  clog << "info: " << logstream::info << "checking connection to IP " << ZYNQ_IP  << std::endl;
 
  /* set up the telnet connection */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) { 
    clog << "error: " << logstream::error << "error opening socket" << std::endl;
    return 1;
  }
 
  server = gethostbyname(ip);
  if (server == NULL) {
    clog << "error: " << logstream::error << "no host found for " << ZYNQ_IP << std::endl;  
    return 1;
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(TELNET_PORT);

  /* set non-blocking */
  int opts = fcntl(sockfd, F_SETFL, O_NONBLOCK);
  connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

  /* set non-blocking */
  FD_ZERO(&fdset);
  FD_SET(sockfd, &fdset);

  /* add timeout */
  tv.tv_sec = CONNECT_TIMEOUT_SEC; 
  tv.tv_usec = 0;
  
  if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
      int so_error;
      socklen_t len = sizeof so_error;
      
      getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
      
      if (so_error == 0) {
	clog << "info: " << logstream::info << "connected to " << ZYNQ_IP << " on port " << TELNET_PORT  << std::endl;

	/* clear non-blocking */
	opts = opts & (~O_NONBLOCK);
	fcntl(sockfd, F_SETFL, opts);   
      }
      else {

	clog << "error: " << logstream::error << "error connecting to " << ZYNQ_IP << " on port " << TELNET_PORT << std::endl;
	return -1;
      }
      
    }
    else {
      std::cout << "telnet connection timeout!" << std::endl;
      clog << "error: " << logstream::error << "connection timeout to " << ZYNQ_IP << " on port " << TELNET_PORT << std::endl;
      return 1;
    }

  return sockfd;   
}

/**
 * check the instrument status 
 */
int ZynqManager::GetInstStatus() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;

  clog << "info: " << logstream::info << "checking the instrument status" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();

  if (sockfd > 0) {
    /* send and receive commands in another */
    status_string = SendRecvTelnet("instrument status\n", sockfd);
    kStatStr = status_string.c_str();
    printf("instrument status: %s\n", kStatStr);
    close(sockfd);
  }

  return 0;
}


/**
 * check the HV status 
 */
int ZynqManager::GetHvpsStatus() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;

  clog << "info: " << logstream::info << "checking the HVPS status" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();

  if (sockfd > 0) {
    /* send and receive commands */
    status_string = SendRecvTelnet("hvps status gpio\n", sockfd);
    kStatStr = status_string.c_str();
    printf("HVPS status: %s\n", kStatStr);
  
    close(sockfd);
  }
  
  return 0;
}

/**
 * turn on the HV.
 * ramps up the dynode voltage in steps of 500 HV DAC <=> ~140 V 
 * @param cv the cathode voltage (int from 1-3)
 * @param dv the dynode voltage (HV DAC from 0 to 4096)
 * to convert from HV DAC to voltage use (dv/4096 * 2.44 * 466)
 */
int ZynqManager::HvpsTurnOn(int cv, int dv) {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  std::string cmd0, cmd1;
  std::stringstream conv0, conv1;

  int sleep_time = 500000;
  
  clog << "info: " << logstream::info << "turning on the HVPS" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* set the cathode voltage */
  /* make the command string from config file values */
  conv0 << "hvps cathode " << cv << " " << cv << " " << cv << " " << cv << " " << cv << " " << cv << " " << cv << " " << cv << " " << cv << std::endl;
  cmd0 = conv0.str();
  
  status_string = SendRecvTelnet(cmd0, sockfd);
  kStatStr = status_string.c_str();
  printf("Set HVPS cathode to %i: %s\n", cv, kStatStr);
  usleep(sleep_time);
  
  /* set the dynode voltage low */
  status_string = SendRecvTelnet("hvps setdac 500 500 500 500 500 500 500 500 500\n", sockfd);
  kStatStr = status_string.c_str();
  printf("Set HVPS dac to 500: %s\n", kStatStr);
  usleep(sleep_time);  

  /* turn on */
  status_string = SendRecvTelnet("hvps turnon 1 1 1 1 1 1 1 1 1\n", sockfd);
  kStatStr = status_string.c_str();
  printf("Turn on HVPS: %s\n", kStatStr);

  /* ramp up in steps of 500 */
  if (dv > 1000) {
    status_string = SendRecvTelnet("hvps setdac 1000 1000 1000 1000 1000 1000 1000 1000 1000\n", sockfd);
    kStatStr = status_string.c_str();
    printf("Set HVPS dac to 1000: %s\n", kStatStr);
    usleep(sleep_time);
    if (dv > 1500) {
      status_string = SendRecvTelnet("hvps setdac 1500 1500 1500 1500 1500 1500 1500 1500 1500\n", sockfd);
      kStatStr = status_string.c_str();
      printf("Set HVPS dac to 1500: %s\n", kStatStr);
      usleep(sleep_time);
      if (dv > 2000) {
	status_string = SendRecvTelnet("hvps setdac 2000 2000 2000 2000 2000 2000 2000 2000 2000\n", sockfd);
	kStatStr = status_string.c_str();
	printf("Set HVPS dac to 2000: %s\n", kStatStr);
	usleep(sleep_time);
	if (dv > 2500) {
	  status_string = SendRecvTelnet("hvps setdac 2500 2500 2500 2500 2500 2500 2500 2500 2500\n", sockfd);
	  kStatStr = status_string.c_str();
	  printf("Set HVPS dac to 2500: %s\n", kStatStr);
	  usleep(sleep_time);
	  if (dv > 3000) {
	    status_string = SendRecvTelnet("hvps setdac 3000 3000 3000 3000 3000 3000 3000 3000 3000\n", sockfd);
	    kStatStr = status_string.c_str();
	    printf("Set HVPS dac to 3000: %s\n", kStatStr);
	    usleep(sleep_time);
	    if (dv > 3500) {
	      status_string = SendRecvTelnet("hvps setdac 3500 3500 3500 3500 3500 3500 3500 3500 3500\n", sockfd);
	      kStatStr = status_string.c_str();
	      printf("Set HVPS dac to 3500: %s\n", kStatStr);
	      usleep(sleep_time);  
	    }
	  }
	}
      }
    }
  }

  /* set the final DAC */
  conv1 << "hvps setdac " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << std::endl;
  cmd1 = conv1.str();
  
  status_string = SendRecvTelnet(cmd1, sockfd);
  kStatStr = status_string.c_str();
  printf("Set HVPS dac to %i: %s\n", dv, kStatStr);
  usleep(sleep_time);

  /* update the HvpsStatus */
  this->hvps_status = ZynqManager::ON;
  
  close(sockfd);
  return 0;
}


/**
 * turn off the HV 
 */
int ZynqManager::HvpsTurnOff() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  std::string cmd;
  std::stringstream conv;

  int sleep_time = 500000;
  
  clog << "info: " << logstream::info << "turning off the HVPS" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();

  /* turn off */
  status_string = SendRecvTelnet("hvps turnoff 1 1 1 1 1 1 1 1 1\n", sockfd);
  kStatStr = status_string.c_str();
  printf("HVPS status: %s\n", kStatStr);
  usleep(sleep_time);

  /* update the HvpsStatus */
  this->hvps_status = ZynqManager::OFF;
  
  close(sockfd);
  return 0;
}


/**
 * take an scurve 
 */
int ZynqManager::Scurve(int start, int step, int stop, int acc) {

  /* definitions */
  std::string status_string;
  int sockfd;
  std::string cmd;
  std::stringstream conv;

  clog << "info: " << logstream::info << "taking an s-curve" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* send and receive commands */
  /* take an s-curve */
  printf("S-Curve acquisition starting");
  conv << "acq scurve " << start << " " << step << " " << stop << " " << acc << std::endl;
  cmd = conv.str();
  std::cout << cmd;
  
  status_string = SendRecvTelnet(cmd, sockfd);

  /* wait for scurve to be taken */
  sleep(15);
  
  close(sockfd);
  return 0;
}

/**
 * set the ASIC DAC on the SPACIROCs 
 * @param dac_level (0 - 1000)
 */
int ZynqManager::SetDac(int dac_level) {

  /* definitions */
  std::string status_string;
  int sockfd;
  std::string cmd;
  std::stringstream conv;

  clog << "info: " << logstream::info << "set the dac level to the SPACIROCs" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* send and receive commands */
  /* set the dac level */
  conv << "slowctrl all dac " << dac_level << std::endl;
  cmd = conv.str();
  std::cout << cmd;
  
  status_string = SendRecvTelnet(cmd, sockfd);

  close(sockfd);
  return 0;
}


/**
 * acquire one GTU frame from the SPACIROCs 
 */
int ZynqManager::AcqShot() {

  /* definitions */
  std::string status_string;
  int sockfd;
  std::string cmd;
  std::stringstream conv;

  clog << "info: " << logstream::info << "acquiring a single frame from the SPACIROCs" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* send and receive commands */
  /* take a single frame */
  conv << "acq shot" << std::endl;
  cmd = conv.str();
  std::cout << cmd;
  
  status_string = SendRecvTelnet(cmd, sockfd);

  close(sockfd);
  return 0;
}

/**
 * set the acquisition mode 
 * @param input_mode the desired mode to set
 * @TODO add status check after mode setting
 */
ZynqManager::InstrumentMode ZynqManager::SetInstrumentMode(ZynqManager::InstrumentMode input_mode) {

  /* definitions */
  std::string status_string;
  int sockfd;
  uint32_t timestamp = time(NULL);
  std::string cmd;
  std::stringstream conv;

  clog << "info: " << logstream::info << "switching to instrument mode " << input_mode << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();


  /* check input mode and update accordingly */
  switch (input_mode) {
  case MODE0:
    this->instrument_mode = MODE0;
    conv << "instrument mode 0 " << timestamp << std::endl;
    cmd = conv.str();
    status_string = SendRecvTelnet(cmd, sockfd);
    break;
  case MODE1:
    this->instrument_mode = MODE1; 
    conv << "instrument mode 1 " << timestamp << std::endl;
    cmd = conv.str();
    status_string = SendRecvTelnet(cmd, sockfd);
    break;
  case PERIODIC:
    this->instrument_mode = PERIODIC;
    conv << "instrument mode 2 " << timestamp << std::endl;
     cmd = conv.str();
    status_string = SendRecvTelnet(cmd, sockfd);
    break;
  case TRIGGER:
    this->instrument_mode = TRIGGER;
    conv << "instrument mode 3 " << timestamp << std::endl;
    cmd = conv.str();
    status_string = SendRecvTelnet(cmd, sockfd);
    break;
  }
  
  /* check the status */
  //ADD THIS
  
  close(sockfd);
  return this->instrument_mode;
}


/**
 * set the test acquisition mode 
 * @param input_mode the desired mode to be set
 * @TODO add status check after mode setting
 */
ZynqManager::TestMode ZynqManager::SetTestMode(ZynqManager::TestMode input_mode) {

  /* definitions */
  std::string status_string;
  int sockfd;

  clog << "info: " << logstream::info << "switching to instrument mode " << input_mode << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* check input mode and update accordingly */
  switch (input_mode) {
  case T_MODE0:
    this->test_mode = T_MODE0; 
    status_string = SendRecvTelnet("acq test 0\n", sockfd);
    break;
  case T_MODE1:
    this->test_mode = T_MODE1; 
    status_string = SendRecvTelnet("acq test 1\n", sockfd);
    break;
  case T_MODE2:
    this->test_mode = T_MODE2; 
    status_string = SendRecvTelnet("acq test 2\n", sockfd);
    break;
  case T_MODE3:
     this->test_mode = T_MODE3; 
    status_string = SendRecvTelnet("acq test 3\n", sockfd);
    break;
  case T_MODE4:
     this->test_mode = T_MODE4; 
    status_string = SendRecvTelnet("acq test 4\n", sockfd);
    break;
  case T_MODE5:
     this->test_mode = T_MODE5; 
     status_string = SendRecvTelnet("acq test 5\n", sockfd);
    break;
  case T_MODE6:
     this->test_mode = T_MODE6; 
    status_string = SendRecvTelnet("acq test 5\n", sockfd);
    break;

  }
  
  /* check the status */
  //ADD THIS
  
  close(sockfd);
  return this->test_mode;
}


/**
 * static function to stop acquisition by setting the insrument to MODE0 
 */
int ZynqManager::StopAcquisition() {

  /* definitions */
  std::string status_string;
  int sockfd;

  clog << "info: " << logstream::info << "switching off the Zynq acquisition" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  status_string = SendRecvTelnet("instrument mode 0\n", sockfd);
  
  close(sockfd);
  return 0;
}


/**
 * set the number of packets for D1 and D2 required every 5.24 s
 * @param N1 the number of packets for data level 1 (1 - 4)
 * @param N2 the number of packets for data level 2 (1 - 4)
 */
int ZynqManager::SetNPkts(int N1, int N2) {

  /* definitions */
  std::string status_string;
  int sockfd;
  std::string cmd1, cmd2;
  std::stringstream conv1, conv2;

  int sleep_time = 500000;
  
  clog << "info: " << logstream::info << "setting N1 to " << N1 << " and N2 to " << N2 << std::endl;

  /* create the command */
  conv1 << "mmg N1 " << N1 << std::endl;
  cmd1 = conv1.str();
  conv2 << "mmg N2 " << N2 << std::endl;
  cmd2 = conv2.str();

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  status_string = SendRecvTelnet(cmd1, sockfd);
  usleep(sleep_time);  
  status_string = SendRecvTelnet(cmd2, sockfd);
  usleep(sleep_time);  
   
  close(sockfd);
  return 0;

}
