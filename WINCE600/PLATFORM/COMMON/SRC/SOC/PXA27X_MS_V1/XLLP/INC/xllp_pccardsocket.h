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
/**************************************************************************

Copyright 1999-2002 Intel Corporation

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

    Xllp_PCCardSocket.h

Abstract:

    This file declares all variables and data structures that are
    specific to the PC Card Socket XLLP API on the BULVERDE-MAINSTONE
    platform.
        
**************************************************************************/

#ifndef __XLLP_PCCARDSOCKET_H__
#define __XLLP_PCCARDSOCKET_H__

#include "xllp_defs.h"
#include "xllp_gpio.h"
#include "xllp_serialization.h"
#include "xllp_bcr.h"
#include "xllp_ost.h"
#include "xllp_memctrl.h"



//****************************************************
//
//                  SYMBOLIC CONSTANTS
//
//****************************************************

//
// PC Card interface socket definitions
//
#define XLLP_MAINSTONE_MAX_PCCARD_SOCKETS 2
#define XLLP_PCCARD_SOCKET0 0
#define XLLP_PCCARD_SOCKET1 1

//
//Supported voltage settings for PC Cards on the
//MAINSTONE reference platform
//
#define XLLP_PCCARD_3_30VOLTS 1
#define XLLP_PCCARD_5_00VOLTS 2

//
//Miscellaneous definitions
//
#define XLLP_NULL_PTR (void *)0
#define XLLP_PCCARD_MAX_READY_WAIT_TIME 2000
#define XLLP_PCCARD_READY_POLL_INTERVAL 50
#define TIMERTICK 4     //1 microsecond is 3.7 clock ticks


//****************************************************
//
//                  TYPE DEFINITIONS
//
//****************************************************

//
//PC Card Expansion Memory Timing Configuration Registers structure
//
typedef struct __Xllp_CardTiming_S
{
    XLLP_VUINT32_T MCMEM0;
    XLLP_VUINT32_T MCMEM1;
    XLLP_VUINT32_T MCATT0;
    XLLP_VUINT32_T MCATT1;
    XLLP_VUINT32_T MCIO0;
    XLLP_VUINT32_T MCIO1;

}XLLP_CARDTIMING_T, *P_XLLP_CARDTIMING_T;

//
//PC Card Socket State structure. This structure is used to reflect the current state
//of PC Card interface status bits (in the PC Card Status registers)
//
typedef struct __Xllp_PCCardSocketState_S
{
    XLLP_VUINT8_T blSocket0CDState;
    XLLP_VUINT8_T blSocket0BVD1State;
    XLLP_VUINT8_T blSocket0BVD2State;
    XLLP_VUINT8_T blSocket0IREQState;

    XLLP_VUINT8_T blSocket1CDState;
    XLLP_VUINT8_T blSocket1BVD1State;
    XLLP_VUINT8_T blSocket1BVD2State;
    XLLP_VUINT8_T blSocket1IREQState;

}XLLP_PCCARD_SOCKET_STATE_T, *P_XLLP_PCCARD_SOCKET_STATE_T;

//
//PC Card Socket Device Handle. This handle is common for all sockets, i.e.
//it is not socket-specific.
//
typedef struct __Xllp_PCCardSocket_S
{
    XLLP_GPIO_T                    *pstrGpioRegsHandle;    //Pointer to the BULVERDE GPIO registers base
    XLLP_BCR_T                     *pstrBcrHandle;         //Pointer to the MAINSTONE Board Control registers
                                                           //structure
    XLLP_PCCARD_SOCKET_STATE_T     *pstrPCCardSocketState; //Pointer to the Socket State structure
    XLLP_OST_T                     *pstrOstRegsHandle;     //Pointer to the OST registers base
    XLLP_CARDTIMING_T              *pstrCardTimingHandle;  //Pointer to the EXPANSION MEMORY TIMING
                                                           //CONFIGURATION structure
    XLLP_MEMORY_CONTROL_REGISTER_T *pstrMemCtrlRegsHandle; //Pointer to the MEMORY CONTROLLER registers base

}XLLP_PCCARDSOCKET_T, *P_XLLP_PCCARDSOCKET_T;


//****************************************************
//
//                  FUNCTION PROTOTYPES
//
//****************************************************

XLLP_STATUS_T XllpPCCardHWSetup(XLLP_PCCARDSOCKET_T *);

void          XllpPCCardConfigureGPIOs(XLLP_PCCARDSOCKET_T *);

XLLP_STATUS_T XllpPCCardGetSocketState(XLLP_PCCARDSOCKET_T *,
                                       XLLP_VUINT16_T);

XLLP_STATUS_T XllpPCCardResetSocket(XLLP_PCCARDSOCKET_T *,
                                    XLLP_VUINT16_T);

XLLP_STATUS_T XllpPCCardPowerOn(XLLP_PCCARDSOCKET_T *,
                                XLLP_VUINT16_T,
                                XLLP_UINT32_T);

XLLP_STATUS_T XllpPCCardPowerOff(XLLP_PCCARDSOCKET_T *,
                                 XLLP_VUINT16_T);

XLLP_STATUS_T XllpPCCardGetVoltageSetting(XLLP_PCCARDSOCKET_T *,
                                          XLLP_VUINT16_T,
                                          XLLP_UINT32_T *);

XLLP_STATUS_T XllpPCCardSetExpMemTiming(XLLP_PCCARDSOCKET_T *);

XLLP_STATUS_T XllpPCCardGetExpMemTiming(XLLP_PCCARDSOCKET_T *,
                                        XLLP_CARDTIMING_T *);


#endif //__XLLP_PCCARDSOCKET_H__
