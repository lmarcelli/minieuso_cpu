#ifndef _SYNCHRONISED_FILE_H
#define _SYNCHRONISED_FILE_H

#include <boost/crc.hpp>  
#include <mutex>
#include <memory>

#include "log.h"
#include "data_format.h"
#include "ConfigManager.h"

/* for use with CRC checksum calculation */
/* redefine this to change to processing buffer size */
#ifndef PRIVATE_BUFFER_SIZE
#define PRIVATE_BUFFER_SIZE  1024
#endif
/* global objects */
std::streamsize const buffer_size = PRIVATE_BUFFER_SIZE;


/* handles asynchronous writing to file from multiple threads */
class SynchronisedFile {
public:
  std::string path;

  SynchronisedFile(std::string path); 
  ~SynchronisedFile();

  enum WriteType : uint8_t {
    CONSTANT = 0,
    VARIABLE_D1 = 1,
    VARIABLE_D2 = 2,
  };

  uint32_t Checksum();
  void Close();
  template <class GenericType>
  size_t Write(GenericType payload, WriteType write_type, Config * ConfigOut = NULL) {

    size_t check = 0;
    
    /* lock to one thread at a time */
    std::lock_guard<std::mutex> lock(_accessMutex);

    clog << "info: " << logstream::info << "writing to SynchronisedFile " << this->path << std::endl;
      
    /*  write the payload to the file */
    switch(write_type) {
    case CONSTANT:
   
      check = fwrite(payload, sizeof(*payload), 1, this->_ptr_to_file);
      if (check != 1) {
	clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;
	return check;
      }
      
    break;
    case VARIABLE_D1:

      check = fwrite(payload, sizeof(*payload), ConfigOut->N1, this->_ptr_to_file);
      if (check != ConfigOut->N1) {
	clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;

	/* DEBUG check why fwrite fails */
	std::cout << "FWRITE FAIL" << std::endl;
	std::cout << "check = " << check << std::endl;
	std::cout << "feof: " << feof(this->_ptr_to_file) << std::endl;
	std::cout << "ferror: " << ferror(this->_ptr_to_file) << std::endl;
  
	return check;
      }

      break;
    case VARIABLE_D2:

      check = fwrite(payload, sizeof(*payload), ConfigOut->N2, this->_ptr_to_file);
      if (check != ConfigOut->N2) {
	clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;

	/* DEBUG check why fwrite fails */
	std::cout << "FWRITE FAIL" << std::endl;
	std::cout << "check = " << check << std::endl;
	std::cout << "feof: " << feof(this->_ptr_to_file) << std::endl;
	std::cout << "ferror: " << ferror(this->_ptr_to_file) << std::endl;
  
	return check;
      }
      break;
   
    }

    return check;
  }

private:
  std::mutex _accessMutex;
  FILE * _ptr_to_file;
};


class Access {
public:
  Access(std::shared_ptr<SynchronisedFile> sf);

  std::string path;
  uint32_t GetChecksum();
  void CloseSynchFile();
 
  template <class GenericType>
  void WriteToSynchFile(GenericType payload, SynchronisedFile::WriteType write_type, Config * ConfigOut = NULL) {
    /* call write to file */
    this->_sf->Write(payload, write_type, ConfigOut);
  }
  
private:
  std::shared_ptr<SynchronisedFile> _sf;
};

#endif
/* _SYNCHRONISED_FILE_H */
