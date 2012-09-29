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
 * Static Global Variables
 *****************************************************************************/

static BYTE g_phd_buffer[PHD_BUFF_SIZE];
/*****************************************************************************
 * State Machine Function Table
 *****************************************************************************/
static PHD_STATE_MC_FUNC const phd_state_mc_func[AG_MAX_STATES][AG_MAX_EVENTS] =
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

static LPTSTR const g_phd_state_string[] = {
    L"PHD_AG_STATE_DISCONNECTED",
    L"PHD_AG_STATE_CON_UNASSOCIATED",
    L"PHD_AG_STATE_CON_ASSOCIATING",
    L"PHD_AG_STATE_CON_ASSOC_CFG_SENDING_CONFIG",
    L"PHD_AG_STATE_CON_ASSOC_CFG_WAITING_APPROVAL",
    L"PHD_AG_STATE_CON_ASSOC_OPERATING",
    L"PHD_AG_STATE_CON_DISASSOCIATING"
};

static UCHAR g_phd_com_state = PHD_AG_STATE_DISCONNECTED;
static BOOL g_phd_buffer_being_used = FALSE;
static BOOL g_sent_resp_get_attr = FALSE;

/******************************************************************************
 * Misc Global Variables (Share with APP)
 ******************************************************************************/
uint_8 g_phd_selected_config = 0; // Current Config
Comm_Object g_Comm_PHD_State_Change = {1, L"PHD_STATE_CHANGE"};
Comm_Object g_Comm_SPEC_Selected = {2, L"SPEC_SELECTED"};
Comm_Object g_Comm_Disconnected = {3, L"DEV_DISCONNECTED"};

/******************************************************************************
 * Interface with APP
 ******************************************************************************/
extern VOID ChangeAppState(UCHAR appState);

/******************************************************************************
 * Functions
 ******************************************************************************/
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

#ifndef SHIP_BUILD
LPTSTR getPhdStateString (
        uint_8 phd_state_idx
        )
{
    uint_8 idx = phd_state_idx & 0xf;
    return g_phd_state_string[idx];
}
#endif

// APP
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
    else
    {
        RETAILMSG(PHD_PROTOCOL_LOG, (L"Don't Change State because of [%s]\r\n", getPhdStateString(g_phd_com_state)));
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
#if 1
                USB_Class_PHDC_Recieve_Data();
#else
                // PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
#endif
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
            // // This should be done by APP, temprorily auto issued here
            // // After the first send complete, press "Change Config
            // {
            //     HANDLE h_ChangeConfigButton;
            //     RETAILMSG(1, (L"[APP] wait 2s and press \"Change Config\"\r\n"));
            //     Sleep(2000);
            //     h_ChangeConfigButton = CreateEvent(0, FALSE, FALSE, g_Button_Select_Device_Spec.buttonName);
            //     SetEvent(h_ChangeConfigButton);
            //     CloseHandle(h_ChangeConfigButton);
            // }
#endif
        }
    }
}

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
                // RETAILMSG(PHD_PROTOCOL_LOG, (L"\t[PHDC] \"USB_APP_ENUM_COMPLETE\"\r\n"));
                // RETAILMSG(PHD_PROTOCOL_LOG, (L"APP state is %s\r\n", getAppStateString(g_appEvent)));
                // RETAILMSG(PHD_PROTOCOL_LOG, (L"PHDC state is %s\r\n", getPhdStateString(g_phd_com_state)));
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
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Sending Complete @ [%s]\r\n", getPhdStateString(g_phd_com_state)));
            g_phd_buffer_being_used = FALSE;

            switch (g_phd_com_state)
            {
                case PHD_AG_STATE_CON_ASSOCIATING:
#if 1
                    USB_Class_PHDC_Recieve_Data();
#else
                    // PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
#endif
                    break;
                case PHD_AG_STATE_CON_ASSOC_CFG_SENDING_CONFIG:
                    ChangePhdcState(PHD_AG_STATE_CON_ASSOC_CFG_WAITING_APPROVAL);
#if 1
                    USB_Class_PHDC_Recieve_Data();
#else
                    // PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
#endif
                    break;
                case PHD_AG_STATE_CON_DISASSOCIATING:
#if 1
                    USB_Class_PHDC_Recieve_Data();
#else
                    // PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
#endif
                    break;
                case PHD_AG_STATE_CON_ASSOC_OPERATING:
                    if(g_sent_resp_get_attr == TRUE)
                    {
                        RETAILMSG(PHD_PROTOCOL_LOG, (L"The sent stuff is spec attribute\r\n"));
                        ChangeAppState(APP_PHD_CONNECTED_TO_HOST);
                        g_sent_resp_get_attr = FALSE;

#ifdef AUTO_BUTTON
                        // // Eric This should be done by APP, but temprorily auto issued here
                        // // Tell the APP thread to send the measurement
                        // {
                        //     HANDLE h_SendMeseasurementButton;
                        //     Sleep(1000);
                        //     RETAILMSG(1, (L"Simulate APP \"Send Measurement\" Button\r\n"));
                        //     h_SendMeseasurementButton = CreateEvent(0, FALSE, FALSE, g_Button_Send_Measurement.buttonName);
                        //     SetEvent(h_SendMeseasurementButton);
                        //     CloseHandle(h_SendMeseasurementButton);
                        // }
#endif
                    }
                    else
                    {
                        RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare for response\r\n"));
#if 1
                        USB_Class_PHDC_Recieve_Data();
#else
                        // PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
#endif
                    }
                    break;
                default:
                    RETAILMSG(PHD_PROTOCOL_LOG, (L"Default Sending Complete\r\n"));
                    break;
            }
            break;
        case USB_APP_DATA_RECEIVED:
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Recieving Complete @ [%s]\r\n", getPhdStateString(g_phd_com_state)));
            trans_event = PHD_AG_EVT_TRANSPORT_APDU_RECIEVED;
#if 1
            USB_Class_PHDC_Recieve_Data();
#else
            // PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
#endif
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
        if ((com_state < AG_MAX_STATES) && (trans_event < AG_MAX_EVENTS) && (phd_state_mc_func[com_state][trans_event] != NULL))
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


// APP
extern "C"
DWORD PHD_Transport_Init(
    LPCTSTR pszActiveKey,
    LPAPP_NOTIFY pAppCallback
    )
{
    DWORD dwErr = PHD_InternalInit(pszActiveKey, PHD_Callback, pAppCallback);
    return dwErr;
}
