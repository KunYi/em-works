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
//  File:  fec.h
//
//  Implementation of FEC Driver
//
//  This file implements header file for FEC
//
//------------------------------------------------------------------------------

#ifndef FEC_H
#define FEC_H

#include <ndis.h>

//-------------------------------------------------------------------------
// OEM Message Tags
//-------------------------------------------------------------------------
#define stringTag       0xFEFA      // Length Byte After String
#define lStringTag      0xFEFB      // Length Byte Before String
#define zStringTag      0xFEFC      // Zero-Terminated String Tag
#define nStringTag      0xFEFD      // No Length Byte Or 0-Term

//-------------------------------------------------------------------------
// Phy related constants
//-------------------------------------------------------------------------
#define PHY_503                 0
#define PHY_100_A               0x000003E0
#define PHY_100_C               0x035002A8
#define PHY_TX_ID               0x015002A8
#define PHY_NSC_TX              0x5c002000
#define PHY_OTHER               0xFFFF
#define PHY_MODEL_REV_ID_MASK   0xFFF0FFFF
#define PARALLEL_DETECT         0
#define N_WAY                   1
#define RENEGOTIATE_TIME        35
#define CONNECTOR_AUTO          0
#define CONNECTOR_TPE           1
#define CONNECTOR_MII           2

//-------------------------------------------------------------------------
// Ethernet Frame Sizes
//-------------------------------------------------------------------------
#define ETHERNET_ADDRESS_LENGTH         6
#define ETHERNET_HEADER_SIZE            14
#define MINIMUM_ETHERNET_PACKET_SIZE    60
#define MAXIMUM_ETHERNET_PACKET_SIZE    1514

#define MAX_MULTICAST_ADDRESSES         32
#define TCB_BUFFER_SIZE                 0XE0
#define COALESCE_BUFFER_SIZE            2048
#define ETH_MAX_COPY_LENGTH             0x80

// Make receive area 1536 for 16 bit alignment.
#define RCB_BUFFER_SIZE                 1520

//- Area reserved for all Non Transmit command blocks
#define MAX_NON_TX_CB_AREA              512

//-------------------------------------------------------------------------
// Ndis/Adapter driver constants
//-------------------------------------------------------------------------
#define MAX_PHYS_DESC                   16
#define NUM_RMD                         10

//--------------------------------------------------------------------------
//    Equates Added for NDIS 6
//--------------------------------------------------------------------------
#define  NUM_BYTES_PROTOCOL_RESERVED_SECTION    16
#define  MAX_NUM_ALLOCATED_RFDS                 64
#define  MIN_NUM_RFD                            4
#define  MAX_ARRAY_SEND_PACKETS                 8
// limit our receive routine to indicating this many at a time
#define  MAX_ARRAY_RECEIVE_PACKETS              16
#define  MAC_RESERVED_SWRFDPTR                  0
#define  MAX_PACKETS_TO_ADD                     32

//-------------------------------------------------------------------------
//- Miscellaneous Equates
//-------------------------------------------------------------------------
#define CR      0x0D        // Carriage Return
#define LF      0x0A        // Line Feed

#ifndef FALSE
#define FALSE       0
#define TRUE        1
#endif

#define DRIVER_NULL ((ULONG)0xffffffff)
#define DRIVER_ZERO 0
#define IRQ_FEC 50

//-------------------------------------------------------------------------
// Bit Mask definitions
//-------------------------------------------------------------------------
#define BIT_0       0x0001
#define BIT_1       0x0002
#define BIT_2       0x0004
#define BIT_3       0x0008
#define BIT_4       0x0010
#define BIT_5       0x0020
#define BIT_6       0x0040
#define BIT_7       0x0080
#define BIT_8       0x0100
#define BIT_9       0x0200
#define BIT_10      0x0400
#define BIT_11      0x0800
#define BIT_12      0x1000
#define BIT_13      0x2000
#define BIT_14      0x4000
#define BIT_15      0x8000
#define BIT_24      0x01000000
#define BIT_28      0x10000000

#define BIT_0_2     0x0007
#define BIT_0_3     0x000F
#define BIT_0_4     0x001F
#define BIT_0_5     0x003F
#define BIT_0_6     0x007F
#define BIT_0_7     0x00FF
#define BIT_0_8     0x01FF
#define BIT_0_13    0x3FFF
#define BIT_0_15    0xFFFF
#define BIT_1_2     0x0006
#define BIT_1_3     0x000E
#define BIT_2_5     0x003C
#define BIT_3_4     0x0018
#define BIT_4_5     0x0030
#define BIT_4_6     0x0070
#define BIT_4_7     0x00F0
#define BIT_5_7     0x00E0
#define BIT_5_9     0x03E0
#define BIT_5_12    0x1FE0
#define BIT_5_15    0xFFE0
#define BIT_6_7     0x00c0
#define BIT_7_11    0x0F80
#define BIT_8_10    0x0700
#define BIT_9_13    0x3E00
#define BIT_12_15   0xF000
#define BIT_8_15    0xFF00

#define BIT_16_20   0x001F0000
#define BIT_21_25   0x03E00000
#define BIT_26_27   0x0C000000

// in order to make our custom oids hopefully somewhat unique
// we will use 0xFF (indicating implementation specific OID)
//               A0 (first byte of non zero intel unique identifier)
//               C9 (second byte of non zero intel unique identifier)
//               XX (the custom OID number - providing 255 possible custom oids)
#define OID_CUSTOM_DRIVER_SET       0xFFA0C901
#define OID_CUSTOM_DRIVER_QUERY     0xFFA0C902
#define OID_CUSTOM_ARRAY            0xFFA0C903
#define OID_CUSTOM_STRING           0xFFA0C904
#define OID_CUSTOM_METHOD           0xFFA0C905

#define CMD_BUS_MASTER              BIT_2

#define DEFAULT_TX_FIFO_LIMIT       0x08
#define DEFAULT_RX_FIFO_LIMIT       0x08
#define DEFAULT_UNDERRUN_RETRY      0x01

#define  FEC_MII_COUNT       20
#define  FEC_MII_END         0

#define BD_SC_EMPTY     ((ushort)0x8000)        /* Recieve is empty */
#define BD_SC_READY     ((ushort)0x8000)        /* Transmit is ready */
#define BD_SC_WRAP      ((ushort)0x2000)        /* Last buffer descriptor */
#define BD_SC_INTRPT    ((ushort)0x1000)        /* Interrupt on change */
#define BD_SC_CM        ((ushort)0x0200)        /* Continous mode */
#define BD_SC_ID        ((ushort)0x0100)        /* Rec'd too many idles */
#define BD_SC_P         ((ushort)0x0100)        /* xmt preamble */
#define BD_SC_BR        ((ushort)0x0020)        /* Break received */
#define BD_SC_FR        ((ushort)0x0010)        /* Framing error */
#define BD_SC_PR        ((ushort)0x0008)        /* Parity error */
#define BD_SC_OV        ((ushort)0x0002)        /* Overrun */
#define BD_SC_CD        ((ushort)0x0001)        /* ?? */

// Buffer descriptor control/status used by Ethernet receive
#define BD_ENET_RX_EMPTY        ((USHORT)0x8000)
#define BD_ENET_RX_PAD          ((USHORT)0x4000)
#define BD_ENET_RX_WRAP         ((USHORT)0x2000)
#define BD_ENET_RX_INTR         ((USHORT)0x1000)
#define BD_ENET_RX_LAST         ((USHORT)0x0800)
#define BD_ENET_RX_FIRST        ((USHORT)0x0400)
#define BD_ENET_RX_MISS         ((USHORT)0x0100)
#define BD_ENET_RX_BC           ((USHORT)0x0080)
#define BD_ENET_RX_MC           ((USHORT)0x0040)
#define BD_ENET_RX_LG           ((USHORT)0x0020)
#define BD_ENET_RX_NO           ((USHORT)0x0010)
#define BD_ENET_RX_SH           ((USHORT)0x0008)
#define BD_ENET_RX_CR           ((USHORT)0x0004)
#define BD_ENET_RX_OV           ((USHORT)0x0002)
#define BD_ENET_RX_CL           ((USHORT)0x0001)
#define BD_ENET_RX_STATS        ((USHORT)0x01ff)        /* All status bits */

// Buffer descriptor control/status used by Ethernet transmit.
#define BD_ENET_TX_READY        ((USHORT)0x8000)
#define BD_ENET_TX_PAD          ((USHORT)0x4000)
#define BD_ENET_TX_WRAP         ((USHORT)0x2000)
#define BD_ENET_TX_INTR         ((USHORT)0x1000)
#define BD_ENET_TX_LAST         ((USHORT)0x0800)
#define BD_ENET_TX_TC           ((USHORT)0x0400)
#define BD_ENET_TX_DEF          ((USHORT)0x0200)
#define BD_ENET_TX_HB           ((USHORT)0x0100)
#define BD_ENET_TX_LC           ((USHORT)0x0080)
#define BD_ENET_TX_RL           ((USHORT)0x0040)
#define BD_ENET_TX_RCMASK       ((USHORT)0x003c)
#define BD_ENET_TX_UN           ((USHORT)0x0002)
#define BD_ENET_TX_CSL          ((USHORT)0x0001)
#define BD_ENET_TX_STATS        ((USHORT)0x03ff)        /* All status bits */

// Register definitions for the PHY
#define MII_REG_CR          0  /* Control Register                         */
#define MII_REG_SR          1  /* Status Register                          */
#define MII_REG_PHYIR1      2  /* PHY Identification Register 1            */
#define MII_REG_PHYIR2      3  /* PHY Identification Register 2            */
#define MII_REG_ANAR        4  /* A-N Advertisement Register               */
#define MII_REG_ANLPAR      5  /* A-N Link Partner Ability Register        */
#define MII_REG_ANER        6  /* A-N Expansion Register                   */
#define MII_REG_ANNPTR      7  /* A-N Next Page Transmit Register          */
#define MII_REG_ANLPRNPR    8  /* A-N Link Partner Received Next Page Reg. */

// values for PHY status

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------

// Make MII read/write commands for the external PHY
#define MII_READ_COMMAND(reg)           CSP_BITFVAL(FEC_MMFR_ST, FEC_MMFR_ST_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_OP, FEC_MMFR_OP_READ)|\
                                        CSP_BITFVAL(FEC_MMFR_TA, FEC_MMFR_TA_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_RA, reg & 0x1f)

#define MII_WRITE_COMMAND(reg, val)     CSP_BITFVAL(FEC_MMFR_ST, FEC_MMFR_ST_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_OP, FEC_MMFR_OP_WRITE)|\
                                        CSP_BITFVAL(FEC_MMFR_TA, FEC_MMFR_TA_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_RA, reg & 0x1f)|\
                                        CSP_BITFVAL(FEC_MMFR_DATA, val & 0xffff)

//-------------------------------------------------------------------------
//  Data Structures
//-------------------------------------------------------------------------
// MII management structure
typedef struct _FEC_MII_LIST
{
    UINT    MIIRegVal;
    void    (*MIIFunction)( UINT RegVal, NDIS_HANDLE  MiniportAdapterHandle );
    struct  _FEC_MII_LIST  *MIINext;
}FEC_MII_LIST, *PFEC_MII_LIST;

//-------------------------------------------------------------------------
// Error Counters
//-------------------------------------------------------------------------
typedef struct _ERR_COUNT_STRUC {
    ULONG       XmtGoodFrames;          // Good frames transmitted
    ULONG       XmtMaxCollisions;       // Fatal frames -- had max collisions
    ULONG       XmtLateCollisions;      // Fatal frames -- had a late coll.
    ULONG       XmtUnderruns;           // Transmit underruns (fatal or re-transmit)
    ULONG       XmtLostCRS;             // Frames transmitted without CRS
    ULONG       XmtDeferred;            // Deferred transmits
    ULONG       XmtSingleCollision;     // Transmits that had 1 and only 1 coll.
    ULONG       XmtMultCollisions;      // Transmits that had multiple coll.
    ULONG       XmtTotalCollisions;     // Transmits that had 1+ collisions.
    ULONG       RcvGoodFrames;          // Good frames received
    ULONG       RcvCrcErrors;           // Aligned frames that had a CRC error
    ULONG       RcvAlignmentErrors;     // Receives that had alignment errors
    ULONG       RcvResourceErrors;      // Good frame dropped due to lack of resources
    ULONG       RcvOverrunErrors;       // Overrun errors - bus was busy
    ULONG       RcvCdtErrors;           // Received frames that encountered coll.
    ULONG       RcvShortFrames;         // Received frames that were to short
    ULONG       CommandComplete;        // A005h indicates cmd completion
} ERR_COUNT_STRUC, *PERR_COUNT_STRUC;

//-------------------------------------------------------------------------
// Transmit Buffer Descriptor (TBD)
//-------------------------------------------------------------------------
typedef struct _TBD_STRUC {
    USHORT DataLen;
    USHORT ControlStatus;   //CommandStatus;
    ULONG  BufferAddress;
} TBD_STRUC, *PTBD_STRUC;

//-------------------------------------------------------------------------
// Receive Frame Descriptor (RFD)
//-------------------------------------------------------------------------
typedef struct _RFD_STRUC {

    USHORT  DataLen;    // Data length @@RFD
    USHORT  ControlStatus;
    ULONG   BufferAddress;
} RFD_STRUC, *PRFD_STRUC;

//-------------------------------------------------------------------------
// Receive Buffer Descriptor (RBD)
//-------------------------------------------------------------------------
typedef struct _RBD_STRUC {
    USHORT      RbdActualCount;         // Number Of Bytes Received
    USHORT      RbdFiller;
    ULONG       RbdLinkAddress;         // Link To Next RBD
    ULONG       RbdRcbAddress;          // Receive Buffer Address
    USHORT      RbdSize;                // Receive Buffer Size
    USHORT      RbdFiller1;
} RBD_STRUC, *PRBD_STRUC;

// The Version of NDIS that the FEC driver is compatible with
#define FEC_NDIS_MAJOR_VERSION 6
#define FEC_NDIS_MINOR_VERSION 0

#define NDIS60_MINIPORT     1

#ifdef DEBUG
extern DBGPARAM dpCurSettings;
#endif

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
// TODO: we have allocated 16 transmit buffers for sending, but
// now we only support one frame per send
#define MAXIMUM_TRANSMIT_PACKET    1

// Hash creation constants.
//
#define CRC_PRIME           0xFFFFFFFF
#define CRC_POLYNOMIAL      0xEDB88320
#define HASH_BITS           6

#define ETHER_ADDR_SIZE             6   // Size of Ethernet address
#define ETHER_HDR_SIZE              14  // Size of Ethernet header

#define MCAST_LIST_SIZE             64  // number of multicast addresses supported

// Packet length definitions
#define NIC_MAX_PACKET_SIZE         2048
#define PKT_MAXBUF_SIZE             1518    //Maximum Frame Length (Recomended to be 1518, 1514+CRC)
#define PKT_CRC_SIZE                4
#define PKT_MAXBLR_SIZE             (NIC_MAX_PACKET_SIZE-PKT_CRC_SIZE) //Recv BufferSize
#define FEC_MAX_PACKET_SIZE         (PKT_MAXBUF_SIZE-PKT_CRC_SIZE)

#define  FEC_RX_RING_SIZE    32
#define  FEC_TX_RING_SIZE    32

#define  FEC_MII_COUNT       20
#define  FEC_MII_END         0

#define MAX_RETRY_AUTONEGO    10000
#define SLEEP_RETRY_AUTONEGO  10000

// Register definitions for the PHY
#define MII_REG_CR          0  /* Control Register                         */
#define MII_REG_SR          1  /* Status Register                          */
#define MII_REG_PHYIR1      2  /* PHY Identification Register 1            */
#define MII_REG_PHYIR2      3  /* PHY Identification Register 2            */
#define MII_REG_ANAR        4  /* A-N Advertisement Register               */
#define MII_REG_ANLPAR      5  /* A-N Link Partner Ability Register        */
#define MII_REG_ANER        6  /* A-N Expansion Register                   */
#define MII_REG_ANNPTR      7  /* A-N Next Page Transmit Register          */
#define MII_REG_ANLPRNPR    8  /* A-N Link Partner Received Next Page Reg. */

// values for PHY status
#define PHY_MAX_COUNT   32

#define PHY_CONF_ANE    0x0001  /* 1 auto-negotiation enabled */
#define PHY_CONF_LOOP   0x0002  /* 1 loopback mode enabled */
#define PHY_CONF_SPMASK 0x00f0  /* mask for speed */
#define PHY_CONF_10HDX  0x0010  /* 10 Mbit half duplex supported */
#define PHY_CONF_10FDX  0x0020  /* 10 Mbit full duplex supported */
#define PHY_CONF_100HDX 0x0040  /* 100 Mbit half duplex supported */
#define PHY_CONF_100FDX 0x0080  /* 100 Mbit full duplex supported */

#define PHY_STAT_LINK   0x0100  /* 1 up - 0 down */
#define PHY_STAT_FAULT  0x0200  /* 1 remote fault */
#define PHY_STAT_ANC    0x0400  /* 1 auto-negotiation complete  */
#define PHY_STAT_SPMASK 0xf000  /* mask for speed */
#define PHY_STAT_10HDX  0x1000  /* 10 Mbit half duplex selected */
#define PHY_STAT_10FDX  0x2000  /* 10 Mbit full duplex selected */
#define PHY_STAT_100HDX 0x4000  /* 100 Mbit half duplex selected */
#define PHY_STAT_100FDX 0x8000  /* 100 Mbit full duplex selected */

#define MMI_DATA_MASK             0xFFFF
#define GALR_CLEAR_ALL            0xFFFFFFFF
#define GAUR_CLEAR_ALL            0xFFFFFFFF
#define IALR_CLEAR_ALL            0xFFFFFFFF
#define IAUR_CLEAR_ALL            0xFFFFFFFF

#define SPEED_100MBPS             10
#define SPEED_10MBPS              1
#define SPEED_FACTOR              100000

#define PMIC_FEC_POWER_ENABLE_ADDRESS       0X00000022
#define PMIC_FEC_POWER_ENABLE_REG_DATA      0X00000040
#define PMIC_FEC_POWER_ENABLE_REG_MASK      0X00000040

// Make MII read/write commands for the external PHY
#define MII_READ_COMMAND(reg)           CSP_BITFVAL(FEC_MMFR_ST, FEC_MMFR_ST_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_OP, FEC_MMFR_OP_READ)|\
                                        CSP_BITFVAL(FEC_MMFR_TA, FEC_MMFR_TA_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_RA, reg & 0x1f)

#define MII_WRITE_COMMAND(reg, val)     CSP_BITFVAL(FEC_MMFR_ST, FEC_MMFR_ST_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_OP, FEC_MMFR_OP_WRITE)|\
                                        CSP_BITFVAL(FEC_MMFR_TA, FEC_MMFR_TA_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_RA, reg & 0x1f)|\
                                        CSP_BITFVAL(FEC_MMFR_DATA, val & 0xffff)

//-------------------------------------------------------------------------
// Ethernet Frame Structure
//-------------------------------------------------------------------------
//- Ethernet 6-byte Address
typedef struct _ETH_ADDRESS_STRUC {
    UCHAR       EthNodeAddress[ETHERNET_ADDRESS_LENGTH];
} ETH_ADDRESS_STRUC, *PETH_ADDRESS_STRUC;

//- Ethernet 14-byte Header
typedef struct _ETH_HEADER_STRUC {
    UCHAR       Destination[ETHERNET_ADDRESS_LENGTH];
    UCHAR       Source[ETHERNET_ADDRESS_LENGTH];
    USHORT      TypeLength;
} ETH_HEADER_STRUC, *PETH_HEADER_STRUC;

typedef struct _ETH_RX_BUFFER_STRUC {
    ETH_HEADER_STRUC    RxMacHeader;
    UCHAR               RxBufferData[(NIC_MAX_PACKET_SIZE - sizeof(ETH_HEADER_STRUC))];
} ETH_RX_BUFFER_STRUC, *PETH_RX_BUFFER_STRUC;

typedef struct _ETH_TX_BUFFER_STRUC {
    ETH_HEADER_STRUC    TxMacHeader;
    UCHAR               TxBufferData[(NIC_MAX_PACKET_SIZE - sizeof(ETH_HEADER_STRUC))];
} ETH_TX_BUFFER_STRUC, *PETH_TX_BUFFER_STRUC;

//------------------------------------------------------------------------------
// Data Structures
//------------------------------------------------------------------------------

// statistic counters for the frames which have been received by the FEC
typedef struct  _FRAME_RCV_STATUS
{
    ULONG   FrameRcvGood;
    ULONG   FrameRcvErrors;
    ULONG   FrameRcvExtraDataErrors;
    ULONG   FrameRcvShortDataErrors;
    ULONG   FrameRcvCRCErrors;
    ULONG   FrameRcvOverrunErrors;
    ULONG   FrameRcvAllignmentErrors;
    ULONG   FrameRcvLCErrors;
}FRAME_RCV_STATUS,  *PFRAME_RCV_STATUS;

// statistic counters for the frames which have been transmitted by the FEC
typedef struct  _FRAME_TXD_STATUS
{
    ULONG   FramesXmitGood;
    ULONG   FramesXmitBad;
    ULONG   FramesXmitHBErrors;
    ULONG   FramesXmitCollisionErrors;
    ULONG   FramesXmitAbortedErrors;
    ULONG   FramesXmitUnderrunErrors;
    ULONG   FramsXmitCarrierErrors;
}FRAME_TXD_STATUS,  *PFRAME_TXD_STATUS;

// Forward declarations of some structures to support different PHYs
typedef struct _FEC_PHY_CMD
{
    UINT    MIIData;
    void    (*MIIFunct)( UINT RegVal, NDIS_HANDLE  MiniportAdapterHandle );
}FEC_PHY_CMD, *PFEC_PHY_CMD;

typedef struct _FEC_PHY_INFO
{
    UINT PhyId;
    WCHAR PhyName[256];

    PFEC_PHY_CMD PhyConfig;
    PFEC_PHY_CMD PhyStartup;
    PFEC_PHY_CMD PhyActint;
    PFEC_PHY_CMD PhyShutdown;
}FEC_PHY_INFO, *PFEC_PHY_INFO;

//------------------------------------------------------------------------------
//  FUNCTION PROTOTYPES
//------------------------------------------------------------------------------

// NDIS Miniport interfaces
NDIS_STATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
static NDIS_STATUS FECInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext
    );
void FECDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
void FECEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
void FECHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
void FECHalt(IN NDIS_HANDLE MiniportAdapterContext);
NDIS_STATUS FECQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_OID_REQUEST   NdisRequest
    );
NDIS_STATUS FECSetInformation (
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_OID_REQUEST   NdisRequest
    );
NDIS_STATUS FECSend(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT SendFlags
    );
NDIS_STATUS FECReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    );
void FECShutdown(
    IN PVOID ShutdownContext
    );
BOOLEAN FECCheckForHang(IN NDIS_HANDLE MiniportAdapterContext);
NDIS_STATUS FECTransferData(
    OUT PNDIS_PACKET  Packet,
    OUT PUINT  BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT  ByteOffset,
    IN UINT  BytesToTransfer
    );
void FECIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN  NDIS_HANDLE MiniportAdapterContext
    );
void FECDriverUnload(
    IN PDRIVER_OBJECT  DriverObject
    );
void FECDevicePnPEventNotify(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNET_DEVICE_PNP_EVENT  NetDevicePnPEvent
    );
NDIS_STATUS  FECInitializeEx(
    IN NDIS_HANDLE  MiniportAdapterHandle,
    IN NDIS_HANDLE  MiniportDriverContext,
    IN PNDIS_MINIPORT_INIT_PARAMETERS  MiniportInitParameters
    );
void FECHaltEx(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_HALT_ACTION  HaltAction
    );
void FECShutdownEx(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_SHUTDOWN_ACTION  ShutdownAction
    );
BOOLEAN FECCheckForHangEx(
    IN NDIS_HANDLE  MiniportAdapterContext
    );
NDIS_STATUS FECResetEx(
    IN NDIS_HANDLE  MiniportAdapterContext,
    OUT PBOOLEAN  AddressingReset
    );
NDIS_STATUS FECOidRequest(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_OID_REQUEST  OidRequest
    );
void MiniportCancelOidRequest(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PVOID  RequestId
    );
void FECSendNetBufferLists(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNET_BUFFER_LIST  NetBufferLists,
    IN NDIS_PORT_NUMBER  PortNumber,
    IN ULONG  SendFlags
    );
void FECReturnNetBufferLists(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNET_BUFFER_LIST  NetBufferLists,
    IN ULONG  ReturnFlags
    );
void FECCancelSendNetBufferLists(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PVOID CancelId
    );
BOOLEAN  FECInterrupt(
    IN PVOID  MiniportInterruptContext,
    OUT PBOOLEAN  QueueDefaultInterruptDpc,
    OUT PULONG  TargetProcessors
    );
void FECCancelOidRequest(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN PVOID                    RequestId
    );
NDIS_STATUS FECPause(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_MINIPORT_PAUSE_PARAMETERS  MiniportPauseParameters
    );
NDIS_STATUS FECRestart(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_MINIPORT_RESTART_PARAMETERS  MiniportRestartParameters
    );

 //....................................................Test code.....................................................//

//
// Take advantage of dispatch level
//
#define FEC_ACQUIRE_SPIN_LOCK(_Lock, DispatchLevel)                          \
    {                                                                       \
        if (DispatchLevel)                                                  \
        {                                                                   \
            NdisDprAcquireSpinLock(_Lock);                                  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            NdisAcquireSpinLock(_Lock);                                     \
        }                                                                   \
    }
#define FEC_RELEASE_SPIN_LOCK(_Lock, DispatchLevel)                          \
    {                                                                       \
        if (DispatchLevel)                                                  \
        {                                                                   \
            NdisDprReleaseSpinLock(_Lock);                                  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            NdisReleaseSpinLock(_Lock);                                     \
        }                                                                   \
    }

//--------------------------------------
// Macros specific to miniport adapter structure
//--------------------------------------
#define FEC_TCB_RESOURCES_AVAIABLE(_M) ((_M)->nBusySend < (_M)->NumTcb)
#define FEC_IS_NOT_READY(_M)       ((_M)->Flags & fFEC_ADAPTER_NOT_READY_MASK)

typedef enum _FEC_STATE
{
    FecPaused,
    FecPausing,
    FecRunning
} FEC_STATE, *PFEC_STATE;

//--------------------------------------
// Macros for flag and ref count operations
//--------------------------------------
#define FEC_SET_FLAG(_M, _F)         ((_M)->Flags |= (_F))
#define FEC_CLEAR_FLAG(_M, _F)       ((_M)->Flags &= ~(_F))
#define FEC_CLEAR_FLAGS(_M)          ((_M)->Flags = 0)
#define FEC_TEST_FLAG(_M, _F)        (((_M)->Flags & (_F)) != 0)
#define FEC_TEST_FLAGS(_M, _F)       (((_M)->Flags & (_F)) == (_F))

#define FEC_INC_REF(_A)              NdisInterlockedIncrement(&(_A)->RefCount)
#define FEC_DEC_REF(_A)              NdisInterlockedDecrement(&(_A)->RefCount)
#define FEC_GET_REF(_A)              ((_A)->RefCount)

#define FEC_INC_RCV_REF(_A)          ((_A)->RcvRefCount++)
#define FEC_DEC_RCV_REF(_A)          ((_A)->RcvRefCount--)
#define FEC_GET_RCV_REF(_A)          ((_A)->RcvRefCount)

//--------------------------------------------------------------------------
// Define 4 macros to get some fields in NetBufferList for miniport's own use
//--------------------------------------------------------------------------
#define FEC_GET_NET_BUFFER_LIST_LINK(_NetBufferList)       (&(NET_BUFFER_LIST_NEXT_NBL(_NetBufferList)))
#define FEC_GET_NET_BUFFER_LIST_NEXT_SEND(_NetBufferList)  ((_NetBufferList)->MiniportReserved[0])
#define FEC_GET_NET_BUFFER_LIST_REF_COUNT(_NetBufferList)  ((ULONG)(ULONG_PTR)((_NetBufferList)->MiniportReserved[1]))

#define FEC_GET_NET_BUFFER_PREV(_NetBuffer)      ((_NetBuffer)->MiniportReserved[0])
#define FEC_GET_NET_BUFFER_LIST_FROM_QUEUE_LINK(_pEntry)  \
    (CONTAINING_RECORD((_pEntry), NET_BUFFER_LIST, Next))

#define ETH_IS_LOCALLY_ADMINISTERED(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x02))

#define InitializeQueueHeader(QueueHeader)                 \
    {                                                      \
        (QueueHeader)->Head = (QueueHeader)->Tail = NULL;  \
    }

#define IsQueueEmpty(QueueHeader) ((QueueHeader)->Head == NULL)
#define GetHeadQueue(QueueHeader) ((QueueHeader)->Head)  //Get the head of the queue

#define RemoveHeadQueue(QueueHeader)                  \
    (QueueHeader)->Head;                              \
    {                                                 \
        PQUEUE_ENTRY pNext;                           \
        ASSERT((QueueHeader)->Head);                  \
        pNext = (QueueHeader)->Head->Next;            \
        (QueueHeader)->Head = pNext;                  \
        if (pNext == NULL)                            \
            (QueueHeader)->Tail = NULL;               \
    }

#define InsertHeadQueue(QueueHeader, QueueEntry)                \
    {                                                           \
        ((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
        (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
        if ((QueueHeader)->Tail == NULL)                        \
            (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
    }

#define InsertTailQueue(QueueHeader, QueueEntry)                     \
    {                                                                \
        ((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                     \
        if ((QueueHeader)->Tail)                                     \
            (QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);  \
        else                                                         \
            (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);        \
        (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);            \
    }

extern struct _MP_ADAPTER;
typedef struct _MP_ADAPTER MP_ADAPTER,*PMP_ADAPTER;

// FEC hardware related functions
BOOL FECEnetInit(IN PMP_ADAPTER Adapter);
void FECEnetDeinit();
void FECEnetReset(IN NDIS_HANDLE MiniportAdapterHandle, IN BOOL DuplexMode);
BOOL FECEnetAutoNego(IN PMP_ADAPTER Adapter);

void FECParseMIICr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseMIIAnar(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseAm79c874Dr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseMIISr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseLAN8700SR2(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseLAN8700Isr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);

void FECDispPHYCfg(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParsePHYLink(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECUpdateLinkStatus(IN NDIS_HANDLE MiniportAdapterHandle);

// Multicast hash tables related functions
void ClearAllMultiCast();
void AddMultiCast( UCHAR *pAddr );
void SetUnicast(PMP_ADAPTER Adapter);
void SetPromiscous();
void ClearPromiscous();

void FECFreeQueuedSendNetBufferLists(IN  PMP_ADAPTER  Adapter);
void FECIndicateLinkState(IN  PMP_ADAPTER  Adapter);

void  FECSetMII(IN  PMP_ADAPTER Adapter);
void  FECResetMII(IN  PMP_ADAPTER Adapter);

#define FEC_GET_ADAPTER_HANDLE(_A) (_A)->ndisAdapterHandle

#define FEC_INIT_NDIS_STATUS_INDICATION(_pStatusIndication, _M, _St, _Buf, _BufSize)        \
    {                                                                                      \
        NdisZeroMemory(_pStatusIndication, sizeof(NDIS_STATUS_INDICATION));                \
        (_pStatusIndication)->Header.Type = NDIS_OBJECT_TYPE_STATUS_INDICATION;            \
        (_pStatusIndication)->Header.Revision = NDIS_STATUS_INDICATION_REVISION_1;         \
        (_pStatusIndication)->Header.Size = sizeof(NDIS_STATUS_INDICATION);                \
        (_pStatusIndication)->SourceHandle = _M;                                           \
        (_pStatusIndication)->StatusCode = _St;                                            \
        (_pStatusIndication)->StatusBuffer = _Buf;                                         \
        (_pStatusIndication)->StatusBufferSize = _BufSize;                                 \
    }
#endif //  FEC_H

