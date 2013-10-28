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
/*---------------------------------------------------------------------------
* Copyright (C) 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
*--------------------------------------------------------------------------*/

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header: sdhc.h
//
//  Provides definitions forSDHC module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4201)
#include "SDCardDDK.h"
#include "SDHCD.h"
#pragma warning(pop)
#ifndef SDHC_H
#define SDHC_H

#if __cplusplus
extern "C" {
#endif

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT             0
#define ZONEID_DEINIT           1
#define ZONEID_IOCTL            2
#define ZONEID_FUNCTION3        8
#define ZONEID_FUNCTION2        9
#define ZONEID_INTERRUPT        10
#define ZONEID_FUNCTION1        11
#define ZONEID_INFO             12
#define ZONEID_FUNCTION         13
#define ZONEID_WARN             14
#define ZONEID_ERROR            15

// Debug zone masks
#define ZONEMASK_INIT           (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT         (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL          (1<<ZONEID_IOCTL)
#define ZONEMASK_FUNCTION3      (1<<ZONEID_FUNCTION3)
#define ZONEMASK_FUNCTION2      (1<<ZONEID_FUNCTION2)
#define ZONEMASK_INTERRUPT      (1<<ZONEID_INTERRUPT)
#define ZONEMASK_FUNCTION1      (1<<ZONEID_FUNCTION1)
#define ZONEMASK_INFO           (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION       (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN           (1<<ZONEID_WARN)
#define ZONEMASK_ERROR          (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT               DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT             DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL              DEBUGZONE(ZONEID_IOCTL)
#define ZONE_FUNCTION3          DEBUGZONE(ZONEID_FUNCTION3)
#define ZONE_FUNCTION2          DEBUGZONE(ZONEID_FUNCTION2)
#define ZONE_INTERRUPT          DEBUGZONE(ZONEID_INTERRUPT)
#define ZONE_FUNCTION1          DEBUGZONE(ZONEID_FUNCTION1)
#define ZONE_INFO               DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION           DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN               DEBUGZONE(ZONEID_WARN)
#endif
#define ZONE_ERROR              1

#define SDH_SLOTS                                   1 //1 SD controller supports 1 SD slot
#define SDHC_DEFAULT_CARD_CONTROLLER_PRIORITY       64
#define SDHC_DATA_BLOCK_LENGTH                      0x200
#define SDH_MAX_BLOCK_SIZE                          2048
#define SDH_MIN_BLOCK_SIZE                          1
#define SDH_RESPONSE_FIFO_DEPTH                     8
#define SDHC_DMA_BUF_DESC                                           64
#define SDHC_MAX_NUM_BLOCKS                                         65535
#define SCL_DEBOUNCE_PERIOD                         100         // msec
#define SCL_DEBOUNCE_CHECKS                         2           // times

#define CACHE_LINE_SIZE_MASK    (32 - 1)    // L1 & L2 cache lines are 32 bytes

//Cards need >= 2.8V for card detection phase
#define SLOT_VOLTAGE_MAX_BITMASK                    SD_VDD_WINDOW_2_9_TO_3_0
#define SDHC_MAX_POWER_SUPPLY_RAMP_UP               250 // SD Phys Layer 6.6

#define SD_R1B_BUSYWAIT_WORKAROUND
#define SD_BUSYWAIT_DETECT_BYPIN

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
// hardware specific context
typedef struct _SDH_HARDWARE_CONTEXT {

    PCSP_SDHC_REG       pSDMMCRegisters;                // SD/MMC controller registers
    PSDCARD_HC_CONTEXT  pHCContext;                     // the host controller context
    DWORD               dwIrqSDHC;                      // SDHC interrupt IRQ value
    DWORD               dwSysintrSDHC ;                 // SDHC interrupt SYSINTR value
    HANDLE              hControllerInterruptThread;     // controller IST thread handle
    HANDLE              hControllerInterruptEvent;      // controller IST thread event
#ifdef SD_R1B_BUSYWAIT_WORKAROUND
    HANDLE              hControllerBusyResponseThread;      // controller busy response thread handle
    HANDLE              hControllerBusyEvent;       // controller busy response thread event
    SD_TRANSFER_CLASS   CurrTransferReq;             // To determine if read or write transfer
#endif
    int                 ControllerIstThreadPriority;    // controller IST thread priority
    DWORD               ControllerIndex ;               // index 1 and 2 for SDHC1 and SDHC2 respectively
    DWORD               RelativeAddress;                // RCA of the current operational card
    BOOL                DriverShutdown;                 // whether driver is shutdown or not
    CRITICAL_SECTION    ControllerCriticalSection;      // card insertion/removal critical section
    BOOL                SendInitClocks;                 // flag to indicate that we need to send the init clock
    WCHAR               RegPath[256];                   // reg path
    BOOL                fSDIOEnabled;                   // SDIO interrupts enabled
    BOOL                f4BitMode;                      // indicates that 4 bit data transfer mode is
    BOOL                DevicePresent;                  // device is present in the slot
    BOOL                DeviceStatusChange;             // change occured in card present status
    BOOL                fWakeOnSDIOInt;                 // Indicate SDIO wakeup interrupt
    BOOL                fWakeOnCardInsInt;              // Indicate card insertion wakeup interrupt
    BOOL                fWakeOnCardRmvInt;              // Indicate card removal wakeup interrupt
    DWORD               BusWidthSetting;                // 1 bit mode or 4 bit mode
    DWORD               Units_in_fifo ;                 // number of unit in FIFO buffer
    DWORD               Bytes_in_fifo;                  // FIFO size
    BOOL                fAppCommandSent;                // Flag if CMD55 sent
    BOOL                fFakeCardRemoval;               // Whether enable fake card removal
    BOOL                fDevicePresent ;                // Whether card is present
    BOOL                fDeviceStatusChange;            // change in card presence status
    CEDEVICE_POWER_STATE PsAtPowerDown;                 // power state at PowerDown()
    CEDEVICE_POWER_STATE CurrentPowerState;             // current power state
    DWORD               dwClockRate;                    // Current card clock rate
    ULONG               ulReadTimeout;                  // ReadTimeout
    UINT32              dwVddSettingMask ;                  // Slot Vdd voltage
    UINT32              dwVoltageWindowMask ;           // Slot voltage window mask
    UINT32              dwOptVoltageMask ;                  // Slot optimum voltage
    UINT32              dwPowerUpDelay ;                // Slot power up delay

    SD_RESPONSE_TYPE    LastResponedR1b;                //record last command responed type
    UINT8               ChanSDHC ;
    DDK_DMA_REQ         CurrentDmaReq;
    DDK_DMA_REQ         DmaReqTx ;
    DDK_DMA_REQ         DmaReqRx ;
    BOOL                fDMATransfer;                   // Indicate whether DMA is used
    UINT32              DmaChainSize;                   // Size of DMA scatter-gather list
    UINT32              DmaStrandedBytes;               // Bytes in FIFO left stranded by DMA
    UINT32              DmaMinTransfer;                 // Minimum transfer size for which DMA should be used
    BOOL                fDmaError;                      // Indicates an error during DMA or not
    BOOL                fDmaBusy;                       // Indicates if DMA transfer is in progress
    BOOL                fDmaRdOpDone;                   // Indicates if read op done interrupt has been received
    BOOL                fDmaUpdateContext;              // Indicates channel context must be updated
    BOOL                fClockGatedOff;                 // TRUE - controller clock is gated off
    BOOL                fClockGatingSupported;          // TRUE - clock gating between cmds is supported
    DWORD               dwNumBytesToTransfer;           // Number of bytes (remaining) to transfer
    DWORD               dwNumWordsToTransfer;           // Number of words (remaining) to transfer
    DWORD               dwMisalignedBytesToTransfer;    // last few bytes which are less than a word
    PUCHAR              pBuffer;                        // Pointer to user data buffer
}SDH_HARDWARE_CONTEXT, *PSDH_HARDWARE_CONTEXT;

//<PK03>
extern BOOL bCsrWifiInsert;
//</PK03>

//-----------------------------------------------------------------------------
// Functions prototypes
//-----------------------------------------------------------------------------
BOOLEAN SDHCancelIoHandler(PSDCARD_HC_CONTEXT pHCContext,DWORD Slot, PSD_BUS_REQUEST pRequest);
SD_API_STATUS SDHBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext,DWORD Slot, PSD_BUS_REQUEST pRequest);
SD_API_STATUS SDHSlotOptionHandler(PSDCARD_HC_CONTEXT    pHCContext,
                                     DWORD                 SlotNumber,
                                     SD_SLOT_OPTION_CODE   Option,
                                     PVOID                 pData,
                                     ULONG                 OptionSize);
SD_API_STATUS SDDeinitialize(PSDCARD_HC_CONTEXT pHCContext);
SD_API_STATUS SDInitialize(PSDCARD_HC_CONTEXT pHCContext);
extern BOOL BSPSdhcLoadPlatformRegistrySettings( HKEY hKeyDevice );
extern BOOL BSPSdhcSetClockGatingMode(BOOL startClocks, DWORD ControllerIndex);
extern UINT32 BSPGetSDHCCLK(void) ;
extern void BSPGetVoltageSlot(UINT32 *mask, UINT32 *voltage, UINT32 *PowerUpDelay) ;
extern void BSPSlotVoltageOn(DWORD dwIndex);
extern void BSPSetVoltageSlot(UINT32 dwIndex, UINT32 mask);
extern BOOL BSPSdhcInit(DWORD ControllerIndex) ;
extern BOOL BspSdhcCardDetectInitialize(DWORD ControllerIndex) ;
extern BOOL BspSdhcSetIOMux(DWORD ControllerIndex);
extern BOOL BspSdhcIsCardPresent (DWORD ControllerIndex);
extern BspSdhcCardDetectDeinitialize(DWORD ControllerIndex);
extern void BspSdhcSetcardDetectType(DWORD ControllerIndex, UINT8 type);
extern BOOL BSPSdhcIsCardWriteProtected(DWORD ControllerIndex) ;
extern void BspSdhcDeregisterCardInterrupt(DWORD ControllerIndex) ;
extern UINT8 BspSdhcGetSdmaChannelPriority(DWORD ControllerIndex);
extern UINT BspSdhcGetSdmaChannelTx(DWORD ControllerIndex);
extern UINT BspSdhcGetSdmaChannelRx(DWORD ControllerIndex);
extern BOOL BspSdhcIsSdmaSupported(DWORD ControllerIndex);
extern UINT BspSdhcGetIrq(DWORD ControllerIndex);
extern BOOL BspSdhcIsClockGatingBetweenCmdsSupported(DWORD ControllerIndex);
extern UINT32 BspSdhcGetSdmaMinTransfer(VOID);
extern UINT32 BSPGetMaxCardCLK(void);
extern UINT32 BSPPollDataLength(void);

// Power handlers
void SDPowerUp(PSDCARD_HC_CONTEXT pHCContext);
void SDPowerDown(PSDCARD_HC_CONTEXT pHCContext);
BOOL BSPSdhcIsCardBusy(void);
#ifdef __cplusplus
}
#endif

#endif // __MXARM11_SDHC_H

