//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  irfir.cpp
//
//   This file implements the device specific functions for iMX51 fir device.
//
//------------------------------------------------------------------------------
#include "IrFir.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines
#ifdef DEBUG
DBGPARAM dpCurSettings =
{
        TEXT("FIR"),
        {
            TEXT("Init"),       TEXT("Deinit"),
            TEXT("Receive"),    TEXT("Send"),
            TEXT("SetInfo"),    TEXT("QueryInfo"),
            TEXT("Thread"),     TEXT("Open"),
            TEXT("Close"),      TEXT("CommMask"),
            TEXT("Undefined"),  TEXT("Misc"),
            TEXT("Alloc"),      TEXT("Function"),
            TEXT("Warning"),    TEXT("Error")
        },
        0x0000C003
};
#endif // DEBUG

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables
//Event using to synchronize with control panel FIR setting application
//When Event is set, then bFirOn change to TRUE, and uart clock will be open
//When Event is reset, then bFirOn change to FALSE, and uart clock will be close
HANDLE hFirset = NULL;
BOOL bFirOn;
//------------------------------------------------------------------------------
// Local Variables

// We keep a linked list of device objects
pFirDevice_t gFirstFirDevice = NULL;

static const UINT
v_rgSupportedOids[] =
{
    // General required OIDs.

    //                                     Query        Set       Bytes
    //                                   supported   supported   required
    //                                  ----------- ----------- ----------
    OID_GEN_SUPPORTED_LIST,             // Query                    Arr(4)
    OID_GEN_MEDIA_IN_USE,               // Query                    Arr(4)
    OID_GEN_MAXIMUM_LOOKAHEAD,          // Query                    4
    OID_GEN_MAXIMUM_FRAME_SIZE,         // Query                    4
    OID_GEN_VENDOR_DRIVER_VERSION,      // Query                    2
    OID_GEN_CURRENT_PACKET_FILTER,      // Query        Set        4
    OID_GEN_MAC_OPTIONS,                // Query                    4
    OID_GEN_MAXIMUM_SEND_PACKETS,       // Query                    4

    // Infrared-specific OIDs.
    OID_IRDA_RECEIVING,                 // Query                    4
    OID_IRDA_TURNAROUND_TIME,           // Query                    4
    OID_IRDA_SUPPORTED_SPEEDS,          // Query                    Arr(4)
    OID_IRDA_LINK_SPEED,                // Query        Set        4
    OID_IRDA_MEDIA_BUSY,                // Query        Set        4
    OID_IRDA_EXTRA_RCV_BOFS,            // Query                    4
    OID_IRDA_REACQUIRE_HW_RESOURCES,    //                Set        4
    OID_IRDA_RELEASE_HW_RESOURCES,      // Query                    4

    // Powermanagement specific OIDs
    OID_PNP_CAPABILITIES,               // Query
    OID_PNP_SET_POWER,                  // Query        Set    
    OID_PNP_QUERY_POWER,                // Query
    OID_PNP_ADD_WAKE_UP_PATTERN,        // Query        Set
    OID_PNP_REMOVE_WAKE_UP_PATTERN,     // Query        Set
    OID_PNP_ENABLE_WAKE_UP              // Query        Set
};


//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
//  This function is entry point of the dll.
//
// Parameters:
//      hInstDll
//          [in] .
//      Op
//          [in] .
//      lpvReserved
//          [in] .
//
// Returns:
//    This function always returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL WINAPI DllEntry( HANDLE hInstDll, DWORD  Op, LPVOID lpvReserved )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch(Op)
    {
        case DLL_PROCESS_ATTACH:
            DEBUGMSG(ZONE_INIT, (TEXT("IrFir: DLL Process Attach.\r\n")));
            // Register debug zones
            DEBUGREGISTER((HMODULE)hInstDll);
            // Disables the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT, (TEXT("IrFir: DLL Process Detach.\r\n")));
            break;

        default:
            break;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: DriverEntry
//
//  This function is the entry point of the FIR Miniport NDIS driver.
//
// Parameters:
//      DriverObject
//          [in] .
//      RegistryPath
//          [in] .
//
// Returns:
//
//-----------------------------------------------------------------------------
NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath )
{
    NDIS_STATUS status;
    NDIS_HANDLE hWrapper;
    NDIS50_MINIPORT_CHARACTERISTICS IrFir;

    DEBUGMSG(ZONE_INIT, (TEXT("IrFir: +DriverEntry(0x%.8X, 0x%.8X)\r\n"), DriverObject, RegistryPath));

    // Initialize the wrapper.
    NdisMInitializeWrapper(&hWrapper,   DriverObject, RegistryPath, NULL);

    // Setup the NDIS characteristics of this miniport driver
    NdisZeroMemory( &IrFir, sizeof(IrFir));

    IrFir.Ndis40Chars.Ndis30Chars.MajorNdisVersion = NDIS_MAJOR_VERSION;
    IrFir.Ndis40Chars.Ndis30Chars.MinorNdisVersion = NDIS_MINOR_VERSION;

    IrFir.Ndis40Chars.Ndis30Chars.InitializeHandler = IrFirInitialize;
    IrFir.Ndis40Chars.Ndis30Chars.CheckForHangHandler = IrFirCheckForHang;
    IrFir.Ndis40Chars.Ndis30Chars.DisableInterruptHandler = IrFirDisableInterrupt;
    IrFir.Ndis40Chars.Ndis30Chars.EnableInterruptHandler = IrFirEnableInterrupt;
    IrFir.Ndis40Chars.Ndis30Chars.HaltHandler = IrFirHalt;
    IrFir.Ndis40Chars.Ndis30Chars.HandleInterruptHandler = IrFirHandleInterrupt;
    IrFir.Ndis40Chars.Ndis30Chars.ISRHandler = NULL;
    IrFir.Ndis40Chars.Ndis30Chars.QueryInformationHandler = IrFirQueryInformation;
    IrFir.Ndis40Chars.Ndis30Chars.ReconfigureHandler = NULL;
    IrFir.Ndis40Chars.Ndis30Chars.ResetHandler = IrFirReset;
    IrFir.Ndis40Chars.Ndis30Chars.SendHandler = IrFirSend;
    IrFir.Ndis40Chars.Ndis30Chars.SetInformationHandler = IrFirSetInformation;
    IrFir.Ndis40Chars.Ndis30Chars.TransferDataHandler = NULL;
    IrFir.Ndis40Chars.ReturnPacketHandler =  IrFirReturnPacket;
    IrFir.Ndis40Chars.SendPacketsHandler = NULL;
    IrFir.Ndis40Chars.AllocateCompleteHandler = NULL;

    status = NdisMRegisterMiniport(hWrapper, &IrFir, sizeof(IrFir));

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(ZONE_INIT, (TEXT("IrFir: -DriverEntry [%s]\r\n"), DBG_NDIS_RESULT_STR(status)));
        NdisTerminateWrapper(hWrapper, NULL);
    }

    DEBUGMSG(ZONE_INIT, (TEXT("IrFir: -DriverEntry [%s]\r\n"), DBG_NDIS_RESULT_STR(status)));

    return (status);
}


//-----------------------------------------------------------------------------
//
// Function: IrFirTimer
//
//  This function is to handle the delayed packet sending specified by Irda protocol.
//
// Parameters:
//      SystemSpecific1
//          [in] .
//      FunctionContext
//          [in] .
//      SystemSpecific2
//          [in] .
//      SystemSpecific3
//          [in] .
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID IrFirTimer( IN PVOID SystemSpecific1, IN PVOID FunctionContext, IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3 )
{
    pFirDevice_t thisDev = (pFirDevice_t)FunctionContext;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: +IrFirTimer timeout\r\n")));
    NdisAcquireSpinLock(&thisDev->Lock);
    //Kick off another Tx
    thisDev->IR_VTbl->m_pDelayedSendPacket(thisDev, TRUE);
    NdisReleaseSpinLock(&thisDev->Lock);
}


//-----------------------------------------------------------------------------
//
// Function: ReleaseAdapterResources
//
//  This function release the hardware resources when requested.
//
// Parameters:
//      pvContext
//          [in] .
//
// Returns:
//
//-----------------------------------------------------------------------------
DWORD ReleaseAdapterResources( PVOID pvContext )
{
    pFirDevice_t thisDev = (pFirDevice_t)pvContext;

    DEBUGMSG(ZONE_THREAD, (TEXT("IrFir: +ReleaseAdapterResources(%#x)\r\n"), thisDev));

    // Wait for all packets to be sent and returned.
    for (;;)
    {
        NdisAcquireSpinLock(&thisDev->Lock);

        if (IsListEmpty(&thisDev->SendQueue) == TRUE && thisDev->writePending == FALSE)
            break;

        DEBUGMSG(ZONE_THREAD, (TEXT("IrFir: Release Adpater- I can't release HW. %d\r\n"), thisDev->writePending));
        NdisReleaseSpinLock(&thisDev->Lock);

        Sleep(100);
    }

    // Disable interrupt first
    thisDev->IR_VTbl->m_pDisableInterrupt(thisDev);
    // Need to indicate that we are releasing resources now so the ISR
    // does not try to perform any real operations.
    thisDev->resourcesReleased = TRUE;
    thisDev->IR_VTbl->m_pReleaseAdapterResources(thisDev);

    // Indicate success back to protocol.
    NdisReleaseSpinLock(&thisDev->Lock);
    NdisMQueryInformationComplete(thisDev->ndisAdapterHandle, NDIS_STATUS_SUCCESS);
    DEBUGMSG(ZONE_THREAD, (TEXT("IrFir: -ReleaseAdapterResources(%#x)\r\n"), thisDev));
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: IrFirInitialize
//
//  This function initializes the Fir device.
//
// Parameters:
//      OpenErrorStatus
//          [out] .
//      SelectedMediumIndex
//          [out] .
//      MediumArray
//          [in] .
//      MediumArraySize
//          [in] .
//      MiniportAdapterHandle
//          [in] .
//      WrapperConfigurationContext
//          [in] .
//
// Returns:
//      This function returns the status of initialization.
//
//-----------------------------------------------------------------------------
NDIS_STATUS IrFirInitialize( OUT PNDIS_STATUS OpenErrorStatus, OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray, IN UINT MediumArraySize, IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext )
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    pFirDevice_t thisDev = NULL;
    UINT mediumIndex;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
    UNREFERENCED_PARAMETER(OpenErrorStatus);

    DEBUGMSG(ZONE_INIT, (TEXT("IrFir: +IrFirInitialize()\r\n")));

    hFirset = CreateEvent(NULL, TRUE, FALSE, TEXT("FIR_Setting"));
    if(hFirset == NULL)
    {
        DEBUGMSG(ZONE_INIT, (TEXT("Sir:  CreateEvent hFirset failed!\r\n")));
        return FALSE;
    }

    bFirOn = FALSE;

    // Search the passed-in array of supported media for the IrDA medium.
    for (mediumIndex = 0; mediumIndex < MediumArraySize; mediumIndex++)
    {
        if (MediumArray[mediumIndex] == NdisMediumIrda)
            break;
    }
    if (mediumIndex < MediumArraySize)
    {
        *SelectedMediumIndex = mediumIndex;
    }
    else
    {
        // IrDA medium not found
        DEBUGMSG(ZONE_WARN, (TEXT("IrFir: Didn't see the IRDA medium in IrFirInitialize\r\n")));
        status = NDIS_STATUS_UNSUPPORTED_MEDIA;
        goto done;
    }

    // Allocate a new device object to represent this connection.
    thisDev = NewDevice();

    if(!thisDev)
    {
        status = NDIS_STATUS_NOT_ACCEPTED;
        goto done;
    }

    NdisAcquireSpinLock(&thisDev->Lock);

    // CE - calls NdisMInitializeTimer instead of NdisInitializeTimer. This
    //      requires the NdisAdapterHandle. Else the timer is initialized
    //      in InitDevice.
    NdisMInitializeTimer(&thisDev->TurnaroundTimer, MiniportAdapterHandle, IrFirTimer, thisDev);

    NdisReleaseSpinLock(&thisDev->Lock);

    //Allocate resources for this connection.
    if (!OpenDevice(thisDev))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: OpenDevice failed\r\n")));
        status = NDIS_STATUS_FAILURE;
        goto done;
    }

    //  This call will associate our adapter handle with the wrapper's
    //  adapter handle.  The wrapper will then always use our handle
    //  when calling us.  We use a pointer to the device object as the context.
    NdisMSetAttributesEx(
        MiniportAdapterHandle,
        (NDIS_HANDLE)thisDev,
        0,    // Default check for hang timeout
        NDIS_ATTRIBUTE_DESERIALIZE,
        NdisInterfaceInternal);

    //  Record the NDIS wrapper's handle for this adapter, which we use
    //  when we call up to the wrapper.
    //  (This miniport's adapter handle is just thisDev, the pointer to the device object.).
    // Set NDIS wrapper context for this miniport.
    thisDev->ndisAdapterHandle = MiniportAdapterHandle;

    // Register an FIR interrupt with NDIS.
    status = NdisMRegisterInterrupt(
        (PNDIS_MINIPORT_INTERRUPT)&(thisDev->FirinterruptObj),
        MiniportAdapterHandle,
        thisDev->sysIntrFir,
        NULL,
        FALSE,   // reqeust ISR
        FALSE,   // share interrupts
        NdisInterruptLatched);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: NdisMRegisterInterrupt failed, the error is %d\r\n"), GetLastError()));
        status = NDIS_STATUS_FAILURE;
        goto done;
    }

    // Register an SIR interrupt with NDIS.
    status = NdisMRegisterInterrupt(
        (PNDIS_MINIPORT_INTERRUPT)&(thisDev->SirinterruptObj),
        MiniportAdapterHandle,
        thisDev->sysIntrSir,
        NULL,
        FALSE,   // reqeust ISR
        FALSE,   // share interrupts
        NdisInterruptLatched);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: NdisMRegisterInterrupt failed, the error is %d\r\n"), GetLastError()));
        status = NDIS_STATUS_FAILURE;
        goto done;
    }
    thisDev->linkSpeedInfo = NULL;
    thisDev->newSpeed = DEFAULT_BAUD_RATE;
    if (!SetSpeed(thisDev))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: SetSpeed failed!\r\n")));
        status = NDIS_STATUS_FAILURE;
        goto done;
    }

done:
    if (status == NDIS_STATUS_SUCCESS)
    {
        // Add this device object to the beginning of our global list.
        thisDev->next = gFirstFirDevice;
        gFirstFirDevice = thisDev;

        // For WinCE, we release the resources and they will be reclaimed,
        // when OID_IRDA_REACQUIRE_HW_RESOURCES msg is recvd.
        thisDev->resourcesReleased=TRUE;

        NdisReleaseSpinLock(&thisDev->Lock);
        DEBUGMSG(ZONE_INIT, (TEXT("IrFir: IrFirInitialize succeeded\r\n")));
    }
    else
    {
        if (thisDev)
        {
            FreeDevice(thisDev);
            thisDev->resourcesReleased=TRUE;
        }
        DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: IrFirInitialize failed\r\n")));
    }

    return status;
}


//-----------------------------------------------------------------------------
//
// Function: IrFirCheckForHang
//
//  This function reports the state of the Fir device.
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//
// Returns:
//      This function returns TRUE if there is a error in device.
//
//-----------------------------------------------------------------------------
BOOLEAN IrFirCheckForHang( IN NDIS_HANDLE MiniportAdapterContext )
{
    BOOLEAN result = FALSE;
    pFirDevice_t thisDev = (pFirDevice_t)MiniportAdapterContext;
    DWORD clock_result;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: +IrFirCheckForHang(0x%x)\r\n"),(UINT)MiniportAdapterContext));

    NdisAcquireSpinLock(&thisDev->Lock);

    //Check whether the hFirset is set , in order to determine the uart clock status
    clock_result = WaitForSingleObject(hFirset,0);

    //If current uart clock is open, only check whether the hFirset is reset,
    //in order to check whether need to change uart clock status to close.
    if(bFirOn == TRUE)
    {
        if(clock_result != WAIT_OBJECT_0)
        {
            DEBUGMSG(ZONE_FUNCTION,(TEXT("FIR ON->OFF")));
            bFirOn = FALSE;
            thisDev->newSpeed = DEFAULT_BAUD_RATE;
            SetSpeed(thisDev);
        }
    }
    //If current uart clock is close, only check whether the hFirset is set,
    //in order to check whether need to change uart clock status to open.
    else
    {
        if(clock_result == WAIT_OBJECT_0)
        {
            DEBUGMSG(ZONE_FUNCTION,(TEXT("FIR OFF->ON")));
            bFirOn = TRUE;
            BSPUartEnableClock(thisDev->SirPhyAddr, TRUE);
        }
    }

    if(thisDev->HangChk)
    {
        PLIST_ENTRY ListEntry;
        DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: Driver Hang is detected for %d times!!!\r\n"),thisDev->HangChk));

        // Free all resources for the SEND buffer queue.
        while ((ListEntry = MyRemoveHeadList(&thisDev->SendQueue)) != 0)
        {
            PNDIS_PACKET Packet = CONTAINING_RECORD(ListEntry,
                NDIS_PACKET,
                MiniportReserved);
            NdisReleaseSpinLock(&thisDev->Lock);
            NdisMSendComplete(thisDev->ndisAdapterHandle, Packet, NDIS_STATUS_FAILURE);
            NdisAcquireSpinLock(&thisDev->Lock);
        }

        thisDev->HangChk = FALSE;
        result = TRUE;
    }
    NdisReleaseSpinLock(&thisDev->Lock);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: -IrFirCheckForHang(0x%x)\r\n"),(UINT)MiniportAdapterContext));

    return result;
}


//-----------------------------------------------------------------------------
//
// Function: IrFirDisableInterrupt
//
//  This function disables interrupts.
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID IrFirDisableInterrupt( IN NDIS_HANDLE MiniportAdapterContext )
{
    pFirDevice_t thisDev = (pFirDevice_t)(MiniportAdapterContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: +IrFirDisableInterrupt(0x%x)\r\n"), GetLastError()));

    NdisAcquireSpinLock(&thisDev->Lock);

    if (!thisDev->resourcesReleased)
        thisDev->IR_VTbl->m_pDisableInterrupt(thisDev);
    else
        DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: Error operation on disable interrupt.\r\n")));

    NdisReleaseSpinLock(&thisDev->Lock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: -IrFirDisableInterrupt(0x%x)\r\n"),(UINT)MiniportAdapterContext));
}


//-----------------------------------------------------------------------------
//
// Function: IrFirEnableInterrupt
//
//  This function enables interrupts.
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID IrFirEnableInterrupt( IN NDIS_HANDLE MiniportAdapterContext )
{
    pFirDevice_t thisDev = (pFirDevice_t)(MiniportAdapterContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: +IrFirEnableInterrupt(0x%x)\r\n"),(UINT)MiniportAdapterContext));

    NdisAcquireSpinLock(&thisDev->Lock);

    if (!thisDev->resourcesReleased)
        thisDev->IR_VTbl->m_pEnableInterrupt(thisDev);
    else
        DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: Error operation on enable interrupt.\r\n")));

    NdisReleaseSpinLock(&thisDev->Lock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: -IrFirEnableInterrupt(0x%x)\r\n"),(UINT)MiniportAdapterContext));
}


//-----------------------------------------------------------------------------
//
// Function: IrFirHalt
//
//  This function Halts the Fir device.
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID IrFirHalt( IN NDIS_HANDLE MiniportAdapterContext )
{
    pFirDevice_t thisDev = (pFirDevice_t)(MiniportAdapterContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: +IrFirHalt(0x%.8X)\r\n"), (UINT)MiniportAdapterContext));

    // Remove this device from our global list
    if (thisDev == gFirstFirDevice)
        gFirstFirDevice = gFirstFirDevice->next;
    else
    {
        pFirDevice_t dev;

        for (dev = gFirstFirDevice; dev && (dev->next != thisDev); dev = dev->next);

        if (dev)
            dev->next = dev->next->next;
        else
        {
            // Don't omit this error check.  I've seen NDIS call
            // MiniportHalt with a bogus context when the system
            // gets corrupted.
            DEBUGMSG(ZONE_WARN, (TEXT("IrFir: Bad context in IrFirHalt\r\n")));
            return;
        }
    }

    // Now destroy the device object.
    NdisAcquireSpinLock(&thisDev->Lock);
    thisDev->IR_VTbl->m_pReleaseAdapterResources(thisDev);
    NdisReleaseSpinLock(&thisDev->Lock);
    //SirClose(thisDev);
    FreeDevice(thisDev);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: -IrFirHalt(0x%x)\r\n"), (UINT)MiniportAdapterContext));
}


//-----------------------------------------------------------------------------
//
// Function: IrFirHandleInterrupt
//
//  This function is the interrupt processing routine.
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID IrFirHandleInterrupt( IN NDIS_HANDLE MiniportAdapterContext )
{
    pFirDevice_t thisDev = (pFirDevice_t)MiniportAdapterContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: +IrFirHandleInterrupt(0x%x)\r\n"), (UINT)MiniportAdapterContext));

    NdisAcquireSpinLock(&thisDev->Lock);
    if (thisDev->resourcesReleased)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: IrFirHandleInterrupt, blow off, no resources!\r\n")));
    }
    else
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: -IrFirHandleInterrupt()\r\n")));
        thisDev->IR_VTbl->m_pInterruptHandler(thisDev);
    }
    NdisReleaseSpinLock(&thisDev->Lock);
}


//-----------------------------------------------------------------------------
//
// Function: IrFirQueryInformation
//
//  This function queries the capabilities and status of the miniport driver.
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//      Oid
//          [in] .
//      InformationBuffer
//          [in] .
//      InformationBufferLength
//          [in] .
//      BytesWritten
//          [in] .
//      BytesNeeded
//          [in] .
//
// Returns:
//      This function returns status of query.
//
//-----------------------------------------------------------------------------
NDIS_STATUS IrFirQueryInformation( IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid, IN PVOID InformationBuffer, IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten, OUT PULONG BytesNeeded )
{
    NDIS_STATUS result = NDIS_STATUS_SUCCESS;
    pFirDevice_t thisDev = (pFirDevice_t)MiniportAdapterContext;
    NDIS_PNP_CAPABILITIES  FirPMCapabilities;
    UINT i, speeds;
    UINT *infoPtr;

    DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: +IrFirQueryInformation(%d)\r\n"), Oid));

    if (InformationBufferLength >= sizeof(int))
    {
        NdisAcquireSpinLock(&thisDev->Lock);

        switch (Oid)
        {
            case OID_GEN_SUPPORTED_LIST:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_GEN_SUPPORTED_LIST)\r\n")));
                *BytesWritten = sizeof(v_rgSupportedOids);
                NdisMoveMemory(InformationBuffer, (PVOID)v_rgSupportedOids, sizeof(v_rgSupportedOids));
                *BytesNeeded = 0;
                break;

            case OID_GEN_VENDOR_DRIVER_VERSION:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_GEN_VENDOR_DRIVER_VERSION)\r\n")));
                *(UINT *)InformationBuffer = VENDOR_DRIVER_VERSION;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_IRDA_RECEIVING:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_RECEIVING)\r\n")));
                *(UINT *)InformationBuffer = (UINT)thisDev->nowReceiving;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_IRDA_MAX_RECEIVE_WINDOW_SIZE:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_MAX_RECEIVE_WINDOW_SIZE)\r\n")));
                *(UINT *)InformationBuffer = MAXIMUM_RCV_PACKETS;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_IRDA_SUPPORTED_SPEEDS:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_SUPPORTED_SPEEDS)\r\n")));
                speeds = CURRENT_SUPPORTED_SPEEDS & ALL_IRDA_SPEEDS;
                *BytesWritten = 0;

                for (i = 0, infoPtr = (PUINT)InformationBuffer; (i < NUM_BAUDRATES) && speeds &&
                    (InformationBufferLength >= sizeof(UINT)); i++)
                {
                    if (supportedBaudRateTable[i].ndisCode & speeds)
                    {
                        *infoPtr++ = supportedBaudRateTable[i].bitsPerSec;
                        InformationBufferLength -= sizeof(UINT);
                        *BytesWritten += sizeof(UINT);
                        speeds &= ~supportedBaudRateTable[i].ndisCode;
                        DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir:  - supporting speed %d bps\r\n"), supportedBaudRateTable[i].bitsPerSec));
                    }
                }

                // We ran out of room in InformationBuffer.
                // Count the remaining number of bits set in speeds
                // to figure out the number of remaining bytes needed.
                if (speeds)
                {
                    for (*BytesNeeded = 0; speeds; *BytesNeeded += sizeof(UINT))
                        speeds &= (speeds - 1); //This instruction clears the lowest set bit in speeds
                    *BytesWritten = 0;
                    result = NDIS_STATUS_INVALID_LENGTH;
                }
                else
                {
                    result = NDIS_STATUS_SUCCESS;
                    *BytesNeeded = 0;
                }
                break;

            case OID_IRDA_LINK_SPEED:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_LINK_SPEED, %d)\r\n"),thisDev->linkSpeedInfo->bitsPerSec));
                if (thisDev->linkSpeedInfo)
                    *(UINT *)InformationBuffer = thisDev->linkSpeedInfo->bitsPerSec;
                else 
                    *(UINT *)InformationBuffer = DEFAULT_BAUD_RATE;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_IRDA_MEDIA_BUSY:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_MEDIA_BUSY)\r\n")));
                *(UINT *)InformationBuffer = (UINT)thisDev->mediaBusy;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_IRDA_RELEASE_HW_RESOURCES:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_RELEASE_HW_RESOURCES)\r\n")));
                if (thisDev->resourcesReleased == TRUE)
                {
                    *BytesWritten = 0;
                    result = NDIS_STATUS_FAILURE;
                    DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: FIRI Query: resources already released!!!\r\n")));
                }
                else
                {
                    HANDLE hRelThread;
                    DWORD  dwRelThreadId;

                    *BytesWritten = sizeof(UINT);

                    hRelThread = CreateThread(NULL, 0, ReleaseAdapterResources, thisDev, 0, &dwRelThreadId);

                    if (hRelThread == NULL)
                        result = NDIS_STATUS_RESOURCES;
                    else
                    {
                        CloseHandle(hRelThread);
                        result = NDIS_STATUS_PENDING;
                    }
                }
                break;

            case OID_GEN_MAXIMUM_LOOKAHEAD:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_GEN_MAXIMUM_LOOKAHEAD)\r\n")));
                *(UINT *)InformationBuffer = MAXIMUM_LOOKAHEAD;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_GEN_MAC_OPTIONS:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_GEN_MAC_OPTIONS)\r\n")));
                *(UINT *)InformationBuffer = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | NDIS_MAC_OPTION_TRANSFERS_NOT_PEND;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_GEN_MAXIMUM_SEND_PACKETS:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_GEN_MAXIMUM_SEND_PACKETS)\r\n")));
                *(UINT *)InformationBuffer = MAXIMUM_SEND_PACKETS;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_IRDA_TURNAROUND_TIME:
                // Indicate the amount of time that the transceiver needs
                // to recuperate after a send.
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_TURNAROUND_TIME)\r\n")));
                *(UINT *)InformationBuffer = DEFAULT_TURNAROUND_TIME;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_IRDA_EXTRA_RCV_BOFS:
                //  Pass back the number of _extra_ BOFs to be prepended
                //  to packets sent to this unit at 115.2 baud, the
                //  maximum Slow IR speed.  This will be scaled for other
                //  speed according to the table in the
                //  'Infrared Extensions to NDIS' spec.
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_EXTRA_RCV_BOFS)\r\n")));
                *(UINT *)InformationBuffer = DEFAULT_EXTRA_BOF_REQUIRED;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_GEN_CURRENT_PACKET_FILTER:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_GEN_CURRENT_PACKET_FILTER)\r\n")));
                *(UINT *)InformationBuffer = NDIS_PACKET_TYPE_PROMISCUOUS;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_GEN_MAXIMUM_FRAME_SIZE:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_GEN_MAXIMUM_FRAME_SIZE)\r\n")));
                *(UINT *)InformationBuffer = MAX_IR_FRAME_SIZE;
                *BytesWritten = sizeof(UINT);
                *BytesNeeded = 0;
                break;

            case OID_IRDA_UNICAST_LIST:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_UNICAST_LIST)\r\n")));
                break;

            case OID_IRDA_MAX_UNICAST_LIST_SIZE:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_MAX_UNICAST_LIST_SIZE)\r\n")));
                break;
 
            case OID_IRDA_RATE_SNIFF:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_IRDA_RATE_SNIFF)\r\n")));
                break;

            // Power management OID's
            case OID_PNP_CAPABILITIES:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_PNP_CAPABILITIES)\r\n")));

                // Advertise our powermanagement capabilities
                // we return by telling wake up is not supported, since iMx51 BSP supports only D0 and D4.           
                FirPMCapabilities.Flags = 0 ;
                FirPMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
                FirPMCapabilities.WakeUpCapabilities.MinPatternWakeUp = NdisDeviceStateUnspecified;
                FirPMCapabilities.WakeUpCapabilities.MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;
                InformationBufferLength = sizeof (FirPMCapabilities);
                InformationBuffer = (PVOID)&FirPMCapabilities;
                *BytesWritten = sizeof(NDIS_PNP_CAPABILITIES);
                *BytesNeeded = 0;
                result = NDIS_STATUS_SUCCESS;
                break;

            case OID_PNP_QUERY_POWER:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(OID_PNP_QUERY_POWER)\r\n")));
                DEVICE_POWER_STATE  DevicePowerState;
                DevicePowerState = *(PDEVICE_POWER_STATE)InformationBuffer;
                DEBUGMSG (ZONE_QUERYINFO, 
                    (TEXT("IrFir: IrFirQueryInformation OID_PNP_QUERY_POWER for [%s].\r\n"),
                        (DevicePowerState == NdisDeviceStateD0) ? TEXT("NdisDeviceStateD0"): 
                        (DevicePowerState == NdisDeviceStateD1) ? TEXT("NdisDeviceStateD1"): 
                        (DevicePowerState == NdisDeviceStateD2) ? TEXT("NdisDeviceStateD2"): 
                        (DevicePowerState == NdisDeviceStateD3) ? TEXT("NdisDeviceStateD3"): 
                        TEXT("UNKNOWN")));
                *BytesWritten = 0;
                *BytesNeeded = 0;
                result = NDIS_STATUS_SUCCESS;
                break;

            default:
                DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation(0x%x), unsupported OID\r\n"), Oid));
                result = NDIS_STATUS_NOT_SUPPORTED;
                break;
        }

        NdisReleaseSpinLock(&thisDev->Lock);
    }
    else
    {
        *BytesNeeded = sizeof(UINT) - InformationBufferLength;
        *BytesWritten = 0;
        result = NDIS_STATUS_INVALID_LENGTH;
    }

    DEBUGMSG(ZONE_QUERYINFO, (TEXT("IrFir: IrFirQueryInformation succeeded (info <- %d)\r\n"), *(UINT *)InformationBuffer));
    return result;
}


//-----------------------------------------------------------------------------
//
// Function: IrFirReset
//
//  This function retrieves the current
//  properties of the communications device.
//
// Parameters:
//      AddressingReset
//          [in] .
//      MiniportAdapterContext
//          [in] .
//
// Returns:
//      This function returns status of reset.
//
//-----------------------------------------------------------------------------
NDIS_STATUS IrFirReset( PBOOLEAN AddressingReset, NDIS_HANDLE MiniportAdapterContext )
{
    pFirDevice_t dev, thisDev = (pFirDevice_t)MiniportAdapterContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: IrFirReset(0x%x)\r\n"), (UINT)MiniportAdapterContext));

    //  Verify that the context is not bogus.
    //  I've seen bad contexts getting passed in when the system gets corrupted.
    for (dev = gFirstFirDevice; dev && (dev != thisDev); dev = dev->next);

    if (!dev)
    {
        DEBUGMSG(ZONE_WARN, (TEXT("IrFir: Bad context in IrFirReset\r\n")));
        return NDIS_STATUS_FAILURE;
    }

    NdisAcquireSpinLock(&thisDev->Lock);
    thisDev->IR_VTbl->m_pReleaseAdapterResources(thisDev);
    //thisDev->linkSpeedInfo->bitsPerSec = thisDev->newSpeed;       // make sure we are resuming from last break point
    thisDev->IR_VTbl->m_pAcquireAdapterResources(thisDev);
    thisDev->IR_VTbl->m_pSetupRecv(thisDev);
    NdisReleaseSpinLock(&thisDev->Lock);

    *AddressingReset = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: IrFirReset done.\r\n")));
    return NDIS_STATUS_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: IrFirSend
//
//  This function Transmits a packet onto the medium..
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//      Packet
//          [in] .
//      Flags
//          [in] .
//
// Returns:
//      This function returns the status of sending the packet.
//
//-----------------------------------------------------------------------------
NDIS_STATUS IrFirSend( IN NDIS_HANDLE MiniportAdapterContext, IN PNDIS_PACKET Packet, IN UINT Flags )
{
    NDIS_STATUS stat;
    pFirDevice_t thisDev = (pFirDevice_t)MiniportAdapterContext;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Flags);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: +IrFirSend(0x%x, packet = 0x%x)\r\n"), (UINT)MiniportAdapterContext, Packet));

    //  If we have temporarily lost access to the hardware, don't queue up any sends.
    //  return failure until we regain access to the hw.
    NdisAcquireSpinLock(&thisDev->Lock);

    if (thisDev->resourcesReleased)
    {
        NdisReleaseSpinLock(&thisDev->Lock);
        ASSERT(FALSE);
        return (NDIS_STATUS_FAILURE);
    }

    stat = thisDev->IR_VTbl->m_pSendPacket(thisDev, Packet);

    NdisReleaseSpinLock(&thisDev->Lock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: -IrFirSend [%s]\r\n"), DBG_NDIS_RESULT_STR(stat)));
    return (stat);
}


//-----------------------------------------------------------------------------
//
// Function: IrFirSetInformation
//
//  This function allows other layers of the network software (e.g., a transport driver) to control
//  the miniport driver by changing information that the miniport driver maintains in its OIDs,
//  such as the packet filters or multicast addresses.
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//      Oid
//          [in] .
//      InformationBuffer
//          [in] .
//      InformationBufferLength
//          [in] .
//      BytesRead
//          [in] .
//      BytesNeeded
//          [in] .
//
// Returns:
//      This function returns status of the action.
//
//-----------------------------------------------------------------------------
NDIS_STATUS IrFirSetInformation( IN NDIS_HANDLE MiniportAdapterContext, IN NDIS_OID Oid, IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength, OUT PULONG BytesRead, OUT PULONG BytesNeeded )
{
    NDIS_STATUS result = NDIS_STATUS_SUCCESS;
    pFirDevice_t thisDev = (pFirDevice_t)MiniportAdapterContext;

    NDIS_DEVICE_POWER_STATE     NewPowerState;  // Power state
    DEBUGMSG(ZONE_SETINFO, (TEXT("IrFir: +IrFirSetInformation(%d)\r\n"), Oid));

    if (InformationBufferLength >= sizeof(UINT))
    {
        UINT info;
        *BytesRead = sizeof(UINT);
        *BytesNeeded = 0;

        NdisAcquireSpinLock(&thisDev->Lock);

        switch (Oid)
        {
            case OID_IRDA_LINK_SPEED:
                info = *(UINT *)InformationBuffer;
                DEBUGMSG(ZONE_SETINFO, (TEXT("IrFir: IrFirSetInformation(OID_IRDA_LINK_SPEED, %d)\r\n"), info));
                result = NDIS_STATUS_INVALID_DATA;

                for (int i = 0; i < NUM_BAUDRATES; i++)
                {
                    if (supportedBaudRateTable[i].bitsPerSec == info)
                    {
                        thisDev->newSpeed = supportedBaudRateTable[i].bitsPerSec;
                        result = NDIS_STATUS_SUCCESS;
                        break;
                    }
                }
                if (result == NDIS_STATUS_SUCCESS)
                {
                    if (!SetSpeed(thisDev))
                        result = NDIS_STATUS_FAILURE;
                }
                else
                {
                    result = NDIS_STATUS_FAILURE;
                    *BytesRead = 0;
                    *BytesNeeded = 0;
                }
                break;

            case OID_IRDA_MEDIA_BUSY:
                info = *(UINT *)InformationBuffer;
                DEBUGMSG(ZONE_SETINFO, (TEXT("IrFir: IrFirSetInformation(OID_IRDA_MEDIA_BUSY, %xh)\r\n"), info));

                //  The protocol can use this OID to reset the busy field
                //  in order to check it later for intervening activity.
                thisDev->mediaBusy = (BOOLEAN)info;
                result = NDIS_STATUS_SUCCESS;
                break;

            case OID_IRDA_REACQUIRE_HW_RESOURCES:
                DEBUGMSG(ZONE_SETINFO, (TEXT("IrFir: IRDA: Set(OID_IRDA_REACQUIRE_HW_RESOURCES)\r\n")));
                *BytesNeeded = 0;

                if (thisDev->resourcesReleased == FALSE)
                {
                    DEBUGMSG(ZONE_WARN, (TEXT("IrFir: IRDA: resources already acquired!!\r\n")));
                    result = NDIS_STATUS_FAILURE;
                }
                else
                {
                    if (thisDev->IR_VTbl->m_pAcquireAdapterResources(thisDev))
                    {
                        // Reset state.
                        thisDev->portInfo.rcvState = STATE_INIT;
                        thisDev->writePending = FALSE;
                        thisDev->resourcesReleased = FALSE;
                        // Setup receive
                        thisDev->IR_VTbl->m_pSetupRecv(thisDev);
                    }
                    else
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: IRDA: initialization failure.\r\n")));
                        result = NDIS_STATUS_FAILURE;
                    }
                }
                break;

            case OID_GEN_CURRENT_PACKET_FILTER:
                DEBUGMSG(ZONE_SETINFO, (TEXT("IrFir: IrFirSetInformation(OID_GEN_CURRENT_PACKET_FILTER)\r\n")));
                result = NDIS_STATUS_SUCCESS;
                break;

            // Power management related OID's
            case OID_PNP_SET_POWER:
                DEBUGMSG(ZONE_SETINFO, (TEXT("IrFir: IrFirSetInformation(OID_PNP_SET_POWER)\r\n")));
                if (InformationBufferLength != sizeof(NDIS_DEVICE_POWER_STATE ))
                {
                    NdisReleaseSpinLock(&thisDev->Lock);
                    return(NDIS_STATUS_INVALID_LENGTH);
                }
                NewPowerState = *(PNDIS_DEVICE_POWER_STATE)InformationBuffer;
                if(NewPowerState != NdisDeviceStateD0)
                    NewPowerState = NdisDeviceStateD3; 
                // Set the power state - Cannot fail this request
                if(NewPowerState == NdisDeviceStateD0)
                {
                    // Power ON
                    if (thisDev->IR_VTbl->m_pAcquireAdapterResources(thisDev))
                    {
                        // Reset state.
                        thisDev->portInfo.rcvState = STATE_INIT;
                        thisDev->writePending = FALSE;
                        thisDev->resourcesReleased = FALSE;
                        // Setup receive
                        thisDev->IR_VTbl->m_pSetupRecv(thisDev);
                    }
                }
                else
                {
                    // Power OFF
                    HANDLE hRelThread;
                    DWORD  dwRelThreadId;

                    hRelThread = CreateThread(NULL, 0, ReleaseAdapterResources, thisDev, 0, &dwRelThreadId);
                    
                    if (hRelThread == NULL)
                        result = NDIS_STATUS_RESOURCES;
                    else
                    {
                        CloseHandle(hRelThread);
                        result = NDIS_STATUS_PENDING;
                    }
                }
                *BytesRead = sizeof(NDIS_DEVICE_POWER_STATE);
                result = NDIS_STATUS_SUCCESS; 
                break;
            // we dont need to support these                
            case OID_PNP_ADD_WAKE_UP_PATTERN:
            case OID_PNP_REMOVE_WAKE_UP_PATTERN:
            case OID_PNP_ENABLE_WAKE_UP:
            // We don't support these
            case OID_IRDA_RATE_SNIFF:
            case OID_IRDA_UNICAST_LIST:

            // These are query-only parameters.
            case OID_IRDA_SUPPORTED_SPEEDS:
            case OID_IRDA_MAX_UNICAST_LIST_SIZE:
            case OID_IRDA_TURNAROUND_TIME:

            default:
                DEBUGMSG(ZONE_SETINFO, (TEXT("IrFir: IrFirSetInformation(OID=%d=0x%x) - unsupported OID\r\n"), Oid, Oid));
                *BytesRead = 0;
                *BytesNeeded = 0;
                result = NDIS_STATUS_NOT_SUPPORTED;
                break;
        }

        NdisReleaseSpinLock(&thisDev->Lock);
    }
    else
    {
        *BytesRead = 0;
        *BytesNeeded = sizeof(UINT);
        result = NDIS_STATUS_INVALID_LENGTH;
    }

    if(result == NDIS_STATUS_SUCCESS)
        DEBUGMSG(ZONE_SETINFO, (TEXT("IrFir: IrFirSetInformation succeeded\r\n")));
    else
        DEBUGMSG(ZONE_WARN, (TEXT("IrFir: IrFir: IrFirSetInformation failed\r\n")));

    return result;
}


//-----------------------------------------------------------------------------
//
// Function: IrFirReturnPacket
//
//  When NdisMIndicateReceivePacket returns asynchronously, the protocol returns ownership
//  of the packet to the miniport via this function.
//
// Parameters:
//      MiniportAdapterContext
//          [in] .
//      Packet
//          [in] .
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID IrFirReturnPacket( NDIS_HANDLE MiniportAdapterContext, PNDIS_PACKET Packet )
{
    pFirDevice_t thisDev = (pFirDevice_t)MiniportAdapterContext;
    rcvBuffer *rcvBuf;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: IrFirReturnPacket(0x%x, packet = 0x%x)\r\n"), (UINT)MiniportAdapterContext, Packet));

    NdisAcquireSpinLock(&thisDev->Lock);

    // MiniportReserved contains the pointer to our rcvBuffer
    rcvBuf = *(rcvBuffer**) Packet->MiniportReserved;

    if (rcvBuf->state == STATE_PENDING)
    {
        PNDIS_BUFFER ndisBuf;

        DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: Reclaimed rcv packet 0x%x.\r\n"), (UINT)Packet));
        NdisUnchainBufferAtFront(Packet, &ndisBuf);

        if (ndisBuf)
            NdisFreeBuffer(ndisBuf);

        if (!rcvBuf->isDmaBuf) 
        {
            // At SIR speeds, we manage a group of buffers that
            // we keep on the rcvBufBuf queue.
            InsertTailList(&thisDev->rcvBufBuf, RCV_BUF_TO_LIST_ENTRY(rcvBuf->dataBuf));
        }

        rcvBuf->dataBuf = NULL;
        rcvBuf->state = STATE_FREE;
        rcvBuf->isDmaBuf = FALSE;

        RemoveEntryList(&rcvBuf->listEntry);
        InsertHeadList(&thisDev->rcvBufFree, &rcvBuf->listEntry);
    }
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: Packet in IrFirReturnPacket was not PENDING\r\n")));

    NdisReleaseSpinLock(&thisDev->Lock);
}


//-----------------------------------------------------------------------------
//
// Function: DeliverFullBuffers
//
//  This function delivers received packets to the protocol.
//
// Parameters:
//      thisDev
//          [in] pointer to Fir Device structure.
//
// Returns:
//      This function returns TRUE if delivered at least one frame
//
//-----------------------------------------------------------------------------
// Remove-Prefast: Warning 28167 workaround
PREFAST_SUPPRESS(28167,"This Warning can be skipped!")
BOOLEAN DeliverFullBuffers( pFirDevice_t thisDev )
{
    BOOLEAN result = FALSE;
    PLIST_ENTRY ListEntry;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: +DeliverFullBuffers(0x%x)\r\n"), (UINT) thisDev));

    // Deliver all full rcv buffers
    for (ListEntry = MyRemoveHeadList(&thisDev->rcvBufFull); ListEntry;
        ListEntry = MyRemoveHeadList(&thisDev->rcvBufFull))
    {
        rcvBuffer *rcvBuf = CONTAINING_RECORD(ListEntry, rcvBuffer, listEntry);

        NDIS_STATUS stat;
        PNDIS_BUFFER packetBuf;
        SLOW_IR_FCS_TYPE fcs;

        if (thisDev->linkSpeedInfo->bitsPerSec <= MAX_SIR_SPEED)
        {
            // The packet we have already has had BOFs,
            // EOF, and * escape-sequences removed.  It
            // contains an FCS code at the end, which we
            // need to verify and then remove before
            // delivering the frame.  We compute the FCS
            // on the packet with the packet FCS attached;
            // this should produce the constant value
            // GOOD_FCS.
            fcs = ComputeFCS(rcvBuf->dataBuf, rcvBuf->dataLen);

            if (fcs != GOOD_FCS)
            {
                //  FCS Error.  Drop this frame.
                DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: Bad FCS in DeliverFullBuffers bad != good 0x%x!=0x%x.\r\n"), (UINT)fcs, (UINT) GOOD_FCS));
                rcvBuf->state = STATE_FREE;

                DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: Dropped pkts; BAD FCS (%xh!=%xh):\r\n"), fcs, GOOD_FCS));

                if (!rcvBuf->isDmaBuf) 
                {
                    InsertTailList(&thisDev->rcvBufBuf, RCV_BUF_TO_LIST_ENTRY(rcvBuf->dataBuf));
                }
                rcvBuf->dataBuf = NULL;
                rcvBuf->isDmaBuf = FALSE;
                InsertHeadList(&thisDev->rcvBufFree, &rcvBuf->listEntry);
                //break;
                continue;
            }

            // Remove the FCS from the end of the packet
            rcvBuf->dataLen -= SLOW_IR_FCS_SIZE;
        }

        // The packet array is set up with its NDIS_PACKET.
        // Now we need to allocate a single NDIS_BUFFER for
        // the NDIS_PACKET and set the NDIS_BUFFER to the
        // part of dataBuf that we want to deliver.
        NdisAllocateBuffer(&stat, &packetBuf, thisDev->bufferPoolHandle,
            (PVOID)rcvBuf->dataBuf, rcvBuf->dataLen);

        if (stat != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("IrFir: NdisAllocateBuffer failed\r\n")));
            break;
        }

        NdisChainBufferAtFront(rcvBuf->packet, packetBuf);

        // Fix up some other packet fields.
        NDIS_SET_PACKET_HEADER_SIZE(rcvBuf->packet, IR_ADDR_SIZE+IR_CONTROL_SIZE);

        DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: Indicating rcv packet 0x%x.\r\n"), (UINT)rcvBuf->packet));

        // Indicate to the protocol that another packet is
        // ready.  Set the rcv buffer's state to PENDING first
        // to avoid a race condition with NDIS's call to the
        // return packet handler.
        rcvBuf->state = STATE_PENDING;

        *(rcvBuffer **)rcvBuf->packet->MiniportReserved = rcvBuf;
        InsertBufferSorted(&thisDev->rcvBufPend, rcvBuf);

        NdisReleaseSpinLock(&thisDev->Lock);
        NdisMIndicateReceivePacket(thisDev->ndisAdapterHandle, &rcvBuf->packet, 1);
        NdisAcquireSpinLock(&thisDev->Lock);

        result = TRUE;

        stat = NDIS_GET_PACKET_STATUS(rcvBuf->packet);

        if (stat == NDIS_STATUS_PENDING)
        {
            // The packet is being delivered asynchronously.
            // Leave the rcv buffer's state as PENDING;
            // we'll get a callback when the transfer is
            // complete.  Do NOT step firstRcvBufIndex.
            // We don't really need to break out here,
            // but we will anyways just to make things
            // simple.  This is ok since we get this
            // deferred interrupt callback for each packet
            // anyway.  It'll give the protocol a chance
            // to catch up.
            DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: DeliverFullBuffers, Rcv Pending. Rcvd packets\r\n")));
        }
        else
        {
            // If there was an error, we are dropping this
            // packet; otherwise, this packet was delivered
            // synchronously.  We can free the packet
            // buffer and make this rcv frame available.
            RemoveEntryList(&rcvBuf->listEntry);

            NdisUnchainBufferAtFront(rcvBuf->packet, &packetBuf);
            if (packetBuf)
                NdisFreeBuffer(packetBuf);

            rcvBuf->state = STATE_FREE;

            if (!rcvBuf->isDmaBuf) 
            {
                // At SIR speeds, we manage a group of buffers that
                // we keep on the rcvBufBuf queue.
                InsertTailList(&thisDev->rcvBufBuf, RCV_BUF_TO_LIST_ENTRY(rcvBuf->dataBuf));
            }
            rcvBuf->dataBuf = NULL;
            rcvBuf->isDmaBuf = FALSE;

            InsertHeadList(&thisDev->rcvBufFree, &rcvBuf->listEntry);

            if (stat == NDIS_STATUS_SUCCESS)
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: DeliverFullBuffers, Rcvd packets\r\n")));
            }
            else
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: DeliverFullBuffers, Dropped rcv packets.\r\n")));
            }
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IrFir: -DeliverFullBuffers\r\n")));
    return result;
}
