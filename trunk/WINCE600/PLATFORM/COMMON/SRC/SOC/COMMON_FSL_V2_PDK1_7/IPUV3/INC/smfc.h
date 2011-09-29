//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  smfc.h
//
//  Sensor Multi FIFO Controller definitions
//
//-----------------------------------------------------------------------------

#ifndef __SMFC_H__
#define __SMFC_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types
typedef enum {
    CSI_SELECT_FRAME0,
    CSI_SELECT_FRAME1,
    CSI_SELECT_FRAME2,
    CSI_SELECT_FRAME3,
} CSI_SELECT_FRAME;


//------------------------------------------------------------------------------
// Functions

BOOL SMFCRegsInit();
void SMFCRegsCleanup();
void SMFCRegsEnable(void);
void SMFCRegsDisable(void);
BOOL SMFCSetCSIMap(DWORD dwChannel,CSI_SELECT csi_sel,CSI_SELECT_FRAME csi_FrameId);
BOOL SMFCSetBurstSize(DWORD dwChannel,UINT8 iBpp, UINT8 iPFS, UINT8 iNPB);
void SMFCDumpRegs();


#ifdef __cplusplus
}
#endif

#endif //__SMFC_H__

