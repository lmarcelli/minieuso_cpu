#ifndef _SYNCHRONISED_FILE_H
#define _SYNCHRONISED_FILE_H

#include <boost/crc.hpp>  
#include <mutex>
#include <memory>

#include "log.h"

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
  
  uint32_t Checksum();
  template <class GenericType>
  size_t Write(GenericType payload) {

    /* lock to one thread at a time */
    std::lock_guard<std::mutex> lock(_accessMutex);
    
    /*  write the payload to the file */
    size_t check = fwrite(payload, sizeof(*payload), 1, this->_ptr_to_file);
    if (check != 1) {
      clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;
      return check;
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
  uint32_t GetChecksum();
  
  template <class GenericType>
  void WriteToSynchFile(GenericType payload) {
    /* call write to file */
    _sf->Write(payload);
  }
  
private:
  std::shared_ptr<SynchronisedFile> _sf;
};

#endif
/* _SYNCHRONISED_FILE_H */
