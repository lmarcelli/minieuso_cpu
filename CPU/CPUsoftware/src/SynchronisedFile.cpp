#include "SynchronisedFile.h"

/* default constructor */
SynchronisedFile::SynchronisedFile(std::string path) {

  this->path = path;
  const char * file_name = path.c_str();
  
  /* open file for writing */
  this->_ptr_to_file = fopen(file_name, "wb");
  if (!this->_ptr_to_file) {
    clog << "error: " << logstream::error << "cannot open the file " << this->path << std::endl;
  }

}

/* destructor */
SynchronisedFile::~SynchronisedFile() {

  /* close the file */
  fclose(this->_ptr_to_file);
}

uint32_t SynchronisedFile::Checksum() {

  /* lock to one thread at a time */
  std::lock_guard<std::mutex> lock(_accessMutex);

  /* close the file */
  fclose(this->_ptr_to_file);
  
  /* calculate the CRC */
  boost::crc_32_type crc_result;
  std::ifstream ifs(this->path, std::ios_base::binary);	
  if(ifs) {
    do {
      char buffer[buffer_size];
      ifs.read(buffer, buffer_size);
      crc_result.process_bytes(buffer, ifs.gcount());
    } while (ifs);
  }
  else {
    clog << "error: " << logstream::error << "cannot open the file " << this->path << std::endl;
    return 1;
  }
  std::cout << std::hex << std::uppercase << "CRC = " << crc_result.checksum() << std::endl;
  clog << "info: " << logstream::info << "CRC for " << this->path << " = "
       << std::hex << std::uppercase << crc_result.checksum() << std::endl;

  /* reopen the file for further writing */
  const char * file_name = this->path.c_str();
  this->_ptr_to_file = fopen(file_name, "wb");
  if (!this->_ptr_to_file) {
    clog << "error: " << logstream::error << "cannot open the file " << this->path << std::endl;
  }
  
  return crc_result.checksum();
}

/* default constructor */
Access::Access(std::shared_ptr<SynchronisedFile> sf) {
  this->_sf = sf;
}

uint32_t Access::GetChecksum() {

  uint32_t checksum =_sf->Checksum();
  return checksum;
}
