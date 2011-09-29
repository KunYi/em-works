//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  sdhc.h
//
//------------------------------------------------------------------------------

#ifndef _SDHC_DEFINED
#define _SDHC_DEFINED

#pragma warning(push)
#pragma warning(disable: 4127 4201)
#include <windows.h>
#include <ceddk.h>
#include <devload.h>
#include <sdcardddk.h>
#include <sdhcd.h>
#include <nkintr.h>
#pragma warning(pop)

#include "csp.h"

#define SDHC_MAX_BLOCK_SIZE             4096
#define SDHC_MIN_BLOCK_SIZE             1

// CLOCK_DIVIDE : 2~254
#define SSP_CLOCK_DIVIDE_2              2 
#define SSP_CLOCK_DIVIDE_4              4
#define SSP_CLOCK_DIVIDE_60             60 
#define SSP_CLOCK_DIVIDE_240            240 


#define SSP_24MHZ_FREQUENCY             24000       // 24000 KHZ
#define SSP_48MHZ_FREQUENCY             48000       // 48000 KHZ
#define SSP_72MHZ_FREQUENCY             72000       // 72000 KHZ



// STMP378x hardware specific context
typedef struct _SDHC_HARDWARE_CONTEXT {

   PSDCARD_HC_CONTEXT   pHCContext;                    // the host controller context
   
}SDHC_HARDWARE_CONTEXT, *PSDHC_HARDWARE_CONTEXT;

// prototypes for handlers
BOOLEAN SDHCCancelIoHandler(PSDCARD_HC_CONTEXT pHCContext,DWORD Slot, PSD_BUS_REQUEST pRequest);
SD_API_STATUS SDHCBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext,DWORD Slot, PSD_BUS_REQUEST pRequest);
SD_API_STATUS SDHCSlotOptionHandler(PSDCARD_HC_CONTEXT  pHCContext,
                                    DWORD               SlotNumber, 
                                    SD_SLOT_OPTION_CODE Option, 
                                    PVOID               pData,
                                    ULONG               OptionSize);

// other prototypes
SD_API_STATUS SDDeinitialize(PSDCARD_HC_CONTEXT pHCContext);
SD_API_STATUS SDInitialize(PSDCARD_HC_CONTEXT pHCContext);
BOOL  FslSDHCIsEthKitlEnable();
VOID HandleInterrupts(PSDCARD_HC_CONTEXT pHCContext);

// platform specific functions
BOOL BSPSDHCInit(void *pContext);
BOOL BSPSDHCSysIntrInit(PDWORD pdwSysIntr);
void BSPSDHCDeinit();
BOOL BSPSDHCIsCardPresent();
BOOL BSPSDHCIsWriteProtected();
UINT8 BSPSDHCGetDMAChannel();
UINT32 BSPSDHCGetDMADescAddress();


#endif // _SDHC_DEFINED


