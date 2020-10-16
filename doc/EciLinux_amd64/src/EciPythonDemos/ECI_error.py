#  Definition of error codes used by the ECI.

# Module Imports
#
from ctypes import *  # @UnusedWildImport

from OsEci import *  # @UnusedWildImport


#//////////////////////////////////////////////////////////////////////////
#// constants and macros

# Maximal length of an error string
TEST_MAX_ERRSTR       = int(256)

# Facility code for the ECI error codes
FACILITY_IXXAT_ECI    = int(0x00FE0000)

# Severity code success
SEV_SUCCESS           = int(0x00000000)
# Severity code info
SEV_INFO              = int(0x40000000)
# Severity code warning
SEV_WARN              = int(0x80000000)
# Severity code error
SEV_ERROR             = int(0xC0000000)

# Customer flag
CUSTOMER_FLAG         = int(0x20000000)
# Reserved flag
RESERVED_FLAG         = int(0x10000000)

# IXXAT defined ECI info code
SEV_ECI_INFO          = int(SEV_INFO  | CUSTOMER_FLAG | FACILITY_IXXAT_ECI)
# IXXAT defined ECI warning code
SEV_ECI_WARN          = int(SEV_WARN  | CUSTOMER_FLAG | FACILITY_IXXAT_ECI)
# IXXAT defined ECI error code
SEV_ECI_ERROR         = int(SEV_ERROR | CUSTOMER_FLAG | FACILITY_IXXAT_ECI)

# Mask to determine the facility code
FACILITY_MASK         = int(0x0FFF0000)
# Mask to determine the status resp. error code
STATUS_MASK           = int(0x0000FFFF)

# Data type for ECI_RESULT
ECI_RESULT            = HRESULT

#  List of ECI error codes.
ECI_OK = int(0x0000)                          # < Operation finished successfully.

# TODO probably this way c enum is not correctly represented
ECI_ERR_FIRST = int(SEV_ECI_ERROR | 0x0000)
( 
ECI_ERR_INVALIDARG,                            # < One or more arguments are invalid.
ECI_ERR_FAILED,                                # < Failed with unknown error.
ECI_ERR_NOTIMPL,                               # < Function is not implemented.
ECI_ERR_NOT_SUPPORTED,                         # < Type or parameter is not supported.
ECI_ERR_ACCESS_DENIED,                         # < Access denied.
ECI_ERR_RESOURCE_BUSY,                         # < The device or resource is busy.
ECI_ERROR_OUTOFMEMORY,                         # < Ran out of memory
ECI_ERR_INVALID_HANDLE,                        # < The handle is invalid.
ECI_ERR_INVALID_POINTER,                       # < The pointer is invalid.
ECI_ERR_INVALID_DATATYPE,                      # < The data type is invalid.
ECI_ERR_INVALID_DATA,                          # < The data is invalid.
ECI_ERR_TIMEOUT,                               # < This operation returned because the timeout period expired.
ECI_ERR_INSUFFICIENT_RESOURCES,                # < The available resources are insufficient to perform this request.
ECI_ERR_RESOURCE_NOT_FOUND,                    # < The device or resource could not be found.
ECI_ERR_REQUEST_FAILED,                        # < The request failed.
ECI_ERR_INVALID_RESPONSE,                      # < The received response is invalid.
ECI_ERR_UNKNOWN_RESPONSE,                      # < The received response is unknown.
ECI_ERR_BAD_COMMAND,                           # < The device does not recognize the command.
ECI_ERR_NO_MORE_DATA,                          # < No more data is available.
ECI_ERR_NO_MORE_SPACE_LEFT,                    # < No more space left to perform this request.
ECI_ERR_UNSUPPORTED_VERSION,                   # < The available or requested version is unsupported.
# ECI specific
ECI_ERR_INVALID_CTRLHANDLE,                    # < The specified ECI board or controller handle is invalid.
ECI_ERR_WRONG_STATE,                           # < ECI is in wrong state to perform this request.
ECI_ERR_CREATE_FAILED,                         # < ECI interface could not be instantiated correctly.
ECI_ERR_IRQTEST_FAILED,                        # < IRQ handler could not be installed correctly.
ECI_ERR_FWDOWNLOAD,                            # < An error occurred while downloading or verifying firmware
ECI_ERR_DPRAM_LOCK_VIOLATION,                  # < A lock violation occurred while communication via DPRAM interface.
ECI_ERR_DRPAM_IO_ERROR,                        # < An I/O error occurred while reading from or writing to DRPAM interface
ECI_ERR_UNSUPPORTED_FWVERSION,                 # < The boot manager or firmware version is unsupported.
ECI_ERR_USB_INCOMPLETE_DESCRIPTOR,             # < The USB device descriptor is invalid.
ECI_ERR_FLASH_WRITE,                           # < The system cannot write to the specified flash memory.
ECI_ERR_FLASH_READ,                            # < The system cannot read from the specified flash memory.
ECI_ERR_FLASH_ERASE,                           # < The requested delete operation could not be performed on the flash memory.
ECI_ERR_FLASH_VERIFY,                          # < The requested verify operation failed.
ECI_ERROR_SUCCESS_REBOOT_REQUIRED,             # < The requested operation finishes successfully. Changes will not be effective until the system is rebooted.
ECI_ERR_LAST
#) = map(c_int, xrange(ECI_ERR_FIRST, 36))
) = map(int, range(ECI_ERR_FIRST + 1, ECI_ERR_FIRST + 37))
# e_ECIERROR;
