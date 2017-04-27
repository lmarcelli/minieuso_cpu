#ifndef _GENERAL_FUNCTIONS_H
#define _GENERAL_FUNCTIONS_H

std::string CreateLogname(void);
bool CopyFile(const char * SRC, const char * DEST);
void SignalHandler(int signum);

#endif

