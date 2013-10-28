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
//  File:  oal_kitl.h
//
//  This header file defines KITL OAL module. This module implements debug
//  communication over KITL protocol.
//
#ifndef __OAL_KITL_H
#define __OAL_KITL_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <kitl.h>

//------------------------------------------------------------------------------
//
//  Define:  OAL_KITL_ID_SIZE
//
//  This value defines maximal length of KITL device name.
//
#define OAL_KITL_ID_SIZE                16

//------------------------------------------------------------------------------
//
//  Define:  OAL_KITL_WINDOW_SIZE
//
//  This value is deprecated, do not use.
//
// #define OAL_KITL_WINDOW_SIZE            8

//------------------------------------------------------------------------------
//
//  Define:  OAL_KITL_BUFFER_SIZE
//
//  This value defines buffer size used by KITL protocol implementation. 
//  For ethernet protocol this buffer is used as network card driver
//  DMA buffer.
//
#define OAL_KITL_BUFFER_SIZE            0x10000

//------------------------------------------------------------------------------
//
//  Enum:  OAL_KITL_TYPE
//
//  This enumeration specifies boot/KITL protocol type. 
//
typedef enum {
    OAL_KITL_TYPE_NONE = 0,
    OAL_KITL_TYPE_SERIAL,               // Serial
    OAL_KITL_TYPE_ETH,                  // UDP/IP4/Ethernet
    OAL_KITL_TYPE_FLASH                 // Flash memory (boot only)
} OAL_KITL_TYPE;

//------------------------------------------------------------------------------
//
//  Enum:  OAL_KITL_FLAGS
//
//  This enumeration specifies optional flags passed as part of OAL_KITL_ARGS
//  structure.
//  
typedef enum {
    OAL_KITL_FLAGS_ENABLED  = 0x0001,   // Kitl enable
    OAL_KITL_FLAGS_PASSIVE  = 0x0002,   // Kitl passive mode
    OAL_KITL_FLAGS_DHCP     = 0x0004,   // DHCP enable
    OAL_KITL_FLAGS_VMINI    = 0x0008,   // VMINI enable
    OAL_KITL_FLAGS_POLL     = 0x0010,   // Use polling (no interrupt)
    OAL_KITL_FLAGS_EXTNAME  = 0x0020    // Extend device name
} OAL_KITL_FLAGS;

//------------------------------------------------------------------------------
//
//  Type:  OAL_KITL_ARGS
//
//  This type is used to pass parameters to OALKitlInit function. When
//  another KITL transport protocol is added this structure can change by 
//  adding new structure in union.
//
#pragma warning(push)
#pragma warning(disable:4201)
typedef struct {
    UINT32 flags;
    DEVICE_LOCATION devLoc;             // KITL device location
    union {
        struct {                        // Serial class parameters
            UINT32 baudRate;
            UINT32 dataBits;
            UINT32 stopBits;
            UINT32 parity;
        };         
        struct {                        // Ether class parameters
            UINT16 mac[3];
            UINT32 ipAddress;
            UINT32 ipMask;
            UINT32 ipRoute;
        };
    };
} OAL_KITL_ARGS;
#pragma warning(pop)
//------------------------------------------------------------------------------
//
//  Type:  KITL_SERIAL_INFO
//
//  This structure is passed to KITL serial driver for initialization. Driver
//  should set bestSize parameter. It is used by KITL framework when it send
//  data to driver. For example, USB serial can set it to 64, and KITL
//  framework will call send/receive with size request of 64 except for the 
//  last packet.
//
typedef struct {
    UINT8* pAddress;                    // Base address
    UINT32 baudRate;                    // Baud rate
    UINT32 dataBits;                    // Data bits
    UINT32 stopBits;                    // Stop bits
    UINT32 parity;                      // Parity
    UINT32 bestSize;                    // Best size to send/receive
} KITL_SERIAL_INFO;

//------------------------------------------------------------------------------
//
//  Interface:  KITL serial driver
//
//  This interface specifies device driver for KITL serial device driver.
//
typedef
BOOL
(*OAL_KITLSERIAL_INIT)(
    __in KITL_SERIAL_INFO *pInfo
    );

typedef
VOID
(*OAL_KITLSERIAL_DEINIT)(
    );

typedef
UINT16 
(*OAL_KITLSERIAL_RECV)(
    __out_bcount(size) UINT8 *pData, 
    UINT16 size
    );

typedef
UINT16
(*OAL_KITLSERIAL_SEND)(
    __in_bcount(size) UINT8 *pData, 
    UINT16 size
    );

typedef
VOID 
(*OAL_KITLSERIAL_SENDCOMPLETE)(
    UINT16 size
    );

typedef
VOID 
(*OAL_KITLSERIAL_ENABLE_INTS)(
    );

typedef
VOID 
(*OAL_KITLSERIAL_DISABLE_INTS)(
    );

typedef
VOID 
(*OAL_KITLSERIAL_POWER_OFF)(
    );

typedef 
VOID 
(*OAL_KITLSERIAL_POWER_ON)(
    );

typedef
VOID
(*OAL_KITLSERIAL_FLOW_CONTROL)(
    BOOL on
    );

typedef struct {
    OAL_KITLSERIAL_INIT pfnInit;
    OAL_KITLSERIAL_DEINIT pfnDeinit;
    OAL_KITLSERIAL_SEND pfnSend;
    OAL_KITLSERIAL_SENDCOMPLETE pfnSendComplete;
    OAL_KITLSERIAL_RECV pfnRecv;
    OAL_KITLSERIAL_ENABLE_INTS pfnEnableInts;
    OAL_KITLSERIAL_DISABLE_INTS pfnDisableInts;
    OAL_KITLSERIAL_POWER_OFF pfnPowerOff;
    OAL_KITLSERIAL_POWER_ON pfnPowerOn;
    OAL_KITLSERIAL_FLOW_CONTROL pfnFlowControl;
} OAL_KITL_SERIAL_DRIVER;

//------------------------------------------------------------------------------
//
//  Interface:  KITL ethernet driver
//
//  This interface specifies device driver for KITL ethernet device driver.
//
typedef
BOOL
(*OAL_KITLETH_INIT)(
    __in_opt UINT8 *pAddress, 
    UINT32 logicalLocation, 
    __inout UINT16 mac[3]
    );

typedef
BOOL
(*OAL_KITLETH_INIT_DMABUFFER)(
    UINT32 address, 
    UINT32 size
    );

typedef 
BOOL
(*OAL_KITLETH_DEINIT)(
    );

typedef
UINT16
(*OAL_KITLETH_SEND_FRAME)(
    __in_bcount(size) UINT8 *pData, 
    UINT32 size
    );

typedef
UINT16
(*OAL_KITLETH_GET_FRAME)(
    __out_bcount(*pSize) UINT8 *pData, 
    __inout UINT16 *pSize
    );

typedef
VOID 
(*OAL_KITLETH_ENABLE_INTS)(
    );

typedef 
VOID
(*OAL_KITLETH_DISABLE_INTS)(
    );

typedef
VOID
(*OAL_KITLETH_POWER_OFF)(
    );

typedef
VOID
(*OAL_KITLETH_POWER_ON)(
    );

typedef
VOID
(*OAL_KITLETH_CURRENT_PACKET_FILTER)(
    UINT32 filter
    );

typedef
BOOL
(*OAL_KITLETH_MULTICAST_LIST)(
    __in_xcount(count) UINT8 *pAddresses, 
    UINT32 count
    );

typedef struct {
    OAL_KITLETH_INIT pfnInit;
    OAL_KITLETH_INIT_DMABUFFER pfnInitDmaBuffer;
    OAL_KITLETH_DEINIT pfnDeinit;
    OAL_KITLETH_SEND_FRAME pfnSendFrame;
    OAL_KITLETH_GET_FRAME pfnGetFrame;
    OAL_KITLETH_ENABLE_INTS pfnEnableInts;
    OAL_KITLETH_DISABLE_INTS pfnDisableInts;
    OAL_KITLETH_POWER_OFF pfnPowerOff;
    OAL_KITLETH_POWER_ON pfnPowerOn;
    OAL_KITLETH_CURRENT_PACKET_FILTER pfnCurrentPacketFilter;
    OAL_KITLETH_MULTICAST_LIST pfnMulticastList;
} OAL_KITL_ETH_DRIVER;

//------------------------------------------------------------------------------
//
//  OAL_KITL_DEVICE
//
//  This structure defines KITL device. First parameter is user readable 
//  device name. Second determine interface type on which device is located.
//  Together with third parameter it is used to identify device.
//
typedef struct {
    LPCWSTR name;           // Device/driver name
    UINT32 ifcType;         // Interface name
    UINT32 id;              // Device identification (depends on interface type)
    UINT32 resource;        // Resource to be used as base address
    OAL_KITL_TYPE type;     // KITL driver type (ethernet, serial, ...)
    VOID *pDriver;          // KITL driver (depends on driver type)
} OAL_KITL_DEVICE;    
    
//------------------------------------------------------------------------------
//
//  Global:  g_oalKitlBuffer
//
//  This global variable is intended to be used by KITL protocol
//  implementation. For ethernet/IP4 it is used as network driver DMA buffer.
//
extern 
UINT8
g_oalKitlBuffer[OAL_KITL_BUFFER_SIZE];

//------------------------------------------------------------------------------
//
//  Function:  OALKitlStart
//
//  This function is called to start KITL module. On image without KITL support
//  it is stubbed. 
//
BOOL
OALKitlStart(
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlInit
//
//  This function is called from OALKitlStart to initialize KITL subsystem.
//
BOOL
OALKitlInit(
    __in_z LPCSTR deviceId, 
    __in OAL_KITL_ARGS *pArgs, 
    __in OAL_KITL_DEVICE *pDevice
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlFindDevice
//
//  This function finds KITL device in device table based on device location.
//  It also updated device location structure members (logical address) 
//  based on device location (bus) and other information.
//
OAL_KITL_DEVICE*
OALKitlFindDevice(
    __in DEVICE_LOCATION *pDevLoc, 
    __in OAL_KITL_DEVICE *pDevices
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlDeviceName
//
//  This function returns device/driver name based on device location and
//  KITL devices table.
//
LPCWSTR
OALKitlDeviceName(
    __in DEVICE_LOCATION *pDevLoc, 
    __in OAL_KITL_DEVICE *pDevices
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlDeviceType
//
//  This function returns KITL device type (ethernet, serial, flash, ...)
//  based on device location and KITL devices table.
//
OAL_KITL_TYPE
OALKitlDeviceType(
    __in DEVICE_LOCATION *pDevLoc, 
    __in OAL_KITL_DEVICE *pDevices
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlInitRegistry
//
//  This function should be called from IOCTL_HAL_INITREGISTRY handler. It is
//  used to intialize registry in way it dissable use of KITL device by OS
//  device driver.
//
VOID
OALKitlInitRegistry(
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlPowerOff
//
//  This function is called by power handler (from OEMPowerOff) before device
//  is moved to suspend. It should set KITL device to power off state.
//
VOID
OALKitlPowerOff(
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlPowerOn
//
//  This function is called by power handler (from OEMPowerOff) after device
//  recovers from suspend. It should restore KITL device to functional state.
//
VOID
OALKitlPowerOn(
    );

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlVBridge
//
//  This is IOCTL handler for VBridge related IOCTL codes. It is called by
//  OEMKitlIoctl to handle the IOCTL_VBRIDGE_802_3_MULTICAST_LIST, 
//  IOCTL_VBRIDGE_ADD_MAC, IOCTL_VBRIDGE_CURRENT_PACKET_FILTER, 
//  IOCTL_VBRIDGE_GET_ETHERNET_MAC, IOCTL_VBRIDGE_GET_RX_PACKET,
//  IOCTL_VBRIDGE_GET_RX_PACKET_COMPLETE, IOCTL_VBRIDGE_GET_TX_PACKET,
//  IOCTL_VBRIDGE_GET_TX_PACKET_COMPLETE, IOCTL_VBRIDGE_SHARED_ETHERNET,
//  IOCTL_VBRIDGE_WILD_CARD, IOCTL_VBRIDGE_WILD_CARD_RESET_BUFFER and 
//  IOCTL_VBRIDGE_WILD_CARD_VB_INITIALIZED codes.
//
BOOL OALIoCtlVBridge(
    UINT32 code, 
    __inout_bcount_opt(inSize) VOID *pInBuffer, 
    UINT32 inSize, 
    __inout_bcount_opt(outSize) VOID *pOutBuffer, 
    UINT32 outSize, 
    __out_opt UINT32 *pOutSize
    );    

//------------------------------------------------------------------------------
//
//  Function:  OALKitlCreateName
//
//  This function create KITL device name from prefix and MAC address. Result
//  is put to szBuffer. The buffer must have OAL_KITL_ID_SIZE size.
//
VOID
OALKitlCreateName(
    __in_z LPSTR szPrefix, 
    __in UINT16 mac[3], 
    __out_bcount(OAL_KITL_ID_SIZE) LPSTR szBuffer
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlGetDevLoc
//
//  This function returns KITL device location. It is used by some KITL
//  optional functions.
//
BOOL
OALKitlGetDevLoc(
    __out DEVICE_LOCATION *pDevLoc
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlGetFlags
//
//  This function allows other modules to obtain KITL flags.
//
BOOL
OALKitlGetFlags(
    __out UINT32 *pFlags
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlIPtoString
//
//  This function converts IP address to string. It is using internal static
//  buffer, so it isn't reentrant.
//
LPWSTR
OALKitlIPtoString(
    UINT32 ip4
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlMACtoString
//
//  This function converts MAC address to string. It is using internal static
//  buffer, so it isn't reentrant.
//
LPWSTR
OALKitlMACtoString(
    __in UINT16 mac[]
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlStringToIP
//
//  This function converts string to IP address. When string is incorrect it
//  returns 0 as result.
//
UINT32
OALKitlStringToIP(
    __in_z LPCWSTR szIP
    );

//------------------------------------------------------------------------------
//
//  Function:  OALKitlStringToMAC
//
//  This function converts string to MAC address. It returns TRUE when
//  conversion was sucessful and FALSE otherwise.
//
BOOL
OALKitlStringToMAC(
    __in_z LPCWSTR szMAC,
    __out UINT16 mac[]
    );

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
