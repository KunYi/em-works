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
//  This file implements  the main  enet  interface  class.
//
//-----------------------------------------------------------------------------

#include "enet.h"

extern "C"  DWORD BSPENETGetIrq(DWORD index);
extern "C"  DWORD BSPENETBaseAddress(DWORD index);
extern "C"  BOOL  BSPFECMacAddress(unsigned char FecMacAddress[6]);		// CS&ZHL NOV-5-2011: use MAC address set in Eboot

extern "C"  BOOL BSPENETIomuxConfig(DWORD index, IN BOOL Enable );
extern "C"  BOOL BSPENETClockConfig( DWORD index,IN BOOL Enable );
extern "C"  void BSPENETPowerEnable(DWORD index,BOOL bEnable);

extern PVOID pv_HWregENET0;
extern PVOID pv_HWregENET1;
HANDLE hMiiEvent;
HANDLE hMiiHandleThread;
HANDLE hMiiMutex;
extern ENETClass* gpENET;



//-----------------------------------------------------------------------------
// Local Variables
//-----------------------------------------------------------------------------
// This constant is used for places where NdisAllocateMemory
// needs to be called and the HighestAcceptableAddress does
// not matter.
static NDIS_PHYSICAL_ADDRESS HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);

// List of supported OIDs for the ENET driver
static NDIS_OID ENETSupportedOids[] = {
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
    OID_TCP_TASK_OFFLOAD,
    OID_GET_XMIT_TIMER,
    OID_GET_RCV_TIMER,
    OID_ENA_PTP_RXTX,
    /* powermanagement */
    OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,
};


__inline 
BOOLEAN  
ENETIsPoMgmtSupported(
   IN NDIS_HANDLE MiniportAdapterContext
   )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    return TRUE;
   
}

//------------------------------------------------------------------------------
//
// Function: ENETFillPoMgmtCaps
//
// ENETFillPoMgmtCaps is a required function that set power mg caps 
//
// Parameters:
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
VOID
ENETFillPoMgmtCaps (
    IN NDIS_HANDLE MiniportAdapterContext, 
    IN OUT PNDIS_PNP_CAPABILITIES  pPower_Management_Capabilities, 
    IN OUT PNDIS_STATUS            pStatus,
    IN OUT PULONG                  pulInfoLen
    )

{

    BOOLEAN bIsPoMgmtSupported;  
    bIsPoMgmtSupported = ENETIsPoMgmtSupported(MiniportAdapterContext);

    if (bIsPoMgmtSupported == TRUE)
    {
        pPower_Management_Capabilities->Flags = NDIS_DEVICE_WAKE_UP_ENABLE ;
        pPower_Management_Capabilities->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPower_Management_Capabilities->WakeUpCapabilities.MinPatternWakeUp     = NdisDeviceStateD3;
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
//        ENETMacAddress
//            [out] Specifies the handle to a ENET MAC Address 
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
BOOL GetMacAdd(unsigned char EnetMacAddress[6])
{
    GUID   uuid;
    DWORD  dwSize = sizeof(uuid);
    UINT32 spiValue = SPI_GETUUID;

	//
	// CS&ZHL NOV-5-2011: try to get Eboot MAC address first
	//
	if(FALSE == BSPFECMacAddress(EnetMacAddress))
	{
		if(FALSE==KernelIoControl(IOCTL_HAL_GET_DEVICE_INFO, &spiValue,sizeof(spiValue), &uuid,dwSize, &dwSize))
			return FALSE;
	   
		if((uuid.Data1==0)&&(uuid.Data2==0)&&(uuid.Data3==0)&&(uuid.Data4[0]==0)&&(uuid.Data4[1]==0)&&(uuid.Data4[2]==0))
			return FALSE;
		else if ((uuid.Data1==0xffffffff)&&(uuid.Data2==0xffff)&&(uuid.Data3==0xffff)&&(uuid.Data4[0]==0xff)&&(uuid.Data4[1]==0xff)&&(uuid.Data4[2]==0xff))
			return FALSE;
		else
		{
			EnetMacAddress[0]=(unsigned char)uuid.Data1;
			EnetMacAddress[0]&=0xf0;
			EnetMacAddress[1]=(unsigned char)uuid.Data2;
			EnetMacAddress[2]=(unsigned char)uuid.Data3;
			EnetMacAddress[3]=uuid.Data4[0];
			EnetMacAddress[4]=uuid.Data4[1];
			EnetMacAddress[5]=uuid.Data4[2];      
		}
	}
        
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: ENETClass::ENETClass
//
// This ENETClass constructor creates all the mutexes, events and heaps required
// Parameters:
//  regindex
//   the enet controller index
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
ENETClass::ENETClass(UINT32 regindex)
{
   //Only need transfer Enent Class index
   index=regindex;
   
}


//-----------------------------------------------------------------------------
//
// Function: ENETClass::~ENETClass
//
// This ENETClass destructor releases all the mutexes, events and heaps created.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
ENETClass::~ENETClass()    
{

}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETSetPower
//
// ENETSetPower is a required function that set ENET power state
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize

//        PowerState
//            [in] Specifies the  ENET power state

//
// Return Value:
//        TURE indicate sucess,F ALSE indicate fail.
//
//------------------------------------------------------------------------------
VOID
ENETClass::ENETSetPower(
     NDIS_HANDLE MiniportAdapterContext ,
     NDIS_DEVICE_POWER_STATE   PowerState 
    )

{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    if (PowerState == NdisDeviceStateD0)
    {     

        ENETEnetPowerHandle(pEthernet,TRUE);     

    }
    else
    {

        ENETEnetPowerHandle(pEthernet,FALSE);
     
    }
           
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETInitialize
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
#pragma NDIS_PAGEABLE_FUNCTION(ENETInitialize)
NDIS_STATUS ENETClass::ENETInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext)
{
    // Pointer to our newly allocated adapter
    pENET_t pEthernet = NULL;
    
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
    
    DEBUGMSG(ZONE_INIT, (TEXT("ENET: +ENETInitialize\r\n")));
    InitializeCriticalSection (&gENETRegCs);
    InitializeCriticalSection (&gENETBufCs);
    InitializeCriticalSection (&gENETTimerCs);
    
    
    do {
        // Step 1: Search for medium type (802.3) in the given array
        for (i = 0;i < MediumArraySize; i++) {
            if (MediumArray[i] == NdisMedium802_3) {
                *SelectedMediumIndex = i;
                break;
            }
        }
        
        if (i == MediumArraySize) {
            DEBUGMSG(ZONE_ERROR, (TEXT("ENET: No supported media\r\n")));
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }
        
        *SelectedMediumIndex = i;
        DEBUGMSG(ZONE_INIT, (TEXT("ENET: Search for media success. \r\n")));
        
        // Step 2: Allocate memory for the adapter block now
        Status = NdisAllocateMemory( (PVOID *)&pEthernet, sizeof(ENET_t), 0, HighestAcceptableMax);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("ENET: NdisAllocateMemory(ENET_t) failed\n")));
            break;
        }
        
        DEBUGMSG(ZONE_INIT, (TEXT("ENET: Allocate memory for the adapter block success. \r\n")));
        
        
        // Step 3: Clear out and initialize the pEthernet block
        NdisZeroMemory (pEthernet, sizeof(ENET_t));

        pEthernet->pENET = gpENET;
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
        
        DEBUGMSG(ZONE_INIT, (TEXT("ENET: NdisZeroMemory success. \r\n")));
        
        NdisMSetAttributes(
            MiniportAdapterHandle,
            (NDIS_HANDLE)pEthernet,
            TRUE,                            // DMA involved
            NdisInterfaceInternal);            // host-specific internal interface
            
        DEBUGMSG(ZONE_INIT, (TEXT("ENET: NdisMSetAttributes completed \r\n")));
        
        // Step 4: Open the configuration space and get MAC address
        NdisOpenConfiguration(&Status, &ConfigHandle, WrapperConfigurationContext);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(ZONE_INIT, (TEXT("ENET: NdisOpenconfiguration failed 0x%x\n"), Status));
            break;
        }
        
        DEBUGMSG(ZONE_INIT, (TEXT("ENET: NdisOpenconfiguration success. \r\n")));

        // read in the MAC address in registry

        if(GetMacAdd(pEthernet->EnetMacAddress))
            DEBUGMSG(ZONE_INIT,
                     (TEXT("ENETInitialize: GetMacAddress from UUID:%x %x %x \r\n"),pEthernet->EnetMacAddress[0],pEthernet->EnetMacAddress[1],pEthernet->EnetMacAddress[2]));  
        else
        {
            
            for(i = 0; i < 3; i++)
            {
                NdisReadConfiguration(&MACStatus, &Value, ConfigHandle, 
                                            &MACAddrString[i], NdisParameterHexInteger);
                                            
                if(MACStatus == NDIS_STATUS_SUCCESS)
                {
                    TmpVal=(USHORT)Value->ParameterData.IntegerData;
                    pEthernet->EnetMacAddress[i*2] = (UCHAR)((TmpVal & 0xff00) >> 8);
                    pEthernet->EnetMacAddress[(i*2)+1] = (UCHAR)(TmpVal & 0x00ff);
                }
                else
                {
                    UINT32 MacLength = 0;
                    
                    // assigned all to 0, use MAC settings hardcoded
                    for (MacLength = 0; MacLength < ETHER_ADDR_SIZE; MacLength++)
                    {
                        pEthernet->EnetMacAddress[MacLength] = 0;
                    }
                    
                    break;
                }
            }
        }

        pEthernet->EnetMacAddress[ETHER_ADDR_SIZE-1] =pEthernet->EnetMacAddress[ETHER_ADDR_SIZE-1] +(UCHAR)index;
        // Step 5: Map the hardware registers into virtual address
        MemPhysAddr = BSPENETBaseAddress(index);
        
        Status = NdisMRegisterIoPortRange((index==0)? (PVOID *)&pv_HWregENET0:(index==1)? (PVOID *)&pv_HWregENET1:0x0,
                                       MiniportAdapterHandle,
                                       MemPhysAddr,
                                       0x4000 );

                                    
        if((Status != NDIS_STATUS_SUCCESS))
        {
             DEBUGMSG(ZONE_ERROR,
                 (TEXT("ENETInitialize: Failed to alloc memory for mapping hardware registers\r\n")));
             break;
        }
        
        // Step 6: register for a shutdown function
      //  NdisMRegisterAdapterShutdownHandler(MiniportAdapterHandle, pEthernet, ENETShutdown);
        pEthernet->CurrentState = NdisHardwareStatusInitializing;

        
        hMiiMutex = CreateMutex(NULL, FALSE, TEXT("MiiAcessProtect"));
        if(NULL == hMiiMutex)
        {
            ERRORMSG(TRUE, (_T("Failed craate hMutex for MiiAcessProtect\r\n")));
            return TRUE;
        }

        hMiiEvent=CreateEvent(NULL, FALSE, FALSE, L"MAC1MiiEvent");

        // Check if CreateEvent failed
        if (hMiiEvent == NULL)
        {
            ERRORMSG(TRUE, (TEXT("%s(): CreateEvent failed!\r\n"),
                __WFUNCTION__));

        }

        if(index==MAC1)
        {

            hMiiHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EnetMiiHandleThread, pEthernet, 0, NULL);
            if (!hMiiHandleThread) 
            {
                ERRORMSG(TRUE, 
                    (TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
                
            }
        }
        
        // Step 7: do the ENET hardware initialization
        result = ENETEnetInit(pEthernet);
        
        if(result == TRUE)
        {
            pEthernet->CurrentState = NdisHardwareStatusReady;
        
            pEthernet->intLine = BSPENETGetIrq(index);
            
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
                    (TEXT("ENETInitialize: NdisMRegisterInterrupt failed 0x%x\r\n"), Status));
                    
                NdisWriteErrorLogEntry(
                    MiniportAdapterHandle,
                    NDIS_ERROR_CODE_INTERRUPT_CONNECT,
                    0);
                    
                break;
            }
            else
            {
                // enable interrupt
                ENETEnableInterrupt((NDIS_HANDLE)pEthernet);
            }
        }
        else
        {
            Status = NDIS_STATUS_FAILURE;
        }
        
        
        // Step 9: now the hardware is initialized and interrupt is registered,
        // issue the auto-negotiation process
        ENETEnetAutoNego(pEthernet);
    } while( cheat );
    
    if (ConfigHandle)
    {
        NdisCloseConfiguration(ConfigHandle);
    }
    
    if (Status == NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(ZONE_INIT, (TEXT("ENETInitialize: Initialize succeeded\n")));
    }
    else
    {
        if(index==0)
        {
            if((pv_HWregENET0) != NULL)
            {
                if(pEthernet!=NULL)
                {
                    NdisMDeregisterIoPortRange( pEthernet->ndisAdapterHandle,
                                        (ULONG)BSPENETBaseAddress(index),
                                         0x4000,
                                        (PVOID)pv_HWregENET0);
                }
                (pv_HWregENET0) = NULL;
            }
        }
        else
          {
            if( (pv_HWregENET1) != NULL)
            {
                if(pEthernet!=NULL)
                {
                    NdisMDeregisterIoPortRange( pEthernet->ndisAdapterHandle,
                                        (ULONG)BSPENETBaseAddress(index),
                                        0x4000,
                                        (PVOID)pv_HWregENET1);
                }
                (pv_HWregENET1) = NULL;
            }
        }   
        
        if (pEthernet)
            NdisFreeMemory(pEthernet, sizeof(ENET_t), 0);
    }
    
    return Status;

}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETHalt
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
#pragma NDIS_PAGABLE_FUNCTION(ENETHalt)
void ENETClass::ENETHalt(IN NDIS_HANDLE MiniportAdapterContext)
{

    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETHalt\r\n")));

    pEthernet->CurrentState = NdisHardwareStatusClosing;

    NdisMDeregisterAdapterShutdownHandler(pEthernet->ndisAdapterHandle);

    NdisMDeregisterInterrupt(&(pEthernet->interruptObj));    

    pEthernet->CurrentState = NdisHardwareStatusNotReady;

    if(index==0)
        {
            if((pv_HWregENET0) != NULL)
            {
                if(pEthernet!=NULL)
                {
                    NdisMDeregisterIoPortRange( pEthernet->ndisAdapterHandle,
                                        (ULONG)BSPENETBaseAddress(index),
                                        0x1000,
                                        (PVOID)pv_HWregENET0);
                }
                (pv_HWregENET0) = NULL;
            }
        }
        else
          {
            if( (pv_HWregENET1) != NULL)
            {
                if(pEthernet!=NULL)
                {
                    NdisMDeregisterIoPortRange( pEthernet->ndisAdapterHandle,
                                        (ULONG)BSPENETBaseAddress(index),
                                        0x1000,
                                        (PVOID)pv_HWregENET1);
                }
                (pv_HWregENET1) = NULL;
            }
        }   
    
    ENETEnetDeinit(pEthernet);

    NdisFreeMemory( (PVOID)pEthernet, (UINT)sizeof(ENET_t), 0);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETHalt\r\n")));
    return;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETQueryInformation
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
NDIS_STATUS ENETClass::ENETQueryInformation(
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
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETQueryInformation\r\n")));
    
    // Make sure that int is 4 bytes.  Else GenericULong must change
    // to something of size 4
    ASSERT(sizeof(ULONG) == 4);

    // Switch on request type
    switch(Oid)
    {
        case OID_GEN_VENDOR_DRIVER_VERSION:
            GenericULong = (ENET_NDIS_MAJOR_VERSION << 16) + ENET_NDIS_MINOR_VERSION;
            break;
            
        case OID_GEN_SUPPORTED_LIST:
            MoveSource = (PVOID)(ENETSupportedOids);
            MoveBytes = sizeof(ENETSupportedOids);
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
            MoveSource = (PVOID)"Freescale Ethernet Adapter";
            MoveBytes = 32;
            break;
            
        case OID_GEN_VENDOR_ID:
            NdisMoveMemory((PVOID)&GenericULong,
                            &(pEthernet->EnetMacAddress),
                            (ULONG)ETHER_ADDR_SIZE);
            GenericULong &= 0xFFFFFF00;
            GenericULong |= 0x01;
            MoveSource = (PVOID)(&GenericULong);
            MoveBytes = sizeof(GenericULong);
            break;
            
        case OID_GEN_DRIVER_VERSION:
            GenericUShort = ((USHORT)ENET_NDIS_MAJOR_VERSION << 8) |
                                      ENET_NDIS_MINOR_VERSION;
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

        case OID_GET_XMIT_TIMER:
            RETAILMSG(0, (TEXT("@@@  OID_GET_XMIT_TIMER \n") ));
            MoveSource = &TxTimer;
            MoveBytes = sizeof(TxTimer);
            break;
            
        case OID_GET_RCV_TIMER:
            RETAILMSG(0, (TEXT("OID_GET_RCV_TIMER %x %x  \n"), RxTimer.sec,RxTimer.nsecs));  
            MoveSource = &RxTimer;
            MoveBytes = sizeof(RxTimer);            
            break;

        case OID_TCP_TASK_OFFLOAD:
             RETAILMSG(0, (TEXT("@@@  OID_TCP_TASK_OFFLOAD \n") ));
             break;
        case OID_GEN_MAXIMUM_SEND_PACKETS:
            GenericULong = (UINT)MAXIMUM_TRANSMIT_PACKET;
            break;
            
        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
            NdisMoveMemory((PCHAR)GenericArray,
                            pEthernet->EnetMacAddress,
                            (ULONG)ETHER_ADDR_SIZE);
                            
            MoveSource = (PVOID)(GenericArray);
            MoveBytes = sizeof(pEthernet->EnetMacAddress);

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

            ENETFillPoMgmtCaps (MiniportAdapterContext, 
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
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETQueryInformation\r\n")));
    
    return result;

}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETReset
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
NDIS_STATUS ENETClass::ENETReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    )
{
    PNDIS_PACKET pNdisPacket;
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETReset\r\n")));
    
    pEthernet->CurrentState = NdisHardwareStatusReset;
    
    // Disable interrupts
    ENETDisableInterrupt(MiniportAdapterContext);
    
    pEthernet->TransmitInProgress = FALSE;
    pEthernet->StartTx = FALSE;
    
    //
    // Remove the packet from the queue.
    //
    EnterCriticalSection (&gENETBufCs);
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
    LeaveCriticalSection (&gENETBufCs);
    
    // issues the hardware reset for ENET adapter
    ENETEnetReset(MiniportAdapterContext, FALSE);
    
    *AddressingReset = TRUE;
    pEthernet->CurrentState = NdisHardwareStatusReady;
    
    // enable interrupts
    ENETEnableInterrupt(MiniportAdapterContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETReset\r\n")));
    
    return NDIS_STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETSend
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
NDIS_STATUS ENETClass::ENETSend(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT SendFlags
    )
{
    UINT PacketLength;
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));

    UNREFERENCED_PARAMETER(SendFlags);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETSend\r\n")));
    
    NdisQueryPacket( Packet,NULL,NULL,NULL,&PacketLength);
    
    if( (PacketLength > PKT_MAXBUF_SIZE) ||(PacketLength < ETHER_HDR_SIZE))
    {
        pEthernet->TxdStatus.FramesXmitBad++;
        return NDIS_STATUS_FAILURE;
    }
    
    // Put the packet on the send queue
    EnterCriticalSection(&gENETBufCs);
    
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
    
    LeaveCriticalSection (&gENETBufCs);
    
    if(pEthernet->TransmitInProgress == FALSE)
    {
        pEthernet->TransmitInProgress = TRUE;
        ENETStartXmit(pEthernet);
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETSend\r\n")));
    
    return NDIS_STATUS_PENDING;

}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETTransferData
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
NDIS_STATUS
ENETClass::ENETTransferData(
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
    pENET_t pEthernet = ((pENET_t)(MiniportReceiveContext));

    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETTransferData\r\n")));
    
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
    NdisQueryBuffer( pCurrentBuffer, (PVOID *)&pBufferStart, (PUINT)&BufferLength );
    
    *BytesTransferred = 0;
    BytesWanted = 0;
    BytesCopied = 0;
    BytesLeft = BytesToTransfer;
    
    do {
    
        if( !BufferLength )
        {
            NdisGetNextBuffer( pCurrentBuffer, &pCurrentBuffer );
            NdisQueryBuffer( pCurrentBuffer, (PVOID *)&pBufferStart, (PUINT)&BufferLength );
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
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETTransferData\r\n")));
    return NDIS_STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETSetInformation
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
NDIS_STATUS ENETClass::ENETSetInformation (
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
 
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    TimeRepr txtime;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETSetInformation\r\n")));
    
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
           
                if(Filter ==(NDIS_PACKET_TYPE_MULTICAST|NDIS_PACKET_TYPE_DIRECTED|NDIS_PACKET_TYPE_BROADCAST))
                {
                    BW_ENET_MAC_RCR_PROM(index,1);

                }     
                if(Filter & NDIS_PACKET_TYPE_MULTICAST )
                {
    
                    BW_ENET_MAC_RCR_PROM(index,0);
                    // clear all the content in hardware Hash Table
                    ClearAllMultiCast();
                    
                    // Add the hash table value according to the multicast address
                    for(LookAhead=0; LookAhead < pEthernet->NumMulticastAddressesInUse; LookAhead++)
                    {
                        AddMultiCast(&(pEthernet->McastList[LookAhead][0]));
                    }
                }               
                if((Filter &NDIS_PACKET_TYPE_PROMISCUOUS)==NDIS_PACKET_TYPE_PROMISCUOUS)
                {
                    BW_ENET_MAC_RCR_PROM(index,1);               
              
                }
                if((Filter &NDIS_PACKET_TYPE_DIRECTED)==NDIS_PACKET_TYPE_DIRECTED)
                {
                    BW_ENET_MAC_RCR_PROM(index,1);
                }
                   
                if((Filter &NDIS_PACKET_TYPE_BROADCAST)==NDIS_PACKET_TYPE_BROADCAST)
                {
                    BW_ENET_MAC_RCR_PROM(index,1);
   
                }

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
            ENETSetPower(MiniportAdapterContext ,NewPowerState );
        
            *BytesRead = sizeof(NDIS_DEVICE_POWER_STATE    );
            StatusToReturn = NDIS_STATUS_SUCCESS; 
            
            break;  
        case OID_UPDATE_NEW_TIMER:
          
            txtime= *(TimeRepr *)InformationBuffer;
            RETAILMSG(0, (TEXT("@@@  OID_UPDATE_NEW_TIMER :%d %x\n"),txtime.sec,txtime.nsecs));
            SetPTPTimer(&txtime);
            break;
            
        case OID_GEN_PROTOCOL_OPTIONS:
            break;
            
        default:
            *BytesRead = 0;
            *BytesNeeded = 0;
            StatusToReturn = NDIS_STATUS_INVALID_OID;
            break;
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETSetInformation\r\n")));
    
    return (StatusToReturn);
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETShutdown
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
void ENETClass::ENETShutdown(
    IN PVOID ShutdownContext
    )
{
    pENET_t pEthernet = ((pENET_t)(ShutdownContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETShutdown\r\n")));
    
    pEthernet->CurrentState = NdisHardwareStatusNotReady;
    
    DeleteCriticalSection (&gENETRegCs);
    DeleteCriticalSection (&gENETBufCs);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETShutdown\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETCheckForHang
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
BOOLEAN ENETClass::ENETCheckForHang(IN NDIS_HANDLE MiniportAdapterContext)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETCheckForHang\r\n")));
    
    if(pEthernet->MediaStateChecking == TRUE) 
    {
        // Media state is checking by the MII interrupt handler now, 
        // just return FALSE
        ProcessMIIInterrupts(pEthernet);
        return FALSE;
    }
    else
    {
        // send the cmd to the external PHY to get the link status
        pEthernet->MediaStateChecking = TRUE;
        
        ENETGetLinkStatus(MiniportAdapterContext);    
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETCheckForHang\r\n")));
    return FALSE;
}

