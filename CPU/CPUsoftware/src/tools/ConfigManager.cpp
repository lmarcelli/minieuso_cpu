#include "ConfigManager.h"

/* default constructor */
ConfigManager::ConfigManager () {
  this->config_file_local = CONFIG_FILE_LOCAL;
  this->config_file = CONFIG_FILE_USB;
    
}

/* constructor */
ConfigManager::ConfigManager (std::string cfl, std::string cf) {
  this->config_file_local = cfl;
  this->config_file = cf;
}

/* copy a file */
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
    }
    if (!src.good()) {
      clog << "error: " << logstream::error << "cannot find destination to copy to" << std::endl; 
    }
  }
  return src && dest;
}

/* parse a configuration file */
Config * ConfigManager::Parse() {

  std::string line;
  std::string config_file_name;

  /* define the parameters to parse */ 
  Config * Output = new Config();
  
  std::ifstream cfg_file;
  std::stringstream cf;
  cf << this->config_file_local;
  config_file_name = cf.str();

  cfg_file.open(config_file_name.c_str());

  printf("CONFIGURATION PARAMETERS\n");  
  if (cfg_file.is_open()) {
    clog << "info: " << logstream::info << "reading from the configuration file" << std::endl; 
    while (getline(cfg_file, line)) {
      std::istringstream in(line);
      std::string type;
      in >> type;

      if (type == "CATHODE_VOLTAGE") {
	in >> Output->cathode_voltage;
	printf("CATHODE_VOLTAGE is %d\n", Output->cathode_voltage);
      }
      else if (type == "DYNODE_VOLTAGE") {
	in >> Output->dynode_voltage;
	printf("DYNODE_VOLTAGE is %d\n", Output->dynode_voltage);
      }
      else if (type == "SCURVE_START") {
	in >> Output->scurve_start;
	printf("SCURVE_START is %d\n", Output->scurve_start);
      }
      else if (type == "SCURVE_STEP") {
	in >> Output->scurve_step;
	printf("SCURVE_STEP is %d\n", Output->scurve_step);
      }
      else if (type == "SCURVE_STOP") {
	in >> Output->scurve_stop;
	printf("SCURVE_STOP is %d\n", Output->scurve_stop);
      }
      else if (type == "SCURVE_ACC") {
	in >> Output->scurve_acc;
	printf("SCURVE_ACC is %d\n", Output->scurve_acc);
      }
      else if (type == "DAC_LEVEL") {
	in >> Output->dac_level;
	printf("DAC_LEVEL is %d\n", Output->dac_level);
      } 
      else if (type == "N1") {
	in >> Output->N1;
	printf("N1 is %d\n", Output->N1);
      }
      else if (type == "N2") {
	in >> Output->N2;
	printf("N2 is %d\n", Output->N2);
      }
      
    }
    cfg_file.close();
  }
  else {
    clog << "error: " << logstream::error << "unable to open configuration file" << std::endl;   
  }
  printf("\n");
     
  return Output;
}


/* reload and parse a configuration file */
Config * ConfigManager::Configure() {

  /* definitions */
  const char * kCfg = this->config_file.c_str();
  const char * kCfgLocal = this->config_file_local.c_str();
  Config * ParseOutput = NULL;

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
    ParseOutput = Parse();

    fclose(file);    
  }
  else {
    clog << "error: " << logstream::error << "configuration file does not exist" << std::endl;

  }

  return ParseOutput;
}

