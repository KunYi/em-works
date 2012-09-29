//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:       backlight.h
//  Purpose:    Definitions for backlight driver module
//
//  Notes:
//
//------------------------------------------------------------------------------


#ifndef _BACKLIGHT_H
#define _BACKLIGHT_H

//------------------------------------------------------------------------------
// MACRO DEFINITIONS 

#ifdef DEBUG
#define ZONE_INFO           DEBUGZONE(0)
#define ZONE_FUNCTION       DEBUGZONE(1)
#define ZONE_WARN           DEBUGZONE(2)
#define ZONE_ERROR          DEBUGZONE(3)
#define ZONE_INIT           DEBUGZONE(4)
#endif

// Default backlight timeout in sec
#define BKL_DEFAULT_TIMEOUT     (60)    // sec

// Backlight levels
#define BKL_LEVEL_MIN           (0)     // off
#define BKL_LEVEL_MAX           (255)   // full on
#define BKL_LEVEL_DEFAULT       ((BKL_LEVEL_MAX - BKL_LEVEL_MIN) / 2)

// Registry to Backlight Control Panel settings
#define REG_PATH                    TEXT("ControlPanel\\Backlight")

// Microsoft timeout settings
#define BATT_TIMEOUT_SUBKEY         TEXT("BatteryTimeout")
#define AC_TIMEOUT_SUBKEY           TEXT("ACTimeout")

// OEM backlight level settings
#define BATT_LEVEL_SUBKEY           TEXT("BattBacklightLevel")
#define AC_LEVEL_SUBKEY             TEXT("ACBacklightLevel")

#if (defined(BSP_POCKETPC) || defined(BSP_SMARTPHONE))
// Microsoft enable backlight restore on user activity settings
#define BATT_BACKLIGHTONTAP_SUBKEY      TEXT("BacklightOnTap")
#define AC_BACKLIGHTONTAP_SUBKEY        TEXT("ACBacklightOnTap")

// Microsoft enable backlight timeout settings
#define BATT_TO_UNCHECKED_SUBKEY        TEXT("BatteryTimeoutUnchecked")
#define AC_TO_UNCHECKED_SUBKEY          TEXT("ACTimeoutUnchecked")
#else
// Microsoft enable backlight time out registry from WinCE 
// advanced backlight dialog
#define BATT_USEBATT_SUBKEY         TEXT("UseBattery")
#define AC_USEEXT_SUBKEY            TEXT("UseExt")
#endif

//------------------------------------------------------------------------------
// ENUMERATIONS AND STRUCTURES 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _bklSettings {
    DWORD   dwBattTimeout;  // in sec
    DWORD   dwACTimeout;        // in sec
    BOOL    fBattTimeoutEnable; // 0 false, 1 true
    BOOL    fACTimeoutEnable;   // 0 false, 1 true
    BOOL    fBattBacklightOnUser;   // 1 enable backlight on user activity
    BOOL    fACBacklightOnUser; // 1 enable backlight on user activity
    DWORD   dwBattBacklightLevel;
    DWORD   dwACBacklightLevel;
} bklSettings_t;
    
// ids of events waited on by interrupt thread
typedef enum _bklWaitEvents {
    bklControlPanelEvent,
    bklPowerNotificationEvent,
    bklUserInactivityEvent,
    bklLevelChangeEvent,
    bklMaxWaitEvents,
} bklWaitEvent_c;

// backlight specific error codes
typedef enum _bklErrRegOpen {
    bklErrNone,
    bklErrRegOpen,
    bklErrGetBattTimeout,
    bklErrGetACTimeout,
    bklErrGetBattTimeoutEn,
    bklErrGetACTimeoutEn,
    bklErrGetBattOnUserEn,
    bklErrGetACOnUserEn,
    bklErrGetBattLevel,
    bklErrGetACLevel,
} bklErr_c;


#ifdef __cplusplus
    }
#endif

#endif // _BACKLIGHT_H
