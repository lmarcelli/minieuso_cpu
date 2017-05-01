#ifndef _GENERAL_FUNCTIONS_H
#define _GENERAL_FUNCTIONS_H

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

std::string CreateLogname(void);
bool CopyFile(const char * SRC, const char * DEST);
void SignalHandler(int signum);
std::string CreateCpuRunName(void);
int CreateCpuRun(std::string cpu_file_name);
int ZynqFileReadOut(std::string zynq_file_name, std::string cpu_file_name);
void ProcessIncomingData(std::string cpu_file_name);

#endif

