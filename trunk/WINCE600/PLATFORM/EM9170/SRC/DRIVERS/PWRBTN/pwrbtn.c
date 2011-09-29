//-----------------------------------------------------------------------------
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
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <devload.h>
#include <Pm.h>
#pragma warning(pop)

#include "bsp.h"

HANDLE g_PwrMonThread = NULL;

#define DEFAULT_DEBOUNCE_PERIOD     20
#define DEFAULT_PRIORITY            10

#define QUEUE_ENTRIES   8
#define MAX_NAMELEN     64
#define QUEUE_SIZE      (QUEUE_ENTRIES*(sizeof(POWER_BROADCAST)+MAX_NAMELEN))
BOOL   g_FlagExitThrd = FALSE;


typedef struct {
    DDK_GPIO_PORT  port;
    DWORD pin;
    DWORD dwDebouncePeriod;
    DWORD dwSysIntr;
    DWORD dwLogintr;
    DWORD dwPriority;
    HANDLE hIntrEvent;
    HANDLE hIST;
    BOOL   bISTTerminate;
    BOOL fIgnoreNextInterrupt;
} T_PWRBTN_PARAM;


#define PWRBTN_PORT DDK_GPIO_PORT2
#define PWRBTN_PIN  11
#define PWRBTN_IRQ  IRQ_GPIO2_PIN11

//-----------------------------------------------------------------------------
//
//  Function:  PwrMonThread
//
//  This is the service thread for monitor Power state.  
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
INT WINAPI PwrMonThread(LPVOID lpParameter)
{
    HANDLE hNotifications;
    HANDLE hReadMsgQ;
    MSGQUEUEOPTIONS msgOptions = {0}; 

    T_PWRBTN_PARAM *pBtnContext = (T_PWRBTN_PARAM *) lpParameter;

    while(!IsAPIReady(SH_SHELL) || !IsAPIReady(SH_WMGR) || !IsAPIReady(SH_GDI))
    {
        Sleep(250);
    }

    // Create a message queue for Power Manager notifications.
    msgOptions.dwSize        = sizeof(MSGQUEUEOPTIONS);
    msgOptions.dwFlags       = 0;
    msgOptions.dwMaxMessages = QUEUE_ENTRIES;
    msgOptions.cbMaxMessage  = sizeof(POWER_BROADCAST) + MAX_NAMELEN;
    msgOptions.bReadAccess   = TRUE;

    hReadMsgQ = CreateMsgQueue(NULL, &msgOptions);
    if ( hReadMsgQ == NULL )
    {
        DEBUGMSG(1, (TEXT("PWM: CreateMsgQueue(Read): error %d\r\n"), GetLastError()));
        g_FlagExitThrd = TRUE;
    }

    // Request Power notifications
    hNotifications = RequestPowerNotifications(hReadMsgQ, POWER_NOTIFY_ALL);
    if ( ! hNotifications ) 
    {
        DEBUGMSG(1, (TEXT("PWM: RequestPowerNotifications: error %d\r\n"), GetLastError()));
        g_FlagExitThrd = TRUE;
    }

    while(!g_FlagExitThrd)
    {
        int   iBytesInQueue = 0;
        DWORD dwFlags;
        UCHAR buf[QUEUE_SIZE];
        PPOWER_BROADCAST pB = (PPOWER_BROADCAST) buf;

        memset(buf, 0, QUEUE_SIZE);
                
        DEBUGMSG(1, (TEXT("PWM: Waiting for PM state transition notification\r\n")));
    
        // Read message from queue.
        if ( ! ReadMsgQueue(hReadMsgQ, buf, QUEUE_SIZE, (LPDWORD)&iBytesInQueue, INFINITE, &dwFlags) )
        {
            DEBUGMSG(1, (TEXT("PWM: ReadMsgQueue: ERROR:%d\r\n"), GetLastError()));
        } 
        else if ( iBytesInQueue < sizeof(POWER_BROADCAST) )
        {
            DEBUGMSG(1, (TEXT("PWM: Received short message: %d bytes, expected: %d\r\n"),
                                  iBytesInQueue, sizeof(POWER_BROADCAST)));
        }
        else 
        {
            switch ( pB->Message )
            {
                case PBT_TRANSITION:
    
                    RETAILMSG(1, (TEXT("PWM: PBT_TRANSITION to system power state [Flags: 0x%x]: '%s'\r\n"),
                                  pB->Flags, pB->SystemPowerState));
    
                    switch ( POWER_STATE(pB->Flags) )
                    {
                        case POWER_STATE_ON:
                            DEBUGMSG(1, (TEXT("PWM: POWER_STATE_ON\r\n")));
                            break;
    
                        case POWER_STATE_OFF:
                            DEBUGMSG(1, (TEXT("PWM: POWER_STATE_OFF\r\n")));
                            break;
    
                        case POWER_STATE_CRITICAL:
                            DEBUGMSG(1, (TEXT("PWM: POWER_STATE_CRITICAL\r\n")));
                            break;
    
                        case POWER_STATE_BOOT:
                            DEBUGMSG(1, (TEXT("PWM: POWER_STATE_BOOT\r\n")));
                            break;
    
                        case POWER_STATE_IDLE:
                            DEBUGMSG(1, (TEXT("PWM: POWER_STATE_IDLE\r\n")));
                            break;
    
                        case POWER_STATE_SUSPEND:
                            DEBUGMSG(1, (TEXT("PWM: POWER_STATE_SUSPEND\r\n")));
                            break;
    
                        case POWER_STATE_RESET:
                            DEBUGMSG(1, (TEXT("PWM: POWER_STATE_RESET\r\n")));
                            break;
    
                        case POWER_STATE_PASSWORD:
                            DEBUGMSG(1, (TEXT("PWM: POWER_STATE_PASSWORD\r\n")));
                            break;
    
                        case 0:
                            DEBUGMSG(1,(TEXT("PWM: Power State Flags:0x%x\r\n"),pB->Flags));
                            break;
    
                        default:
                            DEBUGMSG(1,(TEXT("PWM: Unknown Power State Flags:0x%x\r\n"),pB->Flags));
                            break;
                    }
                    break;

                case PBT_RESUME:
                {
                    RETAILMSG(1, (TEXT("PWM: PBT_RESUME\r\n")));             
                    {
                        
                        DWORD wakeSrc = SYSWAKE_UNKNOWN;
                        DWORD bytesRet= 0;
                        if (KernelIoControl(IOCTL_HAL_GET_WAKE_SOURCE, NULL, 0, &wakeSrc, sizeof(wakeSrc), &bytesRet) &&
                           (bytesRet == sizeof(wakeSrc))) 
                        {
                                
                                SetSystemPowerState(NULL, POWER_STATE_ON, POWER_FORCE);
                                //We need reschedule the thread for PwrButtonIntrThread run first!!!!!!
                                Sleep(100);
                                             
                                pBtnContext->fIgnoreNextInterrupt = FALSE;                                               
                        }
                        else {
                            NKDbgPrintfW(L"PWM: Error getting wake source\r\n");
                        }
                    }
            
                    break;
                }       
                case PBT_POWERSTATUSCHANGE:
                    RETAILMSG(1, (TEXT("PWM: PBT_POWERSTATUSCHANGE\r\n")));
                    break;
                        
                case PBT_POWERINFOCHANGE:
                {
                    PPOWER_BROADCAST_POWER_INFO ppbpi = (PPOWER_BROADCAST_POWER_INFO) pB->SystemPowerState;
    
                    RETAILMSG(1, (TEXT("PWM: PBT_POWERINFOCHANGE\r\n")));
                    RETAILMSG(1, (TEXT("PWM: \tAC line status %u, battery flag %u, backup flag %u, %d levels\r\n"),
                            ppbpi->bACLineStatus, ppbpi->bBatteryFlag, 
                            ppbpi->bBackupBatteryFlag, ppbpi->dwNumLevels));
                    break;
                }
                        
                default:
                    DEBUGMSG(1, (TEXT("PWM: Unknown Message:%d\r\n"), pB->Message));
                    break;
            }
        }
    }

    if ( hNotifications )
    {
        StopPowerNotifications(hNotifications);
        hNotifications = NULL;
    }

    if ( hReadMsgQ )
    {
        CloseMsgQueue(hReadMsgQ);
        hReadMsgQ = NULL;
    }

    return 0;
}


DWORD WINAPI PwrButtonIntrThread(LPVOID lpParameter)
{
    DWORD rc = TRUE;
    T_PWRBTN_PARAM *pBtnContext = (T_PWRBTN_PARAM *) lpParameter;

    CeSetThreadPriority(GetCurrentThread(),pBtnContext->dwPriority);
    DDKGpioClearIntrPin(pBtnContext->port, pBtnContext->pin);

    for(;;)    
    {           
        UINT32 u32Value;

        if(WaitForSingleObject(pBtnContext->hIntrEvent,INFINITE)== WAIT_OBJECT_0  //Wait for  a power button event (pressed or released)        
           && !pBtnContext->bISTTerminate)
        {
            DDKGpioClearIntrPin(pBtnContext->port, pBtnContext->pin);
            InterruptDone(pBtnContext->dwSysIntr);

            for(;;) //wait for a stable state
            {            
                if (WaitForSingleObject(pBtnContext->hIntrEvent, pBtnContext->dwDebouncePeriod) == WAIT_TIMEOUT)
                {                   
                    break;
                }
                else
                {
                    DDKGpioClearIntrPin(pBtnContext->port, pBtnContext->pin);
                    InterruptDone(pBtnContext->dwSysIntr);    
                }            
            }
            
            // Now the button state is stable
            // let's check if t's pressed
            DDKGpioReadDataPin(pBtnContext->port, pBtnContext->pin, &u32Value);
            if ((u32Value == 0) && (pBtnContext->fIgnoreNextInterrupt == FALSE))
            {
                // Change intr trigger as low-level to avoid immediate resume
                DDKGpioSetConfig(pBtnContext->port, pBtnContext->pin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_LOW_LEV);
                // ZZZzzz...
                SetSystemPowerState(NULL,POWER_STATE_SUSPEND,0);
                pBtnContext->fIgnoreNextInterrupt = TRUE;
                // Change intr trigger back
                DDKGpioSetConfig(pBtnContext->port, pBtnContext->pin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_FALL_EDGE);
            }
            else
            {
                pBtnContext->fIgnoreNextInterrupt = FALSE;
            }
        }
        else
        {
            rc = FALSE;
            break;
        }
    } 

    return rc;
}

BOOL  PWR_Deinit(DWORD dwData)
{
    T_PWRBTN_PARAM *pBtnContext = (T_PWRBTN_PARAM *) dwData;
    
    if (pBtnContext == NULL)
    {
        return FALSE;
    }

    pBtnContext->bISTTerminate = TRUE;  
    if (pBtnContext->hIST && pBtnContext->hIntrEvent)
    {
        SetEvent(pBtnContext->hIntrEvent);
        CloseHandle(pBtnContext->hIST);
        pBtnContext->hIST = NULL;
    }

    if (pBtnContext->hIntrEvent)
    {
        CloseHandle(pBtnContext->hIntrEvent);
        pBtnContext->hIntrEvent = NULL;
    }
    if (pBtnContext->dwSysIntr != SYSINTR_UNDEFINED)
    {
        KernelIoControl(IOCTL_HAL_DISABLE_WAKE,    &pBtnContext->dwSysIntr,sizeof(pBtnContext->dwSysIntr), NULL, 0, NULL);
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pBtnContext->dwSysIntr, sizeof(pBtnContext->dwSysIntr), NULL, 0, NULL);
    }
    LocalFree(pBtnContext);

    if (g_PwrMonThread)
    {     
        CloseHandle(g_PwrMonThread);
        g_PwrMonThread = NULL;
    }
    g_FlagExitThrd = TRUE;
    return (TRUE);
}


BOOL WINAPI PWR_Init( LPCTSTR pContext,LPCVOID lpvBusContext)
{    
    HKEY hKey;    
    
    T_PWRBTN_PARAM *pBtnContext = NULL;

    UNREFERENCED_PARAMETER(lpvBusContext);

    //DEBUGMSG(ZONE_INIT, (TEXT("+PWRBTN Init\r\n")));
    RETAILMSG(1, (TEXT("+PWRBTN Init\r\n")));

    pBtnContext = LocalAlloc(LMEM_ZEROINIT,sizeof(T_PWRBTN_PARAM));
    if (pBtnContext == NULL)
    {
        ERRORMSG(ZONE_ERROR,(TEXT("unable to allocate memory for context\r\n")));
        goto error;
    }

    pBtnContext->dwDebouncePeriod = DEFAULT_DEBOUNCE_PERIOD;
    pBtnContext->dwPriority = DEFAULT_PRIORITY;
    pBtnContext->dwSysIntr = (DWORD) SYSINTR_UNDEFINED;
    pBtnContext->port = PWRBTN_PORT;
    pBtnContext->pin = PWRBTN_PIN;
    pBtnContext->dwLogintr = PWRBTN_IRQ;
    
    if ((hKey = OpenDeviceKey(pContext)) != NULL)
    {
        DWORD size,type;
        size = sizeof(DWORD);
        RegQueryValueEx(hKey,TEXT("debouncePeriod"),NULL,&type,(PBYTE) &(pBtnContext->dwDebouncePeriod),&size);
        size = sizeof(DWORD);
        RegQueryValueEx(hKey,TEXT("Piority"),NULL,&type,(PBYTE) &(pBtnContext->dwPriority),&size);
    }

    
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_A25, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_A25, DDK_IOMUX_PAD_SLEW_FAST, (DDK_IOMUX_PAD_DRIVE) 0,
                                        DDK_IOMUX_PAD_OPENDRAIN_ENABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                        DDK_IOMUX_PAD_VOLTAGE_1V8);
    
    // Now configure PIO in INPUT mode with interrupt on both edge
    //DDKGpioSetConfig(pBtnContext->port, pBtnContext->pin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_FALL_EDGE);
    DDKGpioSetConfig(pBtnContext->port, pBtnContext->pin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_FALL_EDGE);

    pBtnContext->bISTTerminate = FALSE; 

    pBtnContext->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pBtnContext->hIntrEvent == NULL)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("ERROR: unable to creae event\r\n")));
        goto error;
    }

    
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &pBtnContext->dwLogintr, sizeof(pBtnContext->dwLogintr), &pBtnContext->dwSysIntr, sizeof(pBtnContext->dwSysIntr), NULL))
    {
        ERRORMSG(ZONE_ERROR, (TEXT("ERROR: Failed to request the sysintr for Power Button (dwLogintr :%d).\r\n"), pBtnContext->dwLogintr));
        goto error;
    }       
    
    if (!InterruptInitialize(pBtnContext->dwSysIntr, pBtnContext->hIntrEvent, 0,0)) 
    {        
        ERRORMSG(ZONE_ERROR, (TEXT("Could not initialize PowerButton interrupt.\r\n")));
        goto error;
    }
    
    KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &pBtnContext->dwSysIntr,sizeof(pBtnContext->dwSysIntr), NULL, 0, NULL);

    pBtnContext->hIST = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) PwrButtonIntrThread, (PVOID)pBtnContext, 0, NULL);
    if ( pBtnContext->hIST == NULL ) 
    {
        ERRORMSG(ZONE_ERROR, (TEXT("Fatal Error!  Failed to create PwrButton Interrupt thread.\r\n")));
        goto error;
    }

    g_PwrMonThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) PwrMonThread, (PVOID)pBtnContext, 0, NULL);
    if ( g_PwrMonThread == NULL )
    {
        ERRORMSG(1, (TEXT("PWB_Init: Failed to create Power Monitor Thread\r\n")));
        goto  error;
    }
    
    //DEBUGMSG(ZONE_INIT, (TEXT("-PWRBTN Init\r\n")));
    RETAILMSG(1, (TEXT("-PWRBTN Init 0x%x\r\n"), (DWORD)pBtnContext));		// CS&ZHL JUN-2-2011: debug
    return (DWORD)pBtnContext; 

error:
    //DEBUGMSG(ZONE_INIT, (TEXT("-PWRBTN Init\r\n")));
    RETAILMSG(1, (TEXT("-PWRBTN Init Failed!\r\n")));											// CS&ZHL JUN-2-2011: debug
    PWR_Deinit((DWORD)pBtnContext);
    return (DWORD)NULL;
}



BOOL WINAPI
DllEntry(
             HANDLE  hInstDll,
             DWORD   dwReason,
             LPVOID  lpvReserved
             )
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch ( dwReason ) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstDll);
        DEBUGMSG(ZONE_INIT, (TEXT("PwrButton : DLL_PROCESS_ATTACH\r\n")));
        break;

    case DLL_PROCESS_DETACH:
        // should be signaling thread here
        DEBUGMSG(ZONE_INIT, (TEXT("PwrButton : DLL_PROCESS_DETACH\r\n")));
        break;
    }

    return (TRUE);
}


void PWR_PowerUp(DWORD hDeviceContext)
{
     UNREFERENCED_PARAMETER(hDeviceContext);
 
}

void PWR_PowerDown(DWORD hDeviceContext)
{

    T_PWRBTN_PARAM *pBtnContext = (T_PWRBTN_PARAM *) hDeviceContext;
    pBtnContext->fIgnoreNextInterrupt = TRUE;

    
}

BOOL PWR_IOControl(DWORD hOpenContext,DWORD dwCode,PBYTE pBufIn,DWORD dwLenIn,PBYTE pBufOut,DWORD dwLenOut,PDWORD pdwActualOut)
{
  UNREFERENCED_PARAMETER(hOpenContext);
  UNREFERENCED_PARAMETER(dwCode);
  UNREFERENCED_PARAMETER(pBufIn);
  UNREFERENCED_PARAMETER(dwLenIn);
  UNREFERENCED_PARAMETER(pBufOut);
  UNREFERENCED_PARAMETER(dwLenOut);
  UNREFERENCED_PARAMETER(pdwActualOut);
  return FALSE;
}

DWORD PWR_Open (DWORD dwData, DWORD dwAccess, DWORD dwShareMode) 
{
    UNREFERENCED_PARAMETER(dwData);
    UNREFERENCED_PARAMETER(dwAccess);
    UNREFERENCED_PARAMETER(dwShareMode);
    return 0;
}
BOOL  PWR_Close(DWORD dwData) 
{
    UNREFERENCED_PARAMETER(dwData);
    return FALSE;
}


//! @}
