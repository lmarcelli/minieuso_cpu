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
    if (fgets(buffer.data(), buf_size, pipe.get()) != nullptr) {
      /* stop if result over a certain length */
      if (result.size() < MAX_STR_LENGTH) {
	result += buffer.data();
      }
    }
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

  /* signal to main program */
  
  /* terminate the program */
  exit(signum);  
}
