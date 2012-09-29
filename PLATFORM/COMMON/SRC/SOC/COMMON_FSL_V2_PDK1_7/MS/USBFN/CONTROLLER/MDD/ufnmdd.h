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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        USBFNMDD.H

Abstract:

        USB Function Controller Model Device Driver.
        
--*/

#ifndef _USBFNMDD_H_
#define _USBFNMDD_H_

#pragma warning(push)
#pragma warning(disable: 4115 4189 4201 4204 4214 4100 4127 4244 4245 4512 4701 6001 6262 6287 6385 28112)
#include <windows.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include "xferlist.h"
#include "pipe.h"
#include "transfer.h"
#include "ufnbus.h"
#include "openlist.h"
#include "descript.h"
#include "ceotgbus.h"
#pragma warning(pop)

enum DEVICE_STATE {
    DS_DETACHED = 0,
    DS_ATTACHED,
    DS_POWERED,
    DS_DEFAULT,
    DS_ADDRESSED,
    DS_CONFIGURED,
    DS_SUSPENDED,
};


typedef struct UFN_MDD_CONTEXT_SIG {
    DWORD dwSig;
} *PUFN_MDD_CONTEXT_SIG;


typedef struct UFN_MDD_CONTEXT : UFN_MDD_CONTEXT_SIG {
    HKEY hKey;
    HANDLE hParentBusHandle;
    BOOL fPddInitialized;
    CFreeTransferList *pFreeTransferList;
    PCPipeBase *rgpPipes;
    BOOL  fRunning;
    BOOL  fRegistered;
    BOOL  fRemoteWakeupEnabled;
    LPUFN_NOTIFY lpDeviceNotify;
    PVOID pvDeviceNotifyParameter;

    CDescriptors *pDescriptors;

    UFN_BUS_SPEED Speed;

    UFN_TRANSFER hEnumTransfer;

    DEVICE_STATE deviceState;
    DEVICE_STATE deviceStatePriorToSuspend;

    CRITICAL_SECTION csMddAccess;
    CRITICAL_SECTION csBusIoctlAccess;

    COpenList *pOpenList;

    BOOL fClientIsBeingAddedOrRemoved;
    HANDLE hevReadyForNotifications;

#ifdef FSL_MERGE
    BOOL bEnterTestMode;  // device to be placed into test mode at next opportunity
    int  iTestMode;       // which test mode to be entered
#endif

    CUfnBus *pUfnBus;

    UFN_PDD_INTERFACE_INFO PddInfo;
    UCHAR  uOTGCapabilityBits ;

    // Buffers for data to send to the host. Note that the host may
    // only ask for one of these at a time.
    union {
        // rgbStringDesc is accessed as a WCHAR. Force DWORD alignment just in case
        // that is required in the future.
        DWORD dwForceUnionDwordAlignment; 
        BYTE rgbBuffer[2];
        USB_DEVICE_QUALIFIER_DESCRIPTOR DeviceQualifierDescToSend;
        USB_OTG_DESCRIPTOR USBOTGDescToSend;
        BYTE rgbStringDesc[256];
    };
    
} *PUFN_MDD_CONTEXT;


typedef struct UFN_MDD_BUS_OPEN_CONTEXT : UFN_MDD_CONTEXT_SIG {
    PUFN_MDD_CONTEXT pMddContext;
    DWORD dwIndex;
    HKEY  hkClients;
    BOOL  fSetupCalled;
} *PUFN_MDD_BUS_OPEN_CONTEXT;


#define UFN_MDD_SIG         'MnfU' // "UfnM" signature
#define UFN_MDD_BUS_SIG     'BnfU' // "UfnB" signature


// How long to stall a notification before giving up.
#define READY_FOR_NOTIFICATIONS_TIMEOUT 1000


#define RegOpenKey(hkey, lpsz, phk) \
        RegOpenKeyEx((hkey), (lpsz), 0, 0, (phk))



#define PSZ_REG_CLIENT_DRIVER_PATH  _T("\\Drivers\\USB\\FunctionDrivers")
#define PSZ_REG_DEFAULT_DEFAULT_CLIENT_DRIVER   _T("DefaultClientDriver")
//#define TYPE_REG_DEFAULT_CLIENT_DRIVER  REG_SZ
#define PSZ_REG_FRIENDLY_NAME       _T("FriendlyName")

#define PSZ_REG_DEFAULT_CLIENT_KEY  _T("DefaultClientKey")


#define REG_FULL_SPEED              (1 << 1)
#define REG_HIGH_SPEED              (1 << 2)


#define NULL_TERMINATE(sz)          ((sz)[dim(sz) - 1] = 0)


// Default size for transfer lookaside buffers.
#define DEFAULT_TRANSFER_LIST_SIZE 16 


#ifdef DEBUG

VOID
ValidateContext(
    PUFN_MDD_CONTEXT pContext
    );

#else
#define ValidateContext(ptr)
#endif


#define IS_VALID_MDD_CONTEXT(ptr) \
    ( (ptr != NULL) && (ptr->dwSig == UFN_MDD_SIG) )


#define VERIFY_PCONTEXT() \
    if (!IS_VALID_MDD_CONTEXT(pContext)) { \
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid device handle\r\n"), pszFname)); \
        dwRet = ERROR_INVALID_HANDLE; \
        pContext = NULL; \
        goto EXIT; \
    }

PCPipeBase
FindPipe(
    PUFN_MDD_CONTEXT pContext,
    DWORD dwEndpointAddress
    );

BOOL 
WINAPI 
DeviceNotify(
    PVOID pvNotifyParameter,
    DWORD dwMsg,
    DWORD dwParam
    );

DWORD
WINAPI
EnumThread(
    LPVOID pvContext
    );

DWORD
DoDeregisterDevice(
    PUFN_MDD_CONTEXT pContext
    );

DWORD
DoStop(
    PUFN_MDD_CONTEXT pContext
    );

BOOL
SendDeviceNotification(
    LPUFN_NOTIFY lpNotify,
    PVOID        pvNotifyParameter,
    DWORD        dwMsg,
    DWORD        dwParam
    );

// Does the PDD support high speed?
inline
BOOL
PddSupportsHighSpeed(
    PUFN_MDD_CONTEXT pContext
    )
{
    PREFAST_DEBUGCHK(pContext);
    BOOL fRet = ((pContext->PddInfo.dwCapabilities & UFN_PDD_CAPS_SUPPORTS_HIGH_SPEED) != 0);
    return fRet;
}

// Does the PDD support multiple configurations?
inline
BOOL
PddSupportsMultipleConfigurations(
    PUFN_MDD_CONTEXT pContext
    )
{
    PREFAST_DEBUGCHK(pContext);
    BOOL fRet = ((pContext->PddInfo.dwCapabilities & UFN_PDD_CAPS_SUPPORTS_MULTIPLE_CONFIGURATIONS) != 0);
    return fRet;
}

// Does the PDD support alternate interfaces?
inline
BOOL
PddSupportsAlternateInterfaces(
    PUFN_MDD_CONTEXT pContext
    )
{
    PREFAST_DEBUGCHK(pContext);
    BOOL fRet = ((pContext->PddInfo.dwCapabilities & UFN_PDD_CAPS_SUPPORTS_ALTERNATE_INTERFACES) != 0);
    return fRet;
}

// Does the PDD support reusable endpoints?
inline
BOOL
PddSupportsReusableEndpoints(
    PUFN_MDD_CONTEXT pContext
    )
{
    PREFAST_DEBUGCHK(pContext);
    BOOL fRet = ((pContext->PddInfo.dwCapabilities & UFN_PDD_CAPS_REUSABLE_ENDPOINTS) != 0);
    return fRet;
}


#ifdef DEBUG

LPCTSTR
GetDeviceStateName(
    DEVICE_STATE ds
    );

#endif


#ifndef SHIP_BUILD
#define STR_MODULE _T("UsbFnMdd!")
#define SETFNAME() LPCTSTR pszFname = STR_MODULE _T(__FUNCTION__) _T(":")

LPCTSTR
GetSpeedString(
    UFN_BUS_SPEED Speed
    );

#else
#define SETFNAME()
#define GetSpeedString(speed)
#endif


// Change the device state and print out a message.
inline
VOID
ChangeDeviceState(
    PUFN_MDD_CONTEXT pContext,
    DEVICE_STATE ds
    )
{
    SETFNAME();
    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s State change: %s -> %s\r\n"), pszFname,
        GetDeviceStateName(pContext->deviceState), GetDeviceStateName(ds)));
    pContext->deviceState = ds;
}

DWORD
GetClientFunctions(
    PUFN_FUNCTIONS  pUfnFunctions
    );

DWORD
WINAPI
UfnMdd_IssueTransfer(
    UFN_HANDLE                  hDevice,
    UFN_PIPE                    hPipe,
    LPTRANSFER_NOTIFY_ROUTINE   lpNotifyRoutine,
    PVOID                       pvNotifyContext,
    DWORD                       dwFlags,
    DWORD                       cbBuffer, 
    PVOID                       pvBuffer,
    DWORD                       dwBufferPhysicalAddress,
    PVOID                       pvPddTransferInfo,
    PUFN_TRANSFER               phTransfer
    );

DWORD
WINAPI
UfnMdd_GetTransferStatus(
    UFN_HANDLE   hDevice,
    UFN_TRANSFER hTransfer,
    PDWORD       pcbTransferred,
    PDWORD       pdwUsbError
    );

DWORD
WINAPI
UfnMdd_CloseTransfer(
    UFN_HANDLE   hDevice,
    UFN_TRANSFER hTransfer
    );

BOOL 
WINAPI 
UfnMdd_Notify(
    PVOID pvNotifyParameter,
    DWORD dwMsg,
    DWORD dwParam
    );

#endif //_USBFNMDD_H_

