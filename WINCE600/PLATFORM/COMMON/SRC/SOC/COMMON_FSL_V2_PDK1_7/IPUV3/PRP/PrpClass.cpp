//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  PrpClass.cpp
//
//  Implementation of Preprocessor driver class
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
#include "dp.h"
#include "prp.h"
#include "PrpClass.h"
#include "idmac.h"
#include "cpmem.h"
#include "dmfc.h"
#include "cm.h"
#include "dc.h"

//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPDropFrameQuietly();
extern BOOL BSPDisableNOICPath();


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define DMFC_DIRECT_PATH    0
#define BANDMODE_ENABLE     0  //software workaround for IC errata MSIls35192
#define DP_NOYUV420_INPUT   1

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

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
    {0x95, 0x00, 0xCC, 0x95, 0x1CE, 0x198, 0x95, 0xFF, 0x00, 0x1E42, 0x10A, 0x1DD6, 2}, //A1
    {0x4A, 0x66, 0x00, 0x4A, 0x1CC, 0x1E7, 0x4A, 0x00, 0x81, 0x1E42, 0x10F, 0x1DD6, 3}, //A0
    {0x80, 0x00, 0xCA, 0x80, 0x1E8, 0x1C4, 0x80, 0xED, 0x00, 0x1E6D, 0xA8,  0x1E25, 0}, //B1
    {0x4A, 0x73, 0x00, 0x4A, 0x1DE, 0x1F2, 0x4A, 0x00, 0x87, 0x1E10, 0x9A,  0x1DBE, 3}  //B0
};


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: PrpClass
//
// Preprocessor class constructor.  Calls PrpInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PrpClass::PrpClass(void)
{
    PrpInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~PrpClass
//
// The destructor for PrpClass.  Calls PrpDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PrpClass::~PrpClass(void)
{
    PrpDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: PrpInit
//
// This function initializes the Image Converter (preprocessor).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpInit(void)
{
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;
    UINT32 oldVal, newVal, iMask, iBitval;
    
    PRP_FUNCTION_ENTRY();

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
    
    // Map TPM memory region entries
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

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for IPU Interrupt\r\n"), __WFUNCTION__));
    m_hPrpIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PRP_INTR_EVENT);
    if (m_hPrpIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IPU PRP Interrupt failed\r\n"), __WFUNCTION__));
        goto Error;
    }
    m_hDPFGIntrEvent= CreateEvent(NULL, FALSE, FALSE, IPU_DPFG_INTR_EVENT);
    if (m_hDPFGIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IPU DP Interrupt failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    InitializeCriticalSection(&m_csStopping);

    InitializeCriticalSection(&m_csPrpEnable);

    m_bVFPrpEnabled = FALSE;
    m_bENCPrpEnabled = FALSE;
    m_bVFIRTEnabled = FALSE;
    m_bENCIRTEnabled = FALSE;
    m_bDMFCChannelUsing = FALSE;
    m_bVFCombEnabled = FALSE;
    m_bSYNCDPEnabled = FALSE;
    m_bASYNCDPEnabled = FALSE;
    m_bSYNCDPCSCChanged = FALSE;
    m_bASYNCDPCSCChanged = FALSE;

    m_bConfigured = FALSE;

    m_hDisplay = NULL;

    m_bRunning = FALSE;

    m_hVFRotBuffer =  NULL;
    m_hENCRotBuffer =  NULL;
    m_hVFDpBuffer =  NULL;
    m_hENCDpBuffer =  NULL;

    //Intialize the number of pixel in one burst
    //IC ENC OUTPUT
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB0_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    //IC VF OUTPUT    
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB1_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    //IC VF COMBINE INPUT
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB3_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    //IC VF INPUT
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB6_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    //DIRECT DATA FROM IC(from sensor)
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB7_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    //IC VF INPUT ALT
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_ALT_CB6_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    //DIRECT DATA FROM IC ALT
    INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_ALT_CB7_BURST_16,
        IPU_IC_IDMAC_1_CB_BURST_8);
    
    IDMACChannelSetPriority(IDMAC_CH_29,IDMAC_CH_PRIORITY_HIGH);
    IDMACChannelSetPriority(IDMAC_CH_27,IDMAC_CH_PRIORITY_HIGH);

    //Enable the lock feature for rotation IDMA
    IDMACChannelLock(IDMAC_CH_IRT_INPUT_PRP_ENC,TRUE);
    IDMACChannelLock(IDMAC_CH_IRT_INPUT_PRP_VF,TRUE);
    IDMACChannelLock(IDMAC_CH_IRT_OUTPUT_PRP_ENC,TRUE);
    IDMACChannelLock(IDMAC_CH_IRT_OUTPUT_PRP_VF,TRUE);
    
    //by default
    OUTREG32(&m_pIC->IC_IDMAC_4, 0x5555);   
    //if there are 2 flows that are not balanced(one flow requries a huge amount of data relatively to the 2nd flow)
    //change 0xaaaa;   

    //only FOR MX37
    iMask = CSP_BITFMASK(IPU_IC_CONF_RWS_EN)
               |CSP_BITFMASK(IPU_IC_CONF_CSI_MEM_WR_EN) ;
    iBitval = CSP_BITFVAL(IPU_IC_CONF_RWS_EN, IPU_IC_CONF_RWS_EN_ENABLE)
                | CSP_BITFVAL(IPU_IC_CONF_CSI_MEM_WR_EN, IPU_IC_CONF_CSI_MEM_WR_EN_DISABLE);       

    // Use interlocked function to Disable IC tasks.
    do
    {
        oldVal = INREG32(&m_pIC->IC_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                oldVal, newVal) != oldVal);

    PRP_FUNCTION_EXIT();

    return TRUE;

Error:
    PrpDeinit();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpDeinit
//
// This function deinitializes the Image Converter (Preprocessor).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpDeinit(void)
{
    PRP_FUNCTION_ENTRY();

    CloseHandle(m_hPrpIntrEvent);
    m_hPrpIntrEvent = NULL;
    
    CloseHandle(m_hDPFGIntrEvent);
    m_hDPFGIntrEvent = NULL;

    if (m_hDisplay != NULL)
    {
        DeleteDC(m_hDisplay);
        m_hDisplay = NULL;
    }

    // Unmap peripheral registers
    if (m_pIC != NULL)
    {
        MmUnmapIoSpace(m_pIC, sizeof(CSP_IPU_IC_REGS));
        m_pIC = NULL;
    }
    
    IPUV3BaseCloseHandle(hIPUBase);

    DeleteCriticalSection(&m_csStopping);

    DeleteCriticalSection(&m_csPrpEnable);


    PRP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function:  PrpAddInputBuffer
//
// This function fill main processing buffer into hardware.
//
// Parameters:
//      pBuf
//          [in] Input buffer address.
//
//      bWait
//          [in]   TRUE:  The function won't return until the first module finished processing.
//                  FALSE:The function returns immediately once the buffer is filled.
//
//      IntType
//          [in] determine which interrupt should be enabled, 
//               only FIRSTMODULE_INTERRUPT available yet
//               FIRSTMODULE_INTERRUPT: first module of the whole chain
//               FRAME_INTERRUPT:           last module of the whole chain
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpAddInputBuffer(UINT32 *pBuf, BOOL bWait, UINT8 IntType)
{

    PHYSICAL_ADDRESS inputPhysAddr;
    UINT16 InputDMAChannel;
    UINT32 starttime;
    BOOL bDropFrameQuietly; 
    UINT32 dwOutputBuf;
    UINT8 EBAnum;
    CPMEMBufOffsets OffsetData;
    OffsetData.bInterleaved = FALSE;
    OffsetData.interlaceOffset= 0;
    
    // Critical section to prevent race condition upon
    // stopping the pre-processing channel
    EnterCriticalSection(&m_csStopping);

    bDropFrameQuietly = BSPDropFrameQuietly();

    if (m_bRunning == FALSE)
    {
        LeaveCriticalSection(&m_csStopping);    
        return FALSE;
    }
    inputPhysAddr.QuadPart = (LONGLONG) pBuf;

    DEBUGMSG(ZONE_DEVICE,
        (TEXT("%s: pPhysical address (input): %x.\r\n"),
         __WFUNCTION__, inputPhysAddr.QuadPart));

    if(m_PrpFirstModule == prpSourceType_PRP)
    {
        InputDMAChannel= IDMAC_CH_PRP_INPUT_VIDEO;
    }
    else if(m_PrpFirstModule == prpSourceType_ROT)
    {
        if(m_bVFIRTEnabled)
            InputDMAChannel= IDMAC_CH_IRT_INPUT_PRP_VF;
        else
            InputDMAChannel= IDMAC_CH_IRT_INPUT_PRP_ENC;
    }
    else if(m_PrpFirstModule == prpSourceType_ARM)
    {
        if(m_bSYNCDPEnabled)
        {
            InputDMAChannel= IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE;
        }
        else
        {
            InputDMAChannel= IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE;
        }
    }
    else
    {
        InputDMAChannel= IDMAC_CH_PRP_INPUT_VIDEO;
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: InputDMAChannel error (%d).\r\n"), __WFUNCTION__, InputDMAChannel));            
    }
    if(m_bENCOversizedFrame)
    {
        //oversized frame processing is always blocked.
        if((m_PrpFirstModule == prpSourceType_PRP)
            &&(m_PrpENCLastModule == prpSourceType_PRP))
        {
            starttime = GetTickCount();
            //Determine which EBA we will use, according to current using EBA
            EBAnum = IDMACChannelCurrentBufIsBuf1(IDMAC_CH_PRP_INPUT_VIDEO) ? 1:0;
            //Get the base address of IC output buffer we will use 
            //from the DP input buffer setting. 
            if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
            {
                //use DP buffer 0
                dwOutputBuf=CPMEMReadBufferAddr(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE,0);
            }
            else
            {
                //use DP buffer 1
                dwOutputBuf=CPMEMReadBufferAddr(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE,1);
            }

            //Confirm the previous IDMAC operation is finished.
            while(IDMACChannelIsBusy(IDMAC_CH_PRP_OUTPUT_ENC))
            {
                Sleep(1);
            }

            //-----------------------------------------------
            //        processing half frame in the left                      
            //-----------------------------------------------
            //Start interrupt
            PrpIntrEnable(FIRSTMODULE_INTERRUPT);            
            //If v offset is not zero and u,v offset is different  it means YV12 format is used, 
            //offset parameters need to be changed for left and right half frame.
            //For left half frame, the offset is the orignal value.
            //Offset parameters are in CPMEM.
            //It can not be changed while corresponding IDMAC is enabled, so 
            //the IDMAC must be disabled durring setting. 
            //NV12 doesn't need this operation
            IDMACChannelDisable(IDMAC_CH_PRP_INPUT_VIDEO);
            if (m_ENCchannelParams.VOffset&&(m_ENCchannelParams.VOffset!=m_ENCchannelParams.UOffset))
            {
                
                OffsetData.uOffset= m_ENCchannelParams.UOffset;
                OffsetData.vOffset = m_ENCchannelParams.VOffset;
                CPMEMWriteOffset(IDMAC_CH_PRP_INPUT_VIDEO,
                                 &OffsetData, OffsetData.bInterleaved);
                
            }
            //Reset the x scrolling value.
            if(m_OversizedInputOffsetStride%8)
            {
                CPMEMWriteXScroll(IDMAC_CH_PRP_INPUT_VIDEO, 0);
            }
            IDMACChannelEnable(IDMAC_CH_PRP_INPUT_VIDEO);
            //Start to fill half buffer in the left side
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, EBAnum, (UINT32 *)dwOutputBuf);
            CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, EBAnum, pBuf);
            
            //Set the right ready flag according to EBA value.
            if(EBAnum)
            {
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_ENC);
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_INPUT_VIDEO);
            }
            else
            {
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_ENC);
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_INPUT_VIDEO);
            }
            //wait for processing finish
            PrpWaitForNotBusy(FIRSTMODULE_INTERRUPT);

            //-----------------------------------------------
            //        processing half frame in the right                     
            //-----------------------------------------------
            PrpIntrEnable(FIRSTMODULE_INTERRUPT);
            
            //If v offset is not zero and u,v offset is different  it means YV12 format is used, 
            //offset parameters need to be changed for left and right half frame.
            //For right half frame, the offset need to be adjusted as the start point address changed.
            //Offset parameters are in CPMEM.
            //It can not be changed while corresponding IDMAC is enabled, so 
            //the IDMAC must be disabled durring setting. 
            //NV12 doesn't need this operation
            IDMACChannelDisable(IDMAC_CH_PRP_INPUT_VIDEO);
            if (m_ENCchannelParams.VOffset&&(m_ENCchannelParams.VOffset!=m_ENCchannelParams.UOffset))
            {
                OffsetData.uOffset= m_ENCchannelParams.UOffset - m_OversizedInputOffsetStride/2;
                OffsetData.vOffset = m_ENCchannelParams.VOffset - m_OversizedInputOffsetStride/2;
                CPMEMWriteOffset(IDMAC_CH_PRP_INPUT_VIDEO,
                                 &OffsetData, OffsetData.bInterleaved);
            }
            //The scrolling value equals to unaligned value
            if(m_OversizedInputOffsetStride%8)
            {
                CPMEMWriteXScroll(IDMAC_CH_PRP_INPUT_VIDEO, m_OversizedInputOffsetStride%8);
            }
            IDMACChannelEnable(IDMAC_CH_PRP_INPUT_VIDEO);
            //Start to fill half buffer in the right side
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, EBAnum, (UINT32 *)(dwOutputBuf+m_OversizedOutputOffsetStride));
            CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, EBAnum, (UINT32 *)((UINT32)pBuf+m_OversizedInputOffsetStride));
            
            if(EBAnum)
            {
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_ENC);
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_INPUT_VIDEO);
            }
            else
            {
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_ENC);
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_INPUT_VIDEO);
            }
            //wait for processing finish
            PrpWaitForNotBusy(FIRSTMODULE_INTERRUPT);
            //When IC finished its processing, 
            //set DP input buffer ready to display it out    
            if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
            {
                //set DP buffer 0 ready
                IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
                while(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
                    Sleep(1);
            }
            else
            {
                //set DP buffer 1 ready
                IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
                while(!IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
                    Sleep(1);                
            }   
            DEBUGMSG(0, (TEXT("%s: ===========FLIP TIME: %dms(%d)====== \r\n"), __WFUNCTION__, GetTickCount()-starttime, m_PrpFirstModule));
        }
        else
        {
            ERRORMSG(1,(TEXT("Oversized frame only support upsized case!\r\n")));
        }
    }
    else if(m_bVFOversizedFrame)
    {
        //oversized frame processing is always blocked.
        if((m_PrpFirstModule == prpSourceType_PRP)
            &&(m_PrpVFLastModule == prpSourceType_PRP))
        {
            starttime = GetTickCount();
            //Determine which EBA we will use, according to current using EBA
            EBAnum = IDMACChannelCurrentBufIsBuf1(IDMAC_CH_PRP_INPUT_VIDEO) ? 1:0;
            //Get the base address of IC output buffer we will use 
            //from the DP input buffer setting. 
            if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
            {
                //use DP buffer 0
                dwOutputBuf=CPMEMReadBufferAddr(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 0);
            }
            else
            {
                //use DP buffer 1
                dwOutputBuf=CPMEMReadBufferAddr(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 1);
            }
            //Confirm the previous IDMAC operation is finished.
            while(IDMACChannelIsBusy(IDMAC_CH_PRP_OUTPUT_VF))
            {
                Sleep(1);
            }
            //-----------------------------------------------
            //        processing half frame in the left                       
            //-----------------------------------------------
            //Start interrupt
            PrpIntrEnable(FIRSTMODULE_INTERRUPT);
            //If v offset is not zero and u,v offset is different  it means YV12 format is used, 
            //offset parameters need to be changed for left and right half frame.
            //For left half frame, the offset is the orignal value.
            //Offset parameters are in CPMEM.
            //It can not be changed while corresponding IDMAC is enabled, so 
            //the IDMAC must be disabled durring setting. 
            //NV12 doesn't need this operation
            IDMACChannelDisable(IDMAC_CH_PRP_INPUT_VIDEO);
            if (m_VFchannelParams.VOffset&&(m_VFchannelParams.VOffset!=m_VFchannelParams.UOffset))
            {
                OffsetData.uOffset= m_VFchannelParams.UOffset;
                OffsetData.vOffset = m_VFchannelParams.VOffset;
                CPMEMWriteOffset(IDMAC_CH_PRP_INPUT_VIDEO,
                                 &OffsetData, OffsetData.bInterleaved);
            }
            //Reset the x scrolling value.
            if(m_OversizedInputOffsetStride%8)
            {
                CPMEMWriteXScroll(IDMAC_CH_PRP_INPUT_VIDEO, 0);
            }
            IDMACChannelEnable(IDMAC_CH_PRP_INPUT_VIDEO);

            //Start to fill half buffer in the left side
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, EBAnum, (UINT32 *)dwOutputBuf);
            CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, EBAnum, pBuf);

            //Set the right ready flag according to EBA value.
            if(EBAnum)
            {
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_VF);
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_INPUT_VIDEO);
            }
            else
            {
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_INPUT_VIDEO);
            }
            //wait for processing finish
            PrpWaitForNotBusy(FIRSTMODULE_INTERRUPT);

            //-----------------------------------------------
            //        processing half frame in the right                      
            //-----------------------------------------------
            PrpIntrEnable(FIRSTMODULE_INTERRUPT);
            //If v offset is not zero and u,v offset is different  it means YV12 format is used, 
            //offset parameters need to be changed for left and right half frame.
            //For right half frame, the offset need to be adjusted as the start point address changed.
            //Offset parameters are in CPMEM.
            //It can not be changed while corresponding IDMAC is enabled, so 
            //the IDMAC must be disabled durring setting. 
            //NV12 doesn't need this operation
            IDMACChannelDisable(IDMAC_CH_PRP_INPUT_VIDEO);
            if (m_VFchannelParams.VOffset&&(m_VFchannelParams.VOffset!=m_VFchannelParams.UOffset))
            {
                OffsetData.uOffset= m_VFchannelParams.UOffset - m_OversizedInputOffsetStride/2;
                OffsetData.vOffset = m_VFchannelParams.VOffset - m_OversizedInputOffsetStride/2;
                CPMEMWriteOffset(IDMAC_CH_PRP_INPUT_VIDEO,
                                 &OffsetData, OffsetData.bInterleaved);
            }
            //The scrolling value equals to unaligned value
            if(m_OversizedInputOffsetStride%8)
            {
                CPMEMWriteXScroll(IDMAC_CH_PRP_INPUT_VIDEO, m_OversizedInputOffsetStride%8);
            }
            IDMACChannelEnable(IDMAC_CH_PRP_INPUT_VIDEO);

            //Start to fill half buffer in the right side
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, EBAnum, (UINT32 *)(dwOutputBuf+m_OversizedOutputOffsetStride));
            CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, EBAnum, (UINT32 *)((UINT32)pBuf+m_OversizedInputOffsetStride));
            if(EBAnum)
            {
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_VF);
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_INPUT_VIDEO);
            }
            else
            {
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_INPUT_VIDEO);
            }
            //wait for processing finish
            PrpWaitForNotBusy(FIRSTMODULE_INTERRUPT);
            //When IC finished its processing, 
            //set DP input buffer ready to display it out                   
            if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
            {
                //set DP buffer 0 ready
                IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
                while(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
                    Sleep(1);
            }
            else
            {
                //set DP buffer 1 ready
                IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
                while(!IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
                    Sleep(1);
            }
            DEBUGMSG(0, (TEXT("%s: ===========FLIP TIME: %dms(%d)====== \r\n"), __WFUNCTION__, GetTickCount()-starttime, m_PrpFirstModule));
        }
        else
        {
            ERRORMSG(1,(TEXT("Oversized frame only support upsized case!\r\n")));
        }
    }
    else
    {
        //If both buffers are filled, we should return FALSE or just drop the ready buffer quietly. 
        if((IDMACChannelBUF0IsReady(InputDMAChannel)
            &&IDMACChannelCurrentBufIsBuf1(InputDMAChannel))
           ||(IDMACChannelBUF1IsReady(InputDMAChannel)
           &&( !IDMACChannelCurrentBufIsBuf1(InputDMAChannel))))
        {
            DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Hardware is busy..\r\n"), __WFUNCTION__));
            if(!bDropFrameQuietly)
            {
                LeaveCriticalSection(&m_csStopping);
                return FALSE;
            }
        }
        
        //Workaround code for ticket engr91819
        //For sync panel, we should get a DP interrupt between the first IC operation done and next IC operation start.
        //It can help to avoid jitter and tearing issue in TVout case.
        //If bWait flag is not set, this workaround can't work properly
        //which means for TVout usecase, we must enable DDFLIP_WAITNOTBUSY.
        //Before next IC operation start, we must fetch an DP interrupt.
        if(m_bSYNCDPEnabled &&(IntType!=FRAME_INTERRUPT)&&(m_PrpFirstModule != prpSourceType_ARM))
        {
            if (WaitForSingleObject(m_hDPFGIntrEvent, 100) == WAIT_TIMEOUT)
            {
                RETAILMSG(1,
                         (TEXT("%s(): Waiting for DP FG EOF interrupt time out!\r\n"), __WFUNCTION__));
            }

        }

        starttime = GetTickCount();
        if(bWait)
            PrpIntrEnable(IntType);
        if (IDMACChannelCurrentBufIsBuf1(InputDMAChannel)) 
        {
            // Program input buffer into task parameter memory
            CPMEMWriteBuffer(InputDMAChannel,0, pBuf);
            IDMACChannelBUF0SetReady(InputDMAChannel);
        }
        else
        {
            // Program input buffer into task parameter memory
            CPMEMWriteBuffer(InputDMAChannel,1, pBuf);
            IDMACChannelBUF1SetReady(InputDMAChannel);
           
        }
        if(bWait)
        {
            PrpWaitForNotBusy(IntType);
            DEBUGMSG(ZONE_DEVICE, (TEXT("%s: ===========FLIP TIME: %dms(%d)====== \r\n"), __WFUNCTION__, GetTickCount()-starttime, m_PrpFirstModule));
        }

        //Workaround code for ticket engr91819
        //For sync panel, we should get a DP interrupt between the first IC operation done and next IC operation start.
        //It can help to avoid jitter and tearing issue in TVout case.
        //After the first IC operation done, we enable the interrupt.
        if(m_bSYNCDPEnabled &&(m_PrpFirstModule != prpSourceType_ARM))
        {
            IDMACChannelClearIntStatus(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, IPU_INTR_TYPE_EOF);
            IDMACChannelIntCntrl(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, IPU_INTR_TYPE_EOF, TRUE);
        }

    }
    LeaveCriticalSection(&m_csStopping);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  PrpAddInputCombBuffer
//
// This function fill combining buffer into hardware.
//
// Parameters:
//      pBuf
//          [in] Input buffer address.
//
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpAddInputCombBuffer(UINT32 *pBuf)
{

    PHYSICAL_ADDRESS inputPhysAddr;

    // Critical section to prevent race condition upon
    // stopping the pre-processing channel
    EnterCriticalSection(&m_csStopping);

    if (m_bRunning == FALSE)
    {
        LeaveCriticalSection(&m_csStopping);    
        return FALSE;
    }

    inputPhysAddr.QuadPart = (LONGLONG) pBuf;

      //If both of buffers is avaiable, we can request the second buffer 

    if((IDMACChannelBUF0IsReady(IDMAC_CH_PRP_INPUT_GRAPHICS)
        &&IDMACChannelCurrentBufIsBuf1(IDMAC_CH_PRP_INPUT_GRAPHICS))
       ||(IDMACChannelBUF1IsReady(IDMAC_CH_PRP_INPUT_GRAPHICS)
       &&( !IDMACChannelCurrentBufIsBuf1(IDMAC_CH_PRP_INPUT_GRAPHICS))))
    {
        DEBUGMSG(ZONE_ERROR,
        (TEXT("%s: Hardware is busy..\r\n"), __WFUNCTION__));
    }

    if (IDMACChannelCurrentBufIsBuf1(IDMAC_CH_PRP_INPUT_GRAPHICS)) 
    {
        // Program input buffer into task parameter memory
        CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_GRAPHICS,0, pBuf);
        IDMACChannelBUF0SetReady(IDMAC_CH_PRP_INPUT_GRAPHICS);
    }
    else
    {
        // Program input buffer into task parameter memory
        CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_GRAPHICS,1, pBuf);
        IDMACChannelBUF1SetReady(IDMAC_CH_PRP_INPUT_GRAPHICS);
       
    }

    LeaveCriticalSection(&m_csStopping);


    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpConfigure
//
// This function configures the IC registers,IRT registers, DP registers and IDMAC
// channels for the preprocessor channel.
//
// Parameters:
//      pPrpConfigureData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigure(pPrpConfigData pConfigData)
{
    icFormat iVFFormat,iENCFormat;
    UINT16 iVFFunctionFlag,iVFinputbpp = 0;
    UINT16 iENCFunctionFlag,iENCinputbpp = 0;
    DP_CH_TYPE DpChannel;
    // We are going to skip the IC when we are dealing with a 32bpp alpha blended surface,
    // regardless of the BSP setting for the path
    BOOL bDisableNOICPath = BSPDisableNOICPath() && (pConfigData->inputIDMAChannel.DataWidth != icDataWidth_32BPP);
    dpCSCConfigData DpCSCConfigData;
    dmfcConfigData dmfcData;
    UINT32 oldVal, newVal, iMask, iBitval;
    memset(&dmfcData,0,sizeof(dmfcConfigData));

    PRP_FUNCTION_ENTRY();

    if (m_bRunning)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Viewfinding channel already running. Please stop it at first.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_bInputisInterlaced = pConfigData->bInputIsInterlaced;
    m_OversizedInputOffsetStride=0; 
    m_OversizedOutputOffsetStride=0; 
    //-----------------------------------
    //pre-process the output size
    //-----------------------------------
    if(pConfigData->bVFEnable)
    {
        iVFinputbpp = IcDataWidthToBPP[pConfigData->inputIDMAChannel.DataWidth];
        iVFFormat= pConfigData->outputVFIDMAChannel.FrameFormat;
 
        if (pConfigData->VFFlipRot.rotate90)
        {
            m_PrpVFOutputHeight= pConfigData->outputVFIDMAChannel.FrameSize.width;
            m_PrpVFOutputWidth = pConfigData->outputVFIDMAChannel.FrameSize.height;
            m_PrpVFOutputStride = pConfigData->outputVFIDMAChannel.FrameSize.height *iVFinputbpp /8;
        }
        else
        {
            m_PrpVFOutputWidth = pConfigData->outputVFIDMAChannel.FrameSize.width;
            m_PrpVFOutputHeight = pConfigData->outputVFIDMAChannel.FrameSize.height;
            // we don't use the output stride setting from display driver
            m_PrpVFOutputStride = pConfigData->outputVFIDMAChannel.FrameSize.width *iVFinputbpp /8;
        }
    }
    if(pConfigData->bENCEnable)
    {
        iENCinputbpp = IcDataWidthToBPP[pConfigData->inputIDMAChannel.DataWidth];
        iENCFormat= pConfigData->outputENCIDMAChannel.FrameFormat;
 
        if (pConfigData->ENCFlipRot.rotate90)
        {
            m_PrpENCOutputHeight= pConfigData->outputENCIDMAChannel.FrameSize.width;
            m_PrpENCOutputWidth = pConfigData->outputENCIDMAChannel.FrameSize.height;
            m_PrpENCOutputStride = pConfigData->outputENCIDMAChannel.FrameSize.height *iENCinputbpp /8;
        }
        else
        {
            m_PrpENCOutputWidth = pConfigData->outputENCIDMAChannel.FrameSize.width;
            m_PrpENCOutputHeight = pConfigData->outputENCIDMAChannel.FrameSize.height;
            // we don't use the output stride setting from display driver
            m_PrpENCOutputStride = pConfigData->outputENCIDMAChannel.FrameSize.width *iENCinputbpp /8;
        }
    }


    //-----------------------------------
    //initalize parameters
    //-----------------------------------
    m_PrpFirstModule = prpSourceType_ARM;
    m_PrpVFLastModule = prpSourceType_ARM;
    m_PrpENCLastModule = prpSourceType_ARM;
    m_bVFPrpEnabled = FALSE;
    m_bENCPrpEnabled = FALSE;
    m_bVFIRTEnabled = FALSE;
    m_bENCIRTEnabled = FALSE;
    m_bDMFCChannelUsing = FALSE;
    m_bVFCombEnabled = FALSE;
    m_bSYNCDPEnabled = FALSE;
    m_bASYNCDPEnabled = FALSE;
    m_bSYNCisDI1= FALSE;
    m_bSYNCisInterlaced = FALSE;
    m_bENCOversizedFrame = FALSE;
    m_bVFOversizedFrame = FALSE;
    
    m_bSYNCDPCSCChanged = FALSE;
    m_bASYNCDPCSCChanged = FALSE;
    
    memset(&m_VFchannelParams, 0, sizeof(prpIDMACChannelParams));       
    memset(&m_ENCchannelParams, 0, sizeof(prpIDMACChannelParams));     
    PrpClearBuffers();

    if(!m_bInputisInterlaced)
    {
        iMask = CSP_BITFMASK(IPU_IC_CONF_RWS_EN)
               |CSP_BITFMASK(IPU_IC_CONF_CSI_MEM_WR_EN) ;
        iBitval = CSP_BITFVAL(IPU_IC_CONF_RWS_EN, IPU_IC_CONF_RWS_EN_ENABLE)
               | CSP_BITFVAL(IPU_IC_CONF_CSI_MEM_WR_EN, IPU_IC_CONF_CSI_MEM_WR_EN_DISABLE);       

    }
    else
    {
        iMask = CSP_BITFMASK(IPU_IC_CONF_RWS_EN)
               |CSP_BITFMASK(IPU_IC_CONF_CSI_MEM_WR_EN) ;
        iBitval = CSP_BITFVAL(IPU_IC_CONF_RWS_EN, IPU_IC_CONF_RWS_EN_DISABLE)
               | CSP_BITFVAL(IPU_IC_CONF_CSI_MEM_WR_EN, IPU_IC_CONF_CSI_MEM_WR_EN_DISABLE);       


    }
    // Use interlocked function to Disable IC tasks.
    do
    {
        oldVal = INREG32(&m_pIC->IC_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                oldVal, newVal) != oldVal);    

    
    //-----------------------------------
    //initalize the processing flow
    //-----------------------------------
    // the source should be mcu
    CMSetProcFlow(IPU_PROC_FLOW_PRPENC_ROT_SRC,
                                IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);
    CMSetProcFlow(IPU_PROC_FLOW_PRPVF_ROT_SRC,
                                IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);
    CMSetProcFlow(IPU_PROC_FLOW_PRP_SRC,
                                IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);

    CMSetProcFlow(IPU_PROC_FLOW_PRPENC_DEST,
                                IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
    CMSetProcFlow(IPU_PROC_FLOW_PRPVF_DEST,
                                IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
    CMSetProcFlow(IPU_PROC_FLOW_PRPENC_ROT_DEST,
                                IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
    CMSetProcFlow(IPU_PROC_FLOW_PRPVF_ROT_DEST,
                                IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);

    CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
                        IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_MCU);
    CMSetDispFlow(IPU_DISP_FLOW_DP_ASYNC0_SRC,
                        IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_MCU);


    IDMACChannelBUF0ClrReady(IDMAC_CH_IRT_INPUT_PRP_VF);
    IDMACChannelBUF0ClrReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);
    IDMACChannelBUF1ClrReady(IDMAC_CH_IRT_INPUT_PRP_VF);
    IDMACChannelBUF1ClrReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);
    IDMACChannelClrCurrentBuf(IDMAC_CH_IRT_INPUT_PRP_VF);
    IDMACChannelClrCurrentBuf(IDMAC_CH_IRT_OUTPUT_PRP_VF);
    
    IDMACChannelBUF0ClrReady(IDMAC_CH_IRT_INPUT_PRP_ENC);
    IDMACChannelBUF0ClrReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);
    IDMACChannelBUF1ClrReady(IDMAC_CH_IRT_INPUT_PRP_ENC);
    IDMACChannelBUF1ClrReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);
    IDMACChannelClrCurrentBuf(IDMAC_CH_IRT_INPUT_PRP_ENC);
    IDMACChannelClrCurrentBuf(IDMAC_CH_IRT_OUTPUT_PRP_ENC);    
    
    IDMACChannelBUF0ClrReady(IDMAC_CH_PRP_INPUT_VIDEO);
    IDMACChannelBUF0ClrReady(IDMAC_CH_PRP_OUTPUT_VF);
    IDMACChannelBUF1ClrReady(IDMAC_CH_PRP_INPUT_VIDEO);
    IDMACChannelBUF1ClrReady(IDMAC_CH_PRP_OUTPUT_VF);
    IDMACChannelClrCurrentBuf(IDMAC_CH_PRP_INPUT_VIDEO);
    IDMACChannelClrCurrentBuf(IDMAC_CH_PRP_OUTPUT_VF);
    IDMACChannelBUF0ClrReady(IDMAC_CH_PRP_INPUT_GRAPHICS);
    IDMACChannelBUF1ClrReady(IDMAC_CH_PRP_INPUT_GRAPHICS);
    IDMACChannelClrCurrentBuf(IDMAC_CH_PRP_INPUT_GRAPHICS);
    
    IDMACChannelBUF0ClrReady(IDMAC_CH_PRP_OUTPUT_ENC);
    IDMACChannelBUF1ClrReady(IDMAC_CH_PRP_OUTPUT_ENC);
    IDMACChannelClrCurrentBuf(IDMAC_CH_PRP_OUTPUT_ENC);  
    
    IDMACChannelBUF0ClrReady(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
    IDMACChannelBUF1ClrReady(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
    IDMACChannelClrCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
    
    IDMACChannelBUF0ClrReady(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE);
    IDMACChannelBUF1ClrReady(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE);
    IDMACChannelClrCurrentBuf(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE);

                                
    
    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PRPVF_CMB,
                IPU_IC_CONF_PRPVF_CMB_DISABLE);       
    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PRPVF_CSC1,
                IPU_IC_CONF_PRPVF_CSC1_DISABLE);
    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PRPVF_CSC2,
                IPU_IC_CONF_PRPVF_CSC2_DISABLE);
    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PRPENC_CSC1,
                IPU_IC_CONF_PRPENC_CSC1_DISABLE);

    //disable dmfc direct path
    dmfcData.DestChannel = IDMAC_INVALID_CHANNEL;
    DMFCConfigure(IDMAC_CH_PRP_OUTPUT_VF,&dmfcData);

    //If both output channel are enabled or combine channel are enabled,
    // we will use sequential module chain instead of flexible chain.
    // The reason is there are too many hardware limitations in the both output  
    // channels and combine channel cases.
    if((pConfigData->bVFEnable && pConfigData->bENCEnable)
        ||(pConfigData->bVFEnable && pConfigData->bCombineEnable))
    {
        //enc +vf path with combine input
        //enc +vf path without combine input
        //vf path with combine input

        //configure vf path
        iVFFunctionFlag = 0;
        if(pConfigData->bInputIsInterlaced)
        {
            // for interlaced input, prp always should be first enabled module.
            iVFFunctionFlag |=PRP_FUNC_DOWNSIZE;
        }
        else if((pConfigData->inputIDMAChannel.FrameSize.width == 
            (m_PrpVFOutputWidth * m_PrpVFOutputHeight 
            / pConfigData->inputIDMAChannel.FrameSize.height)))
        {
            if(pConfigData->inputIDMAChannel.FrameSize.width > m_PrpVFOutputWidth)
                iVFFunctionFlag |=PRP_FUNC_DOWNSIZE;
            else if(pConfigData->inputIDMAChannel.FrameSize.width < m_PrpVFOutputWidth)
                iVFFunctionFlag |=PRP_FUNC_UPSIZE;
            else
                iVFFunctionFlag |=PRP_FUNC_NORESIZE;
        }
        else if(pConfigData->inputIDMAChannel.FrameSize.width > 
            (m_PrpVFOutputWidth * m_PrpVFOutputHeight 
            / pConfigData->inputIDMAChannel.FrameSize.height))
        {
            iVFFunctionFlag |=PRP_FUNC_DOWNSIZE;       
        }
        else
        {
            iVFFunctionFlag |=PRP_FUNC_UPSIZE;
        }
        
        if(pConfigData->VFFlipRot.verticalFlip |
           pConfigData->VFFlipRot.horizontalFlip  |
           pConfigData->VFFlipRot.rotate90 )
        {
            iVFFunctionFlag |=PRP_FUNC_ROTATION;
        }
        //We will use dp csc for most csc function, only when IC need to combine two different color 
        //space frame, we will enable ic csc
#if 0
        if(((pConfigData->bCombineEnable) 
            && (pConfigData->inputIDMAChannel.FrameFormat != pConfigData->inputcombVFIDMAChannel.FrameFormat))
            //&& (pConfigData->outputVFIDMAChannel.FrameFormat == pConfigData->inputcombVFIDMAChannel.FrameFormat))
            && (pConfigData->VFCSCEquation != icCSCNoOp))
        {
                iVFFunctionFlag |=PRP_FUNC_CSC;
                DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: IC csc enabled. XEC DLS may can't work correctly.\r\n"), __WFUNCTION__));               
        }
#endif
        iENCFunctionFlag = 0;
        if(pConfigData->bENCEnable)
        {
            //configure enc path
            if(pConfigData->bInputIsInterlaced)
            {
                // for interlaced input, prp always should be first enabled module.
                iENCFunctionFlag |=PRP_FUNC_DOWNSIZE;
            }
            else if((pConfigData->inputIDMAChannel.FrameSize.width == 
                (m_PrpENCOutputWidth * m_PrpENCOutputHeight 
                / pConfigData->inputIDMAChannel.FrameSize.height)))
            {
                if(pConfigData->inputIDMAChannel.FrameSize.width > m_PrpENCOutputWidth)
                    iENCFunctionFlag |=PRP_FUNC_DOWNSIZE;
                else if(pConfigData->inputIDMAChannel.FrameSize.width < m_PrpENCOutputWidth)
                    iENCFunctionFlag |=PRP_FUNC_UPSIZE;
                else
                    iENCFunctionFlag |=PRP_FUNC_NORESIZE;
            }
            else if(pConfigData->inputIDMAChannel.FrameSize.width > 
                (m_PrpENCOutputWidth * m_PrpENCOutputHeight 
                / pConfigData->inputIDMAChannel.FrameSize.height))
            {
                iENCFunctionFlag |=PRP_FUNC_DOWNSIZE;       
            }
            else
            {
                iENCFunctionFlag |=PRP_FUNC_UPSIZE;
            }
            
            if(((pConfigData->ENCFlipRot.verticalFlip ? 1 : 0) |
                (pConfigData->ENCFlipRot.horizontalFlip ? 1 : 0) << 1  |
                (pConfigData->ENCFlipRot.rotate90 ? 1 : 0) << 2))
            {
                iENCFunctionFlag |=PRP_FUNC_ROTATION;
            }
            //We will use dp csc for most csc function, only when IC need to combine two different color 
            //space frame, we will enable ic csc
            //As enc channel doesn't have combine function, the csc funtion won't be used.
            //iENCFunctionFlag |=PRP_FUNC_CSC;
        }

        //configure prp
        //CONFIGURE VF CHANNEL, vf channel in prp is always enabled for these two cases
        m_PrpFirstModule = prpSourceType_PRP;

        //----------------------------------------------------------------------
        //-------------Start IC configuration for VF Channel which must be done---------
        //----------------------------------------------------------------------
        m_bVFPrpEnabled = TRUE;
        //-----------------------------------------------------------------
        // Setup input source
        //-----------------------------------------------------------------
        if(pConfigData->bCombineEnable)
        {
            m_bVFCombEnabled = TRUE;
            if(!PrpInitChannelParams(&pConfigData->inputcombVFIDMAChannel, &m_VFchannelParams))
            {
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Incorrect input combine channel parameter. \r\n"), __WFUNCTION__ ));   
                return FALSE;
            }
            PrpConfigureInput(prpInputChannel_COMBINE, prpOutputChannel_VF);
            PrpConfigureCombine(pConfigData->inputcombAlpha, pConfigData->inputcombColorkey);
            //-----------------------------------------------------------------
            // Setup CSC parameter for combine surface
            //-----------------------------------------------------------------
            BOOL bInputRGB, bInputCombRGB;
            if((pConfigData->inputIDMAChannel.FrameFormat == icFormat_RGB) ||
                  (pConfigData->inputIDMAChannel.FrameFormat == icFormat_RGBA))
                bInputRGB = TRUE;
            else
                bInputRGB = FALSE;
                
            if((pConfigData->inputcombVFIDMAChannel.FrameFormat == icFormat_RGB) ||
                  (pConfigData->inputcombVFIDMAChannel.FrameFormat == icFormat_RGBA))
                bInputCombRGB = TRUE;
            else
                bInputCombRGB = FALSE;
            
            if(bInputRGB != bInputCombRGB)
            {
                if(bInputRGB)
                {
                    PrpConfigureCSC(CSCR2Y_A1,icCSC1,NULL,prpOutputChannel_VF);
                    PrpConfigureCSC(CSCY2R_A1,icCSC2,NULL,prpOutputChannel_VF);
                }
                else
                {
                    PrpConfigureCSC(CSCY2R_A1,icCSC1,NULL,prpOutputChannel_VF);
                    PrpConfigureCSC(CSCR2Y_A1,icCSC2,NULL,prpOutputChannel_VF);                    
                }
            }                
        }   
        if(!PrpInitChannelParams(&pConfigData->inputIDMAChannel, &m_VFchannelParams))
        {
             DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s: Incorrect input channel parameter. \r\n"), __WFUNCTION__ ));   
             return FALSE;            
        }
        PrpConfigureInput(prpInputChannel_MAIN, prpOutputChannel_VF);
        memcpy(&m_ENCchannelParams, &m_VFchannelParams, sizeof(prpIDMACChannelParams));

        //-----------------------------------------------------------------
        // Setup resizing
        //-----------------------------------------------------------------
        PrpConfigureResize(prpOutputChannel_VF);

        //-----------------------------------------------------------------
        // Setup output destination
        //-----------------------------------------------------------------
        PrpConfigureOutput(&pConfigData->outputVFIDMAChannel,  prpOutputChannel_VF, TRUE);
        m_PrpVFLastModule = prpSourceType_PRP;
        //----------------------------------------------------------------------
        //-----------------End of IC configuration for VF Channel---------------------
        //----------------------------------------------------------------------
        
        if(pConfigData->bENCEnable)
        {
            m_bENCPrpEnabled = TRUE;
            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            // The input source has been configured in vf configuration
            
            //-----------------------------------------------------------------
            // Setup resizing
            //-----------------------------------------------------------------
            PrpConfigureResize(prpOutputChannel_ENC);

            //-----------------------------------------------------------------
            // Setup CSC parameter
            //-----------------------------------------------------------------
            // always no csc

            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            PrpConfigureOutput(&pConfigData->outputENCIDMAChannel,  prpOutputChannel_ENC, TRUE);
            m_PrpENCLastModule = prpSourceType_PRP;
        }

        // configure IRT
        if(iVFFunctionFlag &PRP_FUNC_ROTATION)
        {
            m_bVFIRTEnabled = TRUE;
            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            IRTConfigureInput(&pConfigData->VFFlipRot, prpOutputChannel_VF);
            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            IRTConfigureOutput(&pConfigData->VFFlipRot,  prpOutputChannel_VF);
            
            //confgure IRT
            m_PrpVFLastModule  = prpSourceType_ROT;
        }
        
        if(pConfigData->bENCEnable&&(iENCFunctionFlag &PRP_FUNC_ROTATION))
        {
            m_bENCIRTEnabled = TRUE;

            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            IRTConfigureInput(&pConfigData->ENCFlipRot, prpOutputChannel_ENC);
            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            IRTConfigureOutput(&pConfigData->ENCFlipRot,  prpOutputChannel_ENC);
            
            //confgure IRT
            m_PrpENCLastModule  = prpSourceType_ROT;
        }

        //----------------------------------------------------------------------
        //-------------Start DP configuration for VF Channel which must be done---------
        //----------------------------------------------------------------------

        dmfcData.FrameWidth = m_VFchannelParams.iWidth;
        if(dmfcData.FrameWidth >= 512)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_128x128;
        else if(dmfcData.FrameWidth >= 256)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_64x128;
        else if(dmfcData.FrameWidth >= 128)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128;
        else if(dmfcData.FrameWidth >= 64)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_16x128;
        else if(dmfcData.FrameWidth >= 32)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_8x128;
        else 
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128;

        dmfcData.FrameHeight = m_VFchannelParams.iHeight;
        dmfcData.BurstSize = IPU_DMFC_BURST_SIZE_0_4_WORDS;
        dmfcData.WaterMarkEnable = IPU_DMFC_WM_EN_DISABLE;
        
        // Don't care values
        dmfcData.WaterMarkClear = 1; // Make the value the same as default value.
        dmfcData.WaterMarkSet = 0; // Don't care if Watermark disabled
        dmfcData.DestChannel = 0; // Don't care unless using IC->DMFC path
        dmfcData.PixelPerWord = IPU_DMFC_PPW_C_8BPP; // Only care for DC Read Ch or IC->DMFC path
       
        if(pConfigData->bVFIsSyncFlow)
        {
            dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_4;
            DMFCConfigure(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, &dmfcData);
            DpChannel = DP_CHANNEL_SYNC;
            m_bSYNCDPEnabled = TRUE;
            m_bSYNCisInterlaced = pConfigData->bVFIsInterlaced;
        }
        else
        {
            dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_6;
            DMFCConfigure(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, &dmfcData);
            DpChannel = DP_CHANNEL_ASYNC0;
            m_bASYNCDPEnabled = TRUE;
        }
        DPCSCGetCoeffs(DpChannel, &DpCSCConfigData);
        if (DpCSCConfigData.CSCPosition == DP_CSC_DISABLE)
        {
            DpCSCConfigData.CSCPosition = DP_CSC_FG;
            DpCSCConfigData.CSCEquation = pConfigData->VFCSCEquation; 
            DpCSCConfigData.CSCCoeffs= pConfigData->VFCSCCoeffs;
            DpCSCConfigData.bGamutEnable = TRUE;
        }
        else if(DpCSCConfigData.CSCPosition == DP_CSC_BG)
        {
            DpCSCConfigData.CSCPosition = DP_CSC_BOTH;
        }
        DPCSCConfigure(DpChannel, &DpCSCConfigData);

        DPConfigureInput(pConfigData->bVFIsSyncFlow,
                         pConfigData->bVFIsInterlaced,
                         prpOutputChannel_VF);
        //----------------------------------------------------------------------
        //------------------End of DP configuration for VF Channel -------------------
        //----------------------------------------------------------------------

        if(pConfigData->bENCEnable)
        {
            dmfcData.FrameWidth = m_ENCchannelParams.iWidth;
            if(dmfcData.FrameWidth >= 512)
                dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_128x128;
            else if(dmfcData.FrameWidth >= 256)
                dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_64x128;
            else if(dmfcData.FrameWidth >= 128)
                dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128;
            else if(dmfcData.FrameWidth >= 64)
                dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_16x128;
            else if(dmfcData.FrameWidth >= 32)
                dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_8x128;
            else 
                dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128;
            
            dmfcData.FrameHeight = m_ENCchannelParams.iHeight;
            dmfcData.BurstSize = IPU_DMFC_BURST_SIZE_0_4_WORDS;
            dmfcData.WaterMarkEnable = IPU_DMFC_WM_EN_DISABLE;
            
            // Don't care values
            dmfcData.WaterMarkClear = 1; // Don't care if Watermark disabled
            dmfcData.WaterMarkSet = 0; // Don't care if Watermark disabled
            dmfcData.DestChannel = 0; // Don't care unless using IC->DMFC path
            dmfcData.PixelPerWord = IPU_DMFC_PPW_C_8BPP; // Only care for DC Read Ch or IC->DMFC path

            if(pConfigData->bENCIsSyncFlow)
            {
                dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_4;
                DMFCConfigure(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, &dmfcData);
                DpChannel = DP_CHANNEL_SYNC;
                m_bSYNCDPEnabled = TRUE;
                m_bSYNCisDI1= TRUE;
                m_bSYNCisInterlaced = pConfigData->bENCIsInterlaced;
            }
            else
            {
                dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_6;
                DMFCConfigure(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, &dmfcData);
                DpChannel = DP_CHANNEL_ASYNC0;
                m_bASYNCDPEnabled = TRUE;
            }
            DPCSCGetCoeffs(DpChannel, &DpCSCConfigData);
            if (DpCSCConfigData.CSCPosition == DP_CSC_DISABLE)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_FG;
                DpCSCConfigData.CSCEquation = pConfigData->ENCCSCEquation;
                DpCSCConfigData.CSCCoeffs= pConfigData->ENCCSCCoeffs;
                DpCSCConfigData.bGamutEnable = TRUE;
            }
            else if(DpCSCConfigData.CSCPosition == DP_CSC_BG)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BOTH;
            }
            DPCSCConfigure(DpChannel, &DpCSCConfigData);        
            DPConfigureInput(pConfigData->bENCIsSyncFlow, 
                             pConfigData->bENCIsInterlaced,
                             prpOutputChannel_ENC);
        }
        m_bConfigured = TRUE;
    }
    else if(pConfigData->bVFEnable)
    {
        //only vf path

        //configure vf path
        iVFFunctionFlag = 0;
        if(pConfigData->bInputIsInterlaced)
        {
            // for interlaced input, prp always should be first enabled module.
            iVFFunctionFlag |=PRP_FUNC_DOWNSIZE;
        }
        else if((pConfigData->inputIDMAChannel.FrameSize.width == 
            (m_PrpVFOutputWidth * m_PrpVFOutputHeight 
            / pConfigData->inputIDMAChannel.FrameSize.height)))
        {
            if(pConfigData->inputIDMAChannel.FrameSize.width > m_PrpVFOutputWidth)
                iVFFunctionFlag |=PRP_FUNC_DOWNSIZE;
            else if(pConfigData->inputIDMAChannel.FrameSize.width < m_PrpVFOutputWidth)
                iVFFunctionFlag |=PRP_FUNC_UPSIZE;
            else
                iVFFunctionFlag |=PRP_FUNC_NORESIZE;
        }
        else if(pConfigData->inputIDMAChannel.FrameSize.width > 
            (m_PrpVFOutputWidth * m_PrpVFOutputHeight 
            / pConfigData->inputIDMAChannel.FrameSize.height))
        {
            iVFFunctionFlag |=PRP_FUNC_DOWNSIZE;       
        }
        else
        {
            iVFFunctionFlag |=PRP_FUNC_UPSIZE;
        }
       
        if(((pConfigData->VFFlipRot.verticalFlip ? 1 : 0) |
            (pConfigData->VFFlipRot.horizontalFlip ? 1 : 0) << 1  |
            (pConfigData->VFFlipRot.rotate90 ? 1 : 0) << 2))
        {
            iVFFunctionFlag |=PRP_FUNC_ROTATION;
        }
        if(bDisableNOICPath && (iVFFunctionFlag == 0))
        {
            iVFFunctionFlag |=PRP_FUNC_DOWNSIZE;
        }
        if(!PrpInitChannelParams(&pConfigData->inputIDMAChannel, &m_VFchannelParams))
        {
             DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s: Incorrect input channel parameter. \r\n"), __WFUNCTION__ ));   
             return FALSE;            
        }
        if(iVFFunctionFlag & PRP_FUNC_DOWNSIZE)
        {        
            //CONFIGURE VF CHANNEL
            
            m_bVFPrpEnabled = TRUE;
            if(m_PrpFirstModule == prpSourceType_ARM)
            {
                m_PrpFirstModule = prpSourceType_PRP;
            }
            
            if(m_PrpVFOutputHeight > PRP_MAX_OUTPUT_HEIGHT)
            {
                 RETAILMSG(1,
                     (TEXT("The output height is too large.(%d > 1024) \r\n"), m_PrpVFOutputHeight ));   
                 return FALSE;            
            }            
            
            if( m_PrpVFOutputWidth > PRP_MAX_OUTPUT_WIDTH)
            {
                m_bVFOversizedFrame = TRUE;
                m_VFchannelParams.iWidth = m_VFchannelParams.iWidth/2;
                m_PrpVFOutputWidth = m_PrpVFOutputWidth/2;

                m_OversizedInputOffsetStride = m_VFchannelParams.iWidth* iVFinputbpp /8;
                if(iVFinputbpp == 8)
                    m_OversizedOutputOffsetStride = m_PrpVFOutputWidth* 2;  //convert 420 to 422
                else
                   m_OversizedOutputOffsetStride = m_PrpVFOutputWidth * iVFinputbpp /8;
            }

            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            PrpConfigureInput(prpInputChannel_MAIN, prpOutputChannel_VF);

            //-----------------------------------------------------------------
            // Setup resizing
            //-----------------------------------------------------------------
            PrpConfigureResize(prpOutputChannel_VF);

            //-----------------------------------------------------------------
            // Setup CSC parameter
            //-----------------------------------------------------------------
            //nocsc

            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            PrpConfigureOutput(&pConfigData->outputVFIDMAChannel,  prpOutputChannel_VF, TRUE);

            if(m_bVFOversizedFrame)
            {
                m_PrpVFOutputWidth = m_PrpVFOutputWidth*2;
                m_VFchannelParams.iWidth = m_PrpVFOutputWidth;
            }

            m_PrpVFLastModule = prpSourceType_PRP;
        }
        
        if(iVFFunctionFlag &PRP_FUNC_ROTATION)
        {
            m_bVFIRTEnabled = TRUE;
            if(m_PrpFirstModule == prpSourceType_ARM)
            {
                m_PrpFirstModule = prpSourceType_ROT;
            }
            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            IRTConfigureInput(&pConfigData->VFFlipRot, prpOutputChannel_VF);
            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            IRTConfigureOutput(&pConfigData->VFFlipRot,  prpOutputChannel_VF);
            
            //confgure IRT
            m_PrpVFLastModule  = prpSourceType_ROT;
        }
        
        if(iVFFunctionFlag & PRP_FUNC_UPSIZE)
        {        
            //CONFIGURE VF CHANNEL
            
            m_bVFPrpEnabled = TRUE;
            if(m_PrpFirstModule == prpSourceType_ARM)
            {
                m_PrpFirstModule = prpSourceType_PRP;
            }
            
            if(m_PrpVFOutputHeight > PRP_MAX_OUTPUT_HEIGHT)
            {
                 RETAILMSG(1,
                     (TEXT("The output height is too large.(%d > 1024) \r\n"), m_PrpVFOutputHeight ));   
                 return FALSE;            
            }   
            //For upsized case, if the required output width is too large to prp 
            //acceptable width, we must split it to two parts. For each part, 
            //the input and output width will be half of orignal one.
            //Then Prp can handle each of them.  
            UINT32 tempInputWidth=0,tempOutputWidth=0,tempResizeRatio=0,tempINW=0;
            if( m_PrpVFOutputWidth > PRP_MAX_OUTPUT_WIDTH)
            {
                m_bVFOversizedFrame = TRUE;
                tempInputWidth = m_VFchannelParams.iWidth;
                tempOutputWidth = m_PrpVFOutputWidth;
                tempResizeRatio = 8192* (tempInputWidth-1) /(tempOutputWidth-1);
                tempINW = ((tempInputWidth-1)/2)&0xfffffffe;
                if(m_VFchannelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
                {
                    tempINW = ((tempInputWidth-1)/2)&0xfffffff0;
                }
                m_VFchannelParams.iWidth = (UINT16)((tempInputWidth-tempINW+7)&0xfffffff8);
                m_PrpVFOutputWidth = (UINT16)(m_PrpVFOutputWidth-(((8192*tempINW + tempResizeRatio-1)/tempResizeRatio+3)&0xfffffffc));
                m_OversizedInputOffsetStride = tempINW* iVFinputbpp /8;
                if(iVFinputbpp == 8)
                    m_OversizedOutputOffsetStride = (tempOutputWidth-m_PrpVFOutputWidth)* 2;  //convert 420 to 422
                else
                   m_OversizedOutputOffsetStride = (tempOutputWidth-m_PrpVFOutputWidth) * iVFinputbpp /8;
            }

            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            PrpConfigureInput(prpInputChannel_MAIN, prpOutputChannel_VF);

            //-----------------------------------------------------------------
            // Setup resizing
            //-----------------------------------------------------------------
            PrpConfigureResize(prpOutputChannel_VF);
            //Re-configure the coefficients according to oskar's algorithm
            if(m_bVFOversizedFrame)
            {
                PrpConfigureResize4OversizeWidth((UINT16)tempINW, (UINT16)(tempOutputWidth-m_PrpVFOutputWidth), prpOutputChannel_VF);
            }

            //-----------------------------------------------------------------
            // Setup CSC parameter
            //-----------------------------------------------------------------
            //nocsc

            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            PrpConfigureOutput(&pConfigData->outputVFIDMAChannel, prpOutputChannel_VF, TRUE);
            //After setting up prp's parameter , we should set the input and 
            //output width back, as the following sub-module DP should 
            //processing the whole frame in one time.
            if(m_bVFOversizedFrame)
            {
                m_PrpVFOutputWidth = (UINT16)tempOutputWidth;
                m_VFchannelParams.iWidth = m_PrpVFOutputWidth;
            }

            m_PrpVFLastModule = prpSourceType_PRP;
        }
        

        UINT32 iScale;
        if(pConfigData->bVFIsSyncFlow)
            iScale =2;
        else
            iScale =1;
            
        dmfcData.FrameWidth = m_VFchannelParams.iWidth;
        if(dmfcData.FrameWidth > 1024)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_256x128;
        else if(dmfcData.FrameWidth*iScale >= 512)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_128x128;
        else if(dmfcData.FrameWidth*iScale >= 256)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_64x128;
        else if(dmfcData.FrameWidth*iScale >= 128)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128;
        else if(dmfcData.FrameWidth*iScale >= 64)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_16x128;
        else if(dmfcData.FrameWidth*iScale >= 32)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_8x128;
        else 
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128;

        dmfcData.FrameHeight = m_VFchannelParams.iHeight;
        dmfcData.BurstSize = IPU_DMFC_BURST_SIZE_0_4_WORDS;
        dmfcData.WaterMarkEnable = IPU_DMFC_WM_EN_DISABLE;
        
        // Don't care values
        dmfcData.WaterMarkClear = 1; // Make the value the same as default value.
        dmfcData.WaterMarkSet = 0; // Don't care if Watermark disabled
        dmfcData.DestChannel = 0; // Don't care unless using IC->DMFC path
        dmfcData.PixelPerWord = IPU_DMFC_PPW_C_8BPP; // Only care for DC Read Ch or IC->DMFC path
       
        if(pConfigData->bVFIsSyncFlow)
        {
            dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_4;
            DMFCConfigure(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, &dmfcData);
            DpChannel = DP_CHANNEL_SYNC;
            m_bSYNCDPEnabled = TRUE;
            m_bSYNCisInterlaced = pConfigData->bVFIsInterlaced;
        }
        else
        {
            dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_6;
            DMFCConfigure(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, &dmfcData);
            DpChannel = DP_CHANNEL_ASYNC0;
            m_bASYNCDPEnabled = TRUE;
        }
        if(pConfigData->VFCSCEquation != CSCNoOp)
        {
            DPCSCGetCoeffs(DpChannel, &DpCSCConfigData);
            if (DpCSCConfigData.CSCPosition == DP_CSC_DISABLE)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_FG;
                DpCSCConfigData.CSCEquation = pConfigData->VFCSCEquation; 
                DpCSCConfigData.CSCCoeffs= pConfigData->VFCSCCoeffs;
                DpCSCConfigData.bGamutEnable = TRUE;
            }
            else if(DpCSCConfigData.CSCPosition == DP_CSC_BG)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BOTH;
            }
            DPCSCConfigure(DpChannel, &DpCSCConfigData);
        }
        DPConfigureInput(pConfigData->bVFIsSyncFlow, 
                         pConfigData->bVFIsInterlaced,
                         prpOutputChannel_VF);
        m_bConfigured = TRUE;

    }
    else if(pConfigData->bENCEnable)
    {
        //only enc path

         //configure enc path
        iENCFunctionFlag = 0;
        if(pConfigData->bInputIsInterlaced)
        {
            // for interlaced input, prp always should be first enabled module.
            iENCFunctionFlag |=PRP_FUNC_DOWNSIZE;
        }
        else if((pConfigData->inputIDMAChannel.FrameSize.width == 
            (m_PrpENCOutputWidth * m_PrpENCOutputHeight 
            / pConfigData->inputIDMAChannel.FrameSize.height)))
        {
            if(pConfigData->inputIDMAChannel.FrameSize.width > m_PrpENCOutputWidth)
                iENCFunctionFlag |=PRP_FUNC_DOWNSIZE;
            else if(pConfigData->inputIDMAChannel.FrameSize.width < m_PrpENCOutputWidth)
                iENCFunctionFlag |=PRP_FUNC_UPSIZE;
            else
                iENCFunctionFlag |=PRP_FUNC_NORESIZE;
        }
        else if(pConfigData->inputIDMAChannel.FrameSize.width > 
            (m_PrpENCOutputWidth * m_PrpENCOutputHeight 
            / pConfigData->inputIDMAChannel.FrameSize.height))
        {
            iENCFunctionFlag |=PRP_FUNC_DOWNSIZE;       
        }
        else
        {
            iENCFunctionFlag |=PRP_FUNC_UPSIZE;
        }

        
        if(((pConfigData->ENCFlipRot.verticalFlip ? 1 : 0) |
            (pConfigData->ENCFlipRot.horizontalFlip ? 1 : 0) << 1  |
            (pConfigData->ENCFlipRot.rotate90 ? 1 : 0) << 2))
        {
            iENCFunctionFlag |=PRP_FUNC_ROTATION;
        }
        if(bDisableNOICPath && (iENCFunctionFlag == 0))
        {
            iENCFunctionFlag |=PRP_FUNC_DOWNSIZE;
        }
        if(!PrpInitChannelParams(&pConfigData->inputIDMAChannel, &m_ENCchannelParams))
        {
             DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s: Incorrect input channel parameter. \r\n"), __WFUNCTION__ ));   
             return FALSE;            
        }
        if(iENCFunctionFlag & PRP_FUNC_DOWNSIZE)
        {        
            //CONFIGURE ENC CHANNEL
            
            m_bENCPrpEnabled = TRUE;
            if(m_PrpFirstModule == prpSourceType_ARM)
            {
                m_PrpFirstModule = prpSourceType_PRP;
            }
            
            if(m_PrpENCOutputHeight > PRP_MAX_OUTPUT_HEIGHT)
            {
                 RETAILMSG(1,
                     (TEXT("The output height is too large.(%d > 1024) \r\n"), m_PrpENCOutputHeight ));   
                 return FALSE;            
            } 
            if( m_PrpENCOutputWidth > PRP_MAX_OUTPUT_WIDTH)
            {
                m_bENCOversizedFrame = TRUE;
                m_ENCchannelParams.iWidth = m_ENCchannelParams.iWidth/2;
                m_PrpENCOutputWidth = m_PrpENCOutputWidth/2;

                m_OversizedInputOffsetStride = m_ENCchannelParams.iWidth* iENCinputbpp /8;
                if(iENCinputbpp == 8)
                   m_OversizedOutputOffsetStride = m_PrpENCOutputWidth* 2;  //convert 420 to 422
                else
                   m_OversizedOutputOffsetStride = m_PrpENCOutputWidth * iENCinputbpp /8;

            }

            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            PrpConfigureInput(prpInputChannel_MAIN, prpOutputChannel_ENC);

            //-----------------------------------------------------------------
            // Setup resizing
            //-----------------------------------------------------------------
            PrpConfigureResize(prpOutputChannel_ENC);

            //-----------------------------------------------------------------
            // Setup CSC parameter
            //-----------------------------------------------------------------
            //nocsc

            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            PrpConfigureOutput(&pConfigData->outputENCIDMAChannel,  prpOutputChannel_ENC, TRUE);

            if(m_bENCOversizedFrame)
            {
                m_PrpENCOutputWidth = m_PrpENCOutputWidth*2;
                m_ENCchannelParams.iWidth = m_PrpENCOutputWidth;
            }

            m_PrpENCLastModule = prpSourceType_PRP;
        }
        
        if(iENCFunctionFlag &PRP_FUNC_ROTATION)
        {
            m_bENCIRTEnabled = TRUE;
            if(m_PrpFirstModule == prpSourceType_ARM)
            {
                m_PrpFirstModule = prpSourceType_ROT;
            }
            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            IRTConfigureInput(&pConfigData->ENCFlipRot, prpOutputChannel_ENC);
            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            IRTConfigureOutput(&pConfigData->ENCFlipRot,  prpOutputChannel_ENC);
            
            //confgure IRT
            m_PrpENCLastModule  = prpSourceType_ROT;
        }
        
        if(iENCFunctionFlag & PRP_FUNC_UPSIZE)
        {        
            //CONFIGURE ENC CHANNEL
            
            m_bENCPrpEnabled = TRUE;
            if(m_PrpFirstModule == prpSourceType_ARM)
            {
                m_PrpFirstModule = prpSourceType_PRP;
            }
            
            if(m_PrpENCOutputHeight > PRP_MAX_OUTPUT_HEIGHT)
            {
                 RETAILMSG(1,
                     (TEXT("The output height is too large.(%d > 1024) \r\n"), m_PrpENCOutputHeight ));   
                 return FALSE;            
            }
            
            //For upsized case, if the required output width is too large to prp 
            //acceptable width, we must split it to two parts. For each part, 
            //the input and output width will be half of orignal one.
            //Then Prp can handle each of them.  
            UINT32 tempInputWidth=0,tempOutputWidth=0,tempResizeRatio=0,tempINW=0;
            if( m_PrpENCOutputWidth > PRP_MAX_OUTPUT_WIDTH)
            {
                m_bENCOversizedFrame = TRUE;
                tempInputWidth = m_ENCchannelParams.iWidth;
                tempOutputWidth = m_PrpENCOutputWidth;
                tempResizeRatio = 8192* (tempInputWidth-1) /(tempOutputWidth-1);
                tempINW = ((tempInputWidth-1)/2)&0xfffffffe;
                if(m_ENCchannelParams.iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
                {
                    tempINW = ((tempInputWidth-1)/2)&0xfffffff0;
                }
                m_ENCchannelParams.iWidth = (UINT16)((tempInputWidth-tempINW+7)&0xfffffff8);
                m_PrpENCOutputWidth = (UINT16)(m_PrpENCOutputWidth-(((8192*tempINW + tempResizeRatio-1)/tempResizeRatio+3)&0xfffffffc));
                m_OversizedInputOffsetStride = tempINW* iENCinputbpp /8;
 
                if(iENCinputbpp == 8)
                   m_OversizedOutputOffsetStride = (tempOutputWidth-m_PrpENCOutputWidth)* 2;  //convert 420 to 422
                else
                   m_OversizedOutputOffsetStride = (tempOutputWidth-m_PrpENCOutputWidth) * iENCinputbpp /8;
            }
            //-----------------------------------------------------------------
            // Setup input source
            //-----------------------------------------------------------------
            PrpConfigureInput(prpInputChannel_MAIN, prpOutputChannel_ENC);

            //-----------------------------------------------------------------
            // Setup resizing
            //-----------------------------------------------------------------
            PrpConfigureResize(prpOutputChannel_ENC);
            //Re-configure the coefficients according to oskar's algorithm
            if(m_bENCOversizedFrame)
            {
                PrpConfigureResize4OversizeWidth((UINT16)tempINW, (UINT16)(tempOutputWidth-m_PrpENCOutputWidth), prpOutputChannel_ENC);
            }

            //-----------------------------------------------------------------
            // Setup CSC parameter
            //-----------------------------------------------------------------
            //nocsc

            //-----------------------------------------------------------------
            // Setup output destination
            //-----------------------------------------------------------------
            PrpConfigureOutput(&pConfigData->outputENCIDMAChannel,  prpOutputChannel_ENC, TRUE);
            //After setting up prp's parameter , we should set the input and 
            //output width back, as the following sub-module DP should 
            //processing the whole frame in one time.
            if(m_bENCOversizedFrame)
            {
                m_PrpENCOutputWidth = (UINT16)tempOutputWidth;
                m_ENCchannelParams.iWidth = m_PrpENCOutputWidth;
            }

            m_PrpENCLastModule = prpSourceType_PRP;
        }
        
        dmfcData.FrameWidth = m_ENCchannelParams.iWidth;

        UINT32 iScale;
        if(pConfigData->bENCIsSyncFlow)
            iScale =2;
        else
            iScale =1;
            
        dmfcData.FrameWidth = m_ENCchannelParams.iWidth;
        if(dmfcData.FrameWidth > 1024)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_256x128;
        else if(dmfcData.FrameWidth*iScale >= 512)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_128x128;
        else if(dmfcData.FrameWidth*iScale >= 256)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_64x128;
        else if(dmfcData.FrameWidth*iScale >= 128)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128;
        else if(dmfcData.FrameWidth*iScale >= 64)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_16x128;
        else if(dmfcData.FrameWidth*iScale >= 32)
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_8x128;
        else 
            dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128;

        dmfcData.FrameHeight = m_ENCchannelParams.iHeight;
        dmfcData.BurstSize = IPU_DMFC_BURST_SIZE_0_4_WORDS;
        dmfcData.WaterMarkEnable = IPU_DMFC_WM_EN_DISABLE;
        
        // Don't care values
        dmfcData.WaterMarkClear = 1; // Make the value the same as default value.
        dmfcData.WaterMarkSet = 0; // Don't care if Watermark disabled
        dmfcData.DestChannel = 0; // Don't care unless using IC->DMFC path
        dmfcData.PixelPerWord = IPU_DMFC_PPW_C_8BPP; // Only care for DC Read Ch or IC->DMFC path

        if(pConfigData->bENCIsSyncFlow)
        {
            dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_4;
            DMFCConfigure(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, &dmfcData);
            DpChannel = DP_CHANNEL_SYNC;
            m_bSYNCDPEnabled = TRUE;
            m_bSYNCisDI1= TRUE;
            m_bSYNCisInterlaced = pConfigData->bENCIsInterlaced;
        }
        else
        {
            dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_6;
            DMFCConfigure(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, &dmfcData);
            DpChannel = DP_CHANNEL_ASYNC0;
            m_bASYNCDPEnabled = TRUE;
        }

        if(pConfigData->ENCCSCEquation != CSCNoOp)
        {
            DPCSCGetCoeffs(DpChannel, &DpCSCConfigData);
            if (DpCSCConfigData.CSCPosition == DP_CSC_DISABLE)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_FG;
                DpCSCConfigData.CSCEquation = pConfigData->ENCCSCEquation; 
                DpCSCConfigData.CSCCoeffs= pConfigData->ENCCSCCoeffs;
                DpCSCConfigData.bGamutEnable = TRUE;
            }
            else if(DpCSCConfigData.CSCPosition == DP_CSC_BG)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BOTH;
            }
            DPCSCConfigure(DpChannel, &DpCSCConfigData);

        }
        DPConfigureInput(pConfigData->bENCIsSyncFlow, 
                         pConfigData->bENCIsInterlaced,
                         prpOutputChannel_ENC);
        m_bConfigured = TRUE;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Incorrect channel enable. VF output channel: %d;\
                        Enc output channel: %d; Combine input VFchannel\r\n"),
                        __WFUNCTION__, pConfigData->bVFEnable, 
                        pConfigData->bENCEnable, pConfigData->bCombineEnable));
    }

    PRP_FUNCTION_EXIT();
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpConfigureCombine
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
void PrpClass::PrpConfigureCombine(UINT8 alpha, UINT32 colorkey)    
{
    //always use global alpha
    INSREG32BF(&m_pIC->IC_CONF, 
                    IPU_IC_CONF_IC_GLB_LOC_A, 
                    IPU_IC_CONF_IC_GLB_LOC_A_GLOBAL);

    INSREG32BF(&m_pIC->IC_CMBP_1, IPU_IC_CMBP_1_IC_PRPVF_ALPHA_V, alpha);

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
                    IPU_IC_CONF_IC_KEY_COLOR_EN_ENABLE);
    }

    // Now enable CMB in IC configuration register
    INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PRPVF_CMB,
        IPU_IC_CONF_PRPVF_CMB_ENABLE);   
        
    return;
}

//-----------------------------------------------------------------------------
//
// Function: PrpConfigureResize
//
// This function configures the IC parameters for resizing feature.
//
// Parameters:
//      pChannelParams:
//          [in] the pointer to IDMAC Channel parameters.
//
//      outputChannel:
//          [in] the type of output channel, viewfinder or encoder.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureResize(prpOutputChannel outputChannel)
{
    UINT16 iResizeOutputHeight, iResizeOutputWidth;
    prpResizeCoeffs resizeCoeffs;
    pPrpIDMACChannelParams pIDMAChannelParams;

    if(outputChannel == prpOutputChannel_ENC)
    {
        iResizeOutputHeight = m_PrpENCOutputHeight;
        iResizeOutputWidth = m_PrpENCOutputWidth;
        pIDMAChannelParams = &m_ENCchannelParams;
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        iResizeOutputHeight = m_PrpVFOutputHeight;
        iResizeOutputWidth = m_PrpVFOutputWidth;
        pIDMAChannelParams = &m_VFchannelParams;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: outputChannel type error(%d). \r\n"), __WFUNCTION__, outputChannel));
        return FALSE;        
    }

    // Vertical resizing
    // Get coefficients and then set registers
    if (!PrpGetResizeCoeffs(pIDMAChannelParams->iHeight,
        iResizeOutputHeight, &resizeCoeffs))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Pre-processing viewfinding vertical resizing failed. \r\n"), __WFUNCTION__));
        return FALSE;
    }
    
    if(outputChannel == prpOutputChannel_ENC)
    {
        // Set downsizing field
        INSREG32BF(&m_pIC->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_V,
            resizeCoeffs.downsizeCoeff);

        // Set resizing field
        INSREG32BF(&m_pIC->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_V,
            resizeCoeffs.resizeCoeff);

        INSREG32BF(&m_pIC->IC_IDMAC_2, IPU_IC_IDMAC_2_T1_FR_HEIGHT,iResizeOutputHeight -1);
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        // Set downsizing field
        INSREG32BF(&m_pIC->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_DS_R_V,
            resizeCoeffs.downsizeCoeff);

        // Set resizing field
        INSREG32BF(&m_pIC->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_RS_R_V,
            resizeCoeffs.resizeCoeff);        

        INSREG32BF(&m_pIC->IC_IDMAC_2, IPU_IC_IDMAC_2_T2_FR_HEIGHT,iResizeOutputHeight -1);
    }
    // Horizontal resizing
    // Get coefficients and then set registers
    if (!PrpGetResizeCoeffs(pIDMAChannelParams->iWidth,
        iResizeOutputWidth, &resizeCoeffs))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Pre-processing viewfinding horizontal resizing failed. \r\n"), __WFUNCTION__));
        return FALSE;
    }
    if(outputChannel == prpOutputChannel_ENC)
    {
        // Set downsizing field
        INSREG32BF(&m_pIC->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H,
            resizeCoeffs.downsizeCoeff);

        // Set resizing field
        INSREG32BF(&m_pIC->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_H,
            resizeCoeffs.resizeCoeff);
            
        INSREG32BF(&m_pIC->IC_IDMAC_3, IPU_IC_IDMAC_3_T1_FR_WIDTH,iResizeOutputWidth -1);
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        // Set downsizing field
        INSREG32BF(&m_pIC->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H,
            resizeCoeffs.downsizeCoeff);

        // Set resizing field
        INSREG32BF(&m_pIC->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_RS_R_H,
            resizeCoeffs.resizeCoeff);        

        INSREG32BF(&m_pIC->IC_IDMAC_3, IPU_IC_IDMAC_3_T2_FR_WIDTH,iResizeOutputWidth -1);
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpConfigureResize4OversizeWidth
//
// This function computes the resizing coefficients from
// the input and output size for oversized width.
//
// Parameters:
//      inSize
//          [in] Input size (height or width)
//
//      outSize
//          [in] Output size (height of width)
//
//      outputChannel:
//          [in] the type of output channel, viewfinder or encoder.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureResize4OversizeWidth(UINT16 inSize, UINT16 outSize, prpOutputChannel outputChannel)
{
    UINT16 tempSize;
    UINT16 tempDownsize;
    UINT16 tempOutSize;
    UINT16 downsizeCoeff;
    UINT16 resizeCoeff;

    PRP_FUNCTION_ENTRY();

    // TODO: Need check how to get 8:1 downsize ratio
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

    downsizeCoeff = tempDownsize;

    // compute resizing coefficient using the following equation:
    resizeCoeff =  8192 * tempSize / outSize;

    if(resizeCoeff > 0x3fff)
        resizeCoeff =0x3fff;
        
    if(outputChannel == prpOutputChannel_ENC)
    {
        // Set downsizing field
        INSREG32BF(&m_pIC->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H,
            downsizeCoeff);

        // Set resizing field
        INSREG32BF(&m_pIC->IC_PRP_ENC_RSC, IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_H,
            resizeCoeff);
            
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        // Set downsizing field
        INSREG32BF(&m_pIC->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H,
            downsizeCoeff);

        // Set resizing field
        INSREG32BF(&m_pIC->IC_PRP_VF_RSC, IPU_IC_PRP_VF_RSC_PRPVF_RS_R_H,
            resizeCoeff);        

    }

    return TRUE;

}



//-----------------------------------------------------------------------------
//
// Function: PrpConfigureCSC
//
// This function configures the IC parameters for color space conversion feature.
//
// Parameters:
//      CSCEquation
//          [in] the type of csc equation.
//
//      pCustCSCCoeffs
//          [in] the customized csc coefficients, it will be used when 
//                the type of csc equation equals to icCSCCustom. 
//
//      outputChannel:
//          [in] the type of output channel, viewfinder or encoder.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpConfigureCSC(CSCEQUATION CSCEquation, icCSCMatrix CSCMatrix, 
                    pIcCSCCoeffs pCustCSCCoeffs,  prpOutputChannel outputChannel)
{
    //icCSCCoeffs CSCCoeffs;
    TPMConfigData TPMData;
    // Set up CSC for pre-processing only for vf path
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

        default:
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Invalid PrP CSC equation. \r\n"), __WFUNCTION__));
    }
    if(CSCMatrix == icCSC1)
    {
        if(outputChannel == prpOutputChannel_ENC)
        {
            TPMData.tpmMatrix = TPM_CHANNEL_ENC_CSC1_MATRIX1;
            TPMWrite(&TPMData);

            // Now enable CSC in IC configuration register
            INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PRPENC_CSC1,
                        IPU_IC_CONF_PRPENC_CSC1_ENABLE);

        }
        else
        {
            TPMData.tpmMatrix = TPM_CHANNEL_VF_CSC1_MATRIX1;
            TPMWrite(&TPMData);

            // Now enable CSC in IC configuration register
            INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PRPVF_CSC1,
                        IPU_IC_CONF_PRPVF_CSC1_ENABLE);                
        }
    }
    else
    {
        TPMData.tpmMatrix = TPM_CHANNEL_VF_CSC1_MATRIX2;
        TPMWrite(&TPMData);
        // Now enable CSC in IC configuration register
        INSREG32BF(&m_pIC->IC_CONF, IPU_IC_CONF_PRPVF_CSC2,
            IPU_IC_CONF_PRPVF_CSC2_ENABLE);        
    }
    return;
}

//-----------------------------------------------------------------------------
//
// Function: PrpPixelFormatSetting
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
//      pChannelParams:
//          [in] the pointer to IDMAC channel parameter.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpPixelFormatSetting(icFormat iFormat, icPixelFormat PixelFormat,
                                            icDataWidth iWidth,pPrpIDMACChannelParams pChannelParams)
{
    pChannelParams->iBitsPerPixelCode = CPMEM_BPP_8;   //BY DEFAULT
    pChannelParams->pixelFormat.component0_offset = PixelFormat.component0_offset;
    pChannelParams->pixelFormat.component1_offset = PixelFormat.component1_offset;
    pChannelParams->pixelFormat.component2_offset = PixelFormat.component2_offset;
    pChannelParams->pixelFormat.component3_offset = PixelFormat.component3_offset;
    pChannelParams->pixelFormat.component0_width = PixelFormat.component0_width;
    pChannelParams->pixelFormat.component1_width = PixelFormat.component1_width;
    pChannelParams->pixelFormat.component2_width = PixelFormat.component2_width;
    pChannelParams->pixelFormat.component3_width = PixelFormat.component3_width;

    pChannelParams->iPixelBurstCode = CPMEM_PIXEL_BURST_16;
    switch(iFormat)
    {
        case icFormat_YUV444:
            pChannelParams->bInterleaved = FALSE;
            pChannelParams->iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV444;
            break;

        case icFormat_YUV422:
            pChannelParams->bInterleaved = FALSE;
            pChannelParams->iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422;
            break;

        case icFormat_YUV420:
            pChannelParams->bInterleaved = FALSE;
            pChannelParams->iFormatCode = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420;
            break;
            
        case icFormat_YUV422P:
            pChannelParams->bInterleaved = FALSE;
            pChannelParams->iFormatCode = IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV422;
            break;
        
        case icFormat_YUV420P:
            pChannelParams->bInterleaved = FALSE;
            pChannelParams->iFormatCode = IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420;
            break;

        case icFormat_YUV444IL:
            pChannelParams->bInterleaved = TRUE;
            pChannelParams->iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUV444;
            pChannelParams->iBitsPerPixelCode = CPMEM_BPP_24;
            break;

        case icFormat_YUYV422:
            pChannelParams->bInterleaved = TRUE;
            pChannelParams->iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUY2V;
            pChannelParams->iBitsPerPixelCode = CPMEM_BPP_16;
            break;

        case icFormat_YVYU422:
            pChannelParams->bInterleaved = TRUE;
            pChannelParams->iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_YUY2V;
            pChannelParams->iBitsPerPixelCode = CPMEM_BPP_16;
            break;

        case icFormat_UYVY422:
            pChannelParams->bInterleaved = TRUE;
            pChannelParams->iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_UYVY2;
            pChannelParams->iBitsPerPixelCode = CPMEM_BPP_16;
            break;

        case icFormat_VYUY422:
            pChannelParams->bInterleaved = TRUE;
            pChannelParams->iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_UYVY2;
            pChannelParams->iBitsPerPixelCode = CPMEM_BPP_16;
            break;

        case icFormat_RGB:
            switch (iWidth)
            {
                case icDataWidth_8BPP:
                    pChannelParams->iBitsPerPixelCode = CPMEM_BPP_8;
                    break;

                case icDataWidth_16BPP:
                    pChannelParams->iBitsPerPixelCode = CPMEM_BPP_16;
                    break;

                case icDataWidth_24BPP:
                    pChannelParams->iBitsPerPixelCode = CPMEM_BPP_24;
                    break;

                case icDataWidth_32BPP:
                    pChannelParams->iBitsPerPixelCode = CPMEM_BPP_32;
                    break;

                default:
                    DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Error PrP data width, %d !! \r\n"), __WFUNCTION__, pChannelParams->iBitsPerPixelCode));
                    return  FALSE;
            }

            pChannelParams->bInterleaved = TRUE;
            pChannelParams->iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;

            break;

        case icFormat_RGBA:
            // TODO: Add support for RGBA, and find out data path and use cases for RGBA
            // 32 bits per pixel for RGB data
            pChannelParams->bInterleaved = TRUE;
            pChannelParams->iFormatCode = IDMAC_INTERLEAVED_FORMAT_CODE_RGB;
            pChannelParams->iBitsPerPixelCode = CPMEM_BPP_32;
            break;

        default:
            DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Invalid PrP input format, %d !! \r\n"), __WFUNCTION__, iFormat));
            return  FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: PrpInitChannelParams
//
// This function initialize IDMA channel parameter structure which is used for configure IDMA 
//  channel, according to the input IDMA channel data.
//
// Parameters:
//      pIDMAChannel
//          [in] the pointer to IDMA channel structure, 
//
//      pIDMAChannelParams
//          [out] the pointer to IDMA channel parameter structure, 
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpInitChannelParams(pIdmaChannel pIDMAChannel, pPrpIDMACChannelParams pIDMAChannelParams)
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
    pIDMAChannelParams->iRotation90 = 0;
    pIDMAChannelParams->iFlipHoriz = 0;
    pIDMAChannelParams->iFlipVert = 0;
    pIDMAChannelParams->iBlockMode = 0;
    pIDMAChannelParams->iBandMode = CPMEM_BNDM_DISABLE;
    pIDMAChannelParams->bInterlaced = FALSE;
    

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

    //-----------------------------------------------------------------
    // Setup input format
    //-----------------------------------------------------------------
    result = PrpPixelFormatSetting(iFormat, 
                                            pIDMAChannel->PixelFormat,
                                            pIDMAChannel->DataWidth,
                                            pIDMAChannelParams);
    if(result == FALSE)
    {return FALSE;}

    //-----------------------------------------------------------------
    // Image size validity check
    // Setup input image size
    //-----------------------------------------------------------------

    // Boundary check
    if((iInputWidth  > PRP_MAX_INPUT_WIDTH) ||
        (iInputHeight > PRP_MAX_INPUT_HEIGHT) ||
        (iInputWidth  < PRP_MIN_INPUT_WIDTH) ||
        (iInputHeight < PRP_MIN_INPUT_HEIGHT))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error input size: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Input Size: width (%d), height (%d)\r\n"),
                iInputWidth, iInputHeight));
        return FALSE;
    }

    // Alignment check
    if((iFormat == icFormat_YUV420)||(iFormat == icFormat_YUV420P))
    {
        if((iInputWidth & 0x07) ||
            (iInputHeight & 0x01))
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                    iInputWidth, iInputHeight));
            return FALSE;
        }
    }
    else
    {
        if(iInputWidth & 0x03)
        {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                    iInputWidth, iInputHeight));
            return FALSE;
        }
    }

    // Set channel parameters for frame height and width
    pIDMAChannelParams->iHeight = iInputHeight;
    pIDMAChannelParams->iWidth = iInputWidth;
    pIDMAChannelParams->iLineStride = iInputStride;
    if (m_bCurrentPlanar)
    {
        pIDMAChannelParams->UOffset = pIDMAChannel->UBufOffset;
        pIDMAChannelParams->VOffset = pIDMAChannel->VBufOffset;
    }   

    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: DPConfigureInput
//
// This function configures the input channel of display processor module, meanwhile it also
// allocates the middle buffer and configures the destination of previous module.
//
// Parameters:
//      bIsSyncFlow
//          [in] TRUE: configure synchronous flow.
//                FALSE: configure asynchronous flow.
//
//      bIsInterlaced
//          [in] TRUE: configure interlaced mode (already for tv)
//                FALSE: configure progressive mode.
//
//      outputChannel:
//          [in] the type of output channel, viewfinder or encoder.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::DPConfigureInput(BOOL bIsSyncFlow, BOOL bIsInterlaced, prpOutputChannel outputChannel)
{
    BOOL result = TRUE;
    UINT16 iInputWidth, iInputHeight;
    UINT32 iInputStride;
    prpSourceType LastModule;
    pPrpIDMACChannelParams pIDMAChannelParams;
    CPMEMBufOffsets OffsetData;

    memset(&OffsetData,0,sizeof(CPMEMBufOffsets));

    PRP_FUNCTION_ENTRY();

    if(outputChannel == prpOutputChannel_ENC)
    {
        LastModule = m_PrpENCLastModule;
        pIDMAChannelParams = &m_ENCchannelParams;
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        LastModule = m_PrpVFLastModule;
        pIDMAChannelParams = &m_VFchannelParams;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error input type, outputChannel: %d ! \r\n"), __WFUNCTION__,
                outputChannel));   
        result = FALSE;
        goto _DPinputDone;
    }
    
    pIDMAChannelParams->iBlockMode = 0;
    pIDMAChannelParams->iBandMode = CPMEM_BNDM_DISABLE;
    pIDMAChannelParams->bInterlaced = bIsInterlaced;

    iInputHeight = pIDMAChannelParams->iHeight;
    iInputWidth = pIDMAChannelParams->iWidth;
    iInputStride = pIDMAChannelParams->iLineStride;


    if (m_bCurrentPlanar)
    {
        OffsetData.bInterleaved = FALSE;
        OffsetData.uOffset= pIDMAChannelParams->UOffset;
        OffsetData.vOffset = pIDMAChannelParams->VOffset;
        OffsetData.interlaceOffset= 0;
    }
        
    if(bIsSyncFlow)
    {
        // Clear the bit to disable DMFC direct path.
        CMSetPathIC2DP(FALSE, FALSE);

        if(LastModule == prpSourceType_ARM)
        {
            CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
                                IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_MCU);
        }
        else if(LastModule == prpSourceType_PRP)
        {
            if(outputChannel == prpOutputChannel_ENC)
            {
                if(m_bENCOversizedFrame)
                {
                    //For oversized frame, we can't use automatic chain.
                    CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
                        IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_MCU);
                }
                else
                {
                    CMSetProcFlow(IPU_PROC_FLOW_PRPENC_DEST,
                                        IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_SYNC1);

                    CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
                                        IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_ENCODING);
                }
            }
            else
            {
                if(m_bVFOversizedFrame)
                {
                    //For oversized frame, we can't use automatic chain.
                    CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
                        IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_MCU);
                }
                else
                {

                    CMSetProcFlow(IPU_PROC_FLOW_PRPVF_DEST,
                                        IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_SYNC1);

                    CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
                                        IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_VIEWFINDER);
                }
            }
           
            // allocate ROTATION middle buffer
            IPUBufferInfo IPUBufInfo;
            UINT32 dpAddr;
            //for caculate correct buffer size
            //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
            if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
            ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 3;
            else
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 2;
                
            IPUBufInfo.MemType = MEM_TYPE_VIDEOMEMORY;


            if(outputChannel == prpOutputChannel_ENC)
            {
                if(m_hENCDpBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hENCDpBuffer);
                //get the space
                m_hENCDpBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hENCDpBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hENCDpBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get ENCRotBuffer ! \r\n"), __WFUNCTION__));   
                    result = FALSE;
                    goto _DPinputDone;                    
                }

                dpAddr = (UINT32)m_hENCDpBuffer->PhysAddr();
                
                CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, 0, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 0, (UINT32 *)dpAddr);
                //For oversized frame the output buffer ready flag will be set in function PrpAddInputBuffer()
                if(!m_bENCOversizedFrame)
                    IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_ENC);

                dpAddr += IPUBufInfo.dwBufferSize/2;
                CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, 1, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 1, (UINT32 *)dpAddr);
                //For oversized frame the output buffer ready flag will be set in function PrpAddInputBuffer()
                if(!m_bENCOversizedFrame)
                    IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_ENC);

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_ENC,
                                     &OffsetData, OffsetData.bInterleaved);
            }
            else
            {
                if(m_hVFDpBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hVFDpBuffer);
                //get the space
                m_hVFDpBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hVFDpBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hVFDpBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get VFRotBuffer ! \r\n"), __WFUNCTION__));   
                    result = FALSE;
                    goto _DPinputDone;                    
                }

                dpAddr = (UINT32)m_hVFDpBuffer->PhysAddr();
                                       
                CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, 0, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 0, (UINT32 *)dpAddr);
                //For oversized frame the output buffer ready flag will be set in function PrpAddInputBuffer()
                if(!m_bVFOversizedFrame)                
                    IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);

                dpAddr += IPUBufInfo.dwBufferSize/2;
                CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, 1, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 1, (UINT32 *)dpAddr);
                //For oversized frame the output buffer ready flag will be set in function PrpAddInputBuffer()
                if(!m_bVFOversizedFrame) 
                    IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_VF);

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_VF,
                                     &OffsetData, OffsetData.bInterleaved);
            }   

        }  
        else if(LastModule == prpSourceType_ROT)
        {
            if(outputChannel == prpOutputChannel_ENC)
            {
                CMSetProcFlow(IPU_PROC_FLOW_PRPENC_ROT_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_SYNC1);

                CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
                                    IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IRT_ENCODING);
            }
            else
            {
                CMSetProcFlow(IPU_PROC_FLOW_PRPVF_ROT_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_SYNC1);

                CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
                                    IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IRT_VIEWFINDER);
            }


            // allocate ROTATION middle buffer
            IPUBufferInfo IPUBufInfo;
            UINT32 dpAddr;
            //for caculate correct buffer size
            //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
            if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
                ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
                 IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 3;
            else
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 2;

            IPUBufInfo.MemType = MEM_TYPE_VIDEOMEMORY;

            if(outputChannel == prpOutputChannel_ENC)
            {
                if(m_hENCDpBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hENCDpBuffer);
                //get the space
                m_hENCDpBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hENCDpBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hENCDpBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get ENCDpBuffer ! \r\n"), __WFUNCTION__));   
                    result = FALSE;
                    goto _DPinputDone;                    
                }

                dpAddr = (UINT32)m_hENCDpBuffer->PhysAddr();
                
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_ENC, 0, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 0, (UINT32 *)dpAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);

                dpAddr += IPUBufInfo.dwBufferSize/2;
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_ENC, 1, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 1, (UINT32 *)dpAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);

                if (m_bCurrentPlanar) 
                   CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_ENC,
                                    &OffsetData, OffsetData.bInterleaved);
            }
            else
            {
                if(m_hVFDpBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hVFDpBuffer);
                //get the space
                m_hVFDpBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hVFDpBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hVFDpBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get VFDpBuffer ! \r\n"), __WFUNCTION__));   
                    result = FALSE;
                    goto _DPinputDone;                    
                }

                dpAddr = (UINT32)m_hVFDpBuffer->PhysAddr();
                                       
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_VF, 0, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 0, (UINT32 *)dpAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);

                dpAddr += IPUBufInfo.dwBufferSize/2;
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_VF, 1, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 1, (UINT32 *)dpAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_VF,
                                     &OffsetData, OffsetData.bInterleaved);
            }           
        }     
        else
        {
                DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Error input type, m_PrpLastModule: %d ! \r\n"), __WFUNCTION__,
                        LastModule));   
                result = FALSE;
                goto _DPinputDone;
        }
        //Set the burst size to 16, if the width is large enough.
        pIDMAChannelParams->iPixelBurstCode = 
            (UINT8)((pIDMAChannelParams->iWidth >= (CPMEM_PIXEL_BURST_16+1))? CPMEM_PIXEL_BURST_16 : (pIDMAChannelParams->iWidth - 1));
        if (m_bCurrentPlanar||bIsInterlaced)
        {
            if(bIsInterlaced)
                OffsetData.interlaceOffset = iInputStride;
            CPMEMWriteOffset(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE,
                             &OffsetData, OffsetData.bInterleaved);
        }
        PrpIDMACChannelConfig(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, pIDMAChannelParams);

        
    }
    else //async flow
    {
        // Clear the bit to disable DMFC direct path.
        CMSetPathIC2DP(FALSE, FALSE);

        if(LastModule == prpSourceType_ARM)
        {
            CMSetDispFlow(IPU_DISP_FLOW_DP_ASYNC0_SRC,
                                IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_MCU);
        }
        else if(LastModule == prpSourceType_PRP)
        {
            if(outputChannel == prpOutputChannel_ENC)
            {
                CMSetProcFlow(IPU_PROC_FLOW_PRPENC_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_ASYNC0);

                CMSetDispFlow(IPU_DISP_FLOW_DP_ASYNC0_SRC,
                                    IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_ENCODING);
            }
            else
            {
#if DMFC_DIRECT_PATH
                //use dmfc INSTEAD OF middle memory
                m_bDMFCChannelUsing = TRUE;
                
                // Set the bit to enable DMFC direct path.
                CMSetPathIC2DP(TRUE, FALSE);
#endif
                CMSetProcFlow(IPU_PROC_FLOW_PRPVF_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_ASYNC0);

                CMSetDispFlow(IPU_DISP_FLOW_DP_ASYNC0_SRC,
                                    IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_VIEWFINDER);

                //No need allocate memory
                //DMFC_IC_IN PORT in register DMFC_IC_CTRL should be 0x7 ch29                 
                //IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);
            }

            // allocate ROTATION middle buffer
            IPUBufferInfo IPUBufInfo;
            UINT32 dpAddr;
            //for caculate correct buffer size
            //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
            if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
              ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 3;
            else
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 2;
                
            IPUBufInfo.MemType = MEM_TYPE_VIDEOMEMORY;


            if(outputChannel == prpOutputChannel_ENC)
            {
                if(m_hENCDpBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hENCDpBuffer);
                //get the space
                m_hENCDpBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hENCDpBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hENCDpBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get ENCDpBuffer ! \r\n"), __WFUNCTION__));   
                    result = FALSE;
                    goto _DPinputDone;                    
                }

                dpAddr = (UINT32)m_hENCDpBuffer->PhysAddr();
                
                CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, 0, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, 0, (UINT32 *)dpAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_ENC);

                dpAddr += IPUBufInfo.dwBufferSize/2;
                CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, 1, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, 1, (UINT32 *)dpAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_ENC);

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_ENC,
                                     &OffsetData, OffsetData.bInterleaved);
            }
            else
            {
#if DMFC_DIRECT_PATH
                //No need allocate memory
                //DMFC_IC_IN PORT in register DMFC_IC_CTRL should be 0x7 ch29
                
                //set DMFC WIDTH AND HEIGHT
                dmfcConfigData dmfcConfig;
                dmfcConfig.FrameHeight = iInputHeight-1;
                dmfcConfig.FrameWidth = iInputWidth-1;
                dmfcConfig.DestChannel = IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE;
                switch(pIDMAChannelParams->iBitsPerPixelCode)
                {
                    case CPMEM_BPP_8:
                        dmfcConfig.PixelPerWord=0;
                    break;
                    case CPMEM_BPP_16:
                        dmfcConfig.PixelPerWord =1;
                    break;

                    case CPMEM_BPP_24:
                    case CPMEM_BPP_32:
                        dmfcConfig.PixelPerWord =2;
                    break;
                    default:
                        dmfcConfig.PixelPerWord =0;
                    break;
                    
                }

                // Don't care values
                dmfcConfig.WaterMarkClear = 1; // Don't care if Watermark disabled
                dmfcConfig.WaterMarkSet = 0; // Don't care if Watermark disabled
                dmfcConfig.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128;
                dmfcConfig.BurstSize = IPU_DMFC_BURST_SIZE_0_4_WORDS;
                dmfcConfig.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_6;
                dmfcConfig.WaterMarkEnable = IPU_DMFC_WM_EN_DISABLE;

                
                DMFCConfigure(IDMAC_CH_PRP_OUTPUT_VF,&dmfcConfig);
                
                //IDMACChannelDBMODE(IDMAC_CH_PRP_OUTPUT_VF,TRUE);
                if(!m_bDMFCChannelUsing)                       
                {
                    IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);
                    IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_VF);
                }

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_VF,
                                     &OffsetData, OffsetData.bInterleaved);
#else
                if(m_hVFDpBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hVFDpBuffer);
                //get the space
                m_hVFDpBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hVFDpBuffer);

                //Physical address needn't CEOpenCallerBuffer().
                if(m_hVFDpBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get VFDpBuffer ! \r\n"), __WFUNCTION__));   
                    result = FALSE;
                    goto _DPinputDone;                    
                }

                dpAddr = (UINT32)m_hVFDpBuffer->PhysAddr();

                CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, 0, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, 0, (UINT32 *)dpAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);

                dpAddr += IPUBufInfo.dwBufferSize/2;
                CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, 1, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, 1, (UINT32 *)dpAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_VF);

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_VF,
                                     &OffsetData, OffsetData.bInterleaved);

#endif
            } 

        }  
        else if(LastModule == prpSourceType_ROT)
        {
            if(outputChannel == prpOutputChannel_ENC)
            {
                CMSetProcFlow(IPU_PROC_FLOW_PRPENC_ROT_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_ASYNC0);

                CMSetDispFlow(IPU_DISP_FLOW_DP_ASYNC0_SRC,
                                    IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IRT_ENCODING);
            }
            else
            {
                CMSetProcFlow(IPU_PROC_FLOW_PRPVF_ROT_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_ASYNC0);

                CMSetDispFlow(IPU_DISP_FLOW_DP_ASYNC0_SRC,
                                    IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IRT_VIEWFINDER);
            }

            // allocate ROTATION middle buffer
            IPUBufferInfo IPUBufInfo;
            UINT32 dpAddr;
            //for caculate correct buffer size
            //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
            if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
              ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 3;
            else
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 2;
                
            IPUBufInfo.MemType = MEM_TYPE_VIDEOMEMORY;

            if(outputChannel == prpOutputChannel_ENC)
            {
                if(m_hENCDpBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hENCDpBuffer);
                //get the space
                m_hENCDpBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hENCDpBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hENCDpBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get ENCDpBuffer ! \r\n"), __WFUNCTION__));   
                    result = FALSE;
                    goto _DPinputDone;                    
                }

                dpAddr = (UINT32)m_hENCDpBuffer->PhysAddr();
                
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_ENC, 0, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, 0, (UINT32 *)dpAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);

                dpAddr += IPUBufInfo.dwBufferSize/2;
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_ENC, 1, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, 1, (UINT32 *)dpAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);

                if (m_bCurrentPlanar) 
                   CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_ENC,
                                    &OffsetData, OffsetData.bInterleaved);
            }
            else
            {
                if(m_hVFDpBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hVFDpBuffer);
                //get the space
                m_hVFDpBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hVFDpBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hVFDpBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get VFDpBuffer ! \r\n"), __WFUNCTION__));   
                    result = FALSE;
                    goto _DPinputDone;                    
                }

                dpAddr = (UINT32)m_hVFDpBuffer->PhysAddr();
                                       
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_VF, 0, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, 0, (UINT32 *)dpAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);

                dpAddr += IPUBufInfo.dwBufferSize/2;
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_VF, 1, (UINT32 *)dpAddr);
                CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, 1, (UINT32 *)dpAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_VF,
                                     &OffsetData, OffsetData.bInterleaved);
            }           
        }     
        else
        {
                DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Error input type, m_PrpLastModule: %d ! \r\n"), __WFUNCTION__,
                        LastModule));   
                result = FALSE;
                goto _DPinputDone;
        }

        //Set the burst size to 16, if the width is large enough.
        pIDMAChannelParams->iPixelBurstCode = 
            (UINT8)((pIDMAChannelParams->iWidth >= (CPMEM_PIXEL_BURST_16+1))? CPMEM_PIXEL_BURST_16 : (pIDMAChannelParams->iWidth - 1));



        if (m_bCurrentPlanar||bIsInterlaced)
        {
            if(bIsInterlaced)
                OffsetData.interlaceOffset = iInputStride;
            CPMEMWriteOffset(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE,
                             &OffsetData, OffsetData.bInterleaved);
        }
        PrpIDMACChannelConfig(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, pIDMAChannelParams);
    }

_DPinputDone:
    PRP_FUNCTION_EXIT();
    return result;    
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
//      outputChannel:
//          [in] the type of output channel, viewfinder or encoder.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::IRTConfigureInput(pIcFlipRot pFlipRot,prpOutputChannel outputChannel)
{
    BOOL result = TRUE;
    UINT16 iInputWidth, iInputHeight;
    UINT32 iInputStride;
    prpSourceType LastModule;
    pPrpIDMACChannelParams pIDMAChannelParams;
    CPMEMBufOffsets OffsetData;
    
    memset(&OffsetData,0,sizeof(CPMEMBufOffsets));
    PRP_FUNCTION_ENTRY();

    if(outputChannel == prpOutputChannel_ENC)
    {
        LastModule = m_PrpENCLastModule;
        pIDMAChannelParams = &m_ENCchannelParams;
        INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T1_FLIP_UD,
            (pFlipRot->verticalFlip ? 1 : 0));
        INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T1_FLIP_LR,
            (pFlipRot->horizontalFlip ? 1 : 0));
        INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T1_ROT,
            (pFlipRot->rotate90 ? 1 : 0));
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        LastModule = m_PrpVFLastModule;
        pIDMAChannelParams = &m_VFchannelParams;
        INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T2_FLIP_UD,
            (pFlipRot->verticalFlip ? 1 : 0));
        INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T2_FLIP_LR,
            (pFlipRot->horizontalFlip ? 1 : 0));
        INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_T2_ROT,
            (pFlipRot->rotate90 ? 1 : 0));
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error input type, outputChannel: %d ! \r\n"), __WFUNCTION__,
                outputChannel));   
        result = FALSE;
        goto _IRTinputDone;
    }

    pIDMAChannelParams->iRotation90 = pFlipRot->rotate90 ? 1 : 0;
    pIDMAChannelParams->iFlipHoriz = pFlipRot->horizontalFlip ? 1 : 0;
    pIDMAChannelParams->iFlipVert = pFlipRot->verticalFlip ? 1 : 0;    
    pIDMAChannelParams->iBlockMode = 1;
    pIDMAChannelParams->iBandMode = CPMEM_BNDM_DISABLE;


    iInputHeight = pIDMAChannelParams->iHeight;
    iInputWidth = pIDMAChannelParams->iWidth;
    iInputStride = pIDMAChannelParams->iLineStride;

    if(LastModule == prpSourceType_ARM)
    {
        if(outputChannel == prpOutputChannel_ENC)
        {
            CMSetProcFlow(IPU_PROC_FLOW_PRPENC_ROT_SRC,
                                        IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);
        }
        else
        {
            CMSetProcFlow(IPU_PROC_FLOW_PRPVF_ROT_SRC,
                                        IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);
        }

        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;
            OffsetData.uOffset= pIDMAChannelParams->UOffset;
            OffsetData.vOffset = pIDMAChannelParams->VOffset;
            OffsetData.interlaceOffset= 0;
        }

    }
    else if(LastModule == prpSourceType_PRP)
    {
        if(outputChannel == prpOutputChannel_ENC)
        {
            CMSetProcFlow(IPU_PROC_FLOW_PRPENC_ROT_SRC,
                                        IPU_IPU_FS_PROC_FLOW1_SRC_SEL_ENCODING);
                                        
            CMSetProcFlow(IPU_PROC_FLOW_PRPENC_DEST,
                                        IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IRT_ENCODING);
        }
        else
        {
            CMSetProcFlow(IPU_PROC_FLOW_PRPVF_ROT_SRC,
                                        IPU_IPU_FS_PROC_FLOW1_SRC_SEL_VIEWFINDER);
                                        
            CMSetProcFlow(IPU_PROC_FLOW_PRPVF_DEST,
                                        IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IRT_VIEWFINDER);
        }
#if BANDMODE_ENABLE
        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;
            OffsetData.uOffset=iInputWidth * 8;
            OffsetData.vOffset = OffsetData.uOffset + OffsetData.uOffset/4;
            OffsetData.interlaceOffset= 0;
        }
        pIDMAChannelParams->iBandMode = CPMEM_BNDM_8_LINES;
       
        // allocate ROTATION middle buffer
        IPUBufferInfo IPUBufInfo;
        UINT32 rotAddr;
        //for caculate correct buffer size
        //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
        if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
        ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
            IPUBufInfo.dwBufferSize = iInputStride * 8 * 3;
        else
            IPUBufInfo.dwBufferSize = iInputStride * 8 * 2;

        IPUBufInfo.MemType = MEM_TYPE_IRAM;

        if(outputChannel == prpOutputChannel_ENC)
        {
            if(m_hENCRotBuffer)
                IPUV3DeallocateBuffer(hIPUBase, m_hENCRotBuffer);

            //get the space
            m_hENCRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
            IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hENCRotBuffer);
            
            //Physical address needn't CEOpenCallerBuffer().
            if(m_hENCRotBuffer == NULL)
            {
                DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Fail to get ENCRotBuffer ! \r\n"), __WFUNCTION__));   
                result = FALSE;
                goto _IRTinputDone;                    
            }

            rotAddr = (UINT32)m_hENCRotBuffer->PhysAddr();
            CPMEMWriteBandMode(IDMAC_CH_PRP_OUTPUT_ENC,
                               pIDMAChannelParams->iBandMode);
            IDMACChannelSetBandMode(IDMAC_CH_PRP_OUTPUT_ENC,IDMAC_CH_BAND_MODE_ENABLE);
            
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, 0, (UINT32 *)rotAddr);
            CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PRP_ENC, 0, (UINT32 *)rotAddr);
            IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_ENC);

            rotAddr += IPUBufInfo.dwBufferSize/2;
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, 1, (UINT32 *)rotAddr);
            CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PRP_ENC, 1, (UINT32 *)rotAddr);
            IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_ENC);

            if (m_bCurrentPlanar)
                CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_ENC,
                                 &OffsetData, OffsetData.bInterleaved);
        }
        else
        {
            if(m_hVFRotBuffer)
                    IPUV3DeallocateBuffer(hIPUBase, m_hVFRotBuffer);
            //get the space
            m_hVFRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
            IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hVFRotBuffer);
            
            //Physical address needn't CEOpenCallerBuffer().
            if(m_hVFRotBuffer == NULL)
            {
                DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Fail to get VFRotBuffer ! \r\n"), __WFUNCTION__));   
                result = FALSE;
                goto _IRTinputDone;                    
            }

            rotAddr = (UINT32)m_hVFRotBuffer->PhysAddr();
            CPMEMWriteBandMode(IDMAC_CH_PRP_OUTPUT_VF,
                               pIDMAChannelParams->iBandMode);
            IDMACChannelSetBandMode(IDMAC_CH_PRP_OUTPUT_VF,IDMAC_CH_BAND_MODE_ENABLE);
            
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, 0, (UINT32 *)rotAddr);
            CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PRP_VF, 0, (UINT32 *)rotAddr);
            IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);

            rotAddr += IPUBufInfo.dwBufferSize / 2;
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, 1, (UINT32 *)rotAddr);
            CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PRP_VF, 1, (UINT32 *)rotAddr);
            IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_VF);

            if (m_bCurrentPlanar)
                CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_VF,
                                 &OffsetData, OffsetData.bInterleaved);
        }   
#else
        if (m_bCurrentPlanar)
        {
            OffsetData.bInterleaved = FALSE;
            OffsetData.uOffset=iInputWidth * iInputHeight;
            OffsetData.vOffset = OffsetData.uOffset + OffsetData.uOffset/4;
            OffsetData.interlaceOffset= 0;
        }
        //pIDMAChannelParams->iBandMode = CPMEM_BNDM_8_LINES;

        // allocate ROTATION middle buffer
        IPUBufferInfo IPUBufInfo;
        UINT32 rotAddr;
        //for caculate correct buffer size
        //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
        if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
        ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
           IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 3;
        else
            IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 2;

        IPUBufInfo.MemType = MEM_TYPE_IRAM;

        if(outputChannel == prpOutputChannel_ENC)
        {
            if(m_hENCRotBuffer)
                IPUV3DeallocateBuffer(hIPUBase, m_hENCRotBuffer);

            //get the space
            m_hENCRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
            IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hENCRotBuffer);
            
            //Physical address needn't CEOpenCallerBuffer().
            if(m_hENCRotBuffer == NULL)
            {
                DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Fail to get ENCRotBuffer ! \r\n"), __WFUNCTION__));   
                result = FALSE;
                goto _IRTinputDone;                    
            }

            rotAddr = (UINT32)m_hENCRotBuffer->PhysAddr();
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, 0, (UINT32 *)rotAddr);
            CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PRP_ENC, 0, (UINT32 *)rotAddr);
            IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_ENC);

            rotAddr += IPUBufInfo.dwBufferSize/2;
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_ENC, 1, (UINT32 *)rotAddr);
            CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PRP_ENC, 1, (UINT32 *)rotAddr);
            IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_ENC);

            if (m_bCurrentPlanar)
                CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_ENC,
                                 &OffsetData, OffsetData.bInterleaved);
        }
        else
        {
            if(m_hVFRotBuffer)
                    IPUV3DeallocateBuffer(hIPUBase, m_hVFRotBuffer);
            //get the space
            m_hVFRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
            IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hVFRotBuffer);
            
            //Physical address needn't CEOpenCallerBuffer().
            if(m_hVFRotBuffer == NULL)
            {
                DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Fail to get VFRotBuffer ! \r\n"), __WFUNCTION__));   
                result = FALSE;
                goto _IRTinputDone;                    
            }

            rotAddr = (UINT32)m_hVFRotBuffer->PhysAddr();
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, 0, (UINT32 *)rotAddr);
            CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PRP_VF, 0, (UINT32 *)rotAddr);
            IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);

            rotAddr += IPUBufInfo.dwBufferSize / 2;
            CPMEMWriteBuffer(IDMAC_CH_PRP_OUTPUT_VF, 1, (UINT32 *)rotAddr);
            CPMEMWriteBuffer(IDMAC_CH_IRT_INPUT_PRP_VF, 1, (UINT32 *)rotAddr);
            IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_VF);

            if (m_bCurrentPlanar)
                CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_VF,
                                 &OffsetData, OffsetData.bInterleaved);
        }  

#endif
    }
    else
    {
            DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: Error input type, LastModule: %d ! \r\n"), __WFUNCTION__,
                    LastModule));   
            result = FALSE;
            goto _IRTinputDone;
    }
    pIDMAChannelParams->iPixelBurstCode = CPMEM_PIXEL_BURST_8; // IRT MUST BE 8
    if(outputChannel == prpOutputChannel_ENC)
    {
        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_IRT_INPUT_PRP_ENC,
                             &OffsetData, OffsetData.bInterleaved);
        PrpIDMACChannelConfig(IDMAC_CH_IRT_INPUT_PRP_ENC, pIDMAChannelParams);
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_IRT_INPUT_PRP_VF,
                             &OffsetData, OffsetData.bInterleaved);
        PrpIDMACChannelConfig(IDMAC_CH_IRT_INPUT_PRP_VF, pIDMAChannelParams);
    }    

_IRTinputDone:
    PRP_FUNCTION_EXIT();
    return result;
}


//-----------------------------------------------------------------------------
//
// Function: PrpConfigureInput
//
// This function configures the input channel of image converter module, meanwhile it also
// allocates the middle buffer and configures the destination of preious module.
//
// Parameters:
//      inputChannel
//          [in] the type of input channel, main channel or combine channel.
//
//      outputChannel:
//          [in] the type of output channel, viewfinder or encoder.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureInput(prpInputChannel inputChannel, prpOutputChannel outputChannel)
{
    BOOL result = TRUE;
    UINT16 iInputWidth, iInputHeight;
    UINT32 iInputStride;
    prpSourceType LastModule;
    pPrpIDMACChannelParams pIDMAChannelParams;

    CPMEMBufOffsets OffsetData;

    memset(&OffsetData,0,sizeof(CPMEMBufOffsets));

    PRP_FUNCTION_ENTRY();

    if(outputChannel == prpOutputChannel_ENC)
    {
        LastModule = m_PrpENCLastModule;
        pIDMAChannelParams = &m_ENCchannelParams;
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        LastModule = m_PrpVFLastModule;
        pIDMAChannelParams = &m_VFchannelParams;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error input type, outputChannel: %d ! \r\n"), __WFUNCTION__,
                outputChannel));   
        result = FALSE;
        goto _PRPinputDone;
    }

    pIDMAChannelParams->iBlockMode = 0;
    pIDMAChannelParams->iBandMode = CPMEM_BNDM_DISABLE;

    iInputHeight = pIDMAChannelParams->iHeight;
    iInputWidth = pIDMAChannelParams->iWidth;
    iInputStride = pIDMAChannelParams->iLineStride;
    
    //-------------------------------------------------------
    // According to the previous module to configure the data flow
    //-------------------------------------------------------    
    // Configure the DMA channel, either the Main or Combining channel
    if(inputChannel == prpInputChannel_COMBINE)
    {
            //for combine channel the last module is always MCU, so no process flow configuration
            if (m_bCurrentPlanar)
            {
                OffsetData.bInterleaved = FALSE;
                OffsetData.uOffset= pIDMAChannelParams->UOffset;
                OffsetData.vOffset = pIDMAChannelParams->VOffset;
                OffsetData.interlaceOffset= 0;

                CPMEMWriteOffset(IDMAC_CH_PRP_INPUT_GRAPHICS,
                                 &OffsetData, OffsetData.bInterleaved);
            }            
            PrpIDMACChannelConfig(IDMAC_CH_PRP_INPUT_GRAPHICS, pIDMAChannelParams);    
    }
    else
    {
        //main input channel
        if(LastModule == prpSourceType_ARM)
        {
            if(!m_bInputisInterlaced)
            {
                if(outputChannel == prpOutputChannel_ENC)
                {
                    CMSetProcFlow(IPU_PROC_FLOW_ENC_IN_VALID,1);
                }
                else
                {
                    CMSetProcFlow(IPU_PROC_FLOW_VF_IN_VALID,1);
                }
            }
            else
            {
                    CMSetProcFlow(IPU_PROC_FLOW_ENC_IN_VALID,0);
                    CMSetProcFlow(IPU_PROC_FLOW_VF_IN_VALID,0);                
            }
            CMSetProcFlow(IPU_PROC_FLOW_PRP_SRC,
                                        IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU);

            if (m_bCurrentPlanar)
            {
                OffsetData.bInterleaved = FALSE;
                OffsetData.uOffset= pIDMAChannelParams->UOffset;
                OffsetData.vOffset = pIDMAChannelParams->VOffset;
                OffsetData.interlaceOffset= 0;
            }

        }
        else if(LastModule == prpSourceType_ROT)
        {

            if(outputChannel == prpOutputChannel_ENC)
            {
                CMSetProcFlow(IPU_PROC_FLOW_ENC_IN_VALID,1);
                if(m_bENCOversizedFrame)
                {
                    RETAILMSG(1,(TEXT("rotation doesn't be supported for oversized frame ! \r\n")));
                    return FALSE;
                }
                else
                {
                    CMSetProcFlow(IPU_PROC_FLOW_PRP_SRC,
                                                IPU_IPU_FS_PROC_FLOW1_SRC_SEL_IRT_ENCODING);
                    CMSetProcFlow(IPU_PROC_FLOW_PRPENC_ROT_DEST,
                                                IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IC_PRP);
                }
            }
            else
            {
                CMSetProcFlow(IPU_PROC_FLOW_VF_IN_VALID,1);
                if(m_bVFOversizedFrame)
                {
                    RETAILMSG(1,(TEXT("rotation doesn't be supported for oversized frame ! \r\n")));
                    return FALSE;
                }
                else
                {
                    CMSetProcFlow(IPU_PROC_FLOW_PRP_SRC,
                                                IPU_IPU_FS_PROC_FLOW1_SRC_SEL_IRT_VIEWFINDER);
                    CMSetProcFlow(IPU_PROC_FLOW_PRPVF_ROT_DEST,
                                                IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IC_PRP);
                }
            }
#if BANDMODE_ENABLE

            if (m_bCurrentPlanar)
            {
                OffsetData.bInterleaved = FALSE;
                OffsetData.uOffset=iInputWidth * 8;
                OffsetData.vOffset = OffsetData.uOffset + OffsetData.uOffset/4;
                OffsetData.interlaceOffset= 0;
            }


            pIDMAChannelParams->iBandMode = CPMEM_BNDM_8_LINES;
           
            // allocate ROTATION middle buffer
            IPUBufferInfo IPUBufInfo;
            UINT32 rotAddr;
            //for caculate correct buffer size
            //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
            if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
                ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
                IPUBufInfo.dwBufferSize = iInputStride * 8 * 3;
            else
                IPUBufInfo.dwBufferSize = iInputStride * 8 * 2;

            IPUBufInfo.MemType = MEM_TYPE_IRAM;

            if(outputChannel == prpOutputChannel_ENC)
            {
                if(m_hENCRotBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hENCRotBuffer);
                //get the space
                m_hENCRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hENCRotBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hENCRotBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get ENCRotBuffer ! \r\n"), __WFUNCTION__));    
                    result = FALSE;
                    goto _PRPinputDone;                 
                }

                rotAddr = (UINT32)m_hENCRotBuffer->PhysAddr();
                CPMEMWriteBandMode(   IDMAC_CH_IRT_OUTPUT_PRP_ENC,
                                                    pIDMAChannelParams->iBandMode);
                IDMACChannelSetBandMode(IDMAC_CH_IRT_OUTPUT_PRP_ENC,IDMAC_CH_BAND_MODE_ENABLE);
                
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_ENC, 0, (UINT32 *)rotAddr);
                CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, 0, (UINT32 *)rotAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);

                rotAddr += IPUBufInfo.dwBufferSize / 2;
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_ENC, 1, (UINT32 *)rotAddr);
                CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, 1, (UINT32 *)rotAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);

                if (m_bCurrentPlanar) 
                   CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_ENC,
                                    &OffsetData, OffsetData.bInterleaved);
            }
            else
            {
                if(m_hVFRotBuffer)
                    IPUV3DeallocateBuffer(hIPUBase, m_hVFRotBuffer);
                //get the space
                m_hVFRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hVFRotBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hVFRotBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get VFRotBuffer ! \r\n"), __WFUNCTION__));    
                    result = FALSE;
                    goto _PRPinputDone;                 
                }

                rotAddr = (UINT32)m_hVFRotBuffer->PhysAddr();
                CPMEMWriteBandMode(   IDMAC_CH_IRT_OUTPUT_PRP_VF,
                                                    pIDMAChannelParams->iBandMode);
                IDMACChannelSetBandMode(IDMAC_CH_IRT_OUTPUT_PRP_VF,IDMAC_CH_BAND_MODE_ENABLE);
                
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_VF, 0, (UINT32 *)rotAddr);
                CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, 0, (UINT32 *)rotAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);

                rotAddr += IPUBufInfo.dwBufferSize / 2;
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_VF, 1, (UINT32 *)rotAddr);
                CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, 1, (UINT32 *)rotAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_VF,
                                     &OffsetData, OffsetData.bInterleaved);
            }   
#else
            if (m_bCurrentPlanar)
            {
                OffsetData.bInterleaved = FALSE;
                OffsetData.uOffset=iInputWidth * iInputHeight;
                OffsetData.vOffset = OffsetData.uOffset + OffsetData.uOffset/4;
                OffsetData.interlaceOffset= 0;
            }

            // allocate ROTATION middle buffer
            IPUBufferInfo IPUBufInfo;
            UINT32 rotAddr;
            //for caculate correct buffer size
            //For YUV420, the Buffer size should be 1.5 *realstride * height, StrideForBuf = 1.5 * realstride
            if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
                ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight* 3;
            else
                IPUBufInfo.dwBufferSize = iInputStride * iInputHeight * 2;

            IPUBufInfo.MemType = MEM_TYPE_IRAM;

            if(outputChannel == prpOutputChannel_ENC)
            {
                if(m_hENCRotBuffer)
                        IPUV3DeallocateBuffer(hIPUBase, m_hENCRotBuffer);
                //get the space
                m_hENCRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hENCRotBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hENCRotBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get ENCRotBuffer ! \r\n"), __WFUNCTION__));    
                    result = FALSE;
                    goto _PRPinputDone;                 
                }

                rotAddr = (UINT32)m_hENCRotBuffer->PhysAddr();
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_ENC, 0, (UINT32 *)rotAddr);
                CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, 0, (UINT32 *)rotAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);

                rotAddr += IPUBufInfo.dwBufferSize / 2;
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_ENC, 1, (UINT32 *)rotAddr);
                CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, 1, (UINT32 *)rotAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PRP_ENC);

                if (m_bCurrentPlanar) 
                   CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_ENC,
                                    &OffsetData, OffsetData.bInterleaved);
            }
            else
            {
                if(m_hVFRotBuffer)
                    IPUV3DeallocateBuffer(hIPUBase, m_hVFRotBuffer);
                //get the space
                m_hVFRotBuffer = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
                IPUV3AllocateBuffer(hIPUBase, &IPUBufInfo, m_hVFRotBuffer);
                
                //Physical address needn't CEOpenCallerBuffer().
                if(m_hVFRotBuffer == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                            (TEXT("%s: Fail to get VFRotBuffer ! \r\n"), __WFUNCTION__));    
                    result = FALSE;
                    goto _PRPinputDone;                 
                }

                rotAddr = (UINT32)m_hVFRotBuffer->PhysAddr();
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_VF, 0, (UINT32 *)rotAddr);
                CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, 0, (UINT32 *)rotAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);

                rotAddr += IPUBufInfo.dwBufferSize / 2;
                CPMEMWriteBuffer(IDMAC_CH_IRT_OUTPUT_PRP_VF, 1, (UINT32 *)rotAddr);
                CPMEMWriteBuffer(IDMAC_CH_PRP_INPUT_VIDEO, 1, (UINT32 *)rotAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_IRT_OUTPUT_PRP_VF);

                if (m_bCurrentPlanar)
                    CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_VF,
                                     &OffsetData, OffsetData.bInterleaved);
            }  

        
#endif
        }
        else
        {
                DEBUGMSG(ZONE_ERROR,
                        (TEXT("%s: Error input type, LastModule: %d ! \r\n"), __WFUNCTION__,
                        LastModule));   
                result = FALSE;
                goto _PRPinputDone;
        }
        if((pIDMAChannelParams->iWidth%16)!=0)
            pIDMAChannelParams->iPixelBurstCode = CPMEM_PIXEL_BURST_8;
        else
            pIDMAChannelParams->iPixelBurstCode = CPMEM_PIXEL_BURST_16;
        //IDMACChannelDBMODE(IDMAC_CH_PRP_INPUT_VIDEO,TRUE);
        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_PRP_INPUT_VIDEO,
                             &OffsetData, OffsetData.bInterleaved);
        PrpIDMACChannelConfig(IDMAC_CH_PRP_INPUT_VIDEO, pIDMAChannelParams);
    }
_PRPinputDone:
    PRP_FUNCTION_EXIT();
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: PrpIDMACChannelConfig
//
// This function configures the hardware.
//
// Parameters:
//      ch
//          [in] The IDMAC channel being queried.
//
//      pChannelParams
//          [in] The pointer to prp IDMAC Channel parameters structure.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpIDMACChannelConfig(UINT8 ch,
    pPrpIDMACChannelParams pChannelParams)
{
    CPMEMConfigData CPMEMData;
    CPMEMData.iBitsPerPixel = pChannelParams->iBitsPerPixelCode;
    CPMEMData.iDecAddrSelect = CPMEM_DEC_SEL_ADDR_0_TO_15; // Don't care, only for 4BPP mode   //not support yet
    CPMEMData.iAccessDimension = CPMEM_DIM_2D; // 1D scan is only used for csi generic data   
    CPMEMData.iScanOrder = CPMEM_SO_PROGRESSIVE; //always progressive, not support interlaced yet
    CPMEMData.iThresholdEnable = CPMEM_THE_DISABLE;
    CPMEMData.iThreshold = 0;
    CPMEMData.iCondAccessEnable = CPMEM_CAE_COND_SKIP_DISABLE;    // for future use
    CPMEMData.iCondAccessPolarity = CPMEM_CAP_COND_SKIP_LOW;  // for future use
    CPMEMData.iAlphaUsed = CPMEM_ALU_ALPHA_SAME_CHANNEL;  // seperate alpha not support yet

    if(ch == IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE)
        CPMEMData.iAXI_Id = CPMEM_ID_0; //ID_0 has high priority to access memory, only for sync display channel
    else
        CPMEMData.iAXI_Id = CPMEM_ID_1; 
    
    
    CPMEMData.iBandMode = pChannelParams->iBandMode;
    if(CPMEMData.iBandMode != CPMEM_BNDM_DISABLE)
    {
        IDMACChannelSetBandMode(ch,IDMAC_CH_BAND_MODE_ENABLE);
    }
    else
    {
        IDMACChannelSetBandMode(ch,IDMAC_CH_BAND_MODE_DISABLE);
    }
    CPMEMData.iBlockMode = pChannelParams->iBlockMode;
    CPMEMData.iRotation90 = pChannelParams->iRotation90;
    CPMEMData.iFlipHoriz = pChannelParams->iFlipHoriz;
    CPMEMData.iFlipVert = pChannelParams->iFlipVert;
    CPMEMData.iPixelBurst = pChannelParams->iPixelBurstCode;
    switch(ch)
    {
        case IDMAC_CH_PRP_OUTPUT_ENC:
            INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB0_BURST_16,
            (CPMEMData.iPixelBurst>>3));
            break;        
        case IDMAC_CH_PRP_OUTPUT_VF:
            INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB1_BURST_16,
            (CPMEMData.iPixelBurst>>3));
            break;
        case IDMAC_CH_PRP_INPUT_GRAPHICS:
            INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB3_BURST_16,
            (CPMEMData.iPixelBurst>>3));
            break;
        case IDMAC_CH_PRP_INPUT_VIDEO:
            INSREG32BF(&m_pIC->IC_IDMAC_1, IPU_IC_IDMAC_1_CB6_BURST_16,
            (CPMEMData.iPixelBurst>>3));
            break;
        default:
            break;
    }

    CPMEMData.iPixelFormat = pChannelParams->iFormatCode;
    CPMEMData.iHeight = pChannelParams->iHeight -1;
    CPMEMData.iWidth = pChannelParams->iWidth -1;

    if(pChannelParams->bInterlaced)
    {
        CPMEMData.iScanOrder = CPMEM_SO_INTERLACED;
        CPMEMData.iLineStride = pChannelParams->iLineStride*2 -1;
    }
    else
    {
        CPMEMData.iLineStride = pChannelParams->iLineStride -1;
    }

    CPMEMData.iLineStride_Y = CPMEMData.iLineStride;
    if((pChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422)
    || (pChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420))
    {
        CPMEMData.iLineStride_UV = ((CPMEMData.iLineStride_Y+1) >>1) -1;
    }
    else
    {
        CPMEMData.iLineStride_UV = CPMEMData.iLineStride_Y;        
    }
    memcpy(&CPMEMData.pixelFormatData, &pChannelParams->pixelFormat, sizeof(ICPixelFormatData));
    CPMEMWrite(ch, &CPMEMData, pChannelParams->bInterleaved);
    
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
//      outputChannel:
//          [in] the type of output channel, viewfinder or encoder.
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::IRTConfigureOutput(pIcFlipRot pFlipRot,prpOutputChannel outputChannel)
{
    UINT16 tempWidth;
    CPMEMBufOffsets OffsetData;

    memset(&OffsetData,0,sizeof(CPMEMBufOffsets));
    pPrpIDMACChannelParams pIDMAChannelParams;
    
    if(outputChannel == prpOutputChannel_ENC)
    {
        pIDMAChannelParams = &m_ENCchannelParams;
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        pIDMAChannelParams = &m_VFchannelParams;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error input type, outputChannel: %d ! \r\n"), __WFUNCTION__,
                outputChannel));   
        return FALSE;
    }
    
    if (pFlipRot->rotate90)
    {

        pIDMAChannelParams->iLineStride = pIDMAChannelParams->iHeight * IcBitPerPixelToBPP[pIDMAChannelParams->iBitsPerPixelCode]/8;
        tempWidth = pIDMAChannelParams->iWidth;
        pIDMAChannelParams->iWidth = pIDMAChannelParams->iHeight;
        pIDMAChannelParams->iHeight = tempWidth;
        if(outputChannel == prpOutputChannel_ENC)
        {
            m_PrpENCOutputStride = m_PrpENCOutputHeight * m_PrpENCOutputStride / m_PrpENCOutputWidth;
            tempWidth = m_PrpENCOutputHeight;
            m_PrpENCOutputHeight= m_PrpENCOutputWidth;
            m_PrpENCOutputWidth = tempWidth;
        }
        else
        {
            m_PrpVFOutputStride = m_PrpVFOutputHeight * m_PrpVFOutputStride / m_PrpVFOutputWidth;
            tempWidth = m_PrpVFOutputHeight;
            m_PrpVFOutputHeight= m_PrpVFOutputWidth;
            m_PrpVFOutputWidth = tempWidth;
        }       
    }

    pIDMAChannelParams->iRotation90 = 0;
    pIDMAChannelParams->iFlipHoriz = 0;
    pIDMAChannelParams->iFlipVert = 0;
    pIDMAChannelParams->iBandMode = CPMEM_BNDM_DISABLE;

#if DP_NOYUV420_INPUT
    //For TO2, the output of IRT can't be YV12, otherwise the picture will have some line stripe.
    BOOL result = TRUE;
    icPixelFormat       PixelFormat;   // Input frame RGB format, set NULL 
    icDataWidth         DataWidth;     // Bits per pixel for RGB format
    if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
    ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
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
       
        result = PrpPixelFormatSetting(icFormat_UYVY422, 
                                        PixelFormat,
                                        DataWidth,
                                        pIDMAChannelParams);
        if(result == FALSE)
        {
            return FALSE;
        }
        if(outputChannel == prpOutputChannel_ENC)
        {
            m_PrpENCOutputStride = m_PrpENCOutputWidth * IcDataWidthToBPP[DataWidth]/8;     
            pIDMAChannelParams->iLineStride = m_PrpENCOutputStride;
        }
        else if(outputChannel == prpOutputChannel_VF)
        {
            m_PrpVFOutputStride = m_PrpVFOutputWidth * IcDataWidthToBPP[DataWidth]/8;     
            pIDMAChannelParams->iLineStride = m_PrpVFOutputStride;
        }
        pIDMAChannelParams->iPixelBurstCode = CPMEM_PIXEL_BURST_8; // IRT MUST BE 8
    }
#endif

    if (m_bCurrentPlanar)
    {
        pIDMAChannelParams->UOffset = pIDMAChannelParams->iWidth * pIDMAChannelParams->iHeight;
        pIDMAChannelParams->VOffset = pIDMAChannelParams->UOffset + pIDMAChannelParams->UOffset/4;
        OffsetData.bInterleaved = FALSE;
        OffsetData.uOffset= pIDMAChannelParams->UOffset;
        OffsetData.vOffset = pIDMAChannelParams->VOffset;
        OffsetData.interlaceOffset= 0;
    }

    if(outputChannel == prpOutputChannel_ENC)
    {
        CMSetProcFlow(IPU_PROC_FLOW_PRPENC_ROT_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_ENC,
                             &OffsetData, OffsetData.bInterleaved);
        PrpIDMACChannelConfig(IDMAC_CH_IRT_OUTPUT_PRP_ENC, pIDMAChannelParams);

    }
    else
    {
        CMSetProcFlow(IPU_PROC_FLOW_PRPVF_ROT_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
        if (m_bCurrentPlanar)                            
            CPMEMWriteOffset(IDMAC_CH_IRT_OUTPUT_PRP_VF,
                             &OffsetData, OffsetData.bInterleaved);
        PrpIDMACChannelConfig(IDMAC_CH_IRT_OUTPUT_PRP_VF, pIDMAChannelParams);
    }
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpConfigureOutput
//
// This function configures the output channel of image converter module, 
// and set the default destination IC.
//
// Parameters:
//      pIDMAChannel
//          [in] the pointer to IDMA channel structure.
//
//      outputChannel:
//          [in] the type of output channel, viewfinder or encoder.
//
//      bNoCSC:
//          [in]   TRUE:  needn't csc.
//                  FALSE:csc is needed. 
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpConfigureOutput(pIdmaChannel pIDMAChannel, prpOutputChannel outputChannel, BOOL bNoCSC)
{
    BOOL result = TRUE;
    UINT16 iOutputWidth, iOutputHeight;
    UINT32 iOutputStride;
    icFormat iFormat;
    pPrpIDMACChannelParams pIDMAChannelParams;
    CPMEMBufOffsets OffsetData;

    PRP_FUNCTION_ENTRY();

    memset(&OffsetData,0,sizeof(CPMEMBufOffsets));

    if(outputChannel == prpOutputChannel_ENC)
    {
        pIDMAChannelParams = &m_ENCchannelParams;
        iOutputWidth = m_PrpENCOutputWidth;
        iOutputHeight = m_PrpENCOutputHeight;
        iOutputStride = m_PrpENCOutputStride;
    }
    else if(outputChannel == prpOutputChannel_VF)
    {
        pIDMAChannelParams = &m_VFchannelParams;
        iOutputWidth = m_PrpVFOutputWidth;
        iOutputHeight = m_PrpVFOutputHeight;
        iOutputStride = m_PrpVFOutputStride;        
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error input type, outputChannel: %d ! \r\n"), __WFUNCTION__,
                outputChannel));   
        result = FALSE;
        goto _PRPoutputDone;
    }

    // Set these variables to reduce pointer computations,
    // as these will be referenced several times.
    //pIDMAChannelParams->iRotation90 = 0;
    //pIDMAChannelParams->iFlipHoriz = 0;
    //pIDMAChannelParams->iFlipVert = 0;
    pIDMAChannelParams->iBandMode = CPMEM_BNDM_DISABLE;

#if DP_NOYUV420_INPUT
    //When prp input YUV420 to DP directly and the image resolution larger than QVGA, the screen may crash
    //here is a workaround for this issue.prp, converting YUV420 to UYVY at first.
    if((pIDMAChannelParams->iFormatCode == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
    ||(pIDMAChannelParams->iFormatCode == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
    {
        
        //force convert YUV420 to UYVY422;
        m_bCurrentPlanar = FALSE;
        pIDMAChannel->DataWidth = icDataWidth_16BPP;
        pIDMAChannel->PixelFormat.component0_offset = 0;
        pIDMAChannel->PixelFormat.component1_offset = 8;
        pIDMAChannel->PixelFormat.component2_offset = 16;
        pIDMAChannel->PixelFormat.component3_offset = 24;
        pIDMAChannel->PixelFormat.component0_width = 7;
        pIDMAChannel->PixelFormat.component1_width = 7;
        pIDMAChannel->PixelFormat.component2_width = 7;
        pIDMAChannel->PixelFormat.component3_width = 7;

        
        result = PrpPixelFormatSetting(icFormat_UYVY422, 
                                                pIDMAChannel->PixelFormat,
                                                pIDMAChannel->DataWidth,
                                                pIDMAChannelParams);
        if(result == FALSE)
        {goto _PRPoutputDone;}
        //For oversized frame, the width of processing frame is half of orignal 
        //one. But the stride should be the same as original frame. So we 
        //should keep the original the stride value. 
        if(!(m_bENCOversizedFrame||m_bVFOversizedFrame))
        {
            iOutputStride = iOutputWidth * IcDataWidthToBPP[pIDMAChannel->DataWidth]/8;

        }
        else
        {
            iOutputStride = iOutputStride * 2;  //convert 420 stride(1bytes/pixel) to 422 stride(2bytes/pxiel)
        }
        if(outputChannel == prpOutputChannel_ENC)
        {
            m_PrpENCOutputStride = iOutputStride;
        }
        else if(outputChannel == prpOutputChannel_VF)
        {
            m_PrpVFOutputStride = iOutputStride;        
        }
    }
#endif


    if(!bNoCSC)
    {//impossbile run here
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
        result = PrpPixelFormatSetting(iFormat, 
                                                pIDMAChannel->PixelFormat,
                                                pIDMAChannel->DataWidth,
                                                pIDMAChannelParams);
        if(result == FALSE)
        {goto _PRPoutputDone;}
        iOutputStride = iOutputWidth * IcDataWidthToBPP[pIDMAChannel->DataWidth]/8;
        if(outputChannel == prpOutputChannel_ENC)
        {
            m_PrpENCOutputStride = iOutputStride;
        }
        else if(outputChannel == prpOutputChannel_VF)
        {
            m_PrpVFOutputStride = iOutputStride;        
        }

    }
    
    pIDMAChannelParams->iHeight = iOutputHeight;
    pIDMAChannelParams->iWidth = iOutputWidth;
    pIDMAChannelParams->iLineStride = iOutputStride;
    if (m_bCurrentPlanar)
    {
        pIDMAChannelParams->UOffset = pIDMAChannelParams->iWidth * pIDMAChannelParams->iHeight;
        //For oversized frame, the width of processing frame is half of orignal 
        //one. But the u,v offset should be the same as original frame. So we 
        //should double the offset value, when we use get the offset from width. 
        if(m_bENCOversizedFrame||m_bVFOversizedFrame)
        {
           pIDMAChannelParams->UOffset = pIDMAChannelParams->UOffset<<1;
           RETAILMSG(1,(TEXT("Error: oversized frame doesn't support non-interleaved output!!!")));
        }   
        pIDMAChannelParams->VOffset = pIDMAChannelParams->UOffset + pIDMAChannelParams->UOffset/4;
        OffsetData.bInterleaved = FALSE;
        OffsetData.uOffset= pIDMAChannelParams->UOffset;
        OffsetData.vOffset = pIDMAChannelParams->VOffset;
        OffsetData.interlaceOffset= 0;
    }


    //-----------------------------------------------------------------
    // Image size validity check
    // Setup pre-processing channel output image size
    //-----------------------------------------------------------------

    if((iOutputWidth  < PRP_MIN_OUTPUT_WIDTH) ||
        (iOutputHeight < PRP_MIN_OUTPUT_HEIGHT) ||
        (iOutputWidth  > PRP_MAX_OUTPUT_WIDTH) ||
        (iOutputHeight > PRP_MAX_OUTPUT_HEIGHT) )
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error pre-processing channel size: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR, (TEXT("\t Prp Channel Size: width (%d), height (%d)\r\n"),
                    iOutputWidth, iOutputHeight));
        result = FALSE;
        goto _PRPoutputDone;
    }
    if((pIDMAChannelParams->iWidth%16)!=0)
        pIDMAChannelParams->iPixelBurstCode = CPMEM_PIXEL_BURST_8; 
    else
        pIDMAChannelParams->iPixelBurstCode = CPMEM_PIXEL_BURST_16; 
    if(outputChannel == prpOutputChannel_ENC)
    {
        CMSetProcFlow(IPU_PROC_FLOW_PRPENC_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_ENC,
                             &OffsetData, OffsetData.bInterleaved);
        PrpIDMACChannelConfig(IDMAC_CH_PRP_OUTPUT_ENC, pIDMAChannelParams);
    }
    else
    {
        CMSetProcFlow(IPU_PROC_FLOW_PRPVF_DEST,
                                    IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU);
                        
        if (m_bCurrentPlanar)
            CPMEMWriteOffset(IDMAC_CH_PRP_OUTPUT_VF,
                             &OffsetData, OffsetData.bInterleaved);
        PrpIDMACChannelConfig(IDMAC_CH_PRP_OUTPUT_VF, pIDMAChannelParams);

    }
 

_PRPoutputDone:
    PRP_FUNCTION_EXIT();
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: PrpStartChannel
//
// This function starts the pre-processing channel.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpStartChannel()
{
    UINT32 oldVal, newVal, iMask, iBitval;
    dpCSCConfigData DpCSCConfigData;

    PRP_FUNCTION_ENTRY();

    // PP must have been configured at least once 
    // in order to start the channel
    if (!m_bConfigured)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error.  Cannot start pre-processing viewfinding channel without first configuring\r\n"),
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
    if (m_bVFIRTEnabled)
    {
        //Only IRT also need to enable IC.
        PrpEnable();
        IRTEnable(IPU_DRIVER_PRP_VF);
        
        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_IRT_INPUT_PRP_VF,TRUE);
        IDMACChannelDBMODE(IDMAC_CH_IRT_OUTPUT_PRP_VF,TRUE);
        
        // Compute bitmask and shifted bit value for IDMAC enable register.
        IDMACChannelEnable(IDMAC_CH_IRT_INPUT_PRP_VF);
        IDMACChannelEnable(IDMAC_CH_IRT_OUTPUT_PRP_VF);
        
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPVF_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPVF_ROT_EN, IPU_IC_CONF_PRPVF_ROT_EN_ENABLE);

        // Enable postprocessing path in IC and 
        // enable postprocessing for rotation path in IC.
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);
    }
    
    if(m_bVFPrpEnabled)
    {
        // Enable Presprocessor
        PrpEnable();
    
        if(!m_bInputisInterlaced)
        {
            //Oversized frame processing, we use single buffer mode
            if(!m_bVFOversizedFrame)
                IDMACChannelDBMODE(IDMAC_CH_PRP_INPUT_VIDEO,TRUE);
            IDMACChannelEnable(IDMAC_CH_PRP_INPUT_VIDEO);

            if(m_bVFCombEnabled)
            {
                IDMACChannelDBMODE(IDMAC_CH_PRP_INPUT_GRAPHICS,TRUE);
                IDMACChannelEnable(IDMAC_CH_PRP_INPUT_GRAPHICS);

            }
        }
        if(!m_bDMFCChannelUsing)
        {
            if(!m_bVFOversizedFrame)
                IDMACChannelDBMODE(IDMAC_CH_PRP_OUTPUT_VF,TRUE);
            IDMACChannelEnable(IDMAC_CH_PRP_OUTPUT_VF);
        }

        
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPVF_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPVF_EN, IPU_IC_CONF_PRPVF_EN_ENABLE);

        // enable pre-processing vf path in IC
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

    }

    if (m_bENCIRTEnabled)
    {
        //Only IRT also need to enable IC.
        PrpEnable();
        IRTEnable(IPU_DRIVER_PRP_ENC);
        
        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_IRT_INPUT_PRP_ENC,TRUE);
        IDMACChannelDBMODE(IDMAC_CH_IRT_OUTPUT_PRP_ENC,TRUE);
        
        // Compute bitmask and shifted bit value for IDMAC enable register.
        IDMACChannelEnable(IDMAC_CH_IRT_INPUT_PRP_ENC);
        IDMACChannelEnable(IDMAC_CH_IRT_OUTPUT_PRP_ENC);
        
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPENC_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPENC_ROT_EN, IPU_IC_CONF_PRPENC_ROT_EN_ENABLE);

        // Enable postprocessing path in IC and 
        // enable postprocessing for rotation path in IC.
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);
    }
    
    if(m_bENCPrpEnabled)
    {
        // Enable Presprocessor
        if(!m_bVFPrpEnabled)
            PrpEnable();
    
        //Oversized frame processing, we use single buffer mode
        if((!m_bVFPrpEnabled)&&(!m_bENCOversizedFrame))
            IDMACChannelDBMODE(IDMAC_CH_PRP_INPUT_VIDEO,TRUE);
        if(!m_bENCOversizedFrame)
            IDMACChannelDBMODE(IDMAC_CH_PRP_OUTPUT_ENC,TRUE);

        if(!m_bVFPrpEnabled)        
            IDMACChannelEnable(IDMAC_CH_PRP_INPUT_VIDEO);
        IDMACChannelEnable(IDMAC_CH_PRP_OUTPUT_ENC);
        // Compute bitmask and shifted bit value for IDMAC enable register.
        
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPENC_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPENC_EN, IPU_IC_CONF_PRPENC_EN_ENABLE);

        // enable pre-processing vf path in IC
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

    }

    if(m_bSYNCDPEnabled)
    {
        if(m_bSYNCDPCSCChanged)
        {
            //Enable the colorkey for foreground for system wake up
            DPCSCGetCoeffs(DP_CHANNEL_SYNC, &DpCSCConfigData);
            if (DpCSCConfigData.CSCPosition == DP_CSC_BG)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BOTH;
            }
            else if(DpCSCConfigData.CSCPosition == DP_CSC_DISABLE)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_FG;
            }
            DPCSCConfigure(DP_CHANNEL_SYNC, &DpCSCConfigData);
            m_bSYNCDPCSCChanged = FALSE;
        }

        IDMACChannelDBMODE(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE,TRUE);

        IDMACChannelEnable(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);

        DPGraphicWindowEnable(DP_CHANNEL_SYNC,TRUE);
    }

    if(m_bASYNCDPEnabled&&(!m_bDMFCChannelUsing))
    {
        if(m_bASYNCDPCSCChanged)
        {
            //Enable the colorkey for foreground for system wake up
            DPCSCGetCoeffs(DP_CHANNEL_ASYNC0, &DpCSCConfigData);
            if (DpCSCConfigData.CSCPosition == DP_CSC_BG)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BOTH;
            }
            else if(DpCSCConfigData.CSCPosition == DP_CSC_DISABLE)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_FG;
            }
            DPCSCConfigure(DP_CHANNEL_ASYNC0, &DpCSCConfigData);
            m_bASYNCDPCSCChanged = FALSE;
        }

        IDMACChannelDBMODE(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE,TRUE);

        IDMACChannelEnable(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE);

        DPGraphicWindowEnable(DP_CHANNEL_ASYNC0,TRUE);
    }
    m_bRunning = TRUE;
    ResetEvent(m_hDPFGIntrEvent);

    //IDMACDumpRegs();
    //DMFCDumpRegs();
    //ICDumpRegs();
    //CMDumpRegs();
    
    PRP_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrpIsBusy
//
// This function checks prp module status
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if busy.
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpIsBusy(void)
{
    BOOL ret = FALSE;
    DWORD dwStatus;
    
    PRP_FUNCTION_ENTRY();

    dwStatus = CMGetProcTaskStatus(IPU_PROC_TASK_MEM2PRP);

    if((dwStatus == IPU_PROC_TASK_STAT_ENC_ACTIVE) || (dwStatus == IPU_PROC_TASK_STAT_VF_ACTIVE)
        || (dwStatus == IPU_PROC_TASK_STAT_BOTH_ACTIVE))
        ret = TRUE;


    PRP_FUNCTION_EXIT();
    return ret;
}

//-----------------------------------------------------------------------------
//
// Function: IRTIsBusy
//
// This function checks IRT module status for prp path
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if busy.
//
//-----------------------------------------------------------------------------

BOOL PrpClass::IRTIsBusy(void)
{
    BOOL ret = FALSE;
    PRP_FUNCTION_ENTRY();

    if(CMGetProcTaskStatus(IPU_PROC_TASK_VF_ROT) == IPU_PROC_TASK_STAT_ACTIVE)
        ret = TRUE;


    PRP_FUNCTION_EXIT();
    return ret;
}
//-----------------------------------------------------------------------------
//
// Function: PrpEnable
//
// Enable the Image Converter.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::PrpEnable(void)
{
    //DWORD dwBytesTransferred;
    IPU_DRIVER driver = IPU_DRIVER_PRP_VF;

    PRP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPrpEnable);

    // If either the ENC or VF channels is already running, then
    // the IC must already be enabled, and we can return.
    if (m_bRunning)
    {
        LeaveCriticalSection(&m_csPrpEnable);
        return;
    }

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Enabling IC!\r\n"), __WFUNCTION__));

    if(!IPUV3EnableModule(hIPUBase, IPU_SUBMODULE_IC, driver))
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to enable IC!\r\n"), __WFUNCTION__));
    }


    LeaveCriticalSection(&m_csPrpEnable);

    PRP_FUNCTION_EXIT();
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
void PrpClass::IRTEnable(IPU_DRIVER driver)
{

    PRP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPrpEnable);

    // If either the ENC or VF channels is already running, then
    // the IC must already be enabled, and we can return.
    if (m_bRunning)
    {
        LeaveCriticalSection(&m_csPrpEnable);
        return;
    }

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Enabling IC!\r\n"), __WFUNCTION__));

    if (!IPUV3EnableModule(hIPUBase, IPU_SUBMODULE_IRT, driver))
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to enable IC!\r\n"), __WFUNCTION__));
    }

    LeaveCriticalSection(&m_csPrpEnable);

    PRP_FUNCTION_EXIT();
}
//-----------------------------------------------------------------------------
//
// Function: PrpStopChannel
//
// This function halts the pre-processing channels.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL PrpClass::PrpStopChannel(void)
{
    UINT32 uCount = 0;
    UINT32 oldVal, newVal, iMask, iBitval;
    dpCSCConfigData DpCSCConfigData;

    PRP_FUNCTION_ENTRY();

    EnterCriticalSection(&m_csStopping);

    // If not running, return
    if (m_bRunning == FALSE)
    {
        LeaveCriticalSection(&m_csStopping);    
        return TRUE;
    }
    
    m_bRunning = FALSE;

    if(m_bVFIRTEnabled)
    {
        uCount = 0;
        while(IDMACChannelIsBusy(IDMAC_CH_IRT_INPUT_PRP_VF)
            ||IDMACChannelIsBusy(IDMAC_CH_IRT_OUTPUT_PRP_VF)
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
                    (TEXT("%s(): Error in stopping IRT viewfinding channel!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPVF_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPVF_ROT_EN, IPU_IC_CONF_PRPVF_ROT_EN_DISABLE);

        // disable pre-processing vf path in IRT
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

        IDMACChannelDisable(IDMAC_CH_IRT_INPUT_PRP_VF);
        IDMACChannelDisable(IDMAC_CH_IRT_OUTPUT_PRP_VF);
            
        IDMACChannelDBMODE(IDMAC_CH_IRT_INPUT_PRP_VF,FALSE);
        IDMACChannelDBMODE(IDMAC_CH_IRT_OUTPUT_PRP_VF,FALSE);
        
        IRTDisable(IPU_DRIVER_PRP_VF);
    }

    if(m_bENCIRTEnabled)
    {
        uCount = 0;
        while(IDMACChannelIsBusy(IDMAC_CH_IRT_INPUT_PRP_ENC)
            ||IDMACChannelIsBusy(IDMAC_CH_IRT_OUTPUT_PRP_ENC)
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
                    (TEXT("%s(): Error in stopping IRT encoding channel!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPENC_ROT_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPENC_ROT_EN, IPU_IC_CONF_PRPENC_ROT_EN_DISABLE);

        // disable pre-processing enc path in IRT
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

        IDMACChannelDisable(IDMAC_CH_IRT_INPUT_PRP_ENC);
        IDMACChannelDisable(IDMAC_CH_IRT_OUTPUT_PRP_ENC);
            
        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_IRT_INPUT_PRP_ENC,FALSE);
        IDMACChannelDBMODE(IDMAC_CH_IRT_OUTPUT_PRP_ENC,FALSE);
        
        IRTDisable(IPU_DRIVER_PRP_ENC);
    }

    if(m_bVFPrpEnabled)
    {
        uCount = 0;
        while(IDMACChannelIsBusy(IDMAC_CH_PRP_INPUT_VIDEO)
            ||IDMACChannelIsBusy(IDMAC_CH_PRP_OUTPUT_VF)
            ||(m_bVFCombEnabled && IDMACChannelIsBusy(IDMAC_CH_PRP_INPUT_GRAPHICS))
            ||PrpIsBusy())
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
                    (TEXT("%s(): Error in stopping prp viewfinding channel!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPVF_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPVF_EN, IPU_IC_CONF_PRPVF_EN_DISABLE);

        // disable pre-processing vf path in IC
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

        IDMACChannelDisable(IDMAC_CH_PRP_INPUT_VIDEO);
        if(!m_bDMFCChannelUsing)
            IDMACChannelDisable(IDMAC_CH_PRP_OUTPUT_VF);
        if(m_bVFCombEnabled)
            IDMACChannelDisable(IDMAC_CH_PRP_INPUT_GRAPHICS);
            
        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_PRP_INPUT_VIDEO,FALSE);
        IDMACChannelDBMODE(IDMAC_CH_PRP_OUTPUT_VF,FALSE);
        if(m_bVFCombEnabled)
            IDMACChannelDBMODE(IDMAC_CH_PRP_INPUT_GRAPHICS,FALSE);

    }

    if(m_bENCPrpEnabled)
    {
        uCount = 0;
        while(IDMACChannelIsBusy(IDMAC_CH_PRP_INPUT_VIDEO)
            ||IDMACChannelIsBusy(IDMAC_CH_PRP_OUTPUT_ENC)
            ||PrpIsBusy())
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
                    (TEXT("%s(): Error in stopping prp encoding channel!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        // Compute bitmask and shifted bit value for IC_CONF register.
        iMask = CSP_BITFMASK(IPU_IC_CONF_PRPENC_EN);
        iBitval = CSP_BITFVAL(IPU_IC_CONF_PRPENC_EN, IPU_IC_CONF_PRPENC_EN_DISABLE);

        // enable pre-processing enc path in IC
        do
        {
            oldVal = INREG32(&m_pIC->IC_CONF);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&m_pIC->IC_CONF, 
                    oldVal, newVal) != oldVal);

        IDMACChannelDisable(IDMAC_CH_PRP_INPUT_VIDEO);
        IDMACChannelDisable(IDMAC_CH_PRP_OUTPUT_ENC);
            
        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_PRP_INPUT_VIDEO,FALSE);
        IDMACChannelDBMODE(IDMAC_CH_PRP_OUTPUT_ENC,FALSE);

    }
    PrpDisable();
    LeaveCriticalSection(&m_csStopping);

    if(m_bSYNCDPEnabled)
    {
        //disable the colorkey for foreground
        DPCSCGetCoeffs(DP_CHANNEL_SYNC, &DpCSCConfigData);
        if (DpCSCConfigData.CSCPosition == DP_CSC_BOTH)
        {
            m_bSYNCDPCSCChanged = TRUE;
            DpCSCConfigData.CSCPosition = DP_CSC_BG;
        }
        else if(DpCSCConfigData.CSCPosition == DP_CSC_FG)
        {
            m_bSYNCDPCSCChanged = TRUE;
            DpCSCConfigData.CSCPosition = DP_CSC_DISABLE;
        }
        DPCSCConfigure(DP_CHANNEL_SYNC, &DpCSCConfigData);
        
        if(m_bSYNCisInterlaced)
        {
            //Makesure we set graphic channel disable flag when field1 is updating.
            //Then graphic channel will be disabled after whole frame is updated.
            uCount = 0;
            while(!CPMEMIsCurrentField1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
            {
                if (uCount <= 200)
                {
                    //..give up the remainder of time slice
                    Sleep(1);
                    uCount++;
                }
                else
                {
                    //.. there is something wrong ....break out
                    RETAILMSG(1,
                        (TEXT("%s(): Error in waiting field1!\r\n"),__WFUNCTION__));
                    break;
                }    
            }
        }
        DPGraphicWindowEnable(DP_CHANNEL_SYNC,FALSE);
        
        if(m_bSYNCisInterlaced)
        {
            //Makesure the DP_SF_END is generated after disabling graphic window.
            //Then IDMA can be disabled safely.
            uCount = 0;
            while(CPMEMIsCurrentField1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
            {
                if (uCount <= 200)
                {
                    //..give up the remainder of time slice
                    Sleep(1);
                    uCount++;
                }
                else
                {
                    //.. there is something wrong ....break out
                    RETAILMSG(1,
                        (TEXT("%s(): Error in waiting field1!\r\n"),__WFUNCTION__));
                    break;
                }    
            }
        }
        
        //WAIT FOR dp_sf_end (Sync Flow End)
        uCount = 0;
        IDMACChannelClearIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL);
        while(!IDMACChannelGetIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL))
        {
            if (uCount <= 200)
            {
                //..give up the remainder of time slice
                Sleep(1);
                uCount++;
            }
            else
            {
                //.. there is something wrong ....break out
                RETAILMSG(1,
                    (TEXT("%s(): Error in waiting DP_SF_END signal!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        if(m_bSYNCisDI1)
            DCWait4TripleBufEmpty(DI_SELECT_DI1);
        else
            DCWait4TripleBufEmpty(DI_SELECT_DI0);
        
        IDMACChannelDisable(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
        IDMACChannelDBMODE(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE,FALSE);

    }



    if(m_bASYNCDPEnabled)
    {
        //disable the colorkey for foreground
        DPCSCGetCoeffs(DP_CHANNEL_ASYNC0, &DpCSCConfigData);
        if (DpCSCConfigData.CSCPosition == DP_CSC_BOTH)
        {
            m_bASYNCDPCSCChanged = TRUE;
            DpCSCConfigData.CSCPosition = DP_CSC_BG;
        }
        else if(DpCSCConfigData.CSCPosition == DP_CSC_FG)
        {
            m_bASYNCDPCSCChanged = TRUE;
            DpCSCConfigData.CSCPosition = DP_CSC_DISABLE;
        }
        DPCSCConfigure(DP_CHANNEL_ASYNC0, &DpCSCConfigData);

        uCount = 0;
        while(IDMACChannelIsBusy(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE)
            ||DPIsBusy(DP_CHANNEL_ASYNC0))
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
                    (TEXT("%s(): Error in stopping DP async channel!\r\n"),__WFUNCTION__));
                break;
            }    
        }
     
        DPGraphicWindowEnable(DP_CHANNEL_ASYNC0,FALSE);

        IDMACChannelDisable(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE);

        IDMACChannelDBMODE(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE,FALSE);

        //To clean the picture
        while(DPIsBusy(DP_CHANNEL_ASYNC0))
        {
            Sleep(1);
        }
        IDMACChannelBUF0SetReady(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE);

    }

    PRP_FUNCTION_EXIT();

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
void PrpClass::PrpDisable(void)
{
    IPU_DRIVER driver = IPU_DRIVER_PRP_VF;

    PRP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPrpEnable);

    // If either the ENC or VF channels is still running, then
    // we should NOT disable the IC.
    if (m_bRunning)
    {
        LeaveCriticalSection(&m_csPrpEnable);
        return;
    }

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Disabling IC!\r\n"), __WFUNCTION__));

    if(!IPUV3DisableModule(hIPUBase, IPU_SUBMODULE_IC, driver))
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to disable IC!\r\n"), __WFUNCTION__));
    }


    LeaveCriticalSection(&m_csPrpEnable);

    PRP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: IRTDisable
//
// Disable the Image Rotator.
//
// Parameters:
//      driver
//          [in] Identifies which IRT should be disabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PrpClass::IRTDisable(IPU_DRIVER driver)
{
    PRP_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&m_csPrpEnable);

    // If either the ENC or VF channels is still running, then
    // we should NOT disable the IRT.
    if (m_bRunning)
    {
        LeaveCriticalSection(&m_csPrpEnable);
        return;
    }

    DEBUGMSG (ZONE_ERROR,
        (TEXT("%s: Disabling IRT!\r\n"), __WFUNCTION__));

    if(!IPUV3DisableModule(hIPUBase, IPU_SUBMODULE_IRT, driver))
    {
        DEBUGMSG (ZONE_ERROR,
            (TEXT("%s: Failed to disable IRT!\r\n"), __WFUNCTION__));
    }


    LeaveCriticalSection(&m_csPrpEnable);

    PRP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PrpGetResizeCoeffs
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
BOOL PrpClass::PrpGetResizeCoeffs(UINT16 inSize, UINT16 outSize, pPrpResizeCoeffs resizeCoeffs)
{
    UINT16 tempSize;
    UINT16 tempDownsize;
    UINT16 tempOutSize;

    PRP_FUNCTION_ENTRY();

    // TODO: Need check how to get 8:1 downsize ratio
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
    PRP_FUNCTION_EXIT();

    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: PrpIntrEnable
//
// This function is called for enable certain interrupt.
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
void PrpClass::PrpIntrEnable(UINT8 IntType)
{
    if(IntType & FIRSTMODULE_INTERRUPT)
    {
        if(m_PrpFirstModule == prpSourceType_ROT)
        {
            if(m_bVFIRTEnabled)
            {
                IDMACChannelClearIntStatus(IDMAC_CH_IRT_INPUT_PRP_VF, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_IRT_INPUT_PRP_VF, IPU_INTR_TYPE_EOF, TRUE);
            }
            else
            {
                IDMACChannelClearIntStatus(IDMAC_CH_IRT_INPUT_PRP_ENC, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_IRT_INPUT_PRP_ENC, IPU_INTR_TYPE_EOF, TRUE);
            }            
        }
        else if(m_PrpFirstModule == prpSourceType_PRP)
        {
            IDMACChannelClearIntStatus(IDMAC_CH_PRP_INPUT_VIDEO, IPU_INTR_TYPE_EOF);
            IDMACChannelIntCntrl(IDMAC_CH_PRP_INPUT_VIDEO, IPU_INTR_TYPE_EOF, TRUE);
        }        
        else if(m_PrpFirstModule == prpSourceType_ARM)
        {
            if(m_bSYNCDPEnabled)
            {
                IDMACChannelClearIntStatus(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, IPU_INTR_TYPE_EOF, TRUE);
            }
            else
            {
                IDMACChannelClearIntStatus(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, IPU_INTR_TYPE_EOF);
                IDMACChannelIntCntrl(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, IPU_INTR_TYPE_EOF, TRUE);
            }
        }
        else
        {
            return;
        }
    }
    //frame interrupt only support vf channel
    if(IntType & FRAME_INTERRUPT)
    {
        if(m_PrpVFLastModule == prpSourceType_ROT)
        {
            IDMACChannelClearIntStatus(IDMAC_CH_IRT_OUTPUT_PRP_VF, IPU_INTR_TYPE_EOF);
            IDMACChannelIntCntrl(IDMAC_CH_IRT_OUTPUT_PRP_VF, IPU_INTR_TYPE_EOF, TRUE);
        }
        else if(m_PrpVFLastModule == prpSourceType_PRP)
        {
            IDMACChannelClearIntStatus(IDMAC_CH_PRP_OUTPUT_VF, IPU_INTR_TYPE_EOF);
            IDMACChannelIntCntrl(IDMAC_CH_PRP_OUTPUT_VF, IPU_INTR_TYPE_EOF, TRUE);
        }        
        else if(m_PrpVFLastModule == prpSourceType_ARM)
        {
            return;
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
// Function: PrpWaitForNotBusy
//
// This function is called for waiting certain interrupt.
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
void PrpClass::PrpWaitForNotBusy(UINT8 IntType)
{
    UINT32 timeout = 500;
    
    if(IntType & FIRSTMODULE_INTERRUPT)
    {
        if(m_PrpFirstModule == prpSourceType_ROT)
        {
            if (WaitForSingleObject(m_hPrpIntrEvent, timeout) == WAIT_TIMEOUT)
            {
                DEBUGMSG(ZONE_ERROR,
                         (TEXT("%s(): Waiting for PRP ROT EOF interrupt time out!\r\n"), __WFUNCTION__));
            }
        }
        else if(m_PrpFirstModule == prpSourceType_PRP)
        {
            if (WaitForSingleObject(m_hPrpIntrEvent, timeout) == WAIT_TIMEOUT)
            {
                RETAILMSG(1,
                         (TEXT("%s(): Waiting for PRP EOF interrupt time out!\r\n"), __WFUNCTION__));

                //The following code is a workaround for FSU logic error issue.
                //Some time, the FSU may forget the set the buffer ready flag of prp output channel which causes whole prp channel hangs.
                //Current workaround is set the flag manually.
                if(m_bVFPrpEnabled)
                {
                    if(IDMACChannelBUF0IsReady(IDMAC_CH_PRP_INPUT_VIDEO))
                        IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_VF);
                    else
                        IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_VF);
                }
                if(m_bENCPrpEnabled)
                {
                    if(IDMACChannelBUF0IsReady(IDMAC_CH_PRP_INPUT_VIDEO))
                        IDMACChannelBUF0SetReady(IDMAC_CH_PRP_OUTPUT_ENC);
                    else
                        IDMACChannelBUF1SetReady(IDMAC_CH_PRP_OUTPUT_ENC);                    
                }
                
                //After the flag is set manually, wait again to try to finish the task before this function returns.
                if (WaitForSingleObject(m_hPrpIntrEvent, timeout) == WAIT_TIMEOUT)
                {
                    RETAILMSG(1,
                             (TEXT("%s(): Waiting for PRP EOF interrupt time out again!\r\n"), __WFUNCTION__));
                    CMDumpRegs();
                }
                else
                {
                    RETAILMSG(1,
                             (TEXT("%s(): The workaround for PRP EOF interrupt lost works!\r\n"), __WFUNCTION__));
                    
                }
            }
        }        
        else if(m_PrpFirstModule == prpSourceType_ARM)
        {
            if (WaitForSingleObject(m_hDPFGIntrEvent, timeout) == WAIT_TIMEOUT)
            {
                DEBUGMSG(ZONE_ERROR,
                         (TEXT("%s(): Waiting for DP FG EOF interrupt time out!\r\n"), __WFUNCTION__));
            }
        }
        else
            return;
    }
    else if(IntType & FRAME_INTERRUPT)
    {
        //frame interrupt only support vf channel
        if((m_PrpVFLastModule == prpSourceType_ROT)
        ||(m_PrpVFLastModule == prpSourceType_PRP))
        {
            if (WaitForSingleObject(m_hPrpIntrEvent, timeout) == WAIT_TIMEOUT)
            {
                DEBUGMSG(ZONE_ERROR,
                         (TEXT("%s(): Waiting for PRP/ROT EOF interrupt time out!\r\n"), __WFUNCTION__));
            }
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
// Function: PrpClearBuffers
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
BOOL PrpClass::PrpClearBuffers()
{
    if(m_hVFRotBuffer)
    {
        IPUV3DeallocateBuffer(hIPUBase, m_hVFRotBuffer);
        m_hVFRotBuffer = NULL;
    }

    if(m_hENCRotBuffer)
    {
        IPUV3DeallocateBuffer(hIPUBase, m_hENCRotBuffer);
        m_hENCRotBuffer = NULL;
    }

    if(m_hVFDpBuffer)
    {
        IPUV3DeallocateBuffer(hIPUBase, m_hVFDpBuffer);
        m_hVFDpBuffer = NULL;
    }
    
    if(m_hENCDpBuffer)
    {
        IPUV3DeallocateBuffer(hIPUBase, m_hENCDpBuffer);
        m_hENCDpBuffer = NULL;
    }

    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: PrpBufferStatus
//
// This method gets the first module status of Pre-processor processing chain.
//
// Parameters:
//      pStatus
//          [out] Pointer to status.
//
// Returns:
//      TRUE if success.
//
//------------------------------------------------------------------------------
BOOL PrpClass::PrpBufferStatus(icBufferStatus * pStatus)
{
    UINT16 InputDMAChannel;
    if (m_bRunning == FALSE)
    {
        *pStatus = BufferIdle;
        return FALSE;
    }
    
    if(m_PrpFirstModule == prpSourceType_PRP)
    {
        InputDMAChannel= IDMAC_CH_PRP_INPUT_VIDEO;
    }
    else if(m_PrpFirstModule == prpSourceType_ROT)
    {
        if(m_bVFIRTEnabled)
            InputDMAChannel= IDMAC_CH_IRT_INPUT_PRP_VF;
        else
            InputDMAChannel= IDMAC_CH_IRT_INPUT_PRP_ENC;
    }
    else if(m_PrpFirstModule == prpSourceType_ARM)
    {
        if(m_bSYNCDPEnabled)
        {
            InputDMAChannel= IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE;
        }
        else
        {
            InputDMAChannel= IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE;
        }
    }
    else
    {
        InputDMAChannel= IDMAC_CH_PRP_INPUT_VIDEO;
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: InputDMAChannel error (%d).\r\n"), __WFUNCTION__, InputDMAChannel));            
    }

    if((IDMACChannelBUF0IsReady(InputDMAChannel)
        &&IDMACChannelCurrentBufIsBuf1(InputDMAChannel))
       ||(IDMACChannelBUF1IsReady(InputDMAChannel)
       &&( !IDMACChannelCurrentBufIsBuf1(InputDMAChannel))))
    {
        *pStatus = TwoBufferBusy;
    }
    else if(IDMACChannelIsBusy(InputDMAChannel)
            &&(InputDMAChannel!= IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
    {
        //For sync panel, one buffer will always be occupied, if we return this it may cause 
        //flip hangs, so skip this check, return BufferIdle instead.
        *pStatus = OneBufferBusy;
    }
    else
    {
        *pStatus = BufferIdle;
    }

    return TRUE;
}

void PrpClass::ICDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&m_pIC->IC_CONF;

    RETAILMSG (1, (TEXT("\n\nIC Registers\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_DMFC_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}

