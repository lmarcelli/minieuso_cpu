#include "SynchronisedFile.h"

/* default constructor */
SynchronisedFile::SynchronisedFile(std::string path) {
  this->_path = path;
  const char * file_name = path.c_str();
  
  /* open file for writing */
  this->_ptr_to_file = fopen(file_name, "wb");
  if (!this->_ptr_to_file) {
    clog << "error: " << logstream::error << "cannot open the file " << this->_path << std::endl;
  }

}

/* destructor */
SynchronisedFile::~SynchronisedFile() {
  /* close the file */
  fclose(this->_ptr_to_file);
}

template <class GenericType>
size_t SynchronisedFile::Write(GenericType payload) {

  /* lock to one thread at a time */
  std::lock_guard<std::mutex> lock(_writerMutex);
  
  /*  write the payload to the file */
  size_t check = fwrite(payload, sizeof(*payload), 1, this->_ptr_to_file);
  if (check != 1) {
    clog << "error: " << logstream::error << "fwrite failed to " << this->_path << std::endl;
    return check;
  }

  return check;
}


/* default constructor */
Writer::Writer(std::shared_ptr<SynchronisedFile> sf) {
  this->_sf = sf;
}

template <class GenericType>
void Writer::WriteToSynchFile(GenericType payload) {
  /* call write to file */
  _sf->Write(payload);
}
