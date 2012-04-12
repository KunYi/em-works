//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  ENET.c
//
//  Implementation of ENET Driver
//
//  This file implements NDIS interface for ENET Driver.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <windev.h>
#include <ceddk.h>
#pragma warning(pop)
#include "enet.h"


extern "C"  BOOL BSPENETusesKitlMode();

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
#define REG_DEVINDEX_VAL_NAME L"Index"
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

ENETClass* gpENET;

//We support dual MAC So we need define two global variable
PVOID pv_HWregENET0=NULL ;
PVOID pv_HWregENET1=NULL ;

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
    UNREFERENCED_PARAMETER(lpvReserved);
    
    switch(Op)
    {
        case DLL_PROCESS_ATTACH:
            DEBUGMSG(ZONE_INIT, (TEXT("ENET: DLL Process Attach.\r\n")));
         
            // Disables the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT, (TEXT("ENET: DLL Process Detach.\r\n")));
            break;

        default:
            break;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: GetEnentIndexFromReg
//
// This function accesses the interrupt mask register to disable the RxF, TxF
// and MII interrupts.
//
// Parameters:
//        RegistryPath
//            [in] Specifies the Registry Path of enet driver load
//                 
//                 
//
// Return Value:
//        Enet index number.
//
//------------------------------------------------------------------------------
int GetEnentIndexFromReg(LPCTSTR RegistryPath)
{
    LONG Result;
    HKEY  hKey;
    DWORD disposition;
    DWORD dwIndex=0,error,dwSize,type; 
    
    // try to open active device registry key for this context
    Result = RegCreateKeyEx (HKEY_LOCAL_MACHINE, RegistryPath, 0, 
                                    TEXT(""), 0, KEY_ALL_ACCESS, NULL,  
                                    &hKey, &disposition);
    
    if (Result != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (TEXT("ENET Driver:  OpenDeviceKey failed!!! %s\r\n"),RegistryPath));
        return -1;
    } 

    // try to load ENET driver index from registry data
    dwSize = sizeof(DWORD);
    type =REG_DWORD;
    error = RegQueryValueEx(
        hKey,                       // handle to currently open key
        REG_DEVINDEX_VAL_NAME,      // string containing value to query
        NULL,                       // reserved, set to NULL
        &type ,                       // type not required, set to NULL
        (LPBYTE)(&dwIndex),      // pointer to buffer receiving value
        &dwSize);               // pointer to buffer size

    // check for errors during RegQueryValueEx
    if (error != ERROR_SUCCESS)
    {
       DEBUGMSG(ZONE_ERROR, (TEXT("ENET Driver:  RegQueryValueEx failed!!!Err:%d  dwIndex :%x\r\n"),GetLastError(),dwIndex));
       return -1;
    }

    // close handle to open key
    RegCloseKey(hKey);

    
    return dwIndex;

}

//------------------------------------------------------------------------------
//
// Function: DriverEntry
//
// This function is the main entry of driver. It initializes and registers ENET
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
//        Returns STATUS_SUCCESS if initialization is success. Return 
//        STATUS_UNSUCCESSFUL if not.
//
//------------------------------------------------------------------------------
#pragma NDIS_INIT_FUNCTION(DriverEntry)
NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    NDIS_STATUS Status;    
    DWORD dwIndex;

    // Global var of ENET Ndis Handler
    NDIS_HANDLE  gENETNdisHandle;

    DEBUGMSG (ZONE_INIT, (TEXT("ENET Driver +:%x %s\r\n"),DriverObject,RegistryPath->Buffer));

    if(BSPENETusesKitlMode())
        return NDIS_STATUS_FAILURE;

    //Get the enet index 
    dwIndex=GetEnentIndexFromReg(RegistryPath->Buffer);
    
    gpENET = new ENETClass(dwIndex);

    // Managed to create the class?
    if (gpENET == NULL)
    {
        return NULL;
    }
    
    DEBUGMSG(ZONE_INIT, (TEXT("ENET: +DriverEntry(0x%.8X, 0x%.8X)\r\n"), DriverObject, RegistryPath));

    // Notifies NDIS for new miniport driver
    NdisMInitializeWrapper(&gENETNdisHandle, DriverObject, RegistryPath, NULL);
    
    // Fill-in adapter characterictics before calling NdisMRegisterMiniport
    memset(&gpENET->ENETChar, 0, sizeof(NDIS_MINIPORT_CHARACTERISTICS));
    
    gpENET->ENETChar.Ndis30Chars.MajorNdisVersion = ENET_NDIS_MAJOR_VERSION;
    gpENET->ENETChar.Ndis30Chars.MinorNdisVersion = ENET_NDIS_MINOR_VERSION;
    gpENET->ENETChar.Ndis30Chars.CheckForHangHandler = ENETCheckForHang;
    gpENET->ENETChar.Ndis30Chars.DisableInterruptHandler = ENETDisableInterrupt;
    gpENET->ENETChar.Ndis30Chars.EnableInterruptHandler =ENETEnableInterrupt;
    gpENET->ENETChar.Ndis30Chars.HaltHandler = ENETHalt;
    gpENET->ENETChar.Ndis30Chars.HandleInterruptHandler = ENETHandleInterrupt; 
    gpENET->ENETChar.Ndis30Chars.InitializeHandler = ENETInitialize;
    gpENET->ENETChar.Ndis30Chars.ISRHandler = ENETIsr;
    gpENET->ENETChar.Ndis30Chars.QueryInformationHandler = ENETQueryInformation;
    gpENET->ENETChar.Ndis30Chars.ReconfigureHandler = NULL;
    gpENET->ENETChar.Ndis30Chars.ResetHandler = ENETReset;
    gpENET->ENETChar.Ndis30Chars.SendHandler = ENETSend;
    gpENET->ENETChar.Ndis30Chars.SetInformationHandler = ENETSetInformation;
    gpENET->ENETChar.Ndis30Chars.TransferDataHandler = ENETTransferData;
    
    // Now register Miniport
    Status = NdisMRegisterMiniport(gENETNdisHandle, &gpENET->ENETChar, sizeof(NDIS_MINIPORT_CHARACTERISTICS));
    
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ENET: NdisMRegisterMiniport failure [0x%.8X].\r\n"), Status));
        NdisTerminateWrapper(gENETNdisHandle, NULL);
    }
    
    DEBUGMSG(ZONE_INIT, (TEXT("ENET: -DriverEntry [0x%.8X]\r\n"), Status));

    return (Status);
}

//------------------------------------------------------------------------------
//
// Function: ENETCheckForHang
//
// This function reports the link status of the ENET network adapter. Bu default,
// the NDIS library calls this function approximately every two seconds.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to a ENET driver allocated context area
//                 in which the ENET driver maintains ENET adapter state, set up
//                 by ENETInitialize function.
//
// Return Values:
//        return TRUE if this function detects that the network adapter is not
//        operating correctly, otherwise FALSE.
//
//------------------------------------------------------------------------------
BOOLEAN ENETCheckForHang(IN NDIS_HANDLE MiniportAdapterContext)
{

    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETCheckForHang\r\n")));
    
     
    pEthernet->pENET->ENETCheckForHang(MiniportAdapterContext);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETCheckForHang\r\n")));
    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: ENETDisableInterrupt
//
// This function accesses the interrupt mask register to disable the RxF, TxF
// and MII interrupts.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETDisableInterrupt\r\n")));

    pEthernet->pENET->ENETDisableInterrupt(MiniportAdapterContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETDisableInterrupt\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: ENETEnableInterrupt
//
// This function accesses the interrupt mask register to enable the RxF, TxF
// and MII interrupts.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{   
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETDisableInterrupt\r\n")));

    pEthernet->pENET->ENETEnableInterrupt(MiniportAdapterContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETDisableInterrupt\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: ENETHalt
//
// ENETHalt is a required function that de-allocates resources when the ENET 
// adapter is removed and halts the network adapter.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to a ENET allocated context area in 
//                 which the ENET driver maintains ENET adapter state,
//                 set up by ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETHalt(IN NDIS_HANDLE MiniportAdapterContext)
{
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETDisableInterrupt\r\n")));

    pEthernet->pENET->ENETHalt(MiniportAdapterContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETDisableInterrupt\r\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: ENETTransferData
// 
// This function copies the content of the received packet to a given 
// protocol-allocated packet.
// 
// Parameters:
//        Packet
//            [out] Point to a packet descriptor with chained buffers into which
//                  ENETTransferData should copy the received data
//
//        BytesTransferred
//            [out] Points to a variable that ENETTransferData sets to the number
//                  of bytes it  copied into the packet
//
//        MiniportAdapterContext
//            [in]  Specifies the handle to a miniport-allocated context area in 
//                  which the driver maintains ENET adapter state, set up by
//                  ENETInitialize
//
//        MiniportReceiveContext
//            [in]  Specifies the context handle previously passed to 
//                  NdisMEthIndicateReceive. The miniport can examine this value
//                  to determine which recieve to copy
//
//        ByteOffset
//            [in]  Specifies the offset within the received packet at which
//                  ENETTransferData should begin the copy. If the entire packet
//                  is to be copied, should be set to zero.
//
//        BytesToTransfer
//            [in]  Specifies how many bytes to copy
//
// Return Value:
//        Returns NDIS_STATUS_SUCCESS if success
//        Return NDIS_STATUS_FAILURE if not
//
//------------------------------------------------------------------------------
NDIS_STATUS ENETTransferData(
    OUT PNDIS_PACKET  Packet,
    OUT PUINT  BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT  ByteOffset,
    IN UINT  BytesToTransfer
    )
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    return pEthernet->pENET->ENETTransferData(Packet,BytesTransferred,MiniportAdapterContext,MiniportReceiveContext,ByteOffset,BytesToTransfer);
}


//------------------------------------------------------------------------------
//
// Function: ENETSetInformation
//
// This function is a required function that allows NDIS to request changes in
// the state information that the ENET driver maintains for particular object 
// identifiers, such as changes in multicast addresses.
// 
// Parameters:
//     MiniportAdapterContext
//             [in]  Specifies the handle to a ENET driver allocated context area
//                   in which the ENET driver maintains ENET adapter state, set up
//                   by ENETInitialize function.
//
//     Oid
//             [in]  Specifies the system-defined OID_XXX code designating the 
//                   set operation the ENET driver should carry out.
//
//     InformationBuffer
//             [in]  Points to a buffer containing the OID-specific data used by
//                   ENETSetInformation for the set.
//
//     InformationBufferLength
//             [in]  Specifies the number of bytes at InformationBuffer.
//
//     BytesRead
//             [out] Points to a variable that ENETSetInformation sets to the number
//                   of bytes it read from the buffer at InformationBuffer.
//
//     BytesNeeded
//             [out] Points to a variable that ENETSetInformation sets to the number
//                   of additional bytes it needs to satisfy the request if 
//                   InformationBufferLength is less than Oid requires.
//
// Return Value:
//     Return NDIS_STATUS_SUCCESS if the required Oid is set successfully.
//     Return NDIS_STATUS_INVALID_LENGTH if the InformationBufferLength does not 
//     match the length required by the given Oid.
//
//---------------------------------------------------------------------------------
NDIS_STATUS ENETSetInformation (
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    )
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    return pEthernet->pENET->ENETSetInformation(MiniportAdapterContext,Oid,InformationBuffer,InformationBufferLength,BytesRead,BytesNeeded);
}

//------------------------------------------------------------------------------
//
// Function: ENETReset
//
// This function is a required funtion that issues a hardware reset to the
// network adapter and/or resets the driver's software state. If ENETCheckForHang
// returns TRUE, this function will be called to reset the ENET adapter.
//
// Parameters:
//        AddressingReset
//            [out] Points to a variable that ENETReset sets to TRUE if the NDIS
//                  library call ENETSetInformation to restore addressing 
//                  information to the current values.
//
//        MiniportAdapterContext
//            [in]  Specifies the handle to the driver allocated context area in
//                  which the driver maintains ENET adapter state, set up by
//                  ENETInitialize.
//
// Return Value:
//        Return NDIS_STATUS_SUCCESS always.
//
//------------------------------------------------------------------------------
NDIS_STATUS ENETReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    return pEthernet->pENET->ENETReset(AddressingReset,MiniportAdapterContext);
}

//------------------------------------------------------------------------------
//
// Function: ENETSend
//
// This function is responsible for sending the packet to the ENET and start the 
// sending process.
// 
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
//        Packet
//            [in] Points to a packet descriptor specifying the data to be transmitted
//
//        SendFlags
//            [in] Specifies the packet flags, if any, set by the protocol
//
// Return Value:
//        Returns NDIS_STATUS_PENDING if success 
//        Returns NDIS_STATUS_FAILURE if bad frame encountered
//  
//------------------------------------------------------------------------------
NDIS_STATUS ENETSend(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT SendFlags
    )
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    return pEthernet->pENET->ENETSend(MiniportAdapterContext,Packet,SendFlags);
}

//------------------------------------------------------------------------------
//
// Function: ENETShutdown
//
// Restore the ENET adapter to its initial state when the system is shut down, 
// whether by the user or because an unrecoverable system error occurred.
//
// Parameters:
//        ShutdownContext
//            [in] Points to a context area supplied when the ENETInitialize
//                 function calls NdisMRegisterAdapterShutdownHandler.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETShutdown(
    IN PVOID ShutdownContext
    )
{
        
    gpENET->ENETShutdown(ShutdownContext);
}

//------------------------------------------------------------------------------
//
// Function: ENETQueryInformation
//
// ENETQueryInformation is a required function that returns information about
// the capabilities and status of the ENET driver. The NDIS library makes one
// or more calls to ENETQueryInformation just after ENETInitialize function
// returns NDIS_STATUS_SUCCESS.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
//        Oid
//            [in] Specifies the system-defined OID_XXX code designating the golbal
//                 query operation this function should carry out
//
//        InformationBuffer
//            [in] Points to a buffer in which ENETQueryInformation should return
//                 the OID-specific information
//
//        InformationBufferLength
//            [in] Specifies the number of bytes at InformationBuffer
//
//        BytesWritten
//            [out] Points to a variable that ENETQueryInformation sets to the number
//                  of bytes it is returning at InformationBuffer
//
//        BytesNeeded
//            [out] Points to a variable that ENETQueryInformation sets to the number
//                  of additional bytes it needs to satisfy the request if 
//                  InformationBufferLength is less than Oid requires
//
// Return Values:
//        returns NDIS_STATUS_SUCCESS if ENETQueryInformation returned the requested
//        information at InformationBuffer and set the variable at BytesWritten to
//        the amount of information it returned
//
//        returns NDIS_STATUS_INVALID_OID if the requested Oid is not supported
//
//        returns NDIS_STATUS_INVALID_LENGTH if The InformationBufferLength does
//        not match the length required by the given Oid
// 
//------------------------------------------------------------------------------
NDIS_STATUS ENETQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    )
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    return pEthernet->pENET->ENETQueryInformation(MiniportAdapterContext,Oid,InformationBuffer,InformationBufferLength,BytesWritten,BytesNeeded);
}

//------------------------------------------------------------------------------
//
// Function: ENETIsr
//
// The ENET adapter driver should do as little work as possible in the ENETIsr,
// deferring I/O operations for each interrupt the ENET adapter generates to 
// the ENETHandleInterrupt function.
//
// Parameters:
//        InterruptRecognized
//            [out] Points to a variable in which ENETISR returns whether
//                  the ENET adapter actually generated the interrupt.
//
//        QueueMiniportHandleInterrupt
//            [out] Points to a variable that ENETISR sets to TRUE if the 
//                  ENETHandleInterrupt function should be called to complete
//                  the interrupt-driven I/O operation.
//
//        MiniportAdapterContext
//            [in]  Specifies the handle to the driver allocated context area in
//                  which the driver maintains ENET adapter state, set up by
//                  ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN  NDIS_HANDLE MiniportAdapterContext
    )
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    return pEthernet->pENET->ENETIsr(InterruptRecognized,QueueMiniportHandleInterrupt,MiniportAdapterContext);
}

//------------------------------------------------------------------------------
//
// Function: ENETInitialize
//
// This function finds and initializes the ENET adapter. When the ENET driver calls 
// the NdisMRegisterMiniport from its DriverEntry function, NDIS will call 
// ENETInitialize in the context of NdisMRegisterMiniport.
//
// Parameters:
//        OpenErrorStatus
//            [out] Points to a variable that ENETInitialize sets to an NDIS_STATUS_XXX
//                  code information about the error if ENETInitialize will return
//                  NDIS_STATUS_OPEN_ERROR.
//
//        SelectedMediumIndex
//            [out] Points to a variable in which ENETInitialize sets the index of 
//                  MediumArray element that specifies the medium type the ENET adapter 
//                  uses.
//
//        MediumArray
//            [in]  Specifies an array of NdisMediumXXX values from which ENETInitialize
//                  selects one that the ENET adapter supports.
//
//        MediumArraySize
//            [in]  Specifies the number of elements at MediumArray.
//
//        MiniportAdapterHandle
//            [in]  Specifies a handle identifying the ENET driver, which is assigned by
//                  the NDIS library. It is a required parameter in subsequent calls
//                  to NdisXXX functions.
//
//        WrapperConfigurationContext
//            [in]  Specifies a handle used only during initialization for calls to
//                  NdisXXX configuration and initialization functions. For example, 
//                  this handle is a required parameter to NdisOpenConfiguration 
//                  function.
//
// Returns:
//        Returns NDIS_STATUS_SUCCESS if initialization is success.
//        Return NDIS_STATUS_FAILURE or NDIS_STATUS_UNSUPPORTED_MEDIA if not.
//
//------------------------------------------------------------------------------  
NDIS_STATUS ENETInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext)

{

    return gpENET->ENETInitialize(OpenErrorStatus,SelectedMediumIndex,MediumArray,MediumArraySize,MiniportAdapterHandle,WrapperConfigurationContext);
}

//------------------------------------------------------------------------------
//
// Function: ENETHandleInterrupt
//
// This function handles the interrupt events and calls the respective subroutines
// according to event.
//
// Parameters:
//        MiniportAdapterContext
//            [in]  Specifies the handle to the driver allocated context area in
//                  which the driver maintains ENET adapter state, set up by
//                  ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    return pEthernet->pENET->ENETHandleInterrupt(MiniportAdapterContext); 
}

//------------------------------------------------------------------------------
//
// Function: ENETParseMIICr
//
// This function parses the control register data of the external MII compatible
// PHY(s).
//
// Parameters:
//        RegVal
//            [in] the control register value get from external MII compatible
//                 PHY(s)
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETParseMIICr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETParseMIICr(RegVal,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: ENETParseMIIAnar
//
// This function parses the Auto-Negotiation Advertisement Register data of the  
// external MII compatible PHY(s).
//
// Parameters:
//        RegVal
//            [in] the Auto-Negotiation Advertisement Register value get from 
//                 external MII compatible PHY(s)
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETParseMIIAnar(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETParseMIIAnar(RegVal,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: ENETParseAm79c874Dr
//
// This function parses the diagnostic register data of the external Am79c874
// PHY.
//
// Parameters:
//        RegVal
//            [in] the Adiagnostic Register value get from external Am79c874 PHY
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETParseAm79c874Dr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETParseAm79c874Dr(RegVal,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: ENETParseLAN87xxSCSR
//
// This function parses the diagnostic register data of the external Am79c874
// PHY.
//
// Parameters:
//        RegVal
//            [in] the Adiagnostic Register value get from external Am79c874 PHY
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETParseLAN87xxSCSR(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETParseLAN87xxSCSR(RegVal,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: ENETParseDP83640PHYSTS
//
// This function parses the diagnostic register data of the external DP83640
// PHY.
//
// Parameters:
//        RegVal
//            [in] the Adiagnostic Register value get from external DP83640 PHY
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETParseDP83640PHYSTS(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETParseDP83640PHYSTS(RegVal,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: ENETParseMIISr
//
// This function parses the status register data of the external MII compatible
// PHY(s).
//
// Parameters:
//        RegVal
//            [in] the status register value get from external MII compatible
//                 PHY(s)
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETParseMIISr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETParseMIISr(RegVal,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: ENETDispPHYCfg
//
// This function displays the current status of the external PHY
//
// Parameters:
//        MIIReg
//            [in] the MII frame value which is read from external PHY registers
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETDispPHYCfg(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETDispPHYCfg(RegVal,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: ENETParsePHYLink
//
// This function will update the link status according to the Status register
// of the external PHY.
// 
// Parameters:
//        RegVal
//            [in] the MII frame value which is read from external PHY registers
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETParsePHYLink(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETParsePHYLink(RegVal,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: ENETGetLinkStatus
//
// This function will send the command to the external PHY to get the link 
// status of the cable. The updated link status is stored in the context
// area designated by the parameter MiniportAdapterContext.
//
// Parameters:
//        MiniportAdapterHandle
//            [in] Specifies the handle to a ENET driver allocated context area
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETGetLinkStatus(IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETGetLinkStatus(MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: GetENETPHYId
// 
// Scan all of the MII PHY addresses looking for someone to respond with a valid
// ID. This usually happens quickly.
//
// Parameters:
//        MIIReg
//            [in] the MII frame value which is read from external PHY registers
//
//      MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void GetENETPHYId(IN UINT MIIReg,IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETGetPHYId(MIIReg,MiniportAdapterHandle);
}

//------------------------------------------------------------------------------
//
// Function: GetENETPHYId2
//
// Read the second part of the external PHY id.
//
// Parameters:
//        MIIReg
//            [in] the MII frame value which is read from external PHY registers
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in 
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void GetENETPHYId2(IN UINT MIIReg,IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    return pEthernet->pENET->ENETGetPHYId2(MIIReg,MiniportAdapterHandle);
}


