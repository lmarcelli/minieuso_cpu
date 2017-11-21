#ifndef _SYNCHRONISED_FILE_H
#define _SYNCHRONISED_FILE_H

#include <mutex>

#include "log.h"

/* handles asynchronous writing to file from multiple threads */
class SynchronisedFile {
public:
  std::string path;

  SynchronisedFile(std::string path); 
  ~SynchronisedFile();
  template <class GenericType>
  size_t Write(GenericType payload) {

    /* lock to one thread at a time */
    std::lock_guard<std::mutex> lock(_writerMutex);
    
    /*  write the payload to the file */
    size_t check = fwrite(payload, sizeof(*payload), 1, this->_ptr_to_file);
    if (check != 1) {
      clog << "error: " << logstream::error << "fwrite failed to " << this->path << std::endl;
      return check;
    }

    return check;
  }

private:
  std::mutex _writerMutex;
  FILE * _ptr_to_file;
};


class Writer {
public:
  Writer(std::shared_ptr<SynchronisedFile> sf);
  
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
