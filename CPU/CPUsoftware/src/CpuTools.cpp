#include "CpuTools.h"

/* default constructor */
CpuTools::CpuTools() {

}

/* function to run command and pass stdout to a string */
std::string CpuTools::CommandToStr(const char * cmd) {
  const int buf_size = 512;
  std::array<char, buf_size> buffer;
  std::string result;
  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed!");
  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), buf_size, pipe.get()) != nullptr)
      result += buffer.data();
  }
  return result;
}

/* function to convert int to a fixed length string */
std::string CpuTools::IntToFixedLenStr(const int input, const int length)
{
    std::ostringstream ostr;

    if (input < 0)
        ostr << '-';

    ostr << std::setfill('0') << std::setw(length) << (input < 0 ? -input : input);

    return ostr.str();
}

/* function to clear a directory  */
void CpuTools::ClearFolder(const char * data_dir) {

  DIR * theFolder = opendir(data_dir);
  struct dirent * next_file;
  char filepath[256];   
  while ((next_file = readdir(theFolder)) != NULL) {
    sprintf(filepath, "%s/%s", data_dir, next_file->d_name);
    remove(filepath);
  }
  closedir(theFolder);    
}

/* handle SIGINT */
void CpuTools::SignalHandler(int signum) {

  std::cout << "Interrupt signal (" << signum << ") received" << std::endl;  
  
  /* stop the data acquisition */
  ZynqManager::StopAcquisition();
  std::cout << "Acquisition stopped" << std::endl;  

  /* wait for the HV file to be read out */
  std::cout << "Reading out the HV file..." << std::endl;  
  sleep(2);
  
  /* turn off the HV */
  //ZynqManager::HvpsTurnOff();
  /* cannot do this as causes data scarmbling in Zynq */
  
  /* terminate the program */
  exit(signum);  
}
