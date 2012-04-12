//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File: init.c
//
// Board initialization code.
//
//-----------------------------------------------------------------------------

#include <bsp.h>
#include <dbgserial.h>
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include "poweroff.h"
#pragma warning(pop)

// Need to disable function pointer cast warnings.  We use memcpy to move 
// the bus scaling calibration routine into IRAM and then cast this address 
// as a function pointer.
#pragma warning(disable: 4054 4055)

//-----------------------------------------------------------------------------
// Types

// External Variables
extern UUID g_UUID;

//-----------------------------------------------------------------------------
// External Functions
extern void OALClockInit(void);
extern void OALPowerInit();
extern VOID OALTimerNotifyReschedule(DWORD dwThrdId, DWORD dwPrio, DWORD dwQuantum, DWORD dwFlags);

#define ENABLE_WATCH_DOG

#ifdef ENABLE_WATCH_DOG
extern void InitWatchDogTimer (void);
#endif

// CS&ZHL MAR-17-2012: init CSPDDK library
extern BOOL ClockAlloc(void);
extern BOOL ApbhDmaAlloc(void);
extern BOOL IomuxAlloc(void);

extern BOOL ApbhDmaInit(void);
extern BOOL ApbhDmaDeInit(void);
extern BOOL ApbhDmaDealloc(void);
extern BOOL IomuxDealloc(void);
// end of CS&ZHL MAR-17-2012: init CSPDDK library

//-----------------------------------------------------------------------------
// External Variables
//

#define DR_WATSON_SIZE_NOT_FIXEDUP (-1)
DWORD dwOEMDrWatsonSize = (DWORD) DR_WATSON_SIZE_NOT_FIXEDUP;
UINT8 g_UPID[16];

// CS&ZHL MAR-17-2012: defined in CSPDDK already
extern PVOID    pv_HWregCLKCTRL;
extern PVOID    pv_HWregAPBH;
extern PVOID    pv_HWregPINCTRL;
// end of CS&ZHL MAR-17-2012: defined in CSPDDK already

// CS&ZHL MAR-21-2012: defined in hal_nand_$(_SOCDIR).lib already
#ifdef NAND_PDD
extern PVOID	pv_HWregGPMI;
extern PVOID	pv_HWregBCH;
extern PVOID	pv_HWregDIGCTL;
#else
PVOID	pv_HWregGPMI   = NULL;
PVOID	pv_HWregBCH    = NULL;
PVOID	pv_HWregDIGCTL = NULL;
#endif	//NAND_PDD
// end of CS&ZHL MAR-21-2012: defined in hal_nand_$(_SOCDIR).lib already

/////////////////////////////////////////////////////////
// register variable
PVOID    pv_HWregUARTDbg    = NULL;
PVOID    pv_HWregPOWER      = NULL;
PVOID    pv_HWregTimer      = NULL;
PVOID    pv_HWregRTC        = NULL;
PVOID    pv_HWregUSBPhy0    = NULL;
PVOID    pv_HWregUSBPhy1    = NULL;
PVOID    pv_HWregDRAM       = NULL;
PVOID    pv_HWregUSBCTRL0   = NULL;
PVOID    pv_HWregUSBCTRL1   = NULL;
PVOID    pv_HWregAPBX       = NULL;
PVOID    pv_HWregLRADC      = NULL;
PVOID    pv_HWregOTP        = NULL;
PVOID    pv_HWRegPWM        = NULL;
PVOID    pv_HWregLCDIF      = NULL;
PVOID    pv_HWregICOLL      = NULL;			// CS&ZHL MAR-17-2012: move from $(_SOCDIR)\OAL\intr.c, discard //extern PVOID    pv_HWregICOLL;
//PVOID    pv_HWregGPMI       = NULL;
//PVOID    pv_HWregBCH        = NULL;
//PVOID    pv_HWregDIGCTL     = NULL;

FUNC_POWER_CONTROL   pControlFunc     = NULL;


//-----------------------------------------------------------------------------
//
// Function: OEMHaltSystem
//
// Function turns on leds to indicate error condition before system halt.
//
// Parameters:
//
// Returns:
//
//
//-----------------------------------------------------------------------------
void OEMHaltSystem(void)
{
    // Nothing to do for now
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInit
//
//  This is Windows CE OAL initialization function. It is called from kernel
//  after basic initialization is made.
//------------------------------------------------------------------------------
void OEMInit()
{
    BSP_ARGS *pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;

    // Set up the debug zones according to the fix-up variable
    // initialOALLogZones.
    OALLogSetZones(initialOALLogZones);

    // OALLogSetZones( // set debug zones
    // (1 << OAL_LOG_ERROR)  |
    // (1 << OAL_LOG_WARN)   |
    // (1 << OAL_LOG_FUNC)    |
    // (1 << OAL_LOG_INFO)   |
    // (1 << OAL_LOG_STUB)   |
    // (1 << OAL_LOG_KEYVAL) |
    // (1 << OAL_LOG_ARGS)    |
    // (1 << OAL_LOG_CACHE)  |
    // (1 << OAL_LOG_RTC)    |
    // (1 << OAL_LOG_POWER)    |
    // (1 << OAL_LOG_PCI)    |
    // (1 << OAL_LOG_MEMORY) |
    // (1 << OAL_LOG_IO)        |
    // (1 << OAL_LOG_TIMER)    |
    // (1 << OAL_LOG_IOCTL)    |
    // (1 << OAL_LOG_FLASH)    |
    // (1 << OAL_LOG_INTR)        |
    // (1 << OAL_LOG_VERBOSE)
    //);

    OALMSG(OAL_FUNC, (L"+OEMInit\r\n"));
    
    pv_HWregUARTDbg   =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_UARTDBG,   FALSE);
    pv_HWregPOWER     =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_POWER,     FALSE);
    pv_HWregCLKCTRL   =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_CLKCTRL,   FALSE);
    pv_HWregICOLL     =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_ICOLL,     FALSE);			// CS&ZHL MAR-17-2012: move from $(_SOCDIR)\OAL\intr.c
    pv_HWregPINCTRL   =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_PINCTRL,   FALSE);
    pv_HWregTimer     =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_TIMROT,    FALSE);
    pv_HWregDIGCTL    =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_DIGCTL,    FALSE);
    pv_HWregRTC       =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_RTC,       FALSE);
    pv_HWregUSBPhy0   =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_USBPHY0,   FALSE);
    pv_HWregUSBPhy1   =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_USBPHY1,   FALSE);
    pv_HWregUSBCTRL0  =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_USBCTRL0,  FALSE);
    pv_HWregUSBCTRL1  =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_USBCTRL1,  FALSE);
    pv_HWregGPMI      =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_GPMI,      FALSE);
    pv_HWregAPBH      =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_APBH,      FALSE);
    pv_HWregAPBX      =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_APBX,      FALSE);
    pv_HWregDRAM      =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_DRAM,      FALSE);
    pv_HWregLRADC     =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_LRADC,     FALSE);
    pv_HWregOTP       =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_OCOTP,     FALSE);
    pv_HWRegPWM       =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_PWM,       FALSE);
    pv_HWregBCH       =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_BCH,       FALSE);
    pv_HWregLCDIF     =(PVOID) OALPAtoVA(CSP_BASE_REG_PA_LCDIF,     FALSE);

    pControlFunc      =(FUNC_POWER_CONTROL) OALPAtoVA(IMAGE_WINCE_POWEROFF_IRAM_OFFSET,  FALSE);

    // Set memory size for DrWatson kernel support.
    dwNKDrWatsonSize = 0;
    if (dwOEMDrWatsonSize != DR_WATSON_SIZE_NOT_FIXEDUP)
    {
        dwNKDrWatsonSize = dwOEMDrWatsonSize;
    }

    // Define optional kernel supported features.
    pOEMIsProcessorFeaturePresent = OALIsProcessorFeaturePresent;

    // Initialize system halt handler.
    g_pOemGlobal->pfnHaltSystem = OEMHaltSystem;

    // Give kernel access to the profiling functions.
    g_pOemGlobal->pfnProfileTimerEnable  = OEMProfileTimerEnable;
    g_pOemGlobal->pfnProfileTimerDisable = OEMProfileTimerDisable;

    // Expose the processor type. (Note that the i.MX28 is actually an ARM926Ej-S
    // core but Microsoft currently only defines an ARM920 for ARM9 family
    // processor type. So we should use an ARM920 processor type for now).
    CEProcessorType = PROCESSOR_ARM920;

    // Initilize cache globals
    OALCacheGlobalsInit();

    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"   Cache:\r\n"));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1Flags:       0x%08x\r\n",
                                       g_oalCacheInfo.L1Flags));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1ISetsPerWay: 0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1ISetsPerWay,
                                       g_oalCacheInfo.L1ISetsPerWay));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1INumWays:    0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1INumWays,
                                       g_oalCacheInfo.L1INumWays));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1ILineSize:   0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1ILineSize,
                                       g_oalCacheInfo.L1ILineSize));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1ISize:       0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1ISize,
                                       g_oalCacheInfo.L1ISize));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1DSetsPerWay: 0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1DSetsPerWay,
                                       g_oalCacheInfo.L1DSetsPerWay));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1DNumWays:    0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1DNumWays,
                                       g_oalCacheInfo.L1DNumWays));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1DLineSize:   0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1DLineSize,
                                       g_oalCacheInfo.L1DLineSize));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1DSize:       0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1DSize,
                                       g_oalCacheInfo.L1DSize));
    OALPowerInit();
    OALClockInit();

	// CS&ZHL MAR-17-2012: init CSPDDK
    if(!ClockAlloc())
        OALMSGS(1, (_T("ClockInit() failed!")));
    if (!ApbhDmaAlloc())
        OALMSGS(1, (_T("ApbhDmaAlloc failed!")));
    if (!IomuxAlloc())
        OALMSGS(1, (_T("IomuxAlloc failed!")));
    if (!ApbhDmaInit())
        OALMSGS(1, (_T("ApbhDmaInit failed!")));
	// end of CS&ZHL MAR-17-2012: init CSPDDK

    // Tell filesys.exe that we want a clean boot.
    NKForceCleanBoot();

    memcpy(g_UPID, pBspArgs->uuid, 16);
    memcpy(&g_UUID, g_UPID, 16);

    // Initialize interrupts
    if (!OALIntrInit())
    {
        OALMSG(OAL_ERROR,(L"ERROR: OEMInit: failed to initialize interrupts\r\n"));
        goto cleanUp;
    }

    // Initialize the system clock.
    if (!OALTimerInit(RESCHED_PERIOD, OEM_TICKS_PER_1MS, OEM_TICKS_MARGIN))
    {
        OALMSG(OAL_ERROR,(L"ERROR: OEMInit: Failed to initialize system clock\r\n"));
        goto cleanUp;
    }

    // Initialize the KITL connection if required
    KITLIoctl(IOCTL_KITL_STARTUP, NULL, 0, NULL, 0, NULL);

    //
    // Define ENABLE_WATCH_DOG to enable watchdog timer support.
    // NOTE: When watchdog is enabled, the device will reset itself if watchdog timer is not refreshed within ~4.5 second.
    // Therefore it should not be enabled when kernel debugger is connected, as the watchdog timer will not be refreshed.
    //
#ifdef ENABLE_WATCH_DOG
    InitWatchDogTimer ();
#endif
    // Map reschedule notification kernel function to perform CPU load tracking
    g_pOemGlobal->pfnNotifyReschedule = OALTimerNotifyReschedule;


    // Enable auto restart.
    HW_RTC_PERSISTENT0_SET(BM_RTC_PERSISTENT0_AUTO_RESTART);

cleanUp:
    OALMSG(OAL_FUNC, (L"-OEMInit\r\n"));
}

//------------------------------------------------------------------------------
