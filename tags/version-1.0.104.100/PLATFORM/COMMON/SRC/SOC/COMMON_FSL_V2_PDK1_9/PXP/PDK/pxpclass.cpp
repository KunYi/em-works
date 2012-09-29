//------------------------------------------------------------------------------
//
//  Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  PxpClass.cpp
//
//  Implementation of pixel pipeline driver methods
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 )
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <nkintr.h>
#include <winddi.h>
#include <ddgpe.h>
#pragma warning(pop)


#include "PxpClass.h"
#include "common_macros.h"
#include "common_regspxp.h"

//------------------------------------------------------------------------------
// Defines
#define DEBUG_DUMP 0
//------------------------------------------------------------------------------
// External Functions
extern DWORD CSPPXPGetBaseAddr();
extern DWORD PXPGetIRQ();
extern void PXPIntrEnable(pPxp_registers pPxpRegsVirtAddr, BOOL Enable);
extern void PXPSetBlockSizeValue(pPxp_registers pPxpRegsVirtAddr, UINT32 *BlockSize, UINT32 *BlockExp);
extern void PXPSetScaleFactor(pPxp_registers pPxpRegsVirtAddr, UINT32 XScale, UINT32 YScale);
extern void PXPOperateComplete(HANDLE hPxpContinueEvent);
extern void PXPLoadComplete(HANDLE hPxpContinueEvent, BOOL bProcessPending);
extern void PXPWaitforLoadComplete(HANDLE hPxpResumeLoadEvent);
extern void PXPGetDownScaleFactorExp(UINT32 *pDownScaleExp);

//------------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Global Variables
static PVOID pv_HWregPXP;

// Color conversion coefficient table
//
// YUV(YCbCr) to RGB
static const UINT16 csc_tbl[2][7] =
{
    // C0   UV_OFFSET Y_OFFSET     C1     C4     C2     C3
    {0x100,   0x180,    0x0,     0x123,  0x208, 0x76B, 0x79C},      //YUV
    {0x12A,   0x180,   0x1F0,    0x198,  0x204, 0x730, 0x79C}       //YCbCr
};


//-----------------------------------------------------------------------------
//
// Function: PxpClass
//
// PixelPipeline class constructor.  Calls PxpInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PxpClass::PxpClass(void)
{
    PxpInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~PxpClass
//
// The destructor for PxpClass.  Calls PxpDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
PxpClass::~PxpClass(void)
{
    PxpDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: PxpInit
//
// This function initializes the Image Converter (pixel pipeline).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PxpClass::PxpInit(void)
{
    PHYSICAL_ADDRESS phyAddr;

    phyAddr.QuadPart = CSPPXPGetBaseAddr();
    pv_HWregPXP = (PVOID)MmMapIoSpace(phyAddr, 0x1000, FALSE);

    if (pv_HWregPXP == NULL)
    {
        ERRORMSG(1, (TEXT("PxpInit:  MmMapIoSpace failed!\r\n")));
        goto Error;
    }

    m_pPxpRegsVirtAddr = (pPxp_registers) AllocPhysMem(2*sizeof(pxp_registers), PAGE_EXECUTE_READWRITE, 0, 0, &m_iPxpRegsphysAddr);

    if (!m_pPxpRegsVirtAddr)
    {
        ERRORMSG(1, (TEXT("PxpInit:  AllocPhysMem failed!\r\n")));
        goto Error;
    }        

    m_iPxpRegsphysAddrArr[0] = m_iPxpRegsphysAddr;
    m_iPxpRegsphysAddrArr[1] = m_iPxpRegsphysAddr + sizeof(pxp_registers);

    memset(m_pPxpRegsVirtAddr,0,sizeof(pxp_registers)*2);
    m_pPxpRegsVirtAddr->s0scale = 0x10001000;
    (m_pPxpRegsVirtAddr+ 1 )->s0scale = 0x10001000;
                   
    PXP_FUNCTION_ENTRY();

    PxpEnable();
    PxpClassDataInit();

    m_hPxpIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hPxpIntrEvent == NULL)
    {
        RETAILMSG(1,(TEXT("%s: CreateEvent for PXP Interrupt failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    m_hPxpCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hPxpCompleteEvent == NULL)
    {
        RETAILMSG(1,(TEXT("%s: CreateEvent for PXP Completion failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    m_hPxpContinueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hPxpContinueEvent == NULL)
    {
        RETAILMSG(1,(TEXT("%s: CreateEvent for PXP Operation Continue failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    DWORD irq = PXPGetIRQ();

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
        &SysIntrPXP, sizeof(DWORD), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to obtain sysintr value for PXP interrupt.\r\n")));
        return FALSE;
    }

    if ( !InterruptInitialize(SysIntrPXP,
        m_hPxpIntrEvent,
        NULL,
        0) ) {
            RETAILMSG(1,(TEXT("Error initializing interrupt\n\r")));
            goto Error;
    }
    InterruptDone(SysIntrPXP);

    m_hPxpISRThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PxpIntrThread, this, 0, NULL);

    if (m_hPxpISRThread == NULL)
    {
        ERRORMSG(1 ,(TEXT("%s: create PXP ISR thread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        DEBUGMSG(1, (TEXT("%s: create PXP ISR thread success\r\n"), __WFUNCTION__));
    }
    
    RETAILMSG(1, (TEXT("%s: Enabling pixel pipeline\r\n"), __WFUNCTION__));

    PXP_FUNCTION_EXIT();

    return TRUE;

Error:
    PxpDeinit();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PxpDeinit
//
// This function deinitializes the Pixel Pipeline.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpDeinit(void)
{

    PXP_FUNCTION_ENTRY();

    // Disable pixel pipeline
    PxpDisable();

    // Unmap peripheral registers
    if (pv_HWregPXP != NULL)
    {
        MmUnmapIoSpace(pv_HWregPXP, 0x1000);
        pv_HWregPXP = NULL;
    }

    if (m_pPxpRegsVirtAddr)
    {
        FreePhysMem( m_pPxpRegsVirtAddr );
        m_pPxpRegsVirtAddr = NULL;
    }

    CloseHandle(m_hPxpIntrEvent);
    CloseHandle(m_hPxpCompleteEvent);
    CloseHandle(m_hPxpContinueEvent);
    m_hPxpIntrEvent = NULL;
    m_hPxpCompleteEvent = NULL;
    m_hPxpContinueEvent = NULL;
    m_bOpen = FALSE;

}

//-----------------------------------------------------------------------------
//
// Function: PxpEnable
//
// This function enable pixel pipeline.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpEnable()
{
    PXP_FUNCTION_ENTRY();
    HW_PXP_CTRL_CLR(BM_PXP_CTRL_SFTRST | BM_PXP_CTRL_CLKGATE);
    PXP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: PxpDisable
//
// This function enable pixel pipeline.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpDisable()
{
    PXP_FUNCTION_ENTRY();
    HW_PXP_CTRL_SET(BM_PXP_CTRL_SFTRST | BM_PXP_CTRL_CLKGATE);
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PxpIntrEnable
//
// This function is for enable or disable pxp interrupt.
//
// Parameters:
//      Enable
//          [in] TRUE if enable, FALSE if disable 
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpIntrEnable(BOOL Enable)
{
    PXP_FUNCTION_ENTRY();
    PXPIntrEnable(m_pPxpRegsVirtAddr, Enable);
    PXP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function:  PxpSetS0BufferAddrGroup
//
// This function sets PXP S0 buffer address into hardware.
//
// Parameters:
//      pAddrGroup
//          [in] S0 buffer address group.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetS0BufferAddrGroup(pPxpS0BufferAddrGroup pAddrGroup)
{
//    PXP_FUNCTION_ENTRY();
    (m_pPxpRegsVirtAddr+m_iIndex1)->s0buf = pAddrGroup->iRGBorYBufAddr;
    (m_pPxpRegsVirtAddr+m_iIndex1)->s0ubuf = pAddrGroup->iUorCbBufAddr;
    (m_pPxpRegsVirtAddr+m_iIndex1)->s0vbuf = pAddrGroup->iVorCrBufAddr;

//    PXP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function:  PxpSetOutputBuffer1Addr
//
// This function sets PXP output buffer1 address into hardware.
//
// Parameters:
//      pBuf
//          [in] Output buffer1 address.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetOutputBuffer1Addr(UINT32 * pBuf)
{
    PXP_FUNCTION_ENTRY();
    (m_pPxpRegsVirtAddr+m_iIndex1)->outbuf = (UINT32)pBuf;
    PXP_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function:  PxpSetOutputBuffer2Addr
//
// This function sets PXP output buffer2 address into hardware.
//
// Parameters:
//      pBuf
//          [in] Output buffer1 address.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetOutputBuffer2Addr(UINT32 * pBuf)
{
    PXP_FUNCTION_ENTRY();
    (m_pPxpRegsVirtAddr+m_iIndex1)->outbuf2 = (UINT32)pBuf;
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  PxpSetS0BufferOffsetInOutput
//
// This function sets PXP S0 buffer offset location within the output frame buffer.
//
// Parameters:
//      pCoordinate
//          [in] S0 buffer offset location coordinate.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetS0BufferOffsetInOutput(pPxpCoordinate pCoordinate)
{
    PXP_FUNCTION_ENTRY();
    ((hw_pxp_s0param_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0param)->B.XBASE = (UINT8)((pCoordinate->iXBase + (m_nBlockSize>>1)) >> m_nBlockExp);  //Align to right side block
    ((hw_pxp_s0param_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0param)->B.YBASE = (UINT8)((pCoordinate->iYBase + (m_nBlockSize>>1)) >> m_nBlockExp);  //Align to underside block
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  PxpSetS0BufferSize
//
// This function sets PXP S0 buffer size.
//
// Parameters:
//      pRectSize
//          [in] S0 buffer size.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetS0BufferSize(pPxpRectSize pRectSize)
{
    PXP_FUNCTION_ENTRY();
    m_bSetS0Size = TRUE;
    ((hw_pxp_s0param_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0param)->B.WIDTH = (UINT8)(pRectSize->iWidth>>m_nBlockExp);
    ((hw_pxp_s0param_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0param)->B.HEIGHT = (UINT8)(pRectSize->iHeight>>m_nBlockExp);    
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  PxpSetS0BufferColorKey
//
// This function sets PXP S0 buffer color key into hardware.
//
// Parameters:
//      pColorKey
//          [in] Color key structure pointer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetS0BufferColorKey(pPxpColorKey pColorKey)
{
    PXP_FUNCTION_ENTRY();
    (m_pPxpRegsVirtAddr+m_iIndex1)->s0colorkeyhigh = pColorKey->iColorKeyHigh;
    (m_pPxpRegsVirtAddr+m_iIndex1)->s0colorkeylow = pColorKey->iColorKeyLow;
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  PxpSetOverlayColorKey
//
// This function sets PXP overlay color key into hardware.
//
// Parameters:
//      pColorKey
//          [in] Color key structure pointer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetOverlayColorKey(pPxpColorKey pColorKey)
{
    PXP_FUNCTION_ENTRY();
    (m_pPxpRegsVirtAddr+m_iIndex1)->olcolorkeyhigh = pColorKey->iColorKeyHigh;
    (m_pPxpRegsVirtAddr+m_iIndex1)->olcolorkeylow = pColorKey->iColorKeyLow;    
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  PxpSetOverlayBuffersAddr
//
// This function sets one of the PXP Overlay buffer address into hardware.
//
// Parameters:
//      pBufsAddr
//          [in] Overlay buffer address structure pointer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetOverlayBuffersAddr(pPxpOverlayBuffersAddr pBufsAddr)
{
    PXP_FUNCTION_ENTRY();
    (m_pPxpRegsVirtAddr+m_iIndex1)->ol[pBufsAddr->iOverlayBufNum].ol = pBufsAddr->iOverlayBufAddress;
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  PxpSetOverlayBuffersPos
//
// This function sets one of the PXP Overlay buffer position into hardware.
//
// Parameters:
//      pBufsPos
//          [in] Overlay buffer position structure pointer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetOverlayBuffersPos(pPxpOverlayBuffersPos pBufsPos)
{
    PXP_FUNCTION_ENTRY();
    ((hw_pxp_olnsize_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pBufsPos->iOverlayBufNum].olsize)->B.XBASE = (UINT8)(pBufsPos->rOverlayBufRect.left >>m_nBlockExp);
    ((hw_pxp_olnsize_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pBufsPos->iOverlayBufNum].olsize)->B.WIDTH = (UINT8)((pBufsPos->rOverlayBufRect.right-pBufsPos->rOverlayBufRect.left) >>m_nBlockExp);
    ((hw_pxp_olnsize_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pBufsPos->iOverlayBufNum].olsize)->B.YBASE = (UINT8)(pBufsPos->rOverlayBufRect.top >>m_nBlockExp);
    ((hw_pxp_olnsize_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pBufsPos->iOverlayBufNum].olsize)->B.HEIGHT = (UINT8)((pBufsPos->rOverlayBufRect.bottom-pBufsPos->rOverlayBufRect.top) >>m_nBlockExp);
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PxpConfigureGeneral
//
// This function configures general behavior for pixel pipeline.
//
// Parameters:
//      pParaConfig
//          [in] Pointer to configuration data structure
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpConfigureGeneral(pPxpParaConfig pParaConfig)
{
    PXP_FUNCTION_ENTRY();
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.SFTRST = 0;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.CLKGATE = 0;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.ENABLE = 1;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.INTERLACED_OUTPUT = pParaConfig->eInterlaceOutput;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.INTERLACED_INPUT = pParaConfig->eInterlaceInput;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.IN_PLACE = pParaConfig->bIn_PLACE;
    PxpSetBlockSize(&(pParaConfig->iBlockSize));    
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PxpSetS0BufProperty
//
// This function configures S0 buffer property for pixel pipeline.
//
// Parameters:
//      pS0Property
//          [in] Pointer to S0 buffer property data structure
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetS0BufProperty(pPxpS0Property pS0Property)
{
    PXP_FUNCTION_ENTRY();
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.S0_FORMAT = pS0Property->eS0BufferFormat;
    if(pS0Property->eS0BufferFormat<pxpS0BufferFormat_YUV422)
        m_bS0RGBFormat = TRUE;
    else
        m_bS0RGBFormat = FALSE;

    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.SCALE = pS0Property->bScale;
    if(pS0Property->bScale)
    {
        m_bSetScale = TRUE;
        PXPSetScaleFactor(m_pPxpRegsVirtAddr+m_iIndex1, (UINT32)(4096.0/pS0Property->fXScale), (UINT32)(4096.0/pS0Property->fYScale));
        ((hw_pxp_s0offset_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0offset)->B.XOFFSET = (int)(4096.0*pS0Property->fXScaleOffset);
        ((hw_pxp_s0offset_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0offset)->B.YOFFSET = (int)(4096.0*pS0Property->fYScaleOffset);
        m_fXScale = pS0Property->fXScale;
        m_fYScale = pS0Property->fYScale;
    }

    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.CROP = pS0Property->bCrop;
    if(pS0Property->bCrop)
    {
        m_bSetCrop = TRUE;
        ((hw_pxp_s0crop_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0crop)->B.XBASE = (UINT8)(pS0Property->rSOCropRect.left >>m_nBlockExp);
        ((hw_pxp_s0crop_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0crop)->B.YBASE = (UINT8)(pS0Property->rSOCropRect.top >>m_nBlockExp);
        ((hw_pxp_s0crop_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0crop)->B.WIDTH = (UINT8)((pS0Property->rSOCropRect.right-pS0Property->rSOCropRect.left) >>m_nBlockExp);
        ((hw_pxp_s0crop_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->s0crop)->B.HEIGHT = (UINT8)((pS0Property->rSOCropRect.bottom-pS0Property->rSOCropRect.top) >>m_nBlockExp);
        m_rSOCropRect = pS0Property->rSOCropRect;
    }    

    (m_pPxpRegsVirtAddr+m_iIndex1)->s0background = pS0Property->iS0BKColor;
    
    if(!m_bS0RGBFormat)
    {
        BW_PXP_CSCCOEFF0_YCBCR_MODE(pS0Property->bYCbCrCsc);
        BW_PXP_CSCCOEFF0_C0(csc_tbl[pS0Property->bYCbCrCsc][0]);
        BW_PXP_CSCCOEFF0_UV_OFFSET(csc_tbl[pS0Property->bYCbCrCsc][1]);
        BW_PXP_CSCCOEFF0_Y_OFFSET(csc_tbl[pS0Property->bYCbCrCsc][2]);
        BW_PXP_CSCCOEFF1_C1(csc_tbl[pS0Property->bYCbCrCsc][3]);
        BW_PXP_CSCCOEFF1_C4(csc_tbl[pS0Property->bYCbCrCsc][4]);
        BW_PXP_CSCCOEFF2_C2(csc_tbl[pS0Property->bYCbCrCsc][5]);
        BW_PXP_CSCCOEFF2_C3(csc_tbl[pS0Property->bYCbCrCsc][6]);
        m_bYCbCrCsc = pS0Property->bYCbCrCsc;
    }
    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PxpSetOutputProperty
//
// This function configures output property for pixel pipeline.
//
// Parameters:
//      pOutProperty
//          [in] Pointer to output property data structure
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetOutputProperty(pPxpOutProperty pOutProperty)
{
    PXP_FUNCTION_ENTRY();
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.ALPHA_OUTPUT = pOutProperty->bAlphaOutput;
    ((hw_pxp_outsize_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->outsize)->B.ALPHA = pOutProperty->iOutputAlpha;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.VFLIP = pOutProperty->bVFlip;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.HFLIP = pOutProperty->bHFlip;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.OUTPUT_FORMAT = pOutProperty->epxpOutputFmt;
    ((hw_pxp_ctrl_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ctrl)->B.ROTATEOP = pOutProperty->epxpOutputRot;    
    ((hw_pxp_outsize_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->outsize)->B.WIDTH = pOutProperty->iOutputWidth;
    ((hw_pxp_outsize_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->outsize)->B.HEIGHT = pOutProperty->iOutputHeight;

    PXP_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PxpSetOverlayBufsProperty
//
// This function configures overlay property for pixel pipeline.
//
// Parameters:
//      pPxpOverlayProperty
//          [in] Pointer to overlay property data structure
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetOverlayBufsProperty(pPxpOverlayProperty pOverlayProperty)
{
    PXP_FUNCTION_ENTRY();
    ((hw_pxp_olnparam_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pOverlayProperty->iOverlayBufNum].olparam)->B.ENABLE = pOverlayProperty->bEnableOverlay;
    ((hw_pxp_olnparam_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pOverlayProperty->iOverlayBufNum].olparam)->B.ENABLE_COLORKEY = pOverlayProperty->bColorKey;
    ((hw_pxp_olnparam_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pOverlayProperty->iOverlayBufNum].olparam)->B.ALPHA_CNTL = pOverlayProperty->eAlphaCntl;    
    ((hw_pxp_olnparam_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pOverlayProperty->iOverlayBufNum].olparam)->B.FORMAT = pOverlayProperty->eOverlayFormat;
    ((hw_pxp_olnparam_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pOverlayProperty->iOverlayBufNum].olparam)->B.ROP = pOverlayProperty->epxpROPType;
    ((hw_pxp_olnparam_t *)&(m_pPxpRegsVirtAddr+m_iIndex1)->ol[pOverlayProperty->iOverlayBufNum].olparam)->B.ALPHA = pOverlayProperty->iAlphaOverlay;
    
    PXP_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: PxpClassDataInit
//
// This function initialize pxpClass par.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PxpClass::PxpClassDataInit()
{
    m_bS0RGBFormat = FALSE;
    m_bSetS0Size = FALSE;
    m_bSetCrop = FALSE;
    m_bSetScale = FALSE;
    m_bYCbCrCsc = TRUE;
    m_bOpen = FALSE;
    m_iIndex1 = 0;
    m_iIndex2 = 0;
    m_bHasNextFrame = FALSE;
    m_bInProcess = FALSE;
    m_bProcessPending = FALSE;
    m_nBlockExp = 3;
    m_nBlockSize = 8;
}
                
//------------------------------------------------------------------------------
//
// Function: PXPDumpRegs
//
// This function dump pxp registers value.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------                
void PxpClass::PXPDumpRegs()
{
    int i;

    RETAILMSG (1, (TEXT("\n\nPXP Registers\r\n")));
    RETAILMSG (1, (TEXT("%08x %08x %08x %08x\r\n"), HW_PXP_CTRL_RD(), HW_PXP_STAT_RD(), HW_PXP_OUTBUF_RD(), HW_PXP_OUTBUF2_RD()));
    RETAILMSG (1, (TEXT("%08x %08x %08x %08x\r\n"), HW_PXP_OUTSIZE_RD(), HW_PXP_S0BUF_RD(), HW_PXP_S0UBUF_RD(), HW_PXP_S0VBUF_RD()));
    RETAILMSG (1, (TEXT("%08x %08x %08x %08x\r\n"), HW_PXP_S0PARAM_RD(), HW_PXP_S0BACKGROUND_RD(), HW_PXP_S0CROP_RD(), HW_PXP_S0SCALE_MX23_RD()));
    RETAILMSG (1, (TEXT("%08x %08x %08x %08x\r\n"), HW_PXP_S0OFFSET_RD(), HW_PXP_CSCCOEFF0_RD(), HW_PXP_CSCCOEFF1_RD(), HW_PXP_CSCCOEFF2_RD()));
    RETAILMSG (1, (TEXT("%08x %08x %08x %08x\r\n"), HW_PXP_NEXT_RD(), HW_PXP_PAGETABLE_RD(), HW_PXP_S0COLORKEYLOW_RD(), HW_PXP_S0COLORKEYHIGH_RD()));
    RETAILMSG (1, (TEXT("%08x %08x %08x %08x %08x\r\n"), HW_PXP_OLCOLORKEYLOW_RD(), HW_PXP_OLCOLORKEYHIGH_RD(), HW_PXP_DEBUGCTRL_RD(), HW_PXP_DEBUG_RD(), HW_PXP_VERSION_RD()));

    for (i = 0; i <= 7; i++)
    {
        RETAILMSG (1, (TEXT("%08x %08x %08x\r\n"), HW_PXP_OLn_RD(i), HW_PXP_OLnSIZE_RD(i), HW_PXP_OLnPARAM_RD(i)));
    }

} 

//------------------------------------------------------------------------------
//
// Function: PxpPowerUp
//
// This function restores power to PXP device.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------ 
void PxpClass::PxpPowerUp()
{
    while (HW_PXP_CTRL.B.CLKGATE == 1)
        HW_PXP_CTRL.B.CLKGATE = 0;

    if(m_bHasNextFrame)
    {
        m_bProcessPending = TRUE;
        HW_PXP_NEXT_WR(m_iPxpRegsphysAddrArr[!m_iIndex1]);
        m_bProcessPending = FALSE;
        m_bHasNextFrame = FALSE;
    }

}

//------------------------------------------------------------------------------
//
// Function: PxpPowerDown
//
// This function suspends power to PXP device.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------ 

void PxpClass::PxpPowerDown()
{
    if(HW_PXP_NEXT.B.ENABLED)
    {
        m_bHasNextFrame = TRUE;
        HW_PXP_NEXT_CLR(0xFFFFFFFF);
    }
    else
        m_bHasNextFrame = FALSE;

    while(HW_PXP_CTRL.B.ENABLE)
    {
        Sleep(1);
    }

    while (HW_PXP_CTRL.B.CLKGATE == 0)
        HW_PXP_CTRL.B.CLKGATE = 1;
   
}

//------------------------------------------------------------------------------
//
// Function: PxpOpen
//
// This function opens PXP.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------ 
void PxpClass::PxpOpen()
{
    m_bOpen = TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PxpClose
//
// This function closes PXP.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------ 
void PxpClass::PxpClose()
{
    m_bOpen = FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PxpResetDriverStatus
//
// This function reset PXP driver status.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------ 
void PxpClass::PxpResetDriverStatus()
{
    while(HW_PXP_CTRL.B.ENABLE)
        Sleep(1);

    m_iIndex1 = m_iIndex2 = 0;
    memset(m_pPxpRegsVirtAddr,0,2*sizeof(pxp_registers));
    m_pPxpRegsVirtAddr->s0scale = 0x10001000;
    ResetEvent(m_hPxpCompleteEvent);
    ResetEvent(m_hPxpIntrEvent);
    ResetEvent(m_hPxpContinueEvent);
    InterruptDone(SysIntrPXP);
}

//-----------------------------------------------------------------------------
//
// Function: PxpStartProcess
//
// This function make PXP load the PXP registers for operation.
//
// Parameters:
//      bWait.
//          [in] TRUE if waiting PXP current operation completion is needed, FALSE if not wait             
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpStartProcess(BOOL bWait)
{
    if((HW_PXP_NEXT.B.ENABLED)||m_bProcessPending)   //Hardware double buffering is full
    {
        m_bProcessPending = TRUE;
        WaitForSingleObject(m_hPxpContinueEvent,1000);  //Wait till PXP can access other input
        m_bProcessPending = FALSE;
    }
    (m_pPxpRegsVirtAddr+m_iIndex1)->bWait = bWait;
    HW_PXP_NEXT_WR(m_iPxpRegsphysAddrArr[m_iIndex1]);
    m_iIndex1 = !m_iIndex1;
    memcpy(m_pPxpRegsVirtAddr+m_iIndex1, m_pPxpRegsVirtAddr+!m_iIndex1, sizeof(pxp_registers));     //In most DDraw calling, just minor register settings will be updated.
                                                                                                    //When current buffer setting is updated, next buffer still needs to 
                                                                                                    //be updated in case the previous setting is lost for following operations.
}

//-----------------------------------------------------------------------------
//
// Function: PxpIntrThread
//
// This method is a thread to handle PXP interrupt.
//
// Parameters:
//      lpParameter.
//          [in] The thread data passed to the function using the lpParameter parameter.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpIntrThread(LPVOID lpParameter)
{
    PxpClass *pPxp = (PxpClass *)lpParameter;
    CeSetThreadPriority(GetCurrentThread(), 100);

    for(;;)
    {
        if (WaitForSingleObject(pPxp->m_hPxpIntrEvent, INFINITE) == WAIT_OBJECT_0)
        {
//            RETAILMSG(1,(_T("enter wait m_hPxpIntrEvent")));
            InterruptDone(pPxp->SysIntrPXP);
            if(HW_PXP_STAT.B.IRQ)
            {
                HW_PXP_STAT_CLR(BM_PXP_STAT_IRQ);
                if(pPxp->m_bProcessPending)
                {
                    PXPOperateComplete(pPxp->m_hPxpContinueEvent);  //Mx23 and Mx28 have different strategy to process the situation
                                                                    //that double-buffering is full. Cause Mx28 knows when register has
                                                                    //been loaded, it can signal next register value could be set while 
                                                                    //loading completes. But Mx23 doesn't have this capability, thus for 
                                                                    //safety, it only can tell it is time to set new next register value
                                                                    //when PXP operation completes.
                }
                if((pPxp->m_pPxpRegsVirtAddr+pPxp->m_iIndex2)->bWait)
                    SetEvent(pPxp->m_hPxpCompleteEvent);    //Completion signal is waited by the caller, signal it.
                pPxp->m_iIndex2 = !pPxp->m_iIndex2;
            }
            else if(HW_PXP_STAT.B.NEXT_IRQ)   //Mx23 PXP should never go here
            {
                HW_PXP_STAT_CLR(BM_PXP_STAT_NEXT_IRQ);
                PXPLoadComplete(pPxp->m_hPxpContinueEvent, pPxp->m_bProcessPending);
            }
        }
        else
            RETAILMSG(1,(TEXT("%s(): Waiting for PXP interrupt error!\r\n"), __WFUNCTION__));
    }    

}

//-----------------------------------------------------------------------------
//
// Function: PxpWaitForCompletion
//
// This function is used to wait for the completion of PXP operation by caller.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpWaitForCompletion()
{
    UINT32 timeout = 1000;

    DWORD result = WaitForSingleObject(m_hPxpCompleteEvent, timeout);
    if (WAIT_OBJECT_0 == result)
    {
    }
    else if (WAIT_TIMEOUT == result)
    {
        PXPDumpRegs();
        RETAILMSG(1,(TEXT("%s(): Waiting for PXP interrupt time out!\r\n"), __WFUNCTION__));
    }
    else
        RETAILMSG(1,(TEXT("%s(): Waiting for PXP interrupt error!\r\n"), __WFUNCTION__));
}

//-----------------------------------------------------------------------------
//
// Function: PxpSetBlockSize
//
// This function is used to set PXP block size.
//
// Parameters:
//      pBlockSize.
//          [in/out] point to variable that holds PXP block size value. 
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PxpClass::PxpSetBlockSize(UINT32 * pBlockSize)
{
    PXPSetBlockSizeValue(m_pPxpRegsVirtAddr, pBlockSize, &m_nBlockExp);
    m_nBlockSize = *pBlockSize;
}

//-----------------------------------------------------------------------------
//
// Function: PxpGetDownScaleFactorExponent
//
// This function is used to get PXP down scaling factor exponent.
//
// Parameters:
//      pDownScaleExp
//          [in/out] Address of PXP down scaling factor exponent. 
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PxpClass::PxpGetDownScaleFactorExponent(UINT32 *pDownScaleExp)
{
    PXPGetDownScaleFactorExp(pDownScaleExp);
}
