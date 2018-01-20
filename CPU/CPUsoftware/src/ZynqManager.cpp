
#include "ZynqManager.h"

/* default constructor */
ZynqManager::ZynqManager () {   
  this->hvps_status = ZynqManager::UNDEF;
  this->instrument_mode = ZynqManager::MODE0;
  this->test_mode = ZynqManager::T_MODE0;
};

/* check telnet connection on a certain IP address */
/* closes the telnet connection after */
int ZynqManager::CheckTelnet() {

  /* definitions */
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent * server;
  const char * ip = ZYNQ_IP;

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
  
  /* try to connect */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
    clog << "error: " << logstream::error << "error connecting to " << ZYNQ_IP << " on port " << TELNET_PORT << std::endl;
    return 1;
  }
  else {
    clog << "info: " << logstream::info << "connected to " << ZYNQ_IP << " on port " << TELNET_PORT  << std::endl;
    printf("connected to %s on port %u\n", ip, TELNET_PORT);
  }

  close(sockfd);
  return 0;  
}

/* send and recieve commands over the telnet connection */
/* to be used inside a function which opens the telnet connection */
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

/* connect to telnet */
/* NB: leaves telnet open to be closed with a separate function */
int ZynqManager::ConnectTelnet() {

  /* definitions */
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent * server;
  const char * ip = ZYNQ_IP;

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
  
  /* connect */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
    clog << "error: " << logstream::error << "error connecting to " << ZYNQ_IP << " on port " << TELNET_PORT << std::endl;
    return 1;
  }
  else {
    clog << "info: " << logstream::info << "connected to " << ZYNQ_IP << " on port " << TELNET_PORT  << std::endl;
    printf("connected to %s on port %u\n", ip, TELNET_PORT);
  }
  
  return sockfd;   
}

/* check the instrument status */
int ZynqManager::InstStatus() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  //long status;

  clog << "info: " << logstream::info << "checking the instrument status" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* send and receive commands in another */
  status_string = SendRecvTelnet("instrument status\n", sockfd);
  kStatStr = status_string.c_str();
  //status = std::stol(kStatStr);
  printf("status: %s\n", kStatStr);

  close(sockfd);
  return 0;
}


/* check the instrument status */
/* quick test approach with all telnet setup in one function */
int ZynqManager::InstStatusTest(std::string send_msg) {
  
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  const char * ip = ZYNQ_IP;
  const char * kSendMsg = send_msg.c_str();
  char buffer[256];
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) { 
    printf("ERROR opening socket\n");
    exit(0);
  }
  
  server = gethostbyname(ip);
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(TELNET_PORT);

  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    printf("ERROR opening socket\n");
    exit(0);
  }

  bzero(buffer,256);
  strncpy(buffer, kSendMsg, sizeof(buffer));
  
  n = write(sockfd,buffer,strlen(buffer));
  if (n < 0) {
    printf("ERROR opening socket\n");
    exit(0);
  }
  bzero(buffer,256);
  n = read(sockfd,buffer,255);
  if (n < 0) { 
    printf("ERROR opening socket\n");
    exit(0);
  }
  printf("%s\n",buffer);
  close(sockfd);
  
  return 0;
}


/* check the HV status */
int ZynqManager::HvpsStatus() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  //long status;

  clog << "info: " << logstream::info << "checking the HVPS status" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* send and receive commands */
  status_string = SendRecvTelnet("hvps status gpio\n", sockfd);
  kStatStr = status_string.c_str();
  //status = std::stol(kStatStr);
  printf("HVPS status: %s\n", kStatStr);
  
  close(sockfd);
  return 0;
}

/* turn on the HV */
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
  std::cout << cmd0;
  
  
  status_string = SendRecvTelnet(cmd0, sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);
  usleep(sleep_time);
  
  /* set the dynode voltage low */
  status_string = SendRecvTelnet("hvps setdac 500 500 500 500 500 500 500 500 500\n", sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);
  usleep(sleep_time);  

  /* turn on */
  status_string = SendRecvTelnet("hvps turnon 1 1 1 1 1 1 1 1 1\n", sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);

  /* ramp up in steps of 500 */
  status_string = SendRecvTelnet("hvps setdac 1000 1000 1000 1000 1000 1000 1000 1000 1000\n", sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);
  usleep(sleep_time);  
  status_string = SendRecvTelnet("hvps setdac 1500 1500 1500 1500 1500 1500 1500 1500 1500\n", sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);
  usleep(sleep_time);  
  status_string = SendRecvTelnet("hvps setdac 2000 2000 2000 2000 2000 2000 2000 2000 2000\n", sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);
  usleep(sleep_time);  
  status_string = SendRecvTelnet("hvps setdac 2500 2500 2500 2500 2500 2500 2500 2500 2500\n", sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);
  usleep(sleep_time);
  if (dv > 3000) {
    status_string = SendRecvTelnet("hvps setdac 3000 3000 3000 3000 3000 3000 3000 3000 3000\n", sockfd);
    kStatStr = status_string.c_str();
    printf("status: %s\n", kStatStr);
    usleep(sleep_time);
  }
  if (dv > 3500) {
    status_string = SendRecvTelnet("hvps setdac 3500 3500 3500 3500 3500 3500 3500 3500 3500\n", sockfd);
    kStatStr = status_string.c_str();
    printf("status: %s\n", kStatStr);
    usleep(sleep_time);

    
  }
  
  /* set the final DAC */
  conv1 << "hvps setdac " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << std::endl;
  cmd1 = conv1.str();
  std::cout << cmd1;
  
  status_string = SendRecvTelnet(cmd1, sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);
  usleep(sleep_time);

  /* update the HvpsStatus */
  this->hvps_status = ZynqManager::ON;
  
  close(sockfd);
  return 0;
}


/* turn off the HV */
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
  printf("status: %s\n", kStatStr);
  usleep(sleep_time);

  /* update the HvpsStatus */
  this->hvps_status = ZynqManager::OFF;
  
  close(sockfd);
  return 0;
}


/* take an scurve */
int ZynqManager::Scurve(int start, int step, int stop, int acc) {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  std::string cmd;
  std::stringstream conv;

  clog << "info: " << logstream::info << "taking an s-curve" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* send and receive commands */
  /* take an s-curve */
#ifdef SINGLE_EVENT
  conv << "acq sweep " << start << " " << step << " " << stop << " " << acc << std::endl;
#else
  conv << "acq scurve " << start << " " << step << " " << stop << " " << acc << std::endl;
#endif /* SINGLE_EVENT */
  cmd = conv.str();
  std::cout << cmd;
  
  status_string = SendRecvTelnet(cmd, sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);

  /* wait for scurve to be taken */
  sleep(15);
  
  close(sockfd);
  return 0;
}

/* set the DAC on the SPACIROCs */
int ZynqManager::SetDac(int dac_level) {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
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
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);

  close(sockfd);
  return 0;
}


/* Acquire one GTU frame from the SPACIROCs */
int ZynqManager::AcqShot() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
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
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);

  close(sockfd);
  return 0;
}

/* set the acquisition mode */
ZynqManager::InstrumentMode ZynqManager::SetInstrumentMode(ZynqManager::InstrumentMode input_mode) {

  /* definitions */
  std::string status_string;
  const char * kStatStr = NULL;
  int sockfd;

  clog << "info: " << logstream::info << "switching to instrument mode " << input_mode << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* check input mode and update accordingly */
  switch (input_mode) {
  case MODE0:
    this->instrument_mode = MODE0; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("instrument mode 0\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case MODE1:
    this->instrument_mode = MODE1; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("instrument mode 1\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case PERIODIC:
    this->instrument_mode = PERIODIC; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("instrument mode 2\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case TRIGGER:
     this->instrument_mode = TRIGGER; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("instrument mode 3\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  }
  
  /* check the status */
  //ADD THIS
  
  close(sockfd);
  return this->instrument_mode;
}


/* set the test acquisition mode */
ZynqManager::TestMode ZynqManager::SetTestMode(ZynqManager::TestMode input_mode) {

  /* definitions */
  std::string status_string;
  const char * kStatStr = NULL;
  int sockfd;

  clog << "info: " << logstream::info << "switching to instrument mode " << input_mode << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* check input mode and update accordingly */
  switch (input_mode) {
  case T_MODE0:
    this->test_mode = T_MODE0; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("acq test 0\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case T_MODE1:
    this->test_mode = T_MODE1; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("acq test 1\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case T_MODE2:
    this->test_mode = T_MODE2; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("acq test 2\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case T_MODE3:
     this->test_mode = T_MODE3; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("acq test 3\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case T_MODE4:
     this->test_mode = T_MODE4; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("acq test 4\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case T_MODE5:
     this->test_mode = T_MODE5; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("acq test 5\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;
  case T_MODE6:
     this->test_mode = T_MODE6; 
    /* switch to data acquisition mode specified */
    status_string = SendRecvTelnet("acq test 5\n", sockfd);
    printf("status: %s\n", kStatStr);
    break;

  }
  
  /* check the status */
  //ADD THIS
  
  close(sockfd);
  return this->test_mode;
}


/* static function to turn off acquisition */
int ZynqManager::StopAcquisition() {

  /* definitions */
  std::string status_string;
  const char * kStatStr = NULL;
  int sockfd;

  clog << "info: " << logstream::info << "switching off the Zynq acquisition" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  status_string = SendRecvTelnet("instrument mode 0\n", sockfd);
  printf("status: %s\n", kStatStr);
   
  /* check the status */
  //ADD THIS
  
  close(sockfd);
  return 0;
}

/* depreciated commands for compatibilty */
#ifdef SINGLE_EVENT
int ZynqManager::DataAcquisitionStart() {

  /* definitions */
  std::string status_string;
  const char * kStatStr = NULL;
  int sockfd;

  clog << "info: " << logstream::info << "starting data acquisition" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* send and receive commands */
  /* switch to data acquisition mode 1*/
  status_string = SendRecvTelnet("instrument mode 1\n", sockfd);
  printf("status: %s\n", kStatStr);

  /* check the status */
  //ADD THIS

  /* start data acquisition */
  status_string = SendRecvTelnet("instrument start\n", sockfd);
  printf("status: %s\n", kStatStr);
  
  
  close(sockfd);
  return 0;
}


int ZynqManager::DataAcquisitionStop() {

  /* definitions */
  std::string status_string;
  const char * kStatStr = NULL;
  int sockfd;

  clog << "info: " << logstream::info << "stopping data acquisition" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* send and receive commands */
  /* stop the data acquisition */
  status_string = SendRecvTelnet("instrument stop\n", sockfd);
  printf("status: %s\n", kStatStr);

  /* check the status */
  //ADD THIS

  /* switch to instrument mode 0 */
  status_string = SendRecvTelnet("instrument mode 0\n", sockfd);
  printf("status: %s\n", kStatStr);
  
  
  close(sockfd);
  return 0;
}
#endif /* SINGLE_EVENT */
