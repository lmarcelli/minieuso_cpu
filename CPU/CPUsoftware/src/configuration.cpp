#include "configuration.h"

/* copy a file */
bool CopyFile(const char * SRC, const char * DEST) {
  std::ifstream src(SRC, std::ios::binary);
  std::ofstream dest(DEST, std::ios::binary);
  dest << src.rdbuf();
  return src && dest;
}

/* parsing a configuration file */
Config * Parse(std::string config_file_local) {
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
  std::string line;
  std::string config_file_name;

  /* define the parameters to parse */ 
  Config * Output = new Config();
  
  std::ifstream cfg_file;
  std::stringstream cf;
  cf << config_file_local;
  config_file_name = cf.str();
  cfg_file.open(config_file_name.c_str());

  if (cfg_file.is_open()) {
    clog << "info: " << logstream::info << "reading from the configuration file" << std::endl; 
    while (getline(cfg_file, line)) {
      std::istringstream in(line);
      std::string type;
      in >> type;
      
      if (type == "CATHODE_VOLTAGE") {
	in >> Output->cathode_voltage;
	printf("CATHODE_VOLTAGE is: %d\n", Output->cathode_voltage);
      }
      else if (type == "DYNODE_VOLTAGE") {
	in >> Output->dynode_voltage;
	printf("DYNODE_VOLTAGE is: %d\n", Output->dynode_voltage);
      }
      else if (type == "SCURVE_START") {
	in >> Output->scurve_start;
	printf("SCURVE_START is: %d\n", Output->scurve_start);
      }
      else if (type == "SCURVE_STEP") {
	in >> Output->scurve_step;
	printf("SCURVE_STEP is: %d\n", Output->scurve_step);
      }
      else if (type == "SCURVE_STOP") {
	in >> Output->scurve_stop;
	printf("SCURVE_STOP is: %d\n", Output->scurve_stop);
      }
      else if (type == "SCURVE_ACC") {
	in >> Output->scurve_acc;
	printf("SCURVE_ACC is: %d\n", Output->scurve_acc);
      }
      else if (type == "DAC_LEVEL") {
	in >> Output->dac_level;
	printf("DAC_LEVEL is: %d\n", Output->dac_level);
      } 
    }
    cfg_file.close();
  }
  else {
    clog << "error: " << logstream::error << "unable to open configuration file" << std::endl;   
  }
  
  return Output;
}


/* reload and parse a configuration file */
Config * Configure(std::string config_file, std::string config_file_local) {

  /* definitions */
  const char * kCfg = config_file.c_str();
  const char * kCfgLocal = config_file_local.c_str();
  Config * ParseOutput = NULL;
  
  /* set up logging */
  std::ofstream log_file(log_name,std::ios::app);
  logstream clog(log_file, logstream::all);
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
    ParseOutput = Parse(config_file_local);

    fclose(file);    
  }
  else {
    clog << "error: " << logstream::error << "configuration file does not exist" << std::endl;

  }

  return ParseOutput;
}

