//------------------------------------------------------------------------------
//
// Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 

#pragma warning(push)
#pragma warning(disable: 4201)
#include <windows.h>
#pragma warning(pop)

#include <devload.h>
#include "usbfntypes.h"
#include "transporttypes.h"
#include "transport.h"
#include "phd_com_model.h"
#include "usb_phdc.h"               /* USB PHDC Class Header File */

#define PHD_PROTOCOL_LOG 0
#define PHD_APP_LOG 0
/******************************************************************************
 * User Interface
 ******************************************************************************/
typedef struct _Button_Object {
    DWORD idx;
    LPTSTR buttonName;
} Button_Object;

// PHD and APP state
static UCHAR g_appEvent = APP_PHD_UNINITIALISED;

// Simulation Timer Handles
static HANDLE g_hSelectTimerThread = NULL;

static HANDLE g_hAppThread;

static PVOID g_pvInterface = NULL;

static Button_Object g_Button_Send_Measurement   =  {0, L"BUTTON_SEND_MEASUREMENT"};
static Button_Object g_Button_Disconnect         =  {1, L"BUTTON_DISCONNECT"};
static Button_Object g_Button_Select_Device_Spec =  {2, L"BUTTON_SELECT_DEVICE_SPEC"};

#define BUTTON_NUMBER 3
static HANDLE g_hButtons[BUTTON_NUMBER];

static Comm_Object g_Comm_APP_State_Change = {0, L"APP_STATE_CHANGE"};

static LPTSTR const g_app_state_string[] = {
    L"APP_PHD_UNINITIALISED",
    L"APP_PHD_INITIALISED",
    L"APP_PHD_CONNECTED_TO_HOST",
    L"APP_PHD_DISCONNECTED_FROM_HOST",
    L"APP_PHD_MEASUREMENT_SENT",
    L"APP_PHD_MEASUREMENT_SENDING",
    L"APP_PHD_ASSOCIATION_TIMEDOUT",
    L"APP_PHD_ERROR",
    L"APP_PHD_INITIATED",
    L"APP_PHD_SELECT_TIMER_STARTED",
    L"APP_PHD_DISCONNECTING",
    L"APP_PHD_SELECT_TIMER_OFF"
};

/*****************************************************************************
 * Functions
 *****************************************************************************/
#ifndef SHIP_BUILD
LPTSTR getAppStateString (
        uint_8 app_state_idx
        )
{
    return g_app_state_string[app_state_idx];
}
#endif

// DLL entry point
extern "C"
BOOL
WINAPI
DllEntry(
    HINSTANCE hinstDll,
    DWORD     dwReason,
    LPVOID    lpReserved
    )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpReserved);

#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("DllEntry"));
#endif

    if (dwReason == DLL_PROCESS_ATTACH) {
        DEBUGREGISTER(hinstDll);
        DEBUGMSG(ZONE_INIT, (_T("%s Attached\r\n"), pszFname));
        DisableThreadLibraryCalls((HMODULE) hinstDll);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DEBUGMSG(ZONE_INIT, (_T("%s Detached\r\n"), pszFname));       
    }

    return TRUE;
}

VOID
ChangeAppState(
        UCHAR appState
        )
{
    HANDLE h_MsgChangeAppState;
    RETAILMSG(PHD_PROTOCOL_LOG, (L"\"%s\" ---------> \"%s\"\r\n", getAppStateString(g_appEvent), getAppStateString(appState)));
    g_appEvent = appState;

    // Tell App
    h_MsgChangeAppState = CreateEvent(0, FALSE, FALSE, g_Comm_APP_State_Change.eventName);
    SetEventData(h_MsgChangeAppState, g_appEvent);
    SetEvent(h_MsgChangeAppState);
    CloseHandle(h_MsgChangeAppState);

    return;
}


static void SwitchIEEEConfig(void)
{
    g_phd_selected_config = (g_phd_selected_config + 1) % MAX_DEV_SPEC_SUPPORTED;
    return;
}

static void Button_Pressed(void)
{
    {
        DWORD dwWait = WaitForMultipleObjects(dim(g_hButtons), g_hButtons, FALSE, 0);
        if (dwWait == WAIT_TIMEOUT)
        {
            // No Button is Pressed
        }
        else
        {
            DWORD dwIdx = dwWait - WAIT_OBJECT_0;
            if (dwIdx == g_Button_Send_Measurement.idx)
            {
                RETAILMSG(PHD_APP_LOG, (L"Send Measurement Button !\r\n"));
                if (g_appEvent == APP_PHD_CONNECTED_TO_HOST)
                {
                    ChangeAppState(APP_PHD_MEASUREMENT_SENDING);
                    PHD_Send_Measurements_to_Manager();
                }
            }
            else if (dwIdx == g_Button_Disconnect.idx)
            {
                RETAILMSG(PHD_APP_LOG, (L"Disconnect Button !\r\n"));
                if (g_appEvent == APP_PHD_CONNECTED_TO_HOST)
                {
                    ChangeAppState(APP_PHD_DISCONNECTING);
                    PHD_Disconnect_from_Manager();
                }
            }
            else if (dwIdx == g_Button_Select_Device_Spec.idx)
            {
                HANDLE h_MsgConfigSelected;
                RETAILMSG(PHD_APP_LOG, (L"Select Spec Button!\r\n"));
                if (g_appEvent == APP_PHD_SELECT_TIMER_STARTED)
                {
                    // What this mean
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Change Spec when timer started\r\n"));
                }
                else if (g_appEvent == APP_PHD_CONNECTED_TO_HOST)
                {
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"First Disconnect and re-associating\r\n"));
                    ChangeAppState(APP_PHD_DISCONNECTING);
                    PHD_Disconnect_from_Manager();
                }

                // Change the next configration
                SwitchIEEEConfig();
                // g_phd_selected_config = (g_phd_selected_config + 1) % MAX_DEV_SPEC_SUPPORTED;
                RETAILMSG(PHD_PROTOCOL_LOG, (L"Current Config idx is %x\r\n", g_phd_selected_config));

                // Tell App
                h_MsgConfigSelected = CreateEvent(0, FALSE, FALSE, g_Comm_SPEC_Selected.eventName);
                SetEventData(h_MsgConfigSelected, g_phd_selected_config);
                SetEvent(h_MsgConfigSelected);
                CloseHandle(h_MsgConfigSelected);
            }
        }
    }
    return; 
}

DWORD
WINAPI
PHDC_SelectTimerThread(
        LPVOID lpParameter
        )
{
#define TIMER_LENGTH 1000  // in ms
    UNREFERENCED_PARAMETER(lpParameter);
    RETAILMSG(PHD_PROTOCOL_LOG, (L"\t[PHDC] Select Timer Thread Runs\r\n"));
    Sleep(TIMER_LENGTH);
    ChangeAppState(APP_PHD_SELECT_TIMER_OFF);
    ExitThread(0);
    return 0;
}

DWORD
WINAPI
PHDC_AppThread(
        LPVOID lpParameter
        )
{
    UNREFERENCED_PARAMETER(lpParameter);
    RETAILMSG(1, (L"PHDC Thread Is Running\r\n"));

    // Tell App the initial State
    {
        HANDLE h_MsgConfigSelected;

        // Tell App
        h_MsgConfigSelected = CreateEvent(0, FALSE, FALSE, g_Comm_SPEC_Selected.eventName);
        SetEventData(h_MsgConfigSelected, g_phd_selected_config);
        SetEvent(h_MsgConfigSelected);
        CloseHandle(h_MsgConfigSelected);
    }

    // Create Button Event to recieve signal from APP
    g_hButtons[g_Button_Send_Measurement.idx]   = CreateEvent(0, FALSE, FALSE, g_Button_Send_Measurement.buttonName);
    g_hButtons[g_Button_Disconnect.idx]         = CreateEvent(0, FALSE, FALSE, g_Button_Disconnect.buttonName);
    g_hButtons[g_Button_Select_Device_Spec.idx] = CreateEvent(0, FALSE, FALSE, g_Button_Select_Device_Spec.buttonName);

    for (;;)
    {
        Sleep(200);
        Button_Pressed();

        switch (g_appEvent)
        {
            case APP_PHD_INITIALISED:
                // goes to "APP_PHD_SELECT_TIMER_STARTED"
                // RETAILMSG(1, (L"Main Initialised\r\n"));
                ChangeAppState(APP_PHD_SELECT_TIMER_STARTED);
                // Create A thread, which delays "SELECT_TIMEOUT" and change "g_appEvent" to "APP_PHD_SELECT_TIMER_OFF"
                g_hSelectTimerThread = CreateThread(NULL, 0, PHDC_SelectTimerThread, NULL, 0, NULL);
                break;
            case APP_PHD_DISCONNECTED_FROM_HOST:
                /* 
                    transition to initialised state so the association 
                    procedure can start again 
                */
                ChangeAppState(APP_PHD_INITIALISED);
                break;

            case APP_PHD_MEASUREMENT_SENT:
                /* 
                    enters here each time we receive a response to the
                    measurements sent 
                */
                ChangeAppState(APP_PHD_CONNECTED_TO_HOST);
                break;
            case APP_PHD_SELECT_TIMER_OFF:
                RETAILMSG(PHD_PROTOCOL_LOG, (L"Main Timer OFF\r\n"));
                if (g_hSelectTimerThread != NULL)
                {
                    CloseHandle(g_hSelectTimerThread);
                }
                ChangeAppState(APP_PHD_INITIATED);
                PHD_Connect_to_Manager();
                break;

            case APP_PHD_END:
                goto APP_END;
                break;
                  
            default:
                break;
        }
    }
APP_END:
    return TRUE;
}

// APP
BOOL
WINAPI
APP_Callback(
    PVOID   pvNotifyParameter,
    DWORD   dwMsg,
    DWORD   dwParam
    ) 
{

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pvNotifyParameter);

    switch(dwMsg) {
        case UFN_MSG_BUS_EVENTS:
            switch(dwParam) {
                case UFN_DETACH:
                    ChangeAppState(APP_PHD_UNINITIALISED);
                    ChangePhdcState(PHD_AG_STATE_DISCONNECTED);
                    {
                        // Tell App
                        HANDLE h_MsgDevDisconnected;
                        h_MsgDevDisconnected = CreateEvent(0, FALSE, FALSE, g_Comm_Disconnected.eventName);
                        SetEvent(h_MsgDevDisconnected);
                        CloseHandle(h_MsgDevDisconnected);
                    }
                    break;
                case UFN_RESET:
                    ChangeAppState(APP_PHD_UNINITIALISED);
                    ChangePhdcState(PHD_AG_STATE_DISCONNECTED);
                    break;
                default:
                    break;
            }
            break;
        default:
             break;
     }
    return TRUE;
}

// APP
extern "C"
DWORD
TestApp_Init(
    LPCTSTR pszActiveKey
    )
{
    RETAILMSG(1, (L"\t[PHD] Transport Init\r\n"));
    DWORD dwErr = PHD_Transport_Init(pszActiveKey, APP_Callback);

    if (dwErr == ERROR_SUCCESS)
    {
        RETAILMSG(1, (L"\t[PHD] Crate App thread\r\n"));
        g_hAppThread = CreateThread(NULL, 0, PHDC_AppThread, NULL, 0, NULL);
        if (g_hAppThread == NULL) 
        {
            RETAILMSG(1, (L"PHDC_APP Thread Creation Fail\r\n"));
            dwErr = GetLastError();
        }
    }

    return dwErr;
}

// APP
extern "C"
DWORD
Init(
    LPCTSTR pszActiveKey
    )
{
    DWORD dwErr = UfnInitializeInterface(pszActiveKey, getDeviceHandle(), getFuncsHandle(), &g_pvInterface);

    if (dwErr == ERROR_SUCCESS) {
        RETAILMSG(1, (L"\t[PHD] App Init\r\n"));
        dwErr = TestApp_Init(pszActiveKey);

        if (dwErr != ERROR_SUCCESS) {
            UfnDeinitializeInterface(g_pvInterface);
        }
    }

    return (dwErr == ERROR_SUCCESS);
}


// APP
extern "C" 
BOOL
Deinit(
    DWORD dwContext
    )
{
    PHD_Close();
    DEBUGCHK(g_pvInterface);
    UfnDeinitializeInterface(g_pvInterface);
    ChangeAppState(APP_PHD_END);
    Sleep(500);
    CloseHandle(g_hAppThread);
    
    // Remove-W4: Warning C4100 workaround    
    UNREFERENCED_PARAMETER(dwContext);

    return TRUE;
}
