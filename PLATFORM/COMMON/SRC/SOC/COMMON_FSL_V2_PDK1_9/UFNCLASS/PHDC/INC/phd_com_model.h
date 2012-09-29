//------------------------------------------------------------------------------
//
// Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 

#ifndef _PHD_COM_MODEL_H
#define _PHD_COM_MODEL_H

#include "types_phdc.h"
#include "user_config_phdc.h"
#include "ieee11073_phd_types.h"
#include "ieee11073_nom_codes.h"
#include "usb_phdc.h"

#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(push)
/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
#define SEND_DATA_QOS                                   (0x88)
/* Agent states */
#define  PHD_AG_STATE_DISCONNECTED                      0x00
#define  PHD_AG_STATE_CON_UNASSOCIATED                  0x11
#define  PHD_AG_STATE_CON_ASSOCIATING                   0x12
#define  PHD_AG_STATE_CON_ASSOC_CFG_SENDING_CONFIG      0x73
#define  PHD_AG_STATE_CON_ASSOC_CFG_WAITING_APPROVAL    0x74
#define  PHD_AG_STATE_CON_ASSOC_OPERATING               0x35
#define  PHD_AG_STATE_CON_DISASSOCIATING                0x16

/* Agent event */
#define  PHD_AG_EVT_TRANSPORT_DISCONNECTED       0x00
#define  PHD_AG_EVT_TRANSPORT_CONNECTED          0x01

/* apdu received events */
#define  PHD_AG_EVT_ASSOC_REQ_RECIVED            0x02
#define  PHD_AG_EVT_ASSOC_RES_RECIVED            0x03
#define  PHD_AG_EVT_ASSOC_REL_REQ_RECIVED        0x04
#define  PHD_AG_EVT_ASSOC_REL_RES_RECIVED        0x05
#define  PHD_AG_EVT_ASSOC_ABRT_RECIVED           0x06
#define  PHD_AG_EVT_PRESENTATION_RECIVED         0x07

/* transport send/receive event */
#define  PHD_AG_EVT_TRANSPORT_APDU_RECIEVED      0x80
#define  PHD_AG_EVT_TRANSPORT_SENT_COMPLETED     0x81

#define  AG_MAX_STATES                      7
#define  AG_MAX_EVENTS                      0x08
#define PHD_ASSOC_RETRY_COUNT               3

#define  AG_PHD_STATE_MASK                  0x0f

#define USB_APP_DATA_RECEIVED 0x81
#define USB_APP_SEND_COMPLETE 0x82

/* Events sent to application layer */
#define APP_PHD_UNINITIALISED               0
#define APP_PHD_INITIALISED                 1
#define APP_PHD_CONNECTED_TO_HOST           2
#define APP_PHD_DISCONNECTED_FROM_HOST      3
#define APP_PHD_MEASUREMENT_SENT            4
#define APP_PHD_MEASUREMENT_SENDING         5
#define APP_PHD_ASSOCIATION_TIMEDOUT        6
#define APP_PHD_ERROR                       7
#define APP_PHD_INITIATED                   8
#define APP_PHD_SELECT_TIMER_STARTED        9
#define APP_PHD_DISCONNECTING               10
#define APP_PHD_SELECT_TIMER_OFF            11
#define APP_PHD_END                         20
#define INVALID_TIMER_VALUE                 0xFF

/* PHD Timeouts */
#define PHD_ASSOCIATION_TIMEOUT             10000   /* 10 sec Timeout */
#define PHD_CONFIGURATION_TIMEOUT           10000   /* 10 sec Timeout */
#define PHD_ASSOC_RELEASE_TIMEOUT           3000    /* 3 sec Timeout */
#define PHD_DEFAULT_RESPONSE_TIMEOUT        3000    /* 3 sec Timeout */ 

/* Request/Response Sizes */
#define ASSOC_REQ_SIZE                      54
#define REL_REQ_SIZE                        6
#define REL_RES_SIZE                        6
#define ABRT_SIZE                           6

#define UPPER_BYTE_SHIFT                    8
#define LOW_NIBBLE_MASK                     0x0f
#define LOW_BYTE_MASK                       0xff
#define HIGH_BYTE_MASK                      0xff00

/* APDU Header Size */
#define APDU_HEADER_SIZE                    4

/* Global Variable */
extern uint_8 const                         PHD_ABRT[ABRT_SIZE];
/*****************************************************************************
 * Types
 *****************************************************************************/
#pragma pack(1)

/* callback function pointer structure for Application to handle events */
typedef void (*PHD_STATE_MC_FUNC)(APDU*);

/* callback function pointer structure for Application to handle events */
typedef void (*PHD_CALLBACK)(uint_8);

typedef void (*USB_PHD_SEND_MEASUREMENTS)(void*, void*);

typedef uint_8_ptr      uchar_ptr;           /* ptr to 8-bit*/
typedef struct _phd_cnf_param
{
    uint_8_ptr AssociationReq;
    uint_8_ptr ConfigEvntRpt;
    uint_32 ConfigEvntRptSize;
    uint_8_ptr AssociationRelReq;
    uint_8_ptr AssociationRelRes;
    uint_8_ptr DimGetAttrRes;
    uint_32 DimGetAttrResSize;
    uint_16 ConfigurationVal;
    USB_PHD_SEND_MEASUREMENTS usb_phd_send_msr;
}PHD_CNF_PARAM, *PHD_CNF_PARAM_PTR;
// #pragma options align=reset

typedef struct _Comm_Object {
    DWORD idx;
    LPTSTR eventName;
} Comm_Object;

/*****************************************************************************
 * Global variables
 *****************************************************************************/
extern PHD_CNF_PARAM const g_phd_cnf_param[MAX_DEV_SPEC_SUPPORTED];
extern Comm_Object g_Comm_PHD_State_Change;
extern Comm_Object g_Comm_SPEC_Selected;
extern Comm_Object g_Comm_Disconnected;
extern uint_8 g_phd_selected_config;
/*****************************************************************************
 * Global Functions
 *****************************************************************************/
extern void PHD_Connect_to_Manager();
extern void PHD_Disconnect_from_Manager();
extern void PHD_Send_Measurements_to_Manager();
extern void PHD_Send_Abort_to_Manager(uint_16 abort_reason);
extern VOID ChangePhdcState(UCHAR phdState);
extern DWORD PHD_Transport_Init(
    LPCTSTR pszActiveKey,
    LPAPP_NOTIFY pAppCallback
    );
extern LPTSTR getPhdStateString(uint_8 phd_state_idx);

#pragma pack(pop)
#ifdef __cplusplus
}
#endif
#endif
