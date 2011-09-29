//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pxpClass.h
//
//  Common definitions for Pixel Pipeline module
//
//------------------------------------------------------------------------------

#ifndef __PXPCLASS_H__
#define __PXPCLASS_H__
#include "csp.h"
#include "pxp.h"
//------------------------------------------------------------------------------
// Defines

#define PXP_FUNCTION_ENTRY() \
    DEBUGMSG(1, (TEXT("++%s\r\n"), __WFUNCTION__))
#define PXP_FUNCTION_EXIT() \
    DEBUGMSG(1, (TEXT("--%s\r\n"), __WFUNCTION__))

// Macros to create Unicode function name
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFUNCTION__ WIDEN(__FUNCTION__)


class PxpClass
{
public:
    PxpClass();
    ~PxpClass();

    void PxpEnable();
    void PxpDisable();
    void PxpIntrEnable(BOOL Enable);
    void PxpSetOutputBuffer1Addr(UINT32* pBuf);
    void PxpSetOutputBuffer2Addr(UINT32* pBuf);
    void PxpSetS0BufferAddrGroup(pPxpS0BufferAddrGroup pAddrGroup);
    void PxpSetS0BufferOffsetInOutput(pPxpCoordinate pCoordinate);
    void PxpSetS0BufferColorKey(pPxpColorKey pColorKey);
    void PxpSetOverlayBuffersAddr(pPxpOverlayBuffersAddr pBufsAddr);
    void PxpSetOverlayBuffersPos(pPxpOverlayBuffersPos pBufsPos);
    void PxpConfigureGeneral(pPxpParaConfig pParaConfig);
    void PxpSetS0BufProperty(pPxpS0Property pS0Property);
    void PxpSetS0BufferSize(pPxpRectSize pRectSize);
    void PxpClassDataInit();
    void PxpSetOverlayBufsProperty(pPxpOverlayProperty pOverlayProperty);
    void PxpSetOutputProperty(pPxpOutProperty pOutProperty);
    void PxpSetOverlayColorKey(pPxpColorKey pColorKey);
    void PxpPowerUp();
    void PxpPowerDown();
    void PxpOpen();
    void PxpClose();
    void PxpResetDriverStatus();
    void PxpStartProcess(BOOL bWait);
    void PxpWaitForCompletion();
 
private:
    BOOL PxpInit(void);
    void PxpDeinit(void);
    void PxPClassParaInit(void);
    void PxpISRLoop(UINT32 timout);
    static void PxpIntrThread(LPVOID lpParameter);
    void PXPDumpRegs();

    //Data:
private:
    DWORD SysIntrPXP;
    HANDLE m_hPxpIntrEvent;
    BOOL m_bS0RGBFormat;
    BOOL m_bSetS0Size;
    BOOL m_bSetCrop;
    BOOL m_bSetScale;
    RECT m_rSOCropRect;
    float m_fYScale;
    float m_fXScale;
    pxpRectSize m_S0RectSize;

    ULONG  m_iPxpRegsphysAddr;    //physical address of pxp_registers struct, which is loaded by HW_PXP_NEXT register
    pPxp_registers m_pPxpRegsVirtAddr;  //virtual address of pxp_registers struct

    UINT32 m_iIndex1;   //Indicate the index of current free buffer
    UINT32 m_iIndex2;   //Indicate the index of the buffer which will be processed, this index is used by the interrupt handler thread.
    UINT32 m_iPxpRegsphysAddrArr[2];    //Array for double buffers physical address
    
    BOOL m_bYCbCrCsc;   //flag for YUV or YCbCr type
    BOOL m_bOpen;   //flag for open

    BOOL m_bHasNextFrame;   //Flag for whether double buffers are full while suspend.
    BOOL m_bInProcess;      //Flag for whether PXP is in process while suspend. 
    BOOL m_bProcessPending; //Flag for whether double buffers are full while next operation request coming

    HANDLE m_hPxpISRThread; //Handle for ISR process thread

    HANDLE m_hPxpCompleteEvent; //Handle for PXP operation completion event
    HANDLE m_hPxpContinueEvent; //Handle for event indicating double buffers can accept next operation request
   
};


#endif  // __PXPCLASS_H__

