//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:        cspiutil.h
//
// Description: This header file defines API for CSPI utility library.
//
//------------------------------------------------------------------------------

#ifndef __CSPIUTIL_H__
#define __CSPIUTIL_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define CSPI_WRITE          1
#define CSPI_READ           0

typedef struct {
        unsigned int data:24;
        unsigned int null:1;
        unsigned int address:6;
        unsigned int rw:1;
} CSPI_PACKET32_FIELDS;


typedef union
{
    CSPI_PACKET32_FIELDS reg;
    unsigned int         data;
} CSPI_PACKET32;

//------------------------------------------------------------------------------
// Functions

BOOL cspiInitialize(int index, UINT32 dwFrequency);
VOID cspiRelease(int index);
VOID cspiConfigPolling(BOOL bPoll);
BOOL cspiAddWritePacket(UINT32 addr, UINT32 data);
BOOL cspiAddReadPacket(UINT32 addr);
BOOL cspiDataExchange();
BOOL cspiReceiveData(UINT32* data);
BOOL cspiDiscardData();
VOID cspiPowerDown(void);
VOID cspiSync(void);
VOID cspiLock(void);
VOID cspiUnlock(void);


#ifdef __cplusplus
}
#endif

#endif // __CSPIUTIL_H__
