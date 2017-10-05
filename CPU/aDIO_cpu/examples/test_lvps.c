#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>

#include "aDIO_library.h"

#define ONE_MILLISEC	1000
DeviceHandle aDIO_Device;
uint8_t PortValue[2];
uint8_t Strobe[2];
uint8_t P0Direction = 0;
char *P1Direction = "Output";
char *Str0 = "High";
char *Str1 = "High";
static char *program_name;

static void usage(void);
void PrintHelp();
void PrintValues();

/* Program to pulse port 0 high for 10 ms */
int main () {
  uint32_t minor_number = 0;
  int aDIO_ReturnVal;
  char CommandBuffer[20];
  char *Command;
  char *Arg;
  uint8_t dir_val;
  uint8_t P0Bits[8];
  int clear_count;
  int Quit = 0;
  int Bit = 0;
  int u = 0;
  uint8_t HIGH = FF;
  uint8_t LOW = 0;
  
  /* write the direction of port 0 */
  dir_val = FF;
  for (Bit = 0; Bit < 8; Bit++) {
    P0Bits[Bit] = (dir_val >> Bit) & 0x01;
  }
  
  /* set the bits of port 0 */
  aDIO_ReturnVal =
    LoadPort0BitDir_aDIO(aDIO_Device, P0Bits[7],
			 P0Bits[6], P0Bits[5],
			 P0Bits[4], P0Bits[3],
			 P0Bits[2], P0Bits[1],
			 P0Bits[0]);
  /* sleep 1 ms */
  usleep(ONE_MILLISEC);
  
  /* check the return value */
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  LoadPort0bitDir_aDIO() FAILED");
  }
  
  /* write the value of port 0 = HIGH */
  aDIO_ReturnVal =
    WritePort_aDIO(aDIO_Device, 0, HIGH);
  /* sleep for 1 ms */
  usleep(ONE_MILLISEC); 
  /* check the return */
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  WritePort_aDIO() FAILED");
  }

  /* sleep for 9 ms */
  usleep(9 * ONE_MILLISEC);

  /* write the value of port 0 = LOW */
  aDIO_ReturnVal =
    WritePort_aDIO(aDIO_Device, 0, LOW);
  /* sleep for 1 ms */
  usleep(ONE_MILLISEC); 
  /* check the return */
  if (aDIO_ReturnVal) {
    error(EXIT_FAILURE, errno,
	  "ERROR:  WritePort_aDIO() FAILED");
  }

  getchar();
  
  return 0;
}
