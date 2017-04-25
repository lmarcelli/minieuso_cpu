#ifndef _GENERAL_FUNCTIONS_H
#define _GENERAL_FUNCTIONS_H

std::string create_logname(void);
bool copy_file(const char * SRC, const char * DEST);
void signal_handler(int signum);

#endif

