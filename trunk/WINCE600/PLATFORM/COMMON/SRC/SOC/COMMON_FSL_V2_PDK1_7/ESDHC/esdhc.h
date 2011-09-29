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
//
// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE
//

//------------------------------------------------------------------------------
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Header: esdhc.h
//
//  Provides class definition for the Freescale eSDHC module.
//
//------------------------------------------------------------------------------

#ifndef __ESDHC_H
#define __ESDHC_H

#pragma warning(push)
#pragma warning(disable: 4100 4127 4189 4201 4214 6001 6385)
#include <windows.h>
#include <pm.h>
#include <ceddk.h>
#include <devload.h>
#include <SDCardDDK.h>
#include <SDHCD.h>
#include <creg.hxx>
#include <Nkintr.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_esdhc.h"
#include "esdhcdma.hpp"

#ifndef SHIP_BUILD
#  define STR_MODULE _T("ESDHC!")
#  define SETFNAME(name) LPCTSTR pszFname = STR_MODULE name _T(":")
#else
#define SETFNAME(name)
#endif

#define ENABLE_DEBUG

// ESDHC hardware specific context
typedef class CESDHCBase
{
public:

    CESDHCBase();
     ~CESDHCBase();

    BOOL Init( LPCTSTR pszActiveKey );
    VOID FreeHostContext( BOOL fRegisteredWithBusDriver, BOOL fHardwareInitialized );
    BOOL IOControl(DWORD dwCode,
                   BYTE *pInBuffer,
                   DWORD inSize,
                   BYTE *pOutBuffer,
                   DWORD outSize,
                   DWORD *pOutSize);

    // static callback handlers
    static SD_API_STATUS SDHCDeinitialize(PSDCARD_HC_CONTEXT pHCContext);
    static SD_API_STATUS SDHCInitialize(PSDCARD_HC_CONTEXT pHCContext);
    static BOOLEAN SDHCCancelIoHandler( PSDCARD_HC_CONTEXT pHCContext,
                                        DWORD Slot,
                                        PSD_BUS_REQUEST pRequest );
    static SD_API_STATUS SDHCBusRequestHandler( PSDCARD_HC_CONTEXT pHCContext,
                                                DWORD Slot,
                                                PSD_BUS_REQUEST pRequest );
    static SD_API_STATUS SDHCSlotOptionHandler( PSDCARD_HC_CONTEXT pHCContext,
                                                DWORD SlotNumber,
                                                SD_SLOT_OPTION_CODE Option,
                                                PVOID pData,
                                                ULONG OptionSize );

    // Static stub that calls ControllerIstThread().
    static DWORD WINAPI ISTStub(LPVOID lpParameter) {
        PCESDHCBase pController = (PCESDHCBase) lpParameter;
        return pController->ControllerIstThread();
    }


    // member functions: called by callbacks above
    SD_API_STATUS Stop();
    SD_API_STATUS Start();
    BOOLEAN CancelIoHandler( PSD_BUS_REQUEST pRequest);
    SD_API_STATUS BusRequestHandler( PSD_BUS_REQUEST pRequest ) ;
    SD_API_STATUS SlotOptionHandler(SD_SLOT_OPTION_CODE Option, PVOID pData, ULONG OptionSize );

    
     VOID IndicateBusRequestComplete( PSD_BUS_REQUEST pRequest, SD_API_STATUS Status );

    // chip specific functions
     DWORD CspESDHCGetBaseAddr(DWORD dwIndex);
     DWORD CspESDHCGetIRQ(DWORD dwIndex);
    
    // platform specific functions
     BOOL    BspGetRegistrySettings( CReg *pReg );
     ULONG  BspESDHCGetBaseClk(void);
     BOOL    BspESDHCInit();
     VOID    BspESDHCDeinit();
     BOOL    BspESDHCSysIntrSetup();
     VOID    BspESDHCSetClockGating(DWORD dwPowerState);
     VOID    BspESDHCSetSlotVoltage(DWORD dwVoltage);
     BOOL    BspESDHCIsCardPresent();
     BOOL    BspESDHCSlotStatusChanged();
     VOID     BspESDHCCardDetectInt(BOOL fDetectInsertion);
     BOOL    BspESDHCIsWriteProtected();
     

    // helper functions
     BOOL    InterpretCapabilities();
    VOID    SetInterface(PSD_CARD_INTERFACE pInterface);
    VOID    EnableSDIOInterrupts();
    VOID    DisableSDIOInterrupts();
    inline BOOL TransferIsSDIOSuspend(UINT8 Cmd, UINT32 Arg);
    inline BOOL TransferIsSDIOAbort(UINT8 Cmd, UINT32 Arg);
    SD_API_STATUS SendCommand(PSD_BUS_REQUEST pRequest);
    BOOL    CommandCompleteHandler();
    BOOL    SDIReceive(PBYTE pBuff, DWORD dwLen);
    BOOL    SDITransmit(PBYTE pBuff, DWORD dwLen);
    BOOL    SDIPollingReceive(PBYTE pBuff, DWORD dwLen);
    BOOL    SDIPollingTransmit(PBYTE pBuff, DWORD dwLen);
    VOID SoftwareReset();
    VOID SoftwareResetSDIO();

    // Interrupt handling methods
    VOID HandleRemoval(BOOL fCancelRequest);
    VOID HandleInsertion();
    VOID HandleCommandErrors();
    VOID HandleDataErrors();
    VOID HandleReadReady();
    VOID HandleWriteReady();
    VOID HandleTransferComplete();
    VOID HandleInterrupts();

    VOID ClockGateOn();
    VOID ClockGateOff();
    VOID SetClockRate(PDWORD pdwRate);

    // IST functions
    DWORD ControllerIstThread();
    DWORD DataTransferIstThread(LPVOID lpParameter);

    PVOID SlotAllocDMABuffer(ULONG Length,PPHYSICAL_ADDRESS  LogicalAddress,BOOLEAN CacheEnabled );
    BOOL SlotFreeDMABuffer( ULONG Length,PHYSICAL_ADDRESS  LogicalAddress,PVOID VirtualAddr,BOOLEAN CacheEnabled );

#ifdef ENABLE_DEBUG
    VOID DumpRegisters();
#else
    VOID DumpRegisters() {}
#endif

    PSDCARD_HC_CONTEXT   m_pHCContext;                  // the host controller context
    LPCTSTR              m_pszActiveKey;                //
    CEDEVICE_POWER_STATE m_PowerState;                  // Powerstate for SDHC device
    DWORD              m_dwControllerIndex;           // ESDHC instance in the SOC ( 1,2...,#of slots)

    BOOL                 m_fCommandCompleteOccurred;
    BOOL                 m_bReinsertTheCard;            // force card insertion event
    BOOL                m_fWakeupSource;             // should this slot be able to wakeup system from suspend state?
    DWORD                m_dwCurrentPowerLevel; // current voltage setting

    BOOL                 m_fCardPresent;                // a card is inserted and initialized
    BOOL                 m_fSDIOInterruptsEnabled;      // TRUE - indicates that SDIO interrupts are enabled

    HANDLE               m_hControllerISTEvent;         // SDIO/controller interrupt event
    HANDLE               m_htControllerIST;             // SDIO/controller interrupt thread

    DWORD                m_dwSDIOPriority;              // SDIO IST priority
    BOOL                 m_fDriverShutdown;             // controller shutdown
    DWORD                m_dwControllerSysIntr;         // controller interrupt
    BOOL                 m_fInitialized;                // driver initialized

    DWORD                m_dwMaxClockRate;              // host controller's clock base
    USHORT               m_usMaxBlockLen;               // max block length

    DWORD                m_dwMaxTimeout;                // timeout (in miliseconds) for read/write operations
    BOOL                 m_fFirstTime;                  // set to TRUE after a card is inserted to
                                                        // indicate that 80 clock cycles initialization
                                                        // must be done when the first command is issued
    BOOL                 m_fPowerDownChangedPower;      // did _PowerDown change the power state?

    PCSP_ESDHC_REG    m_pESDHCReg;        // the ESDHC registers

    PSD_BUS_REQUEST      m_pCurrentRequest;              // Current Processing Request.
    BOOL                 m_fCurrentRequestFastPath;
    SD_API_STATUS        m_FastPathStatus;


    CESDHCBaseDMA        *m_SlotDma;             // DMA object
    BOOL                m_fAutoCMD12Success;  // Indicates Auto CMD12 completed successfuly
    BOOL                m_fDisableDMA;             // Disable DMA altogether
    BOOL                m_fUseDMA;                  // Does the current request use DMA for data transfer?
    BOOL                m_fUseExternalDMA;      // Does the platform require use of external DMA as opposed to internal DMA?
    BOOL                m_fADMASupport;          // Does this controller support ADMA?
    BOOL                m_fHighSpeedSupport;   // Is this version of ESDHC High Speed Capable?

    UINT8              m_dwEDMAChanTx;        // transmit channel number for external DMA
    UINT8              m_dwEDMAChanRx;       // receive channel number for external DMA
    DWORD              m_dwRCA;                     // relative card address
    BOOL                m_fCardIsSDIO;             // Is the card in the slot SDIO? Used for clock gating decision-making.
    DWORD            m_dwVendorVersion;      // ESDHC IP version number
    BOOL               m_fSDIOInjectDelay;      // used for workaround to add delay after CMD53 on MX35 for Sychip Wifi card to work
}*PCESDHCBase;

CESDHCBase *CreateESDHC();

#define SHC_INTERRUPT_ZONE              SDCARD_ZONE_0
#define SHC_SEND_ZONE                   SDCARD_ZONE_1
#define SHC_RESPONSE_ZONE               SDCARD_ZONE_2
#define SHC_RECEIVE_ZONE                SDCARD_ZONE_3
#define SHC_CLOCK_ZONE                  SDCARD_ZONE_4
#define SHC_TRANSMIT_ZONE               SDCARD_ZONE_5
#define SHC_SDBUS_INTERACT_ZONE         SDCARD_ZONE_6
#define SHC_BUSY_STATE_ZONE             SDCARD_ZONE_7

#define SHC_INTERRUPT_ZONE_ON           ZONE_ENABLE_0
#define SHC_SEND_ZONE_ON                ZONE_ENABLE_1
#define SHC_RESPONSE_ZONE_ON            ZONE_ENABLE_2
#define SHC_RECEIVE_ZONE_ON             ZONE_ENABLE_3
#define SHC_CLOCK_ZONE_ON               ZONE_ENABLE_4
#define SHC_TRANSMIT_ZONE_ON            ZONE_ENABLE_5
#define SHC_SDBUS_INTERACT_ZONE_ON      ZONE_ENABLE_6
#define SHC_BUSY_STATE_ZONE_ON          ZONE_ENABLE_7

#define SHC_CARD_CONTROLLER_PRIORITY    0x97

#define DEFAULT_TIMEOUT_VALUE               2000    // 2seconds

#define RCA_SHIFT                                16

#endif      // #ifndef __ESDHC_H

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

