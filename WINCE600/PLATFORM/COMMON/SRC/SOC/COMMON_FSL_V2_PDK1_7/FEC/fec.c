//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "fec.h"


extern DWORD BSPFECGetIrq();
extern DWORD BSPFECBaseAddress();

extern BOOL BSPFECIomuxConfig( IN BOOL Enable );
extern BOOL BSPFECClockConfig( IN BOOL Enable );
extern void BSPFECPowerEnable(BOOL bEnable);
void FECEnetPowerHandle(pFEC_t pEthernet,BOOL En);

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Global var of FEC Ndis Handler
NDIS_HANDLE  gFECNdisHandle;
PCSP_FEC_REGS     gpFECReg = NULL;

CRITICAL_SECTION gFECRegCs;
CRITICAL_SECTION gFECBufCs;

//-----------------------------------------------------------------------------
// Local Variables
//-----------------------------------------------------------------------------
// This constant is used for places where NdisAllocateMemory
// needs to be called and the HighestAcceptableAddress does
// not matter.
static NDIS_PHYSICAL_ADDRESS HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);

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
    /* powermanagement */
    OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,
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
    UNREFERENCED_PARAMETER(lpvReserved);
    
    switch(Op)
    {
        case DLL_PROCESS_ATTACH:
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
//        Returns STATUS_SUCCESS if initialization is success. Return 
//        STATUS_UNSUCCESSFUL if not.
//
//------------------------------------------------------------------------------
#pragma NDIS_INIT_FUNCTION(DriverEntry)
NTSTATUS DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    NDIS_STATUS Status;
    NDIS_MINIPORT_CHARACTERISTICS FECChar; // Characteristics table for the FEC driver
    
    //DEBUGMSG(ZONE_INIT, (TEXT("FEC: +DriverEntry(0x%.8X, 0x%.8X)\r\n"), DriverObject, RegistryPath));
    RETAILMSG(1, (TEXT("FEC: +DriverEntry(0x%.8X, 0x%.8X)\r\n"), DriverObject, RegistryPath));

    // Notifies NDIS for new miniport driver
    NdisMInitializeWrapper(&gFECNdisHandle, DriverObject, RegistryPath, NULL);
    
    // Fill-in adapter characterictics before calling NdisMRegisterMiniport
    memset(&FECChar, 0, sizeof(FECChar));
    
    FECChar.MajorNdisVersion = FEC_NDIS_MAJOR_VERSION;
    FECChar.MinorNdisVersion = FEC_NDIS_MINOR_VERSION;
    FECChar.CheckForHangHandler = FECCheckForHang;
    FECChar.DisableInterruptHandler = FECDisableInterrupt;
    FECChar.EnableInterruptHandler = FECEnableInterrupt;
    FECChar.HaltHandler = FECHalt;
    FECChar.HandleInterruptHandler = FECHandleInterrupt; 
    FECChar.InitializeHandler = FECInitialize;
    FECChar.ISRHandler = FECIsr;
    FECChar.QueryInformationHandler = FECQueryInformation;
    FECChar.ReconfigureHandler = NULL;
    FECChar.ResetHandler = FECReset;
    FECChar.SendHandler = FECSend;
    FECChar.SetInformationHandler = FECSetInformation;
    FECChar.TransferDataHandler = FECTransferData;
    
    // Now register Miniport
    Status = NdisMRegisterMiniport(gFECNdisHandle, &FECChar, sizeof(FECChar));
    
    if (Status != NDIS_STATUS_SUCCESS)
    {
        //DEBUGMSG(ZONE_ERROR, (TEXT("FEC: NdisMRegisterMiniport failure [0x%.8X].\r\n"), Status));
        RETAILMSG(1, (TEXT("FEC: NdisMRegisterMiniport failure [0x%.8X].\r\n"), Status));
        NdisTerminateWrapper(gFECNdisHandle, NULL);
    }
    
    //DEBUGMSG(ZONE_INIT, (TEXT("FEC: -DriverEntry [0x%.8X]\r\n"), Status));
    RETAILMSG(1, (TEXT("FEC: -DriverEntry [0x%.8X]\r\n"), Status));

    return (Status);
}

__inline 
BOOLEAN  
FECIsPoMgmtSupported(
   IN NDIS_HANDLE MiniportAdapterContext
   )
{
        UNREFERENCED_PARAMETER(MiniportAdapterContext);
        return TRUE;
 
    
}

//------------------------------------------------------------------------------
//
// Function: FECFillPoMgmtCaps
//
// FECFillPoMgmtCaps is a required function that set power mg caps 
//
// Parameters:
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------


VOID
FECFillPoMgmtCaps (
    IN NDIS_HANDLE MiniportAdapterContext, 
    IN OUT PNDIS_PNP_CAPABILITIES  pPower_Management_Capabilities, 
    IN OUT PNDIS_STATUS            pStatus,
    IN OUT PULONG                  pulInfoLen
    )

{

    BOOLEAN bIsPoMgmtSupported; 
    
    bIsPoMgmtSupported = FECIsPoMgmtSupported(MiniportAdapterContext);

    if (bIsPoMgmtSupported == TRUE)
    {
        pPower_Management_Capabilities->Flags = NDIS_DEVICE_WAKE_UP_ENABLE ;
        pPower_Management_Capabilities->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPower_Management_Capabilities->WakeUpCapabilities.MinPatternWakeUp = NdisDeviceStateD3;
        pPower_Management_Capabilities->WakeUpCapabilities.MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;
        *pulInfoLen = sizeof (*pPower_Management_Capabilities);
        *pStatus = NDIS_STATUS_SUCCESS;
    }
    else
    {
        NdisZeroMemory (pPower_Management_Capabilities, sizeof(*pPower_Management_Capabilities));
        *pStatus = NDIS_STATUS_NOT_SUPPORTED;
        *pulInfoLen = 0;
            
    }
}


//------------------------------------------------------------------------------
//
// Function: GetMacAdd
//
// GetMacAdd is a required function that get MAC address from SOC UUID 
//
// Parameters:
//        FecMacAddress
//            [in] Specifies the handle to a FEC MAC Address 
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------

BOOL GetMacAdd(unsigned char FecMacAddress[6])
{
   
    GUID uuid;
   
    DWORD dwSize = sizeof(uuid);
    UINT32 spiValue = SPI_GETUUID;
    if(FALSE==KernelIoControl(IOCTL_HAL_GET_DEVICE_INFO, &spiValue,sizeof(spiValue), &uuid,dwSize, &dwSize))
        return FALSE;
   
    if((uuid.Data1==0)&&(uuid.Data2==0)&&(uuid.Data3==0)&&(uuid.Data4[0]==0)&&(uuid.Data4[1]==0)&&(uuid.Data4[2]==0))
        return FALSE;
    else
    {
        FecMacAddress[0]=(unsigned char)uuid.Data1;
        FecMacAddress[1]=(unsigned char)uuid.Data2;
        FecMacAddress[2]=(unsigned char)uuid.Data3;
        FecMacAddress[3]=uuid.Data4[0];
        FecMacAddress[4]=uuid.Data4[1];
        FecMacAddress[5]=uuid.Data4[2];
       
    }
        
    return TRUE;
}




//------------------------------------------------------------------------------
//
// Function: FECSetPower
//
// FECSetPower is a required function that set FEC power state
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize

//        PowerState
//            [in] Specifies the  FEC power state

//
// Return Value:
//        TURE indicate sucess,F ALSE indicate fail.
//
//------------------------------------------------------------------------------

VOID
FECSetPower(
     NDIS_HANDLE MiniportAdapterContext ,
     NDIS_DEVICE_POWER_STATE   PowerState 
    )

{

    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    if (PowerState == NdisDeviceStateD0)
    {
      
        BSPFECPowerEnable(TRUE);
        BSPFECClockConfig( TRUE);
        FECEnetPowerHandle(pEthernet,TRUE);     
        FECEnableInterrupt(MiniportAdapterContext);
        pEthernet->MediaStateChecking = FALSE;
        ProcessMIIInterrupts(pEthernet);

    }
    else
    {
        pEthernet->MediaStateChecking = TRUE;
        FECDisableInterrupt(MiniportAdapterContext); 
        FECEnetPowerHandle(pEthernet,FALSE);
        BSPFECPowerEnable(FALSE); 
        NdisMSleep(5);
        BSPFECClockConfig( FALSE);
        
    }
     
      
}


//------------------------------------------------------------------------------
//
// Function: FECInitialize
//
// This function finds and initializes the FEC adapter. When the FEC driver calls 
// the NdisMRegisterMiniport from its DriverEntry function, NDIS will call 
// FECInitialize in the context of NdisMRegisterMiniport.
//
// Parameters:
//        OpenErrorStatus
//            [out] Points to a variable that FECInitialize sets to an NDIS_STATUS_XXX
//                  code information about the error if FECInitialize will return
//                  NDIS_STATUS_OPEN_ERROR.
//
//        SelectedMediumIndex
//            [out] Points to a variable in which FECInitialize sets the index of 
//                  MediumArray element that specifies the medium type the FEC adapter 
//                  uses.
//
//        MediumArray
//            [in]  Specifies an array of NdisMediumXXX values from which FECInitialize
//                  selects one that the FEC adapter supports.
//
//        MediumArraySize
//            [in]  Specifies the number of elements at MediumArray.
//
//        MiniportAdapterHandle
//            [in]  Specifies a handle identifying the FEC driver, which is assigned by
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
#pragma NDIS_PAGEABLE_FUNCTION(FECInitialize)
static NDIS_STATUS FECInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext)
{
    // Pointer to our newly allocated adapter
    pFEC_t pEthernet = NULL;
    
    // The handle for reading from the registry
    NDIS_HANDLE ConfigHandle = 0;
    
    UINT32 i;
    ULONG  MemPhysAddr;
    //NDIS_PHYSICAL_ADDRESS   PhysicalAddress;
    NDIS_STATUS Status=NDIS_STATUS_SUCCESS, MACStatus;
    BOOL result;
    PNDIS_CONFIGURATION_PARAMETER Value;
    USHORT TmpVal;
    BOOL cheat = FALSE;
    
    NDIS_STRING MACAddrString[3]= { NDIS_STRING_CONST("MACAddress1"),
                                    NDIS_STRING_CONST("MACAddress2"),
                                    NDIS_STRING_CONST("MACAddress3")};
                                    
    //PHYSICAL_ADDRESS phyAddr;
    // WORD InterruptEvent;

    UNREFERENCED_PARAMETER(OpenErrorStatus);
    
    //DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECInitialize\r\n")));
    RETAILMSG(1, (TEXT("FEC: +FECInitialize\r\n")));			// CS&ZHL JUN-1-2011: debug 

    InitializeCriticalSection (&gFECRegCs);
    InitializeCriticalSection (&gFECBufCs);
    
    do {
        // Step 1: Search for medium type (802.3) in the given array
        for (i = 0;i < MediumArraySize; i++) {
            if (MediumArray[i] == NdisMedium802_3) {
                *SelectedMediumIndex = i;
                break;
            }
        }
        
        if (i == MediumArraySize) {
            DEBUGMSG(ZONE_ERROR, (TEXT("FEC: No supported media\r\n")));
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }
        
        *SelectedMediumIndex = i;
        DEBUGMSG(ZONE_INIT, (TEXT("FEC: Search for media success. \r\n")));
        
        // Step 2: Allocate memory for the adapter block now
        Status = NdisAllocateMemory( (PVOID *)&pEthernet, sizeof(FEC_t), 0, HighestAcceptableMax);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("FEC: NdisAllocateMemory(FEC_t) failed\n")));
            break;
        }
        
        DEBUGMSG(ZONE_INIT, (TEXT("FEC: Allocate memory for the adapter block success. \r\n")));
        
        
        // Step 3: Clear out and initialize the pEthernet block
        NdisZeroMemory (pEthernet, sizeof(FEC_t));
        pEthernet->ndisAdapterHandle = MiniportAdapterHandle;
        pEthernet->CurrentState = NdisHardwareStatusNotReady;
        pEthernet->CurrentLookAhead = 1500;
        pEthernet->HeadPacket = pEthernet->TailPacket = NULL;
        pEthernet->TransmitInProgress = FALSE;
        pEthernet->StartTx = FALSE;
        pEthernet->SpeedMode = FALSE;
        
        pEthernet->RcvStatus.FrameRcvGood = 0;
        pEthernet->RcvStatus.FrameRcvErrors = 0;
        pEthernet->RcvStatus.FrameRcvExtraDataErrors = 0;
        pEthernet->RcvStatus.FrameRcvShortDataErrors = 0;
        pEthernet->RcvStatus.FrameRcvCRCErrors = 0;
        pEthernet->RcvStatus.FrameRcvOverrunErrors = 0;
        pEthernet->RcvStatus.FrameRcvAllignmentErrors = 0;
        pEthernet->RcvStatus.FrameRcvLCErrors = 0;
        
        pEthernet->TxdStatus.FramesXmitGood = 0;
        pEthernet->TxdStatus.FramesXmitBad = 0;
        pEthernet->TxdStatus.FramesXmitHBErrors = 0;
        pEthernet->TxdStatus.FramesXmitUnderrunErrors = 0;
        pEthernet->TxdStatus.FramesXmitCollisionErrors = 0;
        pEthernet->TxdStatus.FramesXmitAbortedErrors = 0;
        pEthernet->TxdStatus.FramsXmitCarrierErrors = 0;
        
        DEBUGMSG(ZONE_INIT, (TEXT("FEC: NdisZeroMemory success. \r\n")));
        
        NdisMSetAttributes(
            MiniportAdapterHandle,
            (NDIS_HANDLE)pEthernet,
            TRUE,                            // DMA involved
            NdisInterfaceInternal);            // host-specific internal interface
            
        DEBUGMSG(ZONE_INIT, (TEXT("FEC: NdisMSetAttributes completed \r\n")));
        
        // Step 4: Open the configuration space and get MAC address
        NdisOpenConfiguration(&Status, &ConfigHandle, WrapperConfigurationContext);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(ZONE_INIT, (TEXT("FEC: NdisOpenconfiguration failed 0x%x\n"), Status));
            break;
        }
        
        DEBUGMSG(ZONE_INIT, (TEXT("FEC: NdisOpenconfiguration success. \r\n")));

        // read in the MAC address in registry

        if(GetMacAdd(pEthernet->FecMacAddress))
            DEBUGMSG(ZONE_INIT,
                     (TEXT("FECInitialize: GetMacAddress from UUID:%x %x %x \r\n"),pEthernet->FecMacAddress[0],pEthernet->FecMacAddress[1],pEthernet->FecMacAddress[2]));  
        else
        {
            
            for(i = 0; i < 3; i++)
            {
                NdisReadConfiguration(&MACStatus, &Value, ConfigHandle, 
                                            &MACAddrString[i], NdisParameterHexInteger);
                                            
                if(MACStatus == NDIS_STATUS_SUCCESS)
                {
                    TmpVal=(USHORT)Value->ParameterData.IntegerData;
                    pEthernet->FecMacAddress[i*2] = (UCHAR)((TmpVal & 0xff00) >> 8);
                    pEthernet->FecMacAddress[(i*2)+1] = (UCHAR)(TmpVal & 0x00ff);
                }
                else
                {
                    UINT32 MacLength = 0;
                    
                    // assigned all to 0, use MAC settings hardcoded
                    for (MacLength = 0; MacLength < ETHER_ADDR_SIZE; MacLength++)
                    {
                        pEthernet->FecMacAddress[MacLength] = 0;
                    }
                    
                    break;
                }
            }
        }

        // Step 5: Map the hardware registers into virtual address
        MemPhysAddr = BSPFECBaseAddress();
        
        Status = NdisMRegisterIoPortRange( (PVOID *)&gpFECReg,
                                       MiniportAdapterHandle,
                                       MemPhysAddr,
                                       sizeof(CSP_FEC_REGS) );

                                    
        if((Status != NDIS_STATUS_SUCCESS) || !(gpFECReg))
        {
             DEBUGMSG(ZONE_ERROR,
                 (TEXT("FECInitialize: Failed to alloc memory for mapping hardware registers\r\n")));
             break;
        }
        
        // Step 6: register for a shutdown function
        NdisMRegisterAdapterShutdownHandler(MiniportAdapterHandle, pEthernet, FECShutdown);
        pEthernet->CurrentState = NdisHardwareStatusInitializing;
        
        // Step 7: do the FEC hardware initialization
        result = FECEnetInit(pEthernet);
        
        if(result == TRUE)
        {
            pEthernet->CurrentState = NdisHardwareStatusReady;
        
            pEthernet->intLine = BSPFECGetIrq();
            
            // Step 8: if the hardware initialization is successful,
            // register the interrupt
            Status = NdisMRegisterInterrupt(
                        (PNDIS_MINIPORT_INTERRUPT)&(pEthernet->interruptObj),
                        MiniportAdapterHandle,
                        pEthernet->intLine,
                        pEthernet->intLine,
                        TRUE,
                        FALSE,
                        NdisInterruptLatched);
                        
            if (Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGMSG(ZONE_INIT, 
                    (TEXT("FECInitialize: NdisMRegisterInterrupt failed 0x%x\r\n"), Status));
                    
                NdisWriteErrorLogEntry(
                    MiniportAdapterHandle,
                    NDIS_ERROR_CODE_INTERRUPT_CONNECT,
                    0);
                    
                break;
            }
            else
            {
                // enable interrupt
                FECEnableInterrupt((NDIS_HANDLE)pEthernet);
            }
        }
        else
        {
            Status = NDIS_STATUS_FAILURE;
        }
        
        // Step 9: now the hardware is initialized and interrupt is registered,
        // issue the auto-negotiation process
        FECEnetAutoNego(pEthernet);
    } while( cheat );
    
    if (ConfigHandle)
    {
        NdisCloseConfiguration(ConfigHandle);
    }
    
    if (Status == NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(ZONE_INIT, (TEXT("FECInitialize: Initialize succeeded\n")));
    }
    else
    {
        if(gpFECReg != NULL)
        {
            if(pEthernet!=NULL)
            {
                NdisMDeregisterIoPortRange( pEthernet->ndisAdapterHandle,
                                    (ULONG)BSPFECBaseAddress(),
                                    sizeof(CSP_FEC_REGS),
                                    (PVOID)gpFECReg);
            }
            gpFECReg = NULL;
        }
        
        if (pEthernet)
            NdisFreeMemory(pEthernet, sizeof(FEC_t), 0);
    }
    
    return Status;

}

//------------------------------------------------------------------------------
//
// Function: FECHalt
//
// FECHalt is a required function that de-allocates resources when the FEC 
// adapter is removed and halts the network adapter.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to a FEC allocated context area in 
//                 which the FEC driver maintains FEC adapter state,
//                 set up by FECInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
#pragma NDIS_PAGABLE_FUNCTION(FECHalt)
void FECHalt(IN NDIS_HANDLE MiniportAdapterContext)
{

    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECHalt\r\n")));

    pEthernet->CurrentState = NdisHardwareStatusClosing;

    NdisMDeregisterAdapterShutdownHandler(pEthernet->ndisAdapterHandle);

    NdisMDeregisterInterrupt(&(pEthernet->interruptObj));    

    pEthernet->CurrentState = NdisHardwareStatusNotReady;

    if(gpFECReg != NULL)
    {
        NdisMDeregisterIoPortRange( pEthernet->ndisAdapterHandle,
                                    (ULONG)BSPFECBaseAddress(),
                                    sizeof(CSP_FEC_REGS),
                                    (PVOID)gpFECReg);
        gpFECReg = NULL;
    }
    
    FECEnetDeinit(pEthernet);

    NdisFreeMemory( (PVOID)pEthernet, (UINT)sizeof(FEC_t), 0);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECHalt\r\n")));
    return;
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
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
//        Oid
//            [in] Specifies the system-defined OID_XXX code designating the golbal
//                 query operation this function should carry out
//
//        InformationBuffer
//            [in] Points to a buffer in which FECQueryInformation should return
//                 the OID-specific information
//
//        InformationBufferLength
//            [in] Specifies the number of bytes at InformationBuffer
//
//        BytesWritten
//            [out] Points to a variable that FECQueryInformation sets to the number
//                  of bytes it is returning at InformationBuffer
//
//        BytesNeeded
//            [out] Points to a variable that FECQueryInformation sets to the number
//                  of additional bytes it needs to satisfy the request if 
//                  InformationBufferLength is less than Oid requires
//
// Return Values:
//        returns NDIS_STATUS_SUCCESS if FECQueryInformation returned the requested
//        information at InformationBuffer and set the variable at BytesWritten to
//        the amount of information it returned
//
//        returns NDIS_STATUS_INVALID_OID if the requested Oid is not supported
//
//        returns NDIS_STATUS_INVALID_LENGTH if The InformationBufferLength does
//        not match the length required by the given Oid
// 
//------------------------------------------------------------------------------
NDIS_STATUS FECQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    )
{
    NDIS_STATUS result = NDIS_STATUS_SUCCESS;
    UINT BytesLeft = InformationBufferLength;
    PUCHAR InfoBuffer = (PUCHAR)(InformationBuffer);
    
    // These variables hold the results of query
    ULONG GenericULong;
    USHORT GenericUShort;
    UCHAR GenericArray[6];
    UINT MoveBytes = sizeof(ULONG);
    PVOID MoveSource = (PVOID)(&GenericULong);
    NDIS_PNP_CAPABILITIES       Power_Management_Capabilities;
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    USHORT                      ulInfo = 0;  
    ULONG                       ulInfoLen = sizeof(ulInfo);
    PVOID                       pInfo = (PVOID) &ulInfo;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECQueryInformation\r\n")));
    
    // Make sure that int is 4 bytes.  Else GenericULong must change
    // to something of size 4
    ASSERT(sizeof(ULONG) == 4);

    // Switch on request type
    switch(Oid)
    {
        case OID_GEN_VENDOR_DRIVER_VERSION:
            GenericULong = (FEC_NDIS_MAJOR_VERSION << 16) + FEC_NDIS_MINOR_VERSION;
            break;
            
        case OID_GEN_SUPPORTED_LIST:
            MoveSource = (PVOID)(FECSupportedOids);
            MoveBytes = sizeof(FECSupportedOids);
            break;
            
        case OID_GEN_HARDWARE_STATUS:
            GenericULong = pEthernet->CurrentState;
            break;
            
        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
            GenericULong = NdisMedium802_3;
            break;
        
        case OID_GEN_MEDIA_CONNECT_STATUS:
            GenericULong = pEthernet->MediaState;
            break;
            
        case OID_GEN_MAXIMUM_LOOKAHEAD:
            GenericULong = (PKT_MAXBUF_SIZE - ETHER_HDR_SIZE - PKT_CRC_SIZE);
            break;
            
        case OID_GEN_MAXIMUM_FRAME_SIZE:
            GenericULong = (PKT_MAXBUF_SIZE - ETHER_HDR_SIZE - PKT_CRC_SIZE);
            break;
            
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
            GenericULong = (PKT_MAXBUF_SIZE - PKT_CRC_SIZE);
            break;
            
        case OID_GEN_MAC_OPTIONS:
            GenericULong = (ULONG)(
                NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                NDIS_MAC_OPTION_RECEIVE_SERIALIZED |
                NDIS_MAC_OPTION_NO_LOOPBACK
                );
                
            break;
            
        case OID_GEN_LINK_SPEED:
            if(pEthernet->SpeedMode == TRUE)
                GenericULong = 1000000;    // 100 Mbps in 100 bps units
            else
                GenericULong = 100000;     // 10 Mbps in 100 bps units
            break;
            
        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            GenericULong = MAXIMUM_TRANSMIT_PACKET * PKT_MAXBUF_SIZE;
            break;
            
        case OID_GEN_RECEIVE_BUFFER_SPACE:
            GenericULong = PKT_MAXBUF_SIZE;
            break;
            
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
            GenericULong = PKT_MAXBUF_SIZE;
            break;
            
        case OID_GEN_RECEIVE_BLOCK_SIZE:
            GenericULong = PKT_MAXBUF_SIZE;
            break;
            
        case OID_GEN_VENDOR_DESCRIPTION:
            MoveSource = (PVOID)"Freescale Fast Ethernet Adapter";
            MoveBytes = 32;
            break;
            
        case OID_GEN_VENDOR_ID:
            NdisMoveMemory((PVOID)&GenericULong,
                            &(pEthernet->FecMacAddress),
                            (ULONG)ETHER_ADDR_SIZE);
            GenericULong &= 0xFFFFFF00;
            GenericULong |= 0x01;
            MoveSource = (PVOID)(&GenericULong);
            MoveBytes = sizeof(GenericULong);
            break;
            
        case OID_GEN_DRIVER_VERSION:
            GenericUShort = ((USHORT)FEC_NDIS_MAJOR_VERSION << 8) |
                                      FEC_NDIS_MINOR_VERSION;
            MoveSource = (PVOID)(&GenericUShort);
            MoveBytes = sizeof(GenericUShort);
            break;
            
        case OID_GEN_CURRENT_PACKET_FILTER:
            GenericULong = (ULONG)(pEthernet->PacketFilter);
            break;
            
        case OID_GEN_CURRENT_LOOKAHEAD:
            GenericULong = (ULONG)(pEthernet->CurrentLookAhead);
            break;
            
        case OID_GEN_XMIT_OK:
            GenericULong = (UINT)(pEthernet->TxdStatus.FramesXmitGood);
            break;
            
        case OID_GEN_RCV_OK:
            GenericULong = (UINT)(pEthernet->RcvStatus.FrameRcvGood);
            break;
            
        case OID_GEN_XMIT_ERROR:
            GenericULong = (UINT)(pEthernet->TxdStatus.FramesXmitBad);
            break;
            
        case OID_GEN_RCV_ERROR:
            GenericULong = (UINT)(pEthernet->RcvStatus.FrameRcvErrors);
            break;
            
        case OID_GEN_RCV_NO_BUFFER:
            GenericULong = 0;
            break;

        case OID_GEN_MAXIMUM_SEND_PACKETS:
            GenericULong = (UINT)MAXIMUM_TRANSMIT_PACKET;
            break;
            
        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
            NdisMoveMemory((PCHAR)GenericArray,
                            pEthernet->FecMacAddress,
                            (ULONG)ETHER_ADDR_SIZE);
                            
            MoveSource = (PVOID)(GenericArray);
            MoveBytes = sizeof(pEthernet->FecMacAddress);

            break;
            
        case OID_802_3_MULTICAST_LIST:
            MoveSource = (PVOID)(pEthernet->McastList);
            MoveBytes = pEthernet->NumMulticastAddressesInUse * ETHER_ADDR_SIZE;
            break;            
            
        case OID_802_3_MAXIMUM_LIST_SIZE:
            GenericULong = MCAST_LIST_SIZE;
            break;
            
        case OID_802_3_RCV_ERROR_ALIGNMENT:
            GenericULong = (UINT)(pEthernet->RcvStatus.FrameRcvAllignmentErrors);
            break;
            
        case OID_802_3_XMIT_ONE_COLLISION:
        case OID_802_3_XMIT_MORE_COLLISIONS:
        case OID_802_3_XMIT_MAX_COLLISIONS:
            GenericULong = (UINT)(pEthernet->TxdStatus.FramesXmitCollisionErrors);
            break;
            
        case OID_802_3_RCV_OVERRUN:
            GenericULong = (UINT)(pEthernet->RcvStatus.FrameRcvOverrunErrors);
            break;
            
        case OID_802_3_XMIT_UNDERRUN:
            GenericULong = (UINT)(pEthernet->TxdStatus.FramesXmitUnderrunErrors);
            break;
            
        case OID_PNP_CAPABILITIES:

            FECFillPoMgmtCaps (MiniportAdapterContext, 
                                &Power_Management_Capabilities, 
                                &Status,
                                &ulInfoLen);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                pInfo = (PVOID) &Power_Management_Capabilities;
            }
            else
            {
                pInfo = NULL;
            }

            break;

        case OID_PNP_QUERY_POWER:
            // Status is pre-set in this routine to Success

            Status = NDIS_STATUS_SUCCESS; 

            break;
            
            
        default:
            result = NDIS_STATUS_INVALID_OID;
            break;
    }
    
    if (result == NDIS_STATUS_SUCCESS)
    {
        if (MoveBytes > BytesLeft)
        {
            // Not enough room in InformationBuffer
            *BytesNeeded = MoveBytes;
            result = NDIS_STATUS_INVALID_LENGTH;
        }
        else
        {
            // Store result
            NdisMoveMemory(InfoBuffer, MoveSource, MoveBytes);
            *BytesWritten = MoveBytes;
        }
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECQueryInformation\r\n")));
    
    return result;

}

//------------------------------------------------------------------------------
//
// Function: FECReset
//
// This function is a required funtion that issues a hardware reset to the
// network adapter and/or resets the driver's software state. If FECCheckForHang
// returns TRUE, this function will be called to reset the FEC adapter.
//
// Parameters:
//        AddressingReset
//            [out] Points to a variable that FECReset sets to TRUE if the NDIS
//                  library call FECSetInformation to restore addressing 
//                  information to the current values.
//
//        MiniportAdapterContext
//            [in]  Specifies the handle to the driver allocated context area in
//                  which the driver maintains FEC adapter state, set up by
//                  FECInitialize.
//
// Return Value:
//        Return NDIS_STATUS_SUCCESS always.
//
//------------------------------------------------------------------------------
NDIS_STATUS FECReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PNDIS_PACKET pNdisPacket;
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECReset\r\n")));
    
    pEthernet->CurrentState = NdisHardwareStatusReset;
    
    // Disable interrupts
    FECDisableInterrupt(MiniportAdapterContext);
    
    pEthernet->TransmitInProgress = FALSE;
    pEthernet->StartTx = FALSE;
    
    //
    // Remove the packet from the queue.
    //
    EnterCriticalSection (&gFECBufCs);
    pNdisPacket = pEthernet->HeadPacket;
    while(pNdisPacket != NULL)
    {
       pEthernet->HeadPacket = RESERVED(pNdisPacket)->Next;
       if (pNdisPacket == pEthernet->TailPacket) 
       {
            pEthernet->TailPacket = NULL;
       }
       NdisMSendComplete(pEthernet->ndisAdapterHandle, pNdisPacket, NDIS_STATUS_SUCCESS);
       pNdisPacket = pEthernet->HeadPacket;
    }
    LeaveCriticalSection (&gFECBufCs);
    
    // issues the hardware reset for FEC adapter
    FECEnetReset(MiniportAdapterContext, FALSE);
    
    *AddressingReset = TRUE;
    pEthernet->CurrentState = NdisHardwareStatusReady;
    
    // enable interrupts
    FECEnableInterrupt(MiniportAdapterContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECReset\r\n")));
    
    return NDIS_STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: FECSend
//
// This function is responsible for sending the packet to the FEC and start the 
// sending process.
// 
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
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
NDIS_STATUS FECSend(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT SendFlags
    )
{
    UINT PacketLength;
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));

    UNREFERENCED_PARAMETER(SendFlags);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECSend\r\n")));
    
    NdisQueryPacket( Packet,NULL,NULL,NULL,&PacketLength);
    
    if( (PacketLength > PKT_MAXBUF_SIZE) ||(PacketLength < ETHER_HDR_SIZE))
    {
        pEthernet->TxdStatus.FramesXmitBad++;
        return NDIS_STATUS_FAILURE;
    }
    
    // Put the packet on the send queue
    EnterCriticalSection(&gFECBufCs);
    
    if(pEthernet->HeadPacket == NULL)
    {
        pEthernet->HeadPacket = Packet;
    }
    else
    {
        RESERVED(pEthernet->TailPacket)->Next = Packet;
    }
    
    RESERVED(Packet)->Next = NULL;
    pEthernet->TailPacket = Packet;
    
    LeaveCriticalSection (&gFECBufCs);
    
    if(pEthernet->TransmitInProgress == FALSE)
    {
        pEthernet->TransmitInProgress = TRUE;
        FECStartXmit(pEthernet);
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECSend\r\n")));
    
    return NDIS_STATUS_PENDING;

}

//------------------------------------------------------------------------------
//
// Function: FECTransferData
// 
// This function copies the content of the received packet to a given 
// protocol-allocated packet.
// 
// Parameters:
//        Packet
//            [out] Point to a packet descriptor with chained buffers into which
//                  FECTransferData should copy the received data
//
//        BytesTransferred
//            [out] Points to a variable that FECTransferData sets to the number
//                  of bytes it  copied into the packet
//
//        MiniportAdapterContext
//            [in]  Specifies the handle to a miniport-allocated context area in 
//                  which the driver maintains FEC adapter state, set up by
//                  FECInitialize
//
//        MiniportReceiveContext
//            [in]  Specifies the context handle previously passed to 
//                  NdisMEthIndicateReceive. The miniport can examine this value
//                  to determine which recieve to copy
//
//        ByteOffset
//            [in]  Specifies the offset within the received packet at which
//                  FECTransferData should begin the copy. If the entire packet
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
NDIS_STATUS
FECTransferData(
    OUT PNDIS_PACKET  Packet,
    OUT PUINT  BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT  ByteOffset,
    IN UINT  BytesToTransfer
    )
{
    PNDIS_BUFFER pCurrentBuffer;
    UINT   BufferLength;
    PUCHAR pBufferSource;
    PUCHAR pBufferStart;
    UINT   BytesWanted, BytesCopied, BytesLeft;
    ULONG  PacketSize;
    
    // The adapter to transfer from
    pFEC_t pEthernet = ((pFEC_t)(MiniportReceiveContext));

    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECTransferData\r\n")));
    
    // Add the packet header onto the offset
    ByteOffset += ETHER_HDR_SIZE;
    pBufferSource = pEthernet->ReceiveBuffer + ByteOffset;
    PacketSize = pEthernet->ReceivePacketSize;
    
    if((BytesToTransfer == 0) || (ByteOffset >= PacketSize))
    {
        *BytesTransferred = 0;
        return NDIS_STATUS_FAILURE;
    }
    
    NdisQueryPacket( Packet, NULL, NULL, &pCurrentBuffer, NULL );
    NdisQueryBuffer( pCurrentBuffer, &pBufferStart, &BufferLength );
    
    *BytesTransferred = 0;
    BytesWanted = 0;
    BytesCopied = 0;
    BytesLeft = BytesToTransfer;
    
    do {
    
        if( !BufferLength )
        {
            NdisGetNextBuffer( pCurrentBuffer, &pCurrentBuffer );
            NdisQueryBuffer( pCurrentBuffer, &pBufferStart, &BufferLength );
        }
        
        if( BufferLength > BytesLeft )
        {
            BytesCopied += BytesLeft;
            BytesWanted = BytesLeft;
        }
        else
        {
            BytesCopied += BufferLength;
            BytesWanted = BufferLength;
            BufferLength = 0;
        }
        
        NdisMoveMemory( pBufferStart, pBufferSource, BytesWanted );
        pBufferStart += BytesWanted;
        pBufferSource += BytesWanted;
        BytesLeft -= BytesWanted;
        
    } while(BytesCopied < BytesToTransfer);
    
    *BytesTransferred = BytesCopied;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECTransferData\r\n")));
    return NDIS_STATUS_SUCCESS;
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
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    )
{
    UINT BytesLeft = InformationBufferLength;
    PUCHAR InfoBuffer = (PUCHAR)(InformationBuffer);
    
    // Variables for a particular request
    UINT OidLength;
    
    // Variables for holding the new values to be used
    ULONG LookAhead;
    ULONG Filter;
    NDIS_DEVICE_POWER_STATE     NewPowerState;

    // Status of the operation
    NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;
 
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECSetInformation\r\n")));
    
    // Get Oid and Length of request
    OidLength = BytesLeft;
    
    switch (Oid)
    {
        case OID_802_3_MULTICAST_LIST:
            // Verify length
            if ((OidLength % ETHER_ADDR_SIZE) != 0)
            {
                StatusToReturn = NDIS_STATUS_INVALID_DATA;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }
            else
            {
                // Ensure that the multi cast list is not full yet
                if(OidLength <= (MCAST_LIST_SIZE * ETHER_ADDR_SIZE))
                {
                    // Get the multicast address and update hash table
                    NdisMoveMemory(pEthernet->McastList, InfoBuffer, OidLength);
                    *BytesRead = OidLength;
                    pEthernet->NumMulticastAddressesInUse = OidLength / ETHER_ADDR_SIZE;
                    
                    // clear all the content in hardware Hash Table
                    ClearAllMultiCast();
                    
                    // Add the hash table value according to the multicast address
                    for(LookAhead=0; LookAhead < pEthernet->NumMulticastAddressesInUse; LookAhead++)
                    {
                        AddMultiCast(&(pEthernet->McastList[LookAhead][0]));
                    }
                }
                else
                {
                    *BytesRead = 0;
                    StatusToReturn = NDIS_STATUS_MULTICAST_FULL;
                }
            }
            break;
            
        case OID_GEN_CURRENT_PACKET_FILTER:
            // check for length
            if( OidLength != 4 )
            {
                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }
            
            NdisMoveMemory((PUCHAR)&Filter, (PUCHAR)InfoBuffer, 4);
            
            if (Filter & (NDIS_PACKET_TYPE_SOURCE_ROUTING |
                          NDIS_PACKET_TYPE_SMT |
                          NDIS_PACKET_TYPE_MAC_FRAME |
                          NDIS_PACKET_TYPE_FUNCTIONAL |
                          NDIS_PACKET_TYPE_ALL_FUNCTIONAL |
                          NDIS_PACKET_TYPE_GROUP |
                          NDIS_PACKET_TYPE_ALL_MULTICAST
                          ))
            {
                StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;
                *BytesRead = 4;
                *BytesNeeded = 0;
            }
            else
            {
                pEthernet->PacketFilter = Filter;
                
                if(Filter & NDIS_PACKET_TYPE_MULTICAST )
                {
                    // clear all the content in hardware Hash Table
                    ClearAllMultiCast();
                    
                    // Add the hash table value according to the multicast address
                    for(LookAhead=0; LookAhead < pEthernet->NumMulticastAddressesInUse; LookAhead++)
                    {
                        AddMultiCast(&(pEthernet->McastList[LookAhead][0]));
                    }
                }
                
                // TODO: how to set the FEC hardware filter? NO IDEA 
                
                StatusToReturn = NDIS_STATUS_SUCCESS;

            }
            break;
            
        case OID_GEN_CURRENT_LOOKAHEAD:
            // check for length
            if( OidLength != 4 )
            {
                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }
            
            // Stores the new lookahead value
            NdisMoveMemory(&LookAhead, InfoBuffer, 4);
            
            if( LookAhead <= (PKT_MAXBUF_SIZE - ETHER_HDR_SIZE - PKT_CRC_SIZE) )
            {
                pEthernet->CurrentLookAhead= LookAhead;
            }
            else
            {
                StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
            }
            break;

        case OID_PNP_SET_POWER:

          
            

            if (InformationBufferLength != sizeof(NDIS_DEVICE_POWER_STATE ))
            {
                return(NDIS_STATUS_INVALID_LENGTH);
            }

            NewPowerState = *(PNDIS_DEVICE_POWER_STATE    )InformationBuffer;

            //
            // Set the power state - Cannot fail this request
            //
            FECSetPower(MiniportAdapterContext ,NewPowerState );
        
            *BytesRead = sizeof(NDIS_DEVICE_POWER_STATE    );
            StatusToReturn = NDIS_STATUS_SUCCESS; 
            break;      
            
        case OID_GEN_PROTOCOL_OPTIONS:
            break;
            
        default:
            *BytesRead = 0;
            *BytesNeeded = 0;
            StatusToReturn = NDIS_STATUS_INVALID_OID;
            break;
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECSetInformation\r\n")));
    
    return (StatusToReturn);
}

//------------------------------------------------------------------------------
//
// Function: FECShutdown
//
// Restore the FEC adapter to its initial state when the system is shut down, 
// whether by the user or because an unrecoverable system error occurred.
//
// Parameters:
//        ShutdownContext
//            [in] Points to a context area supplied when the FECInitialize
//                 function calls NdisMRegisterAdapterShutdownHandler.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void FECShutdown(
    IN PVOID ShutdownContext
    )
{
    pFEC_t pEthernet = ((pFEC_t)(ShutdownContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECShutdown\r\n")));
    
    pEthernet->CurrentState = NdisHardwareStatusNotReady;
    
    DeleteCriticalSection (&gFECRegCs);
    DeleteCriticalSection (&gFECBufCs);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECShutdown\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: FECCheckForHang
//
// This function reports the link status of the FEC network adapter. Bu default,
// the NDIS library calls this function approximately every two seconds.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to a FEC driver allocated context area
//                 in which the FEC driver maintains FEC adapter state, set up
//                 by FECInitialize function.
//
// Return Values:
//        return TRUE if this function detects that the network adapter is not
//        operating correctly, otherwise FALSE.
//
//------------------------------------------------------------------------------
BOOLEAN FECCheckForHang(IN NDIS_HANDLE MiniportAdapterContext)
{
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECCheckForHang\r\n")));
    
    if(pEthernet->MediaStateChecking == TRUE) 
    {
        // Media state is checking by the MII interrupt handler now, 
        // just return FALSE
        return FALSE;
    }
    else
    {
        // send the cmd to the external PHY to get the link status
        pEthernet->MediaStateChecking = TRUE;
        
        FECGetLinkStatus(MiniportAdapterContext);    
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECCheckForHang\r\n")));
    return FALSE;
}
