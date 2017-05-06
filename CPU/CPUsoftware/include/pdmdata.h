#ifndef SRC_PDMDATA_H_
#define SRC_PDMDATA_H_

// Constants

#define N_OF_PIXELS_PER_PMT		64 /* number of pixel on PMT */
#define N_OF_PMT_PER_ECASIC 	6	/* number of PMT on EC ASIC board */
#define N_OF_ECASIC_PER_PDM		6  /* number of EC ASIC boards in PDM */

//#define N_OF_PIXEL_PER_PDM		(N_OF_PIXELS_PER_PMT * N_OF_PMT_PER_ECASIC * N_OF_ECASIC_PER_PDM)
#define N_OF_PIXEL_PER_PDM 2304
#define N_OF_SCURVE_THRS		1024


//-----------------------------------------------------------------------------

// Common Zynq board header
// for all packets with scientific and configuration data
typedef struct
{
	uint32_t header; // 'Z'(31:24) | instrument_id(23:16) | data_type(15:8) | packet_ver(7:0)
	uint32_t payload_size;
} ZynqBoardHeader; //

#define INSTRUMENT_ME_PDM		1 	/* Instrument Mini-EUSO PDM*/


// Macros for a header build
#define BuildHeader(data_type, packet_ver) \
	(('Z'<<24) | (INSTRUMENT_ME_PDM<<16) | ((data_type)<<8) | (packet_ver))

//-----------------------------------------------------------------------------
// Data types

#define DATA_TYPE_SCI_RAW 1 /* Scientific raw data */
#define DATA_TYPE_SCI_INT16 2 /* Scientific integrated data with 16 bit pixels*/
#define DATA_TYPE_SCI_INT32 3 /* Scientific integrated data with 32 bit pixels */
#define DATA_TYPE_SCI_POLY 4   /* Scientific polytypic data*/
#define DATA_TYPE_SC_COMMON 40 /* Slow control data (common loading) */
#define DATA_TYPE_SC_INDIV 41  /* Slow control data (individual loading)  */
#define DATA_TYPE_HV_DACS 60  /* HV DACs values*/
#define DATA_TYPE_HV_STATUS 61  /* HV status */

//-----------------------------------------------------------------------------
// Timestamp structure in binary format
// Year 0=2017, 1=2018, 2=2019, 3=...
typedef struct
{
  uint32_t TS_dword; // year(31:26) | month(25:22) | date(21:17) | hour(16:12) | min(11:6) | sec(5:0)
  uint32_t gtu_cnt; // reserv(31:20) | gtu_cnt(19:0)
} TimeStamp;

// Macros for a timestamp build
#define BuildTimeStamp_TS_dword(year, month, date, hour, min, sec) \
  (((year)<<26) | ((month)<<22) | ((date)<<17) | ((hour)<<12) | ((min)<<6) | (sec))

typedef struct
{
  uint64_t n_gtu;
} TimeStamp_symplified;

//-----------------------------------------------------------------------------
// Scientific data types
#define N_OF_FRAMES_RAW_POLY_V0 128
#define N_OF_FRAMES_INT16_POLY_V0 128
#define N_OF_FRAMES_INT32_POLY_V0 128

typedef struct
{
  // symplified timestamp
  TimeStamp_symplified ts;
  // HVPS status
  uint32_t hv_status;
  // reserved field in order to make next fields of this structure 64-bytes aligned
  uint32_t reserv[3+24];
  // raw data (2.5 us GTU)
  uint8_t raw_data [N_OF_FRAMES_RAW_POLY_V0][N_OF_PIXEL_PER_PDM];
  // integrated data (320 us GTU)
  uint16_t int16_data [N_OF_FRAMES_INT16_POLY_V0][N_OF_PIXEL_PER_PDM];
  // double integrated data (~40 ms GTU)
  uint32_t int32_data [N_OF_FRAMES_INT32_POLY_V0][N_OF_PIXEL_PER_PDM];
} DATA_TYPE_SCI_POLY_V5;

typedef struct
{
  ZynqBoardHeader zbh;
  DATA_TYPE_SCI_POLY_V5 payload;
} Z_DATA_TYPE_SCI_POLY_V5;


#endif /* SRC_PDMDATA_H_ */

