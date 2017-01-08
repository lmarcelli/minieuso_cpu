/**
    @file

    @brief
        Low level ioctl() request descriptor structure and request code
        definitions

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

	$Id: dm75xx_ioctl.h 65466 2012-12-04 19:25:44Z rgroner $
*/

#ifndef __dm75xx_ioctl_h__
#define __dm75xx_ioctl_h__

#include <linux/ioctl.h>
#include <linux/types.h>

#include "dm75xx_types.h"

/**
 * @defgroup DM75xx_Ioctl_Header DM75xx ioctl header file
 * @{
 */

/*=============================================================================
Enumerations
 =============================================================================*/

/**
 * @defgroup DM75xx_Ioctl_Enumerations DM75xx ioctl enumerations
 * @{
 */
/**
 * @brief
 *      Functions supported by driver DMA management system
 */
typedef enum dm75xx_dma_manage_function {
    /**
     * DMA initialization
     */
	DM75xx_DMA_FUNCTION_INITIALIZE = 0,
    /**
     * DMA abort
     */
	DM75xx_DMA_FUNCTION_ABORT
} dm75xx_dma_manage_function_t;

/**
 * @brief
 *      Functions supported by driver interrupt control system
 */
typedef enum dm75xx_int_control_function {
    /**
     * Enable Interrupts
     */
	DM75xx_INT_CONTROL_ENABLE = 0,
    /**
     * Disable Interrupts
     */
	DM75xx_INT_CONTROL_DISABLE,
    /**
     * Returns with a value indicating which interrupts are currently enabled.
     * This value should then be checked with DM75xx_INTERRUPT_ACTIVE().
     */
	DM75xx_INT_CONTROL_CHECK
} dm75xx_int_control_function_t;
/**
 * @} DM75xx_Ioctl_Enumerations
 */

/*=============================================================================
Structures
 =============================================================================*/

/**
 * @defgroup DM75xx_Ioctl_Structures DM75xx ioctl structures
 * @{
 */

/**
 * @brief
 *      ioctl() request structure for read from or write to PCI region
 */

struct dm75xx_ioctl_region_readwrite {

    /**
     * PCI region access request
     */

	dm75xx_pci_access_request_t access;
};

/**
* typedef for the PCI region access request type
*/
typedef struct dm75xx_ioctl_region_readwrite dm75xx_ioctl_region_readwrite_t;

/**
 * @brief
 *      ioctl() request structure for PCI region read/modify/write
 */

struct dm75xx_ioctl_region_modify {

    /**
     * PCI region access request
     */

	dm75xx_pci_access_request_t access;

    /**
     * Bit mask that controls which bits can be modified.  A zero in a bit
     * position means that the corresponding register bit should not be
     * modified.  A one in a bit position means that the corresponding register
     * bit should be modified.
     *
     * Note that it's possible to set bits outside of the mask depending upon
     * the register value before modification.  When processing the associated
     * request code, the driver will silently prevent this from happening but
     * will not return an indication that the mask or new value was incorrect.
     */

	union {

	/**
	 * Mask for 8-bit operations
	 */

		uint8_t mask8;

	/**
	 * Mask for 16-bit operations
	 */

		uint16_t mask16;

	/**
	 * Mask for 32-bit operations
	 */

		uint32_t mask32;
	} mask;
};

/**
 * @brief
 *      ioctl() PCI region read/modify/write request descriptor type
 */

typedef struct dm75xx_ioctl_region_modify dm75xx_ioctl_region_modify_t;

/**
 * @brief
 *      ioctl() request structure for performing a DMA function
 */

struct dm75xx_ioctl_dma_function {

    /**
     * DMA Local FIFO Source
     */

	dm75xx_dma_source_t source;

    /**
     * The DMA Channel on which to perform the specified operation
     */

	dm75xx_dma_channel_t channel;

    /**
     * Demand mode request source (DREQ)
     */

	dm75xx_dma_request_t request;

    /**
     * Contains the transfer size for the DMA channel
     */

	uint32_t size;

    /**
     * PCI Address
     */

	uint32_t pci_address;

    /**
     * Flag indicating whether or not this transfer is to an arbitrary address
     */
	uint8_t arb;

    /**
     * DMA function to perform
     */

	dm75xx_dma_manage_function_t function;

};

/**
 * @brief
 *      ioctl() request structure for performing a DMA function type.
 */

typedef struct dm75xx_ioctl_dma_function dm75xx_ioctl_dma_function_t;

/**
 * @brief
 *      ioctl() request structure for interrupt control.
 */

struct dm75xx_ioctl_int_control {

    /**
     * Interrupt Sources
     */

	dm75xx_int_source_t source;

    /**
     * Interrupt function to perofmr
     */

	dm75xx_int_control_function_t function;
};

/**
 * @brief
 *      ioctl() request structure for interrupt control.
 */

typedef struct dm75xx_ioctl_int_control dm75xx_ioctl_int_control_t;

/**
 * @brief
 *      ioctl() request structure encapsulating all possible requests.  This is
 *      what gets passed into the kernel from user space on the ioctl() call.
 */

union dm75xx_ioctl_argument {

    /**
     * PCI region read and write
     */

	dm75xx_ioctl_region_readwrite_t readwrite;

    /**
     * PCI region read/modify/write
     */

	dm75xx_ioctl_region_modify_t modify;

    /**
     * DMA management function
     */

	dm75xx_ioctl_dma_function_t dma_function;

    /**
     * Interrupt control function
     */

	dm75xx_ioctl_int_control_t int_control;
};

/**
 * @brief
 *      ioctl() request descriptor type
 */

typedef union dm75xx_ioctl_argument dm75xx_ioctl_argument_t;

/**
 * @} DM75xx_Ioctl_Structures
 */

/*=============================================================================
Macros
 =============================================================================*/

/**
 * @defgroup DM75xx_Ioctl_Macros DM75xx ioctl macros
 * @{
 */

/**
 * @brief
 *      Unique 8-bit value used to generate unique ioctl() request codes
 */

#define DM75xx_IOCTL_MAGIC		'D'

/**
 * @brief
 *      First ioctl() request number
 */

#define DM75xx_IOCTL_REQUEST_BASE	0x00

/**
 * @brief
 *      ioctl() request code for reading from a PCI region
 */

#define DM75xx_IOCTL_REGION_READ \
    _IOWR( \
	DM75xx_IOCTL_MAGIC, \
	(DM75xx_IOCTL_REQUEST_BASE + 1), \
	dm75xx_ioctl_argument_t \
    )

/**
 * @brief
 *      ioctl() request code for writing to a PCI region
 */

#define DM75xx_IOCTL_REGION_WRITE \
    _IOW( \
	DM75xx_IOCTL_MAGIC, \
	(DM75xx_IOCTL_REQUEST_BASE + 2), \
	dm75xx_ioctl_argument_t \
    )

/**
 * @brief
 *      ioctl() request code for PCI region read/modify/write
 */

#define DM75xx_IOCTL_REGION_MODIFY \
    _IOW( \
	DM75xx_IOCTL_MAGIC, \
	(DM75xx_IOCTL_REQUEST_BASE + 3), \
	dm75xx_ioctl_argument_t \
    )

/**
 * @brief
 *      ioctl() request code for DMA function
 */

#define DM75xx_IOCTL_DMA_FUNCTION \
    _IOW( \
	DM75xx_IOCTL_MAGIC, \
	(DM75xx_IOCTL_REQUEST_BASE + 4), \
	dm75xx_ioctl_argument_t \
    )

/**
 * @brief
 * ioctl() request code to wake up a sleeping driver function
 */

#define DM75xx_IOCTL_WAKEUP \
    _IOW( \
    DM75xx_IOCTL_MAGIC, \
    (DM75xx_IOCTL_REQUEST_BASE + 5), \
    dm75xx_ioctl_argument_t \
    )

/**
 * @brief
 * ioctl() request code to get the interrupt status queue
 */

#define DM75xx_IOCTL_INT_STATUS \
    _IOWR( \
    DM75xx_IOCTL_MAGIC, \
    (DM75xx_IOCTL_REQUEST_BASE + 6), \
    dm75xx_int_status_t \
    )

/**
 * @brief
 * ioctl() request code to get the fifo size
 */

#define DM75xx_IOCTL_GET_FIFO_SIZE \
    _IOWR( \
    DM75xx_IOCTL_MAGIC, \
    (DM75xx_IOCTL_REQUEST_BASE + 7), \
    unsigned int \
    )

/**
 * @brief
 * ioctl() request code to get the board type
 */

#define DM75xx_IOCTL_GET_BOARD_TYPE \
    _IOWR( \
    DM75xx_IOCTL_MAGIC, \
    (DM75xx_IOCTL_REQUEST_BASE + 8), \
    dm75xx_board_t \
    )

/**
 * @brief
 * ioctl() request code to control interrupts
 */

#define DM75xx_IOCTL_INT_CONTROL \
    _IOWR( \
    DM75xx_IOCTL_MAGIC, \
    (DM75xx_IOCTL_REQUEST_BASE + 9), \
    dm75xx_ioctl_int_control_t \
    )

/**
 * @brief
 * ioctl() request code to reset the board
 */

#define DM75xx_IOCTL_RESET \
    _IOWR( \
    DM75xx_IOCTL_MAGIC, \
    (DM75xx_IOCTL_REQUEST_BASE + 10), \
    dm75xx_ioctl_argument_t \
    )
/**
 * @brief
 * ioctl() request code to control DMA buffer status
 */
#define DM75xx_IOCTL_RESET_DMA_STATUS \
    _IOWR( \
    DM75xx_IOCTL_MAGIC, \
    (DM75xx_IOCTL_REQUEST_BASE + 11), \
    dm75x_dma_reset_t \
    )
/**
 * @} DM75xx_Ioctl_Macros
 */

/**
 * @} DM75xx_Ioctl_Header
 */

#endif /* __dm75xx_ioctl_h__ */
