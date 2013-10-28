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
#ifndef __BOOT_TRANSPORT_EDBG_NOTIFY_H
#define __BOOT_TRANSPORT_EDBG_NOTIFY_H

#include <bootTransportNotify.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

enum BootNotifyTransportEdbg_e {
    BOOT_NOTIFY_TRANSPORT_EDBG_DHCP_DISCOVER = BOOT_TRANSPORT_NOTIFY(0x8001),
    BOOT_NOTIFY_TRANSPORT_EDBG_DHCP_BOUND = BOOT_TRANSPORT_NOTIFY(0x8002),
    BOOT_NOTIFY_TRANSPORT_EDBG_ARP_REQUEST = BOOT_TRANSPORT_NOTIFY(0x8003),
    BOOT_NOTIFY_TRANSPORT_EDBG_ARP_RESPONSE = BOOT_TRANSPORT_NOTIFY(0x8004),
    BOOT_NOTIFY_TRANSPORT_EDBG_BOOTME = BOOT_TRANSPORT_NOTIFY(0x8005),
    BOOT_NOTIFY_TRANSPORT_EDBG_SERVER_ACK = BOOT_TRANSPORT_NOTIFY(0x8006),
    BOOT_NOTIFY_TRANSPORT_EDBG_SERVER_DATA = BOOT_TRANSPORT_NOTIFY(0x8007)
};

//------------------------------------------------------------------------------

typedef struct BootNotifyTransportEdbgDhcpDiscover_t {
    uint32_t attempt;
} BootNotifyTransportEdbgDhcpDiscover_t;

typedef struct BootNotifyTransportEdbgDhcpBound_t {
    uint32_t clientIp;
    uint32_t serverIp;
} BootNotifyTransportEdbgDhcpBound_t;

typedef struct BootNotifyTransportEdbgArpRequest_t {
    uint32_t ip;
} BootNotifyTransportEdbgArpRequest_t;

typedef struct BootNotifyTransportEdbgArpResponse_t {
    bool_t timeout;
    uint32_t ip;
    uint16_t mac[3];
} BootNotifyTransportEdbgArpResponse_t;

typedef struct BootNotifyTransportEdbgBootMe_t {
    cstring_t name;
    uint32_t attempt;
} BootNotifyTransportEdbgBootMe_t;

typedef struct BootNotifyTransportEdbgServerAck_t {
    uint32_t serverIp;
    uint16_t serverPort;
} BootNotifyTransportEdbgServerAck_t;

typedef struct BootNotifyTransportEdbgServerData_t {
    size_t data;
} BootNotifyTransportEdbgServerData_t;

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_TRANSPORT_EDBG_NOTIFY_H

