/**
    @file

    @brief
        Kernel compatibility issues between 2.6.0 and 3.x kernels

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

    $Id: dm75xx_kernel.h 65703 2012-12-13 17:04:04Z rgroner $
*/

#ifndef __dm75xx_kernel_h__
#define __dm75xx_kernel_h__

#include <asm/ptrace.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/dma-mapping.h>

/**
 * @defgroup DM75xx_Kernel_Header DM75xx kernel compatibility header file
 * @{
 */

/*=============================================================================
Sanity checking on RTD kernel version macro set in make file
 =============================================================================*/

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)

#error "This driver does not support pre-2.6.0 kernels"

#endif

/*
 * SA_SHIRQ has become deprecated in versions after 2.6.18
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 18)
#define IRQF_SHARED     SA_SHIRQ
#endif

/**
 * @} DM75xx_Kernel_Module_Major_Minor_Number_Macros
 */

/*=============================================================================
Interrupt handlers
 =============================================================================*/

/**
 * @defgroup DM75xx_Kernel_Interrupt_Handler_Macros DM75xx kernel compatibility interrupt handler macros
 * @{
 */

/**
 * @def INTERRUPT_HANDLER_TYPE
 * @brief
 *      Type returned by interrupt handler.
 */

/**
 * @typedef dm75xx_handler_t
 * @brief
 *      Type definition for interrupt handling function.
 */

/*
 * 2.6 kernel interrupt handlers return a value indicating whether or not the
 * interrupt was able to be processed.
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)

#define INTERRUPT_HANDLER_TYPE  static irqreturn_t
typedef irqreturn_t(*dm75xx_handler_t) (int, void *, struct pt_regs *);

#else

#define INTERRUPT_HANDLER_TYPE static irqreturn_t
typedef irqreturn_t(*dm75xx_handler_t) (int, void *);

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19) */

/**
 * @} DM75xx_Kernel_Interrupt_Handler_Macros
 */

/**
 * @defgroup DM75xx_Kernel_File_Ops_Struct_Macros kernel compatibility interrupt handler macros
 * @{
 */

 /**
 * @def DM75XX_IOCTL
 * @brief
 *      In Kernel 2.6.35, .ioctl was replaced with .unlocked_ioctl
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)

#define     DM75XX_IOCTL  .ioctl

#else

#define     DM75XX_IOCTL  .unlocked_ioctl

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35) */

/**
 * @} DM75xx_Kernel_File_Ops_Struct_Macros
 */

/*=============================================================================
Access to device I/O memory
 =============================================================================*/

/**
 * @defgroup DM75xx_Kernel_Device_IO_Memory_Access_Macros DM75xx kernel compatibility device I/O memory access macros
 * @{
 */

/**
 * @def IO_MEMORY_READ8
 * @brief
 *      Entity which reads an 8-bit value from device I/O memory
 */

/**
 * @def IO_MEMORY_READ16
 * @brief
 *      Entity which reads a 16-bit value from device I/O memory
 */

/**
 * @def IO_MEMORY_READ32
 * @brief
 *      Entity which reads a 32-bit value from device I/O memory
 */

/**
 * @def IO_MEMORY_WRITE8
 * @brief
 *      Entity which writes an 8-bit value to device I/O memory
 */

/**
 * @def IO_MEMORY_WRITE16
 * @brief
 *      Entity which writes a 16-bit value to device I/O memory
 */

/**
 * @def IO_MEMORY_WRITE32
 * @brief
 *      Entity which writes a 32-bit value to device I/O memory
 */

/*
 * Theoretically, the address returned from ioremap() or ioremap_nocache()
 * should not be used to directly access memory.  This may work on some
 * architectures but may fail on others.  Therefore, special techniques must be
 * used to access device I/O memory to make code portable.  Some 2.6
 * kernels (2.6.8 and older) share the same access methods whereas other 2.6
 * kernels (2.6.9 and newer) use a different scheme.
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 9)

#define IO_MEMORY_READ8     readb
#define IO_MEMORY_READ16    readw
#define IO_MEMORY_READ32    readl
#define IO_MEMORY_WRITE8    writeb
#define IO_MEMORY_WRITE16   writew
#define IO_MEMORY_WRITE32   writel

#else

#define IO_MEMORY_READ8     ioread8
#define IO_MEMORY_READ16    ioread16
#define IO_MEMORY_READ32    ioread32
#define IO_MEMORY_WRITE8    iowrite8
#define IO_MEMORY_WRITE16   iowrite16
#define IO_MEMORY_WRITE32   iowrite32

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 9) */

/**
 * @} DM75xx_Kernel_Header
 */

#endif /* __dm75xx_kernel_h__ */
