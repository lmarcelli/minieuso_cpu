#ifndef _DATA_FORMAT_SE_H
#define _DATA_FORMAT_SE_H

/* CPU data format definition */
/*----------------------------*/
/* OLD SINGLE EVENT FORMAT USED IN EM TESTS JAN - AUG 2017  */
/* for storage of packets coming from the Zynq board and ancillary instruments */
/* Francesca Capel: capel.francesca@gmail.com */
/* NB: the Mini-EUSO CPU is little endian */

/* old single event format for backward compatibility */
#include "pdmdata_se.h"

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

/* scurve readout fixed size parameters */
#define SCURVE_STEPS_MAX (100 + 1)
#define SCURVE_ADDS_MAX 1
#define SCURVE_FRAMES_MAX (SCURVE_STEPS_MAX * SCURVE_ADDS_MAX)

/* file types */
#define CPU_FILE_TYPE 'C'
#define CPU_FILE_VER 1

/* packet types */
#define HK_PACKET_TYPE 'H'
#define SC_PACKET_TYPE 'S'
#define CPU_PACKET_TYPE 'P'
#define HK_PACKET_VER 1
#define SC_PACKET_VER 1
#define CPU_PACKET_VER 1

/* Timestamp structure in binary format */
/* Year 0=2017, 1=2018, 2=2019, 3=... */
/* 4 bytes */
typedef struct
{
  uint32_t cpu_time_stamp; // y | m | d | h | m | s | 0 | 0 
} CpuTimeStamp;


/* housekeeping packet for other data */
/* 358 bytes */
typedef struct
{
  CpuPktHeader hk_packet_header; /* 14 bytes */
  CpuTimeStamp hk_time; /* 4 bytes */
  float photodiode_data[4]; 
  float sipm_data[64];
  float sipm_single;
  float therm_data[16];
} HK_PACKET;

/* CPU packet for incoming data every 5.24 s */
/* 2064784 bytes */
typedef struct
{
  CpuPktHeader cpu_packet_header; /* 14 bytes */
  CpuTimeStamp cpu_time; /* 4 bytes */
  Z_DATA_TYPE_SCI_POLY_V5 zynq_packet; /* 2064408 bytes */
  HK_PACKET hk_packet; /* 358 bytes */
} CPU_PACKET;

/* scurve packet for checking pixels */
/* 232730 bytes */
typedef struct
{
  CpuPktHeader sc_packet_header; /* 14 bytes */
  CpuTimeStamp sc_time; /* 4 bytes */
  uint16_t sc_start;
  uint16_t sc_step;
  uint16_t sc_stop;
  uint16_t sc_add;
  uint8_t sc_data [SCURVE_FRAMES_MAX][N_OF_PIXEL_PER_PDM];
} SCURVE_PACKET;

/* CPU file to store one run */
/* 51852350 bytes */
typedef struct
{
  CpuFileHeader cpu_file_header; /* 10 bytes */
  SCURVE_PACKET scurve_packet; /* 232730 bytes */
  CPU_PACKET cpu_run_payload[RUN_SIZE]; /* 51619600 bytes */
  CpuFileTrailer cpu_file_trailer; /* 10 bytes */
} CPU_FILE;

#pragma pack(pop) /* return to normal packing */

#endif /* _DATA_FORMAT_SE_H */
