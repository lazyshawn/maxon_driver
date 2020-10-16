#  Definition of types, structs and unions for logging feature.

# Module Imports
#
from ctypes import *  # @UnusedWildImport
from ECI_hwtype import *  # @UnusedWildImport


#//////////////////////////////////////////////////////////////////////////
#// data types

#  ECI logging mode
ECI_LOGGING_MODE_UNDEFINED = int(0x00) # < Undefined
ECI_LOGGING_MODE_FIFO      = int(0x01)  # < Use FIFO, new log events are discarded if FIFO is full.
# e_ECI_LOGGING_MODE;


#  ECI logging sources
ECI_LOGGING_SOURCE_NONE          = int(0x0000) # < Log no messages
ECI_LOGGING_SOURCE_INFO_ALL      = int(0x1FFF) # < Log all info messages
ECI_LOGGING_SOURCE_INFO_ECI_API  = int(0x1001) # < Log info ECI API messages
ECI_LOGGING_SOURCE_WARNING_ALL   = int(0x2FFF) # < Log all warning messages
ECI_LOGGING_SOURCE_WARNING_TS    = int(0x2001) # < Log warning timestamp messages
ECI_LOGGING_SOURCE_ERROR_ALL     = int(0x4FFF) # < Log all error messages
ECI_LOGGING_SOURCE_ERROR_CCI     = int(0x4001) # < Log error CCI messages
ECI_LOGGING_SOURCE_DEBUG_ALL     = int(0x8FFF) # < Log all debug messages
ECI_LOGGING_SOURCE_DEBUG_TS      = int(0x8001) # < Log debug timestamp messages
ECI_LOGGING_SOURCE_DEBUG_DEVMSG  = int(0x8002) # < Log debug device messages
ECI_LOGGING_SOURCE_ALL           = int(0xFFFF)  # < Log all message
# e_ECI_LOGGING_SOURCE;


#  ECI logging flags
ECI_LOGGING_FLAG_NONE     = int(0x00) # < Undefined
ECI_LOGGING_FLAG_OVERRUN  = int(0x01)  # < Set if an overrun occured,
                                    # < at least one entry is missing after this entry.
# e_ECI_LOGGING_FLAGS;


#  Structure for a logging message.
class ECI_LOG_ENTRY(Structure):
    class _U(Union):
        class _S(Structure):
            _fields_ = [ 
                ("dwTime", DWORD),    # < System time on which log entry was generated in [ms]
                ("dwSource", DWORD),  # < Source of log entry
                ("dwFlags", DWORD),   # < Flags for log entry @see e_ECI_LOGGING_FLAGS
                ("dwLostCount", DWORD),  # < Total number of log entries lost since last configuration.
                ("szLog", CHAR * 0x100),         # < Logging message
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [ ("V0", _S)]                            # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),                         # < Version of valid union struct
        ("u", _U)                                 # < Version controlled structs container
        ]


#  Structure to configure the logging.
class ECI_LOG_CONFIG(Structure):
    class _U(Union):
        class _S(Structure):
            _fields_ = [ 
                ("dwLogMode", DWORD),    # < Logging mode @see e_ECI_LOGGING_MODE
                ("dwLogSources", DWORD),  # < Sources to log @see e_ECI_LOGGING_SOURCE
                ("dwLogSize", DWORD),   # < Number of entries, dependant from dwLogMode.
                ]
        #_anonymous_ = ("V0",)
        _fields_ = [ ("V0", _S)]                            # < Version 0
    
    _anonymous_ = ("u",)
    _fields_ = [ 
        ("dwVer", DWORD),                         # < Version of valid union struct
        ("u", _U)                                # < Version controlled structs container
        ]

