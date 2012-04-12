//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  owire_priv.h
//
//  Private definitions for One-Wire Driver
//
//------------------------------------------------------------------------------

#ifndef __OWIRE_PRIV_H__
#define __OWIRE_PRIV_H__

//------------------------------------------------------------------------------
// Defines

#define OWIRE_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+++%s\r\n"), __WFUNCTION__))
#define OWIRE_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("---%s\r\n"), __WFUNCTION__))

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT         0
#define ZONEID_DEINIT       1
#define ZONEID_IOCTL        2
#define ZONEID_READWRITE    3

#define ZONEID_INFO         12
#define ZONEID_FUNCTION     13
#define ZONEID_WARN         14
#define ZONEID_ERROR        15

// Debug zone masks
#define ZONEMASK_INIT       (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT     (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL      (1<<ZONEID_IOCTL)
#define ZONEMASK_READWRITE  (1<<ZONEID_READWRITE)

#define ZONEMASK_INFO       (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION   (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN       (1<<ZONEID_WARN)
#define ZONEMASK_ERROR      (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT           DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT         DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL          DEBUGZONE(ZONEID_IOCTL)
#define ZONE_READWRITE      DEBUGZONE(ZONEID_READWRITE)

#define ZONE_INFO           DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION       DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN           DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR          DEBUGZONE(ZONEID_ERROR)
#endif

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions
BOOL OwireInitialize();
void OwireDeinit();
BOOL OwireWriteData(BYTE*, DWORD);
BOOL OwireReadData(BYTE*, DWORD);
BOOL OwireSetResetPresencePulse();
VOID OwireLock();
VOID OwireUnLock();

#endif   // __OWIRE_PRIV_H__
