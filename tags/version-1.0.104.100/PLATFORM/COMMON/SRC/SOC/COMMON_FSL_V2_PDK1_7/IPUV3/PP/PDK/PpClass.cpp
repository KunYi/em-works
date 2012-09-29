//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  PpClass.cpp
//
//  Implementation of postprocessor driver methods
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"


#include "IpuBuffer.h"
#include "IPU_base.h"
#include "IPU_common.h"
#include "tpm.h"
#include "pp.h"
#include "PpClass.h"
#include "idmac.h"
#include "cpmem.h"
#include "dmfc.h"
#include "cm.h"



//------------------------------------------------------------------------------
// External Functions
extern void BSPSetPPISRPriority();
//extern BOOL BSPGetPPFlag();

extern PCSP_IPU_MEM_TPM   g_pIPUV3_TPM;
void TPMDumpRegs();


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define BANDMODE_ENABLE 0
#define DEBUG_DUMP 0
#define NOYUV420_OUTPUT 0 //IPUv3EX in Elvis TO2 can't output YV12 data correctly.


//------------------------------------------------------------------------------
// Global Variables

// Static variables from parent class
// must be defined before they can be used.
//PCSP_IPU_REGS IpuModuleInterfaceClass::m_pIPU = NULL;
//HANDLE IpuModuleInterfaceClass::m_hIpuMutex = NULL;
//
// Color conversion coefficient table
//
// RGB to YUV
static const UINT16 rgb2yuv_tbl[4][13] =
{
    // C00 C01   C02    C10     C11    C12    C20   C21     C22     A0       A1       A2    Scale
    {0x4D, 0x96, 0x1D, 0x1D5, 0x1AB, 0x80, 0x80, 0x195, 0x1EB, 0x00, 0x200, 0x200, 1},  // A1
    {0x42, 0x81, 0x19, 0x1DA, 0x1B6, 0x70, 0x70, 0x1A2, 0x1EE, 0x40, 0x200, 0x200, 1},  // A0
    {0x36, 0xB7, 0x12, 0x1E3, 0x19D, 0x80, 0x80, 0x18C, 0x1F4, 0x00, 0x200, 0x200, 1},  // B1
    {0x2F, 0x9D, 0x10, 0x1E6, 0x1A9, 0x70, 0x70, 0x19A, 0x1F6, 0x40, 0x200, 0x200, 1}   // B0
};



// YUV to RGB
static const UINT16 yuv2rgb_tbl[4][13] =
{
    // C00 C01   C02   C10    C11     C12     C20   C21   C22    A0         A1       A2      Scale
//    {0x95, 0x00, 0xCC, 0x95, 0x1CE, 0x198, 0x95, 0xFF, 0x00, 0x1E42, 0x10A, 0x1DD6, 2}, //A1
    {0x95, 0x00, 0xCC, 0x95, 0x1CE, 0x198, 0x95, 0xFF, 0x00, 0x1E68, 0x134, 0x1E02, 2}, //A1
    {0x4A, 0x66, 0x00, 0x4A, 0x1CC, 0x1E7, 0x4A, 0x00, 0x81, 0x1E42, 0x10F, 0x1DD6, 3}, //A0
    {0x80, 0x00, 0xCA, 0x80, 0x1E8, 0x1C4, 0x80, 0xED, 0x00, 0x1E6D, 0xA8,  0x1E25, 0}, //B1
    {0x4A, 0x73, 0x00, 0x4A, 0x1DE, 0x1F2, 0x4A, 0x00, 0x87, 0x1E10, 0x9A,  0x1DBE, 3}  //B0
};

//-----------------------------------------------------------------------------
//
// Function: PpClass
//
// Postprocessor class constructor.  Calls PpInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PpClass::PpClass(void)
{
    PpInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~PpClass
//
// The destructor for PpClass.  Calls PpDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PpClass::~PpClass(void)
{
    PpDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: PpInit
//
// This function initializes the Image Converter (postprocessor).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpInit(void)
{
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;
    
    PP_FUNCTION_ENTRY();

    // open handle to the IPU_BASE driver in order to enable IC module
    // First, create handle to IPU_BASE driver
    hIPUBase = IPUV3BaseOpenHandle();
    if (hIPUBase == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(1,
            (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    dwIPUBaseAddr = IPUV3BaseGetBaseAddr(hIPUBase);
    if (dwIPUBaseAddr == -1)
    {
        RETAILMSG (1,
            (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Map IC memory region entries
    phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_IC_REGS_OFFSET;

    // Map peripheral physical address to virtual address
    m_pIC = (PCSP_IPU_IC_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_IC_REGS), 
        FALSE); 

    // Check if virtual mapping failed
    if (m_pIC == NULL)
    {
        DEBUGMSG(ZONE_ERROR, 
            (_T("Init:  MmMapIoSpace failed!\r\n")));
        goto Error;
    }

    //Intialize the number of pixel in one burst
    // PP output channel
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB2_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    // PP input (graphic) channel
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB4_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    // PP input  channel
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB5_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);

    if (!CMRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable CM Regs!\r\n"), __WFUNCTION__));
    }
    if (!IDMACRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable IDMAC Regs!\r\n"), __WFUNCTION__));
    }
    if (!CPMEMRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable CPMEM Regs!\r\n"), __WFUNCTION__));
    }    
    if (!TPMRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable TPM Regs!\r\n"), __WFUNCTION__));
    }

    //Enable the lock feature for rotation IDMA
    IDMACChannelLock(IDMAC_CH_IRT_INPUT_PP,TRUE);
    IDMACChannelLock(IDMAC_CH_IRT_OUTPUT_PP,TRUE);

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for IPU Interrupt\r\n"), __WFUNCTION__));
    m_hPpIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PP_INTR_EVENT);
    if (m_hPpIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IPU PP Interrupt failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for PP EOF event\r\n"), __WFUNCTION__));
    // Events to signal pin that frame is ready
    m_hEOFEvent = CreateEvent(NULL, FALSE, FALSE, PP_EOF_EVENT_NAME);
    if (m_hEOFEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for PP EOF\r\n"), __WFUNCTION__));
        goto Error;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for PP EOM event\r\n"), __WFUNCTION__));
    // Events to signal pin that frame is ready
    m_hEOMEvent = CreateEvent(NULL, FALSE, FALSE, PP_EOM_EVENT_NAME);
    if (m_hEOMEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed for PP EOF\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Initialize buffer management handles

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Initializing class variables\r\n"), __WFUNCTION__));
    InitializeCriticalSection(&m_csStopping);
    InitializeCriticalSection(&m_csPpEnable);

    
    m_bConfigured = FALSE;
    m_intrType = NULL;
    m_bInputPlanar = FALSE;
    m_bOutputPlanar = FALSE;
    m_bCurrentPlanar = FALSE;
    
    m_bRunning = FALSE;
    m_hRotBuffer = NULL;

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Spawning threads\r\n"), __WFUNCTION__));

    // Initialize thread for Postprocessor ISR
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = CspiProcessQueue => thread entry point
    //      lpParameter = NULL => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
    m_hPpISRThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PpIntrThread, this, 0, NULL);

    if (m_hPpISRThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create PP ISR thread success\r\n"), __WFUNCTION__));
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Enabling postprocessor\r\n"), __WFUNCTION__));

    PP_FUNCTION_EXIT();

    return TRUE;

Error:
    PpDeinit();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PpDeinit
//
// This function deinitializes the Image Converter (Postprocessor).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpDeinit(void)
{

    PP_FUNCTION_ENTRY();

    // Disable Postprocessor
    PpDisable();

    // Unmap peripheral registers
    if (m_pIC != NULL)
    {
        MmUnmapIoSpace(m_pIC, sizeof(CSP_IPU_IC_REGS));
        m_pIC = NULL;
    }

    if (m_hPpISRThread)
    {
        CloseHandle(m_hPpISRThread);
        m_hPpISRThread = NULL;
    }

    CloseHandle(m_hPpIntrEvent);
    m_hPpIntrEvent = NULL;

    CloseHandle(m_hEOMEvent);
    m_hEOMEvent = NULL;

    CloseHandle(m_hEOMEvent);
    m_hEOMEvent = NULL;

    DeleteCriticalSection(&m_csStopping);

    DeleteCriticalSection(&m_csPpEnable);

    
}

//-----------------------------------------------------------------------------
//
// Function:  PpAddInputCombBuffer
//
// This function fills graphic plane input buffer into hardware.
//
// Parameters:
//      pBuf
//          [in] Input buffer address.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpAddInputCombBuffer(UINT32 *pBuf)
{

    PHYSICAL_ADDRESS inputPhysAddr;
    UINT16 InputDMAChannel;

    // Critical section to prevent race condition upon
    // stopping the pre-processing channel
    EnterCriticalSection(&m_csStopping);

    if (m_bRunning == FALSE)
    {
        LeaveCriticalSection(&m_csStopping);    
        return FALSE;
    }
    inputPhysAddr.QuadPart = (LONGLONG) pBuf;
 
    DEBUGMSG(ZONE_DEVICE,
        (TEXT("%s: pPhysical address (input): %x.\r\n"), __WFUNCTION__, inputPhysAddr.QuadPart));


    InputDMAChannel= IDMAC_CH_PP_INPUT_GRAPHICS;

    //If both of buffers are avaiable, we can request the second buffer 
    if((IDMACChannelBUF0IsReady(InputDMAChannel)
        &&IDMACChannelCurrentBufIsBuf1(InputDMAChannel))
       ||(IDMACChannelBUF1IsReady(InputDMAChannel)
       &&( !IDMACChannelCurrentBufIsBuf1(InputDMAChannel))))
    {
        DEBUGMSG(ZONE_ERROR,
        (TEXT("%s: Hardware is busy..\r\n"), __WFUNCTION__));
        ASSERT(FALSE);                
    }

    if (IDMACChannelCurrentBufIsBuf1(InputDMAChannel)) 
    {
        CPMEMWriteBuffer(InputDMAChannel,0, pBuf);
        IDMACChannelBUF0SetReady(InputDMAChannel);
    }
    else
    {
        CPMEMWriteBuffer(InputDMAChannel,1, pBuf);
        IDMACChannelBUF1SetReady(InputDMAChannel);
       
    }
#if 0
    //According to oskar's mail,IC task is only triggered by video plane channel.
    //If we want to update graphic plane immediately, we must set video plane ready too.
    //But this operation may interrupt video plane switching, so we recommend PPAddInputCombBuffer() 
    //should be followinged by PPAddInputBuffer() in application level. And comment 
    //following code.
    if (IDMACChannelCurrentBufIsBuf1(IDMAC_CH_PP_INPUT_VIDEO)) 
    {
        IDMACChannelBUF1SetReady(IDMAC_CH_PP_INPUT_VIDEO);
    }
    else
    {
        IDMACChannelBUF1SetReady(IDMAC_CH_PP_INPUT_VIDEO);
    }
#endif

    LeaveCriticalSection(&m_csStopping);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  PpAddInputBuffer
//
// This function fills main processing input buffer into hardware.
//
// Parameters:
//      pBuf
//          [in] Input buffer address.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpAddInputBuffer(UINT32 *pBuf)
{

    PHYSICAL_ADDRESS inputPhysAddr;
    //UINT32 tempVal;
    UINT16 InputDMAChannel;
    //UINT32 starttime;

    // Critical section to prevent race condition upon
    // stopping the pre-processing channel
    EnterCriticalSection(&m_csStopping);

    if (m_bRunning == FALSE)
    {
        LeaveCriticalSection(&m_csStopping);    
        return FALSE;
    }
    inputPhysAddr.QuadPart = (LONGLONG) pBuf;
 
    DEBUGMSG(ZONE_DEVICE,
        (TEXT("%s: pPhysical address (input): %x.\r\n"), __WFUNCTION__, inputPhysAddr.QuadPart));

    if(m_PpFirstModule == ppSourceType_PP)
    {
        InputDMAChannel= IDMAC_CH_PP_INPUT_VIDEO;
    }
    else if(m_PpFirstModule == ppSourceType_ROT)
    {
        InputDMAChannel= IDMAC_CH_IRT_INPUT_PP;
    }
    else
    {
        InputDMAChannel= IDMAC_CH_PP_INPUT_VIDEO;
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: InputDMAChannel error (%d).\r\n"), __WFUNCTION__, InputDMAChannel));            
        ASSERT(FALSE);        
    }


    //If both of buffers are avaiable, we can request the second buffer 
    if((IDMACChannelBUF0IsReady(InputDMAChannel)
        &&IDMACChannelCurrentBufIsBuf1(InputDMAChannel))
       ||(IDMACChannelBUF1IsReady(InputDMAChannel)
       &&( !IDMACChannelCurrentBufIsBuf1(InputDMAChannel))))
    {
        DEBUGMSG(ZONE_ERROR,
        (TEXT("%s: Hardware is busy..\r\n"), __WFUNCTION__));
        ASSERT(FALSE);                
    }

    if (IDMACChannelCurrentBufIsBuf1(InputDMAChannel)) 
    {
        CPMEMWriteBuffer(InputDMAChannel,0, pBuf);
        IDMACChannelBUF0SetReady(InputDMAChannel);
    }
    else
    {
        CPMEMWriteBuffer(InputDMAChannel,1, pBuf);
        IDMACChannelBUF1SetReady(InputDMAChannel);
       
    }

    LeaveCriticalSection(&m_csStopping);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  PpAddOutputBuffer
//
// This function fills main processing output buffer into hardware.
//
// Parameters:
//      pBuf
//          [in] Output buffer address.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpAddOutputBuffer(UINT32 * pBuf)
{

    PHYSICAL_ADDRESS outputPhysAddr;
    UINT16 OutputDMAChannel;
    //UINT32 tempVal;

    // Critical section to prevent race condition upon
    // stopping the pre-processing channel
    EnterCriticalSection(&m_csStopping);
    if (m_bRunning == FALSE)
    {
        LeaveCriticalSection(&m_csStopping);    
        return FALSE;
    }
    
    outputPhysAddr.QuadPart = (LONGLONG) pBuf;

    if(m_PpLastModule == ppSourceType_PP)
    {
        OutputDMAChannel= IDMAC_CH_PP_OUTPUT;
    }
    else if(m_PpLastModule == ppSourceType_ROT)
    {
        OutputDMAChannel= IDMAC_CH_IRT_OUTPUT_PP;
    }
    else
    {
        OutputDMAChannel= IDMAC_CH_PP_OUTPUT;
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: OutputDMAChannel error (%d).\r\n"), __WFUNCTION__, m_PpLastModule));            
        ASSERT(FALSE);                
    } 
      
    //If both of buffers are avaiable, we can request the second buffer 
    if((IDMACChannelBUF0IsReady(OutputDMAChannel)
        &&IDMACChannelCurrentBufIsBuf1(OutputDMAChannel))
       ||(IDMACChannelBUF1IsReady(OutputDMAChannel)
       &&( !IDMACChannelCurrentBufIsBuf1(OutputDMAChannel))))
    {
        DEBUGMSG(ZONE_ERROR,
        (TEXT("%s: Hardware is busy..\r\n"), __WFUNCTION__));
        ASSERT(FALSE);                
    }

    if (IDMACChannelCurrentBufIsBuf1(OutputDMAChannel)) 
    {
        CPMEMWriteBuffer(OutputDMAChannel,0, pBuf);
        IDMACChannelBUF0SetReady(OutputDMAChannel);
    }
    else
    {
        CPMEMWriteBuffer(OutputDMAChannel,1, pBuf);
        IDMACChannelBUF1SetReady(OutputDMAChannel);
       
    }
    LeaveCriticalSection(&m_csStopping);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpConfigure
//
// This function configures the IC registers and IDMAC
// channels for the post processor channel.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigure(pPpConfigData pConfigData)
{
    icFormat iFormat;
    UINT16 iFunctionFlag,iOutputbpp;
    //UINT32 oldVal, newVal, iMask, iBitval;

    PP_FUNCTION_ENTRY();
    iOutputbpp = IcDataWidthToBPP[pConfigData->outputIDMAChannel.DataWidth];
    iFormat = pConfigData->outputIDMAChannel.FrameFormat;
#if NOYUV420_OUTPUT
    if(iFormat == icFormat_YUV420)
    {
        ERRORMSG(1,(TEXT("IPUv3EX in Elvis TO2 can't output YV12 data correctly.\r\n")));
        return FALSE;
    }
#endif
    // If we are rotating, we want to resize the input
    // so that it matches the dimensions of the output
    // before rotation. (i.e., width in = height out
    // and height in = width out)
    if (pConfigData->FlipRot.rotate90)
    {
        m_PpOutputHeight= pConfigData->outputIDMAChannel.FrameSize.width;
        m_PpOutputWidth = pConfigData->outputIDMAChannel.FrameSize.height;
        m_PpOutputStride = pConfigData->outputIDMAChannel.FrameSize.height *iOutputbpp /8;
    }
    else
    {
        m_PpOutputWidth = pConfigData->outputIDMAChannel.FrameSize.width;
        m_PpOutputHeight = pConfigData->outputIDMAChannel.FrameSize.height;
        // we don't use the output stride setting from display driver
        m_PpOutputStride = pConfigData->outputIDMAChannel.FrameSize.width *iOutputbpp /8;;
        //m_PpOutputStride = pConfigData->outputStride;
    }

    //-----------------------------------
    //initalize parameters
    //-----------------------------------
    m_PpFirstModule = ppSourceType_ARM;
    m_PpLastModule = ppSourceType_ARM;
    m_bPPEnabled = FALSE;
    m_bIRTEnabled = FALSE;
    m_bPPCombEnabled = FALSE;
    m_bMaskEnable = FALSE;
    m_bPerPixelAlpha = FALSE;
    m_intrType = NULL;    
    memset(&m_channelParams, 0, sizeof(m_channelParams));

    //-----------------------------------
    //initalize the processing flow
    //-----------------------------------
    // the source should be mcu
    CMSetProcFlow(IPU_PROC_FLOW_PP_ROT_SRC,
                                IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);
    CMSetProcFlow(IPU_PROC_FLOW_PP_SRC,
                                IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);

    CMSetProcFlow(IPU_PROC_FLOW_PP_ROT_DEST,
                                IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
    CMSetProcFlow(IPU_PROC_FLOW_PP_DEST,
                                IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);

    IDMACChannelBUF0ClrReady(IDMAC_CH_PP_INPUT_VIDEO);
    IDMACChannelBUF0ClrReady(IDMAC_CH_PP_INPUT_GRAPHICS);
    IDMACChannelBUF0ClrReady(IDMAC_CH_PP_OUTPUT);
    IDMACChannelBUF1ClrReady(IDMAC_CH_PP_INPUT_VIDEO);
    IDMACChannelBUF1ClrReady(IDMAC_CH_PP_INPUT_GRAPHICS);
    IDMACChannelBUF1ClrReady(IDMAC_CH_PP_OUTPUT);
    IDMACChannelClrCurrentBuf(IDMAC_CH_PP_INPUT_VIDEO);
    IDMACChannelClrCurrentBuf(IDMAC_CH_PP_INPUT_GRAPHICS);
    IDMACChannelClrCurrentBuf(IDMAC_CH_PP_OUTPUT);
    IDMACChannelBUF0ClrReady(IDMAC_CH_IRT_INPUT_PP);
    IDMACChannelBUF0ClrReady(IDMAC_CH_IRT_OUTPUT_PP);
    IDMACChannelBUF1ClrReady(IDMAC_CH_IRT_INPUT_PP);
    IDMACChannelBUF1ClrReady(IDMAC_CH_IRT_OUTPUT_PP);
    IDMACChannelClrCurrentBuf(IDMAC_CH_IRT_INPUT_PP);
    IDMACChannelClrCurrentBuf(IDMAC_CH_IRT_OUTPUT_PP);

    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PP_CMB,
                IPU_IC_CONF_PP_CMB_DISABLE);       
    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PP_CSC1,
                IPU_IC_CONF_PP_CSC1_DISABLE);
    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PP_CSC2,
                IPU_IC_CONF_PP_CSC2_DISABLE);

    // If Post-processing enabled, configure
    if(pConfigData->bCombineEnable == TRUE)
    {
        //path with combine input
        //For simplify the flow, if combine is enabled, we will disable rotation, and the output is always RGB
        iFunctionFlag = 0;
        if((pConfigData->inputcombIDMAChannel.FrameSize.width != m_PpOutputWidth)
          ||(pConfigData->inputcombIDMAChannel.FrameSize.height != m_PpOutputHeight))
        {
             ERRORMSG(1,
                 (TEXT("%s: The input combine and output must be have same size for combining \r\n"), __WFUNCTION__ ));   
             return FALSE;     
        }

        if(pConfigData->FlipRot.verticalFlip
            ||pConfigData->FlipRot.horizontalFlip
            ||pConfigData->FlipRot.rotate90)
        {
             ERRORMSG(1,
                 (TEXT("%s: Rotation is not supported for combining \r\n"), __WFUNCTION__ ));   
             return FALSE;     
        }

        if((pConfigData->outputIDMAChannel.FrameFormat != icFormat_RGB) &&
              (pConfigData->outputIDMAChannel.FrameFormat != icFormat_RGBA))
        {
             ERRORMSG(1,
                 (TEXT("%s: Only RGB output format is support for combining. \r\n"), __WFUNCTION__ ));   
             return FALSE;     
        }

        if(!PpParamsCheck(pConfigData))
        {
             ERRORMSG(1,
                 (TEXT("%s: Incorrect configuration data. \r\n"), __WFUNCTION__ ));   
             return FALSE;            
        }
        
        m_bPPCombEnabled = TRUE;

        m_bMaskEnable = (pConfigData->alphaType == icAlphaType_SeparateChannel);
        m_bPerPixelAlpha = (pConfigData->alphaType == icAlphaType_Local);
        
        //Configure combine channel(graphic plane)
        if(!PpInitChannelParams(&pConfigData->inputcombIDMAChannel))
        {
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Incorrect input combine channel parameter. \r\n"), __WFUNCTION__ ));   
            return FALSE;
        }
        
        PpConfigureInput(&pConfigData->FlipRot,TRUE);
        PpConfigureCombine(pConfigData->inputcombAlpha, pConfigData->inputcombColorkey);
        
        //-----------------------------------------------------------------
        // Setup CSC parameter for combining feature, 
        // CSC1 matrix is for video plane(ch11).
        // CSC2 matrix is for both video plane and graphic plane.
        //-----------------------------------------------------------------
        BOOL bInputRGB, bInputCombRGB;
        if((pConfigData->inputIDMAChannel.FrameFormat == icFormat_RGB) ||
              (pConfigData->inputIDMAChannel.FrameFormat == icFormat_RGBA))
            bInputRGB = TRUE;
        else
            bInputRGB = FALSE;
            
        if((pConfigData->inputcombIDMAChannel.FrameFormat == icFormat_RGB) ||
              (pConfigData->inputcombIDMAChannel.FrameFormat == icFormat_RGBA))
            bInputCombRGB = TRUE;
        else
            bInputCombRGB = FALSE;
        
        if(bInputRGB != bInputCombRGB)
        {
            if(bInputRGB)
            {
                PpConfigureCSC(CSCR2Y_A1,icCSC1,NULL);
                PpConfigureCSC(CSCY2R_A1,icCSC2,NULL);
            }
            else
            {
                PpConfigureCSC(CSCY2R_A1,icCSC1,NULL);
                //CSC2 is necessary to enabled in this case.                  
            }
        }
        else if(bInputRGB == FALSE)
        {
            PpConfigureCSC(CSCNoOp,icCSC1,NULL);
            PpConfigureCSC(CSCY2R_A1,icCSC2,NULL);       
        }
        else //not csc needed, all are RGB
        {
            //errata from IPUv1 TLSbo57969, combining require enable CSC1
            PpConfigureCSC(CSCNoOp,icCSC1,NULL);
        }

        //Configure main channel(video plane)        
        if(!PpInitChannelParams(&pConfigData->inputIDMAChannel))
        {
             ERRORMSG(1,
                 (TEXT("%s: Initialize parameter failed. \r\n"), __WFUNCTION__ ));   
             return FALSE;            
        } 
        
        //resize always at first
        m_bPPEnabled = TRUE;
        if(m_PpFirstModule == ppSourceType_ARM)
        {
            m_PpFirstModule = ppSourceType_PP;
        }
        
        //-----------------------------------------------------------------
        // Setup input source
        //-----------------------------------------------------------------
        PpConfigureInput(&pConfigData->FlipRot,FALSE);

        //-----------------------------------------------------------------
        // Setup resizing
        //-----------------------------------------------------------------
        PpConfigureResize();

        //-----------------------------------------------------------------
        // Setup output destination
        //-----------------------------------------------------------------
        PpConfigureOutput(&pConfigData->outputIDMAChannel);

        //Setup the final stride
        m_channelParams.iLineStride = pConfigData->outputIDMAChannel.LineStride;
        PpIDMACChannelConfig(IDMAC_CH_PP_OUTPUT);

        //Configure mask channel
        if(m_bMaskEnable == TRUE)
        {
            PpConfigureMask();
        }
    
        m_PpLastModule = ppSourceType_PP;
      
        m_bConfigured = TRUE;
    }
    else
    {
        iFunctionFlag = 0;
        if((pConfigData->inputIDMAChannel.FrameSize.width == 
            (m_PpOutputWidth * m_PpOutputHeight 
            / pConfigData->inputIDMAChannel.FrameSize.height)))
        {
            if(pConfigData->inputIDMAChannel.FrameSize.width > m_PpOutputWidth)
                iFunctionFlag |=PP_FUNC_DOWNSIZE;
            else if(pConfigData->inputIDMAChannel.FrameSize.width < m_PpOutputWidth)
                iFunctionFlag |=PP_FUNC_UPSIZE;
            else
                iFunctionFlag |=PP_FUNC_NORESIZE;
        }
        else if(pConfigData->inputIDMAChannel.FrameSize.width > 
            (m_PpOutputWidth * m_PpOutputHeight
            / pConfigData->inputIDMAChannel.FrameSize.height))
        {
            iFunctionFlag |=PP_FUNC_DOWNSIZE;       
        }
        else
        {
            iFunctionFlag |=PP_FUNC_UPSIZE;
        }

        if(pConfigData->FlipRot.verticalFlip
            ||pConfigData->FlipRot.horizontalFlip
            ||pConfigData->FlipRot.rotate90)
        {
            iFunctionFlag |=PP_FUNC_ROTATION;
        }

        if(pConfigData->CSCEquation != CSCNoOp)
            iFunctionFlag |=PP_FUNC_CSC;

        if ((TRUE == pConfigData->allowNopPP) && (iFunctionFlag == 0))
             iFunctionFlag |=PP_FUNC_DOWNSIZE;  
        
        if(!PpParamsCheck(pConfigData))
        {
             ERRORMSG(1,
                 (TEXT("%s: Incorrect configuration data. \r\n"), __WFUNCTION__ ));   
             return FALSE;            
        }
        if(!PpInitChannelParams(&pConfigData->inputIDMAChannel))
        {
             ERRORMSG(1,
                 (TEXT("%s: Initialize parameter failed. \r\n"), __WFUNCTION__ ));   
             return FALSE;            
        }    
       
        if(iFunctionFlag & PP_FUNC_DOWNSIZE)
        {
            m_bPPEnabled = TRUE;
            if(m_PpFirstModule == ppSourceType_ARM)
            {
                m_PpFirstModule = ppSourceType_PP;
            }
            
            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            PpConfigureInput(&pConfigData->FlipRot,FALSE);

            //-----------------------------------------------------------------
            // Setup resizing
            //-----------------------------------------------------------------
            PpConfigureResize();

            //-----------------------------------------------------------------
            // Setup CSC parameter
            //-----------------------------------------------------------------
            if(iFunctionFlag & PP_FUNC_CSC)
            {
                PpConfigureCSC(pConfigData->CSCEquation, icCSC1,
                                     &pConfigData->CSCCoeffs);
            }
            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            PpConfigureOutput(&pConfigData->outputIDMAChannel);
        
            m_PpLastModule = ppSourceType_PP;
        }
        
        if(iFunctionFlag &PP_FUNC_ROTATION)
        {
            m_bIRTEnabled = TRUE;
            if(m_PpFirstModule == ppSourceType_ARM)
            {
                m_PpFirstModule = ppSourceType_ROT;
            }

            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            IRTConfigureInput(&pConfigData->FlipRot);
            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            IRTConfigureOutput(&pConfigData->FlipRot);
            
            //confgure IRT
            m_PpLastModule  = ppSourceType_ROT;
        }

        if((!(iFunctionFlag & PP_FUNC_DOWNSIZE))
            &&((iFunctionFlag & PP_FUNC_UPSIZE)
                ||(iFunctionFlag & PP_FUNC_CSC)))
        {
            m_bPPEnabled = TRUE;
            if(m_PpFirstModule == ppSourceType_ARM)
            {
                m_PpFirstModule = ppSourceType_PP;
            }
            
            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            PpConfigureInput(&pConfigData->FlipRot,FALSE);

            //-----------------------------------------------------------------
            // Setup resizing
            //-----------------------------------------------------------------
            PpConfigureResize();

            //-----------------------------------------------------------------
            // Setup CSC parameter
            //-----------------------------------------------------------------
            if(iFunctionFlag & PP_FUNC_CSC)
            {
                PpConfigureCSC(pConfigData->CSCEquation, icCSC1,
                                     &pConfigData->CSCCoeffs);
            }
            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            PpConfigureOutput(&pConfigData->outputIDMAChannel);
            
            //configure pp
            m_PpLastModule  = ppSourceType_PP;
        }

        //Setup the final stride
        m_channelParams.iLineStride = pConfigData->outputIDMAChannel.LineStride;
        if(m_PpLastModule == ppSourceType_PP)
        {
            PpIDMACChannelConfig(IDMAC_CH_PP_OUTPUT);
        }
        else if(m_PpLastModule == ppSourceType_ROT)
        {
            PpIDMACChannelConfig(IDMAC_CH_IRT_OUTPUT_PP);
        }
        else
        {
            ASSERT(FALSE);                
        } 

        m_bConfigured = TRUE;
    }
    


    if(m_PpFirstModule == ppSourceType_ARM)
    {
        RETAILMSG(1,(TEXT("ERROR:All pp module skipped, need not pp operation!")));
        return FALSE;
    }

    PP_FUNCTION_EXIT();
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpParamsCheck
//
// This function checks all configure parameters if there is unsupported setting.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if pass checking
//      FALSE if checking failed
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpParamsCheck(pPpConfigData pConfigData)
{

    // Check if pixel format is supported.
    switch (pConfigData->inputIDMAChannel.FrameFormat)
    {
        case icFormat_YUV420:
        case icFormat_YUV420P:
        case icFormat_RGB:
        case icFormat_UYVY422:      
            // only above formats are supported
            
            break;
        default:
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: unsupported pixel format.\r\n"), __WFUNCTION__));
            return FALSE;            
            break;
    }
    switch (pConfigData->outputIDMAChannel.FrameFormat)
    {
        case icFormat_YUV420:
        case icFormat_YUV420P:
        case icFormat_RGB:
        case icFormat_UYVY422:            
            // only above formats are supported
            
            break;
        default:
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: unsupported pixel format.\r\n"), __WFUNCTION__));
            return FALSE;       
            break;
    }

    // Boundary check
    if((pConfigData->inputIDMAChannel.FrameSize.width> PP_MAX_INPUT_WIDTH) ||
        (pConfigData->inputIDMAChannel.FrameSize.height > PP_MAX_INPUT_HEIGHT) ||
        (pConfigData->inputIDMAChannel.FrameSize.width  < PP_MIN_INPUT_WIDTH) ||
        (pConfigData->inputIDMAChannel.FrameSize.height < PP_MIN_INPUT_HEIGHT))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error input size: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Input Size: width (%d), height (%d)\r\n"),
                pConfigData->inputIDMAChannel.FrameSize.width, 
                pConfigData->inputIDMAChannel.FrameSize.height));
        return FALSE;
    }

    if((m_PpOutputWidth > PP_MAX_OUTPUT_WIDTH) ||
        (m_PpOutputHeight > PP_MAX_OUTPUT_HEIGHT) ||
        (m_PpOutputWidth  < PP_MIN_OUTPUT_WIDTH) ||
        (m_PpOutputHeight < PP_MIN_OUTPUT_HEIGHT))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error output size: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Output Size: width (%d), height (%d)\r\n"),
                m_PpOutputWidth, 
                m_PpOutputHeight));
        return FALSE;
    }

    // Alignment check
    if((pConfigData->inputIDMAChannel.FrameFormat== icFormat_YUV420)
        || (pConfigData->inputIDMAChannel.FrameFormat== icFormat_YUV420P))
    {
        if((pConfigData->inputIDMAChannel.FrameSize.width & 0x07) ||
            (pConfigData->inputIDMAChannel.FrameSize.height & 0x01))
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                    pConfigData->inputIDMAChannel.FrameSize.width, 
                    pConfigData->inputIDMAChannel.FrameSize.height));
            return FALSE;
        }
        if((pConfigData->inputIDMAChannel.UBufOffset & 0x07) ||
            (pConfigData->inputIDMAChannel.VBufOffset & 0x07))
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input offset, u (%d), v (%d)! \r\n"), __WFUNCTION__,
                    pConfigData->inputIDMAChannel.UBufOffset, 
                    pConfigData->inputIDMAChannel.VBufOffset));
            return FALSE;
        }

    }
    else
    {
        if(pConfigData->inputIDMAChannel.FrameSize.width & 0x03)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input size, w (%d)! \r\n"), __WFUNCTION__,
                    pConfigData->inputIDMAChannel.FrameSize.width));
            return FALSE;
        }
    }

    // Alignment check
    if((pConfigData->outputIDMAChannel.FrameFormat== icFormat_YUV420)
        || (pConfigData->outputIDMAChannel.FrameFormat== icFormat_YUV420P))
    {
        if((m_PpOutputWidth & 0x07) ||
            (m_PpOutputHeight & 0x01))
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error output size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                    m_PpOutputWidth, 
                    m_PpOutputHeight));
            return FALSE;
        }
        if((pConfigData->outputIDMAChannel.UBufOffset & 0x07) ||
            (pConfigData->outputIDMAChannel.VBufOffset & 0x07))
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error output offset, u (%d), v (%d)! \r\n"), __WFUNCTION__,
                    pConfigData->outputIDMAChannel.UBufOffset, 
                    pConfigData->outputIDMAChannel.VBufOffset));
            return FALSE;
        }

    }
    else
    {
        if(m_PpOutputWidth & 0x03)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error output size, w (%d)! \r\n"), __WFUNCTION__,
                    m_PpOutputWidth));
            return FALSE;
        }
    }

    if(pConfigData->FlipRot.rotate90)
    {
        if((pConfigData->inputIDMAChannel.FrameSize.width & 0x07) ||
            (pConfigData->inputIDMAChannel.FrameSize.height & 0x07))
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                    pConfigData->inputIDMAChannel.FrameSize.width, 
                    pConfigData->inputIDMAChannel.FrameSize.height));
            return FALSE;
        }        
        if((m_PpOutputWidth & 0x07) ||
            (m_PpOutputHeight & 0x07))
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                    m_PpOutputWidth, 
                    m_PpOutputHeight));
            return FALSE;
        }        
    }
    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: PpInitChannelParams
//
// This function initialize IDMA channel parameter structure which is used for configure IDMA 
//  channel, according to the input IDMA channel data.
//
// Parameters:
//      pIDMAChannel
//          [in] the pointer to IDMA channel structure, 
//
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpInitChannelParams(pIdmaChannel pIDMAChannel)
{
    UINT16 iInputWidth, iInputHeight;
    UINT32 iInputStride;
    icFormat iFormat;
    BOOL result = TRUE;

    if(pIDMAChannel == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error input type, IDMAchannel is NULL! \r\n"), __WFUNCTION__));   
        return FALSE;
    }

    // Configure rotation BAM parameter
    m_channelParams.iRotation90 = 0;
    m_channelParams.iFlipHoriz = 0;
    m_channelParams.iFlipVert = 0;
    m_channelParams.iBlockMode = 0;
    m_channelParams.iBandMode = CPMEM_BNDM_DISABLE;

    // Use proper configuration parameters for the
    // channel being configured
    iFormat = pIDMAChannel->FrameFormat;
    iInputWidth = pIDMAChannel->FrameSize.width;
    iInputHeight = pIDMAChannel->FrameSize.height;
    iInputStride = pIDMAChannel->LineStride;

    if ((iFormat == icFormat_YUV444) || (iFormat == icFormat_YUV422) ||
        (iFormat == icFormat_YUV420) || (iFormat == icFormat_YUV422P) ||
        (iFormat == icFormat_YUV420P))
    {
        m_bInputPlanar = TRUE;
    }
    else
    {
        m_bInputPlanar = FALSE;
    }
    m_bCurrentPlanar = m_bInputPlanar;

    // Set these variables to reduce pointer computations,
    // as these will be referenced several times.

    // The stride value used depends on whether we are
    // configuring the main input channel or the combining
    // input channel.

    //-----------------------------------------------------------------
    // Setup input format
    //-----------------------------------------------------------------
    result = PpPixelFormatSetting(iFormat, 
                                    pIDMAChannel->PixelFormat, 
                                    pIDMAChannel->DataWidth);
                                            
    if(result == FALSE)
    {return FALSE;}

    // Set channel parameters for frame height and width
    m_channelParams.iHeight = iInputHeight;
    m_channelParams.iWidth = iInputWidth;
    m_channelParams.iLineStride = iInputStride;
    if (m_bCurrentPlanar)
    {
        m_channelParams.UOffset = pIDMAChannel->UBufOffset;
        m_channelParams.VOffset = pIDMAChannel->VBufOffset;
    }   

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpIDMACChannelConfig
//
// This function configures the hardware.
//
// Parameters:
//      ch
//          [in] The IDMAC channel being queried.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpIDMACChannelConfig(UINT8 ch)
{
    CPMEMConfigData CPMEMData;

    memset(&CPMEMData, 0, sizeof(CPMEMData));
    CPMEMData.iBitsPerPixel = m_channelParams.iBitsPerPixelCode;
    CPMEMData.iDecAddrSelect = CPMEM_DEC_SEL_ADDR_0_TO_15; // Don't care, only for 4BPP mode   //not support yet
    CPMEMData.iAccessDimension = CPMEM_DIM_2D; // 1D scan is only used for csi generic data   
    CPMEMData.iScanOrder = CPMEM_SO_PROGRESSIVE; //always progressive, not support interlaced yet
    CPMEMData.iThresholdEnable = CPMEM_THE_DISABLE;
    CPMEMData.iThreshold = 0;
    CPMEMData.iCondAccessEnable = CPMEM_CAE_COND_SKIP_DISABLE;    // for future use
    CPMEMData.iCondAccessPolarity = CPMEM_CAP_COND_SKIP_LOW;  // for future use
    CPMEMData.iAlphaUsed = CPMEM_ALU_ALPHA_SAME_CHANNEL;  // seperate alpha not support yet
    CPMEMData.iAXI_Id = CPMEM_ID_0; //not used yet

    if(ch == IDMAC_CH_PP_INPUT_GRAPHICS)
    {
        if(m_bMaskEnable)
        {
            CPMEMData.iAlphaUsed = CPMEM_ALU_ALPHA_SEPARATE_CHANNEL; //FOR creating mask channel
            IDMACChannelSetSepAlpha(IDMAC_CH_PP_INPUT_GRAPHICS, TRUE);
        }
        else
        {
            IDMACChannelSetSepAlpha(IDMAC_CH_PP_INPUT_GRAPHICS, FALSE);
        }
    }
    CPMEMData.iBandMode = m_channelParams.iBandMode;
    if(CPMEMData.iBandMode != CPMEM_BNDM_DISABLE)
    {
        IDMACChannelSetBandMode(ch,IDMAC_CH_BAND_MODE_ENABLE);
    }
    else
    {
        IDMACChannelSetBandMode(ch,IDMAC_CH_BAND_MODE_DISABLE);
    }
    CPMEMData.iBlockMode = m_channelParams.iBlockMode;
    CPMEMData.iRotation90 = m_channelParams.iRotation90;
    CPMEMData.iFlipHoriz = m_channelParams.iFlipHoriz;
    CPMEMData.iFlipVert = m_channelParams.iFlipVert;
    CPMEMData.iPixelBurst = m_channelParams.iPixelBurstCode;
    switch(ch)
    {
        case IDMAC_CH_PP_OUTPUT:
            INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB2_BURST_16,
            (CPMEMData.iPixelBurst>>3));
            break;        
        case IDMAC_CH_PP_INPUT_GRAPHICS:
            INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB4_BURST_16,
            (CPMEMData.iPixelBurst>>3));
            break;
        case IDMAC_CH_PP_INPUT_VIDEO:
            INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB5_BURST_16,
            (CPMEMData.iPixelBurst>>3));
            break;
        case IDMAC_CH_IRT_OUTPUT_PP:
        case IDMAC_CH_IRT_INPUT_PP:
            // No configuration for these 2 channel's burst size
            break;
            
        default:
            //ASSERT(FALSE);
            break;
    }

    CPMEMData.iPixelFormat = m_channelParams.iFormatCode;
    CPMEMData.iHeight = m_channelParams.iHeight -1;
    CPMEMData.iWidth = m_channelParams.iWidth -1;
    CPMEMData.iLineStride = m_channelParams.iLineStride -1;
    CPMEMData.iLineStride_Y = m_channelParams.iLineStride -1;
    if((m_channelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422)
    || (m_channelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420))
    {
        CPMEMData.iLineStride_UV = ((CPMEMData.iLineStride_Y+1) >>1) -1;
    }
    else
    {
        CPMEMData.iLineStride_UV = CPMEMData.iLineStride_Y;        
    }
    memcpy(&CPMEMData.pixelFormatData, &m_channelParams.pixelFormat, sizeof(ICPixelFormatData));
    CPMEMWrite(ch, &CPMEMData, m_channelParams.bInterleaved);

#if DEBUG_DUMP
    CPMEMDumpRegs(ch);
#endif
    

}

//-----------------------------------------------------------------------------
//
// Function: PpChangeMaskPos
//
// This function Changes the IC mask channel content for combine feature.
//
// Parameters:
//      pRect:
//          [in] the active window in mask buffer
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpChangeMaskPos(RECT *pRect)
{
    INT32 width,y;
    UINT8 * pBuf;
    //The unit of Rectangle is pixel. In m_channelParams.iWidth, 1 bit is 1 pixel.
    //So the width in pixels of mask frame equals to m_channelParams.iWidth*8.
    //In following code, we compares the value in pixel.

    //Setup the mask window
    if(m_bMaskEnable == FALSE)
        return FALSE;
    
    if(pRect == NULL)
        return FALSE;
        
    if(m_hRotBuffer == NULL)
        return FALSE;
        
    pBuf = (UINT8 *)(m_hRotBuffer->VirtAddr());
    memset(pBuf,0, m_hRotBuffer->BufSize());// Set opaque

    width = pRect->right-pRect->left;
    if(pRect->left%8 != 0)
    {
        pRect->left = pRect->left &0xffffff00;
        pRect->right = pRect->left + width;
    }
    
    //boundary check
    if(pRect->right > m_channelParams.iWidth*8) 
    {
        width = (m_channelParams.iWidth*8 - pRect->left);
    }
    if(pRect->bottom > m_channelParams.iHeight)
    {
        pRect->bottom = m_channelParams.iHeight;
    }
    
    pBuf=pBuf+pRect->left/8;
    width = width/8; // 1 bit per pixel
    for( y = pRect->top; y< pRect->bottom; y++)
    {
        memset(pBuf+y*m_channelParams.iWidth,0xff,width);
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: PpConfigureMask
//
// This function initializes the IC mask channel for combine feature.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpConfigureMask()
{
    IPUBufferInfo IPUBufInfo;
    UINT8 * pBuf;

    //Create the mask buffer
    m_channelParams.iWidth = m_channelParams.iWidth/8; // 1 bit per pixel
    IPUBufInfo.dwBufferSize = m_channelParams.iWidth* m_channelParams.iHeight;
    IPUBufInfo.dwBufferSize = (IPUBufInfo.dwBufferSize+7)&0xffffff00; //align with 8
    IPUBufInfo.MemType = MEM_TYPE_VIDEOMEMORY;
    
    if(m_hRotBuffer && (m_hRotBuffer->BufSize()!=IPUBufInfo.dwBufferSize))
    {
        IPUV3DeallocateBuffer(hIPUBase, m_hRotBuffer);
        m_hRotBuffer = NULL;
    }
    
    if(m_hRotBuffer == NULL)
    {
        //get the space
        m_hRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
        IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hRotBuffer);
        //Physical address needn't CEOpenCallerBuffer().
        if(m_hRotBuffer == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Fail to get MaskBuffer ! \r\n"), __WFUNCTION__));   
            return;                  
        }

        pBuf = (UINT8 *)(m_hRotBuffer->VirtAddr());
        memset(pBuf,0,IPUBufInfo.dwBufferSize);// Set opaque
    }
    
    //adjust the channel params according to output channel
    m_channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_GENERIC;
    m_channelParams.iLineStride = m_channelParams.iWidth;
    m_channelParams.iBitsPerPixelCode = CPMEM_BPP_8;   
    m_channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_64;
    m_channelParams.pixelFormat.component0_offset = 0;
    m_channelParams.pixelFormat.component1_offset = 8;
    m_channelParams.pixelFormat.component2_offset = 16;
    m_channelParams.pixelFormat.component3_offset = 24;
    m_channelParams.pixelFormat.component0_width = 7;
    m_channelParams.pixelFormat.component1_width = 7;
    m_channelParams.pixelFormat.component2_width = 7;
    m_channelParams.pixelFormat.component3_width = 7;
    PpIDMACChannelConfig(IDMAC_CH_PP_TRANSPARENCY);
    
    CPMEMWriteBuffer(IDMAC_CH_PP_TRANSPARENCY,0, m_hRotBuffer->PhysAddr());
    return;
}
//-----------------------------------------------------------------------------
//
// Function: PpConfigureCombine
//
// This function configures the IC parameters for combine feature.
//
// Parameters:
//      alpha
//          [in] global alpha value of graphic window, 0(opaque)~255(transparent)
//
//      colorKey:
//          [in] the color key value of graphic window, the specific color will be transparent
//                0x00RRGGBB
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpConfigureCombine(UINT8 alpha, UINT32 colorkey)    
{
    //always use global alpha
    if(m_bMaskEnable || m_bPerPixelAlpha)
    {
        INSREG32BF(&m_pIC->IC_CONF, 
                    IPU_IC_CONF_IC_GLB_LOC_A, 
                    IPU_IC_CONF_IC_GLB_LOC_A_LOCAL);
    }
    else
    {
        INSREG32BF(&m_pIC->IC_CONF, 
                        IPU_IC_CONF_IC_GLB_LOC_A, 
                        IPU_IC_CONF_IC_GLB_LOC_A_GLOBAL);
    }

    if(alpha == 0xff) alpha = 0xfe; //IPU bug: if set the alpha to 0xff, some color will be reversed.
    
    INSREG32BF(&m_pIC->IC_CMBP_1, IPU_IC_CMBP_1_IC_PP_ALPHA_V, alpha);

    if (colorkey != 0xFFFFFFFF)
    {
        INSREG32BF(&m_pIC->IC_CONF, 
                    IPU_IC_CONF_IC_KEY_COLOR_EN, 
                    IPU_IC_CONF_IC_KEY_COLOR_EN_ENABLE);
        OUTREG32(&m_pIC->IC_CMBP_2, colorkey);
    }
    else
    {
        INSREG32BF(&m_pIC->IC_CONF, 
                    IPU_IC_CONF_IC_KEY_COLOR_EN, 
                    IPU_IC_CONF_IC_KEY_COLOR_EN_DISABLE);
    }

    // Now enable CMB in IC configuration register
    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PP_CMB,
        IPU_IC_CONF_PP_CMB_ENABLE);   
        
    return;
}

//-----------------------------------------------------------------------------
//
// Function: PpConfigureResize
//
// This function configures the IC parameters for resizing feature.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureResize()
{
    //UINT16 iResizeOutputHeight, iResizeOutputWidth;
    ppResizeCoeffs resizeCoeffs;
        
    // Vertical resizing
    // Get coefficients and then set registers
    if (!PpGetResizeCoeffs(m_channelParams.iHeight,
        m_PpOutputHeight, &resizeCoeffs))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Post-processing vertical resizing failed. \r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Set downsizing field
    INSREG32BF(&m_pIC->IC_PP_RSC, IPU_IC_PP_RSC_PP_DS_R_V,
        resizeCoeffs.downsizeCoeff);

    // Set resizing field
    INSREG32BF(&m_pIC->IC_PP_RSC, IPU_IC_PP_RSC_PP_RS_R_V,
        resizeCoeffs.resizeCoeff);

    INSREG32BF(&m_pIC->IC_IDMAC_2, IPU_IC_IDMAC_2_T3_FR_HEIGHT,m_PpOutputHeight-1);
    
    // Horizontal resizing
    // Get coefficients and then set registers
    if (!PpGetResizeCoeffs(m_channelParams.iWidth,
        m_PpOutputWidth, &resizeCoeffs))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Viewfinding horizontal resizing failed. \r\n"), __WFUNCTION__));
        return FALSE;
    }

    // Set downsizing field
    INSREG32BF(&m_pIC->IC_PP_RSC, IPU_IC_PP_RSC_PP_DS_R_H,
        resizeCoeffs.downsizeCoeff);

    // Set resizing field
    INSREG32BF(&m_pIC->IC_PP_RSC, IPU_IC_PP_RSC_PP_RS_R_H,
        resizeCoeffs.resizeCoeff);

    INSREG32BF(&m_pIC->IC_IDMAC_3, IPU_IC_IDMAC_3_T3_FR_WIDTH,m_PpOutputWidth-1);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpConfigureCSC
//
// This function configures the IC parameters for color space conversion feature.
//
// Parameters:
//      CSCEquation
//          [in] the type of csc equation.
//
//      CSCMatrix
//          [in] the type of csc matrix.
//
//      pCustCSCCoeffs
//          [in] the customized csc coefficients, it will be used when 
//                the type of csc equation equals to icCSCCustom. 
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpConfigureCSC(CSCEQUATION CSCEquation, icCSCMatrix CSCMatrix, pIcCSCCoeffs pCustCSCCoeffs)
{
    TPMConfigData TPMData;
    // Set up CSC for post-processing
    switch (CSCEquation)
    {
        case CSCR2Y_A1:
        case CSCR2Y_A0:
        case CSCR2Y_B0:
        case CSCR2Y_B1:
            TPMData.cscCoeffData.C00 = rgb2yuv_tbl[CSCEquation][0];
            TPMData.cscCoeffData.C01 = rgb2yuv_tbl[CSCEquation][1];
            TPMData.cscCoeffData.C02 = rgb2yuv_tbl[CSCEquation][2];
            TPMData.cscCoeffData.C10 = rgb2yuv_tbl[CSCEquation][3];
            TPMData.cscCoeffData.C11 = rgb2yuv_tbl[CSCEquation][4];
            TPMData.cscCoeffData.C12 = rgb2yuv_tbl[CSCEquation][5];
            TPMData.cscCoeffData.C20 = rgb2yuv_tbl[CSCEquation][6];
            TPMData.cscCoeffData.C21 = rgb2yuv_tbl[CSCEquation][7];
            TPMData.cscCoeffData.C22 = rgb2yuv_tbl[CSCEquation][8];
            TPMData.cscCoeffData.A0 = rgb2yuv_tbl[CSCEquation][9];
            TPMData.cscCoeffData.A1 = rgb2yuv_tbl[CSCEquation][10];
            TPMData.cscCoeffData.A2 = rgb2yuv_tbl[CSCEquation][11];
            TPMData.cscCoeffData.Scale = rgb2yuv_tbl[CSCEquation][12];
            break;

        case CSCY2R_A1:
        case CSCY2R_A0:
        case CSCY2R_B0:
        case CSCY2R_B1:
            TPMData.cscCoeffData.C00 = yuv2rgb_tbl[CSCEquation - 4][0];
            TPMData.cscCoeffData.C01 = yuv2rgb_tbl[CSCEquation - 4][1];
            TPMData.cscCoeffData.C02 = yuv2rgb_tbl[CSCEquation - 4][2];
            TPMData.cscCoeffData.C10 = yuv2rgb_tbl[CSCEquation - 4][3];
            TPMData.cscCoeffData.C11 = yuv2rgb_tbl[CSCEquation - 4][4];
            TPMData.cscCoeffData.C12 = yuv2rgb_tbl[CSCEquation - 4][5];
            TPMData.cscCoeffData.C20 = yuv2rgb_tbl[CSCEquation - 4][6];
            TPMData.cscCoeffData.C21 = yuv2rgb_tbl[CSCEquation - 4][7];
            TPMData.cscCoeffData.C22 = yuv2rgb_tbl[CSCEquation - 4][8];
            TPMData.cscCoeffData.A0 = yuv2rgb_tbl[CSCEquation - 4][9];
            TPMData.cscCoeffData.A1 = yuv2rgb_tbl[CSCEquation - 4][10];
            TPMData.cscCoeffData.A2 = yuv2rgb_tbl[CSCEquation - 4][11];
            TPMData.cscCoeffData.Scale = yuv2rgb_tbl[CSCEquation - 4][12];
            break;

        case CSCCustom:
            TPMData.cscCoeffData.C00 = pCustCSCCoeffs->C00;
            TPMData.cscCoeffData.C01 = pCustCSCCoeffs->C01;
            TPMData.cscCoeffData.C02 = pCustCSCCoeffs->C02;
            TPMData.cscCoeffData.C10 = pCustCSCCoeffs->C10;
            TPMData.cscCoeffData.C11 = pCustCSCCoeffs->C11;
            TPMData.cscCoeffData.C12 = pCustCSCCoeffs->C12;
            TPMData.cscCoeffData.C20 = pCustCSCCoeffs->C20;
            TPMData.cscCoeffData.C21 = pCustCSCCoeffs->C21;
            TPMData.cscCoeffData.C22 = pCustCSCCoeffs->C22;
            TPMData.cscCoeffData.A0 = pCustCSCCoeffs->A0;
            TPMData.cscCoeffData.A1 = pCustCSCCoeffs->A1;
            TPMData.cscCoeffData.A2 = pCustCSCCoeffs->A2;
            TPMData.cscCoeffData.Scale = pCustCSCCoeffs->Scale;
            break;
            
        case CSCNoOp:
            TPMData.cscCoeffData.C00 = 0x80;
            TPMData.cscCoeffData.C01 = 0;
            TPMData.cscCoeffData.C02 = 0;
            TPMData.cscCoeffData.C10 = 0;
            TPMData.cscCoeffData.C11 = 0x80;
            TPMData.cscCoeffData.C12 = 0;
            TPMData.cscCoeffData.C20 = 0;
            TPMData.cscCoeffData.C21 = 0;
            TPMData.cscCoeffData.C22 = 0x80;
            TPMData.cscCoeffData.A0 = 0;
            TPMData.cscCoeffData.A1 = 0;
            TPMData.cscCoeffData.A2 = 0;
            TPMData.cscCoeffData.Scale = 2;        
            break;
        default:
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Invalid PP CSC equation. \r\n"), __WFUNCTION__));
            ASSERT(FALSE);
        break;
    }

    if(CSCMatrix == icCSC1)
    {
        TPMData.tpmMatrix = TPM_CHANNEL_PP_CSC1_MATRIX1;
        TPMWrite(&TPMData);

        // Now enable CSC in IC configuration register
        INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PP_CSC1,
                    IPU_IC_CONF_PP_CSC1_ENABLE);
    }
    else
    {
        TPMData.tpmMatrix = TPM_CHANNEL_PP_CSC1_MATRIX2;
        TPMWrite(&TPMData);
        // Now enable CSC in IC configuration register
        INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PP_CSC2,
            IPU_IC_CONF_PP_CSC2_ENABLE);        
    }
    return;
}

//-----------------------------------------------------------------------------
//
// Function: PpPixelFormatSetting
//
// This function translate Pixel format setting to channel parameters.
//
// Parameters:
//      iFormat
//          [in] the type of pixel format.
//
//      PixelFormat
//          [in] the pixel format parameters.
//
//      iWidth
//          [in] the data width parameters.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpPixelFormatSetting(icFormat iFormat, icPixelFormat PixelFormat,icDataWidth iWidth)
{
    m_channelParams.iBitsPerPixelCode = CPMEM_BPP_8;   //BY DEFAULT
    m_channelParams.pixelFormat.component0_offset = PixelFormat.component0_offset;
    m_channelParams.pixelFormat.component1_offset = PixelFormat.component1_offset;
    m_channelParams.pixelFormat.component2_offset = PixelFormat.component2_offset;
    m_channelParams.pixelFormat.component3_offset = PixelFormat.component3_offset;
    m_channelParams.pixelFormat.component0_width = PixelFormat.component0_width;
    m_channelParams.pixelFormat.component1_width = PixelFormat.component1_width;
    m_channelParams.pixelFormat.component2_width = PixelFormat.component2_width;
    m_channelParams.pixelFormat.component3_width = PixelFormat.component3_width;

    m_channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_16;
    switch(iFormat)
    {
        case icFormat_YUV444:
            m_channelParams.bInterleaved = FALSE;
            m_channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV444;

            break;

        case icFormat_YUV422:
            m_channelParams.bInterleaved = FALSE;
            m_channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422;

            break;

        case icFormat_YUV420:
            m_channelParams.bInterleaved = FALSE;
            m_channelParams.iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420;

            break;

        case icFormat_YUV422P:
            m_channelParams.bInterleaved = FALSE;
            m_channelParams.iFormatCode = IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV422;

            break;

        case icFormat_YUV420P:
            m_channelParams.bInterleaved = FALSE;
            m_channelParams.iFormatCode = IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420;

            break;

        case icFormat_YUV444IL:
            m_channelParams.bInterleaved = TRUE;
            m_channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV444;
            m_channelParams.iBitsPerPixelCode = CPMEM_BPP_24;
            break;

        case icFormat_YUYV422:
            m_channelParams.bInterleaved = TRUE;
            m_channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUY2V;
            m_channelParams.iBitsPerPixelCode = CPMEM_BPP_16;
            break;
        
        case icFormat_YVYU422:
            m_channelParams.bInterleaved = TRUE;
            m_channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUY2V;
            m_channelParams.iBitsPerPixelCode = CPMEM_BPP_16;
            break;
        
        case icFormat_UYVY422:
            m_channelParams.bInterleaved = TRUE;
            m_channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_UYVY2;
            m_channelParams.iBitsPerPixelCode = CPMEM_BPP_16;
            break;
        
        case icFormat_VYUY422:
            m_channelParams.bInterleaved = TRUE;
            m_channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_UYVY2;
            m_channelParams.iBitsPerPixelCode = CPMEM_BPP_16;
            break;

        case icFormat_RGB:
            switch (iWidth)
            {
                case icDataWidth_8BPP:
                    m_channelParams.iBitsPerPixelCode = CPMEM_BPP_8;
                    break;
                
                case icDataWidth_16BPP:
                    m_channelParams.iBitsPerPixelCode = CPMEM_BPP_16;
                    break;
                
                case icDataWidth_24BPP:
                    m_channelParams.iBitsPerPixelCode = CPMEM_BPP_24;
                    break;
                
                case icDataWidth_32BPP:
                    m_channelParams.iBitsPerPixelCode = CPMEM_BPP_32;
                    break;

                default:
                    DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Error PP data width, %d !! \r\n"), __WFUNCTION__, m_channelParams.iBitsPerPixelCode));
                    return  FALSE;
            }

            m_channelParams.bInterleaved = TRUE;
            m_channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;

            break;

        case icFormat_RGBA:
            // TODO: Add support for RGBA, and find out data path and use cases for RGBA
            // 32 bits per pixel for RGB data
            m_channelParams.bInterleaved = TRUE;
            m_channelParams.iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
            m_channelParams.iBitsPerPixelCode = CPMEM_BPP_32;
            break;

        default:
            DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Invalid PP input format, %d !! \r\n"), __WFUNCTION__, iFormat));
            return  FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: IRTConfigureInput
//
// This function configures the input channel of image rotator module, meanwhile it also
// allocates the middle buffer and configures the destination of preious module.
//
// Parameters:
//      pFlipRot
//          [in] the pointer to flipping and rotating parameters.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::IRTConfigureInput(pIcFlipRot pFlipRot)
{
    BOOL result = TRUE;
    UINT16 iInputWidth, iInputHeight;
    UINT32 iInputStride;
    //UINT32 oldVal, newVal, iMask, iBitval;
    CPMEMBufOffsets OffsetData;    

    memset(&OffsetData, 0, sizeof(CPMEMBufOffsets));
    PP_FUNCTION_ENTRY();

    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T3_FLIP_UD,
        (pFlipRot->verticalFlip ? 1 : 0));
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T3_FLIP_LR,
        (pFlipRot->horizontalFlip ? 1 : 0));
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T3_ROT,
        (pFlipRot->rotate90 ? 1 : 0));

    
    m_channelParams.iRotation90 = pFlipRot->rotate90 ? 1 : 0;
    m_channelParams.iFlipHoriz = pFlipRot->horizontalFlip ? 1 : 0;
    m_channelParams.iFlipVert = pFlipRot->verticalFlip ? 1 : 0;    
    m_channelParams.iBlockMode = 1;
    m_channelParams.iBandMode = CPMEM_BNDM_DISABLE;

    //Nothing need to do
    //Use original m_channelParams directly
    iInputHeight = m_channelParams.iHeight;
    iInputWidth = m_channelParams.iWidth;
    iInputStride = m_channelParams.iLineStride;

    if(m_PpLastModule == ppSourceType_ARM)
    {
        CMSetProcFlow(IPU_PROC_FLOW_PP_ROT_SRC,
                      IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);
                   
        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;
            OffsetData.uOffset= m_channelParams.UOffset;
            OffsetData.vOffset = m_channelParams.VOffset;
            OffsetData.interlaceOffset= 0;
        }
    }
    else if(m_PpLastModule == ppSourceType_PP)
    {
        CMSetProcFlow(IPU_PROC_FLOW_PP_ROT_SRC,
                      IPU_IPU_FS_PROC_FLOW1_SRC_SEL_PP);
                                    
        CMSetProcFlow(IPU_PROC_FLOW_PP_DEST,
                      IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IRT_PLAYBACK);
#if BANDMODE_ENABLE
        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;

            OffsetData.vOffset=iInputWidth * 8;
            if (IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420 == m_channelParams.iFormatCode)
                OffsetData.uOffset = OffsetData.vOffset + OffsetData.vOffset/4;            
            else if (IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420  == m_channelParams.iFormatCode)
                OffsetData.uOffset = OffsetData.vOffset;              
            
            OffsetData.interlaceOffset= 0;
        }
        m_channelParams.iBandMode = CPMEM_BNDM_8_LINES;

        // allocate ROTATION middle buffer
        IPUBufferInfo IPUBufInfo;
        UINT32 rotAddr;
        //for caculate correct buffer size
        //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
        if((m_channelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
            || (m_channelParams.iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
            IPUBufInfo.dwBufferSize = iInputStride * 8 * 3;
        else
            IPUBufInfo.dwBufferSize = iInputStride * 8 * 2;

        IPUBufInfo.MemType = MEM_TYPE_IRAM;


        if(m_hRotBuffer)
                IPUV3DeallocateBuffer(hIPUBase, m_hRotBuffer);
        //get the space
        m_hRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
        IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hRotBuffer);

        //Physical address needn't CEOpenCallerBuffer().
        if(m_hRotBuffer == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Fail to get RotBuffer ! \r\n"), __WFUNCTION__));   
            result = FALSE;
            goto _IRTinputDone;                    
        }
        
        CPMEMWriteBandMode(IDMAC_CH_PP_OUTPUT,
                           m_channelParams.iBandMode);
        IDMACChannelSetBandMode(IDMAC_CH_PP_OUTPUT,IDMAC_CH_BAND_MODE_ENABLE);
        rotAddr = (UINT32)m_hRotBuffer->PhysAddr();
        CPMEMWriteBuffer(IDMAC_CH_PP_OUTPUT, 0, (UINT32 *)rotAddr);
        CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PP, 0, (UINT32 *)rotAddr);
        IDMACChannelBUF0SetReady(IDMAC_CH_PP_OUTPUT);

        rotAddr += IPUBufInfo.dwBufferSize/2;
        CPMEMWriteBuffer(IDMAC_CH_PP_OUTPUT, 1, (UINT32 *)rotAddr);
        CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PP, 1, (UINT32 *)rotAddr);
        IDMACChannelBUF1SetReady(IDMAC_CH_PP_OUTPUT);

        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_PP_OUTPUT,
                             &OffsetData, OffsetData.bInterleaved);
#else
        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;

            OffsetData.vOffset=iInputWidth * iInputHeight;
            if (IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420 == m_channelParams.iFormatCode)
                OffsetData.uOffset = OffsetData.vOffset + OffsetData.vOffset/4;            
            else if (IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420  == m_channelParams.iFormatCode)
                OffsetData.uOffset = OffsetData.vOffset;              
            
            OffsetData.interlaceOffset= 0;
        }

        // allocate ROTATION middle buffer
        IPUBufferInfo IPUBufInfo;
        UINT32 rotAddr;
        //for caculate correct buffer size
        //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
        if((m_channelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
            || (m_channelParams.iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
            
            IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 3;
        else
            IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 2;

        IPUBufInfo.MemType = MEM_TYPE_IRAM;


        if(m_hRotBuffer)
                IPUV3DeallocateBuffer(hIPUBase, m_hRotBuffer);
        //get the space
        m_hRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
        IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hRotBuffer);

        //Physical address needn't CEOpenCallerBuffer().
        if(m_hRotBuffer == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Fail to get RotBuffer ! \r\n"), __WFUNCTION__));   
            result = FALSE;
            goto _IRTinputDone;                    
        }

        rotAddr = (UINT32)m_hRotBuffer->PhysAddr();
        CPMEMWriteBuffer(IDMAC_CH_PP_OUTPUT, 0, (UINT32 *)rotAddr);
        CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PP, 0, (UINT32 *)rotAddr);
        IDMACChannelBUF0SetReady(IDMAC_CH_PP_OUTPUT);

        rotAddr += IPUBufInfo.dwBufferSize/2;
        CPMEMWriteBuffer(IDMAC_CH_PP_OUTPUT, 1, (UINT32 *)rotAddr);
        CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PP, 1, (UINT32 *)rotAddr);
        IDMACChannelBUF1SetReady(IDMAC_CH_PP_OUTPUT);

        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_PP_OUTPUT,
                             &OffsetData, OffsetData.bInterleaved);
#endif
    }
    else
    {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input type, m_PpLastModule: %d ! \r\n"), __WFUNCTION__,
                    m_PpLastModule));   
            result = FALSE;
            goto _IRTinputDone;
    }
    m_channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_8; // IRT MUST BE 8
    if (m_bCurrentPlanar)
        CPMEMWriteOffset(IDMAC_CH_IRT_INPUT_PP,
                         &OffsetData, OffsetData.bInterleaved);
    PpIDMACChannelConfig(IDMAC_CH_IRT_INPUT_PP);


_IRTinputDone:
    PP_FUNCTION_EXIT();
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: PpConfigureInput
//
// This function configures the IC registers and IDMAC
// channels for the postprocessor input source.
//
// Parameters:
//      pFlipRot
//          [in] the pointer to flipping and rotating parameters.
//
//      isCombine
//          [in] True if configure combine input channel.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureInput(pIcFlipRot pFlipRot, BOOL isCombine)
{
    BOOL result = TRUE;
    UINT16 iInputWidth, iInputHeight;
    UINT32 iInputStride;
    //UINT32 oldVal, newVal, iMask, iBitval;
    CPMEMBufOffsets OffsetData;
    memset(&OffsetData, 0, sizeof(CPMEMBufOffsets));

    PP_FUNCTION_ENTRY();
    
    // PP will have to handle horizontal and vertical flip
    if(m_PpFirstModule == ppSourceType_PP)
    {
        //marley doesn't support use pp to flip
        // Configure rotation BAM parameter
        if(!pFlipRot->rotate90)
        {
#if 0 
            m_channelParams.iFlipHoriz = pFlipRot->horizontalFlip ? 1 : 0;
            m_channelParams.iFlipVert = pFlipRot->verticalFlip ? 1 : 0;    
#endif
        }

    }

    m_channelParams.iBlockMode = 0;
    m_channelParams.iBandMode = CPMEM_BNDM_DISABLE;
    
    iInputHeight = m_channelParams.iHeight;
    iInputWidth = m_channelParams.iWidth;
    iInputStride = m_channelParams.iLineStride;
    
    // Configure the DMA channel, either the Main or Combining channel
    // TODO:
    // Configure Channel 5 (Mem->IC for Post-processing)
    //PpIDMACChannelConfig(PP_MEM_TO_IC_DMA_CHANNEL, &m_channelParams);

    if(m_PpLastModule == ppSourceType_ARM)
    {
        CMSetProcFlow(IPU_PROC_FLOW_PP_SRC,
                      IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);
                   
        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;
            OffsetData.uOffset= m_channelParams.UOffset;
            OffsetData.vOffset = m_channelParams.VOffset;
            OffsetData.interlaceOffset= 0;
        }
                    
    }
    else if(m_PpLastModule == ppSourceType_ROT)
    {
        CMSetProcFlow(IPU_PROC_FLOW_PP_SRC,
                      IPU_IPU_FS_PROC_FLOW1_SRC_SEL_ROT_FOR_PP);
                                    
        CMSetProcFlow(IPU_PROC_FLOW_PP_ROT_DEST,
                      IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IC_PLAYBACK_PP);
#if BANDMODE_ENABLE
        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;

            OffsetData.vOffset=iInputWidth * 8;
            if (IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420 == m_channelParams.iFormatCode)
                OffsetData.uOffset = OffsetData.vOffset + OffsetData.vOffset/4;            
            else if (IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420  == m_channelParams.iFormatCode)
                OffsetData.uOffset = OffsetData.vOffset;              
            
            OffsetData.interlaceOffset= 0;
        }            
        m_channelParams.iBandMode = CPMEM_BNDM_8_LINES;

        // allocate ROTATION middle buffer
        IPUBufferInfo IPUBufInfo;
        UINT32 rotAddr;
        //for caculate correct buffer size
        //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
        if((m_channelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
            || (m_channelParams.iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
            
            IPUBufInfo.dwBufferSize = iInputStride * 8 * 3;
        else
            IPUBufInfo.dwBufferSize = iInputStride * 8 * 2;

        IPUBufInfo.MemType = MEM_TYPE_IRAM;

        if(m_hRotBuffer)
            IPUV3DeallocateBuffer(hIPUBase, m_hRotBuffer);

        //get the space
        m_hRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
        IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hRotBuffer);

        //Physical address needn't CEOpenCallerBuffer().
        if(m_hRotBuffer == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Fail to get RotBuffer ! \r\n"), __WFUNCTION__));    
            result = FALSE;
            goto _PPinputDone;                 
        }

        CPMEMWriteBandMode(IDMAC_CH_IRT_OUTPUT_PP,
                           m_channelParams.iBandMode);
        IDMACChannelSetBandMode(IDMAC_CH_IRT_OUTPUT_PP,IDMAC_CH_BAND_MODE_ENABLE);
        rotAddr = (UINT32)m_hRotBuffer->PhysAddr();
        CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PP, 0, (UINT32 *)rotAddr);
        CPMEMWriteBuffer(IDMAC_CH_PP_INPUT_VIDEO, 0, (UINT32 *)rotAddr);
        IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PP);

        rotAddr += IPUBufInfo.dwBufferSize/2;
        CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PP, 1, (UINT32 *)rotAddr);
        CPMEMWriteBuffer(IDMAC_CH_PP_INPUT_VIDEO, 1, (UINT32 *)rotAddr);
        IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PP);

        if (m_bCurrentPlanar) 
           CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PP,
                            &OffsetData, OffsetData.bInterleaved);
#else
        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;

            OffsetData.vOffset=iInputWidth * iInputHeight;
            if (IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420 == m_channelParams.iFormatCode)
                OffsetData.uOffset = OffsetData.vOffset + OffsetData.vOffset/4;            
            else if (IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420  == m_channelParams.iFormatCode)
                OffsetData.uOffset = OffsetData.vOffset;              

            OffsetData.interlaceOffset= 0;
        }

        // allocate ROTATION middle buffer
        IPUBufferInfo IPUBufInfo;
        UINT32 rotAddr;
        //for caculate correct buffer size
        //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
        if((m_channelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
            || (m_channelParams.iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
            
            IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 3;
        else
            IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 2;

        IPUBufInfo.MemType = MEM_TYPE_IRAM;

        if(m_hRotBuffer)
            IPUV3DeallocateBuffer(hIPUBase, m_hRotBuffer);

        //get the space
        m_hRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
        IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hRotBuffer);

        //Physical address needn't CEOpenCallerBuffer().
        if(m_hRotBuffer == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Fail to get RotBuffer ! \r\n"), __WFUNCTION__));    
            result = FALSE;
            goto _PPinputDone;                 
        }

        //Physical address needn't CEOpenCallerBuffer().
        rotAddr = (UINT32)m_hRotBuffer->PhysAddr();
        CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PP, 0, (UINT32 *)rotAddr);
        CPMEMWriteBuffer(IDMAC_CH_PP_INPUT_VIDEO, 0, (UINT32 *)rotAddr);
        IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PP);

        rotAddr += IPUBufInfo.dwBufferSize/2;
        CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PP, 1, (UINT32 *)rotAddr);
        CPMEMWriteBuffer(IDMAC_CH_PP_INPUT_VIDEO, 1, (UINT32 *)rotAddr);
        IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PP);

        if (m_bCurrentPlanar) 
           CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PP,
                            &OffsetData, OffsetData.bInterleaved);
#endif
    }
    else
    {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input type, m_PpLastModule: %d ! \r\n"), __WFUNCTION__,
                    m_PpLastModule));   
            result = FALSE;
            goto _PPinputDone;
    }

    if((m_channelParams.iWidth%16)!=0)
        m_channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_8; 
    else
        m_channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_16;
        
    if(isCombine)
    {
        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_PP_INPUT_GRAPHICS,
                             &OffsetData, OffsetData.bInterleaved);
                             
        if(m_bMaskEnable)
            m_channelParams.pixelFormat.component3_width=0; // 1bit alpha data 
            
         PpIDMACChannelConfig(IDMAC_CH_PP_INPUT_GRAPHICS);

    }
    else
    {
        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_PP_INPUT_VIDEO,
                             &OffsetData, OffsetData.bInterleaved);
                             
        PpIDMACChannelConfig(IDMAC_CH_PP_INPUT_VIDEO);
    }

_PPinputDone:
    PP_FUNCTION_EXIT();
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: IRTConfigureInput
//
// This function configures the output channel of image rotator module, 
// and set the default destination IRT.
//
// Parameters:
//      pFlipRot
//          [in] the pointer to flipping and rotating parameters.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PpClass::IRTConfigureOutput(pIcFlipRot pFlipRot)
{
    UINT16 tempWidth;
    //UINT32 oldVal, newVal, iMask, iBitval;
    CPMEMBufOffsets OffsetData;
    memset(&OffsetData,0,sizeof(CPMEMBufOffsets));
    if (pFlipRot->rotate90)
    {

        m_channelParams.iLineStride = m_channelParams.iHeight * IcBitPerPixelToBPP[m_channelParams.iBitsPerPixelCode]/8;
        tempWidth = m_channelParams.iWidth;
        m_channelParams.iWidth = m_channelParams.iHeight;
        m_channelParams.iHeight = tempWidth;

        m_PpOutputStride = m_PpOutputHeight * m_PpOutputStride / m_PpOutputWidth;
        tempWidth = m_PpOutputHeight;
        m_PpOutputHeight= m_PpOutputWidth;
        m_PpOutputWidth = tempWidth;
    }

    m_channelParams.iRotation90 = 0;
    m_channelParams.iFlipHoriz = 0;
    m_channelParams.iFlipVert = 0;
    m_channelParams.iBandMode = CPMEM_BNDM_DISABLE;
    
#if NOYUV420_OUTPUT
    //For TO2, the output of IRT can't be YV12, otherwise the picture will have some line stripe.
    BOOL result = TRUE;
    icPixelFormat       PixelFormat;   // Input frame RGB format, set NULL 
    icDataWidth         DataWidth;     // Bits per pixel for RGB format
    if(m_channelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
    {
        
        //force convert YUV420 to UYVY422;
        m_bCurrentPlanar = FALSE;
        DataWidth = icDataWidth_16BPP;
        PixelFormat.component0_offset = 0;
        PixelFormat.component1_offset = 8;
        PixelFormat.component2_offset = 16;
        PixelFormat.component3_offset = 24;
        PixelFormat.component0_width = 7;
        PixelFormat.component1_width = 7;
        PixelFormat.component2_width = 7;
        PixelFormat.component3_width = 7;
       
        result = PpPixelFormatSetting(icFormat_UYVY422, 
                                        PixelFormat,
                                        DataWidth);
        if(result == FALSE)
        {return FALSE;}
        m_PpOutputStride = m_PpOutputWidth * IcDataWidthToBPP[DataWidth]/8;     
        m_channelParams.iLineStride = m_PpOutputStride;
        m_channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_8; // IRT MUST BE 8
    }
#endif

    if (m_bCurrentPlanar)
    {
        OffsetData.bInterleaved = FALSE;

        OffsetData.vOffset=m_channelParams.iWidth* m_channelParams.iHeight;
        if (IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420 == m_channelParams.iFormatCode)
            OffsetData.uOffset = OffsetData.vOffset + OffsetData.vOffset/4;            
        else if (IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420  == m_channelParams.iFormatCode)
            OffsetData.uOffset = OffsetData.vOffset;              
        
        OffsetData.interlaceOffset= 0;
    }

    CMSetProcFlow(IPU_PROC_FLOW_PP_ROT_DEST,
                  IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
    if (m_bCurrentPlanar)                            
        CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PP,
                                    &OffsetData, OffsetData.bInterleaved);
    PpIDMACChannelConfig(IDMAC_CH_IRT_OUTPUT_PP);

    
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: PpConfigureOutput
//
// This function configures the IC registers and IDMAC
// channels for the post-processor output.
//
// Parameters:
//      pIDMAChannel
//          [in] the pointer to IDMA channel structure.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpConfigureOutput(pIdmaChannel pIDMAChannel)
{
    BOOL result = TRUE;
    UINT16 iOutputWidth, iOutputHeight;
    UINT32 iOutputStride;
    //UINT32 oldVal, newVal, iMask, iBitval;
    icFormat iFormat;
    CPMEMBufOffsets OffsetData;
    
    PP_FUNCTION_ENTRY();
    memset(&OffsetData,0,sizeof(CPMEMBufOffsets));
    // Set these variables to reduce pointer computations,
    // as these will be referenced several times.

    iOutputWidth = m_PpOutputWidth;
    iOutputHeight = m_PpOutputHeight;
    iOutputStride = m_PpOutputStride;

    //m_channelParams.iFlipHoriz = 0;
    //m_channelParams.iFlipVert = 0;    
    m_channelParams.iBandMode = CPMEM_BNDM_DISABLE;

    iFormat = pIDMAChannel->FrameFormat;
    if ((iFormat == icFormat_YUV444) || (iFormat == icFormat_YUV422) ||
        (iFormat == icFormat_YUV420) || (iFormat == icFormat_YUV422P) ||
        (iFormat == icFormat_YUV420P))
    {
        m_bCurrentPlanar = TRUE;
    }
    else
    {
        m_bCurrentPlanar = FALSE;
    }

    //-----------------------------------------------------------------
    // Setup output format
    //-----------------------------------------------------------------
    result = PpPixelFormatSetting(iFormat, 
                                                pIDMAChannel->PixelFormat, 
                                                pIDMAChannel->DataWidth);
    if(result == FALSE)
    {
        goto _PPoutputDone;
    }

    iOutputStride = iOutputWidth * IcDataWidthToBPP[pIDMAChannel->DataWidth]/8;
    m_PpOutputStride = iOutputStride;
    m_channelParams.iHeight = iOutputHeight;
    m_channelParams.iWidth = iOutputWidth;
    m_channelParams.iLineStride = iOutputStride;

    if (m_bCurrentPlanar)
    {
        OffsetData.bInterleaved = FALSE;

        OffsetData.vOffset=m_channelParams.iWidth* m_channelParams.iHeight;
        if (IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420 == m_channelParams.iFormatCode)
            OffsetData.uOffset = OffsetData.vOffset + OffsetData.vOffset/4;            
        else if (IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420  == m_channelParams.iFormatCode)
            OffsetData.uOffset = OffsetData.vOffset;              
        
        OffsetData.interlaceOffset= 0;
    }



    //-----------------------------------------------------------------
    // Image size validity check
    // Setup post-processing channel output image size
    //-----------------------------------------------------------------

    if((iOutputWidth  < PP_MIN_OUTPUT_WIDTH) ||
        (iOutputHeight < PP_MIN_OUTPUT_HEIGHT) ||
        (iOutputWidth  > PP_MAX_OUTPUT_WIDTH) ||
        (iOutputHeight > PP_MAX_OUTPUT_HEIGHT) )
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error post-processing channel size: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Prp Output Channel Size: width (%d), height (%d)\r\n"),
                iOutputWidth, iOutputHeight));
        result = FALSE;
        goto _PPoutputDone;
    }
    CMSetProcFlow(IPU_PROC_FLOW_PP_DEST,
                  IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
    if((m_channelParams.iWidth%16)!=0)
        m_channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_8; 
    else
        m_channelParams.iPixelBurstCode = CPMEM_PIXEL_BURST_16;
    if (m_bCurrentPlanar)                                              
        CPMEMWriteOffset(IDMAC_CH_PP_OUTPUT, &OffsetData, OffsetData.bInterleaved);
    
    PpIDMACChannelConfig(IDMAC_CH_PP_OUTPUT);


_PPoutputDone:
    PP_FUNCTION_EXIT();
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: PpStartChannel
//
// This function starts the postprocessing channel.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpStartChannel()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    PP_FUNCTION_ENTRY();

    // PP must have been configured at least once 
    // in order to start the channel
    if (!m_bConfigured)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error.  Cannot start post-processing channel without first configuring\r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    if (m_bRunning)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Viewfinding channel already running.\r\n"), __WFUNCTION__));
        return TRUE;
    }

    // Reset EOF event, in case stale EOF events were triggered, 
    // which would cause Pin to believe a frame was ready.

    if (m_bIRTEnabled)
    {
        PpEnable();
        IRTEnable(IPU_DRIVER_PP);
        
        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_IRT_INPUT_PP,TRUE);
        IDMACChannelDBMODE(IDMAC_CH_IRT_OUTPUT_PP,TRUE);
        

        // Compute bitmask and shifted bit value for IDMAC enable register.
        IDMACChannelEnable(IDMAC_CH_IRT_INPUT_PP);
        IDMACChannelEnable(IDMAC_CH_IRT_OUTPUT_PP);
        
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PP_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PP_ROT_EN, IPU_IC_CONF_PP_ROT_EN_ENABLE);

        // enable postprocessing for IRT path in IC.
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

    }
    if(m_bPPEnabled)
    {
        // Enable Postprocessor
        PpEnable();
    
        IDMACChannelDBMODE(IDMAC_CH_PP_INPUT_VIDEO,TRUE);
        IDMACChannelDBMODE(IDMAC_CH_PP_OUTPUT,TRUE);

        IDMACChannelEnable(IDMAC_CH_PP_INPUT_VIDEO);
        IDMACChannelEnable(IDMAC_CH_PP_OUTPUT);
        
        if(m_bPPCombEnabled)
        {
            IDMACChannelDBMODE(IDMAC_CH_PP_INPUT_GRAPHICS,TRUE);
            IDMACChannelEnable(IDMAC_CH_PP_INPUT_GRAPHICS);
        }
        
        if(m_bMaskEnable)
        {
            IDMACChannelEnable(IDMAC_CH_PP_TRANSPARENCY);
            IDMACChannelBUF0SetReady(IDMAC_CH_PP_TRANSPARENCY);
        }
        
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PP_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PP_EN, IPU_IC_CONF_PP_EN_ENABLE);

        // enable PP path in IC
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

    }
    m_bRunning = TRUE;
    PP_FUNCTION_EXIT();

#if DEBUG_DUMP
    ICDumpRegs();
#endif

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: IRTEnable
//
// Enable the Image Rotator.
//
// Parameters:
//      driver
//          [in] Identifies which IRT should be enabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::IRTEnable(IPU_DRIVER driver)
{
    //DWORD dwBytesTransferred;

    PP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPpEnable);

    if (m_bRunning)
    {
        LeaveCriticalSection(&m_csPpEnable);
        return;
    }

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Enabling IRT!\r\n"), __WFUNCTION__));

    if (!IPUV3EnableModule(hIPUBase, IPU_SUBMODULE_IRT, driver))
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to enable IRT!\r\n"), __WFUNCTION__));
        ASSERT(FALSE);        
    }

    LeaveCriticalSection(&m_csPpEnable);

    PP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PpEnable
//
// Enable the Image Converter and the IDMAC channels we will need.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpEnable(void)
{
    //DWORD dwBytesTransferred;
    IPU_DRIVER driver = IPU_DRIVER_PP;

    PP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPpEnable);

    if (m_bRunning)
    {
        LeaveCriticalSection(&m_csPpEnable);
        return;
    }

    DEBUGMSG (ZONE_INFO,
        (TEXT("%s: Enabling IC!\r\n"), __WFUNCTION__));

    if(!IPUV3EnableModule(hIPUBase, IPU_SUBMODULE_IC, driver))
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to enable IC!\r\n"), __WFUNCTION__));
        ASSERT(FALSE);                
    }

    LeaveCriticalSection(&m_csPpEnable);

    PP_FUNCTION_EXIT();
}
//-----------------------------------------------------------------------------
//
// Function: PpIsBusy
//
// This function checks pp module status
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if busy.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpIsBusy(void)
{
    BOOL ret = FALSE;
    
    PP_FUNCTION_ENTRY();
    if(CMGetProcTaskStatus(IPU_PROC_TASK_PP) == IPU_PROC_TASK_STAT_ACTIVE)
        ret = TRUE;

    PP_FUNCTION_EXIT();
    return ret;
}
//-----------------------------------------------------------------------------
//
// Function:  IRTIsBusy
//
// This function checks IRT module status
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if busy.
//
//-----------------------------------------------------------------------------
BOOL PpClass::IRTIsBusy(void)
{
    BOOL ret = FALSE;
    
    PP_FUNCTION_ENTRY();
    if(CMGetProcTaskStatus(IPU_PROC_TASK_PP_ROT) == IPU_PROC_TASK_STAT_ACTIVE)
        ret = TRUE;

    PP_FUNCTION_EXIT();
    return ret;
}


//-----------------------------------------------------------------------------
//
// Function: PpStopChannel
//
// This function halts the post-processing channels.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpStopChannel(void)
{
    //UINT32 uTempReg, uTempReg1, uTempReg2;
    UINT32 uCount = 0;
    UINT32 oldVal, newVal, iMask, iBitval;

    PP_FUNCTION_ENTRY();

    EnterCriticalSection(&m_csStopping);

    // If not running, return
    if (m_bRunning == FALSE)
    {
        LeaveCriticalSection(&m_csStopping);    
        return TRUE;
    }

    m_bRunning = FALSE;

    // Protect access to IC_CONF register.
    // Disable IC tasks.  Tasks will stop on completion
    // of the current frame.

    // Compute bitmask and shifted bit value for IC CONF register
    if (m_bIRTEnabled)
    {
        uCount = 0;
        while(IDMACChannelIsBusy(IDMAC_CH_IRT_INPUT_PP)
            ||IDMACChannelIsBusy(IDMAC_CH_IRT_OUTPUT_PP)
            ||IRTIsBusy())
        {
            if (uCount <= 1000)
            {
                //..give up the remainder of time slice
                Sleep(1);
                uCount++;
            }
            else
            {
                //.. there is something wrong ....break out
                RETAILMSG(1,
                    (TEXT("%s(): Error in stopping IRT PP channel!\r\n"),__WFUNCTION__));
                break;
            }    
        }
            // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PP_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PP_ROT_EN, IPU_IC_CONF_PP_ROT_EN_DISABLE);

        // enable pre-processing vf path in IC
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

        IDMACChannelDisable(IDMAC_CH_IRT_INPUT_PP);
        IDMACChannelDisable(IDMAC_CH_IRT_OUTPUT_PP);
            
        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_IRT_INPUT_PP,FALSE);
        IDMACChannelDBMODE(IDMAC_CH_IRT_OUTPUT_PP,FALSE);

        IRTDisable(IPU_DRIVER_PP);
    }
    if(m_bPPEnabled)
    {
        uCount = 0;
        while(IDMACChannelIsBusy(IDMAC_CH_PP_INPUT_VIDEO)
            ||IDMACChannelIsBusy(IDMAC_CH_PP_OUTPUT)
            ||PpIsBusy())
        {
            if (uCount <= 1000)
            {
                //..give up the remainder of time slice
                Sleep(1);
                uCount++;
            }
            else
            {
                //.. there is something wrong ....break out
                RETAILMSG(1,
                    (TEXT("%s(): Error in stopping PP channel!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PP_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PP_EN, IPU_IC_CONF_PP_EN_DISABLE);

        // enable pre-processing vf path in IC
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

        IDMACChannelDisable(IDMAC_CH_PP_INPUT_VIDEO);
        IDMACChannelDisable(IDMAC_CH_PP_OUTPUT);
            
        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_PP_INPUT_VIDEO,FALSE);
        IDMACChannelDBMODE(IDMAC_CH_PP_OUTPUT,FALSE);

        if(m_bPPCombEnabled)
        {
            IDMACChannelDBMODE(IDMAC_CH_PP_INPUT_GRAPHICS,FALSE);
            IDMACChannelDisable(IDMAC_CH_PP_INPUT_GRAPHICS);
        }
        
        if(m_bMaskEnable)
        {
            IDMACChannelDisable(IDMAC_CH_PP_TRANSPARENCY);
        }
        
        PpDisable();
    }


    PP_FUNCTION_EXIT();

    LeaveCriticalSection(&m_csStopping);    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpDisable
//
// Disable the Image Converter.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpDisable(void)
{
    //DWORD dwBytesTransferred;
    IPU_DRIVER driver = IPU_DRIVER_PP;

    PP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPpEnable);

    if (m_bRunning)
    {
        LeaveCriticalSection(&m_csPpEnable);
        return;
    }

    DEBUGMSG (ZONE_INFO,
        (TEXT("%s: Disabling IC!\r\n"), __WFUNCTION__));

    if(!IPUV3DisableModule(hIPUBase, IPU_SUBMODULE_IC, driver))
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to disable IC!\r\n"), __WFUNCTION__));
        ASSERT(FALSE);                
    }


    LeaveCriticalSection(&m_csPpEnable);

    PP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: IRTDisable
//
// Enable the Image Converter.
//
// Parameters:
//      driver
//          [in] Identifies which IRT should be enabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::IRTDisable(IPU_DRIVER driver)
{
    //DWORD dwBytesTransferred;

    PP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPpEnable);

    if (m_bRunning)
    {
        LeaveCriticalSection(&m_csPpEnable);
        return;
    }

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Disabling IRT!\r\n"), __WFUNCTION__));

    if(!IPUV3DisableModule(hIPUBase, IPU_SUBMODULE_IRT, driver))
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to disable IRT!\r\n"), __WFUNCTION__));
        ASSERT(FALSE);                
    }


    LeaveCriticalSection(&m_csPpEnable);

    PP_FUNCTION_EXIT();
}
//-----------------------------------------------------------------------------
//
// Function: PpGetResizeCoeffs
//
// This function computes the resizing coefficients from
// the input and output size.
//
// Parameters:
//      inSize
//          [in] Input size (height or width)
//
//      outSize
//          [in] Output size (height of width)
//
//      resizeCoeffs
//          [out] downsizing and resizing coefficients computed.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpGetResizeCoeffs(UINT16 inSize, UINT16 outSize, pPpResizeCoeffs resizeCoeffs)
{
    UINT16 tempSize;
    UINT16 tempDownsize;
    UINT16 tempOutSize;

    PP_FUNCTION_ENTRY();

    // Cannot downsize more than 8:1
    if ((outSize << 3) < inSize)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Maximum downsize ratio is 8:1.  Input Size specified: %d.  Output Size specified: %d. \r\n"),
            __WFUNCTION__, inSize, outSize));
        return FALSE;
    }

    // compute downsizing coefficient
    tempDownsize = 0;
    tempSize = inSize;
    tempOutSize = outSize << 1;
    //tempsize must be smaller than outSize x 2, the rest part can be handled by resize module.
    while ((tempSize >= tempOutSize) && (tempDownsize < 2))
    {
        tempSize >>= 1;
        tempDownsize++;
    }
    //The main processing section can't accept the resolution larger than 1024
    while((tempSize > 1024)&&(tempDownsize < 2))
    {
        tempSize >>= 1;
        tempDownsize++;
    }
    resizeCoeffs->downsizeCoeff = tempDownsize;

    // compute resizing coefficient using the following equation:
    //      resizeCoeff = M*(SI -1)/(SO - 1)
    //      where M = 2^13, SI - input size, SO - output size
    resizeCoeffs->resizeCoeff =  8192 * (tempSize - 1) / (outSize - 1);

    if(resizeCoeffs->resizeCoeff > 0x3fff)
        resizeCoeffs->resizeCoeff =0x3fff;

    PP_FUNCTION_EXIT();

    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: PpClearBuffers
//
// This function is for deleting all middle buffers.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PpClass::PpClearBuffers()
{
    if(m_hRotBuffer)
    {
            IPUV3DeallocateBuffer(hIPUBase,m_hRotBuffer);
            m_hRotBuffer = NULL;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PpIntrEnable
//
// This function is for enable or disable pp interrupt.
//
// Parameters:
//      IntType
//          [in] determine which interrupt should be enabled, 
//               only FIRSTMODULE_INTERRUPT available yet
//               FIRSTMODULE_INTERRUPT: first module of the whole chain
//               FRAME_INTERRUPT:           last module of the whole chain
//
//      enable
//          [in] TRUE if enable, FALSE if disable 
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpIntrEnable(UINT8 IntType, BOOL enable)
{
    if (TRUE == enable)
    {
        m_intrType |= IntType;    
    }else
    {
        m_intrType &= ~IntType;    
    }
    
    if(IntType & FIRSTMODULE_INTERRUPT)
    {
        if(m_PpFirstModule == ppSourceType_ROT)
        {
             IDMACChannelClearIntStatus(IDMAC_CH_IRT_INPUT_PP, IPU_INTR_TYPE_EOF);
             IDMACChannelIntCntrl(IDMAC_CH_IRT_INPUT_PP, IPU_INTR_TYPE_EOF, enable);
        }
        else if(m_PpFirstModule == ppSourceType_PP)
        {
                IDMACChannelClearIntStatus(IDMAC_CH_PP_INPUT_VIDEO, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_PP_INPUT_VIDEO, IPU_INTR_TYPE_EOF, enable);
        }        
        else
        {
            return;
        }
    }
    if(IntType & FRAME_INTERRUPT)
    {
        if(m_PpLastModule == ppSourceType_ROT)
        {
             IDMACChannelClearIntStatus(IDMAC_CH_IRT_OUTPUT_PP, IPU_INTR_TYPE_EOF);
             IDMACChannelIntCntrl(IDMAC_CH_IRT_OUTPUT_PP, IPU_INTR_TYPE_EOF, enable);
        }
        else if(m_PpLastModule == ppSourceType_PP)
        {
                IDMACChannelClearIntStatus(IDMAC_CH_PP_OUTPUT, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_PP_OUTPUT, IPU_INTR_TYPE_EOF, enable);
        }        
        else
        {
            return;
        }
    }
    return;
}

//-----------------------------------------------------------------------------
//
// Function: PpWaitForNotBusy
//
// This function is for blocking the process, it won't return until receive the interrupt.
//
// Parameters:
//      IntType
//          [in] determine which interrupt should be enabled, 
//               only FIRSTMODULE_INTERRUPT available yet
//               FIRSTMODULE_INTERRUPT: first module of the whole chain
//               FRAME_INTERRUPT:           last module of the whole chain
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PpClass::PpWaitForNotBusy(UINT8 IntType)
{
    UINT32 timeout = 1000;

    if(IntType & FIRSTMODULE_INTERRUPT)
    {
        if (WaitForSingleObject(m_hEOMEvent, timeout) == WAIT_TIMEOUT)
        {
            RETAILMSG(1,
                     (TEXT("%s(): Waiting for FIRSTMODULE_INTERRUPT interrupt time out!\r\n"), __WFUNCTION__));
        }
        if(m_PpFirstModule == ppSourceType_PP)
        {
            IDMACChannelClearIntStatus(IDMAC_CH_PP_INPUT_VIDEO, IPU_INTR_TYPE_EOF);
            IDMACChannelIntCntrl(IDMAC_CH_PP_INPUT_VIDEO, IPU_INTR_TYPE_EOF, FALSE);  
        }
        else if(m_PpFirstModule == ppSourceType_ROT)
        {
             IDMACChannelClearIntStatus(IDMAC_CH_IRT_INPUT_PP, IPU_INTR_TYPE_EOF);
             IDMACChannelIntCntrl(IDMAC_CH_IRT_INPUT_PP, IPU_INTR_TYPE_EOF, FALSE);          
        }
    }
    if(IntType & FRAME_INTERRUPT)
    {
        if (WaitForSingleObject(m_hEOFEvent, timeout) == WAIT_TIMEOUT)
        {
            RETAILMSG(1,
                     (TEXT("%s(): Waiting for PP FRAME_INTERRUPT time out!\r\n"), __WFUNCTION__));
        }
        if(m_PpLastModule == ppSourceType_ROT)
        {
             IDMACChannelClearIntStatus(IDMAC_CH_IRT_OUTPUT_PP, IPU_INTR_TYPE_EOF);
             IDMACChannelIntCntrl(IDMAC_CH_IRT_OUTPUT_PP, IPU_INTR_TYPE_EOF, FALSE);
        }
        else if(m_PpLastModule == ppSourceType_PP)
        {
             IDMACChannelClearIntStatus(IDMAC_CH_PP_OUTPUT, IPU_INTR_TYPE_EOF);
             IDMACChannelIntCntrl(IDMAC_CH_PP_OUTPUT, IPU_INTR_TYPE_EOF, FALSE);
        }    
    }
    return;  
}

//------------------------------------------------------------------------------
//
// Function: PpIntrThread
//
// This function is the IST thread.
//
// Parameters:
//      lpParameter
//          [in] PP handler
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PpClass::PpIntrThread(LPVOID lpParameter)
{
    PpClass *pPp = (PpClass *)lpParameter;

    PP_FUNCTION_ENTRY();

    BSPSetPPISRPriority();

    pPp->PpISRLoop(INFINITE);

    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PpISRLoop
//
// This function is the interrupt handler for the Postprocessor.
// It waits for the PP interrupt, and signals
// the EOF(End of frame) or EOM(End of first module) event registered 
// by the user of the postprocessor.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PpClass::PpISRLoop(UINT32 timeout)
{
    for(;;)
    {
        if (WaitForSingleObject(m_hPpIntrEvent, timeout) == WAIT_OBJECT_0)
        {
            EnterCriticalSection(&m_csStopping);
            if(!m_bRunning)
            {
                LeaveCriticalSection(&m_csStopping);
                continue;
            }
            if((IDMACChannelGetIntStatus(IDMAC_CH_PP_INPUT_VIDEO, IPU_INTR_TYPE_EOF))
                && (FIRSTMODULE_INTERRUPT & m_intrType))
            {
                IDMACChannelClearIntStatus(IDMAC_CH_PP_INPUT_VIDEO, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_PP_INPUT_VIDEO, IPU_INTR_TYPE_EOF, TRUE);  
                SetEvent(m_hEOMEvent);
            }
            else if((IDMACChannelGetIntStatus(IDMAC_CH_IRT_INPUT_PP, IPU_INTR_TYPE_EOF))
                && (FIRSTMODULE_INTERRUPT & m_intrType))
            {
                IDMACChannelClearIntStatus(IDMAC_CH_IRT_INPUT_PP, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_IRT_INPUT_PP, IPU_INTR_TYPE_EOF, TRUE);  
                SetEvent(m_hEOMEvent);
            }
            
            if(IDMACChannelGetIntStatus(IDMAC_CH_PP_OUTPUT, IPU_INTR_TYPE_EOF))
            {
                IDMACChannelClearIntStatus(IDMAC_CH_PP_OUTPUT, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_PP_OUTPUT, IPU_INTR_TYPE_EOF, TRUE);  
                SetEvent(m_hEOFEvent);
            }
            else if(IDMACChannelGetIntStatus(IDMAC_CH_IRT_OUTPUT_PP, IPU_INTR_TYPE_EOF))
            {
                IDMACChannelClearIntStatus(IDMAC_CH_IRT_OUTPUT_PP, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_IRT_OUTPUT_PP, IPU_INTR_TYPE_EOF, TRUE);  
                SetEvent(m_hEOFEvent);
            }
            else
            {
                DEBUGMSG(1,(_T("%s: Error interrupt received!\r\n"), __WFUNCTION__));
                ASSERT(FALSE);
            }
        
            LeaveCriticalSection(&m_csStopping);
        }
    }
}



void PpClass::ICDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&m_pIC->IC_CONF;

    RETAILMSG (1, (TEXT("\n\nIC Registers\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_IC_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}                

void TPMDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&g_pIPUV3_TPM->pp_csc1_matrix1;

    RETAILMSG (1, (TEXT("\n\nTPM-pp_csc1_matrix1 Registers\n")));
    for (i = 0; i <= (sizeof(TPM_ENTRY) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}                


