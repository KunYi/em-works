//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  upp.cpp
//
//  This module wrap a stream interface of the Post-processor (PP)
//  driver.  Client App can use the stream interface to
//  configure the PP driver and run test programs.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#include <winddi.h>
#include <pwinbase.h>
#include "marshal.hpp"
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"

#include "IpuBuffer.h"
#include "IPU_base.h"
#include "IPU_common.h"
#include "tpm.h"
#include "pp.h"
#include "upp.h"



//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT       0
#define ZONEID_DEINIT     1
#define ZONEID_IOCTL      2
#define ZONEID_DEVICE     3

#define ZONEID_INFO       12
#define ZONEID_FUNCTION   13
#define ZONEID_WARN       14
#define ZONEID_ERROR      15

// Debug zone masks
#define ZONEMASK_INIT     (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT   (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL    (1<<ZONEID_IOCTL)
#define ZONEMASK_DEVICE   (1<<ZONEID_DEVICE)

#define ZONEMASK_INFO     (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT       DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL        DEBUGZONE(ZONEID_IOCTL)
#define ZONE_DEVICE       DEBUGZONE(ZONEID_DEVICE)

#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#endif

#define UPP_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define UPP_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("PP"),
    {
        _T("Init"), _T("DeInit"), _T("Ioctl"), _T("Device"),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
//    0xffff//ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_FUNCTION | ZONEMASK_INFO | ZONE_INIT | ZONE_DEVICE
    ZONEMASK_ERROR | ZONEMASK_WARN
};
#endif

HANDLE g_hPP = NULL;
//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
DWORD GetPhysAddress( PVOID lpUnMappedBuffer, ULONG ulSize);

//------------------------------------------------------------------------------
//
// Function: UPP_Init
//
// The Device Manager calls this function as a result of a call to the
// ActivateDevice() function.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//------------------------------------------------------------------------------
DWORD UPP_Init(LPCTSTR pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    UPP_FUNCTION_ENTRY();

    UPP_FUNCTION_EXIT();

    // Otherwise return the created instance
    return (DWORD)pContext;
}


//------------------------------------------------------------------------------
//
// Function: UPP_Deinit
//
// This function uninitializes a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL UPP_Deinit(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER(hDeviceContext);

    UPP_FUNCTION_ENTRY();

    UPP_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: UPP_Open
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function creates
//          and returns this handle.
//
//      AccessCode
//          [in] Access code for the device. The access is a combination of
//          read and write access from CreateFile.
//
//      ShareMode
//          [in] File share mode of the device. The share mode is a
//          combination of read and write access sharing from CreateFile.
//
// Returns:
//      This function returns a handle that identifies the open context of
//      the device to the calling application.
//
//------------------------------------------------------------------------------
DWORD UPP_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    UPP_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    g_hPP = CreateFile(TEXT("POP1:"),        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to PP
    if (g_hPP == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(1, (TEXT("%s:  CreateFile PP failed!It is used by other module.\r\n"), __WFUNCTION__));
        return FALSE;        
    }

    UPP_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: UPP_Close
//
// This function closes the device for reading and writing.
//
// Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//          the open context of the device.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL UPP_Close(DWORD hOpenContext)
{
    UPP_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    // if we don't have handle to PP driver
    if (g_hPP != NULL)
    {
        if (!CloseHandle(g_hPP))
        {
            return FALSE;
        }
    }

    UPP_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: UPP_PowerDown
//
// This function suspends power to the device. It is useful only with
// devices that can power down under software control.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void UPP_PowerDown(DWORD hDeviceContext)
{
    UPP_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    UPP_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: UPP_PowerUp
//
// This function restores power to a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void UPP_PowerUp(void)
{
    UPP_FUNCTION_ENTRY();
    UPP_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: UPP_IOControl
//
// This function sends a command to a device.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      dwCode
//          [in] I/O control operation to perform. These codes are
//          device-specific and are usually exposed to developers through
//          a header file.
//
//      pBufIn
//          [in] Pointer to the buffer containing data to transfer to the
//          device.
//
//      dwLenIn
//          [in] Number of bytes of data in the buffer specified for pBufIn.
//
//      pBufOut
//          [out] Pointer to the buffer used to transfer the output data
//          from the device.
//
//      dwLenOut
//          [in] Maximum number of bytes in the buffer specified by pBufOut.
//
//      pdwActualOut
//          [out] Pointer to the DWORD buffer that this function uses to
//          return the actual number of bytes received from the device.
//
// Returns:
//      The new data pointer for the device indicates success. A value of -1
//      indicates failure.
//
//------------------------------------------------------------------------------
BOOL UPP_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    ppConfigData *pConfigData;
    AllocMemInf  *pAllocMemInf;
    UINT8 IntType;
    RECT * pRect;
    BOOL bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pdwActualOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pBufOut);

    switch(dwCode)
    {
        case UPP_IOCTL_CONFIGURE:
            pConfigData = (pPpConfigData)pBufIn;

            // issue the IOCTL to configure the PP
            bRet = DeviceIoControl(g_hPP,   // file handle to the driver
                                  PP_IOCTL_CONFIGURE,       // I/O control code
                                  pConfigData,              // in buffer
                                  sizeof(ppConfigData),    // in buffer size
                                  NULL,                     // out buffer
                                  0,                        // out buffer size
                                  0,                        // number of bytes returned
                                  NULL);                    // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_CONFIGURE ")
                                  TEXT("occurred\r\n")));
            break;

        case UPP_IOCTL_START:
            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                  PP_IOCTL_START,            // I/O control code
                                  NULL,                      // in buffer
                                  0,                         // in buffer size
                                  NULL,                      // out buffer
                                  0,                         // out buffer size
                                  0,                         // number of bytes returned
                                  NULL);                     // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_START ")
                                  TEXT("occurred\r\n")));
            break;

        case UPP_IOCTL_STOP:
            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                  PP_IOCTL_STOP,             // I/O control code
                                  NULL,                      // in buffer
                                  0,                         // in buffer size
                                  NULL,                      // out buffer
                                  0,                         // out buffer size
                                  0,                         // number of bytes returned
                                  NULL);                     // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_STOP ")
                                  TEXT("occurred\r\n")));
            break;

        case UPP_IOCTL_ADD_INPUT_BUFFER:
            pAllocMemInf = (AllocMemInf *) pBufIn;

            if ( dwLenIn < sizeof(AllocMemInf))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_INPUT_BUFFER!\r\n")));
                break;                
            }

            if ( pAllocMemInf->VirtAdd == NULL ||
                 pAllocMemInf->PhysAdd == NULL ||
                 pAllocMemInf->Size    == NULL)
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_INPUT_BUFFER!\r\n")));
                break;                
            }

            if( pAllocMemInf->PhysAdd != (ULONG)GetPhysAddress((PVOID)(pAllocMemInf->VirtAdd),pAllocMemInf->Size))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_INPUT_BUFFER!\r\n")));
                break;
            }

            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                   PP_IOCTL_ADD_INPUT_BUFFER,  // I/O control code
                                   &(pAllocMemInf->PhysAdd),   // in buffer
                                   sizeof(UINT32),             // in buffer size
                                   NULL,                       // out buffer
                                   0,                          // out buffer size
                                   0,                          // number of bytes returned
                                   NULL);                      // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_ADD_INTPUT_BUFFERS ")
                                  TEXT("occurred\r\n")));

            break;
            
        case UPP_IOCTL_ADD_OUTPUT_BUFFER:
            pAllocMemInf = (AllocMemInf *) pBufIn;

            if ( dwLenIn < sizeof(AllocMemInf))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_OUTPUT_BUFFER!\r\n")));
                break;                
            }

            if ( pAllocMemInf->VirtAdd == NULL ||
                 pAllocMemInf->PhysAdd == NULL ||
                 pAllocMemInf->Size    == NULL)
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_OUTPUT_BUFFER!\r\n")));
                break;                
            }

            if( pAllocMemInf->PhysAdd != (ULONG)GetPhysAddress((PVOID)(pAllocMemInf->VirtAdd),pAllocMemInf->Size))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_OUTPUT_BUFFER!\r\n")));
                break;
            }

            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                  PP_IOCTL_ADD_OUTPUT_BUFFER,  // I/O control code
                                  &(pAllocMemInf->PhysAdd),    // in buffer
                                  sizeof(UINT32),              // in buffer size
                                  NULL,                        // out buffer
                                  0,                           // out buffer size
                                  0,                           // number of bytes returned
                                  NULL);                       // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_ADD_OUTTPUT_BUFFERS ")
                                  TEXT("occurred\r\n")));

            break;

        case UPP_IOCTL_ADD_APP_INPUT_BUFFER:
            pAllocMemInf = (AllocMemInf *) pBufIn;

            if ( dwLenIn < sizeof(AllocMemInf))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_INPUT_BUFFER!\r\n")));
                break;                
            }

            if ( pAllocMemInf->VirtAdd == NULL ||
                 pAllocMemInf->Size    == NULL)
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_INPUT_BUFFER!\r\n")));
                break;                
            }

            pAllocMemInf->PhysAdd = (ULONG)GetPhysAddress((PVOID)(pAllocMemInf->VirtAdd),pAllocMemInf->Size);

            if(pAllocMemInf->PhysAdd == NULL )
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_INPUT_BUFFER!\r\n")));
                break;
            }

            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                   PP_IOCTL_ADD_INPUT_BUFFER,  // I/O control code
                                   &(pAllocMemInf->PhysAdd),   // in buffer
                                   sizeof(UINT32),             // in buffer size
                                   NULL,                       // out buffer
                                   0,                          // out buffer size
                                   0,                          // number of bytes returned
                                   NULL);                      // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_ADD_APP_INPUT_BUFFER ")
                                  TEXT("occurred\r\n")));
            break;

        case UPP_IOCTL_ADD_APP_OUTPUT_BUFFER:
            pAllocMemInf = (AllocMemInf *) pBufIn;

            if ( dwLenIn < sizeof(AllocMemInf))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_OUTPUT_BUFFER!\r\n")));
                break;                
            }

            if ( pAllocMemInf->VirtAdd == NULL ||
                 pAllocMemInf->Size    == NULL)
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_OUTPUT_BUFFER!\r\n")));
                break;                
            }

            pAllocMemInf->PhysAdd = (ULONG)GetPhysAddress((PVOID)(pAllocMemInf->VirtAdd),pAllocMemInf->Size);

            if( pAllocMemInf->PhysAdd == NULL)
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_OUTPUT_BUFFER!\r\n")));
                break;
            }

            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                  PP_IOCTL_ADD_OUTPUT_BUFFER,  // I/O control code
                                  &(pAllocMemInf->PhysAdd),    // in buffer
                                  sizeof(UINT32),              // in buffer size
                                  NULL,                        // out buffer
                                  0,                           // out buffer size
                                  0,                           // number of bytes returned
                                  NULL);                       // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_ADD_APP_OUTPUT_BUFFER ")
                                  TEXT("occurred\r\n")));

            break;

        case UPP_IOCTL_CLEAR_BUFFERS:
            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                   PP_IOCTL_CLEAR_BUFFERS,    // I/O control code
                                   NULL,                      // in buffer
                                   0,                         // in buffer size
                                   NULL,                      // out buffer
                                   0,                         // out buffer size
                                   0,                         // number of bytes returned
                                   NULL);                     // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_CLEAR_BUFFERS ")
                                  TEXT("occurred\r\n")));
            break;
            
        case UPP_IOCTL_ENABLE_INTERRUPT:
            IntType = (UINT8)*pBufIn;

            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                  PP_IOCTL_ENABLE_INTERRUPT,     // I/O control code
                                  &IntType,                      // in buffer
                                  sizeof(UINT8),                 // in buffer size
                                  NULL,                          // out buffer
                                  0,                             // out buffer size
                                  0,                             // number of bytes returned
                                  NULL);                         // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_ENABLE_INTERRUPT ")
                                  TEXT("occurred\r\n")));
            break;
            
        case UPP_IOCTL_DISABLE_INTERRUPT:
            IntType = (UINT8)*pBufIn;

            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                  PP_IOCTL_DISABLE_INTERRUPT,    // I/O control code
                                  &IntType,                      // in buffer
                                  sizeof(UINT8),                 // in buffer size
                                  NULL,                          // out buffer
                                  0,                             // out buffer size
                                  0,                             // number of bytes returned
                                  NULL);                         // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_DISABLE_INTERRUPT ")
                                  TEXT("occurred\r\n")));
            break;
            
        case UPP_IOCTL_WAIT_NOT_BUSY:
             IntType = (UINT8)*pBufIn;

             bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                    PP_IOCTL_WAIT_NOT_BUSY,     // I/O control code
                                    &IntType,                   // in buffer
                                    sizeof(UINT8),              // in buffer size
                                    NULL,                       // out buffer
                                    0,                          // out buffer size
                                    0,                          // number of bytes returned
                                    NULL);                      // ignored (=NULL)
             DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_WAIT_NOT_BUSY ")
                                   TEXT("occurred\r\n")));
             break;

        case UPP_IOCTL_ADD_INPUT_COMBBUFFER:
            pAllocMemInf = (AllocMemInf *) pBufIn;

            if ( dwLenIn < sizeof(AllocMemInf))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_INPUT_COMBBUFFER!\r\n")));
                break;                
            }

            if ( pAllocMemInf->VirtAdd == NULL ||
                 pAllocMemInf->PhysAdd == NULL ||
                 pAllocMemInf->Size    == NULL)
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_INPUT_COMBBUFFER!\r\n")));
                break;                
            }

            if( pAllocMemInf->PhysAdd != (ULONG)GetPhysAddress((PVOID)(pAllocMemInf->VirtAdd),pAllocMemInf->Size))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_INPUT_COMBBUFFER!\r\n")));
                break;
            }

            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                   PP_IOCTL_ADD_INPUT_COMBBUFFER,  // I/O control code
                                   &(pAllocMemInf->PhysAdd),       // in buffer
                                   sizeof(UINT32),                 // in buffer size
                                   NULL,                           // out buffer
                                   0,                              // out buffer size
                                   0,                              // number of bytes returned
                                   NULL);                          // ignored (=NULL)

            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_ADD_INPUT_COMBBUFFER ")
                                  TEXT("occurred\r\n")));

            break;  

        case UPP_IOCTL_ADD_APP_INPUT_COMBBUFFER:
            pAllocMemInf = (AllocMemInf *) pBufIn;

            if ( dwLenIn < sizeof(AllocMemInf))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_INPUT_COMBBUFFER!\r\n")));
                break;                
            }

            if ( pAllocMemInf->VirtAdd == NULL ||
                 pAllocMemInf->Size    == NULL)
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_INPUT_COMBBUFFER!\r\n")));
                break;                
            }

            pAllocMemInf->PhysAdd = (ULONG)GetPhysAddress((PVOID)(pAllocMemInf->VirtAdd),pAllocMemInf->Size);

            if( pAllocMemInf->PhysAdd == NULL)
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ADD_APP_INPUT_COMBBUFFER!\r\n")));
                break;
            }

            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                   PP_IOCTL_ADD_INPUT_COMBBUFFER,  // I/O control code
                                   &(pAllocMemInf->PhysAdd),       // in buffer
                                   sizeof(UINT32),                 // in buffer size
                                   NULL,                           // out buffer
                                   0,                              // out buffer size
                                   0,                              // number of bytes returned
                                   NULL);                          // ignored (=NULL)

            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_ADD_APP_INPUT_COMBBUFFER ")
                                  TEXT("occurred\r\n")));

            break;

        case UPP_IOCTL_SET_WINDOW_POS:
            pRect = (RECT *)pBufIn;
            bRet = DeviceIoControl(g_hPP,     // file handle to the driver
                                  PP_IOCTL_SET_WINDOW_POS,    // I/O control code
                                  pRect,                      // in buffer
                                  sizeof(UINT32),             // in buffer size
                                  NULL,                       // out buffer
                                  0,                          // out buffer size
                                  0,                          // number of bytes returned
                                  NULL);                      // ignored (=NULL)
            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_SET_WINDOW_POS ")
                                  TEXT("occurred\r\n")));
            break;

        case UPP_IOCTL_ALLOC_BUFFER:
            pAllocMemInf = (AllocMemInf *) pBufIn;

            if ( dwLenIn < sizeof(AllocMemInf))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_ALLOC_BUFFER!\r\n")));
                break;                
            }

            pAllocMemInf->Reserved = (UINT) AllocPhysMem(pAllocMemInf->Size, PAGE_READWRITE, 0, 0, &pAllocMemInf->PhysAdd);
            if(pAllocMemInf->Reserved == NULL)
            {
                pAllocMemInf->PhysAdd = NULL;
                pAllocMemInf->VirtAdd = NULL;
                ERRORMSG(TRUE, (TEXT("AllocPhysMem failed!\r\n")));
                break;
            }

            pAllocMemInf->VirtAdd = (UINT) VirtualAllocEx((HANDLE)GetCallerVMProcessId(), 0, pAllocMemInf->Size, MEM_RESERVE,PAGE_NOACCESS);
            if(pAllocMemInf->VirtAdd == NULL)
            {
                FreePhysMem((LPVOID ) pAllocMemInf->Reserved);
                pAllocMemInf->PhysAdd = NULL;
                ERRORMSG(TRUE, (TEXT("VirtualAllocEx failed!\r\n")));
                break;
            }

            if (!VirtualCopyEx((HANDLE)GetCallerVMProcessId(), (PVOID)pAllocMemInf->VirtAdd, GetCurrentProcess(), (PVOID)(pAllocMemInf->PhysAdd >> 8), 
                               pAllocMemInf->Size, PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
            {
                VirtualFreeEx((HANDLE)GetCallerVMProcessId(), (PVOID)pAllocMemInf->VirtAdd, 0, MEM_RELEASE);
                FreePhysMem((LPVOID ) pAllocMemInf->Reserved);
                pAllocMemInf->PhysAdd = NULL;
                pAllocMemInf->VirtAdd = NULL;
                ERRORMSG(TRUE, (TEXT("VirtualCopyEx failed!\r\n")));
                break;
            }

            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_ALLOC_BUFFER ")
                                  TEXT("occurred\r\n")));
            bRet = TRUE;
            break;

        case UPP_IOCTL_DEALLOC_BUFFER:
            pAllocMemInf = (AllocMemInf *) pBufIn;

            if ( dwLenIn < sizeof(AllocMemInf))
            {
                ERRORMSG(TRUE, (TEXT("Invalid parameter for UPP_IOCTL_DEALLOC_BUFFER!\r\n")));
                break;                
            }

            if (!VirtualFreeEx((HANDLE)GetCallerVMProcessId(), (PVOID)(pAllocMemInf->VirtAdd), 0, MEM_RELEASE)) 
            {
                ERRORMSG(TRUE, (TEXT("VirtualFreeEx failed!\r\n")));
                break;
            }
            
            if(!FreePhysMem((LPVOID )pAllocMemInf->Reserved))
            {
                ERRORMSG(TRUE, (TEXT("FreePhysMem failed!\r\n")));
                break;
            }

            DEBUGMSG(ZONE_IOCTL, (TEXT("UPP_IOControl: UPP_IOCTL_DEALLOC_BUFFER ")
                                  TEXT("occurred\r\n")));
            bRet = TRUE;
            break;

        default:
            DEBUGMSG(ZONE_WARN, (TEXT("UPP_IOControl: No matching IOCTL.\r\n")));
            break;
    }

    return bRet;
}


//------------------------------------------------------------------------------
//
// Function: GetPhysAddress
//
// This function translate the virtual address to physical address
//
// Parameters:
//      lpUnMappedBuffer
//          [in] Pointer of  the virtual address.
//      ulSize
//          [in] The buffer size of the virtual address.
//
// Returns:
//      Returns the physical address
//
//------------------------------------------------------------------------------

DWORD GetPhysAddress( PVOID lpUnMappedBuffer, ULONG ulSize)
{
    BOOL  blRet = FALSE;
    DWORD dwPhy = 0;
    DWORD * pPFNs = NULL;
    
    pPFNs = new DWORD[300];
    MarshalledBuffer_t MarshalledBuffer(lpUnMappedBuffer, ulSize, ARG_O_PTR, FALSE, TRUE);
    LPVOID lpMappedBuffer = MarshalledBuffer.ptr();
    
    blRet = LockPages(lpMappedBuffer, ulSize,
                     pPFNs, LOCKFLAG_WRITE);
    if(!blRet)
    {
        if (NULL != pPFNs)
        {
            delete [] pPFNs;
            pPFNs = NULL;
        }
        ERRORMSG(TRUE, (TEXT("LockPages error! error code = %d\r\n"),GetLastError()));
        return (DWORD)-1;
    }
    
    long shift = UserKInfo[KINX_PFN_SHIFT];
    if (NULL != pPFNs)
        dwPhy = (DWORD)(((DWORD)pPFNs[0]) << shift);
    DWORD dwOffset = (DWORD) lpUnMappedBuffer & (UserKInfo[KINX_PAGESIZE]-1);
    dwPhy = ((DWORD)dwPhy+dwOffset);

    blRet = UnlockPages(lpMappedBuffer, ulSize);
    if(!blRet)
    {
        if (NULL != pPFNs)
        {
            delete [] pPFNs;
            pPFNs = NULL;
        }
        ERRORMSG(TRUE, (TEXT("UnlockPages error! error code = %d\r\n"),GetLastError()));
        return (DWORD)-1;
    }

    if (NULL != pPFNs)
    {
        delete [] pPFNs;
        pPFNs = NULL;
    }
    return dwPhy;
}


//------------------------------------------------------------------------------
//
// Function: DllEntry
//
// This function is the driver DLL entry.
//
// Parameters:
//      hInstDll
//          [in] The handle of the DLL.
//      dwReason
//          [in] The action wanted this DLLEntry to do.
//      lpvReserved
//          [in] Reserved, Not use.
//
// Returns:
//      Return TRUE for success
//------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
           //Register Debug Zones
           DEBUGREGISTER((HINSTANCE) hInstDll);
           DEBUGMSG(ZONE_INFO, (TEXT("UPP_DllEntry: DLL_PROCESS_ATTACH ")
                                TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
           DisableThreadLibraryCalls((HMODULE) hInstDll);
           break;

        case DLL_PROCESS_DETACH:
           DEBUGMSG(ZONE_INFO, (TEXT("UPP_DllEntry: DLL_PROCESS_DETACH ")
                                TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
           break;
    }
    // Return TRUE for success
    return TRUE;
}
