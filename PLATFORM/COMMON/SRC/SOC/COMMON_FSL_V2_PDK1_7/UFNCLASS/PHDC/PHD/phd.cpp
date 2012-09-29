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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        PHD.CPP

Abstract:

        USB Personal Health Care Protocol.
        
--*/

//------------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
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

#ifdef DEBUG

DBGPARAM dpCurSettings = {
    _T("usbphdfn"),
    {
        _T("Error"), _T("Warning"), _T("Init"), _T("Function"),
        _T("Comments"), _T(""), _T(""), _T(""),
        _T(""), _T(""), _T(""), _T(""), 
        _T(""), _T(""), _T(""), _T("")
    },
    0x5
};

#endif // DEBUG

/******************************************************************************
 * Macros
 ******************************************************************************/
#define dim(x) (sizeof(x)/sizeof((x)[0]))

#define BOT_RESET_REQUEST 0xFF
#define BOT_GET_MAX_LUN_REQUEST 0xFE

#define PHD_GET_STATUS_REQUEST    0
#define PHD_CLEAR_FEATURE_REQUEST 1
#define PHD_SET_FEATURE_REQUEST   3

#define USB_APP_DATA_RECEIVED 0x81
#define USB_APP_SEND_COMPLETE 0x82

#define CONTROL_TRANSFER 0
#define OUT_TRANSFER 1
#define IN_TRANSFER 2
#define I_IN_TRANSFER 3     // Must Be Consistent with the order in Endponint Descriptors

#define PHD_INI_LOG 0
#define PHD_PROTOCOL_LOG 0
#define PHD_APP_LOG 0

enum CONTROL_RESPONSE {
    CR_SUCCESS = 0,
    CR_SUCCESS_SEND_CONTROL_HANDSHAKE, // Use if no data stage
    CR_STALL_DEFAULT_PIPE,
    CR_UNHANDLED_REQUEST,
};

typedef struct _PIPE_TRANSFER {
    PUFN_PIPE phPipe;
    HANDLE hev;
    UFN_TRANSFER hTransfer;
} PIPE_TRANSFER, *PPIPE_TRANSFER;

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void PHD_Assoc_Response_Handler(APDU* val);
static void PHD_Unhandled_Request(APDU* val);
static void PHD_Disconnect_Handler(APDU* val);
static void PHD_Connect_Handler(APDU* val);
static void PHD_Config_Event_Report_Handler(APDU* val);
static void PHD_ABRT_Request_Handler(APDU* val);
static void PHD_Assoc_RelRes_Handler(APDU* val);
static void PHD_Assoc_RelReq_Handler(APDU* val);
static void PHD_OPN_STATE_PRST_APDU_Handler(APDU* val);

/*****************************************************************************
 * State Machine Function Table
 *****************************************************************************/
PHD_STATE_MC_FUNC const phd_state_mc_func[AG_MAX_STATES][AG_MAX_EVENTS] =
{
   /* PHD_AG_STATE_DISCONNECTED */
   PHD_Disconnect_Handler,PHD_Connect_Handler,                 NULL,                      NULL,                    NULL,                    NULL,                    NULL,NULL,
   /* PHD_AG_STATE_CON_UNASSOCIATED */
   PHD_Disconnect_Handler,PHD_Connect_Handler,PHD_Unhandled_Request,                      NULL,                    NULL,                    NULL,PHD_ABRT_Request_Handler,NULL,
   /* PHD_AG_STATE_CON_ASSOCIATING */
   PHD_Disconnect_Handler,PHD_Connect_Handler,PHD_Unhandled_Request,PHD_Assoc_Response_Handler,                    NULL,                    NULL,PHD_ABRT_Request_Handler,NULL,
   /* PHD_AG_STATE_CON_ASSOC_CFG_SENDING_CONFIG */
   PHD_Disconnect_Handler,PHD_Connect_Handler,PHD_Unhandled_Request,                      NULL,PHD_Assoc_RelReq_Handler,                    NULL,PHD_ABRT_Request_Handler,NULL,
   /* PHD_AG_STATE_CON_ASSOC_CFG_WAITING_APPROVAL */
   PHD_Disconnect_Handler,PHD_Connect_Handler,PHD_Unhandled_Request,                      NULL,PHD_Assoc_RelReq_Handler,                    NULL,PHD_ABRT_Request_Handler,PHD_Config_Event_Report_Handler,
   /* PHD_AG_STATE_CON_ASSOC_OPERATING */
   PHD_Disconnect_Handler,PHD_Connect_Handler,PHD_Unhandled_Request,                      NULL,PHD_Assoc_RelReq_Handler,                    NULL,PHD_ABRT_Request_Handler,PHD_OPN_STATE_PRST_APDU_Handler,
   /* PHD_AG_STATE_CON_DISASSOCIATING */
   PHD_Disconnect_Handler,PHD_Connect_Handler,PHD_Unhandled_Request,                      NULL,PHD_Assoc_RelReq_Handler,PHD_Assoc_RelRes_Handler,PHD_ABRT_Request_Handler,NULL
};

/******************************************************************************
 * User Interface
 ******************************************************************************/
typedef struct _Button_Object {
    DWORD idx;
    LPTSTR buttonName;
} Button_Object;

Button_Object g_Button_Send_Measurement   =  {0, L"BUTTON_SEND_MEASUREMENT"};
Button_Object g_Button_Disconnect         =  {1, L"BUTTON_DISCONNECT"};
Button_Object g_Button_Select_Device_Spec =  {2, L"BUTTON_SELECT_DEVICE_SPEC"};

#define BUTTON_NUMBER 3
HANDLE g_hButtons[BUTTON_NUMBER];

typedef struct _Comm_Object {
    DWORD idx;
    LPTSTR eventName;
} Comm_Object;

Comm_Object g_Comm_APP_State_Change = {0, L"APP_STATE_CHANGE"};
Comm_Object g_Comm_PHD_State_Change = {1, L"PHD_STATE_CHANGE"};
Comm_Object g_Comm_SPEC_Selected = {2, L"SPEC_SELECTED"};
Comm_Object g_Comm_Disconnected = {3, L"DEV_DISCONNECTED"};

/******************************************************************************
 * Misc Global Variables
 ******************************************************************************/
uint_8 g_phd_selected_config = 0; // Current Config

static BOOL g_phd_metadata;

static TCHAR g_szActiveKey[MAX_PATH];

CRITICAL_SECTION g_cs;

HANDLE g_htTransfers;

HANDLE g_hAppThread;

// PHD and APP state
static UCHAR g_appEvent = APP_PHD_UNINITIALISED;
static UCHAR g_phd_com_state = PHD_AG_STATE_DISCONNECTED;

// Simulation Timer Handles
static HANDLE g_hSelectTimerThread = NULL;

static PVOID g_pvInterface = NULL;

static uint_16 g_phd_ep_has_data = 0;

PIPE_TRANSFER g_rgPipeTransfers[] = 
    { { &g_hDefaultPipe }, { &g_hBIPipe }, { &g_hBOPipe }, {&g_hIIPipe} };


static LPCWSTR g_rgpszStrings0409[] = {
    g_RegInfo.szVendor, g_RegInfo.szProduct , g_RegInfo.szSerialNumber
};

static UFN_STRING_SET g_rgStringSets[] = {
    0x0409, g_rgpszStrings0409, dim(g_rgpszStrings0409)
};

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
static bool g_pipesstalled = false;
static bool g_outpipestalled = false;
#endif

LPTSTR const g_phd_state_string[] = {
    L"PHD_AG_STATE_DISCONNECTED",
    L"PHD_AG_STATE_CON_UNASSOCIATED",
    L"PHD_AG_STATE_CON_ASSOCIATING",
    L"PHD_AG_STATE_CON_ASSOC_CFG_SENDING_CONFIG",
    L"PHD_AG_STATE_CON_ASSOC_CFG_WAITING_APPROVAL",
    L"PHD_AG_STATE_CON_ASSOC_OPERATING",
    L"PHD_AG_STATE_CON_DISASSOCIATING"
};

LPTSTR const g_app_state_string[] = {
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
 *
 *****************************************************************************/

static LPTSTR getPhdStateString (
        uint_8 phd_state_idx
        )
{
    uint_8 idx = phd_state_idx & 0xf;
    return g_phd_state_string[idx];
}

static LPTSTR getAppStateString (
        uint_8 app_state_idx
        )
{
    return g_app_state_string[app_state_idx];
}

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

static
inline
VOID
ChangePhdcState(
        UCHAR phdState
        )
{
    HANDLE h_MsgChangePhdState;
    RETAILMSG(PHD_PROTOCOL_LOG, (L"\"%s\" ---------> \"%s\"\r\n", getPhdStateString(g_phd_com_state), getPhdStateString(phdState)));
    g_phd_com_state = phdState;

    // Tell App
    h_MsgChangePhdState = CreateEvent(0, FALSE, FALSE, g_Comm_PHD_State_Change.eventName);
    SetEventData(h_MsgChangePhdState, g_phd_com_state);
    SetEvent(h_MsgChangePhdState);
    CloseHandle(h_MsgChangePhdState);

    return;
}

static
inline
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

DWORD
static
WINAPI 
DefaultTransferComplete(
    PVOID pvNotifyParameter
    )
{   
    HANDLE hev = (HANDLE) pvNotifyParameter;

    DEBUGMSG(ZONE_COMMENT, (_T("DefaultTransferComplete setting event\r\n")));

    SetEvent(hev);

    return ERROR_SUCCESS;
}


// Prepare to receive data from the host.
static
VOID
PHD_SetupRx(
    PPIPE_TRANSFER pPipeTransfer,
    PBYTE pbData,
    DWORD cbData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_SetupRx"));
#endif
    

    DEBUGCHK(pbData != NULL);

    RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare to Recieve Data\r\n"));
    g_pUfnFuncs->lpIssueTransfer(g_hDevice, *pPipeTransfer->phPipe, 
        &DefaultTransferComplete, pPipeTransfer->hev, USB_OUT_TRANSFER, cbData,
        pbData, 0, NULL, &pPipeTransfer->hTransfer);

    
}


// Prepare to send data to the host.
static
VOID
PHD_SetupTx(
    PPIPE_TRANSFER pPipeTransfer,
    PBYTE pbData,
    DWORD cbData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_SetupTx"));
#endif
    

    DEBUGCHK(pbData != NULL);

    // RETAILMSG(1, (L"IssueTransfer Tx\r\n"));
    RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare to Transmit Data\r\n"));
    g_pUfnFuncs->lpIssueTransfer(g_hDevice, *pPipeTransfer->phPipe, 
        &DefaultTransferComplete, pPipeTransfer->hev, USB_IN_TRANSFER, 
        cbData, pbData, 0, NULL, &pPipeTransfer->hTransfer);                // UfnMdd_IssueTransfer

    
}

/**************************************************************************//*!
 *
 * @name  USB_Class_PHDC_Send_Data
 *
 * @brief This fucntion is used by Application to send data through PHDC class
 *
 * @param meta_data             : Packet is meta data or not
 * @param num_tfr               : Number of transfers following meta data packet
 * @param qos                   : Qos of the transfer
 * @param app_buff              : Buffer to send
 * @param size                  : Length of the transfer
 *
 * @return status
 *         USB_OK           : When Successfull
 *         Others           : Errors
 ******************************************************************************
 * This fucntion is used by Application to send data through PHDC class
 *****************************************************************************/
uint_8 USB_Class_PHDC_Send_Data (
    boolean meta_data,      /* [IN] Packet is meta data or not */
    uint_8 num_tfr,         /* [IN] Number of transfers
                                    following meta data packet */
    uint_8 qos,             /* [IN] Qos of the transfer */
    uint_8_ptr app_buff,    /* [IN] Buffer to send */
    USB_PACKET_SIZE size    /* [IN] Length of the transfer */
)
{
    UNREFERENCED_PARAMETER(num_tfr);
    UNREFERENCED_PARAMETER(qos);

    if (meta_data)
    {
        // ERIC : TO-DO
        RETAILMSG(1, (L"To do metadata processing\r\n"));
    }

    // RETAILMSG(1, (L"Will Issue IN transfer of size %d\r\n", size));

    g_phd_ep_has_data |= 1 << (BULK_IN_ENDPOINT_ADDRESS & 0xf);
    PHD_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], app_buff, size);  
    return 0;
}

/******************************************************************************
 *
 *    @name        PHD_Send_Abort_to_Manager
 *
 *    @brief       This function sends abort request to the host
 *
 *    @param       abort_reason  : Reason for abort
 *
 *    @return      None
 *
 ****************************************************************************
 * Called by the application to send abort apdu
 *****************************************************************************/
void PHD_Send_Abort_to_Manager (
    uint_16 abort_reason    /* [IN] Reason for abort */
)
{
    RETAILMSG(PHD_PROTOCOL_LOG, (L"Send Abort to Manager\r\n"));
    ChangePhdcState(PHD_AG_STATE_CON_UNASSOCIATED);
    // assoc_retry_count = PHD_ASSOC_RETRY_COUNT;    
    memcpy(g_phd_buffer, PHD_ABRT, ABRT_SIZE);
    
    /* Update the abort reason */
    g_phd_buffer[4] = (uint_8)abort_reason;
    g_phd_buffer[5] = (uint_8)((abort_reason & HIGH_BYTE_MASK) >> UPPER_BYTE_SHIFT);
    
    /* Send Abort to Manager */
    (void)USB_Class_PHDC_Send_Data(FALSE, 0, SEND_DATA_QOS, (uint_8_ptr)&g_phd_buffer, ABRT_SIZE);
}

/******************************************************************************
 *
 *    @name        PHD_Connect_to_Manager
 *
 *    @brief       This function sends the association request to the host
 *
 *    @param       controller_ID : Controller ID
 *
 *    @return      None
 *
 *****************************************************************************
 * This function is called by the application when enumeration is complete to
 * send the association request
 *****************************************************************************/
void PHD_Connect_to_Manager(
)
{
    RETAILMSG(PHD_PROTOCOL_LOG, (L"\t[PHD] Connect to Manager\r\n"));
    if((g_phd_com_state == PHD_AG_STATE_CON_UNASSOCIATED) || (g_phd_com_state == PHD_AG_STATE_CON_ASSOCIATING))
    {
        // g_phd_com_state = PHD_AG_STATE_CON_ASSOCIATING;
        ChangePhdcState(PHD_AG_STATE_CON_ASSOCIATING);
        /* Send Assoc request to Manager */
        RETAILMSG(PHD_PROTOCOL_LOG, (L"Send Connect Data\r\n"));
        (void)USB_Class_PHDC_Send_Data(FALSE, 0, SEND_DATA_QOS, (uint_8_ptr)(g_phd_cnf_param[g_phd_selected_config].AssociationReq), ASSOC_REQ_SIZE);
    }
}

/******************************************************************************
 *
 *    @name        PHD_Send_Measurements_to_Manager
 *
 *    @brief       This function sends measurements to the host
 *
 *    @return      None
 *
 *****************************************************************************
 * Called by the application to send the measurement data via event report
 *****************************************************************************/
void PHD_Send_Measurements_to_Manager(
)
{
    RETAILMSG(PHD_PROTOCOL_LOG, (L"PHD_Send_Measurements_to_Manager\r\n"));
    if(g_phd_com_state == PHD_AG_STATE_CON_ASSOC_OPERATING)
    {
        RETAILMSG(PHD_PROTOCOL_LOG, (L"Enter\r\n"));
        USB_PACKET_SIZE buff_size = (USB_PACKET_SIZE)PHD_BUFF_SIZE;
        /* if phd_buffer already in use, return */
        if(g_phd_buffer_being_used == TRUE) 
        {
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Something error\r\n"));
            return;
        }
        g_phd_buffer_being_used = TRUE;
        
        /* update measurements of the selected specialization */
        (g_phd_cnf_param[g_phd_selected_config].usb_phd_send_msr)(&g_phd_buffer, (void*)&buff_size);
        /* send measurements */
        (void)USB_Class_PHDC_Send_Data(FALSE, 0, SEND_DATA_QOS, (uint_8_ptr)g_phd_buffer, buff_size);
    }
}

/******************************************************************************
 *
 *    @name        PHD_Disconnect_from_Manager
 *
 *    @brief       This function sends the association release request to the
 *                 host
 *
 *    @return      None
 *
 ****************************************************************************
 * Called by the application to send the assciation release request
 *****************************************************************************/
void PHD_Disconnect_from_Manager(
)
{
    switch(g_phd_com_state)
    {
        case PHD_AG_STATE_CON_ASSOCIATING:
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Disconnect while ASSOCIATING\r\n"));
            ChangePhdcState(PHD_AG_STATE_CON_UNASSOCIATED);
            /* Send abort */
            PHD_Send_Abort_to_Manager((uint_16)ABORT_REASON_UNDEFINED);
            break;
        case PHD_AG_STATE_CON_DISASSOCIATING:
        case PHD_AG_STATE_CON_UNASSOCIATED:
            /* No State Change in case of Dis-Associating or UnAssociated */
            break;
        default:
            RETAILMSG(PHD_PROTOCOL_LOG, (L"default Disconnect routine\r\n"));
            ChangePhdcState(PHD_AG_STATE_CON_DISASSOCIATING);
            /* Send Association release to Manager */
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Send release association request\r\n"));
            (void)USB_Class_PHDC_Send_Data(FALSE, 0, SEND_DATA_QOS, (uint_8_ptr)(g_phd_cnf_param[g_phd_selected_config].AssociationRelReq), REL_REQ_SIZE);
            break;
    }
}
/******************************************************************************
 *
 *    @name        PHD_Disconnect_Handler
 *
 *    @brief       This function handles disconnect request
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * Sets the PHDC state to disconnected
 *****************************************************************************/
static void PHD_Disconnect_Handler(
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    UNREFERENCED_PARAMETER(val);
    RETAILMSG(1, (L"We should take care : PHD_Disconnect_Handler !!!, Not verified yet\r\n"));
    ChangePhdcState(PHD_AG_STATE_DISCONNECTED);
}

/******************************************************************************
 *
 *    @name        PHD_Connect_Handler
 *
 *    @brief       This function handles connect request
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * Sets the PHDC state to unassciated if it was disconnected
 *****************************************************************************/
static void PHD_Connect_Handler(
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    UNREFERENCED_PARAMETER(val);
    RETAILMSG(PHD_PROTOCOL_LOG, (L"PHD_Connect_Handler\r\n"));
    if (g_phd_com_state == PHD_AG_STATE_DISCONNECTED)
    {
        // assoc_retry_count = PHD_ASSOC_RETRY_COUNT;
        ChangePhdcState(PHD_AG_STATE_CON_UNASSOCIATED);
        ChangeAppState(APP_PHD_INITIALISED);
    }
}

/******************************************************************************
 *
 *    @name        PHD_Unhandled_Request
 *
 *    @brief       This function takes care of the unhandled request
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * This function should take care of any request which is not supported or is
 * illegal
 *****************************************************************************/
static void PHD_Unhandled_Request(
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    // As refrence code, not implemented
    UNREFERENCED_PARAMETER(val);
}

/******************************************************************************
 *
 *    @name        PHD_Assoc_Response_Handler
 *
 *    @brief       This function handles the assciation request response sent
 *                 by the host
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * This function parses the association request response to check whether the
 * configuration was already known to the manager or not. In case the
 * configuartion was not known, configuration event report is sent
 *****************************************************************************/
static void PHD_Assoc_Response_Handler (
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    UNREFERENCED_PARAMETER(val);
    RETAILMSG(PHD_PROTOCOL_LOG, (L"PHD_Assoc_Response_Handler\r\n"));
    if((g_phd_com_state == PHD_AG_STATE_CON_ASSOCIATING) && (val->choice == AARE_CHOSEN))
    {
        AARE_apdu *p_assoc_res = &(val->u.aare);
        RETAILMSG(PHD_PROTOCOL_LOG, (L"result is %x\r\n", p_assoc_res->result));
        if(p_assoc_res->result == ACCEPTED_UNKNOWN_CONFIG)
        {
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Will Send PHD Config\r\n"));
            /* if manager says the configuration is unknown, send configuration event report */
            ChangePhdcState(PHD_AG_STATE_CON_ASSOC_CFG_SENDING_CONFIG);
            /* send the configuration information */
            USB_Class_PHDC_Send_Data(FALSE, 0, SEND_DATA_QOS,
               (uint_8_ptr)(g_phd_cnf_param[g_phd_selected_config].ConfigEvntRpt),
               (USB_PACKET_SIZE)(g_phd_cnf_param[g_phd_selected_config].ConfigEvntRptSize));
        }
        else
        {
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Directly to OPERATING\r\n"));
            /* if the configuration is already known to the manager, enter operating state */
            ChangePhdcState(PHD_AG_STATE_CON_ASSOC_OPERATING);
        }
    }
}

/******************************************************************************
 *
 *    @name        PHD_Assoc_RelReq_Handler
 *
 *    @brief       This function handles association release request
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * This function sends a response to the association release request
 *****************************************************************************/
static void PHD_Assoc_RelReq_Handler(
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    RETAILMSG(PHD_PROTOCOL_LOG, (L"Caution !!! PHD_Assoc_RelReq_Handler, may cause problem\r\n"));
    USB_Class_PHDC_Send_Data(FALSE, 0,SEND_DATA_QOS,
                    (uint_8_ptr)(g_phd_cnf_param[g_phd_selected_config].AssociationRelRes), 
                    REL_RES_SIZE);
    // assoc_retry_count = PHD_ASSOC_RETRY_COUNT;
    ChangePhdcState(PHD_AG_STATE_CON_UNASSOCIATED);
    ChangeAppState(APP_PHD_DISCONNECTED_FROM_HOST);
    UNREFERENCED_PARAMETER(val);
}

/******************************************************************************
 *
 *    @name        PHD_Assoc_RelRes_Handler
 *
 *    @brief       This function handles association release response
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * This function handles the association release response
 *****************************************************************************/
static void PHD_Assoc_RelRes_Handler(
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    RETAILMSG(PHD_PROTOCOL_LOG, (L"PHD_Assoc_RelRes_Handler\r\n"));
    // assoc_retry_count = PHD_ASSOC_RETRY_COUNT;
    ChangePhdcState(PHD_AG_STATE_CON_UNASSOCIATED);
    ChangeAppState(APP_PHD_DISCONNECTED_FROM_HOST);
    UNREFERENCED_PARAMETER(val);
}

/******************************************************************************
 *
 *    @name        PHD_ABRT_Request_Handler
 *
 *    @brief       This function handles abrt req
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * This handles the abort request sent by the manager
 *****************************************************************************/
static void PHD_ABRT_Request_Handler(
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    UNREFERENCED_PARAMETER(val);
    // assoc_retry_count = PHD_ASSOC_RETRY_COUNT;
    ChangePhdcState(PHD_AG_STATE_CON_UNASSOCIATED);
    ChangeAppState(APP_PHD_DISCONNECTED_FROM_HOST);
}

/******************************************************************************
 *
 *    @name        PHD_Config_Event_Report_Handler
 *
 *    @brief       This function handles the response sent by the host after
 *                 config report is sent to the host for approval
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * This fucntion parses the response to the configuration event report sent
 * to check whether the configuration was accepted or rejected
 *****************************************************************************/
static void PHD_Config_Event_Report_Handler (
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    RETAILMSG(PHD_PROTOCOL_LOG, (L"PHD_Config_Event_Report_Handler\r\n"));
    if ((g_phd_com_state == PHD_AG_STATE_CON_ASSOC_CFG_WAITING_APPROVAL) && (val->choice == PRST_CHOSEN))
    {
        DATA_apdu *p_data_pdu = (DATA_apdu*)&(val->u.prst.value);
        RETAILMSG(PHD_PROTOCOL_LOG, (L"RORS choice is %x\r\n", p_data_pdu->choice.choice));
        if (p_data_pdu->choice.choice == RORS_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN)
        {
            /* confirmed */
            /* configuration accepted */
            ConfigReportRsp *p_rsp = (ConfigReportRsp*)p_data_pdu->choice.u.rors_cmipConfirmedEventReport.event_reply_info.value;

            RETAILMSG(PHD_PROTOCOL_LOG, (L"report id is %x\r\n", p_rsp->config_report_id));
            RETAILMSG(PHD_PROTOCOL_LOG, (L"config result is %x\r\n", p_rsp->config_result));
            if ((p_rsp->config_report_id == g_phd_cnf_param[g_phd_selected_config].ConfigurationVal) && (p_rsp->config_result == ACCEPTED_CONFIG))
            {
                /* if configuration accepted, enter operating state */
                ChangePhdcState(PHD_AG_STATE_CON_ASSOC_OPERATING);
                RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare for ADPU\r\n"));
                PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
            }
            else
            {
                /* configuration not accepted by the manager */
                ChangePhdcState(PHD_AG_STATE_CON_ASSOC_CFG_SENDING_CONFIG);
            }
        }
    }
}

/******************************************************************************
 *
 *    @name        PHD_OPN_STATE_PRST_APDU_Handler
 *
 *    @brief       This function handles the request/data sent by the host when
 *                 the device is in operating state
 *
 *    @param       val           : Pointer to APDU received
 *
 *    @return      None
 *
 *****************************************************************************
 * This function parses the data received and checks whether it is a response
 * to the attributes sent or the response to the measurement data
 *****************************************************************************/
static void PHD_OPN_STATE_PRST_APDU_Handler (
    APDU* val               /* [IN] Pointer to APDU received */
)
{
    UNREFERENCED_PARAMETER(val);
    RETAILMSG(PHD_PROTOCOL_LOG, (L"PHD_OPN_STATE_PRST_APDU_Handler\r\n"));
    if ((g_phd_com_state == PHD_AG_STATE_CON_ASSOC_OPERATING) && (val->choice == PRST_CHOSEN))
    {
        /* get the APDU received starting from invoke id */
        DATA_apdu *p_data_pdu = (DATA_apdu*)&(val->u.prst.value);

        RETAILMSG(PHD_PROTOCOL_LOG, (L"Choice is %x\r\n", p_data_pdu->choice.choice));
        if (p_data_pdu->choice.choice == ROIV_CMIP_GET_CHOSEN)
        {
            /* its the Get command */
            if(p_data_pdu->choice.u.roiv_cmipGet.attribute_id_list.count == 0)
            { 
                /* count 0 implies the whole MDS class */
                uint_16 invoke_id = p_data_pdu->invoke_id;

                // DisableInterrupts
                /* if phd_buffer is already in use, return */
                if(g_phd_buffer_being_used == TRUE) return;
                g_phd_buffer_being_used = TRUE;
                // EnableInterrupts

                RETAILMSG(PHD_PROTOCOL_LOG, (L"invoke_id is %x\r\n", invoke_id));
                /* copy the get attribute response into the phd_buffer */
                (void)memcpy(g_phd_buffer, g_phd_cnf_param[g_phd_selected_config].DimGetAttrRes, (size_t)g_phd_cnf_param[g_phd_selected_config].DimGetAttrResSize);
                /* get the invoke id from the get attribute request sent by manager */
                // phd_buffer[6] = (uint_8)((invoke_id >> UPPER_BYTE_SHIFT) & LOW_BYTE_MASK);
                g_phd_buffer[6] = (uint_8)invoke_id;
                // phd_buffer[7] = (uint_8)((invoke_id) & LOW_BYTE_MASK);
                g_phd_buffer[7] = (uint_8)((invoke_id & HIGH_BYTE_MASK) >> UPPER_BYTE_SHIFT);
                g_sent_resp_get_attr = TRUE;   // Used to signal that this sending is an attribute, don't need PC response
                /* Send Atributes to Manager */
                RETAILMSG(PHD_PROTOCOL_LOG, (L"Send Attribute to PC\r\n"));
                (void)USB_Class_PHDC_Send_Data(FALSE, 0, SEND_DATA_QOS, (uint_8_ptr)g_phd_buffer, (USB_PACKET_SIZE)(g_phd_cnf_param[g_phd_selected_config].DimGetAttrResSize));
            }
        }
        else if (p_data_pdu->choice.choice == RORS_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN)
        {
            /* confirmed report on completion */
            /* if the received APDU is the response to the measurements sent */
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Host Sending Confirm, means A transfer has been done in protocol layer\r\n"));
            ChangeAppState(APP_PHD_CONNECTED_TO_HOST);

#ifdef AUTO_BUTTON
            // This should be done by APP, temprorily auto issued here
            // After the first send complete, press "Change Config
            {
                HANDLE h_ChangeConfigButton;
                RETAILMSG(1, (L"[APP] wait 2s and press \"Change Config\"\r\n"));
                Sleep(2000);
                h_ChangeConfigButton = CreateEvent(0, FALSE, FALSE, g_Button_Select_Device_Spec.buttonName);
                SetEvent(h_ChangeConfigButton);
                CloseHandle(h_ChangeConfigButton);
            }
#endif
        }
    }
}

static
VOID
PHD_Callback(
    DWORD   dwMsg,
    DWORD   dwParam,
    void*   val
    )
{
    UCHAR trans_event = 0xff;

    switch(dwMsg)
    {
        case UFN_MSG_CONFIGURED:
            if (dwParam == 1)
            {
                RETAILMSG(PHD_PROTOCOL_LOG, (L"\t[PHDC] \"USB_APP_ENUM_COMPLETE\"\r\n"));
                RETAILMSG(PHD_PROTOCOL_LOG, (L"APP state is %s\r\n", getAppStateString(g_appEvent)));
                RETAILMSG(PHD_PROTOCOL_LOG, (L"PHDC state is %s\r\n", getPhdStateString(g_phd_com_state)));
                {
                    HANDLE h_MsgConfigSelected;
                    h_MsgConfigSelected = CreateEvent(0, FALSE, FALSE, g_Comm_SPEC_Selected.eventName);
                    SetEventData(h_MsgConfigSelected, g_phd_selected_config);
                    SetEvent(h_MsgConfigSelected);
                    CloseHandle(h_MsgConfigSelected);
                }
                trans_event = PHD_AG_EVT_TRANSPORT_CONNECTED;
            }
            break;
        case USB_APP_SEND_COMPLETE:
            g_phd_buffer_being_used = FALSE;
            // Update ep data status
            g_phd_ep_has_data &= (1 << dwParam);

            switch (g_phd_com_state)
            {
                case PHD_AG_STATE_CON_ASSOCIATING:
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Sending complete at ASSOCIATING\r\n"));
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare for response\r\n"));
                    PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
                    break;
                case PHD_AG_STATE_CON_ASSOC_CFG_SENDING_CONFIG:
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Sending complete at SENDING_CONFIG\r\n"));
                    ChangePhdcState(PHD_AG_STATE_CON_ASSOC_CFG_WAITING_APPROVAL);
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare for response\r\n"));
                    PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
                    break;
                case PHD_AG_STATE_CON_DISASSOCIATING:
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Sending complete at DIASSOCIATING\r\n"));
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare for response\r\n"));
                    PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
                    break;
                case PHD_AG_STATE_CON_ASSOC_OPERATING:
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Sending complete at OPERATING\r\n"));
                    if(g_sent_resp_get_attr == TRUE)
                    {
                        RETAILMSG(PHD_PROTOCOL_LOG, (L"The sent stuff is spec attribute\r\n"));
                        ChangeAppState(APP_PHD_CONNECTED_TO_HOST);
                        g_sent_resp_get_attr = FALSE;

#ifdef AUTO_BUTTON
                        // Eric This should be done by APP, but temprorily auto issued here
                        // Tell the APP thread to send the measurement
                        {
                            HANDLE h_SendMeseasurementButton;
                            Sleep(1000);
                            RETAILMSG(1, (L"Simulate APP \"Send Measurement\" Button\r\n"));
                            h_SendMeseasurementButton = CreateEvent(0, FALSE, FALSE, g_Button_Send_Measurement.buttonName);
                            SetEvent(h_SendMeseasurementButton);
                            CloseHandle(h_SendMeseasurementButton);
                        }
#endif
                    }
                    else
                    {
                        RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare for response\r\n"));
                        PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
                    }
                    break;
                default:
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Default Sending Complete\r\n"));
                    break;
            }
            break;
        case USB_APP_DATA_RECEIVED:
            trans_event = PHD_AG_EVT_TRANSPORT_APDU_RECIEVED;
            break;
        default:
            break;
    }

    if (trans_event != 0xff)
    {
        uint_8 com_state;
        APDU* p_apdu;

        if (val == NULL)
        {
            p_apdu = NULL;
        }
        else
        {
            p_apdu = (APDU*)(val);

            if(trans_event == PHD_AG_EVT_TRANSPORT_APDU_RECIEVED)
            {
                trans_event = (uint_8)(p_apdu->choice & LOW_NIBBLE_MASK);
            }
        }

        com_state = (uint_8)(g_phd_com_state & AG_PHD_STATE_MASK);
        if(phd_state_mc_func[com_state][trans_event] != NULL)
        {   
            /* incase valid event then call the function */
            RETAILMSG(PHD_PROTOCOL_LOG, (L">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
            RETAILMSG(PHD_PROTOCOL_LOG, (L"%s[%x]\r\n", getPhdStateString(g_phd_com_state), trans_event));
            (void)phd_state_mc_func[com_state][trans_event](p_apdu);
            RETAILMSG(PHD_PROTOCOL_LOG, (L"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
        }
        else
        {
            RETAILMSG(1, (L"\t[PHDC] ERROR no state machine function\r\n"));
        }
    }
}



// Read a configuration value from the registry.
// Query "pszValue" in registry and store the value in pdwResult
static
BOOL
PHD_ReadConfigurationValue(
    HKEY    hClientDriverKey,
    LPCTSTR pszValue,
    PDWORD  pdwResult,
    BOOL    fMustSucceed
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_ReadConfigurationValue"));
#endif
    
    
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbData = sizeof(DWORD);
    DWORD dwType;
    BOOL  fResult = TRUE;

    dwError = RegQueryValueEx(hClientDriverKey, pszValue, NULL, 
        &dwType, (PBYTE) pdwResult, &cbData);
    if ( (dwError != ERROR_SUCCESS) || (dwType != REG_DWORD) ) {
        if (fMustSucceed) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Failed to read %s. Error %d\r\n"), 
                pszFname, pszValue, dwError));
        }
        fResult = FALSE;
    }
    else {
        DEBUGMSG(ZONE_INIT, (_T("%s %s = 0x%x\r\n"), 
            pszFname, pszValue, *pdwResult));
    }

    

    return fResult;
}


// Configure the function controller based on registry settings.  This
// routine is not responsible for validating the data supplied by the registry.
static
BOOL
PHD_Configure(
    LPCTSTR pszActiveKey,
    HKEY hClientDriverKey
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_Configure"));
#endif
    
    UNREFERENCED_PARAMETER(hClientDriverKey);
    
    DWORD dwRet = UfnGetRegistryInfo(pszActiveKey, &g_RegInfo);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }
    
    // Adjust device descriptors
    g_HighSpeedDeviceDesc.idVendor = (USHORT) g_RegInfo.idVendor;
    g_HighSpeedDeviceDesc.idProduct = (USHORT) g_RegInfo.idProduct;
    g_HighSpeedDeviceDesc.bcdDevice = (USHORT) g_RegInfo.bcdDevice;
    
    g_FullSpeedDeviceDesc.idVendor = g_HighSpeedDeviceDesc.idVendor;
    g_FullSpeedDeviceDesc.idProduct = g_HighSpeedDeviceDesc.idProduct;
    g_FullSpeedDeviceDesc.bcdDevice = g_HighSpeedDeviceDesc.bcdDevice;

#if 0
    DWORD cStrings = dim(g_rgpszStrings0409);
    DWORD iSerialNumber = 3;
    if (g_RegInfo.szSerialNumber[0] == 0) {
        DWORD dwSuccessSerialNumber = UfnGetSystemSerialNumber(
            g_RegInfo.szSerialNumber, dim(g_RegInfo.szSerialNumber));
        
        if (dwSuccessSerialNumber != ERROR_SUCCESS) {
            // No serial number
            cStrings = dim(g_rgpszStrings0409) - 1;
            iSerialNumber = 0;
        }
    }

    g_rgStringSets[0].cStrings = cStrings;
    g_HighSpeedDeviceDesc.iSerialNumber = (UCHAR) iSerialNumber;
    g_FullSpeedDeviceDesc.iSerialNumber = (UCHAR) iSerialNumber;
    
    UfnCheckPID(&g_HighSpeedDeviceDesc, &g_FullSpeedDeviceDesc, 
        PID_MICROSOFT_MASS_STORAGE_PROTOTYPE);
#endif

    dwRet = ERROR_SUCCESS;

EXIT:
    return dwRet;
}


// Reset the transfer state of a pipe.
static
VOID
PHD_ResetPipeState(
    PUSB_PIPE_STATE pPipeState
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_ResetPipeState"));
#endif
    pPipeState->fSendingLess = FALSE;
}

// Return the configuration tree structure for the given speed.
static
PUFN_ENDPOINT
GetEndpointDescriptor(
    UFN_BUS_SPEED    Speed
    )
{
    DEBUGCHK( (Speed == BS_FULL_SPEED) || (Speed == BS_HIGH_SPEED) );
    PUFN_ENDPOINT pEndpoint;

    if (Speed == BS_HIGH_SPEED) {
        pEndpoint = &g_HighSpeedEndpoints[0];
    }
    else {
        pEndpoint = &g_FullSpeedEndpoints[0];
    }

    return pEndpoint;
}

// Process a USB Class Request.  Call Request-specific handler.
static
CONTROL_RESPONSE
PHD_HandleClassRequest(
    USB_DEVICE_REQUEST udr
    )
{
    SETFNAME(_T("PHD_HandleClassRequest"));

    CONTROL_RESPONSE response = CR_STALL_DEFAULT_PIPE;
           
    if (udr.bmRequestType == (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_HOST_TO_DEVICE) ) 
    {
        if (udr.bRequest == PHD_SET_FEATURE_REQUEST)
        {
            RETAILMSG(1, (L"PHD Class Set Feature\r\n"));
            g_phd_metadata = TRUE;
            response = CR_SUCCESS;
        }
        else if (udr.bRequest == PHD_CLEAR_FEATURE_REQUEST)
        {
            RETAILMSG(1, (L"PHD Class Set Feature\r\n"));
            g_phd_metadata = FALSE;
            response = CR_SUCCESS;
        }
        else 
        {
            ERRORMSG(1, (_T("%s Unrecognized BOT class bRequest -> 0x%x\r\n"), pszFname, udr.bmRequestType));
        }
    }
    else if (udr.bmRequestType == (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_DEVICE_TO_HOST) ) 
    {
        if (udr.bRequest == PHD_GET_STATUS_REQUEST)
        {
            RETAILMSG(1, (L"PHD Get Status\r\n"));
            PHD_SetupTx(&g_rgPipeTransfers[CONTROL_TRANSFER], (PBYTE)&g_phd_ep_has_data, sizeof(g_phd_ep_has_data));
            response = CR_SUCCESS;
        }
        else 
        {
            ERRORMSG(1, (_T("%s Unrecognized class bRequest -> 0x%x\r\n"), pszFname, udr.bmRequestType));
        }     
    }
    else 
    {
        ERRORMSG(1, (_T("%s Unrecognized class bRequest -> 0x%x\r\n"), pszFname, udr.bmRequestType));
    }
    
    return response;
}


// Process a USB Standard Request.  Call Request-specific handler.
static
VOID
PHD_HandleRequest(
    DWORD dwMsg,
    USB_DEVICE_REQUEST udr
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_HandleRequest"));
#endif

    CONTROL_RESPONSE response;

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) 
    {
        response = CR_SUCCESS; // Don't respond since it was already handled.
        
        if ( udr.bmRequestType == (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT) ) 
        {
            switch (udr.bRequest) {
                case USB_REQUEST_CLEAR_FEATURE:
                    RETAILMSG(1, (L"To Do Clear Feature\r\n"));
                    break;
                default:
                    break;
            }
        }

        else if (udr.bmRequestType == (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE) ) 
        {
            if (udr.bRequest == USB_REQUEST_SET_CONFIGURATION) 
            {
            }
        }
    }
    else 
    {
        DEBUGCHK(dwMsg == UFN_MSG_SETUP_PACKET);
        response = CR_STALL_DEFAULT_PIPE;

        if (udr.bmRequestType & USB_REQUEST_CLASS) 
        {
            DEBUGMSG(ZONE_COMMENT, (_T("%s Class request\r\n"), pszFname));
            response = PHD_HandleClassRequest(udr);
        }
    }

    if (response == CR_STALL_DEFAULT_PIPE) {
        g_pUfnFuncs->lpStallPipe(g_hDevice, g_hDefaultPipe);
        g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
    }
    else if (response == CR_SUCCESS_SEND_CONTROL_HANDSHAKE) 
    {
        g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
    }
}

static
VOID
ProcessBOPipeTransfer(
    DWORD cbTransferred
    )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(cbTransferred);

    // RETAILMSG(1, (L"To Process BO Transfer\r\n"));
    PHD_Callback(USB_APP_DATA_RECEIVED, 0, (void*)&g_rgbPhdRxBuffer[0]);
}


static
VOID
ProcessBIPipeTransfer(
    )
{
    // RETAILMSG(1, (L"To Process BI Transfer\r\n"));
    PHD_Callback(USB_APP_SEND_COMPLETE, (BULK_IN_ENDPOINT_ADDRESS & 0xf), NULL);
}

static
VOID
ProcessIIPipeTransfer(
    )
{
    // N.A.
}

// Open the pipes associated with the default interface.
static
BOOL
PHD_OpenInterface(
    )
{
    SETFNAME(_T("PHD_OpenInterface"));
    
    
    BOOL fResult = FALSE;
    PUFN_ENDPOINT pEndpoint= GetEndpointDescriptor(g_SpeedSupported);

    // Open the pipes of the associated interface.

    DEBUGCHK(g_hBOPipe == NULL);
    DEBUGCHK(g_hBIPipe == NULL);
    DEBUGCHK(g_hIIPipe == NULL);
    
    RETAILMSG(PHD_INI_LOG, (L"Open BI Pipe\r\n"));
    DWORD dwRet = g_pUfnFuncs->lpOpenPipe(g_hDevice, 
        pEndpoint[0].Descriptor.bEndpointAddress,
        &g_hBIPipe);                                    // UfnMdd_OpenPipe
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Failed to open bulk in pipe\r\n"),
            pszFname));
        goto EXIT;
    }

    RETAILMSG(PHD_INI_LOG, (L"Open BO Pipe\r\n"));
    dwRet = g_pUfnFuncs->lpOpenPipe(g_hDevice, 
        pEndpoint[1].Descriptor.bEndpointAddress,
        &g_hBOPipe);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Failed to open bulk in pipe\r\n"),
            pszFname));
        goto EXIT;
    }

    RETAILMSG(PHD_INI_LOG, (L"Open II Pipe\r\n"));
    dwRet = g_pUfnFuncs->lpOpenPipe(g_hDevice, 
        pEndpoint[2].Descriptor.bEndpointAddress,
        &g_hIIPipe);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Failed to interrupt bulk in pipe\r\n"),
            pszFname));
        RETAILMSG(1, (L"\t[PHDC] Interrupt EP Open Failed\r\n"));
        goto EXIT;
    }

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
    g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hBIPipe);
    g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hBOPipe);
    g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hIIPipe);
    g_pipesstalled = FALSE;
    g_outpipestalled = FALSE;
#endif

    fResult = TRUE;
    
EXIT:
    return fResult;
}


// Close the store
DWORD
PHD_Close(
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_Close"));
#endif
    
    // if (g_pbDataBuffer) LocalFree(g_pbDataBuffer);
    
    return ERROR_SUCCESS;
}


// Process a device event.
static
BOOL
WINAPI
PHD_DeviceNotify(
    PVOID   pvNotifyParameter,
    DWORD   dwMsg,
    DWORD   dwParam
    ) 
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pvNotifyParameter);

    EnterCriticalSection(&g_cs);
    
    SETFNAME(_T("PHD_DeviceNotify"));
    

    switch(dwMsg) {
        case UFN_MSG_BUS_EVENTS: {
            // Ensure device is in running state
            DEBUGCHK(g_hDefaultPipe);

            switch(dwParam) {
                case UFN_DETACH:
                    RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify DETACH\r\n"));
                    // Reset device
                    ChangeAppState(APP_PHD_UNINITIALISED);
                    ChangePhdcState(PHD_AG_STATE_DISCONNECTED);

                    PHD_ResetPipeState(&g_psDefaultPipeState);
                    PHD_ResetPipeState(&g_psBOPipeState);
                    PHD_ResetPipeState(&g_psBIPipeState);
                    PHD_ResetPipeState(&g_psIIPipeState);

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }

                    {
                        // Tell App
                        HANDLE h_MsgDevDisconnected;
                        h_MsgDevDisconnected = CreateEvent(0, FALSE, FALSE, g_Comm_Disconnected.eventName);
                        SetEvent(h_MsgDevDisconnected);
                        CloseHandle(h_MsgDevDisconnected);
                    }
                    break;

                case UFN_ATTACH: {
                    // Open store if not already open
                    RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify ATTACH\r\n"));

                    // Reset device
                    PHD_ResetPipeState(&g_psDefaultPipeState);
                    PHD_ResetPipeState(&g_psBOPipeState);
                    PHD_ResetPipeState(&g_psBIPipeState);
                    PHD_ResetPipeState(&g_psIIPipeState);

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }

                    break;
                }
            
                case UFN_RESET:
                    // Reset device
                    RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify RESET\r\n"));
                    PHD_ResetPipeState(&g_psDefaultPipeState);
                    PHD_ResetPipeState(&g_psBOPipeState);
                    PHD_ResetPipeState(&g_psBIPipeState);
                    PHD_ResetPipeState(&g_psIIPipeState);

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }
                    
                    if (g_hIIPipe)
                    {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hIIPipe);
                        g_hIIPipe = NULL;
                    }
                    break;

                default:
                    break;
            }

            break;
        }

        case UFN_MSG_BUS_SPEED:
            RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify SPEED\r\n"));
            g_SpeedSupported = (UFN_BUS_SPEED) dwParam;
            break;

        case UFN_MSG_SETUP_PACKET:
        case UFN_MSG_PREPROCESSED_SETUP_PACKET: {
            RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify [PREPROCESSED] SETUP PACKET\r\n"));
            PUSB_DEVICE_REQUEST pudr = (PUSB_DEVICE_REQUEST) dwParam;
            PHD_HandleRequest(dwMsg, *pudr);
            break;
        }

        case UFN_MSG_CONFIGURED:
            RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify CONFIGURED\r\n"));
            if (dwParam == 0) {
                // Reset device
                PHD_ResetPipeState(&g_psDefaultPipeState);
                PHD_ResetPipeState(&g_psBOPipeState);
                PHD_ResetPipeState(&g_psBIPipeState);
                PHD_ResetPipeState(&g_psIIPipeState);

                if (g_hBOPipe) {
                    g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                    g_hBOPipe = NULL;
                }

                if (g_hBIPipe) {
                    g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                    g_hBIPipe = NULL;
                }
                
                if (g_hIIPipe) 
                {
                    g_pUfnFuncs->lpClosePipe(g_hDevice, g_hIIPipe);
                    g_hIIPipe = NULL;
                }
            }
            else {
                RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] SET CONFIGURATION DONE\r\n"));
                DEBUGCHK(g_hBIPipe == NULL);
                PHD_OpenInterface();
            }
            break;
        default:
            break;
    }

    PHD_Callback(dwMsg, dwParam, NULL);

    LeaveCriticalSection(&g_cs);

    return TRUE;
}



DWORD
WINAPI
PHD_TransferThread(
    LPVOID lpParameter
    )
{    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_TransferThread"));
#endif

    HANDLE rghevWaits[dim(g_rgPipeTransfers)];

    for (DWORD dwIdx = 0; dwIdx < dim(g_rgPipeTransfers); ++dwIdx) {
        DEBUGCHK(g_rgPipeTransfers[dwIdx].hev);
        rghevWaits[dwIdx] = g_rgPipeTransfers[dwIdx].hev;
    }

    for (;;) 
    {
        DWORD dwWait = WaitForMultipleObjects(dim(rghevWaits), rghevWaits, FALSE, INFINITE);

        DEBUGMSG(ZONE_COMMENT, (_T("%s Transfer %u\r\n"), pszFname, dwWait));

        DWORD dwIdx = dwWait - WAIT_OBJECT_0;
            
        if (dwIdx >= dim(rghevWaits)) {
            break;
        }
        
        EnterCriticalSection(&g_cs);
        
        PPIPE_TRANSFER pPipeTransfer;
        pPipeTransfer = &g_rgPipeTransfers[dwIdx];
        DEBUGCHK(pPipeTransfer->hev);
        DEBUGCHK(pPipeTransfer->hTransfer);

        DWORD dwUsbError;
        DWORD cbTransferred;
        DWORD dwErr;
        
        dwErr = g_pUfnFuncs->lpGetTransferStatus(g_hDevice, pPipeTransfer->hTransfer, &cbTransferred, &dwUsbError);
        DEBUGCHK(dwErr == ERROR_SUCCESS);

        DEBUGMSG(ZONE_COMMENT, (_T("%s %u bytes transferred\r\n"), pszFname, cbTransferred));

        RETAILMSG(PHD_PROTOCOL_LOG, (L"%d bytes transferred\r\n", cbTransferred));

        dwErr = g_pUfnFuncs->lpCloseTransfer(g_hDevice, pPipeTransfer->hTransfer);
        DEBUGCHK(dwErr == ERROR_SUCCESS);
        pPipeTransfer->hTransfer = NULL;

        if (dwUsbError != UFN_NO_ERROR) {
            DEBUGCHK(dwUsbError == UFN_CANCELED_ERROR);
            goto CONTINUE;
        }

        // TODO: Don't do these if transfer error
        if (dwIdx == CONTROL_TRANSFER) {
            g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
        }
        else if (dwIdx == OUT_TRANSFER) {
            ProcessBOPipeTransfer(cbTransferred);
        }
        else if (dwIdx == IN_TRANSFER) {
            ProcessBIPipeTransfer();
        }
        else {
            DEBUGCHK(dwIdx == IN_TRANSFER);
            ProcessIIPipeTransfer();
        }

CONTINUE:
        LeaveCriticalSection(&g_cs);
    }

    return 0;
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
                  
          default:
              break;
    
        }
    }
}

// Initialize the Bulk-Only Transport layer.
DWORD
PHD_InternalInit(
    LPCTSTR         pszActiveKey
    )
{
    SETFNAME(_T("PHD_InternalInit"));
    
    HKEY    hClientDriverKey = NULL;
    DWORD   dwRet;
    HRESULT hr = 0;

    PREFAST_DEBUGCHK(pszActiveKey);

    RETAILMSG(1, (L"\t[PHDCPHDC] PHD_InternalInit\r\n"));
    InitializeCriticalSection(&g_cs);

    hr = StringCchCopy(g_szActiveKey, dim(g_szActiveKey), pszActiveKey);
    if (FAILED(hr)) {
        dwRet = GetLastError();
        ERRORMSG(1, (_T("%s Failed to copy device key. Error = %d\r\n"),
            pszFname, dwRet));
        goto EXIT;
    }

    hClientDriverKey = OpenDeviceKey(g_szActiveKey);
    if (hClientDriverKey == NULL) {
        dwRet = GetLastError();
        ERRORMSG(1, (_T("%s Failed to open device key. Error = %d\r\n"),
            pszFname, dwRet));
        goto EXIT;
    }

    // Configure function controller
    dwRet = PHD_Configure(pszActiveKey, hClientDriverKey);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, 
            (_T("%s Failed to configure function controller from registry\r\n"),
            pszFname));
        goto EXIT;
    }

    // initially has no data at any channel
    g_phd_ep_has_data = 0;

    // initially disable meta data feature
    g_phd_metadata = FALSE;
    
    //    g_pUfnFuncs = pUfnFuncs;

    // Register descriptor tree with device controller
    RETAILMSG(1, (L"\t[PHDC] PHD Register Descriptors\r\n"));
    dwRet = g_pUfnFuncs->lpRegisterDevice(g_hDevice,                // UfnMdd_RegisterDevice
        &g_HighSpeedDeviceDesc, &g_HighSpeedConfig, 
        &g_FullSpeedDeviceDesc, &g_FullSpeedConfig, 
        g_rgStringSets, dim(g_rgStringSets));
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Descriptor registration failed\r\n"), 
            pszFname));        
        goto EXIT;
    }

    // Mark registration
    g_fDeviceRegistered = TRUE;

    // Create pipe events
    DWORD dwTransfer;
    for (dwTransfer = 0; dwTransfer < dim(g_rgPipeTransfers); ++dwTransfer) {
        g_rgPipeTransfers[dwTransfer].hev = CreateEvent(0, FALSE, FALSE, NULL);
        if (g_rgPipeTransfers[dwTransfer].hev == NULL) {
            dwRet = GetLastError();
            ERRORMSG(1, (_T("%s Error creating event. Error = %d\r\n"), 
                pszFname, dwRet));
            goto EXIT;
        }
    }

    // Create transfer thread
    RETAILMSG(1, (L"\t[PHDC] Create PHD Thread\r\n"));
    g_htTransfers = CreateThread(NULL, 0, PHD_TransferThread, NULL, 0, NULL);
    if (g_htTransfers == NULL) {
        ERRORMSG(1, (_T("%s Transfer thread creation failed\r\n"), pszFname));
        dwRet = GetLastError();
        goto EXIT;
    }

    // Read transfer thread priority from registry
    DWORD dwTransferThreadPriority;
    if (PHD_ReadConfigurationValue(hClientDriverKey, 
        UMS_REG_TRANSFER_THREAD_PRIORITY_VAL, &dwTransferThreadPriority, FALSE))
    {
        DEBUGMSG(1,
            (_T("%s PHD transfer thread priority = %u (from registry)\r\n"), 
            pszFname, dwTransferThreadPriority));
    }
    else {
        dwTransferThreadPriority = DEFAULT_TRANSFER_THREAD_PRIORITY;
        DEBUGMSG(1, (_T("%s PHD transfer thread priority = %u\r\n"), 
            pszFname, dwTransferThreadPriority));
    }

    // Set transfer thread priority
    if (!CeSetThreadPriority(g_htTransfers, dwTransferThreadPriority)) {
        dwRet = GetLastError();
        DEBUGMSG(1,
            (_T("%s Failed to set thread priority, last error = %u; exiting\r\n"), 
            pszFname, dwRet));
        goto EXIT;
    }

    g_hAppThread = CreateThread(NULL, 0, PHDC_AppThread, NULL, 0, NULL);
    if (g_hAppThread == NULL) {
        RETAILMSG(1, (L"PHDC_APP Thread Creation Fail\r\n"));
        dwRet = GetLastError();
        goto EXIT;
    }

    // Start the device controller
    dwRet = g_pUfnFuncs->lpStart(g_hDevice, PHD_DeviceNotify, NULL,   // UfnMdd_Start
        &g_hDefaultPipe);
    if (dwRet != ERROR_SUCCESS) {
         ERRORMSG(1, (_T("%s Device controller failed to start\r\n"),
            pszFname));
        goto EXIT;
    }
    
    dwRet = ERROR_SUCCESS;    

EXIT:

    if (hClientDriverKey) {
        RegCloseKey(hClientDriverKey);
    }

    if (dwRet != ERROR_SUCCESS) {
        if (g_htTransfers) {
            CloseHandle(g_htTransfers);
        }
        // if (g_pbDataBuffer) {
        //     LocalFree(g_pbDataBuffer);
        // }
        if (g_fDeviceRegistered) {
            g_pUfnFuncs->lpDeregisterDevice(g_hDevice);
        }
    }

    
    
    return dwRet;
}

extern "C"
DWORD
Init(
    LPCTSTR pszActiveKey
    )
{
    DWORD dwErr = UfnInitializeInterface(pszActiveKey, &g_hDevice, &g_ufnFuncs, &g_pvInterface);

    if (dwErr == ERROR_SUCCESS) {
        dwErr = PHD_InternalInit(pszActiveKey);

        if (dwErr != ERROR_SUCCESS) {
            UfnDeinitializeInterface(g_pvInterface);
        }
    }

    return (dwErr == ERROR_SUCCESS);
}


extern "C" 
BOOL
Deinit(
    DWORD dwContext
    )
{
    PHD_Close();
    DEBUGCHK(g_pvInterface);
    UfnDeinitializeInterface(g_pvInterface);
    
    // Remove-W4: Warning C4100 workaround    
    UNREFERENCED_PARAMETER(dwContext);

    return TRUE;
}
