//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

#ifndef _UFNCLIENT_H_
#define _UFNCLIENT_H_

#include <usbfntypes.h>
#include <usbfnioctl.h>
#include <bldver.h>

typedef struct UFN_CLIENT_DATA {
    DWORD           dwVersion;

    UFN_HANDLE      hDevice;
    UFN_FUNCTIONS   ufnFunctions;
    HINSTANCE       hiParent;
} *PUFN_CLIENT_DATA;

#define UFN_CLIENT_INTERFACE_VERSION_MAJOR   CE_MAJOR_VER
#define UFN_CLIENT_INTERFACE_VERSION_MINOR   CE_MINOR_VER
#define UFN_CLIENT_INTERFACE_VERSION \
    MAKELONG(UFN_CLIENT_INTERFACE_VERSION_MINOR, \
        UFN_CLIENT_INTERFACE_VERSION_MAJOR)

#define IOCTL_UFN_GET_CLIENT_DATA                   _UFN_ACCESS_CTL_CODE(10)
#define IOCTL_UFN_GET_CLIENT_DATA_EX                _UFN_ACCESS_CTL_CODE(11)


#endif // _UFNCLIENT_H_

