///////////////////////////////////////////////////////////////////////////////
// Kernel module used by the ECI API for hardware access
//
// Copyright (C) 2008-2012  IXXAT Automation GmbH
//
// This program is free software; you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// A copy of the GNU General Public License can be downloaded from
// <http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

/**
  Implementation of the ECI kernel module

  @file ecikm.c
*/

///////////////////////////////////////////////////////////////////////////////
// compiler directives


///////////////////////////////////////////////////////////////////////////////
// include files

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/msi.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>

#include "OsLnxIoctl.h"
#include "ProductVersion.rh"

#ifndef PCI_DEVID
#define PCI_DEVID(bus, devfn)  ((((u16)bus) << 8) | devfn)
#endif

#if __GNUC__ ==  4 && __GNUC_MINOR__ < 1 && (defined(__i386__) || defined(__x86_64__))
#define __eci_sync_lock_test_and_set(a,b,result)    __asm__ __volatile__ ( \
                                                    "lock; xchgl %0, (%1)\n" \
                                                    : "=r" (result) : "r" (a), "0" (b) : "cc", "memory");

#define __eci_sync_fetch_and_or(a,b,result)  __asm__ __volatile__ ( \
                                             "lock orl %0, (%1)\n" \
                                             : "=r" (result) : "r" (a), "0" (b) : "cc", "memory");
#endif

///////////////////////////////////////////////////////////////////////////////
// User adaptability

// Debugging
//#define __DEBUG__
//#define __DEBUG_ISR__

// Enables ISA/104 Bus scanning while loading the ECI kernel driver.
// Found and supported device will be added automatically (without configured
// interrupt line)
// ATTENTION: Scanning the ISA bus may have negative effects on other ISA hardware
//#define ECI_ENABLE_ISA_BUS_SCAN


// This define can be used to easily add some ISA/104 device while loading the
// ECI kernel driver.
// isa_add() has following parameters.
// isa_add(Base_Address, Address_Range, IRQ, Device_ID)
// e.g.: isa_add(0x000D4000, 0x0400, 5, ECI_DEVICE_I04_104 );
//       isa_add(0x000D8000, 0x0400, 6, ECI_DEVICE_I320_104);
#define ECI_ADD_ISA_DEVICES \
{ \
  /*isa_add(0x000D4000, 0x0400, 5, ECI_DEVICE_I04_104 );*/ \
  /*isa_add(0x000D8000, 0x0400, 6, ECI_DEVICE_I320_104);*/ \
}

///////////////////////////////////////////////////////////////////////////////
// static constants, types, macros, variables

#define ECI_DRIVERNAME            "eci"
#define ECI_CLASSNAME             "eci device"
#define ECI_MAXBOARDCOUNT         64      // Maximum number of supported devices
#define ECI_DEVICENAMELENGTH      64

#define ECI_DEVICE_I04_PCI        0x0002
#define ECI_DEVICE_I04_104        0x0003
#define ECI_DEVICE_I320_PCI       0x0101
#define ECI_DEVICE_I320_104       0x0102
#define ECI_DEVICE_IXC16_PCI      0x0105
#define ECI_DEVICE_IXC16_PCIE     0x010E
#define ECI_DEVICE_FR_IB100_PCIE  0x0110
#define ECI_DEVICE_CAN_IB100_PCIE 0x0005
#define ECI_DEVICE_CAN_IB120_PCIE 0x1005
#define ECI_DEVICE_CAN_IB130_PCIE 0x2005
#define ECI_DEVICE_CAN_IB300_PCI  0x3005
#define ECI_DEVICE_CAN_IB500_PCIE 0x4005
#define ECI_DEVICE_CAN_IB520_PCIE 0x5005
#define ECI_DEVICE_CAN_IB530_PCIE 0x6005
#define ECI_DEVICE_CAN_IB700_PCI  0x7005
#define ECI_DEVICE_CAN_IB200_PCIE 0x0111
#define ECI_DEVICE_CAN_IB400_PCI  0x1111
#define ECI_DEVICE_CAN_IB230_PCIE 0x2111
#define ECI_DEVICE_CAN_IB600_PCIE 0x3111
#define ECI_DEVICE_CAN_IB800_PCI  0x4111
#define ECI_DEVICE_CAN_IB630_PCIE 0x5111
#define ECI_DEVICE_CAN_IB110_XMC  0x8005
#define ECI_DEVICE_CAN_IB310_PMC  0x9005
#define ECI_DEVICE_CAN_IB510_XMC  0xA005
#define ECI_DEVICE_CAN_IB710_PMC  0xB005
#define ECI_DEVICE_CAN_IB210_XMC  0x6111
#define ECI_DEVICE_CAN_IB410_PMC  0x7111
#define ECI_DEVICE_CAN_IB610_XMC  0x8111
#define ECI_DEVICE_CAN_IB810_PMC  0x9111


// older 2.6 kernels did not support IRQF_SHARED
#ifndef IRQF_SHARED
#define IRQF_SHARED SA_SHIRQ
#endif


// IXXAT Vendor, Sub Vendor, Device, and Sub Device ID
#define IXXAT_VENDOR_ID                     0x1BEE
#define IXXAT_SUB_VENDOR_ID                 0x1BEE
#define INVALID_DEVICE_ID                   0xFFFF
#define INVALID_SUB_DEVICE_ID               0xFFFF

// Vendor, Sub Vendor, Device, and Sub Device ID of PC-I 04/PCI Card
#define PC_I04_VENDOR_ID                    0x10B5
#define PC_I04_SUB_VENDOR_ID                0x10B5
#define PC_I04_DEVICE_ID                    0x9050
#define PC_I04_SUB_DEVICE_ID                0x2540

// Vendor, Sub Vendor, Device, and Sub Device ID of iPC-I 320/PCI Card
#define IPC_I320_VENDOR_ID                  0x10B5
#define IPC_I320_SUB_VENDOR_ID              0x10B5
#define IPC_I320_DEVICE_ID                  0x9050
#define IPC_I320_SUB_DEVICE_ID              0x1128

// Vendor, Sub Vendor, Device, and Sub Device ID of iPC-I XC16/PCI Card
#define IPC_IXC16_VENDOR_ID                 0x10B5
#define IPC_IXC16_SUB_VENDOR_ID             0x10B5
#define IPC_IXC16_DEVICE_ID                 0x9030
#define IPC_IXC16_SUB_DEVICE_ID             0x2977

// Vendor, Sub Vendor, Device, and Sub Device ID of iPC-I XC16/PCIe Card
#define IPC_IXC16E_VENDOR_ID                0x10B5
#define IPC_IXC16E_SUB_VENDOR_ID            0x10B5
#define IPC_IXC16E_DEVICE_ID                0x9056
#define IPC_IXC16E_SUB_DEVICE_ID            0x3268

// Vendor, Sub Vendor, Device, and Sub Device ID of PCIe-to-PCI Bridge for iPC-I XC16/PCIe Card
#define BRIDGE_PLX_PEX8311_VENDOR_ID        0x10B5
#define BRIDGE_PLX_PEX8311_DEVICE_ID        0x8111

// Vendor, Sub Vendor, Device, and Sub Device ID of FR-IB100/PCIe Card
#define FR_IB100_VENDOR_ID                  0x1BEE
#define FR_IB100_SUB_VENDOR_ID              0x1BEE
#define FR_IB100_DEVICE_ID                  0x0001
#define FR_IB100_SUB_DEVICE_ID              0x0001

// Size of DMA Buffer 1MiB required DMA page size 1MiB => Add one page
#define FR_IB100_DMA_LEN                    (2*1024*1024);
#define FR_IB100_ADDMEM_LEN                 PAGE_SIZE

// Vendor and Sub Vendor ID of CAN-IB1x0/PCIe Card
#define CAN_IB1X0_VENDOR_ID                 0x1BEE
#define CAN_IB1X0_SUB_VENDOR_ID             0x1BEE

// Device and Sub Device ID of CAN-IB100/PCIe Card
#define CAN_IB100_DEVICE_ID                 0x0002
#define CAN_IB100_SUB_DEVICE_ID             0x0002

/* Device and Sub Device ID of CAN-IB110/XMC Card */
#define CAN_IB110_DEVICE_ID         0x0013
#define CAN_IB110_SUB_DEVICE_ID     0x0013

// Device and Sub Device ID of CAN-IB300/PCI Card
#define CAN_IB300_DEVICE_ID                 0x0010
#define CAN_IB300_SUB_DEVICE_ID             0x0010

/* Device and Sub Device ID of CAN-IB310/PMC Card */
#define CAN_IB310_DEVICE_ID      0x0015
#define CAN_IB310_SUB_DEVICE_ID  0x0015

// Device and Sub Device ID of CAN-IB120/PCIe Mini Card
#define CAN_IB120_DEVICE_ID                 0x0004
#define CAN_IB120_SUB_DEVICE_ID             0x0004

// Device and Sub Device ID of CAN-IB130/PCIe 104 Card
#define CAN_IB130_DEVICE_ID                 0x0005
#define CAN_IB130_SUB_DEVICE_ID             0x0005

// Device and Sub Device ID of CAN-IB500/PCIe Card
#define CAN_IB500_DEVICE_ID                 0x000e
#define CAN_IB500_SUB_DEVICE_ID             0x000e

// Device and Sub Device ID of CAN-IB700/PCI Card
#define CAN_IB700_DEVICE_ID                 0x0019
#define CAN_IB700_SUB_DEVICE_ID             0x0019

/* Device and Sub Device ID of CAN-IB700/PMC Card */
#define CAN_IB710_DEVICE_ID      0x001A
#define CAN_IB710_SUB_DEVICE_ID  0x001A

/* Device and Sub Device ID of CAN-IB510/XMC Card */
#define CAN_IB510_DEVICE_ID         0x0017
#define CAN_IB510_SUB_DEVICE_ID     0x0017

// Device and Sub Device ID of CAN-IB520/PCIe Mini Card
#define CAN_IB520_DEVICE_ID                 0x0012
#define CAN_IB520_SUB_DEVICE_ID             0x0012

// Device and Sub Device ID of CAN-IB530/PCIe 104 Card
#define CAN_IB530_DEVICE_ID                 0x002C
#define CAN_IB530_SUB_DEVICE_ID             0x002C

// Vendor and Sub Vendor ID of CAN-IB200/PCIe Card
#define CAN_IB2X0_VENDOR_ID                 0x1BEE
#define CAN_IB2X0_SUB_VENDOR_ID             0x1BEE

// Device and Sub Device ID of CAN-IB200/PCIe Card
#define CAN_IB200_DEVICE_ID                 0x0003
#define CAN_IB200_SUB_DEVICE_ID             0x0003

/* Device and Sub Device ID of CAN-IB210/XMC Card */
#define CAN_IB210_DEVICE_ID      0x0014
#define CAN_IB210_SUB_DEVICE_ID  0x0014

// Device and Sub Device ID of CAN-IB400/PCI Card
#define CAN_IB400_DEVICE_ID                 0x0011
#define CAN_IB400_SUB_DEVICE_ID             0x0011

/* Device and Sub Device ID of CAN-IB410/PMC Card */
#define CAN_IB410_DEVICE_ID      0x0016
#define CAN_IB410_SUB_DEVICE_ID  0x0016

// Device and Sub Device ID of CAN-IB230/PCIe 104
#define CAN_IB230_DEVICE_ID                 0x0006
#define CAN_IB230_SUB_DEVICE_ID             0x0006

// Device and Sub Device ID of CAN-IB600/PCIe Card
#define CAN_IB600_DEVICE_ID                 0x000f
#define CAN_IB600_SUB_DEVICE_ID             0x000f

/* Device and Sub Device ID of CAN-IB610/XMC Card */
#define CAN_IB610_DEVICE_ID      0x0018
#define CAN_IB610_SUB_DEVICE_ID  0x0018

// Device and Sub Device ID of CAN-IB800/PCI Card
#define CAN_IB800_DEVICE_ID                 0x001b
#define CAN_IB800_SUB_DEVICE_ID             0x001b

/* Device and Sub Device ID of CAN-IB810/PMC Card */
#define CAN_IB810_DEVICE_ID      0x001c
#define CAN_IB810_SUB_DEVICE_ID  0x001c

// Device and Sub Device ID of CAN-IB630/PCIe 104
#define CAN_IB630_DEVICE_ID                 0x002d
#define CAN_IB630_SUB_DEVICE_ID             0x002d

// Size of DMA Buffer 512kiB required DMA page size 4kiB => Add one page
#define CAN_IB2X0_DMA_LEN                   (512*1024 + 4*1024);
#define CAN_IB2X0_ADDMEM_LEN                PAGE_SIZE

// Some useful PCI(e) control registers
#define PLX_9050_LCR_INTCSR                 0x004C
#define PLX_9050_LCR_CNTRL                  0x0050

#define PLX_9030_LCR_INTCSR                 0x004C

#define PLX_9056_LCR_INTCSR                 0x0068
#define PLX_8311_GPIOCTL                    0x1020

#define PCIE_ALTERA_LCR_INTCSR              0x0040
#define PCIE_ALTERALCR_A2P_INTENA           0x0050
#define PCIE_ALTERA_LCR_A2P_MAILBOXRO       0x0900

#define PCIE_ALTERA_LCR_A2P_MAILBOX_COUNT   8

#define SJA_ADDR_SR                         2
#define SJA_ADDR_ISR                        3

#define IPC_I320_104_IDENTIFIER             0x49435069
#define IPC_I320_104_IDENTIFIER_OFFSET      0
#define IPC_I320_104_IDENTIFIER_STRING      "iPCI320"
#define IPC_I320_104_VENDOR_OFFSET          16
#define IPC_I320_104_VENDOR_STRING          "IXXAT"


#define ECI_ISA_ADDRESS_LENGTH              0x0400
#define ECI_ISA_ADDRESS_STEP                0x0400
#define ECI_ISA_ADDRESS_START               0x000C0000
#define ECI_ISA_ADDRESS_END                 0x000FFC00


// For accessing some shadowed interrupt registers
struct __attribute__((aligned(4))) AlteraIsrMailboxShadow
{
  unsigned int dwInterruptStatus;    ///< Shadowed interrupt status register
  unsigned int dwReserved;           ///< Reserved, used for 8 BYTE alignment
  struct
  {
    unsigned int adwA2PMailBox;      ///< Shadowed mailbox register
    unsigned int dwReserved;         ///< Reserved, used for 8 BYTE alignment
  }MBX[PCIE_ALTERA_LCR_A2P_MAILBOX_COUNT];
};


///////////////////////////////////////////////////////////////////////////////
// global variables

static struct class *eci_class                = { 0 };
static        dev_t major                     = { 0 };
static struct proc_dir_entry *eci_proc_entry  = { 0 };

static char * boardname[] =
{
  "PC-I 04/PCI",
  "PC-I 04/104",
  "iPC-I 320/PCI",
  "iPC-I 320/104",
  "iPC-I XC16/PCI",
  "iPC-I XC16/PCIe",
  "FR-IB100/PCIe",
  "CAN-IB100/PCIe",
  "CAN-IB120/PCIe Mini",
  "CAN-IB130/PCIe 104",
  "CAN-IB500/PCIe",
  "CAN-IB520/PCIe Mini",
  "CAN-IB530/PCIe 104",
  "CAN-IB200/PCIe",
  "CAN-IB230/PCIe 104",
  "CAN-IB600/PCIe",
  "CAN-IB630/PCIe 104",
  "CAN-IB300/PCI",
  "CAN-IB700/PCI",
  "CAN-IB400/PCI",
  "CAN-IB800/PCI",
  "CAN-IB110/XMC",
  "CAN-IB310/PMC",
  "CAN-IB510/XMC",
  "CAN-IB710/PMC",
  "CAN-IB210/XMC",
  "CAN-IB410/PMC",
  "CAN-IB610/XMC",
  "CAN-IB810/PMC",
};

///////////////////////////////////////////////////////////////////////////////
// static function prototypes

static int    probe(struct pci_dev *dev, const struct pci_device_id *id);
static void   remove(struct pci_dev *dev);
int           mmap(struct file *file, struct vm_area_struct *vma);
int           open(struct inode *inode, struct file * file);
int           release(struct inode *inode, struct file * file);
int           ioctl(struct inode * inode, struct file * file, unsigned int ioctl, unsigned long ioctl_param);
long          unlocked_ioctl(struct file * file, unsigned int ioctl, unsigned long ioctl_param);
unsigned int  poll(struct file * file, poll_table * wait);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
ssize_t       eci_read_info(struct file *file, char __user *buf, size_t size, loff_t *ppos);
#endif

///////////////////////////////////////////////////////////////////////////////
// global structures
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)
  static struct pci_device_id device_ids[] =
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
  static struct pci_device_id device_ids[] __devinitconst =
#else
  static struct pci_device_id device_ids[] =
#endif
{
  { vendor:      PC_I04_VENDOR_ID,
    device:      PC_I04_DEVICE_ID,
    subvendor:   PC_I04_SUB_VENDOR_ID,
    subdevice:   PC_I04_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      IPC_I320_VENDOR_ID,
    device:      IPC_I320_DEVICE_ID,
    subvendor:   IPC_I320_SUB_VENDOR_ID,
    subdevice:   IPC_I320_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0  },
  { vendor:      IPC_IXC16_VENDOR_ID,
    device:      IPC_IXC16_DEVICE_ID,
    subvendor:   IPC_IXC16_SUB_VENDOR_ID,
    subdevice:   IPC_IXC16_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      IPC_IXC16E_VENDOR_ID,
    device:      IPC_IXC16E_DEVICE_ID,
    subvendor:   IPC_IXC16E_SUB_VENDOR_ID,
    subdevice:   IPC_IXC16E_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      FR_IB100_VENDOR_ID,
    device:      FR_IB100_DEVICE_ID,
    subvendor:   FR_IB100_SUB_VENDOR_ID,
    subdevice:   FR_IB100_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB100_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB100_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB110_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB110_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB300_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB300_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB310_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB310_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB120_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB120_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB130_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB130_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB500_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB500_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB510_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB510_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB700_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB700_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB710_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB710_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB520_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB520_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB1X0_VENDOR_ID,
    device:      CAN_IB530_DEVICE_ID,
    subvendor:   CAN_IB1X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB530_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB200_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB200_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB210_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB210_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB400_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB400_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB410_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB410_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB230_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB230_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB600_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB600_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB610_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB610_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB800_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB800_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB810_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB810_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { vendor:      CAN_IB2X0_VENDOR_ID,
    device:      CAN_IB630_DEVICE_ID,
    subvendor:   CAN_IB2X0_SUB_VENDOR_ID,
    subdevice:   CAN_IB630_SUB_DEVICE_ID,
    class:       0,
    class_mask:  0,
    driver_data: 0 },
  { 0, }
};

MODULE_DEVICE_TABLE(pci, device_ids);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
static const struct file_operations proc_file_fops = {
 .owner = THIS_MODULE,
 .read  = eci_read_info,
};
#endif

static struct pci_driver pci_driver =
{
  .name     = ECI_DRIVERNAME,
  .id_table = device_ids,
  .probe    = probe,
  .remove   = remove,
};

static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .open           = open,
    .release        = release,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
    .ioctl          = ioctl,
#else
    .unlocked_ioctl = unlocked_ioctl,
#endif
#ifdef HAVE_COMPAT_IOCTL
    .compat_ioctl   = unlocked_ioctl,
#endif
    .poll           = poll,
    .mmap           = mmap,
};

struct device_data_t
{
   /* virtual and real memory pointers */
   /* Interface PCI(e) registers 1*/
   unsigned long               reg1add;
   void                      * reg1vadd;
   unsigned long               reg1len;
   /* Interface PCI(e) registers 2*/
   unsigned long               reg2add;
   void                      * reg2vadd;
   unsigned long               reg2len;
   /* Interface PCI(e) memory */
   unsigned long               memadd;
   void                      * memvadd;
   unsigned long               memlen;
   /* coherent DMA memory */
   unsigned long               dmaadd;
   void                      * dmavadd;
   unsigned long               dmalen;
   /* additional special purpose memory */
   void                      * addmemvadd;
   unsigned long               addmemlen;

   /* user mode wake-up mechanism */
   wait_queue_head_t           waitqueue;
   int                         board_ready;

   /* device identification */
   dev_t                       devnum;
   struct cdev                 cdev;
   int                         cardnumber;
   unsigned int                usage_count;
   struct device             * dev;
   struct getdevicetype_param  s_deviceid;
   int                         devicetype;
   char                        name[ECI_DEVICENAMELENGTH];

   /* Device interrupt line*/
   unsigned int                device_irq;

   /* interrupt helper functions */
   unsigned int (*IntGetStat)(struct device_data_t * pDeviceData);
   bool         (*IntEnaReq) (struct device_data_t * pDeviceData, bool fEnable);
   void         (*IntClrReq) (struct device_data_t * pDeviceData);

  #ifdef __DEBUG__
     /* interrupt debugging */
     unsigned int              real_irqcnt;
     unsigned int              usr_irqcnt;
     unsigned int              miss_irqcnt;
  #endif //__DEBUG__
  
   int owner_pid;
   struct pci_dev             * pci_dev;
   int                         msi_enabled;
   bool                        use_msi;
};

static struct device_data_t * device_list[ECI_MAXBOARDCOUNT];


///////////////////////////////////////////////////////////////////////////////
// local functions

int isa_request_irq(struct device_data_t* pDeviceData, unsigned int irq);


///////////////////////////////////////////////////////////////////////////////
// global functions

///////////////////////////////////////////////////////////////////////////////
/**
This function reserves a number in the device array, and sets the
pointer to the device data structure

@param pDeviceData
pointer to the device_data_t structure

@return
negative error code or number or index of the device in the array
*/
int reservecardnumber(struct device_data_t * pDeviceData)
{
  int i = 0;

  for(i = 0; i < ECI_MAXBOARDCOUNT; i++)
  {
    if(device_list[i] == NULL)
    {
      device_list[i] = pDeviceData;
      pDeviceData->cardnumber = i;
      return i;
    }
  }
  return -EINVAL;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function removes a specified pointer to an device_data_t structure from
the array

@param pDeviceData
pointer to the device_data_t structure

@return
negative error code or number or index of the device in the array
*/
int freecardnumber(struct device_data_t * pDeviceData)
{
  int i = 0;

  for(i = 0; i < ECI_MAXBOARDCOUNT; i++)
  {
    if(device_list[i] == pDeviceData)
    {
      device_list[i] = NULL;
      pDeviceData->cardnumber = -1;
      return i;
    }
  }
  return -EINVAL;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function returns a pointer to a string containing the name of the device
specified by deviceid

@param deviceid
device id

@return
pointer to a char array containing the device name
*/
char * gethwname(int deviceid)
{
  char * devicename = NULL;

  switch(deviceid)
  {
    case ECI_DEVICE_I04_PCI:
    {
      devicename = boardname[0];
      break;
    }
    case ECI_DEVICE_I04_104:
    {
      devicename = boardname[1];
      break;
    }
    case ECI_DEVICE_I320_PCI:
    {
      devicename = boardname[2];
      break;
    }
    case ECI_DEVICE_I320_104:
    {
      devicename = boardname[3];
      break;
    }
    case ECI_DEVICE_IXC16_PCI:
    {
      devicename = boardname[4];
      break;
    }
    case ECI_DEVICE_IXC16_PCIE:
    {
      devicename = boardname[5];
      break;
    }
    case ECI_DEVICE_FR_IB100_PCIE:
    {
      devicename = boardname[6];
      break;
    }
    case ECI_DEVICE_CAN_IB100_PCIE:
    {
      devicename = boardname[7];
      break;
    }
    case ECI_DEVICE_CAN_IB120_PCIE:
    {
      devicename = boardname[8];
      break;
    }
    case ECI_DEVICE_CAN_IB130_PCIE:
    {
      devicename = boardname[9];
      break;
    }
    case ECI_DEVICE_CAN_IB500_PCIE:
    {
      devicename = boardname[10];
      break;
    }
    case ECI_DEVICE_CAN_IB520_PCIE:
    {
      devicename = boardname[11];
      break;
    }
    case ECI_DEVICE_CAN_IB530_PCIE:
    {
      devicename = boardname[12];
      break;
    }
    case ECI_DEVICE_CAN_IB200_PCIE:
    {
      devicename = boardname[13];
      break;
    }
    case ECI_DEVICE_CAN_IB230_PCIE:
    {
      devicename = boardname[14];
      break;
    }
    case ECI_DEVICE_CAN_IB600_PCIE:
    {
      devicename = boardname[15];
      break;
    }
    case ECI_DEVICE_CAN_IB630_PCIE:
    {
      devicename = boardname[16];
      break;
    }
    case ECI_DEVICE_CAN_IB300_PCI:
    {
      devicename = boardname[17];
      break;
    }
    case ECI_DEVICE_CAN_IB700_PCI:
    {
      devicename = boardname[18];
      break;
    }
    case ECI_DEVICE_CAN_IB400_PCI:
    {
      devicename = boardname[19];
      break;
    }

    case ECI_DEVICE_CAN_IB800_PCI:
    {
      devicename = boardname[20];
      break;
    }

    case ECI_DEVICE_CAN_IB110_XMC:
    {
      devicename = boardname[21];
      break;
    }

    case ECI_DEVICE_CAN_IB310_PMC:
    {
      devicename = boardname[22];
      break;
    }

    case ECI_DEVICE_CAN_IB510_XMC:
    {
      devicename = boardname[23];
      break;
    }

    case ECI_DEVICE_CAN_IB710_PMC:
    {
      devicename = boardname[24];
      break;
    }

    case ECI_DEVICE_CAN_IB210_XMC:
    {
      devicename = boardname[25];
      break;
    }

    case ECI_DEVICE_CAN_IB410_PMC:
    {
      devicename = boardname[26];
      break;
    }

    case ECI_DEVICE_CAN_IB610_XMC:
    {
      devicename = boardname[27];
      break;
    }

    case ECI_DEVICE_CAN_IB810_PMC:
    {
      devicename = boardname[28];
      break;
    }

    default:
    {
      break;
    }
  }
  return devicename;
}


///////////////////////////////////////////////////////////////////////////////
/**
  This function retrieves the current device hardware interrupt status.

@param pDeviceData
pointer to a device_data_t structure

@return
  The function returns the current interrupt status of the device. If
  the device did not cause an interrupt the function must return 0.
*/
unsigned int IntGetStat_PLX_9050(struct device_data_t * pDeviceData)
{
  unsigned char regval = 0;
  unsigned int  result = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread8(pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR);
    result = regval & 0x024;

    #ifdef __DEBUG_ISR__
      if(result)
        { printk("ECI: %s reg: 0x%px, val:        0x%08X\n", __func__, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR, result); }
    #endif //__DEBUG_ISR__
  }
  return result;
}


///////////////////////////////////////////////////////////////////////////////
/**
  This function retrieves the current device hardware interrupt status.

@param pDeviceData
pointer to a device_data_t structure

@return
  The function returns the current interrupt status of the device. If
  the device did not cause an interrupt the function must return 0.
*/
unsigned int IntGetStat_PLX_9030(struct device_data_t * pDeviceData)
{
  unsigned char regval = 0;
  unsigned int  result = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread8(pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR);
    result = regval & 0x04;

    #ifdef __DEBUG_ISR__
      if(result)
        { printk("ECI: %s reg: 0x%px, val:        0x%08X\n", __func__, pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR, result); }
    #endif //__DEBUG_ISR__
  }
  return result;
}


///////////////////////////////////////////////////////////////////////////////
/**
  This function retrieves the current device hardware interrupt status.

@param pDeviceData
pointer to a device_data_t structure

@return
  The function returns the current interrupt status of the device. If
  the device did not cause an interrupt the function must return 0.
*/
unsigned int IntGetStat_PLX_9056(struct device_data_t * pDeviceData)
{
  unsigned int regval = 0;
  unsigned int result = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread32(pDeviceData->reg1vadd + PLX_9056_LCR_INTCSR);
    result = regval & 0x00008000;

    #ifdef __DEBUG_ISR__
      if(result)
        { printk("ECI: %s reg: 0x%px, val:        0x%08X\n", __func__, pDeviceData->reg1vadd + PLX_9056_LCR_INTCSR, result); }
    #endif //__DEBUG_ISR__
  }
  return result;
}


///////////////////////////////////////////////////////////////////////////////
/**
  This function retrieves the current device hardware interrupt status.

@param pDeviceData
pointer to a device_data_t structure

@return
  The function returns the current interrupt status of the device. If
  the device did not cause an interrupt the function must return 0.
*/
unsigned int IntGetStat_PCIE_ALTERA(struct device_data_t * pDeviceData)
{
  unsigned int regval = 0;
  unsigned int result = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread32(pDeviceData->reg1vadd + PCIE_ALTERA_LCR_INTCSR);
    result = regval & 0x00FF0080;

    #ifdef __DEBUG_ISR__
      if(result)
        { printk("ECI: %s reg: 0x%px, val:        0x%08X\n", __func__, pDeviceData->reg1vadd + PCIE_ALTERA_LCR_INTCSR, result); }
    #endif //__DEBUG_ISR__
  }
  return result;
}


///////////////////////////////////////////////////////////////////////////////
/**
  This function retrieves the current device hardware interrupt status.

@param pDeviceData
pointer to a device_data_t structure

@return
  The function returns the current interrupt status of the device. If
  the device did not cause an interrupt the function must return 0.
*/
unsigned int IntGetStat_ISA(struct device_data_t * pDeviceData)
{
  #ifdef __DEBUG_ISR__
    printk("ECI: %s\n", __func__);
  #endif //__DEBUG_ISR__

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function enables or disables hardware interrupt requests from the device.

@param fEnable
Flag to turn on/off hardware interrupt requests.

@return
true if interrupt was enabled before calling this function, otherwise false
*/
bool IntEnaReq_PLX_9050_Passive(struct device_data_t * pDeviceData, bool fEnable)
{
  bool          fResult = false;
  unsigned char regval  = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    //*** Do not disable for passive interfaces (this is done in user-mode after handling the request)
    fEnable = false;

    regval = ioread8(pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR);

    fResult = 0 != (regval & 0x09);

    #ifdef __DEBUG_ISR__
      printk("ECI: %s  reg: 0x%px, val before: 0x%02X\n", __func__, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__

    if(fEnable)
      { iowrite8(regval | 0x09, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR); }
    else
      { iowrite8(regval & 0xF6, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR); }

    #ifdef __DEBUG_ISR__
      regval = ioread8(pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR);
      printk("ECI: %s  reg: 0x%px, val after:  0x%02X\n", __func__, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__
  }
  return fResult;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function enables or disables hardware interrupt requests from the device.

@param fEnable
Flag to turn on/off hardware interrupt requests.

@return
true if interrupt was enabled before calling this function, otherwise false
*/
bool IntEnaReq_PLX_9050_Active(struct device_data_t * pDeviceData, bool fEnable)
{
  bool          fResult = false;
  unsigned char regval  = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread8(pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR);

    fResult = 0 != (regval & 0x09);

    #ifdef __DEBUG_ISR__
      printk("ECI: %s  reg: 0x%px, val before: 0x%02X\n", __func__, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__

    if(fEnable)
      { iowrite8(regval | 0x09, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR); }
    else
      { iowrite8(regval & 0xF6, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR); }

    #ifdef __DEBUG_ISR__
      regval = ioread8(pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR);
      printk("ECI: %s  reg: 0x%px, val after:  0x%02X\n", __func__, pDeviceData->reg1vadd + PLX_9050_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__
  }
  return fResult;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function enables or disables hardware interrupt requests from the device.

@param fEnable
Flag to turn on/off hardware interrupt requests.

@return
true if interrupt was enabled before calling this function, otherwise false
*/
bool IntEnaReq_PLX_9030(struct device_data_t * pDeviceData, bool fEnable)
{
  bool          fResult = false;
  unsigned char regval  = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread8(pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR);

    fResult = 0 != (regval & 0x01);

    #ifdef __DEBUG_ISR__
      printk("ECI: %s  reg: 0x%px, val before: 0x%02X\n", __func__, pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__

    if(fEnable)
      { iowrite8(regval | 0x01, pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR); }
    else
      { iowrite8(regval & 0xF6, pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR); }

    #ifdef __DEBUG_ISR__
      regval = ioread8(pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR);
      printk("ECI: %s  reg: 0x%px, val after:  0x%02X\n", __func__, pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__
   }
   return fResult;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function enables or disables hardware interrupt requests from the device.

@param fEnable
Flag to turn on/off hardware interrupt requests.

@return
true if interrupt was enabled before calling this function, otherwise false
*/
bool IntEnaReq_PLX_9056(struct device_data_t * pDeviceData, bool fEnable)
{
  bool         fResult = false;
  unsigned int regval  = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread32(pDeviceData->reg1vadd + PLX_9056_LCR_INTCSR);

    fResult = 0 != (regval & 0x00000800);

    #ifdef __DEBUG_ISR__
      printk("ECI: %s  reg: 0x%px, val before: 0x%08X\n", __func__, pDeviceData->reg1vadd + PLX_9056_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__

    if(fEnable)
      { iowrite32(regval | 0x00000800, pDeviceData->reg1vadd + PLX_9056_LCR_INTCSR); }
    else
      { iowrite32(regval & 0xFFFFF7FF, pDeviceData->reg1vadd + PLX_9056_LCR_INTCSR); }

    #ifdef __DEBUG_ISR__
      regval = ioread32(pDeviceData->reg1vadd + PLX_9056_LCR_INTCSR);
      printk("ECI: %s  reg: 0x%px, val after:  0x%08X\n", __func__, pDeviceData->reg1vadd + PLX_9056_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__
   }
   return fResult;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function enables or disables hardware interrupt requests from the device.

@param fEnable
Flag to turn on/off hardware interrupt requests.

@return
true if interrupt was enabled before calling this function, otherwise false
*/
bool IntEnaReq_PCIE_ALTERA_Passive(struct device_data_t * pDeviceData, bool fEnable)
{
  bool         fResult = false;
  unsigned int regval  = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    //*** Do not disable for passive interfaces (this is done in user-mode after handling the request)
    fEnable = false;

    regval = ioread32(pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA);

    fResult = 0 != (regval & 0x00000080);

    #ifdef __DEBUG_ISR__
      printk("ECI: %s reg: 0x%px, val before: 0x%08X\n", __func__, pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA, regval);
    #endif //__DEBUG_ISR__

    if(fEnable)
      { iowrite32(regval | 0x00000080, pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA); }
    else
      { iowrite32(regval & 0xFFFFFF7F, pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA); }

    #ifdef __DEBUG_ISR__
      regval = ioread32(pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA);
      printk("ECI: %s reg: 0x%px, val after:  0x%08X\n", __func__, pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA, regval);
    #endif //__DEBUG_ISR__
  }
  return fResult;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function enables or disables hardware interrupt requests from the device.

@param fEnable
Flag to turn on/off hardware interrupt requests.

@return
true if interrupt was enabled before calling this function, otherwise false
*/
bool IntEnaReq_PCIE_ALTERA_Active(struct device_data_t * pDeviceData, bool fEnable)
{
  bool         fResult = false;
  unsigned int regval  = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread32(pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA);

    fResult = 0 != (regval & 0x00FF0000);

    #ifdef __DEBUG_ISR__
      printk("ECI: %s  reg: 0x%px, val before: 0x%08X\n", __func__, pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA, regval);
    #endif //__DEBUG_ISR__

    if(fEnable)
      { iowrite32(regval | 0x00FF0000, pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA); }
    else
      { iowrite32(regval & 0xFF00FFFF, pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA); }

    #ifdef __DEBUG_ISR__
      regval = ioread32(pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA);
      printk("ECI: %s  reg: 0x%px, val after:  0x%08X\n", __func__, pDeviceData->reg1vadd + PCIE_ALTERALCR_A2P_INTENA, regval);
    #endif //__DEBUG_ISR__
   }
   return fResult;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function enables or disables hardware interrupt requests from the device.

@param fEnable
Flag to turn on/off hardware interrupt requests.

@return
true if interrupt was enabled before calling this function, otherwise false
*/
bool IntEnaReq_ISA(struct device_data_t * pDeviceData, bool fEnable)
{
  #ifdef __DEBUG_ISR__
    printk("ECI: %s\n", __func__);
  #endif //__DEBUG_ISR__

  return true;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function clears hardware interrupt requests from the device after
the interrupt has been serviced.
*/
void IntClrReq_PLX_9050(struct device_data_t * pDeviceData)
{
  unsigned char regval = 0;
  int           i      = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread8(pDeviceData->reg1vadd + PLX_9050_LCR_CNTRL + 1);

    regval &= 0xF6;
    iowrite8(regval, pDeviceData->reg1vadd + PLX_9050_LCR_CNTRL + 1);

    regval |= 0x09;
    for(i = 0; i < 5; i++)
      { iowrite8(regval, pDeviceData->reg1vadd + PLX_9050_LCR_CNTRL + 1); }

    regval &= 0xF6;
    iowrite8(regval, pDeviceData->reg1vadd + PLX_9050_LCR_CNTRL + 1);
  }
  return;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function clears hardware interrupt requests from the device after
the interrupt has been serviced.
*/
void IntClrReq_PLX_9030(struct device_data_t * pDeviceData)
{
  unsigned char regval = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    regval = ioread8(pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR + 1);

    regval |= 0x04;
    iowrite8(regval, pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR + 1);

    regval &= 0xFB;
    iowrite8(regval, pDeviceData->reg1vadd + PLX_9030_LCR_INTCSR + 1);
  }
  return;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function clears hardware interrupt requests from the device after
the interrupt has been serviced.
*/
void IntClrReq_PLX_8311(struct device_data_t * pDeviceData)
{
  unsigned int regval = 0;

  if(NULL != pDeviceData->reg2vadd)
  {
    regval = ioread32(pDeviceData->reg2vadd + PLX_8311_GPIOCTL);

    regval |= 0x00000004;
    iowrite32(regval, pDeviceData->reg2vadd + PLX_8311_GPIOCTL);

    regval &= 0xFFFFFFFB;
    iowrite32(regval, pDeviceData->reg2vadd + PLX_8311_GPIOCTL);
  }
  return;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function clears hardware interrupt requests from the device after
the interrupt has been serviced.
*/
void IntClrReq_PCIE_ALTERA(struct device_data_t * pDeviceData)
{
  unsigned int regval = 0;
  unsigned int status = 0;
  unsigned int result = 0;

  if(NULL != pDeviceData->reg1vadd)
  {
    // read interrupt flags
    regval = ioread32(pDeviceData->reg1vadd + PCIE_ALTERA_LCR_INTCSR);

    // mask relevant sources
    status = regval & 0x00FF0080;
    if(status && (NULL != pDeviceData->addmemvadd))
    {
      struct AlteraIsrMailboxShadow * pShadow  = pDeviceData->addmemvadd;
      unsigned int *                  pMailbox = pDeviceData->reg1vadd + PCIE_ALTERA_LCR_A2P_MAILBOXRO;
      unsigned int                    irqsrc   = status >> 16;
      unsigned int                    idx      = 0;

      // Update all mailbox shadow register
      for(idx=0; idx<PCIE_ALTERA_LCR_A2P_MAILBOX_COUNT; idx++)
      {
        if(irqsrc & 0x01)
        {
          #ifdef __DEBUG_ISR__
            printk( "ECI: %s  reg: 0x%px, Mailbox(%u): 0x%08X\n", __func__, pMailbox, idx, *pMailbox);
          #endif //__DEBUG_ISR__

          // Update Mailbox shadow register
          if(0 != idx)
          { 
#if __GNUC__ ==  4 && __GNUC_MINOR__ < 1 && (defined(__i386__) || defined(__x86_64__))
            __eci_sync_lock_test_and_set(&pShadow->MBX[idx].adwA2PMailBox, *pMailbox, result); // replace completely (assign)
#else
            result = __sync_lock_test_and_set(&pShadow->MBX[idx].adwA2PMailBox, *pMailbox); // replace completely (assign)
#endif
          
          } 
          else
          { 
#if __GNUC__ ==  4 && __GNUC_MINOR__ < 1 && (defined(__i386__) || defined(__x86_64__))
            __eci_sync_fetch_and_or(&pShadow->MBX[idx].adwA2PMailBox, *pMailbox, result); // replace completely (assign)
#else
            result = __sync_fetch_and_or(&pShadow->MBX[idx].adwA2PMailBox, *pMailbox); // Add new flags (or)
#endif
        }
        }
        irqsrc >>= 1;
        pMailbox++;
      }

      // Update Interrupt status shadow register
#if __GNUC__ ==  4 && __GNUC_MINOR__ < 1 && (defined(__i386__) || defined(__x86_64__))
     __eci_sync_fetch_and_or(&pShadow->dwInterruptStatus, status, result); // replace completely (assign)
     
#else
     result = __sync_fetch_and_or(&pShadow->dwInterruptStatus, status);  // Add new flags (or)
#endif
    }

    #ifdef __DEBUG_ISR__
      printk("ECI: %s  reg: 0x%px, val:        0x%08X\n", __func__, pDeviceData->reg1vadd + PCIE_ALTERA_LCR_INTCSR, regval);
    #endif //__DEBUG_ISR__

    // reset interrupt flags
    iowrite32(regval, pDeviceData->reg1vadd + PCIE_ALTERA_LCR_INTCSR);
  }
  return;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function clears hardware interrupt requests from the device after
the interrupt has been serviced.
*/
void IntClrReq_ISA(struct device_data_t * pDeviceData)
{
  #ifdef __DEBUG_ISR__
    printk("ECI: %s\n", __func__);
  #endif //__DEBUG_ISR__

  return;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function is the generic interrupt handler for all supported interfaces

@param irq
Interrupt number

@param dev_id
pointer to the device_data_t structure

@return
IRQ_HANDLED if it is our interrupt, IRQ_NONE otherwise
*/
irqreturn_t irq_handler(int irq, void *dev_id)
{
  struct device_data_t * pDeviceData = (struct device_data_t *)dev_id;
  bool                   fEnable     = false;

  // test if our card caused the interrupt
  if(pDeviceData->IntGetStat)
  {
    if(pDeviceData->IntGetStat(pDeviceData))
    {
      // disable interrupt
      fEnable = pDeviceData->IntEnaReq(pDeviceData, false);

      // clear the hardware interrupt request
      pDeviceData->IntClrReq(pDeviceData);

      // Only if interrupt was enabled
      if(fEnable)
      {
        // enable interrupt
        pDeviceData->IntEnaReq(pDeviceData, true);

        //MC has an information for processing
        #ifdef __DEBUG__
          if(pDeviceData->board_ready == 1)
            { pDeviceData->miss_irqcnt++; }
          pDeviceData->real_irqcnt++;
        #endif //__DEBUG__
        pDeviceData->board_ready = 1;

        wake_up_interruptible(&pDeviceData->waitqueue);
      }

      return IRQ_HANDLED;
    }
  }
  return IRQ_NONE;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the probe function for our driver. It will be called by the kernel for every
device matching the values in the device_ids array.

@param dev
pointer to the pci_dev structure

@param id
pointer to the pci_device_id structure matching the current device

@return
0 if we want to handle the device
*/
static int probe(struct pci_dev * dev, const struct pci_device_id *id)
{
  int                     result      = 1;
  unsigned long           tmpregadd   = 0;
  unsigned long           tmpreglen   = 0;
  struct device_data_t  * pDeviceData = kzalloc( sizeof(struct device_data_t), GFP_KERNEL );

  #ifdef __DEBUG__
    printk("ECI: %s pci_dev: 0x%lx\n", __func__, (unsigned long) dev);
  #endif //__DEBUG__

  init_waitqueue_head(&pDeviceData->waitqueue);

  pci_set_drvdata(dev, pDeviceData);

  result = pci_enable_device(dev);
  if(result != 0)
  {
    printk(KERN_ALERT "ECI: pci_enable_device failed\n");
    goto pci_enable_failed;
  }

  reservecardnumber(pDeviceData);

  printk("ECI: New device probed: %s\n", pci_name(dev));

  #ifdef __DEBUG__
    printk("ECI:   VendorID:    %x\n", id->vendor);
    printk("ECI:   DeviceID:    %x\n", id->device);
    printk("ECI:   SubVendorID: %x\n", id->subvendor);
    printk("ECI:   SubDeviceID: %x\n", id->subdevice);
  #endif

  // Store device and vendor IDs
  pDeviceData->s_deviceid.wVendorId     = id->vendor;
  pDeviceData->s_deviceid.wDeviceId     = id->device;
  pDeviceData->s_deviceid.wSubVendorId  = id->subvendor;
  pDeviceData->s_deviceid.wSubSystemId  = id->subdevice;

  // IXXAT PC-I 04/PCI Card
  if(      (pDeviceData->s_deviceid.wVendorId    == PC_I04_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == PC_I04_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == PC_I04_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == PC_I04_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_I04_PCI;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PLX_9050;
    pDeviceData->IntEnaReq  = IntEnaReq_PLX_9050_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PLX_9050;
  }
  // IXXAT iPC-I 320/PCI Card
  else if( (pDeviceData->s_deviceid.wVendorId    == IPC_I320_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == IPC_I320_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == IPC_I320_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == IPC_I320_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_I320_PCI;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PLX_9050;
    pDeviceData->IntEnaReq  = IntEnaReq_PLX_9050_Active;
    pDeviceData->IntClrReq  = IntClrReq_PLX_9050;
  }
  // IXXAT iPC-I XC16/PCI Card
  else if( (pDeviceData->s_deviceid.wVendorId    == IPC_IXC16_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == IPC_IXC16_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == IPC_IXC16_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == IPC_IXC16_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_IXC16_PCI;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PLX_9030;
    pDeviceData->IntEnaReq  = IntEnaReq_PLX_9030;
    pDeviceData->IntClrReq  = IntClrReq_PLX_9030;
  }
  // IXXAT iPC-I XC16/PCIe Card
  else if( (pDeviceData->s_deviceid.wVendorId    == IPC_IXC16E_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == IPC_IXC16E_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == IPC_IXC16E_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == IPC_IXC16E_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_IXC16_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PLX_9056;
    pDeviceData->IntEnaReq  = IntEnaReq_PLX_9056;
    pDeviceData->IntClrReq  = IntClrReq_PLX_8311;
  }
  // IXXAT FR-IB100/PCIe Card
  else if( (pDeviceData->s_deviceid.wVendorId    == FR_IB100_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == FR_IB100_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == FR_IB100_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == FR_IB100_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_FR_IB100_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = FR_IB100_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = FR_IB100_ADDMEM_LEN;
  }
  // IXXAT CAN-IB100/PCIe Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB100_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB100_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB100_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB110/XMC Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB110_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB110_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB110_XMC;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB300/PCI Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB300_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB300_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB300_PCI;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB310/PMC Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB310_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB310_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB310_PMC;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB120/PCIe Mini Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB120_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB120_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB120_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB130/PCIe 104 Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB130_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB130_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB130_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB500/PCIe Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB500_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB500_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB500_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB510/XMC Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB510_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB510_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB510_XMC;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB700/PCI Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB700_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB700_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB700_PCI;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB710/PMC Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB710_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB710_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB710_PMC;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB520/PCIe Mini Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB520_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB520_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB520_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB530/PCIe 104 Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB1X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB530_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB1X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB530_SUB_DEVICE_ID ) )
  {
    pDeviceData->devicetype = ECI_DEVICE_CAN_IB530_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Passive;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
  }
  // IXXAT CAN-IB200/PCIe Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB200_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB200_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB200_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB210/XMC Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB210_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB210_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB210_XMC;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB400/PCI Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB400_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB400_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB400_PCI;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB410/PMC Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB410_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB410_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB410_PMC;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB230/PCIe 104 Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB230_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB230_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB230_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB600/PCIe Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB600_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB600_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB600_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB610/XMC Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB610_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB610_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB610_XMC;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB800/PCI Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB800_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB800_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB800_PCI;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB810/PMC Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB810_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB810_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB810_PMC;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }
  // IXXAT CAN-IB630/PCIe 104 Card
  else if( (pDeviceData->s_deviceid.wVendorId    == CAN_IB2X0_VENDOR_ID     ) &&
           (pDeviceData->s_deviceid.wDeviceId    == CAN_IB630_DEVICE_ID     ) &&
           (pDeviceData->s_deviceid.wSubVendorId == CAN_IB2X0_SUB_VENDOR_ID ) &&
           (pDeviceData->s_deviceid.wSubSystemId == CAN_IB630_SUB_DEVICE_ID ) )
  {
    pci_set_master(dev);

    pDeviceData->devicetype = ECI_DEVICE_CAN_IB630_PCIE;
    // Set-up interrupt helpers
    pDeviceData->IntGetStat = IntGetStat_PCIE_ALTERA;
    pDeviceData->IntEnaReq  = IntEnaReq_PCIE_ALTERA_Active;
    pDeviceData->IntClrReq  = IntClrReq_PCIE_ALTERA;
    // Add additional DMA buffer
    pDeviceData->dmalen     = CAN_IB2X0_DMA_LEN;
    // Add additional memory for interrupt status mapping
    pDeviceData->addmemlen  = CAN_IB2X0_ADDMEM_LEN;

    // Try to use MSI interrupts
    pDeviceData->use_msi = true;
  }

  printk("ECI: Card seems to be of type: (0x%04X) %s\n", pDeviceData->devicetype, gethwname(pDeviceData->devicetype));

  // Find PCIe - PCI Bridge for IXC16/PCIe
  if(ECI_DEVICE_IXC16_PCIE == pDeviceData->devicetype)
  {
    struct pci_dev* p_bridge_dev = NULL;
    printk( "ECI: Trying to find PEX 8311 PCIe-to-PCI Bridge for %s card\n", gethwname(pDeviceData->devicetype) );

    if(dev)
    {
      if(dev->bus)
      {
        if(dev->bus->self)
        {
          if( (dev->bus->self->vendor == BRIDGE_PLX_PEX8311_VENDOR_ID ) &&
              (dev->bus->self->device == BRIDGE_PLX_PEX8311_DEVICE_ID ))
          {
            p_bridge_dev = dev->bus->self;
            printk ( "ECI: PCI Bridge device %04x:%04x subsystem %04x:%04x irq %d found\n",
                     p_bridge_dev->vendor,
                     p_bridge_dev->device,
                     p_bridge_dev->subsystem_vendor,
                     p_bridge_dev->subsystem_device,
                     p_bridge_dev->irq);

            // Request physical PCI Bridge register address
            pDeviceData->reg2add = pci_resource_start( p_bridge_dev, 0 );
            pDeviceData->reg2len = pci_resource_len( p_bridge_dev, 0 );
            if(!pDeviceData->reg2add)
            {
              printk(KERN_ALERT "ECI: pci_resource_start for Bridge device failed\n");
              goto pci_resource_start_bridge_failed;
            }

            // Map PCI Bridge registers to virtual kernel address
            request_mem_region( pDeviceData->reg2add, pDeviceData->reg2len, "ECI PCI Bridge Registers" );
            pDeviceData->reg2vadd = ioremap_nocache( pDeviceData->reg2add, pDeviceData->reg2len );
          }
        }
      }
    }

    if(NULL == p_bridge_dev)
    {
      printk(KERN_ALERT "ECI: PCI Bridge device not found!\n");
      goto pci_resource_start_bridge_failed;
    }
  }

  // DMA requested?
  if(0 != pDeviceData->dmalen)
  {
    pDeviceData->dmavadd = pci_alloc_consistent(dev, pDeviceData->dmalen, (dma_addr_t*) &pDeviceData->dmaadd);
    if(NULL == pDeviceData->dmavadd)
    {
      printk(KERN_WARNING "ECI: pci_alloc_consistent failed\n" );
    }
    else
    {
      memset(pDeviceData->dmavadd, 0x00, pDeviceData->dmalen);
    }
    }

  // Additional memory requested?
  if(0 != pDeviceData->addmemlen)
  {
    pDeviceData->addmemvadd = kzalloc( pDeviceData->addmemlen, GFP_KERNEL );
    if(NULL == pDeviceData->addmemvadd)
    {
      printk(KERN_WARNING "ECI: kzalloc failed\n" );
    }
  }

  // Request physical PCI register address
  pDeviceData->reg1add = pci_resource_start( dev, 0 );
  pDeviceData->reg1len = pci_resource_len( dev, 0 );
  if(!pDeviceData->reg1add)
  {
    printk(KERN_ALERT "ECI: pci_resource_start failed\n");
    goto pci_resource_start_failed;
  }

  // Do some special checking for I320/PCI or I04/PCI (PLX9050)
  if((ECI_DEVICE_I320_PCI == pDeviceData->devicetype) ||
     (ECI_DEVICE_I04_PCI  == pDeviceData->devicetype) )
  {
    tmpregadd = pci_resource_start( dev, 3 );
    tmpreglen = pci_resource_len( dev, 3 );

    if(test_bit (7, (void*) &pDeviceData->reg1add))
    {
      if(0 == tmpregadd)
      {
        printk(KERN_ALERT "ECI: PCI Region 3 not available, you need to update the EEPROM of your %s card\n", gethwname(pDeviceData->devicetype) );
        printk(KERN_ALERT "ECI: Please contact our support for further instructions on this\n");
        goto pci_resource_start_failed;
      }
      else
      {
        printk("ECI: Using PCI Region 3\n");
        pDeviceData->reg1add = tmpregadd;
        pDeviceData->reg1len = tmpreglen;

        pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, (unsigned int) -1);
        pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, pDeviceData->reg1add);
      }
    }
  }

  // Request physical PCI memory register address
  pDeviceData->memadd = pci_resource_start( dev, 2 );
  pDeviceData->memlen = pci_resource_len( dev, 2 );
  if(!pDeviceData->memadd)
  {
    printk(KERN_ALERT "ECI: pci_resource_start failed\n");
    goto pci_resource_start_failed;
  }

  // Map PCI registers to virtual kernel address
  request_mem_region( pDeviceData->reg1add, pDeviceData->reg1len, "ECI PCI Registers" );
  pDeviceData->reg1vadd = ioremap_nocache( pDeviceData->reg1add, pDeviceData->reg1len );

  // Map DPRAM to virtual kernel address
  request_mem_region( pDeviceData->memadd, pDeviceData->memlen, "ECI PCI Memory" );
  pDeviceData->memvadd = ioremap_nocache( pDeviceData->memadd, pDeviceData->memlen );

  // Register IRQ handler
  pDeviceData->board_ready = 0;
  snprintf( pDeviceData->name , sizeof(pDeviceData->name), "eci%d", pDeviceData->cardnumber);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
  /* Try to enable MSI-interrupts */
  if(pDeviceData->use_msi == true && pci_enable_msi(dev)) {
    pDeviceData->msi_enabled = 0;
  } else {
    pDeviceData->msi_enabled = 1;
  }
#else
  /* Try to enable MSI-interrupts */
  if(pDeviceData->use_msi == true && pci_alloc_irq_vectors(dev, 1, 1, PCI_IRQ_MSI) > 0) {
    pDeviceData->msi_enabled = 1;
  } else {
    pDeviceData->msi_enabled = 0;
  }
#endif

  result = request_irq(dev->irq, irq_handler, IRQF_SHARED, pDeviceData->name, pDeviceData);
  if(result)
  {
    printk(KERN_ALERT "ECI: Request_irq failed assigning irq %d\n", dev->irq);
    goto request_irq_failed;
  }
  else
  {
    printk("ECI: Successfully registered interrupt handler for irq %d\n", dev->irq);
    pDeviceData->device_irq = dev->irq;
  }

  printk("ECI: Assigned card number %d (so device file should be /dev/%s)\n",pDeviceData->cardnumber, pDeviceData->name);

  #ifdef __DEBUG__
    printk(  "ECI:   pDeviceData->reg1add:    0x%lx\n", pDeviceData->reg1add );
    printk(  "ECI:   pDeviceData->reg1len:    0x%lx\n", pDeviceData->reg1len );
    printk(  "ECI:   pDeviceData->reg1vadd:   0x%lx\n", (unsigned long) pDeviceData->reg1vadd );
    if(0 != pDeviceData->reg2len)
    {
      printk(  "ECI:   pDeviceData->reg2add:    0x%lx\n", pDeviceData->reg2add );
      printk(  "ECI:   pDeviceData->reg2len:    0x%lx\n", pDeviceData->reg2len );
      printk(  "ECI:   pDeviceData->reg2vadd:   0x%lx\n", (unsigned long) pDeviceData->reg2vadd );
    }
    printk(  "ECI:   pDeviceData->memadd:     0x%lx\n", pDeviceData->memadd );
    printk(  "ECI:   pDeviceData->memlen:     0x%lx\n", pDeviceData->memlen );
    printk(  "ECI:   pDeviceData->memvadd:    0x%lx\n", (unsigned long) pDeviceData->memvadd );
    if(0 != pDeviceData->dmalen)
    {
      printk("ECI:   pDeviceData->dmaadd:     0x%lx\n", pDeviceData->dmaadd );
      printk("ECI:   pDeviceData->dmalen:     0x%lx\n", pDeviceData->dmalen );
      printk("ECI:   pDeviceData->dmavadd:    0x%lx\n", (unsigned long) pDeviceData->dmavadd );
    }
    if(0 != pDeviceData->addmemlen)
    {
      printk("ECI:   pDeviceData->addmemlen:  0x%lx\n", pDeviceData->addmemlen );
      printk("ECI:   pDeviceData->addmemvadd: 0x%lx\n", (unsigned long) pDeviceData->addmemvadd );
    }
  #endif

  // Create char device file
  pDeviceData->devnum = MKDEV( MAJOR(major), pDeviceData->cardnumber);
  cdev_init(&(pDeviceData->cdev), &fops);
  pDeviceData->cdev.owner = THIS_MODULE;
  result = cdev_add(&(pDeviceData->cdev), pDeviceData->devnum, 1);
  if(result)
  {
    printk(KERN_ALERT "ECI: Failed registering char device\n" );
    goto cdev_add_failed;
  }
  else
  {
    printk("ECI: Successfully registered char device\n" );
  }

  pDeviceData->pci_dev = dev;

  // create sysfs device file
  // kernel versions prior 2.6.27 have to use device_create_drvdata() here
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2, 27, 0)
  pDeviceData->dev = device_create(eci_class, NULL, pDeviceData->devnum, "%s", pDeviceData->name);
  #else
  pDeviceData->dev = device_create(eci_class, NULL, pDeviceData->devnum, NULL, "%s", pDeviceData->name);
  #endif
  if(!pDeviceData->dev)
  {
    printk(KERN_ALERT "ECI: Error creating sysfs file\n" );
    goto device_create_failed;
  }
  return 0;

device_create_failed:
  cdev_del(&(pDeviceData->cdev));

cdev_add_failed:
  free_irq(dev->irq, pDeviceData);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
  if(pDeviceData->msi_enabled)
    pci_disable_msi(dev);
#else
  if(pDeviceData->msi_enabled)
    pci_free_irq_vectors(dev);
#endif

request_irq_failed:
  iounmap(pDeviceData->memvadd);
  release_mem_region(pDeviceData->memadd,pDeviceData->memlen);

  iounmap(pDeviceData->reg1vadd);
  release_mem_region(pDeviceData->reg1add,pDeviceData->reg1len);

pci_resource_start_failed:
  if(0 != pDeviceData->reg2len)
  {
    iounmap(pDeviceData->reg2vadd);
    release_mem_region(pDeviceData->reg2add,pDeviceData->reg2len);
  }

pci_resource_start_bridge_failed:
  freecardnumber(pDeviceData);
  pci_disable_device(dev);

pci_enable_failed:
  kfree(pDeviceData);

  return -ENODEV;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the remove function for our driver. It will be called by the kernel if a device
is removed from the system.

@param dev
pointer to the pci_dev structure
*/
static void remove(struct pci_dev *dev)
{
  struct device_data_t * pDeviceData = pci_get_drvdata(dev);

  #ifdef __DEBUG__
    printk("ECI: %s pci_dev: 0x%lx\n", __func__, (unsigned long) dev);
  #endif //__DEBUG__

  freecardnumber(pDeviceData);

  // destroy sysfs device
  device_destroy(eci_class,pDeviceData->devnum);

  cdev_del(&(pDeviceData->cdev));

  free_irq(dev->irq, pDeviceData);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
  if(pDeviceData->msi_enabled)
    pci_disable_msi(dev);
#else
  if(pDeviceData->msi_enabled)
    pci_free_irq_vectors(dev);
#endif

  // release DMA
  if(NULL != pDeviceData->dmavadd)
  {
    pci_free_consistent(dev, pDeviceData->dmalen, pDeviceData->dmavadd, pDeviceData->dmaadd);
    pDeviceData->dmavadd = NULL;
    pDeviceData->dmaadd  = 0;
  }

  // release additional memory
  if(NULL != pDeviceData->addmemvadd)
  {
    kfree(pDeviceData->addmemvadd);
    pDeviceData->addmemvadd = NULL;
    pDeviceData->addmemlen  = 0;
  }

  iounmap(pDeviceData->memvadd);
  release_mem_region(pDeviceData->memadd,pDeviceData->memlen);

  iounmap(pDeviceData->reg1vadd);
  release_mem_region(pDeviceData->reg1add,pDeviceData->reg1len);

  if(0 != pDeviceData->reg2len)
  {
    iounmap(pDeviceData->reg2vadd);
    release_mem_region(pDeviceData->reg2add,pDeviceData->reg2len);
  }

  pci_disable_device(dev);

  kfree(pDeviceData);
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the ioctl function for our char device. It will be called when a userspace
application calls ioctl on the associated device node.
*/
int eci_ioctl(struct file * file, unsigned int ioctl, unsigned long ioctl_param)
{
  int                   ret         = -EINVAL;
  struct device_data_t* pDeviceData = file->private_data;

  #ifdef __DEBUG__
    printk("ECI: %s pci_dev: 0x%lx, ioctl: 0x%x\n", __func__, (unsigned long) pDeviceData, ioctl);
  #endif //__DEBUG__

  switch (ioctl)
  {
    case IOCTL_GETREGMAPOFFSET:
    {
      unsigned long * pdwparam = (unsigned long *) ioctl_param;
      ret = put_user(pDeviceData->reg1add & 0xFFF, pdwparam);
      break;
    }

    case IOCTL_GETREG2MAPOFFSET:
    {
      unsigned long * pdwparam = (unsigned long *) ioctl_param;
      ret = put_user(pDeviceData->reg2add & 0xFFF, pdwparam);
      break;
    }

    case IOCTL_GETDATAMAPOFFSET:
    {
      unsigned long * pdwparam = (unsigned long *) ioctl_param;
      ret = put_user(pDeviceData->memadd & 0xFFF, pdwparam);
      break;
    }

    case IOCTL_GETADDMEMMAPOFFSET:
    {
      unsigned long * pdwparam = (unsigned long *) ioctl_param;
      ret = put_user((unsigned long)(virt_to_phys(pDeviceData->addmemvadd)) & 0xFFF, pdwparam);
      break;
    }

    case IOCTL_GETDEVICETYPE:
    {
      struct getdevicetype_param * psDevType = (struct getdevicetype_param *) ioctl_param;
      ret = copy_to_user(psDevType, &pDeviceData->s_deviceid, sizeof(struct getdevicetype_param));
      break;
    }

    case IOCTL_GETDEVICEINFO:
    {
      struct getdeviceinfo_param * psDevInfo = (struct getdeviceinfo_param *) ioctl_param;
      struct getdeviceinfo_param   sDevInfo  = {0};
      sDevInfo.lPciRegister1  = pDeviceData->reg1add;
      sDevInfo.lPciRegister2  = pDeviceData->reg2add;
      sDevInfo.lDevMemory     = pDeviceData->memadd;
      sDevInfo.dwDevIrq       = pDeviceData->device_irq;
      ret = copy_to_user(psDevInfo, &sDevInfo, sizeof(struct getdeviceinfo_param));
      break;
    }

    case IOCTL_GETDMAINFO:
    {
      struct getdmainfo_param * psDmaInfo = (struct getdmainfo_param *) ioctl_param;
      struct getdmainfo_param   sDmaInfo  = {0};
      ret = copy_from_user(&sDmaInfo, psDmaInfo, sizeof(struct getdmainfo_param));
      if(0 == ret)
      {
        switch(sDmaInfo.lDmaIndex)
        {
          // DMA 0
          case OSLNXKM_DMA_MAP_OFFSET + 0:
          {
            sDmaInfo.lDmaOffset = pDeviceData->dmaadd & 0xFFF;
            sDmaInfo.lPhysAddr  = pDeviceData->dmaadd;
            break;
          }

          default:
          {
            printk(KERN_WARNING "ECI: IOCTL_GETDMAINFO - unknown DMA index %llu!\n", sDmaInfo.lDmaIndex);
            ret = -EINVAL;
          }
        }
      }
      if(0 == ret)
      {
        ret = copy_to_user(psDmaInfo, &sDmaInfo, sizeof(struct getdmainfo_param));
      }
      break;
    }

    case IOCTL_SETDEVICEIRQ:
    {
      unsigned int device_irq = (unsigned int) ioctl_param;
      #ifdef __DEBUG__
        printk("ECI: IOCTL_SETDEVICEIRQ - Setting device irq %u\n", device_irq);
      #endif //__DEBUG__
      ret = isa_request_irq(pDeviceData, device_irq);
      break;
    }

    case IOCTL_GETDEVID:
    {
      uint32_t  devId  = 0;
      uint32_t* pDevId = (uint32_t*) ioctl_param;
      int bus_nr = 0, domain_nr = 0;

      if(pDeviceData->pci_dev->bus) {
        bus_nr = pDeviceData->pci_dev->bus->number;
        if((domain_nr = pci_domain_nr(pDeviceData->pci_dev->bus)) < 0)
          domain_nr = 0;
      }
        
      devId = PCI_DEVID(bus_nr, pDeviceData->pci_dev->devfn );
      devId |= domain_nr << 16;
    
      ret = copy_to_user(pDevId, &devId, sizeof(devId));

      #ifdef __DEBUG__
        printk("ECI: IOCTL_GETDEVID - Device id %x\n", *pDevId);
      #endif //__DEBUG__

      break;
    }

    default:
    {
      printk(KERN_WARNING "ECI: Unknown IOCTL call 0x%X!\n", ioctl);
      ret = -EBADRQC;
    }
  }

  if(0 < ret)
  {
    printk(KERN_WARNING "ECI: IOCTL copy to or from user failed (%u)!\n", ret);
    ret = -EIO;
  }

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the ioctl function for our char device. It will be called when a userspace
application calls ioctl on the associated device node.
*/
int ioctl(struct inode * inode, struct file * file, unsigned int ioctl, unsigned long ioctl_param)
{
  return eci_ioctl(file, ioctl, ioctl_param);
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the ioctl function for our char device. It will be called when a userspace
application calls ioctl on the associated device node.
*/
long unlocked_ioctl(struct file * file, unsigned int ioctl, unsigned long ioctl_param)
{
  return eci_ioctl(file, ioctl, ioctl_param);
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the poll function for our char device. It will be called when a userspace application
issues a poll or select call on the associated device node.
*/
unsigned int poll (struct file * file, poll_table * wait)
{
  int                     ret         = 0;
  struct device_data_t  * pDeviceData = file->private_data;

  poll_wait( file, &pDeviceData->waitqueue, wait );
  if( pDeviceData->board_ready > 0 )
  {
    pDeviceData->board_ready = 0;
    #ifdef __DEBUG__
      pDeviceData->usr_irqcnt++;
    #endif //__DEBUG__
    ret = POLLIN | POLLRDNORM;
  }

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the open function for our char device. It will be called when a userspace application
opens the associated device file node.
*/
int open (struct inode *inode, struct file * file)
{
  struct device_data_t * pDeviceData;
  pDeviceData = container_of(inode->i_cdev, struct device_data_t, cdev);

  #ifdef __DEBUG__
    printk("ECI: %s inode: 0x%px, file: 0x%px owner pid %d current pid %d current tgid %d\n", __func__, inode, file, pDeviceData->owner_pid, current->pid, current->tgid);
  #endif //__DEBUG__

  if(!(file->f_flags & O_NONBLOCK))
  {
    if(!pDeviceData->owner_pid)
        pDeviceData->owner_pid = current->tgid;

    if(pDeviceData->owner_pid != current->tgid)
    {
      #ifdef __DEBUG__
        printk("ECI: %s inode: 0x%px, file: 0x%px => EBUSY\n", __func__, inode, file);
      #endif //__DEBUG__
      return -EBUSY;
    }
    else
    {
      #ifdef __DEBUG__
        pDeviceData->real_irqcnt = 0;
        pDeviceData->usr_irqcnt  = 0;
        pDeviceData->miss_irqcnt = 0;
      #endif //__DEBUG__

        pDeviceData->usage_count++;
    }
  }
  #ifdef __DEBUG__
  else
  {
    printk("ECI: %s inode: 0x%px, file: 0x%px => O_NONBLOCK\n", __func__, inode, file);
  }
  #endif //__DEBUG__

  file->private_data = pDeviceData;

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the release function for our char device. It will be called when a userspace application
closes the associated device file node.
*/
int release(struct inode *inode, struct file * file)
{
  struct device_data_t * pDeviceData = file->private_data;

  #ifdef __DEBUG__
    printk("ECI: %s inode: 0x%px, file: 0x%px pid %d\n", __func__, inode, file, current->pid);
  #endif //__DEBUG__

  if(pDeviceData->owner_pid == current->tgid &&
     !(file->f_flags & O_NONBLOCK))
    pDeviceData->usage_count--;

  if(pDeviceData->usage_count == 0 &&
    !(file->f_flags & O_NONBLOCK))
  {
    pDeviceData->owner_pid = 0;
    
    //---disable interrupt
    if(pDeviceData->IntEnaReq)
      { pDeviceData->IntEnaReq(pDeviceData, false); }

    #ifdef __DEBUG__
      printk( "ECI: releasing last instance, irq source disabled\n");
      printk( "ECI: irq count real: %d, user: %d, miss: %d\n",
              pDeviceData->real_irqcnt,
              pDeviceData->usr_irqcnt,
              pDeviceData->miss_irqcnt);

      pDeviceData->real_irqcnt = 0;
      pDeviceData->usr_irqcnt  = 0;
      pDeviceData->miss_irqcnt = 0;
    #endif //__DEBUG__
  }


  return 0;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the mmap function for our char device. It will be called when a userspace application
issues an mmap call on the associated device file. We switch our mapping depending on the
vm_pgoff parameter in the vm_area_struct structure. Using this mechanism the user can chose
whether he wants to map the PCI registers or the dpram.
*/
int mmap (struct file *file, struct vm_area_struct *vma)
{
    struct device_data_t * pDeviceData = file->private_data;
    int                    result      = -EINVAL;
    off_t                  offset      = vma->vm_pgoff;
    unsigned long          vsize       = vma->vm_end - vma->vm_start;

    if((offset == OSLNXKM_DEVMEM_MAP_INDEX) &&
       (0      == pDeviceData->memadd) )
      { return -ENODEV; }

    if((offset == OSLNXKM_PCIREG1_MAP_INDEX) &&
       (0      == pDeviceData->reg1add))
      { return -ENODEV; }

    vma->vm_private_data = pDeviceData; // for future use in mmap fops

    switch(offset)
    {
      case OSLNXKM_PCIREG1_MAP_INDEX:
      { //map PCI registers
        vma->vm_page_prot    = pgprot_noncached(vma->vm_page_prot);
        result = io_remap_pfn_range (vma,
                                     vma->vm_start,
                                     pDeviceData->reg1add >> PAGE_SHIFT,
                                     vsize,
                                     vma->vm_page_prot);
        if(result)
        {
          printk("ECI: io_remap_pfn_range failed, PCI reg. mmap aborted\n");
          return -EAGAIN;
        }
        else
        {
          #ifdef __DEBUG__
          printk( "ECI: PCI reg. mmap successful, from 0x%lx(actually:0x%lx) to 0x%lx-0x%lx vsize %ld\n",
                  pDeviceData->reg1add,
                  (pDeviceData->reg1add >> PAGE_SHIFT) << PAGE_SHIFT,
                  vma->vm_start,
                  vma->vm_start+vsize,
                  vsize );
          #endif
        }
        break;
      }

      case OSLNXKM_PCIREG2_MAP_INDEX:
      { //map PCI registers 2
        vma->vm_page_prot    = pgprot_noncached(vma->vm_page_prot);
        result = io_remap_pfn_range (vma,
                                     vma->vm_start,
                                     pDeviceData->reg2add >> PAGE_SHIFT,
                                     vsize,
                                     vma->vm_page_prot);
        if(result)
        {
          printk("ECI: io_remap_pfn_range failed, PCI reg. 2 mmap aborted\n");
          return -EAGAIN;
        }
        else
        {
          #ifdef __DEBUG__
          printk( "ECI: PCI reg. 2 mmap successful, from 0x%lx(actually:0x%lx) to 0x%lx-0x%lx vsize %ld\n",
                  pDeviceData->reg2add,
                  (pDeviceData->reg2add >> PAGE_SHIFT) << PAGE_SHIFT,
                  vma->vm_start,
                  vma->vm_start+vsize,
                  vsize );
          #endif
        }
        break;
      }

      case OSLNXKM_DEVMEM_MAP_INDEX:
      { //map memory
        vma->vm_page_prot    = pgprot_noncached(vma->vm_page_prot);
        result = io_remap_pfn_range (vma,
                                     vma->vm_start,
                                     pDeviceData->memadd >> PAGE_SHIFT,
                                     vsize,
                                     vma->vm_page_prot);
        if(result)
        {
          printk("ECI: io_remap_pfn_range failed, Memory mmap aborted\n");
          return -EAGAIN;
        }
        else
        {
          #ifdef __DEBUG__
          printk( "ECI: Memory   mmap successful, from 0x%lx(actually:0x%lx) to 0x%lx-0x%lx vsize %ld\n",
                  pDeviceData->memadd,
                  (pDeviceData->memadd >> PAGE_SHIFT) << PAGE_SHIFT,
                  vma->vm_start,
                  vma->vm_start+vsize,
                  vsize );
          #endif
        }
        break;
      }

      case OSLNXKM_ADDMEM_MAP_INDEX:
      { //map Additional memory
        if(NULL != pDeviceData->addmemvadd)
        {
          result = remap_pfn_range (vma,
                                    vma->vm_start,
                                    virt_to_phys(pDeviceData->addmemvadd) >> PAGE_SHIFT,
                                    vsize,
                                    vma->vm_page_prot);
          if(result)
          {
            printk("ECI: remap_pfn_range failed, additional memory mmap aborted\n");
            return -EAGAIN;
          }
          else
          {
            #ifdef __DEBUG__
            printk( "ECI: Add Mem  mmap successful, from 0x%lx(actually:0x%lx) to 0x%lx-0x%lx vsize %ld\n",
                    (unsigned long)virt_to_phys(pDeviceData->addmemvadd),
                    (unsigned long)(virt_to_phys(pDeviceData->addmemvadd) >> PAGE_SHIFT) << PAGE_SHIFT,
                    vma->vm_start,
                    vma->vm_start+vsize,
                    vsize );
            #endif
          }
        }
        else
        {
          printk("ECI: No DMA available, DMA mmap aborted\n");
          return -EADDRNOTAVAIL;
        }

        break;
      }

      case OSLNXKM_DMA_MAP_OFFSET + 0:
      { //map DMA 0
        if(0 != pDeviceData->dmaadd)
        {
          #ifndef CONFIG_X86
          vma->vm_page_prot    = pgprot_noncached(vma->vm_page_prot);
          #endif
          result = remap_pfn_range (vma,
                                    vma->vm_start,
                                    pDeviceData->dmaadd >> PAGE_SHIFT,
                                    vsize,
                                    vma->vm_page_prot);
          if(result)
          {
            printk("ECI: remap_pfn_range failed, DMA mmap aborted\n");
            return -EAGAIN;
          }
          else
          {
            #ifdef __DEBUG__
            printk( "ECI: DMA      mmap successful, from 0x%lx(actually:0x%lx) to 0x%lx-0x%lx vsize %ld\n",
                    pDeviceData->dmaadd,
                    (pDeviceData->dmaadd >> PAGE_SHIFT) << PAGE_SHIFT,
                    vma->vm_start,
                    vma->vm_start+vsize,
                    vsize );
            #endif
          }
        }
        else
        {
          printk("ECI: No DMA available, DMA mmap aborted\n");
          return -EADDRNOTAVAIL;
        }

        break;
      }

      default:
      {
        return -EIO;
        break;
      }
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the irq remove function for one ISA device.

@param pDeviceData
ISA device to
*/
void isa_free_irq(struct device_data_t* pDeviceData)
{
  // release ISA irq if assigned
  if(pDeviceData->device_irq != 0)
  {
    free_irq(pDeviceData->device_irq, pDeviceData);
    pDeviceData->device_irq = 0;
  }
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the request irq function for one ISA driver.

@param pDeviceData
ISA device to assign irq to

@param irq
ISA irq to request

@return
0 on success
*/
int isa_request_irq(struct device_data_t* pDeviceData, unsigned int irq)
{
  int result = 0;

  //*** Ensure function is called for an ISA device
  if((IXXAT_VENDOR_ID       != pDeviceData->s_deviceid.wVendorId   ) ||
     (IXXAT_SUB_VENDOR_ID   != pDeviceData->s_deviceid.wSubVendorId) ||
     (INVALID_SUB_DEVICE_ID != pDeviceData->s_deviceid.wSubSystemId) )
  {
      printk(KERN_ALERT "ECI: Request_irq failed assigning irq %d => Device unsupported\n", irq);
      return -EBADF;
  }

  //*** Ensure no old irq is assigned
  isa_free_irq(pDeviceData);

  // Register IRQ handler, if given
  pDeviceData->board_ready = 0;
  if(irq != 0)
  {
    result = request_irq(irq, irq_handler, IRQF_SHARED, pDeviceData->name, pDeviceData);
    if(result)
    {
      printk(KERN_ALERT "ECI: Request_irq failed assigning irq %d\n", irq);
    }
    else
    {
      printk("ECI: Successfully registered interrupt handler for irq %d\n", irq);
      pDeviceData->device_irq = irq;
    }
  }

  return result;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the add function for our isa driver. It will be called by the kernel
when the driver is loaded.

@param memadd
address of ISA memory

@param memlen
length of ISA memory

@param irq
interrupt line if available, otherwise 0

@param devicetype
type of device

@return
0 on success
*/
int isa_add(unsigned long memadd, unsigned long memlen, unsigned int irq, int devicetype)
{
  int                   result      = 1;
  struct device_data_t* pDeviceData = kzalloc( sizeof(struct device_data_t), GFP_KERNEL );

  #ifdef __DEBUG__
    printk("ECI: %s isa_dev: io=0x%lx, len=0x%lx, irq=%u\n", __func__, memadd, memlen, irq);
  #endif //__DEBUG__

  init_waitqueue_head(&pDeviceData->waitqueue);

  reservecardnumber(pDeviceData);

  printk("ECI: Add ISA device:\n");

  // Store device and vendor IDs
  pDeviceData->s_deviceid.wVendorId     = IXXAT_VENDOR_ID;
  pDeviceData->s_deviceid.wDeviceId     = devicetype;
  pDeviceData->s_deviceid.wSubVendorId  = IXXAT_SUB_VENDOR_ID;
  pDeviceData->s_deviceid.wSubSystemId  = INVALID_SUB_DEVICE_ID;

  pDeviceData->devicetype = devicetype;

  // Set-up interrupt helpers
  pDeviceData->IntGetStat = IntGetStat_ISA;
  pDeviceData->IntEnaReq  = IntEnaReq_ISA;
  pDeviceData->IntClrReq  = IntClrReq_ISA;

  printk("ECI: Card seems to be of type: (0x%04X) %s\n", pDeviceData->devicetype, gethwname(pDeviceData->devicetype));

  // Set physical ISA memory address
  pDeviceData->memadd = memadd;
  pDeviceData->memlen = memlen;

  if(request_mem_region(pDeviceData->memadd, pDeviceData->memlen,"ECI ISA Memory"))
  {
    printk(KERN_ALERT "ECI: ISA address 0x%lx already in use\n", pDeviceData->memadd);
    goto isa_check_mem_region_failed;
  }

  // Map ISA memory to virtual kernel address
  pDeviceData->memvadd = ioremap_nocache( pDeviceData->memadd, pDeviceData->memlen );

  // Build unique eci device file name
  snprintf( pDeviceData->name , sizeof(pDeviceData->name), "eci%d", pDeviceData->cardnumber);

  // Register IRQ handler, if given
  isa_request_irq(pDeviceData, irq);

  printk("ECI: Assigned card number %d (so device file should be /dev/%s)\n",pDeviceData->cardnumber, pDeviceData->name);

  #ifdef __DEBUG__
    printk(  "ECI:   pDeviceData->memadd:     0x%lx\n", pDeviceData->memadd );
    printk(  "ECI:   pDeviceData->memlen:     0x%lx\n", pDeviceData->memlen );
    printk(  "ECI:   pDeviceData->memvadd:    0x%lx\n", (unsigned long) pDeviceData->memvadd );
  #endif

  // Create char device file
  pDeviceData->devnum = MKDEV( MAJOR(major), pDeviceData->cardnumber);
  cdev_init(&(pDeviceData->cdev), &fops);
  pDeviceData->cdev.owner = THIS_MODULE;
  result = cdev_add(&(pDeviceData->cdev), pDeviceData->devnum, 1);
  if(result)
  {
    printk(KERN_ALERT "ECI: Failed registering char device\n" );
    goto isa_cdev_add_failed;
  }
  else
  {
    printk("ECI: Successfully registered char device\n" );
  }

  // create sysfs device file
  // kernel versions prior 2.6.27 have to use device_create_drvdata() here
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2, 27, 0)
  pDeviceData->dev = device_create(eci_class, NULL, pDeviceData->devnum, "%s", pDeviceData->name);
  #else
  pDeviceData->dev = device_create(eci_class, NULL, pDeviceData->devnum, NULL, "%s", pDeviceData->name);
  #endif
  
  if(!pDeviceData->dev)
  {
    printk(KERN_ALERT "ECI: Error creating sysfs file\n" );
    goto isa_device_create_failed;
  }
  return 0;

isa_device_create_failed:
  cdev_del(&(pDeviceData->cdev));

isa_cdev_add_failed:
  isa_free_irq(pDeviceData);
  iounmap(pDeviceData->memvadd);
  release_mem_region(pDeviceData->memadd,pDeviceData->memlen);

isa_check_mem_region_failed:
  release_mem_region(pDeviceData->memadd, pDeviceData->memlen);
  freecardnumber(pDeviceData);
  kfree(pDeviceData);

  return -ENODEV;
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the remove function for one ISA driver. It will be called by the kernel
when the driver is removed from the system.

@param pDeviceData
ISA device to remove
*/
void isa_remove(struct device_data_t* pDeviceData)
{
  freecardnumber(pDeviceData);

  #ifdef __DEBUG__
    printk("ECI: %s isa_dev: io=0x%lx\n", __func__, pDeviceData->memadd);
  #endif //__DEBUG__

  // destroy sysfs device
  device_destroy(eci_class,pDeviceData->devnum);

  cdev_del(&(pDeviceData->cdev));

  // release ISA irq
  isa_free_irq(pDeviceData);

  iounmap(pDeviceData->memvadd);
  release_mem_region(pDeviceData->memadd,pDeviceData->memlen);

  kfree(pDeviceData);
}


///////////////////////////////////////////////////////////////////////////////
/**
This is the remove function for our ISA driver. It will be called by the kernel
when the driver is removed from the system.

@param memadd
address of ISA memory
*/
void isa_remove_all(void)
{
  int i = 0;

  // find device by ISA address (only in ISA address space)
  for(i = 0; i < ECI_MAXBOARDCOUNT; i++)
  {
    if(device_list[i] != NULL)
    {
      if((device_list[i]->memadd >= ECI_ISA_ADDRESS_START) &&
         (device_list[i]->memadd <= ECI_ISA_ADDRESS_END  ))
      {
        isa_remove( device_list[i]);
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
/**
Function to scan for IXXAT ISA interfaces. Following interfaces are supported:
PC-I04/104 and IPC-I320/104

*/
int isa_scan_iomem(void)
{
  unsigned long addr  = ECI_ISA_ADDRESS_START;
  unsigned long len   = ECI_ISA_ADDRESS_LENGTH;
  unsigned long step  = ECI_ISA_ADDRESS_STEP;

  for(; addr <= ECI_ISA_ADDRESS_END; addr += step)
  {
    if(request_mem_region(addr, len,"ECI ISA Memory"))
    {
      #ifdef __DEBUG__
        { printk(KERN_ALERT "ECI: ISA address 0x%lx already in use\n", addr); }
      #endif //__DEBUG__
    }
    else
    {
      unsigned char*      pByte     = NULL;
      unsigned short int  wDeviceId = 0;

      pByte = ioremap_nocache( addr, len );

      #ifdef __DEBUG__
      {
        int i = 0;
        printk("ECI: ISA address 0x%lx memdump", addr);
        for(; i< 64; i++)
        {
          if(0 == (i%32))
            { printk("\n"); }
          printk("%02X ", pByte[i]);
        }
        printk("\n");
      }
      #endif //__DEBUG__

      //*** Check for SJA1000 CAN Controller (reset state)
      if( (0x0C == pByte[SJA_ADDR_SR] ) && (0xE0 == pByte[SJA_ADDR_ISR]) )
      {
        int j = 0;

        //*** Check for PC-I04/104
        wDeviceId = ECI_DEVICE_I04_104;
        for(j=0;j<8;j++)
        {
          //*** CAN controller is mirrored 8 times every 32 byte
          if((0x0C != pByte[SJA_ADDR_SR  + j*0x20] ) ||
             (0xE0 != pByte[SJA_ADDR_ISR + j*0x20]))
          {
            //*** Reset device ID if ctrl not mirrored
            wDeviceId = 0;
          }
        }
        if(0 != wDeviceId)
          { printk("Found PC-I04/104 (SJA1000 CAN Controller) at ISA address 0x%lX\n", addr); }
      }
      //*** Check for iPC-I320/104 (reset state)
      else if( IPC_I320_104_IDENTIFIER == *((unsigned int *)pByte) )
      {
        //*** Check Identifier and Vendor String (reset state)
        if((0 == memcmp( &pByte[IPC_I320_104_IDENTIFIER_OFFSET],
                         IPC_I320_104_IDENTIFIER_STRING,
                         strlen(IPC_I320_104_IDENTIFIER_STRING))) &&
           (0 == memcmp( &pByte[IPC_I320_104_VENDOR_OFFSET],
                         IPC_I320_104_VENDOR_STRING,
                         strlen(IPC_I320_104_VENDOR_STRING)))   )
        {
          wDeviceId = ECI_DEVICE_I320_104;
          printk("Found iPC-I320/104 at ISA address 0x%lX\n", addr);
        }
      }

      //*** Must add found device?
      if(0 != wDeviceId)
      {
        isa_add(addr, len, 0, wDeviceId);
        wDeviceId = 0;
      }
    }

    //*** Release memory region before adding a found device
    release_mem_region(addr, len);
  }

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function is called whenever the /proc/eci device file is read. All it does is print
some useful information.
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
int eci_read_info(char * buf, char **start, off_t offset, int count, int *eof, void *data)
#else
ssize_t eci_read_info(struct file *file, char __user *buf, size_t size, loff_t *ppos)
#endif
{
  int   len              =  0;
  int   i                =  0;
  char  *read_info       = NULL;
  size_t buffer_size      = 0;

  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
  static int finished = 0;

  if(finished)
  {
    finished = 0;
    return 0;
  }
  buffer_size = size;
  #else
  buffer_size = count;
  #endif

  read_info = kzalloc(buffer_size, GFP_KERNEL);

  len += snprintf(read_info, buffer_size, "ECI boards:\n");

  for(i = 0; i < ECI_MAXBOARDCOUNT; i++)
  {
    if(device_list[i] != NULL)
    {
      len += snprintf( read_info + len, buffer_size - len,
                      " Hardware type: %s, device: /dev/%s, major: %d, minor: %d, I/O memory 0x%08lx, irq %u\n",
                      gethwname(device_list[i]->devicetype),
                      device_list[i]->name,
                      MAJOR(device_list[i]->devnum),
                      MINOR(device_list[i]->devnum),
                      device_list[i]->memadd,
                      device_list[i]->device_irq);
    }
  }

  copy_to_user(buf, read_info, len);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
  *eof = 1;
#else
  finished = 1;
#endif

  kfree(read_info);

  return len;
}


///////////////////////////////////////////////////////////////////////////////
/**
This function is called on module load.
*/
static int __init eci_km_init(void)
{
  alloc_chrdev_region(&major, 0, ECI_MAXBOARDCOUNT, ECI_DRIVERNAME);

  eci_class = class_create(THIS_MODULE, ECI_CLASSNAME);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
  eci_proc_entry = create_proc_read_entry( ECI_DRIVERNAME,
                                           S_IFREG | S_IRUSR | S_IRGRP |S_IROTH,
                                           NULL,
                                           eci_read_info,
                                           NULL );
#else
   eci_proc_entry = proc_create( ECI_DRIVERNAME,
                                 S_IFREG | S_IRUSR | S_IRGRP |S_IROTH,
                                 NULL,
                                 &proc_file_fops);
#endif
  printk("ECI: Initialized\n");

  // ISA/104 driver support
  #ifdef ECI_ENABLE_ISA_BUS_SCAN
    // scan ISA address range for known IXXAT ISA/104 interfaces and add them
    // ATTENTION: Scanning the ISA bus may have negative effects to other ISA hardware
    isa_scan_iomem();
   #else
    // Add some ISA/104 interfaces manually
    ECI_ADD_ISA_DEVICES
  #endif

  return pci_register_driver(&pci_driver);
}

///////////////////////////////////////////////////////////////////////////////
/**
This function is called on module  unload.
*/
static void __exit eci_km_exit(void)
{
  pci_unregister_driver(&pci_driver);

  isa_remove_all();

  class_unregister(eci_class);
  class_destroy(eci_class);

  remove_proc_entry(ECI_DRIVERNAME, NULL);

  unregister_chrdev_region(major, ECI_MAXBOARDCOUNT);

  printk("ECI: Exited\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module used by the ECI API for hardware access");
MODULE_VERSION(PRODUCT_VERSION_PRODUCTVERSION_ASTEXT_DOT);

module_init(eci_km_init);
module_exit(eci_km_exit);
