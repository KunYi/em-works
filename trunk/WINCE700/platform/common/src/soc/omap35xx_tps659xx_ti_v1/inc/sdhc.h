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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: SDHC.H
//
//  SDHC driver definitions
//

#ifndef _SDHC_DEFINED
#define _SDHC_DEFINED

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <devload.h>
#include <pm.h>
#include <omap35xx.h>
#include <gpio.h>
#include <SDCardDDK.h>
#include <SDHCD.h>
#include <creg.hxx>
#include <oal.h>

#include "dma_utility.h"
//#include "SDHCRegs.h"
#include <dvfs.h>

//#define ENABLE_RETAIL_OUTPUT

// TBD Not required for 35xx
#define MMCHS1_VDDS_WORKAROUND

#define DEFAULT_PBIAS_VALUE  0x00000003

#define SDIO_DMA_ENABLED         // comment out this line to disable DMA support for testing

#ifdef SDIO_DMA_ENABLED
#define SDIO_DMA_READ_ENABLED
#define SDIO_DMA_WRITE_ENABLED
#endif
#define CB_DMA_BUFFER 0x10000       // 64KB buffer

#define MMC_BLOCK_SIZE     0x200
#define MIN_MMC_BLOCK_SIZE 4
#define SD_IO_BUS_CONTROL_BUS_WIDTH_MASK 0x03

#ifndef SHIP_BUILD
#define STR_MODULE _T("SDHC!")
#define SETFNAME(name) LPCTSTR pszFname = STR_MODULE name _T(":")
#else
#define SETFNAME(name)
#endif
#define SD_REMOVE_TIMEOUT 200

#define SD_TIMEOUT 0
#define SD_INSERT 1
#define SD_REMOVE 2

// SDHC hardware specific context
class CSDIOControllerBase
{
public:

    CSDIOControllerBase();
    ~CSDIOControllerBase();

    BOOL Init( LPCTSTR pszActiveKey );
    VOID FreeHostContext( BOOL fRegisteredWithBusDriver, BOOL fHardwareInitialized );
    VOID PowerDown();
    VOID PowerUp();
    // callback handlers
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

    // SD driver callbacks implementation
    SD_API_STATUS SDHCInitializeImpl();
    SD_API_STATUS SDHCDeinitializeImpl();
    BOOLEAN SDHCCancelIoHandlerImpl( UCHAR Slot, PSD_BUS_REQUEST pRequest );
    SD_API_STATUS SDHCBusRequestHandlerImpl( PSD_BUS_REQUEST pRequest );
    SD_API_STATUS SDHCSlotOptionHandlerImpl( UCHAR SlotNumber,
                                             SD_SLOT_OPTION_CODE Option,
                                             PVOID pData,
                                             ULONG OptionSize );


    // platform specific functions
    virtual BOOL    InitializeHardware() = 0;
    virtual void    DeinitializeHardware() = 0;
    virtual BOOL    IsWriteProtected() = 0;
    virtual BOOL    SDCardDetect() = 0;
    virtual DWORD   SDHCCardDetectIstThreadImpl() = 0;
    virtual VOID    TurnCardPowerOn() = 0;
    virtual VOID    TurnCardPowerOff() = 0;
    virtual VOID    PreparePowerChange(CEDEVICE_POWER_STATE curPowerState) = 0;
    virtual VOID    PostPowerChange(CEDEVICE_POWER_STATE curPowerState) = 0;
    SD_API_STATUS   CSDIOControllerBase::SDHCBusRequestHandlerImpl_FastPath(PSD_BUS_REQUEST pRequest);
    SD_API_STATUS   CSDIOControllerBase::SDHCBusRequestHandlerImpl_NormalPath(PSD_BUS_REQUEST pRequest);

    // helper functions
    virtual BOOL    GetRegistrySettings( CReg *pReg );
    virtual BOOL    InterpretCapabilities();
    VOID    SetInterface(PSD_CARD_INTERFACE_EX pInterface);
    VOID    EnableSDHCInterrupts();
    VOID    DisableSDHCInterrupts();
    VOID    EnableSDIOInterrupts();
    VOID    AckSDIOInterrupt();
    VOID    DisableSDIOInterrupts();
    SD_API_STATUS SendCmdNoResp (DWORD cmd, DWORD arg);
    VOID    SendInitSequence();
    SD_API_STATUS SendCommand(PSD_BUS_REQUEST pRequest);
    SD_API_STATUS GetCommandResponse(PSD_BUS_REQUEST pRequest);
    SD_API_STATUS CommandCompleteHandler();
    BOOL    SDIReceive(PBYTE pBuff, DWORD dwLen);
    BOOL    SDITransmit(PBYTE pBuff, DWORD dwLen);
    SD_API_STATUS CommandCompleteHandler_FastPath(PSD_BUS_REQUEST pRequest);
    BOOL    SDIReceive(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode);
    BOOL    SDITransmit(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode);
#ifdef SDIO_DMA_ENABLED
    BOOL    SDIDMAReceive(PBYTE pBuff, DWORD dwLen);
    BOOL    SDIDMATransmit(PBYTE pBuff, DWORD dwLen);
    BOOL    SDIDMAReceive(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode);
    BOOL    SDIDMATransmit(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode);
#endif
    BOOL    SDIPollingReceive(PBYTE pBuff, DWORD dwLen);
    BOOL    SDIPollingTransmit(PBYTE pBuff, DWORD dwLen);
    VOID    SetSlotPowerState( CEDEVICE_POWER_STATE state );
    CEDEVICE_POWER_STATE GetSlotPowerState();

    VOID SoftwareReset(DWORD dwResetBits);

    // Interrupt handling methods
    DWORD HandleCardDetectInterrupt();
    VOID HandleRemoval(BOOL fCancelRequest);
    VOID HandleInsertion();

    VOID SetSDVSVoltage();
    VOID SetClockRate(PDWORD pdwRate);
    BOOL UpdateSystemClock( BOOL enable );
    VOID SystemClockOn() {
        m_InternPowerState = D0;
        UpdateDevicePowerState();
    }
    VOID SystemClockOff() {
        m_InternPowerState = D4;
        UpdateDevicePowerState();
    }
    VOID UpdateDevicePowerState();
    BOOL SetPower(CEDEVICE_POWER_STATE dx);
    BOOL ContextRestore();

    inline DWORD Read_MMC_STAT();
    inline void Write_MMC_STAT( DWORD wVal );
    inline void Set_MMC_STAT( DWORD wVal );

    // IST functions
    static DWORD WINAPI SDHCControllerIstThread(LPVOID lpParameter);
    static DWORD WINAPI SDHCCardDetectIstThread(LPVOID lpParameter);
    static DWORD WINAPI DataTransferIstThread(LPVOID lpParameter);
    DWORD SDHCControllerIstThreadImpl();
    DWORD SDHCPowerTimerThreadImpl();
    static DWORD WINAPI SDHCPowerTimerThread(LPVOID lpParameter);

    static DWORD WINAPI SDHCPrintThread(LPVOID lpParameter);
    DWORD SDHCPrintThreadImpl();
    HANDLE               m_hPrintEvent;         // SDIO/controller print event
    HANDLE               m_hPrintIST;             // SDIO/controller print thread
    const static DWORD        m_cmdArrSize=32;
    DWORD              m_cmdArray[m_cmdArrSize];
    DWORD              m_cmdRdIndex;
    DWORD              m_cmdWrIndex;
#ifdef SDIO_DMA_ENABLED
    DmaDataInfo_t *      m_TxDmaInfo;
    HANDLE               m_hTxDmaChannel;       // TX DMA channel allocated by the DMA lib
    DmaDataInfo_t *      m_RxDmaInfo;
    HANDLE               m_hRxDmaChannel;       // RX DMA channel allocated by the DMA lib

    // DMA functions
    void SDIO_InitDMA(void);
    void SDIO_DeinitDMA(void);
    void SDIO_InitInputDMA(DWORD dwBlkCnt, DWORD dwBlkSize);
    void SDIO_InitOutputDMA(DWORD dwBlkCnt, DWORD dwBlkSize);
    void SDIO_StartInputDMA();
    void SDIO_StartOutputDMA();
    void SDIO_StopInputDMA();
    void SDIO_StopOutputDMA();
    void DumpDMARegs(int inCh);
#endif

#ifdef DEBUG
    VOID DumpRegisters();
#else
    VOID DumpRegisters() {}
#endif
    // DVFS related functions
    void CopyDVFSHandles(UINT processId, HANDLE hAckEvent, HANDLE hActivityEvent);
    VOID CheckAndHaltAllDma(BOOL bHalt);
    void PreDmaActivation(UINT fTxRx);
    void PostDmaDeactivation(UINT fTxRx);
    void SDHC_DumpPRCM();

    PSDCARD_HC_CONTEXT   m_pHCContext;                   // the host controller context
    HANDLE               m_hParentBus;                   // bus parent handle

    CRITICAL_SECTION     m_critSec;                      // used to synchronize access to SDIO controller registers
    CRITICAL_SECTION     m_powerCS;                      // used to synchronize access to SDIO controller registers
    BOOL                 m_fSDIOInterruptInService;      // TRUE - an SDIO interrupt has been detected and has
                                                        // not yet been acknowledge by the client
    CEDEVICE_POWER_STATE m_ExternPowerState;            // current extern power state
    CEDEVICE_POWER_STATE m_InternPowerState;            // current internal power state.
    CEDEVICE_POWER_STATE m_ActualPowerState;            // actual power state.
    BOOL                 m_bReinsertTheCard;            // force card insertion event
    DWORD                m_dwWakeupSources;             // possible wakeup sources (1 - SDIO, 2 - card insert/removal)
    DWORD                m_dwCurrentWakeupSources;      // current wakeup sources

    BOOL                 m_fCardPresent;                // a card is inserted and initialized
    BOOL                 m_fSDIOInterruptsEnabled;      // TRUE - indicates that SDIO interrupts are enabled

    BOOL                 m_fMMCMode;                    // if true, the controller assumed that the card inserted is MMC
    PBYTE                m_pDmaBuffer;                  // virtual address of DMA buffer
    PHYSICAL_ADDRESS     m_pDmaBufferPhys;              // physical address of DMA buffer
    BOOL                 m_fDMATransfer;                // TRUE - the current request will use DMA for data transfer

    BOOL                 m_fAppCmdMode;                 // if true, the controller is in App Cmd mode
    HANDLE               m_hControllerISTEvent;         // SDIO/controller interrupt event
    HANDLE               m_htControllerIST;             // SDIO/controller interrupt thread
    HANDLE               m_hCardDetectEvent;            // card detect interrupt event
    HANDLE               m_htCardDetectIST;             // card detect interrupt thread

    DWORD                m_dwSDIOPriority;              // SDIO IST priority
    DWORD                m_dwCDPriority;                // CardDetect IST priority
    BOOL                 m_fDriverShutdown;             // controller shutdown
    DWORD                m_dwControllerSysIntr;         // controller interrupt
    BOOL                 m_fInitialized;                // driver initialized
    BOOL                 m_fCardInitialized;            // Card Initialized
    WORD                 m_wCTOTimeout;                 // command time-out
    WORD                 m_wDTOTimeout;                 // data read time-out
    DWORD                m_dwMaxClockRate;              // host controller's clock base
    USHORT               m_usMaxBlockLen;               // max block length

    DWORD                m_dwMaxTimeout;                // timeout (in miliseconds) for read/write operations
    BOOL                 m_fFirstTime;                  // set to TRUE after a card is inserted to
                                                        // indicate that 80 clock cycles initialization
                                                        // must be done when the first command is issued
    BOOL                 m_fPowerDownChangedPower;      // did _PowerDown change the power state?

    OMAP_MMCHS_REGS *m_pbRegisters;                 // SDHC controller registers
    DWORD                m_LastCommand;

    HANDLE               m_hGPIO;                       // GPIO driver handle

    BOOL                 m_fCardInserted;               // Is card inserted? ($1)
    DWORD                m_dwMemBase;
    DWORD                m_dwMemLen;
    DWORD                m_dwSlot;
    DWORD                m_dwSDIOCard;
    DWORD                m_dwCPURev;
    DWORD                m_fastPathSDIO;
    DWORD                m_fastPathSDMEM;
    DWORD                m_fastPathReq;
    DWORD                m_LowVoltageSlot;
    DWORD                m_Sdio4BitDisable;
    DWORD                m_SdMem4BitDisable;

    LONG                m_dwClockCnt;
    BOOL                m_bExitThread;
    HANDLE            m_hTimerEvent;
    HANDLE             m_hTimerThreadIST;
    DWORD              m_nNonSDIOActivityTimeout;
    DWORD              m_nSDIOActivityTimeout;
    CRITICAL_SECTION     m_pwrThrdCS;                 // used to synchronize access to m_dwClockCnt
    BOOL                m_bDisablePower;
    // dvfs related variables
    UINT                 nDVFSOrder;
    BOOL                 bDVFSActive;
    LONG                 nActiveDmaCount;
    HANDLE               hDVFSAckEvent;
    HANDLE               hDVFSActivityEvent;
    CRITICAL_SECTION     csDVFS;

    BOOL                 bRxDmaActive;
    BOOL                 bTxDmaActive;
    UINT16               m_TransferClass;
};

typedef CSDIOControllerBase *PCSDIOControllerBase;

#define GET_PCONTROLLER_FROM_HCD(pHCDContext) \
    GetExtensionFromHCDContext(PCSDIOControllerBase, pHCDContext)

CSDIOControllerBase *CreateSDIOController();

#define SHC_INTERRUPT_ZONE              SDCARD_ZONE_0
#define SHC_SEND_ZONE                   SDCARD_ZONE_1
#define SHC_RESPONSE_ZONE               SDCARD_ZONE_2
#define SHC_RECEIVE_ZONE                SDCARD_ZONE_3
#define SHC_CLOCK_ZONE                  SDCARD_ZONE_4
#define SHC_TRANSMIT_ZONE               SDCARD_ZONE_5
#define SHC_SDBUS_INTERACT_ZONE         SDCARD_ZONE_6
#define SHC_BUSY_STATE_ZONE             SDCARD_ZONE_7
#define SHC_DVFS_ZONE                   SDCARD_ZONE_8

#define SHC_INTERRUPT_ZONE_ON           ZONE_ENABLE_0
#define SHC_SEND_ZONE_ON                ZONE_ENABLE_1
#define SHC_RESPONSE_ZONE_ON            ZONE_ENABLE_2
#define SHC_RECEIVE_ZONE_ON             ZONE_ENABLE_3
#define SHC_CLOCK_ZONE_ON               ZONE_ENABLE_4
#define SHC_TRANSMIT_ZONE_ON            ZONE_ENABLE_5
#define SHC_SDBUS_INTERACT_ZONE_ON      ZONE_ENABLE_6
#define SHC_BUSY_STATE_ZONE_ON          ZONE_ENABLE_7
#define SHC_DVFS_ZONE_ON                ZONE_ENABLE_8

#define SHC_CARD_CONTROLLER_PRIORITY    0x97
#define SHC_CARD_DETECT_PRIORITY        0x98

// Registry key words
#define SHC_SDIO_PRIORITY_KEY           TEXT("SDIOPriority")
#define SHC_CD_PRIORITY_KEY             TEXT("CDPriority")
#define SHC_FREQUENCY_KEY               TEXT("BaseClockFrequency")
//#define SHC_RESET_KEY                   TEXT("ResetGpio")
//#define SHC_CD_IRQ_KEY                  TEXT("CardDetectIrq")
#define SHC_RW_TIMEOUT_KEY              TEXT("ReadWriteTimeout")
#define SHC_WAKEUP_SOURCES_KEY          TEXT("WakeupSources")
#define SHC_DTO_TIMEOUT                 TEXT("DTOTimeout")
#define SHC_CTO_TIMEOUT                 TEXT("CTOTimeout")
#define SHC_MEM_BASE_KEY                TEXT("MemBase")
#define SHC_MEM_LEN_KEY                 TEXT("MemLen")

#define SHC_CD_GPIO_KEY                 TEXT("CardDetectGPIO")
#define SHC_WP_GPIO_KEY                 TEXT("CardWPGPIO")
#define SHC_CARD_INSERTED_STATE_KEY     TEXT("CardInsertedState")
#define SHC_CARD_WRITE_PROTECTED_STATE_KEY TEXT("CardWriteProtectedState")

#define WAKEUP_SDIO                     1
#define WAKEUP_CARD_INSERT_REMOVAL      2

#define MMCSLOT_1                       0
#define MMCSLOT_2                       1

#define DMA_TX                          0
#define DMA_RX                          1
#define TIMERTHREAD_TIMEOUT_NONSDIO     1
#define TIMERTHREAD_TIMEOUT             2000
#define TIMERTHREAD_PRIORITY            252
#endif // _SDHC_DEFINED


