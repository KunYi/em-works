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

unimodem.h

Abstract:  

Device specific header for unimodem TSPI

Notes: 


--*/

#ifndef _UNIMODEM_H_
#define _UNIMODEM_H_

#ifdef __cplusplus
extern "C" {
#endif

// DeviceType defines  -- Moved here from modem.h
//
#define DT_NULL_MODEM       0
#define DT_EXTERNAL_MODEM   1
#define DT_INTERNAL_MODEM   2
#define DT_PCMCIA_MODEM     3
#define DT_PARALLEL_PORT    4
#define DT_PARALLEL_MODEM   5
#define DT_IRCOMM_MODEM     6
#define DT_DYNAMIC_MODEM    7
#define DT_DYNAMIC_PORT     8

// The following info is returned from the unimodem GetDevCaps call as the
// device specific data.  The wDeviceType indicates the type of device on
// the port, and the wActive field indicates if the port is currently active.
// This field will normally be 1 for all TAPi device except PCMCIA cards which
// have been removed from the system.
// dwPPPMTU is the maximum transfer unit size in bytes that PPP will use.
// For some media, a size smaller than the default 1500 is more efficient.
typedef struct  UNIMODEM_INFO {
    WORD        wDeviceType;
    WORD        wActive;
    DWORD       dwPPPMTU;
} UNIMODEM_INFO, * PUNIMODEM_INFO;

//
// lineDevSpecific definitions for unimodem.
// The first DWORD of the lpParams block is the command code.
//
#define UNIMDM_CMD_CHG_DEVCFG 0x00000001
#define UNIMDM_CMD_GET_DEVCFG 0x00000002

//
// UNIMDM_CMD_CHG_DEVCFG:
//
// lineDevSpecific - Change device configuration:
// This interface enables a programmatic mechanism for editing a unimodem
// devconfig. (The other mechanism requires user response to a dialog.)
// Changes are applied to the devconfig structure only and do not affect the 
// associated line device's current state.
//
// Example:
//
// PUNIMDM_CHG_DEVCFG pUniMdmChgDevCfg;
//
//  ... (allocate and set up pUniMdmChgDevCfg fields) ...
//
// pUniMdmChgDevCfg->dwCommand = UNIMDM_CMD_CHG_DEVCFG;
//
// rc = lineDevSpecific(
//          hLine,
//          0,
//          NULL,
//          pUniMdmChgDevCfg,
//          sizeof(UNIMDM_CHG_DEVCFG));
//

typedef struct _UNIMDM_CHG_DEVCFG {
    DWORD dwCommand;
    LPCWSTR lpszDeviceClass;
    LPVARSTRING  lpDevConfig; // IN/OUT
    DWORD dwOption;
    DWORD dwValue;
} UNIMDM_CHG_DEVCFG, * PUNIMDM_CHG_DEVCFG;

//
// Values for UNIMDM_CHG_DEVCFG.dwOption
//
#define UNIMDM_OPT_BAUDRATE     1 // use CBR_* values from winbase.h
#define UNIMDM_OPT_BYTESIZE     2
#define UNIMDM_OPT_PARITY       3 // use values from winbase.h
#define UNIMDM_OPT_STOPBITS     4 // use values from winbase.h
#define UNIMDM_OPT_WAITBONG     5 // Seconds to wait for prompt tone
#define UNIMDM_OPT_MDMOPTIONS   6 // use MDM_* values from mcx.h
                                  // Currently MDM_BLIND_DIAL, MDM_FLOWCONTROL_HARD,
                                  // MDM_FLOWCONTROL_SOFT and MDM_SPEED_ADJUST are supported.
                                  // MDM_SPEED_ADJUST enables automatic baud rate detection
                                  // on a DCC client connection.  
#define UNIMDM_OPT_TIMEOUT      7 // call setup fail timer in seconds
#define UNIMDM_OPT_TERMOPTIONS  8 // use NETUI_LCD_TRMOPT_* values from netui.h
#define UNIMDM_OPT_DIALMOD      9 // dial modifier (dwValue is a LPCWSTR)
#define UNIMDM_OPT_DRVNAME      10 // driver name needed to load a dynamic modem (dwValue is a LPCWSTR)
#define UNIMDM_OPT_CFGBLOB      11 // Configuration data for dynamic modem dwValue is a BYTE[MAX_CFG_BLOB]

//
// UNIMDM_CMD_GET_DEVCFG:
//
// Uses same structure and options as UNIMDM_CMD_CHG_DEVCFG except that
// dwValue is filled in with the requested option's value.
//

#ifdef __cplusplus
}
#endif

#endif _UNIMODEM_H_
