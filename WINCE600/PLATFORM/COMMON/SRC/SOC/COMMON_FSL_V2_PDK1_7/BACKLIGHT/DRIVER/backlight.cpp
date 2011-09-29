//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  backlight.cpp
//
//  Implementation of Backlight Driver
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <DevLoad.h>
#include <msgqueue.h>
#include <pm.h>
#pragma warning(pop)

#include "backlight.h"

//------------------------------------------------------------------------------
// External Functions
extern void BSPBacklightInitialize();
extern void BSPBacklightRelease();
extern void BSPBacklightSetIntensity(DWORD level);
extern void BSPBacklightEnable();

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines
#define dim(x)          (sizeof(x) / sizeof(x[0]))

// Backlight Change in Control Panel
#define BKL_CHANGE      TEXT("BKLChangeInControlPanel")


// Queue size for Power Manager Notification MsgQ
#define QUEUE_ENTRIES   1
#define QUEUE_SIZE      (QUEUE_ENTRIES * (sizeof(POWER_BROADCAST) + MAX_PATH))

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

#ifdef DEBUG
DBGPARAM dpCurSettings = {
    L"Backlight", {
        L"Info",    L"Function",    L"Warnings",    L"Errors",
        L"Init",    L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0008
};
#endif

// Backlight device name
static TCHAR szName[DEVNAME_LEN];

// Current Backlight device power state.
static CRITICAL_SECTION cs; 
static CEDEVICE_POWER_STATE CurDx;

// Power Manager notification variables
static HANDLE hMsgQ;
static HANDLE hNotifications;

static bklSettings_t bklSettings;

// Event handling thread
static HANDLE ghInterruptServiceThread;

// thread wait on these handles
static HANDLE hWaitList[bklMaxWaitEvents];
static HANDLE hevDummy;
static HANDLE hUserActivityEvent;
static HANDLE hUserInactivityEvent;

// Backlight Change Event in Control Panel
static HANDLE hBklChangeEvent;

// device on AC/DC status flag
static BOOL bACOnline;

// Interrupt thread loop flag
static BOOL bIntrThreadLoop;

//------------------------------------------------------------------------------
// Local Functions
static void GetBacklightTimeoutSettings(bklSettings_t *pSettings);
static void GetBacklightLevelSettings(bklSettings_t *pSettings);
static DWORD CalculateTimeout(bklSettings_t *pSettings);
extern "C" BOOL BKL_Deinit(DWORD dwContext);
DWORD WINAPI BKL_EventThread(LPVOID lpParameter);

//-----------------------------------------------------------------------------
//
// Function: BKL_Init
//
// This function initializes a device. 
//
// Parameters:
//      dwContext
//          [IN] Pointer to a string containing the registry path 
//               to the active key for the stream interface driver. 
// Returns:
//      Returns a handle to the device context created if successful. 
//
//-----------------------------------------------------------------------------
extern "C" UINT32 BKL_Init(UINT32 dwContext)
{
    SYSTEM_POWER_STATUS_EX status; 
    MSGQUEUEOPTIONS msgOptions;   
    HKEY  hDriverKey;
    DWORD dwStatus;
    DWORD dwType;
    DWORD dwSize;

    //DEBUGMSG(ZONE_FUNCTION, (TEXT("BKL_INIT() +!\r\n")) );
    RETAILMSG(1, (TEXT("BKL_INIT() +!\r\n")) );

    InitializeCriticalSection(&cs);

    // get our activation information
    dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)dwContext, 0, 0, 
                            &hDriverKey);
    if (dwStatus != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("BKL_Init: OpenDeviceKey('%s') failed %u\r\n"), 
                              dwContext, dwStatus));
        goto InitErrCleanup;
    }
    else
    {
        dwSize = sizeof(szName);
        dwStatus = RegQueryValueEx(hDriverKey, DEVLOAD_DEVNAME_VALNAME, NULL, 
                                   &dwType, (LPBYTE) szName, &dwSize);
        if (dwStatus != ERROR_SUCCESS || dwType != DEVLOAD_DEVNAME_VALTYPE)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("BKL_Init: RegQueryValueEx('%s', '%s') failed %u\r\n"),
                      dwContext, DEVLOAD_DEVNAME_VALNAME, dwStatus));
            RegCloseKey(hDriverKey);
            goto InitErrCleanup;
        }
        DEBUGMSG(ZONE_INIT, (TEXT("device name is '%s'\r\n"), szName));
    }

    // Default settings:
    //  enable backlight timeout, enable backlight on user activity
    bklSettings.dwACTimeout = BKL_DEFAULT_TIMEOUT;
    bklSettings.dwBattTimeout = BKL_DEFAULT_TIMEOUT;
    bklSettings.fBattTimeoutEnable = 1;
    bklSettings.fACTimeoutEnable = 1;
    bklSettings.fBattBacklightOnUser = 1;
    bklSettings.fACBacklightOnUser = 1;
    bklSettings.dwBattBacklightLevel = BKL_LEVEL_DEFAULT;
    bklSettings.dwACBacklightLevel = BKL_LEVEL_DEFAULT;
    GetBacklightLevelSettings(&bklSettings);

    // Get current power source
    bACOnline = FALSE;

    if (GetSystemPowerStatusEx(&status, TRUE))
    {
        if (status.ACLineStatus == 1)
        {
            bACOnline = TRUE;
        }
    }

    // Get handles to desired events/msgQ etc.
    // Get handle to backlight timer change Event
    //event from Control Panel
    hWaitList[bklControlPanelEvent] = CreateEvent(NULL, FALSE, FALSE, 
                                                  L"BackLightChangeEvent" ) ;  
    if (!hWaitList[bklControlPanelEvent])
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Get BackLightChangeEvent failed!!\r\n")));
        goto InitErrCleanup;
    }

    // Get handle to Power notification event
    msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    msgOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    msgOptions.dwMaxMessages = 0;
    msgOptions.cbMaxMessage = sizeof(POWER_BROADCAST) + MAX_PATH;
    msgOptions.bReadAccess = TRUE;

    // create message queue
    hMsgQ = CreateMsgQueue(NULL, &msgOptions);
    if (!hMsgQ)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Create MsgQueue fail!!\n"), GetLastError()));
        goto InitErrCleanup;
    }
    else
    {
        hNotifications = RequestPowerNotifications(hMsgQ, PBT_POWERSTATUSCHANGE);
        if (!hNotifications)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Request Power Notification fail!! %d\n"), GetLastError()));
            goto InitErrCleanup;
        }
        hWaitList[bklPowerNotificationEvent] = hMsgQ;
    }

    // Get handle to User Activity Event
    // Backup handle to user activity event.
    hUserActivityEvent = CreateEvent(NULL, FALSE, FALSE, 
                                     L"PowerManager/UserActivity_Active");
    if (!hUserActivityEvent)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Get User Activity Event failed!!\r\n")));
        goto InitErrCleanup;
    }

    hUserInactivityEvent = CreateEvent(NULL, FALSE, FALSE, 
                                       L"PowerManager/UserActivity_Inactive");
    if (!hUserInactivityEvent)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Get User Inactivity Event failed!!\r\n")));
        goto InitErrCleanup;
    }

    // Create a dummy event that will never be signaled.
    hevDummy = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hevDummy)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Create dummy event failed!!\r\n")));
        goto InitErrCleanup;
    }
    hWaitList[bklUserInactivityEvent] = hUserInactivityEvent;

    // Get handle to backlight level change event
    hWaitList[bklLevelChangeEvent] = CreateEvent(NULL, FALSE, FALSE, 
                                                 L"BackLightLevelChangeEvent" );
    if (!hWaitList[bklLevelChangeEvent])
    {
        DEBUGMSG(ZONE_ERROR, 
                 (TEXT("Get BackLightLevelChangeEvent failed!!\r\n")));
        goto InitErrCleanup;
    }
    // Do BSP initialization
    BSPBacklightInitialize();

    // Create Backlight Change Event in Control Panel
    hBklChangeEvent = CreateEvent(NULL, FALSE, FALSE, BKL_CHANGE);
    if(hBklChangeEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Create BKL_CHANGE event failed!\r\n")));
    }

    // Init device power state to D0
    CurDx = D0;

    DEBUGMSG(ZONE_INIT, (TEXT("BKL_Init: CreateThread\r\n")));

    bIntrThreadLoop = TRUE;
    ghInterruptServiceThread = CreateThread(NULL, 0, 
                                            (LPTHREAD_START_ROUTINE)BKL_EventThread, 
                                            NULL, 0, NULL);
    if (ghInterruptServiceThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR, 
                 (TEXT("BKL_Init: Failed to create BLD Interrupt Service Thread\r\n")));
        goto InitErrCleanup;
    }

    //DEBUGMSG(ZONE_FUNCTION, (TEXT("BKL_INIT() -!\r\n")) );
    RETAILMSG(1, (TEXT("BKL_INIT() -!\r\n")) );
    return 1;   

    InitErrCleanup:
    if (hDriverKey)
    {
        RegCloseKey(hDriverKey);
    }

    BKL_Deinit(dwContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("BKL_INIT() -!\r\n")) );
    return 0; 
}
//-----------------------------------------------------------------------------
//
// Function: BKL_Deinit
//
// This function uninitializes a device. 
//
// Parameters:
//      dwContext
//          [IN] Handle to the device context. 
// Returns:
//      TRUE indicates success. FALSE indicates failure. 
//
//-----------------------------------------------------------------------------
extern "C" BOOL BKL_Deinit(DWORD dwContext)
{
    DWORD i;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+BKL_Deinit: dwContext = 0x%x\r\n"), 
                             dwContext));

    // Give the thread a chance to perform any cleanup.
    if (ghInterruptServiceThread)
    {
        bIntrThreadLoop = FALSE;
        SetEvent(hWaitList[bklControlPanelEvent]);
        CloseHandle(ghInterruptServiceThread);
    }
    // Stop Power Notifications
    if (hNotifications)
    {
        StopPowerNotifications(hNotifications);
    }
    if (hMsgQ)
    {
        CloseMsgQueue(hMsgQ);
    }

    // Close wait Handles
    for (i = 0; i < dim(hWaitList); i++)
    {
        if (hWaitList[i])
        {
            CloseHandle(hWaitList[i]);
        }
    }
    if (hevDummy)
    {
        CloseHandle(hevDummy);
    }

    if (hUserActivityEvent)
    {
        CloseHandle(hUserActivityEvent);
    }

    // Close Handle
    if(hBklChangeEvent)
    {
        CloseHandle(hBklChangeEvent);
    }

    DeleteCriticalSection(&cs);

    // Do BSP deinitialization
    BSPBacklightRelease();

    // the device manager does not check the return code
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BKL_Deinit\r\n"), dwContext));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: BKL_EventThread
//
// This function is the application-defined function
// to be executed by the thread. 
//
// Parameters:
//      lpParameter
//          [IN] Thread data passed to the function using 
//               the lpParameter parameter of the CreateThread function.  
// Returns:
//      The function should return a value that 
//      indicates its success or failure. 
//
//-----------------------------------------------------------------------------
DWORD WINAPI BKL_EventThread(LPVOID lpParameter)
{
    UCHAR buf[QUEUE_SIZE];
    SYSTEM_POWER_STATUS_EX2 sps; 

    DWORD dwTimeout;
    DWORD dwStatus;
    DWORD result; 

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

    if (!CeSetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("PMAPI!CeSetThreadPriority ERROR:%d\n"), 
                              GetLastError()));
    }

    // Force initial update of timeout values and backlight levels.
    SetEvent(hWaitList[bklControlPanelEvent]);
    SetEvent(hWaitList[bklLevelChangeEvent]);

    while (bIntrThreadLoop)
    {
        result = WaitForMultipleObjects(dim(hWaitList), hWaitList, FALSE, 
                                        INFINITE);

        if (!bIntrThreadLoop)
        {
            break;
        }

        switch (result)
        {
            case (WAIT_OBJECT_0 + bklControlPanelEvent):
                // EVENT COME FROM CONTROL PANEL 

                // GET backlight settings
                GetBacklightTimeoutSettings(&bklSettings);

                // If Timeout is disabled, Not need wait Inactive event.
                if (bACOnline == TRUE)
                {
                    if (bklSettings.fACTimeoutEnable)
                    {
                        hWaitList[bklUserInactivityEvent] = hUserInactivityEvent;
                    }
                    else
                    {
                        hWaitList[bklUserInactivityEvent] = hevDummy;
                    }
                }
                else
                {
                    if (bklSettings.fBattTimeoutEnable)
                    {
                        hWaitList[bklUserInactivityEvent] = hUserInactivityEvent;
                    }
                    else
                    {
                        hWaitList[bklUserInactivityEvent] = hevDummy;
                    }
                }               

                break;

            case (WAIT_OBJECT_0 + bklPowerNotificationEvent):
                DWORD dwSize;

                // Do not block on our message queue.
                if (ReadMsgQueue(hMsgQ, buf, QUEUE_SIZE, (LPDWORD)&dwSize, 
                                 0, &dwStatus))
                {
                    GetSystemPowerStatusEx2(&sps, sizeof(sps), TRUE); 
                    DEBUGMSG(ZONE_INFO, (_T("Line status is %s (0x%x)\r\n"), 
                                         sps.ACLineStatus == AC_LINE_ONLINE ? _T("AC") : 
                                         sps.ACLineStatus == AC_LINE_OFFLINE ? _T("Offline") :
                                         sps.ACLineStatus == AC_LINE_BACKUP_POWER ? _T("Backup") :
                                         sps.ACLineStatus == AC_LINE_UNKNOWN ? _T("Unknown") : _T("???"), 
                                         sps.ACLineStatus));

                    if (sps.ACLineStatus == AC_LINE_ONLINE)
                    {
                        DEBUGMSG(ZONE_INFO, (TEXT("bACOnline = TRUE;\r\n")));
                        if (bACOnline != TRUE)
                        {
                            bACOnline = TRUE;
                            BSPBacklightSetIntensity(bklSettings.dwACBacklightLevel);
                        }
                    }
                    else
                    {
                        DEBUGMSG(ZONE_INFO, (TEXT("bACOnline = FALSE;\r\n"))); 
                        if (bACOnline == TRUE)
                        {
                            bACOnline = FALSE;
                            BSPBacklightSetIntensity(bklSettings.dwBattBacklightLevel);
                        }
                    }

                    // Force backlight settings update.
                    SetEvent(hWaitList[bklControlPanelEvent]);
                    SetEvent(hWaitList[bklLevelChangeEvent]);
                }
                else
                    DEBUGMSG(ZONE_ERROR, (TEXT("ReadMsgQueue: ERROR:%d\n"), 
                                          GetLastError()));
                break;        

            case (WAIT_OBJECT_0 + bklUserInactivityEvent):

                dwTimeout = CalculateTimeout(&bklSettings);

                if (dwTimeout != INFINITE)
                {
                    if (WaitForSingleObject(hUserActivityEvent, dwTimeout) == WAIT_TIMEOUT)
                    {
                        // shut off backlight
                        dwStatus = DevicePowerNotify(szName, D4, POWER_NAME);
                        if (dwStatus != ERROR_SUCCESS)
                        {
                            DEBUGMSG(ZONE_ERROR, (_T("%s: DevicePowerNotify(D4) failed %d\r\n"), szName, dwStatus));
                        }
                        else
                        {
                            // Keep it off until some user activity occurs 
                            WaitForSingleObject(hUserActivityEvent, INFINITE); 

                            // shut on backlight                        
                            dwStatus = DevicePowerNotify(szName, D0, POWER_NAME);
                            if (dwStatus != ERROR_SUCCESS)
                            {
                                DEBUGMSG(ZONE_ERROR, (_T("%s: DevicePowerNotify(D0) failed %d\r\n"), szName, dwStatus));
                            }
                        }
                    }

                }

                break;                


            case (WAIT_OBJECT_0 + bklLevelChangeEvent):
                // Event for change in backlight level  
                // Get backlight levels from registry
                DEBUGMSG(ZONE_INFO, (TEXT("backlight batt level = %d\r\n"), 
                                     bklSettings.dwBattBacklightLevel));
                DEBUGMSG(ZONE_INFO, (TEXT("backlight ac level = %d\r\n"), 
                                     bklSettings.dwACBacklightLevel));

                GetBacklightLevelSettings(&bklSettings);

                // Set backlight levels depending on currently on battery or AC.
                if (bACOnline)
                {
                    BSPBacklightSetIntensity(bklSettings.dwACBacklightLevel);
                }
                else
                {
                    BSPBacklightSetIntensity(bklSettings.dwBattBacklightLevel);
                }
                DEBUGMSG(ZONE_INFO, (TEXT("new backlight batt level = %d\r\n"), 
                                     bklSettings.dwBattBacklightLevel));
                DEBUGMSG(ZONE_INFO, (TEXT("new backlight ac level = %d\r\n"), 
                                     bklSettings.dwACBacklightLevel));

                // Backlight change
                if(hBklChangeEvent != NULL)
                {
                    SetEvent(hBklChangeEvent);
                }

                break;       

            case WAIT_FAILED:
                DEBUGMSG(ZONE_ERROR, (TEXT("WAIT_FAILED!\r\n")));
                break;

            default:
                break;
        }
    } // end while

    return 0; 
}

//-----------------------------------------------------------------------------
//
// Function: BKL_IOControl
//
// This function sends a command to a device.
//
// Parameters:
//      dwContext
//          [IN] Handle to the open context of the device. 
//      Ioctl
//          [IN] I/O control operation to perform. 
//      pInBuf
//          [IN] Pointer to the buffer containing data to transfer to the device.
//      InBufLen
//          [IN] Number of bytes of data in the buffer specified for pBufIn. 
//      pOutBuf
//          [out] Pointer to the buffer used to transfer the output data 
//                from the device.  
//      OutBufLen
//          [IN] Maximum number of bytes in the buffer specified by pBufOut. 
//      pdwBytesTransferred
//          [IN] Pointer to the DWORD buffer that this function uses to return 
//               the actual number of bytes received from the device. 
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
extern "C" BOOL
BKL_IOControl(
             DWORD  dwContext,
             DWORD  Ioctl,
             PUCHAR pInBuf,
             DWORD  InBufLen, 
             PUCHAR pOutBuf,
             DWORD  OutBufLen,
             PDWORD pdwBytesTransferred
             )
{
    DWORD  dwErr = ERROR_INVALID_PARAMETER;
    BOOL   bRc = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(InBufLen);
    UNREFERENCED_PARAMETER(pInBuf);
    UNREFERENCED_PARAMETER(dwContext);


#ifdef DEBUG
    TCHAR  szBuf[128];
    LPTSTR pszFname;

    // format the routine name
    pszFname = szBuf;

    DEBUGMSG(ZONE_INFO, (TEXT("%s: IOCTL:0x%x, InBuf:0x%x, InBufLen:%d, OutBuf:0x%x, OutBufLen:0x%x)\r\n"),
                         pszFname, Ioctl, pInBuf, InBufLen, pOutBuf, OutBufLen));
#endif  

    switch (Ioctl)
    {
        case IOCTL_POWER_CAPABILITIES:
            // tell the power manager about ourselves.
            DEBUGMSG(ZONE_INFO, (TEXT("%s: BKL_IOCTL_POWER_CAPABILITIES\r\n"), 
                                 pszFname));
            if ( pOutBuf != NULL && 
                 OutBufLen >= sizeof(POWER_CAPABILITIES) && 
                 pdwBytesTransferred != NULL)
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pOutBuf;
                memset(ppc, 0, sizeof(POWER_CAPABILITIES));
                ppc->DeviceDx = 0x11;   // support D0, D4
                ppc->WakeFromDx = 0x00; // No wake capability
                ppc->InrushDx = 0x00;       // No in rush requirement
                ppc->Power[D0] = (DWORD) PwrDeviceUnspecified;
                ppc->Power[D1] = (DWORD) PwrDeviceUnspecified;
                ppc->Power[D2] = (DWORD) PwrDeviceUnspecified;
                ppc->Power[D3] = (DWORD) PwrDeviceUnspecified;
                ppc->Power[D4] = 0;
                ppc->Latency[D0] = 0;
                ppc->Latency[D1] = (DWORD) PwrDeviceUnspecified;
                ppc->Latency[D2] = (DWORD) PwrDeviceUnspecified;
                ppc->Latency[D3] = (DWORD) PwrDeviceUnspecified;
                ppc->Latency[D4] = 0;
                ppc->Flags = 0;
                *pdwBytesTransferred = sizeof(POWER_CAPABILITIES);
                dwErr = ERROR_SUCCESS;
            }
            break;

        case IOCTL_POWER_QUERY: 
            DEBUGMSG(ZONE_INFO, (TEXT("%s: BKL_IOCTL_POWER_QUERY\r\n"), 
                                 pszFname));
            if ( pOutBuf != NULL && 
                 OutBufLen == sizeof(CEDEVICE_POWER_STATE) && 
                 pdwBytesTransferred != NULL)
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuf;
                DEBUGMSG(ZONE_INFO, (TEXT("NewDx = %d\r\n"), NewDx));

                if (VALID_DX(NewDx))
                {
                    // this is a valid Dx state so return a good status
                    *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
                    dwErr = ERROR_SUCCESS;
                }
                DEBUGMSG(ZONE_INFO, (TEXT("%s: IOCTL_POWER_QUERY %u %s\r\n"), 
                                     pszFname, NewDx, dwErr == ERROR_SUCCESS ? 
                                     TEXT("succeeded") : TEXT("failed")));
            }
            break;

        case IOCTL_POWER_SET: 
            DEBUGMSG(ZONE_INFO, (TEXT("%s: BKL_IOCTL_POWER_SET\r\n"), pszFname));
            if ( pOutBuf != NULL && 
                 OutBufLen == sizeof(CEDEVICE_POWER_STATE) && 
                 pdwBytesTransferred != NULL)
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuf;
                DEBUGMSG(ZONE_INFO, (TEXT("NewDx = %d\r\n"), NewDx));
                if (NewDx != CurDx)
                {
                    if (NewDx == D0)
                    {
                        // TURN ON BACKLIGHT
                        if (bACOnline)
                        {
                            BSPBacklightEnable();
                            BSPBacklightSetIntensity(bklSettings.dwACBacklightLevel);
                        }
                        else
                        {
                            BSPBacklightEnable();
                            BSPBacklightSetIntensity(bklSettings.dwBattBacklightLevel);
                        }
                        DEBUGMSG(ZONE_INFO, (TEXT("BackLight ON\r\n"))); 

                        // Backlight change
                        if(hBklChangeEvent != NULL)
                        {
                            SetEvent(hBklChangeEvent);
                        }
                    }
                    else
                    {
                        // if asked for a state we don't support, go to the next lower one
                        // which is D4
                        NewDx = D4;
                        BSPBacklightEnable();
                        BSPBacklightSetIntensity(0);
                        DEBUGMSG(ZONE_INFO, (TEXT("BackLight Off\r\n")));
                    }
                    EnterCriticalSection(&cs) ;
                    CurDx = NewDx;
                    LeaveCriticalSection(&cs);
                    *(PCEDEVICE_POWER_STATE)pOutBuf = CurDx;
                }
                *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
                DEBUGMSG(ZONE_INFO, (TEXT("CurDx = %d\r\n"), CurDx));
                dwErr = ERROR_SUCCESS;
            }
            break;

        case IOCTL_POWER_GET: 
            DEBUGMSG(ZONE_INFO, (TEXT("%s: BKL_IOCTL_POWER_GET\r\n"), pszFname));
            if ( pOutBuf != NULL && 
                 OutBufLen == sizeof(CEDEVICE_POWER_STATE) && 
                 pdwBytesTransferred != NULL)
            {
                // just return our CurrentDx value
                *(PCEDEVICE_POWER_STATE)pOutBuf = CurDx;
                *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
                dwErr = ERROR_SUCCESS;
            }
            break;

        default:
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Unsupported IOCTL code %u\r\n"), 
                                  pszFname, Ioctl));
            dwErr = ERROR_NOT_SUPPORTED;
            break;
    }

    // pass back appropriate response codes
    SetLastError(dwErr);
    if (dwErr != ERROR_SUCCESS)
    {
        bRc = FALSE;
    }
    else
    {
        bRc = TRUE;
    }

    return bRc;
}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This function is an optional method of entry into a DLL. 
//
// Parameters:
//      hInstDll
//          [IN] Handle to the DLL. 
//      dwReason
//          [IN] Specifies a flag indicating why the DLL entry-point 
//               function is being called. 
//      lpvReserved
//          [IN] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      This function returns a handle that identifies 
//      the open context of the device to the calling application.
//
//-----------------------------------------------------------------------------
extern "C" BOOL WINAPI DllEntry(
                                   HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hInstDll);
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE)hInstDll);
            DEBUGMSG(ZONE_INFO, (TEXT("DllEntry: DLL_PROCESS_ATTACH\r\n")));
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INFO, (TEXT("DllEntry: DLL_PROCESS_DETACH\r\n")));
            break;
    }
    // return TRUE for success
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: BKL_Open
//
// This funtion opens a Handle to the device.
//
// Parameters:
//      dwData
//          [IN] Handle to the device context.
//      dwAccess
//          [IN] Access code for the device.
//      dwShareMode
//          [IN] File share mode of the device.
//
// Returns:
//      This function returns a handle that identifies 
//      the open context of the device to the calling application.
//
//-----------------------------------------------------------------------------
extern "C" DWORD BKL_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwAccess);
    UNREFERENCED_PARAMETER(dwShareMode);

    DEBUGMSG(ZONE_FUNCTION, 
             (TEXT("+BKL_Open: dwData = 0x%x, dwAccess = 0x%x, dwShareMode = 0x%x\r\n"), 
              dwData, dwAccess, dwShareMode));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BKL_Open\r\n")));

    return dwData;
}

//-----------------------------------------------------------------------------
//
// Function: BKL_Close
//
// Always Returns TRUE.
//
// Parameters:
//      Handle
//          [IN] Opened Handle to Backlight
//
// Returns:
//      TRUE / FALSE.
//
//-----------------------------------------------------------------------------
extern "C" BOOL BKL_Close(DWORD Handle)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+BKL_Close: Handle = 0x%x\r\n"), Handle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BKL_Close: Handle = 0x%x\r\n"), Handle));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: BKL_PowerDown
//
// Turn off any hardware and/or interrupts in preparation for suspend
//
// Parameters:
//      None
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
extern "C" void BKL_PowerDown(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("BKL_POWERDOWN!!\r\n")));
}

//-----------------------------------------------------------------------------
//
// Function: BKL_PowerUp
//
// Prepare to resume by either re-opening the device or resetting it to closed
//
// Parameters:
//      None
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
extern "C" void BKL_PowerUp(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("BKL_POWERUP!!\r\n")));
}

//-----------------------------------------------------------------------------
//
// Function: BKL_Read
//
// Not used
//
// Parameters:
//      Handle
//          [IN] Opened Handle to Backlight
//      pBuffer      
//          [IN] Not used
//      dwNumBytes      
//          [IN] Not used
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
extern "C" DWORD BKL_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    DEBUGMSG(ZONE_FUNCTION, 
             (TEXT("BKL_Read: Handle = 0x%x, pBuffer = 0x%x, dwNumBytes = 0x%x\r\n"), 
              Handle, pBuffer, dwNumBytes));
    return 0;
}

//-----------------------------------------------------------------------------
//
// Function: BKL_Write
//
// Not used
//
// Parameters:
//      Handle
//          [IN] Opened Handle to Backlight
//      pBuffer      
//          [IN] Not used
//      dwNumBytes      
//          [IN] Not used
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
extern "C" DWORD BKL_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    DEBUGMSG(ZONE_FUNCTION, 
             (TEXT("BKL_Write: Handle = 0x%x, pBuffer = 0x%x, dwNumBytes = 0x%x\r\n"), 
              Handle, pBuffer, dwNumBytes));
    return 0;
}

//-----------------------------------------------------------------------------
//
// Function: BKL_Seek
//
// Not used
//
// Parameters:
//      Handle
//          [IN] Opened Handle to Backlight
//      lDistance      
//          [IN] Not used
//      dwMoveMethod      
//          [IN] Not used
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
extern "C" DWORD BKL_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(lDistance);
    UNREFERENCED_PARAMETER(dwMoveMethod);

    DEBUGMSG(ZONE_FUNCTION, 
             (TEXT("BKL_Seek: Handle = 0x%x, lDistance = 0x%x, dwMoveMethod = 0x%x\r\n"), 
              Handle, lDistance, dwMoveMethod));
    return(DWORD) -1;
}


//-----------------------------------------------------------------------------
//
// Function: GetBacklightLevelSettings
//
// This function is used to calculate the backlight level from Registry
//
// Parameters:
//      bklSettings_t
//          [OUT] Function will fill the backlight level settings from registry
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void GetBacklightLevelSettings(bklSettings_t *pSettings)
{
    HKEY  hDriverKey;
    DWORD dwStatus;
    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwLevel;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pSettings);

    dwStatus = RegOpenKeyEx(HKEY_CURRENT_USER, REG_PATH , 0, 0, &hDriverKey);
    if (dwStatus != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Open BackLight registry fail!!\r\n")));
    }
    else
    {
        dwStatus = RegQueryValueEx(hDriverKey, BATT_LEVEL_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwLevel, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight Batt level registry fail!!\r\n")));
        }
        else
        {
            if (dwLevel > BKL_LEVEL_MAX)
            {
                dwLevel = BKL_LEVEL_MAX;
            }
            if (dwLevel < BKL_LEVEL_MIN)
            {
                dwLevel = BKL_LEVEL_MIN;
            }
            bklSettings.dwBattBacklightLevel = dwLevel;
        }

        dwStatus = RegQueryValueEx(hDriverKey, AC_LEVEL_SUBKEY , NULL, &dwType, 
                                   (LPBYTE)&dwLevel, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight AC level registry fail!!\r\n")));
        }
        else
        {
            if (dwLevel > BKL_LEVEL_MAX)
            {
                dwLevel = BKL_LEVEL_MAX;
            }

            if (dwLevel < BKL_LEVEL_MIN)
            {
                dwLevel = BKL_LEVEL_MIN;
            }

            bklSettings.dwACBacklightLevel = dwLevel;
        }

        RegCloseKey(hDriverKey);
    }


}

//-----------------------------------------------------------------------------
//
// Function: GetBacklightTimeoutSettings
//
// This function is used to calculate the backlight timeout settings
//
// Parameters:
//      bklSettings_t
//          [OUT] Function will fill the backlight timeout settings from registry
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
#if !(defined(BSP_POCKETPC) || defined(BSP_SMARTPHONE))
static void GetBacklightTimeoutSettings(bklSettings_t *pSettings)
{
    HKEY  hDriverKey;
    DWORD dwStatus;
    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwSetting;

    DEBUGMSG(ZONE_INFO, (TEXT("BACKLIGHT BATT TIMEOUT = %d seconds\r\n"), 
                         pSettings->dwBattTimeout));
    DEBUGMSG(ZONE_INFO, (TEXT("BACKLIGHT AC TIMEOUT = %d seconds\r\n"), 
                         pSettings->dwACTimeout));

    dwStatus = RegOpenKeyEx(HKEY_CURRENT_USER, REG_PATH , 0, 0, &hDriverKey);
    if (dwStatus != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Open BackLight registry fail!!\r\n")));
    }
    else
    {
        // Update backlight timeout setttings if timeout is enabled
        dwStatus = RegQueryValueEx(hDriverKey, BATT_TIMEOUT_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight Batt timeout registry fail!!\r\n")));
        }
        else
        {
            pSettings->dwBattTimeout = dwSetting;
        }

        dwStatus = RegQueryValueEx(hDriverKey, AC_TIMEOUT_SUBKEY , NULL, &dwType, 
                                   (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight AC timeout registry fail!!\r\n")));
        }
        else
        {
            pSettings->dwACTimeout = dwSetting;
        }

        // Get backlight timeout enable
        dwStatus = RegQueryValueEx(hDriverKey, BATT_USEBATT_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight Use Battery Check Box fail!!\r\n")));
        }
        else
        {
            pSettings->fBattTimeoutEnable = dwSetting? TRUE : FALSE;
        }

        dwStatus = RegQueryValueEx(hDriverKey, AC_USEEXT_SUBKEY , NULL, &dwType, 
                                   (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight Use Ext check box fail!!\r\n")));
        }
        else
        {
            pSettings->fACTimeoutEnable = dwSetting? TRUE : FALSE;
        }

        RegCloseKey(hDriverKey);
    }
    // Always enable backlight on user activity
    pSettings->fBattBacklightOnUser = TRUE;
    pSettings->fACBacklightOnUser = TRUE;
} 
#else
static void GetBacklightTimeoutSettings(bklSettings_t *pSettings)
{
    HKEY  hDriverKey;
    DWORD dwStatus;
    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwSetting;

    DEBUGMSG(ZONE_INFO, (TEXT("BACKLIGHT BATT TIMEOUT = %d seconds\r\n"), 
                         pSettings->dwBattTimeout));
    DEBUGMSG(ZONE_INFO, (TEXT("BACKLIGHT AC TIMEOUT = %d seconds\r\n"), 
                         pSettings->dwACTimeout));

    dwStatus = RegOpenKeyEx(HKEY_CURRENT_USER, REG_PATH , 0, 0, &hDriverKey);
    if (dwStatus != ERROR_SUCCESS)
    {
        RETAILMSG(1, (TEXT("Open BackLight registry fail!!\r\n")));
    }
    else
    {
        // Get backlight timeout enable
        dwStatus = RegQueryValueEx(hDriverKey, BATT_TO_UNCHECKED_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("Get BackLight Battery Timeout") 
                                  TEXT("Unchecked check box fail!!\r\n")));
        }
        else
        {
            // registry 1 == disable timeout
            pSettings->fBattTimeoutEnable = dwSetting? 0 : 1;
        }

        dwStatus = RegQueryValueEx(hDriverKey, AC_TO_UNCHECKED_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight AC Timeout Unchecked check box fail!!\r\n")));
        }
        else
        {
            // registry 1 == disable timeout
            pSettings->fACTimeoutEnable = dwSetting? 0 : 1;
        }

        // Get backlight timeout setttings
        dwStatus = RegQueryValueEx(hDriverKey, BATT_TIMEOUT_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight Batt timeout registry fail!!\r\n")));
        }
        else
        {
            pSettings->dwBattTimeout = dwSetting;
        }

        dwStatus = RegQueryValueEx(hDriverKey, AC_TIMEOUT_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight AC timeout registry fail!!\r\n")));
        }
        else
        {
            pSettings->dwACTimeout = dwSetting;
        }

        // Get enable backlight on user activity
        dwStatus = RegQueryValueEx(hDriverKey, BATT_BACKLIGHTONTAP_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight BacklightOnTap fail!!\r\n")));
        }
        else
        {
            pSettings->fBattBacklightOnUser = dwSetting? TRUE : FALSE;
        }

        dwSize = sizeof(pSettings->fACBacklightOnUser);
        dwStatus = RegQueryValueEx(hDriverKey, AC_BACKLIGHTONTAP_SUBKEY , NULL, 
                                   &dwType, (LPBYTE)&dwSetting, &dwSize);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_ERROR, 
                     (TEXT("Get BackLight ACBacklightOnTap fail!!\r\n")));
        }
        else
        {
            pSettings->fACBacklightOnUser = dwSetting? TRUE : FALSE;
        }

        RegCloseKey(hDriverKey);
    }

} 
#endif

//-----------------------------------------------------------------------------
//
// Function: CalculateTimeout
//
// This function is used to calculate the timeout.
//
// Parameters:
//      bklSettings_t
//          [OUT] Function will fill the backlight timeout settings from registry
//
// Returns:
//      Timeout Value.
//
//-----------------------------------------------------------------------------
DWORD CalculateTimeout(bklSettings_t *pSettings)
{
    DWORD dwTimeout = INFINITE ;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pSettings);

    if (bACOnline == TRUE)
    {
        if (bklSettings.fACTimeoutEnable)
        {
            dwTimeout = bklSettings.dwACTimeout * 1000;
        }
        else
        {
            dwTimeout = INFINITE ;
        }
    }
    else
    {
        if (bklSettings.fBattTimeoutEnable)
        {
            dwTimeout = bklSettings.dwBattTimeout * 1000 ;
        }
        else
        {
            dwTimeout = INFINITE ;
        }
    }

    return dwTimeout;
}
