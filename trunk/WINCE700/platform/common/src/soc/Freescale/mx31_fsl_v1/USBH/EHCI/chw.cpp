//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Module Name:
//     CHW.cpp
// Abstract:
//     This file implements the EHCI specific register routines
//
// Notes:
//
//

/*---------------------------------------------------------------------------
* Copyright (C) 2005-2007, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/



#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <Uhcdddsi.h>
#include <globals.hpp>
#pragma warning(pop)
#include <td.h>
#include <ctd.h>
#include <chw.h>
#include <mx31_usbname.h>
#include <mx31_usbcommon.h>
#undef HCD_SUSPEND_RESUME
#define HCD_SUSPEND_RESUME 0 // Should never set since it is handled differently

#define RETAIL_LOG 0
#define CRITICAL_LOG 0
#define DEBUG_LOG_USBCV 0
typedef struct _SEHCDPdd
{
#ifdef IRAM_PATCH
    LPVOID lpvMemoryObject[3];
#else
    LPVOID lpvMemoryObject;
#endif
    LPVOID lpvEHCDMddObject;
    PVOID pvVirtualAddress;                        // DMA buffers as seen by the CPU
    DWORD dwPhysicalMemSize;
    PHYSICAL_ADDRESS LogicalAddress;        // DMA buffers as seen by the DMA controller and bus interfaces
    DMA_ADAPTER_OBJECT AdapterObject;
    TCHAR szDriverRegKey[MAX_PATH];
    PUCHAR ioPortBase;
    DWORD dwSysIntr;
    CRITICAL_SECTION csPdd;                     // serializes access to the PDD object
    HANDLE          IsrHandle;
    HANDLE hParentBusHandle;
    BOOL   bIsMX31TO2; // TRUE- If hardware is TO2 else FALSE
    DWORD   dwOTGSupport;
    TCHAR szOTGGroup[15];
} SEHCDPdd;

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

#define USB_DISCONNECT_TIMEOUT 3000
#define USB_IDLE_TIMEOUT 3000
#define USB_CHECK_CABLE_TIMEOUT 500

#ifdef COLD_MSC_RECOGNIZE
#define ULPI_VENDERID_LOW_R  0
#define ULPI_VENDERID_HIGH_R 1
#define ULPI_PRODUCT_LOW_R   2
#define ULPI_PRODUCT_HIGH_R  3
#define ULPI_FUNCTION_CTRL_RW    4
#define ULPI_FUNCTION_CTRL_S     5
#define ULPI_FUNCTION_CTRL_C     6
#define ULPI_INTERFACE_CTRL_RW   7
#define ULPI_INTERFACE_CTRL_S    8
#define ULPI_INTERFACE_CTRL_C    9
#define ULPI_OTG_CTRL_RW         0xA
#define ULPI_OTG_CTRL_S          0xB
#define ULPI_OTG_CTRL_C          0xC
#define ULPI_INTR_RISING_RW      0xD
#define ULPI_INTR_RISING_S       0xE
#define ULPI_INTR_RISING_C       0xF
#define ULPI_INTR_FALLING_RW     0x10
#define ULPI_INTR_FALLING_S      0x11
#define ULPI_INTR_FALLING_C      0x12
#define ULPI_INTR_STATUS_R       0x13
#define ULPI_INTR_LATCH_RC       0x14
#define ULPI_DEBUG_R             0x15
#define ULPI_SCRATCH_RW          0x16
#define ULPI_SCRATCH_S           0x17
#define ULPI_SCRATCH_C           0x18
#define ULPI_ACCESS_EXT_W        0x2F
#define ULPI_POWER_CTRL_RW       0x3D
#define ULPI_POWER_CTRL_S        0x3E
#define ULPI_POWER_CTRL_C        0x3F

#define ULPI_OTGCTRL_DRV_VBUS        0x20
#define ULPI_OTGCTRL_DRV_VBUS_EXT    0x40
#define ULPI_OTGCTRL_EXT_VBUS_IND    0x80
#endif

//extern "C" void DumpUSBHostRegs(void);
extern "C" void SetPHYPowerMgmt(BOOL fSuspend);
extern "C" void BSPUsbSetWakeUp(BOOL fEnable);
extern "C" BOOL BSPUsbCheckWakeUp(void);
// ******************************* CDummyPipe **********************************
const USB_ENDPOINT_DESCRIPTOR dummpDesc = {
    sizeof(USB_ENDPOINT_DESCRIPTOR),USB_ENDPOINT_DESCRIPTOR_TYPE, 0xff,  USB_ENDPOINT_TYPE_INTERRUPT,8,1
};
#ifdef IRAM_PATCH
CDummyPipe::CDummyPipe(IN CPhysMem * const pCPhysMem, IN CPhysMem * const pCPhysEleMem)
: CPipe( &dummpDesc,FALSE,TRUE,0xff,0xff,0xff,NULL)
, m_pCPhysMem(pCPhysMem)
, m_pCPhysEleMem(pCPhysEleMem)
{
    ASSERT( m_pCPhysMem!=NULL);
    m_bFrameSMask = 0xff;
    m_bFrameCMask = 0;
};
#else
CDummyPipe::CDummyPipe(IN CPhysMem * const pCPhysMem)
: CPipe( &dummpDesc,FALSE,TRUE,0xff,0xff,0xff,NULL)
, m_pCPhysMem(pCPhysMem)
{
    ASSERT( m_pCPhysMem!=NULL);
    m_bFrameSMask = 0xff;
    m_bFrameCMask = 0;
};
#endif
// ************************************CPeriodicMgr******************************
#ifdef IRAM_PATCH
CPeriodicMgr::CPeriodicMgr(IN CPhysMem * const pCPhysMem, IN CPhysMem * const pCPhysEleMem, DWORD dwFrameSize)
#else
CPeriodicMgr::CPeriodicMgr(IN CPhysMem * const pCPhysMem, DWORD dwFrameSize)
#endif
//
// Purpose: Contructor :Periodic Transfer or Queue Head Manage module
//
// Parameters:  pCPhysMem - pointer to CPhysMem object
//              dwFrameSize - Isoch Frame Size (Mached with hardware).
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
    : m_pCPhysMem(pCPhysMem)
    , m_dwFrameSize(dwFrameSize)
#ifdef IRAM_PATCH
    , m_pIRamEleMem(pCPhysEleMem)
    , m_pCDumpPipe(new CDummyPipe(pCPhysMem, pCPhysEleMem))
#else
    , m_pCDumpPipe(new CDummyPipe(pCPhysMem))
#endif
{
    ASSERT(pCPhysMem);
    ASSERT(dwFrameSize == 0x400|| dwFrameSize== 0x200 || dwFrameSize== 0x100);
    m_pFrameList = NULL;
    m_pFramePhysAddr = 0;
    m_dwFrameMask=0xff;
    switch(dwFrameSize) {
        case 0x400: default:
            m_dwFrameMask=0x3ff;
            break;
        case 0x200:
            m_dwFrameMask=0x1ff;
            break;
        case 0x100:
            m_dwFrameMask=0xff;
            break;
    }
    ASSERT(m_pCDumpPipe);
    // Create Dummy Pipe for static
}
// ******************************************************************
CPeriodicMgr::~CPeriodicMgr()
//
// Purpose: Decontructor :Periodic Transfer or Queue Head Manage module
//
// Parameters:  
//
// Returns: Nothing
//
// Notes: 
// ******************************************************************
{
    DeInit();
    if (m_pCDumpPipe)
        delete m_pCDumpPipe;
}
// ******************************************************************
BOOL CPeriodicMgr::Init()
//
// Purpose: Decontructor :Periodic Transfer or Queue Head Manage module Initilization
//
// Parameters:
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    Lock();
#ifdef EHCI_PROBE
    RETAILMSG(CRITICAL_LOG, (L"PM Init\r\n"));
    RETAILMSG(CRITICAL_LOG, (L"MObject is %x\r\n", m_pCPhysMem));
#endif
    if ( m_dwFrameSize == 0x400 ||  m_dwFrameSize== 0x200 ||  m_dwFrameSize== 0x100) {
         if (m_pCPhysMem && m_pCPhysMem->AllocateSpecialMemory(m_dwFrameSize*sizeof(DWORD),  ( UCHAR ** )&m_pFrameList))
             m_pFramePhysAddr = m_pCPhysMem->VaToPa((UCHAR *)m_pFrameList);
         else {
            Unlock();
            ASSERT(FALSE);
        }
    }
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"Special finished\r\n"));
#endif
    ASSERT(m_pFrameList!=NULL);
    for(DWORD dwIndex=0;dwIndex< 2*PERIOD_TABLE_SIZE;dwIndex++) {
#ifdef IRAM_PATCH
#ifdef IRAM_PATCH_EXTEND
        m_pStaticQHArray[dwIndex]= new(m_pCPhysMem) CQH(m_pCDumpPipe, NULL, NULL, NULL, FALSE);
#else
        m_pStaticQHArray[dwIndex]= new(m_pCPhysMem) CQH(m_pCDumpPipe, NULL, NULL, FALSE);
#endif
        RETAILMSG(RETAIL_LOG, (L"%d OK\r\n", dwIndex));
#else
        m_pStaticQHArray[dwIndex]= new(m_pCPhysMem) CQH(m_pCDumpPipe);
#endif
        if (m_pStaticQHArray[dwIndex] == NULL) {
            Unlock();
            return FALSE;
        }
    }
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"Static finished\r\n"));
#endif
    // Actually the 0 never be used.
    m_pStaticQHArray[0]->QueueQHead(NULL);
    m_pStaticQHArray[1]->QueueQHead(NULL);
    DWORD dwForwardBase=1;
    DWORD dwForwardMask=0;
    for(dwIndex=2;dwIndex< 2*PERIOD_TABLE_SIZE;dwIndex++) {
        if ((dwIndex & (dwIndex-1))==0) { // power of 2.
            dwForwardBase = dwIndex/2;
            dwForwardMask = dwForwardBase -1 ;
        }
        if (m_pStaticQHArray[dwIndex]) {
            m_pStaticQHArray[dwIndex]->QueueQHead(m_pStaticQHArray[dwForwardBase + (dwIndex & dwForwardMask)]);// binary queue head.
        }
        else {
            Unlock();
            return FALSE;
        }
    }
    //Attahed QHead to  FrameList;
    if (m_dwFrameSize && m_pFrameList) {
        for (dwIndex=0;dwIndex<m_dwFrameSize;dwIndex++) {
            CQH * pQH = m_pStaticQHArray[PERIOD_TABLE_SIZE +  dwIndex % PERIOD_TABLE_SIZE];
            if (pQH) {
                CNextLinkPointer staticQueueHead;
                staticQueueHead.SetNextPointer(pQH->GetPhysAddr(),TYPE_SELECT_QH,TRUE);
                *(m_pFrameList+dwIndex) = staticQueueHead.GetDWORD(); //Invalid Physical pointer.
            }
            else {
                Unlock();
                return FALSE;
            }
        }
    }
    else {
        Unlock();
        return FALSE;
    }
    Unlock();
    return TRUE;
}
// ******************************************************************
void CPeriodicMgr::DeInit()
//
// Purpose: Decontructor :Periodic Transfer or Queue Head Manage module DeInitilization
//
// Parameters:
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    Lock();
    for(DWORD dwIndex=0;dwIndex< 2*PERIOD_TABLE_SIZE;dwIndex++) {
        if (m_pStaticQHArray[dwIndex]) {
            //delete( m_pCPhysMem, 0) m_pStaticQHArray[dwIndex];
            m_pStaticQHArray[dwIndex]->~CQH();
            m_pCPhysMem->FreeMemory((PBYTE)m_pStaticQHArray[dwIndex],m_pCPhysMem->VaToPa((PBYTE)m_pStaticQHArray[dwIndex]),CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
            m_pStaticQHArray[dwIndex] = NULL;
        }
    }
    if (m_pFrameList) {
         m_pCPhysMem->FreeSpecialMemory((PBYTE)m_pFrameList);
         m_pFrameList = NULL;
    }
    Unlock();
}
// ******************************************************************
BOOL CPeriodicMgr::QueueITD(CITD * piTD,DWORD FrameIndex)
//
// Purpose: Decontructor :Queue High Speed Isoch Trasnfer.
//
// Parameters:
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    FrameIndex &= m_dwFrameMask;
    Lock();
    if (piTD && m_pFrameList && FrameIndex< m_dwFrameSize) {
        ASSERT(piTD->CNextLinkPointer::GetLinkValid()==FALSE);
        CNextLinkPointer thisITD;
        thisITD.SetNextPointer(piTD->GetPhysAddr(),TYPE_SELECT_ITD,TRUE);
        piTD->CNextLinkPointer::SetDWORD(*(m_pFrameList + FrameIndex));
        *(m_pFrameList+FrameIndex) = thisITD.GetDWORD();
        Unlock();
        return TRUE;
    }
    else {
        ASSERT(FALSE);
    }
    Unlock();
    return FALSE;
}
// ******************************************************************
BOOL CPeriodicMgr::QueueSITD(CSITD * psiTD,DWORD FrameIndex)
//
// Purpose: Decontructor :Queue High Speed Isoch Trasnfer.
//
// Parameters:
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    FrameIndex &= m_dwFrameMask;
    Lock();
    if (psiTD && m_pFrameList && FrameIndex < m_dwFrameSize ) {
        ASSERT(psiTD->CNextLinkPointer::GetLinkValid()==FALSE);
        CNextLinkPointer thisITD;
        thisITD.SetNextPointer( psiTD->GetPhysAddr(),TYPE_SELECT_SITD,TRUE);
        psiTD->CNextLinkPointer::SetDWORD(*(m_pFrameList+ FrameIndex  ));
        *(m_pFrameList+ FrameIndex) = thisITD.GetDWORD();
        Unlock();
        return TRUE;
    }
    else {
        ASSERT(FALSE);
    }
    Unlock();
    return FALSE;
}

// ******************************************************************
BOOL CPeriodicMgr::DeQueueTD(DWORD dwPhysAddr,DWORD FrameIndex)
//
// Purpose: Dequeue the transfer descriptor for High speed Isoch Transfer
//
// Parameters:  dwPhysAddr - pointer to the physical address of TD
//              FrameIndex - frame index of corresponding TD
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    FrameIndex &= m_dwFrameMask;
    Lock();
    if (m_pFrameList && FrameIndex< m_dwFrameSize) {
        CNextLinkPointer * curPoint = (CNextLinkPointer *)(m_pFrameList+ FrameIndex);
        if (curPoint!=NULL && curPoint->GetLinkValid() && 
                curPoint->GetLinkType()!= TYPE_SELECT_QH &&
                curPoint->GetPointer() != dwPhysAddr ) {
            curPoint=curPoint->GetNextLinkPointer(m_pCPhysMem);
        }
        if (curPoint && curPoint->GetPointer() == dwPhysAddr) { // We find it
            CNextLinkPointer * pNextPoint=curPoint->GetNextLinkPointer(m_pCPhysMem);
            if (pNextPoint ) {
                curPoint->SetDWORD(pNextPoint->GetDWORD());
                Unlock();
                return TRUE;
            }
            else
                ASSERT(FALSE);
        }
        //else
        //    ASSERT(FALSE);
    }
    else 
        ASSERT(FALSE);
    Unlock();
    return FALSE;
}
PERIOD_TABLE CPeriodicMgr::periodTable[64] =
   {   // period, qh-idx, s-mask
        1,  0, 0xFF,        // Dummy
        1,  0, 0xFF,        // 1111 1111 bits 0..7

        2,  0, 0x55,        // 0101 0101 bits 0,2,4,6
        2,  0, 0xAA,        // 1010 1010 bits 1,3,5,7

        4,  0, 0x11,        // 0001 0001 bits 0,4
        4,  0, 0x44,        // 0100 0100 bits 2,6
        4,  0, 0x22,        // 0010 0010 bits 1,5
        4,  0, 0x88,        // 1000 1000 bits 3,7

        8,  0, 0x01,        // 0000 0001 bits 0
        8,  0, 0x10,        // 0001 0000 bits 4
        8,  0, 0x04,        // 0000 0100 bits 2
        8,  0, 0x40,        // 0100 0000 bits 6
        8,  0, 0x02,        // 0000 0010 bits 1
        8,  0, 0x20,        // 0010 0000 bits 5
        8,  0, 0x08,        // 0000 1000 bits 3
        8,  0, 0x80,        // 1000 0000 bits 7

        16,  1, 0x01,       // 0000 0001 bits 0
        16,  2, 0x01,       // 0000 0001 bits 0
        16,  1, 0x10,       // 0001 0000 bits 4
        16,  2, 0x10,       // 0001 0000 bits 4
        16,  1, 0x04,       // 0000 0100 bits 2
        16,  2, 0x04,       // 0000 0100 bits 2
        16,  1, 0x40,       // 0100 0000 bits 6
        16,  2, 0x40,       // 0100 0000 bits 6
        16,  1, 0x02,       // 0000 0010 bits 1
        16,  2, 0x02,       // 0000 0010 bits 1
        16,  1, 0x20,       // 0010 0000 bits 5
        16,  2, 0x20,       // 0010 0000 bits 5
        16,  1, 0x08,       // 0000 1000 bits 3
        16,  2, 0x08,       // 0000 1000 bits 3
        16,  1, 0x80,       // 1000 0000 bits 7
        16,  2, 0x80,       // 1000 0000 bits 7

        32,  3, 0x01,       // 0000 0000 bits 0
        32,  5, 0x01,       // 0000 0000 bits 0
        32,  4, 0x01,       // 0000 0000 bits 0
        32,  6, 0x01,       // 0000 0000 bits 0
        32,  3, 0x10,       // 0000 0000 bits 4
        32,  5, 0x10,       // 0000 0000 bits 4
        32,  4, 0x10,       // 0000 0000 bits 4
        32,  6, 0x10,       // 0000 0000 bits 4
        32,  3, 0x04,       // 0000 0000 bits 2
        32,  5, 0x04,       // 0000 0000 bits 2
        32,  4, 0x04,       // 0000 0000 bits 2
        32,  6, 0x04,       // 0000 0000 bits 2
        32,  3, 0x40,       // 0000 0000 bits 6
        32,  5, 0x40,       // 0000 0000 bits 6
        32,  4, 0x40,       // 0000 0000 bits 6
        32,  6, 0x40,       // 0000 0000 bits 6
        32,  3, 0x02,       // 0000 0000 bits 1
        32,  5, 0x02,       // 0000 0000 bits 1
        32,  4, 0x02,       // 0000 0000 bits 1
        32,  6, 0x02,       // 0000 0000 bits 1
        32,  3, 0x20,       // 0000 0000 bits 5
        32,  5, 0x20,       // 0000 0000 bits 5
        32,  4, 0x20,       // 0000 0000 bits 5
        32,  6, 0x20,       // 0000 0000 bits 5
        32,  3, 0x04,       // 0000 0000 bits 3
        32,  5, 0x04,       // 0000 0000 bits 3
        32,  4, 0x04,       // 0000 0000 bits 3
        32,  6, 0x04,       // 0000 0000 bits 3
        32,  3, 0x40,       // 0000 0000 bits 7
        32,  5, 0x40,       // 0000 0000 bits 7
        32,  4, 0x40,       // 0000 0000 bits 7
        32,  6, 0x40,       // 0000 0000 bits 7

    };

//******************************************************************************
CQH * CPeriodicMgr::QueueQHead(CQH * pQh,UCHAR uInterval,UCHAR offset,BOOL bHighSpeed)
//
// Purpose: Periodic Manager: Queue the queue head for the Periodic Transfer
//
// Parameters:
//
// Returns: Pointer to the object CQH
//
// Notes:
//******************************************************************
{
    if (pQh) {
        if (uInterval> PERIOD_TABLE_SIZE)
            uInterval= PERIOD_TABLE_SIZE;
        Lock();
        for (UCHAR bBit=PERIOD_TABLE_SIZE;bBit!=0;bBit>>=1) {
            if ((bBit & uInterval)!=0) { // THis is correct interval
                // Normalize the parameter.
                uInterval = bBit;
                if (offset>=uInterval)
                    offset = uInterval -1;
                CQH * pStaticQH=NULL ;
                if (bHighSpeed) {
                    pStaticQH=m_pStaticQHArray[ periodTable[uInterval+offset].qhIdx +1] ;
                    pQh->SetSMask(periodTable[uInterval+offset].InterruptScheduleMask);
                }
                else 
                    pStaticQH =  m_pStaticQHArray[uInterval+offset];
                if (pStaticQH!=NULL) {
                    pQh->QueueQHead( pStaticQH->GetNextQueueQHead(m_pCPhysMem));
                    pStaticQH->QueueQHead( pQh );
                    Unlock();
                    return pStaticQH;
                }
                else
                    ASSERT(FALSE);
            }
        }
        ASSERT(FALSE);
        CQH * pStaticQH = m_pStaticQHArray[1];
        if (pStaticQH!=NULL) {
            pQh->QueueQHead( pStaticQH->GetNextQueueQHead(m_pCPhysMem));
            if (bHighSpeed)
                pQh->SetSMask(0xff);
            pStaticQH->QueueQHead( pQh );
            Unlock();
            return pStaticQH;
        }
        else
            ASSERT(FALSE);

        Unlock();
    }
    ASSERT(FALSE);
    return NULL;

}

//**********************************************************************
BOOL CPeriodicMgr::DequeueQHead( CQH * pQh)
//
// Purpose: CPeriodicMgr: Dequeue the queue head after transfer completed
//
// Parameters:
//
// Returns: TRUE - success, FALSE - failure, not found
//
// Notes:
// ******************************************************************
{
    if (pQh==NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
    Lock();
    for (DWORD dwIndex=PERIOD_TABLE_SIZE;dwIndex<2*PERIOD_TABLE_SIZE;dwIndex ++) {
        CQH *pCurPrev= m_pStaticQHArray[dwIndex];
        if (pCurPrev!=NULL) {
            while (pCurPrev!=NULL) {
                CQH *pCur=pCurPrev->GetNextQueueQHead(m_pCPhysMem);
                if (pCur == pQh)
                    break;
                else
                    pCurPrev = pCur;
            }
            if (pCurPrev!=NULL) { // Found
                ASSERT(pCurPrev->GetNextQueueQHead(m_pCPhysMem) == pQh);
                pCurPrev->QueueQHead(pQh->GetNextQueueQHead(m_pCPhysMem));
                pQh->QueueQHead(NULL);
                Unlock();
                Sleep(2); // Make Sure it outof EHCI Scheduler.
                return TRUE;
            }

        }
        else
            ASSERT(FALSE);
    }
    Unlock();
    ASSERT(FALSE);
    return FALSE;
}


// ************************************CAsyncMgr******************************
//********************************************************************
#ifdef IRAM_PATCH
CAsyncMgr::CAsyncMgr(IN CPhysMem * const pCPhysMem, IN CPhysMem * const pCPhysEleMem)
    :m_pCPhysMem(pCPhysMem)
    ,m_pCPhysEleMem(pCPhysEleMem)
    ,m_pCDumpPipe(new CDummyPipe(pCPhysMem, pCPhysEleMem))        /*BookMarks Async Dummy Pipe Created*/
#else
CAsyncMgr::CAsyncMgr(IN CPhysMem * const pCPhysMem)
    :m_pCPhysMem(pCPhysMem)
    , m_pCDumpPipe(new CDummyPipe(pCPhysMem))
#endif
//
// Purpose: Constructor for Asynchronous Manager
//
// Parameters:  pCPhysMem - Pointer to CPhysMem object
//
// Returns: Nothing
//
// Notes:
// ******************************************************************

{
     m_pStaticQHead =NULL;
#ifdef EHCI_PROBE
    RETAILMSG(CRITICAL_LOG, (L"AsynMgr DumpP @ %x\r\n", m_pCDumpPipe->GetCPhysMem()));
#endif
}

//*******************************************************************
CAsyncMgr::~CAsyncMgr()
//
// Purpose: Decontructor :Asynchronous Manager
//
// Parameters:
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    DeInit();
}

//********************************************************************
BOOL CAsyncMgr::Init()
//
// Purpose: Initialize the Asychronous Manager including setting up CQH,
//          queue head.
//
// Parameters:  
//
// Returns: TRUE - success, FALSE - failure
//
// Notes: 
// ******************************************************************
{
    Lock();
#ifdef IRAM_PATCH
    RETAILMSG(CRITICAL_LOG, (L"AM Init\r\n"));
#ifdef IRAM_PATCH_EXTEND
    m_pStaticQHead = new (m_pCPhysEleMem) CQH(m_pCDumpPipe, NULL, NULL, NULL, TRUE);
#else
    m_pStaticQHead = new (m_pCPhysEleMem) CQH(m_pCDumpPipe, NULL, NULL, TRUE);
#endif
    RETAILMSG(CRITICAL_LOG, (L"OK, A static QH @ %x\r\n", m_pStaticQHead->GetPhysAddr()));
#else
    m_pStaticQHead = new (m_pCPhysMem) CQH(m_pCDumpPipe);
#endif
    if (m_pStaticQHead) {
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Set Queue Head with H = 1 at 0x%x\r\n"), m_pStaticQHead->GetPhysAddr()));
        m_pStaticQHead->SetReclamationFlag(TRUE);
        m_pStaticQHead ->QueueQHead(m_pStaticQHead); // Point to itself.
        Unlock();
        return TRUE;
    }
    Unlock();
    return FALSE;
}

//********************************************************************
void CAsyncMgr::DeInit()
//
// Purpose: De-initialize the Asychronous Manager 
//
// Parameters:  
//
// Returns: Nothing
//
// Notes: 
// ******************************************************************
{
    Lock();
    if (m_pStaticQHead){
        //delete (m_pCPhysMem) m_pStaticQHead;
        m_pStaticQHead->~CQH();
        m_pCPhysMem->FreeMemory((PBYTE)m_pStaticQHead,m_pCPhysMem->VaToPa((PBYTE)m_pStaticQHead),CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
    }
    m_pStaticQHead= NULL;
    Unlock();
}

//**********************************************************************
CQH *  CAsyncMgr::QueueQH(CQH * pQHead)
//
// Purpose: Put the new queue head into the Asychronous Manager to be transferred
//
// Parameters:  pQHead - Pointer to CQH object
//
// Returns: Pointer to the first object of linked-list of queue head, NULL - failure
//
// Notes: 
// ******************************************************************
{
    if (m_pStaticQHead && pQHead){
        Lock();
#ifdef IRAM_PATCH
        pQHead->QueueQHead( m_pStaticQHead->GetNextQueueQHead(m_pCPhysEleMem));
#else
        pQHead->QueueQHead( m_pStaticQHead->GetNextQueueQHead(m_pCPhysMem));
#endif
        m_pStaticQHead ->QueueQHead(pQHead);
        Unlock();
        return m_pStaticQHead;
    };
    return NULL;
}

//***********************************************************************
BOOL CAsyncMgr::DequeueQHead( CQH * pQh)
//
// Purpose: Dequeue the queue head from the Asynchronous Manager       
//
// Parameters:  pQh - Queue head to be dequeue
//
// Returns: TRUE - success, FALSE - failure
//
// Notes: 
// ******************************************************************
{
    CQH * pPrevQH = m_pStaticQHead;
    CQH * pCurQH = NULL;
    Lock();
#ifdef EHCI_PROBE
    RETAILMSG(CRITICAL_LOG, (L"De+\r\n"));
#endif
    for (DWORD dwIndex=0;dwIndex<0x1000;dwIndex++)
        if (pPrevQH) {
#ifdef IRAM_PATCH
            pCurQH= pPrevQH->GetNextQueueQHead(m_pCPhysEleMem);
#else
            pCurQH= pPrevQH->GetNextQueueQHead(m_pCPhysMem);
#endif
            if (pCurQH == m_pStaticQHead || pCurQH == pQh)
                break;
            else
                pPrevQH = pCurQH;
        };
    if ( pCurQH && pPrevQH &&  pCurQH == pQh) {
#ifdef IRAM_PATCH
        pPrevQH->QueueQHead(pCurQH->GetNextQueueQHead(m_pCPhysEleMem));
#else
        pPrevQH->QueueQHead(pCurQH ->GetNextQueueQHead(m_pCPhysMem));
#endif
        Unlock();
#ifdef EHCI_PROBE
        RETAILMSG(CRITICAL_LOG, (L"DeT-\r\n"));
#endif
        return TRUE;
    }
    else
        ASSERT(FALSE);
    Unlock();
#ifdef EHCI_PROBE
        RETAILMSG(CRITICAL_LOG, (L"DeF-\r\n"));
#endif
    return FALSE;

};

// ******************************BusyPipeList****************************
//*******************************************************************
BOOL  CBusyPipeList::Init()
//
// Purpose: Initialize the CBusyPipeList class. The busy pipe list class is to
//          spawn out a thread keeping on monitoring all the created pipes and see
//          if the USB transfer has been completed.
//
// Parameters:
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    m_fCheckTransferThreadClosing=FALSE;
    m_pBusyPipeList = NULL;
    m_hCheckForDoneTransfersEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( m_hCheckForDoneTransfersEvent == NULL ) {
        DEBUGMSG(ZONE_ERROR, (TEXT("-CPipe::Initialize. Error creating process done transfers event\n")));
        return FALSE;
    }
#ifdef DEBUG
    m_debug_numItemsOnBusyPipeList=0;
#endif

    // set up our thread to check for done transfers
    // currently, the context passed to CheckForDoneTransfersThread is ignored
    m_hCheckForDoneTransfersThread = CreateThread( 0, 0, CheckForDoneTransfersThreadStub, (PVOID)this, 0, NULL );
    if ( m_hCheckForDoneTransfersThread == NULL ) {
        DEBUGMSG(ZONE_ERROR, (TEXT("-CPipe::Initialize. Error creating process done transfers thread\n")));
        return FALSE;
    }
    CeSetThreadPriority( m_hCheckForDoneTransfersThread, g_IstThreadPriority + RELATIVE_PRIO_CHECKDONE );
    return TRUE;

}

//**********************************************************************************
void CBusyPipeList::DeInit()
//
// Purpose: Deinitialize the busy pipe list and close the CheckForDoneTransfersThread
//
// Parameters:  
//
// Returns: Nothing
//
// Notes: 
// ******************************************************************
{
    m_fCheckTransferThreadClosing=TRUE;
    if ( m_hCheckForDoneTransfersEvent != NULL ) {
        SignalCheckForDoneTransfers();
        if ( m_hCheckForDoneTransfersThread ) {
            DWORD dwWaitReturn = WaitForSingleObject( m_hCheckForDoneTransfersThread, 5000 );
            if ( dwWaitReturn != WAIT_OBJECT_0 ) {
                DEBUGCHK( 0 ); // check why thread is blocked
#pragma prefast(suppress: 258, "Try to recover gracefully from a pathological failure")
                TerminateThread( m_hCheckForDoneTransfersThread, DWORD(-1) );
            }
            CloseHandle( m_hCheckForDoneTransfersThread );
            m_hCheckForDoneTransfersThread = NULL;
        }
        CloseHandle( m_hCheckForDoneTransfersEvent );
        m_hCheckForDoneTransfersEvent = NULL;
    }
    
}
// ******************************************************************
// Scope: public static
void CBusyPipeList::SignalCheckForDoneTransfers( void )
//
// Purpose: This function is called when an interrupt is received by
//          the CHW class. We then signal the CheckForDoneTransfersThread
//          to check for any transfers which have completed
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: DO MINIMAL WORK HERE!! Most processing should be handled by
//        The CheckForDoneTransfersThread. If this procedure blocks, it
//        will adversely affect the interrupt processing thread.
// ******************************************************************
{
    DEBUGCHK( m_hCheckForDoneTransfersEvent && m_hCheckForDoneTransfersThread );
    SetEvent( m_hCheckForDoneTransfersEvent );
}
// ******************************************************************
ULONG CALLBACK CBusyPipeList::CheckForDoneTransfersThreadStub( IN PVOID pContext)
{
    return ((CBusyPipeList *)pContext)->CheckForDoneTransfersThread( );
}
// Scope: private static
ULONG CBusyPipeList::CheckForDoneTransfersThread( )
//
// Purpose: Thread for checking whether busy pipes are done their
//          transfers. This thread should be activated whenever we
//          get a USB transfer complete interrupt (this can be
//          requested by the InterruptOnComplete field of the TD)
//
// Parameters: 32 bit pointer passed when instantiating thread (ignored)
//
// Returns: 0 on thread exit
//
// Notes: 
// ******************************************************************
{
    // Note that for WinCE 6.00 the USB EHCI driver runs in kernel mode by
    // default and the SetKMode() API is no longer supported.

    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("+CPipe::CheckForDoneTransfersThread\n")) );

    PPIPE_LIST_ELEMENT pPrev = NULL;
    PPIPE_LIST_ELEMENT pCurrent = NULL;

    DEBUGCHK( m_hCheckForDoneTransfersEvent != NULL );

    while ( !m_fCheckTransferThreadClosing ) {
#ifdef IRAM_PATCH
        WaitForSingleObject( m_hCheckForDoneTransfersEvent, INFINITE);
        RETAILMSG(CRITICAL_LOG, (L"HW+\r\n"));
#else
        WaitForSingleObject( m_hCheckForDoneTransfersEvent, m_FrameListSize/2 );
#endif
        if ( m_fCheckTransferThreadClosing ) {
            break;
        }
        Lock();
    #ifdef DEBUG // make sure m_debug_numItemsOnBusyPipeList is accurate
        {
            int debugCount = 0;
            PPIPE_LIST_ELEMENT pDebugElement = m_pBusyPipeList;
            while ( pDebugElement != NULL ) {
                pDebugElement = pDebugElement->pNext;
                debugCount++;
            }
            DEBUGCHK( debugCount == m_debug_numItemsOnBusyPipeList );
        }
        BOOL fDebugNeedProcessing = m_debug_numItemsOnBusyPipeList > 0;
        DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE && fDebugNeedProcessing, (TEXT("CPipe::CheckForDoneTransfersThread - #pipes to check = %d\n"), m_debug_numItemsOnBusyPipeList) );
        DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE && !fDebugNeedProcessing, (TEXT("CPipe::CheckForDoneTransfersThread - warning! Called when no pipes were busy\n")) );
    #endif // DEBUG
        pPrev = NULL;
        pCurrent = m_pBusyPipeList;
        while ( pCurrent != NULL ) {
            pCurrent->pPipe->CheckForDoneTransfers();
                // this pipe is still busy. Move to next item
            pPrev = pCurrent;
            pCurrent = pPrev->pNext;
        }
//        DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE && fDebugNeedProcessing, (TEXT("CPipe::CheckForDoneTransfersThread - #pipes still busy = %d\n"), m_debug_numItemsOnBusyPipeList) );
        Unlock();
    }
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("-CPipe::CheckForDoneTransfersThread\n")) );
    return 0;
}
// ******************************************************************
// Scope: protected static 
BOOL CBusyPipeList::AddToBusyPipeList( IN CPipe * const pPipe,
                               IN const BOOL fHighPriority )
//
// Purpose: Add the pipe indicated by pPipe to our list of busy pipes.
//          This allows us to check for completed transfers after 
//          getting an interrupt, and being signaled via 
//          SignalCheckForDoneTransfers
//
// Parameters: pPipe - pipe to add to busy list
//
//             fHighPriority - if TRUE, add pipe to start of busy list,
//                             else add pipe to end of list.
//
// Returns: TRUE if pPipe successfully added to list, else FALSE
//
// Notes: 
// ******************************************************************
{
    //DEBUGMSG( ZONE_PIPE, (TEXT("+CPipe::AddToBusyPipeList - new pipe(%s) 0x%x, pri %d\n"), pPipe->GetPipeType(), pPipe, fHighPriority ));

    PREFAST_DEBUGCHK( pPipe != NULL );
    BOOL fSuccess = FALSE;

    // make sure there nothing on the pipe already (it only gets officially added after this function succeeds).
    Lock();
#ifdef DEBUG
{
    // make sure this pipe isn't already in the list. That should never happen.
    // also check that our m_debug_numItemsOnBusyPipeList is correct
    PPIPE_LIST_ELEMENT pBusy = m_pBusyPipeList;
    int count = 0;
    while ( pBusy != NULL ) {
        DEBUGCHK( pBusy->pPipe != NULL &&
                  pBusy->pPipe != pPipe );
        pBusy = pBusy->pNext;
        count++;
    }
    DEBUGCHK( m_debug_numItemsOnBusyPipeList == count );
}
#endif // DEBUG
    
    PPIPE_LIST_ELEMENT pNewBusyElement = new PIPE_LIST_ELEMENT;
    if ( pNewBusyElement != NULL ) {
        pNewBusyElement->pPipe = pPipe;
        if ( fHighPriority || m_pBusyPipeList == NULL ) {
            // add pipe to start of list
            pNewBusyElement->pNext = m_pBusyPipeList;
            m_pBusyPipeList = pNewBusyElement;
        } else {
            // add pipe to end of list
            PPIPE_LIST_ELEMENT pLastElement = m_pBusyPipeList;
            while ( pLastElement->pNext != NULL ) {
                pLastElement = pLastElement->pNext;
            }
            pNewBusyElement->pNext = NULL;
            pLastElement->pNext = pNewBusyElement;
        }
        fSuccess = TRUE;
    #ifdef DEBUG
        m_debug_numItemsOnBusyPipeList++;
    #endif // DEBUG
    }
    Unlock();
//    DEBUGMSG( ZONE_PIPE, (TEXT("-CPipe::AddToBusyPipeList - new pipe(%s) 0x%x, pri %d, returning BOOL %d\n"), pPipe->GetPipeType(), pPipe, fHighPriority, fSuccess) );

    //Debug Purpose Only
    //this->DumpBusyPipeList();

    return fSuccess;
}

// ******************************************************************
// Scope: protected static
void CBusyPipeList::RemoveFromBusyPipeList( IN CPipe * const pPipe )
//
// Purpose: Remove this pipe from our busy pipe list. This happens if
//          the pipe is suddenly aborted or closed while a transfer
//          is in progress
//
// Parameters: pPipe - pipe to remove from busy list
//
// Returns: Nothing
//
// Notes: 
// ******************************************************************
{
//    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CPipe::RemoveFromBusyPipeList - pipe(%s) 0x%x\n"), pPipe->GetPipeType(), pPipe ) );
    Lock();
#ifdef DEBUG
    BOOL debug_fRemovedPipe = FALSE;
{
    // check m_debug_numItemsOnBusyPipeList
    PPIPE_LIST_ELEMENT pBusy = m_pBusyPipeList;
    int count = 0;
    while ( pBusy != NULL ) {
        DEBUGCHK( pBusy->pPipe != NULL );
        pBusy = pBusy->pNext;
        count++;
    }
    DEBUGCHK( m_debug_numItemsOnBusyPipeList == count );
}
#endif // DEBUG
    PPIPE_LIST_ELEMENT pPrev = NULL;
    PPIPE_LIST_ELEMENT pCurrent = m_pBusyPipeList;
    while ( pCurrent != NULL ) {
        if ( pCurrent->pPipe == pPipe ) {
            // Remove item from the linked list
            if ( pCurrent == m_pBusyPipeList ) {
                DEBUGCHK( pPrev == NULL );
                m_pBusyPipeList = m_pBusyPipeList->pNext;
            } else {
                DEBUGCHK( pPrev != NULL &&
                          pPrev->pNext == pCurrent );
                pPrev->pNext = pCurrent->pNext;
            }
            delete pCurrent;
            pCurrent = NULL;
        #ifdef DEBUG
            debug_fRemovedPipe = TRUE;
            DEBUGCHK( --m_debug_numItemsOnBusyPipeList >= 0 );
        #endif // DEBUG
            break;
        } else {
            // Check next item
            pPrev = pCurrent;
            pCurrent = pPrev->pNext;
        }
    }
    Unlock();
//    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE && debug_fRemovedPipe, (TEXT("-CPipe::RemoveFromBusyPipeList, removed pipe(%s) 0x%x\n"), pPipe->GetPipeType(), pPipe));
//    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE && !debug_fRemovedPipe, (TEXT("-CPipe::RemoveFromBusyPipeList, pipe(%s) 0x%x was not on busy list\n"), pPipe->GetPipeType(), pPipe ));
}


#ifdef IRAM_PATCH
//Eric Just implement tool function here, not used yet
void CBusyPipeList::RemoveAllFromBusyPipeList(void)
{
    PPIPE_LIST_ELEMENT pCur, pNext;
    pCur = m_pBusyPipeList;
    pNext = pCur->pNext;
    for (;;)
    {
        pCur->pPipe->ClosePipe();
        pCur = pNext;
        if (pCur == NULL) break;
        pNext = pCur->pNext;
    }
    return;
}

void CBusyPipeList::DumpBusyPipeList(void)
{
    PPIPE_LIST_ELEMENT pCur;
    DWORD index = 0;
    RETAILMSG(1, (L"Dump list @ %x\r\n", this));
    pCur = m_pBusyPipeList;
    while (pCur != NULL)
    {
        index++;
        RETAILMSG(1, (L"No.%d is %x\r\n", index, pCur));
        pCur = pCur->pNext;
    }
    return;
}
#endif

#ifdef ASYNC_PARK_MODE
#undef ASYNC_PARK_MODE
#endif
#define ASYNC_PARK_MODE 1
#define FRAME_LIST_SIZE 0x400

// ************************************CHW ******************************  

//*************************************************************
#ifdef IRAM_PATCH
CHW::CHW( IN const REGISTER portBase,
                              IN const DWORD dwSysIntr,
                              IN CPhysMem** const pCPhysMem,
                              //IN CUhcd * const pHcd,
                              IN LPVOID pvUhcdPddObject,
                              IN LPCTSTR lpDeviceRegistry)
: m_cBusyPipeList(FRAME_LIST_SIZE)
, m_cPeriodicMgr (pCPhysMem[0], pCPhysMem[2], FRAME_LIST_SIZE)
, m_cAsyncMgr(pCPhysMem[0], pCPhysMem[2])
, m_deviceReg(HKEY_LOCAL_MACHINE,lpDeviceRegistry)
#else
CHW::CHW( IN const REGISTER portBase,
                              IN const DWORD dwSysIntr,
                              IN CPhysMem * const pCPhysMem,
                              //IN CUhcd * const pHcd,
                              IN LPVOID pvUhcdPddObject,
                              IN LPCTSTR lpDeviceRegistry)
: m_cBusyPipeList(FRAME_LIST_SIZE)
, m_cPeriodicMgr (pCPhysMem,FRAME_LIST_SIZE)
, m_cAsyncMgr(pCPhysMem)
, m_deviceReg(HKEY_LOCAL_MACHINE,lpDeviceRegistry)

#endif
//
// Purpose: Constructor for CHW
//
// Parameters:  
//
// Returns: Nothing
//
// Notes: 
// ******************************************************************
{
// definitions for static variables
    DEBUGMSG( ZONE_INIT, (TEXT("+CHW::CHW base=0x%x, intr=0x%x\n"), portBase, dwSysIntr));
    //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("CHW::CHW created 0x%x\r\n"), this));
    g_fPowerUpFlag = FALSE;
    g_fPowerResuming = FALSE;
    m_capBase = portBase;
        m_portBase = portBase+Read_CapLength();//EHCI 2.2.1   
    m_NumOfPort=Read_HCSParams().bit.N_PORTS;
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"enter CHW Initializer\r\n"));
#endif
    //m_pHcd = pHcd;
#ifdef IRAM_PATCH
    m_pMem = pCPhysMem[0];
    m_pIRamDataMem = pCPhysMem[1];
    m_pIRamEleMem = pCPhysMem[2];
    RETAILMSG(RETAIL_LOG, (L"CHW 3 memory object is %x %x and %x\r\n", m_pMem, m_pIRamDataMem, m_pIRamEleMem));
#else
    m_pMem = pCPhysMem;
#endif
    m_pPddContext = pvUhcdPddObject;
    m_frameCounterHighPart = 0;
    m_frameCounterLowPart = 0;
    m_FrameListMask = FRAME_LIST_SIZE-1;  
    m_pFrameList = 0;
    m_dwSysIntr = dwSysIntr;
    m_hUsbInterruptEvent = NULL;
    m_hUsbHubChangeEvent = NULL;
    m_hUsbInterruptThread = NULL;
    m_fUsbInterruptThreadClosing = FALSE;

    m_fFrameLengthIsBeingAdjusted = FALSE;
    m_fStopAdjustingFrameLength = FALSE;
    m_hAdjustDoneCallbackEvent = NULL;
    m_uNewFrameLength = 0;
    m_dwCapability = 0;
    m_bDoResume=FALSE;  
    m_bUSBClockStop = FALSE;

    RETAILMSG(FALSE,(TEXT("((SEHCDPdd*)m_pPddContext)->bIsMX31TO2= %d"),((SEHCDPdd*)m_pPddContext)->bIsMX31TO2));
    if(!((SEHCDPdd*)m_pPddContext)->bIsMX31TO2)
    {
        m_bUSBPanicMode = TRUE;
    }
    m_bUSBIdleSuspend = FALSE;
    SetForceReAttach(0);
   

    m_dwOTGSupport = ((SEHCDPdd *)(pvUhcdPddObject))->dwOTGSupport;
    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("OTG Support = %d\r\n"), m_dwOTGSupport));

    DWORD StringSize = sizeof(m_szOTGGroup) / sizeof(TCHAR);
    StringCchCopy(m_szOTGGroup,StringSize,((SEHCDPdd *)(pvUhcdPddObject))->szOTGGroup);

    //lstrcpy(m_szOTGGroup, ((SEHCDPdd *)(pvUhcdPddObject))->szOTGGroup);

    m_hAsyncDoorBell=CreateEvent(NULL, FALSE,FALSE,NULL);
    InitializeCriticalSection( &m_csFrameCounter );
}

//*********************************************************
CHW::~CHW()
//
// Purpose: Destructor of CHW object
//
// Parameters:  
//
// Returns: Nothing
//
// Notes: 
// ******************************************************************
{
    if (m_dwSysIntr)
        KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &m_dwSysIntr, sizeof(m_dwSysIntr), NULL, 0, NULL);
    DeInitialize();
    if (m_hAsyncDoorBell)
        CloseHandle(m_hAsyncDoorBell);
    DeleteCriticalSection( &m_csFrameCounter );
    //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("CHW::~CHW created 0x%x\r\n"), this));
}

// **************************************************************
BOOL CHW::DisableAsyncSchedule()
//
// Purpose: Request controller to skip the process of asynchronous scheduling
//          This is used during the setup.
//
// Parameters:  
//
// Returns: TRUE - success, FALSE - failure
//
// Notes: 
// ******************************************************************
{
    USBCMD usbcmd=Read_USBCMD();
    // Follow the rule in 4.8 EHCI
    while (usbcmd.bit.ASchedEnable!= Read_USBSTS().bit.ASStatus)
         Sleep(1);

    if (usbcmd.bit.ASchedEnable ==1) {
        usbcmd.bit.ASchedEnable=0;
        Write_USBCMD(usbcmd);
    }

    while (usbcmd.bit.ASchedEnable!= Read_USBSTS().bit.ASStatus)
         Sleep(1);

    return TRUE;
}

//*********************************************************************
BOOL CHW::EnableAsyncSchedule()
//
// Purpose: Request controller to enable the process of asynchronous scheduling
//          This is used during the setup and necessary for Asynchronous Transfer.
//
// Parameters:  
//
// Returns: TRUE - success, FALSE - failure
//
// Notes: 
// ******************************************************************
{
#ifdef ASYNC_PARK_MODE
    if (Read_HHCCP_CAP().bit.Async_Park) {
            USBCMD usbcmd=Read_USBCMD();
            usbcmd.bit.ASchedPMEnable=1;
            usbcmd.bit.ASchedPMCount =3;
            Write_USBCMD(usbcmd);
    }
#endif

    USBCMD usbcmd=Read_USBCMD();
    // Follow the rule in 4.8 EHCI
    while (usbcmd.bit.ASchedEnable!= Read_USBSTS().bit.ASStatus)
         Sleep(1);

    if (usbcmd.bit.ASchedEnable!=1) {
        usbcmd.bit.ASchedEnable=1;
        Write_USBCMD(usbcmd);
    }

    return TRUE;
}

//******************************************************************
BOOL CHW::PowerOnAllPorts()
//
// Purpose: Power on all the USB ports on the controller
//
// Parameters:  
//
// Returns: TRUE - success, FALSE - failure
//
// Notes: 
// ******************************************************************
{
    //Write_EHCIRegister(CONFIGFLAG,1);
    // Power On all the port.
    for (DWORD dwPort=1; dwPort <= m_NumOfPort ; dwPort ++ ) {
        PORTSC portSc = Read_PORTSC( dwPort);
        portSc.bit.Power=1;
        portSc.bit.Owner=0;
        // Do not touch write to clean register
        portSc.bit.ConnectStatusChange=0;
        portSc.bit.EnableChange=0;
        portSc.bit.OverCurrentChange=0;

        // Testing for full speed
        //rtSc.bit.PFSC = 0x1;
        //portSc.bit.TestControl = 0x6;
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Power On PortSC b/f =0x%x\r\n"), portSc.ul));
            
        Write_PORTSC(dwPort,portSc);         
        portSc = Read_PORTSC( dwPort);
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Power On PortSC after =0x%x\r\n"), portSc.ul));

    }

    Sleep(50);
    return TRUE;
}

//**********************************************************
void CHW::WriteAsyncListAddr(IN const DWORD addr) 
//
// Purpose: Setup the address of next asynchronous queue head to be executed.
//
// Parameters:  addr - asynchronous address
//
// Returns: Nothing
//
// Notes: 
// ******************************************************************
{ 
    Write_EHCIRegister(ASYNCLISTADDR,addr);
}
// *****************************************************************
BOOL CHW::ConfigureHS()
//
//  Purpose: Configure the Host Controller i.e. it would be used for
//           OTG transceiver
//
//  Parameters: 
//
//  Returns:  TRUE - success, FALSE - failure
// *****************************************************************
{
    {
        USBCMD usbcmd=Read_USBCMD();

        usbcmd.bit.HCReset=1;
        Write_USBCMD(usbcmd);
        for (DWORD dwCount=0;dwCount<50 && (Read_USBCMD().bit.HCReset!=0);dwCount++)
            Sleep( 20 );
        usbcmd=Read_USBCMD();
        if (usbcmd.bit.HCReset!=0) // If can not reset within 1 second, we assume this is bad device.
            return FALSE;

        //Add specific for tdi
            //Check the value of USB MODE
        // In OTG case, the USBMODE need to set again after reset.
        {
            DWORD value;
            value = Read_EHCIRegister(USBMODE);
            //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USBMODE = 0x%x\r\n"), value));
            if ((value & 0x3) != 0x3)
            {
                Write_EHCIRegister(USBMODE, value|0x3);
                while ((Read_EHCIRegister(USBMODE) & 0x3) != 0x3);
            }
        }

        //usbcmd.bit.FrameListSize=0;// We use 1k for Periodic List.
        usbcmd.bit.FrameListSize=1024;// We use 1k for Periodic List.
        m_FrameListMask = 0x3ff;  // Available Bit in 
        Write_USBCMD(usbcmd);
    }
    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS, (TEXT("CHW::Initialize - end signalling global reset\n")));
    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS, (TEXT("CHW::Initialize - setting USBINTR to all interrupts on\n")));

    {
        USBINTR usbint;
        // initialize interrupt register - set all interrupts to enabled
        usbint.ul=(DWORD)0x457;
        usbint.bit.Reserved=0;
        Write_USBINTR(usbint );
    }

    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS && ZONE_VERBOSE, (TEXT("CHW::Initialize - setting FRNUM = 0\n")));
    // initialize FRNUM register with index 0 of frame list
    {
        FRINDEX frindex;
        frindex.ul=0;
        Write_FRINDEX(frindex);
    }
    Write_EHCIRegister(CTLDSSEGMENT,0);//We only support 32-bit address space now.

    // initialize FLBASEADD with address of frame list
    {
        ULONG frameListPhysAddr = m_cPeriodicMgr.GetFrameListPhysAddr();
        DEBUGMSG(ZONE_INIT && ZONE_REGISTERS && ZONE_VERBOSE, (TEXT("CHW::Initialize - setting FLBASEADD = 0x%X\n"), frameListPhysAddr));
        DEBUGCHK( frameListPhysAddr != 0 );
        // frame list should be aligned on a 4Kb boundary
        DEBUGCHK( (frameListPhysAddr & EHCD_FLBASEADD_MASK) == frameListPhysAddr );
        Write_EHCIRegister(PERIODICLISTBASE,frameListPhysAddr);
        // Follow the rule in 4.8 EHCI
        USBCMD usbcmd=Read_USBCMD();
        while (usbcmd.bit.PSchedEnable!= Read_USBSTS().bit.PSStatus)
            Sleep(1);
        if (usbcmd.bit.PSchedEnable!=1) {
            usbcmd.bit.PSchedEnable=1;
            Write_USBCMD(usbcmd);
        }
    }
    // Initial Async Shedule to Enable.
    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS && ZONE_VERBOSE, (TEXT("CHW::Initialize - Enable Async Sched \n")));
    {
#ifdef EHCI_PROBE
        RETAILMSG(1, (L"Async start @ %x\r\n", m_cAsyncMgr.GetPhysAddr()));  
#endif
        Write_EHCIRegister(ASYNCLISTADDR,m_cAsyncMgr.GetPhysAddr());
#ifdef ASYNC_PARK_MODE
        if (Read_HHCCP_CAP().bit.Async_Park) {
            USBCMD usbcmd=Read_USBCMD();
            usbcmd.bit.ASchedPMEnable=1;
            usbcmd.bit.ASchedPMCount =3;
            Write_USBCMD(usbcmd);
        }
#else
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("NO ASYNC_PARK_MODE\r\n")));
#endif
        USBCMD usbcmd=Read_USBCMD();
        // Follow the rule in 4.8 EHCI
        while (usbcmd.bit.ASchedEnable!= Read_USBSTS().bit.ASStatus)
            Sleep(1);
        if (usbcmd.bit.ASchedEnable!=1) {
            usbcmd.bit.ASchedEnable=1;
            Write_USBCMD(usbcmd);
        }
    }    


    {
        TXFILLTUNING txft = Read_TXFILLTUNING();   
        txft.bit.txfifothres = 0x4;
        Write_TXFILLTUNING(txft);
        txft = Read_TXFILLTUNING();    
    }

    return TRUE;
}

// ******************************************************************
BOOL CHW::Initialize( )
// Purpose: Reset and Configure the Host Controller with the schedule.
//
// Parameters: portBase - base address for host controller registers
//
//             dwSysIntr - system interrupt number to use for USB
//                         interrupts from host controller
//
//             frameListPhysAddr - physical address of frame list index
//                                 maintained by CPipe class
//
//             pvUhcdPddObject - PDD specific structure used during suspend/resume
//
// Returns: TRUE if initialization succeeded, else FALSE
//
// Notes: This function is only called from the CUhcd::Initialize routine.
//
//        This function is static
// ******************************************************************
{
    DEBUGMSG( ZONE_INIT, (TEXT("+CHW::Initialize\n")));

    DEBUGCHK( m_frameCounterLowPart == 0 &&
              m_frameCounterHighPart == 0 );

    // set up the frame list area.
    if ( m_portBase == 0 || 
            m_cPeriodicMgr.Init()==FALSE ||
            m_cAsyncMgr.Init() == FALSE ||
            m_cBusyPipeList.Init()==FALSE) {
        DEBUGMSG( ZONE_ERROR, (TEXT("-CHW::Initialize - zero Register Base or CeriodicMgr or CAsyncMgr fails\n")));
        ASSERT(FALSE);
        return FALSE;
    }
    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS, (TEXT("CHW::Initialize - signalling global reset\n")));

    // Create a file mapping for USB clock state
    BSPUSBClockCreateFileMapping();

    ConfigureHS();

    // m_hUsbInterrupt - Auto Reset, and Initial State = non-signaled
    DEBUGCHK( m_hUsbInterruptEvent == NULL );
    m_hUsbInterruptEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hUsbHubChangeEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( m_hUsbInterruptEvent == NULL || m_hUsbHubChangeEvent==NULL ) {
        DEBUGMSG(ZONE_ERROR, (TEXT("-CHW::Initialize. Error creating USBInterrupt or USBHubInterrupt event\n")));
        return FALSE;
    }

    InterruptDisable( m_dwSysIntr ); // Just to make sure this is really ours.
    // Initialize Interrupt. When interrupt id # m_sysIntr is triggered,
    // m_hUsbInterruptEvent will be signaled. Last 2 params must be NULL
    if ( !InterruptInitialize( m_dwSysIntr, m_hUsbInterruptEvent, NULL, NULL) ) {
        DEBUGMSG(ZONE_ERROR, (TEXT("-CHW::Initialize. Error on InterruptInitialize\r\n")));
        return FALSE;
    }

    if (m_dwOTGSupport)
        InterruptDisable(m_dwSysIntr);
    else
    {
        //KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &m_dwSysIntr, sizeof(m_dwSysIntr), NULL, 0, NULL);
    }

    // Start up our IST - the parameter passed to the thread
    // is unused for now
    DEBUGCHK( m_hUsbInterruptThread == NULL &&
              m_fUsbInterruptThreadClosing == FALSE );
    if (m_hUsbInterruptThread==NULL)
        m_hUsbInterruptThread = CreateThread( 0, 0, UsbInterruptThreadStub, this, 0, NULL );
    if ( m_hUsbInterruptThread == NULL ) {
        DEBUGMSG(ZONE_ERROR, (TEXT("-CHW::Initialize. Error creating IST\n")));
        return FALSE;
    }
    CeSetThreadPriority( m_hUsbInterruptThread, g_IstThreadPriority );
    // Initial All port route to this host.
    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS && ZONE_VERBOSE, (TEXT("CHW::Initialize - Initial All port route \n")));

    PowerOnAllPorts();

    EnableAsyncSchedule();

    DEBUGMSG( ZONE_INIT, (TEXT("-CHW::Initialize, success!\n")));
    return TRUE;
}
// ******************************************************************
void CHW::DeInitialize( void )
//
// Purpose: Delete any resources associated with static members
//
// Parameters: none
//
// Returns: nothing
//
// Notes: This function is only called from the ~CUhcd() routine.
//
//        This function is static
// ******************************************************************
{
    m_fUsbInterruptThreadClosing = TRUE; // tell USBInterruptThread that we are closing
    // m_hAdjustDoneCallbackEvent <- don't need to do anything to this
    // m_uNewFrameLength <- don't need to do anything to this

    // Wake up the interrupt thread and give it time to die.
    if ( m_hUsbInterruptEvent ) {
        SetEvent(m_hUsbInterruptEvent);
        if ( m_hUsbInterruptThread ) {
            DWORD dwWaitReturn = WaitForSingleObject(m_hUsbInterruptThread, 1000);
            if ( dwWaitReturn != WAIT_OBJECT_0 ) {
                DEBUGCHK( 0 );
#pragma prefast(suppress: 258, "Try to recover gracefully from a pathological failure")
                TerminateThread(m_hUsbInterruptThread, DWORD(-1));
            }
            CloseHandle(m_hUsbInterruptThread);
            m_hUsbInterruptThread = NULL;
        }
        // we have to close our interrupt before closing the event!
        InterruptDisable( m_dwSysIntr );

        CloseHandle(m_hUsbInterruptEvent);
        m_hUsbInterruptEvent = NULL;
    } else {
        InterruptDisable( m_dwSysIntr );
    }
    // Stop The Controller.
    {
        USBCMD usbcmd=Read_USBCMD();
        usbcmd.bit.RunStop=0;
        Write_USBCMD(usbcmd);
        while( Read_USBSTS().bit.HCHalted == 0 ) //Wait until it stop.
            Sleep(1);
    }
    m_cPeriodicMgr.DeInit();
    m_cAsyncMgr.DeInit();
    m_cBusyPipeList.DeInit();
    // no need to free the frame list; the entire pool will be freed as a unit.
    m_pFrameList = 0;
    m_fUsbInterruptThreadClosing = FALSE;
    m_frameCounterLowPart = 0;
    m_frameCounterHighPart = 0;

    // Remove file mapping from it
    BSPUSBClockDeleteFileMapping();
}

// ******************************************************************
void CHW::EnterOperationalState( void )
//
// Purpose: Signal the host controller to start processing the schedule
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: This function is only called from the CUhcd::Initialize routine.
//        It assumes that CPipe::Initialize and CHW::Initialize
//        have already been called.
//
//        This function is static
// ******************************************************************
{
    DEBUGMSG( ZONE_INIT, (TEXT("+CHW::EnterOperationalState\n")));
    DWORD dwIntThreshCtrl = EHCI_REG_IntThreshCtrl_DEFAULT;
    if (!(m_deviceReg.IsKeyOpened() && m_deviceReg.GetRegValue(EHCI_REG_IntThreshCtrl, (LPBYTE)&dwIntThreshCtrl, sizeof(dwIntThreshCtrl)))) {
        dwIntThreshCtrl = EHCI_REG_IntThreshCtrl_DEFAULT;
    }
    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS && ZONE_VERBOSE, (TEXT("CHW::EnterOperationalState - clearing status reg\n")));
    Clear_USBSTS( );
    USBCMD usbcmd=Read_USBCMD();

    DEBUGMSG(ZONE_INIT && ZONE_REGISTERS && ZONE_VERBOSE, (TEXT("CHW::EnterOperationalState - setting USBCMD run bit\n")));
    usbcmd.bit.FrameListSize = 0; // 1k Flame Entry. Sync with Initialization.
    usbcmd.bit.IntThreshCtrl = dwIntThreshCtrl; // Setup by registry.
    usbcmd.bit.RunStop = 1;
    Write_USBCMD( usbcmd );

    // According to 21.13.1 of Freescale, set CONFIGFLAG after RUN
    Write_EHCIRegister(CONFIGFLAG,1);

    // Just for safety, make sure those flags must set properly after enter into operation state
    m_bUSBClockStop = FALSE;

    if(!((SEHCDPdd*)m_pPddContext)->bIsMX31TO2)
         m_bUSBPanicMode = TRUE;



    DEBUGMSG( ZONE_INIT, (TEXT("-CHW::EnterOperationalState\n")));
}

//************************************************************************
void CHW::RunStopUSBController(BOOL fRun)
//
// Purpose: Start or Stop the USB Host Controller by controlling the run/stop bit
//          in USBCMD register.
//
// Parameters: fRun - TRUE: Start to run, FALSE: stop the controller
//
// Returns: Nothing.
//
// ******************************************************************
{
    USBCMD usbcmd=Read_USBCMD();
    if (fRun)
    {
        if(usbcmd.bit.RunStop == 0) {
            // clear run bit
            usbcmd.bit.RunStop= 1;
            Write_USBCMD( usbcmd );  
            USBINTR usbIntr;
            usbIntr.ul=0x457;
            // clear all interrupts
            Write_USBINTR(usbIntr);
            // spin until the controller really is stopped
            while( Read_USBSTS().bit.HCHalted == 1 )
                Sleep(0); //Wait until it stop.
        }
    }
    else
    {
        if(usbcmd.bit.RunStop) {
            // clear run bit
            usbcmd.bit.RunStop= 0;
            Write_USBCMD( usbcmd );  
            USBINTR usbIntr;
            usbIntr.ul=0x4;  // only enable the PORTSTATUS change one.
            // clear all interrupts
            Write_USBINTR(usbIntr);
            // spin until the controller really is stopped
            while( Read_USBSTS().bit.HCHalted == 0 ) //Wait until it stop.
                Sleep(0);
        }
    }
}

//*****************************************************************
void CHW::StartHostController(void)
//
// Purpose: This is to resume the host controller from suspend stage.
//          It would call RunStopUSBController(TRUE) to resume execution
//
// Parameter: 
//
// Returns:
//
//*******************************************************************
{
    int i = 0;
#if 0 // Remove-W4: Warning C4189 workaround
    USBCMD usbcmd=Read_USBCMD();
    USBSTS usbsts=Read_USBSTS();
#else
    Read_USBCMD();
    Read_USBSTS();
#endif

    // Check run bit. Despite what the UHCI spec says, Intel's controller
    // does not always set the HCHALTED bit when the controller is stopped.
    for (UINT port =1; port <= m_NumOfPort; port ++)
    {
        // We need to enable back the SUSP flag before performing reset
        PORTSC portSC = Read_PORTSC(port);      
        if (portSC.bit.Suspend == 1)
        {           
            portSC.bit.ForcePortResume =1;
            portSC.bit.Suspend = 0;
            Write_PORTSC(port, portSC);
        }
                
        do {
            i++;
            portSC = Read_PORTSC(port);            
            if (i > 1000)
            {
                // No matter what we still wake up this device
                // If you connect a device with no driver loaded and then 
                // cancel the dialog for driver input, power saving suspend
                // and it would not wakeup due to SUSPEND = 1 all the time.
                // Resolve that .. let it go.
                if (portSC.bit.ForcePortResume == 0)
                {
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Force WakeUp on Host Suspend(%d), ForcePortResume(%d)\r\n"),
                        portSC.bit.Suspend, portSC.bit.ForcePortResume));
                    break;
                }
                i = 0;
            }
        } while ((portSC.bit.Suspend == 1) || (portSC.bit.ForcePortResume == 1));

        portSC=Read_PORTSC(port);;      
        portSC.bit.EnableChange = 0;
        Write_PORTSC(port, portSC);     
    }

    RunStopUSBController(TRUE);

    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("StartHostController completed\r\n")));
}   

// ******************************************************************
void CHW::StopHostController( void )
//
// Purpose: Signal the host controller to stop processing the schedule
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes:
//
//        This function is static
// ******************************************************************
{
#if 0 // Remove-W4: Warning C4189 workaround
    USBCMD usbcmd=Read_USBCMD();
#else
    Read_USBCMD();
#endif

    // Check run bit. Despite what the UHCI spec says, Intel's controller
    // does not always set the HCHALTED bit when the controller is stopped.
    RunStopUSBController(FALSE);

    for (UINT port =1; port <= m_NumOfPort; port ++) {
        PORTSC portSC=Read_PORTSC(port);;
        // no point doing any work unless the port is enabled
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("PORTSC before stop 0x%x\r\n"), portSC.ul));
        if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.Enabled ) {
             //portSC.bit.ConnectStatusChange=0;
             //portSC.bit.EnableChange=1;
             //portSC.bit.OverCurrentChange=0;        
                //
             //portSC.bit.ForcePortResume =0;
             portSC.bit.Suspend=1;
             //portSC.bit.WakeOnConnect = 1;
             //portSC.bit.WakeOnDisconnect =1;
             //portSC.bit.WakeOnOverCurrent =1;
             Write_PORTSC( port, portSC );
        }
    }
}

#ifdef IRAM_PATCH
//Eric : Just implement tool function here, not used yet
void CHW::RemoveAllFromBusyPipeList(void)
{
    Lock();
    m_cBusyPipeList.RemoveAllFromBusyPipeList();
    Unlock();
}

void CHW::DumpBusyPipeList(void)
{
    Lock();
    m_cBusyPipeList.DumpBusyPipeList();
    Unlock();
}
#endif

// ******************************************************************
BOOL CHW::AsyncBell()
//
// Purpose: This function is to enable the interrupt on async advance 
//          doorbell bit on USBCMD.  It would then wait for interrupt coming in.
//        
//          Basically, this is to tell the host controller to issue an
//          interrupt the next time it advances asynchronous schedule.
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes:
//
//********************************************************************
{
    m_DoorBellLock.Lock();
    ResetEvent(m_hAsyncDoorBell);
    USBCMD usbcmd=Read_USBCMD();
    usbcmd.bit.IntOnAADoorbell=1;
    Write_USBCMD( usbcmd );  
    DWORD dwReturn=WaitForSingleObject( m_hAsyncDoorBell,10);
    m_DoorBellLock.Unlock();
    return (dwReturn == WAIT_OBJECT_0);
}

//*************************************************************
BOOL  CHW::AsyncDequeueQH( CQH * pQh) 
//
// Purpose: This function is to de-queue the queue head from the 
//          asynchronous queue manager and wait for Async Bell.
//
// Parameters: pQh - pointer to CQH object to be deqeueue.
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
//
//********************************************************************
{   
#ifdef EHCI_PROBE
    RETAILMSG(CRITICAL_LOG, (L"A DequeueQHead\r\n"));
#endif
    BOOL bReturn= m_cAsyncMgr.DequeueQHead( pQh);
    if (bReturn) {
        AsyncBell();
    }
    return bReturn;
};

//*****************************************************************
BOOL CHW::PeriodQueueITD(CITD * piTD,DWORD FrameIndex) 
//
// Purpose: Queue the transfer descriptor for Isoschronous High Speed Transfer
//
// Parameters: piTD - pointer to CITD object
//             FrameIndex - frame index where to scheudle
//
// Returns: Nothing.
//
// Notes:
//
//********************************************************************
{ 
    FRINDEX frameIndex= Read_FRINDEX();
    if (((FrameIndex  - frameIndex.bit.FrameIndex) & m_FrameListMask) > 1)        
        return  m_cPeriodicMgr.QueueITD(piTD,FrameIndex); 
    else 
        return FALSE;// To Close EHCI 4.7.2.1
};

//******************************************************************
BOOL CHW::PeriodQueueSITD(CSITD * psiTD,DWORD FrameIndex)
//
// Purpose: Queue the transfer descriptor for Isoschronous Full/Low Speed Transfer
//
// Parameters: piTD - pointer to CITD object
//             FrameIndex - frame index where to scheudle
//
// Returns: Nothing.
//
// Notes:
//
//********************************************************************
{ 
    FRINDEX frameIndex= Read_FRINDEX();
    if (((FrameIndex  - frameIndex.bit.FrameIndex) & m_FrameListMask) > 1)        
        return  m_cPeriodicMgr.QueueSITD(psiTD,FrameIndex);
    else
        return FALSE;
};

//****************************************************************************
BOOL CHW::PeriodDeQueueTD(DWORD dwPhysAddr,DWORD FrameIndex) 
//
// Purpose: De-queue the TD from the periodic list
//
// Parameters: dwPhysAddr - Physical address of TD
//             FrameIndex - frame index where to scheudle
//
// Returns: Nothing.
//
// Notes:
//
//********************************************************************
{ 
    FRINDEX frameIndex= Read_FRINDEX();
    
    while (((FrameIndex  - frameIndex.bit.FrameIndex) & m_FrameListMask) <=1)  {
        Sleep(1);
        frameIndex= Read_FRINDEX();
    }
    return  m_cPeriodicMgr.DeQueueTD(dwPhysAddr, FrameIndex); 
};

DWORD CALLBACK CHW::CeResumeThreadStub ( IN PVOID context )
{
    return ((CHW *)context)->CeResumeThread ( );
}
// ******************************************************************
DWORD CHW::CeResumeThread ( )
//
// Purpose: Force the HCD to reset and regenerate itself after power loss.
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes: Because the PDD is probably maintaining pointers to the Hcd and Memory
//   objects, we cannot free/delete them and then reallocate. Instead, we destruct
//   them explicitly and use the placement form of the new operator to reconstruct
//   them in situ. The two flags synchronize access to the objects so that they
//   cannot be accessed before being reconstructed while also guaranteeing that
//   we don't miss power-on events that occur during the reconstruction.
//
//        This function is static
// ******************************************************************
{
    // reconstruct the objects at the same addresses where they were before;
    // this allows us not to have to alert the PDD that the addresses have changed.

    DEBUGCHK( g_fPowerResuming == FALSE );

    // order is important! resuming indicates that the hcd object is temporarily invalid
    // while powerup simply signals that a powerup event has occurred. once the powerup
    // flag is cleared, we will repeat this whole sequence should it get resignalled.
    g_fPowerUpFlag = FALSE;
    g_fPowerResuming = TRUE;

    DeviceDeInitialize();
    for (;;) {  // breaks out upon successful reinit of the object

        if (DeviceInitialize())
            break;
        // getting here means we couldn't reinit the HCD object!
        ASSERT(FALSE);
        DEBUGMSG(ZONE_ERROR, (TEXT("USB cannot reinit the HCD at CE resume; retrying...\n")));
        DeviceDeInitialize();
        Sleep(15000);
    }

    // the hcd object is valid again. if a power event occurred between the two flag
    // assignments above then the IST will reinitiate this sequence.
    g_fPowerResuming = FALSE;
    if (g_fPowerUpFlag)
        PowerMgmtCallback(TRUE);

    return 0;
}
DWORD CHW::UsbInterruptThreadStub( IN PVOID context )
{
    return ((CHW *)context)->UsbInterruptThread();
}

// ******************************************************************
DWORD CHW::UsbInterruptThread( )
//
// Purpose: Main IST to handle interrupts from the USB host controller
//
// Parameters: context - parameter passed in when starting thread,
//                       (currently unused)
//
// Returns: 0 on thread exit.
//
// Notes:
//
//        This function is private
// ******************************************************************
{
    HANDLE hHost = NULL, hXcvr = NULL;
    HANDLE hDetach = NULL;
    ULONG WaitReturn;
    ULONG ret;
    BOOL bPortChanged;
    BOOL bCableOnly;
    TCHAR szUSBHostObjectName[30];
    TCHAR szUSBXcvrObjectName[30];
    TCHAR szUSBHostDetachName[30];
    DWORD expiretime;
    DWORD currtime;
    BOOL fIdle;
    DWORD timeout = USB_IDLE_TIMEOUT;

    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USB Interrupt Thread running again IsOTGSupport? %d\r\n"), m_dwOTGSupport));
    if (m_dwOTGSupport)
    {
        DWORD StringSize = sizeof(szUSBHostObjectName) / sizeof(TCHAR);
        StringCchCopy(szUSBHostObjectName,StringSize,USBHostObjectName);
        //lstrcpy(szUSBHostObjectName, USBHostObjectName);

        StringCchCat(szUSBHostObjectName, StringSize, m_szOTGGroup);
        //lstrcat(szUSBHostObjectName, m_szOTGGroup);
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("CHW: CreateEvent:%s\r\n"), szUSBHostObjectName));
        hHost = CreateEvent(NULL, FALSE, FALSE, szUSBHostObjectName);
        if (GetLastError() == ERROR_ALREADY_EXISTS)
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USBHost: Opened an existing Func Event\r\n")));
        else
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("UFN: Created a new Func Event\r\n")));

        if (hHost == NULL)
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("UFN: Create Event Failed for func!\r\n")));

        StringSize = sizeof(szUSBXcvrObjectName) / sizeof(TCHAR);
        StringCchCopy(szUSBXcvrObjectName,StringSize,USBXcvrObjectName);
        //lstrcpy(szUSBXcvrObjectName, USBXcvrObjectName);
        StringCchCat(szUSBXcvrObjectName, StringSize, m_szOTGGroup);
        //lstrcat(szUSBXcvrObjectName, m_szOTGGroup);
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USBHost: CreateEvent:%s\r\n"), szUSBXcvrObjectName));
        hXcvr = CreateEvent(NULL, FALSE, FALSE, szUSBXcvrObjectName);

        if (GetLastError() == ERROR_ALREADY_EXISTS)
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("UFN: Opened an existing XCVR Event\r\n")));
        else
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("UFN: Created a new XCVR Event\r\n")));

        if (hXcvr == NULL)
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("UFN: Create Event Failed for xcvr!\r\n")));

        StringSize = sizeof(szUSBHostDetachName) / sizeof(TCHAR);
        StringCchCopy(szUSBHostDetachName,StringSize,USBHostDetachName);
        //lstrcpy(szUSBHostDetachName, USBHostDetachName);
        StringCchCat(szUSBHostDetachName, StringSize, m_szOTGGroup);
        //lstrcat(szUSBHostDetachName, m_szOTGGroup);
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USBHost: CreateEvent:%s\r\n"), szUSBHostDetachName));

        hDetach = CreateEvent(NULL, FALSE, FALSE, szUSBHostDetachName);

        if (GetLastError() == ERROR_ALREADY_EXISTS)
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("UFN: Opened an existing Detach Event\r\n")));
        else
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("UFN: Created a new Detach Event\r\n")));

        if (hDetach == NULL)
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("UFN: Create Event Failed for detach!\r\n")));

XCVR_SIG:
        bInHost = FALSE;
        bPortChanged = FALSE;
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Host: Waiting for signal from XCVR!!!\r\n")));
        WaitReturn = WaitForSingleObject(hHost, INFINITE);
        RETAILMSG(DEBUG_LOG_USBCV, (L"Host Get Start\r\n"));

        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Host: Host Driver in charge now!!!\r\n")));
        bInHost = TRUE;

        // We need to setup the host configuration again.
        ConfigureHS();

        if (!InterruptInitialize(m_dwSysIntr, m_hUsbInterruptEvent, NULL, 0))
        {
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (L"ERROR: UfnPdd_Init: Interrupt initialization failed\r\n"));
            return FALSE;
        }

#ifdef COLD_MSC_RECOGNIZE
        {
            //software disconnect
            //RETAILMSG(1, (L"---- ulpiData is %x\r\n", EHCI_ULPI_ReadReg(ULPI_OTG_CTRL_RW)));
            //RETAILMSG(1, (L"---- ULPI disconnect\r\n"));
            EHCI_ULPI_WriteReg(ULPI_OTG_CTRL_C, ULPI_OTGCTRL_DRV_VBUS_EXT);
            EHCI_ULPI_WriteReg(ULPI_OTG_CTRL_C, ULPI_OTGCTRL_EXT_VBUS_IND);
            EHCI_ULPI_WriteReg(ULPI_OTG_CTRL_C, ULPI_OTGCTRL_DRV_VBUS);
            //RETAILMSG(1, (L"---- ulpiData is %x\r\n", EHCI_ULPI_ReadReg(ULPI_OTG_CTRL_RW)));
            //RETAILMSG(1, (L"Sleep\r\n"));
            Sleep(200);
            //RETAILMSG(1, (L"connect again\r\n"));
            EHCI_ULPI_WriteReg(ULPI_OTG_CTRL_S, ULPI_OTGCTRL_DRV_VBUS_EXT);
            EHCI_ULPI_WriteReg(ULPI_OTG_CTRL_S, ULPI_OTGCTRL_DRV_VBUS);
            EHCI_ULPI_WriteReg(ULPI_OTG_CTRL_S, ULPI_OTGCTRL_EXT_VBUS_IND);
            //RETAILMSG(1, (L"---- ulpiData is %x\r\n", EHCI_ULPI_ReadReg(ULPI_OTG_CTRL_RW)));
        }
#endif

        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USBHost SysIntr:0x%x\r\n"), m_dwSysIntr));
        PowerOnAllPorts();
        EnableAsyncSchedule();
        EnterOperationalState();

        // Handle attach, the PCI doesn't generate again
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("HUB Status Change (A)\r\n")));
        SetEvent(m_hUsbHubChangeEvent);
        bPortChanged = TRUE;
        timeout = USB_CHECK_CABLE_TIMEOUT;

    }


    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("+CHW::Entered USBInterruptThread\n")));

    bCableOnly = FALSE;
    fIdle = FALSE;
    expiretime = 0;
    currtime = 0;

    while ( !m_fUsbInterruptThreadClosing ) {
        USBSTS usbsts;
        USB_HUB_AND_PORT_STATUS rStatus;
        bPortChanged = FALSE;

        while (WaitForSingleObject(m_hUsbInterruptEvent, timeout) == WAIT_TIMEOUT )
        {
            // if the mini-A is disconnected while we're in the host driver, but have not
            // had a device attachment, then the host won't know it's been unplugged.
            // We'll see it here.
            RETAILMSG(DEBUG_LOG_USBCV, (L"Host Timeout\r\n"));
            if (GetForceReAttach() != 0)
            {
                if ((GetForceReAttach() == 2) && (!m_dwOTGSupport))
                    goto INTERRUPT_PROCESSING;
                else
                {
                    timeout = INFINITE;
                    continue;
                }
            }

            GetPortStatus(1, rStatus, NULL);

            if (!m_dwOTGSupport)
            {
                // This is timeout and suspend the port, put the system to sleep
                if (rStatus.status.port.PortConnected == 0)
                {
                    // Go into lower power mode now
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USB Host in Idle State and Suspend\r\n")));
                    PowerMgmtCallback(TRUE);
                    timeout = INFINITE;
                    m_bUSBIdleSuspend = TRUE;
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Host - SUSPEND\r\n")));
                }
                else
                {
                    // This is something attached
                    timeout = INFINITE;
                }
                continue;
            }


            //PORTSC portSC=Read_PORTSC(1);
            //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("PORTSC = 0x%x\r\n"), portSC.ul));

            if ( IsOTGHostDevice() && (rStatus.status.port.PortConnected == 0) )
                bCableOnly = TRUE;
            else if ( !IsOTGHostDevice() && (rStatus.status.port.PortConnected == 0) )
            {
                DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("mini-A unplugged... going back into XVCR\r\n")));
                bCableOnly = FALSE;
                InterruptDisable(m_dwSysIntr);
                RETAILMSG(CRITICAL_LOG, (L"H->X1\r\n"));
                SetEvent(hXcvr);
                bInHost = FALSE;
                SetForceReAttach(0);
                goto XCVR_SIG;
            }
        }

INTERRUPT_PROCESSING:
        RETAILMSG(FALSE, (L"INTERRUPT_PROCESSING\r\n"));
#ifdef USBCV_FIX
        if (m_dwOTGSupport)
        {
            OTGSC otgSc;
            
            otgSc = Read_OTGSC();
            RETAILMSG(DEBUG_LOG_USBCV, (L"h otgsc %x\r\n", otgSc.ul));
            
            Write_EHCIRegister(OTGSC_OFFSET, otgSc.ul);
        }
#endif

        if (!m_dwOTGSupport)
        {

            if (m_bUSBClockStop)
            {
                //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USB Clock start\r\n")));
                m_bUSBClockStop = FALSE;
                StartUSBClock();
            }

            if(!((SEHCDPdd*)m_pPddContext)->bIsMX31TO2)
            {
                if (m_bUSBPanicMode == FALSE)
                {
                    m_bUSBPanicMode = TRUE;
                    DDKClockEnablePanicMode();
                }
            }

            if (m_bUSBIdleSuspend)
            {
                for (UINT port =1; port <= m_NumOfPort; port ++)
                {
                    PORTSC portSC=Read_PORTSC(port);;
                    if ( portSC.bit.PHCD == 1 )
                    {
                        portSC.bit.PHCD=0;
                        if (portSC.bit.Suspend)
                            portSC.bit.ForcePortResume = 1;
                        Write_PORTSC( port, portSC );
                    }
                }
                SetForceReAttach(1);
                SetPHYPowerMgmt(FALSE);
                CHW::StartHostController();
                m_bUSBIdleSuspend = FALSE;
            }
        }

        BSPUsbCheckWakeUp();

        GetPortStatus(1, rStatus, NULL);


        usbsts = Read_USBSTS();

        if ((m_dwOTGSupport) && (GetForceReAttach() == 0))
        {

            GetPortStatus(1, rStatus, NULL);

            if (bCableOnly)
            {
                if ( !IsOTGHostDevice() && (rStatus.status.port.PortConnected == 0) )
                {
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("mini-A unplugged... going back into XVCR\r\n")));
                    InterruptDisable(m_dwSysIntr);
                    RETAILMSG(DEBUG_LOG_USBCV, (L"H->X2\r\n"));
                    SetEvent(hXcvr);
                    bInHost = FALSE;
                    bCableOnly = FALSE;
                    goto XCVR_SIG;
                }

                if (IsOTGHostDevice() && (rStatus.status.port.PortConnected))
                    bCableOnly = FALSE;
            }
            else
            {
                // We need to implement a timer just in case of extremely quick connect, disconnect
                // If it is more than 2 sec and no response, we can then disconnect it.
                if ((rStatus.status.port.PortConnected == 0) && (!IsOTGHostDevice()) && (!usbsts.bit.PortChanged))
                {
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Disconnect with nothing\r\n")));
                    if (fIdle == FALSE)
                    {
                        expiretime = GetTickCount() + USB_DISCONNECT_TIMEOUT;
                        fIdle = TRUE;
                    }
                    else
                    {
                        fIdle = TRUE;
                        currtime = GetTickCount();

                        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("CurrTime(0x%x), ExpireTime(0x%x), timeout(0x%x)\r\n"), currtime, expiretime, USB_DISCONNECT_TIMEOUT));
                        if (((expiretime > currtime) && ((expiretime - currtime) >= USB_DISCONNECT_TIMEOUT)) ||
                            // The GetTickCount loop back, so we just workaround by using currtime only
                            ((expiretime < currtime) && (currtime >= USB_DISCONNECT_TIMEOUT)))
                        {
                            fIdle = FALSE;
                            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("HUB STATUS CHANGE (C)\r\n")));
                            SetEvent(m_hUsbHubChangeEvent);
                            bPortChanged = TRUE;
                        }
                    }
                }
                else
                {
                    fIdle = FALSE;
                }
            }
        }

        if ( m_fUsbInterruptThreadClosing ) {
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("m_fUsbInterruptThreadClosing\r\n")));
            break;
        }

    #ifdef DEBUG
        DWORD dwFrame;
        GetFrameNumber(&dwFrame); // calls UpdateFrameCounter
        DEBUGMSG( ZONE_REGISTERS, (TEXT("!!!interrupt!!!! on frame index + 1 = 0x%08x, USBSTS = 0x%04x\n"), dwFrame, usbsts.ul ) );
        if (usbsts.bit.HSError) { // Error Happens.
            DumpAllRegisters( );
            //ASSERT(FALSE);
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("HSError but let continue USBSTS(0x%x)\r\n"), usbsts.ul));
        }
    #else
        UpdateFrameCounter();
    #endif // DEBUG

        if ((usbsts.bit.PortChanged) || (GetForceReAttach()!=0)) {
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("HUB Status Change (B) with ForceReAttach %d\r\n"), GetForceReAttach()));
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("PortSC[0] = 0x%x\r\n"), Read_PORTSC(1).ul));
            for (UINT port =1; port <= m_NumOfPort; port ++)
            {
                // This would happen on some suspended USB HUB. If it is idle for a while
                // without anything attached, it would suspend itself. When a device plugs in,
                // the port would return SUSPEND.  We need to do ReAttach again to get the HUB
                // running to normal mode.
                PORTSC portSC = Read_PORTSC(port);
                if (portSC.bit.Suspend == 1)
                {
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Port %d need to wakeup (0x%x)\r\n"), port, portSC.ul));
                    SetForceReAttach(1);
                    break;
                }
            }

            SetEvent(m_hUsbHubChangeEvent);
            bPortChanged = TRUE;
            // this part should only be called by non-OTG case
            if ((GetForceReAttach() == 1) && (!m_dwOTGSupport))
                timeout = USB_CHECK_CABLE_TIMEOUT;
            else if (GetForceReAttach() == 2)
            {
                if (!m_dwOTGSupport)
                    timeout = INFINITE;
            }

        }


        Write_USBSTS(usbsts);//        Clear_USBSTS( );

        if (usbsts.bit.ASAdvance)
            SetEvent(m_hAsyncDoorBell);

        // TODO - differentiate between USB interrupts, which are
        // for transfers, and host interrupts (UHCI spec 2.1.2).
        // For the former, we need to call CPipe::SignalCheckForDoneTransfers.
        // For the latter, we need to call whoever will handle
        // resume/error processing.

        // For now, we just notify CPipe so that transfers
        // can be checked for completion


        // This flag gets cleared in the resume thread.
        if(g_fPowerUpFlag)
        {
            if (m_bDoResume) {
                g_fPowerUpFlag=FALSE;
                USBCMD USBCmd = Read_USBCMD();
                USBCmd.bit.RunStop=1;
                Sleep(20);
                Write_USBCMD(USBCmd);
            }
            else {
                if (g_fPowerResuming) {
                    // this means we've restarted an IST and it's taken an early interrupt;
                    // just pretend it didn't happen for now because we're about to be told to exit again.
                    continue;
                }

                HcdPdd_InitiatePowerUp((DWORD) m_pPddContext);
                HANDLE ht;
                while ((ht = CreateThread(NULL, 0, CeResumeThreadStub, this, 0, NULL)) == NULL) {
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("HCD IST: cannot spin a new thread to handle CE resume of USB host controller; sleeping.\n")));
                    Sleep(15000);  // 15 seconds later, maybe it'll work.
                }
                CeSetThreadPriority( ht, g_IstThreadPriority );
                CloseHandle(ht);

                // The CE resume thread will force this IST to exit so we'll be cooperative proactively.
                DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("HCD Resume from working\r\n")));
                break;
            }
        }
        else if (GetForceReAttach() == 0)
            SignalCheckForDoneTransfers( );

        InterruptDone(m_dwSysIntr);

        if ((bPortChanged) && (m_dwOTGSupport))
        {

            GetPortStatus(1, rStatus, NULL);

//          DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("rStatus connect = 0x%x\r\n"), rStatus.status.port.PortConnected));

            //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Check bResumeReset = %d\r\n"), m_bResumeReset));
            if ((rStatus.status.port.PortConnected == 0) || (GetForceReAttach() != 0))
            {
                int i = 2000;
                // it may be the chance just unplug the device but not line
                // just wait for 2 sec max
                if ((GetForceReAttach() == 0)||(rStatus.status.port.PortConnected == 0)) // Don't do the checking on resume from suspend
                {
                    while (IsOTGHostDevice() == TRUE) // Wait until switch back to 1 for ID in OTGSC
                    {
                       Sleep(1);
                       i--;
                       if (i == 0)
                          break;

                       GetPortStatus(1, rStatus, NULL);
                       if (rStatus.status.port.PortConnected != 0)
                          break;
                    }
                }

                // It may have been slow in "connecting" after the ResetAndEnable
                // Check again before giving up
                //
                // Some device really response slow and can not update the PortConnected
                // bit until after a while
                Sleep(50);
                //PORTSC portSC=Read_PORTSC(1);
                //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Finally PORTSC = 0x%x\r\n"), portSC.ul));

                GetPortStatus(1, rStatus, NULL);
                if ((rStatus.status.port.PortConnected == 0) || (GetForceReAttach()!= 0))
                {
                    //Wait for the posting of semaphore
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Wait for Detach\r\n")));
                    ret = WaitForSingleObject(hDetach, INFINITE);
                    SetForceReAttach(0);
                    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Exit the host driver going back into XVCR\r\n")));
                    InterruptDisable(m_dwSysIntr);
                    RETAILMSG(CRITICAL_LOG, (L"H->X3\r\n"));
                    SetEvent(hXcvr);
                    bInHost = FALSE;
                    bCableOnly = FALSE;
                    goto XCVR_SIG;
                }
            }
        } // If (bPortChanged && m_dwOTGSupport)
        else if (!m_dwOTGSupport)
        {
            GetPortStatus(1, rStatus, NULL);
            if (rStatus.status.port.PortConnected == 0)
            {
                DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Switch back to IDLE timeout mode\r\n")));
                timeout = USB_IDLE_TIMEOUT;
            }
        }


    }

    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("-CHW::Leaving USBInterruptThread\n")));

    return (0);
}
// ******************************************************************
void CHW::UpdateFrameCounter( void )
//
// Purpose: Updates our internal frame counter
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: The UHCI frame number register is only 11 bits, or 2047
//        long. Thus, the counter will wrap approx. every 2 seconds.
//        That is insufficient for Isoch Transfers, which
//        may need to be scheduled out into the future by more
//        than 2 seconds. So, we maintain an internal 32 bit counter
//        for the frame number, which will wrap in 50 days.
//
//        This function should be called at least once every two seconds,
//        otherwise we will miss frames.
//
// ******************************************************************
{
#ifdef DEBUG
    DWORD dwTickCountLastTime = GetTickCount();
#endif

    EnterCriticalSection( &m_csFrameCounter );

#ifdef DEBUG
    // If this fails, we haven't been called in a long time,
    // so the frame number is no longer accurate
    if (GetTickCount() - dwTickCountLastTime >= 800 )
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("!UHCI - CHW::UpdateFrameCounter missed frame count;")
                     TEXT(" isoch packets may have been dropped.\n")));
    dwTickCountLastTime = GetTickCount();
#endif // DEBUG

    DWORD currentFRNUM = Read_FRINDEX().bit.FrameIndex;
    DWORD dwCarryBit = m_FrameListMask + 1;
    if ((currentFRNUM & dwCarryBit ) != (m_frameCounterHighPart & dwCarryBit ) ) { // Overflow
        m_frameCounterHighPart += dwCarryBit;
    }
    m_frameCounterLowPart = currentFRNUM;

    LeaveCriticalSection( &m_csFrameCounter );
}

// ******************************************************************
BOOL CHW::GetFrameNumber( OUT LPDWORD lpdwFrameNumber )
//
// Purpose: Return the current frame number
//
// Parameters: None
//
// Returns: 32 bit current frame number
//
// Notes: See also comment in UpdateFrameCounter
// ******************************************************************
{
    EnterCriticalSection( &m_csFrameCounter );

    // This algorithm is right out of the Win98 uhcd.c code
    UpdateFrameCounter();
    DWORD frame = m_frameCounterHighPart + (m_frameCounterLowPart & m_FrameListMask);

    LeaveCriticalSection( &m_csFrameCounter );

    *lpdwFrameNumber=frame;
    return TRUE;
}
// ******************************************************************
BOOL CHW::GetFrameLength( OUT LPUSHORT lpuFrameLength )
//
// Purpose: Return the current frame length in 12 MHz clocks
//          (i.e. 12000 = 1ms)
//
// Parameters: None
//
// Returns: frame length
//
// Notes: Only part of the frame length is stored in the hardware
//        register, so an offset needs to be added.
// ******************************************************************
{
    *lpuFrameLength=60000;
    return TRUE;
}
// ******************************************************************
BOOL CHW::SetFrameLength( IN HANDLE , IN USHORT  )
//
// Purpose: Set the Frame Length in 12 Mhz clocks. i.e. 12000 = 1ms
//
// Parameters:  hEvent - event to set when frame has reached required
//                       length
//
//              uFrameLength - new frame length
//
// Returns: TRUE if frame length changed, else FALSE
//
// Notes:
// ******************************************************************
{
    return FALSE;
}
// ******************************************************************
BOOL CHW::StopAdjustingFrame( void )
//
// Purpose: Stop modifying the host controller frame length
//
// Parameters: None
//
// Returns: TRUE
//
// Notes:
// ******************************************************************
{
    return FALSE;
}
// ******************************************************************
BOOL CHW::DidPortStatusChange( IN const UCHAR port )
//
// Purpose: Determine whether the status of root hub port # "port" changed
//
// Parameters: port - 1 for the hub itself, otherwise the hub port number
//
// Returns: TRUE if status changed, else FALSE
//
// Notes:
// ******************************************************************
{
    // port == specifies root hub itself, whose status never changes
    if ( port > 0 ) {
        PORTSC portSC=Read_PORTSC(port);
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("DidPortStatusChange return 0x%x\r\n"), portSC.ul));
        // We have to determine routine the device here if there is new attached device.
        if (portSC.bit.Power && portSC.bit.Owner==0) {
            if (portSC.bit.ConnectStatus == 0 ) { // If device not present now. just return status changes.
                return ((GetForceReAttach() != 0)|| portSC.bit.ConnectStatusChange!=0 || portSC.bit.EnableChange != 0 || portSC.bit.OverCurrentChange!=0);
            }
            else
            if ((portSC.bit.ConnectStatusChange) || (GetForceReAttach() != 0)) {
                if (GetForceReAttach() != 0)
                    return TRUE;

                // This success if there is HighSpeed device. Otherwise it will route to companiou chip
                if (ResetAndEnablePort(port)) {
                    DisablePort(port);
                    return TRUE;
                }
                else
                {
                    // For Sony Cyber Camera DSC-P73 for some reasons in PTP mode, the PORTSC
                    // value change during ResetAndEnablePort, as such, read PORTSC one more time
                    // read the connection status
                    portSC = Read_PORTSC(port);
                    if (portSC.bit.ConnectStatus == 0)
                        return (portSC.bit.ConnectStatusChange!=0 || portSC.bit.EnableChange != 0 || portSC.bit.OverCurrentChange!=0);
                    else
                        return FALSE;
                }
            }
            else
                return (portSC.bit.EnableChange != 0 || portSC.bit.OverCurrentChange!=0 || portSC.bit.ForcePortResume!=0) ;
        }
/* This is done by hardware.
        else
        if (portSC.bit.ConnectStatus==0) { // There is no device . Set default route to this HC
            portSC.bit.Power=1;
            portSC.bit.Owner=0;
            // Do not touch write to clean register
            portSC.bit.ConnectStatusChange=0;
            portSC.bit.EnableChange=0;
            portSC.bit.OverCurrentChange=0;
            Write_PORTSC(port, portSC);
        }
*/
    }
    return FALSE;
}
// ******************************************************************
BOOL CHW::GetPortStatus( IN const UCHAR port,
                         OUT USB_HUB_AND_PORT_STATUS& rStatus,
                         OUT PDWORD pdwForceDetach)
//
// Purpose: This function will return the current root hub port
//          status in a non-hardware specific format
//
// Parameters: port - 1 for the hub itself, otherwise the hub port number
//
//             rStatus - reference to USB_HUB_AND_PORT_STATUS to get the
//                       status
//
// Returns: TRUE
//
// Notes:
// ******************************************************************
{
    memset( &rStatus, 0, sizeof( USB_HUB_AND_PORT_STATUS ) );
    if ( port > 0 ) {
        // request refers to a root hub port

        // read the port status register
        PORTSC portSC = Read_PORTSC( port );
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("GetPortStatus = 0x%x\r\n"), portSC.ul));

        if (pdwForceDetach)
        {
            if (GetForceReAttach()!= 0)
                *pdwForceDetach = 1;
            else
                *pdwForceDetach = 0;
        }

        if (portSC.bit.Power && portSC.bit.Owner==0) {
            // Now fill in the USB_HUB_AND_PORT_STATUS structure
            rStatus.change.port.ConnectStatusChange = portSC.bit.ConnectStatusChange;
            rStatus.change.port.PortEnableChange = portSC.bit.EnableChange;
            rStatus.change.port.OverCurrentChange = portSC.bit.OverCurrentChange;
            // for root hub, we don't set any of these change bits:
            DEBUGCHK( rStatus.change.port.SuspendChange == 0 );
            DEBUGCHK( rStatus.change.port.ResetChange == 0 );
                        // 00 ¨C Full Speed
                        // 01 ¨C Low Speed
                        // 10 ¨C High Speed
            rStatus.status.port.DeviceIsLowSpeed = ( 0x01 == portSC.bit.PSPD );
            rStatus.status.port.DeviceIsHighSpeed = ( 0x02 == portSC.bit.PSPD );
            rStatus.status.port.PortConnected = portSC.bit.ConnectStatus;
            rStatus.status.port.PortEnabled =  portSC.bit.Enabled;
            rStatus.status.port.PortOverCurrent = portSC.bit.OverCurrentActive ;
            // root hub ports are always powered
            rStatus.status.port.PortPower = 1;
            rStatus.status.port.PortReset = portSC.bit.Reset;
            rStatus.status.port.PortSuspended =  portSC.bit.Suspend;
            if (portSC.bit.ForcePortResume) { // Auto Resume Status special code.

                //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("GetPortStatus:ForcePortResume\r\n")));
                rStatus.change.port.SuspendChange=1;
                rStatus.status.port.PortSuspended=0;

                portSC.bit.ConnectStatusChange=0;
                portSC.bit.EnableChange=0;
                portSC.bit.OverCurrentChange=0;

                portSC.bit.ForcePortResume =0;
                portSC.bit.Suspend=0;
                Write_PORTSC(port,portSC);
            }
        }
    }
#ifdef DEBUG
    else {
        // request is to Hub. rStatus was already memset to 0 above.
        DEBUGCHK( port == 0 );
        // local power supply good
        DEBUGCHK( rStatus.status.hub.LocalPowerStatus == 0 );
        // no over current condition
        DEBUGCHK( rStatus.status.hub.OverCurrentIndicator == 0 );
        // no change in power supply status
        DEBUGCHK( rStatus.change.hub.LocalPowerChange == 0 );
        // no change in over current status
        DEBUGCHK( rStatus.change.hub.OverCurrentIndicatorChange == 0 );
    }
#endif // DEBUG

    return TRUE;
}

// ******************************************************************
BOOL CHW::RootHubFeature( IN const UCHAR port,
                          IN const UCHAR setOrClearFeature,
                          IN const USHORT feature )
//
// Purpose: This function clears all the status change bits associated with
//          the specified root hub port.
//
// Parameters: port - 0 for the hub itself, otherwise the hub port number
//
// Returns: TRUE iff the requested operation is valid, FALSE otherwise.
//
// Notes: Assume that caller has already verified the parameters from a USB
//        perspective. The HC hardware may only support a subset of that
//        (which is indeed the case for UHCI).
// ******************************************************************
{
    if (port == 0) {
        // request is to Hub but...
        // uhci has no way to tweak features for the root hub.
        return FALSE;
    }

    // mask the change bits because we write 1 to them to clear them //
    PORTSC portSC= Read_PORTSC( port );
    if ( portSC.bit.Power && portSC.bit.Owner==0) {
        portSC.bit.ConnectStatusChange=0;
        portSC.bit.EnableChange=0;
        portSC.bit.OverCurrentChange=0;
        if (setOrClearFeature == USB_REQUEST_SET_FEATURE)
            switch (feature) {
              case USB_HUB_FEATURE_PORT_RESET:              portSC.bit.Reset=1;break;
              case USB_HUB_FEATURE_PORT_SUSPEND:            portSC.bit.Suspend=1; break;
              case USB_HUB_FEATURE_PORT_POWER:              portSC.bit.Power=1;break;
              default: return FALSE;
            }
        else
            switch (feature) {
              case USB_HUB_FEATURE_PORT_ENABLE:             portSC.bit.Enabled=0; break;
              case USB_HUB_FEATURE_PORT_SUSPEND:            // EHCI 2.3.9
                if (portSC.bit.Suspend !=0 ) {
                    portSC.bit.ForcePortResume=1;
                    Write_PORTSC( port, portSC );
                    Sleep(20);
                    portSC.bit.ForcePortResume=0;
                }
                break;
              case USB_HUB_FEATURE_C_PORT_CONNECTION:       portSC.bit.ConnectStatusChange=1;break;
              case USB_HUB_FEATURE_C_PORT_ENABLE:           portSC.bit.EnableChange=1; break;
              case USB_HUB_FEATURE_C_PORT_RESET:
              case USB_HUB_FEATURE_C_PORT_SUSPEND:
              case USB_HUB_FEATURE_C_PORT_OVER_CURRENT:
              case USB_HUB_FEATURE_PORT_POWER:
              default: return FALSE;
            }

        Write_PORTSC( port, portSC );
        return TRUE;
    }
    else
        return FALSE;
}


// ******************************************************************
BOOL CHW::ResetAndEnablePort( IN const UCHAR port)
//
// Purpose: reset/enable device on the given port so that when this
//          function completes, the device is listening on address 0
//
// Parameters: port - root hub port # to reset/enable
//             timeout - if timeout is allowed, since it is reused in wakeup sequence
//
// Returns: TRUE if port reset and enabled, else FALSE
//
// Notes: This function takes approx 60 ms to complete, and assumes
//        that the caller is handling any critical section issues
//        so that two different ports (i.e. root hub or otherwise)
//        are not reset at the same time. please refer 4.2.2 for detail
// Remark: NOT ALLOW to use in Power Management PowerUp/PowerDown routine
// ******************************************************************
{
    BOOL fSuccess = FALSE;
    int i;

    PORTSC portSC=Read_PORTSC(port);
    // no point reset/enabling the port unless something is attached
    //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("ResetAndEnablePort PORTSC=0x%x\r\n"), portSC.ul));
    // There is a problem in which even the ConnectStatusChange bit is set, the
    // ConnectStatus is still "connected" in the detach case, need to wait for 10ms to
    // make sure that.
    if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.ConnectStatusChange)
    {
        Sleep(100);  // sleep for 100ms and make sure the connect status change completed
        portSC = Read_PORTSC(port);
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("ResetAndEnablePort again PORTSC=0x%x\r\n"), portSC.ul));
    }

    if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.ConnectStatus ) {
        // Do not touch Write to Clear Bit.
        portSC.bit.ConnectStatusChange=0;
        portSC.bit.EnableChange=0;
        portSC.bit.OverCurrentChange=0;

        // turn on reset bit
        portSC.bit.Reset =1 ;
        portSC.bit.Enabled=0;
        Write_PORTSC( port, portSC );
        // Note - Win98 waited 10 ms here. But, the new USB 1.1 spec
        // section 7.1.7.3 recommends 50ms for root hub ports
// Specific for iMx31 Freescale, no set to 1 is required
#if 0
        Sleep( 50 );

        // Clear the reset bit
        portSC.bit.Reset =0 ;
        Write_PORTSC( port, portSC );
        for (DWORD dwIndex=0; dwIndex<10 && Read_PORTSC(port).bit.Reset!=0 ; dwIndex++)
            Sleep(10);
#else
        // Wait for port reset completed
        while (Read_PORTSC(port).bit.Reset != 0);
        // If reset cannot set the port enable... reset again
        i = 0;
        Sleep(50);

        portSC = Read_PORTSC(port);
        while ((portSC.bit.Enabled == 0) && (i < 2000))
        {        // turn on reset bit
          //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Reset again?\r\n")));
            if (portSC.bit.ConnectStatus == 0)
              return FALSE;

            portSC.bit.Reset =1 ;
            portSC.bit.Enabled=0;
            Write_PORTSC( port, portSC );
            while (Read_PORTSC(port).bit.Reset != 0);
            i++;
            Sleep(50);

            portSC = Read_PORTSC(port);
        }

#endif
        portSC = Read_PORTSC( port );
        if ( portSC.bit.Enabled && portSC.bit.Reset == 0 ) {
            // port is enabled
            fSuccess = TRUE;
        }
        //
        // clear port connect & enable change bits
        //
        if (fSuccess) {
            portSC.bit.ConnectStatusChange=0; // Do not clean ConnectStatusChange.
            portSC.bit.EnableChange=1;
        }
        else  // Turn Off the OwnerShip. EHCI 4.2.2
            portSC.bit.Owner=1;
        Write_PORTSC( port, portSC );

        // USB 1.1 spec, 7.1.7.3 - device may take up to 10 ms
        // to recover after reset is removed
        Sleep( 10 );
    }

    DEBUGMSG( ZONE_REGISTERS, (TEXT("Root hub, after reset & enable, port %d portsc = 0x%04x\n"), port, Read_PORTSC( port ) ) );
    return fSuccess;

}
// ******************************************************************
void CHW::DisablePort( IN const UCHAR port )
//
// Purpose: disable the given root hub port
//
// Parameters: port - port # to disable
//
// Returns: nothing
//
// Notes: This function will take about 10ms to complete
// ******************************************************************
{
    PORTSC portSC=Read_PORTSC(port);;
    // no point doing any work unless the port is enabled
    if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.Enabled ) {
        // clear port enabled bit and enabled change bit,
        // but don't alter the connect status change bit,
        // which is write-clear.
        portSC.bit.Enabled=0;
        portSC.bit.ConnectStatusChange=0;
        portSC.bit.EnableChange=1;
        portSC.bit.OverCurrentChange=0;
        Write_PORTSC( port, portSC );

        // disable port can take some time to act, because
        // a USB request may have been in progress on the port.
        Sleep( 10 );
    }
}
BOOL CHW::WaitForPortStatusChange (HANDLE m_hHubChanged)
{
    if (m_hUsbHubChangeEvent) {
        if (m_hHubChanged!=NULL) {
            HANDLE hArray[2];
            hArray[0]=m_hHubChanged;
            hArray[1]=m_hUsbHubChangeEvent;
            WaitForMultipleObjects(2,hArray,FALSE,INFINITE);
        }
        else
            WaitForSingleObject(m_hUsbHubChangeEvent,INFINITE);
        return TRUE;
    }
    return FALSE;
}

// ******************************************************************
VOID CHW::PowerMgmtCallback( IN BOOL fOff )
//
// Purpose: System power handler - called when device goes into/out of
//          suspend.
//
// Parameters:  fOff - if TRUE indicates that we're entering suspend,
//                     else signifies resume
//
// Returns: Nothing
//
// Notes: This needs to be implemented for HCDI
// ******************************************************************
{
    // Don't process anything if it is not in Host mode
    if ((!bInHost) && (m_dwOTGSupport))
    {
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("PowerMgmt with no action\r\n")));
        return;
    }

    if ( fOff )
    {
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("PowerMgmt -> SuspendResume support (%d)\r\n"), (GetCapability() & HCD_SUSPEND_RESUME)));
        m_bDoResume = FALSE;

        // If it is in suspend mode already, why bother to process it.
        if (m_bUSBIdleSuspend)
            return;

        // We would need to make sure the USB clock and panic mode both ON before
        // we proceed with the power down sequence.
        if (m_bUSBClockStop)
        {
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USB Clock start\r\n")));
            m_bUSBClockStop = FALSE;
            StartUSBClock();
        }

        if(!((SEHCDPdd*)m_pPddContext)->bIsMX31TO2)
        {
            if (m_bUSBPanicMode == FALSE)
            {
                m_bUSBPanicMode = TRUE;
                DDKClockEnablePanicMode();
            }
        }

        for (UINT port =1; port <= m_NumOfPort; port ++)
        {
            PORTSC portSC=Read_PORTSC(port);;
            if ( portSC.bit.PHCD == 1 ) 
            {               
                portSC.bit.PHCD=0;
                if (portSC.bit.Suspend)
                    portSC.bit.ForcePortResume = 1;
                Write_PORTSC( port, portSC );
            }
        }
                
        BSPUsbCheckWakeUp();
        CHW::StopHostController();          
        SetPHYPowerMgmt(TRUE);

        //DumpUSBHostRegs();

        // Now we need to turn off the phy and stop the USB CLock. Also, let DVFS handle the voltage
        // Just trying to put PHY to sleep first.       
        for (UINT port =1; port <= m_NumOfPort; port ++)
        {
            PORTSC portSC=Read_PORTSC(port);
            if (portSC.bit.Suspend == 0)
            {
                portSC.bit.Suspend = 1;
                Write_PORTSC(port, portSC);
            }
        }

        for (UINT port =1; port <= m_NumOfPort; port ++)
        {
            PORTSC portSC=Read_PORTSC(port);
            if ( portSC.bit.PHCD == 0 ) {
                    portSC.bit.PHCD=1;
                    Write_PORTSC( port, portSC );
                    //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Port %d PHY to suspend portsc (0x%x)\r\n"), port, portSC.ul));
            }
        }

        BSPUsbSetWakeUp(TRUE);

        if(!((SEHCDPdd*)m_pPddContext)->bIsMX31TO2)
        {
        // now we need to switch to non-panic mode
            if (m_bUSBPanicMode)
            {
                //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Panic Mode off\r\n")));
                m_bUSBPanicMode = FALSE;
                DDKClockDisablePanicMode();
            }
       }

        // Stop USB Clock now
        if (m_bUSBClockStop == FALSE)
        {
            //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USB Clock stop\r\n")));
            m_bUSBClockStop = TRUE;
            StopUSBClock();
        }
    }
    else
    {   // resuming...
        DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Resume the port\r\n")));

        if (m_bUSBClockStop)
        {
            DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("USB Clock start\r\n")));
            m_bUSBClockStop = FALSE;
            StartUSBClock();
        }

        if(!((SEHCDPdd*)m_pPddContext)->bIsMX31TO2)
        {
            if (m_bUSBPanicMode == FALSE)
            {
                m_bUSBPanicMode = TRUE;
                DDKClockEnablePanicMode();
            }
        }

        for (UINT port =1; port <= m_NumOfPort; port ++)
        {
            PORTSC portSC=Read_PORTSC(port);;
            if ( portSC.bit.PHCD == 1 )
            {
                portSC.bit.PHCD=0;
                if (portSC.bit.Suspend)
                    portSC.bit.ForcePortResume = 1;
                Write_PORTSC( port, portSC );
            }
        }


        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("PowerMgmtCallback up\r\n")));
        //SetPHYPowerMgmt(FALSE);
        BSPUsbCheckWakeUp();
        SetForceReAttach(1);
        if (!m_dwOTGSupport)
            SetPHYPowerMgmt(FALSE);
        CHW::StartHostController();
        //BSPUsbCheckWakeUp();
        //g_fResumeOTG = TRUE;
        SetInterruptEvent(m_dwSysIntr);
    }

    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("-PowerMgmtCallBack()\r\n")));
    return;
}

//******************************************************************
VOID CHW::SuspendHostController()
//
// Purpose: This is to suspend the host controller and put into suspend mode
//
// Parameters: Nothing
//
// Returns: Nothing.
//
// Notes:
//
//********************************************************************
{
    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("+SuspendHostController\r\n")));
    if ( m_portBase != 0 ) {
        // initialize interrupt register - set only RESUME interrupts to enabled
        USBCMD usbcmd=Read_USBCMD();
        usbcmd.bit.RunStop=0;
        Write_USBCMD(usbcmd);
        // EHCI do not have group suspend. But. We can suspend each port.
        for (UINT port =1; port <= m_NumOfPort; port ++) {
            PORTSC portSC=Read_PORTSC(port);;
            // no point doing any work unless the port is enabled
            if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.Enabled ) {
                portSC.bit.ConnectStatusChange=0;
                portSC.bit.EnableChange=0;
                portSC.bit.OverCurrentChange=0;        
                //
                portSC.bit.ForcePortResume =0;
                portSC.bit.Suspend=1;
                portSC.bit.WakeOnConnect = 1;
                portSC.bit.WakeOnDisconnect =0;
                portSC.bit.WakeOnOverCurrent =1;
                Write_PORTSC( port, portSC );
            }
        }

    }
    DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("-SuspendHostController\r\n")));
}

//***************************************************************
VOID CHW::ResumeHostController()
//
// Purpose: This is to resume the host controller from suspend
//
// Parameters: Nothing            
//
// Returns: Nothing.
//
// Notes: This function is obselete and not used by Freescale project
//
//********************************************************************
{
    if ( m_portBase != 0 ) {
        // I need 20 ms delay here 30(30ns)*1000*20
        for (UINT port =1; port <= m_NumOfPort; port ++) {
            PORTSC portSC=Read_PORTSC(port);;
            // no point doing any work unless the port is enabled
            if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.Enabled ) {
                portSC.bit.ConnectStatusChange=0;
                portSC.bit.EnableChange=0;
                portSC.bit.OverCurrentChange=0;        
                //
                portSC.bit.ForcePortResume =1;
                portSC.bit.Suspend=0;
                Write_PORTSC( port, portSC );
            }
        }
        for (DWORD dwIndex =0; dwIndex<30*1000*20; dwIndex++)
            Read_USBCMD();
        for (port =1; port <= m_NumOfPort; port ++) {
            PORTSC portSC=Read_PORTSC(port);;
            // no point doing any work unless the port is enabled
            if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.Enabled ) {
                portSC.bit.ConnectStatusChange=0;
                portSC.bit.EnableChange=0;
                portSC.bit.OverCurrentChange=0;        
                //
                portSC.bit.ForcePortResume =0;
                portSC.bit.Suspend=0;
                Write_PORTSC( port, portSC );
            }
        }
        USBCMD usbcmd=Read_USBCMD();
        usbcmd.bit.RunStop=1;
        Write_USBCMD(usbcmd);
    }
    //ResumeNotification();

}

//************************************************************************
DWORD CHW::SetCapability(DWORD dwCap)
//
// Purpose: This function is to check if suspend_resume is enabled, we would
//          support the wakeup enable. 
//
// Parameters: Nothing
//
// Returns: Nothing.
//
// Notes: This function is obsolete & not used in Freescale project as we would
//        need to permanent support in non-OTG support case
//
//********************************************************************
{
    m_dwCapability |= dwCap; 
    if ( (m_dwCapability & HCD_SUSPEND_RESUME)!=0) {
        //KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &m_dwSysIntr, sizeof(m_dwSysIntr), NULL, 0, NULL);
    }
    return m_dwCapability;
};

//*************************************************************************
USHORT CHW::GetPortSpeed(IN const USHORT port)
//
// Purpose: Return the speed of the port through PORTSC register
//
// Parameters: port - port to be read
//
// Returns: Speed : 0 - Full Speed
//                  1 - Low Speed
//                  2 - High Speed
//
// Notes:
//
//********************************************************************
{
    PORTSC portSC = Read_PORTSC( port );
    //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Port speed = 0x%x\r\n"), portSC.bit.PSPD));
    return (portSC.bit.PSPD);

}

//***********************************************************
BOOL CHW::IsOTGHostDevice(void)
//
// Purpose: Return if it is host device.  This is used in OTG cases
//
// Parameters: Nothing
//
// Returns: TRUE - A-device (Host mode operating)
//          FALSE - B-device (Client mode operating)
//
// Notes:
//
//********************************************************************
{
    OTGSC otgsc;
    if (m_dwOTGSupport)
    {
        otgsc= Read_OTGSC();
        //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("ID Pin = 0x%x\r\n"), otgsc.bit.ID));
        return (otgsc.bit.ID? FALSE: TRUE);
    }

    return TRUE;
}

//******************************************************************
BOOL CHW::StopUSBClock(void)
//
// Purpose: Stop the USB Core Clock
//
// Parameters: Nothing
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
//
//********************************************************************
{
    BOOL rc = FALSE;

    rc = BSPUSBClockDisable(TRUE);

    return rc;
}

//********************************************************************
BOOL CHW::StartUSBClock(void)
//
// Purpose: Start the USB Core Clock
//
// Parameters: Nothing
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
//
//********************************************************************
{
    BOOL rc = FALSE;

    //DEBUGMSG(ZONE_INIT && ZONE_VERBOSE, (TEXT("Start the USB clock\r\n")));
    rc = BSPUSBClockDisable(FALSE);
    return rc;
}

//********************************************************************
void CHW::WakeUpSysIntr(void)
//
// Purpose: Manually wakeup the sysintr and enable the interrupt routine
//
// Parameters: Nothing
//
// Returns: Nothing
//
// Notes:
//
//********************************************************************
{
    SetInterruptEvent(m_dwSysIntr);
}

#ifdef DEBUG
// ******************************************************************
void CHW::DumpUSBCMD( void )
//
// Purpose: Queries Host Controller for contents of USBCMD, and prints
//          them to DEBUG output. Bit definitions are in UHCI spec 2.1.1
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: used in DEBUG mode only
//
//        This function is static
// ******************************************************************
{
    __try {
         USBCMD usbcmd=Read_USBCMD();

        DEBUGMSG(ZONE_REGISTERS, (TEXT("\tCHW - USB COMMAND REGISTER (USBCMD) = 0x%X. Dump:\n"), usbcmd.ul));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tHost Controller Reset = %s\n"), (usbcmd.bit.HCReset ? TEXT("Set") : TEXT("Not Set"))));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tRun/Stop = %s\n"), ( usbcmd.bit.RunStop ? TEXT("Run") : TEXT("Stop"))));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW - FAILED WHILE DUMPING USBCMD!!!\n")));
    }
}
// ******************************************************************
void CHW::DumpUSBSTS( void )
//
// Purpose: Queries Host Controller for contents of USBSTS, and prints
//          them to DEBUG output. Bit definitions are in UHCI spec 2.1.2
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: used in DEBUG mode only
//
//        This function is static
// ******************************************************************
{
    __try {
        USBSTS usbsts = Read_USBSTS();
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\tCHW - USB STATUS REGISTER (USBSTS) = 0x%X. Dump:\n"), usbsts.ul));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tHCHalted = %s\n"), ( usbsts.bit.HCHalted ? TEXT("Halted") : TEXT("Not Halted"))));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tHost System Error = %s\n"), (usbsts.bit.HSError ? TEXT("Set") : TEXT("Not Set"))));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tUSB Error Interrupt = %s\n"), (usbsts.bit.USBERRINT ? TEXT("Set") : TEXT("Not Set"))));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tUSB Interrupt = %s\n"), (usbsts.bit.USBINT ? TEXT("Set") : TEXT("Not Set"))));

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW - FAILED WHILE DUMPING USBSTS!!!\n")));
    }
}
// ******************************************************************
void CHW::DumpUSBINTR( void )
//
// Purpose: Queries Host Controller for contents of USBINTR, and prints
//          them to DEBUG output. Bit definitions are in UHCI spec 2.1.3
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: used in DEBUG mode only
//
//        This function is static
// ******************************************************************
{
    __try {
        USBINTR usbintr = Read_USBINTR();
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\tCHW - USB INTERRUPT REGISTER (USBINTR) = 0x%X. Dump:\n"), usbintr.ul));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW - FAILED WHILE DUMPING USBINTR!!!\n")));
    }
}
// ******************************************************************
void CHW::DumpFRNUM( void )
//
// Purpose: Queries Host Controller for contents of FRNUM, and prints
//          them to DEBUG output. Bit definitions are in UHCI spec 2.1.4
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: used in DEBUG mode only
//
//        This function is static
// ******************************************************************
{
    __try {
        FRINDEX frindex = Read_FRINDEX();
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\tCHW - FRAME NUMBER REGISTER (FRNUM) = 0x%X. Dump:\n"), frindex.ul));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tMicroFrame number (bits 2:0) = %d\n"), frindex.bit.microFlame));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tFrame index (bits 11:3) = %d\n"), frindex.bit.FrameIndex));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW - FAILED WHILE DUMPING FRNUM!!!\n")));
    }
}

// ******************************************************************
void CHW::DumpFLBASEADD( void )
//
// Purpose: Queries Host Controller for contents of FLBASEADD, and prints
//          them to DEBUG output. Bit definitions are in UHCI spec 2.1.5
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: used in DEBUG mode only
//
//        This function is static
// ******************************************************************
{
    DWORD    dwData = 0;

    __try {
        dwData = Read_EHCIRegister( PERIODICLISTBASE );
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\tCHW - FRAME LIST BASE ADDRESS REGISTER (FLBASEADD) = 0x%X. Dump:\n"), dwData));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tFLBASEADD address base (bits 11:0 masked) = 0x%X\n"), (dwData & EHCD_FLBASEADD_MASK)));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW - FAILED WHILE DUMPING FLBASEADD!!!\n")));
    }
}
// ******************************************************************
void CHW::DumpSOFMOD( void )
//
// Purpose: Queries Host Controller for contents of SOFMOD, and prints
//          them to DEBUG output. Bit definitions are in UHCI spec 2.1.6
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: used in DEBUG mode only
//
//        This function is static
// ******************************************************************
{
    __try {
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\tCHW - ASYNCLISTADDR = 0x%X. Dump:\n"),Read_EHCIRegister( ASYNCLISTADDR)));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW CONFIGFLAG = %x\n"), Read_EHCIRegister(CONFIGFLAG)));
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW CTLDSSEGMENT = %x\n"), Read_EHCIRegister(CTLDSSEGMENT)));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW - FAILED WHILE DUMPING SOFMOD!!!\n")));
    }
}

// ******************************************************************
void CHW::DumpPORTSC(IN const USHORT port)
//
// Purpose: Queries Host Controller for contents of PORTSC #port, and prints
//          them to DEBUG output. Bit definitions are in UHCI spec 2.1.7
//
// Parameters: port - the port number to read. It must be such that
//                    1 <= port <= UHCD_NUM_ROOT_HUB_PORTS
//
// Returns: Nothing
//
// Notes: used in DEBUG mode only
//
//        This function is static
// ******************************************************************
{
    DWORD    dwData = 0;

    __try {
        DEBUGCHK( port >=  1 && port <=  m_NumOfPort );
        if (port >=  1 && port <=  m_NumOfPort ) {
            PORTSC portSC = Read_PORTSC( port );
            DEBUGMSG(ZONE_REGISTERS, (TEXT("\tCHW - PORT STATUS AND CONTROL REGISTER (PORTSC%d) = 0x%X. Dump:\n"), port, dwData));
            if ( portSC.bit.Power && portSC.bit.Owner==0 && portSC.bit.Enabled ) {
                DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tHub State = %s\n"), (portSC.bit.Suspend ? TEXT("Suspend") : TEXT("Enable"))));
                DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tPort Reset = %s\n"), ( portSC.bit.Reset ? TEXT("Reset") : TEXT("Not Reset"))));
                DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tResume Detect = %s\n"), (portSC.bit.ForcePortResume ? TEXT("Set") : TEXT("Not Set"))));
                DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tLine Status = %d\n"), ( portSC.bit.LineStatus )));
                DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tPort Enable/Disable Change  = %s\n"), ( portSC.bit.EnableChange ? TEXT("Set") : TEXT("Not Set"))));
                DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tConnect Status Change = %s\n"), (portSC.bit.ConnectStatusChange? TEXT("Set") : TEXT("Not Set"))));
                DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tConnect Status = %s\n"), (portSC.bit.ConnectStatus ? TEXT("Device Present") : TEXT("No Device Present"))));
            } else {
                DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tHub State this port Disabled\n")));
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_REGISTERS, (TEXT("\t\tCHW - FAILED WHILE DUMPING PORTSC%d!!!\n"), port));
    }
}

// ******************************************************************
void CHW::DumpAllRegisters( void )
//
// Purpose: Queries Host Controller for all registers, and prints
//          them to DEBUG output. Register definitions are in UHCI spec 2.1
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: used in DEBUG mode only
//
//        This function is static
// ******************************************************************
{
    DEBUGMSG(ZONE_REGISTERS, (TEXT("CHW - DUMP REGISTERS BEGIN\n")));
    DumpUSBCMD();
    DumpUSBSTS();
    DumpUSBINTR();
    DumpFRNUM();
    DumpFLBASEADD();
    DumpSOFMOD();
    for ( USHORT port = 1; port <=  m_NumOfPort; port++ ) {
        DumpPORTSC( port );
    }
    DEBUGMSG(ZONE_REGISTERS, (TEXT("CHW - DUMP REGISTERS DONE\n")));
}
#endif
