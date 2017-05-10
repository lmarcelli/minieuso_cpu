#include "globals.h"

/* parsing a configuration file */
Config * Parse(std::string config_file_local) {
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  std::string line;
  std::string config_file_name;

  /* define the parameters to parse */ 
  Config * Output = new Config();
  
  std::ifstream cfg_file;
  std::stringstream cf;
  cf << config_file_local;
  config_file_name = cf.str();
  cfg_file.open(config_file_name.c_str());

  if (cfg_file.is_open()) {
    clog << "info: " << logstream::info << "reading from the configuration file" << std::endl; 
    while (getline(cfg_file, line)) {
      std::istringstream in(line);
      std::string type;
      in >> type;
      
      if (type == "CATHODE_VOLTAGE") {
	in >> Output->cathode_voltage;
	printf("CATHODE_VOLTAGE is: %d\n", Output->cathode_voltage);
      }
      else if (type == "DYNODE_VOLTAGE") {
	in >> Output->dynode_voltage;
	printf("DYNODE_VOLTAGE is: %d\n", Output->dynode_voltage);
      }
      else if (type == "SCURVE_START") {
	in >> Output->scurve_start;
	printf("SCURVE_START is: %d\n", Output->scurve_start);
      }
      else if (type == "SCURVE_STEP") {
	in >> Output->scurve_step;
	printf("SCURVE_STEP is: %d\n", Output->scurve_step);
      }
      else if (type == "SCURVE_STOP") {
	in >> Output->scurve_stop;
	printf("SCURVE_STOP is: %d\n", Output->scurve_stop);
      }
      else if (type == "SCURVE_ACC") {
	in >> Output->scurve_acc;
	printf("SCURVE_ACC is: %d\n", Output->scurve_acc);
      }
      else if (type == "DAC_LEVEL") {
	in >> Output->dac_level;
	printf("DAC_LEVEL is: %d\n", Output->dac_level);
      } 
    }
    cfg_file.close();
  }
  else {
    clog << "error: " << logstream::error << "unable to open configuration file" << std::endl;   
  }
  
  return Output;
}


/* reload and parse a configuration file */
Config * Configure(std::string config_file, std::string config_file_local) {

  /* definitions */
  const char * kCfg = config_file.c_str();
  const char * kCfgLocal = config_file_local.c_str();
  Config * ParseOutput = NULL;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "running configuration" << std::endl;

  if (FILE *file = fopen(kCfg, "r")) {

    /* copy the file locally */ 
    if (CopyFile(kCfg, kCfgLocal)) {
      clog << "info: " << logstream::info << "configuration file sucessfully copied over" << std::endl;     
    }
    else {
      clog << "warning: " << logstream::warning << "unable to copy the configuration file" << std::endl;
    }

    /* parse the file */
    ParseOutput = Parse(config_file_local);

    fclose(file);    
  }
  else {
    clog << "error: " << logstream::error << "configuration file does not exist" << std::endl;

  }

  return ParseOutput;
}

/* check telnet connection on a certain IP address */
/* closes the telnet connection after */
int CheckTelnet(std::string ip_address, int portno) {

  /* definitions */
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent * server;
  const char * ip = ip_address.c_str();

  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
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
std::string SendRecvTelnet(std::string send_msg, int sockfd) {

  const char * kSendMsg = send_msg.c_str();
  char buffer[256];
  std::string recv_msg;
  std::string err_msg = "error";
  int n;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);

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
/* leaves telnet open to be closed with a separate function */
int ConnectTelnet(std::string ip_address, int portno) {

  /* definitions */
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent * server;
  const char * ip = ip_address.c_str();

  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
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
int InstStatus() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  //long status;

  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "checking the instrument status" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet(ZYNQ_IP, TELNET_PORT);
  
  
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
int InstStatusTest(std::string ip_address, int portno, std::string send_msg) {
  
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
int HvpsStatus() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  //long status;

  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "checking the HVPS status" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet(ZYNQ_IP, TELNET_PORT);
  
  /* send and receive commands */
  status_string = SendRecvTelnet("hvps status gpio\n", sockfd);
  kStatStr = status_string.c_str();
  //status = std::stol(kStatStr);
  printf("HVPS status: %s\n", kStatStr);
  
  close(sockfd);
  return 0;
}

/* turn on the HV */
int HvpsTurnOn(int cv, int dv) {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  std::string cmd;
  std::stringstream conv;

  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "turning on the HVPS" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet(ZYNQ_IP, TELNET_PORT);
  
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
int Scurve(int start, int step, int stop, int acc) {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  std::string cmd;
  std::stringstream conv;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "taking an s-curve" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet(ZYNQ_IP, TELNET_PORT);
  
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

int DataAcquisitionStart() {

  /* definitions */
  std::string status_string;
  const char * kStatStr = NULL;
  int sockfd;

  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "starting data acquisition" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet(ZYNQ_IP, TELNET_PORT);
  
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


int DataAcquisitionStop() {

  /* definitions */
  std::string status_string;
  const char * kStatStr = NULL;
  int sockfd;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "stopping data acquisition" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet(ZYNQ_IP, TELNET_PORT);
  
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

/* set the DAC on the SPACIROCs */
int SetDac(int dac_level) {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  std::string cmd;
  std::stringstream conv;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "set the dac level to the SPACIROCs" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet(ZYNQ_IP, TELNET_PORT);
  
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
int AcqShot() {

  /* definitions */
  std::string status_string;
  const char * kStatStr;
  int sockfd;
  std::string cmd;
  std::stringstream conv;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "acquiring a single frame from the SPACIROCs" << std::endl;

  /* setup the telnet connection */
  sockfd = ConnectTelnet(ZYNQ_IP, TELNET_PORT);
  
  /* send and receive commands */
  /* take a dingle frame */
  conv << "acq shot" << std::endl;
  cmd = conv.str();
  std::cout << cmd;
  
  status_string = SendRecvTelnet(cmd, sockfd);
  kStatStr = status_string.c_str();
  printf("status: %s\n", kStatStr);

  close(sockfd);
  return 0;
}
