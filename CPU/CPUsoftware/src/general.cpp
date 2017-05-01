#include "globals.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

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

/* create cpu run file name */
std::string CreateCpuRunName(void) {
  struct timeval tv;
  char cpu_file_name[40];
  std::string done_str(DONE_DIR);
  std::string time_str("/CPU_RUN_%Y-%m-%d_%H:%M:%S.dat");
  std::string cpu_str = done_str + time_str;
  const char * kCpuCh = cpu_str.c_str();

  gettimeofday(&tv,0);
  time_t now = tv.tv_sec;
  struct tm * now_tm = localtime(&now);
  
  strftime(cpu_file_name, sizeof(cpu_file_name), kCpuCh, now_tm);
  return cpu_file_name;
}


/* make a cpu data file for a new run */
int CreateCpuRun(std::string cpu_file_name) {

  FILE * ptr_cpufile;
  const char * kCpuFileName = cpu_file_name.c_str();
  CpuFileHeader cpu_file_header;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "creating a new cpu run file called " << cpu_file_name << std::endl;


  /* set up the cpu file structure */
  cpu_file_header.header = 111;
  cpu_file_header.run_size = RUN_SIZE;

  /* open the cpu run file */
  ptr_cpufile = fopen(kCpuFileName, "wb");
  if (!ptr_cpufile) {
    clog << "error: " << logstream::error << "cannot open the file " << cpu_file_name << std::endl;
    return 1;
  }

  /* write to the cpu run file */
  fwrite(&cpu_file_header, sizeof(cpu_file_header), 1, ptr_cpufile);

  /* close the cpu run file */
  fclose(ptr_cpufile);
  
  return 0;
}

/* Look for new files in the data directory and process them */
void ProcessIncomingData() {

  int length, i = 0;
  int fd;
  int wd;
  char buffer[BUF_LEN];

  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "starting background process of processing incoming data" << std::endl;

  /* watch the data directory for incoming files */
  fd = inotify_init();
  if (fd < 0) {
    clog << "error: " << logstream::error << "unable to start inotify service" << std::endl;
  }
  
  wd = inotify_add_watch(fd, DATA_DIR, IN_CREATE);
  
  while(1) {
    
    struct inotify_event * event;
    
    length = read(fd, buffer, BUF_LEN); 
    if (length < 0) {
      clog << "error: " << logstream::error << "unable to read from inotify file descriptor" << std::endl;
    } 
    
    event = (struct inotify_event *) &buffer[i];
    
    if (event->len) {
      if ( event->mask & IN_CREATE ) {
	if ( event->mask & IN_ISDIR ) {
	  printf( "The directory %s was created.\n", event->name );       
	}
	else {
	  printf( "The file %s was created.\n", event->name );
	}
      }
    }
  }
  
  inotify_rm_watch(fd, wd);
  close(fd);

  return 0;
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
  
  return 0;
}
