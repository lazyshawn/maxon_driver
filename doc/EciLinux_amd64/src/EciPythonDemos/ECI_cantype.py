#  Definition of types, structs and unions for CAN initialization and 
#  communication.

# Module Imports
#
from ctypes import *  # @UnusedWildImport

from OsEci import *  # @UnusedWildImport

#//////////////////////////////////////////////////////////////////////////
#// data types
#/**
#  CAN controller bit timing parameter
#
#  @code
#  Computation of nominal bit time Tbit
#
#  |<------------ Tbit ------------>|
#  +------+------------+------------+
#  | SYNC | Time Seg 1 | Time Seg 2 |
#  +------+------------+------------+
#                      |
#                      +-> Sample Point

#  Fs    := Frequency of the CAN system clock see also: ECI_CANCAPABILITIES
#  Ts    := duration of a CAN system clock tick
#  Tq    := duration of a time quanta
#  Fbit  := bit rate in bits per second
#  Tbit  := duration of a bit
#  Tsync := duration of Sync Segment (always 1)
#  Tseg1 := duration of Time Segment 1
#  Tseg2 := duration of Time Segment 2
#  Qbit  := number of time quanta for one bit
#  Qsync := number of time quanta for Sync Segment (always 1)
#  Qseg1 := number of time quanta for Time Segment 1
#  Qseg2 := number of time quanta for Time Segment 2
#  BRP   := baud rate pre-scaler

#  Ts = 1 / Fs
#  Tq = Ts * BRP = BRP / Fs
#  Tq = Tbit / Qbit

#  Tbit = 1 / Fbit

#  Qbit = Qsync + Qseg1 + Qseg2

#  Tsync = Tq * 1
#  Tseg1 = Tq * Qseg1
#  Tseg2 = Tq * Qseg2

#  Tbit = Tsync + Tseg1 + Tseg2
#  Tbit = Tq * (1 + Qseg1 + Qseg2)
#  Tbit = Ts * BRP * (1 + Qseg1 + Qseg2)
class ECI_CANBTP(Structure):
    _pack_ = 1
    _fields_ = [ 
        ("dwMode", DWORD),      # < Timing mode  @see e_CANBTMODE constants
        ("dwBPS",  DWORD),      # < Bits per second or prescaler @see e_CANBTMODE.
                                # < In native mode this parameter contains the controller specific
                                # < value for the baud rate pre-scaler.
        ("wTS1",   WORD),       # < Duration of time segment 1 in quantas
        ("wTS2",   WORD),       # < Duration of time segment 2 in quantas
        ("wSJW",   WORD),       # < Re-synchronisation jump width in quantas
        ("wTDO",   WORD),       # < Transceiver delay compensation offset in quantas
                                # < (0 = disabled). Used to specify the sample point
                                # < during bit monitoring in fast data bit rate.
        ]
        
        


#//////////////////////////////////////////////////////////////////////////
#// constants and macros
##***************************************************************************
#* CAN baud rates (for CAN controller with 16Mhz)
#****************************************************************************/
ECI_CAN_BT0_10KB           = int(0x31)   # <  BT0   10 KB @ingroup CanTypes
ECI_CAN_BT1_10KB           = int(0x1C)   # <  BT1   10 KB @ingroup CanTypes
ECI_CAN_BT0_20KB           = int(0x18)   # <  BT0   20 KB @ingroup CanTypes
ECI_CAN_BT1_20KB           = int(0x1C)   # <  BT1   20 KB @ingroup CanTypes
ECI_CAN_BT0_50KB           = int(0x09)   # <  BT0   50 KB @ingroup CanTypes
ECI_CAN_BT1_50KB           = int(0x1C)   # <  BT1   50 KB @ingroup CanTypes
ECI_CAN_BT0_100KB          = int(0x04)   # <  BT0  100 KB @ingroup CanTypes
ECI_CAN_BT1_100KB          = int(0x1C)   # <  BT1  100 KB @ingroup CanTypes
ECI_CAN_BT0_125KB          = int(0x03)   # <  BT0  125 KB @ingroup CanTypes
ECI_CAN_BT1_125KB          = int(0x1C)   # <  BT1  125 KB @ingroup CanTypes
ECI_CAN_BT0_250KB          = int(0x01)   # <  BT0  250 KB @ingroup CanTypes
ECI_CAN_BT1_250KB          = int(0x1C)   # <  BT1  250 KB @ingroup CanTypes
ECI_CAN_BT0_500KB          = int(0x00)   # <  BT0  500 KB @ingroup CanTypes
ECI_CAN_BT1_500KB          = int(0x1C)   # <  BT1  500 KB @ingroup CanTypes
ECI_CAN_BT0_800KB          = int(0x00)   # <  BT0  800 KB @ingroup CanTypes
ECI_CAN_BT1_800KB          = int(0x16)   # <  BT1  800 KB @ingroup CanTypes
ECI_CAN_BT0_1000KB         = int(0x00)   # <  BT0 1000 KB @ingroup CanTypes
ECI_CAN_BT1_1000KB         = int(0x14)   # <  BT1 1000 KB @ingroup CanTypes

"""
TODO
ECI_CAN_BT01_10KB          = int(0x31,0x1C)   # <   10 KB @ingroup CanTypes
ECI_CAN_BT01_20KB          = int(0x18,0x1C)   # <   20 KB @ingroup CanTypes
ECI_CAN_BT01_50KB          = int(0x09,0x1C)   # <   50 KB @ingroup CanTypes
ECI_CAN_BT01_100KB         = int(0x04,0x1C)   # <  100 KB @ingroup CanTypes
ECI_CAN_BT01_125KB         = int(0x03,0x1C)   # <  125 KB @ingroup CanTypes
ECI_CAN_BT01_250KB         = int(0x01,0x1C)   # <  250 KB @ingroup CanTypes
ECI_CAN_BT01_500KB         = int(0x00,0x1C)   # <  500 KB @ingroup CanTypes
ECI_CAN_BT01_800KB         = int(0x00,0x16)   # <  800 KB @ingroup CanTypes
ECI_CAN_BT01_1000KB        = int(0x00,0x14)   # < 1000 KB @ingroup CanTypes
"""

#/*****************************************************************************
#* CAN baud rates
#****************************************************************************/
ECI_CAN_BTP_10KB   = ECI_CANBTP(dwMode = 0, dwBPS = 10000,   wTS1 = 13, wTS2 = 2, wSJW = 1, wTDO = 0)   # <     10 KB, SP 87,5% @ingroup CanTypes
ECI_CAN_BTP_20KB   = ECI_CANBTP(dwMode = 0, dwBPS = 20000,   wTS1 = 13, wTS2 = 2, wSJW = 1, wTDO = 0)   # <     20 KB, SP 87,5% @ingroup CanTypes
ECI_CAN_BTP_50KB   = ECI_CANBTP(dwMode = 0, dwBPS = 50000,   wTS1 = 13, wTS2 = 2, wSJW = 1, wTDO = 0)   # <     50 KB, SP 87,5% @ingroup CanTypes
ECI_CAN_BTP_100KB  = ECI_CANBTP(dwMode = 0, dwBPS = 100000,  wTS1 = 13, wTS2 = 2, wSJW = 1, wTDO = 0)   # <    100 KB, SP 87,5% @ingroup CanTypes
ECI_CAN_BTP_125KB  = ECI_CANBTP(dwMode = 0, dwBPS = 125000,  wTS1 = 13, wTS2 = 2, wSJW = 1, wTDO = 0)   # <    125 KB, SP 87,5% @ingroup CanTypes
ECI_CAN_BTP_250KB  = ECI_CANBTP(dwMode = 0, dwBPS = 250000,  wTS1 = 13, wTS2 = 2, wSJW = 1, wTDO = 0)   # <    250 KB, SP 87,5% @ingroup CanTypes
ECI_CAN_BTP_500KB  = ECI_CANBTP(dwMode = 0, dwBPS = 500000,  wTS1 = 13, wTS2 = 2, wSJW = 1, wTDO = 0)   # <    500 KB, SP 87,5% @ingroup CanTypes
ECI_CAN_BTP_800KB  = ECI_CANBTP(dwMode = 0, dwBPS = 800000,  wTS1 =  7, wTS2 = 2, wSJW = 1, wTDO = 0)   # <    800 KB, SP 80,0% @ingroup CanTypes
ECI_CAN_BTP_1000KB = ECI_CANBTP(dwMode = 0, dwBPS = 1000000, wTS1 =  5, wTS2 = 2, wSJW = 1, wTDO = 0)   # <   1000 KB, SP 75,0% @ingroup CanTypes


#/*****************************************************************************
#* CAN FD baud rates, used for arbitration and fast data rate
#****************************************************************************/
ECI_CAN_SDR_BTP_500KB   = ECI_CANBTP(dwMode = 0, dwBPS = 500000,   wTS1 = 63, wTS2 = 16, wSJW = 16, wTDO = 0)   # <     500 KB, SP 75,0% @ingroup CanTypes
ECI_CAN_SDR_BTP_1000KB  = ECI_CANBTP(dwMode = 0, dwBPS = 1000000,  wTS1 = 31, wTS2 =  8, wSJW =  8, wTDO = 0)   # <    1000 KB, SP 75,0% @ingroup CanTypes

ECI_CAN_FDR_BTP_2000KB  = ECI_CANBTP(dwMode = 0, dwBPS =  2000000,  wTS1 = 15, wTS2 = 4, wSJW = 4, wTDO = 32)   # <    2000 KB, SP 75,0% @ingroup CanTypes
ECI_CAN_FDR_BTP_4000KB  = ECI_CANBTP(dwMode = 0, dwBPS =  4000000,  wTS1 =  7, wTS2 = 2, wSJW = 2, wTDO = 16)   # <    4000 KB, SP 75,0% @ingroup CanTypes
ECI_CAN_FDR_BTP_5000KB  = ECI_CANBTP(dwMode = 0, dwBPS =  5000000,  wTS1 =  5, wTS2 = 2, wSJW = 2, wTDO = 12)   # <    5000 KB, SP 75,0% @ingroup CanTypes
ECI_CAN_FDR_BTP_6666KB  = ECI_CANBTP(dwMode = 0, dwBPS =  6666666,  wTS1 =  3, wTS2 = 2, wSJW = 2, wTDO =  8)   # <    6666 KB, SP 75,0% @ingroup CanTypes
ECI_CAN_FDR_BTP_8000KB  = ECI_CANBTP(dwMode = 0, dwBPS =  8000000,  wTS1 =  3, wTS2 = 1, wSJW = 1, wTDO =  8)   # <    8000 KB, SP 75,0% @ingroup CanTypes
ECI_CAN_FDR_BTP_10000KB = ECI_CANBTP(dwMode = 0, dwBPS = 10000000,  wTS1 =  2, wTS2 = 1, wSJW = 1, wTDO =  6)   # <   10000 KB, SP 75,0% @ingroup CanTypes


# Maximum possible 11bit CAN ID @ingroup CanTypes */
ECI_CAN_MAX_11BIT_ID = int(0x7FF)

# Maximum possible 29bit CAN ID @ingroup CanTypes */
ECI_CAN_MAX_29BIT_ID = int(0x1FFFFFFF)

# CAN controller types
ECI_CAN_CTRL_UNKNOWN = int(0x00)         # < unknown
ECI_CAN_CTRL_82527   = int(0x01)         # < Intel 82527
ECI_CAN_CTRL_82C200  = int(0x02)         # < Intel 82C200
ECI_CAN_CTRL_81C90   = int(0x03)         # < Intel 81C90
ECI_CAN_CTRL_81C92   = int(0x04)         # < Intel 81C92
ECI_CAN_CTRL_SJA1000 = int(0x05)         # < Philips SJA 1000
ECI_CAN_CTRL_82C900  = int(0x06)         # < Infineon 82C900 (TwinCAN)
ECI_CAN_CTRL_TOUCAN  = int(0x07)         # < Motorola TOUCAN
ECI_CAN_CTRL_MSCAN   = int(0x08)         # < Freescale Star12 MSCAN
ECI_CAN_CTRL_FLEXCAN = int(0x09)         # < Freescale Coldfire FLEXCAN
ECI_CAN_CTRL_IFI     = int(0x0A)         # < IFI CAN ( ALTERA FPGA CAN )
ECI_CAN_CTRL_CCAN    = int(0x0B)         # < CCAN (Bosch C_CAN)
ECI_CAN_CTRL_BXCAN   = int(0x0C)         # < BXCAN (ST BX_CAN)
ECI_CAN_CTRL_IFIFD   = int(0x0D)         # < IFI CAN FD (ALTERA FPGA CAN FD)
ECI_CAN_CTRL_MAXVAL  = int(0xFF)         # < Maximum value for controller type
# e_CANCTRLCLASS;


# CAN controller supported features.
ECI_CAN_FEATURE_UNDEFINED    = int(0x00000)  # < undefined
ECI_CAN_FEATURE_STDOREXT     = int(0x00001)  # < 11 OR 29 bit (exclusive)
ECI_CAN_FEATURE_STDANDEXT    = int(0x00002)  # < 11 AND 29 bit (simultaneous)
ECI_CAN_FEATURE_RMTFRAME     = int(0x00004)  # < Reception of remote frames
ECI_CAN_FEATURE_ERRFRAME     = int(0x00008)  # < Reception of error frames
ECI_CAN_FEATURE_BUSLOAD      = int(0x00010)  # < Bus load measurement
ECI_CAN_FEATURE_IDFILTER     = int(0x00020)  # < Exact message filter
ECI_CAN_FEATURE_LISTONLY     = int(0x00040)  # < Listen only mode
ECI_CAN_FEATURE_SCHEDULER    = int(0x00080)  # < Cyclic message scheduler
ECI_CAN_FEATURE_GENERRFRM    = int(0x00100)  # < Error frame generation
ECI_CAN_FEATURE_DELAYEDTX    = int(0x00200)  # < Delayed message transmission
ECI_CAN_FEATURE_SSM          = int(0x00400)  # < Single shot mode
ECI_CAN_FEATURE_HI_PRIO      = int(0x00800)  # < High priority message
ECI_CAN_FEATURE_EXTDATA      = int(0x01000)  # < Extended data length respectively
                                            # < improved frame format (CAN FD - EDL/IFF)
ECI_CAN_FEATURE_FASTDATA     = int(0x02000)  # < Fast data rate (CAN FD - BRS)
ECI_CAN_FEATURE_STT          = int(0x04000)  # < single transmission try messages with acknowledge error
ECI_CAN_FEATURE_ISOFD        = int(0x08000) # < ISO CAN FD
ECI_CAN_FEATURE_NONISOFD     = int(0x10000) # < non-ISO CAN FD
# e_CANCTRLFEATURE;


# CAN controller bus coupling types.
ECI_CAN_BUSC_UNDEFINED = int(0x0000)     # < undefined
ECI_CAN_BUSC_LOWSPEED  = int(0x0001)     # < Low speed coupling
ECI_CAN_BUSC_HIGHSPEED = int(0x0002)     # < High speed coupling
# e_CANBUSC ;


# CAN controller operating modes.
ECI_CAN_OPMODE_UNDEFINED     = int(0x00)     # < undefined
ECI_CAN_OPMODE_STANDARD      = int(0x01)     # < Reception of 11-bit id messages
ECI_CAN_OPMODE_EXTENDED      = int(0x02)     # < Reception of 29-bit id messages
ECI_CAN_OPMODE_ERRFRAME      = int(0x04)     # < Enable reception of error frames
ECI_CAN_OPMODE_LISTONLY      = int(0x08)     # < Listen only mode (TX passive)
ECI_CAN_OPMODE_LOWSPEED      = int(0x10)     # < Use low speed bus interface
ECI_CAN_OPMODE_STT           = int(0x20)     # < @deprecated enable single transmission try messages with acknowledge error
ECI_CAN_OPMODE_CACHED_STATUS = int(0x40)     #< The status will be cached within the driver
                                             #< which leads to very fast ECIDRV_CtrlGetStatus
                                             #< calls. However you may need to set a status
                                             #< update rate with ECIDRV_CtrlSetStatusUpdateRate.
                                             
# e_CANOPMODE;

# CAN controller extended operating modes.
# Bit coded information, resulting value can be any combination of values below.
ECI_CAN_EXMODE_DISABLED  = int(0x00)     # < No extended operation
ECI_CAN_EXMODE_EXTDATA   = int(0x01)     # < Enable extended data length respectively
                                         # < improved frame format (CAN FD - EDL/IFF)
ECI_CAN_EXMODE_FASTDATA  = int(0x02)     # < Enable fast data bit rate (CAN FD - BRS)
ECI_CAN_EXMODE_ISOFD     = int(0x04)     # < Enable ISO CAN FD mode otherwise non-ISO CAN FD is enabled

# CAN controller bit-timing modes modes.
# Bit coded information, resulting value can be any combination of values below.
ECI_CAN_BTMODE_UNDEFINED = int(0x00000000)     # < undefined
ECI_CAN_BTMODE_NATIVE    = int(0x00000001)     # < Enable native mode setting.
ECI_CAN_BTMODE_TRISAMPLE = int(0x00000002)     # < Enable triple sampling mode.


# CAN message types (used by <CANMSGINFO.Bytes.bType>)
ECI_CAN_MSGTYPE_DATA    = int(0)         # < Data frame
ECI_CAN_MSGTYPE_INFO    = int(1)         # < Info frame
ECI_CAN_MSGTYPE_ERROR   = int(2)         # < Error frame
ECI_CAN_MSGTYPE_STATUS  = int(3)         # < Status frame
ECI_CAN_MSGTYPE_WAKEUP  = int(4)         # < Wakeup frame
ECI_CAN_MSGTYPE_TIMEOVR = int(5)         # < Timer overrun
ECI_CAN_MSGTYPE_TIMERST = int(6)          # < Timer reset
# e_CANMSGTYPE;


# CAN message information flags (used by <CANMSGINFO.Bytes.bFlags>).
ECI_CAN_MSGFLAGS_DLC = int(0x0F)         # < Data length code
ECI_CAN_MSGFLAGS_OVR = int(0x10)         # < Data overrun flag
ECI_CAN_MSGFLAGS_SRR = int(0x20)         # < Self reception request
ECI_CAN_MSGFLAGS_RTR = int(0x40)         # < Remote transmission request
ECI_CAN_MSGFLAGS_EXT = int(0x80)          # < Frame format (0=11-bit, 1=29-bit)
# e_CANMSGFLAGS;


# CAN message information flags2 (used by <CANMSGINFO.Bytes.bFlags2>).
ECI_CAN_MSGFLAGS_SSM = int(0x01)         # < Single shot message. @copydetails ECI_CANMSGINFO::ssm
ECI_CAN_MSGFLAGS_HPM = int(0x06)         # < High priority message channel 0-3. @copydetails ECI_CANMSGINFO::hpm
ECI_CAN_MSGFLAGS_EDL = int(0x08)         # < Extended data length respectively improved frame format (CAN FD - EDL/IFF).
                                         # < @copydetails ECI_CANMSGINFO::edl
ECI_CAN_MSGFLAGS_BRS = int(0x10)         # < Fast data rate, bit rate switch (CAN FD - BRS). @copydetails ECI_CANMSGINFO::brs
ECI_CAN_MSGFLAGS_ESI = int(0x20)         # < Error state indicator (CAN FD - ESI). @copydetails ECI_CANMSGINFO::esi
# e_CANMSGFLAGS2;


# Information supplied in the abData[0] field of info frames
#  (CANMSGINFO.Bytes.bType = CAN_MSGTYPE_INFO).
ECI_CAN_INFO_START = int(1)              # < Start of CAN controller
ECI_CAN_INFO_STOP  = int(2)              # < Stop of CAN controller
ECI_CAN_INFO_RESET = int(3)               # < Reset of CAN controller
# e_CANINFO;


# Error information supplied in the abData[0] field of error frames
#  (CANMSGINFO.Bytes.bType = CAN_MSGTYPE_ERROR).
ECI_CAN_ERROR_UNDEFINED = int(0)             # < Unknown or no error
ECI_CAN_ERROR_STUFF     = int(1)             # < Stuff error
ECI_CAN_ERROR_FORM      = int(2)             # < Form error
ECI_CAN_ERROR_ACK       = int(3)             # < Acknowledgment error
ECI_CAN_ERROR_BIT       = int(4)             # < Bit error
ECI_CAN_ERROR_FAST_DATA = int(5)             # < Fast data bit error (CAN FD)
ECI_CAN_ERROR_CRC       = int(6)             # < CRC error
ECI_CAN_ERROR_OTHER     = int(7)             # < Other (unspecified) error
# e_CANERROR;


# Status information supplied in the abData[0] field of status frames
# (CANMSGINFO.Bytes.bType = ECI_CAN_MSGTYPE_STATUS) and in
# ECI_CANSTATUS::u::V0::dwStatus.
ECI_CAN_STATUS_TXPEND       = int(0x01)      # < Transmission pending
ECI_CAN_STATUS_OVRRUN       = int(0x02)      # < Data overrun occurred
ECI_CAN_STATUS_ERRLIM       = int(0x04)      # < Error warning limit exceeded
ECI_CAN_STATUS_BUSOFF       = int(0x08)      # < Bus off status
ECI_CAN_STATUS_ININIT       = int(0x10)      # < Init mode active
ECI_CAN_STATUS_BUSCERR      = int(0x20)      # < Bus coupling error
ECI_CAN_STATUS_STTMSGACKERR = int(0x40)      # < single transmission try messages with acknowledge error
# e_CANSTATUS;


#  Information on which tx fifo should be cleared
#
#  @ingroup CanClearTxFifo
#
ECI_CAN_CLRDEV_REMPEND_BIT   = int(0x01) # < Remove the actual pending message
ECI_CAN_CLRDEV_CTRL_BIT      = int(0x02) # < Clear the controller fifo
ECI_CAN_CLRDEV_FIFO_BIT      = int(0x04) # < Clear the hardware fifo
ECI_CAN_CLRDEV_COMM_BIT      = int(0x08) # < Clear the communication buffers
ECI_CAN_CLRDRV_FIFO_BIT      = int(0x10) # < Clear the software fifo
ECI_CAN_CLRSRV_FIFO_BIT      = int(0x20) # < Clear the software fifo
ECI_CAN_CLRALL               = int(0xFF) # < Clear all
# e_CANCLEARTX;


# Filter selection whether you want to set a software filter where
# all traffic is handled by the driver or, if supported, you want to 
# set the hardware filter directly on the interface.

# @ingroup CanTypes
ECI_CAN_SW_FILTER_STD = int(0x00)         # < select standard software filter (11-bit)
ECI_CAN_SW_FILTER_EXT = int(0x01)         # < select extended software filter (29-bit)
ECI_CAN_HW_FILTER_STD = int(0x02)         # < select standard hardware filter (11-bit)
ECI_CAN_HW_FILTER_EXT = int(0x03)         # < select extended hardware filter (29-bit)

CAN_FILTER_ACC_CODE_ALL  = int(0x00000000)     # < filter id to receive all messages
CAN_FILTER_ACC_MASK_ALL  = int(0x00000000)     # < filter mask to receive all messages

CAN_FILTER_ACC_CODE_NONE = int(0xFFFFFFFF)     # < filter id to filter all messages
CAN_FILTER_ACC_MASK_NONE = int(0x80000000)     # < filter mask to filter all messages

ECI_CAN_MAX_11BIT_FILTERID = ((ECI_CAN_MAX_11BIT_ID << 1) | 0x1)
ECI_CAN_MAX_29BIT_FILTERID = ((ECI_CAN_MAX_29BIT_ID << 1) | 0x1)

# CAN controller configuration.
class ECI_CANINITLINE(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("bOpMode", BYTE),  # < CAN operating mode @see e_CANOPMODE
                ("bReserved", BYTE), # < Reserved set to 0
                ("bBtReg0", BYTE), # < Bus timing register 0 according to SJA1000/16MHz
                ("bBtReg1", BYTE), # < Bus timing register 1 according to SJA1000/16MHz
                ]
        
        #_anonymous_ = "V0"
        class _S1(Structure):
            _pack_ = 1
            _fields_ = [
                ("bOpMode", BYTE),  # < CAN operating mode @see e_CANOPMODE
                ("bExMode", BYTE),  # < Extended operation mode @see e_CANEXMODE
                ("sBtpSdr", ECI_CANBTP), # < Standard / arbitration bit rate timing
                ("sBtpFdr", ECI_CANBTP), # < Fast data bit rate timing
                ]

        _fields_ = [("V0", _S0),("V1", _S1)]
    _anonymous_ = "u"
    _fields_ = [ ("dwVer", DWORD),       # < Version of valid union struct
                  ("u", _U)]             # < Version controlled structs container

#  CAN controller capabilities.
class ECI_CANCAPABILITIES(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("wCanType", WORD),   # < Type of CAN controller @see e_CANCTRLCLASS
                ("wBusCoupling", WORD), # < Type of Bus coupling @see e_LINBUSC
                ("dwFeatures", DWORD), # < Supported features @see e_LINCTRLFEATURE
                ("dwClockFreq", DWORD), # < Clock frequency of the primary counter in Hz
                ("dwTscDivisor", DWORD),  # < Divisor for the message time stamp counter
                ("dwDtxDivisor", DWORD),  # < Divisor for the delayed message transmitter
                ("dwDtxMaxTicks", DWORD),  # < Maximum tick count value of the delayed message transmitter
                ("dwNoOfPrioQueues", DWORD),  # < Number of priority TX queues
                ]
        class _S1(Structure):
            _pack_ = 1
            _fields_ = [
                ("wCanType", WORD),            # < Type of CAN controller @see e_CANCTRLCLASS
                ("wBusCoupling", WORD),        # < Type of Bus coupling @see e_LINBUSC
                ("dwFeatures", DWORD),         # < Supported features @see e_LINCTRLFEATURE
                ("dwPriClkFreq", DWORD),       # < Clock frequency of the primary counter in Hz
                ("dwTscDivisor", DWORD),       # < Divisor for the message time stamp counter
                ("dwDtxDivisor", DWORD),       # < Divisor for the delayed message transmitter
                ("dwDtxMaxTicks", DWORD),      # < Maximum tick count value of the delayed message transmitter
                ("dwNoOfPrioQueues", DWORD),   # < Number of priority TX queues
                ("dwCanClkFreq", DWORD),       # < Number of priority TX queues
                ("sSdrRangeMin", ECI_CANBTP),  # < Minimum bit timing values for standard bit rate
                ("sSdrRangeMax", ECI_CANBTP),  # < Maximum bit timing values for standard bit rate
                ("sFdrRangeMin", ECI_CANBTP),  # < Minimum bit timing values for fast data bit rate
                ("sFdrRangeMax", ECI_CANBTP),  # < Maximum bit timing values for fast data bit rate
                ]

        _fields_ = [("V0", _S0),("V1", _S1)]
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U),             # < Version controlled structs container
        ]

#  CAN controller status.
class ECI_CANSTATUS(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("bOpMode", BYTE),   # < Current CAN operating mode @see e_CANOPMODE
                ("bBusLoad", BYTE),  # < Average bus load in percent (0..100)
                ("bBtReg0", BYTE),   # < Current bus timing register 0 value according to SJA1000/16MHz
                ("bBtReg1", BYTE),   # < Current bus timing register 1 value according to SJA1000/16MHz
                ("dwStatus", DWORD), # < Status of the CAN controller @see e_CANSTATUS
                ]
        class _S1(Structure):
            _pack_ = 1
            _fields_ = [
                ("bOpMode", BYTE),       # < Current CAN operating mode @see e_CANOPMODE
                ("bBusLoad", BYTE),      # < Average bus load in percent (0..100)
                ("bExMode", BYTE),       # < Extended operation mode @see e_CANEXMODE
                ("bReserved", BYTE),     # < Reserved set to 0
                ("dwStatus", DWORD),     # < Status of the CAN controller @see e_CANSTATUS
                ("sBtpSdr", ECI_CANBTP), # < Standard / arbitration bit rate timing
                ("sBtpFdr", ECI_CANBTP), # < Fast data bit rate timing
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S0),("V1", _S1)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)]             # < Version controlled structs container  

#  CAN message information.
class ECI_CANMSGINFO(Union):
    _pack_ = 1
    class _Sbytes(Structure):
        _pack_ = 1
        _fields_ = [
            ("bType", BYTE),   # < Message type @see e_CANMSGTYPE
            ("bFlags2", BYTE), # < Flags @see e_CANMSGFLAGS2
            ("bFlags", BYTE),  # < Flags @see e_CANMSGFLAGS
            ("bAccept", BYTE), # < Reserved for future use
            ]
    
    class _Sbits(Structure):
        _pack_ = 1
        _fields_ = [
            ("type", DWORD, 8),# < Message type @see e_LINMSGTYPE
            ("ssm", DWORD, 1), # < Single shot message (TX direction only).
                               # < If set is tried to sent the message once only. If
                               # < transmission or bus arbitration fails the message is discarded.
            ("hpm", DWORD, 2), # < High priority message channel 0-3 (TX direction only).
                               # < Selects the priority of the message where 0 is the lowest
                               # < and "normal" message channel and 3 is the highest priority 
                               # < message channel. The number of supported priority queues 
                               # < is available in @ref ECI_CANCAPABILITIES::dwNoOfPrioQueues.
            ("edl", DWORD, 1), # < Extended data length respectively improved frame format (CAN FD - EDL/IFF)
                               # < If set the dlc field is interpreted according to CAN FD
                               # < specification. This means up to 64 data bytes are supported.
            ("brs", DWORD, 1), # < Fast data rate, bit rate switch (CAN FD - BRS)
                               # < If set the data field is send or was received with a higher
                               # < data baud rate. Therefore the CAN controller has to be configured
                               # < with two independant baud rates.
            ("esi", DWORD, 1), # < Error state indicator (CAN FD - ESI) (RX direction only).
                               # < Indicates if a sending CAN FD node is in error passive level.
            ("res", DWORD, 2), # < Reserved for future use
            ("dlc", DWORD, 4), # < Data length code
            ("ovr", DWORD, 1), # < Possible data overrun (RX direction only).
            ("srr", DWORD, 1), # < Self reception request
            ("rtr", DWORD, 1), # < Remote transmission request
            ("ext", DWORD, 1), # < Extended frame format (0=standard, 1=extended)
            ("afc", DWORD, 8), # < Acceptance filter code
            ]

    _anonymous_ = ("Bytes", "Bits",)
    
    _fields_ = [ 
        ("Bytes", _Sbytes),       # < CAN Message information in byte format
        ("Bits", _Sbits)          # < CAN Message information in bit format
        ]

#  CAN message structure.
class ECI_CANMESSAGE(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [
                ("dwTime", DWORD),  # < Time stamp for receive message in [us]
                ("dwMsgId", DWORD), # < CAN message identifier (INTEL format)
                ("uMsgInfo", ECI_CANMSGINFO), # < CAN message information (bit field)
                ("abData", BYTE*8), # < Message data
                ]
        class _S1(Structure):
            _pack_ = 1
            _fields_ = [
                ("dwTime", DWORD),            # < Time stamp for receive message in [us]
                ("dwMsgId", DWORD),           # < CAN message identifier (INTEL format)
                ("uMsgInfo", ECI_CANMSGINFO), # < CAN message information (bit field)
                ("abData", BYTE*64),          # < Message data
                ]
        _fields_ = [("V0", _S0),("V1", _S1)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)             # < Version controlled structs container
        ]
  
#  CAN command structure.
class ECI_CAN_CMD_REQ_HD(Structure):
    _pack_ = 1
    _fields_ = [ 
        ("wCode", WORD),       # < Command request code
        ]

#  CAN Command structure.
class ECI_CAN_CMD_RES_HD(Structure):
    _pack_ = 1
    _fields_ = [ 
        ("wResult", WORD),       # < Command result code
        ]

# CAN command structure.
class ECI_CANCMDREQUEST(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S(Structure):
            _pack_ = 1
            _fields_ = [
                ("sCmdHeader", ECI_CAN_CMD_REQ_HD),  # < CAN command header @see ECI_CAN_CMD_REQ_HD
                ("dwReserved", DWORD), # < reserved for future use
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U)             # < Version controlled structs container
        ]
      

# CAN command structure.
class ECI_CANCMDRESPONSE(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S(Structure):
            _pack_ = 1
            _fields_ = [
                ("sCmdHeader", ECI_CAN_CMD_RES_HD), # < CAN command header @see ECI_CAN_CMD_RES_HD
                ("dwReserved", DWORD), # < reserved for future use
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U),             # < Version controlled structs container
        ]  


# CAN filter structure.
class ECI_CANFILTER(Structure):
    _pack_ = 1
    class _U(Union):
        _pack_ = 1
        class _S0(Structure):
            _pack_ = 1
            _fields_ = [ 
                ("dwIsExtended", DWORD), # < specifies whether to choose extended or standard id
                ("dwCode", DWORD),       # < id code for filter settings
                ("dwMask", DWORD),       # < mask code for filter settings
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [("V0", _S0)]          # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),       # < Version of valid union struct
        ("u", _U),             # < Version controlled structs container
        ]
