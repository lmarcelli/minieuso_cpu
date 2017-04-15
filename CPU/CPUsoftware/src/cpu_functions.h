#ifndef _CPU_FUNCTIONS_H
#define _CPU_FUNCTIONS_H

/* struct for output of the configuration file */
struct config {
  int cathode_voltage;
  int dynode_voltage;
};

config parse(std::string config_file_local);
config configure(std::string config_file, std::string config_file_local);
int check_telnet(std::string ip_address, int portno);
std::string send_recv_telnet(std::string send_msg, int sockfd);
int connect_telnet(std::string ip_address, int portno);
int inst_status();
int inst_status_test(std::string ip_address, int portno, std::string send_msg);
int hvps_status();
int hvps_turnon();
int scurve();
int data_acqusition();
int data_acqusition_stop();


#endif
