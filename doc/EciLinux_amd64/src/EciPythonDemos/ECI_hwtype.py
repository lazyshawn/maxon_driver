#  Definition of types, structs and unions for hardware initialization and 
#  communication.
#

# Module Imports
#
from ctypes import *  # @UnusedWildImport
from ECI_cantype import *  # @UnusedWildImport
from ECI_lintype import *  # @UnusedWildImport
from ECI_thread import *  # @UnusedWildImport

#//////////////////////////////////////////////////////////////////////////
#// constants and macros
#//////////////////////////////////////////////////////////////////////////

ECI_NO_WAIT          = DWORD(0)   # Timeout definition for polling mode.
ECI_WAIT_FOREVER     = DWORD(0xFFFFFFFF)      # Timeout definition for blocking mode.
ECI_INVALID_HANDLE   = DWORD(0)   # Definition of an invalid handle.
ECI_MAXBOARDCOUNT    = DWORD(8)   # Maximum supported boards by the ECI API.
ECI_MAXCTRLCOUNT     = DWORD(8)     # Maximum supported controllers per board. @ingroup CtrlTypes

#  ECI controller handle 
ECI_CTRL_HDL = DWORD 

#  ECI structure versions
ECI_STRUCT_VERSION_V0 = int(0)         # < Version 0
ECI_STRUCT_VERSION_V1 = int(1)         # < Version 1
ECI_STRUCT_VERSION_V2 = int(2)         # < Version 2
ECI_STRUCT_VERSION_V3 = int(3)         # < Version 3
# e_ECI_STRUCT_VERSION

#  Library states
ECI_UNINITIALIZED    = int(0x00)     # < Library is uninitialized
ECI_INITIALIZED      = int(0x01)     # < Library is initialized
ECI_CONFIGURED       = int(0x02)     # < Library in the configured state
# e_LIBSTATES;

#  Controller states
ECI_CTRL_UNCONFIGURED = int(0x00)    # < Controller is unconfigured
ECI_CTRL_INITIALIZED  = int(0x01)    # < Controller is initialized
ECI_CTRL_RUNNING      = int(0x02)    # < Controller is running
# e_CTRLSTATES;

#  Hardware classes
ECI_HW_UNDEFINED  = int(0x00)        # < undefined
ECI_HW_PCI        = int(0x01)        # < PCI hardware like iPCI-165/PCI
ECI_HW_ISA        = int(0x02)        # < ISA hardware like iPCI-320/ISA
ECI_HW_USB        = int(0x03)        # < USB hardware like USB-to-CAN compact
ECI_HW_IP         = int(0x04)        # < IP hardware like CAN@@net II
# e_HWCLASS;

#  Controller classes
ECI_CTRL_UNDEFINED = int(0x00)       # < undefined
ECI_CTRL_CAN       = int(0x01)       # < CAN controller
ECI_CTRL_LIN       = int(0x02)       # < LIN controller
ECI_CTRL_FLX       = int(0x03)       # < FlexRay controller
ECI_CTRL_KLI       = int(0x04)       # < K-Line controller
# e_CTRLCLASS;

#  Controller settings flags
"""No flags used"""
ECI_SETTINGS_FLAG_NONE            = int(0x00000000) 

"""If set the hardware is initialized in polling mode, that means no interrupt 
is assigned and the interrupt line is not used. To use the recommended 
interrupt driven mode set this flag resp. bit to zero.
Please also refer to \ref api_InterruptVsPolling.
"""
ECI_SETTINGS_FLAG_POLLING_MODE    = int(0x00000001)

"""If set the the board is opened without verifying the ECI firmware after 
downloading it. This switch can be used if start time of the board shall
by reduced.
"""
ECI_SETTINGS_FLAG_NO_FW_VERIFY    = int(0x40000000)

"""If set the the board is opened without downloading the ECI firmware. This
flag is only used by maintenance programs and must not be set by any ECI
application
"""
ECI_SETTINGS_FLAG_NO_FW_DOWNLOAD  = int(0x80000000)
# e_SETTINGS_FLAGS;



#  Controller stop flags
"""No flags used"""
ECI_STOP_FLAG_NONE            = int(0x00000000) 

"""If set an ongoing transmission is aborted and the controller configuration is reset. (RESET) @n
If not set the controller is stopped after an ongoing transmission, the configuration is kept (STOP).
"""
ECI_STOP_FLAG_RESET_CTRL      = int(0x00000001) 
                                        
"""If set all entries from TX FIFO are cleared @n
If not set all entries are kept in TX FIFO.
"""
ECI_STOP_FLAG_CLEAR_TX_FIFO   = int(0x00000002) 

"""If set all entries from RX FIFO are cleared. @n
If not set all entries are kept in RX FIFO. @n
@note 
Please note if RX FIFO clearing is not enabled while re-starting the controller. The timestamps
are also not reset upon re-start of the controller.
"""
ECI_STOP_FLAG_CLEAR_RX_FIFO   = int(0x00000004)
# e_STOP_FLAGS;

#  Definition of the PCI hardware configuration parameters.
#  Board information.
class ECI_PCI_SETTINGS(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("dwReserved", DWORD),                 # < Reserved for future use
                ]

        class _S1(Structure):
            _pack_ = 1
            _fields_ = [
                ("sSchedSettings", ECI_SCHEDULER_SETTINGS), # < Scheduler settings
                ]
    
        #_anonymous_ = ("V0","V1")
        _fields_ = [("V0", _S0),                      # < Version 0
		            ("V1", _S1)]                      # < Version 1
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),                         # < Version of valid union struct
        ("u", _U)                                # < Version controlled structs container
        ]

#  Definition of the ISA hardware configuration parameters.
class ECI_ISA_SETTINGS(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("dwAddress",  DWORD),               # < Address of the ISA hardware
                ("bInterrupt", BYTE),                # < Interrupt of the ISA hardware
                ]

        class _S1(Structure):
            _pack_ = 1
            _fields_ = [
                ("dwAddress",  DWORD),                      # < Address of the ISA hardware
                ("bInterrupt", BYTE),                       # < Interrupt of the ISA hardware
                ("sSchedSettings", ECI_SCHEDULER_SETTINGS), # < Scheduler settings
                ]
            
        #_anonymous_ = ("V0","V1")
        _fields_ = [("V0", _S0),                      # < Version 0
		            ("V1", _S1)]                      # < Version 1
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),                         # < Version of valid union struct
        ("u", _U),                                # < Version controlled structs container
        ]

#  Definition of the USB hardware configuration parameters.
class ECI_USB_SETTINGS(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("dwReserved", DWORD),                 # < Reserved for future use
                ]

        class _S1(Structure):
            _pack_ = 1
            _fields_ = [
                ("sRxSchedSettings", ECI_SCHEDULER_SETTINGS), # < Scheduler settings for rx threads
                ("sTxSchedSettings", ECI_SCHEDULER_SETTINGS), # < Scheduler settings for tx threads
                ]

        #_anonymous_ = ("V0","V1")
        _fields_ = [("V0", _S0),                      # < Version 0
		            ("V1", _S1)]                      # < Version 1
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),                         # < Version of valid union struct
        ("u", _U)                                # < Version controlled structs container
        ]


#  Definition of the IP hardware configuration parameters.
class ECI_IP_SETTINGS(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [ 
                ("dwProtocol", DWORD),                      # < Type of Protocol to use
                ("szIpAddress", CHAR * 0x100),              # < IP Address, either IPv4, IPv6 oder DNS Name
                ("szPassword", CHAR * 0x100),               # < Readable or encrypted password
                ("adwPort", DWORD * 16),                    # < Array of Port Numbers
                ]

        class _S1(Structure):
            _pack_ = 1
            _fields_ = [ 
                ("dwProtocol", DWORD),                      # < Type of Protocol to use
                ("szIpAddress", CHAR * 0x100),              # < IP Address, either IPv4, IPv6 oder DNS Name
                ("szPassword", CHAR * 0x100),               # < Readable or encrypted password
                ("adwPort", DWORD * 16),                    # < Array of Port Numbers
                ("sSchedSettings", ECI_SCHEDULER_SETTINGS), # < Scheduler settings
                ]
    
        #_anonymous_ = ("V0","V1")
        _fields_ = [("V0", _S0),                      # < Version 0
		            ("V1", _S1)]                      # < Version 1
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),                         # < Version of valid union struct
        ("u", _U)                                # < Version controlled structs container
        ]

#  Definition of the hardware configuration parameters.

class ECI_HW_PARA(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        _fields_ = [
            ("sPciSettings", ECI_PCI_SETTINGS), # < PCI hardware settings @see ECI_PCI_SETTINGS
            ("sIsaSettings", ECI_ISA_SETTINGS), # < ISA hardware settings @see ECI_ISA_SETTINGS
            ("sUsbSettings", ECI_USB_SETTINGS), # < USB hardware settings @see ECI_USB_SETTINGS 
            ("sIpSettings",  ECI_IP_SETTINGS),  # < IP hardware settings @see ECI_IP_SETTINGS 
            ]

    _anonymous_ = ("u",)
    _fields_ = [
        ("wHardwareClass", WORD),               # < Hardware class @see e_HWCLASS
        ("dwFlags", DWORD),                     # < Flag field for special purpose @see e_SETTINGS_FLAGS
        ("u", _U),                              # < Hardware configuration container
        ]

#  Controller information.
class ECI_CTRL_INFO(Structure):
    _pack_ = 1
    _fields_ = [
        ("wCtrlClass", WORD),                   # < Controller class @see e_CTRLCLASS
        ("wCtrlState", WORD),                   # < Current controller state @see e_CTRLSTATES
        ]


#  Controller information.

#  Board information.
class ECI_HW_INFO(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("szHwBoardType", CHAR * 32),       # < Hardware board type string
                ("szHwSerial", CHAR * 16),          # < Hardware board serial number string
                ("abHwVersion", BYTE * 4),          # < Hardware version (Branch, Major, Minor, Build)
                ("abBmVersion", BYTE * 4),          # < Bootmanager version (Branch, Major, Minor, Build)
                ("szFwIdentification", CHAR * 64),  # < Firmware identification string
                ("abFwVersion", BYTE * 4),          # < Firmware version (Branch, Major, Minor, Build)
                ("adwApiVersion", DWORD * 4),       # < ECI API version (Branch, Major, Minor, Build)
                ("dwCtrlCount", DWORD),             # < Number of supported controllers
                ("sCtrlInfo", ECI_CTRL_INFO * ECI_MAXCTRLCOUNT.value),  # < Controller info
                ]

        class _S1(Structure):
            _pack_ = 1
            _fields_ = [
                ("szHwBoardType", CHAR * 32),       # < Hardware board type string
                ("szHwSerial", CHAR * 16),          # < Hardware board serial number string
                ("abHwVersion", BYTE * 4),          # < Hardware version (Branch, Major, Minor, Build)
                ("abBmVersion", BYTE * 4),          # < Bootmanager version (Branch, Major, Minor, Build)
                ("szFwIdentification", CHAR * 64),  # < Firmware identification string
                ("abFwVersion", BYTE * 4),          # < Firmware version (Branch, Major, Minor, Build)
                ("adwApiVersion", DWORD * 4),       # < ECI API version (Branch, Major, Minor, Build)
                ("dwCtrlCount", DWORD),             # < Number of supported controllers
                ("sCtrlInfo", ECI_CTRL_INFO * ECI_MAXCTRLCOUNT.value),  # < Controller info
                ("dwDevId", DWORD),                 # < Device ID
                ]            
    
        #_anonymous_ = ("V0","V1")
        _fields_ = [("V0", _S0),                      # < Version 0
                    ("V1", _S1)]                      # < Version 1
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),                         # < Version of valid union struct
        ("u", _U),                             # < Version controlled structs container
        ]


#  Controller capabilities.
class ECI_CTRL_CAPABILITIES(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        _fields_ = [
            ("sCanCaps", ECI_CANCAPABILITIES),  # < CAN controller capability structure @see ECI_CANCAPABILITIES
            ("sLinCaps", ECI_LINCAPABILITIES),  # < LIN controller capability structure @see ECI_LINCAPABILITIES
            ("sCanConfig", ECI_CANCAPABILITIES),# < Obsolete use ECI_CTRL_CAPABILITIES::sCanCaps instead
            ("sLinConfig", ECI_LINCAPABILITIES),# < Obsolete use ECI_CTRL_CAPABILITIES::sLinCaps instead
            ]
    
    _anonymous_ = ("u",)                
    _fields_ = [ 
        ("wCtrlClass", WORD),                   # < Controller class @see e_CTRLCLASS
        ("u", _U),                             # < Controller capability container
        ] 


#  Current controller status.
class ECI_CTRL_STATUS(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        _fields_ = [
            ("sCanStatus", ECI_CANSTATUS),       # < CAN controller capability structure @see ECI_CANCAPABILITIES
            ("sLinStatus", ECI_LINSTATUS),       # < LIN controller capability structure @see ECI_LINCAPABILITIES
            ]
    
    _anonymous_ = ("u",)                
    _fields_ = [ 
        ("wCtrlClass", WORD),                   # < Controller class @see e_CTRLCLASS
        ("u", _U),                              # < Controller status container
        ] 
  

#  Controller configuration settings.
class ECI_CTRL_CONFIG(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        _fields_ = [
            ("sCanConfig", ECI_CANINITLINE),       # < CAN controller configuration structure @see ECI_CANINITLINE
            ("sLinConfig", ECI_LININITLINE),       # < LIN controller configuration structure @see ECI_LININITLINE
            ]
    
    _anonymous_ = ("u",)                
    _fields_ = [ 
        ("wCtrlClass", WORD),                   # < Controller class @see e_CTRLCLASS
        ("u", _U),                             # < Controller configuration container
        ] 
  

#  Controller message definitions
class ECI_CTRL_MESSAGE(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        _fields_ = [
            ("sCanMessage", ECI_CANMESSAGE),    # < CAN controller message structure @see ECI_CANMESSAGE
            ("sLinMessage", ECI_LINMESSAGE),    # < LIN controller message structure @see ECI_LINMESSAGE
            ]
    
    _anonymous_ = ("u",)                
    _fields_ = [ 
        ("wCtrlClass", WORD),                   # < Controller class @see e_CTRLCLASS
        ("u", _U),                              # < Controller message container
        ]

#  Controller command request definitions.
class ECI_CTRL_CMDREQUEST(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        _fields_ = [
            ("sCanCmdRequest", ECI_CANCMDREQUEST), # < CAN controller command request structure @see ECI_CANCMDREQUEST
            ("sLinCmdRequest", ECI_LINCMDREQUEST), # < LIN controller command request structure @see ECI_LINCMDREQUEST
            ]
    
    _anonymous_ = ("u",)                
    _fields_ = [ 
        ("wCtrlClass", WORD),                   # < Controller class @see e_CTRLCLASS
        ("u", _U),                             # < Controller command request container
        ]

#  Controller command response definitions.
class ECI_CTRL_CMDRESPONSE(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        _fields_ = [
            ("sCanCmdResponse", ECI_CANCMDRESPONSE), # < CAN controller command response structure @see ECI_CANCMDRESPONSE
            ("sLinCmdResponse", ECI_LINCMDRESPONSE),  # < LIN controller command response structure @see ECI_LINCMDRESPONSE
            ]
    
    _anonymous_ = ("u",)                
    _fields_ = [ 
        ("wCtrlClass", WORD),                   # < Controller class @see e_CTRLCLASS
        ("u", _U),                             # < Controller command response container
        ]
  

#  Structure for filter definition.
class ECI_CTRL_FILTER(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        _fields_ = [
            ("sCanFilter", ECI_CANFILTER),        # < CAN controller message filter structure @see ECI_CANFILTER
            ("sLinFilter", ECI_LINFILTER),       # < LIN controller message filter structure @see ECI_LINFILTER
            ]
    
    _anonymous_ = ("u",)                
    _fields_ = [ 
        ("wCtrlClass", WORD),                   # < Controller class @see e_CTRLCLASS
        ("wPadding"  , WORD),                   # < padding
        ("u", _U),                              # < Controller message filter container
        ]
