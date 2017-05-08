#ifndef _CONFIG_H
#define _CONFIG_H

/* struct for output of the configuration file */
struct Config {
  int cathode_voltage;
  int dynode_voltage;
  int scurve_start;
  int scurve_step;
  int scurve_stop;
  int scurve_acc;
  int dac_level;
};


#endif
