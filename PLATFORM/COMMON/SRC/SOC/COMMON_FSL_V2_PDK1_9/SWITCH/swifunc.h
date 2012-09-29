//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  swifunc.h
//
//  Implementation of SWITCH Driver
//
//  This file implements header file for SWITCH
//
//------------------------------------------------------------------------------

#ifndef _SRC_DRIVERS_SWIFUNC_H
#define _SRC_DRIVERS_SWIFUNC_H

#ifdef __cplusplus
extern "C" {
#endif

// Variable

typedef    struct
{
    UINT32  MACAddress:32;
} LMENTRY0;
typedef    struct
{
    UINT32  MACAddress:16;
    UINT32  RecordValid:1;
    UINT32  RecordType:1;
    UINT32  Timestamp:10;
    UINT32  Port:4;
} LMENTRY1DYN;
typedef    struct
{
    UINT32  MACAddress:16;
    UINT32  RecordValid:1;
    UINT32  RecordType:1;
    UINT32  Priority:3;
    UINT32  PortMask:11;
} LMENTRY1STA;
typedef    union
{
    LMENTRY1DYN  LMEntry1Dyn;
    LMENTRY1STA  LMEntry1Sta;
} LMENTRY1;


typedef    struct
{
    LMENTRY0  LMEntry0;
    LMENTRY1  LMEntry1;
} LMENTRY;

typedef    struct
{
    UINT32  MACAddress:16;
    UINT32  Hash:8;
    UINT32  Port:4;
    UINT32  Reserved:4;
} LRNREC1;


// API
#define OID_QUERY_SWITCH  0xEDB81010
#define OID_SET_SWITCH  0xEDB81011
typedef enum {
    PORTENA = 0,
    INPUTLEARNBLOCK,
    VLANVERIFY,
    VLANRESTABLE,
    IPSNOOP,    
    PORTSNOOP,
    MCASTDEFAULTMASK,
    BCASTDEFAULTMASK,
    MIRRORCONTROL,
    MIRRORCONFIG,
    IPPRIORITY,
    PRIORITYCFG,    
    VLANPRIORITY,
    MGMTCONFIG
}SWITCH_SETTING;

////
// s programapp -PORTENA -query
// s programapp -PORTENA -set -ENA_TRANSMIT_0 0 -ENA_TRANSMIT_1 0 -ENA_TRANSMIT_2 0 -ENA_RECEIVE_0 0 -ENA_RECEIVE_1 0 -ENA_RECEIVE_2 0

typedef struct
{
    UINT32 ID;
    UINT32 ENA_TRANSMIT_0;
    UINT32 ENA_TRANSMIT_1;
    UINT32 ENA_TRANSMIT_2;
    UINT32 ENA_RECEIVE_0;
    UINT32 ENA_RECEIVE_1;
    UINT32 ENA_RECEIVE_2;
} SWI_PORTENA, *PSWI_PORTENA;
void QueryPORTENA(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetPORTENA(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -INPUTLEARNBLOCK -query
// s programapp -INPUTLEARNBLOCK -set -BLOCKING_ENA_P0 0 -BLOCKING_ENA_P1 0 -BLOCKING_ENA_P2 0 -LEARNING_DIS_P0 0 -LEARNING_DIS_P1 0 -LEARNING_DIS_P2 0

typedef struct
{
    UINT32 ID;
    UINT32 BLOCKING_ENA_P0;
    UINT32 BLOCKING_ENA_P1;
    UINT32 BLOCKING_ENA_P2;
    UINT32 LEARNING_DIS_P0;
    UINT32 LEARNING_DIS_P1;
    UINT32 LEARNING_DIS_P2;
} SWI_INPUTLEARNBLOCK, *PSWI_INPUTLEARNBLOCK;
void QueryINPUTLEARNBLOCK(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetINPUTLEARNBLOCK(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -VLANVERIFY -query
// s programapp -VLANVERIFY -set -VLAN_VERIFY_0 0 -VLAN_VERIFY_1 0 -VLAN_VERIFY_2 0 -DISCARD_P0 0 -DISCARD_P1 0 -DISCARD_P2 0

typedef struct
{
    UINT32 ID;
    UINT32 VLAN_VERIFY_0;
    UINT32 VLAN_VERIFY_1;
    UINT32 VLAN_VERIFY_2;
    UINT32 DISCARD_P0;
    UINT32 DISCARD_P1;
    UINT32 DISCARD_P2;
} SWI_VLANVERIFY, *PSWI_VLANVERIFY;
void QueryVLANVERIFY(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetVLANVERIFY(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -VLANRESTABLE -query -ENTRY 0
// s programapp -VLANRESTABLE -set -ENTRY 0 -PORT_0 0 -PORT_1 0 -PORT_2 0 -VLAN_ID 0

typedef struct
{
    UINT32 ID;
    UINT32 ENTRY;
    UINT32 PORT_0;
    UINT32 PORT_1;
    UINT32 PORT_2;
    UINT32 VLAN_ID;
} SWI_VLANRESTABLE, *PSWI_VLANRESTABLE;
void QueryVLANRESTABLE(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetVLANRESTABLE(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -PORTSNOOP -query -ENTRY 0
// s programapp -PORTSNOOP -set -ENTRY 0 -ENABLE 0 -MODE 0 -COMPARE_DEST 0 -COMPARE_SOURCE 0 -DESTINATION_PORT 0

typedef struct
{
    UINT32 ID;
    UINT32 ENTRY;
    UINT32 ENABLE;
    UINT32 MODE;
    UINT32 COMPARE_DEST;
    UINT32 COMPARE_SOURCE;
    UINT32 DESTINATION_PORT;
} SWI_PORTSNOOP, *PSWI_PORTSNOOP;
void QueryPORTSNOOP(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetPORTSNOOP(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -IPSNOOP -query -ENTRY 0
// s programapp -IPSNOOP -set -ENTRY 0 -ENABLE 0 -MODE 0 -PROTOCOL 0

typedef struct
{
    UINT32 ID;
    UINT32 ENTRY;
    UINT32 ENABLE;
    UINT32 MODE;
    UINT32 PROTOCOL;
} SWI_IPSNOOP, *PSWI_IPSNOOP;
void QueryIPSNOOP(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetIPSNOOP(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -MCASTDEFAULTMASK -query
// s programapp -MCASTDEFAULTMASK -set -MCAST_DEFAULT_MASK_0 0 -MCAST_DEFAULT_MASK_1 0 -MCAST_DEFAULT_MASK_2 0

typedef struct
{
    UINT32 ID;
    UINT32 MCAST_DEFAULT_MASK_0;
    UINT32 MCAST_DEFAULT_MASK_1;
    UINT32 MCAST_DEFAULT_MASK_2;
} SWI_MCASTDEFAULTMASK, *PSWI_MCASTDEFAULTMASK;
void QueryMCASTDEFAULTMASK(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetMCASTDEFAULTMASK(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -BCASTDEFAULTMASK -query
// s programapp -BCASTDEFAULTMASK -set -BCAST_DEFAULT_MASK_0 0 -BCAST_DEFAULT_MASK_1 0 -BCAST_DEFAULT_MASK_2 0

typedef struct
{
    UINT32 ID;
    UINT32 BCAST_DEFAULT_MASK_0;
    UINT32 BCAST_DEFAULT_MASK_1;
    UINT32 BCAST_DEFAULT_MASK_2;
} SWI_BCASTDEFAULTMASK, *PSWI_BCASTDEFAULTMASK;
void QueryBCASTDEFAULTMASK(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetBCASTDEFAULTMASK(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);


////
// s programapp -IPPRIORITY -query
// s programapp -IPPRIORITY -set -ADDRESS 0 -IPV4_SELECT 0 -PRIORITY_PORT0 0 -PRIORITY_PORT1 0 -PRIORITY_PORT2 0 -READ 0

typedef struct
{
    UINT32 ID;
    UINT32 ADDRESS;
    UINT32 IPV4_SELECT;
    UINT32 PRIORITY_PORT0;
    UINT32 PRIORITY_PORT1;
    UINT32 PRIORITY_PORT2;
    UINT32 READ;
} SWI_IPPRIORITY, *PSWI_IPPRIORITY;
void QueryIPPRIORITY(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetIPPRIORITY(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -PRIORITYCFG -query -PORT 0
// s programapp -PRIORITYCFG -set -PORT 0 -VLAN_EN 0 -IP_EN 0 -MAC_EN 0 -DEFAULT_PRIORITY 0

typedef struct
{
    UINT32 ID;
    UINT32 PORT;
    UINT32 VLAN_EN;
    UINT32 IP_EN;
    UINT32 MAC_EN;
    UINT32 DEFAULT_PRIORITY;
} SWI_PRIORITYCFG, *PSWI_PRIORITYCFG;
void QueryPRIORITYCFG(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetPRIORITYCFG(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -MIRRORCONTROL -query
// s programapp -MIRRORCONTROL -set -PORTx 0 -MIRROR_ENABLE 0 -ING_MAP_ENABLE 0 -EG_MAP_ENABLE 0 -ING_SA_MATCH 0 -ING_DA_MATCH 0 -EG_SA_MATCH 0 -EG_DA_MATCH 0

typedef struct
{
    UINT32 ID;
    UINT32 PORTx; // 0,1,2
    UINT32 MIRROR_ENABLE;
    UINT32 ING_MAP_ENABLE;
    UINT32 EG_MAP_ENABLE;
    UINT32 ING_SA_MATCH;
    UINT32 ING_DA_MATCH;
    UINT32 EG_SA_MATCH;
    UINT32 EG_DA_MATCH;
} SWI_MIRRORCONTROL, *PSWI_MIRRORCONTROL;
void QueryMIRRORCONTROL(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetMIRRORCONTROL(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -MIRRORCONFIG -query
// s programapp -MIRRORCONFIG -set -ING_MAP 0 -EG_MAP 0 -ISRC_0 0 -ISRC_1 0 -IDST_0 0 -IDST_1 0 -ESRC_0 0 -ESRC_1 0 -EDST_0 0 -EDST_1 0 -CNT 0

typedef struct
{
    UINT32 ID;
    UINT32 ING_MAP;
    UINT32 EG_MAP;
    UINT32 ISRC_0;
    UINT32 ISRC_1;
    UINT32 IDST_0;
    UINT32 IDST_1;
    UINT32 ESRC_0;
    UINT32 ESRC_1;
    UINT32 EDST_0;
    UINT32 EDST_1;
    UINT32 CNT;
} SWI_MIRRORCONFIG, *PSWI_MIRRORCONFIG;
void QueryMIRRORCONFIG(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetMIRRORCONFIG(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -VLANPRIORITY -query -PORT 0
// s programapp -VLANPRIORITY -set -PORT 0 -P0 0 -P1 0 -P2 0 -P3 0 -P4 0 -P5 0 -P6 0 -P7 0

typedef struct
{
    UINT32 ID;
    UINT32 PORT;
    UINT32 P0      :  3;
    UINT32 P1      :  3;
    UINT32 P2      :  3;
    UINT32 P3      :  3;
    UINT32 P4      :  3;
    UINT32 P5      :  3;
    UINT32 P6      :  3;
    UINT32 P7      :  3;
} SWI_VLANPRIORITY, *PSWI_VLANPRIORITY;
void QueryVLANPRIORITY(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetVLANPRIORITY(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

////
// s programapp -MGMTCONFIG -query
// s programapp -MGMTCONFIG -set -PORT 0 -MESSAGE_TRANSMITTED 0 -ENABLE 0 -DISCARD 0 -PRIORITY 0 -PORTMASK 0
typedef struct
{
    UINT32 ID;
    UINT32 PORT                 :  4;
    UINT32 MESSAGE_TRANSMITTED  :  1;
    UINT32 ENABLE               :  1;
    UINT32 DISCARD              :  1;
    UINT32 PRIORITY             :  3;
    UINT32 PORTMASK             :  3;
} SWI_MGMTCONFIG, *PSWI_MGMTCONFIG;
void QueryMGMTCONFIG(PUCHAR InfoBuffer, PVOID MoveSource, UINT *MoveBytes);
void SetMGMTCONFIG(PUCHAR InfoBuffer, UINT *pBytesLeft, PULONG BytesNeeded, PULONG BytesRead, NDIS_STATUS *pStatusToReturn);

// Function
void SwitchInit(void);
void SwitchLearningThread(void);
void SwitchAgeingThread(void);


#ifdef __cplusplus
}
#endif

#endif //  _SRC_DRIVERS_SWIFUNC_H
