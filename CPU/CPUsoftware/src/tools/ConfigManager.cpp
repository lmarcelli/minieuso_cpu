#include "ConfigManager.h"

/** constructor
 * initialise the file paths and configuration output
 */
ConfigManager::ConfigManager () {
  this->config_file_local = CONFIG_FILE_LOCAL;
  this->config_file = CONFIG_FILE_USB;

  this->ConfigOut = std::make_shared<Config>();
  
  /* initialise struct members to -1 */
  this->ConfigOut->cathode_voltage = -1;
  this->ConfigOut->dynode_voltage = -1;
  this->ConfigOut->scurve_start = -1;
  this->ConfigOut->scurve_step = -1;
  this->ConfigOut->scurve_stop = -1;
  this->ConfigOut->scurve_acc = -1;
  this->ConfigOut->dac_level = -1;
  this->ConfigOut->N1 = -1;
  this->ConfigOut->N2 = -1;
  this->ConfigOut->L2_N_BG = -1;
  this->ConfigOut->L2_LOW_THRESH = -1;

  /* initialise HV switch to be set by InputParser */
  /* stored here to be easily passed around the DataAcquisition */
  this->ConfigOut->hv_on = false;

  /* initialise instrument and acquisition modes to be set by RunInstrument */
  /* stored here to be easily passed around the DataAcquisition */
  this->ConfigOut->instrument_mode = 0;
  this->ConfigOut->acquisition_mode = 0;
  this->ConfigOut->hvps_log_len = 0;
}

/**
 * constructor
 * @param cfl path to the local configuration file
 * @param cf path to the configuration file to be copied over 
 */
ConfigManager::ConfigManager (std::string cfl, std::string cf) {
  this->config_file_local = cfl;
  this->config_file = cf;

  this->ConfigOut = std::make_shared<Config>();
  
  /* initialise struct members to -1 */
  this->ConfigOut->cathode_voltage = -1;
  this->ConfigOut->dynode_voltage = -1;
  this->ConfigOut->scurve_start = -1;
  this->ConfigOut->scurve_step = -1;
  this->ConfigOut->scurve_stop = -1;
  this->ConfigOut->scurve_acc = -1;
  this->ConfigOut->dac_level = -1;
  this->ConfigOut->N1 = -1;
  this->ConfigOut->N2 = -1;
  this->ConfigOut->L2_N_BG = -1;
  this->ConfigOut->L2_LOW_THRESH = -1;
  

  /* initialise HV switch to be set by InputParser */
  /* stored here to be easily passed around the DataAcquisition */
  this->ConfigOut->hv_on = false;

  /* initialise instrument and acquisition modes to be set by RunInstrument */
  /* stored here to be easily passed around the DataAcquisition */
  this->ConfigOut->instrument_mode = 0;
  this->ConfigOut->acquisition_mode = 0;
  this->ConfigOut->hvps_log_len = 0;

}

/**
 * copy a file 
 * @param SRC path to source
 * @param DEST path to destination 
 */
bool ConfigManager::CopyFile(const char * SRC, const char * DEST) { 

  std::ifstream src(SRC, std::ios::binary);
  std::ofstream dest(DEST, std::ios::binary);

  /* check file exists */
  if (src.good() && dest.good()) {
    clog << "info: " << logstream::info << "copying " << SRC << " to " << DEST << std::endl; 
    dest << src.rdbuf();
  }
  else {
    if (!src.good()) {
      clog << "error: " << logstream::error << "cannot find file to copy" << std::endl; 
      std::cout << "ERROR: cannot find config file to copy" << std::endl;
    }
    if (!src.good()) {
      clog << "error: " << logstream::error << "cannot find destination to copy to" << std::endl; 
      std::cout << "ERROR: cannot find config file destination to copy to" << std::endl;
    }
  }
  return src && dest;
}

/**
 * parse the configuration file 
 */
void ConfigManager::Parse() {

  std::string line;
  std::string config_file_name;
  
  std::ifstream cfg_file;
  std::stringstream cf;
  cf << this->config_file_local;
  config_file_name = cf.str();

  cfg_file.open(config_file_name.c_str());

  if (cfg_file.is_open()) {
    clog << "info: " << logstream::info << "reading from the configuration file" << std::endl; 
    while (getline(cfg_file, line)) {
      std::istringstream in(line);
      std::string type;
      in >> type;

      if (type == "CATHODE_VOLTAGE") {
	in >> this->ConfigOut->cathode_voltage;
      }
      else if (type == "DYNODE_VOLTAGE") {
	in >> this->ConfigOut->dynode_voltage;
      }
      else if (type == "SCURVE_START") {
	in >> this->ConfigOut->scurve_start;
      }
      else if (type == "SCURVE_STEP") {
	in >> this->ConfigOut->scurve_step;
      }
      else if (type == "SCURVE_STOP") {
	in >> this->ConfigOut->scurve_stop;
      }
      else if (type == "SCURVE_ACC") {
	in >> this->ConfigOut->scurve_acc;
      }
      else if (type == "DAC_LEVEL") {
	in >> this->ConfigOut->dac_level;
      } 
      else if (type == "N1") {
	in >> this->ConfigOut->N1;
      }
      else if (type == "N2") {
	in >> this->ConfigOut->N2;
      }
      else if (type == "L2_N_BG") {
	in >> this->ConfigOut->L2_N_BG;
      }
      else if (type == "L2_LOW_THRESH") {
	in >> this->ConfigOut->L2_LOW_THRESH;
      }
      
    }
    cfg_file.close();
    	
  }
  else {
    clog << "error: " << logstream::error << "unable to open configuration file" << std::endl;   
    std::cout << "ERROR: unable to open configuration file" << std::endl;
    std::cout << "configuration is not set" << std::endl;
 }
  printf("\n");
     
}


/**
 * reload and parse the configuration file 
 */
void ConfigManager::Configure() {

  /* definitions */
  const char * kCfg = this->config_file.c_str();
  const char * kCfgLocal = this->config_file_local.c_str();

  clog << "info: " << logstream::info << "running configuration" << std::endl;
  
  if (FILE *file = fopen(kCfg, "r")) {

    /* copy the file locally */ 
    if (CopyFile(kCfg, kCfgLocal)) {
      clog << "info: " << logstream::info << "configuration file sucessfully copied over" << std::endl;     
    }
    else {
      clog << "warning: " << logstream::warning << "unable to copy the configuration file" << std::endl;
    }

    /* parse the file */
    this->Parse();

    fclose(file);    
  }
  else {
    clog << "error: " << logstream::error << "configuration file " << config_file << " does not exist" << std::endl;
    std::cout << "ERROR: configuration file " << config_file << " does not exist" << std::endl;
  }

  return;
}

/**
 * check a configuration file has indeed been parsed, by checking output != -1
 * @param ConfigOut the output to be checked
 */
bool ConfigManager::IsParsed() {

  if (this->ConfigOut->cathode_voltage != -1 &&
      this->ConfigOut->dynode_voltage != -1 &&
      this->ConfigOut->scurve_start != -1 &&
      this->ConfigOut->scurve_step != -1 &&
      this->ConfigOut->scurve_acc != -1 &&
      this->ConfigOut->dac_level != -1 &&
      this->ConfigOut->N1 != -1 &&
      this->ConfigOut->N2 != -1) {
    
    return true;
  }
  else {
    return false;
  }
}
