//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

#ifndef _SRC_DRIVERS_FEC_H
#define _SRC_DRIVERS_FEC_H

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4214)
#include <ndis.h>
#include <ntddndis.h>  // defines OID's
#include <Linklist.h>
#include <windows.h>
#include <nkintr.h>
#include <Winbase.h>
#pragma warning(pop)

#include "common_fec.h"
#include "common_macros.h"

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

// The Version of NDIS that the FEC driver is compatible with
#define FEC_NDIS_MAJOR_VERSION       4
#define FEC_NDIS_MINOR_VERSION       0

// TODO: we have allocated 16 transmit buffers for sending, but
// now we only support one frame per send
#define MAXIMUM_TRANSMIT_PACKET     4//1

// Hash creation constants.
//
#define CRC_PRIME                    0xFFFFFFFF
#define CRC_POLYNOMIAL               0xEDB88320
#define HASH_BITS                    6


#define ETHER_ADDR_SIZE              6   // Size of Ethernet address
#define ETHER_HDR_SIZE                 14  // Size of Ethernet header


#define MCAST_LIST_SIZE              64  // number of multicast addresses supported

// Packet length definitions
#define PKT_MAXBUF_SIZE                 1518
#define PKT_MINBUF_SIZE                 64
#define PKT_MAXBLR_SIZE                 1520
#define PKT_CRC_SIZE                 4


#define  FEC_RX_RING_SIZE    16
#define  FEC_TX_RING_SIZE    16
#define     FEC_ENET_RX_FRSIZE     2048
#define     FEC_ENET_TX_FRSIZE     2048

#define  FEC_MII_COUNT       20
#define     FEC_MII_END         0

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
#define BD_ENET_RX_WRAP         ((USHORT)0x2000)
#define BD_ENET_RX_INTR         ((USHORT)0x1000)
#define BD_ENET_RX_LAST         ((USHORT)0x0800)
#define BD_ENET_RX_FIRST        ((USHORT)0x0400)
#define BD_ENET_RX_MISS         ((USHORT)0x0100)
#define BD_ENET_RX_LG           ((USHORT)0x0020)
#define BD_ENET_RX_NO           ((USHORT)0x0010)
#define BD_ENET_RX_SH           ((USHORT)0x0008)
#define BD_ENET_RX_CR           ((USHORT)0x0004)
#define BD_ENET_RX_OV           ((USHORT)0x0002)
#define BD_ENET_RX_CL           ((USHORT)0x0001)
#define BD_ENET_RX_STATS        ((USHORT)0x013f)        /* All status bits */

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

#define PHY_MAX_COUNT   32

#define PHY_CONF_ANE    0x0001  /* 1 auto-negotiation enabled */
#define PHY_CONF_LOOP    0x0002  /* 1 loopback mode enabled */
#define PHY_CONF_SPMASK    0x00f0  /* mask for speed */
#define PHY_CONF_10HDX    0x0010  /* 10 Mbit half duplex supported */
#define PHY_CONF_10FDX    0x0020  /* 10 Mbit full duplex supported */ 
#define PHY_CONF_100HDX    0x0040  /* 100 Mbit half duplex supported */
#define PHY_CONF_100FDX    0x0080  /* 100 Mbit full duplex supported */ 

#define PHY_STAT_LINK    0x0100  /* 1 up - 0 down */
#define PHY_STAT_FAULT    0x0200  /* 1 remote fault */
#define PHY_STAT_ANC    0x0400  /* 1 auto-negotiation complete    */
#define PHY_STAT_SPMASK    0xf000  /* mask for speed */
#define PHY_STAT_10HDX    0x1000  /* 10 Mbit half duplex selected    */
#define PHY_STAT_10FDX    0x2000  /* 10 Mbit full duplex selected    */ 
#define PHY_STAT_100HDX    0x4000  /* 100 Mbit half duplex selected */
#define PHY_STAT_100FDX    0x8000  /* 100 Mbit full duplex selected */

// debug zone definitions
#ifdef DEBUG

#define ZONE_INIT                   TRUE
#define ZONE_DEINIT                 TRUE

#define ZONE_INFO                   FALSE
#define ZONE_FUNCTION               FALSE
#define ZONE_WARN                   FALSE
#define ZONE_ERROR                  TRUE

#endif


//------------------------------------------------------------------------------
// Micros
//------------------------------------------------------------------------------ 


// Make MII read/write commands for the external PHY
#define MII_READ_COMMAND(reg)            CSP_BITFVAL(FEC_MMFR_ST, FEC_MMFR_ST_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_OP, FEC_MMFR_OP_READ)|\
                                        CSP_BITFVAL(FEC_MMFR_TA, FEC_MMFR_TA_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_RA, reg & 0x1f)
                                
#define MII_WRITE_COMMAND(reg, val)        CSP_BITFVAL(FEC_MMFR_ST, FEC_MMFR_ST_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_OP, FEC_MMFR_OP_WRITE)|\
                                        CSP_BITFVAL(FEC_MMFR_TA, FEC_MMFR_TA_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_RA, reg & 0x1f)|\
                                        CSP_BITFVAL(FEC_MMFR_DATA, val & 0xffff)
                                        
//------------------------------------------------------------------------------
// Data Structures
//------------------------------------------------------------------------------

//
// What we map into the reserved section of a packet.
// Cannot be more than 8 bytes 
//
typedef struct _MINIPORT_RESERVED {
    PNDIS_PACKET Next;    
} MINIPORT_RESERVED, * PMINIPORT_RESERVED;

//
// Retrieve the MINIPORT_RESERVED structure from a packet.
// 
#define RESERVED(Packet) ((PMINIPORT_RESERVED)((Packet)->MiniportReserved))


// statistic counters for the frames which have been received by the FEC 
typedef struct  _FRAME_RCV_STATUS
{
    ULONG    FrameRcvGood;
    ULONG    FrameRcvErrors;
    ULONG    FrameRcvExtraDataErrors;
    ULONG    FrameRcvShortDataErrors;
    ULONG    FrameRcvCRCErrors;
    ULONG    FrameRcvOverrunErrors;
    ULONG    FrameRcvAllignmentErrors;
    ULONG    FrameRcvLCErrors;
}FRAME_RCV_STATUS,  *PFRAME_RCV_STATUS;

// statistic counters for the frames which have been transmitted by the FEC
typedef struct  _FRAME_TXD_STATUS
{
    ULONG    FramesXmitGood;
    ULONG    FramesXmitBad;
    ULONG    FramesXmitHBErrors;
    ULONG    FramesXmitCollisionErrors;
    ULONG    FramesXmitAbortedErrors;
    ULONG    FramesXmitUnderrunErrors;
    ULONG    FramsXmitCarrierErrors;
}FRAME_TXD_STATUS,  *PFRAME_TXD_STATUS;

// the buffer descriptor structure for the FEC
typedef struct  _BUFFER_DESC
{
    USHORT  DataLen;
    USHORT  ControlStatus;
    ULONG   BufferAddr;
}BUFFER_DESC,  *PBUFFER_DESC;

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

// This structure contains information about the buffer descriptors, transmit buffer,
// receive buffer and MII management.
typedef  struct  _FEC_ENET_PRIVATE
{
    // virtual address and physical address for BDs
    PVOID               RingBase;
    PHYSICAL_ADDRESS    RingPhysicalBase;
    
    // virtual address for receive and transmit buffers
    PVOID               RxBufferBase;
    PVOID               TxBufferBase;
    
    PBUFFER_DESC    RxBufferDescBase;
    PBUFFER_DESC    TxBufferDescBase;
    
    PUCHAR    RxBuffAddr[FEC_RX_RING_SIZE];
    PUCHAR    TxBuffAddr[FEC_RX_RING_SIZE];
    
    PBUFFER_DESC    CurrentRx;
    PBUFFER_DESC    CurrentTx,  DirtyTx;
    
    BOOL    TxFull;
    
    FEC_PHY_INFO *MIIPhy;
    UINT    MIIPhyId;
    BOOL    MIIPhyIdDone;
    
    UINT    MIIPhySpeed;
    UINT    MIIPhyAddr;
    UINT    MIIPhyStatus;
    
    BOOL    MIISeqDone;
    
    
    BOOL    LinkStatus;
    BOOL    FullDuplex;
        
    BOOL    fUseRMII;
}FEC_ENET_PRIVATE,  *PFEC_ENET_PRIVATE;

// MII management structure
typedef struct _FEC_MII_LIST
{
    UINT    MIIRegVal;
    void    (*MIIFunction)( UINT RegVal, NDIS_HANDLE  MiniportAdapterHandle );
    struct  _FEC_MII_LIST  *MIINext;
}FEC_MII_LIST, *PFEC_MII_LIST;


// FEC configuration register
typedef struct  _FEC_CONFIGURATION
{
    NDIS_HANDLE        ndisAdapterHandle;
    NDIS_MINIPORT_INTERRUPT        interruptObj;
    
    DWORD            intLine;
    
    UCHAR            FecMacAddress[ETHER_ADDR_SIZE];
    UCHAR            MediaType;
    BOOL            FullDuplexMode;
    BOOL            SpeedMode;
    UCHAR            TxMaxCount;
    
    NDIS_HARDWARE_STATUS        CurrentState;
    NDIS_MEDIA_STATE             MediaState;
    BOOL                        MediaStateChecking;
    
    ULONG            PacketFilter;
    ULONG           CurrentLookAhead;
    
    FRAME_RCV_STATUS            RcvStatus;
    FRAME_TXD_STATUS            TxdStatus;
    
    // multicast addresses list
    UCHAR            McastList[MCAST_LIST_SIZE][ETHER_ADDR_SIZE];
    ULONG            NumMulticastAddressesInUse;
    
    PNDIS_PACKET    HeadPacket;
    PNDIS_PACKET    TailPacket;
    
    BOOL            TransmitInProgress;
    BOOL            StartTx;
    
    BOOL            ReceiveCompleteNotifyFlag;
    UCHAR            ReceiveBuffer[FEC_ENET_RX_FRSIZE];
    ULONG            ReceivePacketSize;
    
    FEC_ENET_PRIVATE               FECPrivateInfo;
    
}FEC_t, *pFEC_t;

//------------------------------------------------------------------------------
//  FUNCTION PROTOTYPES
//------------------------------------------------------------------------------

// NDIS Miniport interfaces

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);

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
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );
    
NDIS_STATUS FECSetInformation (
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
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
    
// FEC hardware related functions
BOOL FECStartXmit(pFEC_t pEthernet);

BOOL FECEnetInit(pFEC_t pEthernet);
void FECEnetDeinit(pFEC_t pEthernet);
void FECEnetReset(IN NDIS_HANDLE MiniportAdapterHandle, IN BOOL DuplexMode);
void FECEnetAutoNego(pFEC_t pEthernet);

void FECParseMIICr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseMIIAnar(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseAm79c874Dr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseLAN8700SCSR(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParseDP83640PHYSTS(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
// CS&ZHL JUN-2-2011: supporting DM9161A
void FECParseDM9161State(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);

void FECParseMIISr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);

void FECDispPHYCfg(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECParsePHYLink(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void FECGetLinkStatus(IN NDIS_HANDLE MiniportAdapterHandle);

// Interrupt handler
void ProcessReceiveInterrupts(IN pFEC_t pEthernet);
void ProcessTransmitInterrupts(IN pFEC_t pEthernet);
void ProcessMIIInterrupts(IN pFEC_t pEthernet);

// Multicast hash tables related functions
void ClearAllMultiCast();
void AddMultiCast( UCHAR *pAddr );

#endif //  _SRC_DRIVERS_FEC_H
