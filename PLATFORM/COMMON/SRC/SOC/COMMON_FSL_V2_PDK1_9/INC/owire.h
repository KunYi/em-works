//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  owire.h
//
//  Definitions for One-Wire Driver
//
//------------------------------------------------------------------------------

#ifndef __OWIRE_H__
#define __OWIRE_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// IOCTL to perform the reset sequence with
// reset pulse and presence pulse
#define OWIRE_IOCTL_RESET_PRESENCE_PULSE   2
// IOCTL to lock the OWIRE bus
#define OWIRE_IOCTL_BUS_LOCK               3
// IOCTL to unlock the OWIRE bus
#define OWIRE_IOCTL_BUS_UNLOCK             4

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions
HANDLE OwireOpenHandle(void);
BOOL OwireCloseHandle(HANDLE);
BOOL OwireResetPresencePulse(HANDLE);
BOOL OwireRead(HANDLE, BYTE *, DWORD);
BOOL OwireWrite(HANDLE, BYTE *, DWORD);
VOID OwireBusLock(HANDLE);
VOID OwireBusUnLock(HANDLE);

#ifdef __cplusplus
}
#endif

#endif   // __OWIRE_H__
