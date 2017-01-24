/**
    @file

    @brief
        Type definitions used both in kernel and user space

    @verbatim
    --------------------------------------------------------------------------
    This file and its contents are copyright (C) RTD Embedded Technologies,
    Inc.  All Rights Reserved.

    This software is licensed as described in the RTD End-User Software License
    Agreement.  For a copy of this agreement, refer to the file LICENSE.TXT
    (which should be included with this software) or contact RTD Embedded
    Technologies, Inc.
    --------------------------------------------------------------------------
    @endverbatim

    $Id: dm75xx_types.h 79763 2014-06-13 21:01:02Z rgroner $
*/

#ifndef __dm75xx_types_h__
#define __dm75xx_types_h__

/**
 * @defgroup DM75xx_Types_Header DM75xx type definition header file
 * @{
 */

/*=============================================================================
Enumerations
 =============================================================================*/

/**
 * @defgroup DM75xx_Types_Enumerations DM75xx type enumerations
 * @{
 */

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
PCI region access enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @defgroup DM75xx_Types_PCI_Enumerations DM75xx type PCI enumerations
 * @{
 */

/**
 * @brief
 *      Standard PCI region number
 */

enum dm75xx_pci_region_num {

    /**
     * Memory-mapped PLX registers
     */

	DM75xx_PLX_MEM = 0,

    /**
     * I/O-mapped PLX registers
     */

	DM75xx_PLX_IO,

    /**
     * Memory-mapped LAS 0 registers
     */

	DM75xx_LAS0,

    /**
     * Memory-mapped LAS 1 registers
     */

	DM75xx_LAS1
};

/**
 * @brief
 *      Standard PCI region number type
 */

typedef enum dm75xx_pci_region_num dm75xx_pci_region_num_t;

/**
 * @brief
 *      Desired size in bits of access to standard PCI region
 */

enum dm75xx_pci_region_access_size {

    /**
     * 8-bit access
     */

	DM75xx_PCI_REGION_ACCESS_8 = 0,

    /**
     * 16-bit access
     */

	DM75xx_PCI_REGION_ACCESS_16,

    /**
     * 32-bit access
     */

	DM75xx_PCI_REGION_ACCESS_32
};

/**
 * @brief
 *      Standard PCI region access size type
 */

typedef enum dm75xx_pci_region_access_size dm75xx_pci_region_access_size_t;

/**
 * @brief
 *      DM75xx Board
 */

enum _dm75xx_board {
    /**
     * DM7520
     */
	DM75xx_BOARD_DM7520 = 0,
    /**
     * SDM7540
     */
	DM75xx_BOARD_SDM7540
};
/**
 * @brief
 *      DM75xx Board Type
 */
typedef enum _dm75xx_board dm75xx_board_t;
/**
 * @} DM75xx_Types_PCI_Enumerations
 */
/**
 * @defgroup DM75xx_Types_DSP_Enumerations DM75xx type DSP enumerations
 * @{
 */
/**
 * @brief
 *      DSP Command
 */
enum _dm75xx_dsp_command {
    /**
     * Auto Calibration of SDM7540
     */
	DM75xx_DSP_CAL_AUTO = 1,
    /**
     * Internal Flash Download
     */
	DM75xx_DSP_FLASH_DOWNLOAD = 2,
    /**
     * Run User Program
     */
	DM75xx_DSP_USER_RUN = 3,
    /**
     * Upgrade User Program
     */
	DM75xx_DSP_USER_UPGRADE = 4,
    /**
     * Erase Internal Flash
     */
	DM75xx_DSP_INT_FLASH_ERASE = 5,
    /**
     * Erase External Flash
     */
	DM75xx_DSP_EXT_FLASH_ERASE = 6,
    /**
     * Check if DSP still alive
     */
	DM75xx_DSP_ATTENTION = 7,
    /**
     * Load Default Calibration
     */
	DM75xx_DSP_CAL_DEFAULT = 8,
    /**
     * Get Calibration Algorithm Version
     */
	DM75xx_DSP_CAL_VERSION = 10,
    /**
     * Get Boot Loader Version
     */
	DM75xx_DSP_BOOT_VERSION = 11
};
/**
 * @brief
 * DSP Command Type
 */
typedef enum _dm75xx_dsp_command dm75xx_dsp_command_t;
 /**
  * @} DM75xx_Types_DSP_Enumerations
  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 DMA Enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_DMA_Enumerations DM75xx type DMA
 * @{
 */

/**
 * @brief
 * DMA Control Flag
 */
enum _dm75xx_dma_flag {
    /**
     * DMA Initialized
     */
	DM75xx_DMA_FLAG_INIT = 0x01,
    /**
     * DMA Memory Map
     */
	DM75xx_DMA_FLAG_MMAP = 0x02,
    /**
     * DMA Reset DREQ
     */
	DM75xx_DMA_FLAG_RESET = 0x04,
    /**
     * DMA Non Demand Mode
     */
	DM75xx_DMA_FLAG_NONDEMAND = 0x08,
    /**
     * DMA Channel Status
     */
	DM75xx_DMA_FLAG_STATUS = 0x10,
    /**
     * DMA Arbitrary
     */
	DM75xx_DMA_FLAG_ARB = 0x20
};
/**
 * @brief
 * DMA Control Flag Type
 */
typedef enum _dm75xx_dma_flag dm75xx_dma_flag_t;
/**
 * @brief
 * DMA Status Reset Flag
 */
enum _dm75xx_dma_reset {
    /**
     * DMA 0
     */
	DM75xx_DMA_RESET_SEL = 0x01,
    /**
     * DMA Channel Reset Value
     */
	DM75xx_DMA_RESET_VAL = 0x10
};
/**
 * @brief
 * DMA Status Reset Flag Type
 */
typedef enum _dm75xx_dma_reset dm75x_dma_reset_t;
/**
 * @brief
 * DMA Demand Mode Source
 */
enum _dm75xx_dma_request {
    /**
     * Request Disable
     */
	DM75xx_DMA_DEMAND_DISABLE = 0,
    /**
     * A/D Sample Counter
     */
	DM75xx_DMA_DEMAND_SCNT_ADC = 1,
    /**
     * D/A 1 Sample Counter
     */
	DM75xx_DMA_DEMAND_SCNT_DAC1 = 2,
    /**
     * D/A 2 Sample Counter
     */
	DM75xx_DMA_DEMAND_SCNT_DAC2 = 3,
    /**
     * User Timer/Counter 1
     */
	DM75xx_DMA_DEMAND_UTC1 = 4,
    /**
     * A/D FIFO Half Full
     */
	DM75xx_DMA_DEMAND_FIFO_ADC = 8,
    /**
     * D/A 1 FIFO Half Empty
     */
	DM75xx_DMA_DEMAND_FIFO_DAC1 = 9,
    /**
     * D/A 2 FIFO Half Empty
     */
	DM75xx_DMA_DEMAND_FIFO_DAC2 = 10,
};
/**
 * @brief
 * DMA Demand Mode Source Type
 */
typedef enum _dm75xx_dma_request dm75xx_dma_request_t;
/**
 * @brief
 * DMA Local Source
 */
enum _dm75xx_dma_source {
    /**
     * DMA A/D FIFO
     */
	DM75xx_DMA_FIFO_ADC = 0,
    /**
     * DMA D/A 1 FIFO
     */
	DM75xx_DMA_FIFO_DAC1,
    /**
     * DMA D/A 2 FIFO
     */
	DM75xx_DMA_FIFO_DAC2,
    /**
     * DMA HSDIN FIFO
     */
	DM75xx_DMA_FIFO_HSDIN
};
/**
 * @brief
 * DMA Source Type
 */
typedef enum _dm75xx_dma_source dm75xx_dma_source_t;
/**
 * @brief
 * DMA Channel
 */
enum _dm75xx_dma_channel {
    /**
     * DMA Channel 0
     */
	DM75xx_DMA_CHANNEL_0 = 0,
    /**
     * DMA Channel 1
     */
	DM75xx_DMA_CHANNEL_1
};
/**
 * @brief
 * DMA Channel Type
 */
typedef enum _dm75xx_dma_channel dm75xx_dma_channel_t;
/**
 * @} DM75xx_Types_DMA_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Interrupt Enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_INT_Enumerations DM75xx type Interrupt
 * @{
 */
/**
 * @brief
 * Interrupt Source
 */
enum _dm75xx_int_source {
    /**
     * FIFO Write
     */
	DM75xx_INT_FIFO_WRITE = 0x0001,
    /**
     *  Reset CGT
     */
	DM75xx_INT_CGT_RESET = 0x0002,
    /**
     * Reserved
     */
	DM75xx_INT_RESERVED = 0x0004,
    /**
     * Pause CGT
     */
	DM75xx_INT_CGT_PAUSE = 0x0008,
    /**
     * About Counter Out
     */
	DM75xx_INT_ABOUT = 0x0010,
    /**
     * Delay Counter Out
     */
	DM75xx_INT_DELAY = 0x0020,
    /**
     * A/D Sample Counter
     */
	DM75xx_INT_SCNT_ADC = 0x0040,
    /**
     * D/A 1 Update Counter
     */
	DM75xx_INT_SCNT_DAC1 = 0x0080,
    /**
     * D/A 2 Update Counter
     */
	DM75xx_INT_SCNT_DAC2 = 0x0100,
    /**
     * User Timer/Counter 1 Out
     */
	DM75xx_INT_UTC1 = 0x0200,
    /**
     * Inverted User Timer/Counter 1 Out
     */
	DM75xx_INT_UTC1_INV = 0x0400,
    /**
     * User Timer/Counter 2 Out
     */
	DM75xx_INT_UTC2 = 0x0800,
    /**
     * Digital Interrupt
     */
	DM75xx_INT_DIO = 0x1000,
    /**
     * External Interrupt
     */
	DM75xx_INT_EXTERNAL = 0x2000,
    /**
     * External Trigger Rising Edge
     */
	DM75xx_INT_ETRIG_RISING = 0x4000,
    /**
     * External Trigger Falling Edge
     */
	DM75xx_INT_ETRIG_FALLING = 0x8000,
    /**
     * DMA Channel 0 -- ENABLED BY DEFAULT
     */
	DM75xx_INT_DMA_0 = 0x00200000,
    /**
     * DMA Channel 1 -- ENABLED BY DEFAULT
     */
	DM75xx_INT_DMA_1 = 0x00400000,
    /**
     * Analog DIO Pin 1 Pos Edge
     */
	DM75xx_INT_ALGDIO_POS_PIN1 = 0x04000000,
    /**
     * Analog DIO Pin 2 Pos Edge
     */
	DM75xx_INT_ALGDIO_POS_PIN2 = 0x08000000,
    /**
     * Analog DIO Pin 1 Neg Edge
     */
	DM75xx_INT_ALGDI0_NEG_PIN1 = 0x10000000,
    /**
     * Analog DIO Pin 2 Neg Edge
     */
	DM75xx_INT_ALGDIO_NEG_PIN2 = 0x20000000
};
/**
 * @brief
 * Interrupt Source Type
 */
typedef enum _dm75xx_int_source dm75xx_int_source_t;
/**
 * @} DM75xx_Types_INT_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 User Analog DIO Enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_ALGDIO_Enumerations DM75xx type Analog DIO
 * @{
 */
/**
 * @brief
 * Analog DIO Mask
 */
enum _dm75xx_algdio_mask {
    /**
     * Masked
     */
	DM75xx_ALGDIO_MASKED = 0,
    /**
     * Unmasked
     */
	DM75xx_ALGDIO_UNMASKED
};
/**
 * @brief
 * Analog DIO Mask Type
 */
typedef enum _dm75xx_algdio_mask dm75xx_algdio_mask_t;
/**
 * @brief
 * Analog DIO Pins
 */
enum _dm75xx_algdio_pin {
    /**
     * Pin 1
     */
	DM75xx_ALGDIO_PIN1 = 0,
    /**
     * Pin 2
     */
	DM75xx_ALGDIO_PIN2
};
/**
 * @brief
 * Analog DIO Pin Type
 */
typedef enum _dm75xx_algdio_pin dm75xx_algdio_pin_t;
/**
 * @brief
 * Analog DIO Direction
 */
enum _dm75xx_algdio_direction {
    /**
     * Input
     */
	DM75xx_ALGDIO_INPUT = 0,
    /**
     * Output
     */
	DM75xx_ALGDIO_OUTPUT
};
/**
 * @brief
 * Analog DIO Direction Type
 */
typedef enum _dm75xx_algdio_direction dm75xx_algdio_direction_t;
/**
 * @} DM75xx_Types_ALGDIO_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 User I/O Enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_UIO_Enumerations DM75xx type User Output
 * @{
 */
/**
 * @brief
 * User I/O Channel
 */
enum _dm75xx_uio_channel {
    /**
     * User I/O Channel 0
     */
	DM75xx_UIO0 = 0,
    /**
     * User I/O Channel 1
     */
	DM75xx_UIO1
};
/**
 * @brief
 * User I/O Channel Type
 */
typedef enum _dm75xx_uio_channel dm75xx_uio_channel_t;
/**
 * @brief
 * User I/O Source
 */
enum _dm75xx_uio_source {
    /**
     * A/D Conversion Signal
     */
	DM75xx_UIO_ADC = 0,
    /**
     * D/A 1 Update
     */
	DM75xx_UIO_DAC1,
    /**
     * D/A 2 Update
     */
	DM75xx_UIO_DAC2,
    /**
     * Software Programmable
     */
	DM75xx_UIO_PRG
};
/**
 * @brief
 * User I/O Source Type
 */
typedef enum _dm75xx_uio_source dm75xx_uio_source_t;
/**
 * @} DM75xx_Types_UIO_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Digital Input/Output Enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_DIO_Enumerations DM75xx type Digital Input/Output
 * @{
 */
/**
 * @brief
 * Digital I/O Clock
 */
enum _dm75xx_dio_clk {
    /**
     * 8MHZ Clock
     */
	DM75xx_DIO_CLK_8MHZ = 0,
    /**
     * Programmable Clock
     */
	DM75xx_DIO_CLK_UTC1
};
/**
 * @brief
 * Digital I/O Clock Type
 */
typedef enum _dm75xx_dio_clk dm75xx_dio_clk_t;
/**
 * @brief
 * Digital I/O IRQ Mode
 */
enum _dm75xx_dio_mode {
    /**
     * Event Mode
     */
	DM75xx_DIO_MODE_EVENT = 0,
    /**
     * Match Mode
     */
	DM75xx_DIO_MODE_MATCH
};
/**
 * @brief
 * Digital I/O IRQ Mode Type
 */
typedef enum _dm75xx_dio_mode dm75xx_dio_mode_t;
/**
 * @brief
 * Digital I/O Port
 */
enum _dm75xx_dio_port {
    /**
     * Port 0
     */
	DM75xx_DIO_PORT0 = 0,
    /**
     * Port 1
     */
	DM75xx_DIO_PORT1
};
/**
 * @brief
 * Digital I/O Port Type
 */
typedef enum _dm75xx_dio_port dm75xx_dio_port_t;
/**
 * @} DM75xx_Types_DIO_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 External Trigger/Interrupt Enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_EXT_Enumerations DM75xx type External Trigger/Interrupt
 * @{
 */
/**
 * @brief
 * Polarity
 */
enum _dm75xx_ext_polarity {
    /**
     * Positive Edge
     */
	DM75xx_EXT_POLARITY_POS = 0,
    /**
     * Negative Edge
     */
	DM75xx_EXT_POLARITY_NEG,
};
/**
 * @brief
 * Polarity Type
 */
typedef enum _dm75xx_ext_polarity dm75xx_ext_polarity_t;
/**
 * @} DM75xx_Types_EXT_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 SyncBus Enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_SBUS_Enumerations DM75xx type SyncBus enumerations
 * @{
 */
/**
 * @brief
 * SyncBus Enumerations
 */
enum _dm75xx_sbus {
    /**
     * SyncBus 0
     */
	DM75xx_SBUS0 = 0,
    /**
     * SyncBus 1
     */
	DM75xx_SBUS1,
    /**
     * SyncBus 2
     */
	DM75xx_SBUS2
};
/**
 * @brief
 * SyncBus Enumeration Type
 */
typedef enum _dm75xx_sbus dm75xx_sbus_t;
/**
 * @brief
 * SyncBus Source Select
 */
enum _dm75xx_sbus_src {
    /**
     * Software A/D Start
     */
	DM75xx_SBUS_SRC_SOFT_ADC = 0,
    /**
     * Pacer Clock
     */
	DM75xx_SBUS_SRC_PCLK = 1,
    /**
     * Software Pacer Start
     */
	DM75xx_SBUS_SRC_PCLK_START = 1,
    /**
     * Burst Clock
     */
	DM75xx_SBUS_SRC_BCLK = 2,
    /**
     * Software Pacer Stop
     */
	DM75xx_SBUS_SRC_PCLK_STOP = 2,
    /**
     * Digital Interrupt
     */
	DM75xx_SBUS_SRC_DIG_IT = 3,
    /**
     * Software D/A1 Update
     */
	DM75xx_SBUS_SRC_DAC1 = 3,
    /**
     * External Trigger
     */
	DM75xx_SBUS_SRC_ETRIG = 4,
    /**
     * Software D/A2 Update
     */
	DM75xx_SBUS_SRC_DAC2 = 4,
    /**
     * Simultaneous D/A Update
     */
	DM75xx_SBUS_SRC_DAC_UPDATE = 5,
    /**
     * External Pacer Clock
     */
	DM75xx_SBUS_SRC_EPCLK = 5,
    /**
     * D/A Clock
     */
	DM75xx_SBUS_SRC_DAC_CLK = 6,
    /**
     * External Trigger
     */
	DM75xx_SBUS_SRC_ETRIG2 = 6,
    /**
     * User Timer/Counter 2 Out
     */
	DM75xx_SBUS_SRC_UTC2 = 7
};
/**
 * @brief
 * SyncBus Source Select Type
 */
typedef enum _dm75xx_sbus_src dm75xx_sbus_src_t;
/**
 * @} DM75xx_Types_SBUS_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 HighSpeed Digital Enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_HSDIN_Enumerations DM75xx type HighSpeed Digital enumerations
 * @{
 */
/**
 * @brief
 * HSDIN Sampling Signal
 */
enum _dm75xx_hsdin_signal {
    /**
     * Software
     */
	DM75xx_HSDIN_SIGNAL_SOFTWARE = 0,
    /**
     * A/D Conversion Signal
     */
	DM75xx_HSDIN_SIGNAL_ADC,
    /**
     * User Timer/Counter 0 Out
     */
	DM75xx_HSDIN_SIGNAL_UTC0,
    /**
     * User Timer/Counter 1 Out
     */
	DM75xx_HSDIN_SIGNAL_UTC1,
    /**
     * User Timer/Counter 2 Out
     */
	DM75xx_HSDIN_SIGNAL_UTC2,
    /**
     * External Pacer Clock
     */
	DM75xx_HSDIN_SIGNAL_EPCLK,
    /**
     * External Trigger
     */
	DM75xx_HSDIN_SIGNAL_ETRIG
};
/**
 * @brief
 * HSDIN Sampling Signal Type
 */
typedef enum _dm75xx_hsdin_signal dm75xx_hsdin_signal_t;
/**
 * @} DM75xx_Types_HSDIN_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Digital to Analog enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_DAC_Enumerations DM75xx type Digital to Analog enumerations
 * @{
 */
/**
 * @brief
 * DAC primary clock source
 */
enum _dm75xx_dac_freq {
    /**
     * 8 MHz Clock
     */
	DM75xx_DAC_FREQ_8_MHZ = 0,
    /**
     * 20 MHz Clock
     */
	DM75xx_DAC_FREQ_20_MHZ
};
/**
 * @brief
 * DAC primary clock source type
 */
typedef enum _dm75xx_dac_freq dm75xx_dac_freq_t;
/**
 * @brief
 * DAC clock stop source
 */
enum _dm75xx_dac_clk_stop {
    /**
     * Software Pacer Stop
     */
	DM75xx_DAC_CLK_STOP_SOFTWARE_PACER = 0,
    /**
     * External trigger
     */
	DM75xx_DAC_CLK_STOP_ETRIG,
    /**
     * Digital Interrupt
     */
	DM75xx_DAC_CLK_STOP_DIG_IT,
    /**
     * User Timer/Counter 2 out
     */
	DM75xx_DAC_CLK_STOP_UTC2,
    /**
     * Syncbus 0
     */
	DM75xx_DAC_CLK_STOP_SBUS0,
    /**
     * Syncbus 1
     */
	DM75xx_DAC_CLK_STOP_SBUS1,
    /**
     * Syncbus 2
     */
	DM75xx_DAC_CLK_STOP_SBUS2,
    /**
     * Software DAC clock stop
     */
	DM75xx_DAC_CLK_STOP_SOFTWARE,
    /**
     * DAC1 Update Counter
     */
	DM75xx_DAC_CLK_STOP_DAC1_UCNT,
    /**
     * DAC2 Update Counter
     */
	DM75xx_DAC_CLK_STOP_DAC2_UCNT
};
/**
 * @brief
 * DAC clock stop source type
 */
typedef enum _dm75xx_dac_clk_stop dm75xx_dac_clk_stop_t;
/**
 * @brief
 * DAC clock start source
 */
enum _dm75xx_dac_clk_start {
    /**
     * Software pacer start
     */
	DM75xx_DAC_CLK_START_SOFTWARE_PACER = 0,
    /**
     * External trigger
     */
	DM75xx_DAC_CLK_START_ETRIG,
    /**
     * Digital Interrupt
     */
	DM75xx_DAC_CLK_START_DIG_IT,
    /**
     * User Timer/Counter 2 out
     */
	DM75xx_DAC_CLK_START_UTC2,
    /**
     * SyncBus 0
     */
	DM75xx_DAC_CLK_START_SBUS0,
    /**
     * SyncBus 1
     */
	DM75xx_DAC_CLK_START_SBUS1,
    /**
     * SyncBus 2
     */
	DM75xx_DAC_CLK_START_SBUS2,
    /**
     * Software DAC clock start
     */
	DM75xx_DAC_CLK_START_SOFTWARE
};
/**
 * @brief
 * DAC clock start source type
 */
typedef enum _dm75xx_dac_clk_start dm75xx_dac_clk_start_t;
/**
 * @brief
 * DAC Cycle Mode
 */
enum _dm75xx_dac_mode {
    /**
     * Not cycle
     */
	DM75xx_DAC_MODE_NOT_CYCLE = 0,
    /**
     * Cycle
     */
	DM75xx_DAC_MODE_CYCLE
};
/**
 * @brief
 * DAC Cycle Mode Type
 */
typedef enum _dm75xx_dac_mode dm75xx_dac_mode_t;
/**
 * @brief
 * DAC Update Source
 */
enum _dm75xx_dac_update_src {
    /**
     * Software DAC Update
     */
	DM75xx_DAC_UPDATE_SOFTWARE = 0,
    /**
     * CGT Controlled Update
     */
	DM75xx_DAC_UPDATE_CGT,
    /**
     * DAC Clock
     */
	DM75xx_DAC_UPDATE_CLOCK,
    /**
     * External pacer clock
     */
	DM75xx_DAC_UPDATE_EPCLK,
    /**
     * Syncbus 0
     */
	DM75xx_DAC_UPDATE_SBUS0,
    /**
     * Syncbus 1
     */
	DM75xx_DAC_UPDATE_SBUS1,
    /**
     * Syncbus 2
     */
	DM75xx_DAC_UPDATE_SBUS2
};
/**
 * @brief
 * DAC Update Source Type
 */
typedef enum _dm75xx_dac_update_src dm75xx_dac_update_src_t;
/**
 * @brief
 * DAC Output Range
 */
enum _dm75xx_dac_range {
    /**
     * Unipolar 0V to 5V
     */
	DM75xx_DAC_RANGE_UNIPOLAR_5 = 0,
    /**
     * Unipolar 0V to 10V
     */
	DM75xx_DAC_RANGE_UNIPOLAR_10,
    /**
     * Bipolar -5V to 5V
     */
	DM75xx_DAC_RANGE_BIPOLAR_5,
    /**
     * Bipolar -10V to 10V
     */
	DM75xx_DAC_RANGE_BIPOLAR_10
};
/**
 * @brief
 * DAC Output Range Type
 */
typedef enum _dm75xx_dac_range dm75xx_dac_range_t;
/**
 * @brief
 * DAC channels
 *
 * Note: These are given these values specifically so
 * they can be bitwise combined and compared.
 */
enum _dm75xx_dac_channel {
     /**
      * Digital to Analog channel 1
      */
	DM75xx_DAC1 = 1,
     /**
      * Digital to Analog channel 2
      */
	DM75xx_DAC2
};
/**
 * DAC channel type
 */
typedef enum _dm75xx_dac_channel dm75xx_dac_channel_t;
/**
 * @brief
 * DAC Clock Mode
 */
enum _dm75xx_dac_clk_mode {
    /**
     * Free Run Mode
     */
	DM75xx_DAC_CLK_FREE_RUN = 0,
    /**
     * Start/Stop Mode
     */
	DM75xx_DAC_CLK_START_STOP
};
/**
 * @brief
 * DAC Clock Mode Type
 */
typedef enum _dm75xx_dac_clk_mode dm75xx_dac_clk_mode_t;
/**
 * @} DM75xx_Types_DAC_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Analog to Digital enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_ADC_Enumerations DM75xx type Analog to Digital enumerations
 * @{
 */
/**
 * @brief
 * ADC Sample Counter Source
 */
enum _dm75xx_adc_scnt_src {
    /**
     * Reset Channel Gain Table
     */
	DM75xx_ADC_SCNT_SRC_CGT = 0,
    /**
     * A/D FIFO Write
     */
	DM75xx_ADC_SCNT_SRC_FIFO
};
/**
 * @brief
 * ADC Sample Counter Source Type
 */
typedef enum _dm75xx_adc_scnt_src dm75xx_adc_scnt_src_t;
/**
 * @brief
 * ADC Conversion Signal Select
 */
enum _dm75xx_adc_conv_signal {
    /**
     * Software
     */
	DM75xx_ADC_CONV_SIGNAL_SOFTWARE = 0,
    /**
     * Pacer Clock
     */
	DM75xx_ADC_CONV_SIGNAL_PCLK,
    /**
     * Burst Clock
     */
	DM75xx_ADC_CONV_SIGNAL_BCLK,
    /**
     * Digital Interrupt
     */
	DM75xx_ADC_CONV_SIGNAL_DIG_IT,
    /**
     * DAC1 Marker Bit 1
     */
	DM75xx_ADC_CONV_SIGNAL_DAC1_MRKR1,
    /**
     * DAC2 Marker Bit 2
     */
	DM75xx_ADC_CONV_SIGNAL_DAC2_MRKR1,
    /**
     * SyncBus 0
     */
	DM75xx_ADC_CONV_SIGNAL_SBUS0,
    /**
     * SyncBus 1
     */
	DM75xx_ADC_CONV_SIGNAL_SBUS1,
    /**
     * SyncBus 2
     */
	DM75xx_ADC_CONV_SIGNAL_SBUS2
};
/**
 * @brief
 * ADC Conversion Signal Select type
 */
typedef enum _dm75xx_adc_conv_signal dm75xx_adc_conv_signal_t;
/**
 * @} DM75xx_Types_ADC_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Burst Clock enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_BCLK_Enumerations DM75xx type Burst Clock enumerations
 * @{
 */

/**
 * @brief
 * Burst Clock primary frequency
 */
enum _dm75xx_bclk_freq {
    /**
     * 8 MHz Clock
     */
	DM75xx_BCLK_FREQ_8_MHZ = 0,
    /**
     * 20 MHz Clock
     */
	DM75xx_BCLK_FREQ_20_MHZ
};
/**
 * @brief
 * Burst Clock primary frequency type
 */
typedef enum _dm75xx_bclk_freq dm75xx_bclk_freq_t;

/**
 * @brief
 * Burst Clock Start Trigger
 */
enum _dm75xx_bclk_start {
    /**
     * Software A/D
     */
	DM75xx_BCLK_START_SOFTWARE = 0,
    /**
     * Pacer Clock
     */
	DM75xx_BCLK_START_PACER,
    /**
     * External Trigger
     */
	DM75xx_BCLK_START_ETRIG,
    /**
     * Digital Interrupt
     */
	DM75xx_BCLK_START_DIG_IT,
    /**
     * SyncBus 0
     */
	DM75xx_BCLK_START_SBUS0,
    /**
     * SyncBus 1
     */
	DM75xx_BCLK_START_SBUS1,
    /**
     * SyncBus 2
     */
	DM75xx_BCLK_START_SBUS2
};
/**
 * @brief
 * Burst Clock Start Trigger Type
 */
typedef enum _dm75xx_bclk_start dm75xx_bclk_start_t;
/**
 * @} DM75xx_Types_BCLK_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Pacer Clock enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_PCLK_Enumerations DM75xx type Pacer Clock enumerations
 * @{
 */

/**
 * @brief
 * Pacer Clock Trigger Mode
 */

enum _dm75xx_pclk_mode {

    /**
     * Single Cycle Mode
     */
	DM75xx_PCLK_NO_REPEAT = 0,

    /**
     * Repeat Mode
     */
	DM75xx_PCLK_REPEAT
};

/**
 * @brief
 * Pacer Clock Trigger Mode Type
 */

typedef enum _dm75xx_pclk_mode dm75xx_pclk_mode_t;

/**
 * @brief
 * Pacer Clock Stop
 */
enum _dm75xx_pclk_stop {
    /**
     * Software
     */
	DM75xx_PCLK_STOP_SOFTWARE = 0,
    /**
     * External Trigger
     */
	DM75xx_PCLK_STOP_ETRIG,
    /**
     * Digital Interrupt
     */
	DM75xx_PCLK_STOP_DIGITAL_IT,
    /**
     * About Counter
     */
	DM75xx_PCLK_STOP_ACNT,
    /**
     * User Timer/Counter 2 Out
     */
	DM75xx_PCLK_STOP_UTC2,
    /**
     * SyncBus 0
     */
	DM75xx_PCLK_STOP_SBUS0,
    /**
     * SyncBus 1
     */
	DM75xx_PCLK_STOP_SBUS1,
    /**
     * SyncBus 2
     */
	DM75xx_PCLK_STOP_SBUS2,
    /**
     * About Software
     */
	DM75xx_PCLK_STOP_ASOFTWARE,
    /**
     * About External Trigger
     */
	DM75xx_PCLK_STOP_AETRIG,
    /**
     * About Digital Interrupt
     */
	DM75xx_PCLK_STOP_ADIGITAL_IT,
    /**
     * Reserved
     */
	DM75xx_PCLK_STOP_RES,
    /**
     * About User Timer/Counter 2 Out
     */
	DM75xx_PCLK_STOP_AUTC2,
    /**
     * About SyncBus 0
     */
	DM75xx_PCLK_STOP_ASBUS0,
    /**
     * About SyncBus 1
     */
	DM75xx_PCLK_STOP_ASBUS1,
    /**
     * About SyncBus 2
     */
	DM75xx_PCLK_STOP_ASBUS2
};
/**
 * @brief
 *      Pacer Clock Stop Type
 */
typedef enum _dm75xx_pclk_stop dm75xx_pclk_stop_t;

/**
 * @brief
 * Pacer Clock Start
 */
enum _dm75xx_pclk_start {
    /**
     * Software
     */
	DM75xx_PCLK_START_SOFTWARE = 0,
    /**
     * External Trigger
     */
	DM75xx_PCLK_START_ETRIG,
    /**
     * Digital Interrupt
     */
	DM75xx_PCLK_START_DIGITAL_IT,
    /**
     * User Timer/Counter 2 Out
     */
	DM75xx_PCLK_START_UTC2,
    /**
     * SyncBus 0
     */
	DM75xx_PCLK_START_SBUS0,
    /**
     * SyncBus 1
     */
	DM75xx_PCLK_START_SBUS1,
    /**
     * SyncBus 2
     */
	DM75xx_PCLK_START_SBUS2,
    /**
     * Reserved
     */
	DM75xx_PCLK_START_RES,
    /**
     * Delayed Software
     */
	DM75xx_PCLK_START_DSOFTWARE,
    /**
     * Delayed External Trigger
     */
	DM75xx_PCLK_START_DETRIG,
    /**
     * Delayed Digital Interrupt
     */
	DM75xx_PCLK_START_DDIGITAL_IT,
    /**
     * Delayed User Timer/Counter 2 Out
     */
	DM75xx_PCLK_START_DUTC2,
    /**
     * Delayed SyncBus 0
     */
	DM75xx_PCLK_START_DSBUS0,
    /**
     * Delayed SyncBus 1
     */
	DM75xx_PCLK_START_DSBUS1,
    /**
     * Delayed SyncBus 2
     */
	DM75xx_PCLK_START_DSBUS2,
    /**
     * External Trigger Gated
     */
	DM75xx_PCLK_START_ETRIG_GATE
};
/**
 * @brief
 *      Pacer Clock Start Type
 */
typedef enum _dm75xx_pclk_start dm75xx_pclk_start_t;
/**
 * @brief
 * Pacer Clock Select
 */
enum _dm75xx_pclk_select {

    /**
     * External Pacer Clock
     */
	DM75xx_PCLK_EXTERNAL = 0,

    /**
     * Internal Pacer Clock
     */
	DM75xx_PCLK_INTERNAL
};

/**
 * @brief
 *      Pacer Clock Select Type
 */
typedef enum _dm75xx_pclk_select dm75xx_pclk_select_t;
 /**
 * @brief
 * Pacer Clock Frequency Select
 */
enum _dm75xx_pclk_freq {

    /**
     * 8Mhz Frequency
     */
	DM75xx_PCLK_FREQ_8_MHZ = 0,

    /**
     * 20Mhz Frequency
     */
	DM75xx_PCLK_FREQ_20_MHZ
};

/**
 * @brief
 *      Pacer Clock Frequency type
 */

typedef enum _dm75xx_pclk_freq dm75xx_pclk_freq_t;
/**
 * @} DM75xx_Types_PCLK_Enumerations
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
8254 timer/counter enumerations
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Types_UTC_Enumerations DM75xx type timer/counter enumerations
 * @{
 */

/**
 * @brief
 *      8254 timers/counters
 */

enum _dm75xx_utc_timer {

    /**
     * Timer 0 on 8254 chip
     */

	DM75xx_UTC_0 = 0,

    /**
     * Timer 1 on 8254 chip
     */

	DM75xx_UTC_1,

    /**
     * Timer 2 on 8254 chip
     */

	DM75xx_UTC_2
};

/**
 * @brief
 *      8254 timer/counter type
 */

typedef enum _dm75xx_utc_timer dm75xx_utc_timer_t;

/**
 * @brief
 *      8254 timer/counter clock selectors
 */

enum _dm75xx_utc_clk {

    /**
     * 8 MHz clock
     */

	DM75xx_CUTC_8_MHZ = 0,

    /**
     * External Timer Counter Clock 1
     */

	DM75xx_CUTC_EXT_TC_CLOCK_1 = 1,

    /**
     * External Timer Counter Clock 2
     */

	DM75xx_CUTC_EXT_TC_CLOCK_2 = 2,

    /**
     * External Pacer Clock
     */

	DM75xx_CUTC_EXT_PCLK = 3,

    /**
     * User Timer/Counter 0 Out
     */

	DM75xx_CUTC_UTC_0_OUT = 4,

    /**
     * User Timer/Counter 1 Out
     */

	DM75xx_CUTC_UTC_1_OUT = 4,

    /**
     * High Speed Digital Input Sample Signal
     */

	DM75xx_CUTC_HSDIN_SIGNAL = 5
};

/**
 * @brief
 *      8254 timer/counter clock selector type
 */

typedef enum _dm75xx_utc_clk dm75xx_utc_clk_t;

/**
 * @brief
 *      8254 timer/counter gate selectors
 */

enum _dm75xx_utc_gate {

    /**
     * Logic 0
     */

	DM75xx_GUTC_NOT_GATED = 0,

    /**
     * Logic 1
     */

	DM75xx_GUTC_GATED = 1,

    /**
     * 8254 timer/counter
     */

	DM75xx_GUTC_EXT_TC_CLK_1 = 2,

    /**
     * 8254 timer/counter
     */

	DM75xx_GUTC_EXT_TC_CLK_2 = 3,

    /**
     * 8254 timer/counter
     */

	DM75xx_GUTC_UTC_0_OUT = 4,

    /**
     * 8254 timer/counter
     */

	DM75xx_GUTC_UTC_1_OUT = 4,

};

/**
 * @brief
 *      8254 timer/counter gate selector type
 */

typedef enum _dm75xx_utc_gate dm75xx_utc_gate;

/**
 * @brief
 *      8254 timer/counter waveform mode selectors
 */

enum _dm75xx_utc_mode {

    /**
     * Event counter
     */

	DM75xx_UTC_MODE_EVENT_COUNTER = 0,

    /**
     * Programmable one shot
     */

	DM75xx_UTC_MODE_PROG_ONE_SHOT,

    /**
     * Rate generator
     */

	DM75xx_UTC_MODE_RATE_GENERATOR,

    /**
     * Square wave generator
     */

	DM75xx_UTC_MODE_SQUARE_WAVE,

    /**
     * Software triggered strobe
     */

	DM75xx_UTC_MODE_SOFTWARE_STROBE,

    /**
     * Hardware triggered strobe
     */

	DM75xx_UTC_MODE_HARDWARE_STROBE
};

/**
 * @brief
 *      8254 timer/counter waverform mode selector type
 */

typedef enum _dm75xx_utc_mode dm75xx_utc_mode;

/**
 * @} DM75xx_Types_UTC_Enumerations
 */

/**
 * @} DM75xx_Types_Enumerations
 */

/*=============================================================================
Structures
 =============================================================================*/

/**
 * @defgroup DM75xx_Types_Structures DM75xx type definition structures
 * @{
 */
/**
 * @brief
 *      FIFO status
 */
enum _dm75xx_fifo_status {
    /**
     * DAC1 FIFO Not Empty
     */
	DM75xx_FIFO_DAC1_NOT_EMPTY = 0x0001,
    /**
     * DAC1 FIFO Half Empty
     */
	DM75xx_FIFO_DAC1_HALF_EMPTY = 0x0002,
    /**
     * DAC1 FIFO Not Full
     */
	DM75xx_FIFO_DAC1_NOT_FULL = 0x0004,
    /**
     * DAC2 FIFO Not Empty
     */
	DM75xx_FIFO_DAC2_NOT_EMPTY = 0x0010,
    /**
     * DAC2 FIFO Half Empty
     */
	DM75xx_FIFO_DAC2_HALF_EMPTY = 0x0020,
    /**
     * DAC2 FIFO Not Full
     */
	DM75xx_FIFO_DAC2_NOT_FULL = 0x0040,
    /**
     * ADC FIFO Not Empty
     */
	DM75xx_FIFO_ADC_NOT_EMPTY = 0x0100,
    /**
     * ADC FIFO Half Empty
     */
	DM75xx_FIFO_ADC_HALF_EMPTY = 0x0200,
    /**
     * ADC FIFO Not Full
     */
	DM75xx_FIFO_ADC_NOT_FULL = 0x0400,
    /**
     * HSDIN FIFO Not Empty
     */
	DM75xx_FIFO_HSDIN_NOT_EMPTY = 0x1000,
    /**
     * HSDIN FIFO Half Empty
     */
	DM75xx_FIFO_HSDIN_HALF_EMPTY = 0x2000,
    /**
     * HSDIN FIFO Not Full
     */
	DM75xx_FIFO_HSDIN_NOT_FULL = 0x4000
};
/**
 * @brief
 *      FIFO Status Type
 */
typedef enum _dm75xx_fifo_status dm75xx_fifo_status_t;
/**
 * @brief
 *      Interrupts status
 */
struct _dm75xx_int_status {

    /**
     * Number of interrupts remaining in the interrupt status queue
     */

	int int_remaining;

    /**
     * Number of interrupts missed
     */

	unsigned int int_missed;

    /**
     * Interrupt Status
     */

	uint32_t status;

};
/**
 * @brief
 *      Interrupt status type
 */
typedef struct _dm75xx_int_status dm75xx_int_status_t;
/**
 * @brief
 *      Channel gain table entry
 */
struct _dm75xx_cgt_entry {
    /**
     * Analog input channel
     */
	uint8_t channel:4;
    /**
     * Gain
     */
	uint8_t gain:3;
    /**
     * AGND/AINSENSE
     */
	uint8_t nrse:1;
    /**
     * Output Range
     */
	uint8_t range:2;
    /**
     * Single/Differential
     */
	uint8_t ground:1;
    /**
     * Pause
     */
	uint8_t pause:1;
    /**
     * DAC1 Update
     */
	uint8_t dac1:1;
    /**
     * DAC2 Update
     */
	uint8_t dac2:1;
    /**
     * Skip
     */
	uint8_t skip:1;
    /**
     * Reserved
     */
	uint8_t reserved:1;
};

/**
 * @brief
 *      Channel gain table entry type
 */
typedef struct _dm75xx_cgt_entry dm75xx_cgt_entry_t;

/**
 * @brief
 *      PCI region access request descriptor.  This structure holds information
 *      about a request to read data from or write data to one of a device's
 *      PCI regions.
 */

struct dm75xx_pci_access_request {

    /**
     * Size of access in bits
     */

	dm75xx_pci_region_access_size_t size;

    /**
     * The PCI region to access
     */

	dm75xx_pci_region_num_t region;

    /**
     * Offset within region to access
     */

	uint16_t offset;

    /**
     * Data to write or the data read
     */

	union {

    /**
     * 8-bit value
     */

		uint8_t data8;

    /**
     * 16-bit value
     */

		uint16_t data16;

    /**
     * 32-bit value
     */

		uint32_t data32;
	} data;
};

/**
 * @brief
 *      PCI region access request descriptor type
 */

typedef struct dm75xx_pci_access_request dm75xx_pci_access_request_t;

/**
 * @} DM75xx_Types_Structures
 */

/**
 * @} DM75xx_Types_Header
 */

#endif /* __dm75xx_types_h__ */
