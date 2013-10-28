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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  fec.c
//
//  Implementation of FEC Driver
//
//  This file implements NDIS interface for FEC.
//
//-----------------------------------------------------------------------------
#include "precomp.h"
//#include "fec.h"

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Global var of FEC Ndis Handler
NDIS_HANDLE     NdisMiniportDriverHandle;
NDIS_HANDLE     MiniportDriverContext = NULL;
PCSP_FEC_REGS   gpFECReg = NULL;

extern VOID NICIssueFullReset(PMP_ADAPTER Adapter);
extern NDIS_MEDIA_CONNECT_STATE NICGetMediaState( IN PMP_ADAPTER Adapter  );
extern ULONG VendorDriverVersion;

VOID MPSetPowerD0(PMP_ADAPTER  Adapter);
NDIS_STATUS MPSetPowerLow(PMP_ADAPTER  Adapter , NDIS_DEVICE_POWER_STATE  PowerState);

#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("FEC"), {
        TEXT("Init"),TEXT("Error"),TEXT("Warning"),TEXT("Function"),
        TEXT("Info"),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("") },
    0x6
};
#endif

//-----------------------------------------------------------------------------
// Local Variables
//-----------------------------------------------------------------------------

// List of supported OIDs for the FEC driver
static NDIS_OID FECSupportedOids[] = {
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_VENDOR_ID,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,
    OID_802_3_XMIT_MAX_COLLISIONS,
    OID_802_3_RCV_OVERRUN,
    OID_802_3_XMIT_UNDERRUN,
    OID_PNP_CAPABILITIES,
};

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
    switch(Op)
    {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER(hInstDll);
            DEBUGMSG(ZONE_INIT, (TEXT("FEC: DLL Process Attach.\r\n")));

            // Disables the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT, (TEXT("FEC: DLL Process Detach.\r\n")));
            break;

        default:
            break;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: DriverEntry
//
// This function is the main entry of driver. It initializes and registers FEC
// driver to NDIS.
//
// Parameters:
//      DriverObject
//          [in] pointer to driver object created by the system.
//
//      RegistryPath
//          [in] Driver Registry path.
//
// Returns:
//      Returns STATUS_SUCCESS if initialization is success. Return
//      STATUS_UNSUCCESSFUL if not.
//
//------------------------------------------------------------------------------
#pragma NDIS_INIT_FUNCTION(DriverEntry)
NDIS_STATUS DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    NDIS_STATUS Status;
    NDIS_MINIPORT_DRIVER_CHARACTERISTICS FECChar;

    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +DriverEntry(0x%.8X, 0x%.8X)\r\n"), DriverObject, RegistryPath));

    // Fill-in adapter characterictics before calling NdisMRegisterMiniport
    //memset(&FECChar, 0, sizeof(FECChar));
    NdisZeroMemory(&FECChar, sizeof(FECChar));

    FECChar.Header.Type                  = NDIS_OBJECT_TYPE_MINIPORT_DRIVER_CHARACTERISTICS,
    FECChar.Header.Size                  = sizeof(NDIS_MINIPORT_DRIVER_CHARACTERISTICS);
    FECChar.Header.Revision              = NDIS_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_1;

    FECChar.MajorNdisVersion            = FEC_NDIS_MAJOR_VERSION;
    FECChar.MinorNdisVersion            = FEC_NDIS_MINOR_VERSION;
    FECChar.MajorDriverVersion          = 1;
    FECChar.MinorDriverVersion          = 0;

    //FECChar.Flags                     = 0;
    FECChar.SetOptionsHandler           = NULL;
    FECChar.InitializeHandlerEx         = FECInitializeEx;
    FECChar.HaltHandlerEx               = FECHaltEx;
    FECChar.UnloadHandler               = FECDriverUnload;
    FECChar.PauseHandler                = FECPause;
    FECChar.RestartHandler              = FECRestart;
    FECChar.OidRequestHandler           = FECOidRequest;
    FECChar.SendNetBufferListsHandler   = FECSendNetBufferLists;
    FECChar.ReturnNetBufferListsHandler = FECReturnNetBufferLists;
    FECChar.CancelSendHandler           = FECCancelSendNetBufferLists;
    FECChar.CheckForHangHandlerEx       = FECCheckForHangEx;
    FECChar.ResetHandlerEx              = FECResetEx;
    FECChar.DevicePnPEventNotifyHandler = FECDevicePnPEventNotify;
    FECChar.ShutdownHandlerEx           = FECShutdownEx;
    FECChar.CancelOidRequestHandler     = FECCancelOidRequest;

    // Now register Miniport
    Status = NdisMRegisterMiniportDriver(DriverObject,
                                         RegistryPath,
                                         (PNDIS_HANDLE)MiniportDriverContext,
                                         &FECChar,
                                         &NdisMiniportDriverHandle );

    if (Status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("FEC: NdisMRegisterMiniportDriver failure [0x%.8X].\r\n"), Status));
        NdisMDeregisterMiniportDriver(NdisMiniportDriverHandle);
    }
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: -DriverEntry [0x%.8X]\r\n"), Status));
    return (Status);
}



//------------------------------------------------------------------------------
//
// Function: FECInitializeEx
//
// This function finds and initializes the FEC adapter. When the FEC driver calls
// the NdisMRegisterMiniport from its DriverEntry function, NDIS will call
// FECInitialize in the context of NdisMRegisterMiniport.
//
// Parameters:
//      MiniportAdapterHandle
//          [in]  Specifies a handle identifying the FEC driver, which is assigned by
//                the NDIS library. It is a required parameter in subsequent calls
//                to NdisXXX functions.
//
//      MiniportDriverContext
//          [in]  Specifies a handle used only during initialization for calls to
//                NdisXXX configuration and initialization functions. For example,
//                this handle is passed to NDIS when we registered the driver
//
//      MiniportInitParameters
//          [in]   Initialization parameters
//
// Returns:
//      Returns NDIS_STATUS_SUCCESS if initialization is success.
//      Return NDIS_STATUS_FAILURE or NDIS_STATUS_UNSUPPORTED_MEDIA if not.
//
//------------------------------------------------------------------------------
//#pragma NDIS_PAGEABLE_FUNCTION(FECInitializeEx)

NDIS_STATUS FECInitializeEx(
    IN NDIS_HANDLE  MiniportAdapterHandle,
    IN NDIS_HANDLE  MiniportDriverContext,
    IN PNDIS_MINIPORT_INIT_PARAMETERS  MiniportInitParameters)
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    NDIS_MINIPORT_INTERRUPT_CHARACTERISTICS  Interrupt;
    NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES   RegistrationAttributes;
    NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES        GeneralAttributes;
    NDIS_PNP_CAPABILITIES          PowerManagementCapabilities;
    PMP_ADAPTER     Adapter = NULL;
    ULONG MemPhysAddress;
    ULONG           ulInfoLen;

#if DBG
    LARGE_INTEGER   TS;
#endif

    DBGPRINT(MP_TRACE, ("====> FECInitialize\n"));
    UNREFERENCED_PARAMETER(MiniportDriverContext);

#if DBG
    NdisGetCurrentSystemTime(&TS);
#endif

    do
    {
        //
        // Allocate MP_ADAPTER structure
        //
        Status = MpAllocAdapterBlock(&Adapter, MiniportAdapterHandle);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        Adapter->AdapterHandle = MiniportAdapterHandle;

        NdisZeroMemory(&RegistrationAttributes, sizeof(NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES));
        NdisZeroMemory(&GeneralAttributes, sizeof(NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES));
        //
        // setting registration attributes
        //
        RegistrationAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES;
        RegistrationAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_1;
        RegistrationAttributes.Header.Size = sizeof(NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES);

        RegistrationAttributes.MiniportAdapterContext = (NDIS_HANDLE)Adapter;
        RegistrationAttributes.AttributeFlags = NDIS_MINIPORT_ATTRIBUTES_HARDWARE_DEVICE |
                                                NDIS_MINIPORT_ATTRIBUTES_BUS_MASTER;

        RegistrationAttributes.CheckForHangTimeInSeconds = 5;
        RegistrationAttributes.InterfaceType = NIC_INTERFACE_TYPE;
        Status = NdisMSetMiniportAttributes(MiniportAdapterHandle,
                                            (PNDIS_MINIPORT_ADAPTER_ATTRIBUTES)&RegistrationAttributes);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        //
        // Read the registry parameters
        //
        Status = NICReadRegParameters(Adapter);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        //
        // set up generic attributes
        //
        GeneralAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES;
        GeneralAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_1;
        GeneralAttributes.Header.Size = sizeof(NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES);
        GeneralAttributes.MediaType = NIC_MEDIA_TYPE;
        GeneralAttributes.MtuSize = FEC_MAX_PACKET_SIZE-NIC_HEADER_SIZE;
        GeneralAttributes.MaxXmitLinkSpeed = NIC_MEDIA_MAX_SPEED;
        GeneralAttributes.MaxRcvLinkSpeed = NIC_MEDIA_MAX_SPEED;
        GeneralAttributes.XmitLinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
        GeneralAttributes.RcvLinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
        GeneralAttributes.MediaConnectState = MediaConnectStateUnknown;
        GeneralAttributes.MediaDuplexState = MediaDuplexStateUnknown;
        GeneralAttributes.LookaheadSize = FEC_MAX_PACKET_SIZE-NIC_HEADER_SIZE;

        MPFillPoMgmtCaps (Adapter,
                          &PowerManagementCapabilities,
                          &Status,
                          &ulInfoLen);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            GeneralAttributes.PowerManagementCapabilities = &PowerManagementCapabilities;
        }
        else
        {
            GeneralAttributes.PowerManagementCapabilities = NULL;
        }

        //
        // do not fail the call because of failure to get PM caps
        //
        Status = NDIS_STATUS_SUCCESS;

        GeneralAttributes.MacOptions = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
                                       NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                                       NDIS_MAC_OPTION_NO_LOOPBACK;

        GeneralAttributes.SupportedPacketFilters = NIC_SUPPORTED_FILTERS;
        GeneralAttributes.MaxMulticastListSize = NIC_MAX_MCAST_LIST;
        GeneralAttributes.MacAddressLength = ETH_LENGTH_OF_ADDRESS;
        NdisMoveMemory(GeneralAttributes.PermanentMacAddress,
                       Adapter->PermanentAddress,
                       ETH_LENGTH_OF_ADDRESS);

        NdisMoveMemory(GeneralAttributes.CurrentMacAddress,
                       Adapter->CurrentAddress,
                       ETH_LENGTH_OF_ADDRESS);

        GeneralAttributes.PhysicalMediumType = NdisPhysicalMediumUnspecified;
        GeneralAttributes.RecvScaleCapabilities = NULL;
        GeneralAttributes.AccessType = NET_IF_ACCESS_BROADCAST; // NET_IF_ACCESS_BROADCAST for a typical ethernet adapter
        GeneralAttributes.DirectionType = NET_IF_DIRECTION_SENDRECEIVE; // NET_IF_DIRECTION_SENDRECEIVE for a typical ethernet adapter
        GeneralAttributes.ConnectionType = NET_IF_CONNECTION_DEDICATED;  // NET_IF_CONNECTION_DEDICATED for a typical ethernet adapter
        GeneralAttributes.IfType = IF_TYPE_ETHERNET_CSMACD; // IF_TYPE_ETHERNET_CSMACD for a typical ethernet adapter (regardless of speed)
        GeneralAttributes.IfConnectorPresent = TRUE; // RFC 2665 TRUE if physical adapter
        GeneralAttributes.SupportedStatistics = NDIS_STATISTICS_XMIT_OK_SUPPORTED |
                                                NDIS_STATISTICS_RCV_OK_SUPPORTED |
                                                NDIS_STATISTICS_XMIT_ERROR_SUPPORTED |
                                                NDIS_STATISTICS_RCV_ERROR_SUPPORTED |
                                                NDIS_STATISTICS_RCV_CRC_ERROR_SUPPORTED |
                                                NDIS_STATISTICS_RCV_NO_BUFFER_SUPPORTED |
                                                NDIS_STATISTICS_TRANSMIT_QUEUE_LENGTH_SUPPORTED |
                                                NDIS_STATISTICS_GEN_STATISTICS_SUPPORTED;
        GeneralAttributes.SupportedOidList = FECSupportedOids;
        GeneralAttributes.SupportedOidListLength = sizeof(FECSupportedOids);

        Status = NdisMSetMiniportAttributes(MiniportAdapterHandle,
                                            (PNDIS_MINIPORT_ADAPTER_ATTRIBUTES)&GeneralAttributes);

        MemPhysAddress = CSP_BASE_REG_PA_FEC;
        Status = NdisMRegisterIoPortRange(
            (PVOID *)&gpFECReg,
            MiniportAdapterHandle,
            MemPhysAddress,
            sizeof(CSP_FEC_REGS) );
        if((Status != NDIS_STATUS_SUCCESS) || !(gpFECReg))
        {
            DEBUGMSG(1,
                (TEXT("FECInitialize: Failed to alloc memory for mapping hardware registers\r\n")));
            break;
        }

        //
        // Allocate all other memory blocks including shared memory
        //
        Status = NICAllocAdapterMemory(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        //
        // Init send data structures
        //
        NICInitSend(Adapter);

        //
        // Init receive data structures
        //
        Status = NICInitRecv(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        //
        // Init the hardware and set up everything
        //
        Status = FECEnetInit(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        //
        // Disable interrupts here which is as soon as possible
        //
        NICDisableInterrupt(Adapter);
        //
        // Register the interrupt
        //
        // the embeded NDIS interrupt structure is already zero'ed out
        // as part of the adapter structure
        //
        Adapter->InterruptLevel = IRQ_FEC;
        Adapter->InterruptVector = IRQ_FEC;

        NdisZeroMemory(&Interrupt, sizeof(NDIS_MINIPORT_INTERRUPT_CHARACTERISTICS));

        Interrupt.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_INTERRUPT;
        Interrupt.Header.Revision = NDIS_MINIPORT_INTERRUPT_REVISION_1;
        Interrupt.Header.Size = sizeof(NDIS_MINIPORT_INTERRUPT_CHARACTERISTICS);

        Interrupt.InterruptHandler = FecIsr;
        Interrupt.InterruptDpcHandler = FecHandleInterrupt;
        Interrupt.DisableInterruptHandler = NULL;//NICDisableInterrupt;
        Interrupt.EnableInterruptHandler = NULL;//NICEnableInterrupt;

        Status = NdisMRegisterInterruptEx(Adapter->AdapterHandle,
                                        Adapter,
                                        &Interrupt,
                                        &Adapter->NdisInterruptHandle );
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(MP_ERROR, ("NdisMRegisterInterrupt failed\n"));

            NdisWriteErrorLogEntry(
                Adapter->AdapterHandle,
                NDIS_ERROR_CODE_INTERRUPT_CONNECT,
                0);

            break;
        }

        //
        // If the driver support MSI
        //
        Adapter->InterruptType = Interrupt.InterruptType;

        if (Adapter->InterruptType == NDIS_CONNECT_MESSAGE_BASED)
        {
            Adapter->MessageInfoTable = Interrupt.MessageInfoTable;
        }

        //
        // If the driver supports MSI, here it should what kind of interrupt is granted. If MSI is granted,
        // the driver can check Adapter->MessageInfoTable to get MSI information
        //

        MP_SET_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE);

        //
        // initial state is paused
        //
        Adapter->AdapterState = NicPaused;
        DBGPRINT(MP_TRACE, ("FECInitializeEx- NIC State is Paused \n"));

        //
        // Set the link detection flag
        //
        MP_SET_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION);

        //
        // Increment the reference count so halt handler will wait
        //
        MP_INC_REF(Adapter);

        //
        // Enable the interrupt
        //
        NICEnableInterrupt(Adapter);

        Status = FECEnetAutoNego(Adapter);
        if (NDIS_STATUS_SUCCESS != Status )
        {
            RETAILMSG(TRUE, (TEXT("Phy Address Could not be obtained")));
            break;
        }

        //
        // Minimize init-time
        //
        NdisMInitializeTimer(
            &Adapter->LinkDetectionTimer, 
            Adapter->AdapterHandle,
            MpLinkDetectionDpc, 
            Adapter);

        NdisMSetTimer(&Adapter->LinkDetectionTimer, NIC_LINK_DETECTION_DELAY);

    } while (FALSE);

    if (Adapter && (Status != NDIS_STATUS_SUCCESS))
    {
        RETAILMSG(TRUE,(TEXT("FEC is getting unloaded")));

        //
        // Undo everything if it failed
        //

        if(MP_GET_REF(Adapter))
          MP_DEC_REF(Adapter);

        if(NIC_IS_RECV_READY(Adapter))
        {
            NICDisableInterrupt(Adapter);
        }

        MpFreeAdapter(Adapter);
    }
    DBGPRINT_S(Status, ("<==== MPInitialize, Status=%x\n", Status));
    return Status;
}


//------------------------------------------------------------------------------
//
// Function: FECHaltEx
//
// FECHaltEx is a required function that de-allocates resources when the FEC
// adapter is removed and halts the network adapter.
//
// Parameters:
//      MiniportAdapterContext
//          [in] Specifies the handle to a FEC allocated context area in
//               which the FEC driver maintains FEC adapter state,
//               set up by FECInitialize.
//
// Return Value:
//      None.
//
//------------------------------------------------------------------------------
#pragma NDIS_PAGABLE_FUNCTION(FECHaltEx)
void FECHaltEx(IN NDIS_HANDLE MiniportAdapterContext,
               IN NDIS_HALT_ACTION HaltAction)

{
    LONG            Count;
    BOOLEAN         TimerCancelled;

    PMP_ADAPTER     Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    ASSERT(Adapter->AdapterState == NicPaused);
    MP_SET_FLAG(Adapter, fMP_ADAPTER_HALT_IN_PROGRESS);
    DBGPRINT(MP_TRACE, ("====> FECHalt\n"));

    //
    // Call Shutdown handler to disable interrupts and turn the hardware off
    // by issuing a full reset. since we are not calling our shutdown handler
    // as a result of a bugcheck, we can use NdisShutdownPowerOff as the reason for
    // shutting down the adapter.
    //
    FECShutdownEx(MiniportAdapterContext, NdisShutdownPowerOff);

    //
    // Deregister interrupt as early as possible
    //
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE))
    {
        NdisMDeregisterInterruptEx(Adapter->NdisInterruptHandle);
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE);
    }

    NdisMCancelTimer( &Adapter->LinkDetectionTimer, &TimerCancelled );

    //
    // Decrement the ref count which was incremented in MPInitialize
    //
    Count = MP_DEC_REF(Adapter);

    //
    // Possible non-zero ref counts mean one or more of the following conditions:
    // 1) Pending async shared memory allocation;
    // 2) DPC's are not finished (e.g. link detection)
    //
    if (Count)
    {
        DBGPRINT(MP_WARN, ("RefCount=%d --- waiting!\n", MP_GET_REF(Adapter)));
        while (TRUE)
        {
            if (NdisWaitEvent(&Adapter->ExitEvent, 2000))
            {
                break;
            }
            DBGPRINT(MP_WARN, ("RefCount=%d --- rewaiting!\n", MP_GET_REF(Adapter)));
        }
    }

    //
    // Reset the PHY chip.  We do this so that after a warm boot, the PHY will
    // be in a known state, with auto-negotiation enabled.
    //

    //
    // Free the entire adapter object, including the shared memory structures.
    //
    MpFreeAdapter(Adapter);
    DBGPRINT(MP_TRACE, ("<==== MPHalt\n"));
}



//------------------------------------------------------------------------------
//
// Function: FECQueryInformation
//
// FECQueryInformation is a required function that returns information about
// the capabilities and status of the FEC driver. The NDIS library makes one
// or more calls to FECQueryInformation just after FECInitialize function
// returns NDIS_STATUS_SUCCESS.
//
// Parameters:
//      MiniportAdapterContext
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize
//
//      NdisRequest
//          [in] Pointer to the query request
//
// Return Values:
//      returns NDIS_STATUS_SUCCESS if FECQueryInformation returned the requested
//      information at InformationBuffer and set the variable at BytesWritten to
//      the amount of information it returned
//
//      returns NDIS_STATUS_INVALID_OID if the requested Oid is not supported
//
//      returns NDIS_STATUS_INVALID_LENGTH if The InformationBufferLength does
//      not match the length required by the given Oid
//
//------------------------------------------------------------------------------

NDIS_STATUS FECQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_OID_REQUEST   NdisRequest
    )
{
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER                 Adapter;
    NDIS_OID                    Oid;
    PVOID                       InformationBuffer;
    ULONG                       InformationBufferLength;
    ULONG                       BytesWritten;
    ULONG                       BytesNeeded;
    NDIS_HARDWARE_STATUS        HardwareStatus = NdisHardwareStatusReady;
    NDIS_MEDIUM                 Medium = NIC_MEDIA_TYPE;
    NDIS_PHYSICAL_MEDIUM        PhysMedium = NdisPhysicalMediumUnspecified;
    UCHAR                       VendorDesc[] = NIC_VENDOR_DESC;
    ULONG                       VendorDescSize;
    ULONG                       i;
    ANSI_STRING                 strAnsi;
    WCHAR                       *WcharBuf;
    NDIS_PNP_CAPABILITIES       Power_Management_Capabilities;
    ULONG                       ulInfo = 0;
    ULONG64                     ul64Info = 0;
    USHORT                      usInfo = 0;
    PVOID                       pInfo = (PVOID) &ulInfo;
    ULONG                       ulInfoLen = sizeof(ulInfo);
    ULONG                       ulBytesAvailable = ulInfoLen;
    NDIS_MEDIA_CONNECT_STATE    CurrMediaState;
    NDIS_MEDIA_STATE            LegacyMediaState;
    BOOLEAN                     DoCopy = TRUE;

    DBGPRINT(MP_TRACE, ("====> FECQueryInformation\n"));
    Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    Oid = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    InformationBuffer = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    InformationBufferLength = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;

    //
    // Initialize the result
    //
    BytesWritten = 0;
    BytesNeeded = 0;

    //
    // Process different type of requests
    //
    switch(Oid)
    {
        case OID_GEN_SUPPORTED_LIST:
            pInfo = (PVOID) FECSupportedOids;
            ulBytesAvailable = ulInfoLen = sizeof(FECSupportedOids);
            break;

        case OID_GEN_HARDWARE_STATUS:
            pInfo = (PVOID) &HardwareStatus;
            ulBytesAvailable = ulInfoLen = sizeof(NDIS_HARDWARE_STATUS);
            break;

        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
            pInfo = (PVOID) &Medium;
            ulBytesAvailable = ulInfoLen = sizeof(NDIS_MEDIUM);
            break;

        case OID_GEN_PHYSICAL_MEDIUM:
            pInfo = (PVOID) &PhysMedium;
            ulBytesAvailable = ulInfoLen = sizeof(NDIS_PHYSICAL_MEDIUM);
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_MAXIMUM_LOOKAHEAD:
            if (Adapter->ulLookAhead == 0)
            {
                Adapter->ulLookAhead = FEC_MAX_PACKET_SIZE - NIC_HEADER_SIZE; 
            }
            ulInfo = Adapter->ulLookAhead;
            break;

        case OID_GEN_MAXIMUM_FRAME_SIZE:
            ulInfo = FEC_MAX_PACKET_SIZE - NIC_HEADER_SIZE; 
            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:
            ulInfo = (ULONG) FEC_MAX_PACKET_SIZE;
            break;

        case OID_GEN_MAC_OPTIONS:
            // Notes:
            // The protocol driver is free to access indicated data by any means.
            // Some fast-copy functions have trouble accessing on-board device
            // memory. NIC drivers that indicate data out of mapped device memory
            // should never set this flag. If a NIC driver does set this flag, it
            // relaxes the restriction on fast-copy functions.

            // This miniport indicates receive with NdisMIndicateReceiveNetBufferLists
            // function. It has no MiniportTransferData function. Such a driver
            // should set this flag.
            ulInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
                     NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                     NDIS_MAC_OPTION_NO_LOOPBACK;

            break;

        case OID_GEN_LINK_SPEED:
        case OID_GEN_MEDIA_CONNECT_STATUS:
            if (InformationBufferLength < ulInfoLen)
            {
                DBGPRINT(MP_WARN, ("FECQueryInformation: Media State OID 0x%08x- Too Small\n", Oid));
                Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = ulInfoLen;
                break;
            }
            NdisAcquireSpinLock(&Adapter->Lock);
            if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
            {
                ASSERT(Adapter->PendingRequest == NULL);
                Adapter->PendingRequest = NdisRequest;
                NdisReleaseSpinLock(&Adapter->Lock);

                DBGPRINT(MP_WARN, ("FECQueryInformation: Media State OID 0x%08x is pended\n", Oid));

                Status = NDIS_STATUS_PENDING;
                break;
            }
            else
            {
                NdisReleaseSpinLock(&Adapter->Lock);
                if (Oid == OID_GEN_LINK_SPEED)
                {
                    if (Adapter->SpeedMode)
                    {
                        Adapter->usLinkSpeed = SPEED_100MBPS;
                    }
                    else
                    {
                        Adapter->usLinkSpeed = SPEED_10MBPS;
                    }
                    ulInfo = Adapter->usLinkSpeed * SPEED_FACTOR;
                }
                else  // OID_GEN_MEDIA_CONNECT_STATUS
                {
                    CurrMediaState = NICGetMediaState(Adapter);
                    if (CurrMediaState == MediaConnectStateConnected)
                    {
                        LegacyMediaState = NdisMediaStateConnected;
                        DBGPRINT(MP_TRACE, ("OID- Media State Connected\n", Oid));
                    }
                    else
                    {
                        //
                        // treat unknown media state the same as disconnected
                        //
                        LegacyMediaState = NdisMediaStateDisconnected;
                        DBGPRINT(MP_TRACE, ("OID- Media State Disconnected\n", Oid));
                    }
                    ulInfo = LegacyMediaState;
                }
            }
            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            ulInfo = NIC_MAX_PACKET_SIZE * Adapter->NumTcb;
            break;

        case OID_GEN_RECEIVE_BUFFER_SPACE:
            ulInfo = NIC_MAX_PACKET_SIZE * Adapter->CurrNumRfd;
            break;

        case OID_GEN_VENDOR_ID:
            NdisMoveMemory(&ulInfo, Adapter->PermanentAddress, 3);
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            pInfo = VendorDesc;
            ulBytesAvailable = ulInfoLen = sizeof(VendorDesc);
            break;

        case OID_GEN_VENDOR_DRIVER_VERSION:
            ulInfo = VendorDriverVersion;
            break;

        case OID_GEN_DRIVER_VERSION:
            usInfo = (USHORT) NIC_DRIVER_VERSION;
            pInfo = (PVOID) &usInfo;
            ulBytesAvailable = ulInfoLen = sizeof(USHORT);
            break;

        case OID_802_3_PERMANENT_ADDRESS:
            pInfo = Adapter->PermanentAddress;
            ulBytesAvailable = ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_CURRENT_ADDRESS:
            pInfo = Adapter->CurrentAddress;
            ulBytesAvailable = ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:
            ulInfo = NIC_MAX_MCAST_LIST;
            break;

        case OID_GEN_MAXIMUM_SEND_PACKETS:
            ulInfo = NIC_MAX_SEND_PACKETS;
            break;

        case OID_802_3_MULTICAST_LIST:
            pInfo = Adapter->MCList;
            ulBytesAvailable = ulInfoLen = ETH_LENGTH_OF_ADDRESS*Adapter->MCAddressCount;
            break;

        case OID_PNP_CAPABILITIES:
            MPFillPoMgmtCaps (Adapter,
                                &Power_Management_Capabilities,
                                &Status,
                                &ulInfoLen);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                pInfo = (PVOID) &Power_Management_Capabilities;
            }
            else
            {
                Status = NDIS_STATUS_NOT_SUPPORTED;
                pInfo = NULL;
            }
            break;

        case OID_PNP_QUERY_POWER:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;

            // WMI support
        case OID_CUSTOM_DRIVER_QUERY:
            // this is the uint case
            DBGPRINT(MP_INFO,("CUSTOM_DRIVER_QUERY got a QUERY\n"));
            ulInfo = ++Adapter->CustomDriverSet;
            break;

        case OID_CUSTOM_DRIVER_SET:
            DBGPRINT(MP_INFO,("CUSTOM_DRIVER_SET got a QUERY\n"));
            ulInfo = Adapter->CustomDriverSet;
            break;

        case OID_CUSTOM_ARRAY:

            DBGPRINT(MP_INFO,("CUSTOM_ARRAY got a QUERY\n"));
            //
            // Fill in the correct format
            //
            BytesNeeded = sizeof(ULONG) + 4;
            if (InformationBufferLength < BytesNeeded)
            {
                Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BytesNeeded;
                return Status;
            }
            //
            // Buffer length is enough
            //
            *(PULONG)InformationBuffer = 4;

            NdisMoveMemory((PUCHAR)InformationBuffer+sizeof(ULONG),
                            Adapter->PermanentAddress,
                            4);
            NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG) + 4;
            return Status;

        case OID_CUSTOM_STRING:
            //
            // ANSI string, convert to unicode string
            //
            DBGPRINT(MP_INFO, ("CUSTOM_STRING got a QUERY\n"));

            VendorDescSize = sizeof(VendorDesc);
            BytesNeeded = sizeof(USHORT) + (VendorDescSize * sizeof(WCHAR));
            if (InformationBufferLength < BytesNeeded)
            {
                Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BytesNeeded;
                return Status;
            }
            NdisInitAnsiString(&strAnsi, VendorDesc);
            WcharBuf = (WCHAR *)((PUCHAR)InformationBuffer + sizeof(USHORT));
            for (i = 0; i < strAnsi.Length; i++)
            {
                WcharBuf[i] = (WCHAR)(strAnsi.Buffer[i]);
            }
            Status = NDIS_STATUS_SUCCESS;

            if (NDIS_STATUS_SUCCESS == Status)
            {
                *(PUSHORT)InformationBuffer = 2 * strAnsi.Length;
                NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = BytesNeeded ;
            }
            return Status;

        case OID_GEN_XMIT_OK:
        case OID_GEN_RCV_OK:
        case OID_GEN_XMIT_ERROR:
        case OID_GEN_RCV_ERROR:
        case OID_GEN_RCV_NO_BUFFER:
        case OID_GEN_RCV_CRC_ERROR:
        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
        case OID_802_3_RCV_ERROR_ALIGNMENT:
        case OID_802_3_XMIT_ONE_COLLISION:
        case OID_802_3_XMIT_MORE_COLLISIONS:
        case OID_802_3_XMIT_DEFERRED:
        case OID_802_3_XMIT_MAX_COLLISIONS:
        case OID_802_3_RCV_OVERRUN:
        case OID_802_3_XMIT_UNDERRUN:
        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
        case OID_802_3_XMIT_TIMES_CRS_LOST:
        case OID_802_3_XMIT_LATE_COLLISIONS:
        case OID_GEN_BYTES_RCV:
        case OID_GEN_BYTES_XMIT:
        case OID_GEN_RCV_DISCARDS:
        case OID_GEN_DIRECTED_BYTES_XMIT:
        case OID_GEN_DIRECTED_FRAMES_XMIT:
        case OID_GEN_MULTICAST_BYTES_XMIT:
        case OID_GEN_MULTICAST_FRAMES_XMIT:
        case OID_GEN_BROADCAST_BYTES_XMIT:
        case OID_GEN_BROADCAST_FRAMES_XMIT:
        case OID_GEN_DIRECTED_BYTES_RCV:
        case OID_GEN_DIRECTED_FRAMES_RCV:
        case OID_GEN_MULTICAST_BYTES_RCV:
        case OID_GEN_MULTICAST_FRAMES_RCV:
        case OID_GEN_BROADCAST_BYTES_RCV:
        case OID_GEN_BROADCAST_FRAMES_RCV:

            Status = NICGetStatsCounters(Adapter, Oid, &ul64Info);
            ulBytesAvailable = ulInfoLen = sizeof(ul64Info);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                if (InformationBufferLength < sizeof(ULONG))
                {
                    Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                    BytesNeeded = ulBytesAvailable;
                    break;
                }
                ulInfoLen = MIN(InformationBufferLength, ulBytesAvailable);
                pInfo = &ul64Info;
            }
        break;

        case OID_GEN_STATISTICS:
            // we are going to directly fill the information buffer
            DoCopy = FALSE;
            ulBytesAvailable = ulInfoLen = sizeof(NDIS_STATISTICS_INFO);
            if (InformationBufferLength < ulInfoLen)
            {
                break;
            }
            Status = NICGetStatsCounters(Adapter, Oid, (PULONG64)InformationBuffer);
            break;

        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }

    if (Status == NDIS_STATUS_SUCCESS)
    {
        BytesNeeded = ulBytesAvailable;
        if (ulInfoLen <= InformationBufferLength)
        {
            //
            // Copy result into InformationBuffer
            //
            BytesWritten = ulInfoLen;
            if (ulInfoLen && DoCopy)
            {
                NdisMoveMemory(InformationBuffer, pInfo, ulInfoLen);
            }
        }
        else
        {
            //
            // too short
            //
            BytesNeeded = ulInfoLen;
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
        }
    }
    NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = BytesWritten;
    NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BytesNeeded;

    DBGPRINT(MP_TRACE, ("<==== FECQueryInformation, OID=0x%08x, Status=%x\n", Oid, Status));
    return(Status);
}



//------------------------------------------------------------------------------
//
// Function: FECSetInformation
//
// This function is a required function that allows NDIS to request changes in
// the state information that the FEC driver maintains for particular object
// identifiers, such as changes in multicast addresses.
//
// Parameters:
//     MiniportAdapterContext
//             [in]  Specifies the handle to a FEC driver allocated context area
//                   in which the FEC driver maintains FEC adapter state, set up
//                   by FECInitialize function.
//
//     Oid
//             [in]  Specifies the system-defined OID_XXX code designating the
//                   set operation the FEC driver should carry out.
//
//     InformationBuffer
//             [in]  Points to a buffer containing the OID-specific data used by
//                   FECSetInformation for the set.
//
//     InformationBufferLength
//             [in]  Specifies the number of bytes at InformationBuffer.
//
//     BytesRead
//             [out] Points to a variable that FECSetInformation sets to the number
//                   of bytes it read from the buffer at InformationBuffer.
//
//     BytesNeeded
//             [out] Points to a variable that FECSetInformation sets to the number
//                   of additional bytes it needs to satisfy the request if
//                   InformationBufferLength is less than Oid requires.
//
// Return Value:
//     Return NDIS_STATUS_SUCCESS if the required Oid is set successfully.
//     Return NDIS_STATUS_INVALID_LENGTH if the InformationBufferLength does not
//     match the length required by the given Oid.
//
//---------------------------------------------------------------------------------

NDIS_STATUS FECSetInformation (
    IN NDIS_HANDLE           MiniportAdapterContext,
    IN PNDIS_OID_REQUEST     NdisRequest
    )
{
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER                 Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    NDIS_OID                    Oid;
    PVOID                       InformationBuffer;
    ULONG                       InformationBufferLength;
    ULONG                       BytesRead;
    ULONG                       BytesNeeded;
    ULONG                       PacketFilter;
    NDIS_DEVICE_POWER_STATE     NewPowerState;
    ULONG                       MCAddressCount;

    DBGPRINT(MP_TRACE, ("====> FECSetInformation\n"));

    Oid = NdisRequest->DATA.SET_INFORMATION.Oid;
    InformationBuffer = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    InformationBufferLength = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    BytesRead = 0;
    BytesNeeded = 0;

    switch(Oid)
    {
        case OID_802_3_MULTICAST_LIST:
            //
            // Verify the length
            //
            if (InformationBufferLength % ETH_LENGTH_OF_ADDRESS != 0)
            {
                return(NDIS_STATUS_INVALID_LENGTH);
            }
            //
            // Save the number of MC list size
            //
            MCAddressCount = InformationBufferLength / ETH_LENGTH_OF_ADDRESS;
            ASSERT(MCAddressCount <= NIC_MAX_MCAST_LIST);

            if (MCAddressCount > NIC_MAX_MCAST_LIST)
            {
                return (NDIS_STATUS_INVALID_DATA);
            }
            Adapter->MCAddressCount = MCAddressCount;

            //
            // Save the MC list
            //
            NdisMoveMemory(
                Adapter->MCList,
                InformationBuffer,
                MCAddressCount * ETH_LENGTH_OF_ADDRESS);

            BytesRead = InformationBufferLength;
            NdisAcquireSpinLock(&Adapter->Lock);
            NdisAcquireSpinLock(&Adapter->RcvLock);

            // clear all the content in hardware Hash Table
            ClearAllMultiCast();

            Status = NICSetMulticastList(Adapter);

            NdisReleaseSpinLock(&Adapter->RcvLock);
            NdisReleaseSpinLock(&Adapter->Lock);
            break;

        case OID_GEN_CURRENT_PACKET_FILTER:
            //
            // Verify the Length
            //
            if (InformationBufferLength != sizeof(ULONG))
            {
                return(NDIS_STATUS_INVALID_LENGTH);
            }

            BytesRead = InformationBufferLength;

            NdisMoveMemory(&PacketFilter,
                          InformationBuffer,
                          sizeof(ULONG));
            //
            // any bits not supported?
            //
            if (PacketFilter & ~NIC_SUPPORTED_FILTERS)
            {
                return(NDIS_STATUS_NOT_SUPPORTED);
            }

            //
            // any filtering changes?
            //
            if (PacketFilter == Adapter->PacketFilter)
            {
                break;
            }
            NdisAcquireSpinLock(&Adapter->Lock);
            NdisAcquireSpinLock(&Adapter->RcvLock);

            if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
            {
                ASSERT(Adapter->PendingRequest == NULL);
                Adapter->PendingRequest = NdisRequest;
                NdisReleaseSpinLock(&Adapter->RcvLock);
                NdisReleaseSpinLock(&Adapter->Lock);

                DBGPRINT(MP_TRACE, ("FECSetInformation- Packet Filter Pended \n"));

                Status = NDIS_STATUS_PENDING;
                break;
            }
            Status = NICSetPacketFilter(
                         Adapter,
                         PacketFilter);

            NdisReleaseSpinLock(&Adapter->RcvLock);
            NdisReleaseSpinLock(&Adapter->Lock);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                Adapter->PacketFilter = PacketFilter;
            }

            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
            //
            // Verify the Length
            //
            if (InformationBufferLength < sizeof(ULONG))
            {
                BytesNeeded = sizeof(ULONG);
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            if (*(UNALIGNED PULONG)InformationBuffer > (FEC_MAX_PACKET_SIZE - NIC_HEADER_SIZE))
            {
                Status = NDIS_STATUS_INVALID_DATA;
                break;
            }

            NdisMoveMemory(&Adapter->ulLookAhead, InformationBuffer, sizeof(ULONG));
            BytesRead = sizeof(ULONG);
            Status = NDIS_STATUS_SUCCESS;
            break;

      case OID_PNP_SET_POWER:

          DBGPRINT(MP_TRACE, ("SET: Power State change, "PTR_FORMAT"!!!\n", InformationBuffer));
          if (InformationBufferLength != sizeof(NDIS_DEVICE_POWER_STATE ))
          {
              return(NDIS_STATUS_INVALID_LENGTH);
          }
          NewPowerState = *(PNDIS_DEVICE_POWER_STATE UNALIGNED)InformationBuffer;

          //
          // Set the power state - Cannot fail this request
          //
         Status = MPSetPower(Adapter ,NewPowerState );
         if (Status == NDIS_STATUS_PENDING)
         {
              Adapter->PendingRequest = NdisRequest;
              break;
          }
          if (Status != NDIS_STATUS_SUCCESS)
          {
              DBGPRINT(MP_ERROR, ("SET power: Hardware error !!!\n"));
              break;
          }
          BytesRead = sizeof(NDIS_DEVICE_POWER_STATE    );
          Status = NDIS_STATUS_SUCCESS;
          break;

      case OID_PNP_ADD_WAKE_UP_PATTERN:
          //
          // call a function that would program the adapter's wake
          // up pattern, return success
          //
          DBGPRINT(MP_TRACE, ("SET: Add Wake Up Pattern, !!!\n"));

#if defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
          if (MPIsPoMgmtSupported(Adapter) )
          {
              Status = MPAddWakeUpPattern(Adapter,
                                          InformationBuffer,
                                          InformationBufferLength,
                                          &BytesRead,
                                          &BytesNeeded);
          }
          else
#endif  //  defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
          {
              Status = NDIS_STATUS_NOT_SUPPORTED;
          }
          break;


      case OID_PNP_REMOVE_WAKE_UP_PATTERN:
          DBGPRINT(MP_TRACE, ("SET: Got a WakeUpPattern REMOVE Call\n"));
          //
          // call a function that would remove the adapter's wake
          // up pattern, return success
          //

#if defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
          if (MPIsPoMgmtSupported(Adapter) )
          {
              Status = MPRemoveWakeUpPattern(Adapter,
                                             InformationBuffer,
                                             InformationBufferLength,
                                             &BytesRead,
                                             &BytesNeeded);
          }
          else
#endif  //  defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
          {
              Status = NDIS_STATUS_NOT_SUPPORTED;
          }
          break;

      case OID_PNP_ENABLE_WAKE_UP:
          DBGPRINT(MP_TRACE, ("SET: Got a EnableWakeUp Call, "PTR_FORMAT"\n",InformationBuffer));
          //
          // call a function that would enable wake up on the adapter
          // return success
          //
          if (MPIsPoMgmtSupported(Adapter) )
          {
              ULONG       WakeUpEnable;
              NdisMoveMemory(&WakeUpEnable, InformationBuffer,sizeof(ULONG));
              //
              // The WakeUpEable can only be 0, or NDIS_PNP_WAKE_UP_PATTERN_MATCH since the driver only
              // supports wake up pattern match
              //
              if ((WakeUpEnable != 0)
                     && ((WakeUpEnable & NDIS_PNP_WAKE_UP_PATTERN_MATCH) != NDIS_PNP_WAKE_UP_PATTERN_MATCH ))
              {
                  Status = NDIS_STATUS_NOT_SUPPORTED;
                  Adapter->WakeUpEnable = 0;
                  break;
              }
              //
              // When the driver goes to low power state, it would check WakeUpEnable to decide
              // which wake up methed it should use to wake up the machine. If WakeUpEnable is 0,
              // no wake up method is enabled.
              //
              Adapter->WakeUpEnable = WakeUpEnable;

              BytesRead = sizeof(ULONG);
              Status = NDIS_STATUS_SUCCESS;
          }
          else
          {
              Status = NDIS_STATUS_NOT_SUPPORTED;
          }

          break;

        case OID_GEN_PROTOCOL_OPTIONS:
        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;

    }
    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisRequest->DATA.SET_INFORMATION.BytesRead = BytesRead;
    }
    NdisRequest->DATA.SET_INFORMATION.BytesNeeded = BytesNeeded;

    DBGPRINT(MP_TRACE, ("<==== FECSetInformationSet, OID=0x%08x, Status=%x\n", Oid, Status));

    return(Status);
}

VOID FECShutdownEx(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_SHUTDOWN_ACTION    ShutdownAction
    )
/*++

Routine Description:

    MiniportShutdown handler

Arguments:

    MiniportAdapterContext  Pointer to our adapter
    ShutdownAction          The reason for Shutdown

Return Value:

    None

--*/
{
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;

    UNREFERENCED_PARAMETER(ShutdownAction);

    DBGPRINT(MP_TRACE, ("====> FECShutdownEx\n"));

    //
    // Disable interrupt and issue a full reset
    //
    NICDisableInterrupt(Adapter);
    NICIssueFullReset(Adapter);

    DBGPRINT(MP_TRACE, ("<==== FECShutdownEx\n"));
}

VOID
FECDevicePnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNET_DEVICE_PNP_EVENT   NetDevicePnPEvent
    )
/*++

Routine Description:

    MiniportPnPEventNotify handler - NDIS51 and later

Arguments:

    MiniportAdapterContext      Pointer to our adapter
    PnPEvent                    Self-explanatory
    InformationBuffer           Self-explanatory
    InformationBufferLength     Self-explanatory

Return Value:

    None

--*/
{
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    NDIS_DEVICE_PNP_EVENT   PnPEvent = NetDevicePnPEvent->DevicePnPEvent;
    PVOID                   InformationBuffer = NetDevicePnPEvent->InformationBuffer;
    ULONG                   InformationBufferLength = NetDevicePnPEvent->InformationBufferLength;

    //
    // Turn off the warings.
    //
    UNREFERENCED_PARAMETER(InformationBuffer);
    UNREFERENCED_PARAMETER(InformationBufferLength);
    UNREFERENCED_PARAMETER(Adapter);

    DBGPRINT(MP_TRACE, ("====> FECDevicePnPEventNotify\n"));

    switch (PnPEvent)
    {
        case NdisDevicePnPEventQueryRemoved:
            DBGPRINT(MP_WARN, ("FECDevicePnPEventNotify: NdisDevicePnPEventQueryRemoved\n"));
            break;

        case NdisDevicePnPEventRemoved:
            DBGPRINT(MP_WARN, ("FECDevicePnPEventNotify: NdisDevicePnPEventRemoved\n"));
            break;

        case NdisDevicePnPEventSurpriseRemoved:
            DBGPRINT(MP_WARN, ("FECDevicePnPEventNotify: NdisDevicePnPEventSurpriseRemoved\n"));
            break;

        case NdisDevicePnPEventQueryStopped:
            DBGPRINT(MP_WARN, ("FECDevicePnPEventNotify: NdisDevicePnPEventQueryStopped\n"));
            break;

        case NdisDevicePnPEventStopped:
            DBGPRINT(MP_WARN, ("FECDevicePnPEventNotify: NdisDevicePnPEventStopped\n"));
            break;

        case NdisDevicePnPEventPowerProfileChanged:
            DBGPRINT(MP_WARN, ("FECDevicePnPEventNotify: NdisDevicePnPEventPowerProfileChanged\n"));
            break;

        default:
            DBGPRINT(MP_ERROR, ("FECDevicePnPEventNotify: unknown PnP event %x \n", PnPEvent));
            break;
    }
    DBGPRINT(MP_TRACE, ("<==== FECDevicePnPEventNotify\n"));
}

VOID
FECCancelSendNetBufferLists(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId
    )
/*++

Routine Description:

    MiniportCancelNetBufferLists handler - This function walks through all
    of the queued send NetBufferLists and cancels all the NetBufferLists that
    have the correct CancelId

Arguments:

    MiniportAdapterContext      Pointer to our adapter
    CancelId                    All the Net Buffer Lists with this Id should be cancelled

Return Value:

    None

--*/
{
    PMP_ADAPTER         Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    PQUEUE_ENTRY        pEntry, pPrevEntry, pNextEntry;
    PNET_BUFFER_LIST    NetBufferList;
    PNET_BUFFER_LIST    CancelHeadNetBufferList = NULL;
    PNET_BUFFER_LIST    CancelTailNetBufferList = NULL;
    PVOID               NetBufferListId;

    DBGPRINT(MP_TRACE, ("====> FECCancelSendNetBufferLists\n"));

    pPrevEntry = NULL;

    NdisAcquireSpinLock(&Adapter->SendLock);

    //
    // Walk through the send wait queue and complete the sends with matching Id
    //
    do
    {
        if (IsQueueEmpty(&Adapter->SendWaitQueue))
        {
            break;
        }
        pEntry = GetHeadQueue(&Adapter->SendWaitQueue);

        while (pEntry != NULL)
        {
            NetBufferList = MP_GET_NET_BUFFER_LIST_FROM_QUEUE_LINK(pEntry);

            NetBufferListId = NDIS_GET_NET_BUFFER_LIST_CANCEL_ID(NetBufferList);

            if ((NetBufferListId == CancelId)
                    && (NetBufferList != Adapter->SendingNetBufferList))
            {
                NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_REQUEST_ABORTED;
                Adapter->nWaitSend--;
                //
                // This packet has the right CancelId
                //
                pNextEntry = pEntry->Next;

                if (pPrevEntry == NULL)
                {
                    Adapter->SendWaitQueue.Head = pNextEntry;
                    if (pNextEntry == NULL)
                    {
                        Adapter->SendWaitQueue.Tail = NULL;
                    }
                }
                else
                {
                    pPrevEntry->Next = pNextEntry;
                    if (pNextEntry == NULL)
                    {
                        Adapter->SendWaitQueue.Tail = pPrevEntry;
                    }
                }
                pEntry = pEntry->Next;

                //
                // Queue this NetBufferList for cancellation
                //
                if (CancelHeadNetBufferList == NULL)
                {
                    CancelHeadNetBufferList = NetBufferList;
                    CancelTailNetBufferList = NetBufferList;
                }
                else
                {
                    NET_BUFFER_LIST_NEXT_NBL(CancelTailNetBufferList) = NetBufferList;
                    CancelTailNetBufferList = NetBufferList;
                }
            }
            else
            {
                // This packet doesn't have the right CancelId
                pPrevEntry = pEntry;
                pEntry = pEntry->Next;
            }
        }
    } while (FALSE);

    NdisReleaseSpinLock(&Adapter->SendLock);
    //
    // Get the packets from SendCancelQueue and complete them if any
    //
    if (CancelHeadNetBufferList != NULL)
    {
        NET_BUFFER_LIST_NEXT_NBL(CancelTailNetBufferList) = NULL;

        NdisMSendNetBufferListsComplete(
               MP_GET_ADAPTER_HANDLE(Adapter),
               CancelHeadNetBufferList,
               0);
    }
    DBGPRINT(MP_TRACE, ("<==== FECCancelSendNetBufferLists\n"));
    return;
}

VOID
FECSendNetBufferLists(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNET_BUFFER_LIST    NetBufferList,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  ULONG               SendFlags
    )
/*++

Routine Description:

    SendNetBufferLists handler

Arguments:

    MiniportAdapterContext  Pointer to our adapter
    NetBufferList           Pointer to a list of Net Buffer Lists.
    Dispatchlevel           Whether that caller is at DISPATCH_LEVEL or not

Return Value:

    None

--*/
{

    PMP_ADAPTER         Adapter;
    NDIS_STATUS         Status = NDIS_STATUS_PENDING;
    UINT                NetBufferCount = 0;
    PNET_BUFFER         NetBuffer;
    PNET_BUFFER_LIST    CurrNetBufferList;
    PNET_BUFFER_LIST    NextNetBufferList;
    BOOLEAN             DispatchLevel;

    DBGPRINT(MP_TRACE, ("====> FECSendNetBufferLists- Buffer "PTR_FORMAT" \n",NetBufferList));

    Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    DispatchLevel = NDIS_TEST_SEND_AT_DISPATCH_LEVEL(SendFlags);
    do
    {
        //
        // If the adapter is in Pausing or paused state, just fail the send.
        //
        if ((Adapter->AdapterState == NicPausing) || (Adapter->AdapterState == NicPaused))
        {
            DBGPRINT(MP_TRACE, ("FECSendNetBufferLists.. NIC is in Pause/Pausing State %d \n", Adapter->AdapterState));
            Status =  NDIS_STATUS_PAUSED;
            break;
        }

        MP_ACQUIRE_SPIN_LOCK(&Adapter->SendLock, DispatchLevel);

        if (Adapter->NextPowerState != NdisDeviceStateD0)
        {
            Status =  NDIS_STATUS_PAUSED;

            //Reset the check for hang counter in Low Power State
            Adapter->CurrSendHead->Count = 0;

            MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, DispatchLevel);
            break;
        }
        //
        // Is this adapter ready for sending?
        //
        if (MP_IS_NOT_READY(Adapter))
        {
            DBGPRINT(MP_TRACE, ("FECSendNetBufferLists- Adapter Not Ready\n"));

            //
            // there  is link
            //
            if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
            {
                DBGPRINT(MP_TRACE, ("FECSendNetBufferLists- Adapter Link Detection- Adding to the WaitQ\n"));

                //
                // Insert Net Buffer List into the queue
                //
                for (CurrNetBufferList = NetBufferList;
                        CurrNetBufferList != NULL;
                        CurrNetBufferList = NextNetBufferList)
                {
                    NextNetBufferList = NET_BUFFER_LIST_NEXT_NBL(CurrNetBufferList);
                    //
                    // Get how many net buffers inside the net buffer list
                    //

                    NetBufferCount = 0;
                    for (NetBuffer = NET_BUFFER_LIST_FIRST_NB(CurrNetBufferList);
                            NetBuffer != NULL;
                            NetBuffer = NET_BUFFER_NEXT_NB(NetBuffer))
                    {
                        NetBufferCount++;
                    }
                    ASSERT(NetBufferCount > 0);

                    DBGPRINT(MP_TRACE, ("FECSendNetBufferLists- %d Buffers in BufferList\n",NetBufferCount));

                    MP_GET_NET_BUFFER_LIST_REF_COUNT(CurrNetBufferList) = NetBufferCount;
                    MP_GET_NET_BUFFER_LIST_NEXT_SEND(CurrNetBufferList) = NET_BUFFER_LIST_FIRST_NB(CurrNetBufferList);
                    NET_BUFFER_LIST_STATUS(CurrNetBufferList) = NDIS_STATUS_SUCCESS;
                    InsertTailQueue(&Adapter->SendWaitQueue,
                               MP_GET_NET_BUFFER_LIST_LINK(CurrNetBufferList));
                    Adapter->nWaitSend++;
                    DBGPRINT(MP_WARN, ("FECSendNetBufferLists: link detection - queue NetBufferList "PTR_FORMAT" %d Pending \n", CurrNetBufferList,Adapter->nWaitSend));
                }
                MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, DispatchLevel);

                break;
            }

            //
            // Adapter is not ready and there is not link
            //
            Status = MP_GET_STATUS_FROM_FLAGS(Adapter);
            MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, DispatchLevel);
            break;
        }
        DBGPRINT(MP_TRACE, ("FECSendNetBufferLists- Adapter is Ready\n"));

        //
        // Adapter is ready, send this net buffer list, in this case, we always return pending
        //
        for (CurrNetBufferList = NetBufferList;
                CurrNetBufferList != NULL;
                CurrNetBufferList = NextNetBufferList)
        {
            NextNetBufferList = NET_BUFFER_LIST_NEXT_NBL(CurrNetBufferList);
            //
            // Get how many net buffers inside the net buffer list
            //
            NetBufferCount = 0;
            for (NetBuffer = NET_BUFFER_LIST_FIRST_NB(CurrNetBufferList);
                    NetBuffer != NULL;
                    NetBuffer = NET_BUFFER_NEXT_NB(NetBuffer))
            {
                NetBufferCount++;
            }
            ASSERT(NetBufferCount > 0);
            DBGPRINT(MP_TRACE, ("FECSendNetBufferLists- %d Buffers in BufferList\n",NetBufferCount));

            MP_GET_NET_BUFFER_LIST_REF_COUNT(CurrNetBufferList) = NetBufferCount;
            //
            // queue is not empty or tcb is not available, or another thread is sending
            // a NetBufferList.
            //
            if (!IsQueueEmpty(&Adapter->SendWaitQueue) ||
                (!MP_TCB_RESOURCES_AVAIABLE(Adapter) ||
                 Adapter->SendingNetBufferList != NULL))
            {

                //if we have no more send interrupts are expected to arrive, the queue will continue to
                //grow, and no other packets will be sent by FEC
                if( 0 == Adapter->nPendingSendPacket)
                {
                    //Handle cases where the packets are queued in the send Wait Queue, and all the other [non queued] 
                    //packets were already sent and processed by the interrupt handler.
                    while(!IsQueueEmpty(&Adapter->SendWaitQueue)&& MP_TCB_RESOURCES_AVAIABLE(Adapter) && (Adapter->SendingNetBufferList == NULL))
                    {
                        PQUEUE_ENTRY pEntry;
                        PNET_BUFFER_LIST    NetBufferList;

                        pEntry = GetHeadQueue(&Adapter->SendWaitQueue);
                        NetBufferList = MP_GET_NET_BUFFER_LIST_FROM_QUEUE_LINK (pEntry);

                        DBGPRINT(MP_INFO, ("MpHandleSendInterrupt - send a queued NetBufferList "PTR_FORMAT" \n",NetBufferList));

                        Adapter->SendingNetBufferList = NetBufferList;
                        MiniportSendNetBufferList(Adapter, NetBufferList, TRUE);
                    }
                }

                //
                // The first net buffer is the buffer to send
                //
                MP_GET_NET_BUFFER_LIST_NEXT_SEND(CurrNetBufferList) = NET_BUFFER_LIST_FIRST_NB(CurrNetBufferList);

                if(!IsQueueEmpty(&Adapter->SendWaitQueue) ||(!MP_TCB_RESOURCES_AVAIABLE(Adapter)) )
                {
                    NET_BUFFER_LIST_STATUS(CurrNetBufferList) = NDIS_STATUS_SUCCESS;
                    InsertTailQueue(&Adapter->SendWaitQueue, MP_GET_NET_BUFFER_LIST_LINK(CurrNetBufferList));
                    Adapter->nWaitSend++;
                    RETAILMSG(TRUE,(TEXT("Adding to Waiting Q{1} %d %x %x %d %d \n"),Adapter->nWaitSend, Adapter->SendingNetBufferList,CurrNetBufferList,Adapter->nBusySend,Adapter->nPendingSendPacket));
                    DBGPRINT(MP_TRACE, ("FECSendNetBufferLists- Adding to SendWaitQ- %d Pending\n",Adapter->nWaitSend));
                }else
                {
                    Adapter->SendingNetBufferList = CurrNetBufferList;
                    MiniportSendNetBufferList(Adapter, CurrNetBufferList, FALSE);
                }
            }
            else
            {
                DBGPRINT(MP_INFO, ("FECSendNetBufferLists - setting SendingNetBufferList "PTR_FORMAT" \n",CurrNetBufferList));

                Adapter->SendingNetBufferList = CurrNetBufferList;
                NET_BUFFER_LIST_STATUS(CurrNetBufferList) = NDIS_STATUS_SUCCESS;

                DBGPRINT(MP_TRACE, ("FECSendNetBufferLists- Sending BufferList directly \n"));
                MiniportSendNetBufferList(Adapter, CurrNetBufferList, FALSE);
            }
        }
        MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, DispatchLevel);
    }
    while (FALSE);

    if (Status != NDIS_STATUS_PENDING)
    {
        ULONG SendCompleteFlags = 0;

        DBGPRINT(MP_TRACE, ("FECSendNetBufferLists- Status not Pending..\n"));
        for (CurrNetBufferList = NetBufferList;
                 CurrNetBufferList != NULL;
                 CurrNetBufferList = NextNetBufferList)
        {
            NextNetBufferList = NET_BUFFER_LIST_NEXT_NBL(CurrNetBufferList);
            NET_BUFFER_LIST_STATUS(CurrNetBufferList) = Status;
        }

        if (NDIS_TEST_SEND_AT_DISPATCH_LEVEL(SendFlags))
        {
            NDIS_SET_SEND_COMPLETE_FLAG(SendCompleteFlags, NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);
        }

      

        DBGPRINT(MP_TRACE, ("FECSendNetBufferLists - Sending the BufferListComplete from FEC, Send Busy Count \n",Adapter->nBusySend));
        NdisMSendNetBufferListsComplete(
            MP_GET_ADAPTER_HANDLE(Adapter),
            NetBufferList,
            SendCompleteFlags);
    }
    DBGPRINT(MP_TRACE, ("<==== FECSendNetBufferLists\n"));
}


BOOLEAN
FecIsr(
    IN  NDIS_HANDLE     MiniportInterruptContext,
    OUT PBOOLEAN        QueueMiniportInterruptDpcHandler,
    OUT PULONG          TargetProcessors)
/*++

Routine Description:

    MiniportIsr handler

Arguments:

    MiniportInterruptContext: Pointer to the interrupt context.
        In our case, this is a pointer to our adapter structure.

    QueueMiniportInterruptDpcHandler: TRUE on return if MiniportHandleInterrupt
        should be called on default CPU

    TargetProcessors: Pointer to a bitmap specifying
        Target processors which should run the DPC

Return Value:

    TRUE  --- The miniport recognizes the interrupt
    FALSE   --- Otherwise

--*/
{
    PMP_ADAPTER  Adapter = (PMP_ADAPTER)MiniportInterruptContext;
    BOOLEAN                         InterruptRecognized = FALSE;

    DBGPRINT(MP_LOUD, ("====> FecIsr\n"));

    // Set to TRUE, FEC IRQ is not shared with other network
    // adapter
    InterruptRecognized = TRUE;

    // FECHandleInterrupt will be called to complete the
    // operation
    *QueueMiniportInterruptDpcHandler = TRUE;

    NICDisableInterrupt(Adapter);
    DBGPRINT(MP_LOUD, ("<==== FecIsr\n"));

    return InterruptRecognized;
}


VOID
FecHandleInterrupt(
    IN  NDIS_HANDLE  MiniportInterruptContext,
    IN  PVOID        MiniportDpcContext,
    IN  PULONG       NdisReserved1,
    IN  PULONG       NdisReserved2
    )
/*++

Routine Description:

    MiniportHandleInterrupt handler

Arguments:

    MiniportInterruptContext:  Pointer to the interrupt context.
        In our case, this is a pointer to our adapter structure.

Return Value:

    None

--*/
{
    PMP_ADAPTER  Adapter = (PMP_ADAPTER)MiniportInterruptContext;
    UINT         InterruptEvent=0;
    UINT         nSendPendingIndex=0;   //keep track of the pending index


    UNREFERENCED_PARAMETER(MiniportDpcContext);
    UNREFERENCED_PARAMETER(NdisReserved1);
    UNREFERENCED_PARAMETER(NdisReserved2);

    while( (InterruptEvent = INREG32(&gpFECReg->EIR)) != 0 )
    {
        // Clear the EIR
        OUTREG32(&gpFECReg->EIR, InterruptEvent);

        // Handle MII interrupt
        if(InterruptEvent & CSP_BITFVAL(FEC_EIR_MII, 1))
        {
            MpHandleMIIInterrupt(Adapter);
        }
        //
        // Handle send interrupt
        //
        if((InterruptEvent & CSP_BITFVAL(FEC_EIR_TXF, 1)))
        {
            DBGPRINT(MP_TRACE, (" EIR set -TXF \n"));

            NdisDprAcquireSpinLock(&Adapter->SendLock);
            nSendPendingIndex = Adapter->HwSendTbdCurrentCount;
            MpHandleSendInterrupt(Adapter);

            //if we have not been able to process any packet at this index, but the next packet is being processed by HW
            if( nSendPendingIndex == Adapter->HwSendTbdCurrentCount)
            {
               if( ((Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_READY)) &&
                   ((Adapter->HwTbdChip[(Adapter->HwSendTbdCurrentCount+1)%Adapter->NumTcb]->ControlStatus & BD_ENET_TX_READY)==0) &&
                   (Adapter->HwTbdChip[(Adapter->HwSendTbdCurrentCount+1)%Adapter->NumTcb]->ControlStatus & BD_ENET_TX_PAD) )
               {
                    Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus &= ~BD_ENET_TX_READY;

                    RETAILMSG(TRUE, (TEXT("MpHandleSendInterrupt - unblocking TBD; @ %d \n"),Adapter->HwSendTbdCurrentCount));
                    INSREG32BF(&gpFECReg->TDAR, FEC_TDAR_ACTIVE, FEC_TDAR_ACTIVE_ACTIVE);
               }
            }


            NdisDprReleaseSpinLock(&Adapter->SendLock);
        }
        if( InterruptEvent & CSP_BITFVAL(FEC_EIR_RXF, 1))
        {
            //
            // if we have a Recv interrupt and have reported a media disconnect status
            // time to indicate the new status
            //
            if ((MediaConnectStateConnected != Adapter->MediaState)&& (TRUE == Adapter->MIISeqDone) )
            {
                NdisDprAcquireSpinLock(&Adapter->Lock);
                RETAILMSG(TRUE,(TEXT("Intr- Media State Toggle- %d \n"),MediaConnectStateConnected));
                DBGPRINT(MP_WARN, ("Media state changed to Connected\n"));

                MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);

                Adapter->MediaState = MediaConnectStateConnected;

                FECIndicateLinkState(Adapter);
                NdisDprReleaseSpinLock(&Adapter->Lock);
            }
            NdisDprAcquireSpinLock(&Adapter->RcvLock);
            MpHandleRecvInterrupt(Adapter);
            NdisDprReleaseSpinLock(&Adapter->RcvLock);
        }

        //In case of Late Collision, Tx Underrun, RetransmissionLimit errors,
        //FEC will abort sending the current Frame and start transmitting the next
        //Frame, so the current Tx BD will be busy (not processed) and the subsequent BDs may be
        //ready to transmit (processed). In such case unblock send interrupt handler
        //from waiting for the current BD to be processed.
        if( (InterruptEvent & CSP_BITFVAL(FEC_EIR_LC, 1)) ||
            (InterruptEvent & CSP_BITFVAL(FEC_EIR_RL, 1)) ||
            (InterruptEvent & CSP_BITFVAL(FEC_EIR_UN, 1)) )
        {
            NdisDprAcquireSpinLock(&Adapter->SendLock);

            if(((Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_READY) == 1 ) &&
            (Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_PAD) )
            {
                RETAILMSG(TRUE, (TEXT("MpHandleSendInterrupt - unblocking TBD; Index %d \n"),Adapter->HwSendTbdCurrentCount));
                Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus &= ~BD_ENET_TX_READY;
            }
            MpHandleSendInterrupt(Adapter);
            NdisDprReleaseSpinLock(&Adapter->SendLock);
        }

        
        if( InterruptEvent & CSP_BITFVAL(FEC_EIR_EBERR, 1) )
        {
            RETAILMSG(TRUE,(TEXT("Bus Error %d \n"),Adapter->HwSendTbdCurrentCount));
        }

        if( InterruptEvent & CSP_BITFVAL(FEC_EIR_HBERR, 1) )
        {
            RETAILMSG(TRUE,(TEXT("Heartbeat Error %d \n"),Adapter->HwSendTbdCurrentCount));
        }

        if( InterruptEvent & CSP_BITFVAL(FEC_EIR_BABT, 1) )
        {
            RETAILMSG(TRUE,(TEXT("Babling TX Error %d \n"),Adapter->HwSendTbdCurrentCount));
        }

    }

    if( Adapter->NextPowerState == NdisDeviceStateD0 )
    {
        //
        // Re-enable the interrupt (disabled in FecIsr)
        //
        NdisMSynchronizeWithInterruptEx(
            Adapter->NdisInterruptHandle,
            0,
            NICEnableInterrupt,
            Adapter);
    }
}

VOID
MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNET_DEVICE_PNP_EVENT   NetDevicePnPEvent
    )
/*++

Routine Description:

    MiniportPnPEventNotify handler - NDIS51 and later

Arguments:

    MiniportAdapterContext      Pointer to our adapter
    PnPEvent                    Self-explanatory
    InformationBuffer           Self-explanatory
    InformationBufferLength     Self-explanatory

Return Value:

    None

--*/
{
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    NDIS_DEVICE_PNP_EVENT   PnPEvent = NetDevicePnPEvent->DevicePnPEvent;
    PVOID                   InformationBuffer = NetDevicePnPEvent->InformationBuffer;
    ULONG                   InformationBufferLength = NetDevicePnPEvent->InformationBufferLength;

    //
    // Turn off the warings.
    //
    UNREFERENCED_PARAMETER(InformationBuffer);
    UNREFERENCED_PARAMETER(InformationBufferLength);
    UNREFERENCED_PARAMETER(Adapter);

    DBGPRINT(MP_TRACE, ("====> MPPnPEventNotify\n"));

    switch (PnPEvent)
    {
        case NdisDevicePnPEventQueryRemoved:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventQueryRemoved\n"));
            break;

        case NdisDevicePnPEventRemoved:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventRemoved\n"));
            break;

        case NdisDevicePnPEventSurpriseRemoved:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventSurpriseRemoved\n"));
            break;

        case NdisDevicePnPEventQueryStopped:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventQueryStopped\n"));
            break;

        case NdisDevicePnPEventStopped:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventStopped\n"));
            break;

        case NdisDevicePnPEventPowerProfileChanged:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventPowerProfileChanged\n"));
            break;

        default:
            DBGPRINT(MP_ERROR, ("MPPnPEventNotify: unknown PnP event %x \n", PnPEvent));
            break;
    }
    DBGPRINT(MP_TRACE, ("<==== MPPnPEventNotify\n"));
}

//------------------------------------------------------------------------------
//
// Function: FECDriverUnload
// Routine Description:
//
//    The Unload handler
//    This handler is registered through NdisMRegisterUnloadHandler
//
// Arguments:
//
//    DriverObject        Not used
//
// Return Value:
//
//    None
//------------------------------------------------------------------------------

VOID
FECDriverUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
/*++

Routine Description:

    The Unload handler
    This handler is registered through NdisMRegisterUnloadHandler

Arguments:

    DriverObject        Not used

Return Value:

    None

--*/
{
    //
    // Deregister Miniport driver
    //
    NdisMDeregisterMiniportDriver(NdisMiniportDriverHandle);
}


NDIS_STATUS
FECRestart(
    IN  NDIS_HANDLE                         MiniportAdapterContext,
    IN  PNDIS_MINIPORT_RESTART_PARAMETERS   MiniportRestartParameters
    )
/*++

Routine Description:

    MiniportRestart handler
    The driver resumes its normal working state

Argument:

    MiniportAdapterContext  Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_PENDING  Can it return pending
    NDIS_STATUS_XXX      The driver fails to restart


--*/
{
    PMP_ADAPTER                  Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    PNDIS_RESTART_ATTRIBUTES     NdisRestartAttributes;
    PNDIS_RESTART_GENERAL_ATTRIBUTES  NdisGeneralAttributes;
    ULONG Mac1,Mac2;
    UNREFERENCED_PARAMETER(MiniportRestartParameters);

    DBGPRINT(MP_TRACE, ("====> FECRestart\n"));

    NdisRestartAttributes = MiniportRestartParameters->RestartAttributes;

    //
    // If NdisRestartAttributes is not NULL, then miniport can modify generic attributes and add
    // new media specific info attributes at the end. Otherwise, NDIS restarts the miniport because
    // of other reason, miniport should not try to modify/add attributes
    //
    if (NdisRestartAttributes != NULL)
    {
        DBGPRINT(MP_TRACE, ("FECRestart- NdisRestartAttributes\n"));
        ASSERT(NdisRestartAttributes->Oid == OID_GEN_MINIPORT_RESTART_ATTRIBUTES);

        NdisGeneralAttributes = (PNDIS_RESTART_GENERAL_ATTRIBUTES)NdisRestartAttributes->Data;

        //
        // Check to see if we need to change any attributes, for example, the driver can change the current
        // MAC address here. Or the driver can add media specific info attributes.
        //
    }
    NdisAcquireSpinLock(&Adapter->RcvLock);
    MP_INC_RCV_REF(Adapter);
    Adapter->AdapterState = NicRunning;

    DBGPRINT(MP_TRACE, ("FECRestart Adapter State is Running\n"));

    Mac1 = INREG32(&gpFECReg->PALR);
    Mac2 = INREG32(&gpFECReg->PAUR);

    DBGPRINT(MP_TRACE, ("Station Addr %02x-%02x-%02x-%02x-%02x-%02x \n",Mac1>>24,(Mac1>>16)&0xFF,(Mac1>>8)&0xFF,Mac1&0xFF,Mac2>>24,(Mac2>>16)&0xFF));

    NdisReleaseSpinLock(&Adapter->RcvLock);

    DBGPRINT(MP_TRACE, ("<==== FECRestart\n"));
    return NDIS_STATUS_SUCCESS;

}


NDIS_STATUS
FECPause(
    IN  NDIS_HANDLE                         MiniportAdapterContext,
    IN  PNDIS_MINIPORT_PAUSE_PARAMETERS     MiniportPauseParameters
    )
/*++

Routine Description:

    MiniportPause handler
    The driver can't indicate any packet, send complete all the pending send requests.
    and wait for all the packets returned to it.

Argument:

    MiniportAdapterContext  Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_PENDING

NOTE: A miniport can't fail the pause request

--*/
{
    PMP_ADAPTER         Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    NDIS_STATUS         Status;
    LONG                Count;

    UNREFERENCED_PARAMETER(MiniportPauseParameters);

    DBGPRINT(MP_TRACE, ("====> FECPause\n"));

    ASSERT(Adapter->AdapterState == NicRunning);

    NdisAcquireSpinLock(&Adapter->RcvLock);
    Adapter->AdapterState = NicPausing;
    DBGPRINT(MP_TRACE, ("Adapter State is Pausing \n"));

    NdisReleaseSpinLock(&Adapter->RcvLock);

    do
    {
        //
        // Complete all the pending sends
        //
        NdisAcquireSpinLock(&Adapter->SendLock);

        MpFreeQueuedSendNetBufferLists(Adapter);
        NdisReleaseSpinLock(&Adapter->SendLock);

        NdisAcquireSpinLock(&Adapter->RcvLock);
        MP_DEC_RCV_REF(Adapter);
        Count = MP_GET_RCV_REF(Adapter);
        if (Count ==0)
        {
            DBGPRINT(MP_TRACE, ("FECPause- Adapter State changed to Paused \n"));
            Adapter->AdapterState = NicPaused;
            Status = NDIS_STATUS_SUCCESS;
        }
        else
        {
            Status = NDIS_STATUS_PENDING;
            DBGPRINT(MP_TRACE, ("FECPause- in PENDING state-> Recv Pending Count %d\n", Count));
        }
        NdisReleaseSpinLock(&Adapter->RcvLock);
    }
    while (FALSE);

    DBGPRINT(MP_TRACE, ("<==== FECPause- Adapter State %d , NDIS State %d \n",Adapter->AdapterState, Status ));
    return Status;
}


VOID
FECFreeQueuedSendNetBufferLists(
    IN  PMP_ADAPTER  Adapter
    )
/*++
Routine Description:

    Free and complete the pended sends on SendWaitQueue
    Assumption: spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter

Return Value:

     None
NOTE: Run at DPC

--*/
{
    PQUEUE_ENTRY        pEntry;
    PNET_BUFFER_LIST    NetBufferList;
    PNET_BUFFER_LIST    NetBufferListToComplete = NULL;
    PNET_BUFFER_LIST    LastNetBufferList = NULL;
    NDIS_STATUS         Status = MP_GET_STATUS_FROM_FLAGS(Adapter);
    PNET_BUFFER         NetBuffer;

    DBGPRINT(MP_TRACE, ("--> MpFreeQueuedSendNetBufferLists\n"));

    while (!IsQueueEmpty(&Adapter->SendWaitQueue))
    {
        DBGPRINT(MP_TRACE, ("MpFreeQueuedSendNetBufferLists- clearing packets from the SendWaitQ\n"));

        pEntry = RemoveHeadQueue(&Adapter->SendWaitQueue);
        ASSERT(pEntry);

        Adapter->nWaitSend--;

        NetBufferList = MP_GET_NET_BUFFER_LIST_FROM_QUEUE_LINK(pEntry);

        NET_BUFFER_LIST_STATUS(NetBufferList) = Status;
        //
        // The sendLock is held
        //
        NetBuffer = MP_GET_NET_BUFFER_LIST_NEXT_SEND(NetBufferList);

        for (; NetBuffer != NULL; NetBuffer = NET_BUFFER_NEXT_NB(NetBuffer))
        {
            MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList)--;
        }
        //
        // If Ref count goes to 0, then complete it.
        // Otherwise, Send interrupt DPC would complete it later
        //
        if (MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList) == 0)
        {
            if (NetBufferListToComplete == NULL)
            {
                NetBufferListToComplete = NetBufferList;
            }
            else
            {
                NET_BUFFER_LIST_NEXT_NBL(LastNetBufferList) = NetBufferList;
            }
            NET_BUFFER_LIST_NEXT_NBL(NetBufferList) = NULL;
            LastNetBufferList = NetBufferList;

        }
    }
    if (NetBufferListToComplete != NULL)
    {
        MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);
        NdisMSendNetBufferListsComplete(
               MP_GET_ADAPTER_HANDLE(Adapter),
               NetBufferListToComplete,
               NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);

        MP_ACQUIRE_SPIN_LOCK(&Adapter->SendLock, TRUE);
    }
    DBGPRINT(MP_TRACE, ("<-- MpFreeQueuedSendNetBufferLists\n"));
}


VOID
FECReturnNetBufferLists(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNET_BUFFER_LIST    NetBufferLists,
    IN  ULONG               ReturnFlags
    )
/*++

Routine Description:

    MiniportReturnNetBufferLists handler
    NDIS calls this handler to return the ownership of one or more NetBufferLists and
    their embedded NetBuffers to the miniport driver.

Arguments:

    MiniportAdapterContext  Pointer to our adapter
    NetBufferLists          A linked list of NetBufferLists that miniport driver allocated for one or more
                            previous receive indications. The list can include NetBuferLists from different
                            previous calls to NdisMIndicateNetBufferLists.
    ReturnFlags             Flags specifying if the caller is at DISPATCH level

Return Value:

    None


--*/
{
    PMP_ADAPTER         Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    PMP_RFD             pMpRfd;
    ULONG               Count;
    PNET_BUFFER_LIST    NetBufList;
    PNET_BUFFER_LIST    NextNetBufList;
    BOOLEAN             DispatchLevel;
    int i =0;

    // Later we need to check if the request control size change
    // If yes, return the NetBufferList  to pool, and reallocate
    // one the RFD
    DispatchLevel = NDIS_TEST_RETURN_AT_DISPATCH_LEVEL(ReturnFlags);

    MP_ACQUIRE_SPIN_LOCK(&Adapter->RcvLock, DispatchLevel);

    for (NetBufList = NetBufferLists;
            NetBufList != NULL;
            NetBufList = NextNetBufList)
    {
        ++i;
        NextNetBufList = NET_BUFFER_LIST_NEXT_NBL(NetBufList);
        pMpRfd = MP_GET_NET_BUFFER_LIST_RFD(NetBufList);

        ASSERT(pMpRfd);
        ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RECV_PEND));
        MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RECV_PEND);

        //
        // Decrement the Power Mgmt Ref.
        //
        Adapter->PoMgmt.OutstandingRecv--;

        //
        // If we have set power request pending, then complete it
        //
        if (((Adapter->PendingRequest)
                && ((Adapter->PendingRequest->RequestType == NdisRequestSetInformation)
                && (Adapter->PendingRequest->DATA.SET_INFORMATION.Oid == OID_PNP_SET_POWER)))
                && (Adapter->PoMgmt.OutstandingRecv == 0))
        {
            MpSetPowerLowComplete(Adapter);
            DBGPRINT_S(MP_INFO, ("MPReturnNetBufferLists- Setting Status as Power Complete\n"));
        }

        NICReturnRFD(Adapter, pMpRfd);
        INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);


        // note that we get the ref count here, but check
        // to see if it is zero and signal the event -after-
        // releasing the SpinLock. otherwise, we may let the Halthandler
        // continue while we are holding a lock.
        //
        Count = MP_DEC_RCV_REF(Adapter);
    }

    Count =  MP_GET_RCV_REF(Adapter);
    if ((Count == 0) && (Adapter->AdapterState == NicPausing))
    {
        //
        // If all the NetBufferLists are returned and miniport is pausing,
        // complete the pause
        //
        DBGPRINT(MP_TRACE, ("MPReturnNetBufferLists- NIC was in pausing state. All Recv packet returned. Changing to Adapter Paused State \n"));

        Adapter->AdapterState = NicPaused;
        MP_RELEASE_SPIN_LOCK(&Adapter->RcvLock, DispatchLevel);
        NdisMPauseComplete(Adapter->AdapterHandle);
        MP_ACQUIRE_SPIN_LOCK(&Adapter->RcvLock, DispatchLevel);
    }
    MP_RELEASE_SPIN_LOCK(&Adapter->RcvLock, DispatchLevel);

    //
    // Only halting the miniport will set AllPacketsReturnedEvent
    //
    if (Count == 0)
    {
        NdisSetEvent(&Adapter->AllPacketsReturnedEvent);
    }
}


BOOLEAN
FECCheckForHangEx(
    IN  NDIS_HANDLE     MiniportAdapterContext
    )
/*++

Routine Description:

    MiniportCheckForHang handler
    Ndis call this handler forcing the miniport to check if it needs reset or not,
    If the miniport needs reset, then it should call its reset function

Arguments:

    MiniportAdapterContext  Pointer to our adapter

Return Value:

    None.

Note:
    CheckForHang handler is called in the context of a timer DPC.
    take advantage of this fact when acquiring/releasing spinlocks

    NOTE: NDIS60 Miniport won't have CheckForHang handler.

--*/
{
    PMP_ADAPTER         Adapter = (PMP_ADAPTER) MiniportAdapterContext;
#ifndef UNDER_CE
    NDIS_STATUS         Status;
#endif  //  UNDER_CE
    PMP_TCB             pMpTcb;
    BOOLEAN             NeedReset = TRUE;
#ifndef UNDER_CE
    NDIS_REQUEST_TYPE   RequestType;
#endif  //  UNDER_CE
#ifdef UNDER_CE
    BOOLEAN             DispatchLevel = FALSE;
#else  //  UNDER_CE
    BOOLEAN             DispatchLevel = (NDIS_CURRENT_IRQL() == DISPATCH_LEVEL);
#endif  //  UNDER_CE
    //
    // Just skip this part if the adapter is doing link detection
    //
    do
    {

       if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
        {
            NeedReset = FALSE;
            break;
        }
        //
        // any nonrecoverable hardware error?
        //
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_NON_RECOVER_ERROR))
        {
            DBGPRINT(MP_WARN, ("Non recoverable error - remove\n"));
            break;
        }
        //
        // hardware failure?
        //
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_HARDWARE_ERROR))
        {
            DBGPRINT(MP_WARN, ("hardware error - reset\n"));
            break;
        }
        MP_ACQUIRE_SPIN_LOCK(&Adapter->SendLock, DispatchLevel);

        //Check for send stuck need to be done only if the Adapter is in
        //D0 state
        if( Adapter->NextPowerState == NdisDeviceStateD0 )
        {
            //
            // Is send stuck?
            //
            if (Adapter->nBusySend > 0)
            {
                pMpTcb = Adapter->CurrSendHead;
                pMpTcb->Count++;
                if (pMpTcb->Count > NIC_SEND_HANG_THRESHOLD)
                {
                    MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, DispatchLevel);

                    RETAILMSG(TRUE, (TEXT("Waiting Send Packets Detected, Busy Count %d Pending Send %d @ %d \n"),Adapter->nBusySend, Adapter->nPendingSendPacket,  Adapter->HwSendTbdCurrentCount));

                    //Dump TBDs if send is hung!
                    RETAILMSG(TRUE,(TEXT("0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x \n"), 
                        Adapter->HwTbdChip[0]->ControlStatus,Adapter->HwTbdChip[1]->ControlStatus,Adapter->HwTbdChip[2]->ControlStatus,Adapter->HwTbdChip[3]->ControlStatus,
                        Adapter->HwTbdChip[4]->ControlStatus,Adapter->HwTbdChip[5]->ControlStatus,Adapter->HwTbdChip[6]->ControlStatus,Adapter->HwTbdChip[7]->ControlStatus));
                    RETAILMSG(TRUE,(TEXT("0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x \n"),
                        Adapter->HwTbdChip[8]->ControlStatus,Adapter->HwTbdChip[9]->ControlStatus,Adapter->HwTbdChip[10]->ControlStatus,Adapter->HwTbdChip[11]->ControlStatus,
                        Adapter->HwTbdChip[12]->ControlStatus,Adapter->HwTbdChip[13]->ControlStatus,Adapter->HwTbdChip[14]->ControlStatus,Adapter->HwTbdChip[15]->ControlStatus));
                    RETAILMSG(TRUE,(TEXT("0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x \n"), 
                        Adapter->HwTbdChip[16]->ControlStatus,Adapter->HwTbdChip[17]->ControlStatus,Adapter->HwTbdChip[18]->ControlStatus,Adapter->HwTbdChip[19]->ControlStatus,
                        Adapter->HwTbdChip[20]->ControlStatus,Adapter->HwTbdChip[21]->ControlStatus,Adapter->HwTbdChip[22]->ControlStatus,Adapter->HwTbdChip[23]->ControlStatus));
                    RETAILMSG(TRUE,(TEXT("0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x \n"), 
                      Adapter->HwTbdChip[24]->ControlStatus,Adapter->HwTbdChip[25]->ControlStatus,Adapter->HwTbdChip[26]->ControlStatus,Adapter->HwTbdChip[27]->ControlStatus,
                        Adapter->HwTbdChip[28]->ControlStatus,Adapter->HwTbdChip[29]->ControlStatus,Adapter->HwTbdChip[30]->ControlStatus,Adapter->HwTbdChip[31]->ControlStatus));
                    break;
                }
            }
        }
        NeedReset = FALSE;
        MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, DispatchLevel);
     
        //Start the link status check only if the Auto Nego is done
        if(TRUE == Adapter->MIISeqDone)
            FECUpdateLinkStatus(Adapter);

    }
    while (FALSE);

    //Dont reset if the adapter is already in low power state
    if( Adapter->NextPowerState != NdisDeviceStateD0)
        NeedReset = FALSE;

    if( TRUE == NeedReset)
        DBGPRINT(MP_WARN, ("FECCheckForHangEx- Requesting Reset to NDIS \n"));
    //
    // If need reset, ask NDIS to reset the miniport
    //
    return NeedReset;
}

NDIS_STATUS
FECResetEx(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    OUT PBOOLEAN        AddressingReset
    )
/*++

Routine Description:

    MiniportReset handler

Arguments:

    AddressingReset         To let NDIS know whether we need help from it with our reset
    MiniportAdapterContext  Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_PENDING
    NDIS_STATUS_RESET_IN_PROGRESS
    NDIS_STATUS_HARD_ERRORS

Note:

--*/
{
    NDIS_STATUS         Status;
    PNDIS_OID_REQUEST   PendingRequest;
    PMP_ADAPTER         Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    DBGPRINT(MP_TRACE, ("====> FECReset\n"));

    *AddressingReset = TRUE;

    NdisAcquireSpinLock(&Adapter->Lock);
    NdisDprAcquireSpinLock(&Adapter->SendLock);
    NdisDprAcquireSpinLock(&Adapter->RcvLock);

    do
    {
        ASSERT(!MP_TEST_FLAG(Adapter, fMP_ADAPTER_HALT_IN_PROGRESS));

        //
        // Is this adapter already doing a reset?
        //
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS))
        {
            DBGPRINT(MP_TRACE, ("FECReset- Reset is already in progress. Skipping this one\n"));

            Status = NDIS_STATUS_RESET_IN_PROGRESS;
            MP_EXIT;
        }
        MP_SET_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);
        //
        // Abort any pending request
        //
        if (Adapter->PendingRequest != NULL)
        {
            DBGPRINT(MP_TRACE, ("FECReset- There are pending requests\n"));

            PendingRequest = Adapter->PendingRequest;
            Adapter->PendingRequest = NULL;

            NdisDprReleaseSpinLock(&Adapter->RcvLock);
            NdisDprReleaseSpinLock(&Adapter->SendLock);
            NdisReleaseSpinLock(&Adapter->Lock);

            NdisMOidRequestComplete(Adapter->AdapterHandle,
                                    PendingRequest,
                                    NDIS_STATUS_REQUEST_ABORTED);

            NdisAcquireSpinLock(&Adapter->Lock);
            NdisDprAcquireSpinLock(&Adapter->SendLock);
            NdisDprAcquireSpinLock(&Adapter->RcvLock);

        }
        //
        // Is this adapter doing link detection?
        //
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
        {
            DBGPRINT(MP_WARN, ("FECReset Reset is pended...\n"));

            Status = NDIS_STATUS_PENDING;
            Adapter->bResetPending = TRUE;
            MP_EXIT;
        }
        //
        // Is this adapter going to be removed
        //
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_NON_RECOVER_ERROR))
        {
           DBGPRINT(MP_TRACE, ("FECReset- Adapter is currently in NON RECOVER ERROR State \n"));
           Status = NDIS_STATUS_HARD_ERRORS;
           if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_REMOVE_IN_PROGRESS))
           {
               MP_EXIT;
           }
           //
           // This is an unrecoverable hardware failure.
           // We need to tell NDIS to remove this miniport
           //
           MP_SET_FLAG(Adapter, fMP_ADAPTER_REMOVE_IN_PROGRESS);
           MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);

           NdisDprReleaseSpinLock(&Adapter->RcvLock);
           NdisDprReleaseSpinLock(&Adapter->SendLock);
           NdisReleaseSpinLock(&Adapter->Lock);
           NdisWriteErrorLogEntry(
               Adapter->AdapterHandle,
               NDIS_ERROR_CODE_HARDWARE_FAILURE,
               1,
               ERRLOG_REMOVE_MINIPORT);

           NdisMRemoveMiniport(Adapter->AdapterHandle);

           DBGPRINT_S(Status, ("<==== FECReset, NonRecoverable- Status=%x\n", Status));

           return Status;
        }
        //
        // Disable the interrupt and issue a reset to the NIC
        //
        NICDisableInterrupt(Adapter);
        FECEnetReset(Adapter,TRUE);    //HW reset

        //
        // release all the locks and then acquire back the send lock
        // we are going to clean up the send queues
        // which may involve calling Ndis APIs
        // release all the locks before grabbing the send lock to
        // avoid deadlocks
        //
        NdisDprReleaseSpinLock(&Adapter->RcvLock);
        NdisDprReleaseSpinLock(&Adapter->SendLock);
        NdisReleaseSpinLock(&Adapter->Lock);
        NdisAcquireSpinLock(&Adapter->SendLock);

        //
        // This is a deserialized miniport, we need to free all the send packets
        // Free the packets on SendWaitList
        //
        MpFreeQueuedSendNetBufferLists(Adapter);

        //
        // Free the packets being actively sent & stopped
        //
        FECFreeBusySendNetBufferLists(Adapter);
#if DBG
        if (MP_GET_REF(Adapter) > 1)
        {
            DBGPRINT(MP_WARN, ("RefCount=%d\n", MP_GET_REF(Adapter)));
        }
#endif
        NdisZeroMemory(Adapter->MpTcbMem, Adapter->NumTcb * sizeof(MP_TCB));

        //
        // Re-initialize the send structures
        //
        NICInitSend(Adapter);

        NdisReleaseSpinLock(&Adapter->SendLock);

        //
        // get all the locks again in the right order
        //
        NdisAcquireSpinLock(&Adapter->Lock);
        NdisDprAcquireSpinLock(&Adapter->SendLock);
        NdisDprAcquireSpinLock(&Adapter->RcvLock);

        //
        // Reset the RFD list and re-start RU
        //

        //clean up ds allocated
        NICResetRecv(Adapter);

        Adapter->HwErrCount = 0;
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_HARDWARE_ERROR);
        NICEnableInterrupt(Adapter);
    }
    while (FALSE);

    MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);

    exit:
    NdisDprReleaseSpinLock(&Adapter->RcvLock);
    NdisDprReleaseSpinLock(&Adapter->SendLock);
    NdisReleaseSpinLock(&Adapter->Lock);

    DBGPRINT_S(Status, ("<==== FECReset, Status=%x\n", Status));
    return(Status);
}
