///////////////////////////////////////////////////////////////////////////////
// (C) 2008-2013 IXXAT Automation GmbH, all rights reserved
///////////////////////////////////////////////////////////////////////////////
/**
  ECI API Demo for iPC-I XC16 / PCIe

  @author Michael Ummenhofer (ummenhofer@ixxat.de)
  @file EciDemo10E.c
*/


//////////////////////////////////////////////////////////////////////////
// compiler directives

//////////////////////////////////////////////////////////////////////////
// include files
#include <EciDemo10E.h>
#include <EciDemoCommon.h>


//////////////////////////////////////////////////////////////////////////
// static constants, types, macros, variables

/** ECI Demo send timeout in [ms] @ingroup EciDemo */
#define ECIDEMO_TX_TIMEOUT 500

/** ECI Demo TX message count for CAN @ingroup EciDemo */
#define ECIDEMO_TX_MSGCOUNT_CAN (0x800 * 10)

/** ECI Demo TX message count for LIN @ingroup EciDemo */
#define ECIDEMO_TX_MSGCOUNT_LIN (0x800 * 1)

/** ECI Demo receive timeout in [ms] @ingroup EciDemo */
#define ECIDEMO_RX_TIMEOUT 500

/** ECI Demo total receive timeout in [ms] @ingroup EciDemo */
#define ECIDEMO_RX_TOTALTIMEOUT 30000

/**
 If defined ECI Demo will run in polling mode instead of using an
 event triggered mode. This mode can be used, if it is not possible to
 assign an unique IRQ to the device to use. The device driver than
 will not use an IRQ based message reception and transmission.

  @ingroup EciDemo
*/
//#define ECIDEMO_HWUSEPOLLINGMODE

/** ECI Demo error check macro @ingroup EciDemo */
#define ECIDEMO_CHECKERROR(FuncName) \
{\
  if(ECI_OK == hResult)\
  {\
    OS_Printf(#FuncName "...succeeded.\n"); \
  }\
  else\
  {\
    OS_Printf( #FuncName "...failed with error code: 0x%08X. %s\n", \
               hResult, \
               ECI10E_GetErrorString(hResult)); \
  }\
}
//////////////////////////////////////////////////////////////////////////
// global variables

//////////////////////////////////////////////////////////////////////////
// static function prototypes

ECI_RESULT EciCanDemo10E  ( DWORD   dwHwIndex,
                            DWORD   dwCtrlIndex);
ECI_RESULT EciLinDemo10E  ( DWORD   dwHwIndex,
                            DWORD   dwCtrlIndex);

//////////////////////////////////////////////////////////////////////////
// global functions

//////////////////////////////////////////////////////////////////////////
// static functions


///////////////////////////////////////////////////////////////////////////////
/**
  ECI Demo for iPC-I XC16 / PCIe

  @return ECI_OK on success, otherwise an error code
  @ingroup EciDemo
*/
ECI_RESULT EciDemo10E(void)
{
  ECI_RESULT  hResult       = ECI_OK;
  ECI_HW_PARA astcHwPara[4] = {{0}};
  ECI_HW_INFO stcHwInfo     = {0};
  DWORD       dwIndex       = 0;
  DWORD       dwHwIndex     = 0;
  DWORD       dwCtrlIndex   = 0;

  OS_Printf("\n>> ECI Demo for iPC-I XC16 / PCIe <<\n");

  //*** Prepare Hardware parameter structure for multiple boards
  for(dwIndex=0; dwIndex < _countof(astcHwPara); dwIndex++)
  {
    astcHwPara[dwIndex].wHardwareClass = ECI_HW_PCI;
    #ifdef ECIDEMO_HWUSEPOLLINGMODE
      astcHwPara[dwIndex].dwFlags = ECI_SETTINGS_FLAG_POLLING_MODE;
    #endif //ECIDEMO_HWUSEPOLLINGMODE
  }

  //*** At first call Initialize to prepare ECI driver
  hResult = ECI10E_Initialize(_countof(astcHwPara), astcHwPara);
  ECIDEMO_CHECKERROR(ECI10E_Initialize);

  //*** Retrieve hardware info
  if(ECI_OK == hResult)
  {
    //*** Retrieve hardware info
    hResult = ECI10E_GetInfo(dwHwIndex, &stcHwInfo);
    ECIDEMO_CHECKERROR(ECI10E_GetInfo);
    if(ECI_OK == hResult)
      {EciPrintHwInfo(&stcHwInfo);}
  }

  //*** Find first CAN Controller of Board
  if(ECI_OK == hResult)
  {
    hResult = EciGetNthCtrlOfClass(&stcHwInfo,
                                   ECI_CTRL_CAN,
                                   0, //first relative controller
                                   &dwCtrlIndex);
    if(ECI_OK == hResult)
    {
      //*** Start CAN Demo
      hResult =  EciCanDemo10E(dwHwIndex, dwCtrlIndex);
      ECIDEMO_CHECKERROR(EciCanDemo10E);
    }
    else
    {
      //*** Ignore if no controller was found
      hResult = ECI_OK;
    }
  }

  //*** Find first LIN Controller of Board
  if(ECI_OK == hResult)
  {
    hResult = EciGetNthCtrlOfClass(&stcHwInfo,
                                   ECI_CTRL_LIN,
                                   0, //first relative controller
                                   &dwCtrlIndex);
    if(ECI_OK == hResult)
    {
      //*** Start LIN Demo
      hResult =  EciLinDemo10E(dwHwIndex, dwCtrlIndex);
      ECIDEMO_CHECKERROR(EciLinDemo10E);
    }
    else
    {
      //*** Ignore if no controller was found
      hResult = ECI_OK;
    }
  }

  //*** Clean up ECI driver
  ECI10E_Release();

  OS_Printf("-> Returning from ECI Demo for iPC-I XC16 / PCIe <-\n");

  return hResult;
}


///////////////////////////////////////////////////////////////////////////////
/**
  ECI CAN Demo

  @param dwHwIndex
    Hardware index of board to use.

  @param dwCtrlIndex
    Controller index to use.

  @return ECI_OK on success, otherwise an error code

  @ingroup EciDemo
*/
ECI_RESULT EciCanDemo10E(DWORD dwHwIndex,
                         DWORD dwCtrlIndex)
{
  ECI_RESULT            hResult         = ECI_OK;
  ECI_CTRL_CAPABILITIES stcCtrlCaps     = {0};
  ECI_CTRL_HDL          dwCtrlHandle    = ECI_INVALID_HANDLE;
  DWORD                 dwCtrlFeatures  = 0;

  OS_Printf("\n>> ECI CAN Demo <<\n");

  //*** Open given controller of given board
  if(ECI_OK == hResult)
  {
    ECI_CTRL_CONFIG stcCtrlConfig = {0};

    //*** Use Basic settings to open controller
    stcCtrlConfig.wCtrlClass                 = ECI_CTRL_CAN;
    stcCtrlConfig.u.sCanConfig.dwVer         = ECI_STRUCT_VERSION_V0;
    stcCtrlConfig.u.sCanConfig.u.V0.bBtReg0  = ECI_CAN_BT0_1000KB;
    stcCtrlConfig.u.sCanConfig.u.V0.bBtReg1  = ECI_CAN_BT1_1000KB;
    stcCtrlConfig.u.sCanConfig.u.V0.bOpMode  = ECI_CAN_OPMODE_STANDARD;

    //*** Open and Initialize given Controller of given board
    hResult = ECI10E_CtrlOpen(&dwCtrlHandle, dwHwIndex, dwCtrlIndex, &stcCtrlConfig);
    ECIDEMO_CHECKERROR(ECI10E_CtrlOpen);
 }

  //*** Get Controller Capabilities
  if(ECI_OK == hResult)
  {
    //*** Enable ECI structure Version 1 support
    stcCtrlCaps.wCtrlClass       = ECI_CTRL_CAN;
    stcCtrlCaps.u.sCanCaps.dwVer = ECI_STRUCT_VERSION_V1;
    hResult = ECI10E_CtrlGetCapabilities(dwCtrlHandle, &stcCtrlCaps);
    ECIDEMO_CHECKERROR(ECI10E_CtrlGetCapabilities);
    if(ECI_OK == hResult)
    {
      EciPrintCtrlCapabilities(&stcCtrlCaps);

      //*** Check if CAN Controller and if Struct version up to V1 is supported
      if(ECI_CTRL_CAN          == stcCtrlCaps.wCtrlClass)
      {
        if(ECI_STRUCT_VERSION_V0 == stcCtrlCaps.u.sCanCaps.dwVer)
          { dwCtrlFeatures = stcCtrlCaps.u.sCanCaps.u.V0.dwFeatures; }
        else
        if(ECI_STRUCT_VERSION_V1 == stcCtrlCaps.u.sCanCaps.dwVer)
          { dwCtrlFeatures = stcCtrlCaps.u.sCanCaps.u.V1.dwFeatures; }
      }
    }
  }

  //*** Re-configure given controller of given board
  if(ECI_OK == hResult)
  {
    ECI_CTRL_CONFIG stcCtrlConfig = {0};

    //*** Set CAN Controller configuration
    stcCtrlConfig.wCtrlClass                 = ECI_CTRL_CAN;
    stcCtrlConfig.u.sCanConfig.dwVer         = ECI_STRUCT_VERSION_V0;
    stcCtrlConfig.u.sCanConfig.u.V0.bBtReg0  = ECI_CAN_BT0_1000KB;
    stcCtrlConfig.u.sCanConfig.u.V0.bBtReg1  = ECI_CAN_BT1_1000KB;
    stcCtrlConfig.u.sCanConfig.u.V0.bOpMode  = ECI_CAN_OPMODE_STANDARD | ECI_CAN_OPMODE_EXTENDED;
    stcCtrlConfig.u.sCanConfig.u.V0.bOpMode |= (dwCtrlFeatures & ECI_CAN_FEATURE_ERRFRAME) ? ECI_CAN_OPMODE_ERRFRAME : 0;

    //*** Re-configure given controller of given board
    hResult = ECI10E_CtrlOpen(&dwCtrlHandle, dwHwIndex, dwCtrlIndex, &stcCtrlConfig);
    ECIDEMO_CHECKERROR(ECI10E_CtrlOpen);
  }

  //*** Start Controller
  if(ECI_OK == hResult)
  {
    hResult = ECI10E_CtrlStart(dwCtrlHandle);
    ECIDEMO_CHECKERROR(ECI10E_CtrlStart);
  }


  //*** Send some CAN Messages
  if(ECI_OK == hResult)
  {
    ECI_CTRL_MESSAGE stcCtrlMsg   = {0};
    DWORD            dwTxMsgCount = ECIDEMO_TX_MSGCOUNT_CAN;
    DWORD            dwIndex      = 0;

    OS_Printf("Now, sending %u CAN Messages\n", dwTxMsgCount);

    //*** Send Loop
    for(dwIndex=0; dwIndex < dwTxMsgCount; dwIndex++)
    {
      //*** Prepare CAN Message to send
      stcCtrlMsg.wCtrlClass                            = ECI_CTRL_CAN;
      stcCtrlMsg.u.sCanMessage.dwVer                   = ECI_STRUCT_VERSION_V0;
      stcCtrlMsg.u.sCanMessage.u.V0.dwMsgId            = (dwIndex % (ECI_CAN_MAX_11BIT_ID +1));
      stcCtrlMsg.u.sCanMessage.u.V0.uMsgInfo.Bits.dlc  = (dwIndex % (_countof(stcCtrlMsg.u.sCanMessage.u.V0.abData)+1));
      memcpy( &stcCtrlMsg.u.sCanMessage.u.V0.abData[0],
              &dwIndex,
              min(4, sizeof(dwIndex)));
      memcpy( &stcCtrlMsg.u.sCanMessage.u.V0.abData[4],
              &dwIndex,
              min(4, sizeof(dwIndex)));

      //*** Send one message
      #ifdef ECIDEMO_HWUSEPOLLINGMODE
      {
        DWORD dwStartTime = OS_GetTimeInMs();

        //*** Loop until message is sent or timeout has passed
        do
        {
          hResult = ECI10E_CtrlSend( dwCtrlHandle, &stcCtrlMsg, 0);
          if(ECI_OK != hResult)
            { OS_Sleep(1);}
        }while((ECI_OK != hResult) && ((OS_GetTimeInMs() - dwStartTime) < ECIDEMO_TX_TIMEOUT));
      }
      #else
        hResult = ECI10E_CtrlSend( dwCtrlHandle, &stcCtrlMsg, ECIDEMO_TX_TIMEOUT);
      #endif //ECIDEMO_HWUSEPOLLINGMODE
      if(ECI_OK != hResult)
      {
        OS_Printf("Error while sending CAN Messages\n");
        ECIDEMO_CHECKERROR(ECI10E_CtrlSend);
        hResult = ECI_OK;
        break;
      }
      else
      {
        //*** Read out all received messages
        ECI_CTRL_MESSAGE  astcCtrlMsg[20] = {{0}};
        DWORD             dwCount         = _countof(astcCtrlMsg);
        DWORD             dwMsgIndex      = 0;

        //*** Try to read some messages
        hResult = ECI10E_CtrlReceive( dwCtrlHandle, &dwCount, astcCtrlMsg, 0);

        //*** Loop through all received messages
        dwMsgIndex = 0;
        while((ECI_OK == hResult) && (dwCount > dwMsgIndex))
        {
          EciPrintCtrlMessage(&astcCtrlMsg[dwMsgIndex]);
          OS_Printf("\n");

          //*** Proceed with next message
          dwMsgIndex++;
        }//end while
        //*** Reset Error code and proceed with transmission
        hResult = ECI_OK;
      }//end else
    }//end for
  }//endif


  //*** Try to receive some CAN Messages
  if(ECI_OK == hResult)
  {
    ECI_CTRL_MESSAGE stcCtrlMsg    = {0};
    DWORD            dwStartTime   = 0;
    DWORD            dwCurrentTime = 0;
    DWORD            dwRxTimeout   = ECIDEMO_RX_TOTALTIMEOUT;
    DWORD            dwCount       = 0;

    //*** Receive Messages
    OS_Printf("Now, receiving CAN Messages for %u seconds\n", dwRxTimeout/1000);

    //*** Get current time
    dwStartTime   = OS_GetTimeInMs();
    dwCurrentTime = dwStartTime;
    hResult       = ECI_ERR_TIMEOUT;

    //*** Loop until timeout
    while(dwRxTimeout >= (dwCurrentTime - dwStartTime))
    {
      //*** Try to read Message
      #ifdef ECIDEMO_HWUSEPOLLINGMODE
      {
        DWORD dwStartTime = OS_GetTimeInMs();

        //*** Loop until message is sent or timeout has passed
        do
        {
          dwCount = 1;
          hResult = ECI10E_CtrlReceive( dwCtrlHandle, &dwCount, &stcCtrlMsg, 0);
          if((ECI_OK != hResult) || (0 == dwCount))
            { OS_Sleep(1);}
        }while((ECI_OK != hResult) && ((OS_GetTimeInMs() - dwStartTime) < ECIDEMO_RX_TIMEOUT));
      }
      #else
      {
        dwCount = 1;
        hResult = ECI10E_CtrlReceive( dwCtrlHandle, &dwCount, &stcCtrlMsg, ECIDEMO_RX_TIMEOUT);
      }
      #endif //ECIDEMO_HWUSEPOLLINGMODE
      if((ECI_OK == hResult) && (dwCount > 0))
      {
        OS_Printf("\n");
        EciPrintCtrlMessage(&stcCtrlMsg);
        OS_Fflush(stdout);
      }//endif
      else
      {
        OS_Printf(".");
        OS_Fflush(stdout);
      }

      //*** Get current Time
      dwCurrentTime = OS_GetTimeInMs();
    }//end while
    OS_Printf("\n");

    //*** Reset error code
    hResult = ECI_OK;
  }//endif


  //*** Stop Controller
  if(ECI_OK == hResult)
  {
    hResult = ECI10E_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_NONE);
    ECIDEMO_CHECKERROR(ECI10E_CtrlStop);
  }

  //*** Wait some time to ensure bus idle
  OS_Sleep(250);

  //*** Reset Controller
  if(ECI_OK == hResult)
  {
    hResult = ECI10E_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_RESET_CTRL);
    ECIDEMO_CHECKERROR(ECI10E_CtrlStop);
  }

  //*** Close ECI Controller
  ECI10E_CtrlClose(dwCtrlHandle);
  dwCtrlHandle = ECI_INVALID_HANDLE;

  return hResult;
}


///////////////////////////////////////////////////////////////////////////////
/**
  ECI LIN Demo

  @param dwHwIndex
    Hardware index of board to use.

  @param dwCtrlIndex
    Controller index to use.

  @return ECI_OK on success, otherwise an error code

  @ingroup EciDemo
*/
ECI_RESULT EciLinDemo10E(DWORD dwHwIndex,
                         DWORD dwCtrlIndex)
{
  ECI_RESULT    hResult       = ECI_OK;
  ECI_CTRL_HDL  dwCtrlHandle  = ECI_INVALID_HANDLE;

  OS_Printf("\n>> ECI LIN Demo <<\n");

  //*** Open given controller of given board
  if(ECI_OK == hResult)
  {
    ECI_CTRL_CONFIG stcCtrlConfig = {0};

    //*** Set LIN Controller configuration
    stcCtrlConfig.wCtrlClass                  = ECI_CTRL_LIN;
    stcCtrlConfig.u.sLinConfig.dwVer          = ECI_STRUCT_VERSION_V0;
    stcCtrlConfig.u.sLinConfig.u.V0.wBitrate  = ECI_LIN_BITRATE_19200;
    stcCtrlConfig.u.sLinConfig.u.V0.bOpMode   = ECI_LIN_OPMODE_MASTER; //Use Master Mode to send messages

    //*** Open and Initialize given Controller of given board
    hResult = ECI10E_CtrlOpen(&dwCtrlHandle, dwHwIndex, dwCtrlIndex, &stcCtrlConfig);
    ECIDEMO_CHECKERROR(ECI10E_CtrlOpen);
  }

  //*** Get Controller Capabilities
  if(ECI_OK == hResult)
  {
    ECI_CTRL_CAPABILITIES stcCtrlCaps = {0};

    hResult = ECI10E_CtrlGetCapabilities(dwCtrlHandle, &stcCtrlCaps);
    ECIDEMO_CHECKERROR(ECI10E_CtrlGetCapabilities);
    if(ECI_OK == hResult)
      {EciPrintCtrlCapabilities(&stcCtrlCaps);}
  }

  //*** Start Controller
  if(ECI_OK == hResult)
  {
    hResult = ECI10E_CtrlStart(dwCtrlHandle);
    ECIDEMO_CHECKERROR(ECI10E_CtrlStart);
  }


  //*** Send some LIN Messages
  if(ECI_OK == hResult)
  {
    ECI_CTRL_MESSAGE stcCtrlMsg   = {0};
    DWORD            dwTxMsgCount = ECIDEMO_TX_MSGCOUNT_LIN;
    DWORD            dwIndex      = 0;

    OS_Printf("Now, sending %u LIN Messages\n", dwTxMsgCount);

    //*** Send Loop
    for(dwIndex=0; dwIndex < dwTxMsgCount; dwIndex++)
    {
      //*** Prepare LIN Message to send
      stcCtrlMsg.wCtrlClass                            = ECI_CTRL_LIN;
      stcCtrlMsg.u.sLinMessage.dwVer                   = ECI_STRUCT_VERSION_V0;
      stcCtrlMsg.u.sLinMessage.u.V0.dwMsgId            = (dwIndex % (ECI_LIN_MAX_6BIT_ID +1));
      stcCtrlMsg.u.sLinMessage.u.V0.uMsgInfo.Bits.dlc  = ((dwIndex % _countof(stcCtrlMsg.u.sLinMessage.u.V0.abData)) +1);
      memcpy( &stcCtrlMsg.u.sLinMessage.u.V0.abData[0],
              &dwIndex,
              min(4, sizeof(dwIndex)));
      memcpy( &stcCtrlMsg.u.sLinMessage.u.V0.abData[4],
              &dwIndex,
              min(4, sizeof(dwIndex)));

      //*** Send one message
      #ifdef ECIDEMO_HWUSEPOLLINGMODE
      {
        DWORD dwStartTime = OS_GetTimeInMs();

        //*** Loop until message is sent or timeout has passed
        do
        {
          hResult = ECI10E_CtrlSend( dwCtrlHandle, &stcCtrlMsg, 0);
          if(ECI_OK != hResult)
            { OS_Sleep(1);}
        }while((ECI_OK != hResult) && ((OS_GetTimeInMs() - dwStartTime) < ECIDEMO_TX_TIMEOUT));
      }
      #else
        hResult = ECI10E_CtrlSend( dwCtrlHandle, &stcCtrlMsg, ECIDEMO_TX_TIMEOUT);
      #endif //ECIDEMO_HWUSEPOLLINGMODE
      if(ECI_OK != hResult)
      {
        OS_Printf("Error while sending LIN Messages\n");
        ECIDEMO_CHECKERROR(ECI10E_CtrlSend);
        hResult = ECI_OK;
        break;
      }
    }//end for
  }//endif

  //*** Stop Controller
  if(ECI_OK == hResult)
  {
    //*** Ensure all TX message are transmitted before
    OS_Sleep(1000);

    hResult = ECI10E_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_NONE);
    ECIDEMO_CHECKERROR(ECI10E_CtrlStart);
  }

  //*** Re-initialize given controller of given board
  if(ECI_OK == hResult)
  {
    ECI_CTRL_CONFIG stcCtrlConfig = {0};

    //*** Set LIN Controller configuration
    stcCtrlConfig.wCtrlClass                  = ECI_CTRL_LIN;
    stcCtrlConfig.u.sLinConfig.dwVer          = ECI_STRUCT_VERSION_V0;
    stcCtrlConfig.u.sLinConfig.u.V0.wBitrate  = ECI_LIN_BITRATE_19200;
    stcCtrlConfig.u.sLinConfig.u.V0.bOpMode   = ECI_LIN_OPMODE_SLAVE; //Use Slave Mode to receive messages

    //*** Re-initialize given Controller of given board
    hResult = ECI10E_CtrlOpen(&dwCtrlHandle, dwHwIndex, dwCtrlIndex, &stcCtrlConfig);
    ECIDEMO_CHECKERROR(ECI10E_CtrlOpen);
  }

  //*** Start Controller
  if(ECI_OK == hResult)
  {
    hResult = ECI10E_CtrlStart(dwCtrlHandle);
    ECIDEMO_CHECKERROR(ECI10E_CtrlStart);
  }

  //*** Try to receive some LIN Messages
  if(ECI_OK == hResult)
  {
    ECI_CTRL_MESSAGE stcCtrlMsg    = {0};
    DWORD            dwStartTime   = 0;
    DWORD            dwCurrentTime = 0;
    DWORD            dwRxTimeout   = ECIDEMO_RX_TOTALTIMEOUT;
    DWORD            dwCount       = 0;

    //*** Receive Messages
    OS_Printf("Now, receiving LIN Messages for %u seconds\n", dwRxTimeout/1000);

    //*** Get current time
    dwStartTime   = OS_GetTimeInMs();
    dwCurrentTime = dwStartTime;
    hResult       = ECI_ERR_TIMEOUT;

    //*** Loop until timeout
    while(dwRxTimeout >= (dwCurrentTime - dwStartTime))
    {
      //*** Try to read Message
      #ifdef ECIDEMO_HWUSEPOLLINGMODE
      {
        DWORD dwStartTime = OS_GetTimeInMs();

        //*** Loop until message is sent or timeout has passed
        do
        {
          dwCount = 1;
          hResult = ECI10E_CtrlReceive( dwCtrlHandle, &dwCount, &stcCtrlMsg, 0);
          if((ECI_OK != hResult) || (0 == dwCount))
            { OS_Sleep(1);}
        }while((ECI_OK != hResult) && ((OS_GetTimeInMs() - dwStartTime) < ECIDEMO_RX_TIMEOUT));
      }
      #else
      {
        dwCount = 1;
        hResult = ECI10E_CtrlReceive( dwCtrlHandle, &dwCount, &stcCtrlMsg, ECIDEMO_RX_TIMEOUT);
      }
      #endif //ECIDEMO_HWUSEPOLLINGMODE
      if((ECI_OK == hResult) && (dwCount > 0))
      {
        OS_Printf("\n");
        EciPrintCtrlMessage(&stcCtrlMsg);
        OS_Fflush(stdout);
      }//endif
      else
      {
        OS_Printf(".");
        OS_Fflush(stdout);
      }

      //*** Get current Time
      dwCurrentTime = OS_GetTimeInMs();
    }//end while
    OS_Printf("\n");

    //*** Reset error code
    hResult = ECI_OK;
  }//endif


  //*** Stop Controller
  if(ECI_OK == hResult)
  {
    hResult = ECI10E_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_NONE);
    ECIDEMO_CHECKERROR(ECI10E_CtrlStop);
  }

  //*** Wait some time to ensure bus idle
  OS_Sleep(250);

  //*** Reset Controller
  if(ECI_OK == hResult)
  {
    hResult = ECI10E_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_RESET_CTRL);
    ECIDEMO_CHECKERROR(ECI10E_CtrlStop);
  }

  //*** Close ECI Controller
  ECI10E_CtrlClose(dwCtrlHandle);
  dwCtrlHandle = ECI_INVALID_HANDLE;

  return hResult;
}
