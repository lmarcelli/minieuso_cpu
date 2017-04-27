#include "globals.h"

/* create log file name */
std::string CreateLogname(void) {
  struct timeval tv;
  gettimeofday(&tv,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  char logname[40];
  strftime(logname, sizeof(logname), "../log/CPU_MAIN_%Y-%m-%d_%H:%M:%S.log", now_tm);
  return logname;
}

/* copy a file */
bool CopyFile(const char *SRC, const char* DEST) {
    std::ifstream src(SRC, std::ios::binary);
    std::ofstream dest(DEST, std::ios::binary);
    dest << src.rdbuf();
    return src && dest;
}

/* handle SIGINT */
void SignalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received" << std::endl;
  std::cout << "Stopping the acquisition" << std::endl;
  
  /* handle the signal*/
  DataAcquisitionStop();
  std::cout << "Acquisition stopped" << std::endl;  
  
  /* terminate the program */
  exit(signum);  

}

/* read out a zynq data file and append to a cpu data file */
int ZynqFileReadOut(std::string zynq_file_name, std::string cpu_file_name) {

  FILE * ptr_zfile;
  FILE * ptr_cpufile;
  Z_DATA_TYPE_SCI_POLY_V5 zynq_data_file;
  CPU_PACKET cpu_packet;
  const char * kZynqFileName = zynq_file_name.c_str();
  const char * kCpuFileName = cpu_file_name.c_str();

  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "reading out the file " << zynq_file_name << " and appending to  " << cpu_file_name << std::endl;

  ptr_zfile = fopen(kZynqFileName, "rb");
  if (!ptr_zfile) {
    clog << "error: " << logstream::error << "cannot open the file " << zynq_file_name << std::endl;
    return 1;
  }
  
  /* read out the zynq structure, defined in "pdmdata.h" */
  fread(&zynq_data_file, sizeof(zynq_data_file), 1, ptr_zfile);

  /* DEBUG: print records to check */
  printf("header = %d\n", zynq_data_file.zbh.header);
  printf("payload_size = %d\n", zynq_data_file.zbh.payload_size);
  printf("hv_status = %d\n", zynq_data_file.payload.hv_status);
  printf("n_gtu = %llu\n", zynq_data_file.payload.ts.n_gtu);

  /* close the zynq file */
  fclose(ptr_zfile);

  /* create the cpu packet from the zynq file */
  cpu_packet.cpu_packet_header.header = 888;
  cpu_packet.cpu_packet_header.pkt_size = 777;
  cpu_packet.cpu_time.cpu_time_stamp = 666;
  cpu_packet.zynq_data = zynq_data_file;
  
  /* open the cpu file to append */
  ptr_cpufile = fopen(kCpuFileName, "a+b");
  if (!ptr_cpufile) {
    clog << "error: " << logstream::error << "cannot open the file " << zynq_file_name << std::endl;
    return 1;
  }

  /* write the cpu packet */
  fwrite(&cpu_packet, sizeof(cpu_packet), 1, ptr_cpufile);
  
  /* close the cpu file */
  fclose(ptr_cpufile);
  
  return 0
}
