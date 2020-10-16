#  Definition of types, structs and unions for LIN initialization and 
#  communication.

# Module Imports
#
from ctypes import *  # @UnusedWildImport

from OsEci import *  # @UnusedWildImport

#//////////////////////////////////////////////////////////////////////////
#// constants and macros

#****************************************************************************
#* LIN baud rates
#****************************************************************************
ECI_LIN_BITRATE_UNDEF      = int(0xFFFF) # < Undefined bit-rate @ingroup LinTypes
ECI_LIN_BITRATE_MIN        = int(1000)   # < Lowest specified bit-rate @ingroup LinTypes
ECI_LIN_BITRATE_MAX        = int(20000)  # < Highest specified bit-rate @ingroup LinTypes

#  LIN bitrate definitions
ECI_LIN_BITRATE_AUTO  = int(0)           # <  Automatic bit-rate detection
ECI_LIN_BITRATE_1000  = int(1000)        # <  1000 baud
ECI_LIN_BITRATE_1200  = int(1200)        # <  1200 baud
ECI_LIN_BITRATE_2400  = int(2400)        # <  2400 baud
ECI_LIN_BITRATE_4800  = int(4800)        # <  4800 baud
ECI_LIN_BITRATE_9600  = int(9600)        # <  9600 baud
ECI_LIN_BITRATE_10400 = int(10400)       # < 10400 baud
ECI_LIN_BITRATE_19200 = int(19200)       # < 19200 baud
ECI_LIN_BITRATE_20000 = int(20000)       # < 20000 baud
# e_LINBITRATES;


# Maximum possible 6bit LIN ID  @ingroup LinTypes */
ECI_LIN_MAX_6BIT_ID = int(0x3F)

#  LIN controller types
ECI_LIN_CTRL_UNKNOWN = int(0x00)         # < unknown
ECI_LIN_CTRL_GENERIC = int(0x01)         # < Generic LIN Controller
ECI_LIN_CTRL_USB_V2  = int(0x02)         # < USB-to-CAN V2 LIN Controller
ECI_LIN_CTRL_MAXVAL  = int(0xFF)         # < Maximum value for controller type
#} e_LINCTRLCLASS;


#  LIN controller supported features.
ECI_LIN_FEATURE_UNDEFINED = int(0x0000)   # < undefined
ECI_LIN_FEATURE_MASTER    = int(0x0001)   # < Master mode
ECI_LIN_FEATURE_AUTORATE  = int(0x0002)   # < Automatic bitrate detection
ECI_LIN_FEATURE_ERRFRAME  = int(0x0004)   # < Reception of error frames
ECI_LIN_FEATURE_BUSLOAD   = int(0x0008)   # < Bus load measurement
# e_LINCTRLFEATURE;


#  LIN controller bus coupling types.
ECI_LIN_BUSC_UNDEFINED = int(0x0000)     # < undefined
ECI_LIN_BUSC_STANDARD  = int(0x0001)     # < Standard
# e_LINBUSC ;

#  LIN controller operating modes.
ECI_LIN_OPMODE_SLAVE         = int(0x00) # < Enable slave mode
ECI_LIN_OPMODE_MASTER        = int(0x01) # < Enable master mode
ECI_LIN_OPMODE_ERRFRAME      = int(0x02) # < Enable reception of error frames
ECI_LIN_OPMODE_CACHED_STATUS = int(0x04) # < The status will be cached within the driver
                                         # < which leads to very fast ECIDRV_CtrlGetStatus
                                         # < calls. However you may need to set a status
                                         # < update rate with ECIDRV_CtrlSetStatusUpdateRate.
# e_LINOPMODE;


#  LIN message types (used by <LINMSGINFO.Bytes.bType>).
ECI_LIN_MSGTYPE_DATA    = int(0)         # < Data frame
ECI_LIN_MSGTYPE_INFO    = int(1)         # < Info frame
ECI_LIN_MSGTYPE_ERROR   = int(2)         # < Error frame
ECI_LIN_MSGTYPE_STATUS  = int(3)         # < Status frame
ECI_LIN_MSGTYPE_WAKEUP  = int(4)         # < Wakeup frame
ECI_LIN_MSGTYPE_TIMEOVR = int(5)         # < Timer overrun
ECI_LIN_MSGTYPE_TIMERST = int(6)         # < Timer reset
# e_LINMSGTYPE;


#  LIN message information flags (used by <LINMSGINFO.Bytes.bFlags>).
ECI_LIN_MSGFLAGS_DLC = int(0x0F)         # < Data length code
ECI_LIN_MSGFLAGS_OVR = int(0x10)         # < Data overrun flag (RX direction only).
ECI_LIN_MSGFLAGS_SOR = int(0x20)         # < Sender of response. @copydetails ECI_LINMSGINFO::sor
ECI_LIN_MSGFLAGS_ECS = int(0x40)         # < Enhanced checksum (according to LIN Spec. 2.0)
ECI_LIN_MSGFLAGS_IDO = int(0x80)         # < ID only. @copydetails ECI_LINMSGINFO::ido
# e_LINMSGFLAGS;


#  LIN message information flags2 (used by <LINMSGINFO.Bytes.bFlags2>).
ECI_LIN_MSGFLAGS2_BUF = int(0x01)        # < Update LIN slave response buffer only
                                            # < (TX direction only).
# e_LINMSGFLAGS2;


#  Information supplied in the abData[0] field of info frames
#  (LINMSGINFO.Bytes.bType = LIN_MSGTYPE_INFO).
ECI_LIN_INFO_START = int(1)              # < Start of LIN controller
ECI_LIN_INFO_STOP  = int(2)              # < Stop of LIN controller
ECI_LIN_INFO_RESET = int(3)              # < Reset of LIN controller
# e_LININFO;


#  Error information supplied in the abData[0] field of error frames
#  (LINMSGINFO.Bytes.bType = LIN_MSGTYPE_ERROR).
ECI_LIN_ERROR_BIT    = int(1)            # < Bit error
ECI_LIN_ERROR_CHKSUM = int(2)            # < Checksum error
ECI_LIN_ERROR_PARITY = int(3)            # < Identifier parity error
ECI_LIN_ERROR_SLNORE = int(4)            # < Slave not responding error
ECI_LIN_ERROR_SYNC   = int(5)            # < Inconsistent sync field error
ECI_LIN_ERROR_NOBUS  = int(6)            # < No bus activity error
ECI_LIN_ERROR_OTHER  = int(7)            # < Other (unspecified) error
# e_LINERROR;


#  Status information supplied in the abData[0] field of status frames
#  (LINMSGINFO.Bytes.bType = ECI_LIN_MSGTYPE_STATUS) and in
#  ECI_LINSTATUS::u::V0::dwStatus.
ECI_LIN_STATUS_OVRRUN = int(0x01)        # < Data overrun occurred
ECI_LIN_STATUS_ININIT = int(0x10)        # < Init mode active
# e_LINSTATUS;


#//////////////////////////////////////////////////////////////////////////
#// data types


#  LIN controller configuration.
class ECI_LININITLINE(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("bOpMode", BYTE),   # < LIN operating mode @see e_LINOPMODE
                ("bReserved", BYTE), # < Reserved set to 0
                ("wBitrate", WORD),  # < LIN bitrate @see e_LINBITRATES
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S0)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U),             # < Version controlled structs container
        ]

#  LIN controller capabilities.
class ECI_LINCAPABILITIES(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S(Structure):
            _pack_ = 1
            _fields_ = [
                ("wLinType", WORD),   # < Type of LIN controller @see e_LINCTRLCLASS
                ("wBusCoupling", WORD), # < Type of Bus coupling @see e_LINBUSC
                ("dwFeatures", DWORD), # < Supported features @see e_LINCTRLFEATURE
                ("dwClockFreq", DWORD), # < Clock frequency of the primary counter in Hz
                ("dwTscDivisor", DWORD),  # < Divisor for the message time stamp counter
                ("dwDtxDivisor", DWORD),  # < Divisor for the delayed message transmitter
                ("dwDtxMaxTicks", DWORD),  # < Maximum tick count value of the delayed message transmitter
                ("dwNoOfPrioQueues", DWORD),  # < Number of priority TX queues
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)             # < Version controlled structs container
        ]
  
#  LIN controller status.
class ECI_LINSTATUS(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S(Structure):
            _pack_ = 1
            _fields_ = [
                ("bOpMode", BYTE),   # < Current LIN operating mode @see e_LINOPMODE
                ("bBusLoad", BYTE), # < Average bus load in percent (0..100)
                ("wBitrate", WORD), # < Current LIN bitrate @see e_LINBITRATES
                ("dwStatus", DWORD), # < Status of the LIN controller @see e_LINSTATUS
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)             # < Version controlled structs container
        ]
    
#  LIN message information.
class ECI_LINMSGINFO(Union):
    _pack_ = 1
    class _Sbytes(Structure):
        _pack_ = 1
        _fields_ = [
            ("bType", BYTE),   # < Message type @see e_LINMSGTYPE
            ("bFlags2", BYTE), # < Flags @see e_LINMSGFLAGS2
            ("bFlags", BYTE), # < Flags @see e_LINMSGFLAGS
            ("bReserved2", BYTE), # < Reserved for future use
            ]
    
    class _Sbits(Structure):
        _pack_ = 1
        _fields_ = [
            ("type", DWORD, 8),   # < Message type @see e_LINMSGTYPE
            ("buf", DWORD, 1), # < Update LIN slave response buffer only (TX direction only).
            ("res", DWORD, 7), # < Reserved for future use
            ("dlc", DWORD, 4), # < Data length code
            ("ovr", DWORD, 1), # < Possible data overrun (RX direction only).
            ("sor", DWORD, 1), # < Sender of response. Set to send slave buffer data upon reception if LIN ID in 
                            # < slave mode. To disable a slave buffer send a slave message without setting this flag.
            ("ecs", DWORD, 1), # < Enhanced checksum (according to LIN Spec. 2.0)
            ("ido", DWORD, 1), # < ID only. Set to send a LIN frame without data in master mode. Upon reception 
                            # < this flag indicates that no slave responded to this LIN ID.
            ("res2", DWORD, 8), # < Reserved for future use
            ]
    _anonymous_ = ("Bytes", "Bits")
    
    _fields_ = [ 
        ("Bytes", _Sbytes),       # < LIN Message information in byte format
        ("Bits", _Sbits)         # < LIN Message information in bit format
        ]

#  LIN message information.
class ECI_LINMESSAGE(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S(Structure):
            _pack_ = 1
            _fields_ = [
                ("dwTime", DWORD),  # < Time stamp for receive message in [us]
                ("dwMsgId", DWORD), # < LIN message identifier (INTEL format)
                ("uMsgInfo", ECI_LINMSGINFO), # < LIN message information (bit field)
                ("abData", BYTE*8), # < Message data
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)             # < Version controlled structs container
        ]

#  LIN command structure.
class ECI_LIN_CMD_REQ_HD(Structure):
    _pack_ = 1
    _fields_ = [ 
        ("wCode", WORD),       # < Command request code
        ]             
  
#  LIN command structure.
class ECI_LIN_CMD_RES_HD(Structure):
    _pack_ = 1
    _fields_ = [ 
        ("wResult", WORD),       # < Command result code
        ]             
  
#  LIN command structure.
class ECI_LINCMDREQUEST(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S(Structure):
            _pack_ = 1
            _fields_ = [
                ("sCmdHeader", ECI_LIN_CMD_REQ_HD),  # < LIN command header @see ECI_LIN_CMD_REQ_HD
                ("dwReserved", DWORD), # < reserved for future use
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)              # < Version controlled structs container
        ]
  
#  LIN Command structure.
class ECI_LINCMDRESPONSE(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S(Structure):
            _pack_ = 1
            _fields_ = [
                ("sCmdHeader", ECI_LIN_CMD_RES_HD),  # < LIN command header @see ECI_LIN_CMD_RES_HD
                ("dwReserved", DWORD), # < reserved for future use
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)             # < Version controlled structs container
        ]
  
#  LIN filter structure.
class ECI_LINFILTER(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S(Structure):
            _pack_ = 1
            _fields_ = [ 
                ("dwReserved", DWORD), # < reserved for future use
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)              # < Version controlled structs container
        ]
