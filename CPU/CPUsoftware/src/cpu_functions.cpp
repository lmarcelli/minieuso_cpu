#include "globals.h"

/* parsing a configuation file */
void parse() {
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  std::string line;
  std::string parfilename;

  /* define the parameters to parse */
  float par_a, par_b, par_c; 
   
  std::ifstream parfile;
  std::string destination;
  std::stringstream pf;
  pf << "../config/dummy.conf";
  parfilename = pf.str();
  parfile.open(parfilename.c_str());
  destination = "NIR";

  if (parfile.is_open()) {
    clog << "info: " << logstream::info << "reading from the configuration file" << std::endl; 
    while (getline(parfile, line)) {
      std::istringstream in(line);
      std::string type;
      in >> type;
      
      if (type == "PARAMETER_A") {
	in >> par_a;
      }
      else if (type == "PARAMETER_B") {
	in >> par_b;
      }
      else if (type == "PARAMETER_C") {
	in >> par_c;
      }  
    }
    parfile.close();
  }
  else {
    clog << "error: " << logstream::error << "unable to open configuration file" << std::endl;   
  } 
}


/* reload and parse a configuration file */
void configure(std::string config_file) {
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "running configuration" << std::endl;
  const char * cfg = config_file.c_str();

  if (FILE *file = fopen(cfg, "r")) {

    /* copy the file locally */ 
    if (copy_file(cfg, CONFIG_FILE_LOCAL)) {
      clog << "info: " << logstream::info << "configuration file sucessfully copied over" << std::endl;     
    }
    else {
      clog << "warning: " << logstream::warning << "unable to copy the configuration file" << std::endl;
    }

    /* parse the file */
    //see camera code for dummy test
    parse();

  }
  else {
    clog << "error: " << logstream::error << "configuration file does not exist" << std::endl;
  }
}

/* check IP address connection */
int check_IP_com(char * ip_address)
{
    printf("start");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(65432);  // Could be anything
    inet_pton(AF_INET, ip_address, &sin.sin_addr);

    if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
    {
        printf("Error connecting: %d (%s)\n", errno, strerror(errno));
    }

    sin.sin_family = AF_INET;
    sin.sin_port   = htons(65432);  // Could be anything
    inet_pton(AF_INET, "192.168.0.9", &sin.sin_addr);

    if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
    {
        printf("Error connecting 192.168.0.9: %d (%s)\n", errno, strerror(errno));
    }
    return 0;
}
