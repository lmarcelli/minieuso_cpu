#include "CpuTools.h"

/**
 * default constructor 
 */
CpuTools::CpuTools() {

}


/**
 * function to run command and pass stdout to a string 
 * @param cmd the command to send using popen
 * NB: maximum size of returned string is limited to MAX_STR_LENGTH
 * defined in CpuTools.h
 */
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

/**
 * function to convert int to a fixed length string 
 */
std::string CpuTools::IntToFixedLenStr(const int input, const int length)
{
    std::ostringstream ostr;

    if (input < 0)
        ostr << '-';

    ostr << std::setfill('0') << std::setw(length) << (input < 0 ? -input : input);

    return ostr.str();
}


/**
 * function to clear a directory  
 */
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


/**
 * replaces spaces in a std::string with underscores
 */
std::string CpuTools::SpaceToUnderscore(std::string text) {

  for(size_t i = 0; i < text.length(); i++) {
    if(text[i] == ' ') {
	text[i] = '_';
    }
  }
  
  return text;
}

/**
 * check if you can ping an IP address
 * @param ip_adress the IP address as a string
 */
bool CpuTools::PingConnect(std::string ip_address) {

  bool is_connected = false;

  /* define ping for single packet with 1 sec timeout */
  std::string cmd = "ping " + ip_address + " -c 10 -i 0.001 -w 1";
  std::string output = CommandToStr(cmd.c_str());

  /* debug */
  std::cout << output << std::endl;
  
  /* look for successful output */
  size_t found = output.find("10 packets transmitted, 10 received, 0% packet loss");
  if (found != std::string::npos) {
    is_connected = true;
  }
  
  return is_connected;
}

/**
 * check if FTP server is up
 */
bool CpuTools::CheckFtp() {

  bool ftp = false;
  std::string cmd = "netstat -a | grep ftp";
  std::string output = CommandToStr(cmd.c_str());
  size_t found = output.find("tcp        0        0        *:ftp         *:*        LISTEN");
  if (found != std::string::npos) {
    ftp =true;
  }
}
