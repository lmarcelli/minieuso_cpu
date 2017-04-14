#ifndef _CPU_FUNCTIONS_H
#define _CPU_FUNCTIONS_H

void parse(std::string config_file_local);
void configure(std::string config_file, std::string config_file_local);
int check_telnet(std::string ip_address, int portno);
std::string send_recv_telnet(std::string send_msg, int sockfd);
int connect_telnet(std::string ip_address, int portno);
int inst_status();

#endif
