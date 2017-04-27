/* CPU data format definition 
 * for storage of packets coming from the Zynq board 
 * Francesca Capel: capel.francesca@gmail.com
*/

#ifndef _DATA_FORMAT_H
#define _DATA_FORMAT_H

/* cpu file header */
typedef struct
{
  uint32_t header; /* 'CPU'(31:24) | instrument_id(23:16) | data_type(15:8) | packet_ver(7:0) */
  uint32_t run_size; /* number of zynq packets in the run */
} CpuFileHeader; 

/* cpu packet header */
typedef struct
{
  uint32_t header; /* 'PKT_CPU'(31:24) | instrument_id(23:16) | data_type(15:8) | packet_ver(7:0) */
  uint32_t pkt_size; /* number of zynq packets in the run */
} CpuPktHeader; 


#define INSTRUMENT_ME_PDM 1 /* Instrument Mini-EUSO PDM*/
#define RUN_SIZE 10

/* macros for building the headers */
#define BuildCpuFileHeader(data_type, packet_ver) \
  (('CPU'<<24) | (INSTRUMENT_ME_PDM<<16) | ((data_type)<<8) | (packet_ver))
#define BuildCpuPktHeader(data_type, packet_ver) \
  (('PKT_CPU'<<24) | (INSTRUMENT_ME_PDM<<16) | ((data_type)<<8) | (packet_ver))


/* Timestamp structure in binary format */
/* Year 0=2017, 1=2018, 2=2019, 3=... */
typedef struct
{
  uint32_t cpu_time_stamp; // year(31:26) | month(25:22) | date(21:17) | hour(16:12) | min(11:6) | sec(5:0)
} CpuTimeStamp;

/* macro for building the timestamp */
#define BuildCpuTimeStamp(year, month, date, hour, min, sec) \
  (((year)<<26) | ((month)<<22) | ((date)<<17) | ((hour)<<12) | ((min)<<6) | (sec))

/* CPU packet for incoming data every 5.34 s */
typedef struct
{
  CpuPktHeader cpu_packet_header;
  CpuTimeStamp cpu_time;
  Z_DATA_TYPE_SCI_POLY_V5 zynq_data;
} CPU_PACKET;

/* CPU file to store one run of ~40 min */
typedef struct
{
  CpuFileHeader cpu_file_header;
  CPU_PACKET cpu_run_payload[RUN_SIZE];
} CPU_FILE;


#endif /* _DATA_FORMAT_H */
