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
/*++

Module Name:
    mp_def.h

Abstract:
    NIC specific definitions

Revision History:

Notes:

--*/

#ifndef _MP_DEF_H
#define _MP_DEF_H

// memory tag for this driver   
#define NIC_TAG                         ((ULONG)'1CEF')
#define NIC_DBG_STRING                  ("**FEC**") 

// packet and header sizes
#define NIC_MAX_PACKET_SIZE             2048
#define NIC_MIN_PACKET_SIZE             60
#define NIC_HEADER_SIZE                 14
    
// multicast list size                          
#define NIC_MAX_MCAST_LIST              32

// update the driver version number every time you release a new driver
// The high word is the major version. The low word is the minor version.
// let's start with 6.0 for NDIS 6.0 driver
#define NIC_MAJOR_DRIVER_VERSION        0x06
#define NIC_MINOR_DRIVER_VERISON        0x00

// update the driver version number every time you release a new driver
// The high word is the major version. The low word is the minor version.
// this should be the same as the version reported in miniport driver characteristics

#define NIC_VENDOR_DRIVER_VERSION       ((NIC_MAJOR_DRIVER_VERSION << 16) | NIC_MINOR_DRIVER_VERISON)

// NDIS version in use by the NIC driver. 
// The high byte is the major version. The low byte is the minor version. 
#define NIC_DRIVER_VERSION              0x0600


// media type, we use ethernet, change if necessary
#define NIC_MEDIA_TYPE                  NdisMedium802_3


//
// maximum link speed for send dna recv in bps
//
#define NIC_MEDIA_MAX_SPEED             100000000

// interface type
#define NIC_INTERFACE_TYPE              NdisInterfaceInternal
#define NIC_INTERRUPT_MODE              NdisInterruptLevelSensitive 

// buffer size passed in NdisMQueryAdapterResources                            
// We should only need three adapter resources (IO, interrupt and memory),
// Some devices get extra resources, so have room for 10 resources 
#define NIC_RESOURCE_BUF_SIZE           (sizeof(NDIS_RESOURCE_LIST) + \
                                        (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))


// hardware TCB (Transmit Control Block) structure
typedef TBD_STRUC                      HW_TBD; 
typedef PTBD_STRUC                     PHW_TBD;

// hardware RFD (Receive Frame Descriptor) structure                         
typedef RFD_STRUC                       HW_RFD;  
typedef PRFD_STRUC                      PHW_RFD;               

// change to your company name instead of using Microsoft
#define NIC_VENDOR_DESC                 "Microsoft"

// max number of physical fragments supported per TCB
#define NIC_MAX_PHYS_BUF_COUNT          1     

// minimum number of RFD
#define NIC_MIN_RFDS                    1 

// local data buffer size (to copy send packet data into a local buffer)
#define NIC_BUFFER_SIZE                 2048

// max lookahead size
#define NIC_MAX_LOOKAHEAD               (FEC_MAX_PACKET_SIZE - NIC_HEADER_SIZE)

// max number of send packets the MiniportSendPackets function can accept                            
#define NIC_MAX_SEND_PACKETS            10

// supported filters
#define NIC_SUPPORTED_FILTERS (     \
    NDIS_PACKET_TYPE_DIRECTED       | \
    NDIS_PACKET_TYPE_MULTICAST      | \
    NDIS_PACKET_TYPE_BROADCAST      | \
    NDIS_PACKET_TYPE_PROMISCUOUS)
// Threshold for a remove 
#define NIC_HARDWARE_ERROR_THRESHOLD    8

// The CheckForHang intervals before we decide the send is stuck
#define NIC_SEND_HANG_THRESHOLD         16


// NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION
#define ERRLOG_INVALID_SPEED_DUPLEX     0x00000301L
#define ERRLOG_SET_SECONDARY_FAILED     0x00000302L

// NDIS_ERROR_CODE_OUT_OF_RESOURCES
#define ERRLOG_OUT_OF_MEMORY            0x00000401L
#define ERRLOG_OUT_OF_SHARED_MEMORY     0x00000402L
#define ERRLOG_OUT_OF_BUFFER_POOL       0x00000404L
#define ERRLOG_OUT_OF_NDIS_BUFFER       0x00000405L
#define ERRLOG_OUT_OF_PACKET_POOL       0x00000406L
#define ERRLOG_OUT_OF_NDIS_PACKET       0x00000407L
#define ERRLOG_OUT_OF_LOOKASIDE_MEMORY  0x00000408L
#define ERRLOG_OUT_OF_SG_RESOURCES      0x00000409L

// NDIS_ERROR_CODE_HARDWARE_FAILURE
#define ERRLOG_SELFTEST_FAILED          0x00000501L
#define ERRLOG_INITIALIZE_ADAPTER       0x00000502L
#define ERRLOG_REMOVE_MINIPORT          0x00000503L

// NDIS_ERROR_CODE_RESOURCE_CONFLICT
#define ERRLOG_MAP_IO_SPACE             0x00000601L
#define ERRLOG_QUERY_ADAPTER_RESOURCES  0x00000602L
#define ERRLOG_NO_IO_RESOURCE           0x00000603L
#define ERRLOG_NO_INTERRUPT_RESOURCE    0x00000604L
#define ERRLOG_NO_MEMORY_RESOURCE       0x00000605L

// NIC specific macros                                        
#define NIC_RFD_GET_STATUS(_HwRfd) ((_HwRfd)->ControlStatus)
#define NIC_RFD_STATUS_COMPLETED(_Status) (((_Status) & ~BD_ENET_RX_EMPTY) & 0x1000)
#define NIC_RFD_STATUS_SUCCESS(_Status) (~(_Status) & (BD_ENET_RX_LG | BD_ENET_RX_NO | BD_ENET_RX_SH \
                                            | BD_ENET_RX_CR | BD_ENET_RX_OV))
#define NIC_RFD_GET_PACKET_SIZE(_HwRfd) ((_HwRfd)->DataLen)
#define NIC_RFD_VALID_ACTUALCOUNT(_HwRfd) ((_HwRfd)->DataLen)

// Constants for various purposes of NdisStallExecution

#define NIC_DELAY_POST_RESET            20
// Wait 5 milliseconds for the self-test to complete
#define NIC_DELAY_POST_SELF_TEST_MS     5

                                      
// delay used for link detection to minimize the init time
// change this value to match your hardware 

#define NIC_LINK_DETECTION_DELAY        100




#endif  // _MP_DEF_H


