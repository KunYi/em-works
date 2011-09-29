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
#ifndef __ACODEC_H__
#define __ACODEC_H__
/******************************************************************************
**
**  COPYRIGHT (C) 2001, 2002 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be construed as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document. 
**  Except as permitted by such license, no part of this document may be 
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation. 
**
**  FILENAME:       xllp_acodec.h
**
**  PURPOSE: contains all audio specific macros, typedefs, and prototypes.
**           Declares no storage.
**                  
**
******************************************************************************/


//#include <bvd1.h>
//#include <bvd1bd.h>
#include "xllp_ost.h"

#include ".\xllp_gpio.h"
#include ".\xllp_i2c.h"
#include ".\xllp_ssp.h"
#include "xllp_i2s.h"
#include "xllp_defs.h"
#include "xllp_bcr.h"
#include "xllp_clkmgr.h"
#include "xllp_defs.h"

typedef unsigned int XLLP_ACODEC_DEVICE_ID_T;

#define AK_2440_ID 0x9e // 10011110 //this should be the I2C address
#define WM_8753_ID 0x34 //this should be the I2C address
#define WM_8734_ID 2 //this should be the I2C address

#define UCB_1400_ID 0x4304 //this should be the 7E register value
#define WM_9712_ID 3 //this should be the 7E register value
#define WM_9713_ID 4 //this should be the 7E register value



typedef enum _XLLP_ACODEC_OWNER_T {
	XLLP_NEARSIDE =0,
	XLLP_FARSIDE
} XLLP_ACODEC_OWNER_T;

typedef enum _XLLP_ACODEC_ERROR_T {
     
    XLLP_ACODEC_SUCCESS = 0,
	XLLP_ACODEC_GPIO_NULL,
	XLLP_ACODEC_CONTROLLER_POINTER_NULL,
	XLLP_ACODEC_CONTROLLER_INTERFACE_TIMEOUT,
	RESERVED0,
	RESERVED1,
	XLLP_ACODEC_CONTROLLER_NOT_INITIALIZED,
	XLLP_ACODEC_SAMPLERATE_INVALID,
    XLLP_ACODEC_NO_SSP_HW,
    XLLP_ACODEC_SSP_IN_USE,
    XLLP_ACODEC_NO_CODEC_SUPPORT,
	XLLP_ACODEC_NO_CODEC_ID_MATCH, //the ID passed in the context function doesn't match the codec
	XLLP_ACODEC_CODEC_FEATURE_NOT_SUPPORTED


}XLLP_ACODEC_ERROR_T;

typedef XLLP_UINT32_T XLLP_ACODEC_EQUIPMENT_T;
#define EQP_CTRL_0              0x00000001
#define EQP_CTRL_1              0x00000002
#define EQP_HIFI_DIGINT_0       0x00000004
#define EQP_HIFI_DIGINT_1       0x00000008
#define EQP_PCM_DIGINT_0        0x00000010
#define EQP_PCM_DIGINT_1        0x00000020
#define EQP_HIDAC_0             0x00000040
#define EQP_HIDAC_1             0x00000080
#define EQP_HIDAC_2             0x00000100
#define EQP_PCMDAC_0            0x00000200
#define EQP_PCMDAC_1            0x00000400
#define EQP_PCMDAC_2            0x00000800
#define EQP_HIADC_0             0x00001000
#define EQP_HIADC_1             0x00002000
#define EQP_HIADC_2             0x00004000
#define EQP_PCMADC_0            0x00008000
#define EQP_PCMADC_1            0x00010000
#define EQP_PCMADC_2            0x00020000
#define EQP_ANALOGMIXER_0       0x00040000
#define EQP_TOUCHCTRL           0x00080000
#define EQP_ADCMONITOR_0        0x00100000
#define EQP_ADCMONITOR_1        0x00200000
#define EQP_PCM_HI_Z            0x00400000


typedef struct _CONTEXT_T {
	XLLP_ACODEC_DEVICE_ID_T ACodecId;		//  - an ID that uniquely identifies the codec to be used
    volatile XLLP_GPIO_T * pGpioReg;		//  - input ptr to the GPIO registers, this value is cashed and used in subsequent calls.  Be sure to call DeInitACODEC if you deallocate this pointer

//	XLLP_ACODEC_REGS_T AcodecReg;
	volatile XLLP_I2S_T * pPCMReg;
	volatile XLLP_I2C_T * pCtrlReg;
	
	volatile XLLP_CLKMGR_T * pClockReg;  	// input ptr to the ACODEC controller registers, this value is cashed and used in subsequent calls.  Be sure to call DeInitACODEC if you deallocate this pointer
	volatile XLLP_SSP_REGS      *pSSPReg;     //pointer to the SSP port controller
	XLLP_INT32_T uMaxReadWriteTimeOutMs;	// - input the max time to wait in milliseconds before giving up on a read or write operation
	XLLP_INT32_T uMaxSetupTimeOutMs;		//  - input the maximum time in milliseconds to wait during initial setup of the ACODEC controller and codec
	XLLP_BOOL_T  bUseSecondaryCodec;		//  - input set to true if there is a secondary codec, default is false
	XLLP_OST_T * pOSTRegs;					// needed for time out

	//member functions //these pointers must be set by
	XLLP_ACODEC_ERROR_T (* g_pfnSetMasterVol)           (struct _CONTEXT_T *pDeviceContext, XLLP_UINT16_T GainInDb);
	XLLP_ACODEC_ERROR_T (* g_pfnSetMasterInputGain)     (struct _CONTEXT_T *pDeviceContext, XLLP_UINT16_T GainInDb);
	XLLP_ACODEC_ERROR_T (* g_pfnGetInSampleRate)        (struct _CONTEXT_T *pDeviceContext, XLLP_UINT16_T * RateInHz);
	XLLP_ACODEC_ERROR_T (* g_pfnGetOutSampleRate)       (struct _CONTEXT_T *pDeviceContext, XLLP_UINT16_T * RateInHz);
	XLLP_ACODEC_ERROR_T (* g_pfnSetInSampleRate)        (struct _CONTEXT_T *pDeviceContext, XLLP_UINT16_T RateInHz);
	XLLP_ACODEC_ERROR_T (* g_pfnSetOutSampleRate)       (struct _CONTEXT_T *pDeviceContext, XLLP_UINT16_T RateInHz);
	XLLP_ACODEC_ERROR_T (* g_pfnEnableSspPath)          (struct _CONTEXT_T *pDeviceContext, XLLP_UINT8_T uiDirection);
	XLLP_ACODEC_ERROR_T (* g_pfnDisableSspPath)         (struct _CONTEXT_T *pDeviceContext, XLLP_UINT8_T uiDirection);
	XLLP_ACODEC_ERROR_T (* g_pfnEnterEquipmentState)    (struct _CONTEXT_T *pDeviceContext,XLLP_ACODEC_EQUIPMENT_T equipmentState);
	XLLP_ACODEC_ERROR_T (* g_pfnQueryEquipmentState)    (struct _CONTEXT_T *pDeviceContext,XLLP_ACODEC_EQUIPMENT_T *pEquipmentState);	
	XLLP_ACODEC_ERROR_T (* g_pfnCodecSpecificInit)      (struct _CONTEXT_T *pDeviceContext);
	XLLP_ACODEC_ERROR_T (* g_pfnCodecSpecificDeInit)    (struct _CONTEXT_T *pDeviceContext);
    XLLP_ACODEC_ERROR_T (* g_pfnACodecRead)             (struct _CONTEXT_T *pDevContext, XLLP_UINT16_T regAddr, XLLP_UINT16_T *pRegvalue);
    XLLP_ACODEC_ERROR_T (* g_pfnACodecWrite)            (struct _CONTEXT_T *pDevContext, XLLP_UINT16_T regAddr, XLLP_UINT16_T regvalue);
} XLLP_ACODEC_CONTEXT_T, *P_XLLP_ACODEC_CONTEXT_T;

#ifdef __cplusplus
extern "C" {
#endif

extern XLLP_ACODEC_ERROR_T XllpACodecInit(XLLP_ACODEC_CONTEXT_T *pDeviceContext);
extern XLLP_ACODEC_ERROR_T XllpACodecDeInit(XLLP_ACODEC_CONTEXT_T *pDeviceContext);

#ifdef __cplusplus
}
#endif


#endif //__ACODEC_H__
