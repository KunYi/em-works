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
#define SSP_CLOCK_DIVIDE_40             40 
#define SSP_CLOCK_DIVIDE_60             60 
#define SSP_CLOCK_DIVIDE_240            240 

#define SSP_CLK                         240000000   // 240MHz

#define SSP_20MHZ_FREQUENCY             20000000    // 20MHZ
#define SSP_24MHZ_FREQUENCY             24000000    // 24MHZ
#define SSP_48MHZ_FREQUENCY             48000000    // 48MHZ
#define SSP_72MHZ_FREQUENCY             72000000    // 72MHZ

#define SDHC_CD_THREAD_PRIORITY         151         // just above "below-real time" level

#define RunContext  (pContext->runContext)

typedef struct _SDHC_RUN_CONTEXT
{
    BOOL bCardPresent;        // Card Present
    BOOL bSDIOInterruptsEnabled ;        // SDIO Interrupts Enabled
    BOOL bSDBus4BitMode;        // SD Bus 4Bit Mode
    BOOL bSDBus8BitMode;        // SD Bus 4Bit Mode
    BOOL bMMCDDRMode;
    DWORD dwSysIntr;                          // sys intr for SSP hw interrupt and DMA interrupt
    HANDLE hISTEvent ;             // handle to interrupt event
    HANDLE htIST        ;             // handle to Interrupt Service Thread
    BOOL bDriverShutdown;
    BOOL bCurrentReqFastPath;
    BOOL bUseDMA;
    
    BOOL bReinsertTheCard;

    SD_API_STATUS cmdStatus;

    PVOID pv_HWregCLKCTRL;
    PVOID pv_HWregSSP   ;
    PVOID pv_HWregAPBH   ;
    PVOID pv_HWregDIGCTL ;

    HANDLE hCardDetectEvent;                // card insert/remove event
    HANDLE hCardDetectThread;        // card insert/remove thread
    DWORD  dwCardDetectSysintr;      // card insert/remove SYSINTR
    BOOL bCardDetectThreadRunning ;

    DMA_ADAPTER_OBJECT dmaAdapter;
    PVOID pVirDMADesc;                // pointer to buffers used for DMA transfers
    PVOID pPhyDMADesc;                // pointer to buffers used for DMA transfers
    PBYTE            pDMABuffer;                  // pointer to buffers used for DMA transfers
    PHYSICAL_ADDRESS pDMABufferPhys;              // physical address of the SMA buffer

    DWORD                            dwSSPIndex;

    UINT8 uChannel;                             // SDMMC DMA Channel, SSP1 or SSP2
	//LQK:Jul-18-2012 
	DWORD dwDetection;	//add SD-card detection  polarity
	DWORD dwPowerPin;	//add SD-card Power control
    
} SDHC_RUN_CONTEXT, *PSDHC_RUN_CONTEXT;

// hardware specific context
typedef struct _SDHC_HARDWARE_CONTEXT {

   PSDCARD_HC_CONTEXT   pHCContext;                    // the host controller context
   SDHC_RUN_CONTEXT       runContext;
   
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
void BSPSDHCDeinit(void *pContext);
BOOL BSPSDHCSysIntrInit(DWORD dwIndex, PDWORD pdwSysIntr);
BOOL BSPSDHCIsCardPresent(DWORD dwIndex);
BOOL BSPSDHCIsWriteProtected(DWORD dwIndex);
UINT32 BSPSDHCGetDMADescAddress(DWORD dwIndex, DWORD dwDescSize);
BOOL BSPGetCardDetectIRQ(DWORD dwIndex, PDWORD pIRQ);
BOOL BSPSDHCCardDetectThread(void *pHardwareContext);
BOOL BSPSDHCSupport8Bit(DWORD dwIndex);
BOOL BSPSDHCSupportDDRMode(DWORD dwIndex);
VOID BSPSDHCDisableDDRMode(void *pContext);
VOID BSPSDHCEnableDDRMode(void *pContext);

#endif // _SDHC_DEFINED


