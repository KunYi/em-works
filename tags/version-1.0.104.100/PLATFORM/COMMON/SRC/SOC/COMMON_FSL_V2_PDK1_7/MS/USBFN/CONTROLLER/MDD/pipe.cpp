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
#pragma warning(push)
#pragma warning(disable: 4512 6001 6262 6285 6287 6385)
#include "pipe.h"
#pragma warning(pop)
#include "ufnmdd.h"

static
inline
VOID
FreeTransfer(
    CFreeTransferList              *pFreeTransferList,
    PCUfnMddTransfer               pTransfer
    )
{
    SETFNAME();
    PREFAST_DEBUGCHK(pFreeTransferList);
    DEBUGCHK(pTransfer);
    
    DEBUGMSG(ZONE_TRANSFER, (_T("%s Freeing transfer 0x%08x\r\n"), 
        pszFname, pTransfer));

    pFreeTransferList->FreeTransfer(pTransfer);
}


CPipeBase::CPipeBase(
    DWORD dwPhysicalEndpoint, 
    PUFN_PDD_INTERFACE_INFO pPddInfo, 
    CFreeTransferList *pFreeTransferList,
    PUFN_MDD_CONTEXT pContext
    ) : m_TransferQueue()
{
    DEBUGCHK(pPddInfo);
    DEBUGCHK(pFreeTransferList);
    
    InitializeCriticalSection(&m_cs);
    m_dwPhysicalEndpoint = dwPhysicalEndpoint;
    m_pPddInfo = pPddInfo;
    m_pFreeTransferList = pFreeTransferList;
    m_fOpen = FALSE;
    m_dwSig = UFN_PIPE_SIG;
    m_pContext = pContext;

#ifdef DEBUG
    m_dwThreadId = 0;
    m_dwLockCount = 0;
#endif
}


CPipeBase::~CPipeBase()
{
    DEBUGCHK(!IsOpen());
    DEBUGCHK(m_dwLockCount == 0);
    DeleteCriticalSection(&m_cs);
}

CStaticPipe::CStaticPipe(
    DWORD dwPhysicalEndpoint, 
    PUFN_PDD_INTERFACE_INFO pPddInfo, 
    CFreeTransferList *pFreeTransferList,
    PUFN_MDD_CONTEXT pContext
    ) : CPipeBase( dwPhysicalEndpoint, pPddInfo, pFreeTransferList, pContext )
{
    Reset();
}

CStaticPipe::~CStaticPipe()
{
}

CDynamicPipe::CDynamicPipe(
    DWORD dwPhysicalEndpoint, 
    PUFN_PDD_INTERFACE_INFO pPddInfo, 
    CFreeTransferList *pFreeTransferList,
    PUFN_MDD_CONTEXT pContext
    ) : CPipeBase( dwPhysicalEndpoint, pPddInfo, pFreeTransferList, pContext )
{
    Reset();
}

CDynamicPipe::~CDynamicPipe()
{
}

DWORD CStaticPipe::OwningInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration ) const
{
    if( speed == BS_FULL_SPEED )
    {
        ASSERT( dwConfiguration == m_dwFullSpeedConfiguration );
        return m_dwFullSpeedInterfaceNumber;
    }
    else
    {
        ASSERT( dwConfiguration == m_dwHighSpeedConfiguration );
        return m_dwHighSpeedInterfaceNumber;
    }
}

BOOL CStaticPipe::IsPartOfInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterface ) const
{
    if( speed == BS_FULL_SPEED )
    {
        if( dwConfiguration == m_dwFullSpeedConfiguration && dwInterface == m_dwFullSpeedInterfaceNumber )
        {
            return TRUE;
        }
    }
    else
    {
        if( dwConfiguration == m_dwHighSpeedConfiguration && dwInterface == m_dwHighSpeedInterfaceNumber )
        {
            return TRUE;
        }
    }

    return FALSE;
}

DWORD CDynamicPipe::OwningInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration ) const
{
    ce::hash_map<DWORD,DWORD>::const_iterator valueIterator;

    if( m_pContext->Speed == BS_FULL_SPEED ) {
        valueIterator = m_rgdwFullSpeedInterfaceNumber.find( dwConfiguration );
        if( valueIterator != m_rgdwFullSpeedInterfaceNumber.end() )
        {
            return valueIterator->second;
        }
    } else {
        valueIterator = m_rgdwHighSpeedInterfaceNumber.find( dwConfiguration );
        if( valueIterator != m_rgdwHighSpeedInterfaceNumber.end() )
        {
            return valueIterator->second;
        }
    }

    ASSERT( FALSE );
    return -1;
}

BOOL CDynamicPipe::IsPartOfInterface( UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterface ) const
{
    ce::hash_map<DWORD,DWORD>::const_iterator valueIterator;

    if( m_pContext->Speed == BS_FULL_SPEED ) {
        valueIterator = m_rgdwFullSpeedInterfaceNumber.find( dwConfiguration );
        if( valueIterator != m_rgdwFullSpeedInterfaceNumber.end() )
        {
            if( valueIterator->second == dwInterface )
            {
                return TRUE;
            }
        }
    } else {
        valueIterator = m_rgdwHighSpeedInterfaceNumber.find( dwConfiguration );
        if( valueIterator != m_rgdwHighSpeedInterfaceNumber.end() )
        {
            if( valueIterator->second == dwInterface )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

PUSB_ENDPOINT_DESCRIPTOR CStaticPipe::GetEndpointDescriptor( UFN_BUS_SPEED speed ) const
{
    if( speed == BS_FULL_SPEED ) {
        return m_pFullSpeedEndpointDesc;
    } else {
        return m_pHighSpeedEndpointDesc;
    }
}

VOID CStaticPipe::Reset() {
    Lock();
    DEBUGCHK(m_dwSig == UFN_PIPE_SIG);
    DEBUGCHK(IsOpen() == FALSE);
    DEBUGCHK(m_TransferQueue.IsEmpty());

    m_dwFullSpeedInterfaceNumber = -1;
    m_dwFullSpeedAlternateNumber = -1;
    m_dwFullSpeedConfiguration = -1;
    m_pFullSpeedEndpointDesc = NULL;
    m_dwHighSpeedInterfaceNumber = -1;
    m_dwHighSpeedAlternateNumber = -1;
    m_dwHighSpeedConfiguration = -1;
    m_pHighSpeedEndpointDesc = NULL;

    m_dwReservedSpeedMask = 0;
    Unlock();
}

BOOL CStaticPipe::IsReserved(UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterfaceNumber, DWORD dwAlternateSetting) const
{ 
    return (m_dwReservedSpeedMask & speed) != 0; 
}

VOID CStaticPipe::Reserve(
    UFN_BUS_SPEED Speed,
    BOOL fReserve, 
    DWORD bConfiguration, DWORD dwInterfaceNumber, DWORD bAlternateSetting,
    PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc
    ) 
{
    Lock();

    PUSB_ENDPOINT_DESCRIPTOR *ppOurEndpointDesc;
    DWORD *pdwInterfaceNumber, *pdwAlternateNumber, *pdwConfiguration;

    if (Speed == BS_FULL_SPEED) {
        ppOurEndpointDesc = &m_pFullSpeedEndpointDesc;
        pdwInterfaceNumber = &m_dwFullSpeedInterfaceNumber;
        pdwAlternateNumber = &m_dwFullSpeedAlternateNumber;
        pdwConfiguration = &m_dwFullSpeedConfiguration;
    }
    else {
        ppOurEndpointDesc = &m_pHighSpeedEndpointDesc;
        pdwInterfaceNumber = &m_dwHighSpeedInterfaceNumber;
        pdwAlternateNumber = &m_dwHighSpeedAlternateNumber;
        pdwConfiguration = &m_dwHighSpeedConfiguration;
    }
    
    if (fReserve) {
        m_dwReservedSpeedMask |= Speed;

        if (GetPhysicalEndpoint() != 0) {
            *ppOurEndpointDesc = pEndpointDesc;
            *pdwInterfaceNumber = dwInterfaceNumber;
            *pdwAlternateNumber = bAlternateSetting;
            *pdwConfiguration = bConfiguration;
        }
    }
    else {
        m_dwReservedSpeedMask &= ~Speed;
        *ppOurEndpointDesc = NULL;
        *pdwInterfaceNumber = -1;
        *pdwAlternateNumber = -1;
        *pdwConfiguration = -1;
    }

    Unlock();
}

VOID CDynamicPipe::Reset() {
    Lock();
    DEBUGCHK(m_dwSig == UFN_PIPE_SIG);
    DEBUGCHK(IsOpen() == FALSE);
    DEBUGCHK(m_TransferQueue.IsEmpty());

    m_rgdwFullSpeedInterfaceNumber.clear();
    m_rgdwHighSpeedInterfaceNumber.clear();
    m_rgdwReservedSpeedMask.clear();
    m_rgpFullSpeedEndpointDesc.clear();
    m_rgpHighSpeedEndpointDesc.clear();
    Unlock();
}

BOOL CDynamicPipe::IsReserved(UFN_BUS_SPEED speed, DWORD dwConfiguration, DWORD dwInterfaceNumber, DWORD dwAlternateSetting) const
{ 
    DWORD dwInterfaceIndex = -1;

    ce::hash_map<DWORD,DWORD>::const_iterator valueIterator;

    if( GetPhysicalEndpoint() != 0 )
    {
        // first determine what interface (if any) is this endpoint currently assigned to
        if( speed == BS_FULL_SPEED ) {
            valueIterator = m_rgdwFullSpeedInterfaceNumber.find( dwConfiguration );
            if( valueIterator != m_rgdwFullSpeedInterfaceNumber.end() )
            {
                dwInterfaceIndex = valueIterator->second;
            }
        } else {
            valueIterator = m_rgdwHighSpeedInterfaceNumber.find( dwConfiguration );
            if( valueIterator != m_rgdwHighSpeedInterfaceNumber.end() )
            {
                dwInterfaceIndex = valueIterator->second;
            }
        }

        // if this endpoint is not assigned to any interface, then it is not reserved
        if( dwInterfaceIndex == -1 )
        {
            return FALSE;
        }

        // if this endpoint is currently assigned to an interface, we can only assigned to an alternate of this interface
        if( dwInterfaceIndex != dwInterfaceNumber )
        {
            return TRUE;
        }
    }

    DWORD dwKey = ( ( dwConfiguration & 0xffff ) << 16 ) | ( dwAlternateSetting & 0xffff );
    valueIterator = m_rgdwReservedSpeedMask.find( dwKey );
    if( valueIterator != m_rgdwReservedSpeedMask.end() )
    {
        DWORD dwSpeedMask = valueIterator->second;
        return ( dwSpeedMask & speed ) != 0;
    }
    else
    {
        return FALSE;
    }
}

VOID CDynamicPipe::Reserve(
    UFN_BUS_SPEED Speed,
    BOOL fReserve, 
    DWORD bConfiguration, DWORD dwInterfaceNumber, DWORD bAlternateSetting,
    PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc
    ) 
{
    Lock();

    DWORD dwKey = ( (bConfiguration & 0xffff) << 16 ) | (bAlternateSetting & 0xffff);
    ce::hash_map<DWORD,PUSB_ENDPOINT_DESCRIPTOR>::iterator descriptorIterator;
    ce::hash_map<DWORD,DWORD>::iterator valueIterator;

    typedef std::pair<DWORD,PUSB_ENDPOINT_DESCRIPTOR> DescriptorPair;
    typedef std::pair<DWORD,DWORD> DwordPair;

    if (fReserve) {
        valueIterator = m_rgdwReservedSpeedMask.find(dwKey);
        if( valueIterator == m_rgdwReservedSpeedMask.end() ) {
            m_rgdwReservedSpeedMask.insert( DwordPair( dwKey, Speed ) );
        } else {
            valueIterator->second |= Speed;
        }

        if (GetPhysicalEndpoint() != 0) {
            if (Speed == BS_FULL_SPEED) {
                m_rgdwFullSpeedInterfaceNumber.insert( DwordPair( bConfiguration, dwInterfaceNumber ) );
                m_rgpFullSpeedEndpointDesc.insert( DescriptorPair( dwKey, pEndpointDesc ) );
            }
            else {
                m_rgdwHighSpeedInterfaceNumber.insert( DwordPair( bConfiguration, dwInterfaceNumber ) );
                m_rgpHighSpeedEndpointDesc.insert( DescriptorPair( dwKey, pEndpointDesc ) );
            }
        }
    }
    else {
        valueIterator = m_rgdwReservedSpeedMask.find(dwKey);
        ASSERT( valueIterator != m_rgdwReservedSpeedMask.end() );
        if( valueIterator != m_rgdwReservedSpeedMask.end() ) {
            valueIterator->second &= ~Speed;
            if( valueIterator->second == 0 )
            {
                m_rgdwReservedSpeedMask.erase( valueIterator );
            }
        }

        if (GetPhysicalEndpoint() != 0) {
            if (Speed == BS_FULL_SPEED) {
                valueIterator = m_rgdwFullSpeedInterfaceNumber.find(bConfiguration);
                ASSERT( valueIterator != m_rgdwFullSpeedInterfaceNumber.end() );
                if( valueIterator != m_rgdwFullSpeedInterfaceNumber.end() )
                {
                    m_rgdwFullSpeedInterfaceNumber.erase( valueIterator );
                }
                descriptorIterator = m_rgpFullSpeedEndpointDesc.find( dwKey );
                ASSERT( descriptorIterator != m_rgpFullSpeedEndpointDesc.end() );
                if( descriptorIterator != m_rgpFullSpeedEndpointDesc.end() )
                {
                    m_rgpFullSpeedEndpointDesc.erase( descriptorIterator );
                }
            }
            else {
                valueIterator = m_rgdwHighSpeedInterfaceNumber.find(bConfiguration);
                ASSERT( valueIterator != m_rgdwHighSpeedInterfaceNumber.end() );
                if( valueIterator != m_rgdwHighSpeedInterfaceNumber.end() )
                {
                    m_rgdwHighSpeedInterfaceNumber.erase( valueIterator );
                }
                descriptorIterator = m_rgpHighSpeedEndpointDesc.find( dwKey );
                ASSERT( descriptorIterator != m_rgpHighSpeedEndpointDesc.end() );
                if( descriptorIterator != m_rgpHighSpeedEndpointDesc.end() )
                {
                    m_rgpHighSpeedEndpointDesc.erase( descriptorIterator );
                }
            }
        }
    }

    Unlock();
}

PUSB_ENDPOINT_DESCRIPTOR CDynamicPipe::GetEndpointDescriptor( UFN_BUS_SPEED speed ) const 
{
    DWORD dwCurrentAltSetting;
    DWORD dwInterfaceIndex;

    DWORD dwConfiguration = m_pContext->pDescriptors->GetConfiguration(speed);

    ce::hash_map<DWORD,DWORD>::const_iterator valueIterator;
    if( speed == BS_FULL_SPEED ) {
        valueIterator = m_rgdwFullSpeedInterfaceNumber.find(dwConfiguration);
        if( valueIterator == m_rgdwFullSpeedInterfaceNumber.end() )
        {
            return NULL;
        }
        dwInterfaceIndex = valueIterator->second;
    } else {
        valueIterator = m_rgdwHighSpeedInterfaceNumber.find(dwConfiguration);
        if( valueIterator == m_rgdwHighSpeedInterfaceNumber.end() )
        {
            return NULL;
        }
        dwInterfaceIndex = valueIterator->second;
    }

    DWORD dwError = m_pContext->pDescriptors->GetInterface(speed, 
        dwInterfaceIndex, &dwCurrentAltSetting);

    if( dwError != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
        return NULL;
    }

    ce::hash_map<DWORD,PUSB_ENDPOINT_DESCRIPTOR>::const_iterator descriptorIterator;

    DWORD dwKey = ( ( dwConfiguration & 0xffff ) << 16 ) | ( dwCurrentAltSetting & 0xffff );

    if (speed == BS_FULL_SPEED) {
        descriptorIterator = m_rgpFullSpeedEndpointDesc.find( dwKey );
        if( descriptorIterator != m_rgpFullSpeedEndpointDesc.end() )
        {
            return descriptorIterator->second;
        }
    }
    else {
        descriptorIterator = m_rgpHighSpeedEndpointDesc.find( dwKey );
        if( descriptorIterator != m_rgpHighSpeedEndpointDesc.end() )
        {
            return descriptorIterator->second;
        }
    }
    
    return NULL;
}

DWORD CPipeBase::GetEndpointAddress() const { 
    DWORD dwEndpointAddress = -1;

    if (GetPhysicalEndpoint() == 0) {
        dwEndpointAddress = 0;
    }
    else
    {
        PUSB_ENDPOINT_DESCRIPTOR pEndpointDescriptor = GetEndpointDescriptor( m_pContext->Speed );
        if( pEndpointDescriptor != NULL )
        {
            dwEndpointAddress = pEndpointDescriptor->bEndpointAddress;
        }
    } 

    return dwEndpointAddress; 
}

BOOL CPipeBase::IsReserved(UFN_BUS_SPEED speed) const
{
    DWORD dwConfiguration = m_pContext->pDescriptors->GetConfiguration(speed);
    DWORD dwInterface = OwningInterface( speed, dwConfiguration );
    if( dwInterface == -1 )
    {
        return FALSE;
    }
    DWORD dwCurrentAltSetting;
    if( m_pContext->pDescriptors->GetInterface(speed, dwInterface, &dwCurrentAltSetting) != ERROR_SUCCESS )
    {
        ASSERT(FALSE);
        dwCurrentAltSetting = 0; // assume the alt setting is 0 (default)
    }
    return IsReserved( speed, dwConfiguration, dwInterface, dwCurrentAltSetting );
}

DWORD CPipeBase::Open(UFN_BUS_SPEED Speed, PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc)
{
    SETFNAME();
    DWORD dwRet;
    
    Lock();

    DEBUGCHK(m_TransferQueue.IsEmpty());
    DEBUGCHK( ( pEndpointDesc && pEndpointDesc->bEndpointAddress == 0 ) || IsReserved(Speed));

    if (IsOpen()) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Pipe (0x%08x) is already opened\r\n"), pszFname, this));
        dwRet = ERROR_INVALID_PARAMETER;
    }
    else {
        PUSB_ENDPOINT_DESCRIPTOR pOurEndpointDesc = GetEndpointDescriptor( Speed );

        DWORD dwConfiguration, dwInterfaceIndex, dwCurrentAltSetting;

        if (pOurEndpointDesc == NULL) {
            DEBUGCHK(GetPhysicalEndpoint() == 0);
            DEBUGCHK(pEndpointDesc);
            pOurEndpointDesc = pEndpointDesc;
            dwConfiguration = 0;
            dwInterfaceIndex = 0;
            dwCurrentAltSetting = 0;
        }
        else {
            DEBUGCHK(GetPhysicalEndpoint() != 0);
            DEBUGCHK(pEndpointDesc == NULL);
        
            // Get the current configuration value
            dwConfiguration = m_pContext->pDescriptors->GetConfiguration(Speed);

            // Get the current interface value
            dwInterfaceIndex = OwningInterface( Speed, dwConfiguration );

            // Get the current alternate interface value
            DWORD dwError = m_pContext->pDescriptors->GetInterface(Speed, 
                dwInterfaceIndex, &dwCurrentAltSetting);
            if( dwError != ERROR_SUCCESS )
            {
                ASSERT( FALSE );
                return ERROR_GEN_FAILURE;
            }
        }

        dwRet = m_pPddInfo->pfnInitEndpoint(m_pPddInfo->pvPddContext, 
            m_dwPhysicalEndpoint, Speed, pOurEndpointDesc, NULL, dwConfiguration, dwInterfaceIndex, dwCurrentAltSetting);

        if (dwRet == ERROR_SUCCESS) {
            DEBUGMSG(ZONE_INIT || ZONE_PIPE, (_T("%s Opened pipe (0x%08x): physical address %u, bus address 0x%02x\r\n"),
                pszFname, this, m_dwPhysicalEndpoint, GetEndpointAddress()));
            m_fOpen = TRUE;
        }
        else {
            DEBUGMSG(ZONE_ERROR, (_T("%s Failed to open pipe (0x%08x): physical address %u, bus address 0x%02x\r\n"),
                pszFname, this, m_dwPhysicalEndpoint, GetEndpointAddress()));
        }
    }

    Unlock();

    return dwRet;
}

DWORD CPipeBase::Close() {
    SETFNAME();
    DWORD dwRet = ERROR_SUCCESS;
    
    Lock();

    if (IsOpen() == FALSE) {
        DEBUGMSG(ZONE_PIPE, (_T("%s Pipe (0x%08x): physical address %u is not open\r\n"), 
            pszFname, this, GetPhysicalEndpoint()));
        dwRet = ERROR_INVALID_HANDLE;
    }
    else {
        AbortAllTransfers();
    
        dwRet = m_pPddInfo->pfnDeinitEndpoint(m_pPddInfo->pvPddContext, m_dwPhysicalEndpoint);  // UfnPdd_DeinitEndpoint

        if (dwRet == ERROR_SUCCESS) {
            DEBUGMSG(ZONE_INIT || ZONE_PIPE, (_T("%s Closed pipe (0x%08x): physical address %u, bus address 0x%02x\r\n"),
                pszFname, this, m_dwPhysicalEndpoint, GetEndpointAddress()));
        }
        else {
            DEBUGMSG(ZONE_ERROR, (_T("%s Failed to close pipe (0x%08x): physical address %u, bus address 0x%02x\r\n"),
                pszFname, this, m_dwPhysicalEndpoint, GetEndpointAddress()));
        }

        // We will say we are closed even if there was a failure
        m_fOpen = FALSE; 
    }

    Unlock();
    
    return (dwRet == ERROR_SUCCESS);
}


VOID CPipeBase::AbortAllTransfers()
{
    SETFNAME();
    Lock();
    
    DEBUGCHK(IsOpen());
    
    while (m_TransferQueue.IsEmpty() == FALSE) {
        PCUfnMddTransfer pTransfer = m_TransferQueue.Back();
        pTransfer->Validate();
    
        DEBUGCHK(!pTransfer->IsComplete());
        DWORD dwErr = AbortTransfer(pTransfer);
        DEBUGCHK(dwErr == ERROR_SUCCESS);
    }

    Unlock();
}


DWORD CPipeBase::IssueTransfer(
    LPTRANSFER_NOTIFY_ROUTINE   lpNotifyRoutine,
    PVOID                       pvNotifyContext,
    DWORD                       dwFlags,
    DWORD                       cbBuffer, 
    PVOID                       pvBuffer,
    DWORD                       dwBufferPhysicalAddress,
    PCUfnMddTransfer           *ppTransfer,
    PVOID                       pvPddTransferInfo
    )
{
    SETFNAME();

    Lock();
    FUNCTION_ENTER_MSG();

    DEBUGCHK( (lpNotifyRoutine && pvNotifyContext) || 
              ( (lpNotifyRoutine == NULL) && (pvNotifyContext == NULL) ) );
    DEBUGCHK( (cbBuffer == 0) || (pvBuffer) );
    DEBUGCHK(ppTransfer);

    DWORD dwRet;
    BOOL fTransferOnQueue = FALSE;
    BOOL fStartTransfer = FALSE;
    PCUfnMddTransfer pTransfer = NULL;
    
    if (IsOpen() == FALSE) {
        DEBUGMSG(ZONE_WARNING, (_T("%s Pipe (0x%08x): physical address %u is not open\r\n"), 
            pszFname, this, GetPhysicalEndpoint()));
        dwRet = ERROR_INVALID_HANDLE;
        goto EXIT;
    }
    
    pTransfer = m_pFreeTransferList->AllocTransfer();
    if (pTransfer == NULL) {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to allocate new transfer. Error: %d\r\n"),
            pszFname, dwRet));
        goto EXIT;
    }

    pTransfer->Init(dwFlags, pvBuffer, dwBufferPhysicalAddress, cbBuffer,
        pvPddTransferInfo, lpNotifyRoutine, pvNotifyContext, this);
    pTransfer->AddRef();

    if (m_TransferQueue.IsEmpty()) {
        fStartTransfer = TRUE;
    }
    dwRet = m_TransferQueue.PushBack(pTransfer);
    
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    pTransfer->AddRef();
    fTransferOnQueue = TRUE;

    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        *ppTransfer = pTransfer;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        RETAILMSG(1, (_T("%s Exception!\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
    }

    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    DEBUGMSG(ZONE_TRANSFER, (_T("%s Issuing %s transfer (0x%08x) on ep %u (0x%02x) for %u bytes\r\n"),
        pszFname, 
        (pTransfer->GetDwFlags() & USB_REQUEST_DEVICE_TO_HOST) ? _T("IN") : _T("OUT"),
        pTransfer, GetPhysicalEndpoint(), GetEndpointAddress(), 
        pTransfer->GetCbBuffer()));

    if (fStartTransfer) {
        DEBUGCHK(pTransfer == m_TransferQueue.Front());
        pTransfer->EnteringPdd();
        dwRet = m_pPddInfo->pfnIssueTransfer(m_pPddInfo->pvPddContext, 
            GetPhysicalEndpoint(), pTransfer->GetPddTransfer());
        if (dwRet != ERROR_SUCCESS) {
            RETAILMSG(1, (_T("%s Could not start transfer 0x%08x\r\n"), 
                pszFname, pTransfer));
            pTransfer->LeavingPdd();
        }
    }

EXIT:
    if (pTransfer) {
        if (dwRet != ERROR_SUCCESS) {
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try {
                *ppTransfer = NULL;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                DEBUGMSG(ZONE_ERROR, (_T("%s Exception!\r\n"), pszFname));
            }

            if (fTransferOnQueue) {
                // We must remove this transfer from the list.
                // Note that we've been holding the endpoint lock
                // this whole time so no one has taken the transfer off
                // the queue yet.
                DEBUGCHK(m_TransferQueue.Back() == pTransfer);
                m_TransferQueue.PopBack();
                pTransfer->Release();
            }
        }
        
        if (dwRet != ERROR_SUCCESS) {
            // Free the transfer
            pTransfer->Release();
        }
    }
    
    FUNCTION_LEAVE_MSG();

    Unlock();

    return dwRet;
}


DWORD CPipeBase::AbortTransfer( PCUfnMddTransfer pTransfer )
{
    SETFNAME();
    DWORD dwRet = ERROR_SUCCESS;

    pTransfer->AddRef();

    Lock();

    DEBUGMSG(ZONE_TRANSFER, (_T("%s Aborting transfer 0x%08x on pipe at address %u (0x%08x)\r\n"),
        pszFname, pTransfer, GetPhysicalEndpoint(), this));
    
    pTransfer->Validate();

    if (!pTransfer->IsComplete()) {
#ifdef DEBUG
        DEBUGCHK(pTransfer == m_TransferQueue.Front() ||
            pTransfer == m_TransferQueue.Back()); // TODO: Perform a search for correct transfer
#endif

        if (pTransfer->IsInPdd()) {
            DWORD dwErr = m_pPddInfo->pfnAbortTransfer(
                m_pPddInfo->pvPddContext, GetPhysicalEndpoint(), 
                pTransfer->GetPddTransfer());
            DEBUGCHK(dwErr == ERROR_SUCCESS);
        }
        else {
            pTransfer->SetDwUsbError(UFN_CANCELED_ERROR);
            DWORD dwErr = TransferComplete(pTransfer, FALSE);
            DEBUGCHK(dwErr == ERROR_SUCCESS);
        }
    }
        
    Unlock();

    pTransfer->Release();

    return dwRet;
}


DWORD CPipeBase::GetTransferStatus(
    const CUfnMddTransfer      *pTransfer,
    PDWORD                      pcbTransferred,
    PDWORD                      pdwUsbError
    )
{
    SETFNAME();
    
    DEBUGCHK(pTransfer);
    DEBUGCHK(pcbTransferred);
    DEBUGCHK(pdwUsbError);

    DWORD dwRet = ERROR_SUCCESS;

    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        // Get the USB error value first. If it is a completed value,
        // then cbTransferred will be accurrate. If we got cbTransferred
        // first, it may not be correct according to the error value.
        *pdwUsbError = pTransfer->GetDwUsbError();
        *pcbTransferred = pTransfer->GetCbTransferred();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Exception!\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
    }

    return dwRet;
}


DWORD CPipeBase::CloseTransfer( PCUfnMddTransfer pTransfer )
{
    SETFNAME();
    DWORD dwRet = ERROR_SUCCESS;

    Lock();

    DEBUGMSG(ZONE_TRANSFER, (_T("%s Closing transfer 0x%08x on pipe at address %u (0x%08x)\r\n"),
        pszFname, pTransfer, GetPhysicalEndpoint(), this));

    pTransfer->Validate();

    if (!pTransfer->IsComplete()) {
        DWORD dwErr = AbortTransfer(pTransfer);
        DEBUGCHK(dwErr == ERROR_SUCCESS); // Why would this fail?
    }

    pTransfer->Release();

    Unlock();

    return dwRet;
}


VOID CPipeBase::FreeTransfer( PCUfnMddTransfer pTransfer )
{
    SETFNAME();
    DEBUGCHK(pTransfer);
    DEBUGMSG(ZONE_TRANSFER, (_T("%s Placing transfer 0x%08x on free list\r\n"),
        pszFname, pTransfer));
    m_pFreeTransferList->FreeTransfer(pTransfer);
}


DWORD CPipeBase::TransferComplete( PCUfnMddTransfer pTransfer, BOOL fTransferWasInPdd )
{
    SETFNAME();
    DWORD dwRet = ERROR_SUCCESS;

    PREFAST_DEBUGCHK(pTransfer);

    Lock();

    DEBUGMSG(ZONE_TRANSFER, (_T("%s Completing transfer 0x%08x on pipe at address %u (0x%08x)\r\n"),
        pszFname, pTransfer, GetPhysicalEndpoint(), this));

    pTransfer->Validate();
    DEBUGCHK(pTransfer->IsInPdd() == FALSE);

    DEBUGCHK(pTransfer->IsComplete() == TRUE);
    DEBUGCHK(m_TransferQueue.IsEmpty() == FALSE);

    if (pTransfer == m_TransferQueue.Front()) {
        m_TransferQueue.PopFront();
    }
    else {
        DEBUGCHK(pTransfer == m_TransferQueue.Back());
        DEBUGCHK(fTransferWasInPdd == FALSE);
        m_TransferQueue.PopBack();
    }
    pTransfer->Release();

    pTransfer->CallCompletionRoutine();

    PCUfnMddTransfer pNewTransfer = NULL;
    if (fTransferWasInPdd) {
        if (m_TransferQueue.IsEmpty() == FALSE) {
            // Start the next transfer
            pNewTransfer = m_TransferQueue.Front();
        }
    }

    if (pNewTransfer) {
        pNewTransfer->EnteringPdd();
        DWORD dwErr = m_pPddInfo->pfnIssueTransfer(m_pPddInfo->pvPddContext,
            GetPhysicalEndpoint(), pNewTransfer->GetPddTransfer());
        if (dwErr != ERROR_SUCCESS) {
            RETAILMSG(1, (_T("%s Could not start next transfer 0x%08x\r\n"), 
                pszFname, pTransfer));
            pNewTransfer->LeavingPdd();
            pNewTransfer->SetDwUsbError(UFN_DEVICE_NOT_RESPONDING_ERROR);
            TransferComplete(pNewTransfer, FALSE);
        }
    }

    Unlock();

    return dwRet;
}


DWORD CPipeBase::Stall()
{
    SETFNAME();
    DWORD dwRet;

    Lock();
    
    if (IsOpen() == FALSE) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Pipe is not open\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
    }
    else {
        DEBUGMSG(ZONE_PIPE, (_T("%s Stalling endpoint %u pipe (0x%08x)\r\n"),
            pszFname, GetEndpointAddress(), this));
        dwRet = m_pPddInfo->pfnStallEndpoint(m_pPddInfo->pvPddContext, 
            GetPhysicalEndpoint());
    }

    Unlock();

    return dwRet;
}

DWORD CPipeBase::ClearStall()
{
    SETFNAME();
    DWORD dwRet;

    Lock();
    
    if (IsOpen() == FALSE) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Pipe is not open\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
    }
    else {
        DEBUGMSG(ZONE_PIPE, (_T("%s Clearing stall on endpoint %u pipe (0x%08x)\r\n"),
            pszFname, GetEndpointAddress(), this));
        dwRet = m_pPddInfo->pfnClearEndpointStall(m_pPddInfo->pvPddContext, GetPhysicalEndpoint());
    }

    Unlock();

    return dwRet;
}


DWORD CPipeBase::SendControlStatusHandshake()
{
    SETFNAME();

    Lock();

    DEBUGCHK(GetPhysicalEndpoint() == 0);
    DEBUGCHK(IsOpen());
    
    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Sending Control Status Handshake\r\n"),
        pszFname, GetEndpointAddress(), this));
    DWORD dwRet = m_pPddInfo->pfnSendControlStatusHandshake(
        m_pPddInfo->pvPddContext, 0);

    Unlock();

    return dwRet;
}


// Check a pipe handle from the caller for correctness.
DWORD CPipeBase::ValidatePipeHandle( const CPipeBase *pPipe )
{
    SETFNAME();

    DWORD dwRet = ERROR_SUCCESS;
    
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        if (!pPipe) {
            DEBUGMSG(ZONE_WARNING, (_T("%s Invalid Pipe handle 0x%08x\r\n"),
                pszFname, pPipe));
            dwRet = ERROR_INVALID_HANDLE;
        }
        else if ( (pPipe->m_dwSig != UFN_PIPE_SIG) || !pPipe->IsOpen() ) {
            DEBUGMSG(ZONE_PIPE, (_T("%s Invalid Pipe handle 0x%08x (sig 0x%X)\r\n"),
                pszFname, pPipe, pPipe->m_dwSig));
            dwRet = ERROR_INVALID_HANDLE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Exception checking Pipe handle 0x%08x\r\n"),
            pszFname, pPipe));
        dwRet = ERROR_INVALID_HANDLE;
    }
    
    return dwRet;
}


#ifdef DEBUG
VOID CPipeBase::Validate()
{
    Lock();
    
    DEBUGCHK(m_dwSig == UFN_PIPE_SIG);
    DEBUGCHK(m_pPddInfo != NULL);
    DEBUGCHK(m_pFreeTransferList != NULL);
    
    if (m_TransferQueue.IsEmpty() == FALSE) {
        PCUfnMddTransfer pTransfer = m_TransferQueue.Front();
        pTransfer->Validate();
        DEBUGCHK(pTransfer->GetPipe() == this);
    }
    
    Unlock();
}
#endif

