//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  vdi_config.cpp
//
//  Implementation of video de-interlacer (VDI) driver methods
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
#include "ipu_base.h"
#include "ipu_common.h"
#include "tpm.h"
#include "vdi.h"
#include "vdi_priv.h"
#include "idmac.h"
#include "cpmem.h"
#include "dmfc.h"
#include "cm.h"



//------------------------------------------------------------------------------
// External Functions
extern void BSPSetVDIISRPriority();


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

#define VDI_PIXEL_BURST 4

#define BANDMODE_ENABLE 0
#define DEBUG_DUMP 0
#define PRINT_VDI_TASK_DURATION 0

//------------------------------------------------------------------------------
// Global Variables

HANDLE g_hIPUBase;

// Event variables
HANDLE  g_hVDIIntrEvent;
HANDLE  g_hEOFEvent;

// Thread handles
HANDLE  g_hVDIISRThread;

BOOL g_bConfigured;
VDI_OUTPUT_DEST g_OutputDest;

// If TRUE, the VDI channel has been 
// stopped, so we should wait for the current frame to 
// be completed, and be sure to not re-enable the channel.
BOOL    g_bRunning;

CRITICAL_SECTION g_csStopping;
CRITICAL_SECTION g_csVDIEnable;


//-----------------------------------------------------------------------------
//
// Function: VDIInit
//
// This function initializes the Video De-Interlacer (VDI).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL VDIInit(void)
{
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    VDI_FUNCTION_ENTRY();

    // open handle to the IPU_BASE driver in order to enable IC module
    // First, create handle to IPU_BASE driver
    g_hIPUBase = IPUV3BaseOpenHandle();
    if (g_hIPUBase == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(1,
            (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    dwIPUBaseAddr = IPUV3BaseGetBaseAddr(g_hIPUBase);
    if (dwIPUBaseAddr == -1)
    {
        RETAILMSG (1,
            (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Map IC memory region entries
    phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_VDI_REGS_OFFSET;

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
    if (!VDIRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable VDI Regs!\r\n"), __WFUNCTION__));
    }

    // Initialize VDI registers to acceptable starting values
    VDIMotionSelect(IPU_VDI_C_VDI_MOT_SEL_ROM1);

    VDISetBurstSize(VDI_CHANNEL_1, VDI_PIXEL_BURST);
    VDISetBurstSize(VDI_CHANNEL_2, VDI_PIXEL_BURST);
    VDISetBurstSize(VDI_CHANNEL_3, VDI_PIXEL_BURST);

    //Set it the same as validation code
    // Initial Watermark configuration
    VDISetWatermarkLevel(VDI_CHANNEL_1, IPU_VDI_C_WATERMARK_FIFO_1_8TH);
    VDIClearWatermarkLevel(VDI_CHANNEL_1, IPU_VDI_C_WATERMARK_FIFO_2_8TH);

    VDISetWatermarkLevel(VDI_CHANNEL_3, IPU_VDI_C_WATERMARK_FIFO_1_8TH);
    VDIClearWatermarkLevel(VDI_CHANNEL_3, IPU_VDI_C_WATERMARK_FIFO_2_8TH);

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for IPU Interrupt\r\n"), __WFUNCTION__));
    g_hVDIIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PRP_INTR_EVENT);
    if (g_hVDIIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IPU VDI Interrupt failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Initialize buffer management handles

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Initializing class variables\r\n"), __WFUNCTION__));
    InitializeCriticalSection(&g_csStopping);
    InitializeCriticalSection(&g_csVDIEnable);

    g_hEOFEvent = NULL;
    g_bConfigured = FALSE;
    g_bRunning = FALSE;

    DEBUGMSG(ZONE_INIT, (TEXT("%s: Enabling VDI\r\n"), __WFUNCTION__));

    VDI_FUNCTION_EXIT();

    return TRUE;

Error:
    VDIDeinit();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: VDIDeinit
//
// This function deinitializes the VDI.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDIDeinit(void)
{
    VDI_FUNCTION_ENTRY();

    // Disable Postprocessor
    VDIDisable();

    if (g_hVDIISRThread)
    {
        CloseHandle(g_hVDIISRThread);
        g_hVDIISRThread = NULL;
    }

    CloseHandle(g_hVDIIntrEvent);
    g_hVDIIntrEvent = NULL;

    CloseHandle(g_hEOFEvent);
    g_hEOFEvent = NULL;

    DeleteCriticalSection(&g_csStopping);
    DeleteCriticalSection(&g_csVDIEnable);

    g_bConfigured = FALSE;

    VDI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: VDIConfigure
//
// This function configures the VDI registers and IDMAC
// channels for the VDI channel.
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
BOOL VDIConfigure(pVdiConfigData pConfigData)
{
    DWORD dwVDIPixFormat;
    VDI_FUNCTION_ENTRY();

    // Check configuration parameters
    if(!VDIParamsCheck(pConfigData))
    {
         ERRORMSG(1,
             (TEXT("%s: Incorrect configuration data. \r\n"), __WFUNCTION__ ));
         return FALSE;
    }

    g_OutputDest = pConfigData->outputDst;

    // ****** Untested feature...we only support output to PRP currently ******
    // When VDI output goes directly to Memory, we must spawn a thread to handle EOF
    if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
    {
        // Only create event and thread once
        if (g_hEOFEvent == NULL)
        {
            DEBUGMSG(ZONE_INFO, (TEXT("%s: CreateEvent for VDI EOF event\r\n"), __WFUNCTION__));
            // Events to signal pin that frame is ready
            g_hEOFEvent = CreateEvent(NULL, FALSE, FALSE, VDI_EOF_EVENT_NAME);
            if (g_hEOFEvent == NULL)
            {
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("%s: CreateEvent failed for VDI EOF\r\n"), __WFUNCTION__));
                return FALSE;
            }

            DEBUGMSG(ZONE_INFO, (TEXT("%s: Spawning threads\r\n"), __WFUNCTION__));

            // Initialize thread for Postprocessor ISR
            //      pThreadAttributes = NULL (must be NULL)
            //      dwStackSize = 0 => default stack size determined by linker
            //      lpStartAddress = CspiProcessQueue => thread entry point
            //      lpParameter = NULL => point to thread parameter
            //      dwCreationFlags = 0 => no flags
            //      lpThreadId = NULL => thread ID is not returned
            PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
            g_hVDIISRThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)VDIIntrThread, 0, 0, NULL);

            if (g_hVDIISRThread == NULL)
            {
                DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
                return FALSE;
            }
            else
            {
                DEBUGMSG(ZONE_INFO, (TEXT("%s: create VDI ISR thread success\r\n"), __WFUNCTION__));
            }
        }
    }

    IDMACChannelBUF0ClrReady(IDMAC_CH_VDI_INPUT_PREV_FIELD);
    IDMACChannelBUF0ClrReady(IDMAC_CH_VDI_INPUT_CUR_FIELD);
    IDMACChannelBUF0ClrReady(IDMAC_CH_VDI_INPUT_NEXT_FIELD);
    IDMACChannelBUF1ClrReady(IDMAC_CH_VDI_INPUT_PREV_FIELD);
    IDMACChannelBUF1ClrReady(IDMAC_CH_VDI_INPUT_CUR_FIELD);
    IDMACChannelBUF1ClrReady(IDMAC_CH_VDI_INPUT_NEXT_FIELD);
    IDMACChannelClrCurrentBuf(IDMAC_CH_VDI_INPUT_PREV_FIELD);
    IDMACChannelClrCurrentBuf(IDMAC_CH_VDI_INPUT_CUR_FIELD);
    IDMACChannelClrCurrentBuf(IDMAC_CH_VDI_INPUT_NEXT_FIELD);

    // Only the 2 4:2:0 planar formats correspond to 420 format...otherwise, 
    // format is 422
    if ((pConfigData->dwPixelFormat == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
        || (pConfigData->dwPixelFormat == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
    {
        dwVDIPixFormat = IPU_VDI_C_VDI_CH_422_FORMAT_420;
    }
    else
    {
        dwVDIPixFormat = IPU_VDI_C_VDI_CH_422_FORMAT_422;
    }

    // Set up VDI registers based on config info
    VDISetFieldDimensions(pConfigData->iWidth-1, pConfigData->iHeight-1);
    VDISetPixelFormat(dwVDIPixFormat);
    VDIMotionSelect(pConfigData->dwMotionSel);
    VDISetTopField(pConfigData->inputSrc, pConfigData->dwTopField);
    // Configure the processing flow
    CMSetProcFlow(IPU_PROC_FLOW_VDI_SRC, pConfigData->inputSrc);

    // Configure output
    if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
    {
        // TODO:  configure the output destination.
        // if VDI wants to output data to memory through ch5, the CSI_MEM_WR_EN bit in IC_CONF reg must be set.
        // by default the output is PRP FIFO.
    }

    // Configure VDI channels
    VDIIDMACChannelConfig(pConfigData);

    // VDI has been configured, so we can now start
    g_bConfigured = TRUE;

    VDI_FUNCTION_EXIT();
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: VDIParamsCheck
//
// This function checks all configure parameters for invalid parameters.
//
// Parameters:
//      pConfigData
//          [in] Pointer to configuration data structure
//
// Returns:
//      TRUE if parameters pass the validity check
//      FALSE if parameters fail validity check
//
//-----------------------------------------------------------------------------
BOOL VDIParamsCheck(pVdiConfigData pConfigData)
{
    // Check if pixel format is supported.
    switch (pConfigData->dwPixelFormat)
    {
        case IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420:
        case IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420:
        case IDMAC_INTERLEAVED_FORMAT_CODE_UYVY2:
        case IDMAC_INTERLEAVED_FORMAT_CODE_YUY2V:
        case IDMAC_INTERLEAVED_FORMAT_CODE_Y2UYV:
        case IDMAC_INTERLEAVED_FORMAT_CODE_UY2VY:
            // only above formats are supported
            break;

        default:
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: unsupported pixel format.\r\n"), __WFUNCTION__));
            return FALSE;
            break;
    }

    // Boundary check
    if((pConfigData->iWidth> VDI_MAX_INPUT_WIDTH) ||
        (pConfigData->iHeight > VDI_MAX_INPUT_HEIGHT))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Error frame dimensions: \r\n"), __WFUNCTION__));
        DEBUGMSG(ZONE_ERROR,
                (TEXT("\t Input Size: width (%d), height (%d)\r\n"),
                pConfigData->iWidth,
                pConfigData->iHeight));
        return FALSE;
    }

    // Alignment check
    if((pConfigData->iWidth & 0x07) ||
        (pConfigData->iHeight & 0x01))
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error input size, w (%d), h (%d)! \r\n"), __WFUNCTION__,
                pConfigData->iWidth,
                pConfigData->iHeight));
        return FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: VDIIDMACChannelConfig
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
void VDIIDMACChannelConfig(pVdiConfigData pConfigData)
{
    CPMEMConfigData CPMEMData;
    CPMEMBufOffsets CPMEMOffsetData;

    memset(&CPMEMData, 0, sizeof(CPMEMData));
    CPMEMData.iDecAddrSelect = CPMEM_DEC_SEL_ADDR_0_TO_15; // Don't care, only for 4BPP mode   //not support yet
    CPMEMData.iAccessDimension = CPMEM_DIM_2D; // 1D scan is only used for csi generic data
    CPMEMData.iScanOrder = CPMEM_SO_PROGRESSIVE; //always progressive, not support interlaced yet
    CPMEMData.iThresholdEnable = CPMEM_THE_DISABLE;
    CPMEMData.iThreshold = 0;
    CPMEMData.iCondAccessEnable = CPMEM_CAE_COND_SKIP_DISABLE;    // for future use
    CPMEMData.iCondAccessPolarity = CPMEM_CAP_COND_SKIP_LOW;  // for future use
    CPMEMData.iAlphaUsed = CPMEM_ALU_ALPHA_SAME_CHANNEL;  // seperate alpha not support yet
    CPMEMData.iAXI_Id = CPMEM_ID_1;     //ID_0 has high priority to access memory, only for sync display channel
    

    CPMEMData.iBandMode = IDMAC_CH_BAND_MODE_DISABLE;
    IDMACChannelSetBandMode(IDMAC_CH_VDI_INPUT_PREV_FIELD, IDMAC_CH_BAND_MODE_DISABLE);
    IDMACChannelSetBandMode(IDMAC_CH_VDI_INPUT_CUR_FIELD, IDMAC_CH_BAND_MODE_DISABLE);
    IDMACChannelSetBandMode(IDMAC_CH_VDI_INPUT_NEXT_FIELD, IDMAC_CH_BAND_MODE_DISABLE);
    IDMACChannelSetBandMode(IDMAC_CH_VDI_OUTPUT, IDMAC_CH_BAND_MODE_DISABLE);

    CPMEMData.iBlockMode = 0;
    CPMEMData.iRotation90 = 0;
    CPMEMData.iFlipHoriz = 0;
    CPMEMData.iFlipVert = 0;

    if(pConfigData->iWidth%16)
        CPMEMData.iPixelBurst = 7;

    else
        CPMEMData.iPixelBurst = 15;

    //Each VDI access is 4 pixels.
    VDISetBurstSize(VDI_CHANNEL_1, (CPMEMData.iPixelBurst+1)/4);
    VDISetBurstSize(VDI_CHANNEL_2, (CPMEMData.iPixelBurst+1)/4);
    VDISetBurstSize(VDI_CHANNEL_3, (CPMEMData.iPixelBurst+1)/4);

    if((pConfigData->dwPixelFormat == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
        || (pConfigData->dwPixelFormat == IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420))
    {
        CPMEMData.iBitsPerPixel = CPMEM_BPP_8;
        CPMEMOffsetData.bInterleaved = FALSE;
    }
    else
    {
        // Using YUV422 interleaved format (16 bpp)
        CPMEMData.iBitsPerPixel = CPMEM_BPP_16;
        CPMEMOffsetData.bInterleaved = TRUE;
    }

    CPMEMData.iHeight = pConfigData->iHeight/2 -1;
    CPMEMData.iWidth = pConfigData->iWidth -1;
    CPMEMData.iLineStride = pConfigData->iStride * 2 -1;

    CPMEMData.iPixelFormat = (UINT8)pConfigData->dwPixelFormat;

    // Additional configuration steps for planar formats - U/V offsets, Y/UV strides
    if (CPMEMOffsetData.bInterleaved == FALSE)
    {
        CPMEMOffsetData.interlaceOffset  = 0;
        CPMEMOffsetData.uOffset= pConfigData->Uoffset;
        CPMEMOffsetData.vOffset= pConfigData->Voffset;
        CPMEMWriteOffset(IDMAC_CH_VDI_INPUT_PREV_FIELD,&CPMEMOffsetData,CPMEMOffsetData.bInterleaved);
        CPMEMWriteOffset(IDMAC_CH_VDI_INPUT_CUR_FIELD,&CPMEMOffsetData,CPMEMOffsetData.bInterleaved);
        CPMEMWriteOffset(IDMAC_CH_VDI_INPUT_CUR_FIELD,&CPMEMOffsetData,CPMEMOffsetData.bInterleaved);

        CPMEMData.iLineStride_Y = CPMEMData.iLineStride;
        if(pConfigData->dwPixelFormat == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
        {
                // Divide stride by 2 for UV
            CPMEMData.iLineStride_UV = ((CPMEMData.iLineStride_Y+1) >>1) -1;
        }
        else
        {
                // For partially interleaved UV, UV stride = 2 * (Stride_Y / 2) = Stride_Y
            CPMEMData.iLineStride_UV = CPMEMData.iLineStride_Y;        
        }
    }
    CPMEMWrite(IDMAC_CH_VDI_INPUT_PREV_FIELD, &CPMEMData, CPMEMOffsetData.bInterleaved);
    CPMEMWrite(IDMAC_CH_VDI_INPUT_CUR_FIELD, &CPMEMData, CPMEMOffsetData.bInterleaved);
    CPMEMWrite(IDMAC_CH_VDI_INPUT_NEXT_FIELD, &CPMEMData, CPMEMOffsetData.bInterleaved);
    
    // for VDI, output channels is always progressive
    // TODO: Examine use case for using VDI Output channel.
    // VDI Output by default goes directly to CSI.
    if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
    {
        // Adjust a few settings that are different for the output channel
        CPMEMData.iHeight = pConfigData->iHeight -1;
        CPMEMData.iLineStride = pConfigData->iStride -1;
        CPMEMData.iLineStride_Y = CPMEMData.iLineStride;
        // Divide stride by 2 for UV
        if(pConfigData->dwPixelFormat == IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420)
        {
            CPMEMData.iLineStride_UV = ((CPMEMData.iLineStride_Y+1) >>1) -1;
        }
        else
        {
            CPMEMData.iLineStride_UV = CPMEMData.iLineStride_Y;        
        }

        // Now write to configure CPMEM for output channel
        CPMEMWrite(IDMAC_CH_VDI_OUTPUT, &CPMEMData, CPMEMOffsetData.bInterleaved);
    }

#if DEBUG_DUMP
    CPMEMDumpRegs(IDMAC_CH_VDI_INPUT_PREV_FIELD);
    CPMEMDumpRegs(IDMAC_CH_VDI_INPUT_CUR_FIELD);
    CPMEMDumpRegs(IDMAC_CH_VDI_INPUT_NEXT_FIELD);
    CPMEMDumpRegs(IDMAC_CH_VDI_OUTPUT);
#endif

}

//-----------------------------------------------------------------------------
//
// Function: VDIStartChannel
//
// This function starts the VDI channel.
//
// Parameters:
//      pStartData
//          [in] Buffer pointer data needed to start VDI task.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL VDIStartChannel(pVdiStartParams pStartData)
{
    CPMEMBufOffsets CPMEMOffsetData;
    
    VDI_FUNCTION_ENTRY();

    // VDI must have been configured at least once
    // in order to start the channel
    if (!g_bConfigured)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error.  Cannot start post-processing channel without first configuring\r\n"),
            __WFUNCTION__));
        return FALSE;
    }

    // Alignment check on U and V buffer offsets
    if((pStartData->prevField.uOffset & 0x07) ||
        (pStartData->prevField.vOffset & 0x07))
    {
        DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: Error input offset, u (%d), v (%d)! \r\n"), __WFUNCTION__,
                pStartData->prevField.uOffset,
                pStartData->prevField.vOffset));
        return FALSE;
    }

    if (!g_bRunning)
    {
        // Reset EOF event, in case stale EOF events were triggered,
        // which would cause Pin to believe a frame was ready.

        // Enable Postprocessor
        VDIEnable();

        // Compute bitmask and shifted bit value for DB mode sel register.
        IDMACChannelDBMODE(IDMAC_CH_VDI_INPUT_PREV_FIELD, TRUE);
        IDMACChannelDBMODE(IDMAC_CH_VDI_INPUT_CUR_FIELD, TRUE);
        IDMACChannelDBMODE(IDMAC_CH_VDI_INPUT_NEXT_FIELD, TRUE);

        IDMACChannelEnable(IDMAC_CH_VDI_INPUT_PREV_FIELD);
        IDMACChannelEnable(IDMAC_CH_VDI_INPUT_CUR_FIELD);
        IDMACChannelEnable(IDMAC_CH_VDI_INPUT_NEXT_FIELD);

        // Untested feature...we only support output to PRP currently
        if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
        {
            IDMACChannelDBMODE(IDMAC_CH_VDI_OUTPUT, TRUE);
            IDMACChannelEnable(IDMAC_CH_VDI_OUTPUT);
        }

        g_bRunning = TRUE;
    }

    // Enable interrupt
    VDIIntrEnable();

    if(pStartData->prevField.uOffset != pStartData->prevField.vOffset)
    {
        // Disable IDMAC channels so that we can edit  U and V offsets in CPMEM
        IDMACChannelDisable(IDMAC_CH_VDI_INPUT_PREV_FIELD);
        IDMACChannelDisable(IDMAC_CH_VDI_INPUT_CUR_FIELD);
        IDMACChannelDisable(IDMAC_CH_VDI_INPUT_NEXT_FIELD);
        if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
        {
            IDMACChannelDisable(IDMAC_CH_VDI_OUTPUT);
        }

        // Update CPMEM U and V offsets for all channels involved
        CPMEMOffsetData.bInterleaved = FALSE;
        CPMEMOffsetData.interlaceOffset  = 0;
        CPMEMOffsetData.uOffset= pStartData->prevField.uOffset;
        CPMEMOffsetData.vOffset= pStartData->prevField.vOffset;
        CPMEMWriteOffset(IDMAC_CH_VDI_INPUT_PREV_FIELD,&CPMEMOffsetData,FALSE);
        CPMEMOffsetData.uOffset= pStartData->curField.uOffset;
        CPMEMOffsetData.vOffset= pStartData->curField.vOffset;
        CPMEMWriteOffset(IDMAC_CH_VDI_INPUT_CUR_FIELD,&CPMEMOffsetData,FALSE);
        CPMEMOffsetData.uOffset= pStartData->nextField.uOffset;
        CPMEMOffsetData.vOffset= pStartData->nextField.vOffset;
        CPMEMWriteOffset(IDMAC_CH_VDI_INPUT_NEXT_FIELD,&CPMEMOffsetData,FALSE);
        if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
        {
            CPMEMOffsetData.uOffset= pStartData->outputFrame.uOffset;
            CPMEMOffsetData.vOffset= pStartData->outputFrame.vOffset;
            CPMEMWriteOffset(IDMAC_CH_VDI_OUTPUT,&CPMEMOffsetData,FALSE);
        }

        // Re-enable IDMAC channels after updating U and V offsets in CPMEM
        IDMACChannelEnable(IDMAC_CH_VDI_INPUT_PREV_FIELD);
        IDMACChannelEnable(IDMAC_CH_VDI_INPUT_CUR_FIELD);
        IDMACChannelEnable(IDMAC_CH_VDI_INPUT_NEXT_FIELD);
        if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
        {
            IDMACChannelEnable(IDMAC_CH_VDI_OUTPUT);
        }
    }

    // Set CPMEM buffers and Buf_Rdy bits based on which buffer is the next to become active
    if (IDMACChannelCurrentBufIsBuf1(IDMAC_CH_VDI_INPUT_CUR_FIELD)) 
    {
        CPMEMWriteBuffer(IDMAC_CH_VDI_INPUT_PREV_FIELD, 0, pStartData->prevField.yBufPtr);
        CPMEMWriteBuffer(IDMAC_CH_VDI_INPUT_CUR_FIELD, 0, pStartData->curField.yBufPtr);
        CPMEMWriteBuffer(IDMAC_CH_VDI_INPUT_NEXT_FIELD, 0, pStartData->nextField.yBufPtr);
        if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
        {
            CPMEMWriteBuffer(IDMAC_CH_VDI_OUTPUT, 0, pStartData->outputFrame.yBufPtr);
        }
        
        IDMACChannelBUF0SetReady(IDMAC_CH_VDI_INPUT_PREV_FIELD);
        IDMACChannelBUF0SetReady(IDMAC_CH_VDI_INPUT_CUR_FIELD);
        IDMACChannelBUF0SetReady(IDMAC_CH_VDI_INPUT_NEXT_FIELD);
        if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
        {
            IDMACChannelBUF0SetReady(IDMAC_CH_VDI_OUTPUT);
        }
    }
    else
    {
        CPMEMWriteBuffer(IDMAC_CH_VDI_INPUT_PREV_FIELD, 1, pStartData->prevField.yBufPtr);
        CPMEMWriteBuffer(IDMAC_CH_VDI_INPUT_CUR_FIELD, 1, pStartData->curField.yBufPtr);
        CPMEMWriteBuffer(IDMAC_CH_VDI_INPUT_NEXT_FIELD, 1, pStartData->nextField.yBufPtr);
        if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
        {
            CPMEMWriteBuffer(IDMAC_CH_VDI_OUTPUT, 1, pStartData->outputFrame.yBufPtr);
        }

        IDMACChannelBUF1SetReady(IDMAC_CH_VDI_INPUT_PREV_FIELD);
        IDMACChannelBUF1SetReady(IDMAC_CH_VDI_INPUT_CUR_FIELD);
        IDMACChannelBUF1SetReady(IDMAC_CH_VDI_INPUT_NEXT_FIELD);
        if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
        {
            IDMACChannelBUF1SetReady(IDMAC_CH_VDI_OUTPUT);
        }
    }

    //wait for VDI finish
    VDIWaitForEOF(1000);
  
    VDI_FUNCTION_EXIT();

#if DEBUG_DUMP
    VDIDumpRegs();
#endif

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: VDIStopChannel
//
// This function halts the VDI channels.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL VDIStopChannel(void)
{
    UINT32 uCount = 0;
    DWORD idmacWaitChan;

    VDI_FUNCTION_ENTRY();

    EnterCriticalSection(&g_csStopping);

    // If not running, return
    if (g_bRunning == FALSE)
    {
        LeaveCriticalSection(&g_csStopping);
        return TRUE;
    }

    g_bRunning = FALSE;

    // Disable VDI tasks.  Tasks will stop on completion
    // of the current frame.

    if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
    {
        idmacWaitChan = IDMAC_CH_VDI_OUTPUT;
    }
    else
    {
        idmacWaitChan = IDMAC_CH_PRP_OUTPUT_VF;
    }

    uCount = 0;
    while(IDMACChannelIsBusy(idmacWaitChan))
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
                (TEXT("%s(): Error in stopping VDI channel!\r\n"),__WFUNCTION__));
            break;
        }
    }

    IDMACChannelDisable(IDMAC_CH_VDI_INPUT_PREV_FIELD);
    IDMACChannelDisable(IDMAC_CH_VDI_INPUT_CUR_FIELD);
    IDMACChannelDisable(IDMAC_CH_VDI_INPUT_NEXT_FIELD);

    // Compute bitmask and shifted bit value for DB mode sel register.
    IDMACChannelDBMODE(IDMAC_CH_VDI_INPUT_PREV_FIELD, FALSE);
    IDMACChannelDBMODE(IDMAC_CH_VDI_INPUT_CUR_FIELD, FALSE);
    IDMACChannelDBMODE(IDMAC_CH_VDI_INPUT_NEXT_FIELD, FALSE);

    // Untested feature...we only support output to PRP currently
    if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
    {
        IDMACChannelDisable(IDMAC_CH_VDI_OUTPUT);
        IDMACChannelDBMODE(IDMAC_CH_VDI_OUTPUT, FALSE);
    }
    
    VDIDisable();

    VDI_FUNCTION_EXIT();

    LeaveCriticalSection(&g_csStopping);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: VDIEnable
//
// Enable the VDI and the IDMAC channels we will need.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDIEnable(void)
{
    VDI_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&g_csVDIEnable);

    if (g_bRunning)
    {
        LeaveCriticalSection(&g_csVDIEnable);
        return;
    }

    VDIModuleEnable();

    LeaveCriticalSection(&g_csVDIEnable);

    VDI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: VDIDisable
//
// Disable the VDI.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDIDisable(void)
{
    VDI_FUNCTION_ENTRY();

    // Protect from being disabled and enabled at the same time
    EnterCriticalSection(&g_csVDIEnable);

    if (g_bRunning)
    {
        LeaveCriticalSection(&g_csVDIEnable);
        return;
    }

    VDIModuleDisable();

    LeaveCriticalSection(&g_csVDIEnable);

    VDI_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: VDIIntrEnable
//
// This function enables the VDI interrupt.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void VDIIntrEnable()
{
    if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
    {
        IDMACChannelClearIntStatus(IDMAC_CH_VDI_OUTPUT, IPU_INTR_TYPE_EOF);
        IDMACChannelIntCntrl(IDMAC_CH_VDI_OUTPUT, IPU_INTR_TYPE_EOF, TRUE);
    }
    else
    {
        IDMACChannelClearIntStatus(IDMAC_CH_PRP_OUTPUT_VF, IPU_INTR_TYPE_EOF);
        IDMACChannelIntCntrl(IDMAC_CH_PRP_OUTPUT_VF, IPU_INTR_TYPE_EOF, TRUE);
    }
}

//-----------------------------------------------------------------------------
//
// Function: VDIWaitForEOF
//
// This function is a blocking call that will wait until the VDI task
// has completed, or until the timeout value has been reached.
//
// Parameters:
//      timeout
//          [in] value, in milliseconds, to wait before returning
//
// Returns:
//      TRUE if VDI->PRP EOF successfully received
//      FALSE if we hit timeout and returned
//
//-----------------------------------------------------------------------------
BOOL VDIWaitForEOF(UINT32 timeout)
{
    BOOL retVal = TRUE;

    UINT32 time = GetTickCount();

    if (g_OutputDest == VDI_OUTPUT_DEST_MEM)
    {
        // Untested feature...we only support output to PRP currently

        if (WaitForSingleObject(g_hEOFEvent, timeout) == WAIT_TIMEOUT)
        {
            ERRORMSG(1, (TEXT("%s(): Waiting for VDI->Mem EOF interrupt time out!\r\n"), __WFUNCTION__));
            CMDumpRegs();
            VDIDumpRegs();
            retVal = FALSE;
        }

        time = GetTickCount() - time;
        DEBUGMSG (PRINT_VDI_TASK_DURATION, (TEXT("%s: VDI->Mem Task Duration = %d sec\r\n"), __WFUNCTION__, time));

        IDMACChannelClearIntStatus(IDMAC_CH_VDI_OUTPUT, IPU_INTR_TYPE_EOF);
        IDMACChannelIntCntrl(IDMAC_CH_VDI_OUTPUT, IPU_INTR_TYPE_EOF, FALSE);
    }
    else
    {
        if (WaitForSingleObject(g_hVDIIntrEvent, timeout) == WAIT_TIMEOUT)
        {
            ERRORMSG(1, (TEXT("%s(): Waiting for VDI->PRP EOF interrupt time out!\r\n"), __WFUNCTION__));
            CMDumpRegs();
            VDIDumpRegs();
            retVal = FALSE;
        }

        time = GetTickCount() - time;
        DEBUGMSG (PRINT_VDI_TASK_DURATION, (TEXT("%s: VDI->PRP Task Duration = %d sec\r\n"), __WFUNCTION__, time));

        IDMACChannelClearIntStatus(IDMAC_CH_PRP_OUTPUT_VF, IPU_INTR_TYPE_EOF);
        IDMACChannelIntCntrl(IDMAC_CH_PRP_OUTPUT_VF, IPU_INTR_TYPE_EOF, FALSE);
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: VDIIntrThread
//
// This function is the IST thread.
//
// Parameters:
//      lpParameter
//          [in] VDI handler
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
static void VDIIntrThread(LPVOID lpParameter)
{
    UNREFERENCED_PARAMETER(lpParameter);

    VDI_FUNCTION_ENTRY();

    BSPSetVDIISRPriority();

    VDIISRLoop(INFINITE);

    VDI_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: VDIISRLoop
//
// This function is the interrupt handler for the VDI.
// It waits for the VDI interrupt, and signals
// the EOF(End of frame) event registered
// by the user of the VDI.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void VDIISRLoop(UINT32 timeout)
{
    for(;;)
    {
        if (WaitForSingleObject(g_hVDIIntrEvent, timeout) == WAIT_OBJECT_0)
        {
            EnterCriticalSection(&g_csStopping);

            if(!g_bRunning)
            {
                LeaveCriticalSection(&g_csStopping);
                continue;
            }

            // Enable VDI interrupt
            VDIIntrEnable();
            SetEvent(g_hEOFEvent);

            LeaveCriticalSection(&g_csStopping);
        }
    }
}

