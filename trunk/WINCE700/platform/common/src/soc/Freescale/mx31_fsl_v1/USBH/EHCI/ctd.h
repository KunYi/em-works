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
//     CTd.h
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//



/*---------------------------------------------------------------------------
* Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/


#ifndef __CTD_H_
#define __CTD_H_

#include "td.h"
#include <cphysmem.hpp>
#define MAX_PHYSICAL_BLOCK 7
#define MAX_TRNAS_PER_ITD 8

#define TD_SETUP_PID 0x2d
#define TD_IN_PID 0x69
#define TD_OUT_PID 0xe1

#define TD_OUT_TOKEN   0
#define TD_IN_TOKEN    1
#define TD_SETUP_TOKEN 2

#define CRITICAL_LOG_H 0
// Remove-W4: Warning C4512 workaround
#pragma warning(push)
#pragma warning(disable: 4512)

class CNextLinkPointer  {
protected:
    volatile NextLinkPointer nextLinkPointer;
public:
    void * operator new(size_t stSize, CPhysMem * const pCPhysMem);
    void operator delete (void *pointer);
    CNextLinkPointer() {  nextLinkPointer.dwLinkPointer=1;}; // Invalid Zero Pointer.
    DWORD   SetDWORD(DWORD dwValue) { 
        DWORD dwReturn= nextLinkPointer.dwLinkPointer;
         nextLinkPointer.dwLinkPointer=dwValue;
        return dwReturn;
    }
    DWORD   GetDWORD() { return  nextLinkPointer.dwLinkPointer ;};
    
    BOOL    SetLinkValid(BOOL bTrue) {
        BOOL bReturn=( nextLinkPointer.lpContext.Terminate==0);
         nextLinkPointer.lpContext.Terminate=(bTrue?0:1);
        return bReturn;
    }
    BOOL    GetLinkValid() { return ( nextLinkPointer.lpContext.Terminate==0); };
    
    NEXTLINKTYPE   SetLinkType(NEXTLINKTYPE nLinkType) {
        NEXTLINKTYPE nReturn=(NEXTLINKTYPE) nextLinkPointer.lpContext.TypeSelect;
         nextLinkPointer.lpContext.TypeSelect=(DWORD)nLinkType;
        return nReturn;
    }
    NEXTLINKTYPE   GetLinkType() { return ((NEXTLINKTYPE)nextLinkPointer.lpContext.TypeSelect); };
    
    DWORD   SetPointer(DWORD dwPointer) {
        DWORD dwReturn= ( nextLinkPointer.lpContext.LinkPointer <<5);
        ASSERT((dwPointer & 0x1f) == 0 ); // Alignment check.
        nextLinkPointer.lpContext.LinkPointer = (dwPointer>>5);
        return dwReturn;
    }
    DWORD   GetPointer() {  return ( nextLinkPointer.lpContext.LinkPointer <<5); };

    BOOL SetNextPointer(DWORD dwPhysAddr,NEXTLINKTYPE nLinkType,BOOL bValid) {
        CNextLinkPointer cNext;
        cNext.SetLinkValid(bValid);
        cNext.SetLinkType(nLinkType);
        cNext.SetPointer(dwPhysAddr);
        nextLinkPointer.dwLinkPointer = cNext.GetDWORD();
        return TRUE;
    }
    CNextLinkPointer * GetNextLinkPointer(CPhysMem *pPhysMem) {
        ASSERT(pPhysMem);
        if (GetLinkValid() && pPhysMem ) {
            return (CNextLinkPointer * )pPhysMem->PaToVa(GetPointer());
        }
        else 
            return NULL;
    }
};

class CITransfer;
class CITD;
#define CITD_CHECK_FLAG_VALUE 0xc3a5f101
class CITD: public CNextLinkPointer, protected ITD{
public:
    CITD(CITransfer * pIsochTransfer);
    void ReInit(CITransfer * pIsochTransfer);
    ~CITD(){CheckStructure ();};
    DWORD IssueTransfer(DWORD dwNumOfTrans,PDWORD pdwTransLenArray, PDWORD pdwFrameAddrArray,BOOL bIoc,BOOL bIn);
    BOOL ActiveTrasfer() {CheckStructure ();
        for (DWORD dwCount =0; dwCount < MAX_PHYSICAL_BLOCK; dwCount ++) {
            if ( iTD_BufferPagePointer[dwCount].dwITD_BufferPagePointer!=0)
                iTD_StatusControl[dwCount].iTD_SCContext.Active = 1;
        }
        return TRUE;
    }
    BOOL IsActive() { CheckStructure ();
        BOOL bReturn = FALSE;
        for (DWORD dwCount =0; dwCount < MAX_PHYSICAL_BLOCK; dwCount ++) {
            if ( iTD_BufferPagePointer[dwCount].dwITD_BufferPagePointer!=0 && iTD_StatusControl[dwCount].iTD_SCContext.Active == 1)
                bReturn = TRUE;
        }
        return bReturn;
    }
    void SetIOC(BOOL bSet);
    DWORD GetPhysAddr() { return m_dwPhys; };
    DWORD SetPhysAddr (DWORD dwPhys) {
        CheckStructure ();
        DWORD dwReturn=m_dwPhys;
        m_dwPhys=dwPhys;
        return dwReturn;
    }
    BOOL CheckStructure () {
        ASSERT(m_CheckFlag==CITD_CHECK_FLAG_VALUE);
        return (m_CheckFlag==CITD_CHECK_FLAG_VALUE);
    }
private:
    CITransfer * m_pTrans;
    DWORD m_dwPhys;
    const DWORD m_CheckFlag;
    friend class CITransfer;
};
class CSITransfer ;
class CSITD;
#define CSITD_CHECK_FLAG_VALUE 0xc3a5f102
class CSITD : public CNextLinkPointer, SITD {
public:
    CSITD(CSITransfer * pTransfer,CSITD * pPrev);
    void ReInit(CSITransfer * pTransfer,CSITD * pPrev);
    ~CSITD() {CheckStructure ();;};
    DWORD IssueTransfer(DWORD dwPhysAddr, DWORD dwEndPhysAddr, DWORD dwLen,BOOL bIoc,BOOL bIn);
    DWORD GetPhysAddr() { CheckStructure ();return m_dwPhys; };
    DWORD SetPhysAddr (DWORD dwPhys) {
        CheckStructure ();
        DWORD dwReturn=m_dwPhys;
        m_dwPhys=dwPhys;
        return dwReturn;
    }
    void SetIOC(BOOL bSet) { CheckStructure ();sITD_TransferState.sITD_TSContext.IOC = (bSet?1:0); };
    BOOL CheckStructure () {
        ASSERT(m_CheckFlag==CSITD_CHECK_FLAG_VALUE);
        return (m_CheckFlag==CSITD_CHECK_FLAG_VALUE);
    }
private:
    CSITransfer  * m_pTrans;
    CSITD * m_pPrev;
    DWORD m_dwPhys;
    const DWORD m_CheckFlag;
    friend class CSITransfer;
};

#define CQTD_CHECK_FLAG_VALUE 0xc3a5f103
class CQTransfer;
class CQH;
class CQTD;
class CQTD :  public CNextLinkPointer,QTD {
public:
    CQTD(CQTransfer * pTransfer, CQH * pQh);
    ~CQTD() {CheckStructure ();};
    DWORD IssueTransfer(DWORD dwPID, BOOL bToggle1, DWORD dwTransLength, PPhysBufferArray pPhysBufferArray,BOOL bIoc);
    DWORD GetPhysAddr() {         CheckStructure ();return m_dwPhys; };
    CQTD *GetNextTD() {         CheckStructure ();return m_pNext; };
    CQTD *QueueNextTD(CQTD * pNextTD) ;
#ifdef IRAM_PATCH
    void FlushNextTD();
    void ResetIRamBP(ULONG iRamStartAddr);
    BOOL IsOutTransfer() {
        CheckStructure();
        //RETAILMSG(1, (L"PID is %x\r\n", qTD_Token.qTD_TContext.PID));
        return (qTD_Token.qTD_TContext.PID != TD_IN_TOKEN?TRUE:FALSE);
    }
    PUCHAR StartOfBuffer() {
        CheckStructure();
        return m_pOrigStart;
    }
    DWORD BufLength() {
        CheckStructure();
        return m_dwQtdBufLength;
    }
    void SetPhysAddr(DWORD physAddr) {
        m_dwPhys = physAddr;
    }
    void Dump() {
        CheckStructure();
        RETAILMSG(1, (L"phy addr %x\r\n", m_dwPhys));
        RETAILMSG(1, (L"NL %x\r\n", GetDWORD()));
        RETAILMSG(1, (L"AN %x\r\n", altNextQTDPointer.dwLinkPointer));
        RETAILMSG(1, (L"QTD Token %x\r\n", qTD_Token.dwQTD_Token));
        RETAILMSG(1, (L"Bytes to Trans %d\r\n", qTD_Token.qTD_TContext.BytesToTransfer));
        RETAILMSG(1, (L"BP %x\r\n", qTD_BufferPointer[0].dwQTD_BufferPointer));
    }
    void Overlay(CQTD* clonedTD);
    DWORD CQTD::GetDwQTD_Token();
#endif
    void SetAltNextQTDPointer(DWORD dwPhysAddr) { 
        CheckStructure ();
        ASSERT((dwPhysAddr & 0x1f)== 0 ) ;// 32-byte alignment
        altNextQTDPointer.dwLinkPointer=dwPhysAddr;
    }
    CQTransfer * GetTransfer() {         CheckStructure ();return m_pTrans; };
    void CheckStructure () {
        ASSERT(m_CheckFlag==CQTD_CHECK_FLAG_VALUE);
    }
    void DeActiveTD() {
        CheckStructure ();
        qTD_Token.qTD_TContext.Active =0;// Deativate TD
        SetDWORD(1); // Physical Terminate Transfer Linker
    }
private:
#ifdef IRAM_PATCH
    PUCHAR m_pOrigStart;
    DWORD m_dwQtdBufLength;
#endif
    CQTransfer * const m_pTrans;
    CQH  * const m_pQh;
    CQTD * m_pNext;
    DWORD m_dwPhys;
    const DWORD m_CheckFlag;
    friend class CQTransfer;
#ifdef IRAM_PATCH
    friend class CQH;
#endif
};

class CPipe;
#define CQH_CHECK_FLAG_VALUE 0xc3a5f104
class CQH : public CNextLinkPointer, public QH {  /*BookMarks : class CQH*/
public :
#ifdef IRAM_PATCH
#ifdef IRAM_PATCH_EXTEND
    CQH(CPipe *pPipe, CPhysMem *pDRamMem, CPhysMem *pIRamEleMem, CPhysMem *pIRamDataMem, BOOL bInIRAM);
#else
    CQH(CPipe *pPipe, CPhysMem *pIRamEleMem, CPhysMem *pIRamDataMem, BOOL bInIRAM);
#endif
#else
    CQH(CPipe *pPipe);
#endif
    ~CQH() {;};
    BOOL IsActive() {
        CheckStructure ();
        return ((nextQTDPointer.lpContext.Terminate == 0 || qTD_Overlay.qTD_Token.qTD_TContext.Active ==1) && 
                qTD_Overlay.qTD_Token.qTD_TContext.Halted ==0 );
    }
    void SetDeviceAddress(DWORD dwDeviceAddress) { 
        CheckStructure ();
        qH_StaticEndptState.qH_SESContext.DeviceAddress = dwDeviceAddress;
    }
    void SetReclamationFlag(BOOL bSet) {  CheckStructure ();qH_StaticEndptState.qH_SESContext.H=(bSet?1:0);  }
    BOOL GetReclamationFlag() {CheckStructure ();return (qH_StaticEndptState.qH_SESContext.H==1); }

    void SetDTC(BOOL bSet) {CheckStructure (); qH_StaticEndptState.qH_SESContext.DTC = (bSet?1:0); };
    void SetControlEnpt(BOOL bSet) { CheckStructure ();qH_StaticEndptState.qH_SESContext.C = (bSet?1:0);};
    void SetSMask(UCHAR SMask) { CheckStructure ();qH_StaticEndptState.qH_SESContext.UFrameSMask=SMask; };
    void SetCMask(UCHAR CMask) { CheckStructure ();qH_StaticEndptState.qH_SESContext.UFrameCMask=CMask; };
    void SetMaxPacketLength (DWORD dwMaxPacketLength) {CheckStructure (); qH_StaticEndptState.qH_SESContext.MaxPacketLength = (dwMaxPacketLength & 0x7ff); };
    void SetReLoad(DWORD dwCount) { CheckStructure ();qH_StaticEndptState.qH_SESContext.RL = dwCount; };
    void SetINT(BOOL bTrue) {CheckStructure (); qH_StaticEndptState.qH_SESContext.I=(bTrue?1:0); };

    BOOL QueueTD(CQTD * pCurTD);
#ifdef IRAM_PATCH
    BOOL QueueNextSubTD();
    PUCHAR StartOfIRamBuffer() {
        CheckStructure();
        return m_pIRamStart;
    }
    CQTD* GetCurrentTD() {
        CheckStructure();
        return m_pCurrentTd;
    }
    CQTD* GetIRamTD() {
        CheckStructure();
        return m_pIRamTd;
    }
    BOOL IsIRamTdDone() {
        CheckStructure();
        BOOL bReturn=TRUE;
        CQTD* pCurTD = m_pIRamTd;
 
        if (pCurTD->qTD_Token.qTD_TContext.DataBufferError == 1)
        {
            if (pCurTD->qTD_Token.qTD_TContext.PID == TD_IN_PID)
                DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("TD Token: DataBufferError in IN\r\n")));
            else if (pCurTD->qTD_Token.qTD_TContext.PID == TD_OUT_PID)
                DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("TD Token: DataBufferError in OUT\r\n")));
        }

        if (pCurTD->qTD_Token.qTD_TContext.Halted==1) 
        { 
            // This Transfer Has been halted due to error.
            DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer: Halted\r\n")));
        }
        if (pCurTD->qTD_Token.qTD_TContext.Active == 1) 
        { 
            //RETAILMSG(1, (L"IRAM still Active\r\n"));
            bReturn = FALSE;
        }

        if (pCurTD ->GetLinkValid() == FALSE) 
        { // No link connected. This Transfer is  aborted.
        }
        DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::IsTransferDone (this=0x%x) return %d \r\n"),this,bReturn));
        return bReturn;
    }

    BOOL AdvanceTD() {
        m_pCurrentTd = m_pCurrentTd->GetNextTD();
        RETAILMSG(CRITICAL_LOG_H, (L"n subTD %x\r\n", m_pCurrentTd));
        return (m_pCurrentTd==NULL)?FALSE:TRUE;
    }

    void FreeIRamSpace();

    void SetBufSize(DWORD dwSize) {
        m_dwBufSize = dwSize;
    }
    DWORD GetBufSize() {
        return m_dwBufSize;
    }
    void Dump() {
        CheckStructure();
        RETAILMSG(1, (L"phy addr %x\r\n", m_dwPhys));
        RETAILMSG(1, (L"QH Hor %x\r\n", GetDWORD()));
        RETAILMSG(1, (L"CQTD %x\r\n", currentQTDPointer.dwLinkPointer));
        RETAILMSG(1, (L"NQTD %x\r\n", nextQTDPointer.dwLinkPointer));
        RETAILMSG(1, (L"Overlay H %d, A %d, alt %x\r\n", 
                    qTD_Overlay.qTD_Token.qTD_TContext.Halted,
                    qTD_Overlay.qTD_Token.qTD_TContext.Active,
                    qTD_Overlay.altNextQTDPointer.dwLinkPointer));
    }
#endif
    void InvalidNextTD() { 
        CheckStructure ();
#ifdef IRAM_PATCH
        //RETAILMSG(1, (L"H!\r\n"));
#endif
        currentQTDPointer.dwLinkPointer = 0; 
        nextQTDPointer.lpContext.Terminate=1; 
        qTD_Overlay.altNextQTDPointer.dwLinkPointer=1;
        qTD_Overlay.qTD_Token.qTD_TContext.Active = 0; 
        qTD_Overlay.qTD_Token.qTD_TContext.Halted = 1;
        qTD_Overlay.qTD_Token.qTD_TContext.BytesToTransfer=0;

    };
    void ResetOverlayDataToggle() {
        qTD_Overlay.qTD_Token.qTD_TContext.DataToggle = 0 ;
    }
    DWORD GetPhysAddr() { CheckStructure ();return m_dwPhys; };
    CQH *GetNextQueueQHead(IN CPhysMem * const pCPhysMem) { 
#ifdef IRAM_PATCH
        UNREFERENCED_PARAMETER(pCPhysMem);
        CheckStructure();
        return m_pNextQHead; }
#else
        ASSERT(GetNextLinkPointer(pCPhysMem) == m_pNextQHead);
        CheckStructure ();
        return (CQH *) GetNextLinkPointer(pCPhysMem); };
#endif
    BOOL QueueQHead(CQH *pNextQH){
        CheckStructure ();
        m_pNextQHead = pNextQH;
        if (pNextQH) {
            return SetNextPointer(pNextQH->GetPhysAddr(), TYPE_SELECT_QH, TRUE);
        }
        else {
            SetDWORD(1);// ValidPointer;
            return TRUE;
        }
    }

    BOOL CheckStructure () {
        ASSERT(m_CheckFlag==CQH_CHECK_FLAG_VALUE);
        return (m_CheckFlag==CQH_CHECK_FLAG_VALUE);
    }
private:
    CPipe * const m_pPipe;
    DWORD   m_dwPhys;
    CQH *   m_pNextQHead;
#ifdef IRAM_PATCH
    /*
     * we add 3 data strcutres to each CQH.
     *     m_pVirtualTdList is a pointer to actural TD List of a sTransfer
     *     m_pCurrentTd is a pointer to current processed actural TD
     *     m_pIRamTd is a pointer to cloned TD in iRAM, need to malloc
     */
    CQTD* m_pVirtualTdList;
    CQTD* m_pCurrentTd;
    CQTD* m_pIRamTd;
    CPhysMem* m_pIRamEleMem;
    CPhysMem* m_pIRamDataMem;
#ifdef IRAM_PATCH_EXTEND
    CPhysMem* m_pDRamMem;
    BOOL m_bEleUseExtendMem;  //indicate if we use extend dram memory when not enough memory in iram
    BOOL m_bDatUseExtendMem;
#endif
    PUCHAR m_pIRamStart;
    DWORD m_dwBufSize;
#endif
    const DWORD m_CheckFlag;
};
#pragma warning(pop)

#endif

