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
std::string IntToFixedLenStr(const int input, const int length)
{
    std::ostringstream ostr;

    if (input < 0)
        ostr << '-';

    ostr << std::setfill('0') << std::setw(length) << (input < 0 ? -input : input);

    return ostr.str();
}
