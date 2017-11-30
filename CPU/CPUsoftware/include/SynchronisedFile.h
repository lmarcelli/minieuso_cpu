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
    VARIABLE = 1,
  };

  uint32_t Checksum();
  void Close();
  template <class GenericType>
  size_t Write(GenericType payload, WriteType write_type, Config * ConfigOut = NULL) {

    size_t check = 0;
    size_t actual_size = 0;
    
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
    case VARIABLE:
      if (ConfigOut != NULL) {
	actual_size = (sizeof(CpuPktHeader) + sizeof(CpuTimeStamp)
			      + sizeof(HK_PACKET)
			      + (sizeof(Z_DATA_TYPE_SCI_L1_V2) * ConfigOut->N1)
			      + (sizeof(Z_DATA_TYPE_SCI_L2_V2) * ConfigOut->N2)
			      + sizeof(Z_DATA_TYPE_SCI_L3_V2));
      }

      /* DEBUG: print actual size */
      std::cout << "actual_size = " << actual_size << std::endl;
      
      check = fwrite(payload, actual_size, 1, this->_ptr_to_file);
      if (check != 1) {
	clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;
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
