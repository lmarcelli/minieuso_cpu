/**
    @file

    @brief
        DM75xx user library definitions

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

	$Id: dm75xx_library.h 99065 2016-04-26 18:03:23Z rgroner $
*/

#ifndef __dm75xx_library_h__
#define __dm75xx_library_h__

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>

#include "dm75xx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup DM75xx_Library_Header DM75xx user library header file
 * @{
 */

/*=============================================================================
Macros
 =============================================================================*/

/**
 * @defgroup DM75xx_Library_Macros DM75xx user library macros
 * @{
 */
/**
 * @brief
 *      Determine whether or not the specified interrupt source has occurred
 *      in your the user space ISR.
 *
 * @param
 *      status
 *
 *      Interrupt status to examine.
 *
 * @param
 *      source
 *
 *      Interrupt source to determine state of.
 *
 * @retval
 *      0x00
 *
 *      The specified interrupt source is not pending.
 *
 * @retval
 *      0xFF
 *
 *      The specified interrupt source is pending.
 */

#define DM75xx_INTERRUPT_ACTIVE(status, source) \
    (((status) & (source)) ? 0xFF : 0x00)

/**
 * @brief
 *      This macro will return the sample portion of raw analog data
 *
 * @param
 *      data
 *
 *      The raw analog data
 *
 * @return
 *      The 12 bit signed analog sample
 *
 * @note
 *      The value returned by this macro should be stored in an int16_t
 */

#define DM75xx_ADC_ANALOG_DATA(data) (((int16_t) (data)) >> 3)

/**
 * @brief
 *      This macro will turn the data marker portion of raw analog data
 *
 * @param
 *      data
 *
 *      The raw analog data
 *
 * @return
 *      The 3 marker bits
 *
 * @note
 *      The value returned by this macro should be stored in a uint8_t
 */

#define DM75xx_ADC_MARKERS(data) ((data) & 0x07)

/**
 * @brief
 *      This macro will assemble a package to be sent to the Digital to Analog
 *      FIFO
 *
 * @param
 *      data
 *
 *      The 12 bit signed data to write to the Digital to Analog channel
 *
 * @param
 *      mcbsp_bit
 *
 *      A bit designating which Digital to Analog channel will receive McBSP
 *      data
 *
 * @param
 *      data_markers
 *
 *      The 2 data marker bits
 *
 * @return
 *      The combined DAC Data in an int16_t
 */

#define DM75xx_DAC_PACK_DATA(data, mcbsp_bit, data_markers) \
        ((int16_t)((int16_t)(data) << 3) | \
        (mcbsp_bit & 0x0004) | \
        (data_markers & 0x0003))

/**
 * @} DM75xx_Library_Macros
 */

/*=============================================================================
Type definitions
 =============================================================================*/

/**
 * @defgroup DM75xx_Library_Types DM75xx user library type definitions
 * @{
 */

/**
 * @brief
 *      DM75xx user library error code type
 */

	typedef int DM75xx_Error;

/**
 * @} DM75xx_Library_Types
 */

/*=============================================================================
Structures
 =============================================================================*/

/**
 * @defgroup DM75xx_Library_Structures DM75xx user library structures
 * @{
 */

/**
 * @brief
 *      DM75xx board descriptor.  This structure holds information about a
 *      device needed by the library.
 */

	struct DM75xx_Board_Descriptor {

    /**
     * File descriptor for device returned from open()
     */

		int file_descriptor;
	/**
	 * The currently installed user-space ISR for this device.
	 */
		void (*isr) (unsigned int status);
	/**
	 * The parent ID of the thread watching for interrupts.
	 */
		pthread_t pid;
    /**
     * Status of thread after execution
     */
		int thread_status;
    /**
     * Pointer to Kernel Space Buffers
     */
		uint16_t *k_buf[2];
    /**
     * Pointer to User Space DMA Buffers
     */
		uint16_t *u_buf[2];
    /**
     * Kernel Buffer Sizes
     */
		unsigned long k_buf_siz[2];
    /**
     * User Space Buffer Sizes
     */
		unsigned long u_buf_siz[2];
	};

/**
 * DM75xx board descriptor type
 */

	typedef struct DM75xx_Board_Descriptor DM75xx_Board_Descriptor;

/**
 * @} DM75xx_Library_Structures
 */

/*=============================================================================
Public library functions
 =============================================================================*/

/**
 * @defgroup DM75xx_Library_Functions DM75xx user library functions
 * @{
 */

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Board control block functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @defgroup DM75xx_Library_BrdCtl_Functions DM75xx user library board control functions
 * @{
 */

/**
*******************************************************************************
@brief
    Determine whether or not a device is PCI master capable.

@param
    handle

    Address of device's library board descriptor.

@param
    pci_master

    Address where PCI master capable flag should be stored.  Zero will be stored
    here if the device is not PCI master capable.  A non-zero value will be
    stored here if the device is PCI master capable.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */

	DM75xx_Error DM75xx_Board_PCI_Master(DM75xx_Board_Descriptor * handle,
					     uint8_t * pci_master);

/**
*******************************************************************************
@brief
    Reset a DM75xx device.

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
    This function does not reset the PLX chip or 8254 chips.
 *******************************************************************************
 */

	DM75xx_Error DM75xx_Board_Reset(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Clear Interrupts via Mask.

@param
    handle

    Address of device's library board descriptor.

@param
	mask

	Mask of the interrupt bits to clear.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_Clear_ITMask(DM75xx_Board_Descriptor * handle,
				uint16_t mask);
/**
*******************************************************************************
@brief
    Clear Interrupt Overrun Register.

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_Clear_IT_Overrun(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Tests the return status of a library function, and if it's an error we
    clean up the board and exit.

@param
    handle

    Address of the device's library board descriptor.

@param
    status

    The return status we are testing.

@param
    str

    The string to print in the case of failure.

 *******************************************************************************
 */
	void DM75xx_Exit_On_Error(DM75xx_Board_Descriptor * handle,
				  DM75xx_Error status, char *str);
/**
*******************************************************************************
@brief
    Initialize a Board.  This function performs the following to attempt to
    get the device into a known state:

    Board Reset
    Clear A/D FIFO
    Clear D/A 1 FIFO
    Clear D/A 2 FIFO
    Clear High Speed Digital FIFO
    Clear Channel Gain Table
    Reset Digital I/O Chip
    Clear Digital Interrupts
    Clear Interrupts
    Clear Interrupt Overrun register

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_Board_Init(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Enable one or more DM75xx interrupt source(s).

@param
    handle

    Address of device's library board descriptor.

@param
	int_source

	Interrupt source to enable.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_Interrupt_Enable(DM75xx_Board_Descriptor * handle,
					     dm75xx_int_source_t int_source);
/**
*******************************************************************************
@brief
    Disable one or more DM75xx interrupt source(s).

@param
    handle

    Address of device's library board descriptor.

@param
	int_source

	Interrupt source to disable.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_Interrupt_Disable(DM75xx_Board_Descriptor * handle,
					      dm75xx_int_source_t int_source);
/**
*******************************************************************************
@brief
    Returns the value of current active/enabled interrupts on the device.

@param
    handle

    Address of device's library board descriptor.

@param
	int_source

	Address of variable to store the returned interrupt enable status

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_Interrupt_Check(DM75xx_Board_Descriptor * handle,
					    dm75xx_int_source_t * int_source);
/**
 * @} DM75xx_Library_BrdCtl_Functions
 */

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
DMA functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @defgroup DM75xx_Library_DMA_Functions DM75xx user library DMA functions
 * @{
 */
/**
*******************************************************************************
@brief
    Copy the User Space buffers data incrementally into our Kernel Space buffer

@param
    handle

    Address of device's library board descriptor.

@param
    channel

    The DMA Channel on which to perform the operation.

@param
    num_ints

    Number of DMA interrupt received, this helps us keep track of our place in
    the buffer

@retval
    0

    Success

@retval
    -1

@note
    Use this function if you would like the library to handle your DMA buffer
    writes.  This function effectively copies to the buffer mapped to the
    kernel dma buffer by DMA_Initialize() into the buffer allocated by
    DMA_Buffer_Create().

    This function must be used in conjuction with DMA_Buffer_Create() and
    DMA_Buffer_Free().

    The user is more than welcome to manage the buffers via memcpy().

 ******************************************************************************
 */
	DM75xx_Error DM75xx_DMA_Buffer_Write(DM75xx_Board_Descriptor * handle,
					     dm75xx_dma_channel_t channel,
					     unsigned long num_ints);
/**
*******************************************************************************
@brief
    Copy the Kernel Space buffers data incrementally into our User Space buffer.

@param
    handle

    Address of device's library board descriptor.

@param
    channel

    The DMA Channel on which to perform the operation.

@param
    num_ints

    Number of DMA interrupt received, this helps us keep track of our place in
    the buffer.

@retval
    0

    Success

@retval
    -1

    Failure

@note
    Use this function if you would like the library to handle your DMA buffer
    reads.  This function effectively copies from the buffer mapped to the
    kernel dma buffer by DMA_Initialize() into the buffer allocated by
    DMA_Buffer_Create().

    This function must be used in conjuction with DMA_Buffer_Create() and
    DMA_Buffer_Free().

    The user is more than welcome to manage the buffers via memcpy().

 ******************************************************************************
 */
	DM75xx_Error DM75xx_DMA_Buffer_Read(DM75xx_Board_Descriptor * handle,
					    dm75xx_dma_channel_t channel,
					    unsigned long num_ints);
/**
*******************************************************************************
@brief
    Create a buffer in which the user should place data from the device's DMA
    buffers.

@param
    handle

    Address of device's library board descriptor.

@param
    buffer

    Address of the pointer which create for the user.

@param
    channel

    The DMA channel for which to create a user-space buffer.

@param
    samples

    The size of the buffer required in samples.

@retval
    0

    Success.

@retval
    -1

    Failure.

@note
    This function MUST be called if you are planning on use the
    DMA_Buffer_Read() or DMA_Buffer_Write() function calls to manage your DMA
    data.

 ******************************************************************************
 */
	DM75xx_Error DM75xx_DMA_Buffer_Create(DM75xx_Board_Descriptor * handle,
					      uint16_t ** buffer,
					      dm75xx_dma_channel_t channel,
					      uint32_t samples);
/**
*******************************************************************************
@brief
    Free a buffer previously allocated with DM75xx_DMA_Buffer_Create().

@param
    handle

    Address of device's library board descriptor.

@param
    buffer

    Address of the pointer which to free.

@param
    channel

    The DMA Channel's buffer we want to free.

@retval
    0

    Success.

@retval
    -1

    Failure.

@note
    This function MUST be called if you are planning on use the
    DMA_Buffer_Read() or DMA_Buffer_Write() function calls to manage your DMA
    data.
 ******************************************************************************
 */
	DM75xx_Error DM75xx_DMA_Buffer_Free(DM75xx_Board_Descriptor * handle,
					    uint16_t ** buffer,
					    dm75xx_dma_channel_t channel);
/**
*******************************************************************************
@brief
    Set up direct memory access (DMA) for the given DMA/FIFO channel to/from an
    arbitrary PCI address.

@param
    handle

    Address of device's library board descriptor.

@param
    channel

    The DMA channel to use.

@param
    source

    The FIFO to/from which DMA will be used.

@param
    request

    The DREQ line source.

@param
    samples

    The number of samples desired from the FIFO via DMA.  For HD DMA, this value
    is required to be a multiple of half the FIFO size of the board.  This is
    a limitation of the UTC1 DREQ source used for HD DMA.

@param
    pci_address

    The PCI Address to transfer to/from.

@retval
    0

    Success.

@retval
    -1

    Failure.@n@n
    errno may be set as follows:
        @arg \c
            EAGAIN      DMA has already been initialized for fifo.

        @arg \c
            EINVAL      fifo is not valid.

        @arg \c
            ENOMEM      Kernel memory allocation failed.

        @arg \c
            EOPNOTSUPP  The device is not PCI master capable.

    Please see the ioctl(2) man page for information on other possible values
    errno may have in this case.

@note
    When using DMA to/from arbitrary PCI addresses, no kernel buffer is
    allocated for DMA.

@note
    DMA to arbitrary PCI addresses can cause unknown behavior if you are not
    careful about what you are doing.

@note
    As the application designer, you have some flexibility to configure DMA as
    as your purpose suits.  However, if this function fails with errno ENOMEM,
    you need to allocate smaller buffers.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DMA_Init_Arb(DM75xx_Board_Descriptor * handle,
				dm75xx_dma_channel_t channel,
				dm75xx_dma_source_t source,
				dm75xx_dma_request_t request,
				uint32_t samples, uint32_t pci_address);
/**
*******************************************************************************
@brief
    Set up direct memory access (DMA) for the given DMA/FIFO channel.

@param
    handle

    Address of device's library board descriptor.

@param
    channel

    The DMA channel to use.

@param
    source

    The FIFO to/from which DMA will be used.

@param
    request

    The DREQ line source.

@param
    samples

    The number of samples desired from the FIFO via DMA.  For HD DMA, this value
    is required to be a multiple of half the FIFO size of the board.  This is
    a limitation of the UTC1 DREQ source used for HD DMA.

@param
    buf

    The user space buffer which will be mapped to the kernel space DMA buffer

@retval
    0

    Success.

@retval
    -1

    Failure.@n@n
    errno may be set as follows:
        @arg \c
            EAGAIN      DMA has already been initialized for fifo.

        @arg \c
            EINVAL      fifo is not valid.

        @arg \c
            ENOMEM      Kernel memory allocation failed.

        @arg \c
            EOPNOTSUPP  The device is not PCI master capable.

    Please see the ioctl(2) man page for information on other possible values
    errno may have in this case.

@note
    Since a single DMA buffer must exist in physically contiguous memory, the
    probability of DMA buffer allocation failure increases as both the number of
    buffers to allocate and the size of each buffer increase.

@note
    Factors beyond the number and size of DMA buffers affect the probability of
    DMA buffer allocation failure.  These factors include the number of
    processes on the system, how much system memory is already in use, and the
    presence of processes (such as the X server) which use a lot of memory.

@note
    System memory can be a scarce resource.  Every system entity needs some
    amount of memory.  Memory is being allocated and released all the time.

@note
    The default value for DM75xx_MAX_DMA_BUFFER_SIZE is 131,072 bytes (128
    kilobytes or 65k samples).  If you need to change this, edit
    include/dm75xx_driver.h, save the changes, recompile the driver, and reload
    the driver.

@note
    As the application designer, you have some flexibility to configure DMA as
    as your purpose suits.  However, if this function fails with errno ENOMEM,
    you need to allocate smaller buffers.

@note
    This function also maps a user space buffer to the kernel memory buffer
    allocated for DMA.  This was done to prevent successive calls to
    copy_to_user() or copy_from_user() as both of these functions will sleep
    if the user space buffer was paged out.
 *******************************************************************************
 */

	 DM75xx_Error
	    DM75xx_DMA_Initialize(DM75xx_Board_Descriptor * handle,
				  dm75xx_dma_channel_t channel,
				  dm75xx_dma_source_t source,
				  dm75xx_dma_request_t request,
				  uint32_t samples, uint16_t ** buf);

/**
*******************************************************************************
@brief
    Abort any active transfer on the specified DMA channel.

@param
    handle

    Address of device's library board descriptor.

@param
    channel

    The channel on which to abort DMA transfer.

@retval
    0

    Success

@retval
    -1

    Failure

 *******************************************************************************
 */

	 DM75xx_Error
	    DM75xx_DMA_Abort(DM75xx_Board_Descriptor * handle,
			     dm75xx_dma_channel_t channel);

/**
********************************************************************************
@brief
    Set the enable bit for a particular DMA channel.  To start DMA after this,
    call DM75xx_DMA_Start().

@param
    handle

    Address of device's library board descriptor.

@param
    channel

    The DMA channel to enable on the specified device.

@param
    enable

    0 for disable, > 0 for enable

@return
    0

    Success.

@return
    -1

    Failure.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DMA_Enable(DM75xx_Board_Descriptor * handle,
			      dm75xx_dma_channel_t channel, uint8_t enable);
/**
********************************************************************************
@brief
    Set the demand mode request source for a specified DMA channel.

@param
    handle

    Address of the device's library board descriptor.

@param
    channel

    The specified channel for which to set the request source.

@param
    request

    The demand mode request source to set.

@return
    0

    Success

@return
    -1

    Failure
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DMA_Request_Source(DM75xx_Board_Descriptor * handle,
				      dm75xx_dma_channel_t channel,
				      dm75xx_dma_request_t request);

/**
********************************************************************************
@brief
    Sets the start bit for a particular DMA Channel.  DMA will start if the
    enable bit has been set by DM75xx_DMA_Enable().

@param
    handle

    Address of device's library board descriptor.

@param
    channel

    The DMA channel to start on the specified device.

@return
    0

    Success.

@return
    -1

    Failure.

 *******************************************************************************
 */

	 DM75xx_Error
	    DM75xx_DMA_Start(DM75xx_Board_Descriptor * handle,
			     dm75xx_dma_channel_t channel);

/**
 * @} DM75xx_Library_DMA_Functions
 */

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
General functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @defgroup DM75xx_Library_General_Functions DM75xx user library general functions
 * @{
 */

/**
*******************************************************************************
@brief
    Close a DM75xx device file.

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.@n@n
    errno may be set as follows:
        @arg \c
            ENODATA     handle is NULL.

    Please see the close(2) man page for information on other possible values
    errno may have in this case.

@note
    This function frees the memory allocated for the library board descriptor.

@note
    When processing the close request, the driver disables PLX PCI interrupts,
    disables PLX local interrupt input, and disables PLX DMA channel 0/1
    interrupts.

@warning
    Whether or not this function succeeds, the library board descriptor must
    not be referenced in any way after the function returns.
 *******************************************************************************
 */

	DM75xx_Error DM75xx_Board_Close(DM75xx_Board_Descriptor * handle);

/**
*******************************************************************************
@brief
    Open a DM75xx device file.

@param
    dev_num

    Minor number of DM75xx device file.

@param
    handle

    Address where address of memory allocated for library device descriptor
    should be stored.  If the first open of a device file fails, then NULL will
    be stored here.

@retval
    0

    Success.

@retval
    -1

    Failure.@n@n
    errno may be set as follows:
        @arg \c
            EBUSY       The DM75xx device file with minor number dev_num is
                        already open.
        @arg \c
            ENODEV      dev_num is not a valid DM75xx minor number; 2.4 kernel
                        only.

        @arg \c
            ENOMEM      Library device descriptor memory allocation failed.

        @arg \c
            ENXIO       dev_num is not a valid DM75xx minor number; 2.6 kernel
                        only.

    Please see the open(2) man page for information on other possible values
    errno may have in this case.

@note
    Once a device file is open, it cannot be opened again until it is closed.

@note
    When processing the open request, the driver disables & clears all device
    interrupts, enables PLX PCI interrupts, enables PLX local interrupt input,
    and enables PLX DMA channel 0/1 interrupts.
 *******************************************************************************
 */

	DM75xx_Error DM75xx_Board_Open(uint8_t dev_num,
				       DM75xx_Board_Descriptor ** handle);

/**
********************************************************************************
@brief
    Retrieve the FIFO size of the board from the kernel space device descriptor.

@param
    handle

    Address of the device's library board descriptor.

@param
    data

    Address of the variable in which to store the fifo size.

@retval
    0

    Success

@retval
    -1

    Failure

@note

    This function does not calculate the value of the FIFO upon each call.  The
    FIFO size is determined at 'insmod' time and is stored in the drivers device
    descriptor.

 *******************************************************************************
 */

	 DM75xx_Error
	    DM75xx_FIFO_Size(DM75xx_Board_Descriptor * handle,
			     unsigned int *data);

/**
********************************************************************************
@brief
    Determine the family of the board (DM7520 or SDM7540/8540).

@param
    handle

    Address of the device's library board descriptor.

@param
    data

    This value will be returned as 0 if the board is a DM7520 or a positive
    value if the board is an SDM7540/8540.

@retval
    0

    Success

@retval
    -1

    Failure

********************************************************************************
*/

	 DM75xx_Error
	    DM75xx_Board_Type(DM75xx_Board_Descriptor * handle,
			      dm75xx_board_t * data);

/**
********************************************************************************
@brief
    Install userspace ISR

@param
    handle

    Address of the device's library board descriptor.

@param
    isr_fnct

    Function pointer to the user ISR that will be called in the event of an
    interrupt

@retval
    0

    Success

@retval
    -1

    Failure

@note
    Any previously installed ISR will be removed before installing a new ISR

@note
    This function creates another thread that runs DM75xx_WaitForInterrupt().
    This thread is removed by a call to DM75xx_RemoveISR().

********************************************************************************
*/

	DM75xx_Error DM75xx_InstallISR(DM75xx_Board_Descriptor * handle,
				       void (*isr_fnct) (unsigned int status));

/**
********************************************************************************
@brief
    Uninstall userspace ISR

@param
    handle

    Address of the device's library board descriptor.

@retval
    0

    Success

@retval
    -1

    Failure

@note

    This function makes an ioctl call into the kernel which makes a call to wake
    the thread from the select() call.

********************************************************************************
*/

	DM75xx_Error DM75xx_RemoveISR(DM75xx_Board_Descriptor * handle);

/**
********************************************************************************
@brief
    Function that will have its own thread and wait for interrupts to occur.
    Once an interrupt is received this function will call our callback ISR and
    pass it the interrupt status.

@param
    ptr

    Pointer to be typecasted to the device handle.

@retval
    0

    Success

@retval
    -1

    Failure

@note

    This function should not be called directly by the user.
********************************************************************************
*/

	void *DM75xx_WaitForInterrupt(void *ptr);

/**
 * @} DM75xx_Library_General_Functions
 */

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
User Timer/Counter Functions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
 * @defgroup DM75xx_Library_UTC_Functions DM75xx user library user timer/counter control
 * @{
 */

/**
*******************************************************************************
@brief
    Set a User Timer/Counter Clock Source

@param
    handle

    Address of device's library board descriptor.

@param
	utc

	User Timer/Counter we are configuring.

@param
	source

	The User Timer/Counter source to be set.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UTC_Set_Clock_Source(DM75xx_Board_Descriptor * handle,
					dm75xx_utc_timer_t utc,
					dm75xx_utc_clk_t source);
/**
*******************************************************************************
@brief
    Set a User Timer/Counter Gate

@param
    handle

    Address of device's library board descriptor.

@param
	utc

	User Timer/Counter we are configuring.

@param
	gate

	The User Timer/Counter gate option.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UTC_Set_Gate(DM75xx_Board_Descriptor * handle,
				dm75xx_utc_timer_t utc, dm75xx_utc_gate gate);
/**
*******************************************************************************
@brief
    Set a User Timer/Counter Mode

@param
    handle

    Address of device's library board descriptor.

@param
	utc

	User Timer/Counter we are configuring.

@param
	mode

	The User Timer/Counter mode to be set.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UTC_Set_Mode(DM75xx_Board_Descriptor * handle,
				dm75xx_utc_timer_t utc, dm75xx_utc_mode mode);
/**
*******************************************************************************
@brief
    Set a User Timer/Counter Mode

@param
    handle

    Address of device's library board descriptor.

@param
	utc

	Which user timer/counter's mode to read.

@param
	mode

	Variable to store the retrieved mode value.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UTC_Get_Mode(DM75xx_Board_Descriptor * handle,
				dm75xx_utc_timer_t utc, uint16_t * mode);
/**
*******************************************************************************
@brief
    Set a User Timer/Counter Divisor

@param
    handle

    Address of device's library board descriptor.

@param
	utc

	User Timer/Counter we are configuring.

@param
	rate

	The rate to set for this User Timer/Counter

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UTC_Set_Divisor(DM75xx_Board_Descriptor * handle,
				   dm75xx_utc_timer_t utc, uint16_t rate);
/**
*******************************************************************************
@brief
    Return current value of a User Timer/Counter

@param
    handle

    Address of device's library board descriptor.

@param
	utc

	User Timer/Counter we are configuring.

@param
	count

	Variable to store the current count in.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UTC_Get_Count(DM75xx_Board_Descriptor * handle,
				 dm75xx_utc_timer_t utc, uint16_t * count);
/**
*******************************************************************************
@brief
    Return current status of a User Timer/Counter

@param
    handle

    Address of device's library board descriptor.

@param
	utc_select

	User Timer/Counter we are configuring.

@param
	utc_status

	Variable to store the current status in.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UTC_Get_Status(DM75xx_Board_Descriptor * handle,
				  dm75xx_utc_timer_t utc_select,
				  uint8_t * utc_status);
/**
*******************************************************************************
@brief
    Setup a User Timer/Counter

@param
    handle

    Address of device's library board descriptor.

@param
	utc

	User Timer/Counter we are configuring.

@param
	source

	The User Timer/Counter source to be set.

@param
	gate

	The User Timer/Counter gate option to set.

@param
	mode

	The User Timer/Counter mode option to set.

@param
	divisor

	The divisor to set the User Timer/Counter with.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UTC_Setup(DM75xx_Board_Descriptor * handle,
			     dm75xx_utc_timer_t utc,
			     dm75xx_utc_clk_t source,
			     dm75xx_utc_gate gate, dm75xx_utc_mode mode,
			     uint16_t divisor);
/**
 * @} DM75xx_Library_UTC_Funtions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Burst Clock Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Library_BCLK_Functions DM75xx user library burst clock control
 * @{
 */
/**
*******************************************************************************
@brief
   	Get the current Burst Clock count

@param
    handle

    Address of device's library board descriptor.

@param
	data

	Address of the variable to store the value.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_BCLK_Get_Count(DM75xx_Board_Descriptor * handle,
				  uint16_t * data);
/**
*******************************************************************************
@brief
   	Set the current Burst Clock count.

@param
    handle

    Address of device's library board descriptor.

@param
	data

	Value to write to the Burst Clock count.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_BCLK_Set_Count(DM75xx_Board_Descriptor * handle,
				  uint16_t data);
/**
*******************************************************************************
@brief
   	Set the Burst Clock rate.

@param
    handle

    Address of device's library board descriptor.

@param
	freq

	Set the Burst Clock primary frequency.

@param
	rate

	The desired Burst Clock rate.

@param
	actualRate

	The actual rate set.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_BCLK_Set_Rate(DM75xx_Board_Descriptor * handle,
				 dm75xx_bclk_freq_t freq, float rate,
				 float *actualRate);
/**
*******************************************************************************
@brief
   	Set Burst Clock start trigger

@param
    handle

    Address of device's library board descriptor.

@param
	start

	Start trigger to set.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_BCLK_Set_Start(DM75xx_Board_Descriptor * handle,
				  dm75xx_bclk_start_t start);
/**
*******************************************************************************
@brief
   	Set the Burst Clock primary frequency

@param
    handle

    Address of device's library board descriptor.

@param
	freq

	Frequency to select.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_BCLK_Set_Frequency(DM75xx_Board_Descriptor * handle,
				      dm75xx_bclk_freq_t freq);
/**
*******************************************************************************
@brief
    Setup Burst Clock

@param
    handle

    Address of device's library board descriptor.

@param
	start

	Burst Clock start trigger.

@param
	freq

	Burst Clock primary frequency.

@param
	rate

	Rate at which to set the clock.

@param
	actualRate

	Rate at which the clock is actually set.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 **/
	 DM75xx_Error
	    DM75xx_BCLK_Setup(DM75xx_Board_Descriptor * handle,
			      dm75xx_bclk_start_t start,
			      dm75xx_bclk_freq_t freq, float rate,
			      float *actualRate);

/**
 * @} DM75xx_Library_BCLK_Funtions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Pacer Clock Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Library_PCLK_Functions DM75xx user library pacer clock control
 * @{
 */
/**
*******************************************************************************
@brief
    Set the Pacer Clock frequency

@param
    handle

    Address of device's library board descriptor.

@param
	pclk_freq

	Frequency to set for the pacer clock.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */

	 DM75xx_Error
	    DM75xx_PCLK_Set_Frequency(DM75xx_Board_Descriptor * handle,
				      dm75xx_pclk_freq_t pclk_freq);

/**
*******************************************************************************
@brief
   	Set the Pacer Clock source

@param
    handle

    Address of device's library board descriptor.

@param
	pclk_select

	Source for the Pacer Clock

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */

	 DM75xx_Error
	    DM75xx_PCLK_Set_Source(DM75xx_Board_Descriptor * handle,
				   dm75xx_pclk_select_t pclk_select);
/**
*******************************************************************************
@brief
   	Set the Pacer Clock start trigger

@param
    handle

    Address of device's library board descriptor.

@param
	pclk_start

	Start trigger for the Pacer Clock

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_PCLK_Set_Start(DM75xx_Board_Descriptor * handle,
				  dm75xx_pclk_start_t pclk_start);
/**
*******************************************************************************
@brief
   	Set the Pacer Clock stop trigger

@param
    handle

    Address of device's library board descriptor.

@param
	pclk_stop

	Stop trigger for the Pacer Clock

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_PCLK_Set_Stop(DM75xx_Board_Descriptor * handle,
				 dm75xx_pclk_stop_t pclk_stop);
/**
*******************************************************************************
@brief
   	Read the current pacer clock value

@param
    handle

    Address of device's library board descriptor.

@param
	pacer_value

	Address of the variable to store the value of the pacer

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_PCLK_Read(DM75xx_Board_Descriptor * handle,
			     uint32_t * pacer_value);
/**
*******************************************************************************
@brief
   	Set the Pacer Clock trigger mode.

@param
    handle

    Address of device's library board descriptor.

@param
	pclk_mode

	Mode in which to set the pacer clock.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_PCLK_Set_Trigger_Mode(DM75xx_Board_Descriptor * handle,
					 dm75xx_pclk_mode_t pclk_mode);
/**
*******************************************************************************
@brief
   	Set the Pacer Clock Count

@param
    handle

    Address of device's library board descriptor.

@param
	count

	Pacer Clock count

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_PCLK_Set_Count(DM75xx_Board_Descriptor * handle,
				  uint32_t count);
/**
*******************************************************************************
@brief
   	Set the Pacer Clock Rate.

@param
    handle

    Address of device's library board descriptor.

@param
	freq

	Pacer Clock primary frequency.

@param
	rate

	Rate desired or the Pacer Clock.

@param
	actualRate

	Address to store the actual rate value.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_PCLK_Set_Rate(DM75xx_Board_Descriptor * handle,
				 dm75xx_pclk_freq_t freq, float rate,
				 float *actualRate);
/**
*******************************************************************************
@brief
   	Setup the Pacer Clock

@param
    handle

    Address of device's library board descriptor.

@param
	pclk_select

	Select Internal/External Pacer Clock.

@param
	pclk_freq

	Select the primary clock frequency.

@param
	pclk_mode

	Select the trigger mode.

@param
	pclk_start

	Select the start trigger.

@param
	pclk_stop

	Select the stop trigger.

@param
	rate

	Desired rate for the Pacer Clock.

@param
	actualRate

	Rate actually set.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_PCLK_Setup(DM75xx_Board_Descriptor * handle,
			      dm75xx_pclk_select_t pclk_select,
			      dm75xx_pclk_freq_t pclk_freq,
			      dm75xx_pclk_mode_t pclk_mode,
			      dm75xx_pclk_start_t pclk_start,
			      dm75xx_pclk_stop_t pclk_stop, float rate,
			      float *actualRate);
/**
*******************************************************************************
@brief
   	Software Pacer Clock Start.

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	DM75xx_Error DM75xx_PCLK_Start(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
   	Software Pacer Clock Stop.

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	DM75xx_Error DM75xx_PCLK_Stop(DM75xx_Board_Descriptor * handle);

/**
 * @} DM75xx_Library_PCLK_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Channel Gain Table Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @defgroup DM75xx_Library_CGT_Functions DM75xx user library channel gain table
 * @{
 */
 /**
*******************************************************************************
@brief
    Reset Channel Gain Table.

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_CGT_Reset(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Clear Channel Gain Table.

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_CGT_Clear(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Create a channel gain table entry.

@param
    cgt

    Struct that holds the values for the channel gain table.

@param
	cgt_entry

	The channel gain table converted to a uint16_t for register entry.

@retval
    0

    Success.

@retval
    -1

    Failure.

@note
    This function should not be called by the user.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_CGT_Create_Entry(dm75xx_cgt_entry_t * cgt,
				    uint16_t * cgt_entry);
/**
*******************************************************************************
@brief
    Write a channel gain table entry.  This function utilizes
    DM75xx_CGT_Create_Entry() to create the 16 bit entry.

@param
    handle

	Address of device's library board descriptor.

@param
	cgt

	The channel gain table entry to write.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_CGT_Write(DM75xx_Board_Descriptor * handle,
				      dm75xx_cgt_entry_t cgt);
/**
*******************************************************************************
@brief
    Write ADC channel gain table latch for single channel sampling.

@param
    handle

	Address of device's library board descriptor.

@param
	cgt

	Channel gain table entry to write to the latch.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_CGT_Latch(DM75xx_Board_Descriptor * handle,
			     dm75xx_cgt_entry_t cgt);
/**
*******************************************************************************
@brief
    Enable/disable A/D channel gain table.

@param
    handle

	Address of device's library board descriptor.

@param
	enable

	A 0 denotes CGT Disable and CG Latch Enable, a 1 denotes CGT Enable and
	CG Latch Disable.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_CGT_Enable(DM75xx_Board_Descriptor * handle,
			      uint16_t enable);
/**
*******************************************************************************
@brief
    Enable/disable Digital Table.

@param
    handle

	Address of device's library board descriptor.

@param
	enable

	Enable/Disable the Digital Table and Digital I/O Port 1.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DT_Enable(DM75xx_Board_Descriptor * handle, uint16_t enable);
/**
*******************************************************************************
@brief
    Write Digital Table entry.

@param
    handle

	Address of device's library board descriptor.

@param
	data

	Entry to add to the digital table.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DT_Write_Entry(DM75xx_Board_Descriptor * handle,
				  uint8_t data);
/**
*******************************************************************************
@brief
    Pause the Channel Gain Table

@param
    handle

	Address of device's library board descriptor.

@param
	pause

	Enable/Disable CGT Pause.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note

	Pause is ignored in burst mode.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_CGT_Pause(DM75xx_Board_Descriptor * handle,
				      uint16_t pause);

/**
 * @} DM75xx_Library_CGT_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Analog to Digital Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_ADC_Functions DM75xx user library analog to digital
 */
/**
********************************************************************************
@brief
    Read a value from the A/D FIFO

@param
    handle

	Address of device's library board descriptor.

@param
	value

	Address of the variable to store the value read.

@return
    0

    Success

@return
	-1

    Failure.@n@n
	errno may be set as follows:
		@arg \c
			EINVAL		pointer was NULL.

    Please see the close(2) man page for information on other possible values
    errno may have in this case.

********************************************************************************
*/
	 DM75xx_Error
	    DM75xx_ADC_FIFO_Read(DM75xx_Board_Descriptor * handle,
				 uint16_t * value);
/**
*******************************************************************************
@brief
    Analog to Digital Software Sample

@param
    handle

	Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_ADC_Software_Sample(DM75xx_Board_Descriptor *
						handle);
/**
*******************************************************************************
@brief
    Select the A/D Conversion Signal

@param
    handle

	Address of device's library board descriptor.

@param
	adc_conv_signal

	The A/D conversion signal to select.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ADC_Conv_Signal(DM75xx_Board_Descriptor * handle,
				   dm75xx_adc_conv_signal_t adc_conv_signal);
/**
*******************************************************************************
@brief
    Select the A/D Sample Counter Source

@param
    handle

	Address of device's library board descriptor.

@param
	src

	The selected sample counter source.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ADC_SCNT_Source(DM75xx_Board_Descriptor * handle,
				   dm75xx_adc_scnt_src_t src);
/**
*******************************************************************************
@brief
    Enable/Disable About Counter stop

@param
    handle

	Address of device's library board descriptor.

@param
	enable

	Enable/Disable.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ADC_About_Enable(DM75xx_Board_Descriptor * handle,
				    uint16_t enable);
/**
*******************************************************************************
@brief
    Clear Analag to Digital FIFO

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_ADC_Clear(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Read the value in the A/D Sample Counter

@param
    handle

    Address of device's library board descriptor.

@param
	data

	Address of the variable to store the data.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ADC_SCNT_Read(DM75xx_Board_Descriptor * handle,
				 uint16_t * data);
/**
*******************************************************************************
@brief
    Load a value into the A/D Sample Counter.

@param
    handle

    Address of device's library board descriptor.

@param
	data

	Value to load into the sample counter.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ADC_SCNT_Load(DM75xx_Board_Descriptor * handle,
				 uint16_t data);
/**
 * @} DM75xx_Library_ADC_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Digital to Analog Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_DAC_Functions DM75xx user library digital to analog
 */
/**
*******************************************************************************
@brief
    Cause a DAC software update on the specified channel.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specific DAC channel(s).

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Soft_Update(DM75xx_Board_Descriptor * handle,
				   uint8_t dac);
/**
*******************************************************************************
@brief
    Get DAC update counter for a specified channel.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specific DAC channel.

@param
	data

	Address of the variable to store the data.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Get_Update_Counter(DM75xx_Board_Descriptor * handle,
					  dm75xx_dac_channel_t dac,
					  uint16_t * data);
/**
*******************************************************************************
@brief
    Set the DAC update counter for a specified channel.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specific DAC channel.

@param
	data

	The value to write to the DAC update counter

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Update_Counter(DM75xx_Board_Descriptor * handle,
					  dm75xx_dac_channel_t dac,
					  uint16_t data);
/**
*******************************************************************************
@brief
    Set the DAC output range for a specified channel.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specified DAC channel.

@param
	range

	The specified output range.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Range(DM75xx_Board_Descriptor * handle,
				 dm75xx_dac_channel_t dac,
				 dm75xx_dac_range_t range);
/**
*******************************************************************************
@brief
    Set the DAC Update Source for the specified channel.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specified DAC channel.

@param
	src

	The specified update source.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Update_Source(DM75xx_Board_Descriptor * handle,
					 dm75xx_dac_channel_t dac,
					 dm75xx_dac_update_src_t src);

/**
*******************************************************************************
@brief
    Set the DAC mode for a specified channel.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specified DAC channel.

@param
	mode

	The specified mode.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Mode(DM75xx_Board_Descriptor * handle,
				dm75xx_dac_channel_t dac,
				dm75xx_dac_mode_t mode);

/**
*******************************************************************************
@brief
    Write a value to the DAC FIFO of a specified channel.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specified DAC channel.

@param
	data

	Value to write to the DAC FIFO.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_FIFO_Write(DM75xx_Board_Descriptor * handle,
				  dm75xx_dac_channel_t dac, uint16_t data);
/**
*******************************************************************************
@brief
    Set the primary slock frequency for DAC conversion.

@param
    handle

	Address of device's library board descriptor.

@param
	freq

	The specified primary clock frequency.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Frequency(DM75xx_Board_Descriptor * handle,
				     dm75xx_dac_freq_t freq);
/**
*******************************************************************************
@brief
    Set the DAC Clock Count.

@param
    handle

	Address of device's library board descriptor.

@param
	count

	The value to which to set the DAC Clock Count

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	This function calls DM75xx_DAC_Set_Clock()
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Count(DM75xx_Board_Descriptor * handle,
				 uint32_t count);
/**
*******************************************************************************
@brief
    Set the DAC conversion rate.

@param
    handle

	Address of device's library board descriptor.

@param
	freq

	The specified primary frequency.

@param
	rate

	The chosen rate for conversion (in Hz).

@param
	actualRate

	Address of the variable to store the precise rate the clock was set to.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	This function calls DM75xx_DAC_Set_Clock()
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Rate(DM75xx_Board_Descriptor * handle,
				dm75xx_dac_freq_t freq, uint32_t rate,
				float *actualRate);
/**
*******************************************************************************
@brief
    Set the DAC Clock Stop Value

@param
    handle

	Address of device's library board descriptor.

@param
	stop

	The selected clock stop value to be written.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	This function calls DM75xx_DAC_Set_Clock()
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Clock_Stop(DM75xx_Board_Descriptor * handle,
				      dm75xx_dac_clk_stop_t stop);
/**
*******************************************************************************
@brief
    Set the DAC Clock Start Value

@param
    handle

	Address of device's library board descriptor.

@param
	start

	The selected clock start value to be written.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	This function calls DM75xx_DAC_Set_Clock()
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_Clock_Start(DM75xx_Board_Descriptor * handle,
				       dm75xx_dac_clk_start_t start);
/**
*******************************************************************************
@brief
    Causes a DAC Software Start.

@param
    handle

	Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	This function calls DM75xx_DAC_Set_Clock()
 *******************************************************************************
 */
	DM75xx_Error DM75xx_DAC_Start(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Causes a DAC Software Stop.

@param
    handle

	Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	This function calls DM75xx_DAC_Set_Clock()
 *******************************************************************************
 */
	DM75xx_Error DM75xx_DAC_Stop(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Setup a DAC channel.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specified DAC channel.

@param
	range

	The specified DAC range.

@param
	src

	The specified DAC update source.

@param
	mode

	The specified DAC mode.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Setup(DM75xx_Board_Descriptor * handle,
			     dm75xx_dac_channel_t dac,
			     dm75xx_dac_range_t range,
			     dm75xx_dac_update_src_t src,
			     dm75xx_dac_mode_t mode);
/**
*******************************************************************************
@brief
    Reset a DAC Fifo.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specified DAC channel.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_DAC_Reset(DM75xx_Board_Descriptor * handle,
				      dm75xx_dac_channel_t dac);
/**
*******************************************************************************
@brief
    Clear a DAC Fifo.

@param
    handle

	Address of device's library board descriptor.

@param
	dac

	The specified DAC channel.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_DAC_Clear(DM75xx_Board_Descriptor * handle,
				      dm75xx_dac_channel_t dac);
/**
*******************************************************************************
@brief
    Set DAC Clock Mode

@param
    handle

	Address of device's library board descriptor.

@param
	clk_mode

	The mode set the DAC Clock (Free Run or Start/Stop).

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DAC_Set_CLK_Mode(DM75xx_Board_Descriptor * handle,
				    dm75xx_dac_clk_mode_t clk_mode);
/**
 * @} DM75xx_Library_DAC_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Highspeed Digital Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_HSDIN_Functions DM75xx user library high speed digital
 */
/**
*******************************************************************************
@brief
    Software high speed digital input sample command

@param
    handle

	Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	This function calls DM75xx_DAC_Set_Clock()
 *******************************************************************************
 */
	DM75xx_Error DM75xx_HSDIN_Software_Sample(DM75xx_Board_Descriptor *
						  handle);
/**
*******************************************************************************
@brief
    Set HighSpeed digital sampling signal.

@param
    handle

	Address of device's library board descriptor.

@param
	signal

	Sampling signal to select.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_HSDIN_Sample_Signal(DM75xx_Board_Descriptor * handle,
				       dm75xx_hsdin_signal_t signal);

/**
*******************************************************************************
@brief
    Clear High Speed Digital FIFO

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	DM75xx_Error DM75xx_HSDIN_Clear(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Read value from High Speed Digital FIFO

@param
    handle

    Address of device's library board descriptor.

@param
	data

	Address of the variable to store the data.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 ****
 */
	 DM75xx_Error
	    DM75xx_HSDIN_FIFO_Read(DM75xx_Board_Descriptor * handle,
				   uint16_t * data);
/**
 * @} DM75xx_Library_HSDIN_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 SyncBus Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_SBUS_Functions DM75xx user library syncbus
 */
/**
*******************************************************************************
@brief
	Set SyncBus Source

@param
    handle

	Address of device's library board descriptor.

@param
	sbus

	The specified SyncBus.

@param
	src

	Source to set for the specified SyncBus.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_SBUS_Set_Source(DM75xx_Board_Descriptor * handle,
				   dm75xx_sbus_t sbus, dm75xx_sbus_src_t src);
/**
*******************************************************************************
@brief
    Enable/Disable Syncbus

@param
    handle

	Address of device's library board descriptor.

@param
	sbus

	The specified SyncBus.

@param
	enable

	Value determining whether to enable/disable the syncbus.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_SBUS_Enable(DM75xx_Board_Descriptor * handle,
			       dm75xx_sbus_t sbus, uint16_t enable);
/**
 * @} DM75xx_Library_SBUS_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 About Counter Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_ACNT_Functions DM75xx user library about counter
 */
/**
*******************************************************************************
@brief
    Get About Counter value

@param
    handle

	Address of device's library board descriptor.

@param
	data

	Address of the variable to store the value.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ACNT_Get_Count(DM75xx_Board_Descriptor * handle,
				  uint16_t * data);
/**
*******************************************************************************
@brief
    Set the About Counter value.

@param
    handle

	Address of device's library board descriptor.

@param
	data

	Value at which to set the About Counter.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ACNT_Set_Count(DM75xx_Board_Descriptor * handle,
				  uint16_t data);
/**
 * @} DM75xx_Library_ACNT_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Delay Counter Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_DCNT_Functions DM75xx user library delay counter
 */
/**
*******************************************************************************
@brief
    Get the Delay Counter value.

@param
    handle

	Address of device's library board descriptor.

@param
	data

	Address of the variable to store the value.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DCNT_Get_Count(DM75xx_Board_Descriptor * handle,
				  uint16_t * data);
/**
*******************************************************************************
@brief
    Set the Delay Counter value.

@param
    handle

	Address of device's library board descriptor.

@param
	data

	Value at which to set the Delay Counter.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DCNT_Set_Count(DM75xx_Board_Descriptor * handle,
				  uint16_t data);
/**
 * @} DM75xx_Library_DCNT_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 Digital Input/Output
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_DIO_Functions DM75xx user library digital input/output
 */
/**
*******************************************************************************
@brief
    Set a specified Digital I/O Port to the given value.

@param
    handle

    Address of device's library board descriptor.

@param
	port

	The specified Digital I/O Port.

@param
	data

	The value to set on the Digital I/O Port.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Set_Port(DM75xx_Board_Descriptor * handle,
				dm75xx_dio_port_t port, uint8_t data);
/**
*******************************************************************************
@brief
    Get the value from the specified Digital I/O Port.

@param
    handle

    Address of device's library board descriptor.

@param
	port

	The specified Digital I/O Port.

@param
	data

	The address of the variable to store the Digital I/O Port's value.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Get_Port(DM75xx_Board_Descriptor * handle,
				dm75xx_dio_port_t port, uint8_t * data);
/**
*******************************************************************************
@brief
    Get the Digital I/O Status byte.

@param
    handle

    Address of device's library board descriptor.

@param
	data

	Address of the variable to store the status byte.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Get_Status(DM75xx_Board_Descriptor * handle,
				  uint8_t * data);
/**
*******************************************************************************
@brief
    Clear Digital I/O IRQ Status

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_DIO_Clear_IRQ(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Clear Digital I/O Chip.

@param
    handle

    Address of device's library board descriptor.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_DIO_Reset(DM75xx_Board_Descriptor * handle);
/**
*******************************************************************************
@brief
    Set the direction of the specified Digital I/O Port.

@param
    handle

    Address of device's library board descriptor.

@param
	port

	The specified Digital I/O Port.

@param
	direction

	The direction to set for the specified Digital I/O Port.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	Port 0 is bit directional and Port 1 is byte directional.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Set_Direction(DM75xx_Board_Descriptor * handle,
				     dm75xx_dio_port_t port, uint8_t direction);
/**
*******************************************************************************
@brief
    Set Digital I/O Port 0 Mask.

@param
    handle

    Address of device's library board descriptor.

@param
	mask

	The mask to set for Digital I/O Port 0.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
    A mask value can only be set for Digital I/O Port 0.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Set_Mask(DM75xx_Board_Descriptor * handle, uint8_t mask);
/**
*******************************************************************************
@brief
    Set the compare register for Digital I/O Port 0

@param
    handle

    Address of device's library board descriptor.

@param
	compare

	The value to compare for Match Mode.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
    A compare value can only be set for Digital I/O Port 0.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Set_Compare(DM75xx_Board_Descriptor * handle,
				   uint8_t compare);
/**
*******************************************************************************
@brief
    Get the compare register for Digital I/O Port 0

@param
    handle

    Address of device's library board descriptor.

@param
	compare

	Address of the variable to store the value in the compare register.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

@note
	This register is used as a latch when in event mode.  The value that caused
	the event will be latched to this register and can subsequently be read.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Get_Compare(DM75xx_Board_Descriptor * handle,
				   uint8_t * compare);
/**
*******************************************************************************
@brief
    Set the IRQ Mode for Digital I/O.

@param
    handle

    Address of device's library board descriptor.

@param
	mode

	Set event or match mode IRQ.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_IRQ_Mode(DM75xx_Board_Descriptor * handle,
				dm75xx_dio_mode_t mode);
/**
*******************************************************************************
@brief
    Set the Digital I/O Sample Clock.

@param
    handle

    Address of device's library board descriptor.

@param
	clock

	The clock

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Clock(DM75xx_Board_Descriptor * handle,
			     dm75xx_dio_clk_t clock);
/**
*******************************************************************************
@brief
    Enable/Disable Digital I/O Interrupts

@param
    handle

    Address of device's library board descriptor.

@param
	enable

	0 for Disable anything else for Enable

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DIO_Enable_IRQ(DM75xx_Board_Descriptor * handle,
				  uint8_t enable);
/**
 * @} DM75xx_Library_DIO_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 User I/O Functions.
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_UIO_Functions DM75xx user library user I/O
 */
/**
 *******************************************************************************
@brief
    Selects the source of a user I/O signal

@param
    handle

    Address of device's library board descriptor

@param
    channel

    The user output channel on which the signal will be sent

@param
    source

    The source for the signal

@retval
    0

    Success

@retval
    -1

    Failure

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_UIO_Select(DM75xx_Board_Descriptor * handle,
			      dm75xx_uio_channel_t channel,
			      dm75xx_uio_source_t source);
/**
 *******************************************************************************
@brief
    Read the current status of the user I/O

@param
    handle

    Address of device's library board descriptor

@param
    data

    Address of the variable to store the read value.

@retval
    0

    Success

@retval
    -1

    Failure

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_UIO_Read(DM75xx_Board_Descriptor * handle,
				     uint32_t * data);
/**
 *******************************************************************************
@brief
    Write the value of the user I/O

@param
    handle

    Address of device's library board descriptor

@param
    data

    Value to write to the user I/O

@retval
    0

    Success

@retval
    -1

    Failure

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_UIO_Write(DM75xx_Board_Descriptor * handle,
				      uint32_t data);
/**
 * @} DM75xx_Library_UIO_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 McBSP Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
* @brief
* @{ DM75xx_Library_McBSP_Functions DM75xx user library mcbsp
*/
/**
 *******************************************************************************
@brief
    Enable/Disable A/D FIFO to DSP

@param
    handle

    Address of device's library board descriptor

@param
    enable

    0x00 disables, 0xFF enables

@retval
    0

    Success

@retval
    -1

    Failure

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_McBSP_ADC_FIFO(DM75xx_Board_Descriptor * handle,
					   uint8_t enable);
/**
 *******************************************************************************
@brief
    Enable/Disable D/A FIFO to DSP

@param
    handle

    Address of device's library board descriptor

@param
    enable

    0x00 disables, 0xFF enables

@retval
    0

    Success

@retval
    -1

    Failure

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_McBSP_DAC_FIFO(DM75xx_Board_Descriptor * handle,
					   uint8_t enable);

/**
 * @} DM75xx_Library_McBSP_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 External Trigger/Interrupt Polarity Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_EXT_Functions DM75xx user library external trigger/interrupt
 */
/**
*******************************************************************************
@brief
    Set the External Trigger polarity

@param
    handle

	Address of device's library board descriptor.

@param
	polarity

	Positive/Negative polarity select.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ETRIG_Polarity_Select(DM75xx_Board_Descriptor * handle,
					 dm75xx_ext_polarity_t polarity);
/**
*******************************************************************************
@brief
    Set the External Interrupt polarity

@param
    handle

	Address of device's library board descriptor.

@param
	polarity

	Positive/Negative polarity select.

@retval
    0

    Success.

@retval
    -1

    Failure.

	Please see the ioctl(2) man page for information on possible values errno
    may have in this case.

 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_EINT_Polarity_Select(DM75xx_Board_Descriptor * handle,
					dm75xx_ext_polarity_t polarity);
/**
 * @} DM75xx_Library_EXT_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 General Status Functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_STATUS_Functions DM75xx user library status
 */
 /**
*******************************************************************************
@brief
    Get current FIFO Status

@param
    handle

    Address of device's library board descriptor.

@param
	fifo_status

	Variable in which to store the current status.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_FIFO_Get_Status(DM75xx_Board_Descriptor * handle,
				   uint16_t * fifo_status);

/**
*******************************************************************************
@brief
    Get status of pacer/burst clocks.

@param
    handle

    Address of device's library board descriptor.

@param
	status

	Variable in which to store the current status.

@retval
    0

    Success.

@retval
    -1

    Failure.

    Please see the ioctl(2) man page for information on possible values errno
    may have in this case.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_CLK_Get_Status(DM75xx_Board_Descriptor * handle,
				  uint16_t * status);
/**
 * @} DM75xx_Library_STATUS_Functions
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 SDM7540 Specific functions
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/**
 * @brief
 * @{ DM75xx_Library_SDM7540_Functions DM75xx user library SDM7540 functions
 */
/**
*******************************************************************************
@brief
    Calibrate an SDM7540/SDM8540.

@param
    handle

    Address of the device's library board descriptor.

@param
    dac1_value

    Value to set on DAC1 after calibration.

@param
    dac2_value

    Value to set on DAC2 after calibration.

@param
    dac1_range

    The voltage range by which to calibrate dac1.

@param
    dac2_range

    The voltage range by which to calibrate dac2.

@retval
    0

    Success.

@retval
    -1

    Failure.

 *******************************************************************************
 */
	DM75xx_Error DM75xx_Calibrate(DM75xx_Board_Descriptor * handle,
				      uint16_t dac1_value,
				      uint16_t dac2_value,
				      dm75xx_dac_range_t dac1_range,
				      dm75xx_dac_range_t dac2_range);
/**
*******************************************************************************
@brief
    Issue a command to the 7540 onboard DSP.

@param
    handle

    Address of the device's library board descriptor.

@param
    command

    The DSP Command to issue.

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DSP_CMD_Send(DM75xx_Board_Descriptor * handle,
				dm75xx_dsp_command_t command);
/**
*******************************************************************************
@brief
    Checks if the last command given to the DSP is finished.

@param
    handle

    Address of the device's library board descriptor.

@param
    data

    This value will be zero if the DSP has completed the instruction and greater
    than zero if an instruction is still being executed.

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DSP_CMD_Complete(DM75xx_Board_Descriptor * handle,
				    uint8_t * data);
/**
*******************************************************************************
@brief
    Checks whether or not a command successfully completed on the DSP.

@param
    handle

    Address of the device's library board descriptor.

@param
    command

    The DSP command status that is being checked.

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_DSP_CMD_Status(DM75xx_Board_Descriptor * handle,
				  dm75xx_dsp_command_t command);
/**
*******************************************************************************
@brief
    Get the the mask of the Analog DIO.

@param
    handle

    Address of the device's library board descriptor.

@param
    pin1

    Pin1 mask enabled/disabled.

@param
    pin2

    Pin2 mask enabled/disabled.

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ALGDIO_Get_Mask(DM75xx_Board_Descriptor * handle,
				   dm75xx_algdio_mask_t * pin1,
				   dm75xx_algdio_mask_t * pin2);
/**
*******************************************************************************
@brief
    Set the Analog DIO Mask.

@param
    handle

    Address of the device's library board descriptor.

@param
    pin1

    Pin1 mask enabled/disabled.

@param
    pin2

    Pin2 mask enabled/disabled.

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ALGDIO_Set_Mask(DM75xx_Board_Descriptor * handle,
				   dm75xx_algdio_mask_t pin1,
				   dm75xx_algdio_mask_t pin2);
/**
*******************************************************************************
@brief
    Get the Analog DIO Direction.

@param
    handle

    Address of the device's library board descriptor.

@param
    pin1

    Pin1 Direction 0 = Input, Positive vals = Output.

@param
    pin2

    Pin2 Direction 0 = Input, Positive vals = Output.

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ALGDIO_Get_Direction(DM75xx_Board_Descriptor * handle,
					dm75xx_algdio_direction_t * pin1,
					dm75xx_algdio_direction_t * pin2);
/**
*******************************************************************************
@brief
    Set the Analog DIO Direction.

@param
    handle

    Address of the device's library board descriptor.

@param
    pin1

    Pin1 Direction 0 = Input, Positive vals = Output.

@param
    pin2

    Pin2 Direction 0 = Input, Positive vals = Output.

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ALGDIO_Set_Direction(DM75xx_Board_Descriptor * handle,
					dm75xx_algdio_direction_t pin1,
					dm75xx_algdio_direction_t pin2);
/**
*******************************************************************************
@brief
    Set the Analog DIO pin values.

@param
    handle

    Address of the device's library board descriptor.

@param
    pin1

    Value at logic high (0xFF) or logic low (0x00).

@param
    pin2

    Value at logic high (0xFF) or logic low (0x00).

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ALGDIO_Set_Data(DM75xx_Board_Descriptor * handle,
				   uint8_t pin1, uint8_t pin2);
/**
*******************************************************************************
@brief
    Get the Analog DIO pin values.

@param
    handle

    Address of the device's library board descriptor.

@param
    pin1

    Value at logic high (0xFF) or logic low (0x00).

@param
    pin2

    Value at logic high (0xFF) or logic low (0x00).

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ALGDIO_Get_Data(DM75xx_Board_Descriptor * handle,
				   uint8_t * pin1, uint8_t * pin2);
/**
*******************************************************************************
@brief
    Get Analog DIO IRQ Status.

@param
    handle

    Address of the device's library board descriptor.

@param
    status

    The IRQ status.

@retval
    0

    Success.

@retval
    -1

    Failure.
 *******************************************************************************
 */
	 DM75xx_Error
	    DM75xx_ALGDIO_Get_IRQ_Status(DM75xx_Board_Descriptor * handle,
					 uint8_t * status);
/**
*******************************************************************************
@brief
    Get the temperature from the board.

@param
    handle

    Address of the device's library board descriptor.

@param
    temp

    The temperature returned from the board.

@retval
    0

    Success

@retval
    -1

    Failure
 *******************************************************************************
 */
	DM75xx_Error DM75xx_Get_Temp(DM75xx_Board_Descriptor * handle,
				     uint8_t * temp);

/**
 * @} DM75xx_Library_SDM7540_Functions
 */
/**
 * @} DM75xx_Library_Functions
 */

/**
 * @} DM75xx_Library_Header
 */

#ifdef __cplusplus
}
#endif
#endif				/* __dm75xx_library_h__ */
