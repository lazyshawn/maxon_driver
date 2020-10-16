IXXAT Automation GmbH - ECI Linux Driver
=========================================


About ECI Driver
----------------
The ECI driver allows you to use IXXAT CAN/LIN Interfaces within your own 
applications. Therefore the ECI provides you an easy to use API including demos 
how to use it.


System requirements
-------------------
* PC with Linux Kernel Version from 2.6.x up to 4.4.0
* IXXAT CAN Interface: 
  - "PC-I 04/PCI"
  - "PC-I 04/104"
  - "iPC-I 320/PCI"
  - "iPC-I 320/ISA"
  - "iPC-I 320/104"
  - "iPC-I XC16/PCI"
  - "iPC-I XC16/PMC"
  - "iPC-I XC16/PCIe"
  - "USB-to-CAN compact"
  - "USB-to-CAN II"
  - "USB-to-CAN V2 compact"
  - "USB-to-CAN V2 embedded"
  - "USB-to-CAN V2 professional"
  - "USB-to-CAN V2 automotive"
  - "USB-to-CAN FD compact"
  - "USB-to-CAN FD embedded"
  - "USB-to-CAN FD automotive"
  - "USB-to-CAN FD PCIe Mini"
  - "USB-to-CAR"
  - "CAN-IB100/PCIe"
  - "CAN-IB120/PCIe Mini"
  - "CAN-IB130/PCIe 104"
  - "CAN-IB200/PCIe"
  - "CAN-IB210/XMC"
  - "CAN-IB230/PCIe 104"
  - "CAN-IB300/PCI"
  - "CAN-IB400/PCI"
  - "CAN-IB410/PMC"
  - "CAN-IB500/PCIe FD"
  - "CAN-IB520/PCIe Mini FD"
  - "CAN-IB530/PCIe 104"
  - "CAN-IB600/PCIe FD"
  - "CAN-IB610/XMC FD"
  - "CAN-IB630/PCIe 104"
  - "CAN-IB810/PMC FD"
  
* IXXAT ECI Linux Driver
* Gcc 4.2 and a working kernel build environment (for PCI(e) interfaces)
* USB interfaces require the latest version (V2.X) of the bootmanager
* libusb-0.1 0.1.12 or newer (for USB interfaces)
* libusb-1.0 1.0.12-2 or newer (for USB-to-CAN V2)
* libc6 2.3.4 or newer
* libstdc++6 3.4 or newer
* libgcc1 3.0 or newer
* Code::Blocks IDE 8.02 or newer (optional)

Note: 
  The driver should work with most Linux distributions running a Kernel from 2.6.x up to 4.4.0, 
  but was only tested using Debian/Ubuntu and openSUSE Linux.

  
Getting started (for USB Interfaces)
------------------------------------
1) If you plan to only use USB-devices, go to
   'src/KernelModule' and run 'make install-usb',
   preferably as root. This skips the building
   and installing of the kernel module, which is
   only needed for pci interfaces.
2) Compile the demo application ( see "Compiling ECI Demo" )
3) Now you should be able to run the demo application

Getting started (for PCI Interfaces)
------------------------------------
1) Install one or more IXXAT CAN Interface boards
2) Compile and install the ECI kernel module ( see "Compiling the Kernel Module" )
3) Compile the demo application ( see "Compiling ECI Demo" )
4) Now you should be able to run the demo application

Getting started (for ISA/104 Interfaces)
------------------------------------
1) Check the PC for free ISA address spaces and configure the IXXAT CAN Interfaces accordingly.
2) Install one ore more IXXAT CAN Interface boards.
3) Either enable ISA/104 bus scanning or add the installed ISA/104 devices manually to the ECI
   kernel module. Therefor open the ecikm.c and enable either the define "ECI_ENABLE_ISA_BUS_SCAN" or
   add one or more devices to the define "ECI_ADD_ISA_DEVICES".
4) Compile and install the ECI kernel module 
   ( see \ref InstallationCompilingKernelModule "Compiling the Kernel Module" ).
5) Compile the demo application 
   ( see \ref InstallationCompilingECIDemo "Compiling ECI Demo" ).
6) Now you should be able to run the demo application.


Compiling the Kernel Module
---------------------------
Before you can use the ECI you have to compile and install the ECI kernel module.
For this you should have the source code of your operating kernel installed on your
system and the kernel configuration file '/usr/src/linux/.config' should exist.
The "linux-headers" package matching your kernel version can also be sufficient
to build the ECI kernel module.

An easy way to get a working kernel build environment is to use module-assistant
(m-a). You can install it by typing 'sudo apt-get install module-assistant'. Now you
can use 'sudo module-assistant prepare' to let module-assistant install the necessary
packages for you.

1) To build the kernel module just run 'make' in the kernel module source directory
   'src/KernelModule'
2) To install the kernel module run 'make install'.
   This will copy the module to the '/lib/modules/$(KERNEL_VERSION)/kernel/drivers/misc/'
   directory. Also it will copy the accompanied udev rules file to '/etc/udev/rules.d'.
   You can edit this file if you want the device nodes located in the '/dev' directory
   to have different permissions. Afterwards the makefile will call "depmod -a" to export
   the modalias file and "modprobe" to load the module. The makefile will also copy the
   libraries to '/usr/lib', set up all necessary links and run "ldconfig". Depending
   on your distribution you may need to adapt the 'TARGET_PATH' in the Makefile. 
   For instance on an 64-bit openSUSE system the default library path is '/usr/lib64'.
3) You can use the command 'cat /proc/eci' to check which interfaces were found and which 
   I/O addresses and interrupts were occupied.


Using ECI Demo
--------------
1) Extract the files from the ECI driver package to your Linux PC
2) Navigate to 'EciLinux_[ARCH]/bin/release' (where [ARCH] can be
   i386 or amd64) and start the demo by
   typing "./LinuxEciDemo" in your console
3) The demo tries to open all supported boards, if the demo succeeds to open 
   one ore more boards it shows an output similare to it below for each board:
    >> ECI Demo for iPC-I XC16 / PCI <<
    CapsVer  : "0"
    CanType  : "0x0006"
    BusCoupl : "0x0003"
    Feature  : "0x02ff"
    ClockFreq: "40000000"
    TscDiv   : "32"
    DtxDiv   : "4000"
    DtxMaxTic: "65535"
    PrioQueue: "1"
    Now, sending 20480 CAN Messages
    Now, receiving CAN Messages for 30 seconds
    ............................................................
    -> Returning from ECI Demo for iPC-I XC16 / PCI <-
    -> Closing Linux ECI API Demo program <-
4) To test the CAN functionality you have to connect the IXXAT can interfaces
   to a terminated CAN bus with a CAN counterpart with a speed of 1 Mbit
5) The demo tries to send 20480 standard CAN Messages with ascending CAN 
   identifiers and varying length and data
6) When the demo shows the hint "Now, receiving CAN Messages for 30 seconds" you
   can send CAN messages to the demo application. This messages than will be 
   displayed
7) If the opened interface supports LIN, this controller is opened after the CAN 
   controller and some LIN messages are sent and received at a LIN Baudrate of 
   19200 BAUD


Compiling ECI Demo
------------------
If you do not have Code::Blocks IDE installed you can compile the demo
using the accompanied Makefile:
1) Go to 'EciLinux_[ARCH]/src/EciDemos' 
2) run 'make'

If you want to use Code::Blocks IDE:
1) Navigate to 'EciLinux_[ARCH]/src/EciDemos/LinuxEciDemo' and open LinuxEciDemo.cbp
   using Code::Blocks IDE
2) Compile and start the demo within your IDE


ECI API
-------
1) The ECI Header files can be found in the folder EciLinux_[ARCH]/inc
2) Every IXXAT Interface has its own header file e.g.:
   ECI101.h -> "iPC-I 320 / PCI"
   ECI105.h -> "iPC-I XC16 / PCI"
   These header files map the common name "ECIDRV_*" to the device dependent 
   name "ECI101_*" for instance. For a complete list please refer to the HTML
   documentation.
3) The ECI provides following functions:
    ECIDRV_Initialize
    ECIDRV_Release
    ECIDRV_GetInfo
    ECIDRV_CtrlOpen
    ECIDRV_CtrlClose
    ECIDRV_CtrlStart
    ECIDRV_CtrlStop
    ECIDRV_CtrlGetCapabilities
    ECIDRV_CtrlGetStatus
    ECIDRV_CtrlSetStatusUpdateRate
    ECIDRV_CtrlSend
    ECIDRV_CtrlReceive
    ECIDRV_CtrlSetAccFilter
    ECIDRV_CtrlAddFilterIds
    ECIDRV_CtrlRemFilterIds
    ECIDRV_CtrlCommand
    ECIDRV_LogConfig
    ECIDRV_LogRead
    ECIDRV_LogStart
    ECIDRV_LogStop
    ECIDRV_LogClear
    ECIDRV_GetErrorString
   A description of the function can be found in the header file ECI.h
4) The ECI structures are defined in the files named ECI_*.h

Following functionality is currently not implemented / available:
  ECIDRV_CtrlCommand
