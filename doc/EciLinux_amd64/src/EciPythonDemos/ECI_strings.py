#  Utility function to convert ECI defines to human readable string.

# Module Imports
#
from ctypes import *  # @UnusedWildImport
from ECI_hwtype import *  # @UnusedWildImport


from OsEci import *  # @UnusedWildImport

#//////////////////////////////////////////////////////////////////////////
#// functions


"""
Function to retrieve a human readable string from a e_HWCLASS define.

@param dwHwClass
  Numeric value of e_HWCLASS to convert to a human readable string.

@retval Sting
  Human readable string.

@ingroup EciStrings
"""
def ECI_StrFromHwClass(dwHwClass):
    # Switch on define
    if dwHwClass == ECI_HW_UNDEFINED:
        pszString = "undefined"
    elif dwHwClass == ECI_HW_PCI:
        pszString = "PCI"
    elif dwHwClass == ECI_HW_ISA:
        pszString = "ISA"
    elif dwHwClass == ECI_HW_USB:
        pszString = "USB"
    elif dwHwClass == ECI_HW_IP:
        pszString = "Ethernet"
    else:
        pszString = "n/a"
    
    return pszString

"""
Function to retrieve a human readable string from a e_CTRLCLASS define.

@param dwCtrlClass
  Numeric value of e_CTRLCLASS to convert to a human readable string.

@retval Sting
  Human readable string.

@ingroup EciStrings
"""
def ECI_StrFromCtrlClass(dwCtrlClass):
    # Switch on define
    if dwCtrlClass == ECI_CTRL_UNDEFINED:
        pszString = "undefined"
    elif dwCtrlClass == ECI_CTRL_CAN:
        pszString = "CAN"
    elif dwCtrlClass == ECI_CTRL_LIN:
        pszString = "LIN"
    elif dwCtrlClass == ECI_CTRL_FLX:
        pszString = "FlexRay"
    elif dwCtrlClass == ECI_CTRL_KLI:
        pszString = "K-Line"
    else:
        pszString = "n/a"
    
    return pszString

"""
Function to retrieve a human readable string from a e_CANCTRLCLASS define.

@param dwCanCtrlClass
  Numeric value of e_CANCTRLCLASS to convert to a human readable string.

@retval Sting
  Human readable string.

@ingroup EciStrings
"""
def ECI_StrFromCanCtrlClass(dwCanCtrlClass):
    # Switch on define
    if dwCanCtrlClass == ECI_CAN_CTRL_UNKNOWN:
        pszString = "undefined"
    elif dwCanCtrlClass == ECI_CAN_CTRL_82527:
        pszString = "Intel 82527"
    elif dwCanCtrlClass == ECI_CAN_CTRL_82C200:
        pszString = "Intel 82C200"
    elif dwCanCtrlClass == ECI_CAN_CTRL_81C90:
        pszString = "Intel 81C90"
    elif dwCanCtrlClass == ECI_CAN_CTRL_81C92:
        pszString = "Intel 81C92"
    elif dwCanCtrlClass == ECI_CAN_CTRL_SJA1000:
        pszString = "Philips SJA 1000"
    elif dwCanCtrlClass == ECI_CAN_CTRL_82C900:
        pszString = "Infineon 82C900 (TwinCAN)"
    elif dwCanCtrlClass == ECI_CAN_CTRL_TOUCAN:
        pszString = "Motorola TOUCAN"
    elif dwCanCtrlClass == ECI_CAN_CTRL_MSCAN:
        pszString = "Freescale Star12 MSCAN"
    elif dwCanCtrlClass == ECI_CAN_CTRL_FLEXCAN:
        pszString = "Freescale Coldfire FLEXCAN"
    elif dwCanCtrlClass == ECI_CAN_CTRL_IFI:
        pszString = "IFI CAN (ALTERA FPGA CAN)"
    elif dwCanCtrlClass == ECI_CAN_CTRL_CCAN:
        pszString = "CCAN (Bosch C_CAN)"
    elif dwCanCtrlClass == ECI_CAN_CTRL_BXCAN:
        pszString = "BXCAN (ST BX_CAN)"
    elif dwCanCtrlClass == ECI_CAN_CTRL_IFIFD:
        pszString = "IFI CAN FD (ALTERA FPGA CAN FD)"
    else:
        pszString = "n/a"
    
    return pszString


"""
Function to retrieve a human readable string from a e_CANBUSC define.

@param dwCanBusCoupling
  Numeric value of e_CANBUSC to convert to a human readable string.

@retval Sting
  Human readable string.

@ingroup EciStrings
"""
def ECI_StrFromCanBusCoupling(dwCanBusCoupling):
    # Switch on define
    if dwCanBusCoupling == ECI_CAN_BUSC_UNDEFINED:
        pszString = "undefined"
    elif dwCanBusCoupling == ECI_CAN_BUSC_LOWSPEED:
        pszString = "Low Speed (ISO/IS 11898-3)"
    elif dwCanBusCoupling == ECI_CAN_BUSC_HIGHSPEED:
        pszString = "High Speed (ISO/IS 11898-2)"
    elif dwCanBusCoupling == ECI_CAN_BUSC_HIGHSPEED | ECI_CAN_BUSC_LOWSPEED:
        pszString = "High or Low Speed (ISO/IS 11898-2 or 11898-3)"
    else:
        pszString = "n/a"
    return pszString


"""
Function to retrieve a human readable string from a e_LINCTRLCLASS define.

@param dwLinCtrlClass
  Numeric value of e_LINCTRLCLASS to convert to a human readable string.

@retval Sting
  Human readable string.

@ingroup EciStrings
"""
def ECI_StrFromLinCtrlClass(dwLinCtrlClass):
    # Switch on define
    if dwLinCtrlClass == ECI_LIN_CTRL_UNKNOWN:
        pszString = "undefined"
    elif dwLinCtrlClass == ECI_LIN_CTRL_GENERIC:
        pszString = "Generic"
    else:
        pszString = "n/a"
    return pszString
    

"""
Function to retrieve a human readable string from a e_LINBUSC define.

@param dwLinBusCoupling
  Numeric value of e_LINBUSC to convert to a human readable string.

@retval Sting
  Human readable string.

@ingroup EciStrings
"""
def ECI_StrFromLinBusCoupling(dwLinBusCoupling):
    # Switch on define
    if dwLinBusCoupling == ECI_LIN_BUSC_UNDEFINED:
        pszString = "undefined"
    if dwLinBusCoupling == ECI_LIN_BUSC_STANDARD:
        pszString = "Standard"
    else:
        pszString = "n/a"
    
    return pszString
