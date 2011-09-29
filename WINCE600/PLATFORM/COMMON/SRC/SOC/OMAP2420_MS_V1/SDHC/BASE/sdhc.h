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

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

// SDHC driver definitions

#ifndef _SDHC_DEFINED
#define _SDHC_DEFINED

#include <windows.h>
#include <pm.h>
#include <ceddkex.h>
#include <ceddk.h>
#include <devload.h>
#include <omap2420.h>
#include <SDCardDDK.h>
#include <SDHCD.h>
#include <creg.hxx>

//#define ENABLE_DEBUG

// Debug Zones
//#define ZONE_SHC       0
//#define ZONE_FUNCTION  0
//#define ZONE_ERROR     1

#define NUM_BYTE_FOR_POLLING_MODE 0x800

#ifndef SHIP_BUILD
#  define STR_MODULE _T("SDHC!")
#  define SETFNAME(name) LPCTSTR pszFname = STR_MODULE name _T(":")
#else
#define SETFNAME(name)
#endif

// SDHC hardware specific context
class CSDIOControllerBase
{
public:

    CSDIOControllerBase();
    virtual ~CSDIOControllerBase();

    BOOL Init( LPCTSTR pszActiveKey );
    VOID FreeHostContext( BOOL fRegisteredWithBusDriver, BOOL fHardwareInitialized );
    BOOL IOControl(DWORD dwCode,
                   BYTE *pInBuffer,
                   DWORD inSize,
                   BYTE *pOutBuffer,
                   DWORD outSize,
                   DWORD *pOutSize);

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

    // give derived class an opportunity to intercept call
    virtual VOID IndicateBusRequestComplete( PSD_BUS_REQUEST pRequest, SD_API_STATUS Status );

    // platform specific functions
    virtual BOOL    InitializeHardware()    = 0;
    virtual void    DeinitializeHardware()  = 0;
    virtual BOOL    IsWriteProtected()      = 0;
    virtual BOOL    SDCardDetect()          = 0;
    virtual DWORD   SDHCCardDetectIstThreadImpl() = 0;
    virtual VOID    TurnCardPowerOn()       = 0;
    virtual VOID    TurnCardPowerOff()      = 0;

    // helper functions
    virtual BOOL    GetRegistrySettings( CReg *pReg );
    virtual BOOL    InterpretCapabilities();
    VOID    SetInterface(PSD_CARD_INTERFACE pInterface);
    VOID    EnableSDIOInterrupts();
    VOID    AckSDIOInterrupt();
    VOID    DisableSDIOInterrupts();
    SD_API_STATUS SendCommand(PSD_BUS_REQUEST pRequest);
    SD_API_STATUS GetCommandResponse(PSD_BUS_REQUEST pRequest);
    BOOL    CommandCompleteHandler();
    BOOL    SDIReceive(PBYTE pBuff, DWORD dwLen);
    BOOL    SDITransmit(PBYTE pBuff, DWORD dwLen);
    BOOL    SDIPollingReceive(PBYTE pBuff, DWORD dwLen);
    BOOL    SDIPollingTransmit(PBYTE pBuff, DWORD dwLen);
    VOID SoftwareReset(BYTE bResetBits);

    // Interrupt handling methods
    VOID HandleCardDetectInterrupt();
    VOID HandleRemoval(BOOL fCancelRequest);
    VOID HandleInsertion();

    VOID ClockOn();
    VOID ClockOff();
    VOID SetClockRate(PDWORD pdwRate);

    inline WORD Read_MMC_SDIO();
    inline void Write_MMC_SDIO( WORD wVal );
    inline WORD Read_MMC_STAT();
    inline void Write_MMC_STAT( WORD wVal );

    // IST functions
    static DWORD WINAPI SDHCControllerIstThread(LPVOID lpParameter);
    static DWORD WINAPI SDHCCardDetectIstThread(LPVOID lpParameter);
    static DWORD WINAPI DataTransferIstThread(LPVOID lpParameter);
    DWORD SDHCControllerIstThreadImpl();

#ifdef ENABLE_DEBUG
    VOID DumpRegisters();
#else
    VOID DumpRegisters() {}
#endif

    PSDCARD_HC_CONTEXT   m_pHCContext;                  // the host controller context
    HANDLE               m_hParentBus;                  // bus parent handle
    LPCTSTR              m_pszActiveKey;                //
    CEDEVICE_POWER_STATE m_PowerState;                  // Powerstate for SDHC device
    CRITICAL_SECTION     m_critSec;                     // used to synchronize access to SDIO controller registers
    BOOL                 m_fSDIOInterruptInService;     // TRUE - an SDIO interrupt has been detected and has
                                                        // not yet been acknowledge by the client
    BOOL                 m_bReinsertTheCard;            // force card insertion event
    DWORD                m_dwWakeupSources;             // possible wakeup sources (1 - SDIO, 2 - card insert/removal)
    DWORD                m_dwCurrentWakeupSources;      // current wakeup sources

    BOOL                 m_fCardPresent;                // a card is inserted and initialized
    BOOL                 m_fSDIOInterruptsEnabled;      // TRUE - indicates that SDIO interrupts are enabled

    BOOL                 m_fMMCMode;                    // if true, the controller assumed that the card inserted is MMC

    BOOL                 m_fAppCmdMode;                 // if true, the controller is in App Cmd mode
    HANDLE               m_hControllerISTEvent;         // SDIO/controller interrupt event
    HANDLE               m_htControllerIST;             // SDIO/controller interrupt thread
    HANDLE               m_hCardDetectEvent;            // card detect interrupt event
    HANDLE               m_htCardDetectIST;             // card detect interrupt thread

    DWORD                m_dwSDIOPriority;              // SDIO IST priority
    DWORD                m_dwCDPriority;                // CardDetect IST priority
    BOOL                 m_fDriverShutdown;             // controller shutdown
    DWORD                m_dwCardDetectSysIRQSize;      // size of card detec IRQ Array
    DWORD                m_dwCardDetectSysIntr;         // card detect interrupt
    DWORD                m_dwControllerSysIntr;         // controller interrupt
    BOOL                 m_fInitialized;                // driver initialized

    DWORD                m_dwMaxClockRate;              // host controller's clock base
    USHORT               m_usMaxBlockLen;               // max block length

    DWORD                m_dwMaxTimeout;                // timeout (in miliseconds) for read/write operations
    BOOL                 m_fFirstTime;                  // set to TRUE after a card is inserted to
                                                        // indicate that 80 clock cycles initialization
                                                        // must be done when the first command is issued
    BOOL                 m_fPowerDownChangedPower;      // did _PowerDown change the power state?

    volatile OMAP2420_SDIO_REGS    *m_vpSDIOReg;        // the SDIO controller registers
    volatile OMAP2420_PRCM_REGS    *m_vpPRCMReg;        // pin multiplexing configuration register

    PSD_BUS_REQUEST      m_pCurrentRequest;              // Current Processing Request.
    BOOL                 m_fCurrentRequestFastPath;
    SD_API_STATUS        FastPathStatus;
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

#define SHC_INTERRUPT_ZONE_ON           ZONE_ENABLE_0
#define SHC_SEND_ZONE_ON                ZONE_ENABLE_1
#define SHC_RESPONSE_ZONE_ON            ZONE_ENABLE_2
#define SHC_RECEIVE_ZONE_ON             ZONE_ENABLE_3
#define SHC_CLOCK_ZONE_ON               ZONE_ENABLE_4
#define SHC_TRANSMIT_ZONE_ON            ZONE_ENABLE_5
#define SHC_SDBUS_INTERACT_ZONE_ON      ZONE_ENABLE_6
#define SHC_BUSY_STATE_ZONE_ON          ZONE_ENABLE_7

#define SHC_CARD_CONTROLLER_PRIORITY    0x97
#define SHC_CARD_DETECT_PRIORITY        0x98

#define SHC_SDIO_PRIORITY_KEY           TEXT("SDIOPriority")
#define SHC_CD_PRIORITY_KEY             TEXT("CDPriority")
#define SHC_FREQUENCY_KEY               TEXT("BaseClockFrequency")
#define SHC_RESET_KEY                   TEXT("ResetGpio")
#define SHC_SDIO_IRQ_KEY                TEXT("SDIOIrq")
#define SHC_SDIO_SYSINTR_KEY            TEXT("SDIOSysIntr")
#define SHC_CD_IRQ_KEY                  TEXT("CardDetectIrq")
#define SHC_CD_SYSINTR_KEY              TEXT("CardDetectSysIntr")
#define SHC_RW_TIMEOUT_KEY              TEXT("ReadWriteTimeout")
#define SHC_WAKEUP_SOURCES_KEY          TEXT("WakeupSources")

#define WAKEUP_SDIO                     1
#define WAKEUP_CARD_INSERT_REMOVAL      2

#define SDIO_PIN_GPIO   28

#endif // _SDHC_DEFINED

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

