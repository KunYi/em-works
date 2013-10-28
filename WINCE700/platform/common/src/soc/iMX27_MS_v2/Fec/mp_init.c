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
/*++

Module Name:
    mp_init.c

Abstract:
    This module contains miniport initialization related routines

Revision History:

Notes:

--*/

#include "precomp.h"
#include "fec.h"

#if DBG
#define _FILENUMBER     'TINI'
#endif

FEC_MII_LIST           gMIICmds[FEC_MII_COUNT];
extern PCSP_FEC_REGS   gpFECReg; 

typedef struct _MP_REG_ENTRY
{
    NDIS_STRING RegName;                // variable name text
    BOOLEAN     bRequired;              // 1 -> required, 0 -> optional
    UINT        FieldOffset;            // offset to MP_ADAPTER field
    UINT        FieldSize;              // size (in bytes) of the field
    UINT        Default;                // default value to use
    UINT        Min;                    // minimum value allowed
    UINT        Max;                    // maximum value allowed
} MP_REG_ENTRY, *PMP_REG_ENTRY;

void FECSetMII(IN  PMP_ADAPTER             Adapter);

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern PFEC_MII_LIST  gMIIFree;
extern PFEC_MII_LIST  gMIIHead;
extern PFEC_MII_LIST  gMIITail;
extern BOOL BSPFECIomuxConfig( IN BOOL Enable );
extern BOOL BSPFECClockConfig( IN BOOL Enable );
extern void BSPGetFECMACAddressFromNAND(__out_ecount(length)UINT16 * FecMacArray,UINT length);


NDIS_STATUS MpAllocAdapterBlock(
    OUT PMP_ADAPTER     *pAdapter,
    IN  NDIS_HANDLE     MiniportAdapterHandle
    )
/*++
Routine Description:

    Allocate MP_ADAPTER data block and do some initialization

Arguments:

    Adapter     Pointer to receive pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_FAILURE

--*/    
{
    PMP_ADAPTER     Adapter;
    NDIS_STATUS     Status;

    DBGPRINT(MP_TRACE, ("--> NICAllocAdapter\n"));
    *pAdapter = NULL;

    do
    {
        // Allocate MP_ADAPTER block
        (PVOID)Adapter = MP_ALLOCMEMTAG(MiniportAdapterHandle, sizeof(MP_ADAPTER));
        if (Adapter == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            DBGPRINT(MP_ERROR, ("Failed to allocate memory - ADAPTER\n"));
            break;
        }       
        Status = NDIS_STATUS_SUCCESS;

        // Clean up the memory block
        NdisZeroMemory(Adapter, sizeof(MP_ADAPTER));

        MP_INC_REF(Adapter);
        MP_SET_FLAG(Adapter,fMP_ADAPTER_INITIALIZE_IN_PROGRESS);

        //
        // Init lists, spinlocks, etc.
        // 
        InitializeQueueHeader(&Adapter->SendWaitQueue);
        InitializeQueueHeader(&Adapter->SendCancelQueue);
 
        // PendingRequestQueue
        InitializeQueueHeader(&Adapter->PendingRequestQueue);
        
        InitializeListHead(&Adapter->RecvList);
        InitializeListHead(&Adapter->RecvPendList);
        InitializeListHead(&Adapter->PoMgmt.PatternList);

        NdisInitializeEvent(&Adapter->ExitEvent);
        NdisInitializeEvent(&Adapter->AllPacketsReturnedEvent);

        NdisAllocateSpinLock(&Adapter->Lock);
        NdisAllocateSpinLock(&Adapter->SendLock);
        NdisAllocateSpinLock(&Adapter->RcvLock);

    } while (FALSE);
    *pAdapter = Adapter;
    DBGPRINT_S(Status, ("<-- NICAllocAdapter, Status=%x\n", Status));
    return Status;
}

VOID MpFreeAdapter(                 
    IN  PMP_ADAPTER     Adapter     
    )                               
/*++
Routine Description:

    Free all the resources and MP_ADAPTER data block

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    None                                                    

--*/    
{
    PMP_TXBUF       pMpTxBuf;
    PMP_RFD         pMpRfd;

    DBGPRINT(MP_TRACE, ("--> NICFreeAdapter\n"));

    // No active and waiting sends
    ASSERT(Adapter->nBusySend == 0);
    ASSERT(Adapter->nWaitSend == 0);
    ASSERT(IsQueueEmpty(&Adapter->SendWaitQueue));
    ASSERT(IsQueueEmpty(&Adapter->SendCancelQueue));

    // No other pending operations
    ASSERT(IsListEmpty(&Adapter->RecvPendList));
    ASSERT(Adapter->bAllocNewRfd == FALSE);
    ASSERT(!MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION));
    ASSERT(MP_GET_REF(Adapter) == 0);

    // Free hardware resources
    //      
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE))
    {
        NdisMDeregisterInterruptEx(Adapter->NdisInterruptHandle);
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE);
    }
    if (gpFECReg)
    {
        NdisMDeregisterIoPortRange( 
            Adapter->AdapterHandle,
            (ULONG)CSP_BASE_REG_PA_FEC,
            sizeof(CSP_FEC_REGS),
            (PVOID)gpFECReg);
    }
    FECEnetDeinit();        // SP FEC hardware deinitialized.
              
    // Free RECV memory/NDIS buffer/NDIS packets/shared memory
    ASSERT(Adapter->nReadyRecv == Adapter->CurrNumRfd);

    while (!IsListEmpty(&Adapter->RecvList))
    {
        pMpRfd = (PMP_RFD)RemoveHeadList(&Adapter->RecvList);
        NICFreeRfd(Adapter, pMpRfd, FALSE);
    }
    // Free receive buffer pool
    if (Adapter->RecvBufferPool)
    {
        Adapter->RecvBufferPool = NULL;
    }
    // Free receive packet pool
    if (Adapter->RecvNetBufferListPool)
    {
        NdisFreeNetBufferListPool(Adapter->RecvNetBufferListPool);
        Adapter->RecvNetBufferListPool = NULL;
    }    
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE))
    {
        NdisDeleteNPagedLookasideList(&Adapter->RecvLookaside);
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE);
    }                         
    // Free SEND memory/NDIS buffer/NDIS packets/shared memory
    while (!IsSListEmpty(&Adapter->SendBufList))
    {
        pMpTxBuf = (PMP_TXBUF)PopEntryList(&Adapter->SendBufList);
        ASSERT(pMpTxBuf);            
        // Free the NDIS buffer 
        if (pMpTxBuf->Mdl)
        {
            NdisFreeMdl(pMpTxBuf->Mdl);
            pMpTxBuf->Mdl = NULL;
        }
    }
    if (Adapter->MpTxBufAllocVa != NULL)
    {
        NdisMFreeSharedMemory(
            Adapter->AdapterHandle,
            Adapter->MpTxBufAllocSize,
            TRUE,
            Adapter->MpTxBufAllocVa,
            Adapter->MpTxBufAllocPa);
    
        Adapter->MpTxBufAllocVa = NULL;
    }
    //
    // Free the send buffer pool
    // 
    if (Adapter->SendBufferPool)
    {
        Adapter->SendBufferPool = NULL;
    }
    //
    // Free the shared memory for HW_TBD structures
    // 
    if (Adapter->HwSendMemAllocVa)
    {
        NdisMFreeSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwSendMemAllocSize,
            FALSE,
            Adapter->HwSendMemAllocVa,
            Adapter->HwSendMemAllocPa);
        Adapter->HwSendMemAllocVa = NULL;
    }
    //
    // Free the shared memory for HW_TBD structures
    // 
    if (Adapter->HwSendChipMemAllocVa)
    {
        NdisMFreeSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwSendChipMemAllocSize,
            FALSE,
            Adapter->HwSendChipMemAllocVa,
            Adapter->HwSendChipMemAllocPa);
        Adapter->HwSendChipMemAllocVa = NULL;
    }
    //
    // Free the shared memory for Recv Buf structures
    // 
    if (Adapter->HwRecvMemAllocVa)
    {
        NdisMFreeSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwRecvMemAllocSize,
            FALSE,
            Adapter->HwRecvMemAllocVa,
            Adapter->HwRecvMemAllocPa);
        Adapter->HwRecvMemAllocVa = NULL;
    }   
    //
    // Free the shared memory for Recv Buf structures
    // 
    if (Adapter->HwRecvBufAllocVa)
    {
        NdisMFreeSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwRecvBufAllocSize,
            FALSE,
            Adapter->HwRecvBufAllocVa,
            Adapter->HwRecvBufAllocPa);
        Adapter->HwRecvBufAllocVa = NULL;
    }
    //
    // Deregister the DMA handle after freeing all shared memory
    //
    if (Adapter->NdisMiniportDmaHandle != NULL)
    {
        NdisMDeregisterScatterGatherDma(Adapter->NdisMiniportDmaHandle);
    }
    Adapter->NdisMiniportDmaHandle = NULL;
    if (Adapter->pSGListMem)
    {
        MP_FREEMEM(Adapter->pSGListMem);
        Adapter->pSGListMem = NULL;
    }
    //
    // Free the memory for MP_TCB structures
    // 
    if (Adapter->MpTcbMem)
    {
        MP_FREEMEM(Adapter->MpTcbMem);
        Adapter->MpTcbMem = NULL;
    }
    NdisFreeSpinLock(&Adapter->Lock);
    NdisFreeSpinLock(&Adapter->SendLock);
    NdisFreeSpinLock(&Adapter->RcvLock);    
    MP_FREEMEM(Adapter);  

#if DBG
    if (MPInitDone)
    {
        NdisFreeSpinLock(&MPMemoryLock);
    }
#endif

    DBGPRINT(MP_TRACE, ("<-- NICFreeAdapter\n"));
}

NDIS_STATUS
NICReadRegParameters(
    IN  PMP_ADAPTER     Adapter
    )
/*++
Routine Description:

    Read the following from the registry
    1. All the parameters
    2. NetworkAddres

Arguments:

    Adapter                         Pointer to our adapter
    MiniportAdapterHandle           For use by NdisOpenConfiguration
    
Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_FAILURE
    NDIS_STATUS_RESOURCES                                       

--*/    
{
    NDIS_STATUS                     Status = NDIS_STATUS_SUCCESS;
    NDIS_HANDLE                     ConfigurationHandle;
    UINT                            i;
    PUCHAR                          NetworkAddress;
    UINT                            Length;
    NDIS_CONFIGURATION_OBJECT       ConfigObject;
    UINT16                          FecMacFromNAND[3]={0,0,0};
    PNDIS_CONFIGURATION_PARAMETER   Value;
    USHORT                          TmpVal;
    NDIS_STATUS                     MACStatus;
    NDIS_STRING MACAddrString[3]= { NDIS_STRING_CONST("MACAddress1"),
                                    NDIS_STRING_CONST("MACAddress2"),
                                    NDIS_STRING_CONST("MACAddress3")};

    DBGPRINT(MP_TRACE, ("--> NICReadRegParameters\n"));

    // Get the FEC Mac address set at boot time.    
    BSPGetFECMACAddressFromNAND(FecMacFromNAND,3);  
    
    for(i = 0; i < 3; i++)
    {
        Adapter->FecMacAddress[i*2]     =   (FecMacFromNAND[i] & 0x00FF);
        Adapter->FecMacAddress[(i*2)+1] =   (FecMacFromNAND[i] >> 8);
    }   
    // If the MAC address is not set at boot time, it needs to be read from registry.
    if(!(Adapter->FecMacAddress[0] | Adapter->FecMacAddress[1] | Adapter->FecMacAddress[2] | 
        Adapter->FecMacAddress[3] | Adapter->FecMacAddress[4] | Adapter->FecMacAddress[5] ))
    {
        ConfigObject.Header.Type = NDIS_OBJECT_TYPE_CONFIGURATION_OBJECT;
        ConfigObject.Header.Revision = NDIS_CONFIGURATION_OBJECT_REVISION_1;
        ConfigObject.Header.Size = sizeof(NDIS_CONFIGURATION_OBJECT);
        ConfigObject.NdisHandle = Adapter->AdapterHandle;
        ConfigObject.Flags = 0;

        Status = NdisOpenConfigurationEx(&ConfigObject,
        &ConfigurationHandle);   
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(MP_ERROR, ("NdisOpenConfiguration failed\n"));
            DBGPRINT_S(Status, ("<-- NICReadRegParameters, Status=%x\n", Status));
            return Status;
        }   
        // read in the MAC address in registry
        for(i = 0; i < 3; i++)
        {
            NdisReadConfiguration(&MACStatus, &Value, ConfigurationHandle, 
            &MACAddrString[i], NdisParameterHexInteger);
            if(MACStatus == NDIS_STATUS_SUCCESS)
            {
                TmpVal=(USHORT)Value->ParameterData.IntegerData;
                Adapter->FecMacAddress[i*2] = (UCHAR)((TmpVal & 0xff00) >> 8);
                Adapter->FecMacAddress[(i*2)+1] = (UCHAR)(TmpVal & 0x00ff);
            }
            else
            {
                UINT32 MacLength = 0;

                // assigned all to 0, use MAC settings hardcoded
                for (MacLength = 0; MacLength < ETH_LENGTH_OF_ADDRESS; MacLength++)
                {
                    Adapter->FecMacAddress[MacLength] = 0;
                }
                break;
            }
        }
        ETH_COPY_NETWORK_ADDRESS(Adapter->CurrentAddress,Adapter->FecMacAddress);
        ETH_COPY_NETWORK_ADDRESS(Adapter->PermanentAddress,Adapter->FecMacAddress);
        // Read NetworkAddress registry value 
        // Use it as the current address if any
        if (Status == NDIS_STATUS_SUCCESS)
        {
            NdisReadNetworkAddress(
            &Status,
            &NetworkAddress,
            &Length,
            ConfigurationHandle);
            // If there is a NetworkAddress override in registry, use it 
            if ((Status == NDIS_STATUS_SUCCESS) && (Length == ETH_LENGTH_OF_ADDRESS))
            {
                if ((ETH_IS_MULTICAST(NetworkAddress) 
                || ETH_IS_BROADCAST(NetworkAddress))
                || !ETH_IS_LOCALLY_ADMINISTERED (NetworkAddress))
                {
                    DBGPRINT(MP_ERROR, 
                    ("Overriding NetworkAddress is invalid - %02x-%02x-%02x-%02x-%02x-%02x\n", 
                    NetworkAddress[0], NetworkAddress[1], NetworkAddress[2],
                    NetworkAddress[3], NetworkAddress[4], NetworkAddress[5]));
                }
                else
                {
                ETH_COPY_NETWORK_ADDRESS(Adapter->CurrentAddress, NetworkAddress);
                Adapter->bOverrideAddress = TRUE;
                }
            }
            Status = NDIS_STATUS_SUCCESS;
        }
        // Close the registry
        NdisCloseConfiguration(ConfigurationHandle);
    }
    else
    {
        ETH_COPY_NETWORK_ADDRESS(Adapter->CurrentAddress,Adapter->FecMacAddress);
        ETH_COPY_NETWORK_ADDRESS(Adapter->PermanentAddress,Adapter->FecMacAddress);
    } 
    DBGPRINT(MP_TRACE, ("FEC MAC address is - %02x-%02x-%02x-%02x-%02x-%02x\n", 
                        Adapter->FecMacAddress[0], Adapter->FecMacAddress[1], Adapter->FecMacAddress[2],
                        Adapter->FecMacAddress[3], Adapter->FecMacAddress[4], Adapter->FecMacAddress[5]));
    DBGPRINT_S(Status, ("<-- NICReadRegParameters, Status=%x\n", Status));

    return Status;
}

NDIS_STATUS 
NICAllocAdapterMemory(
    IN  PMP_ADAPTER     Adapter
    )
/*++
Routine Description:

    Allocate all the memory blocks for send, receive and others

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_FAILURE
    NDIS_STATUS_RESOURCES

--*/    
{
#ifdef UNDER_CE
    NDIS_STATUS                     Status = NDIS_STATUS_SUCCESS;
#else  //  UNDER_CE
    NDIS_STATUS                     Status;
#endif  //  UNDER_CE
    PMP_TXBUF                       pMpTxbuf;
    PUCHAR                          pMem;
    LONG                            index;
    ULONG                           ErrorValue = 0;
#ifndef UNDER_CE
    UINT                            MaxNumBuffers;
#endif  //  UNDER_CE
    NDIS_SG_DMA_DESCRIPTION         DmaDescription;
    ULONG                           pMemSize;
    PUCHAR                          AllocVa;
    NDIS_PHYSICAL_ADDRESS           AllocPa;
    PVOID                           MpTxBufMem;
    NET_BUFFER_LIST_POOL_PARAMETERS PoolParameters;
    ULONGLONG                       TcbLength, SgListLength, SendBufLength;
    
    DBGPRINT(MP_TRACE, ("--> NICAllocMemory\n"));

    do
    {
        Adapter->NumTcb = FEC_TX_RING_SIZE;
        NdisZeroMemory(&DmaDescription, sizeof(DmaDescription));
        DmaDescription.Header.Type = NDIS_OBJECT_TYPE_SG_DMA_DESCRIPTION;
        DmaDescription.Header.Revision = NDIS_SG_DMA_DESCRIPTION_REVISION_1;
        DmaDescription.Header.Size = sizeof(NDIS_SG_DMA_DESCRIPTION);
        DmaDescription.Flags = 0;       // we don't do 64 bit DMA
        
        //
        // Even if offload is enabled, the packet size for mapping shouldn't change
        //
        DmaDescription.MaximumPhysicalMapping = NIC_MAX_PACKET_SIZE;
        DmaDescription.ProcessSGListHandler = MpProcessSGList;
        //FEC does not call NdisMAllocateSharedMemoryAsyncEx, hence no need for complete handler
        DmaDescription.SharedMemAllocateCompleteHandler = NULL;

        Status = NdisMRegisterScatterGatherDma(
                    Adapter->AdapterHandle,
                    &DmaDescription,
                    &Adapter->NdisMiniportDmaHandle);                   
        if (Status == NDIS_STATUS_SUCCESS)
        {
            Adapter->ScatterGatherListSize = DmaDescription.ScatterGatherListSize;
            MP_SET_FLAG(Adapter, fMP_ADAPTER_SCATTER_GATHER);
        }
        else
        {
            DBGPRINT(MP_WARN, ("Failed to init ScatterGather DMA\n"));

            //
            // NDIS 6.0 miniport should NOT use map register
            //
            ErrorValue = ERRLOG_OUT_OF_SG_RESOURCES;                 
            break;  
        }
        //
        // Send
        //
        // Allocate MP_TCB's
        //         
        Adapter->MpTcbMemSize = Adapter->NumTcb * (sizeof(MP_TCB) + sizeof(MP_TXBUF));

        //
        // Integer overflow test
        //
        if ((Adapter->NumTcb <= 0) || 
            ((sizeof(MP_TCB) + sizeof(MP_TXBUF)) < sizeof(MP_TCB)))
        {
            Status = NDIS_STATUS_RESOURCES;
            ErrorValue = ERRLOG_OUT_OF_MEMORY;
            DBGPRINT(MP_ERROR, ("Integer overflow caused by invalid NumTcb\n"));
            break;                
        }
        TcbLength = (ULONGLONG)Adapter->NumTcb * (ULONGLONG)(sizeof(MP_TCB) + sizeof(MP_TXBUF));        
        if (TcbLength > (ULONG)-1)            
        {
            Status = NDIS_STATUS_RESOURCES;
            ErrorValue = ERRLOG_OUT_OF_MEMORY;
            DBGPRINT(MP_ERROR, ("Integer overflow caused by invalid NumTcb\n"));
            break;        
        }
        pMem = MP_ALLOCMEMTAG(Adapter->AdapterHandle, Adapter->MpTcbMemSize);
        if (pMem == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            ErrorValue = ERRLOG_OUT_OF_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate MP_TCB's\n"));
            break;
        }
        NdisZeroMemory(pMem, Adapter->MpTcbMemSize);
        Adapter->MpTcbMem = pMem;
        MpTxBufMem = pMem + (Adapter->NumTcb * sizeof(MP_TCB));
        //
        // Allocate memory for scatter-gather list
        //
        pMemSize = ((ULONG)Adapter->NumTcb) * Adapter->ScatterGatherListSize;

        //
        // Integer overflow check        
        //
        SgListLength = (ULONG)Adapter->NumTcb * (ULONG)Adapter->ScatterGatherListSize;
        if (SgListLength > (ULONG)-1)
        {
            Status = NDIS_STATUS_RESOURCES;                
            ErrorValue = ERRLOG_OUT_OF_MEMORY;
            DBGPRINT(MP_ERROR, ("Interger overflow while allocating Scatter Gather list\n"));
            break;           
        }
        pMem = MP_ALLOCMEMTAG(Adapter->AdapterHandle, pMemSize);        
        if (pMem == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;                
            ErrorValue = ERRLOG_OUT_OF_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate Scatter Gather list buffer\n"));
            break;
        }
        NdisZeroMemory(pMem, pMemSize);
        Adapter->pSGListMem = pMem;        
        Adapter->nPendingSendPacket = 0;   

        //
        // The driver should allocate more data than sizeof(RFD_STRUC) to allow the
        // driver to align the data(after ethernet header) at 8 byte boundary
        //

        Adapter->HwRfdSize = sizeof(RFD_STRUC);
        Adapter->NumRfd = FEC_RX_RING_SIZE;
        Adapter->HwRecvMemAllocSize =  Adapter->NumRfd * Adapter->HwRfdSize;
        NdisMAllocateSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwRecvMemAllocSize,
            FALSE,
            (PVOID) &Adapter->HwRecvMemAllocVa,
            &Adapter->HwRecvMemAllocPa);
        if (!Adapter->HwRecvMemAllocVa)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            DBGPRINT (ZONE_ERROR, ("Failed to allocate recv BD memory\n"));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        NdisZeroMemory(Adapter->HwRecvMemAllocVa, Adapter->HwRecvMemAllocSize);
        NdisInitializeNPagedLookasideList(
            &Adapter->RecvLookaside,
            NULL,
            NULL,
            0,
            sizeof(MP_RFD),
            NIC_TAG, 
            0);

        MP_SET_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE);        

        //Allocate NumTCB TX BDs for wrapper & actual communication with FEC
        Adapter->HwSendChipMemAllocSize = Adapter->NumTcb * sizeof(TBD_STRUC);

        NdisMAllocateSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwSendChipMemAllocSize,
            FALSE,
            (PVOID) &Adapter->HwSendChipMemAllocVa,
            &Adapter->HwSendChipMemAllocPa);

        if (!Adapter->HwSendChipMemAllocVa)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate send memory for Chip\n"));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        NdisZeroMemory(Adapter->HwSendChipMemAllocVa, Adapter->HwSendMemAllocSize);
        //
        // Allocate send buffers
        //
        Adapter->CacheFillSize = NdisMGetDmaAlignment(Adapter->AdapterHandle);
        DBGPRINT(MP_INFO, ("CacheFillSize=%d\n", Adapter->CacheFillSize));

        // HW_START
        //
        // Allocate shared memory for send
        //
        
        //Allocate NumTCB TX BDs
        Adapter->HwSendMemAllocSize = ((ULONG)Adapter->NumTcb) * sizeof(TBD_STRUC);

        if (Adapter->HwSendMemAllocSize > (ULONG)-1)
        {
            Status = NDIS_STATUS_RESOURCES;
            ErrorValue = ERRLOG_OUT_OF_MEMORY;
            DBGPRINT(MP_ERROR, ("Integer overflow caused by invalid NumTcb\n"));
            break;        
        }
        NdisMAllocateSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwSendMemAllocSize,
            FALSE,
            (PVOID) &Adapter->HwSendMemAllocVa,
            &Adapter->HwSendMemAllocPa);

        if (!Adapter->HwSendMemAllocVa)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate send memory\n"));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
        NdisZeroMemory(Adapter->HwSendMemAllocVa, Adapter->HwSendMemAllocSize);

        // HW_END
        //
        // Recv
        //
        //Allocate Recv Buffer
        Adapter->HwRecvBufAllocSize = Adapter->NumRfd * NIC_MAX_PACKET_SIZE; //2048*16
        if (Adapter->HwRecvBufAllocSize > (ULONG)-1)
        {
            break;
        }       
        NdisMAllocateSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwRecvBufAllocSize,
            FALSE,
            &Adapter->HwRecvBufAllocVa,
            &Adapter->HwRecvBufAllocPa);

        //
        // alloc the recv net buffer list pool with net buffer inside the list
        //
        NdisZeroMemory(&PoolParameters, sizeof(NET_BUFFER_LIST_POOL_PARAMETERS));
        PoolParameters.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
        PoolParameters.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
        PoolParameters.Header.Size = sizeof(PoolParameters);
        PoolParameters.ProtocolId = 0;
        PoolParameters.ContextSize = 0;
        PoolParameters.fAllocateNetBuffer = TRUE;
        PoolParameters.PoolTag = NIC_TAG;

        Adapter->RecvNetBufferListPool = NdisAllocateNetBufferListPool(
                    MP_GET_ADAPTER_HANDLE(Adapter),
                    &PoolParameters); 

        if (Adapter->RecvNetBufferListPool == NULL)
        {
            ErrorValue = ERRLOG_OUT_OF_PACKET_POOL;
            break;
        }       
        Adapter->MpTxBufAllocSize = Adapter->NumTcb * (NIC_MAX_PACKET_SIZE+ Adapter->CacheFillSize );

        // 
        // Integer overflow check
        //
        if ((NIC_MAX_PACKET_SIZE + Adapter->CacheFillSize ) <
            NIC_MAX_PACKET_SIZE)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            DBGPRINT(MP_ERROR, ("Integer overflow while allocating send buffers\n"));
            Status = NDIS_STATUS_RESOURCES;
            break;        
        }
        SendBufLength = (ULONGLONG)Adapter->NumTcb * 
                        (ULONGLONG) (NIC_MAX_PACKET_SIZE + Adapter->CacheFillSize);
        if (SendBufLength > (ULONG)-1)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            DBGPRINT(MP_ERROR, ("Integer overflow while allocating send buffers\n"));
            Status = NDIS_STATUS_RESOURCES;
            break;        
        }
        NdisMAllocateSharedMemory(
                        Adapter->AdapterHandle,
                        Adapter->MpTxBufAllocSize,
                        TRUE,                              // CACHED
                        &Adapter->MpTxBufAllocVa,  
                        &Adapter->MpTxBufAllocPa);

        if (Adapter->MpTxBufAllocVa == NULL)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate a big buffer\n"));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }      
        AllocVa = Adapter->MpTxBufAllocVa;
        AllocPa = Adapter->MpTxBufAllocPa;
        pMpTxbuf = (PMP_TXBUF)MpTxBufMem;      
        for (index = 0; index < Adapter->NumTcb; index++)
        {
            pMpTxbuf->AllocSize = NIC_MAX_PACKET_SIZE + Adapter->CacheFillSize;
            pMpTxbuf->BufferSize = NIC_MAX_PACKET_SIZE;
            pMpTxbuf->AllocVa = AllocVa;
            pMpTxbuf->AllocPa = AllocPa;
            //
            // Align the buffer on the cache line boundary
            //
            pMpTxbuf->pBuffer = MP_ALIGNMEM(pMpTxbuf->AllocVa, Adapter->CacheFillSize);
            pMpTxbuf->BufferPa.QuadPart = MP_ALIGNMEM_PA(pMpTxbuf->AllocPa, Adapter->CacheFillSize);

            pMpTxbuf->Mdl = NdisAllocateMdl(Adapter->AdapterHandle, pMpTxbuf->pBuffer, pMpTxbuf->BufferSize);

            if (pMpTxbuf->Mdl == NULL)
            {
                ErrorValue = ERRLOG_OUT_OF_NDIS_BUFFER;
                DBGPRINT(MP_ERROR, ("Failed to allocate MDL for a big buffer\n"));
                break;
            }
            PushEntryList(&Adapter->SendBufList, &pMpTxbuf->SList);         
            pMpTxbuf++;
            AllocVa += NIC_MAX_PACKET_SIZE;
            AllocPa.QuadPart += NIC_MAX_PACKET_SIZE; 
        }
        if (Status != NDIS_STATUS_SUCCESS) 
            break;        
        
        Status = NDIS_STATUS_SUCCESS;
    } while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisWriteErrorLogEntry(
            Adapter->AdapterHandle,
            NDIS_ERROR_CODE_OUT_OF_RESOURCES,
            1,
            ErrorValue);
    }
    DBGPRINT_S(Status, ("<-- NICAllocMemory, Status=%x\n", Status));
    return Status;
}

VOID NICInitSend(
    IN  PMP_ADAPTER     Adapter
    )
/*++
Routine Description:

    Initialize send data structures

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    None                                                    

--*/    
{
    PMP_TCB         pMpTcb;
    ULONG           HwTbdPhys;
    LONG            TcbCount;
    PMP_TXBUF       pMpTxBuf;
    PTBD_STRUC      pHwTbd;  
    PTBD_STRUC      pHwTbdChip;
    
    DBGPRINT(MP_TRACE, ("--> NICInitSend\n"));

    //
    // Setup the initial pointers to the SW and HW TCB data space
    //
    pMpTcb = (PMP_TCB) Adapter->MpTcbMem;
    pMpTxBuf = (PMP_TXBUF) (pMpTcb + Adapter->NumTcb);
    pHwTbd = (PHW_TBD) Adapter->HwSendMemAllocVa;
    HwTbdPhys = NdisGetPhysicalAddressLow(Adapter->HwSendMemAllocPa);
    pHwTbdChip = (PHW_TBD) Adapter->HwSendChipMemAllocVa;
    Adapter->HwSendTbdCurrentCount = 0;

    // Go through and set up each TCB
    for (TcbCount = 0; TcbCount < Adapter->NumTcb; TcbCount++)
    {
        Adapter->HwTbdChip[TcbCount] = pHwTbdChip;
        pMpTcb->HwTbd = pHwTbd;                 // save ptr to HW TBD
        pMpTcb->HwTbdPhys = HwTbdPhys;      // save HW TBD physical address
        pMpTcb->HwTbdIndex = -1;
        pMpTcb->MpTxBuf = pMpTxBuf;

        if (TcbCount)
            pMpTcb->PrevHwTbd = pHwTbd - 1;
        else
            pMpTcb->PrevHwTbd   = (PHW_TBD)((PUCHAR)Adapter->HwSendMemAllocVa +
                                      ((Adapter->NumTcb - 1) * sizeof(HW_TBD)));

        ASSERT(Adapter->pSGListMem != NULL);
        pMpTcb->ScatterGatherListBuffer = Adapter->pSGListMem + TcbCount * (LONG) Adapter->ScatterGatherListSize;       
        pHwTbd->ControlStatus = 0;        // clear the status 
        pHwTbd->BufferAddress = (ULONG) pMpTxBuf->BufferPa.QuadPart;


        // Set the link pointer in HW TBD to the next TBD in the chain.  
        // If this is the last TCB in the chain, then set it to the first TCB.
        if (TcbCount < Adapter->NumTcb - 1)
        {
            pMpTcb->Next = pMpTcb + 1;
        }
        else
        {
            pMpTcb->Next = (PMP_TCB) Adapter->MpTcbMem;
        }
        // Set the previous TCB, if this is the first TCB in the chain, set it to the last
        // TCB.
        if (TcbCount)
        {
            pMpTcb->Prev = pMpTcb - 1;
        }
        else
        {
            pMpTcb->Prev = (PMP_TCB)((PUCHAR)Adapter->MpTcbMem + ((Adapter->NumTcb - 1) * sizeof(MP_TCB)));
        }
        pHwTbdChip->ControlStatus = 0;
        pHwTbdChip->DataLen = 0;
        pHwTbdChip->BufferAddress = pHwTbd->BufferAddress;

        pHwTbdChip++;

        pMpTcb++; 
        pMpTxBuf++;
        pHwTbd++;
        HwTbdPhys += sizeof(TBD_STRUC);
    }
    pHwTbd--;
    pHwTbd->ControlStatus |= BD_SC_WRAP;
    Adapter->HwTbdChip[TcbCount - 1]->ControlStatus |= BD_SC_WRAP;
    Adapter->nPendingSendPacket = 0;
    
    // set the TCB head/tail indexes
    // head is the olded one to free, tail is the next one to use
    Adapter->CurrSendHead = (PMP_TCB) Adapter->MpTcbMem;
    Adapter->CurrSendTail = (PMP_TCB) Adapter->MpTcbMem;
    Adapter->TxdStatus.FramesXmitGood = 0;
    Adapter->TxdStatus.FramesXmitBad = 0;
    Adapter->TxdStatus.FramesXmitHBErrors = 0;
    Adapter->TxdStatus.FramesXmitUnderrunErrors = 0;
    Adapter->TxdStatus.FramesXmitCollisionErrors = 0;
    Adapter->TxdStatus.FramesXmitAbortedErrors = 0;
    Adapter->TxdStatus.FramsXmitCarrierErrors = 0;

    DBGPRINT(MP_TRACE, ("<-- NICInitSend, Status=%x\n"));
}

NDIS_STATUS NICInitRecv(
    IN  PMP_ADAPTER     Adapter
    )
/*++
Routine Description:

    Initialize receive data structures

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_RESOURCES

--*/    
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;//NDIS_STATUS_RESOURCES;
    PMP_RFD                 pMpRfd;      
    LONG                    RfdCount;
    ULONG                   ErrorValue = 0;
    PUCHAR                  HwRfds;
    NDIS_PHYSICAL_ADDRESS   HwRfdsPa;
    PRFD_STRUC              pHwRfd=NULL;
    ULONG                   RecvBufPhyAddr;

    DBGPRINT(MP_TRACE, ("--> NICInitRecv\n"));
    do
    {
        //
        // Setup each RFD
        // 
        HwRfds =  Adapter->HwRecvMemAllocVa;
        HwRfdsPa = Adapter->HwRecvMemAllocPa;
        pHwRfd = (PRFD_STRUC) Adapter->HwRecvMemAllocVa;

        //Allocate numTCB RX TBDs next.
        for (RfdCount = 0; RfdCount < Adapter->NumRfd; RfdCount++)
        {
            pMpRfd = NdisAllocateFromNPagedLookasideList(&Adapter->RecvLookaside);
            if (!pMpRfd)
            {
                ErrorValue = ERRLOG_OUT_OF_LOOKASIDE_MEMORY;
                continue;
            }
            //We have a fixed TBD Structure for FEC, so RFD to TBD is 1:1 and association fixed always
            pMpRfd->HwRfdIndex = RfdCount; 

            // Get a 8-byts aligned memory from the original HwRfd
            //
            pMpRfd->HwRfd = pHwRfd;

            //
            // Update physical address accordingly
            // 
            pMpRfd->HwRfdPa.QuadPart = HwRfdsPa.QuadPart;           
            RecvBufPhyAddr = (ULONG)Adapter->HwRecvBufAllocPa.QuadPart + (RfdCount * NIC_MAX_PACKET_SIZE);
            pMpRfd->HwRfd->BufferAddress = (ULONG)RecvBufPhyAddr ; 
            pMpRfd->HwRfd->ControlStatus = BD_ENET_RX_EMPTY|BD_ENET_RX_PAD;
            ErrorValue = NICAllocRfd(Adapter, pMpRfd,RfdCount);
            if (ErrorValue)
            {
                NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pMpRfd);
                continue;
            }

            //
            // Add this RFD to the RecvList
            // 
            Adapter->CurrNumRfd++;
            HwRfds += Adapter->HwRfdSize;
            HwRfdsPa.QuadPart += Adapter->HwRfdSize;
            pHwRfd++;
            
            Adapter->pMpRfdList[RfdCount] = (PUCHAR)pMpRfd;
            NICReturnRFD(Adapter, pMpRfd);

        }

        pHwRfd--;
        pHwRfd->ControlStatus |= BD_SC_WRAP;
    }
    while (FALSE);

    Adapter->HwRecvRbdCurrentCount = 0;     //Set the recv TBD to the first
    if (Adapter->CurrNumRfd >= NIC_MIN_RFDS)
    {
        Status = NDIS_STATUS_SUCCESS;
    }
    //
    // Adapter->CurrNumRfd < NIC_MIN_RFDs
    //
    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisWriteErrorLogEntry(
            Adapter->AdapterHandle,
            NDIS_ERROR_CODE_OUT_OF_RESOURCES,
            1,
            ErrorValue);
    }
    Adapter->RcvStatus.FrameRcvGood = 0;
    Adapter->RcvStatus.FrameRcvErrors = 0;
    Adapter->RcvStatus.FrameRcvExtraDataErrors = 0;
    Adapter->RcvStatus.FrameRcvShortDataErrors = 0;
    Adapter->RcvStatus.FrameRcvCRCErrors = 0;
    Adapter->RcvStatus.FrameRcvOverrunErrors = 0;
    Adapter->RcvStatus.FrameRcvAllignmentErrors = 0;
    Adapter->RcvStatus.FrameRcvLCErrors = 0;

    DBGPRINT_S(Status, ("<-- NICInitRecv, Status=%x\n", Status));
    return Status;
}

ULONG NICAllocRfd(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_RFD         pMpRfd,
    IN  LONG            BufferIndex
    )
/*++
Routine Description:

    Allocate NET_BUFFER_LIST, NET_BUFFER and MDL associated with a RFD

Arguments:

    Adapter     Pointer to our adapter
    pMpRfd      pointer to a RFD

Return Value:

    ERRLOG_OUT_OF_NDIS_PACKET
    ERRLOG_OUT_OF_NDIS_BUFFER

--*/    
{
#ifndef UNDER_CE
    NDIS_STATUS         Status;
#endif  //  UNDER_CE
    PHW_RFD             pHwRfd;    
#ifndef UNDER_CE
    ULONG               HwRfdPhys;  
#endif  //  UNDER_CE
    ULONG               ErrorValue = 0;

    do
    {
        pHwRfd = pMpRfd->HwRfd;
        pMpRfd->HwRfdPhys = NdisGetPhysicalAddressLow(pMpRfd->HwRfdPa);
        pMpRfd->Flags = 0;
        pMpRfd->NetBufferList = NULL;
        pMpRfd->Mdl = NULL;
        
        //
        // point our buffer for receives at this Rfd
        // 
        pMpRfd->Mdl = NdisAllocateMdl(Adapter->AdapterHandle, (PVOID)(Adapter->HwRecvBufAllocVa+(BufferIndex*NIC_MAX_PACKET_SIZE)), NIC_MAX_PACKET_SIZE);          
        if (pMpRfd->Mdl == NULL)
        {
            ErrorValue = ERRLOG_OUT_OF_NDIS_BUFFER;
            break;
        }
        //
        // the NetBufferList may change later(RequestControlSize is not enough)
        //
        pMpRfd->NetBufferList = NdisAllocateNetBufferAndNetBufferList(
                                        Adapter->RecvNetBufferListPool,
                                        0,                      // RequestContolOffsetDelta
                                        0,                      // BackFill Size
                                        pMpRfd->Mdl,     // MdlChain
                                        0,                      // DataOffset
                                        0);                     // DataLength
        
        if (pMpRfd->NetBufferList == NULL)
        {
            ErrorValue = ERRLOG_OUT_OF_NDIS_PACKET;
            break;
        }
        pMpRfd->NetBufferList->SourceHandle = MP_GET_ADAPTER_HANDLE(Adapter);
        
        //
        // Initialize the NetBufferList
        //
        // Save ptr to MP_RFD in the NBL, used in MPReturnNetBufferLists 
        // 
        MP_GET_NET_BUFFER_LIST_RFD(pMpRfd->NetBufferList) = pMpRfd;
        DBGPRINT(MP_TRACE, ("NICAllocRfd- Associating RFD "PTR_FORMAT" with Buffer "PTR_FORMAT" \n",pMpRfd,pMpRfd->NetBufferList));
    } while (FALSE);

    if (ErrorValue)
    {
        if (pMpRfd->Mdl)
        {
            NdisFreeMdl(pMpRfd->Mdl);
        }
        //
        // Do not free the shared memory
        //
    }
    return ErrorValue;
}

VOID NICFreeRfd(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_RFD         pMpRfd,
    IN  BOOLEAN         DispatchLevel
    )
/*++
Routine Description:

    Free a RFD and assocaited NET_BUFFER_LIST

Arguments:

    Adapter         Pointer to our adapter
    pMpRfd          Pointer to a RFD
    DisptchLevel    Specify if the caller is at DISPATCH level

Return Value:

    None                                                    

--*/    
{
    ASSERT(pMpRfd->Mdl);      
    ASSERT(pMpRfd->NetBufferList);  
    ASSERT(pMpRfd->HwRfd);    

    NdisFreeMdl(pMpRfd->Mdl);
    NdisFreeNetBufferList(pMpRfd->NetBufferList);
    
    pMpRfd->Mdl = NULL;
    pMpRfd->NetBufferList = NULL;
    pMpRfd->HwRfd = NULL;

    NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pMpRfd);
}





