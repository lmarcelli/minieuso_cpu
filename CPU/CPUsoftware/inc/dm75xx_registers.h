/**
    @file

    @brief
        Register definitions for DM75xx devices

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
	
	$Id: dm75xx_registers.h 56268 2011-10-25 18:55:35Z rgroner $
*/

#ifndef __dm75xx_registers_h__
#define __dm75xx_registers_h__

/*##############################################################################
 #
 #							PLX Registers
 #
 #############################################################################*/
 /**
 * @brief
 *	Interupt Control/Status
 */
#define DM75xx_PLX_ITCSR		0x68
/**
 * @brief
 *	DMA Channel 0 Mode
 */
#define DM75xx_PLX_DMA_MODE0	0x80
/**
 * @brief
 *	DMA Channel 0 PCI Address
 */
#define DM75xx_PLX_DMA_PADR0	0x84
/**
 * @brief
 *	DMA Channel 0 Local Address
 */
#define DM75xx_PLX_DMA_LADR0	0x88
/**
 * @brief
 *	DMA Channel 0 Transfer Size (Bytes)
 */
#define DM75xx_PLX_DMA_SIZE0	0x8C
/**
 * @brief
 *	DMA Channel 0 Descriptor Pointer
 */
#define DM75xx_PLX_DMA_DPR0		0x90
/**
 * @brief
 *	DMA Channel 1 Mode
 */
#define DM75xx_PLX_DMA_MODE1	0x94
/**
 * @brief
 *	DMA Channel 1 PCI Address
 */
#define DM75xx_PLX_DMA_PADR1	0x98
/**
 * @brief
 *	DMA Channel 1 Local Address
 */
#define DM75xx_PLX_DMA_LADR1	0x9C
/**
 * @brief
 *	DMA Channel 1 Transfer Size (Bytes)
 */
#define DM75xx_PLX_DMA_SIZE1	0xA0
/**
 * @brief
 *	DMA Channel 1 Descriptor Pointer
 */
#define DM75xx_PLX_DMA_DPR1		0xA4
/**
 * @brief
 *	DMA Command/Status channel 0
 */
#define DM75xx_PLX_DMA_CSR0		0xA8
/**
 * @brief
 *  DMA Command/Status channel 1
 */
#define DM75xx_PLX_DMA_CSR1     0xA9
/**
 * @brief
 *	DMA Arbitration
 */
#define DM75xx_PLX_DMA_ARB		0xAC
/**
 * @brief
 *	DMA Threshold
 */
#define DM75xx_PLX_DMA_THR		0xB0
/**
 * @brief
 * PLX LAS1 SRAM access space
 */
#define DM75xx_PLX_SRAM         0xF4
/*##############################################################################
 #
 #							LAS 0 Runtime Area
 #
 #############################################################################*/
/**
 * @brief
 * Read Master/Target Only mode and Firmware version.
 */
#define DM75xx_LAS0_MT_MODE			0x00
/**
 * @brief
 * Spare 0x04
 */
#define DM75xx_LAS0_SPARE_04		0x04
/**
 * @brief
 * Read/Write user inputs.
 */
#define DM75xx_LAS0_USER_IO			0x08
/**
 * @brief
 * Start/Stop software clock
 */
#define DM75xx_LAS0_DAC_CLK_ST		0x0C
/**
 * @brief
 * Read FIFO Status
 */
#define DM75xx_LAS0_FIFO_STATUS		0x10
/**
 * @brief
 * Software DAC1 Update
 */
#define DM75xx_LAS0_DAC1			0x14
/**
 * @brief
 * Software DAC2 Update
 */
#define DM75xx_LAS0_DAC2			0x18
/**
 * @brief
 * Spare 0x1C
 */
#define DM75xx_LAS0_SPARE_1C		0x1C
/**
 * @brief
 * Spare 0x20
 */
#define DM75xx_LAS0_SPARE_20		0x20
/**
 * @brief
 * Software simultaneous DAC1 and DAC2 Update
 */
#define DM75xx_LAS0_DAC				0x24
/**
 * @brief
 * Start/Stop software Pacer
 */
#define DM75xx_LAS0_PACER			0x28
/**
 * @brief
 * Read: Read Timer Counter Status
 * Write: Software HSDIN Sample Command
 */
#define DM75xx_LAS0_TIMER			0x2C
/**
 * @brief
 * Read: Read interrupt status
 * Write: Write IT enable mask
 */
#define DM75xx_LAS0_IT				0x30
/**
 * @brief
 * Read: Clear ITs via mask
 * Write: Set IT clear mask
 */
#define DM75xx_LAS0_CLEAR_IT		0x34
/**
 * @brief
 * Read: Read IT overrrun
 * Write: Clear IT overrun
 */
#define DM75xx_LAS0_IT_OVERRUN		0x38
/**
 * @brief
 * Spare 0x3C
 */
#define DM75xx_LAS0_SPARE_3C		0x3C
/**
 * @brief
 * Read/Write Pacer Clock
 */
#define DM75xx_LAS0_PCLK			0x40
/**
 * @brief
 * Read/Write Burst Clock
 */
#define DM75xx_LAS0_BCLK			0x44
/**
 * @brief
 * Read/Write ADC sample counter
 */
#define DM75xx_LAS0_ADC_SCNT		0x48
/**
 * @brief
 * Read/Write DAC1 update counter
 */
#define DM75xx_LAS0_DAC1_UCNT		0x4C
/**
 * @brief
 * Read/Write DAC2 update counter
 */
#define DM75xx_LAS0_DAC2_UCNT		0x50
/**
 * @brief
 * Read/Write delay counter
 */
#define DM75xx_LAS0_DCNT			0x54
/**
 * @brief
 * Read/Write about counter
 */
#define DM75xx_LAS0_ACNT			0x58
/**
 * @brief
 * Read/Write DAC clock
 */
#define DM75xx_LAS0_DAC_CLK			0x5C
/**
 * @brief
 * Read/Write UTC0 value
 */
#define DM75xx_LAS0_UTC0			0x60
/**
 * @brief
 * Read/Write UTC1 value
 */
#define DM75xx_LAS0_UTC1			0x64
/**
 * @brief
 * Read/Write UTC2 value
 */
#define DM75xx_LAS0_UTC2			0x68
/**
 * @brief
 * UTC Control
 */
#define DM75xx_LAS0_UTC_CTRL		0x6C
/**
 * @brief
 * Read/Program digital input port 0
 */
#define DM75xx_LAS0_DIO0			0x70
/**
 * @brief
 * Read/Program digital input port 1
 */
#define DM75xx_LAS0_DIO1			0x74
/**
 * @brief
 * Clear digital IRQ status, read/program 
 * port 0 direction, mask, or compare register
 */
#define DM75xx_LAS0_DIO_CTRL		0x78
/**
 * @brief
 * Read DIO Status or Program digital control/interrupts
 */
#define DM75xx_LAS0_DIO_STATUS		0x7C
/*%%%%%%%%%%%%%%%%%%%%%%
 * DM75xx Specific Registers
 %%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * Read: DSP Command register to be written from
 * the Host side and read from the DSP
 * Write: DSP status written to by the DSP and 
 * read from the Host side
 */
#define DM75xx_LAS0_DSP				0xB0
/**
 * @brief
 * Read/Write analog connection DIO mask
 */
#define DM75xx_LAS0_ALGDIO_MASK	    0xE0
/**
 * @brief
 * Read/Write analog connection DIO data
 */
#define DM75xx_LAS0_ALGDIO_DATA	    0xE4
/**
 * @brief
 * read/Write analog connection DIO direction
 */
#define DM75xx_LAS0_ALGDIO_DIR  	0xE8
/**
 * @brief
 * Read analog connection DIO IRQ status
 */
#define DM75xx_LAS0_ALGDIO_IRQ		0xEC
/**
 * @brief
 * I2C Bus Address
 */
#define DM75xx_LAS0_I2C_ADDR        0xC0
/**
 * @brief
 * I2C Bus Pointer
 */
#define DM75xx_LAS0_I2C_PTR         0xC4
/**
 * @brief
 * I2C Bus Data
 */
#define DM75xx_LAS0_I2C_DATA        0xC8
/**
 * @brief
 * I2C Bus Go
 */
#define DM75xx_LAS0_I2C_GO          0xCC
/**
 * @brief
 * I2C Bus Read
 */
#define DM75xx_LAS0_I2C_READ        0xD0
/*##############################################################################
 #
 #						LAS 0 Setup Area
 #
 #############################################################################*/
/**
 * @brief
 * Software board reset
 */
#define DM75xx_LAS0_BOARD_RESET	    0x100
/**
 * @brief
 * DMA Channel 0 Source
 */
#define DM75xx_LAS0_DMA0_SRC		0x104
/**
 * @brief
 * DMA Channel 1 Source
 */
#define DM75xx_LAS0_DMA1_SRC		0x108
/**
 * @brief
 * Reset DMA Channel 0 Request Machine
 */
#define DM75xx_LAS0_DMA_RSTRQST0	0x1CC
/**
 * @brief
 * Reset DMA Channel 1 Request Machine
 */
#define DM75xx_LAS0_DMA_RSTRQST1	0x1D0
/**
 * @brief
 * Select ADC Conversion Signal
 */
#define	DM75xx_LAS0_ADC_CONV	0x10C
/**
 * @brief
 * Select Burst Clock Start Trigger
 */
#define DM75xx_LAS0_BURST_START	0x110
/**
 * @brief
 * Select Pacer Clock Start Trigger
 */
#define DM75xx_LAS0_PACER_START	0x114
/**
 * @brief
 * Select Pacer Clock Stop Trigger
 */
#define DM75xx_LAS0_PACER_STOP		0x118
/**
 * @brief
 * About Counter Stop Enable
 */
#define DM75xx_LAS0_ACNT_ENABLE	0x11C
/**
 * @brief
 * Pacer Clock Start Trigger Mode
 */
#define DM75xx_LAS0_PACER_MODE		0x120
/**
 * @brief
 * Select HighSpeed Digital Sampling Signal
 */
#define DM75xx_LAS0_HSDIN_START	0x124
/**
 * @brief
 * Clear HighSpeed Digital FIFO
 */
#define DM75xx_LAS0_HSDIN_FIFO_CLR	0x128
/**
 * @brief
 * Clear ADC FIFO
 */
#define DM75xx_LAS0_ADC_FIFO_CLR	0x12C
/**
 * @brief
 * Write CGT Multi-Channel
 */
#define DM75xx_LAS0_CGT_WRITE		0x130
/**
 * @brief
 * Write CGT Latch Single-Channel
 */
#define DM75xx_LAS0_CGT_LATCH		0x134
/**
 * @brief
 * Write Digital Table
 */
#define DM75xx_LAS0_DT_WRITE		0x138
/**
 * @brief
 * Enable CGT
 */
#define DM75xx_LAS0_CGT_ENABLE		0x13C
/**
 * @brief
 * Enable Digital Table
 */
#define DM75xx_LAS0_DT_ENABLE		0x140
/**
 * @brief
 * Table Pause Enable
 */
#define DM75xx_LAS0_PAUSE_TABLE	0x144
/**
 * @brief
 * Reset CGT
 */
#define DM75xx_LAS0_CGT_RESET		0x148
/**
 * @brief
 * Clear CGT
 */
#define DM75xx_LAS0_CGT_CLEAR		0x14C
/**
 * @brief
 * Select DAC1 Output Range
 */
#define DM75xx_LAS0_DAC1_RANGE		0x150
/**
 * @brief
 * Select DAC1 Update Source
 */
#define DM75xx_LAS0_DAC1_SRC		0x154
/**
 * @brief
 * Select DAC1 Cycle Mode
 */
#define DM75xx_LAS0_DAC1_CYCLE		0x158
/**
 * @brief
 * Reset DAC1 FIFO
 */
#define DM75xx_LAS0_DAC1_RESET		0x15C
/**
 * @brief
 * Clear DAC1 FIFO
 */
#define DM75xx_LAS0_DAC1_CLEAR		0x160
/**
 * @brief
 * Select DAC2 Output Range
 */
#define DM75xx_LAS0_DAC2_RANGE		0x164
/**
 * @brief
 * Select DAC2 Update Source
 */
#define DM75xx_LAS0_DAC2_SRC		0x168
/**
 * @brief
 * Select DAC2 Cycle Mode
 */
#define DM75xx_LAS0_DAC2_CYCLE		0x16C
/**
 * @brief
 * Reset DAC2 FIFO
 */
#define DM75xx_LAS0_DAC2_RESET		0x170
/**
 * @brief
 * Clear DAC2 FIFO
 */
#define DM75xx_LAS0_DAC2_CLEAR		0x174
/**
 * @brief
 * Select SyncBus 0 Source
 */
#define DM75xx_LAS0_SBUS0_SOURCE	0x184
/**
 * @brief
 * Syncbus 0 Enable
 */
#define DM75xx_LAS0_SBUS0_ENABLE	0x188
/**
 * @brief
 * Select SyncBus 1 Source
 */
#define DM75xx_LAS0_SBUS1_SOURCE	0x18C
/**
 * @brief
 * SyncBus 1 Enable
 */
#define DM75xx_LAS0_SBUS1_ENABLE	0x190
/**
 * @brief
 * Select SyncBus 2 Source
 */
#define DM75xx_LAS0_SBUS2_SOURCE	0x198
/**
 * @brief
 * SyncBus 2 Enable
 */
#define DM75xx_LAS0_SBUS2_ENABLE	0x19C
/**
 * @brief
 * Select External Trigger Polarity
 */
#define DM75xx_LAS0_ETRG_POLARITY	0x1A4
/**
 * @brief
 * Select External Interrupt Polarity
 */
#define DM75xx_LAS0_EINT_POLARITY 	0x1A8
/**
 * @brief
 * Select UTC0 Clock
 */
#define DM75xx_LAS0_UTC0_CLOCK		0x1AC
/**
 * @brief
 * Select UTC0 Gate
 */
#define DM75xx_LAS0_UTC0_GATE		0x1B0
/**
 * @brief
 * Select UTC1 Clock
 */
#define DM75xx_LAS0_UTC1_CLOCK		0x1B4
/**
 * @brief
 * Select UTC1 Gate
 */
#define DM75xx_LAS0_UTC1_GATE		0x1B8
/**
 * @brief
 * Select UTC2 Clock
 */
#define DM75xx_LAS0_UTC2_CLOCK		0x1BC
/**
 * @brief
 * Select UTC2 Gate
 */
#define DM75xx_LAS0_UTC2_GATE		0x1C0
/**
 * @brief
 * Select User Output Signal 0
 */
#define DM75xx_LAS0_UIO0_SELECT	    0x1C4
/**
 * @brief
 * Select User Output Signal 1
 */
#define DM75xx_LAS0_UIO1_SELECT	    0x1C8
/**
 * @brief
 * Select ADC Sample Counter Source
 */
#define DM75xx_LAS0_ADC_SCNT_SRC	0x178
/**
 * @brief
 * Select Pacer Clock
 */
#define DM75xx_LAS0_PACER_SELECT	0x180
/**
 * @brief
 * Select DAC Clock Start
 */
#define DM75xx_LAS0_DAC_CLK_START	0x1D4
/**
 * @brief
 * Select DAC Clock Stop
 */
#define DM75xx_LAS0_DAC_CLK_STOP	0x1D8
/**
 * @brief
 * Select Pacer Clock Frequency
 */
#define DM75xx_LAS0_PCLK_FREQ		0x1DC
/**
 * @brief
 * Select Burst Clock Frequency
 */
#define DM75xx_LAS0_BCLK_FREQ		0x1E0
/**
 * @brief
 * Select DAC Clock Source
 */
#define DM75xx_LAS0_DAC_CLK_SOURCE	0x1E4
/**
 * @brief
 * Select DAC Clock Mode
 */
#define DM75xx_LAS0_DAC_CLK_MODE	0x1E8
/**
 * @brief
 * ADC FIFO Data to DSP Enable
 */
#define DM75xx_LAS0_MCBSP_AD_CTRL	0x1EC
/**
 * @brief
 * DAC FIFO Data from DSP Enable
 */
#define DM75xx_LAS0_MCBSP_DA_CTRL 	0x1F0
/**
 * @brief
 * Select FIFO Addressing Mode
 */
#define DM75xx_LAS0_FIFO_ADR_MODE	0x1F4

/*##############################################################################
 #
 #								LAS1
 #
 #############################################################################*/
/**
 * @brief
 * Read ADC FIFO 
 */
#define DM75xx_LAS1_ADC_FIFO		0x00
/**
 * @brief
 * Read HighSpeed Digital FIFO
 */
#define DM75xx_LAS1_HSDIN_FIFO		0x04
/**
 * @brief
 * Write DAC1 FIFO
 */
#define DM75xx_LAS1_DAC1_FIFO 		0x08
/**
 * @brief
 * Write DAC2 FIFO
 */
#define DM75xx_LAS1_DAC2_FIFO		0x0C

/*##############################################################################
 #
 #								Other Register Constants
 #
 #############################################################################*/
/**
 * @brief
 * Local Address for ADC FIFO
 */
#define DMALADDR_ADC		0x40000000
/**
 * @brief
 * Local Address for HSDIN FIFO
 */
#define DMALADDR_HSDIN		0x40000004
/**
 * @brief
 * Local Address for DAC1 FIFO
 */
#define DMALADDR_DAC1		0x40000008
/**
 * @brief
 * Local Address for DAC2 FIFO
 */
#define DMALADDR_DAC2		0x4000000C
/**
 * @brief
 * Generic Disable Logic 0x00
 */
#define DISABLED				0x00
/**
 * @brief
 * Generic Enable Logic 0x01
 */
#define ENABLED					0x01
/**
 * @brief
 * Dummy Value
 */
#define NO_ARG 					0x00
/*
 * "Helper" constants for register masking and bit manipulation
 */
/**
 * @brief
 * Bit 0
 */
#define BIT_00			0x00000001
/**
 * @brief
 * Bit 1
 */
#define BIT_01			0x00000002
/**
 * @brief
 * Bit 2
 */
#define BIT_02			0x00000004
/**
 * @brief
 * Bit 3
 */
#define BIT_03			0x00000008
/**
 * @brief
 * Bit 4
 */
#define BIT_04			0x00000010
/**
 * @brief
 * Bit 5
 */
#define BIT_05			0x00000020
/**
 * @brief
 * Bit 6
 */
#define BIT_06			0x00000040
/**
 * @brief
 * Bit 7
 */
#define BIT_07			0x00000080
/**
 * @brief
 * Bit 8
 */
#define BIT_08			0x00000100
/**
 * @brief
 * Bit 9
 */
#define BIT_09			0x00000200
/**
 * @brief
 * Bit 10
 */
#define BIT_10			0x00000400
/**
 * @brief
 * Bit 11
 */
#define BIT_11			0x00000800
/**
 * @brief
 * Bit 12
 */
#define BIT_12			0x00001000
/**
 * @brief
 * Bit 13
 */
#define BIT_13			0x00002000
/**
 * @brief
 * Bit 14
 */
#define BIT_14			0x00004000
/**
 * @brief
 * Bit 15
 */
#define BIT_15			0x00008000
/**
 * @brief
 * Bit 16
 */
#define BIT_16			0x00010000
/**
 * @brief
 * Bit 17
 */
#define BIT_17			0x00020000
/**
 * @brief
 * Bit 18
 */
#define BIT_18			0x00040000
/**
 * @brief
 * Bit 19
 */
#define BIT_19			0x00080000
/**
 * @brief
 * Bit 20
 */
#define BIT_20			0x00100000
/**
 * @brief
 * Bit 21
 */
#define BIT_21			0x00200000
/**
 * @brief
 * Bit 22
 */
#define BIT_22			0x00400000
/**
 * @brief
 * Bit 23
 */
#define BIT_23			0x00800000
/**
 * @brief
 * Bit 24
 */
#define BIT_24			0x01000000
/**
 * @brief
 * Bit 25
 */
#define BIT_25			0x02000000
/**
 * @brief
 * Bit 26
 */
#define BIT_26			0x04000000
/**
 * @brief
 * Bit 27
 */
#define BIT_27			0x08000000
/**
 * @brief
 * Bit 28
 */
#define BIT_28			0x10000000
/**
 * @brief
 * Bit 29
 */
#define BIT_29			0x20000000
/**
 * @brief
 * Bit 30
 */
#define BIT_30			0x40000000
/**
 * @brief
 * Bit 31
 */
#define BIT_31			0x80000000
/*##############################################################################
 #
 #								    PLX DMA Mode
 #
 #############################################################################*/
/**
 * @brief
 * PLX DMA Local Bus Width
 */
#define PLX_DMA_WIDTH_16        BIT_00
/**
 * @brief
 * PLX DMA Ready
 */
#define PLX_DMA_READY           BIT_06
/**
 * @brief
 * PLX DMA Local Burst
 */
#define PLX_DMA_LOCAL_BURST     BIT_08
/**
 * @brief
 * PLX DMA Chaining 
 */
#define PLX_DMA_CHAINING        BIT_09
/**
 * @brief
 * PLX DMA Done Interrupt
 */
#define PLX_DMA_DONE_IT         BIT_10
/**
 * @brief
 * PLX DMA Local Addressing Mode
 */
#define PLX_DMA_LA_MODE         BIT_11
/**
 * @brief
 * PLX DMA Demand Mode
 */
#define PLX_DMA_DEMAND_MODE     BIT_12
/**
 * @brief
 * PLX DMA PCI Interrupt Enable
 */
#define PLX_DMA_PCI_IT          BIT_17
/**
 * @brief
 * Demand Mode DMA Configuration
 */
#define PLX_DMA_CONFIG  PLX_DMA_WIDTH_16|PLX_DMA_READY|PLX_DMA_LOCAL_BURST|PLX_DMA_DONE_IT|PLX_DMA_LA_MODE|PLX_DMA_PCI_IT
#endif /* __dm75xx_registers_h__ */
