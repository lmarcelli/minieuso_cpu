#include "globals.h"

/* parsing a configuation file */
void parse(std::string config_file_local) {
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  std::string line;
  std::string config_file_name;

  /* define the parameters to parse */
  float par_a, par_b, par_c; 
   
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
      
      if (type == "PARAMETER_A") {
	in >> par_a;
	printf("parameter A is: %f\n", par_a);
      }
      else if (type == "PARAMETER_B") {
	in >> par_b;
	printf("parameter B is: %f\n", par_b);
      }
      else if (type == "PARAMETER_C") {
	in >> par_c;
	printf("parameter C is: %f\n", par_c);
      }  
    }
    cfg_file.close();
  }
  else {
    clog << "error: " << logstream::error << "unable to open configuration file" << std::endl;   
  } 
}


/* reload and parse a configuration file */
void configure(std::string config_file, std::string config_file_local) {
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "running configuration" << std::endl;
  const char * cfg = config_file.c_str();
  const char * cfg_local = config_file_local.c_str();

  if (FILE *file = fopen(cfg, "r")) {

    /* copy the file locally */ 
    if (copy_file(cfg, cfg_local)) {
      clog << "info: " << logstream::info << "configuration file sucessfully copied over" << std::endl;     
    }
    else {
      clog << "warning: " << logstream::warning << "unable to copy the configuration file" << std::endl;
    }

    /* parse the file */
    parse(config_file_local);

  }
  else {
    clog << "error: " << logstream::error << "configuration file does not exist" << std::endl;
  }
}

/* check telnet connection on a certain IP address */
int check_telnet(std::string ip_address, int portno) {

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

       
  return 0;  
}

/* send and recieve commands over the telnet connection */
/* to be used inside a function which opens the telnet connection */
std::string send_recv_telnet(std::string send_msg, int sockfd) {

  const char * s_msg = send_msg.c_str();
  char buffer[256];
  std::string recv_msg;
  int n;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);

  /* prepare the message to send */
  bzero(buffer, 256);
  strncpy(buffer, s_msg, sizeof(buffer));
  clog << "info: " << logstream::info << "sending via telnet: " << send_msg << std::endl;
 
  n = write(sockfd, buffer, strlen(buffer));
  if (n < 0) 
    clog << "error: " << logstream::error << "error writing to socket" << std::endl;
   
  bzero(buffer, 256);
  n = read(sockfd, buffer, 255);
  if (n < 0)
     clog << "error: " << logstream::error << "error reading to socket" << std::endl;

  recv_msg = buffer;
  clog << "error: " << logstream::error << "receiving via telnet: " << recv_msg << std::endl;
  return recv_msg;
 }

/* connect to telnet */
/* to be called in a separate thread and wait for commands */
int connect_telnet(std::string ip_address, int portno) {

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
  
  return 0;   
}

/* check the instrument status */
int inst_status() {

  std::string status_string;
  long status;
  const char * stat_str;
  int sockfd;

  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "checking the instrument status" << std::endl;

  /* setup the telnet connection in a separate thread */
  //std::thread th1(connect_telnet(ZYNQ_IP,TELNET_PORT));
  //sockfd = connect_telnet(ZYNQ_IP, TELNET_PORT);
  
  /* send and receive commands in another */
  status_string = send_recv_telnet("instrument status\n", sockfd);
  stat_str = status_string.c_str();
  status = std::stol(stat_str);
  printf("status: %ld", status);
  
  /* join the telnet thread */
  //th1.join();
  
  return 0;
}
