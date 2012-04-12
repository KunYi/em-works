//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  swifunc.cpp
//
//  Implementation of SWITCH Driver
//
//  This file implements hardware related functions for SWITCH.
//
//-----------------------------------------------------------------------------
#include "common_macros.h"
#include "enetswi.h"
#include "swifunc.h"


//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
SYSTEMTIME SystemTimestamp;
BOOL AgeingInterval=FALSE;
extern PVOID pv_HWregENET0;
extern PVOID pv_HWregENET1;
extern PVOID pv_HWregENETSWI;
extern PVOID pv_HWLMENETSWI;

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// File-local(static) Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Child Functions for SWITCH feature Init/Query/Set
//------------------------------------------------------------------------------

void QueryPORTENA(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_PORTENA QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.ENA_TRANSMIT_0 = BF_RD(ENET_SWI_PORT_ENA, ENA_TRANSMIT_0);
    QuerySetData.ENA_TRANSMIT_1 = BF_RD(ENET_SWI_PORT_ENA, ENA_TRANSMIT_1);
    QuerySetData.ENA_TRANSMIT_2 = BF_RD(ENET_SWI_PORT_ENA, ENA_TRANSMIT_2);
    QuerySetData.ENA_RECEIVE_0 = BF_RD(ENET_SWI_PORT_ENA, ENA_RECEIVE_0);
    QuerySetData.ENA_RECEIVE_1 = BF_RD(ENET_SWI_PORT_ENA, ENA_RECEIVE_1);
    QuerySetData.ENA_RECEIVE_2 = BF_RD(ENET_SWI_PORT_ENA, ENA_RECEIVE_2);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetPORTENA(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_PORTENA QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        BF_WR(ENET_SWI_PORT_ENA, ENA_TRANSMIT_0, QuerySetData.ENA_TRANSMIT_0);
        BF_WR(ENET_SWI_PORT_ENA, ENA_TRANSMIT_1, QuerySetData.ENA_TRANSMIT_1);
        BF_WR(ENET_SWI_PORT_ENA, ENA_TRANSMIT_2, QuerySetData.ENA_TRANSMIT_2);
        BF_WR(ENET_SWI_PORT_ENA, ENA_RECEIVE_0, QuerySetData.ENA_RECEIVE_0);
        BF_WR(ENET_SWI_PORT_ENA, ENA_RECEIVE_1, QuerySetData.ENA_RECEIVE_1);
        BF_WR(ENET_SWI_PORT_ENA, ENA_RECEIVE_2, QuerySetData.ENA_RECEIVE_2);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}

void InitPORTENA(void)
{
    BF_WR(ENET_SWI_PORT_ENA, ENA_TRANSMIT_0, 1); // enable transmit on port 0
    BF_WR(ENET_SWI_PORT_ENA, ENA_TRANSMIT_1, 1); // enable transmit on port 1
    BF_WR(ENET_SWI_PORT_ENA, ENA_TRANSMIT_2, 1); // enable transmit on port 2
    BF_WR(ENET_SWI_PORT_ENA, ENA_RECEIVE_0, 1); // enable receive on port 0
    BF_WR(ENET_SWI_PORT_ENA, ENA_RECEIVE_1, 1); // enable receive on port 1
    BF_WR(ENET_SWI_PORT_ENA, ENA_RECEIVE_2, 1); // enable receive on port 2
}

void QueryINPUTLEARNBLOCK(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_INPUTLEARNBLOCK QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.BLOCKING_ENA_P0 = BF_RD(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P0);
    QuerySetData.BLOCKING_ENA_P1 = BF_RD(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P1);
    QuerySetData.BLOCKING_ENA_P2 = BF_RD(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P2);
    QuerySetData.LEARNING_DIS_P0 = BF_RD(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P0);
    QuerySetData.LEARNING_DIS_P1 = BF_RD(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P1);
    QuerySetData.LEARNING_DIS_P2 = BF_RD(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P2);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetINPUTLEARNBLOCK(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_INPUTLEARNBLOCK QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P0, QuerySetData.BLOCKING_ENA_P0);
        BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P1, QuerySetData.BLOCKING_ENA_P1);
        BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P2, QuerySetData.BLOCKING_ENA_P2);
        BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P0, QuerySetData.LEARNING_DIS_P0);
        BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P1, QuerySetData.LEARNING_DIS_P1);
        BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P2, QuerySetData.LEARNING_DIS_P2);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}

void InitINPUTLEARNBLOCK(void)
{
    // Enable block on input port // frame(except BPDU frame) will be marked for discard and will not be forwarded to any output port
    BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P0, 0); // not enable block on input port 0
    BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P1, 0); // not enable block on input port 1
    BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, BLOCKING_ENA_P2, 0); // not enable block on input port 2
    // Disable learning on input port // incoming frames (except BPDU frame) will not source address extraction // reduce processing load from the firmware
    BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P0, 0); // not disable learning on input port 0
    BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P1, 0); // not disable learning on input port 1
    BF_WR(ENET_SWI_INPUT_LEARN_BLOCK, LEARNING_DIS_P2, 0); // not disable learning on input port 2
}

void QueryVLANVERIFY(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_VLANVERIFY QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.VLAN_VERIFY_0 = BF_RD(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_0);
    QuerySetData.VLAN_VERIFY_1 = BF_RD(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_1);
    QuerySetData.VLAN_VERIFY_2 = BF_RD(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_2);
    QuerySetData.DISCARD_P0 = BF_RD(ENET_SWI_VLAN_VERIFY, DISCARD_P0);
    QuerySetData.DISCARD_P1 = BF_RD(ENET_SWI_VLAN_VERIFY, DISCARD_P1);
    QuerySetData.DISCARD_P2 = BF_RD(ENET_SWI_VLAN_VERIFY, DISCARD_P2);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetVLANVERIFY(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_VLANVERIFY QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        BF_WR(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_0, QuerySetData.VLAN_VERIFY_0);
        BF_WR(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_1, QuerySetData.VLAN_VERIFY_1);
        BF_WR(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_2, QuerySetData.VLAN_VERIFY_2);
        BF_WR(ENET_SWI_VLAN_VERIFY, DISCARD_P0, QuerySetData.DISCARD_P0);
        BF_WR(ENET_SWI_VLAN_VERIFY, DISCARD_P1, QuerySetData.DISCARD_P1);
        BF_WR(ENET_SWI_VLAN_VERIFY, DISCARD_P2, QuerySetData.DISCARD_P2);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}


void InitVLANVERIFY(void)
{
    BF_WR(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_0, 0); // If VLAN verify
    BF_WR(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_1, 0); // If VLAN verify
    BF_WR(ENET_SWI_VLAN_VERIFY, VLAN_VERIFY_2, 0); // If VLAN verify
    BF_WR(ENET_SWI_VLAN_VERIFY, DISCARD_P0, 0); // If discard
    BF_WR(ENET_SWI_VLAN_VERIFY, DISCARD_P1, 0); // If discard
    BF_WR(ENET_SWI_VLAN_VERIFY, DISCARD_P2, 0); // If discard
}

void QueryVLANRESTABLE(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_VLANRESTABLE QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
    DWORD i;

    i=QuerySetData.ENTRY;
    QuerySetData.PORT_0 = BF_RDn(ENET_SWI_VLAN_RES_TABLE, i, PORT_0);
    QuerySetData.PORT_1 = BF_RDn(ENET_SWI_VLAN_RES_TABLE, i, PORT_1);
    QuerySetData.PORT_2 = BF_RDn(ENET_SWI_VLAN_RES_TABLE, i, PORT_2);
    QuerySetData.VLAN_ID = BF_RDn(ENET_SWI_VLAN_RES_TABLE, i, VLAN_ID);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetVLANRESTABLE(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_VLANRESTABLE QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    DWORD i;


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        i=QuerySetData.ENTRY;
        BF_WRn(ENET_SWI_VLAN_RES_TABLE, i, PORT_0, QuerySetData.PORT_0);
        BF_WRn(ENET_SWI_VLAN_RES_TABLE, i, PORT_1, QuerySetData.PORT_1);
        BF_WRn(ENET_SWI_VLAN_RES_TABLE, i, PORT_2, QuerySetData.PORT_2);
        BF_WRn(ENET_SWI_VLAN_RES_TABLE, i, VLAN_ID, QuerySetData.VLAN_ID);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}


void InitVLANRESTABLE(DWORD i)
{
    BF_WRn(ENET_SWI_VLAN_RES_TABLE, i, PORT_0, 0); // disable
    BF_WRn(ENET_SWI_VLAN_RES_TABLE, i, PORT_1, 0); // disable
    BF_WRn(ENET_SWI_VLAN_RES_TABLE, i, PORT_2, 0); // disable
    BF_WRn(ENET_SWI_VLAN_RES_TABLE, i, VLAN_ID, 0); // VLAN ID: such as 0x234
}

void QueryPORTSNOOP(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_PORTSNOOP QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
    DWORD i;

    i=QuerySetData.ENTRY;
    QuerySetData.ENABLE = BF_RDn(ENET_SWI_PORTSNOOP, i, ENABLE);
    QuerySetData.MODE = BF_RDn(ENET_SWI_PORTSNOOP, i, MODE);
    QuerySetData.COMPARE_DEST = BF_RDn(ENET_SWI_PORTSNOOP, i, COMPARE_DEST);
    QuerySetData.COMPARE_SOURCE = BF_RDn(ENET_SWI_PORTSNOOP, i, COMPARE_SOURCE);
    QuerySetData.DESTINATION_PORT = BF_RDn(ENET_SWI_PORTSNOOP, i, DESTINATION_PORT);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetPORTSNOOP(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_PORTSNOOP QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    DWORD i;


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        i=QuerySetData.ENTRY;
        BF_WRn(ENET_SWI_PORTSNOOP, i, ENABLE, QuerySetData.ENABLE);
        BF_WRn(ENET_SWI_PORTSNOOP, i, MODE, QuerySetData.MODE);
        BF_WRn(ENET_SWI_PORTSNOOP, i, COMPARE_DEST, QuerySetData.COMPARE_DEST);
        BF_WRn(ENET_SWI_PORTSNOOP, i, COMPARE_SOURCE, QuerySetData.COMPARE_SOURCE);
        BF_WRn(ENET_SWI_PORTSNOOP, i, DESTINATION_PORT, QuerySetData.DESTINATION_PORT);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}


void InitPORTSNOOP(DWORD i)
{
    BF_WRn(ENET_SWI_PORTSNOOP, i, ENABLE, 0); // disable
    BF_WRn(ENET_SWI_PORTSNOOP, i, MODE, 0); // mode: only to MGMT port
    BF_WRn(ENET_SWI_PORTSNOOP, i, COMPARE_DEST, 0); // compare dst port
    BF_WRn(ENET_SWI_PORTSNOOP, i, COMPARE_SOURCE, 0); // compare src port
    BF_WRn(ENET_SWI_PORTSNOOP, i, DESTINATION_PORT, 0); //  port number compare value: 0x0044 for DHCP
}

void QueryIPSNOOP(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_IPSNOOP QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
    DWORD i;

    i=QuerySetData.ENTRY;
    QuerySetData.ENABLE = BF_RDn(ENET_SWI_IPSNOOP, i, ENABLE);
    QuerySetData.MODE = BF_RDn(ENET_SWI_IPSNOOP, i, MODE);
    QuerySetData.PROTOCOL = BF_RDn(ENET_SWI_IPSNOOP, i, PROTOCOL);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetIPSNOOP(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_IPSNOOP QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    DWORD i;


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        i=QuerySetData.ENTRY;
        BF_WRn(ENET_SWI_IPSNOOP, i, ENABLE, QuerySetData.ENABLE);
        BF_WRn(ENET_SWI_IPSNOOP, i, MODE, QuerySetData.MODE);
        BF_WRn(ENET_SWI_IPSNOOP, i, PROTOCOL, QuerySetData.PROTOCOL);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}


void InitIPSNOOP(DWORD i)
{
    BF_WRn(ENET_SWI_IPSNOOP, i, ENABLE, 0); // disable
    BF_WRn(ENET_SWI_IPSNOOP, i, MODE, 0); // mode: only to MGMT port
    BF_WRn(ENET_SWI_IPSNOOP, i, PROTOCOL, 0); // protocol: 0x11 for UDP protocol
}

void QueryMCASTDEFAULTMASK(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_MCASTDEFAULTMASK QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.MCAST_DEFAULT_MASK_0 = BF_RD(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_0);
    QuerySetData.MCAST_DEFAULT_MASK_1 = BF_RD(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_1);
    QuerySetData.MCAST_DEFAULT_MASK_2 = BF_RD(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_2);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetMCASTDEFAULTMASK(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_MCASTDEFAULTMASK QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        BF_WR(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_0, QuerySetData.MCAST_DEFAULT_MASK_0);
        BF_WR(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_1, QuerySetData.MCAST_DEFAULT_MASK_1);
        BF_WR(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_2, QuerySetData.MCAST_DEFAULT_MASK_2);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}


void InitMCASTDEFAULTMASK(void)
{
    BF_WR(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_0, 1); // multicast to all 3 Output Port
    BF_WR(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_1, 1); // multicast to all 3 Output Port
    BF_WR(ENET_SWI_MCAST_DEFAULT_MASK, MCAST_DEFAULT_MASK_2, 1); // multicast to all 3 Output Port
}


void QueryBCASTDEFAULTMASK(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_BCASTDEFAULTMASK QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.BCAST_DEFAULT_MASK_0 = BF_RD(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_0);
    QuerySetData.BCAST_DEFAULT_MASK_1 = BF_RD(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_1);
    QuerySetData.BCAST_DEFAULT_MASK_2 = BF_RD(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_2);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetBCASTDEFAULTMASK(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_BCASTDEFAULTMASK QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        BF_WR(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_0, QuerySetData.BCAST_DEFAULT_MASK_0);
        BF_WR(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_1, QuerySetData.BCAST_DEFAULT_MASK_1);
        BF_WR(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_2, QuerySetData.BCAST_DEFAULT_MASK_2);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}


void InitBCASTDEFAULTMASK(void)
{
    BF_WR(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_0, 1); // multicast to all 3 Output Port
    BF_WR(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_1, 1); // multicast to all 3 Output Port
    BF_WR(ENET_SWI_BCAST_DEFAULT_MASK, BCAST_DEFAULT_MASK_2, 1); // multicast to all 3 Output Port
}

void QueryMIRRORCONTROL(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_MIRRORCONTROL QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.PORTx = BF_RD(ENET_SWI_MIRROR_CONTROL, PORTx);
    QuerySetData.MIRROR_ENABLE = BF_RD(ENET_SWI_MIRROR_CONTROL, MIRROR_ENABLE);
    QuerySetData.ING_MAP_ENABLE = BF_RD(ENET_SWI_MIRROR_CONTROL, ING_MAP_ENABLE);
    QuerySetData.EG_MAP_ENABLE = BF_RD(ENET_SWI_MIRROR_CONTROL, EG_MAP_ENABLE);
    QuerySetData.ING_SA_MATCH = BF_RD(ENET_SWI_MIRROR_CONTROL, ING_SA_MATCH);
    QuerySetData.ING_DA_MATCH = BF_RD(ENET_SWI_MIRROR_CONTROL, ING_DA_MATCH);
    QuerySetData.EG_SA_MATCH = BF_RD(ENET_SWI_MIRROR_CONTROL, EG_SA_MATCH);
    QuerySetData.EG_DA_MATCH = BF_RD(ENET_SWI_MIRROR_CONTROL, EG_DA_MATCH);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetMIRRORCONTROL(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_MIRRORCONTROL QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        BF_WR(ENET_SWI_MIRROR_CONTROL, PORTx, QuerySetData.PORTx);
        BF_WR(ENET_SWI_MIRROR_CONTROL, MIRROR_ENABLE, QuerySetData.MIRROR_ENABLE);
        BF_WR(ENET_SWI_MIRROR_CONTROL, ING_MAP_ENABLE, QuerySetData.ING_MAP_ENABLE);
        BF_WR(ENET_SWI_MIRROR_CONTROL, EG_MAP_ENABLE, QuerySetData.EG_MAP_ENABLE);
        BF_WR(ENET_SWI_MIRROR_CONTROL, ING_SA_MATCH, QuerySetData.ING_SA_MATCH);
        BF_WR(ENET_SWI_MIRROR_CONTROL, ING_DA_MATCH, QuerySetData.ING_DA_MATCH);
        BF_WR(ENET_SWI_MIRROR_CONTROL, EG_SA_MATCH, QuerySetData.EG_SA_MATCH);
        BF_WR(ENET_SWI_MIRROR_CONTROL, EG_DA_MATCH, QuerySetData.EG_DA_MATCH);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}

void InitMIRRORCONTROL(void)
{
    BF_WR(ENET_SWI_MIRROR_CONTROL, PORTx, 0);
    BF_WR(ENET_SWI_MIRROR_CONTROL, MIRROR_ENABLE, 0);
    BF_WR(ENET_SWI_MIRROR_CONTROL, ING_MAP_ENABLE, 0);
    BF_WR(ENET_SWI_MIRROR_CONTROL, EG_MAP_ENABLE, 0);
    BF_WR(ENET_SWI_MIRROR_CONTROL, ING_SA_MATCH, 0);
    BF_WR(ENET_SWI_MIRROR_CONTROL, ING_DA_MATCH, 0);
    BF_WR(ENET_SWI_MIRROR_CONTROL, EG_SA_MATCH, 0);
    BF_WR(ENET_SWI_MIRROR_CONTROL, EG_DA_MATCH, 0);
}


void QueryMGMTCONFIG(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_MGMTCONFIG QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.PORT = BF_RD(ENET_SWI_MGMT_CONFIG, PORT);
    QuerySetData.MESSAGE_TRANSMITTED = BF_RD(ENET_SWI_MGMT_CONFIG, MESSAGE_TRANSMITTED);
    QuerySetData.ENABLE = BF_RD(ENET_SWI_MGMT_CONFIG, ENABLE);
    QuerySetData.DISCARD = BF_RD(ENET_SWI_MGMT_CONFIG, DISCARD);
    QuerySetData.PRIORITY = BF_RD(ENET_SWI_MGMT_CONFIG, PRIORITY);
    QuerySetData.PORTMASK = BF_RD(ENET_SWI_MGMT_CONFIG, PORTMASK);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetMGMTCONFIG(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_MGMTCONFIG QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        BF_WR(ENET_SWI_MGMT_CONFIG, PORT, QuerySetData.PORT);
        BF_WR(ENET_SWI_MGMT_CONFIG, MESSAGE_TRANSMITTED, QuerySetData.MESSAGE_TRANSMITTED);
        BF_WR(ENET_SWI_MGMT_CONFIG, ENABLE, QuerySetData.ENABLE);
        BF_WR(ENET_SWI_MGMT_CONFIG, DISCARD, QuerySetData.DISCARD);
        BF_WR(ENET_SWI_MGMT_CONFIG, PRIORITY, QuerySetData.PRIORITY);
        BF_WR(ENET_SWI_MGMT_CONFIG, PORTMASK, QuerySetData.PORTMASK);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}


void InitMGMTCONFIG(void)
{
    BF_WR(ENET_SWI_MGMT_CONFIG, PORT, 0);
    BF_WR(ENET_SWI_MGMT_CONFIG, MESSAGE_TRANSMITTED, 0);
    BF_WR(ENET_SWI_MGMT_CONFIG, ENABLE, 0);
    BF_WR(ENET_SWI_MGMT_CONFIG, DISCARD, 0);
    BF_WR(ENET_SWI_MGMT_CONFIG, PRIORITY, 0);
    BF_WR(ENET_SWI_MGMT_CONFIG, PORTMASK, 0);
}

void QueryIPPRIORITY(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_IPPRIORITY QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.ADDRESS = BF_RD(ENET_SWI_IP_PRIORITY, ADDRESS);
    QuerySetData.IPV4_SELECT = BF_RD(ENET_SWI_IP_PRIORITY, IPV4_SELECT);
    QuerySetData.PRIORITY_PORT0 = BF_RD(ENET_SWI_IP_PRIORITY, PRIORITY_PORT0);
    QuerySetData.PRIORITY_PORT1 = BF_RD(ENET_SWI_IP_PRIORITY, PRIORITY_PORT1);
    QuerySetData.PRIORITY_PORT2 = BF_RD(ENET_SWI_IP_PRIORITY, PRIORITY_PORT2);
    QuerySetData.READ = BF_RD(ENET_SWI_IP_PRIORITY, READ);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetIPPRIORITY(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_IPPRIORITY QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);

    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        BF_WR(ENET_SWI_IP_PRIORITY, ADDRESS, QuerySetData.ADDRESS);
        BF_WR(ENET_SWI_IP_PRIORITY, IPV4_SELECT, QuerySetData.IPV4_SELECT);
        BF_WR(ENET_SWI_IP_PRIORITY, PRIORITY_PORT0, QuerySetData.PRIORITY_PORT0);
        BF_WR(ENET_SWI_IP_PRIORITY, PRIORITY_PORT1, QuerySetData.PRIORITY_PORT1);
        BF_WR(ENET_SWI_IP_PRIORITY, PRIORITY_PORT2, QuerySetData.PRIORITY_PORT2);
        BF_WR(ENET_SWI_IP_PRIORITY, READ, QuerySetData.READ);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}

void InitIPPRIORITY(void)
{
    BF_WR(ENET_SWI_IP_PRIORITY, ADDRESS, 0);
    BF_WR(ENET_SWI_IP_PRIORITY, IPV4_SELECT, 0);
    BF_WR(ENET_SWI_IP_PRIORITY, PRIORITY_PORT0, 0);
    BF_WR(ENET_SWI_IP_PRIORITY, PRIORITY_PORT1, 0);
    BF_WR(ENET_SWI_IP_PRIORITY, PRIORITY_PORT2, 0);
    BF_WR(ENET_SWI_IP_PRIORITY, READ, 0);
}

void QueryPRIORITYCFG(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_PRIORITYCFG QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;
    NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
    DWORD i;

    i=QuerySetData.PORT;
    QuerySetData.VLAN_EN = BF_RDn(ENET_SWI_PRIORITY_CFG, i, VLAN_EN);
    QuerySetData.IP_EN = BF_RDn(ENET_SWI_PRIORITY_CFG, i, IP_EN);
    QuerySetData.MAC_EN = BF_RDn(ENET_SWI_PRIORITY_CFG, i, MAC_EN);
    QuerySetData.DEFAULT_PRIORITY = BF_RDn(ENET_SWI_PRIORITY_CFG, i, DEFAULT_PRIORITY);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetPRIORITYCFG(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_PRIORITYCFG QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    DWORD i;

    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        i=QuerySetData.PORT;
        BF_WRn(ENET_SWI_PRIORITY_CFG, i, VLAN_EN, QuerySetData.VLAN_EN);
        BF_WRn(ENET_SWI_PRIORITY_CFG, i, IP_EN, QuerySetData.IP_EN);
        BF_WRn(ENET_SWI_PRIORITY_CFG, i, MAC_EN, QuerySetData.MAC_EN);
        BF_WRn(ENET_SWI_PRIORITY_CFG, i, DEFAULT_PRIORITY, QuerySetData.DEFAULT_PRIORITY);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}

void InitPRIORITYCFG(DWORD i)
{
    BF_WRn(ENET_SWI_PRIORITY_CFG, i, VLAN_EN, 0);
    BF_WRn(ENET_SWI_PRIORITY_CFG, i, IP_EN, 0);
    BF_WRn(ENET_SWI_PRIORITY_CFG, i, MAC_EN, 0);
    BF_WRn(ENET_SWI_PRIORITY_CFG, i, DEFAULT_PRIORITY, 0);
}

void QueryVLANPRIORITY(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_VLANPRIORITY QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;
    NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
    DWORD i;

    i=QuerySetData.PORT;
    QuerySetData.P0 = BF_RDn(ENET_SWI_VLAN_PRIORITY, i, P0);
    QuerySetData.P1 = BF_RDn(ENET_SWI_VLAN_PRIORITY, i, P1);
    QuerySetData.P2 = BF_RDn(ENET_SWI_VLAN_PRIORITY, i, P2);
    QuerySetData.P3 = BF_RDn(ENET_SWI_VLAN_PRIORITY, i, P3);
    QuerySetData.P4 = BF_RDn(ENET_SWI_VLAN_PRIORITY, i, P4);
    QuerySetData.P5 = BF_RDn(ENET_SWI_VLAN_PRIORITY, i, P5);
    QuerySetData.P6 = BF_RDn(ENET_SWI_VLAN_PRIORITY, i, P6);
    QuerySetData.P7 = BF_RDn(ENET_SWI_VLAN_PRIORITY, i, P7);
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetVLANPRIORITY(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_VLANPRIORITY QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    DWORD i;


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        i=QuerySetData.PORT;
        BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P0, QuerySetData.P0);
        BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P1, QuerySetData.P1);
        BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P2, QuerySetData.P2);
        BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P3, QuerySetData.P3);
        BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P4, QuerySetData.P4);
        BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P5, QuerySetData.P5);
        BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P6, QuerySetData.P6);
        BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P7, QuerySetData.P7);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}

void InitVLANPRIORITY(DWORD i)
{
    BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P0, 0);
    BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P1, 0);
    BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P2, 0);
    BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P3, 0);
    BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P4, 0);
    BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P5, 0);
    BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P6, 0);
    BF_WRn(ENET_SWI_VLAN_PRIORITY, i, P7, 0);
}

void QueryMIRRORCONFIG(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes)
{
    SWI_MIRRORCONFIG QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);
    InfoBuffer=InfoBuffer;

    QuerySetData.ING_MAP = HW_ENET_SWI_MIRROR_ING_MAP_RD();
    QuerySetData.EG_MAP = HW_ENET_SWI_MIRROR_EG_MAP_RD();
    QuerySetData.ISRC_0 = HW_ENET_SWI_MIRROR_ISRC_0_RD();
    QuerySetData.ISRC_1 = HW_ENET_SWI_MIRROR_ISRC_1_RD();
    QuerySetData.IDST_0 = HW_ENET_SWI_MIRROR_IDST_0_RD();
    QuerySetData.IDST_1 = HW_ENET_SWI_MIRROR_IDST_1_RD();
    QuerySetData.ESRC_0 = HW_ENET_SWI_MIRROR_ESRC_0_RD();
    QuerySetData.ESRC_1 = HW_ENET_SWI_MIRROR_ESRC_1_RD();
    QuerySetData.EDST_0 = HW_ENET_SWI_MIRROR_EDST_0_RD();
    QuerySetData.EDST_1 = HW_ENET_SWI_MIRROR_EDST_1_RD();
    QuerySetData.CNT = HW_ENET_SWI_MIRROR_CNT_RD();
    memcpy(MoveSource, &QuerySetData, QuerySetDataLength);
    *MoveBytes = QuerySetDataLength; // size of query return in InformationBuffer
}

void SetMIRRORCONFIG(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn)
{
    SWI_MIRRORCONFIG QuerySetData;
    DWORD QuerySetDataLength=sizeof(QuerySetData);


    if (QuerySetDataLength > *pBytesLeft)
    {
        *BytesNeeded = QuerySetDataLength ;
        *BytesRead = 0;
        *pStatusToReturn = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        // Set result
        NdisMoveMemory(&QuerySetData, InfoBuffer, QuerySetDataLength);
        *BytesRead = QuerySetDataLength; // size of set read from InformationBuffer
        *BytesNeeded = 0;
        HW_ENET_SWI_MIRROR_ING_MAP_WR(QuerySetData.ING_MAP);
        HW_ENET_SWI_MIRROR_EG_MAP_WR(QuerySetData.EG_MAP);
        HW_ENET_SWI_MIRROR_ISRC_0_WR(QuerySetData.ISRC_0);
        HW_ENET_SWI_MIRROR_ISRC_1_WR(QuerySetData.ISRC_1);
        HW_ENET_SWI_MIRROR_IDST_0_WR(QuerySetData.IDST_0);
        HW_ENET_SWI_MIRROR_IDST_1_WR(QuerySetData.IDST_1);
        HW_ENET_SWI_MIRROR_ESRC_0_WR(QuerySetData.ESRC_0);
        HW_ENET_SWI_MIRROR_ESRC_1_WR(QuerySetData.ESRC_1);
        HW_ENET_SWI_MIRROR_EDST_0_WR(QuerySetData.EDST_0);
        HW_ENET_SWI_MIRROR_EDST_1_WR(QuerySetData.EDST_1);
        HW_ENET_SWI_MIRROR_CNT_WR(QuerySetData.CNT);
        *pStatusToReturn = NDIS_STATUS_SUCCESS; 
    }
}

void InitMIRRORCONFIG(void)
{
    HW_ENET_SWI_MIRROR_ING_MAP_WR(0);
    HW_ENET_SWI_MIRROR_EG_MAP_WR(0);
    HW_ENET_SWI_MIRROR_ISRC_0_WR(0);
    HW_ENET_SWI_MIRROR_ISRC_1_WR(0);
    HW_ENET_SWI_MIRROR_IDST_0_WR(0);
    HW_ENET_SWI_MIRROR_IDST_1_WR(0);
    HW_ENET_SWI_MIRROR_ESRC_0_WR(0);
    HW_ENET_SWI_MIRROR_ESRC_1_WR(0);
    HW_ENET_SWI_MIRROR_EDST_0_WR(0);
    HW_ENET_SWI_MIRROR_EDST_1_WR(0);
    HW_ENET_SWI_MIRROR_CNT_WR(0);
}

//------------------------------------------------------------------------------
// Main Functions for SWITCH feature Init
//------------------------------------------------------------------------------
void SwitchInit(void)
{
    UINT32 i, n;
    DWORD OQMGRStatus;
// REVISION
    RETAILMSG (0, (TEXT("SwitchInit REVISION 0x%x\r\n"), HW_ENET_SWI_REVISION_RD())); // 0x10121
// SCRATCH
    RETAILMSG (0, (TEXT("SCRATCH 0x%x\r\n"), HW_ENET_SWI_SCRATCH_RD()));
    HW_ENET_SWI_SCRATCH_WR(0xf);
    RETAILMSG (0, (TEXT("SCRATCH 0x%x\r\n"), HW_ENET_SWI_SCRATCH_RD())); // 0xfffffff0
// LOOKUP_MEMORY
    for(i=0; i<2048; i++)
    {
        n=2*i;
        HW_ENET_SWI_LOOKUP_MEMORY_WR(n, 0x0);
        n=2*i+1;
        HW_ENET_SWI_LOOKUP_MEMORY_WR(n, 0x0);
    }
// Reset / Enable
    BF_WR(ENET_SWI_MODE_CONFIG, STATSRESET, 1); // Reset Statistics
    BF_WR(ENET_SWI_MODE_CONFIG, SWITCH_EN, 1); // Enable
    while((OQMGRStatus = HW_ENET_SWI_OQMGR_STATUS_RD())&0x1) Sleep(0); // 0x6000(0)4a
    BF_WR(ENET_SWI_QMGR_MINCELLS, QMGR_MINCELLS, 9); // set minimum cells threshold to 9
    HW_ENET_SWI_OQMGR_STATUS_WR(0x0); // write to clear // 0x600040
    while((OQMGRStatus = HW_ENET_SWI_OQMGR_STATUS_RD())&0x2) Sleep(0); 
    HW_ENET_SWI_QMGR_ST_MINCELLS_WR(0x0); // write to clear
// Resolution Common
    // broadcast resolution for all frame
    InitBCASTDEFAULTMASK();
    // broadcast resolution for multicast frame    
    InitMCASTDEFAULTMASK();
// Resolution BPDU frame
    // resolution for Bridge Protocol Frames(BPDU frame)
    InitMGMTCONFIG();
// Resolution IP protocal type (UDP/TCP) frame
    // IPSNOOP  
    for(i=0; i<8; i++)
    {
        InitIPSNOOP(i);
    }
// Resolution IP protocal port  (src/dst) frame
    // PORTSNOOP    
    for(i=0; i<8; i++)
    {
        InitPORTSNOOP(i);
    }
// Resolution mirror
// outgress: from switch port to outside // ingress: from outside to swtich port
    // Mirror port2 to port1 // set mirror port and mirror rule
    InitMIRRORCONTROL();
    InitMIRRORCONFIG();
    InitIPPRIORITY();
// VLAN priority
    // map 3 bit VLAN priority for each Input Port
    for(i=0; i<3; i++)
    {
        InitVLANPRIORITY(i);            
    }
    // enable 3 bit VLAN priority map for each Input Port
    for(i=0; i<3; i++)
    {
        InitPRIORITYCFG(i);
    }
// Resolution VLAN frame
    // VLAN Resolution Table
    for(i=0; i<32; i++)
    {
        InitVLANRESTABLE(i);
    }
    InitVLANVERIFY();
// Input port 
    // Enable block/ Disable learning on input port
    InitINPUTLEARNBLOCK();
// Port Enable
    // Enable transmit/receive on port
    // transmit: from switch port to outside // receive: from outside to switch port
    InitPORTENA();
}

//------------------------------------------------------------------------------
// Child Functions for SWITCH Learning
//------------------------------------------------------------------------------

BOOL GetSwitchLR(DWORD* pMACAddress0, DWORD* pMACAddress1, DWORD* pHash, DWORD* pPort)
{   
    LRNREC1 LRNRec1;
    if(BF_RD(ENET_SWI_LRN_STATUS, LRN_STATUS)==1) // Learning record is valid and can be read
    {
        *pMACAddress0 = HW_ENET_SWI_LRN_REC_0_RD();
        // A read to LRN_REC_1 triggers the retrieval of the next record pair from the FIFO if any
        *(PUINT32)(&LRNRec1)=HW_ENET_SWI_LRN_REC_1_RD();
        *pMACAddress1 = LRNRec1.MACAddress;
        *pHash = LRNRec1.Hash;
        *pPort = LRNRec1.Port;
        // RETAILMSG (1, (TEXT("%s Learning record 0x%x 0x%x %d %d\r\n"), __WFUNCTION__, *pMACAddress0, *pMACAddress1, *pHash, *pPort));
        return TRUE;
    }
    // RETAILMSG (1, (TEXT("%s no Learning record\r\n"), __WFUNCTION__));
    return FALSE;
}

void GetSwitchLM(DWORD Index, DWORD* pMACAddress0, DWORD* pMACAddress1, DWORD* pRecordValid, DWORD* pRecordType, DWORD* pTimestamp, DWORD* pPort, DWORD* pPriority, DWORD* pPortMask)
{
    UINT32 n;
    LMENTRY LMEntry;
    n=2*Index;
    *(PUINT32)(&(LMEntry.LMEntry0)) = HW_ENET_SWI_LOOKUP_MEMORY_RD(n);
    n=2*Index+1;
    *(PUINT32)(&(LMEntry.LMEntry1)) = HW_ENET_SWI_LOOKUP_MEMORY_RD(n);

    *pMACAddress0 = LMEntry.LMEntry0.MACAddress;
    *pMACAddress1 = LMEntry.LMEntry1.LMEntry1Dyn.MACAddress;
    *pRecordValid = LMEntry.LMEntry1.LMEntry1Dyn.RecordValid;
    *pRecordType = LMEntry.LMEntry1.LMEntry1Dyn.RecordType; 
    if(*pRecordType==0) // dyn
    {
        *pTimestamp = LMEntry.LMEntry1.LMEntry1Dyn.Timestamp;
        *pPort = LMEntry.LMEntry1.LMEntry1Dyn.Port;
//      RETAILMSG (1, (TEXT("dyn GetSwitchLM %d 0x%x 0x%x %d %d %d %d\r\n"), Index, *pMACAddress0, *pMACAddress1, *pRecordValid, *pRecordType, *pTimestamp, *pPort));
    }
    if(*pRecordType==1) // static
    {
        *pPriority=LMEntry.LMEntry1.LMEntry1Sta.Priority;
        *pPortMask=LMEntry.LMEntry1.LMEntry1Sta.PortMask;
//      RETAILMSG (1, (TEXT("sta GetSwitchLM %d 0x%x 0x%x %d %d %d %d\r\n"), Index, *pMACAddress0, *pMACAddress1, *pRecordValid, *pRecordType, *pPriority, *pPortMask));
    }    
}

void SetSwitchLM(DWORD Index, DWORD MACAddress0, DWORD MACAddress1,DWORD RecordValid, DWORD RecordType, DWORD Timestamp, DWORD Port, DWORD Priority, DWORD PortMask)
{
    UINT32 n;
    LMENTRY LMEntry;
    LMEntry.LMEntry0.MACAddress = (MACAddress0&0xffffffff);
    LMEntry.LMEntry1.LMEntry1Dyn.MACAddress = (MACAddress1&0xffff);
    LMEntry.LMEntry1.LMEntry1Dyn.RecordValid = (RecordValid &0x1);
    LMEntry.LMEntry1.LMEntry1Dyn.RecordType = (RecordType&0x1);
    if(RecordType==0) // dyn
    {
        LMEntry.LMEntry1.LMEntry1Dyn.Timestamp = (Timestamp&0x3ff);
        LMEntry.LMEntry1.LMEntry1Dyn.Port = (Port&0xf);
    }
    if(RecordType==1) // static
    {
        LMEntry.LMEntry1.LMEntry1Sta.Priority = (Priority&0x3);
        LMEntry.LMEntry1.LMEntry1Sta.PortMask = (PortMask&0x7);
    }

    n=2*Index;
    HW_ENET_SWI_LOOKUP_MEMORY_WR(n, *(PUINT32)(&(LMEntry.LMEntry0)));
    n=2*Index+1;
    HW_ENET_SWI_LOOKUP_MEMORY_WR(n, *(PUINT32)(&(LMEntry.LMEntry1)));
}

//------------------------------------------------------------------------------
// Main Functions for SWITCH Learning
//------------------------------------------------------------------------------

BOOL SwitchLearning()
{
    DWORD TimestampMax, IndexMax, DeltaTimestampMax, DeltaTimestamp=0;  
    DWORD MACAddressLR0, MACAddressLR1, HashLR, PortLR, TimestampLR=0;
    DWORD Index, MACAddressLM0, MACAddressLM1, RecordValid, RecordType=0, TimestampLM, PortLM=0, Priority=0, PortMask=0;
    if(GetSwitchLR(&MACAddressLR0, &MACAddressLR1, &HashLR, &PortLR)) // learning record is valid and can be read
    {
        for(Index =HashLR*8;Index < HashLR*8+8;Index++) // find match or empty record
        {
            GetSwitchLM(Index, &MACAddressLM0, &MACAddressLM1, &RecordValid, &RecordType, &TimestampLM, &PortLM, &Priority, &PortMask); // compare
            if(RecordType==0) // The aging and learning functions ignore static records
            {
                GetSystemTime(&SystemTimestamp);
                TimestampLR=SystemTimestamp.wSecond;
                if((MACAddressLM0== MACAddressLR0)&&(MACAddressLM1== MACAddressLR1)) // if match, Update
                {
                    RETAILMSG (0, (TEXT("%s updateLM Index %d MAC 0x%x Port %d time %d\r\n"), __WFUNCTION__, Index, MACAddressLR1, PortLR, TimestampLR));
                
                    SetSwitchLM(Index, MACAddressLR0, MACAddressLR1, 1, RecordType, TimestampLR, PortLR, Priority, PortMask);
                    return TRUE;
                }
                else if(RecordValid == 0) // if empty, add (no hole in LM)
                {
                    RETAILMSG (0, (TEXT("%s addLM Index %d MAC 0x%x Port %d time %d\r\n"), __WFUNCTION__, Index, MACAddressLR1, PortLR, TimestampLR));
                    SetSwitchLM(Index, MACAddressLR0, MACAddressLR1, 1, RecordType, TimestampLR, PortLR, Priority, PortMask);
                    return TRUE;
                }
            }
        } // end of find match or empty record

        TimestampMax= 0;
        IndexMax= 0;
        DeltaTimestampMax=0;
        for(Index =HashLR*8;Index < HashLR*8+8;Index++) // find oldest record
        {
            GetSwitchLM(Index, &MACAddressLM0, &MACAddressLM1, &RecordValid, &RecordType, &TimestampLM, &PortLM, &Priority, &PortMask); // compare
            if(RecordType==0) // The aging and learning functions ignore static records
            {
                if(TimestampLM>=TimestampLR)
                    DeltaTimestamp=TimestampLR+60-TimestampLM;
                else
                    DeltaTimestamp=TimestampLR-TimestampLM;
                if(DeltaTimestamp>DeltaTimestampMax) 
                {
                    TimestampMax= TimestampLM;
                    IndexMax= Index;
                    DeltaTimestampMax=DeltaTimestamp;
                }
            }
        } // end of find oldest record
        if(DeltaTimestampMax!=0)
        {
            RETAILMSG (1, (TEXT("%s replace oldest record\r\n"), __WFUNCTION__));
            SetSwitchLM(IndexMax, MACAddressLR0, MACAddressLR1, 1, RecordType, TimestampLR, PortLR, Priority, PortMask); // replace oldest record
            return TRUE;
        }
        else
        {
            RETAILMSG (1, (TEXT("%s error,  no place to learn the record\r\n"), __WFUNCTION__));
            return FALSE; // error,  no place to learn the record
        }
    }
    return FALSE; // error,  no place to learn the record
} // end of learning

//------------------------------------------------------------------------------
// Child Functions for SWITCH Ageing
//------------------------------------------------------------------------------

void DelSwitchLM(DWORD Index)
{
    DWORD IndexEnd, i;
    DWORD MACAddressLM0, MACAddressLM1, RecordValid, RecordType=0, TimestampLM, PortLM=0, Priority=0, PortMask=0;
    IndexEnd=(Index /8)*8+7;
    if(Index== IndexEnd) // Last record in the block, just delete it
        SetSwitchLM(Index, 0, 0, 0, 0, 0, 0, 0, 0);
    else // shift valid record one by one
    {
        for(i=Index;i< IndexEnd;i++)
        {
            GetSwitchLM((i+1), &MACAddressLM0, &MACAddressLM1, &RecordValid, &RecordType, &TimestampLM, &PortLM, &Priority, &PortMask); // get next
            SetSwitchLM(i, MACAddressLM0, MACAddressLM1, RecordValid, RecordType, TimestampLM, PortLM, Priority, PortMask); // replace current
            if(RecordValid==0) // if empty, then finished (no hole in LM)
                break;
        }
        if(i==IndexEnd)
            SetSwitchLM(i, 0, 0, 0, 0, 0, 0, 0, 0);         
    }
} // end of del

void SwitchAgeingOnTimer()
{
    AgeingInterval=TRUE;
}

//------------------------------------------------------------------------------
// Main Functions for SWITCH Ageing
//------------------------------------------------------------------------------

void SwitchAgeing()
{
    static DWORD Index=0;
    DWORD DeltaTimestampAge=10; // 
    DWORD TimestampAge, DeltaTimestamp;
    DWORD IndexEnd, i;
    DWORD MACAddressLM0, MACAddressLM1, RecordValid, RecordType=0, TimestampLM, PortLM=0, Priority=0, PortMask=0;
    if(AgeingInterval==TRUE) // Can Ageing
    {
        IndexEnd= (Index/8) *8+7;
        for(i= Index;i<= IndexEnd;i++)
        {
            GetSwitchLM(i, &MACAddressLM0, &MACAddressLM1, &RecordValid, &RecordType, &TimestampLM, &PortLM, &Priority, &PortMask);
            if(RecordType==0) // The aging and learning functions ignore static records
            {
                if(RecordValid == 0) // if empty, then finished (no hole in LM)
                    break;
                else // judge
                {               
                    GetSystemTime(&SystemTimestamp);
                    TimestampAge=SystemTimestamp.wSecond;
                    RETAILMSG (0, (TEXT("%s checkLM %d MAC 0x%x port %d time %d now time %d\r\n"), __WFUNCTION__, i, MACAddressLM1, PortLM, TimestampLM, TimestampAge));
                    
                    if(TimestampLM>TimestampAge)
                        DeltaTimestamp=TimestampAge+60-TimestampLM;
                    else
                        DeltaTimestamp=TimestampAge-TimestampLM;

                    if(DeltaTimestamp> DeltaTimestampAge)
                    {
                        RETAILMSG (0, (TEXT("%s delLM %d MAC 0x%x port %d time %d now time %d\r\n"), __WFUNCTION__, i, MACAddressLM1, PortLM, TimestampLM, TimestampAge));
                        DelSwitchLM(i);
                    }
                }
            }
        }
        Index+=8;
        if(Index==2048) // end of LM
        {
            Index=0;
            AgeingInterval=FALSE; // current interval Ageing finished, Ageing need wait next interval
        }
    } // end of Can Ageing
} // end of Ageing

//------------------------------------------------------------------------------
// Thread for SWITCH Learning/Ageing
//------------------------------------------------------------------------------
#define AgeingIntervalTime 60000 // 60s
void SwitchAgeingThread()
{
    BOOL AgeingThread;
    AgeingThread=TRUE;
    while(AgeingThread)
    {
        SwitchAgeingOnTimer();
        Sleep(AgeingIntervalTime);
    }       
}

#define LearningIntervalTime 100 // 100ms
void SwitchLearningThread()
{
    BOOL LearningThread;
    LearningThread=TRUE;
    while(LearningThread)
    {
        SwitchLearning();
        SwitchAgeing();
        Sleep(LearningIntervalTime);
    }        
}

