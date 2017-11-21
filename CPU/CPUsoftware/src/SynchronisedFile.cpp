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

/* default constructor */
Writer::Writer(std::shared_ptr<SynchronisedFile> sf) {
  this->_sf = sf;
}

