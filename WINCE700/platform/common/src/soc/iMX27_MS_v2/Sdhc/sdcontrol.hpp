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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Module Name:
//     sdcontrol.hpp
// Abstract:
//     Definition for the sdcontrol.
//
//
//
// Notes:
//
//
#ifndef _SDHC_CONTROL
#define _SDHC_CONTROL

#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>

#include "csp.h"
#include "sdhc.h"
#include "sdbus.hpp"
#include "sddevice.hpp"
#include "sdslot.hpp"
#include "sdbusreq.hpp"

static DWORD SDControllerBusyWaitResponse(PSDH_HARDWARE_CONTEXT pHCDevice);
static DWORD SDControllerIstThread(PSDH_HARDWARE_CONTEXT pHCDevice);
static void IndicateBusRequestComplete(PSDH_HARDWARE_CONTEXT pController,
                                       PSD_BUS_REQUEST pRequest,
                                       SD_API_STATUS status);
static BOOL HandleCommandComplete(PSDH_HARDWARE_CONTEXT pController);
static BOOL HandleTransferDone(PSDH_HARDWARE_CONTEXT pController);
static void SetRate(PSDH_HARDWARE_CONTEXT pHc, PDWORD pRate, BOOL fSetNewRate);
static void SetClock(PSDH_HARDWARE_CONTEXT pController, BOOL Start) ;
static BOOL Drivstrset(DWORD dwIndex);
static void SoftwareReset(PSDH_HARDWARE_CONTEXT pController, BOOL bResetClock);
static void SetHardwarePowerState(PSDH_HARDWARE_CONTEXT pHc, CEDEVICE_POWER_STATE ds);
static void ResetCard(SDH_HARDWARE_CONTEXT *pController);
static void ProcessCardInsertion(void *pContext);
static void ProcessCardRemoval(void *pContext);
static void InitGlobals(PSDH_HARDWARE_CONTEXT pController);
static inline BOOL TransferIsSDIOAbort(PSD_BUS_REQUEST pRequest);
static DWORD SDControllerBusyWaitResponse(PSDH_HARDWARE_CONTEXT pHCDevice);
#if DMA
static BOOL InitDMA(PSDH_HARDWARE_CONTEXT pController) ;
static BOOL DeInitDMA(PSDH_HARDWARE_CONTEXT pController) ;
#endif //DMA
static BOOL SetupCardDetectIST(PSDH_HARDWARE_CONTEXT pHardwareContext) ;
static void CleanupCardDetectIST(PSDH_HARDWARE_CONTEXT pHardwareContext);
static void DumpRegisters(PSDH_HARDWARE_CONTEXT pHc);


#endif //_SDHC_CONTROL
