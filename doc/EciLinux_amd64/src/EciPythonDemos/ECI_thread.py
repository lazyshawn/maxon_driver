#  Definition of types, structs and unions for hardware initialization and 
#  communication.
#

# Module Imports
#
from ctypes import *  # @UnusedWildImport
from OsEci import *  # @UnusedWildImport

#//////////////////////////////////////////////////////////////////////////
#// constants and macros
#//////////////////////////////////////////////////////////////////////////

#  Scheduler types
ECI_SCHED_NOCHANGE   = int(0x00)     # < keep actual scheduler
ECI_SCHED_FIFO       = int(0x01)     # < fifo scheduling
ECI_SCHED_RR         = int(0x02)     # < round robin scheduling
ECI_SCHED_OTHER      = int(0x03)     # < other scheduling
# e_SCHEDPOLICY;

#  Definition of the PCI hardware configuration parameters.
#  Board information.
class ECI_SCHEDULER_SETTINGS(Structure):
    _pack_ = 1
    _fields_ = [ 
        ("dwSchedulerPolicy", DWORD),        # < Scheduler policy class @see e_SCHEDPOLICY
        ("dwSchedulerPriority", DWORD),      # < Scheduler thread priority
        ("dwCpuAffinity", DWORD),            # < CPU affinity of the thread @see e_CPUAFFINITY
        ]
