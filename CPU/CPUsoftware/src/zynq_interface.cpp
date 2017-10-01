#include "zynq_interface.h"

/* default constructor */
ZynqManager::ZynqManager () {   
  ip_address = ZYNQ_IP;
  portno = TELNET_PORT;
};

/* check telnet connection on a certain IP address */
/* closes the telnet connection after */
int ZynqManager::CheckTelnet() {

  /* definitions */
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent * server;
  const char * ip = ip_address.c_str();

  clog << "info: " << logstream::info << "checking connection to IP " << ip_address  << std::endl;
 
  /* set up the telnet connection */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) { 
    clog << "error: " << logstream::error << "error opening socket" << std::endl;
    return 1;
  }
 
  server = gethostbyname(ip);
  if (server == NULL) {
    clog << "error: " << logstream::error << "no host found for " << ip_address << std::endl;  
    return 1;
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(portno);
  
  /* try to connect */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
    clog << "error: " << logstream::error << "error connecting to " << ip_address << " on port " << portno << std::endl;
    return 1;
  }
  else {
    clog << "info: " << logstream::info << "connected to " << ip_address << " on port " << portno  << std::endl;
    printf("connected to %s on port %u\n", ip, portno);
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
  const char * ip = ip_address.c_str();

  clog << "info: " << logstream::info << "checking connection to IP " << ip_address  << std::endl;
 
  /* set up the telnet connection */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) { 
    clog << "error: " << logstream::error << "error opening socket" << std::endl;
    return 1;
  }
 
  server = gethostbyname(ip);
  if (server == NULL) {
    clog << "error: " << logstream::error << "no host found for " << ip_address << std::endl;  
    return 1;
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(portno);
  
  /* connect */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
    clog << "error: " << logstream::error << "error connecting to " << ip_address << " on port " << portno << std::endl;
    return 1;
  }
  else {
    clog << "info: " << logstream::info << "connected to " << ip_address << " on port " << portno  << std::endl;
    printf("connected to %s on port %u\n", ip, portno);
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
  const char * ip = ip_address.c_str();
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
  serv_addr.sin_port = htons(portno);

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
  std::string cmd;
  std::stringstream conv;

  clog << "info: " << logstream::info << "turning on the HVPS" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet();
  
  /* set the cathode voltage */
  /* make the command string from config file values */
  conv << "hvps cathode " << cv << " " << cv << " " << cv << " " << cv << " " << cv << " " << cv << " " << cv << " " << cv << " " << cv << std::endl;
  cmd = conv.str();
  std::cout << cmd;
  
  status_string = SendRecvTelnet(cmd, sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);

  /* turn on */
  status_string = SendRecvTelnet("hvps turnon 1 1 1 1 1 1 1 1 1\n", sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);

  /* set the dynode voltage */
  /* make the command string from config file values */
  conv << "hvps setdac " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << " " << dv << std::endl;
  cmd = conv.str();
  std::cout << cmd;
  
  status_string = SendRecvTelnet(cmd, sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);
  
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
  conv << "acq sweep " << start << " " << step << " " << stop << " " << acc << std::endl;
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
