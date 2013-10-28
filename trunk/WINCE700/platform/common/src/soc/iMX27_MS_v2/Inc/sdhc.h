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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: sdhc.h
//
//  Provides definitions forSDHC module based on Freescale MX27
//
//------------------------------------------------------------------------------
#include "SDCardDDK.h"
#include "SDHCD.h"
#include "SDbus.hpp"
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
#define ZONEID_DMA              9
#define ZONEID_INTERRUPT        10
#define ZONEID_COMMAND          11
#define ZONEID_INFO             12
#define ZONEID_FUNCTION         13
#define ZONEID_WARN             14
#define ZONEID_ERROR            15

// Debug zone masks
#define ZONEMASK_INIT           (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT     (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL          (1<<ZONEID_IOCTL)
#define ZONEMASK_FUNCTION3  (1<<ZONEID_FUNCTION3)
#define ZONEMASK_DMA            (1<<ZONEID_DMA)
#define ZONEMASK_INTERRUPT  (1<<ZONEID_INTERRUPT)
#define ZONEMASK_COMMAND        (1<<ZONEID_COMMAND)
#define ZONEMASK_INFO           (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION       (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN           (1<<ZONEID_WARN)
#define ZONEMASK_ERROR          (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT               DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT             DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL              DEBUGZONE(ZONEID_IOCTL)
#define ZONE_FUNCTION3          DEBUGZONE(ZONEID_FUNCTION3)
#define ZONE_DMA                DEBUGZONE(ZONEID_DMA)
#define ZONE_INTERRUPT          DEBUGZONE(ZONEID_INTERRUPT)
#define ZONE_COMMAND          DEBUGZONE(ZONEID_COMMAND)
#define ZONE_INFO               DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION           DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN               DEBUGZONE(ZONEID_WARN)
#endif
#define ZONE_ERROR              1


//1 if we use DMA, 0 if use CPU
#define DMA             1

#define SDH_SLOTS                                   1 //1 SD controller supports 1 SD slot
#define SDHC_DEFAULT_CARD_CONTROLLER_PRIORITY       64
#define SDHC_DATA_BLOCK_LENGTH                      0x200
#define SDH_MAX_BLOCK_SIZE                          2048
#define SDH_MIN_BLOCK_SIZE                          1
#define SDH_RESPONSE_FIFO_DEPTH                     8
#define SDHC_DMA_PAGE_SIZE                          51200       // Size in bytes
#define SDMA_CHANNEL_PRIORITY                       5           // SDMA channel priority
#define SCL_DEBOUNCE_PERIOD                         100         // msec
#define SCL_DEBOUNCE_CHECKS                         20          // times

//Used to select one of the two hardware implementatons for card insertion/removal detection
// 0 - use the level transition on DATA3 line. Needs DATA3 to be pulled low
// 1 - use the  mechanical socket detect switch in the MMC/SD connector (routed through GPIO)
// We use the second mechanism since some MMC cards go to test mode if DATA3 is pulled low.
#define EXTERNAL_CARD_DETECT                        1 //This macro is OBSOLETE. Always follow mechanism 2

//Set to 2.8V for card detection phase
#define SLOT_VOLTAGE_MAX_BITMASK                    SD_VDD_WINDOW_2_9_TO_3_0
#define SDHC_MAX_POWER_SUPPLY_RAMP_UP               250 // SD Phys Layer 6.6

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
    int                 ControllerIstThreadPriority;    // controller IST thread priority
    DWORD               ControllerIndex ;               // index 1 and 2 for SDHC1 and SDHC2 respectively
    BOOL                DriverShutdown;                 // whether driver is shutdown or not
    CRITICAL_SECTION    ControllerCriticalSection;      // controller critical section
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
    DWORD               dwSysIntrCardDetect ;           //Card interrupt SYSINTR value
    HANDLE              hCardInsertInterruptThread;     //Card interrupt thread
    HANDLE              hCardInsertInterruptEvent;      //Card interrupt event
    DWORD               dwCardDetectIstThreadPriority;  //Card thread priority
    PVOID                           pBspSpecificContext;            // BSP specific context
    PHYSICAL_ADDRESS phySDHCAddr;

    SD_COMMAND_STATUS       SDCommandStatus ;       // Command status
    SD_RESPONSE_TYPE            LastResponedR1b;            //record last command responed type

    //DMA resource
    PHYSICAL_ADDRESS    PhysDMABufferAddr;
    UINT8       CurrentDmaChan;
    DWORD   DmaTxBufSize;
    DWORD   DmaRxBufSize;
    UINT8       DmaReqTxCH;
    UINT8       DmaReqRxCH;
    DMAC_REQUEST_SRC DmaReq;
    LPBYTE  DmaLogicalAddressTX;
    LPBYTE  DmaLogicalAddressRX;
    BOOL        fDMATransfer;                   // Indicate whether DMA is used
    volatile PSD_BUS_REQUEST      pCurrentRequest; // Current Processing Request.
    volatile BOOL                 fCurrentRequestFastPath;
    volatile SD_API_STATUS        FastPathStatus;
    BOOL                 fFastPathEnabled;             // fastpath is enabled
    DWORD                dwPollingModeSize;          // Fast path polling mode size.
}SDH_HARDWARE_CONTEXT, *PSDH_HARDWARE_CONTEXT;

#define ACQUIRE_LOCK(pHC) EnterCriticalSection(&(pHC)->ControllerCriticalSection)
#define RELEASE_LOCK(pHC) LeaveCriticalSection(&(pHC)->ControllerCriticalSection)

#define NUM_BYTE_FOR_POLLING_MODE 0x800

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
extern void BSPSlotVoltageOff(DWORD dwIndex);
extern void BSPSetVoltageSlot(UINT32 dwIndex, UINT32 mask);
extern BOOL BSPSdhcInit(void) ;
extern PVOID BspSdhcCardDetectInitialize(DWORD ControllerIndex) ;
extern BOOL BspSdhcSetIOMux(UINT32 dwIndex);
extern BOOL BspSdhcIsCardPresent (PVOID pCardDetectContext);
extern BspSdhcCardDetectDeinitialize(PVOID pCardDetectContext);
//extern void BspSdhcSetcardDetectType(PVOID pCardDetectContext, UINT8 type);
extern PVOID BspSdhcCardDetectInterruptType(DWORD ControllerIndex, BOOL  DetctType);
extern BOOL BSPSdhcIsCardWriteProtected(PVOID pCardDetectContext) ;
extern HANDLE BspSdhcRegisterCardInterrupt(DWORD *pSysIntrCardDetect) ;
extern void BspSdhcDeregisterCardInterrupt(PVOID pSpecificContext) ;

extern void  BspSdhcSetGPIO(DWORD ControllerIndex);
extern UINT8    BSPSdhcGetTxDmaChannel(DWORD dwIndex);
extern UINT8    BSPSdhcGetRxDmaChannel(DWORD dwIndex);
extern UINT32   BSPSdhcGetTxDmaBufferSize(void);
extern UINT32   BSPSdhcGetRxDmaBufferSize(void);
extern void BspSdhcEnableTransceiver(bool);

// Power handlers
void SDPowerUp(PSDCARD_HC_CONTEXT pHCContext);
void SDPowerDown(PSDCARD_HC_CONTEXT pHCContext);
void SDSetPowerState(PSDCARD_HC_CONTEXT pHCContext, CEDEVICE_POWER_STATE ds);

#ifdef __cplusplus
}
#endif

#endif // __MXARM11_SDHC_H

