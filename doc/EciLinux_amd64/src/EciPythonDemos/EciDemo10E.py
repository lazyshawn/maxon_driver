#
# ECI API Demo for iPC-I XC16 / PCIe
#
# converted from IXXAT example by Michael Hengler (mhengler@ixxat.de)
# 
#

from ECI10E import *  # @UnusedWildImport
from EciDemoCommon import *  # @UnusedWildImport
import time
import struct
import sys

    
#//////////////////////////////////////////////////////////////////////////
#// static constants, types, macros, variables

# ECI Demo send timeout in [s]
ECIDEMO_TX_TIMEOUT = DWORD(500)

# ECI Demo TX message count for CAN
ECIDEMO_TX_MSGCOUNT_CAN = 0x800 * 10

# ECI Demo TX message count for LIN
ECIDEMO_TX_MSGCOUNT_LIN = 0x800 * 1

# ECI Demo receive timeout in [s]
ECIDEMO_RX_TIMEOUT = DWORD(500)

# ECI Demo total receive timeout in [s]
ECIDEMO_RX_TOTALTIMEOUT = 30.0

# If defined ECI Demo will run in polling mode instead of using an
# event triggered mode. This mode can be used, if it is not possible to
# assign an unique IRQ to the device to use. The device driver than
# will not use an IRQ based message reception and transmission.
ECIDEMO_HWUSEPOLLINGMODE = False

"""
  ECI Demo for iPC-I XC16 / PCIe

  @return ECI_OK on success, otherwise an error code

  @ingroup EciDemo
"""
class EciDemo10E:
    def __init__(self):
        self.eci = ECI10E()
        
        

    def _chkStatus(self, hResult, sFuncName):
        if ECI_OK == hResult:
            print ("%s ...succeeded." % sFuncName )
        else:
            print ("%s ...failed with error code: 0x%08X. %s" %
                    (sFuncName,
                     hResult,
                     self.eci.ECIDRV_GetErrorString(hResult))
                     )
    
    def RunDemo(self):
        hResult     = ECI_OK  # @UnusedVariable
        stcHwPara   = ECI_HW_PARA() # 0
        stcHwInfo   = ECI_HW_INFO() # 0
        dwHwIndex   = DWORD(0)
        dwCtrlIndex = DWORD(0)
        print ("\n>> ECI Demo for iPC-I XC16 / PCIe <<\n")
        
        # Prepare Hardware parameter structure
        stcHwPara.wHardwareClass = ECI_HW_PCI
        if ECIDEMO_HWUSEPOLLINGMODE:
            stcHwPara.dwFlags = ECI_SETTINGS_FLAG_POLLING_MODE
        
        # At first call Initialize to prepare ECI driver
        # with one board
        hResult = self.eci.ECIDRV_Initialize(1, stcHwPara)
        self._chkStatus(hResult, "ECIDRV_Initialize")
        
        # Retrieve hardware info
        if ECI_OK == hResult:
            # Retrieve hardware info
            hResult = self.eci.ECIDRV_GetInfo(dwHwIndex, stcHwInfo)
            self._chkStatus(hResult, "ECIDRV_GetInfo")
            if ECI_OK == hResult:
                EciPrintHwInfo(stcHwInfo)
        
        # Find first CAN Controller of Board
        if ECI_OK == hResult: 
            dwCtrlIndex = EciGetNthCtrlOfClass(stcHwInfo,
                                               ECI_CTRL_CAN,
                                               0, # first relative controller
                                               dwCtrlIndex)
            if ECI_OK == hResult:
                # Start CAN Demo
                hResult = self.EciCanDemo(dwHwIndex, dwCtrlIndex)
                self._chkStatus(hResult, "EciCanDemo")
            else:
                # Ignore if no controller was found
                hResult = ECI_OK
        
        # Find first LIN Controller of Board
        if ECI_OK == hResult:
            dwCtrlIndex = EciGetNthCtrlOfClass(stcHwInfo,
                                               ECI_CTRL_LIN,
                                               0, # first relative controller
                                               dwCtrlIndex)
            if ECI_OK == hResult:
                # Start LIN Demo
                hResult = self.EciLinDemo(dwHwIndex, dwCtrlIndex)
                self._chkStatus(hResult, "EciLinDemo")
            else:
                # Ignore if no controller was found
                hResult = ECI_OK
        
        # Clean up ECI driver
        self.eci.ECIDRV_Release()
        
        print ("-> Returning from ECI Demo for iPC-I XC16 / PCIe <-\n")
        
        return hResult

    """
    ECI CAN Demo
    
    @param dwHwIndex
      Hardware index of board to use.
    
    @param dwCtrlIndex
      Controller index to use.
    
    @return ECI_OK on success, otherwise an error code
    
    @ingroup EciDemo
    """
    def EciCanDemo(self, dwHwIndex, dwCtrlIndex):
        hResult       = ECI_OK
        dwCtrlHandle  = ECI_INVALID_HANDLE
    
        print ("\n>> ECI CAN Demo <<\n")
    
        # Open given controller of given board
        if ECI_OK == hResult:
            stcCtrlConfig = ECI_CTRL_CONFIG()
    
            # Set CAN Controller configuration
            stcCtrlConfig.wCtrlClass                = WORD(ECI_CTRL_CAN)
            stcCtrlConfig.u.sCanConfig.dwVer        = DWORD(ECI_STRUCT_VERSION_V0)
            stcCtrlConfig.u.sCanConfig.u.V0.bBtReg0 = BYTE(ECI_CAN_BT0_1000KB)
            stcCtrlConfig.u.sCanConfig.u.V0.bBtReg1 = BYTE(ECI_CAN_BT1_1000KB)
            stcCtrlConfig.u.sCanConfig.u.V0.bOpMode = BYTE(ECI_CAN_OPMODE_STANDARD | ECI_CAN_OPMODE_EXTENDED | ECI_CAN_OPMODE_ERRFRAME)
    
            # Open and Initialize given Controller of given board
            hResult = self.eci.ECIDRV_CtrlOpen(dwCtrlHandle, dwHwIndex, dwCtrlIndex, stcCtrlConfig)
            self._chkStatus(hResult, "ECIDRV_CtrlOpen")
    
        # Get Controller Capabilites
        if ECI_OK == hResult:
            stcCtrlCaps = ECI_CTRL_CAPABILITIES() # 0
    
            hResult = self.eci.ECIDRV_CtrlGetCapabilities(dwCtrlHandle, stcCtrlCaps)
            self._chkStatus(hResult, "ECIDRV_CtrlGetCapabilities")
    
            if ECI_OK == hResult:
                EciPrintCtrlCapabilities(stcCtrlCaps)
    
        # Start Controller
        if ECI_OK == hResult:
            hResult = self.eci.ECIDRV_CtrlStart(dwCtrlHandle)
            self._chkStatus(hResult, "ECIDRV_CtrlStart")
    
        # Send some CAN Messages
        if ECI_OK == hResult:
            stcCtrlMsg   = ECI_CTRL_MESSAGE()
            dwTxMsgCount = ECIDEMO_TX_MSGCOUNT_CAN
            dwIndex      = 0
    
            print("Now, sending %u CAN Messages" % dwTxMsgCount)
    
            # Send Loop
            for dwIndex in range(0, dwTxMsgCount):
                # Prepare CAN Message to send
                stcCtrlMsg.wCtrlClass                            = ECI_CTRL_CAN
                stcCtrlMsg.u.sCanMessage.dwVer                   = ECI_STRUCT_VERSION_V0
                stcCtrlMsg.u.sCanMessage.u.V0.dwMsgId            = (dwIndex % (ECI_CAN_MAX_11BIT_ID + 1))
                stcCtrlMsg.u.sCanMessage.u.V0.uMsgInfo.Bits.dlc  = (dwIndex % (len(stcCtrlMsg.u.sCanMessage.u.V0.abData) + 1))

                # TODO probably very ineficient. How to do it natively?
                a = []
                a[0:] = struct.pack("<I", dwIndex)
                for i in range(0,4):
                    stcCtrlMsg.u.sCanMessage.u.V0.abData[i] = ord(a[i])
                    stcCtrlMsg.u.sCanMessage.u.V0.abData[i+4] = ord(a[i])

                """
                memcpy( &stcCtrlMsg.u.sCanMessage.u.V0.abData[0],
                        &dwIndex,
                        min(4, sizeof(dwIndex)))
                memcpy( &stcCtrlMsg.u.sCanMessage.u.V0.abData[4],
                        &dwIndex,
                        min(4, sizeof(dwIndex)))
                """
    
                # Send one message
                if ECIDEMO_HWUSEPOLLINGMODE:
                    dwStartTime = time.time()
    
                    # Loop until message is sent or timeout has passed
                    while True:
                        hResult = self.eci.ECIDRV_CtrlSend( dwCtrlHandle, stcCtrlMsg, 0)
                        if ECI_OK != hResult:
                            time.sleep(0.001)
                        if (ECI_OK != hResult and ((time.time() - dwStartTime) < ECIDEMO_TX_TIMEOUT)):
                            break
                else:
                    hResult = self.eci.ECIDRV_CtrlSend( dwCtrlHandle, stcCtrlMsg, ECIDEMO_TX_TIMEOUT)
          
                if ECI_OK != hResult:
                    print("Error while sending CAN Messages")
                    self._chkStatus(hResult, "ECIDRV_CtrlSend")
    
                    hResult = ECI_OK
                    break
                else:
                    # Read out all received messages
                    astcPyCtrlMsg = ECI_CTRL_MESSAGE * 20 # 0
                    astcCtrlMsg = astcPyCtrlMsg()
                    dwCount     = DWORD(len(astcCtrlMsg))
                    dwMsgIndex  = 0
    
                    # Try to read some messages
                    hResult = self.eci.ECIDRV_CtrlReceive( dwCtrlHandle, dwCount, astcCtrlMsg, 0)

                    # Loop through all received messages
                    dwMsgIndex = 0
                    while (ECI_OK == hResult) and (dwCount.value > dwMsgIndex):
                        EciPrintCtrlMessage(astcCtrlMsg[dwMsgIndex])
    
                        # Proceed with next message
                        dwMsgIndex += 1
                
                    # Reset Error code and proceed with transmission
                    hResult = ECI_OK
    
        # Try to receive some CAN Messages
        if ECI_OK == hResult:
            stcCtrlMsg    = ECI_CTRL_MESSAGE()
            dwStartTime   = 0
            dwCurrentTime = 0
            dwRxTimeout   = ECIDEMO_RX_TOTALTIMEOUT
            dwCount       = DWORD(0)
    
            # Receive Messages
            print("Now, receiving CAN Messages for %u seconds" % int(dwRxTimeout))
        
            # Get current time
            dwStartTime = time.time()
            dwCurrentTime = dwStartTime
            hResult       = ECI_ERR_TIMEOUT
        
            # Loop until timeout
            while (dwRxTimeout >= (dwCurrentTime - dwStartTime)):
                # Try to read Message
                if ECIDEMO_HWUSEPOLLINGMODE:
                    dwStartTime = time.time()
                
                    # Loop until message is received or timeout has passed
                    while True:
                        dwCount = DWORD(1)
                        hResult = self.eci.ECIDRV_CtrlReceive( dwCtrlHandle, dwCount, stcCtrlMsg, 0)
                        if((ECI_OK != hResult) or (0 == dwCount)):
                            time.time(0.001)
                        if (ECI_OK != hResult and (time.time() - dwStartTime) < ECIDEMO_RX_TIMEOUT):
                            break
    
                else:
                    dwCount = DWORD(1)
                    hResult = self.eci.ECIDRV_CtrlReceive( dwCtrlHandle, dwCount, stcCtrlMsg, ECIDEMO_RX_TIMEOUT)

    
                if ECI_OK == hResult and dwCount > 0:
                    print("")
                    EciPrintCtrlMessage(stcCtrlMsg, sameLine = True)
                else:
                    sys.stdout.write(".")
                    sys.stdout.flush()
                
                # Get current Time
                dwCurrentTime = time.time()
            print("")
    
            # Reset error code
            hResult = ECI_OK
    
        # Stop Controller
        if ECI_OK == hResult:
            print("")
            hResult = self.eci.ECIDRV_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_NONE)
            self._chkStatus(hResult, "ECIDRV_CtrlStop")
    
        # Wait some time to ensure bus idle
        time.sleep(0.250)
    
        # Reset Controller
        if ECI_OK == hResult:
            hResult = self.eci.ECIDRV_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_RESET_CTRL)
            self._chkStatus(hResult, "ECIDRV_CtrlStop")
    
        # Close ECI Controller
        self.eci.ECIDRV_CtrlClose(dwCtrlHandle)
        dwCtrlHandle = ECI_INVALID_HANDLE
    
        return hResult
    
    
    """
    ECI LIN Demo
    
    @param dwHwIndex
        Hardware index of board to use.
    
    @param dwCtrlIndex
        Controller index to use.
    
    @return ECI_OK on success, otherwise an error code
    
    @ingroup EciDemo
    """
    def EciLinDemo(self, dwHwIndex, dwCtrlIndex):
    
        hResult      = ECI_OK
        dwCtrlHandle = ECI_INVALID_HANDLE
    
        print("\n>> ECI LIN Demo <<\n")
    
        # Open given controller of given board
        if ECI_OK == hResult:
            stcCtrlConfig = ECI_CTRL_CONFIG()
    
            # Set LIN Controller configuration
            stcCtrlConfig.wCtrlClass                 = ECI_CTRL_LIN
            stcCtrlConfig.u.sLinConfig.dwVer         = ECI_STRUCT_VERSION_V0
            stcCtrlConfig.u.sLinConfig.u.V0.wBitrate = ECI_LIN_BITRATE_19200
            stcCtrlConfig.u.sLinConfig.u.V0.bOpMode  = ECI_LIN_OPMODE_MASTER # //Use Master Mode to send messages
    
            # Open and Initialize given Controller of given board
            hResult = self.eci.ECIDRV_CtrlOpen(dwCtrlHandle, dwHwIndex, dwCtrlIndex, stcCtrlConfig)
            self._chkStatus(hResult, "ECIDRV_CtrlOpen")
    
        # Get Controller Capabilites
        if ECI_OK == hResult:
            stcCtrlCaps = ECI_CTRL_CAPABILITIES()
    
            hResult = self.eci.ECIDRV_CtrlGetCapabilities(dwCtrlHandle, stcCtrlCaps)
            self._chkStatus(hResult, "ECIDRV_CtrlGetCapabilities")
            if ECI_OK == hResult:
                EciPrintCtrlCapabilities(stcCtrlCaps)
    
        # Start Controller
        if ECI_OK == hResult:
        
            hResult = self.eci.ECIDRV_CtrlStart(dwCtrlHandle)
            self._chkStatus(hResult, "ECIDRV_CtrlStart")
    
        # Send some LIN Messages
        if ECI_OK == hResult:
        
            stcCtrlMsg   = ECI_CTRL_MESSAGE()
            dwTxMsgCount = ECIDEMO_TX_MSGCOUNT_LIN
            dwIndex      = 0
    
            print("Now, sending %u LIN Messages" % dwTxMsgCount)
    
            # Send Loop
            for dwIndex in range (0, dwTxMsgCount):
                # Prepare LIN Message to send
                stcCtrlMsg.wCtrlClass                           = ECI_CTRL_LIN
                stcCtrlMsg.u.sLinMessage.dwVer                  = ECI_STRUCT_VERSION_V0
                stcCtrlMsg.u.sLinMessage.u.V0.dwMsgId           = (dwIndex % (ECI_LIN_MAX_6BIT_ID +1))
                stcCtrlMsg.u.sLinMessage.u.V0.uMsgInfo.Bits.dlc = ((dwIndex % len(stcCtrlMsg.u.sLinMessage.u.V0.abData)) +1)

                # TODO probably very ineficient. How to do it natively?
                a = []
                a[0:] = struct.pack("<I", dwIndex)
                for i in range(0,4):
                    stcCtrlMsg.u.sLinMessage.u.V0.abData[i] = ord(a[i])
                    stcCtrlMsg.u.sLinMessage.u.V0.abData[i+4] = ord(a[i])

                """
                memcpy( &stcCtrlMsg.u.sLinMessage.u.V0.abData[0],
                                &dwIndex,
                                min(4, sizeof(dwIndex)))
                memcpy( &stcCtrlMsg.u.sLinMessage.u.V0.abData[4],
                                &dwIndex,
                                min(4, sizeof(dwIndex)))
                """
                # Send one message
                if ECIDEMO_HWUSEPOLLINGMODE:
                    dwStartTime = time.time()
    
                    # Loop until message is sent or timeout has passed
                    while True:
                        hResult = self.eci.ECIDRV_CtrlSend( dwCtrlHandle, stcCtrlMsg, 0)
                        if ECI_OK != hResult:
                            time.time(0.001)
                        if (ECI_OK != hResult and (time.time() - dwStartTime) < ECIDEMO_TX_TIMEOUT):
                            break
                else:
                    hResult = self.eci.ECIDRV_CtrlSend( dwCtrlHandle, stcCtrlMsg, ECIDEMO_TX_TIMEOUT)
    
                if ECI_OK != hResult:
                    print("Error while sending LIN Messages")
                    self._chkStatus(hResult, "ECIDRV_CtrlSend")
                    hResult = ECI_OK
                    break
    
        # Stop Controller
        if ECI_OK == hResult:
            # Ensure all TX message are transmitted before
            time.sleep(1.0)
    
            hResult = self.eci.ECIDRV_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_NONE)
            self._chkStatus(hResult, "ECIDRV_CtrlStop")
    
        # Re-initialize given controller of given board
        if ECI_OK == hResult:
            stcCtrlConfig = ECI_CTRL_CONFIG()
    
            # Set LIN Controller configuration
            stcCtrlConfig.wCtrlClass                 = ECI_CTRL_LIN
            stcCtrlConfig.u.sLinConfig.dwVer         = ECI_STRUCT_VERSION_V0
            stcCtrlConfig.u.sLinConfig.u.V0.wBitrate = ECI_LIN_BITRATE_19200
            stcCtrlConfig.u.sLinConfig.u.V0.bOpMode  = ECI_LIN_OPMODE_SLAVE # //Use Slave Mode to receive messages
    
            # Re-initialize given Controller of given board
            hResult = self.eci.ECIDRV_CtrlOpen(dwCtrlHandle, dwHwIndex, dwCtrlIndex, stcCtrlConfig)
            self._chkStatus(hResult, "ECIDRV_CtrlOpen")
    
        # Start Controller
        if ECI_OK == hResult:
            hResult = self.eci.ECIDRV_CtrlStart(dwCtrlHandle)
            self._chkStatus(hResult, "ECIDRV_CtrlStart")
    
        # Prepare LIN message reception
        if ECI_OK == hResult:
            stcCtrlMsg = ECI_CTRL_MESSAGE()
            dwIndex    = 0
    
            print("Now, preparing LIN Messages for reception in slave mode")
    
            # RX Message preparation loop
            for dwIndex in range(0, ECI_LIN_MAX_6BIT_ID +1):
                # *************************************************************************
                # The CAN-IB2x0/PCIe interfaces use a hardware LIN controller for
                # receiving and transmitting LIN messages. This controller needs to
                # know the message length and the CRC type prior to receiving the message.
                # To configure LIN Message reception the LIN buffers are used.
                # If another IXXAT interface is used, and the message buffer are prepared
                # for reception the LIN controller on the interface board ignores these
                # settings. So configuring is harmless for other interfaces but necessary
                # for CAN-IB2x0/PCIe interfaces.
                # *************************************************************************
    
                # Prepare LIN Message to receive
                stcCtrlMsg.wCtrlClass                           = ECI_CTRL_LIN
                stcCtrlMsg.u.sLinMessage.dwVer                  = ECI_STRUCT_VERSION_V0
                stcCtrlMsg.u.sLinMessage.u.V0.dwMsgId           = (dwIndex % (ECI_LIN_MAX_6BIT_ID +1))
                stcCtrlMsg.u.sLinMessage.u.V0.uMsgInfo.Bits.dlc = 8 #//Expect message with data length 8
                stcCtrlMsg.u.sLinMessage.u.V0.uMsgInfo.Bits.buf = 1 #//Update LIN Buffer
                stcCtrlMsg.u.sLinMessage.u.V0.uMsgInfo.Bits.sor = 0 #//Do not sent message
                stcCtrlMsg.u.sLinMessage.u.V0.uMsgInfo.Bits.ecs = 1 #//Expect enhanced checksum (LIN Spec. 2.0)
    
                # Prepare RX message using send function
                if ECIDEMO_HWUSEPOLLINGMODE:
                    dwStartTime = time.time()
    
                    # Loop until message is sent or timeout has passed
                    while True:
                        hResult = self.eci.ECIDRV_CtrlSend( dwCtrlHandle, stcCtrlMsg, 0)
                        if ECI_OK != hResult:
                            time.time(0.001)
                        if (ECI_OK != hResult and (time.time() - dwStartTime) < ECIDEMO_TX_TIMEOUT):
                            break
                else:
                    hResult = self.eci.ECIDRV_CtrlSend( dwCtrlHandle, stcCtrlMsg, ECIDEMO_TX_TIMEOUT)
    
                if ECI_OK != hResult:
                    print("Error while preparing LIN Messages for reception")
                    self._chkStatus(hResult, "ECIDRV_CtrlSend")
                    hResult = ECI_OK
                    break
    
        # Try to receive some LIN Messages
        if ECI_OK == hResult:
            stcCtrlMsg = ECI_CTRL_MESSAGE()
            dwStartTime   = 0
            dwCurrentTime = 0
            dwRxTimeout   = ECIDEMO_RX_TOTALTIMEOUT
            dwCount       = DWORD(0)
    
            # Receive Messages
            print("Now, receiving LIN Messages for %u seconds" % int(dwRxTimeout))
    
            # Get current time
            dwStartTime   = time.time()
            dwCurrentTime = dwStartTime
            hResult       = ECI_ERR_TIMEOUT
    
            # Loop until timeout
            while (dwRxTimeout >= (dwCurrentTime - dwStartTime)):
                # Try to read Message
                if ECIDEMO_HWUSEPOLLINGMODE:
                    dwStartTime = time.time()
    
                    # Loop until message is sent or timeout has passed
                    while True:
                        dwCount = DWORD(1)
                        hResult = self.eci.ECIDRV_CtrlReceive( dwCtrlHandle, dwCount, stcCtrlMsg, 0)
                        if (ECI_OK != hResult) or (0 == dwCount):
                            time.time(0.001)
                        if ((ECI_OK != hResult) and (time.time() - dwStartTime) < ECIDEMO_RX_TIMEOUT):
                            break
                else:
                    dwCount = DWORD(1)
                    hResult = self.eci.ECIDRV_CtrlReceive( dwCtrlHandle, dwCount, stcCtrlMsg, ECIDEMO_RX_TIMEOUT)
    
                if((ECI_OK == hResult) and (dwCount > 0)):
                    print("")
                    EciPrintCtrlMessage(stcCtrlMsg, sameLine=True)
                else:
                    sys.stdout.write(".")
                    sys.stdout.flush()
    
                # Get current Time
                dwCurrentTime = time.time()
            # end while
            print("")
    
            # Reset error code
            hResult = ECI_OK
    
        # Stop Controller
        if ECI_OK == hResult:
        
            hResult = self.eci.ECIDRV_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_NONE)
            self._chkStatus(hResult, "ECIDRV_CtrlStop")
    
        # Wait some time to ensure bus idle
        time.sleep(0.250)
    
        # Reset Controller
        if ECI_OK == hResult:
        
            hResult = self.eci.ECIDRV_CtrlStop(dwCtrlHandle, ECI_STOP_FLAG_RESET_CTRL)
            self._chkStatus(hResult, "ECIDRV_CtrlStop")
    
        # Close ECI Controller
        self.eci.ECIDRV_CtrlClose(dwCtrlHandle)
        dwCtrlHandle = ECI_INVALID_HANDLE
    
        return hResult
