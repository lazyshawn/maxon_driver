#  OS dependent function hiding


# Module Imports
#
from ctypes import *  # @UnusedWildImport

#//////////////////////////////////////////////////////////////////////////
#// constants and macros


# < Define used datatype to easier porting the ECI to another OS
VOID    = c_ubyte      
CHAR    = c_char       
BYTE    = c_uint8      
BOOL    = c_int32      
WORD    = c_uint16     
DWORD   = c_uint32     
QWORD   = c_uint64     
LONG    = c_long       
ULONG   = c_ulong     
INT32   = c_int32     
INT64   = c_int64     
HRESULT = c_int32   
FLOAT   = c_float     

PVOID   = c_void_p    
PCHAR   = c_char_p    
PBYTE   = POINTER(BYTE)    
PBOOL   = POINTER(BOOL)    
PWORD   = POINTER(WORD)   
PDWORD  = POINTER(DWORD)   
PLONG   = POINTER(LONG)     
PFLOAT  = POINTER(FLOAT)   
HANDLE  = c_void_p      
PHANDLE = POINTER(HANDLE)   


#define LOBYTE(wVal)  ((BYTE) wVal)           # < Macro for accessing low byte @ingroup OsEci
#define HIBYTE(wVal)  ((BYTE) ( wVal >> 8))   # < Macro for accessing high byte @ingroup OsEci
#define LOWORD(dwVal) ((WORD) dwVal)          # < Macro for accessing low word @ingroup OsEci
#define HIWORD(dwVal) ((WORD) ( dwVal >> 16)) # < Macro for accessing high word @ingroup OsEci

OS_WAIT_FOREVER = DWORD(0xFFFFFFFF)  # < Blocking function call @ingroup OsEci

# return values of OS_WaitForSingleObject
OS_WAIT_OBJECT_0 = ULONG(0)                  # < Wait succeeded @ingroup OsEci
OS_WAIT_TIMEOUT  = ULONG(1)                  # < Wait timed out @ingroup OsEci
OS_WAIT_FAILED   = DWORD(0xFFFFFFFF)         # < Wait failed @ingroup OsEci


# Macro to swap the bytes of a WORD (ab -> ba)  @ingroup OsEci */
#define SWAP16(w)  ( (WORD)  ((LOBYTE(w) << 8) | HIBYTE (w)))

# Macro to swap the bytes of a DWORD (abcd -> dcba)  @ingroup OsEci */
#define SWAP32(dw) ( (DWORD) (((SWAP16 (LOWORD (dw))) << 16) | (SWAP16 (HIWORD (dw)))))

# Macro for printf redirection  @ingroup OsEci */
#define OS_Printf printf

# Macro for fflush redirection  @ingroup OsEci */
#define OS_Fflush(stream) fflush(stream)


