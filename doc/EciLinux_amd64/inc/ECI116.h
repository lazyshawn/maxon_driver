///////////////////////////////////////////////////////////////////////////////
// (C) 2008-2016 HMS Technology Center Ravensburg GmbH, all rights reserved
///////////////////////////////////////////////////////////////////////////////
/**
  ECI API function redefintions for IXXAT Interface: USB-to-CAN FD.

  Headerfile of the ECI ( embedded / realtime communication interface),
  a generic library for IXXAT hardware interfaces.

  @file ECI116.h
*/

#ifndef __ECI116_H__
#define __ECI116_H__

//*** At first undefine maybe previously defined ECI export functions
#undef ECIDRV_
#undef ECIDRV_Initialize
#undef ECIDRV_Release
#undef ECIDRV_GetInfo
#undef ECIDRV_CtrlOpen
#undef ECIDRV_CtrlClose
#undef ECIDRV_CtrlStart
#undef ECIDRV_CtrlStop
#undef ECIDRV_CtrlGetCapabilities
#undef ECIDRV_CtrlSetStatusUpdateRate
#undef ECIDRV_CtrlGetStatus
#undef ECIDRV_CtrlSend
#undef ECIDRV_CtrlReceive
#undef ECIDRV_CtrlSetAccFilter
#undef ECIDRV_CtrlAddFilterIds        
#undef ECIDRV_CtrlRemFilterIds       
#undef ECIDRV_CtrlCommand
#undef ECIDRV_LogConfig
#undef ECIDRV_LogRead
#undef ECIDRV_LogStart
#undef ECIDRV_LogStop
#undef ECIDRV_LogClear
#undef ECIDRV_GetErrorString


//*** Define ECI export functions for defined hardware type
#define ECIDRV_                        ECI116_
#define ECIDRV_Initialize              ECI116_Initialize
#define ECIDRV_Release                 ECI116_Release
#define ECIDRV_GetInfo                 ECI116_GetInfo
#define ECIDRV_CtrlOpen                ECI116_CtrlOpen
#define ECIDRV_CtrlClose               ECI116_CtrlClose
#define ECIDRV_CtrlStart               ECI116_CtrlStart
#define ECIDRV_CtrlStop                ECI116_CtrlStop
#define ECIDRV_CtrlGetCapabilities     ECI116_CtrlGetCapabilities
#define ECIDRV_CtrlSetStatusUpdateRate ECI116_CtrlSetStatusUpdateRate
#define ECIDRV_CtrlGetStatus           ECI116_CtrlGetStatus
#define ECIDRV_CtrlSend                ECI116_CtrlSend
#define ECIDRV_CtrlReceive             ECI116_CtrlReceive
#define ECIDRV_CtrlSetAccFilter        ECI116_CtrlSetAccFilter
#define ECIDRV_CtrlAddFilterIds        ECI116_CtrlAddFilterIds
#define ECIDRV_CtrlRemFilterIds        ECI116_CtrlRemFilterIds
#define ECIDRV_CtrlCommand             ECI116_CtrlCommand
#define ECIDRV_LogConfig               ECI116_LogConfig
#define ECIDRV_LogRead                 ECI116_LogRead
#define ECIDRV_LogStart                ECI116_LogStart
#define ECIDRV_LogStop                 ECI116_LogStop
#define ECIDRV_LogClear                ECI116_LogClear
#define ECIDRV_GetErrorString          ECI116_GetErrorString


/** @def ECIDRV_
  General ECI API function renaming */
/** @def ECIDRV_Initialize
  @brief @copybrief ECIDRV_Initialize @n <b> See function:</b> @ref ECIDRV_Initialize */
/** @def ECIDRV_Release
  @brief @copybrief ECIDRV_Release @n <b> See function:</b> @ref ECIDRV_Release */
/** @def ECIDRV_GetInfo
  @brief @copybrief ECIDRV_GetInfo @n <b> See function:</b> @ref ECIDRV_GetInfo */
/** @def ECIDRV_CtrlOpen
  @brief @copybrief ECIDRV_CtrlOpen @n <b> See function:</b> @ref ECIDRV_CtrlOpen */
/** @def ECIDRV_CtrlClose
  @brief @copybrief ECIDRV_CtrlClose @n <b> See function:</b> @ref ECIDRV_CtrlClose */
/** @def ECIDRV_CtrlStart
  @brief @copybrief ECIDRV_CtrlStart @n <b> See function:</b> @ref ECIDRV_CtrlStart */
/** @def ECIDRV_CtrlStop
  @brief @copybrief ECIDRV_CtrlStop @n <b> See function:</b> @ref ECIDRV_CtrlStop */
/** @def ECIDRV_CtrlGetCapabilities
  @brief @copybrief ECIDRV_CtrlGetCapabilities @n <b> See function:</b> @ref ECIDRV_CtrlGetCapabilities */
/** @def ECIDRV_CtrlSetStatusUpdateRate
  @brief @copybrief ECIDRV_CtrlSetStatusUpdateRate @n <b> See function:</b> @ref ECIDRV_CtrlSetStatusUpdateRate */
/** @def ECIDRV_CtrlGetStatus
  @brief @copybrief ECIDRV_CtrlGetStatus @n <b> See function:</b> @ref ECIDRV_CtrlGetStatus */
/** @def ECIDRV_CtrlSend
  @brief @copybrief ECIDRV_CtrlSend @n <b> See function:</b> @ref ECIDRV_CtrlSend */
/** @def ECIDRV_CtrlReceive
  @brief @copybrief ECIDRV_CtrlReceive @n <b> See function:</b> @ref ECIDRV_CtrlReceive */
/** @def ECIDRV_CtrlSetAccFilter 
  @brief @copybrief ECIDRV_CtrlSetAccFilter @n <b> See function:</b> @ref ECIDRV_CtrlSetAccFilter */
/** @def ECIDRV_CtrlAddFilterIds 
  @brief @copybrief ECIDRV_CtrlAddFilterIds @n <b> See function:</b> @ref ECIDRV_CtrlAddFilterIds */
/** @def ECIDRV_CtrlCommand
  @brief @copybrief ECIDRV_CtrlCommand @n <b> See function:</b> @ref ECIDRV_CtrlCommand */
/** @def ECIDRV_LogConfig
  @brief @copybrief ECIDRV_LogConfig @n <b> See function:</b> @ref ECIDRV_LogConfig */
/** @def ECIDRV_LogRead
  @brief @copybrief ECIDRV_LogRead @n <b> See function:</b> @ref ECIDRV_LogRead */
/** @def ECIDRV_LogStart
  @brief @copybrief ECIDRV_LogStart @n <b> See function:</b> @ref ECIDRV_LogStart */
/** @def ECIDRV_LogStop
  @brief @copybrief ECIDRV_LogStop @n <b> See function:</b> @ref ECIDRV_LogStop */
/** @def ECIDRV_LogClear
  @brief @copybrief ECIDRV_LogClear @n <b> See function:</b> @ref ECIDRV_LogClear */
/** @def ECIDRV_GetErrorString
  @brief @copybrief ECIDRV_GetErrorString @n <b> See function:</b> @ref ECIDRV_GetErrorString */


//////////////////////////////////////////////////////////////////////////
// include files
#include <ECI.h>


#endif //__ECI116_H__
