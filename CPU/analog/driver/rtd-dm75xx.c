
/**
    @file

    @brief
        DM75xx driver source code

//----------------------------------------------------------------------------
//  COPYRIGHT (C) RTD EMBEDDED TECHNOLOGIES, INC.  ALL RIGHTS RESERVED.
//
//  This software package is dual-licensed.  Source code that is compiled for
//  kernel mode execution is licensed under the GNU General Public License
//  version 2.  For a copy of this license, refer to the file
//  LICENSE_GPLv2.TXT (which should be included with this software) or contact
//  the Free Software Foundation.  Source code that is compiled for user mode
//  execution is licensed under the RTD End-User Software License Agreement.
//  For a copy of this license, refer to LICENSE.TXT or contact RTD Embedded
//  Technologies, Inc.  Using this software indicates agreement with the
//  license terms listed above.
//----------------------------------------------------------------------------

    $Id: rtd-dm75xx.c 99118 2016-04-28 12:54:37Z rgroner $
*/

#include <linux/sched.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>

#include "dm75xx_driver.h"
#include "dm75xx_ioctl.h"
#include "dm75xx_registers.h"
#include "dm75xx_types.h"
#include "dm75xx_kernel.h"

/*=============================================================================
Driver documentation
 =============================================================================*/

#define DRIVER_COPYRIGHT \
        "Copyright (C) RTD Embedded Technologies, Inc.  All Rights Reserved."

#define DRIVER_DESCRIPTION "DM75xx device driver"

#define DRIVER_NAME "rtd-dm75xx"

#define DRIVER_RELEASE "2.0.0"

MODULE_AUTHOR(DRIVER_COPYRIGHT);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_LICENSE("GPL");

/*=============================================================================
Global variables
 =============================================================================*/

/**
 * Character device major number; dynamically assigned
 */

static int dm75xx_major;

/**
 * DM75xx device descriptors
 */

static dm75xx_device_descriptor_t *dm75xx_devices;

/**
 * Character device descriptor for 2.6 kernels.
 */
static struct cdev dm75xx_cdev;

/**
 * Number of devices detected during probe
 */

static uint32_t dm75xx_device_count;

/**
 * Device class pointer
 */
static struct class *dev_class = NULL;

/**
 * Table of devices supported by the driver.  This array is used by
 * dm75xx_probe_devices() to walk the entire PCI device list looking for DM75xx
 * devices.  The table is terminated by a "NULL" entry.
 *
 * The individual structures in this array are set up using ANSI C standard
 * format initialization (which is the preferred method in 2.6 kernels) instead
 * of tagged initialization (which is the preferred method in 2.4 kernels).
 */

static const struct pci_device_id dm75xx_pci_device_table[] = {
	{
	 .vendor = RTD_PCI_VENDOR_ID,
	 .device = DM7520_PCI_DEVICE_ID,
	 .subvendor = PCI_ANY_ID,
	 .subdevice = PCI_ANY_ID,
	 .class = 0,
	 .class_mask = 0,
	 .driver_data = 0},
	{
	 .vendor = RTD_PCI_VENDOR_ID,
	 .device = DM7540_PCI_DEVICE_ID,
	 .subvendor = PCI_ANY_ID,
	 .subdevice = PCI_ANY_ID,
	 .class = 0,
	 .class_mask = 0,
	 .driver_data = 0},
	{
	 .vendor = 0,
	 .device = 0,
	 .subvendor = 0,
	 .subdevice = 0,
	 .class = 0,
	 .class_mask = 0,
	 .driver_data = 0}
};

static struct file_operations dm75xx_file_ops = {
	.owner = THIS_MODULE,
	.llseek = NULL,
	.read = NULL,
	.write = NULL,
	.poll = dm75xx_poll,
	DM75XX_IOCTL = dm75xx_ioctl,
	.mmap = dm75xx_mmap,
	.open = dm75xx_open,
	.flush = NULL,
	.release = dm75xx_release,
	.fsync = NULL,
	.fasync = NULL,
	.lock = NULL,
	.sendpage = NULL,
	.get_unmapped_area = NULL
};

/*=============================================================================
Driver functions
 =============================================================================*/

/******************************************************************************
Access a standard PCI region
 ******************************************************************************/

static void
dm75xx_access_pci_region(const dm75xx_device_descriptor_t * dm75xx,
			 dm75xx_pci_access_request_t * pci_request,
			 dm75xx_pci_region_access_dir_t direction)
{
	unsigned long address;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Compute the address to be accessed
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	address = (unsigned long)pci_request->offset;

	if (dm75xx->pci[pci_request->region].virt_addr != NULL) {
		address += (unsigned long)
		    dm75xx->pci[pci_request->region].virt_addr;
	} else {
		address +=
		    (unsigned long)dm75xx->pci[pci_request->region].io_addr;
	}

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Determine whether access is a read or write
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	if (direction == DM75xx_PCI_REGION_ACCESS_READ) {

		/*
		 * Region is to be read
		 */

		/*#####################################################################
		   Determine whether the region is memory or I/O mapped
		   ################################################################## */

		if (dm75xx->pci[pci_request->region].virt_addr != NULL) {

			/*
			 * Region is memory mapped
			 */

			/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			   Determine how many bits are to be accessed
			   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

			switch (pci_request->size) {
			case DM75xx_PCI_REGION_ACCESS_8:
				pci_request->data.data8 =
				    IO_MEMORY_READ8((unsigned long *)address);
				break;

			case DM75xx_PCI_REGION_ACCESS_16:
				pci_request->data.data16 =
				    IO_MEMORY_READ16((unsigned long *)address);
				break;

			case DM75xx_PCI_REGION_ACCESS_32:
				pci_request->data.data32 =
				    IO_MEMORY_READ32((unsigned long *)address);
				break;
			}
		} else {

			/*
			 * Region is I/O mapped
			 */

			/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			   Determine how many bits are to be accessed
			   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

			switch (pci_request->size) {
			case DM75xx_PCI_REGION_ACCESS_8:
				pci_request->data.data8 =
				    (uint8_t) inb(address);
				break;

			case DM75xx_PCI_REGION_ACCESS_16:
				pci_request->data.data16 =
				    (uint16_t) inw(address);
				break;

			case DM75xx_PCI_REGION_ACCESS_32:
				pci_request->data.data32 =
				    (uint32_t) inl(address);
				break;
			}
		}
	} else {

		/*
		 * Region is to be written
		 */

		/*#####################################################################
		   Determine whether the region is memory or I/O mapped
		   ################################################################## */

		if (dm75xx->pci[pci_request->region].virt_addr != NULL) {

			/*
			 * Region is memory mapped
			 */

			/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			   Determine how many bits are to be accessed
			   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

			switch (pci_request->size) {
			case DM75xx_PCI_REGION_ACCESS_8:
				IO_MEMORY_WRITE8(pci_request->data.data8,
						 (unsigned long *)address);
				break;

			case DM75xx_PCI_REGION_ACCESS_16:
				IO_MEMORY_WRITE16(pci_request->data.data16,
						  (unsigned long *)address);
				break;

			case DM75xx_PCI_REGION_ACCESS_32:
				IO_MEMORY_WRITE32(pci_request->data.data32,
						  (unsigned long *)address);
				break;
			}
		} else {

			/*
			 * Region is I/O mapped
			 */

			/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			   Determine how many bits are to be accessed
			   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

			switch (pci_request->size) {
			case DM75xx_PCI_REGION_ACCESS_8:
				outb(pci_request->data.data8, address);
				break;

			case DM75xx_PCI_REGION_ACCESS_16:
				outw(pci_request->data.data16, address);
				break;

			case DM75xx_PCI_REGION_ACCESS_32:
				outl(pci_request->data.data32, address);
				break;
			}
		}
	}
}

/******************************************************************************
Read from a PCI region
 ******************************************************************************/
static int
dm75xx_read_pci_region(dm75xx_device_descriptor_t * dm75xx,
		       unsigned long ioctl_param)
{
	dm75xx_ioctl_argument_t ioctl_argument;
	unsigned long irq_flags;
	int status;
	/*
	 * Copy arguments from user space.
	 */

	if (copy_from_user
	    (&ioctl_argument, (dm75xx_ioctl_argument_t *) ioctl_param,
	     sizeof(dm75xx_ioctl_argument_t))) {
		return -EFAULT;
	}

	/*
	 * validate the parameters
	 */
	status =
	    dm75xx_validate_pci_access(dm75xx,
				       &(ioctl_argument.readwrite.access)
	    );
	if (status != 0) {
		return status;
	}

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	/*
	 * Perform the read
	 */
	dm75xx_access_pci_region(dm75xx,
				 &(ioctl_argument.readwrite.access),
				 DM75xx_PCI_REGION_ACCESS_READ);

	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
	/*
	 * Send results back to user space.
	 */
	if (copy_to_user((dm75xx_ioctl_argument_t *) ioctl_param,
			 &ioctl_argument, sizeof(dm75xx_ioctl_argument_t))) {
		return -EFAULT;
	}
	return 0;
}

/******************************************************************************
Validate user-space PCI region access
 ******************************************************************************/

static int
dm75xx_validate_pci_access(const dm75xx_device_descriptor_t * dm75xx,
			   const dm75xx_pci_access_request_t * pci_request)
{
	uint16_t align_mask;
	uint8_t access_bytes;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Validate the data size
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	/*
	 * Verify the data size in bits.  Set the number of bytes being accessed;
	 * this is used to determine whether or not the region offset actually lies
	 * within the region.  Set the offset alignment bit mask; this is used to
	 * determine whether the region offset is suitably aligned for the access.
	 */

	switch (pci_request->size) {
	case DM75xx_PCI_REGION_ACCESS_8:
		access_bytes = 1;
		align_mask = 0x0;
		break;

	case DM75xx_PCI_REGION_ACCESS_16:
		access_bytes = 2;
		align_mask = 0x1;
		break;

	case DM75xx_PCI_REGION_ACCESS_32:
		access_bytes = 4;
		align_mask = 0x3;
		break;

	default:
		return -EMSGSIZE;
		break;
	}

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Validate the PCI region
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	switch (pci_request->region) {
	case DM75xx_PLX_MEM:
	case DM75xx_PLX_IO:
	case DM75xx_LAS0:
	case DM75xx_LAS1:
		break;

	default:
		printk(KERN_ERR "%s: Invalid PCI region (%d).\n", DRIVER_NAME,
		       pci_request->region);
		return -EINVAL;

		break;
	}

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Validate the PCI region offset
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	/*
	 * All bytes being accessed must lie entirely within the PCI region
	 */

	if (pci_request->offset
	    > (dm75xx->pci[pci_request->region].length - access_bytes)
	    ) {
		return -ERANGE;
	}

	/*
	 * Offset where access will occur must be suitably aligned for the size of
	 * access
	 */

	if (pci_request->offset & align_mask) {
		return -EOPNOTSUPP;
	}

	return 0;
}

/******************************************************************************
Write to a PCI region
 ******************************************************************************/
static int
dm75xx_write_pci_region(dm75xx_device_descriptor_t * dm75xx,
			unsigned long ioctl_param)
{
	dm75xx_ioctl_argument_t ioctl_argument;
	unsigned long irq_flags;
	int status;

	/*
	 * Copy arguments from user space
	 */
	if (copy_from_user
	    (&ioctl_argument, (dm75xx_ioctl_argument_t *) ioctl_param,
	     sizeof(dm75xx_ioctl_argument_t))) {
		return -EFAULT;
	}

	status =
	    dm75xx_validate_pci_access(dm75xx,
				       &(ioctl_argument.readwrite.access));

	if (status != 0) {
		return status;
	}

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	/*
	 * Perform the write
	 */
	dm75xx_access_pci_region(dm75xx,
				 &(ioctl_argument.readwrite.access),
				 DM75xx_PCI_REGION_ACCESS_WRITE);
	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
	return 0;
}

/******************************************************************************
IRQ line allocation
 ******************************************************************************/

static int
dm75xx_allocate_irq(dm75xx_device_descriptor_t * dm75xx,
		    const struct pci_dev *pci_device)
{
	int status;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_allocate_irq\n", &((dm75xx->name)[0]));
#endif

	/*
	 * The fourth request_irq() argument MUST refer to memory which will remain
	 * valid until the driver is unloaded.  request_irq() simply stores this
	 * address in a structure rather than making a copy of the string it points
	 * to.
	 */

	status = request_irq(pci_device->irq,
			     (irq_handler_t) dm75xx_interrupt_handler,
			     IRQF_SHARED, dm75xx->name, dm75xx);

	if (status != 0) {
		printk(KERN_ERR
		       "%s> ERROR: Unable to allocate IRQ %u (error = %u)\n",
		       dm75xx->name, pci_device->irq, -status);
		return status;
	}

	dm75xx->irq_number = pci_device->irq;

	printk(KERN_INFO "%s> Allocated IRQ %u\n",
	       dm75xx->name, pci_device->irq);

	return 0;
}

/*******************************************************************************
 Control PLX Interrupts
 ******************************************************************************/
static void
dm75xx_enable_plx_interrupts(const dm75xx_device_descriptor_t * dm75xx,
			     uint8_t enable)
{
	dm75xx_pci_access_request_t pci_request;

#ifdef DM75XX_DEBUG
	if (enable) {
		printk(KERN_DEBUG "%s: dm75xx_enable_plx_interrupts (enable)\n",
		       &((dm75xx->name)[0]));
	} else {
		printk(KERN_DEBUG
		       "%s: dm75xx_enable_plx_interrupts (DISABLE)\n",
		       &((dm75xx->name)[0]));
	}
#endif

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = DM75xx_PLX_ITCSR;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = 0x00000000;

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_READ);
	/*
	 * Preserve all bits except bits 8, 11, 18 and 19.  These are the bits for
	 * PCI IT enable, PCI Local IT enable, and DMA 0 and DMA 1 IT Enable.
	 */

	pci_request.data.data32 &= ~0x000C0900;

	/*
	 * If PLX interrupts should be enabled, set bits 8, 11
	 */

	if (enable) {
		pci_request.data.data32 |= 0x00000900;
	}

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = DM75xx_PLX_ITCSR;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);
}

/*******************************************************************************
 Control PLX DMA Mode
 ******************************************************************************/
static void
dm75xx_enable_plx_dma(const dm75xx_device_descriptor_t * dm75xx,
		      dm75xx_dma_channel_t channel)
{
	dm75xx_pci_access_request_t pci_request;
	uint16_t offset;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_enable_plx_dma\n", &((dm75xx->name)[0]));
#endif

	if (channel == DM75xx_DMA_CHANNEL_0) {
		offset = DM75xx_PLX_DMA_MODE0;
	} else {
		offset = DM75xx_PLX_DMA_MODE1;
	}

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = offset;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = 0x00000000;

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_READ);
	/*
	 * Mask reserved bits
	 */
	pci_request.data.data32 &= ~0x0003FFFF;
	/*
	 * Set the desired bits for...
	 * Local Bus Width - 16 bits
	 * Ready Input enable
	 * Continuous Burst
	 * Local Burst
	 * Done Interrupt Enabled
	 * Local Addressing Mode Constant
	 * PCI Interrupt Enabled
	 */
	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = offset;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;

	if (dm75xx->dma_flag[channel] & DM75xx_DMA_FLAG_ARB) {
		pci_request.data.data32 |= PLX_DMA_CONFIG | PLX_DMA_DEMAND_MODE;
	} else {
		pci_request.data.data32 |=
		    PLX_DMA_CONFIG | PLX_DMA_CHAINING | PLX_DMA_DEMAND_MODE;
	}

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);
}

/******************************************************************************
Determine PCI master status
 ******************************************************************************/

static void
dm75xx_get_pci_master_status(dm75xx_device_descriptor_t * dm75xx,
			     uint8_t * pci_master)
{
	dm75xx_pci_access_request_t pci_request;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_get_pci_master_status\n",
	       &((dm75xx->name)[0]));
#endif

	pci_request.data.data8 = 0x0000;
	pci_request.region = DM75xx_LAS0;
	pci_request.offset = DM75xx_LAS0_MT_MODE;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_8;

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_READ);

	if (pci_request.data.data8 & 0x0001) {
		*pci_master = 0xFF;
	} else {
		*pci_master = 0x00;
	}
}

/******************************************************************************
DM75xx device descriptor initialization
 ******************************************************************************/

static void
dm75xx_initialize_device_descriptor(dm75xx_device_descriptor_t * dm75xx)
{

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_initialize_device_descriptor\n",
	       DRIVER_NAME);
#endif

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Interrupt status initialization
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	dm75xx->int_count = 0;
	dm75xx->int_queue_in = 0;
	dm75xx->int_queue_out = 0;
	dm75xx->int_queue_missed = 0;

	dm75xx->remove_isr_flag = 0x00;
	init_waitqueue_head(&(dm75xx->int_wait_queue));

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   DMA initialization
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	dm75xx->dma_flag[DM75xx_DMA_CHANNEL_0] = 0x00;
	dm75xx->dma_flag[DM75xx_DMA_CHANNEL_1] = 0x00;
	dm75xx->dma_size[DM75xx_DMA_CHANNEL_0] = 0x00;
	dm75xx->dma_size[DM75xx_DMA_CHANNEL_1] = 0x00;
	dm75xx->dma_chain[DM75xx_DMA_CHANNEL_0] = NULL;
	dm75xx->dma_chain[DM75xx_DMA_CHANNEL_1] = NULL;

}

static void
dm75xx_initialize_hardware(const dm75xx_device_descriptor_t * dm75xx)
{
	dm75xx_pci_access_request_t pci_request;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Generic device initialization
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	/*
	 * Reset the board
	 * Write a dummy value to the reset register to reset the board to default
	 * settings.
	 */

	pci_request.region = DM75xx_LAS0;
	pci_request.offset = DM75xx_LAS0_BOARD_RESET;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = NO_ARG;

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   PLX Interrupt Initialization
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	dm75xx_enable_plx_interrupts(dm75xx, 0x00);

}

/******************************************************************************
DM75xx interrupt handler
 ******************************************************************************/

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)

INTERRUPT_HANDLER_TYPE dm75xx_interrupt_handler(int irq_number,
						void *device_id,
						struct pt_regs *registers)
#else

INTERRUPT_HANDLER_TYPE dm75xx_interrupt_handler(int irq_number, void *device_id)
#endif
{
	dm75xx_device_descriptor_t *dm75xx;
	unsigned long address;
	uint32_t plx_intcsr;
	uint32_t dma_interrupt;
	uint32_t status = 0x00000000;
	uint16_t reset_reg;
	uint16_t pci_reg;
	uint16_t csr_reg;
	uint16_t data16;
	uint16_t status_bit;
	uint8_t data8;
	uint8_t i;

	dm75xx = (dm75xx_device_descriptor_t *) device_id;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Sanity checking
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Make sure we have a valid DM75xx device descriptor
	 */
	if (dm75xx_validate_device(dm75xx) != 0) {
		printk(KERN_ERR
		       "%s> ERROR: Invalid device descriptor in interrupt.\n",
		       &((dm75xx->name)[0])
		    );
		return IRQ_NONE;
	}

	/*
	 * We have a valid device descriptor, so lock it up for multiprocessor
	 * protection.
	 */
	spin_lock(&(dm75xx->lock));

	/*
	 * Check PLX Interrupt Status Register
	 */
	plx_intcsr = IO_MEMORY_READ32(dm75xx->pci[DM75xx_PLX_MEM].virt_addr +
				      DM75xx_PLX_ITCSR);
	/*
	 * If neither a PLX local interrupt nor a PLX DMA interrupt is pending
	 * than it must have been some other device that caused the interrupt.
	 */
	if (!(plx_intcsr & 0x00608000)) {
		spin_unlock(&(dm75xx->lock));
		return IRQ_NONE;
	}
	/*
	 * Make sure kernel's interrupt line number matches the one allocated for
	 * this device
	 */
	if (irq_number != dm75xx->irq_number) {
		spin_unlock(&(dm75xx->lock));
		printk(KERN_ERR "%s> ERROR: Interrupt on wrong IRQ line: "
		       "expected %d, but got %d.\n",
		       &((dm75xx->name)[0]), dm75xx->irq_number, irq_number);
		return IRQ_NONE;
	}
	/*
	 * This is our interrupt
	 */

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: Interrupt!\n", &((dm75xx->name)[0]));
#endif

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Check for DMA Done interrupts
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	for (i = 0; i <= 1; i++) {

		if (i == 0) {
			dma_interrupt = DM75xx_INT_DMA_0;
			pci_reg = DM75xx_PLX_DMA_PADR0;
			reset_reg = DM75xx_LAS0_DMA_RSTRQST0;
			csr_reg = DM75xx_PLX_DMA_CSR0;
		} else {
			dma_interrupt = DM75xx_INT_DMA_1;
			pci_reg = DM75xx_PLX_DMA_PADR1;
			reset_reg = DM75xx_LAS0_DMA_RSTRQST1;
			csr_reg = DM75xx_PLX_DMA_CSR1;
		}

		if (plx_intcsr & dma_interrupt) {
			/*
			 * Read the PLX DMA Control/Status registers
			 */
			address = csr_reg;
			address +=
			    (unsigned long)dm75xx->
			    pci[DM75xx_PLX_MEM].virt_addr;

			data8 = IO_MEMORY_READ8((unsigned long *)address);
			/*
			 * Clear interrupt
			 */
			data8 |= 0x0B;
			/*
			 * Write the value back to the register
			 */
			IO_MEMORY_WRITE8(data8, (unsigned long *)address);
			/*
			 * Reset the DREQ Flip-Flop
			 */
			address = reset_reg;
			address +=
			    (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;

			IO_MEMORY_WRITE32(NO_ARG, (unsigned long *)address);
			/*
			 * Get Last Used PCI Address
			 */
			address = pci_reg;
			address +=
			    (unsigned long)dm75xx->
			    pci[DM75xx_PLX_MEM].virt_addr;

			address = IO_MEMORY_READ32((unsigned long *)address);

			address -= dm75xx->dma_buffers[i].bus_address;

			if (address == 0) {
				/*
				 * Last DMA Chain entry
				 */
				status = dma_interrupt;
				/*
				 * Check for DMA Buffer Overflow
				 */
				if ((dm75xx->dma_flag[i] &
				     DM75xx_DMA_FLAG_STATUS)) {
					printk(KERN_WARNING
					       "%s> DMA Channel %d buffer overrun. Flag: 0x%x\n",
					       &((dm75xx->name)[0]), i,
					       dm75xx->dma_flag[i]);
				}
			} else if (address == (dm75xx->dma_buffers[i].size / 2)) {
				/*
				 * Middle DMA Chain entry
				 */
				status = dma_interrupt;
				/*
				 * Check for DMA Buffer Overflow
				 */
				if (!
				    (dm75xx->dma_flag[i] &
				     DM75xx_DMA_FLAG_STATUS)) {
					printk(KERN_WARNING
					       "%s> DMA Channel %d buffer overrun. Flag: 0x%x\n",
					       &((dm75xx->name)[0]), i,
					       dm75xx->dma_flag[i]);
				}
			}

			if (dm75xx->dma_flag[i] & DM75xx_DMA_FLAG_ARB) {
				status = dma_interrupt;
			}
			// If there is an interrupt, then put it on the interrupt queue
			if (status != 0) {
#ifdef DM75XX_DEBUG
				printk(KERN_DEBUG "%s: DMA Interrupt\n",
				       &((dm75xx->name)[0]));
#endif
				dm75xx_put_interrupt(dm75xx, status);
				status = 0;
			}

		}
	}

	status = 0;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Check for other interrupts
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Check for Interrupt Overrun
	 */
	address = DM75xx_LAS0_IT_OVERRUN;
	address += (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;
	data16 = IO_MEMORY_READ16((unsigned long *)address);

	if (data16) {

		printk(KERN_ERR "%s> Interrupt Overrun: 0x%04x\n",
		       &((dm75xx->name)[0]), data16);

		IO_MEMORY_WRITE16(data16, (unsigned long *)address);
	}
	/*
	 * Read Interrupt Status Register
	 */
	address = DM75xx_LAS0_IT;
	address += (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;
	status = IO_MEMORY_READ16((unsigned long *)address);
	/*
	 * Write Interrupt Clear Mask
	 */
	address = DM75xx_LAS0_CLEAR_IT;
	address += (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;
	IO_MEMORY_WRITE16(status, (unsigned long *)address);
	/*
	 * Peform the actual clear
	 */
	IO_MEMORY_READ16((unsigned long *)address);

	/*
	 * Write the interrupts to the queue
	 */
	if (status & dm75xx->int_control) {
#ifdef DM75XX_DEBUG
		printk(KERN_DEBUG "%s: Other Interrupt\n",
		       &((dm75xx->name)[0]));
#endif
		for (status_bit = 0; status_bit < 16; status_bit++) {
			if (status & (1 << status_bit)) {
				dm75xx_put_interrupt(dm75xx, 1 << status_bit);
			}
		}

		status = 0;
	}

	/*
	 * If board is an SDM7540, we must check the analog DIO connector IRQ
	 */
	if (dm75xx->board_type & DM75xx_BOARD_SDM7540) {
		address = DM75xx_LAS0_ALGDIO_IRQ;
		address += (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;
		status =
		    (IO_MEMORY_READ8((unsigned long *)address) & 0x3C) << 24;
	}

	plx_intcsr =
	    IO_MEMORY_READ32(dm75xx->pci[DM75xx_PLX_MEM].virt_addr +
			     DM75xx_PLX_ITCSR);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Log this interrupt in our interrupt status queue.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * This is where the information is added to the queue if there is room
	 * otherwise we indicate queue overflow and log a missed interrupt
	 */
	if (status & dm75xx->int_control) {
#ifdef DM75XX_DEBUG
		printk(KERN_DEBUG "%s: SDM7540 Interrupt\n",
		       &((dm75xx->name)[0]));
#endif
		for (status_bit = 26; status_bit < 30; status_bit++) {
			if (status & (1 << status_bit)) {
				dm75xx_put_interrupt(dm75xx, 1 << status_bit);
			}
		}

		status = 0;
	}
	/*
	 * Release multiprocessor lock.
	 */
	spin_unlock(&(dm75xx->lock));
	/*
	 * Wake up any sleeping processes waiting for interrupts.
	 */
	wake_up_interruptible(&(dm75xx->int_wait_queue));

	return IRQ_HANDLED;
}

/******************************************************************************
Handle ioctl(2) system calls
 ******************************************************************************/

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static int
dm75xx_ioctl(struct inode *inode,
	     struct file *file,
	     unsigned int request_code, unsigned long ioctl_param)
#else
static long
dm75xx_ioctl(struct file *file,
	     unsigned int request_code, unsigned long ioctl_param)
#endif
{
	dm75xx_device_descriptor_t *dm75xx;
	int status;

	status = dm75xx_validate_device(file->private_data);
	if (status != 0) {
		return status;
	}

	dm75xx = (dm75xx_device_descriptor_t *) file->private_data;

	switch (request_code) {
	case DM75xx_IOCTL_REGION_READ:
		status = dm75xx_read_pci_region(dm75xx, ioctl_param);
		break;

	case DM75xx_IOCTL_REGION_WRITE:
		status = dm75xx_write_pci_region(dm75xx, ioctl_param);
		break;

	case DM75xx_IOCTL_REGION_MODIFY:
		status = dm75xx_modify_pci_region(dm75xx, ioctl_param);
		break;

	case DM75xx_IOCTL_DMA_FUNCTION:
		status = dm75xx_service_dma_function(dm75xx, ioctl_param);
		break;

	case DM75xx_IOCTL_WAKEUP:
		{
			unsigned long irq_flags;
			spin_lock_irqsave(&(dm75xx->lock), irq_flags);
			dm75xx->remove_isr_flag = 0xFF;
			spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
			wake_up_interruptible(&(dm75xx->int_wait_queue));
			status = 0;
#ifdef DM75XX_DEBUG
			printk(KERN_DEBUG "%s: ioctl - Wakeup\n", DRIVER_NAME);
#endif
			break;
		}
	case DM75xx_IOCTL_INT_STATUS:
		status = dm75xx_get_interrupt(dm75xx, ioctl_param);
		break;

	case DM75xx_IOCTL_GET_FIFO_SIZE:
		status = copy_to_user((unsigned int *)ioctl_param,
				      &(dm75xx->fifo_size),
				      sizeof(unsigned int));
		break;

	case DM75xx_IOCTL_GET_BOARD_TYPE:
		status = copy_to_user((dm75xx_board_t *) ioctl_param,
				      &(dm75xx->board_type),
				      sizeof(dm75xx_board_t));
		break;
	case DM75xx_IOCTL_INT_CONTROL:
		status = dm75xx_interrupt_control(dm75xx, ioctl_param);
		break;
	case DM75xx_IOCTL_RESET:
		dm75xx_board_reset(dm75xx);
		status = 0;
		break;
	case DM75xx_IOCTL_RESET_DMA_STATUS:
		{
			unsigned long irq_flags;
			uint8_t *flag = (uint8_t *) ioctl_param;
#ifdef DM75XX_DEBUG
			printk(KERN_DEBUG "%s: ioctl - Reset DMA status\n",
			       DRIVER_NAME);
#endif
			spin_lock_irqsave(&(dm75xx->lock), irq_flags);
			if (*flag & DM75xx_DMA_RESET_SEL) {
				dm75xx->dma_flag[1] &= ~DM75xx_DMA_FLAG_STATUS;
				dm75xx->dma_flag[1] |=
				    (*flag & DM75xx_DMA_FLAG_STATUS);

			} else {
				dm75xx->dma_flag[0] &= ~DM75xx_DMA_FLAG_STATUS;
				dm75xx->dma_flag[0] |=
				    (*flag & DM75xx_DMA_FLAG_STATUS);

			}
			spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

			break;
		}
	default:
		printk(KERN_ERR "%s: Unexpected IOCTL value: %d\n", DRIVER_NAME,
		       request_code);
		status = -EINVAL;
		break;
	}

	return status;
}

/******************************************************************************
 Board Reset
 ******************************************************************************/
static void dm75xx_board_reset(dm75xx_device_descriptor_t * dm75xx)
{
	unsigned long irq_flags;
	unsigned long address;
	uint8_t data8;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_board_reset\n", &((dm75xx->name)[0]));
#endif

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	/*
	 * Disable PLX Interrupts
	 */
	dm75xx_enable_plx_interrupts(dm75xx, 0x00);

	/*
	 * Disable all other on board interrupts
	 */
	dm75xx_interrupt_enable(dm75xx, 0xFFFFFFFF, 0x00);

	/*
	 * Clear DMA Channel 0 Interrupts
	 */
	address = DM75xx_PLX_DMA_CSR0;
	address += (unsigned long)dm75xx->pci[DM75xx_PLX_MEM].virt_addr;

	data8 = IO_MEMORY_READ8((unsigned long *)address);

	data8 |= 0x08;

	IO_MEMORY_WRITE8(data8, (unsigned long *)address);

	/*
	 * Clear DMA Channel 1 Interrupts
	 */

	address = DM75xx_PLX_DMA_CSR1;
	address += (unsigned long)dm75xx->pci[DM75xx_PLX_MEM].virt_addr;

	data8 = IO_MEMORY_READ8((unsigned long *)address);

	data8 |= 0x08;

	IO_MEMORY_WRITE8(data8, (unsigned long *)address);

	/*
	 * Perform hardware reset
	 */

	address = DM75xx_LAS0_BOARD_RESET;
	address += (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;
	IO_MEMORY_WRITE32(NO_ARG, (unsigned long *)address);

	/*
	 * Reset interrupt flags
	 */

	dm75xx->int_control = 0x00000000;

	dm75xx->int_queue_in = 0x00;
	dm75xx->int_queue_out = 0x00;
	dm75xx->int_queue_missed = 0x00;
	dm75xx->int_count = 0x00;

	/*
	 * Reenable PLX Interrupts
	 */
	dm75xx_enable_plx_interrupts(dm75xx, 0xFF);

	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
}

/******************************************************************************
 Enable/Disable interrupts
 ******************************************************************************/
static void
dm75xx_interrupt_enable(dm75xx_device_descriptor_t * dm75xx,
			dm75xx_int_source_t source, uint8_t enable)
{
	unsigned long address;
	uint32_t dma_ints;
	uint16_t board_ints;
	uint8_t analog_dio_ints;

#ifdef DM75XX_DEBUG
	if (enable) {
		printk(KERN_DEBUG "%s: dm75xx_interrupt_enable (enable)\n",
		       &((dm75xx->name)[0]));
	} else {
		printk(KERN_DEBUG "%s: dm75xx_interrupt_enable (DISABLE)\n",
		       &((dm75xx->name)[0]));
	}
#endif

	/*
	 * Update our interrupt tracking variable
	 */

	if (enable) {
		dm75xx->int_control |= source;
	} else {
		dm75xx->int_control &= ~source;
	}

	/*
	 * Grab Analog DIO Mask Register
	 */
	if (dm75xx->board_type == DM75xx_BOARD_SDM7540) {
		address = DM75xx_LAS0_ALGDIO_MASK;
		address += (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;
		analog_dio_ints =
		    IO_MEMORY_READ8((unsigned long *)address) & 0x3C;
	} else {
		analog_dio_ints = 0x00;
	}

	address = DM75xx_PLX_ITCSR;
	address += (unsigned long)dm75xx->pci[DM75xx_PLX_MEM].virt_addr;
	dma_ints = IO_MEMORY_READ32((unsigned long *)address);

	/*
	 * Update the interrupt register values
	 */

	board_ints = dm75xx->int_control & 0xFFFF;

	if (enable) {
		analog_dio_ints |= (source >> 24) & 0x3C;
		dma_ints |= ((source & 0x00600000) >> 3);
	} else {
		analog_dio_ints &= ~(source >> 24) & 0x3C;
		dma_ints &= ~((source & 0x00600000) >> 3);
	}

	/*
	 * Update the registers
	 */

	address = DM75xx_LAS0_IT;
	address += (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;
	IO_MEMORY_WRITE16(board_ints, (unsigned long *)address);

	if (dm75xx->board_type == DM75xx_BOARD_SDM7540) {
		address = DM75xx_LAS0_ALGDIO_MASK;
		address += (unsigned long)dm75xx->pci[DM75xx_LAS0].virt_addr;
		IO_MEMORY_WRITE8(analog_dio_ints, (unsigned long *)address);
	}

	address = DM75xx_PLX_ITCSR;
	address += (unsigned long)dm75xx->pci[DM75xx_PLX_MEM].virt_addr;
	IO_MEMORY_WRITE32(dma_ints, (unsigned long *)address);

}

/******************************************************************************
 Service interrupt control requests
 ******************************************************************************/
static int
dm75xx_interrupt_control(dm75xx_device_descriptor_t * dm75xx,
			 unsigned long ioctl_param)
{
	dm75xx_ioctl_argument_t ioctl_arg;
	int status;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_interrupt_control\n",
	       &((dm75xx->name)[0]));
#endif

	/*
	 * Copy in arguments from user-space and validate them
	 */

	if (copy_from_user(&ioctl_arg,
			   (dm75xx_ioctl_argument_t *) ioctl_param,
			   sizeof(dm75xx_ioctl_argument_t))) {
		return -EFAULT;
	}

	switch (ioctl_arg.int_control.function) {
	case DM75xx_INT_CONTROL_ENABLE:
		dm75xx_interrupt_enable(dm75xx,
					ioctl_arg.int_control.source, 0xFF);
		status = 0;
		break;
	case DM75xx_INT_CONTROL_DISABLE:
		dm75xx_interrupt_enable(dm75xx,
					ioctl_arg.int_control.source, 0x00);
		status = 0;
		break;
	case DM75xx_INT_CONTROL_CHECK:
		ioctl_arg.int_control.source = dm75xx->int_control;
		status = copy_to_user((dm75xx_ioctl_argument_t *) ioctl_param,
				      &(ioctl_arg),
				      sizeof(dm75xx_ioctl_argument_t));
		break;
	default:
		status = -ENOSYS;
		break;
	}
	return status;
}

/******************************************************************************
 Get interrupt status
 ******************************************************************************/
static int
dm75xx_get_interrupt(dm75xx_device_descriptor_t * dm75xx,
		     unsigned long ioctl_param)
{
	dm75xx_int_status_t interrupt_status;
	unsigned long irq_flags;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_get_interrupt\n", &((dm75xx->name)[0]));
#endif

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	/*
	 * If there is an interrupt in the queue then retreive the data.
	 */

	if (dm75xx->int_count > 0) {

		/*
		 * Cache local copies of the interrupt status
		 */
		interrupt_status.status =
		    dm75xx->int_status[dm75xx->int_queue_out];

		/*
		 * Make copy of the calculated number of interrupts in the queue and
		 * return this value -1 to signify how many more interrupts the
		 * reading device needs to receive
		 */
		dm75xx->int_count--;
		interrupt_status.int_remaining = dm75xx->int_count;
#ifdef DM75XX_DEBUG
		printk(KERN_DEBUG
		       "%s: Removed interrupt 0x%x, now %d in queue.\n",
		       &((dm75xx->name)[0]), interrupt_status.status,
		       dm75xx->int_count);
#endif
		/*
		 * Increment the number of interrupt statuses we have sent to the user
		 */

		dm75xx->int_queue_out++;

		if (dm75xx->int_queue_out == (DM75xx_INT_QUEUE_SIZE)) {

			/*
			 * wrap around if we have to
			 */

			dm75xx->int_queue_out = 0;

		}

	} else {

		/*
		 * Indicate that there are no interrupts in the queue
		 */

		interrupt_status.int_remaining = -1;
	}

	/*
	 * Pass back the number of missed interrupts regardless of its value
	 */

	interrupt_status.int_missed = dm75xx->int_queue_missed;

	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

	/*
	 * Copy the interrupt status back to user space
	 */
	if (copy_to_user((dm75xx_int_status_t *) ioctl_param,
			 &interrupt_status, sizeof(dm75xx_int_status_t))) {
		return -EFAULT;
	}

	return 0;
}

/******************************************************************************
 Put an interrupt in the queue
 This function assumes the caller has the spinlock
 ******************************************************************************/
static void
dm75xx_put_interrupt(dm75xx_device_descriptor_t * dm75xx, uint32_t interrupt)
{

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_put_interrupt\n", &((dm75xx->name)[0]));
#endif

	if (dm75xx->int_count < DM75xx_INT_QUEUE_SIZE) {
		/*
		 * Collect interrupt data and store in the device structure
		 */
		dm75xx->int_status[dm75xx->int_queue_in] = interrupt;
		dm75xx->int_queue_in++;

		/*
		 * Increment interrupt count
		 */
		dm75xx->int_count++;

#ifdef DM75XX_DEBUG
		printk(KERN_DEBUG
		       "%s: Added interrupt 0x%x, now %d in queue.\n",
		       &((dm75xx->name)[0]), interrupt, dm75xx->int_count);
#endif

		if (dm75xx->int_queue_in == (DM75xx_INT_QUEUE_SIZE)) {
			/*
			 * Wrap around to the front of the queue
			 */
			dm75xx->int_queue_in = 0;

		}

	} else {
		/*
		 * Indicate interrupt status queue overflow
		 */
		printk(KERN_WARNING
		       "%s> Missed interrupt info because queue is full\n",
		       &((dm75xx->name)[0]));

		dm75xx->int_queue_missed++;

	}

}

/******************************************************************************
 Service DMA Function Requests
 ******************************************************************************/
static int
dm75xx_service_dma_function(dm75xx_device_descriptor_t * dm75xx,
			    unsigned long ioctl_param)
{
	dm75xx_ioctl_argument_t ioctl_argument;
	int status = 0;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_service_dma_function\n",
	       &((dm75xx->name)[0]));
#endif

	/*
	 * Copy in arguments from user-space and validate them
	 */

	if (copy_from_user(&ioctl_argument,
			   (dm75xx_ioctl_argument_t *) ioctl_param,
			   sizeof(dm75xx_ioctl_argument_t))) {
		return -EFAULT;
	}

	switch (ioctl_argument.dma_function.function) {
	case DM75xx_DMA_FUNCTION_INITIALIZE:
		status = dm75xx_dma_initialize(dm75xx, &ioctl_argument);
		break;
	case DM75xx_DMA_FUNCTION_ABORT:
		status = dm75xx_dma_abort(dm75xx,
					  ioctl_argument.dma_function.channel);
		break;
	default:
		status = -ENOSYS;
		break;
	}
	return status;
}

/******************************************************************************
 Abort a DMA transfer
 ******************************************************************************/
static int
dm75xx_dma_abort(dm75xx_device_descriptor_t * dm75xx,
		 dm75xx_dma_channel_t channel)
{
	dm75xx_pci_access_request_t pci_request;
	unsigned long irq_flags;
	uint16_t status_offset;
	uint16_t mode_offset;
	unsigned int dma_done_checks = 0;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_dma_abort\n", &((dm75xx->name)[0]));
#endif

	switch (channel) {
	case DM75xx_DMA_CHANNEL_0:
		status_offset = DM75xx_PLX_DMA_CSR0;
		mode_offset = DM75xx_PLX_DMA_MODE0;
		break;
	case DM75xx_DMA_CHANNEL_1:
		status_offset = DM75xx_PLX_DMA_CSR1;
		mode_offset = DM75xx_PLX_DMA_MODE1;
		break;
	default:
		printk(KERN_ERR
		       "%s: Unexpected DMA channel (%d) during abort\n",
		       DRIVER_NAME, channel);
		return -EINVAL;
		break;
	}

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	/*
	 * Disable DMA Done interrupt as we are killing the transfer here
	 */

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = mode_offset;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = 0x00000000;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_READ);

	pci_request.data.data32 &= ~0x00000400;

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = mode_offset;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);

	/*
	 * Set the abort bit and clear the enable bit.
	 */

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = status_offset;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_8;
	pci_request.data.data8 = 0x00;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_READ);

	pci_request.data.data8 &= ~0x05;
	pci_request.data.data8 |= 0x04;

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = status_offset;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_8;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);

	/*
	 * Wait for the Done bit
	 */

	do {

		pci_request.region = DM75xx_PLX_MEM;
		pci_request.offset = status_offset;
		pci_request.size = DM75xx_PCI_REGION_ACCESS_8;
		pci_request.data.data8 = 0x00;
		dm75xx_access_pci_region(dm75xx, &pci_request,
					 DM75xx_PCI_REGION_ACCESS_READ);
		dma_done_checks++;

	} while (!(pci_request.data.data8 & 0x10) && (dma_done_checks < 10000));

	/*
	 * Enable DMA Done interrupts
	 */

	if (dma_done_checks == 10000) {
		printk(KERN_ERR
		       "%s: Aborted DMA (Channel %d), but never received DMA Complete bit\n",
		       DRIVER_NAME, channel);
	}

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = mode_offset;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = 0x00000000;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_READ);

	pci_request.data.data32 &= ~0x00000400;
	pci_request.data.data32 |= 0x00000400;

	pci_request.region = DM75xx_PLX_MEM;
	pci_request.offset = mode_offset;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);

	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

	return 0;

}

/******************************************************************************
 Allocate a DMA buffer
 ******************************************************************************/
static int
dm75xx_dma_alloc_buffer(dm75xx_device_descriptor_t * dm75xx,
			dm75xx_dma_channel_t channel)
{

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_dma_alloc_buffer\n",
	       &((dm75xx->name)[0]));
#endif

	dm75xx->dma_buffers[channel].virtual_address =
	    dma_alloc_coherent(NULL,
			       dm75xx->dma_buffers[channel].size,
			       &(dm75xx->dma_buffers[channel]).bus_address,
			       (GFP_ATOMIC | __GFP_NOWARN | __GFP_DMA));

	if (dm75xx->dma_buffers[channel].virtual_address == NULL) {
		printk(KERN_ERR "%s> Failed to allocate DMA buffer.\n",
		       &((dm75xx->name)[0]));
		return -ENOMEM;
	}

	return 0;
}

/*******************************************************************************
 Initialize DMA DREQ
 ******************************************************************************/
static int
dm75xx_dreq_init(dm75xx_device_descriptor_t * dm75xx,
		 dm75xx_dma_channel_t channel, dm75xx_dma_request_t dreq)
{
#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_dreq_init\n", DRIVER_NAME);
#endif
	switch (dreq) {
	case DM75xx_DMA_DEMAND_UTC1:
		dm75xx->dma_flag[channel] |= DM75xx_DMA_FLAG_RESET;
		dm75xx->dma_size[channel] = dm75xx->fifo_size;
		break;
	case DM75xx_DMA_DEMAND_FIFO_ADC:
	case DM75xx_DMA_DEMAND_FIFO_DAC1:
	case DM75xx_DMA_DEMAND_FIFO_DAC2:
		dm75xx->dma_flag[channel] &= ~DM75xx_DMA_FLAG_RESET;
		dm75xx->dma_size[channel] = dm75xx->fifo_size;
		break;
	default:
		printk(KERN_ERR
		       "%s: Unexpected DREQ value (%d) during DREQ init\n",
		       DRIVER_NAME, dreq);
		return -EINVAL;
		break;
	}

	return 0;
}

/******************************************************************************
 Initialize DMA
 ******************************************************************************/
static int
dm75xx_dma_initialize(dm75xx_device_descriptor_t * dm75xx,
		      dm75xx_ioctl_argument_t * ioctl_argument)
{
	dm75xx_pci_access_request_t pci_request;
	dm75xx_dma_channel_t channel;
	unsigned long irq_flags;
	uint32_t local_address;
	uint32_t direction;
	phys_addr_t phys_addr;
	uint32_t pci_address;
	void *virtual_addr;
	uint16_t reset_reg;
	uint16_t num_desc;
	uint8_t size_reg;
	uint8_t la_reg;
	uint8_t pci_reg;
	uint8_t dir_reg;
	uint8_t arb;
	int i;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_dma_initialize (start)\n",
	       &((dm75xx->name)[0]));
#endif

	/*
	 * Protect against interference from interrupts and other processors
	 */

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	channel = ioctl_argument->dma_function.channel;
	dm75xx->dma_buffers[channel].size = ioctl_argument->dma_function.size;
	arb = ioctl_argument->dma_function.arb;
	pci_address = ioctl_argument->dma_function.pci_address;

	if (arb) {
		dm75xx->dma_flag[channel] |= DM75xx_DMA_FLAG_ARB;
	} else {
		dm75xx->dma_flag[channel] &= ~DM75xx_DMA_FLAG_ARB;
	}
	/*
	 * The board must be PCI master capable to perform DMA operations
	 */
	dm75xx_get_pci_master_status(dm75xx, &pci_reg);
	if (!pci_reg) {
		printk(KERN_ERR
		       "%s> Device must be PCI master capable for DMA\n",
		       &((dm75xx->name)[0]));
		spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
		return -EOPNOTSUPP;
	}
	/*
	 * Do not allow multiple initializations of the same DMA channel
	 */
	if (dm75xx->dma_flag[channel] & DM75xx_DMA_FLAG_INIT) {
		printk(KERN_ERR
		       "%s> DMA channel %d has already been initialized\n",
		       &((dm75xx->name)[0]), channel);
		spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
		return -EAGAIN;
	}
	/*
	 * Do not allow buffers over the maximum size
	 */
	if (!arb) {
		if (dm75xx->dma_buffers[channel].size >
		    DM75xx_MAX_DMA_BUFFER_SIZE) {
			printk(KERN_ERR
			       "%s> The requested DMA buffer size of %lu is too large\n",
			       &((dm75xx->name)[0]),
			       dm75xx->dma_buffers[channel].size);
			spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

			return -EINVAL;
		}
	}
	/*
	 * Decide which channel and FIFO we are using and select the corresponding
	 * register sets.
	 */
	switch (channel) {
	case DM75xx_DMA_CHANNEL_0:
		la_reg = DM75xx_PLX_DMA_LADR0;
		pci_reg = DM75xx_PLX_DMA_PADR0;
		size_reg = DM75xx_PLX_DMA_SIZE0;
		dir_reg = DM75xx_PLX_DMA_DPR0;
		reset_reg = DM75xx_LAS0_DMA_RSTRQST0;
		break;
	case DM75xx_DMA_CHANNEL_1:
		la_reg = DM75xx_PLX_DMA_LADR1;
		pci_reg = DM75xx_PLX_DMA_PADR1;
		size_reg = DM75xx_PLX_DMA_SIZE1;
		dir_reg = DM75xx_PLX_DMA_DPR1;
		reset_reg = DM75xx_LAS0_DMA_RSTRQST1;
		break;
	default:
		spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
		printk(KERN_ERR
		       "%s: Unexpected DMA channel (%d) during DMA initialize.\n",
		       DRIVER_NAME, channel);
		return -EINVAL;
		break;
	}
	/*
	 * Determine the local address and the direction of the transfer based on
	 * the FIFO we will be using.
	 */
	switch (ioctl_argument->dma_function.source) {
	case DM75xx_DMA_FIFO_ADC:
		local_address = DMALADDR_ADC;
		direction = 0x00000008;
		break;
	case DM75xx_DMA_FIFO_DAC1:
		local_address = DMALADDR_DAC1;
		direction = 0x00000000;
		break;
	case DM75xx_DMA_FIFO_DAC2:
		local_address = DMALADDR_DAC2;
		direction = 0x00000000;
		break;
	case DM75xx_DMA_FIFO_HSDIN:
		local_address = DMALADDR_HSDIN;
		direction = 0x00000008;
		break;
	default:
		spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
		printk(KERN_ERR
		       "%s: Unexpected DMA source (%d) during DMA initialize.\n",
		       DRIVER_NAME, ioctl_argument->dma_function.source);

		return -EINVAL;
		break;
	}
	/*
	 * Demand mode request source
	 */
	if (dm75xx_dreq_init(dm75xx, channel,
			     ioctl_argument->dma_function.request) != 0) {
		spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
		return -1;
	}
	/*
	 * Setup PLX DMA Mode Registers
	 */
	dm75xx_enable_plx_dma(dm75xx, channel);

	if (!arb) {
		/*####################################################################
		   Allocate Coherent/Consistent DMA Mappings
		   ################################################################## */
		if (dm75xx_dma_alloc_buffer(dm75xx, channel) != 0) {
			spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
			dm75xx_free_dma_mappings(dm75xx, channel);
			return -1;
		}
		/*######################################################################
		   DMA Chaining descriptors need to be set up here.
		   These are 16 byte blocks in memory that consist of copies of 4
		   registers.
		   These registers are as follows:
		   1.  PCI Address Register - 32 bits
		   2.  Local Address Register - 32 bits
		   3.  Transfer Size Register - 32 bits
		   4.  Descriptor Pointer Register - 32 bits
		   A structure has been created to make this process easier.
		   ################################################################## */
		/*
		 * Allocate memory for the descriptors.
		 * We are grabbing a whole page here as we as assured it is quad word
		 * aligned.  However, we only really need half of this memory space.
		 */
		dm75xx->dma_chain[channel] = (dm75xx_dma_chain_descriptor_t *)
		    __get_free_page(GFP_ATOMIC | __GFP_DMA);

		if (dm75xx->dma_chain[channel] == 0) {

			spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
			dm75xx_free_dma_mappings(dm75xx, channel);
			printk(KERN_ERR
			       "%s>Allocation of DMA Chain Descriptors FAILED!\n",
			       &((dm75xx->name)[0]));
			return -ENOMEM;
		}

		if (dm75xx->dma_flag[channel] & DM75xx_DMA_FLAG_RESET) {
			num_desc = dm75xx->dma_buffers[channel].size /
			    dm75xx->dma_size[channel];
		} else {
			num_desc = 2;
		}
		/*######################################################################
		   Populate the descriptors.
		   ################################################################## */

		for (i = 0; i < num_desc; i++) {

			if (dm75xx->dma_flag[channel] & DM75xx_DMA_FLAG_RESET) {
				/* Transfer Size */
				(dm75xx->dma_chain[channel])[i].transfer_size =
				    dm75xx->dma_size[channel];
				/* PCI Address */
				(dm75xx->dma_chain[channel])[i].pci_address =
				    dm75xx->dma_buffers[channel].bus_address +
				    (i * dm75xx->dma_size[channel]);
			} else {
				/* Transfer Size */
				(dm75xx->dma_chain[channel])[i].transfer_size =
				    dm75xx->dma_buffers[channel].size / 2;
				/* PCI Address */
				if (i == 0) {
					/* First Half */
					(dm75xx->
					 dma_chain[channel])[i].pci_address =
				    dm75xx->dma_buffers[channel].bus_address;
				} else {
					/* Second Half */
					(dm75xx->
					 dma_chain[channel])[i].pci_address =
				    dm75xx->dma_buffers[channel].bus_address +
				    (dm75xx->dma_buffers[channel].size / 2);
				}
			}
			/* Local Address is constant */
			(dm75xx->dma_chain[channel])[i].local_address =
			    local_address;
			/*
			 * Descriptor Pointer to the next Descriptor.
			 * The descriptor is not constant, the interrupt enable bit must be
			 * set on the middle and last entries.
			 */
			if (i == (num_desc - 1)) {
				/* This is the last entry in the chain. */
				virtual_addr =
				    &((dm75xx->dma_chain[channel])[0]);
				phys_addr = virt_to_phys(virtual_addr);
				(dm75xx->
				 dma_chain[channel])[i].descriptor_pointer =
			    direction | 0x05 | phys_addr;

			} else if (i == (num_desc - 1) / 2) {
				/* This is the middle entry in the chain. */
				virtual_addr =
				    &((dm75xx->dma_chain[channel])[i]);
				phys_addr = virt_to_phys(virtual_addr);

				(dm75xx->
				 dma_chain[channel])[i].descriptor_pointer =
			    direction | 0x05 | (phys_addr +
						sizeof
						(dm75xx_dma_chain_descriptor_t));
			} else {
				/* This is a regular entry in the chain. */
				virtual_addr =
				    &((dm75xx->dma_chain[channel])[i]);
				phys_addr = virt_to_phys(virtual_addr);

				(dm75xx->
				 dma_chain[channel])[i].descriptor_pointer =
			    direction | 0x05 | (phys_addr +
						sizeof
						(dm75xx_dma_chain_descriptor_t));
			}
		}
		/*
		 * Configure First Descriptor
		 */
		virtual_addr = &((dm75xx->dma_chain[channel])[0]);
		phys_addr = virt_to_phys(virtual_addr);
		pci_request.region = DM75xx_PLX_MEM;
		pci_request.offset = dir_reg;
		pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
		pci_request.data.data32 = 0x01 | phys_addr;

		dm75xx_access_pci_region(dm75xx, &pci_request,
					 DM75xx_PCI_REGION_ACCESS_WRITE);
	} else {
		/* PCI Address */
		pci_request.region = DM75xx_PLX_MEM;
		pci_request.offset = pci_reg;
		pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
		pci_request.data.data32 = pci_address;
		dm75xx_access_pci_region(dm75xx, &pci_request,
					 DM75xx_PCI_REGION_ACCESS_WRITE);
		/* Local Address */
		pci_request.region = DM75xx_PLX_MEM;
		pci_request.offset = la_reg;
		pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
		pci_request.data.data32 = local_address;
		dm75xx_access_pci_region(dm75xx, &pci_request,
					 DM75xx_PCI_REGION_ACCESS_WRITE);
		/* Transfer Size */
		pci_request.region = DM75xx_PLX_MEM;
		pci_request.offset = size_reg;
		pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
		pci_request.data.data32 = ioctl_argument->dma_function.size;
		dm75xx_access_pci_region(dm75xx, &pci_request,
					 DM75xx_PCI_REGION_ACCESS_WRITE);
		/* Direction */
		pci_request.region = DM75xx_PLX_MEM;
		pci_request.offset = dir_reg;
		pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
		pci_request.data.data32 = direction;
		dm75xx_access_pci_region(dm75xx, &pci_request,
					 DM75xx_PCI_REGION_ACCESS_WRITE);
	}
	/*
	 * Reset the DREQ
	 */
	pci_request.region = DM75xx_LAS0;
	pci_request.offset = reset_reg;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = NO_ARG;

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);
	/*
	 * Set various initialization flags
	 */
	dm75xx->dma_flag[channel] |=
	    DM75xx_DMA_FLAG_INIT | DM75xx_DMA_FLAG_STATUS |
	    DM75xx_DMA_FLAG_MMAP;

	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_dma_initialize (finish)\n",
	       &((dm75xx->name)[0]));
#endif
	return 0;
}

/******************************************************************************
 Free DMA channel buffer mappings
 ******************************************************************************/
static void
dm75xx_free_dma_mappings(dm75xx_device_descriptor_t * dm75xx,
			 dm75xx_dma_channel_t channel)
{
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Release DMA mappings.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	if (dm75xx->dma_flag[channel]
	    & (DM75xx_DMA_FLAG_INIT & ~DM75xx_DMA_FLAG_ARB)) {
		struct page *pend;
		pend =
		    virt_to_page(dm75xx->dma_buffers[channel].virtual_address +
				 dm75xx->dma_buffers[channel].size - 1);

		/*
		 * Now we can release the memory
		 */

		dma_free_coherent(NULL,
				  dm75xx->dma_buffers[channel].size,
				  dm75xx->dma_buffers[channel].virtual_address,
				  dm75xx->dma_buffers[channel].bus_address);

		if (dm75xx->dma_chain[channel] != NULL) {
			free_page((unsigned long)
				  dm75xx->dma_chain[channel]);
		}
	}
}

/******************************************************************************
Initialize DM75xx driver and devices
 ******************************************************************************/

int dm75xx_load(void)
{
	int status;

	printk(KERN_INFO "%s> Initializing module (version %s).\n",
	       DRIVER_NAME, DRIVER_RELEASE);

	printk(KERN_INFO "%s> %s\n", DRIVER_NAME, DRIVER_DESCRIPTION);
	printk(KERN_INFO "%s> %s\n", DRIVER_NAME, DRIVER_COPYRIGHT);

	dm75xx_devices = NULL;
	dm75xx_major = 0;
	/*
	 * Find the devices, map their IO/MEM regions, and grab an IRQ.
	 */
	status = dm75xx_probe_devices(&dm75xx_device_count, &dm75xx_devices);
	if (status != 0) {
		return status;
	}
	/*
	 * Register this devices with the system.
	 */
	status = dm75xx_register_char_device(&dm75xx_major);
	if (status != 0) {
		printk(KERN_ERR
		       "%s> ERROR: Dynamic character device registration FAILED (errno "
		       "= %d)\n", DRIVER_NAME, -status);
		dm75xx_release_resources();
		return status;
	}

	printk(KERN_INFO
	       "%s> Driver registered using character major number %d\n",
	       DRIVER_NAME, dm75xx_major);

	return 0;
}

/******************************************************************************
Read from a PCI region, modify bits in the value, and write the new value back
to the region
 ******************************************************************************/

static int
dm75xx_modify_pci_region(dm75xx_device_descriptor_t * dm75xx,
			 unsigned long ioctl_param)
{
	dm75xx_pci_access_request_t pci_request;
	dm75xx_ioctl_argument_t ioctl_argument;
	unsigned long irq_flags;
	int status;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Copy arguments in from user space and validate them
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	if (copy_from_user(&ioctl_argument,
			   (dm75xx_ioctl_argument_t *) ioctl_param,
			   sizeof(dm75xx_ioctl_argument_t)
	    )
	    ) {
		return -EFAULT;
	}

	status =
	    dm75xx_validate_pci_access(dm75xx, &(ioctl_argument.modify.access));

	if (status != 0) {
		return status;
	}

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Do the actual read/modify/write
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	/*
	 * Make a copy of user arguments to keep from overwriting them
	 */

	pci_request = ioctl_argument.modify.access;

	/*
	 * Read current value
	 */

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_READ);

	/*
	 * Modify the value based upon mask
	 */

	switch (ioctl_argument.modify.access.size) {
	case DM75xx_PCI_REGION_ACCESS_8:

		/*
		 * Preserve bits which are not to be changed and clear bits which
		 * will be changed
		 */

		pci_request.data.data8 &= ~ioctl_argument.modify.mask.mask8;

		/*
		 * Fold in the new value but don't allow bits to be set which are
		 * are not modifiable according to the mask
		 */

		pci_request.data.data8 |=
		    (ioctl_argument.modify.access.data.
		     data8 & ioctl_argument.modify.mask.mask8);

		break;

	case DM75xx_PCI_REGION_ACCESS_16:

		/*
		 * Preserve bits which are not to be changed and clear bits which
		 * will be changed
		 */

		pci_request.data.data16 &= ~ioctl_argument.modify.mask.mask16;

		/*
		 * Fold in the new value but don't allow bits to be set which are
		 * are not modifiable according to the mask
		 */

		pci_request.data.data16 |=
		    (ioctl_argument.modify.access.data.
		     data16 & ioctl_argument.modify.mask.mask16);

		break;

	case DM75xx_PCI_REGION_ACCESS_32:

		/*
		 * Preserve bits which are not to be changed and clear bits which
		 * will be changed
		 */

		pci_request.data.data32 &= ~ioctl_argument.modify.mask.mask32;

		/*
		 * Fold in the new value but don't allow bits to be set which are
		 * are not modifiable according to the mask
		 */

		pci_request.data.data32 |=
		    (ioctl_argument.modify.access.data.
		     data32 & ioctl_argument.modify.mask.mask32);

		break;
	}

	/*
	 * Write new value
	 */

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);

	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

	return 0;
}

/******************************************************************************
Open a DM75xx device
 ******************************************************************************/

static int dm75xx_open(struct inode *inode, struct file *file)
{
	dm75xx_device_descriptor_t *dm75xx;
	unsigned int minor_number;
	unsigned long irq_flags;

	minor_number = iminor(inode);

	if (minor_number >= dm75xx_device_count) {

		/*
		 * This will never be returned on a 2.6 kernel.  A file system layer
		 * above this one checks for invalid character device minor numbers and
		 * returns -ENXIO before this function is ever invoked.
		 */

		return -ENODEV;
	}

	dm75xx = &(dm75xx_devices[minor_number]);

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_open-----------\n", &((dm75xx->name)[0]));
#endif

	spin_lock_init(&(dm75xx->lock));

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	if (dm75xx->reference_count) {
		spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);
		return -EBUSY;
	}

	file->private_data = dm75xx;
	dm75xx->reference_count++;

	/*
	 * Disable all interrupts
	 */

	dm75xx_interrupt_enable(dm75xx, 0xFFFFFFFF, 0x00);

	/*
	 * Enable PLX interrupts
	 */

	dm75xx_enable_plx_interrupts(dm75xx, 0xFF);

	dm75xx_initialize_device_descriptor(dm75xx);

	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

	return 0;
}

/******************************************************************************
Determine whether or not a device is readable
 ******************************************************************************/

static unsigned int
dm75xx_poll(struct file *file, struct poll_table_struct *poll_table)
{
	dm75xx_device_descriptor_t *dm75xx;
	unsigned int interrupts_in_queue;
	unsigned int status_mask = 0;
	unsigned long irq_flags;

	/*
	 * If we don't have a valid DM75xx device descriptor, no status is available
	 */

	if (dm75xx_validate_device(file->private_data) != 0) {

		/*
		 * This value causes select(2) to indicate that a file descriptor is
		 * present in its file descriptor sets but it will be in the exception
		 * set rather than in the input set.
		 */

		return POLLPRI;
	}

	dm75xx = (dm75xx_device_descriptor_t *) file->private_data;

	/*
	 * Register with the file system layer so that it can wait on and check for
	 * DM75xx events
	 */

	poll_wait(file, &(dm75xx->int_wait_queue), poll_table);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Waiting is done interruptibly, which means that a signal could have been
	   delivered.  Thus we might have been woken up by a signal before an
	   interrupt occurred.  Therefore, the process needs to examine the device's
	   interrupt flag.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	/*
	 * Prevent a race condition with the interrupt handler and make a local copy
	 * of the interrupt flag
	 */

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	if (dm75xx->int_queue_out <= dm75xx->int_queue_in) {

		interrupts_in_queue = dm75xx->int_queue_in -
		    dm75xx->int_queue_out;

	} else {
		interrupts_in_queue = (DM75xx_INT_QUEUE_SIZE + 1) -
		    (dm75xx->int_queue_out - dm75xx->int_queue_in);
	}

	if (dm75xx->remove_isr_flag) {
		dm75xx->remove_isr_flag = 0x00;
		status_mask = (POLLIN | POLLRDNORM);
	}

	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Interpret interrupt flag
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	/*
	 * The flag is cleared after reading.  See if it is clear or not.
	 */
	if (interrupts_in_queue > 0) {

		/*
		 * Not clear, therefore an interrupt occurred since value was read last
		 */

		status_mask |= (POLLIN | POLLRDNORM);

	}

	return status_mask;
}

/******************************************************************************
Probe and configure all DM75xx devices
 ******************************************************************************/

static int
dm75xx_probe_devices(uint32_t * device_count,
		     dm75xx_device_descriptor_t ** device_descriptors)
{
	struct pci_dev *pci_device;
	uint32_t minor_number;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Count the number of supported devices in the kernel's PCI device list
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	*device_count = 0;
	pci_device = NULL;

	while ((pci_device = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pci_device))
	       != NULL) {

#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 2, 6, 12 ))
		if (pci_match_id(dm75xx_pci_device_table, pci_device) == NULL) {
#else
		if (pci_match_device(dm75xx_pci_device_table, pci_device) ==
		    NULL) {
#endif
			continue;
		}

		(*device_count)++;
	}

	if (*device_count == 0) {
		printk(KERN_ERR "%s> ERROR: No devices found\n", DRIVER_NAME);
		return -ENODEV;
	}

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Allocate memory for the device descriptors
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	*device_descriptors = kmalloc((*device_count *
				       sizeof(dm75xx_device_descriptor_t)),
				      GFP_KERNEL);
	if (*device_descriptors == NULL) {
		printk(KERN_ERR
		       "%s> ERROR: Device descriptor memory allocation FAILED\n",
		       DRIVER_NAME);
		return -ENOMEM;
	}

	memset(*device_descriptors,
	       0, (*device_count * sizeof(dm75xx_device_descriptor_t))
	    );

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Set up all DM75xx devices
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	pci_device = NULL;
	minor_number = 0;

	while ((pci_device = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pci_device))
	       != NULL) {
		const struct pci_device_id *current_device;
		dm75xx_device_descriptor_t *dm75xx;
		int status;
		uint8_t pci_master;
		unsigned int size = 0;

		/*
		 * See if the current PCI device is in the table of devices supported
		 * by the driver.  If not, just ignore it and go to next PCI device.
		 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 2, 6, 12 ))
		current_device =
		    pci_match_id(dm75xx_pci_device_table, pci_device);
#else
		current_device =
		    pci_match_device(dm75xx_pci_device_table, pci_device);
#endif
		if (current_device == NULL) {
			continue;
		}

		printk(KERN_INFO
		       "%s> Minor %u: DM%4x found at bus %u, slot %02X, function %02X\n",
		       DRIVER_NAME, minor_number, current_device->device,
		       pci_device->bus->number, PCI_SLOT(pci_device->devfn),
		       PCI_FUNC(pci_device->devfn)
		    );

		dm75xx = &((*device_descriptors)[minor_number]);

		if (current_device->device == 0x7540) {
			dm75xx->board_type = DM75xx_BOARD_SDM7540;
		} else {
			dm75xx->board_type = DM75xx_BOARD_DM7520;
		}

		spin_lock_init(&(dm75xx->lock));

		dm75xx_initialize_device_descriptor(dm75xx);

		/*
		 * Create the full device name
		 */

		status = snprintf(&((dm75xx->name)[0]),
				  DM75xx_DEVICE_NAME_LENGTH,
				  "%s-%u", DRIVER_NAME, minor_number);
		if (status >= DM75xx_DEVICE_NAME_LENGTH) {
			printk(KERN_ERR
			       "%s-%u> ERROR: Device name creation FAILED\n",
			       DRIVER_NAME,
			       (unsigned int)(dm75xx -
					      &((*device_descriptors)[0]))
			    );
			dm75xx_release_resources();
			pci_dev_put(pci_device);
			return -ENAMETOOLONG;
		}

		status = pci_enable_device(pci_device);

		if (status != 0) {
			dm75xx_release_resources();
			pci_disable_device(pci_device);
			pci_dev_put(pci_device);
			return status;
		}

		/*
		 * Determine 1) how many standard PCI regions are present, 2) how the
		 * regions are mapped, and 3) how many bytes are in each region.  Also,
		 * remap any memory-mapped region into the kernel's address space.
		 */

		status = dm75xx_process_pci_regions(dm75xx, pci_device);
		if (status != 0) {
			dm75xx_release_resources();
			pci_disable_device(pci_device);
			pci_dev_put(pci_device);
			return status;
		}

		/*
		 * Associate device IRQ line with device in kernel
		 */

		status = dm75xx_allocate_irq(dm75xx, pci_device);
		if (status != 0) {
			dm75xx_release_resources();
			pci_disable_device(pci_device);
			pci_dev_put(pci_device);
			return status;
		}

		/*
		 * Print a warning message if device is not PCI master capable
		 */

		dm75xx_get_pci_master_status(dm75xx, &pci_master);
		if (!pci_master) {
			printk(KERN_WARNING
			       "%s> WARNING: Device does not support DMA\n",
			       &((dm75xx->name)[0])
			    );
		}

		/*
		 * Initialize the board.
		 */

		dm75xx_initialize_hardware(dm75xx);

		/*
		 * Find the fifo size of the board.
		 */

		status = dm75xx_get_fifo_size(dm75xx, &size);

		if (size == 0) {
			printk(KERN_WARNING
			       "%s> WARNING: Device reported FIFO size of ZERO\n",
			       &((dm75xx->name)[0])
			    );
			dm75xx_release_resources();
			pci_disable_device(pci_device);
			pci_dev_put(pci_device);
			return -EINVAL;
		}
		printk(KERN_INFO "%s> Fifo Size %d\n",
		       &((dm75xx->name)[0]), size);
		dm75xx->fifo_size = size;
		minor_number++;
	}

	printk(KERN_INFO "%s> Found %u DM75xx device(s)\n", DRIVER_NAME,
	       *device_count);

	return 0;
}

/******************************************************************************
Set up standard PCI regions
 ******************************************************************************/

static int
dm75xx_process_pci_regions(dm75xx_device_descriptor_t * dm75xx,
			   const struct pci_dev *pci_device)
{
	uint8_t region;

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Process each standard PCI region
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	for (region = 0; region < DM75xx_PCI_REGIONS; region++) {
		unsigned long address = 0;
		unsigned long length;
		unsigned long flags;

		/*#####################################################################
		   Get region's physical address and length in bytes.  If either is zero,
		   the region is unused and should be ignored.
		   ################################################################## */

		address = pci_resource_start(pci_device, region);
		if (address == 0) {
			continue;
		}

		length = pci_resource_len(pci_device, region);
		if (length == 0) {
			continue;
		}

		/*#####################################################################
		   Save information in PCI region descriptor
		   ################################################################## */

		dm75xx->pci[region].phys_addr = address;
		dm75xx->pci[region].length = length;

		/*#####################################################################
		   Determine how the region is mapped
		   ################################################################## */

		flags = pci_resource_flags(pci_device, region);

		if (flags & IORESOURCE_IO) {

			/*
			 * The region is I/O mapped
			 */

			/*
			 * Allocate the I/O port range
			 */

			if (request_region(address, length, &((dm75xx->name)[0])
			    )
			    == NULL) {
				printk(KERN_ERR
				       "%s> ERROR: I/O port range %#lx-%#lx allocation FAILED\n",
				       &((dm75xx->name)[0]),
				       address, (address + length - 1)
				    );
				dm75xx_release_resources();
				return -EBUSY;
			}

			dm75xx->pci[region].io_addr = address;

			printk(KERN_INFO
			       "%s> Allocated I/O port range %#lx-%#lx\n",
			       &((dm75xx->name)[0]), address,
			       (address + length - 1)
			    );
		} else if (flags & IORESOURCE_MEM) {

			/*
			 * The region is memory mapped
			 */

			/*
			 * Remap the region's physical address into the kernel's virtual
			 * address space and allocate the memory range
			 */

			dm75xx->pci[region].virt_addr =
			    ioremap_nocache(address, length);
			if (dm75xx->pci[region].virt_addr == NULL) {
				printk(KERN_ERR
				       "%s> ERROR: BAR%u remapping FAILED\n",
				       &((dm75xx->name)[0]), region);
				return -ENOMEM;
			}

			if (request_mem_region
			    (address, length, &((dm75xx->name)[0])
			    )
			    == NULL) {
				printk(KERN_ERR
				       "%s> ERROR: I/O memory range %#lx-%#lx allocation FAILED\n",
				       &((dm75xx->name)[0]),
				       address, (address + length - 1)
				    );
				return -EBUSY;
			}

			dm75xx->pci[region].allocated = 0xFF;

			printk(KERN_INFO
			       "%s> Allocated I/O memory range %#lx-%#lx\n",
			       &((dm75xx->name)[0]), address,
			       (address + length - 1)
			    );
		} else {

			/*
			 * The region has invalid resource flags
			 */

			printk(KERN_ERR "%s> ERROR: Invalid PCI region flags\n",
			       &((dm75xx->name)[0])
			    );
			return -EIO;
		}

		/*#####################################################################
		   Print information about the region
		   ################################################################## */

		printk(KERN_INFO "%s> PCI%u Region:\n",
		       &((dm75xx->name)[0]), region);

		if (dm75xx->pci[region].io_addr != 0) {
			printk(KERN_INFO "    Address: %#lx (I/O mapped)\n",
			       dm75xx->pci[region].io_addr);
		} else {
			printk(KERN_INFO "    Address: %#lx (memory mapped)\n",
			       (unsigned long)dm75xx->pci[region].virt_addr);
			printk(KERN_INFO "    Address: %x (physical)\n",
			       dm75xx->pci[region].phys_addr);
		}

		printk(KERN_INFO "    Length:  %#lx\n",
		       dm75xx->pci[region].length);
	}

	return 0;
}

/******************************************************************************
Register DM75xx character device with kernel
 ******************************************************************************/

static int dm75xx_register_char_device(int *major)
{
	dev_t device, devno;
	int status;
	struct device *dev = NULL;
	char dev_file_name[30];
	int minor = 0;

	status =
	    alloc_chrdev_region(&device, 0, dm75xx_device_count, DRIVER_NAME);
	if (status < 0) {
		return status;
	}

	cdev_init(&dm75xx_cdev, &dm75xx_file_ops);
	dm75xx_cdev.owner = THIS_MODULE;

	status = cdev_add(&dm75xx_cdev, device, dm75xx_device_count);
	if (status < 0) {
		unregister_chrdev_region(device, dm75xx_device_count);
		return status;
	}

	*major = MAJOR(device);

	dev_class = class_create(THIS_MODULE, DRIVER_NAME);

	if (dev_class == NULL) {
		unregister_chrdev_region(device, dm75xx_device_count);
		return -ENODEV;
	}

	for (minor = 0; minor < dm75xx_device_count; minor++) {
		sprintf(dev_file_name, "%s-%u", DRIVER_NAME, minor);
		devno = MKDEV(*major, minor);
		dev = device_create(dev_class,
				    NULL, devno, NULL, dev_file_name, 0);

		if (dev == NULL) {
			return -ENODEV;
		}
	}

	return 0;
}

/******************************************************************************
Close a DM75xx device file
 ******************************************************************************/

static int dm75xx_release(struct inode *inode, struct file *file)
{
	dm75xx_device_descriptor_t *dm75xx;
	unsigned long irq_flags;
	int status;

	status = dm75xx_validate_device(file->private_data);
	if (status != 0) {
		return status;
	}

	dm75xx = (dm75xx_device_descriptor_t *) file->private_data;

	spin_lock_irqsave(&(dm75xx->lock), irq_flags);

	dm75xx->reference_count--;
	file->private_data = NULL;

	/*
	 * Disable PLX interrupts
	 */

	dm75xx_enable_plx_interrupts(dm75xx, 0x00);

	/*
	 * Spin lock must be unlocked before the call to free_dma_mappings
	 * This is due to the fact you cannot have irqs disable when calling
	 * dma_free_coherent().
	 */
	spin_unlock_irqrestore(&(dm75xx->lock), irq_flags);

	dm75xx_free_dma_mappings(dm75xx, DM75xx_DMA_CHANNEL_0);
	dm75xx_free_dma_mappings(dm75xx, DM75xx_DMA_CHANNEL_1);

	return 0;
}

/******************************************************************************
Release resources allocated by driver
 ******************************************************************************/

static void dm75xx_release_resources(void)
{
	uint32_t minor_number;

	if (dm75xx_devices != NULL) {
		/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		   Release device-level resources
		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

		for (minor_number = 0; minor_number < dm75xx_device_count;
		     minor_number++) {
			dm75xx_device_descriptor_t *dm75xx;
			uint8_t region;

			dm75xx = &((dm75xx_devices)[minor_number]);

			/*
			 * Free any allocated IRQ
			 */
			if (dm75xx->irq_number != 0) {
				free_irq(dm75xx->irq_number, dm75xx);
				printk(KERN_INFO "%s> Freed IRQ %u\n",
				       &((dm75xx->name)[0]),
				       dm75xx->irq_number);
			}

			/*
			 * Free any resources allocated for the PCI regions
			 */

			for (region = 0; region < DM75xx_PCI_REGIONS; region++) {

				/*
				 * Determine how region is mapped
				 */

				if (dm75xx->pci[region].virt_addr != NULL) {

					/*
					 * Region is memory-mapped
					 */

					/*
					 * If memory range allocation succeeded, free the range
					 */

					if (dm75xx->pci[region].allocated !=
					    0x00) {
						release_mem_region(dm75xx->pci
								   [region].phys_addr,
								   dm75xx->pci
								   [region].length);

						printk(KERN_INFO
						       "%s> Released I/O memory range %#lx-%#lx\n",
						       &((dm75xx->name)
							 [0]), (unsigned long)
						       dm75xx->
						       pci[region].phys_addr,
						       ((unsigned long)
							dm75xx->
							pci[region].phys_addr +
							dm75xx->
							pci[region].length - 1)
						    );
					}

					/*
					 * Unmap region from kernel's address space
					 */

					iounmap(dm75xx->pci[region].virt_addr);

					printk(KERN_INFO
					       "%s> Unmapped kernel mapping at %#lx\n",
					       &((dm75xx->name)[0]),
					       (unsigned long)
					       dm75xx->pci[region].virt_addr);
				} else if (dm75xx->pci[region].io_addr != 0) {

					/*
					 * Region is I/O-mapped
					 */

					/*
					 * Free I/O port range
					 */

					release_region(dm75xx->
						       pci[region].phys_addr,
						       dm75xx->
						       pci[region].length);

					printk(KERN_INFO
					       "%s> Released I/O port range %#lx-%#lx\n",
					       &((dm75xx->name)[0]),
					       (unsigned long)
					       dm75xx->pci[region].phys_addr,
					       ((unsigned long)
						dm75xx->pci[region].phys_addr +
						dm75xx->pci[region].length - 1)
					    );
				}
			}
		}

		/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		   Release driver-level resources
		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

		/*
		 * Free device descriptor memory after all references to it are finished
		 */

		kfree(dm75xx_devices);
		dm75xx_devices = NULL;
	}
}

/******************************************************************************
Deinitialize DM75xx driver and devices
 ******************************************************************************/

void dm75xx_unload(void)
{
	int status;

	dm75xx_release_resources();

	status = dm75xx_unregister_char_device();
	if (status != 0) {
		printk(KERN_ERR
		       "%s> ERROR: Character device unregistration FAILED (errno "
		       " = %d)!\n", DRIVER_NAME, -status);
		printk(KERN_ERR
		       "%s> ERROR: A system reboot should be performed\n",
		       DRIVER_NAME);
	}

	printk(KERN_INFO "%s> Character device %d unregistered\n",
	       DRIVER_NAME, dm75xx_major);

	printk(KERN_INFO "%s> Module unloaded.\n", DRIVER_NAME);
}

/******************************************************************************
Unregister DM75xx character device with kernel
 ******************************************************************************/

static int dm75xx_unregister_char_device(void)
{
	unsigned int minor;

	cdev_del(&dm75xx_cdev);

	for (minor = 0; minor < dm75xx_device_count; minor++) {
		device_destroy(dev_class, MKDEV(dm75xx_major, minor));

	}

	class_unregister(dev_class);

	class_destroy(dev_class);

	unregister_chrdev_region(MKDEV(dm75xx_major, 0), dm75xx_device_count);
	return 0;
}

/******************************************************************************
 Validate a DM75xx device descriptor
 ******************************************************************************/

static int dm75xx_validate_device(const dm75xx_device_descriptor_t * dm75xx)
{
	uint32_t minor_number;

	for (minor_number = 0; minor_number < dm75xx_device_count;
	     minor_number++) {
		if (dm75xx == &((dm75xx_devices)[minor_number])) {
			return 0;
		}
	}

	return -EBADFD;
}

/*******************************************************************************
 Find the board's FIFO size.
 ******************************************************************************/
static int
dm75xx_get_fifo_size(dm75xx_device_descriptor_t * dm75xx, unsigned int *size)
{
	dm75xx_pci_access_request_t pci_request;
	uint32_t i = 0;

	*size = 0;

	/*
	 * Clear ADC FIFO
	 */
	pci_request.region = DM75xx_LAS0;
	pci_request.offset = DM75xx_LAS0_ADC_FIFO_CLR;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = NO_ARG;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);
	/*
	 * Enable the CGT
	 */
	pci_request.region = DM75xx_LAS0;
	pci_request.offset = DM75xx_LAS0_CGT_ENABLE;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = 0x00000000;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);
	/*
	 * Latch the CGT as we are only using 1 channel.
	 */
	pci_request.region = DM75xx_LAS0;
	pci_request.offset = DM75xx_LAS0_CGT_LATCH;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = 0x00000000;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);
	/*
	 * Fill the fifo until it is half full, this will indicate the fifo size.
	 */
	for (i = 0; (*size == 0) && (i < 8192); i++) {
		/*
		 * Take in one value into the FIFO
		 */
		pci_request.region = DM75xx_LAS0;
		pci_request.offset = DM75xx_LAS0_FIFO_STATUS;
		pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
		pci_request.data.data32 = NO_ARG;
		dm75xx_access_pci_region(dm75xx, &pci_request,
					 DM75xx_PCI_REGION_ACCESS_WRITE);
		/*
		 * Read the FIFO Status
		 */
		pci_request.region = DM75xx_LAS0;
		pci_request.offset = DM75xx_LAS0_FIFO_STATUS;
		pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
		pci_request.data.data32 = 0x00000000;
		dm75xx_access_pci_region(dm75xx, &pci_request,
					 DM75xx_PCI_REGION_ACCESS_READ);
		if (0 == (pci_request.data.data32 & DM75xx_FIFO_ADC_HALF_EMPTY)) {
			*size = i * 2;
		}
	}
	/*
	 * Clear the ADC FIFO again.
	 */
	pci_request.region = DM75xx_LAS0;
	pci_request.offset = DM75xx_LAS0_ADC_FIFO_CLR;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = NO_ARG;
	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);
	/*
	 * Reset the board.
	 */
	pci_request.region = DM75xx_LAS0;
	pci_request.offset = DM75xx_LAS0_BOARD_RESET;
	pci_request.size = DM75xx_PCI_REGION_ACCESS_32;
	pci_request.data.data32 = NO_ARG;

	dm75xx_access_pci_region(dm75xx, &pci_request,
				 DM75xx_PCI_REGION_ACCESS_WRITE);

	/*
	 * If the FIFO size was discovered to be a known value
	 * plus or minus 5 samples then we can figure it is safe
	 * to assume it is that specific known value.  This is
	 * done in the case that the DMA HALF FULL flag is not
	 * immediatly asserted.
	 */
	if ((*size < (8192 + 5)) && (*size > (8192 - 5))) {
		*size = 8192;
	} else if ((*size < (1024 + 5)) && (*size > (1024 - 5))) {
		*size = 1024;
	} else {
		printk(KERN_ERR
		       "%s Invalid size detected: %u after %u writes.\n",
		       DRIVER_NAME, *size, i);
	}

	return 0;
}

/*******************************************************************************
 Map DMA buffer into user space
 ******************************************************************************/

static int dm75xx_mmap(struct file *file, struct vm_area_struct *vma)
{
	dm75xx_device_descriptor_t *dm75xx_device;
	dm75xx_dma_channel_t channel;
	unsigned long irq_flags;
	size_t vm_size;
	int status;

#ifdef DM75XX_DEBUG
	printk(KERN_DEBUG "%s: dm75xx_mmap\n", DRIVER_NAME);
#endif

	/*
	 * Calculate size of the area to be memory mapped.
	 */
	vm_size = vma->vm_end - vma->vm_start;
	/*
	 * Validate the device descriptor.
	 */
	status = dm75xx_validate_device(file->private_data);
	if (status != 0) {
		return status;
	}

	dm75xx_device = (dm75xx_device_descriptor_t *) file->private_data;
	/*
	 * Grab spinlock while we are messing with device flags
	 */
	spin_lock_irqsave(&(dm75xx_device->lock), irq_flags);
	/*
	 * Which DMA channel are we working with.
	 */
	if ((dm75xx_device->dma_flag[DM75xx_DMA_CHANNEL_0] &
	     DM75xx_DMA_FLAG_MMAP)
	    && !(dm75xx_device->dma_flag[DM75xx_DMA_CHANNEL_1] &
		 DM75xx_DMA_FLAG_MMAP)
	    ) {
		/*
		 * Reset the flag.
		 */
		dm75xx_device->dma_flag[DM75xx_DMA_CHANNEL_0] &=
		    ~DM75xx_DMA_FLAG_MMAP;

		/*
		 * Select the channel.
		 */
		channel = DM75xx_DMA_CHANNEL_0;
	} else if (!(dm75xx_device->dma_flag[DM75xx_DMA_CHANNEL_0] &
		     DM75xx_DMA_FLAG_MMAP) &&
		   (dm75xx_device->dma_flag[DM75xx_DMA_CHANNEL_1] &
		    DM75xx_DMA_FLAG_MMAP)) {
		/*
		 * Reset the flag.
		 */
		dm75xx_device->dma_flag[DM75xx_DMA_CHANNEL_1] &=
		    ~DM75xx_DMA_FLAG_MMAP;
		/*
		 * Select the channel.
		 */
		channel = DM75xx_DMA_CHANNEL_1;
	} else {
		spin_unlock_irqrestore(&(dm75xx_device->lock), irq_flags);
		return -EFAULT;
	}
	spin_unlock_irqrestore(&(dm75xx_device->lock), irq_flags);
	/*
	 * Mapping memory into userspace is different in 2.4 and 2.6
	 */

	if (remap_pfn_range(vma,
			    vma->vm_start,
			    (dm75xx_device->
			     dma_buffers[channel].bus_address >> PAGE_SHIFT),
			    vm_size, vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

/*=============================================================================
Module entry point definitions
 =============================================================================*/

module_init(dm75xx_load);
module_exit(dm75xx_unload);
