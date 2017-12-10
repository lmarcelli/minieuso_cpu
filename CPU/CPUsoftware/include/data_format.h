#ifndef _DATA_FORMAT_H
#define _DATA_FORMAT_H

/* CPU data format definition */
/*----------------------------*/
/* NEW VARIABLE PACKET FORMAT FROM NOV 2017 */
/* for storage of packets coming from the Zynq board and ancillary instruments */
/* Francesca Capel: capel.francesca@gmail.com */
/* NB:the Mini-EUSO CPU is little endian */

/* new multi event data format */
#include <vector>

#include "pdmdata.h"

/* instrument definitions */
#define INSTRUMENT_ME_PDM 1 /* Instrument Mini-EUSO PDM */
#define ID_TAG 0xAA55
#define RUN_SIZE 25

#pragma pack(push, 1) /* force no padding in structs */

/* cpu file header */
/* 10 bytes */
typedef struct
{
  uint16_t spacer = ID_TAG; /* AA55 HEX */
  uint32_t header; /* 'C'(31:24) | instrument_id(23:16) | file_type(15:8) | file_ver(7:0) */
  uint32_t run_size; /* number of cpu packets in the run */
} CpuFileHeader; 

/* cpu file trailer */
/* 10 bytes */
typedef struct
{
  uint16_t spacer = ID_TAG; /* AA55 HEX */
  uint32_t run_size; /* number of cpu packets in the run */
  uint32_t crc; /* checksum */
} CpuFileTrailer; 

/* generic packet header for all cpu packets and hk/scurve sub packets */
/* the zynq packet has its own header defined in pdmdata.h */
/* 14 bytes */
typedef struct
{
  uint16_t spacer = ID_TAG; /* AA55 HEX */
  uint32_t header; /* 'P'(31:24) | instrument_id(23:16) | pkt_type(15:8) | pkt_ver(7:0) */
  uint32_t pkt_size; /* size of packet */
  uint32_t pkt_num; /* counter for each pkt_type, reset each run */
} CpuPktHeader; 

/* file types */
#define CPU_FILE_TYPE 'C'
#define CPU_FILE_VER 1

/* packet types */
#define THERM_PACKET_TYPE 'T'
#define HK_PACKET_TYPE 'H'
#define SC_PACKET_TYPE 'S'
#define CPU_PACKET_TYPE 'P'
#define THERM_PACKET_VER 1
#define HK_PACKET_VER 1
#define SC_PACKET_VER 2
#define CPU_PACKET_VER 2

/* for the analog readout */
#define N_CHANNELS_PHOTODIODE 4
#define N_CHANNELS_SIPM 64
#define N_CHANNELS_THERM 10

/* size of the zynq packets */
#define MAX_PACKETS_L1 4
#define MAX_PACKETS_L2 4
#define MAX_PACKETS_L3 1

/* timestamp */
/* 4 bytes */
typedef struct
{
  uint32_t cpu_time_stamp; // unix time in s
} CpuTimeStamp;


/* thermistor packet for temperature data */
/* 58 bytes */
typedef struct
{
  CpuPktHeader therm_packet_header; /* 14 bytes */
  CpuTimeStamp therm_time; /* 4 bytes */
  float therm_data[N_CHANNELS_THERM]; /* 40 bytes */
} THERM_PACKET;


/* housekeeping packet for other data */
/* 294 bytes */
typedef struct
{
  CpuPktHeader hk_packet_header; /* 14 bytes */
  CpuTimeStamp hk_time; /* 4 bytes */
  float photodiode_data[N_CHANNELS_PHOTODIODE]; /* 16 bytes */ 
  float sipm_data[N_CHANNELS_SIPM]; /* 256 bytes */
  float sipm_single; /* 4 bytes */
} HK_PACKET;

/* zynq packet passed to the CPU every 5.24 s */
/* variable size, depending on configurable N1 and N2 */
typedef struct
{
  uint8_t N1; /* 1 byte */
  uint8_t N2; /* 1 byte */
  std::vector<Z_DATA_TYPE_SCI_L1_V2> level1_data; /* 294944 * N1 bytes */
  std::vector<Z_DATA_TYPE_SCI_L2_V2> level2_data; /* 589856 * N2 bytes */
  Z_DATA_TYPE_SCI_L3_V2 level3_data; /* 1179684 bytes */
  
} ZYNQ_PACKET;

/* CPU packet for incoming data every 5.24 s */
/* 4719148 bytes */
typedef struct
{
  CpuPktHeader cpu_packet_header; /* 14 bytes */
  CpuTimeStamp cpu_time; /* 4 bytes */
  ZYNQ_PACKET zynq_packet; /* variable size */
  HK_PACKET hk_packet; /* 294 bytes */
} CPU_PACKET;

/* CPU file to store one run */
/* shown here as demonstration only */
/* vraiable size */
typedef struct
{
  CpuFileHeader cpu_file_header; /* 10 bytes */
  CPU_PACKET cpu_run_payload[RUN_SIZE]; /* variable size */
  CpuFileTrailer cpu_file_trailer; /* 10 bytes */
} CPU_FILE;

/* SC packet to store S-curve from Zynq  */
/* 9437218 */
typedef struct
{
  CpuPktHeader sc_packet_header; /* 14 bytes */
  CpuTimeStamp sc_time; /* 4 bytes */
  uint16_t sc_start;
  uint16_t sc_step;
  uint16_t sc_stop;
  uint16_t sc_acc;
  Z_DATA_TYPE_SCURVE_V1 sc_data; /* 9437192 bytes */
} SC_PACKET;

/* SC file to store a single S-curve */
/* shown here as demonstration only */
/* 9437238 bytes (~9 MB) */
typedef struct
{
  CpuFileHeader cpu_file_header; /* 10 bytes */
  SC_PACKET scurve_packet; /* 9437218 bytes */
  CpuFileTrailer cpu_file_trailer; /* 10 bytes */
} SC_FILE;

#pragma pack(pop) /* return to normal packing */

#endif /* _DATA_FORMAT_H */
