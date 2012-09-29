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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#ifndef _PIPE_H_
#define _PIPE_H_
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4100)
#pragma warning(disable:4127)

#include <hash.hxx>
#include <hash_map.hxx>
#include "xferqueue.h"
#pragma warning(pop)
#include "xferlist.h"
#include "transfer.h"


// Forward declaration
typedef struct UFN_MDD_CONTEXT *PUFN_MDD_CONTEXT;

#define UFN_PIPE_SIG        'PnfU' // "UfnP" signature

struct UFN_MDD_CONTEXT;

class CPipeBase {
public:
    CPipeBase(DWORD dwPhysicalEndpoint, PUFN_PDD_INTERFACE_INFO pPddInfo, 
        CFreeTransferList *pFreeTransferList, PUFN_MDD_CONTEXT pContext);
    virtual ~CPipeBase();

    DWORD Open(UFN_BUS_SPEED Speed, PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc);
    DWORD Close();

    BOOL IsOpen() const { return m_fOpen; }
    virtual PUSB_ENDPOINT_DESCRIPTOR GetEndpointDescriptor( UFN_BUS_SPEED speed ) const = 0;
    virtual DWORD OwningInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration ) const = 0;
    virtual BOOL IsPartOfInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterface ) const = 0;
    DWORD GetPhysicalEndpoint() const { return m_dwPhysicalEndpoint; }
    DWORD GetEndpointAddress() const;    
    virtual VOID Reserve(UFN_BUS_SPEED Speed, BOOL fReserve, DWORD bConfiguration, DWORD dwInterfaceNumber, DWORD bAlternateSetting,
        PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc) = 0;

    virtual BOOL IsReserved(UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterfaceNumber, DWORD dwAlternateSetting) const = 0;
    virtual BOOL IsReserved(UFN_BUS_SPEED speed) const;

    DWORD IssueTransfer(
        LPTRANSFER_NOTIFY_ROUTINE   lpNotifyRoutine,
        PVOID                       pvNotifyContext,
        DWORD                       dwFlags,
        DWORD                       cbBuffer, 
        PVOID                       pvBuffer,
        DWORD                       dwBufferPhysicalAddress,
        PCUfnMddTransfer           *ppTransfer,
        PVOID                       pDMATransferInfo);
    DWORD AbortTransfer( PCUfnMddTransfer pTransfer );
    VOID AbortAllTransfers();
    static DWORD GetTransferStatus(
        const CUfnMddTransfer      *pTransfer,
        PDWORD                      pcbTransferred,
        PDWORD                      pdwUsbError
        );
    DWORD CloseTransfer( PCUfnMddTransfer pTransfer );
    DWORD TransferComplete( PCUfnMddTransfer pTransfer, BOOL fTransferWasInPdd );
    DWORD Stall();
    DWORD ClearStall();
    DWORD SendControlStatusHandshake();
    virtual VOID Reset() = 0;
    VOID FreeTransfer( PCUfnMddTransfer pTransfer );
    static DWORD ValidatePipeHandle(const CPipeBase *pPipe);

#ifdef DEBUG
    VOID Validate();
#else
    VOID Validate() {}
#endif

protected:
    VOID Lock() {
        EnterCriticalSection(&m_cs);
        
#ifdef DEBUG
        if (m_dwLockCount++ == 0) {
            m_dwThreadId = GetCurrentThreadId();
        }
#endif
    }

    VOID Unlock() {
#ifdef DEBUG
        if (--m_dwLockCount == 0) {
            m_dwThreadId = 0;
        }
#endif

        LeaveCriticalSection(&m_cs);
    }

    PUFN_MDD_CONTEXT m_pContext;
    DWORD m_dwSig;
    CRITICAL_SECTION m_cs;
    BOOL   m_fOpen;
    PUFN_PDD_INTERFACE_INFO m_pPddInfo;
    CTransferQueue<CUfnMddTransfer> m_TransferQueue;
    CFreeTransferList *m_pFreeTransferList;
    DWORD  m_dwPhysicalEndpoint; // PDD endpoint index (0 based)
#ifdef DEBUG
    DWORD               m_dwThreadId;
    DWORD               m_dwLockCount;
#endif
};

class CDynamicPipe : public CPipeBase
{
public:
    CDynamicPipe(DWORD dwPhysicalEndpoint, PUFN_PDD_INTERFACE_INFO pPddInfo, 
        CFreeTransferList *pFreeTransferList, PUFN_MDD_CONTEXT pContext);
    virtual ~CDynamicPipe();
    virtual DWORD OwningInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration ) const;
    virtual BOOL IsPartOfInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterface ) const;
    virtual PUSB_ENDPOINT_DESCRIPTOR GetEndpointDescriptor( UFN_BUS_SPEED speed ) const;
    virtual VOID Reserve(UFN_BUS_SPEED Speed, BOOL fReserve, DWORD bConfiguration, DWORD dwInterfaceNumber, DWORD bAlternateSetting,
        PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc);
    virtual BOOL IsReserved(UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterfaceNumber, DWORD dwAlternateSetting) const;
    virtual VOID Reset();

    ce::hash_map<DWORD,DWORD> m_rgdwFullSpeedInterfaceNumber;  // Number of the interface in full speed descriptor
    ce::hash_map<DWORD,DWORD> m_rgdwHighSpeedInterfaceNumber;  // Number of the interface in high speed descriptor
    ce::hash_map<DWORD,DWORD> m_rgdwReservedSpeedMask;         // Reserved speed mask (for Configuration/Alternate combination)
    ce::hash_map<DWORD,PUSB_ENDPOINT_DESCRIPTOR> m_rgpFullSpeedEndpointDesc; // endpoint descriptor (for Configuration/Alternate combination)
    ce::hash_map<DWORD,PUSB_ENDPOINT_DESCRIPTOR> m_rgpHighSpeedEndpointDesc; // endpoint descriptor (for Configuration/Alternate combination)
};

class CStaticPipe : public CPipeBase
{
public:
    CStaticPipe(DWORD dwPhysicalEndpoint, PUFN_PDD_INTERFACE_INFO pPddInfo, 
        CFreeTransferList *pFreeTransferList, PUFN_MDD_CONTEXT pContext);
    virtual ~CStaticPipe();
    virtual DWORD OwningInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration ) const;
    virtual BOOL IsPartOfInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterface ) const;
    virtual PUSB_ENDPOINT_DESCRIPTOR GetEndpointDescriptor( UFN_BUS_SPEED speed ) const;
    virtual VOID Reserve(UFN_BUS_SPEED Speed, BOOL fReserve, DWORD bConfiguration, DWORD dwInterfaceNumber, DWORD bAlternateSetting,
        PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc);
    virtual BOOL IsReserved(UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterfaceNumber, DWORD dwAlternateSetting) const;
    virtual VOID Reset();

    DWORD  m_dwFullSpeedInterfaceNumber;
    DWORD  m_dwFullSpeedAlternateNumber;
    DWORD  m_dwFullSpeedConfiguration;
    PUSB_ENDPOINT_DESCRIPTOR m_pFullSpeedEndpointDesc;
    DWORD  m_dwHighSpeedInterfaceNumber;
    DWORD  m_dwHighSpeedAlternateNumber;
    DWORD  m_dwHighSpeedConfiguration;
    PUSB_ENDPOINT_DESCRIPTOR m_pHighSpeedEndpointDesc;

    DWORD  m_dwReservedSpeedMask;
};


#endif // _PIPE_H_

