//------------------------------------------------------------------------------
//
//  Copyright (C) 2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dvfc.c
//
//  This file contains driver support for the DVFS and DPTC power
//  management features of the SoC.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)
#include "common_macros.h"

//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPDvfcInit(void);
extern BOOL BSPDvfcDeinit(void);
extern void BSPDvfcIntrServ(void);
extern UCHAR BSPDvfcGetSupportedDx(void); 
extern BOOL BSPDvfcPowerSet(UINT32 domain, CEDEVICE_POWER_STATE dx);
extern CEDEVICE_POWER_STATE BSPDvfcPowerGet(UINT32 domain);
extern int BSPDvfcGetThreadPriority(void);


//-----------------------------------------------------------------------------
// External Variables
extern DWORD g_DvfcIrq;


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static DWORD g_dwDvfcSysIntr;
static HANDLE g_hDvfcIntrEvent;
static HANDLE g_hDvfcIntrServThread;

//-----------------------------------------------------------------------------
// Local Functions
static DWORD WINAPI DvfcIntrServThread (LPVOID lpParam);

//-----------------------------------------------------------------------------
//
//  Function:  DllEntry
//
//  This function is an optional method of entry into a DLL. If the function
//  is used, it is called by the system when processes and threads are
//  initialized and terminated, or on calls to the LoadLibrary and
//  FreeLibrary functions.
//
//  Parameters:
//      hinstDLL
//          [in] Handle to the DLL. The value is the base address of the DLL.
//
//      dwReason
//          [in] Specifies a flag indicating why the DLL entry-point function
//          is being called.
//
//      lpvReserved
//          [in] Specifies further aspects of DLL initialization and cleanup.
//          If dwReason is DLL_PROCESS_ATTACH, lpvReserved is NULL for
//          dynamic loads and nonnull for static loads. If dwReason is
//          DLL_PROCESS_DETACH, lpvReserved is NULL if DllMain is called
//          by using FreeLibrary and nonnull if DllMain is called during
//          process termination.
//
//  Returns:
//      When the system calls the DllMain function with the
//      DLL_PROCESS_ATTACH value, the function returns TRUE if it
//      succeeds or FALSE if initialization fails.
//
//      If the return value is FALSE when DllMain is called because the
//      process uses the LoadLibrary function, LoadLibrary returns NULL.
//
//      If the return value is FALSE when DllMain is called during
//      process initialization, the process terminates with an error.
//
//      When the system calls the DllMain function with a value other
//      than DLL_PROCESS_ATTACH, the return value is ignored.
//
//-----------------------------------------------------------------------------
BOOL WINAPI DllEntry(HINSTANCE hDllHandle, DWORD  dwReason,
                           LPVOID lpreserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpreserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls((HMODULE) hDllHandle);
        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }
    
    return TRUE;
}
       

//-----------------------------------------------------------------------------
//
//  Function:  Init
//
//  This function initializes the DVFC driver.  Called by the Device Manager to
//  initialize a device.
//
//  Parameters:
//      pContext 
//          [in] Pointer to a string containing the registry path to the 
//                active key for the stream interface driver.
//
//  Returns:
//      Returns a handle to the device context created if successful. 
//      Returns zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD DVF_Init(LPCTSTR pContext)
{
    DWORD rc = 0;
    DWORD irq;
    HKEY  hKey;
    DWORD dwDomain, dwSize;
    LONG error;
    static BOOL bInit = FALSE;
    
    // Try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        ERRORMSG(TRUE, (_T("OpenDeviceKey failed!\r\n")));
        goto cleanUp;
    }

    // Load DVFS domain index from registry data
    dwDomain = 0;
    dwSize = sizeof(dwDomain);
    error = RegQueryValueEx(hKey,
                            L"Index",
                            NULL,
                            NULL,
                            (LPBYTE)(&dwDomain),
                            &dwSize);

    // Close handle to open key
    RegCloseKey(hKey);

    // Check for errors during RegQueryValueEx
    if (error != ERROR_SUCCESS)
    {
        ERRORMSG(TRUE, (_T("RegQueryValueEx failed!\r\n")));
        goto cleanUp;
    }

    if (!bInit)
    {
        // Create event for IST signaling
        g_hDvfcIntrEvent = CreateEvent(NULL, FALSE, FALSE, L"INTR_EVENT_DVFC");
        if (!g_hDvfcIntrEvent)
        {
            ERRORMSG(TRUE, (_T("CreateEvent failed for DVFC IST signal!\r\n")));
            goto cleanUp;
        }
        
        // Translate IRQ into SYSINTR
        irq = g_DvfcIrq;
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(irq),
            &g_dwDvfcSysIntr, sizeof(g_dwDvfcSysIntr), NULL))
        {
            ERRORMSG(TRUE, (_T("IOCTL_HAL_REQUEST_SYSINTR failed for IRQ_CCM1!\r\n")));
            goto cleanUp;
        }

        // Create event for IST signaling
        g_hDvfcIntrEvent = CreateEvent(NULL, FALSE, FALSE, L"INTR_EVENT_DVFC");
        if (!g_hDvfcIntrEvent)
        {
            ERRORMSG(TRUE, (_T("CreateEvent failed for DVFC IST signal!\r\n")));
            goto cleanUp;
        }
        
        // Register interrupt
        if (!InterruptInitialize(g_dwDvfcSysIntr, g_hDvfcIntrEvent, NULL, 0))
        {
            ERRORMSG(TRUE, (_T("InterruptInitialize failed for DVFC SYSINTR!\r\n")));
            goto cleanUp;
        }
        // Allow platform-specific DVFC initialization
        if (!BSPDvfcInit())
        {
            ERRORMSG(TRUE, (_T("BSPDvfcInit failed!\r\n")));
            goto cleanUp;
        }
        
        // Create IST for DVFC interrupts
        g_hDvfcIntrServThread = CreateThread(NULL, 0, DvfcIntrServThread, NULL, 0, NULL);      
        if (!g_hDvfcIntrServThread) 
        {
            ERRORMSG(TRUE, (_T("CreateThread failed for DVFC IST!\r\n")));
            goto cleanUp;
        }
    }

    bInit = TRUE;
    rc = dwDomain;

cleanUp:

    return rc;

}


//-----------------------------------------------------------------------------
//
//  Function:  Deinit
//
//  This function deinitializes the DPTC.  Called by the Device Manager to
//  deinitialize a device.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DVF_Deinit(void)
{
    BOOL rc;

    rc = BSPDvfcDeinit();

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: Open
//
//  This function opens a device for reading, writing, or both.  An application 
//  indirectly invokes this function when it calls the CreateFile function to 
//  open special device file names.
//
//  Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function creates
//                and returns this handle.
//      AccessCode
//          [in] Access code for the device. The access is a combination of
//                read and write access from CreateFile.
//      ShareMode
//          [in] File share mode of the device. The share mode is a
//                combination of read and write access sharing from CreateFile.
//
//  Returns:
//      This function returns a handle that identifies the open context of
//      the device to the calling application.
//
//-----------------------------------------------------------------------------
DWORD DVF_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    return hDeviceContext;
}


//-----------------------------------------------------------------------------
//
//  Function: Close
//
//  This function closes a device context created by the hOpenContext 
//  parameter.
//
//  Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//                the open context of the device.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL DVF_Close(DWORD hOpenContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  IOControl
//
//  This function sends a command to the DVFC driver.
//
//  Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//              (Device Manager) function creates and returns this identifier. 
//
//      dwCode 
//          [in] I/O control operation to perform.
//
//      pBufIn 
//          [in] Pointer to the buffer containing data to transfer to the 
//          device. 
//
//      dwLenIn 
//          [in] Number of bytes of data in the buffer specified for pBufIn. 
//
//      pBufOut 
//          [out] Pointer to the buffer used to transfer the output data from 
//          the device. 
//
//      dwLenOut 
//          [in] Maximum number of bytes in the buffer specified by pBufOut. 
//
//      pdwActualOut 
//          [out] Pointer to the DWORD buffer that this function uses to 
//          return the actual number of bytes received from the device. 
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL DVF_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
    PBYTE pBufOut,  DWORD dwLenOut, PDWORD pdwActualOut)
{
    BOOL rc;
    DWORD  dwErr = ERROR_INVALID_PARAMETER;
    PPOWER_CAPABILITIES ppc;
    CEDEVICE_POWER_STATE dx;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);

    switch (dwCode) 
    {
        
    case IOCTL_POWER_CAPABILITIES:
        // Tell the power manager about ourselves.
        if (pBufOut != NULL 
            && dwLenOut >= sizeof(POWER_CAPABILITIES) 
            && pdwActualOut != NULL) 
        {
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try 
            {
                ppc = (PPOWER_CAPABILITIES) pBufOut;
                memset(ppc, 0, sizeof(*ppc));              
                ppc->DeviceDx = BSPDvfcGetSupportedDx();
                *pdwActualOut = sizeof(*ppc);
                dwErr = ERROR_SUCCESS;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE, (_T("Exception in DVFC IOCTL_POWER_CAPABILITIES\r\n")));
            }
        }

        break;
        
        
    case IOCTL_POWER_SET: 
        if(pBufOut != NULL 
            && dwLenOut == sizeof(CEDEVICE_POWER_STATE) 
            && pdwActualOut != NULL) 
        {
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try 
            {
                dx = *(PCEDEVICE_POWER_STATE) pBufOut;
                if(VALID_DX(dx)) 
                {                    
                    *(PCEDEVICE_POWER_STATE) pBufOut = dx;
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    if (BSPDvfcPowerSet((UINT32) hOpenContext, dx))
                    {
                        dwErr = ERROR_SUCCESS;
                    }
                }
            } 
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE, (_T("Exception in DVFC IOCTL_POWER_SET\r\n")));
            }
        }
        break;
        
    case IOCTL_POWER_GET: 
        if(pBufOut != NULL 
            && dwLenOut == sizeof(CEDEVICE_POWER_STATE) 
            && pdwActualOut != NULL) {
            // Just return our current Dx value
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try 
            {
                *(PCEDEVICE_POWER_STATE) pBufOut = BSPDvfcPowerGet((UINT32) hOpenContext);
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                dwErr = ERROR_SUCCESS;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE, (_T("Exception in DVFC IOCTL_POWER_SET\r\n")));
            }
        }
        break;
        
    default:
        ERRORMSG(TRUE, (_T("Unsupported DVFC IOCTL code %u\r\n"), dwCode));
        dwErr = ERROR_NOT_SUPPORTED;
        break;
    }
    
    // Pass back appropriate response codes
    SetLastError(dwErr);

    if(dwErr != ERROR_SUCCESS) 
    {
        rc = FALSE;
    } 

    else 
    {
        rc = TRUE;
    }
    
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  DvfcIntrServThread
//
//  This is the interrupt service thread for DPTC interrupts.  
//  deinitialize a device.
//
//  Parameters:
//      lpParam
//          [in] Thread data passed to the function using the 
//          lpParameter parameter of the CreateThread function. Not used.
//
//  Returns:
//      Returns thread exit code.
//
//-----------------------------------------------------------------------------
static DWORD WINAPI DvfcIntrServThread (LPVOID lpParam)
{
    DWORD rc = TRUE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParam);
    
    CeSetThreadPriority(GetCurrentThread(), BSPDvfcGetThreadPriority());

    for (;;)
    {
        if(WaitForSingleObject(g_hDvfcIntrEvent, INFINITE) == WAIT_OBJECT_0)
        {
            BSPDvfcIntrServ();
            InterruptDone(g_dwDvfcSysIntr);
        }
        else 
        {
            // Abnormal signal
            rc = FALSE;
            break;
        }
    }

    return rc;
}
