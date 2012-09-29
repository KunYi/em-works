//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  ENET.h
//
//  Implementation of ENET Driver
//
//  This file implements header file for ENET
//
//------------------------------------------------------------------------------

#ifndef _SRC_DRIVERS_ENET_H
#define _SRC_DRIVERS_ENET_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4214)
#include <ndis.h>
#include <ntddndis.h>  // defines OID's
#include <Linklist.h>
#include <windows.h>
#include <nkintr.h>
#include <Winbase.h>
#pragma warning(pop)
#include "common_macros.h"
#include "common_enet.h"

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

// The Version of NDIS that the ENET driver is compatible with
#define ENET_NDIS_MAJOR_VERSION       4
#define ENET_NDIS_MINOR_VERSION       0

#define OID_GET_XMIT_TIMER   0xEDB81000
#define OID_GET_RCV_TIMER    0xEDB81001
#define OID_ENA_PTP_RXTX     0xEDB81002
#define OID_UPDATE_NEW_TIMER 0xEDB81003


// TODO: we have allocated 16 transmit buffers for sending, but
// now we only support one frame per send
#define MAXIMUM_TRANSMIT_PACKET     4//1

#define MAC0 0
#define MAC1 1

// Hash creation constants.
//
#define CRC_PRIME                    0xFFFFFFFF
#define CRC_POLYNOMIAL               0xEDB88320
#define HASH_BITS                    6


#define ETHER_ADDR_SIZE              6   // Size of Ethernet address
#define ETHER_HDR_SIZE               14  // Size of Ethernet header


#define MCAST_LIST_SIZE             12//   number of multicast addresses supported

// Packet length definitions
#define PKT_MAXBUF_SIZE              1518
#define PKT_MINBUF_SIZE              64
#define PKT_MAXBLR_SIZE              1520
#define PKT_CRC_SIZE                 4

#define PTP_TYPE                     0x88F7

#define  ENET_RX_RING_SIZE       16
#define  ENET_TX_RING_SIZE       16
#define  ENET_ENET_RX_FRSIZE     2048
#define  ENET_ENET_TX_FRSIZE     2048

#define  ENET_MII_COUNT          20
#define  ENET_MII_END            0

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

// ----------------------------------------------------------------------
// TX Enhanced BD Bit Definitions
// ----------------------------------------------------------------------
#define TX_BD_INT       0x40000000 
#define TX_BD_TS        0x20000000 
#define TX_BD_PINS      0x10000000 
#define TX_BD_IINS      0x08000000 
#define TX_BD_TXE       0x00008000 
#define TX_BD_UE        0x00002000 
#define TX_BD_EE        0x00001000 
#define TX_BD_FE        0x00000800 
#define TX_BD_LCE       0x00000400 
#define TX_BD_OE        0x00000200 
#define TX_BD_TSE       0x00000100 

#define TX_BD_BDU       0x80000000  

// ----------------------------------------------------------------------
// RX Enhanced BD Bit Definitions
// ----------------------------------------------------------------------
#define RX_BD_ME               0x80000000    
#define RX_BD_PE               0x04000000    
#define RX_BD_CE               0x02000000    
#define RX_BD_UC               0x01000000    
#define RX_BD_INT              0x00800000    
#define RX_BD_ICE              0x00000020    
#define RX_BD_PCR              0x00000010    
#define RX_BD_VLAN             0x00000004    
#define RX_BD_IPV6             0x00000002    
#define RX_BD_FRAG             0x00000001    

#define RX_BD_BDU              0x80000000    




// EBuffer descriptor control2/status used by Ethernet receive

#define BD_ENET_RX_INT         ((USHORT)0x0080)
#define BD_ENET_RX_UC          ((USHORT)0x0100)
#define BD_ENET_RX_CE          ((USHORT)0x0200)
#define BD_ENET_RX_PE          ((USHORT)0x0400)
#define BD_ENET_RX_ME          ((USHORT)0x8000)
#define BD_ENET_RX_FRAG        ((USHORT)0x10000)
#define BD_ENET_RX_IPV6        ((USHORT)0x20000)
#define BD_ENET_RX_VLAN        ((USHORT)0x40000)
#define BD_ENET_RX_PCR        ((USHORT)0x100000)
#define BD_ENET_RX_ICE        ((USHORT)0x200000)
//#define BD_ENET_RX_STATS      ((USHORT)0x378780)        /* All status bits */

// Buffer descriptor control/status used by Ethernet transmit.

#define BD_ENET_TX_IINS         ((USHORT)0x0800)
#define BD_ENET_TX_PINS         ((USHORT)0x1000)
#define BD_ENET_TX_TS           ((USHORT)0x2000)
#define BD_ENET_TX_INT          ((USHORT)0x4000)
#define BD_ENET_TX_TSE          ((USHORT)0x1000000)
#define BD_ENET_TX_OE           ((USHORT)0x2000000)
#define BD_ENET_TX_LCE          ((USHORT)0x4000000)
#define BD_ENET_TX_FE           ((USHORT)0x8000000)
#define BD_ENET_TX_EE           ((USHORT)0x10000000)
#define BD_ENET_TX_UE           ((USHORT)0x20000000)
#define BD_ENET_TX_TXE          ((USHORT)0x80000000)
//#define BD_ENET_TX_STATS        ((USHORT)0x03ff)        /* All status bits */


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
#define MII_READ_COMMAND(reg)           (ENET_MMFR_ST_VALUE << BP_ENET_MAC_MMFR_ST)|(ENET_MMFR_OP_READ  << BP_ENET_MAC_MMFR_OP)|(ENET_MMFR_TA_VALUE << BP_ENET_MAC_MMFR_TA)|((reg & 0x1f) << BP_ENET_MAC_MMFR_RA)
#define MII_WRITE_COMMAND(reg, val)     (ENET_MMFR_ST_VALUE << BP_ENET_MAC_MMFR_ST)|(ENET_MMFR_OP_WRITE << BP_ENET_MAC_MMFR_OP)|(ENET_MMFR_TA_VALUE << BP_ENET_MAC_MMFR_TA)|((reg & 0x1f) << BP_ENET_MAC_MMFR_RA)|(val & 0xffff)
                                        

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


// statistic counters for the frames which have been received by the ENET 
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

// statistic counters for the frames which have been transmitted by the ENET
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

// the buffer descriptor structure for the ENET Legacy Buffer Descriptor
typedef struct  _BUFFER_DESC
{
    USHORT  DataLen;
    USHORT  ControlStatus;
    ULONG   BufferAddr;
}BUFFER_DESC,  *PBUFFER_DESC;


//the buffer descriptor structure for the ENET Enhanced Buffer Descriptor
typedef struct  _EBUFFER_DESC
{
    USHORT  DataLen;
    USHORT  ControlStatus;
    ULONG   BufferAddr;
    ULONG   Control2Status;
    USHORT  CheckSum;
    USHORT  Protocol;
    USHORT  RESERVED0;
    USHORT  BDU;
    ULONG   Timestamp;
    ULONG   RESERVED1;
    ULONG   RESERVED2;     
}EBUFFER_DESC,  *PEBUFFER_DESC;


// Forward declarations of some structures to support different PHYs
typedef struct _ENET_PHY_CMD
{
    UINT    MIIData;
    void    (*MIIFunct)( UINT RegVal, NDIS_HANDLE  MiniportAdapterHandle );
}ENET_PHY_CMD, *PENET_PHY_CMD;

typedef struct _ENET_PHY_INFO
{
    UINT PhyId;
    WCHAR PhyName[256];
    
    PENET_PHY_CMD PhyConfig;
    PENET_PHY_CMD PhyStartup;
    PENET_PHY_CMD PhyActint;
    PENET_PHY_CMD PhyShutdown;
}ENET_PHY_INFO, *PENET_PHY_INFO;

// This structure contains information about the buffer descriptors, transmit buffer,
// receive buffer and MII management.
typedef  struct  _ENET_ENET_PRIVATE
{
    // virtual address and physical address for BDs
    PVOID               RingBase;
    PHYSICAL_ADDRESS    RingPhysicalBase;
    
    // virtual address for receive and transmit buffers
    PVOID            RxBufferBase;
    PVOID            TxBufferBase;
    
    PEBUFFER_DESC    RxBufferDescBase;
    PEBUFFER_DESC    TxBufferDescBase;
    
    PUCHAR    RxBuffAddr[ENET_RX_RING_SIZE];
    PUCHAR    TxBuffAddr[ENET_RX_RING_SIZE];
    
    PEBUFFER_DESC    CurrentRx;
    PEBUFFER_DESC    CurrentTx,  DirtyTx;
    
    BOOL    TxFull;
    
    ENET_PHY_INFO *MIIPhy;
    UINT    MIIPhyId;
    BOOL    MIIPhyIdDone;
    
    UINT    MIIPhySpeed;
    UINT    MIIPhyAddr;
    UINT    MIIPhyStatus;
    
    BOOL    MIISeqDone;
    
    
    BOOL    LinkStatus;
    BOOL    FullDuplex;
        
    BOOL    fUseRMII;
}ENET_ENET_PRIVATE,  *PENET_ENET_PRIVATE;

// MII management structure
typedef struct _ENET_MII_LIST
{
    UINT    MIIRegVal;
    void    (*MIIFunction)( UINT RegVal, NDIS_HANDLE  MiniportAdapterHandle );
    struct  _ENET_MII_LIST  *MIINext;
}ENET_MII_LIST, *PENET_MII_LIST;




//PTP Data struct
typedef struct 
{
  unsigned int      sec;                                            ///< Timestamp with respect to epoch
  int               nsecs;                                          ///< Time increments
} TimeRepr; 


typedef struct 
{
  unsigned int      sec;                                            ///< Timestamp with respect to epoch
  int               nsecs;                                          ///< Time increments
  DWORD             sequenceId;                                     //For Rx frame sequence ID
  DWORD             messageType;                                    //For Message Type  
} RxTimeRepr; 


 BOOL WINAPI EnetMiiHandleThread (LPVOID lpParam);

 NDIS_STATUS ENETInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext
    );

void ENETDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
void ENETEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
void ENETHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext);

void ENETHalt(IN NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS ENETQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );
    
NDIS_STATUS ENETSetInformation (
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    );
    
NDIS_STATUS ENETSend(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT SendFlags
    );
    
NDIS_STATUS ENETReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    );
    
void ENETShutdown(
    IN PVOID ShutdownContext
    );

VOID
ENETSetPower(
     NDIS_HANDLE MiniportAdapterContext ,
     NDIS_DEVICE_POWER_STATE   PowerState 
    );

BOOLEAN ENETCheckForHang(IN NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS ENETTransferData(
    OUT PNDIS_PACKET  Packet,
    OUT PUINT  BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT  ByteOffset,
    IN UINT  BytesToTransfer
    );

void ENETIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN  NDIS_HANDLE MiniportAdapterContext
    );


void ENETParseMIICr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void ENETParseMIIAnar(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void ENETParseAm79c874Dr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void ENETParseLAN87xxSCSR(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void ENETParseDP83640PHYSTS(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);


void ENETParseMIISr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);

void ENETDispPHYCfg(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void ENETParsePHYLink(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
void ENETGetLinkStatus(IN NDIS_HANDLE MiniportAdapterHandle);

void GetENETPHYId(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
);
void GetENETPHYId2(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
);



//------------------------------------------------------------------------------
//  FUNCTION PROTOTYPES
//------------------------------------------------------------------------------

// NDIS Miniport interfaces

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);


class ENETClass
{
public:
   
    ENETClass(UINT32 index);
    ~ENETClass(void);
    
     NDIS_STATUS ENETInitialize(
        OUT PNDIS_STATUS OpenErrorStatus,
        OUT PUINT SelectedMediumIndex,
        IN PNDIS_MEDIUM MediumArray,
        IN UINT MediumArraySize,
        IN NDIS_HANDLE MiniportAdapterHandle,
        IN NDIS_HANDLE WrapperConfigurationContext
        );

    void ENETDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
    void ENETEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);
    void ENETHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext);

    void ENETHalt(IN NDIS_HANDLE MiniportAdapterContext);

    NDIS_STATUS ENETQueryInformation(
        IN NDIS_HANDLE MiniportAdapterContext,
        IN NDIS_OID Oid,
        IN PVOID InformationBuffer,
        IN ULONG InformationBufferLength,
        OUT PULONG BytesWritten,
        OUT PULONG BytesNeeded
        );
        
    NDIS_STATUS ENETSetInformation (
        IN NDIS_HANDLE MiniportAdapterContext,
        IN NDIS_OID Oid,
        IN PVOID InformationBuffer,
        IN ULONG InformationBufferLength,
        OUT PULONG BytesRead,
        OUT PULONG BytesNeeded
        );
        
    NDIS_STATUS ENETSend(
        IN NDIS_HANDLE MiniportAdapterContext,
        IN PNDIS_PACKET Packet,
        IN UINT SendFlags
        );
        
    NDIS_STATUS ENETReset(
        OUT PBOOLEAN AddressingReset,
        IN NDIS_HANDLE MiniportAdapterContext
        );
        
    void ENETShutdown(
        IN PVOID ShutdownContext
        );

    VOID
    ENETSetPower(
         NDIS_HANDLE MiniportAdapterContext ,
         NDIS_DEVICE_POWER_STATE   PowerState 
        );

    BOOLEAN ENETCheckForHang(IN NDIS_HANDLE MiniportAdapterContext);

    NDIS_STATUS ENETTransferData(
        OUT PNDIS_PACKET  Packet,
        OUT PUINT  BytesTransferred,
        IN NDIS_HANDLE MiniportAdapterContext,
        IN NDIS_HANDLE MiniportReceiveContext,
        IN UINT  ByteOffset,
        IN UINT  BytesToTransfer
        );

    void ENETIsr(
        OUT PBOOLEAN InterruptRecognized,
        OUT PBOOLEAN QueueMiniportHandleInterrupt,
        IN  NDIS_HANDLE MiniportAdapterContext
        );
        
    // ENET hardware related functions
    BOOL ENETStartXmit(IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETSetMII(  IN PENET_ENET_PRIVATE pENET  );

    BOOL ENETQueueMII(
     IN PENET_ENET_PRIVATE pENET,
     IN UINT RegValue,
     IN void (*OpFunc)(UINT, NDIS_HANDLE)
     );

    void ENETDoMIICmd(
    IN PENET_ENET_PRIVATE pENET,
    IN PENET_PHY_CMD pCmd
    );

    BOOL ENETEnetInit(IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETEnetDeinit(IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETEnetReset(IN NDIS_HANDLE MiniportAdapterHandle, IN BOOL DuplexMode);
    void ENETEnetAutoNego(IN NDIS_HANDLE MiniportAdapterHandle);
 
   UCHAR CalculateHashValue(UCHAR *pAddr);
   
   void ENETEnetPowerHandle(IN NDIS_HANDLE MiniportAdapterHandle,BOOL En);
   

    // Interrupt handler
    void ProcessReceiveInterrupts(IN NDIS_HANDLE MiniportAdapterHandle);
    void ProcessTransmitInterrupts(IN NDIS_HANDLE MiniportAdapterHandle);
    void ProcessMIIInterrupts(IN NDIS_HANDLE MiniportAdapterHandle);
    void ProcessPTPTxTimer(IN NDIS_HANDLE MiniportAdapterHandle);
    
    void ENETParseMIICr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETParseMIIAnar(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETParseAm79c874Dr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETParseLAN87xxSCSR(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETParseDP83640PHYSTS(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);


    void ENETParseMIISr(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);

    void ENETDispPHYCfg(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETParsePHYLink(IN UINT RegVal, IN NDIS_HANDLE MiniportAdapterHandle);
    void ENETGetLinkStatus(IN NDIS_HANDLE MiniportAdapterHandle);

    void ENETGetPHYId(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    );
    void ENETGetPHYId2(
        IN UINT MIIReg,
        IN NDIS_HANDLE MiniportAdapterHandle
        );
    // Multicast hash tables related functions
    void ClearAllMultiCast();
    void AddMultiCast( UCHAR *pAddr );  
    
    NDIS_MINIPORT_CHARACTERISTICS ENETChar; // Characteristics table for the ENET driver
    
    //PTP Function
    void InitTimer();
    void UpdateCurrentTimer(void);
    void GetPTPTxTimer(TimeRepr *pTxTimer);
    void SetPTPTimer(TimeRepr *pTimer);
    void GetPTPRxTimer (RxTimeRepr *pRxTimer, ULONG Timestamp, DWORD sequenceId, DWORD messageType);

private:
    CRITICAL_SECTION gENETRegCs;
    CRITICAL_SECTION gENETBufCs;
    CRITICAL_SECTION gENETTimerCs;

    TimeRepr   CurTimer;
    TimeRepr   TxTimer;
    RxTimeRepr RxTimer;  

    ENET_MII_LIST   gMIICmds[ENET_MII_COUNT];
    PENET_MII_LIST  gMIIFree ;
    PENET_MII_LIST  gMIIHead;
    PENET_MII_LIST  gMIITail;
    DWORD index;   //MAC index

};

// ENET configuration register
typedef struct  _ENET_CONFIGURATION
{
    NDIS_HANDLE        ndisAdapterHandle;
    NDIS_MINIPORT_INTERRUPT        interruptObj;
    
    DWORD            intLine;
    
    UCHAR            EnetMacAddress[ETHER_ADDR_SIZE];
    UCHAR            MediaType;
    BOOL             FullDuplexMode;
    BOOL             SpeedMode;
    UCHAR            TxMaxCount;
    
    NDIS_HARDWARE_STATUS         CurrentState;
    NDIS_MEDIA_STATE             MediaState;
    BOOL                         MediaStateChecking;
    
    ULONG            PacketFilter;
    ULONG            CurrentLookAhead;
    
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
    UCHAR           ReceiveBuffer[ENET_ENET_RX_FRSIZE];
    ULONG           ReceivePacketSize;
    
    ENET_ENET_PRIVATE  ENETPrivateInfo;
    
    ENETClass*      pENET;
    
}ENET_t, *pENET_t;


#ifdef __cplusplus
}
#endif


#endif //  _SRC_DRIVERS_ENET_H
