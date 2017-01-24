/**
    @file

    @brief
        Definitions for the DM75xx driver

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

    $Id: dm75xx_driver.h 99068 2016-04-26 18:25:17Z rgroner $
*/

#ifndef __dm75xx_driver_h__
#define __dm75xx_driver_h__

#include <linux/fs.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include "dm75xx_ioctl.h"
#include "dm75xx_types.h"
#include "dm75xx_kernel.h"

/**
 * @defgroup DM75xx_Driver_Header DM75xx driver header file
 * @{
 */

/*=============================================================================
Enumerations
 =============================================================================*/

/**
 * @defgroup DM75xx_Driver_Enumerations DM75xx driver enumerations
 * @{
 */

/**
 * @brief
 *      Direction of access to standard PCI region
 */

enum dm75xx_pci_region_access_dir {

    /**
     * Read from the region
     */

	DM75xx_PCI_REGION_ACCESS_READ = 0,

    /**
     * Write to the region
     */

	DM75xx_PCI_REGION_ACCESS_WRITE
};

/**
 * @brief
 *      Standard PCI region access direction type
 */

typedef enum dm75xx_pci_region_access_dir dm75xx_pci_region_access_dir_t;

/**
 * @} DM75xx_Driver_Enumerations
 */

/*=============================================================================
Macros
 =============================================================================*/

/**
 * @defgroup DM75xx_Driver_Macros DM75xx driver macros
 * @{
 */

/**
 * @brief
 *      Maximum number of characters in device's name
 */

#define DM75xx_DEVICE_NAME_LENGTH   22

/**
 * @brief
 *      DM7520 PCI device ID
 */

#define DM7520_PCI_DEVICE_ID        0x7520

/**
 * @brief
 *      DM7540 PCI device ID
 */

#define DM7540_PCI_DEVICE_ID        0x7540

/**
 * @brief
 *      RTD Embedded Technologies PCI vendor ID
 */

#define RTD_PCI_VENDOR_ID           0x1435

/**
 * @brief
 *      Number of standard PCI regions
 */

#define DM75xx_PCI_REGIONS          PCI_ROM_RESOURCE

/**
 * @brief
 *      Number of FIFO channels per device
 */

#define DM75xx_DMA_CHANNELS         2

/**
 * @brief
 *      Maximum size in bytes of any DMA buffer
 *
 * @note
 *      Be aware that the probability of DMA buffer allocation failure increases
 *      as the buffer size increases.
 *
 * @note
 *      If this default value does not suit your needs, you can change it and
 *      then recompile the driver.
 *
 * @note
 *      The max buffer size is set to 128k to remain architecture independent.
 *      it is more than likely that much more than this can be allocated on an
 *      x86 system at one time.
 */

#define DM75xx_MAX_DMA_BUFFER_SIZE  0x20000

/**
 * @brief
 *      Maximum size in entries of the interrupt status queue
 */

#define DM75xx_INT_QUEUE_SIZE       0x10

/**
 * @} DM75xx_Driver_Macros
 */

/*=============================================================================
Structures
 =============================================================================*/

/**
 * @defgroup DM75xx_Driver_Structures DM75xx driver structures
 * @{
 */

/**
 * @brief
 *      DM75xx PCI region descriptor.  This structure holds information about
 *      one of a device's PCI memory regions.
 */

struct dm75xx_pci_region {

    /**
     * I/O port number if I/O mapped
     */

	unsigned long io_addr;

    /**
     * Length of region in bytes
     */

	unsigned long length;

    /**
     * Region's physical address if memory mapped or I/O port number if I/O
     * mapped
     */

	unsigned int phys_addr;

    /**
     * Address at which region is mapped in kernel virtual address space if
     * memory mapped
     */

	void *virt_addr;

    /**
     * Flag indicating whether or not the I/O-mapped memory ranged was
     * allocated.  A value of zero means the memory range was not allocated.
     * Any other value means the memory range was allocated.
     */

	uint8_t allocated;
};

/**
 * @brief
 *      DM75xx PCI region descriptor type
 */

typedef struct dm75xx_pci_region dm75xx_pci_region_t;

/**
 * @brief
 *      Dm75xx DMA chaining descriptor.
 */
struct dm75xx_dma_chain_descriptor {
    /**
     * PCI Address
     */
	uint32_t pci_address;
    /**
     * Local Address
     */
	uint32_t local_address;
    /**
     * Transfer Size
     */
	uint32_t transfer_size;
    /**
     * Descriptor Pointer
     */
	uint32_t descriptor_pointer;
};

/**
 * @brief
 *      DM75xx DMA Chaining descriptor type
 */

typedef struct dm75xx_dma_chain_descriptor dm75xx_dma_chain_descriptor_t;

/**
 * @brief
 *      DM75xx DMA buffer descriptor.  This structure holds allocation
 *      information for a single DMA buffer.
 */

struct dm75xx_dma_descriptor {

    /**
     * Bus/physical address
     */

	dma_addr_t bus_address;

    /**
     * Virtual address
     */

	void *virtual_address;

    /**
     * Buffer size
     */

	unsigned long size;
};
/**
 * @brief
 * DM75xx DMA buffer descriptor type
 */
typedef struct dm75xx_dma_descriptor dm75xx_dma_descriptor_t;

/**
 * @brief
 *      DM75xx device descriptor.  This structure holds information about a
 *      device needed by the kernel.
 */

struct dm75xx_device_descriptor {

    /**
     * Device name used when requesting resources; a NUL terminated string of
     * the form rtd-dm75xx-x where x is the device minor number.
     */

	char name[DM75xx_DEVICE_NAME_LENGTH];

    /**
     * Flag which indicates if the board has SDM7540/8540 functionality
     */

	dm75xx_board_t board_type;

    /**
     * Information about each of the standard PCI regions
     */

	dm75xx_pci_region_t pci[PCI_ROM_RESOURCE];

    /**
     * Interrupt Control
     */

	dm75xx_int_source_t int_control;

    /**
     * Interrupt status queue
     */

	uint32_t int_status[DM75xx_INT_QUEUE_SIZE];

    /**
     * Number of entries in the interrupt status queue
     */

	unsigned int int_queue_in;

    /**
     * Number of entries read from the interrupt status queue
     */

	unsigned int int_queue_out;

    /**
     * Number of interrupts missed because of a full queue
     */

	unsigned int int_queue_missed;

    /**
     * Number of interrupts in the queue
     */

	unsigned int int_count;

    /**
     * The board's FIFO capacity.
     */

	unsigned int fifo_size;

    /**
     * Concurrency control
     */

	spinlock_t lock;

    /**
     * Number of entities which have the device file open.  Used to enforce
     * single open semantics.
     */

	uint8_t reference_count;

    /**
     * IRQ line number
     */

	unsigned int irq_number;

    /**
     * Used to assist in shutting down the thread waiting for interrupts
     */

	uint8_t remove_isr_flag;

    /**
     * Queue of processes waiting to be woken up when an interrupt occurs
     */

	wait_queue_head_t int_wait_queue;

    /**
     * Per-FIFO channel DMA transfer size in bytes
     */

	uint32_t dma_size[DM75xx_DMA_CHANNELS];

    /**
     * Per DMA channel buffer information
     */

	dm75xx_dma_descriptor_t dma_buffers[DM75xx_DMA_CHANNELS];

    /**
     * Per DMA channel chaining descriptors
     */

	dm75xx_dma_chain_descriptor_t *dma_chain[DM75xx_DMA_CHANNELS];

    /**
     * Flag used for DMA control
     */

	dm75xx_dma_flag_t dma_flag[DM75xx_DMA_CHANNELS];

};

/**
 * @brief
 *      DM75xx device descriptor type
 */

typedef struct dm75xx_device_descriptor dm75xx_device_descriptor_t;

/**
 * @} DM75xx_Driver_Structures
 */

/*=============================================================================
Forward declarations
 =============================================================================*/

/**
 * @defgroup DM75xx_Driver_Forward_Declarations DM75xx driver forward declarations
 * @{
 */

/**
 * @brief
 *      File operations supported by driver
 */

static struct file_operations dm75xx_file_ops;

/**
 * @} DM75xx_Driver_Forward_Declarations
 */

/*=============================================================================
Function prototypes
 =============================================================================*/

/**
 * @defgroup DM75xx_Driver_Functions DM75xx driver functions
 * @{
 */

/**
*******************************************************************************
@brief
    Read from or write to one of the standard PCI regions.

@param
    dm75xx

    Address of device's DM75xx device descriptor.

@param
    pci_request

    Address of access' PCI request descriptor.

@param
    direction

    Direction of access to PCI region (read from or write to).

@warning
    This function performs no validation on its arguments.  All arguments are
    assumed correct.
 *******************************************************************************
 */

static void dm75xx_access_pci_region(const dm75xx_device_descriptor_t *
				     dm75xx,
				     dm75xx_pci_access_request_t * pci_request,
				     dm75xx_pci_region_access_dir_t direction);

static int
dm75xx_read_pci_region(dm75xx_device_descriptor_t * dm75xx,
		       unsigned long ioctl_param);
static int
dm75xx_write_pci_region(dm75xx_device_descriptor_t * dm75xx,
			unsigned long ioctl_param);
/**
*******************************************************************************
@brief
    Allocate an interrupt line for a DM75xx device.

@param
    dm75xx

    Address of device's DM75xx device descriptor.

@param
    pci_device

    Address of kernel's PCI device structure for the current DM75xx device.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EBUSY      The interrupt line is allocated to another device which
                        requested it as unsharable; returned by request_irq().

        @arg \c
            -EINVAL     The interrupt line is not valid; returned by
                        request_irq().

        @arg \c
            -EINVAL     No interrupt handler is to be associated with the
                        requested interrupt line; returned by request_irq().

        @arg \c
            -ENOMEM     Memory for interrupt action descriptor could not be
                        allocated; returned by request_irq().

@note
    On failure, this function will clean up by releasing any resources allocated
    by the driver to this point.
 *******************************************************************************
 */

static int dm75xx_allocate_irq(dm75xx_device_descriptor_t * dm75xx,
			       const struct pci_dev *pci_device);

/**
********************************************************************************
@brief
    Enable PLX interrupts for the specified DM75xx Device.

@param
    dm75xx

    Address of the devices' DM75xx Device Descriptor.

@param
    enable

    Flag indicating whether or not PLX interrupts should be enabled.  A value of
    zero indicates disable and any other value indicates enable.
********************************************************************************
*/

static void
dm75xx_enable_plx_interrupts(const dm75xx_device_descriptor_t * dm75xx,
			     uint8_t enable);

/**
********************************************************************************
@brief
    Configure PLX Mode register for the specified DMA Channel.

@param
    dm75xx

    Address of the devices' DM75xx Device Descriptor.

@param
    channel

    The DMA Channel to configure.
********************************************************************************
*/

static void
dm75xx_enable_plx_dma(const dm75xx_device_descriptor_t * dm75xx,
		      dm75xx_dma_channel_t channel);

/**
*******************************************************************************
@brief
    Determine whether or not a device is PCI master capable.

@param
    dm75xx

    Address of device's DM75xx device descriptor.

@param
    pci_master

    Address where pci master capable flag should be stored.  Zero will be stored
    if the device is not PCI master capable.  A non-zero value will be stored
    here if the device is PCI master capable.

@note
    PCI Master capability is required for DMA operations.
 *******************************************************************************
 */

static void dm75xx_get_pci_master_status(dm75xx_device_descriptor_t *
					 dm75xx, uint8_t * pci_master);

/**
*******************************************************************************
@brief
    Initialize the device descriptor for the specified DM75xx device.

@param
    dm75xx

    Address of device's DM75xx device descriptor.

@note
    When initializing the device descriptor, the driver will perform the
    following:
    1) Reset interrupt tracking and status variables
    2) Initialize wait queue
    3) Reset DMA information

 *******************************************************************************
 */

static void dm75xx_initialize_device_descriptor(dm75xx_device_descriptor_t *
						dm75xx);

/**
*******************************************************************************
@brief
    Initialize the specified DM75xx device.

@param
    dm75xx

    Address of device's DM75xx device descriptor.

@note
    When initializing a device, the driver will perform the following:
    1) Hardware reset of the board
    2) disables PLX PCI interrupts
    3) disables PLX local interrupt input
    4) disables PLX DMA channel 0/1 interrupts

 *******************************************************************************
 */

static void dm75xx_initialize_hardware(const dm75xx_device_descriptor_t *
				       dm75xx);

/**
*******************************************************************************
@brief
    DM75xx device interrupt handler.

@param
    irq_number

    Interrupt line number.

@param
    device_id

    Address of device's DM75xx device descriptor.  This is set on request_irq()
    call.

@retval
    IRQ_HANDLED

    Interrupt successfully processed;

@retval
    IRQ_NONE

    Interrupt could not be processed;

 *******************************************************************************
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)

INTERRUPT_HANDLER_TYPE dm75xx_interrupt_handler(int irq_number,
						void *device_id,
						struct pt_regs *);

#else

INTERRUPT_HANDLER_TYPE dm75xx_interrupt_handler(int irq_number,
						void *device_id);

#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
/**
*******************************************************************************
@brief
    Process ioctl(2) system calls directed toward a DM75xx device file.

@param
    inode

    Address of kernel's inode descriptor for the device file.  Unused.

@param
    file

    Address of kernel's file descriptor for the device file.

@param
    request_code

    The service being requested.

@param
    ioctl_param

    Third parameter given on ioctl() call.  Depending upon request_code,
    ioctl_param may or may not be used.  Also based upon request_code,
    ioctl_param may be an actual value or may be an address.  If the third
    parameter is not given on the ioctl() call, then ioctl_param has some
    undefined value.

@retval
    0
    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EINVAL     request_code is not valid.

    Please see the descriptions of dm75xx_validate_device(),
    dm75xx_read_pci_region(), dm75xx_write_pci_region(),
    dm75xx_modify_pci_region(), dm75xx_service_dma_function(),
    dm75xx_get_interrupt(), dm75xx_interrupt_control(), and
    dm75xx_board_reset() for information on other possible values
    returned in this case.
 *******************************************************************************
 */
static int dm75xx_ioctl(struct inode *inode,
			struct file *file,
			unsigned int request_code, unsigned long ioctl_param);

#else
/**
*******************************************************************************
@brief
    Process ioctl(2) system calls directed toward a DM75xx device file.

@param
    file

    Address of kernel's file descriptor for the device file.

@param
    request_code

    The service being requested.

@param
    ioctl_param

    Third parameter given on ioctl() call.  Depending upon request_code,
    ioctl_param may or may not be used.  Also based upon request_code,
    ioctl_param may be an actual value or may be an address.  If the third
    parameter is not given on the ioctl() call, then ioctl_param has some
    undefined value.

@retval
    0
    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EINVAL     request_code is not valid.

    Please see the descriptions of dm75xx_validate_device(),
    dm75xx_read_pci_region(), dm75xx_write_pci_region(),
    dm75xx_modify_pci_region(), dm75xx_service_dma_function(),
    dm75xx_get_interrupt(), dm75xx_interrupt_control(), and
    dm75xx_board_reset() for information on other possible values
    returned in this case.
 *******************************************************************************
 */
static long dm75xx_ioctl(struct file *file,
			 unsigned int request_code, unsigned long ioctl_param);
#endif

/**
*******************************************************************************
@brief
    Performs a reset of the board and device descriptor

@param
    dm75xx

    Address of the device's DM75xx device descriptor

 *******************************************************************************
 */

static void dm75xx_board_reset(dm75xx_device_descriptor_t * dm75xx);

/**
*******************************************************************************
@brief
    Performs the actual enable/disable of the interrupt sources

@param
    dm75xx

    Address of the device's DM75xx device descriptor

@param
    source

    A bit mask indicating which interrupt sources to enable/disable

@param
    enable

    flag indicating if we are performing an enable or a disable

 *******************************************************************************
 */

static void
dm75xx_interrupt_enable(dm75xx_device_descriptor_t * dm75xx,
			dm75xx_int_source_t source, uint8_t enable);
/**
*******************************************************************************
@brief
    Control the interrupts on the boards.  This includes enabling, disabling and
    checking the enable/disable status of the interrupts.

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    ioctl_param

    Third parameter given on ioctl() call.  This is the user space address of
    the structure used to pass in the arguments.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n
    The following may be returned:

        @arg \c
            -EFAULT     Copy from user failed

            -ENOSYS     Interrupt control function requested does not exist
 *******************************************************************************
 */

static int
dm75xx_interrupt_control(dm75xx_device_descriptor_t * dm75xx,
			 unsigned long ioctl_param);
/**
*******************************************************************************
@brief
    Returns the top entry from the interrupt status queue.

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    ioctl_param

    Third parameter given on ioctl() call.  This is the user space address of
    the structure used to pass in the arguments.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n
    The following values may be returned:

        @arg \c
            -EFAULT     Copy to user failed

 *******************************************************************************
 */
static int
dm75xx_get_interrupt(dm75xx_device_descriptor_t * dm75xx,
		     unsigned long ioctl_param);

/**
*******************************************************************************
@brief
    Adds an interrupt to the interrupt status queue.

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    interrupt

    Interrupt to add to the queue.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n
    The following values may be returned:

        @arg \c
            -EFAULT     Copy to user failed

 *******************************************************************************
 */
static void
dm75xx_put_interrupt(dm75xx_device_descriptor_t * dm75xx, uint32_t interrupt);

/**
*******************************************************************************
@brief
    Process user space DMA function requests

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    ioctl_param

    Third parameters given on ioctl() call.  This is the user space address of
    the structure used to pass in the arguments.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EFAULT     ioctl_param is not a valid user address

        @arg \c
            -EFAULT     DMA channel or source used to operate upon is not valid.

        @arg \c
            -ENOSYS     DMA function request is not valid

    Please see the descriptions of dm75xx_dma_initialize(), and
    dm75xx_dma_abort() for information on other possible values
    returned in this case.
 *******************************************************************************
 */
static int
dm75xx_service_dma_function(dm75xx_device_descriptor_t * dm75xx,
			    unsigned long ioctl_param);

/**
*******************************************************************************
@brief
    Aborts any DMA transfers on the given channel

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    channel

    The DMA channel on which to cancel any transfers.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EINVAL     The channel entered is invalid

 *******************************************************************************
 */

static int
dm75xx_dma_abort(dm75xx_device_descriptor_t * dm75xx,
		 dm75xx_dma_channel_t channel);

/**
*******************************************************************************
@brief
    Allocates a coherent and consistent buffer for our DMA operations

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    channel

    The DMA channel for which to allocate a buffer.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -ENOMEM     Failed to allocate DMA buffer
@note
    The buffer allocated by this function will be mapped to user space.

@note
    The pages allocated by this function will be reserved in 2.4 kernel.  This
    is done to allow successful userspace mapping.
 *******************************************************************************
 */

static int
dm75xx_dma_alloc_buffer(dm75xx_device_descriptor_t * dm75xx,
			dm75xx_dma_channel_t channel);

/**
*******************************************************************************
@brief
    Performs some DMA initialization work based on the DREQ source

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    channel

    The DMA Channel to perform the initialization for.

@param
    dreq

    The selected DREQ source.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EINVAL     Invalid DREQ source

 *******************************************************************************
 */
static int
dm75xx_dreq_init(dm75xx_device_descriptor_t * dm75xx,
		 dm75xx_dma_channel_t channel, dm75xx_dma_request_t dreq);

/**
*******************************************************************************
@brief
    Initialize DMA for the specified channel and source for the DM75xx device.

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    ioctl_argument

    Address of the kerne's ioctl() request structure.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EAGAIN         DMA has already been initialized.

        @arg \c
            -ENOMEM         Failed to allocate page for DMA chain descriptors.

        @arg \c
            -EOPNOTSUPP     The device is not PCI master capable.

        @arg \c
            -EINVAL         Invalid buffer size requested.

        @arg \c
            -EINVAL         Invalid DMA channel.

        @arg \c
            -EINVAL         Invalid DREQ source.

    Please see the descriptions of dm75xx_dreq_init(), and
    dm75xx_dma_alloc_buffer() for information on other possible values
    returned in this case.

@note
    When initializing DMA, this function: 1) allocates coherent/consistent DMA
    mappings, 2) allocates memory to store DMA buffer allocation information,
    3) allocates memory to link DMA buffers into device's DMA buffer list, 4)
    links all DMA buffers into the device's DMA buffer list, 5) allocates memory
    to link DMA buffers in device's free DMA buffer list, and 6) links all DMA
    buffers into the device's free DMA buffer list.

@note
    Factors beyond the number and size of DMA buffers affect the probability of
    DMA buffer allocation failure.  These factors include the number of
    processes on the system, how much system memory is already in use, and the
    presence of processes (such as the X server) which use a lot of memory.

@note
    System memory can be a scarce resource  Every system entity needs some
    amount of memory.  Memory is being allocated and released all the time.
*******************************************************************************
*/

static int
dm75xx_dma_initialize(dm75xx_device_descriptor_t * dm75xx,
		      dm75xx_ioctl_argument_t * ioctl_argument);

/**
*******************************************************************************
@brief
    Free all coherent/consistent DMA mappings for the given DMA channel on the
    specified DM75xx device.

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    channel

    The DMA channel for which to free the DMA mappings.

@note
    This function also frees the memory allocated to manage the DMA buffer
    allocation information and the DMA buffer lists.
 *******************************************************************************
 */

static void
dm75xx_free_dma_mappings(dm75xx_device_descriptor_t * dm75xx,
			 dm75xx_dma_channel_t channel);
/**
*******************************************************************************
@brief
    Perform all actions necessary to initialize the DM75xx driver and devices.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -ENOMEM     /proc entry creation failed.

    Please see the descriptions of dm75xx_probe_devices() and
    dm75xx_register_char_device() for information on other possible values
    returned in this case.

@note
    On failure, this function will clean up by releasing any resources allocated
    by the driver.

@note
    When loaded, the driver performs a board reset, disables PLX PCI interrupts,
    disables PLX local interrupt input, and disables PLX DMA channel 0/1
    interrupts.
 *******************************************************************************
 */

int dm75xx_load(void);

/**
*******************************************************************************
@brief
    Read an unsigned value from one of a device's PCI regions, modify certain
    bits in the value, and then write it back to the region.

@param
    dm75xx

    Address of device's DM75xx device descriptor.

@param
    ioctl_param

    Third parameter given on ioctl() call.  This is the user space address of
    the structure used to pass in the arguments.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EFAULT     ioctl_param is not a valid user address.

    Please see the description of dm75xx_validate_pci_access() for information
    on other possible values returned in this case.
 *******************************************************************************
 */

static int
dm75xx_modify_pci_region(dm75xx_device_descriptor_t * dm75xx,
			 unsigned long ioctl_param);

/**
*******************************************************************************
@brief
    Prepare a DM75xx device file to be opened and used.

@param
    inode

    Address of kernel's inode descriptor for the device file.

@param
    file

    Address of kernel's file descriptor for the device file.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EBUSY      The device file is already open.

        @arg \c
            -ENODEV     The device's inode does not refer to a valid DM75xx
                        device; 2.4 kernel only.

@note
    When a device is opened, the driver disables & clears all device interrupts,
    enables PLX PCI interrupts, enables PLX local interrupt input, and enables
    PLX DMA channel 0/1 interrupts.
 *******************************************************************************
 */

static int dm75xx_open(struct inode *inode, struct file *file);

/**
*******************************************************************************
@brief
    Determine whether or not a DM75xx device is readable.  This function
    supports the poll(2) and select(2) system calls.

@param
    file

    Address of kernel's file descriptor for the device file.

@param
    poll_table

    Address of kernel's poll table descriptor.  This keeps track of all event
    queues on which the process can wait.

@retval
    status mask

    Bit mask describing the status of the device.@n@n
    The following bits may be set in the mask:
        @arg \c
            POLLPRI will be set if the file descriptor contains an invalid
            device descriptor.

        @arg \c
            POLLIN will be set if an interrupt occurred since the last time the
            interrupt status was read.

        @arg \c
            POLLRDNORM will be set if an interrupt occurred since the last time
            the interrupt status was read.

@note
    A DM75xx device is readable if and only if an interrupt just occurred on the
    device and a process has not yet obtained the interrupt status from it.

@note
    This function is used in the process of waiting until an interrupt occurs on
    a device.

@note
    This function can be executed before an interrupt occurs, which happens if
    something sends a signal to the process.
 *******************************************************************************
 */

static unsigned int dm75xx_poll(struct file *file,
				struct poll_table_struct *poll_table);
/**
*******************************************************************************
@brief
    Probe and set up all DM75xx devices.

@param
    device_count

    Address where DM75xx device count should be stored.  The content of this
    this memory is undefined if the function fails.

@param
    device_descriptors

    Address where address of device descriptor memory should be stored.  The
    content of this memory is undefined if the function fails.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -ENAMETOOLONG       Device name creation failed.

        @arg \c
            -ENODEV             No DM75xx devices found.

        @arg \c
            -ENOMEM             Device descriptor memory allocation failed.

    Please see the descriptions of dm75xx_process_pci_regions(),
    dm75xx_allocate_irq()
    ... for information on other possible values returned in this case.

@note
    If set up of any device fails, then all device set up fails.

@note
    This function allocates memory for the DM75xx device descriptors based upon
    the number of devices found.

@note
    On failure, this function will clean up by releasing any resources allocated
    by the driver to this point.
 *******************************************************************************
 */

static int dm75xx_probe_devices(uint32_t * device_count,
				dm75xx_device_descriptor_t **
				device_descriptors);

/**
*******************************************************************************
@brief
    For each of the standard PCI regions, get the region's base address and
    length from kernel PCI resource information set up at boot.  Also, remap
    any memory-mapped region into the kernel's virtual address space.

@param
    dm75xx

    Address of device's DM75xx device descriptor.

@param
    pci_device

    Address of kernel's PCI device structure for the current DM75xx device.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EBUSY      I/O port or I/O memory range allocation failed.

        @arg \c
            -EIO        A region's resource flags are not valid.

        @arg \c
            -ENOMEM     Remapping a memory-mapped region into the kernel's
                        virtual address space failed.

@note
    Currently, only BAR0 through BAR2 are used.  BAR0 is the memory-mapped PLX
    DMA register region.  BAR1 is the I/O-mapped PLX DMA register region.  BAR2
    is the memory-mapped FPGA register region.

@note
    On failure, this function will clean up by releasing any resources allocated
    by the driver to this point.
 *******************************************************************************
 */

static int dm75xx_process_pci_regions(dm75xx_device_descriptor_t *
				      dm75xx, const struct pci_dev *pci_device);

/**
*******************************************************************************
@brief
    Register the DM75xx character device and request dynamic allocation of a
    character device major number.

@param
    major

    Address where character device major number should be stored.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EBUSY      A character device major number could not be allocated;
                        returned by alloc_chrdev_region().

        @arg \c
            -EBUSY      All character device major numbers are in use; returned
                        by register_chrdev().

        @arg \c
            -ENOMEM     Memory allocation failed; returned by
                        alloc_chrdev_region().

@note
    This function hides the character device interface differences between 2.4
    and 2.6 kernels.
 *******************************************************************************
 */

static int dm75xx_register_char_device(int *major);

/**
*******************************************************************************
@brief
    Do all processing necessary after the last reference to a DM75xx device
    file is released elsewhere in the kernel.

@param
    inode

    Address of kernel's inode descriptor for the device file.  Unused.

@param
    file

    Address of kernel's file descriptor for the device file.

@retval
    0

    Success.

@retval
    < 0

    Failure.  Please see the description of dm75xx_validate_device() for
    information on possible values returned in this case.

@note
    When a device is released, the driver disables PLX PCI interrupts, disables
    PLX local interrupt input, and disables PLX DMA channel 0/1 interrupts.
 *******************************************************************************
 */

static int dm75xx_release(struct inode *inode, struct file *file);

/**
*******************************************************************************
@brief
    Release any resources allocated by the driver.

@note
    This function is called both at module unload time and when the driver is
    cleaning up after some error occurred.
 *******************************************************************************
 */

static void dm75xx_release_resources(void
    );

/**
*******************************************************************************
@brief
    Perform all actions necessary to deinitialize the DM75xx driver and devices.
 *******************************************************************************
 */

void dm75xx_unload(void
    );

/**
*******************************************************************************
@brief
    Unregister the DM75xx character device and free the character device major
    number.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EINVAL     Character major number is not valid; returned by
                        unregister_chrdev(); 2.4 kernel only.

            -EINVAL     Character major number has no file operations registered
                        for it; returned by unregister_chrdev(); 2.4 kernel
                        only.

            -EINVAL     Device name specified when character major number was
                        registered does not match the name being unregistered;
                        returned by unregister_chrdev(); 2.4 kernel only.

@note
    This function hides the character device interface differences between 2.4
    and 2.6 kernels.

@note
    This function does not fail on 2.6 kernels.
 *******************************************************************************
 */

static int dm75xx_unregister_char_device(void
    );

/**
*******************************************************************************
@brief
    Given what is assumed to be the address of a DM75xx device descriptor, make
    sure it corresponds to a valid DM75xx device descriptor.

@param
    dm75xx

    Address of device descriptor to be verified.

@retval
    0

    Success.

@retval
    < 0

    Failure.@n@n
    The following values may be returned:
        @arg \c
            -EBADFD     dm75xx is not a valid DM75xx device descriptor
                        address.
 *******************************************************************************
 */

static int dm75xx_validate_device(const dm75xx_device_descriptor_t * dm75xx);

/**
*******************************************************************************
@brief
    Validate a user-space access to one of the device's PCI regions.

@param
    dm75xx

    Address of the device's DM75xx device descriptor.

@param
    pci_request

    Address of PCI region access request descriptor.

@retval
    0

    Success

@retval
    < 0

    Failure.@n@n

    The following values may be returned:
        @arg \c
            -EINVAL         The PCI region is not vaild.

        @arg \c
            -EMSGSIZE       The access size is not valid.

        @arg \c
            -EOPNOTSUPP     The PCI region offset is valid but is not suitably
                            aligned for the number of bytes to be accessed.

        @arg \c
            -ERANGE         The PCI region offset is not valid.

@note
    This function accesses information in the device descriptor.  Therefore,
    the device descriptor spin lock should be held when this function is called.
*******************************************************************************
 */

static int dm75xx_validate_pci_access(const dm75xx_device_descriptor_t *
				      dm75xx,
				      const dm75xx_pci_access_request_t *
				      pci_request);
/**
 *******************************************************************************
@brief
    Measure the size of the fifo by filling it until it is half-full than
    doubling that value to get the size of the fifo.
@param
    dm75xx

    Address of the device descriptor.

@param
    size

    Address of the variable to store the size once it is found

@retval
    0

    Success.

@retval
    < 0

    Failure
 *******************************************************************************
 */
static int
dm75xx_get_fifo_size(dm75xx_device_descriptor_t * dm75xx, unsigned int *size);

static int dm75xx_mmap(struct file *file, struct vm_area_struct *vma);

/**
 * @} DM75xx_Driver_Functions
 */

/**
 * @} DM75xx_Library_Header
 */

#endif /* __dm75xx_driver_h__ */
