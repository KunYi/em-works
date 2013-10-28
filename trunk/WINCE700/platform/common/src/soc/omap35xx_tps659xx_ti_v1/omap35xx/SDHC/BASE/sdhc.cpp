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
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File: SDHC.CPP
//  SDHC controller driver implementation

#include "SDHC.h"
#include "SDHCRegs.h"
#include <oalex.h>
#include <nkintr.h>
#include <bus.h>

// BusIoControl definition is not correct, unused parameters may be NULL/0
#pragma warning (disable: 6309)
#pragma warning (disable: 6387)

#define MMC_INT_EN_MASK                     0x00330033

#define DEFAULT_TIMEOUT_VALUE               10000
#define DATACMD_TIMEOUT_VALUE               10000
#define START_BIT                           0x00
#define TRANSMISSION_BIT                    0x00
#define START_RESERVED                      0x3F
#define END_RESERVED                        0xFE
#define END_BIT                             0x01
#define SDIO_MAX_LOOP                       0x0080000

#define IndicateBusRequestComplete(pRequest, status) \
    SDHCDIndicateBusRequestComplete(m_pHCContext, \
    (pRequest), (status))

#define IndicateSlotStateChange(event) \
    SDHCDIndicateSlotStateChange(m_pHCContext, \
    (UCHAR) 0, (event))

#define GetAndLockCurrentRequest() \
    SDHCDGetAndLockCurrentRequest(m_pHCContext, (UCHAR) 0)

#define PowerUpDown(fPowerUp, fKeepPower) \
    SDHCDPowerUpDown(m_pHCContext, (fPowerUp), (fKeepPower), \
    (UCHAR) 0)



#define TRANSFER_IS_WRITE(pRequest)        (SD_WRITE == (pRequest)->TransferClass)
#define TRANSFER_IS_READ(pRequest)         (SD_READ == (pRequest)->TransferClass)
#define TRANSFER_IS_COMMAND_ONLY(pRequest) (SD_COMMAND == (pRequest)->TransferClass)

#define TRANSFER_SIZE(pRequest)            ((pRequest)->BlockSize * (pRequest)->NumBlocks)

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"MemBase", PARAM_MULTIDWORD, TRUE,
        offset(CSDIOControllerBase, m_dwMemBase),
        fieldsize(CSDIOControllerBase, m_dwMemBase), NULL
    }, {
        L"MemLen", PARAM_MULTIDWORD, TRUE,
        offset(CSDIOControllerBase, m_dwMemLen),
        fieldsize(CSDIOControllerBase, m_dwMemLen), NULL
    }, {
        L"DVFSOrder", PARAM_DWORD, FALSE,
        offset(CSDIOControllerBase, nDVFSOrder),
        fieldsize(CSDIOControllerBase, nDVFSOrder), (VOID*)45
    }, {
        L"NonSDIOActivityTimeout", PARAM_DWORD, FALSE,
        offset(CSDIOControllerBase, m_nNonSDIOActivityTimeout),
        fieldsize(CSDIOControllerBase, m_nNonSDIOActivityTimeout), (VOID*)TIMERTHREAD_TIMEOUT_NONSDIO
    }, {
        L"SDIOActivityTimeout", PARAM_DWORD, FALSE,
        offset(CSDIOControllerBase, m_nSDIOActivityTimeout),
        fieldsize(CSDIOControllerBase, m_nSDIOActivityTimeout), (VOID*)TIMERTHREAD_TIMEOUT
    }, {
        L"FastPath_SDMEM", PARAM_DWORD, FALSE,
        offset(CSDIOControllerBase, m_fastPathSDMEM),
        fieldsize(CSDIOControllerBase, m_fastPathSDMEM), (VOID*)0
    }, {
        L"FastPath_SDIO", PARAM_DWORD, FALSE,
        offset(CSDIOControllerBase, m_fastPathSDIO),
        fieldsize(CSDIOControllerBase, m_fastPathSDIO), (VOID*)1
    }, {
        L"LowVoltageSlot", PARAM_DWORD, FALSE,
        offset(CSDIOControllerBase, m_LowVoltageSlot),
        fieldsize(CSDIOControllerBase, m_LowVoltageSlot), (VOID*)0
    }, {
        L"Sdio4BitDisable", PARAM_DWORD, FALSE,
        offset(CSDIOControllerBase, m_Sdio4BitDisable),
        fieldsize(CSDIOControllerBase, m_Sdio4BitDisable), (VOID*)0
    }, {
        L"SdMem4BitDisable", PARAM_DWORD, FALSE,
        offset(CSDIOControllerBase, m_SdMem4BitDisable),
        fieldsize(CSDIOControllerBase, m_SdMem4BitDisable), (VOID*)0
    }
};

#ifdef DEBUG
// dump the current request info to the debugger
static
VOID
DumpRequest(PSD_BUS_REQUEST pRequest)
{
    DEBUGCHK(pRequest);

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("DumpCurrentRequest: 0x%08X\n"), pRequest));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("\t Command: %d\n"),  pRequest->CommandCode));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("\t ResponseType: %d\n"),  pRequest->CommandResponse.ResponseType));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("\t NumBlocks: %d\n"),  pRequest->NumBlocks));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("\t BlockSize: %d\n"),  pRequest->BlockSize));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("\t HCParam: %d\n"),    pRequest->HCParam));

}
#else
#define DumpRequest(ptr)
#endif



#if defined(DEBUG) || defined(ENABLE_RETAIL_OUTPUT)
#define HEXBUFSIZE 1024
char szHexBuf[HEXBUFSIZE];
#endif

struct CMD
{
    BYTE Cmd;           // 1 - this is a known SD CMD; 2 - this is a known SDIO CMD
    BYTE ACmd;          // 1 - this is a known ACMD
    BYTE MMCCmd;        // 1 - this is a known MMC CMD
    DWORD flags;
};

// table of command codes...  at this time only SD/SDIO commands are implemented
const CMD gwaCMD[] =
{
    { 1, 0, 0, MMCHS_RSP_NONE }, // CMD 00
    { 0, 0, 1, MMCHS_CMD_CICE | MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 01 (known MMC command)
    { 1, 0, 1, MMCHS_RSP_LEN136 |MMCHS_CMD_CCCE | MMCHS_CMD_NORMAL }, // CMD 02
    { 1, 0, 1, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 03
    { 1, 0, 0, MMCHS_RSP_NONE  | MMCHS_CMD_NORMAL }, // CMD 04
    { 2, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 05
    { 0, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 06
    { 1, 0, 0, MMCHS_CMD_CICE|MMCHS_CMD_CCCE|MMCHS_RSP_LEN48B | MMCHS_CMD_NORMAL }, // CMD 07
    { 1, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL}, // CMD 08
    { 1, 0, 0, MMCHS_RSP_LEN136 | MMCHS_CMD_NORMAL }, // CMD 09
    { 1, 0, 0, MMCHS_RSP_LEN136 | MMCHS_CMD_NORMAL }, // CMD 10
    { 0, 0, 1, MMCHS_RSP_LEN48 | MMCHS_CMD_READ | MMCHS_CMD_DP | MMCHS_CMD_NORMAL }, // CMD 11 (known MMC command)
    { 1, 0, 0, MMCHS_RSP_LEN48B | MMCHS_CMD_ABORT }, // CMD 12   
    { 1, 1, 0, MMCHS_CMD_CICE|MMCHS_CMD_CCCE| MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL}, // CMD 13
    { 0, 0, 0, 0 }, // CMD 14
    { 1, 0, 0, MMCHS_RSP_NONE | MMCHS_CMD_NORMAL }, // CMD 15
    { 1, 0, 0, MMCHS_CMD_CCCE | MMCHS_CMD_CICE | MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 16
    { 1, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_READ | MMCHS_CMD_DP | MMCHS_CMD_NORMAL}, // CMD 17
    { 1, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_MSBS | MMCHS_CMD_BCE | MMCHS_CMD_READ |MMCHS_CMD_DP|MMCHS_CMD_NORMAL }, // CMD 18
    { 0, 1, 0, 0 }, // CMD 19
    { 0, 1, 1, MMCHS_RSP_LEN48B | MMCHS_CMD_DP | MMCHS_CMD_NORMAL }, // CMD 20 (known MMC command)
    { 0, 1, 0, 0 }, // CMD 21
    { 0, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_READ | MMCHS_CMD_NORMAL }, // CMD 22
    { 0, 1, 0, 0 }, // CMD 23 (known MMC command)
    { 1, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_DP | MMCHS_CMD_MSBS | MMCHS_CMD_BCE| MMCHS_CMD_NORMAL }, // CMD 24
    { 1, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_DP | MMCHS_CMD_MSBS | MMCHS_CMD_BCE | MMCHS_CMD_NORMAL }, // CMD 25
    { 0, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 26
    { 1, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 27
    { 1, 0, 0, MMCHS_RSP_LEN48B | MMCHS_CMD_NORMAL }, // CMD 28
    { 1, 0, 0, MMCHS_RSP_LEN48B | MMCHS_CMD_NORMAL }, // CMD 29
    { 1, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_READ | MMCHS_CMD_NORMAL }, // CMD 30
    { 0, 0, 0, 0 }, // CMD 31
    { 1, 0, 0, MMCHS_CMD_CICE|MMCHS_CMD_CCCE|MMCHS_RSP_LEN48| MMCHS_CMD_NORMAL  }, // CMD 32
    { 1, 0, 0, MMCHS_CMD_CICE|MMCHS_CMD_CCCE|MMCHS_RSP_LEN48| MMCHS_CMD_NORMAL }, // CMD 33
    { 0, 0, 0, 0 }, // CMD 34
    { 1, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 35
    { 1, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 36
    { 0, 0, 0, 0 }, // CMD 37
    { 1, 1, 0, MMCHS_CMD_CICE|MMCHS_CMD_CCCE|MMCHS_RSP_LEN48B | MMCHS_CMD_NORMAL }, // CMD 38
    { 0, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 39 (known MMC command)
    { 1, 1, 1, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 40
    { 0, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 41 (known MMC command)
    { 1, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 42
    { 0, 1, 0, 0 }, // CMD 43
    { 0, 1, 0, 0 }, // CMD 44
    { 0, 1, 0, 0 }, // CMD 45
    { 0, 1, 0, 0 }, // CMD 46
    { 0, 1, 0, 0 }, // CMD 47
    { 0, 1, 0, 0 }, // CMD 48
    { 0, 1, 0, 0 }, // CMD 49
    { 0, 1, 0, 0 }, // CMD 50
    { 0, 1, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_DP| MMCHS_CMD_NORMAL }, // CMD 51 (known MMC command)
    { 2, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 52
    { 2, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_DP| MMCHS_CMD_NORMAL}, // CMD 53
    { 0, 0, 0, 0 }, // CMD 54
    { 1, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 55
    { 1, 0, 0, 0 }, // CMD 56
    { 0, 0, 0, 0 }, // CMD 57
    { 0, 0, 0, 0 }, // CMD 58
    { 0, 0, 0, MMCHS_RSP_LEN48 | MMCHS_CMD_NORMAL }, // CMD 59
    { 0, 0, 0, 0 }, // CMD 60
    { 0, 0, 0, 0 }, // CMD 61
    { 0, 0, 0, 0 }, // CMD 62
    { 0, 0, 0, 0 }, // CMD 63
};


CSDIOControllerBase::CSDIOControllerBase()
{
    InitializeCriticalSection( &m_critSec );
    InitializeCriticalSection( &m_powerCS );
    m_fSDIOInterruptInService = FALSE;
    m_fFirstTime = TRUE;
    m_hControllerISTEvent = NULL;
    m_htControllerIST = NULL;
    m_dwControllerSysIntr = (DWORD)SYSINTR_UNDEFINED;
    m_hCardDetectEvent = NULL;
    m_htCardDetectIST = NULL;
    m_fAppCmdMode = FALSE;

    m_pbRegisters = NULL;
    m_fCardPresent = FALSE;
    m_fSDIOInterruptsEnabled = FALSE;
    m_pDmaBuffer = NULL;
    m_pDmaBufferPhys.QuadPart = 0;

    m_dwMaxTimeout = DEFAULT_TIMEOUT_VALUE;
    m_bReinsertTheCard = FALSE;
    m_dwWakeupSources = 0;
    m_dwCurrentWakeupSources = 0;
    m_fMMCMode = FALSE;

    m_ExternPowerState = D4;
    m_InternPowerState = D0;
    m_ActualPowerState = D4;
    m_hParentBus = NULL;
    m_hGPIO = NULL;

    m_dwSlot = MMCSLOT_1;
    m_dwSDIOCard = 0;

    // initialize dvfs variables
    bDVFSActive = FALSE;
    nActiveDmaCount = 0;
    hDVFSAckEvent = NULL;
    hDVFSActivityEvent = NULL;

    // initialize dvfs variables
    bRxDmaActive = FALSE;
    bTxDmaActive = FALSE;
    m_dwClockCnt = 0;
    m_fCardInitialized = FALSE;
    m_bExitThread = FALSE;
    m_TransferClass = 0;

    // Initalize DVFS critical section
    InitializeCriticalSection(&csDVFS);

}

DWORD CSDIOControllerBase::Read_MMC_STAT()
{
    DWORD dwVal;
    EnterCriticalSection( &m_critSec );
    dwVal = INREG32(&m_pbRegisters->MMCHS_STAT);
    LeaveCriticalSection( &m_critSec );
    return dwVal;
}

void CSDIOControllerBase::Write_MMC_STAT( DWORD dwVal )
{
    EnterCriticalSection( &m_critSec );
    OUTREG32(&m_pbRegisters->MMCHS_STAT, dwVal);
    LeaveCriticalSection( &m_critSec );
}

void CSDIOControllerBase::Set_MMC_STAT( DWORD dwVal )
{
    EnterCriticalSection( &m_critSec );
    OUTREG32(&m_pbRegisters->MMCHS_STAT, dwVal);
    LeaveCriticalSection( &m_critSec );
}

//  Reset the controller.
VOID CSDIOControllerBase::SoftwareReset( DWORD dwResetBits )
{
    DWORD               dwCountStart;

    DEBUGCHK(sizeof(OMAP_MMCHS_REGS) % sizeof(DWORD) == 0);


    dwResetBits &= (MMCHS_SYSCTL_SRA | MMCHS_SYSCTL_SRC | MMCHS_SYSCTL_SRD);

    // Reset the controller
    SETREG32(&m_pbRegisters->MMCHS_SYSCTL, dwResetBits);

    // get starting tick count for timeout
    dwCountStart = GetTickCount();

    // Verify that reset has completed.
    while ((INREG32(&m_pbRegisters->MMCHS_SYSCTL) & dwResetBits))
    {
        // check for timeout (see CE Help to understand how this calculation works)
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SoftwareReset() - exit: TIMEOUT.\n")));
            break;
        }

        Sleep(0);
    }
    // enable autoidle, disable wakeup, enable smart-idle, ClockActivity (interface and functional clocks may be switched off)
    OUTREG32(&m_pbRegisters->MMCHS_SYSCONFIG, MMCHS_SYSCONFIG_AUTOIDLE | MMCHS_SYSCONFIG_SIDLEMODE(SIDLE_SMART));
}

// Set up the controller according to the interface parameters.
VOID CSDIOControllerBase::SetInterface(PSD_CARD_INTERFACE_EX pInterface)
{
    DEBUGCHK(pInterface);

    if(m_ActualPowerState == D4) return;

    UpdateSystemClock(TRUE);
    if (SD_INTERFACE_SD_MMC_1BIT == pInterface->InterfaceModeEx.bit.sd4Bit)
    {

        DEBUGMSG(SDCARD_ZONE_INIT,
            (TEXT("SHCSDSlotOptionHandler - Setting for 1 bit mode \n")));

        CLRREG32(&m_pbRegisters->MMCHS_HCTL, MMCHS_HCTL_DTW);
        DEBUGMSG(SDCARD_ZONE_INIT,(TEXT("SetInterface MMCHS_HCTL value = %X\n"), m_pbRegisters->MMCHS_HCTL ));
    }
    else if (SD_INTERFACE_SD_4BIT == pInterface->InterfaceModeEx.bit.sd4Bit)
    {
        DEBUGMSG(SDCARD_ZONE_INIT,
            (TEXT("SHCSDSlotOptionHandler - Setting for 4 bit mode \n")));

        SETREG32(&m_pbRegisters->MMCHS_HCTL, MMCHS_HCTL_DTW);

        DEBUGMSG(SDCARD_ZONE_INIT,(TEXT("SetInterface MMCHS_HCTL value = %X\n"), m_pbRegisters->MMCHS_HCTL ));
    }
    else
    {
        DEBUGCHK(FALSE);
    }

    SetClockRate(&pInterface->ClockRate);
    UpdateSystemClock(FALSE);
}

// Enable SDHC Interrupts.
VOID CSDIOControllerBase::EnableSDHCInterrupts()
{
    EnterCriticalSection( &m_critSec );
    OUTREG32(&m_pbRegisters->MMCHS_ISE, MMC_INT_EN_MASK);
    OUTREG32(&m_pbRegisters->MMCHS_IE,  MMC_INT_EN_MASK);
    LeaveCriticalSection( &m_critSec );
}

// Disable SDHC Interrupts.
void CSDIOControllerBase::DisableSDHCInterrupts()
{
    EnterCriticalSection( &m_critSec );
    OUTREG32(&m_pbRegisters->MMCHS_ISE, 0);
    OUTREG32(&m_pbRegisters->MMCHS_IE,  0);
    LeaveCriticalSection( &m_critSec );
}

// Enable SDIO Interrupts.
VOID CSDIOControllerBase::EnableSDIOInterrupts()
{
    ASSERT( !m_fSDIOInterruptsEnabled );
    m_fSDIOInterruptsEnabled = TRUE;

    DEBUGMSG(SHC_INTERRUPT_ZONE, (TEXT("CSDHCSlot::EnableSDIOInterrupts\n")));
    EnterCriticalSection( &m_critSec );
    SETREG32(&m_pbRegisters->MMCHS_CON, MMCHS_CON_CTPL);

    // enable exit from smart idle mode on SD/SDIO card interrupt
    SETREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_ISE_CIRQ);
    // enable SD/SDIO card interrupt
    SETREG32(&m_pbRegisters->MMCHS_IE,  MMCHS_IE_CIRQ);
    LeaveCriticalSection( &m_critSec );
}


// Acknowledge an SDIO Interrupt.
VOID CSDIOControllerBase::AckSDIOInterrupt(
    )
{
    ASSERT( m_fSDIOInterruptsEnabled );
    DEBUGMSG(SHC_INTERRUPT_ZONE, (TEXT("CSDHCSlot::AckSDIOInterrupt\n")));
    DWORD dwRegValue = Read_MMC_STAT();
    if( dwRegValue & MMCHS_STAT_CIRQ )
    {
        Set_MMC_STAT(MMCHS_STAT_CIRQ);
        SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceInterrupting);
    }
    else
    {
        EnterCriticalSection( &m_critSec );
        SETREG32(&m_pbRegisters->MMCHS_IE,  MMCHS_IE_CIRQ);
        LeaveCriticalSection( &m_critSec );
        m_fSDIOInterruptInService = FALSE;
    }
}


// Disable SDIO Interrupts.
VOID CSDIOControllerBase::DisableSDIOInterrupts()
{
    m_fSDIOInterruptsEnabled = FALSE;

    DEBUGMSG(SHC_INTERRUPT_ZONE, (TEXT("CSDHCSlot::DisableSDIOInterrupts\n")));

    EnterCriticalSection( &m_critSec );
    CLRREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_ISE_CIRQ);
    CLRREG32(&m_pbRegisters->MMCHS_IE,  MMCHS_IE_CIRQ);
    LeaveCriticalSection( &m_critSec );
}

//  Set clock rate based on HC capability
VOID CSDIOControllerBase::SetClockRate(PDWORD pdwRate)
{
    DWORD dwClockRate = *pdwRate;

    if(dwClockRate > m_dwMaxClockRate) dwClockRate = m_dwMaxClockRate;

    // calculate the register value
    DWORD dwDiv = (DWORD)((MMCSD_CLOCK_INPUT + dwClockRate - 1) / dwClockRate);

    DEBUGMSG(SHC_CLOCK_ZONE, (TEXT("actual wDiv = 0x%x  requested:0x%x"), dwDiv, *pdwRate));
    // Only 10 bits available for the divider, so mmc base clock / 1024 is minimum.
    if ( dwDiv > 0x03FF )
        dwDiv = 0x03FF;
   

    DEBUGMSG(SHC_CLOCK_ZONE, (TEXT("dwDiv = 0x%x 0x%x"), dwDiv, *pdwRate));

    // Program the divisor, but leave the rest of the register alone.
    INT32 dwRegValue = INREG32(&m_pbRegisters->MMCHS_SYSCTL);

    dwRegValue = (dwRegValue & ~MMCHS_SYSCTL_CLKD_MASK) | MMCHS_SYSCTL_CLKD(dwDiv);
    dwRegValue = (dwRegValue & ~MMCHS_SYSCTL_DTO_MASK) | MMCHS_SYSCTL_DTO(0x0e); // DTO
    dwRegValue &= ~MMCHS_SYSCTL_CEN;
    dwRegValue &= ~MMCHS_SYSCTL_ICE;

    CLRREG32(&m_pbRegisters->MMCHS_SYSCTL, MMCHS_SYSCTL_CEN);

    OUTREG32(&m_pbRegisters->MMCHS_SYSCTL, dwRegValue);

    SETREG32(&m_pbRegisters->MMCHS_SYSCTL, MMCHS_SYSCTL_ICE); // enable internal clock

    DWORD dwTimeout = 500;
    while(((INREG32(&m_pbRegisters->MMCHS_SYSCTL) & MMCHS_SYSCTL_ICS) != MMCHS_SYSCTL_ICS) && (dwTimeout>0))
    {
        dwTimeout--;
    }

    if(0==dwTimeout)
    {
        DEBUGMSG(ZONE_ENABLE_INFO, (TEXT("Timeout for ICS ")));
    }


    SETREG32(&m_pbRegisters->MMCHS_SYSCTL, MMCHS_SYSCTL_CEN);
    SETREG32(&m_pbRegisters->MMCHS_HCTL, MMCHS_HCTL_SDBP); // power up the card

    dwTimeout = 500;
    while(((INREG32(&m_pbRegisters->MMCHS_SYSCTL) & MMCHS_SYSCTL_CEN) != MMCHS_SYSCTL_CEN) && (dwTimeout>0))
    {
        dwTimeout--;
    }

    if(0==dwTimeout)
    {
        DEBUGMSG(ZONE_ENABLE_INFO, (TEXT("Timeout for CEN ")));
    }

    *pdwRate = MMCSD_CLOCK_INPUT / dwDiv;

    DEBUGMSG(SHC_CLOCK_ZONE,(TEXT("SDHCSetRate - Actual clock rate = 0x%x, MMCHS_SYSCTL = 0x%x\n"), *pdwRate, INREG32(&m_pbRegisters->MMCHS_SYSCTL)));
}


//
VOID CSDIOControllerBase::SetSDVSVoltage()
{
    UINT32 val1, val2;

    if ( m_dwSlot == MMCSLOT_1 )
    {
        if(m_dwCPURev == 1) // ES 1.0
        {
          val1 = MMCHS_CAPA_VS30;
          val2 = MMCHS_HCTL_SDVS_3V0;
        }
        else if(m_dwCPURev == 2) // ES 2.0
        {
          val1 = MMCHS_CAPA_VS18;
          val2 = MMCHS_HCTL_SDVS_1V8;
        }
        else if(m_dwCPURev == 3) // ES 2.1
        {
            if (m_LowVoltageSlot)
            {
                val1 = MMCHS_CAPA_VS18;
                val2 = MMCHS_HCTL_SDVS_1V8;
            }
            else
            {
                val1 = MMCHS_CAPA_VS30;
                val2 = MMCHS_HCTL_SDVS_3V0;
            }
        }
        else
        {
          val1 = MMCHS_CAPA_VS30;
          val2 = MMCHS_HCTL_SDVS_3V0;
        }

        SETREG32(&m_pbRegisters->MMCHS_CAPA, val1);
        SETREG32(&m_pbRegisters->MMCHS_HCTL, val2);
    }
    else if (m_dwSlot == MMCSLOT_2)
    {
        if(m_dwCPURev == 1) // ES 1.0
        {
          val1 = MMCHS_CAPA_VS18;
          val2 = MMCHS_HCTL_SDVS_1V8;
        }
        else if(m_dwCPURev == 2) // ES 2.0
        {
          val1 = MMCHS_CAPA_VS18;
          val2 = MMCHS_HCTL_SDVS_1V8;
        }
        else if(m_dwCPURev == 3) // ES 2.1
        {
            if (m_LowVoltageSlot)
            {
                val1 = MMCHS_CAPA_VS18;
                val2 = MMCHS_HCTL_SDVS_1V8;
            }
            else
            {
                val1 = MMCHS_CAPA_VS30;
                val2 = MMCHS_HCTL_SDVS_3V0;
            }
        }
        else
        {
          val1 = MMCHS_CAPA_VS18;
          val2 = MMCHS_HCTL_SDVS_1V8;
        }
        SETREG32(&m_pbRegisters->MMCHS_CAPA, val1);
        SETREG32(&m_pbRegisters->MMCHS_HCTL, val2);
    }
    else
    {
        DEBUGMSG(ZONE_ENABLE_ERROR, (L"MMC Slot number is not Valid\r\n"));
        return;
    }
}

#ifdef SDIO_PRINT_THREAD

DWORD WINAPI CSDIOControllerBase::SDHCPrintThread(LPVOID lpParameter)
{
    CSDIOControllerBase *pController = (CSDIOControllerBase*)lpParameter;
    return pController->SDHCPrintThreadImpl();
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCPrintThread - SDIO print thread for driver
//  Input:  lpParameter - pController - controller instance
//  Output:
//  Return: Thread exit status
//  Notes:
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI CSDIOControllerBase::SDHCPrintThreadImpl()
{
    DWORD dwEventVal;
    while (TRUE)
    {
        WaitForSingleObject(m_hPrintEvent, INFINITE);
        while (m_cmdRdIndex != m_cmdWrIndex)
        {
            dwEventVal = m_cmdArray[m_cmdRdIndex];

            if (dwEventVal & SD_FAST_PATH_AVAILABLE)
                dwEventVal & 0xFFFF));

            m_cmdRdIndex++;
            if (m_cmdRdIndex >= m_cmdArrSize)
                m_cmdRdIndex = 0;
        }
    }
    return 1;
}
#endif


///////////////////////////////////////////////////////////////////////////////
//  SDHCPowerTimerThread - SDIO power timer thread for driver
//  Input:  lpParameter - pController - controller instance
//  Output:
//  Return: Thread exit status
//  Notes:
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI CSDIOControllerBase::SDHCPowerTimerThread(LPVOID lpParameter)
{
    CSDIOControllerBase *pController = (CSDIOControllerBase*)lpParameter;
    return pController->SDHCPowerTimerThreadImpl();
}

//------------------------------------------------------------------------------
//
//  Function:  SDHCPowerTimerThread
//
//  timeout thread, checks to see if the power can be disabled.
//
DWORD CSDIOControllerBase::SDHCPowerTimerThreadImpl()
{
    DWORD nTimeout = INFINITE;

    CeSetThreadPriority(GetCurrentThread(), TIMERTHREAD_PRIORITY);

    while (TRUE)
    {
        WaitForSingleObject(m_hTimerEvent, nTimeout);

        if (m_bExitThread == TRUE) break;

        // serialize access to power state changes
        EnterCriticalSection(&m_critSec);

        // by the time this thread got the cs hTimerEvent may
        // have gotten resignaled.  Clear the event to  make
        // sure the activity timer thread isn't awaken prematurely
        //
        ResetEvent(m_hTimerEvent);

        // check if we need to reset the timer
        if (m_dwClockCnt == 0)
        {
            // We disable the power only when this thread
            // wakes-up twice in a row with no power state
            // change to D0.  This is achieved by using the
            // bDisablePower flag to determine if power state
            // changed since the last time this thread woke-up
            //
            if (m_bDisablePower == TRUE)
            {
                if (m_ActualPowerState < D3)
                {
                    // enable autoidle, disable wakeup, enable smart-idle, ClockActivity (interface and functional clocks may be switched off)
                    OUTREG32(&m_pbRegisters->MMCHS_SYSCONFIG, MMCHS_SYSCONFIG_AUTOIDLE | MMCHS_SYSCONFIG_SIDLEMODE(SIDLE_FORCE));
                    UpdateDevicePowerState();
                 }
                nTimeout = INFINITE;
            }
            else
            {
                // wait for activity time-out before shutting off power.
                m_bDisablePower = TRUE;
                nTimeout = (m_fCardInitialized && !m_dwSDIOCard) ? m_nNonSDIOActivityTimeout : m_nSDIOActivityTimeout;

            }
        }
        else
        {
            nTimeout = INFINITE;
        }
        LeaveCriticalSection(&m_critSec);
    }

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  UpdateSystemClock
//
//  This function enable/disable AutoIdle Mode
//
//
BOOL
CSDIOControllerBase::UpdateSystemClock(
           BOOL enable
           )
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+UpdateSystemClock()\r\n"));
    static BOOL bFirstTime = FALSE;
    if (enable)
    {
        EnterCriticalSection( &m_critSec );
        m_InternPowerState = D0;
        if(!m_dwClockCnt)
        {
            if (m_ActualPowerState >= D3)
            {
                UpdateDevicePowerState();
                // enable autoidle, disable wakeup, enable smart-idle, ClockActivity (interface and functional clocks may be switched off)
                OUTREG32(&m_pbRegisters->MMCHS_SYSCONFIG, MMCHS_SYSCONFIG_AUTOIDLE | MMCHS_SYSCONFIG_SIDLEMODE(SIDLE_IGNORE));
            }
        }
        m_bDisablePower = FALSE;
        LeaveCriticalSection( &m_critSec );
        InterlockedIncrement(&m_dwClockCnt);
    }
    else
    {
        InterlockedDecrement(&m_dwClockCnt);
        if(m_dwClockCnt < 0)
            m_dwClockCnt = 0;
        if(!m_dwClockCnt)
        {
            m_InternPowerState = D4;
            if (m_hTimerEvent != NULL)
                SetEvent(m_hTimerEvent);
            else
               UpdateDevicePowerState();
        }
    }

    return TRUE;
}

VOID CSDIOControllerBase::UpdateDevicePowerState()
{
    CEDEVICE_POWER_STATE curPowerState =  min(m_ExternPowerState,m_InternPowerState);
    if( m_hParentBus )
    {
        if((m_ActualPowerState == D4 &&  curPowerState < D4 ) || (m_ActualPowerState < D4 &&  curPowerState == D4 ))
            {
                PreparePowerChange(curPowerState);
            }
        SetDevicePowerState( m_hParentBus, curPowerState, NULL );
        if((m_ActualPowerState == D4 &&  curPowerState < D4 ) || (m_ActualPowerState < D4 &&  curPowerState == D4 ))
            {
                PostPowerChange(curPowerState);
            }
    }

    m_ActualPowerState = curPowerState;
}

// Send command without response
SD_API_STATUS CSDIOControllerBase::SendCmdNoResp (DWORD cmd, DWORD arg)
{
    DWORD MMC_CMD;
    DWORD dwTimeout;

    OUTREG32(&m_pbRegisters->MMCHS_STAT, 0xFFFFFFFF);
    dwTimeout = DATACMD_TIMEOUT_VALUE;
    while(((INREG32(&m_pbRegisters->MMCHS_PSTATE) & MMCHS_PSTAT_CMDI)) && (dwTimeout>0))
    {
        dwTimeout--;
    }

    MMC_CMD = MMCHS_INDX(cmd);
    MMC_CMD |= gwaCMD[cmd].flags;

    // Program the argument into the argument registers
    OUTREG32(&m_pbRegisters->MMCHS_ARG, arg);
    // Issue the command.
    OUTREG32(&m_pbRegisters->MMCHS_CMD, MMC_CMD);

    dwTimeout = 5000;
    DWORD dwVal;
    while(dwTimeout > 0)
    {
        dwTimeout --;
        dwVal = INREG32(&m_pbRegisters->MMCHS_STAT);
        if(dwVal & (MMCHS_STAT_CC | MMCHS_STAT_CTO | MMCHS_STAT_CERR)) break;
    }

    dwVal = INREG32(&m_pbRegisters->MMCHS_STAT);
    OUTREG32(&m_pbRegisters->MMCHS_STAT, dwVal);
    // always return 0 if no response needed
    return SD_API_STATUS_SUCCESS;
}

// Send init sequence to card
VOID CSDIOControllerBase::SendInitSequence()
{
    EnterCriticalSection( &m_critSec );
    OUTREG32(&m_pbRegisters->MMCHS_IE,  0xFFFFFEFF);
    SETREG32(&m_pbRegisters->MMCHS_CON, MMCHS_CON_INIT);

    DWORD dwCount;
    for(dwCount = 0; dwCount < 10; dwCount ++)
    {
        SendCmdNoResp(SD_CMD_GO_IDLE_STATE, 0xFFFFFFFF);
    }
    OUTREG32(&m_pbRegisters->MMCHS_STAT, 0xFFFFFFFF);
    CLRREG32(&m_pbRegisters->MMCHS_CON, MMCHS_CON_INIT);
    LeaveCriticalSection( &m_critSec );
}

// Issues the specified SDI command
SD_API_STATUS CSDIOControllerBase::SendCommand( PSD_BUS_REQUEST pRequest )
{
    DWORD MMC_CMD;
    DWORD Cmd = pRequest->CommandCode;
    DWORD Arg = pRequest->CommandArgument;
    UINT16 respType = pRequest->CommandResponse.ResponseType;
    DWORD dwRegVal;
    DWORD dwCountStart;


    m_TransferClass = (UINT16) pRequest->TransferClass;

    DEBUGMSG(SHC_SEND_ZONE, (TEXT("SendCommand() - Cmd = 0x%x Arg = 0x%x respType = 0x%x m_TransferClass = 0x%x\r\n"),
        Cmd, Arg, respType, m_TransferClass));

    if ((Cmd == SD_CMD_IO_RW_EXTENDED) || (Cmd == SD_CMD_IO_RW_DIRECT))
    {
        m_dwSDIOCard = 1;
    } else
    if ((Cmd == SD_CMD_MMC_SEND_OPCOND) || (Cmd == SD_CMD_GO_IDLE_STATE))
    {
        m_dwSDIOCard = 0;
    }

    // turn the clock on
    if(!(pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE))
       UpdateSystemClock(TRUE);

    if( m_TransferClass == SD_READ || m_TransferClass == SD_WRITE )
    {
        DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (TEXT("SendCommand (Cmd=0x%08X, Arg=0x%08x, RespType=0x%08X, Data=0x%x <%dx%d>) starts\r\n"),
            Cmd, Arg, respType, (m_TransferClass==SD_COMMAND)?FALSE:TRUE, pRequest->NumBlocks, pRequest->BlockSize ) );

        // get starting tick count for timeout
        dwCountStart = GetTickCount();

        while((INREG32(&m_pbRegisters->MMCHS_PSTATE) & MMCHS_PSTAT_DATI))
        {
            if (GetTickCount() - dwCountStart > DATACMD_TIMEOUT_VALUE)
            {
                DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("WARNING- SendCommand (Cmd=0x%08X, Arg=0x%08x, RespType=0x%08X, Data=0x%x) Data Line in use!\r\n"),
                Cmd, Arg, respType, (m_TransferClass==SD_COMMAND)?FALSE:TRUE) );
                break;
            }
        }
    }
    else
    {
        DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (TEXT("SendCommand (Cmd=0x%08X, Arg=0x%08x, RespType=0x%08X, Data=0x%x) starts\r\n"),
            Cmd, Arg, respType, (m_TransferClass==SD_COMMAND)?FALSE:TRUE) );

    }



    Write_MMC_STAT(0xFFFFFFFF);

    dwCountStart = GetTickCount();
    while((INREG32(&m_pbRegisters->MMCHS_PSTATE) & MMCHS_PSTAT_DATI))
    {
        if (GetTickCount() - dwCountStart > DATACMD_TIMEOUT_VALUE)
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("WARNING- SendCommand (Cmd=0x%08X, Arg=0x%08x, RespType=0x%08X, Data=0x%x) Data Line in use!\r\n"),
            Cmd, Arg, respType, (m_TransferClass==SD_COMMAND)?FALSE:TRUE) );
            break;
        }
    }

    MMC_CMD = MMCHS_INDX(Cmd);

    MMC_CMD |= gwaCMD[Cmd].flags;
    if ((Cmd == SD_CMD_SELECT_DESELECT_CARD) && (respType == NoResponse))
    {
        MMC_CMD &= ~MMCHS_RSP_MASK;
        MMC_CMD |= MMCHS_RSP_NONE;
    }

    m_fDMATransfer = FALSE;
    MMC_CMD &= ~MMCHS_CMD_DE;

    if (Cmd == SD_CMD_IO_RW_EXTENDED)
    {
        if(pRequest->NumBlocks > 1)
        {
           MMC_CMD |= MMCHS_CMD_MSBS | MMCHS_CMD_BCE;
        }
    }
    
    if( m_TransferClass == SD_READ )
    {
        MMC_CMD |= MMCHS_CMD_DDIR;

#ifdef SDIO_DMA_READ_ENABLED
       // if we can use the DMA for transfer...
       if( ((( pRequest->NumBlocks > 1) && !(pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)) || 
       (( pRequest->NumBlocks > 0) && (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE))) &&
            ( TRANSFER_SIZE(pRequest) % MIN_MMC_BLOCK_SIZE == 0 ) &&
            ( TRANSFER_SIZE(pRequest) <= CB_DMA_BUFFER ) )
        {
            MMC_CMD |= MMCHS_CMD_DE;
            m_fDMATransfer = TRUE;
            // program the DMA controller
            EnterCriticalSection( &m_critSec );
            SETREG32(&m_pbRegisters->MMCHS_IE, MMCHS_IE_BRR);
            SETREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_IE_BRR);
            LeaveCriticalSection( &m_critSec );
            SDIO_InitInputDMA( pRequest->NumBlocks,  pRequest->BlockSize);
        
       }

#endif
        dwRegVal = (DWORD)(pRequest->BlockSize & 0xFFFF);
        dwRegVal += ((DWORD)(pRequest->NumBlocks & 0xFFFF)) << 16;
        OUTREG32(&m_pbRegisters->MMCHS_BLK, dwRegVal);
    }
    else if( m_TransferClass == SD_WRITE )
    {
        MMC_CMD &= ~MMCHS_CMD_DDIR;

#ifdef SDIO_DMA_WRITE_ENABLED
        // if we can use the DMA for transfer...
       if( ((( pRequest->NumBlocks > 1) && !(pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)) || 
       (( pRequest->NumBlocks > 0) && (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE))) &&
            ( TRANSFER_SIZE(pRequest) % MIN_MMC_BLOCK_SIZE == 0 ) &&
            ( TRANSFER_SIZE(pRequest) <= CB_DMA_BUFFER ) )
        {
           
            MMC_CMD |= MMCHS_CMD_DE;
            m_fDMATransfer = TRUE;
            // program the DMA controller
            EnterCriticalSection( &m_critSec );
            SETREG32(&m_pbRegisters->MMCHS_IE, MMCHS_IE_BWR);
            SETREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_ISE_BWR);
            LeaveCriticalSection( &m_critSec );
            SDIO_InitOutputDMA( pRequest->NumBlocks,  pRequest->BlockSize );
            
        }
        
#endif
        dwRegVal = (DWORD)(pRequest->BlockSize & 0xFFFF);
        dwRegVal += ((DWORD)(pRequest->NumBlocks & 0xFFFF)) << 16;
        OUTREG32(&m_pbRegisters->MMCHS_BLK, dwRegVal);
    }

    //ACMD13 has associated data, but the CMD13 doesnt have data
    //Hence we need to set the Data Present Select (CMD:DP- bit 21) for ACMD13 alone
    if(m_fAppCmdMode && (pRequest->CommandCode == SD_ACMD_SD_STATUS))
    {
       MMC_CMD |= MMCHS_CMD_DP;
    }

    //Both SD_CMD_MMC_SEND_EXT_CSD, and SD_CMD_SEND_IF_COND maps to CMD 8
    //SD_CMD_MMC_SEND_EXT_CSD (which is MMC v4.x specific) has associated data, and SD_CMD_SEND_IF_COND (which is SD specific) doesnt have data
    //Hence we need to set the Data Present Select (CMD:DP- bit 21) for CMD 8 for MMC.
    if(m_fMMCMode&& (pRequest->CommandCode == SD_CMD_MMC_SEND_EXT_CSD))
    {
       MMC_CMD |= MMCHS_CMD_DP;
    }

    //check for card initialization is done.
    if(!m_fCardInitialized && (Cmd == SD_CMD_READ_SINGLE_BLOCK))
        m_fCardInitialized = TRUE;

    // Program the argument into the argument registers
    OUTREG32(&m_pbRegisters->MMCHS_ARG, Arg);

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("*** SendCommand() - registers:Command = %d (0x%x), Arg 0x%x, MMCHS_ARG = 0x%x\n"), Cmd, MMC_CMD, Arg, INREG32(&m_pbRegisters->MMCHS_ARG)));


    // Issue the command.
    OUTREG32(&m_pbRegisters->MMCHS_CMD, MMC_CMD);

    return SD_API_STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Remove the device instance in the slot
VOID CSDIOControllerBase::HandleRemoval(BOOL fCancelRequest)
{
    m_fCardPresent = FALSE;
    m_fMMCMode = FALSE;

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("HandleRemoval\n")));

    if(m_TransferClass == SD_WRITE || m_TransferClass == SD_READ)
    {
        SetEvent(m_hControllerISTEvent);
        Sleep(100);
    }

    IndicateSlotStateChange(DeviceEjected);
    SystemClockOn();

    // turn off SDIO interrupts
    DisableSDIOInterrupts();

    if (fCancelRequest)
    {
        // get the current request
        PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

        if (pRequest != NULL)
        {
            DEBUGMSG(SDCARD_ZONE_WARN,
                (TEXT("Card Removal Detected - Canceling current request: 0x%08X, command: %d\n"),
                pRequest, pRequest->CommandCode));
            DumpRequest(pRequest);
            IndicateBusRequestComplete(pRequest, SD_API_STATUS_DEVICE_REMOVED);
            //SystemClockOff();
        }
    }
    if(m_ActualPowerState == D4) return;
    SoftwareReset(SOFT_RESET_ALL);
    DisableSDHCInterrupts();
    // turn clock off and remove power from the slot
    OUTREG32(&m_pbRegisters->MMCHS_SYSCONFIG, MMCHS_SYSCONFIG_AUTOIDLE | MMCHS_SYSCONFIG_SIDLEMODE(SIDLE_FORCE));
    ResetEvent(m_hControllerISTEvent);
    Sleep(100);

    m_fCardInitialized = FALSE;

    // get and lock the current bus request
    while(SDHCDGetAndLockCurrentRequest(m_pHCContext, 0) != NULL)
    {
        CommandCompleteHandler();
    }

    TurnCardPowerOff();  // try to turn slot power off
    UpdateSystemClock(FALSE);  // try to turn MMC clocks and slot off.  

}

// Initialize the card
VOID CSDIOControllerBase::HandleInsertion()
{
    DWORD dwClockRate = SD_DEFAULT_CARD_ID_CLOCK_RATE;

    m_fCardPresent = TRUE;
    m_dwSDIOCard = 0;

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("HandleInsertion\n")));

    // turn power to the card on
    TurnCardPowerOn();

    SystemClockOn();

    SoftwareReset(SOFT_RESET_ALL);

    // Check for debounce stable
    DWORD dwTimeout = 5000;
    while(((INREG32(&m_pbRegisters->MMCHS_PSTATE) & 0x00020000)!= 0x00020000) && (dwTimeout>0))
    {
        dwTimeout--;
    }

    OUTREG32(&m_pbRegisters->MMCHS_CON, 0x01 << 7); // CDP

    SetSDVSVoltage();

    SetClockRate(&dwClockRate);
    if(m_LowVoltageSlot && m_dwSlot == MMCSLOT_1 && m_dwCPURev == 3)
    {
        SendInitSequence();
    }
    EnableSDHCInterrupts();
    InterruptDone( m_dwControllerSysIntr );

    // indicate device arrival
    IndicateSlotStateChange(DeviceInserted);
}

// Static interrupt routine for the entire controller.
DWORD CSDIOControllerBase::HandleCardDetectInterrupt()
{
    DWORD eStatus = SD_TIMEOUT;
    DEBUGMSG(SHC_INTERRUPT_ZONE, (TEXT("CSDHCSlot::HandleCardDetectInterrupt \n")));

    SDHCDAcquireHCLock(m_pHCContext);

    if( SDCardDetect() )
    {
        DEBUGMSG(SHC_INTERRUPT_ZONE, (TEXT("SDHCControllerIst - Card is Inserted! \n")));

        if( m_fCardPresent == FALSE || m_bReinsertTheCard )
        {
            m_bReinsertTheCard = FALSE;
            m_fFirstTime = TRUE;
            HandleInsertion();
        }
        eStatus = SD_INSERT;
    }
    else if( m_fCardPresent )
    {
        DEBUGMSG(SHC_INTERRUPT_ZONE, (TEXT("SDHCControllerIst - Card is Removed! \n")));

        HandleRemoval(TRUE);
        eStatus = SD_REMOVE;
    } 
    else
    {
        DEBUGMSG(SHC_INTERRUPT_ZONE, (TEXT("SDHCControllerIst - no card detected! \n")));

        // this is a bad idea, card detect pin is likely to generate multiple interrupts, 
        // extra interrupt after card removal would cause SDHC driver to stop working.
        // turn clock off and remove power from the slot
        //SystemClockOff();

        // remove power from the slot (should already be off)
        TurnCardPowerOff();
        eStatus = SD_TIMEOUT;
    }

    SDHCDReleaseHCLock(m_pHCContext);
    return eStatus;
}

#ifdef DEBUG

// Reads from SD Standard Host registers and writes them to the debugger.
VOID CSDIOControllerBase::DumpRegisters()
{
    EnterCriticalSection( &m_critSec );
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("+DumpStdHCRegs-------------------------\n")));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_CMD 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_CMD)    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_ARG 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_ARG)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_CON  0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_CON)   ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_PWCNT  0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_PWCNT)   ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_STAT 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_STAT)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_PSTATE 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_PSTATE)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_IE 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_IE)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_ISE 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_ISE)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_BLK 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_BLK)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_REV 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_REV)    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_RSP10 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_RSP10)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_RSP32 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_RSP32)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_RSP54 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_RSP54)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_RSP76 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_RSP76)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_HCTL 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_HCTL)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_SYSCTL 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_SYSCTL)  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_SYSCONFIG 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_SYSCONFIG) ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_CAPA 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_CAPA) ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MMCHS_CUR_CAPA 0x%08X \n"), INREG32(&m_pbRegisters->MMCHS_CUR_CAPA) ));
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("-DumpStdHCRegs-------------------------\n")));
    LeaveCriticalSection( &m_critSec );
}

#endif

CSDIOControllerBase::~CSDIOControllerBase()
{
    // release dvfs resources
    if (hDVFSAckEvent != NULL) CloseHandle(hDVFSAckEvent);
    if (hDVFSActivityEvent != NULL) CloseHandle(hDVFSActivityEvent);

    // Delete critical section
    DeleteCriticalSection(&csDVFS);

    DeleteCriticalSection( &m_critSec );
    DeleteCriticalSection( &m_powerCS );
}

BOOL CSDIOControllerBase::Init( LPCTSTR pszActiveKey )
{
    SD_API_STATUS      status;              // SD status
    HKEY               hKeyDevice = NULL;   // device key
    CReg               regDevice;           // encapsulated device key
    DWORD              dwRet = 0;           // return value
    BOOL               fRegisteredWithBusDriver = FALSE;
    BOOL               fHardwareInitialized = FALSE;
    DWORD              dwIDVal;
    OMAP_DEVICE         id = OMAP_DEVICE_GPTIMER10;

    PHYSICAL_ADDRESS PortAddress;

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC +Init\n")));

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC Active RegPath: %s \n"),pszActiveKey));

    if (KernelIoControl(
                IOCTL_HAL_GET_CPUID, NULL, 0, &dwIDVal, sizeof(dwIDVal), NULL
                ))
    {
        m_dwCPURev = (dwIDVal >> 28) + 1;

    } else
    {
        m_dwCPURev = 1;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC: Failed to detect cpu_rev\n")));
    }

    // open the parent bus driver handle
    m_hParentBus = CreateBusAccessHandle(pszActiveKey);
    if ( m_hParentBus == NULL )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC: Failed to obtain parent bus handle\n")));
        goto cleanUp;
    }

    hKeyDevice = OpenDeviceKey(pszActiveKey);
    if ( (hKeyDevice == NULL) || !regDevice.Open(hKeyDevice, NULL) )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC: Failed to open device key\n")));
        goto cleanUp;
    }

    // allocate the context - we only support one slot
    status = SDHCDAllocateContext(1, &m_pHCContext);
    if (!SD_API_SUCCESS(status))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC Failed to allocate context : 0x%08X \n"),
            status));
        goto cleanUp;
    }

    // Set our extension
    m_pHCContext->pHCSpecificContext = this;

    if( !GetRegistrySettings(&regDevice) )
    {
        ERRORMSG(1, (_T("SDHCInitialize:: Error reading registry settings\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    if (GetDeviceRegistryParams(
        pszActiveKey, this, dimof(s_deviceRegParams), s_deviceRegParams
        ) != ERROR_SUCCESS)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"ERROR: CSDIOControllerBase:Init: "
            L"Failed read SDHC driver registry parameters\r\n"
            ));
        goto cleanUp;
    }


    switch(m_dwMemBase)
    {
      case OMAP_MMCHS1_REGS_PA:
           m_dwSlot = MMCSLOT_1;
           break;
      case OMAP_MMCHS2_REGS_PA:
           m_dwSlot = MMCSLOT_2;
           break;
    }

    // Open the GPIO driver
    m_hGPIO = GPIOOpen();
    if( m_hGPIO == NULL )
    {
        ERRORMSG(1, (_T("SDHCInitialize:: Error opening the GPIO driver!\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    // map hardware memory space

    PortAddress.QuadPart = m_dwMemBase;
    m_pbRegisters = (OMAP_MMCHS_REGS *)MmMapIoSpace(PortAddress, m_dwMemLen, FALSE );
    if ( !m_pbRegisters )
    {
        ERRORMSG(1, (_T("SDHCInitialize:: Error allocating SDHC controller register\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    if( !InitializeHardware() )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating CD/RW GPIO registers\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    fHardwareInitialized = TRUE;

    //3430-ES1.0  Sil errata fix
    BusIoControl(
        m_hParentBus, IOCTL_BUS_REQUEST_CLOCK, &id, sizeof(id),
        NULL, 0, NULL, NULL
        );

    // turn power and system clocks on
    m_ExternPowerState = D0;
    m_InternPowerState = D0;
    //m_ActualPowerState = D0;
    UpdateDevicePowerState();

    // Initialize the slot
    SoftwareReset(SOFT_RESET_ALL);
    Sleep(10); // Allow time for card to power down after a device reset
#ifdef DEBUG
    DumpRegisters();
#endif
    // Read SD Host Controller Info from register.
    if (!InterpretCapabilities())
    {
        goto cleanUp;
    }

    // now register the host controller
    status = SDHCDRegisterHostController(m_pHCContext);

    if (!SD_API_SUCCESS(status))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC Failed to register host controller: %0x08X \n"),status));
        goto cleanUp;
    }

    fRegisteredWithBusDriver = TRUE;

    EnableSDHCInterrupts();

    m_dwSDIOCard = 0;
    // return the controller context
    dwRet = (DWORD) this;

cleanUp:
    if (hKeyDevice) RegCloseKey(hKeyDevice);

    if ( (dwRet == 0) && m_pHCContext )
    {
        FreeHostContext( fRegisteredWithBusDriver, fHardwareInitialized );
    }

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC -Init\n")));

    return dwRet;
}


// Free the host context and associated resources.
VOID CSDIOControllerBase::FreeHostContext( BOOL fRegisteredWithBusDriver, BOOL fHardwareInitialized )
{
    DEBUGCHK(m_pHCContext);

    if (fRegisteredWithBusDriver)
    {
        // deregister the host controller
        SDHCDDeregisterHostController(m_pHCContext);
    }

    // unmap hardware memory space

    DeinitializeHardware();
    if (m_pbRegisters) MmUnmapIoSpace((PVOID)m_pbRegisters, sizeof(OMAP_MMCHS_REGS));
    if( m_hParentBus != NULL )
    {
        m_InternPowerState = m_ExternPowerState = D4;
        UpdateDevicePowerState();
        CloseBusAccessHandle( m_hParentBus );
        m_hParentBus = NULL;
    }

    if( m_hGPIO != NULL )
    {
        GPIOClose( m_hGPIO );
        m_hGPIO = NULL;
    }

    // cleanup the host context
    SDHCDDeleteContext(m_pHCContext);
}

// Read the registry settings
BOOL CSDIOControllerBase::GetRegistrySettings(CReg *pReg)
{
    BOOL fRet = TRUE;

    DEBUGCHK(pReg);

     // get the Command and Data timeouts
    m_wCTOTimeout = (WORD) pReg->ValueDW(SHC_CTO_TIMEOUT, MMC_CTO_CONTROL_DEFAULT);
    m_wCTOTimeout = min(m_wCTOTimeout, MMC_CTO_CONTROL_MAX);
    m_wDTOTimeout = (WORD) pReg->ValueDW(SHC_DTO_TIMEOUT, MMC_DTO_CONTROL_DEFAULT);
    m_wDTOTimeout = min(m_wDTOTimeout, MMC_DTO_CONTROL_MAX);

    // get the controller IST thread priority
    m_dwSDIOPriority = pReg->ValueDW(SHC_SDIO_PRIORITY_KEY, SHC_CARD_CONTROLLER_PRIORITY);

    // get the card detect IST thread priority
    m_dwCDPriority = pReg->ValueDW(SHC_CD_PRIORITY_KEY, SHC_CARD_DETECT_PRIORITY);

    // get the max clock frequency from the registry (we allow the registry to override)
    m_dwMaxClockRate = pReg->ValueDW(SHC_FREQUENCY_KEY);
    if (m_dwMaxClockRate == 0)
    {
        // No clock frequency specified. Use the highest possible that
        // could have been specified so that a working clock divisor
        // will be chosen.
        m_dwMaxClockRate = STD_HC_MAX_CLOCK_FREQUENCY;
    }
    else
    {
        m_dwMaxClockRate = min(m_dwMaxClockRate,
            STD_HC_MAX_CLOCK_FREQUENCY);
    }

    // get the read/write timeout value
    m_dwMaxTimeout = pReg->ValueDW(SHC_RW_TIMEOUT_KEY, DEFAULT_TIMEOUT_VALUE);

    // get the wakeup sources
    m_dwWakeupSources = pReg->ValueDW(SHC_WAKEUP_SOURCES_KEY, 0);
    m_dwCurrentWakeupSources = m_dwWakeupSources & (~WAKEUP_SDIO);

    return fRet;
}

// Process the capabilities register
BOOL CSDIOControllerBase::InterpretCapabilities()
{
    BOOL fRet = TRUE;

    // set the host controller name
    SDHCDSetHCName(m_pHCContext, TEXT("SDHC"));

    // set init handler
    SDHCDSetControllerInitHandler(m_pHCContext, CSDIOControllerBase::SDHCInitialize);

    // set deinit handler
    SDHCDSetControllerDeinitHandler(m_pHCContext, CSDIOControllerBase::SDHCDeinitialize);

    // set the Send packet handler
    SDHCDSetBusRequestHandler(m_pHCContext, CSDIOControllerBase::SDHCBusRequestHandler);

    // set the cancel I/O handler
    SDHCDSetCancelIOHandler(m_pHCContext, CSDIOControllerBase::SDHCCancelIoHandler);

    // set the slot option handler
    SDHCDSetSlotOptionHandler(m_pHCContext, CSDIOControllerBase::SDHCSlotOptionHandler);

    // set maximum block length
    m_usMaxBlockLen = STD_HC_MAX_BLOCK_LENGTH;

    return fRet;
}

///////////////////////////////////////////////////////////////////////////////
//  SDHCControllerIstThread - SDIO/controller IST thread for driver
//  Input:  lpParameter - pController - controller instance
//  Output:
//  Return: Thread exit status
//  Notes:
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI CSDIOControllerBase::SDHCControllerIstThread(LPVOID lpParameter)
{
    CSDIOControllerBase *pController = (CSDIOControllerBase*)lpParameter;
    return pController->SDHCControllerIstThreadImpl();
}

///////////////////////////////////////////////////////////////////////////////
//  SDHCControllerIstThreadImpl - implementation of SDIO/controller IST thread
//                                for driver
//  Input:
//  Output:
//  Return: Thread exit status
//  Notes:
///////////////////////////////////////////////////////////////////////////////
DWORD CSDIOControllerBase::SDHCControllerIstThreadImpl()
{
    DWORD dwWaitTime    = INFINITE;

    if (!CeSetThreadPriority(GetCurrentThread(), m_dwSDIOPriority))
    {
        DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCControllerIstThread: warning, failed to set CEThreadPriority \n")));
    }

    InterruptDone( m_dwControllerSysIntr );

    while (TRUE)
    {
        // wait for the SDIO/controller interrupt
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hControllerISTEvent, dwWaitTime))
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("CSDIOControllerBase::SDHCControllerIstThread(): Wait Failed!\n")));
            break;
        }

        if (m_fDriverShutdown)
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("CSDIOControllerBase::SDHCControllerIstThread(): Thread exiting!\n")));
            break;
        }

        if(m_ActualPowerState == D4)
        {
            InterruptDone( m_dwControllerSysIntr );
            ResetEvent(m_hControllerISTEvent);
            continue;
        }

        DWORD dwStat;
        UpdateSystemClock(TRUE);

        dwStat = Read_MMC_STAT();

        EnterCriticalSection( &m_critSec );
        dwStat &= INREG32(&m_pbRegisters->MMCHS_IE);
        LeaveCriticalSection( &m_critSec );
        if( dwStat & (MMCHS_STAT_CC|MMCHS_STAT_CERR|MMCHS_STAT_CCRC|MMCHS_STAT_CTO|MMCHS_STAT_DTO|MMCHS_STAT_DCRC) )
        {
            CommandCompleteHandler();
        }
        if( dwStat & MMCHS_STAT_CIRQ )
        {

            ASSERT( m_fSDIOInterruptsEnabled );
            // indicate that the card is interrupting
            DEBUGMSG(SHC_INTERRUPT_ZONE, (TEXT("CSDIOControllerBase::SDHCControllerIstThread: got SDIO interrupt!\n")));
            DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (TEXT("Received SDIO interrupt\r\n")));

            // disable the SDIO interrupt
            EnterCriticalSection( &m_critSec );
            CLRREG32(&m_pbRegisters->MMCHS_IE, MMCHS_IE_CIRQ);
            // ??? customer reports that this line is needed, needs review
            //CLRREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_IE_CIRQ);
            LeaveCriticalSection( &m_critSec );

            // notify the SDBusDriver of the SDIO interrupt
            m_fSDIOInterruptInService = TRUE;
            SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceInterrupting);

            // ??? Note: Customer reports delay here will fix SDIO WiFi card problems.
            // Tests with failing Socket 802.11b SDIO card showed no improvement. 
            // Not a valid fix - it would cause a serious performance degredation.
            //Sleep(10);
        }
        InterruptDone( m_dwControllerSysIntr );

        UpdateSystemClock(FALSE);
    }

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHCCardDetectIstThread: Thread Exiting\n")));
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//  SDHCCardDetectIstThread - card detect IST thread for driver
//  Input:  lpParameter - pController - controller instance
//  Output:
//  Return: Thread exit status
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI CSDIOControllerBase::SDHCCardDetectIstThread(LPVOID lpParameter)
{
    CSDIOControllerBase *pController = (CSDIOControllerBase*)lpParameter;
    return pController->SDHCCardDetectIstThreadImpl();
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::SDHCDeinitializeImpl - Deinitialize the SDHC Controller
//  Input:
//  Output:
//  Return: SD_API_STATUS
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::SDHCDeinitializeImpl()
{
    // mark for shutdown
    m_fDriverShutdown = TRUE;

    if (m_fInitialized)
    {
        if( m_dwControllerSysIntr != SYSINTR_UNDEFINED )
        {
            // disable wakeup on SDIO interrupt
            if ( m_dwCurrentWakeupSources & WAKEUP_SDIO )
            {
                KernelIoControl( IOCTL_HAL_DISABLE_WAKE,
                    &m_dwControllerSysIntr,
                    sizeof( m_dwControllerSysIntr ),
                    NULL,
                    0,
                    NULL );
            }

            // disable controller interrupt
            InterruptDisable(m_dwControllerSysIntr);

            // release the SYSINTR value
            KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwControllerSysIntr, sizeof(DWORD), NULL, 0, NULL);
            m_dwControllerSysIntr = (DWORD) SYSINTR_UNDEFINED;
        }


        if (m_fCardPresent)
        {
           // remove device
           HandleRemoval(FALSE);
        }
    }

    // clean up controller IST
    if (NULL != m_htControllerIST)
    {
        // wake up the IST
        SetEvent(m_hControllerISTEvent);
        // wait for the thread to exit
        WaitForSingleObject(m_htControllerIST, INFINITE);
        CloseHandle(m_htControllerIST);
        m_htControllerIST = NULL;
    }

    // free controller interrupt event
    if (NULL != m_hControllerISTEvent)
    {
        CloseHandle(m_hControllerISTEvent);
        m_hControllerISTEvent = NULL;
    }

    // clean up card detect IST
    if (NULL != m_htCardDetectIST)
    {
        // wake up the IST
        SetEvent(m_hCardDetectEvent);
        // wait for the thread to exit
        WaitForSingleObject(m_htCardDetectIST, INFINITE);
        CloseHandle(m_htCardDetectIST);
        m_htCardDetectIST = NULL;
    }

    // free card detect interrupt event
    if (NULL != m_hCardDetectEvent)
    {
        CloseHandle(m_hCardDetectEvent);
        m_hCardDetectEvent = NULL;
    }
    // clean up power thread
    if (NULL != m_hTimerThreadIST)
        {
        m_bExitThread = TRUE;
        SetEvent(m_hTimerEvent);
        WaitForSingleObject(m_hTimerThreadIST, INFINITE);
        CloseHandle(m_hTimerThreadIST);
        m_hTimerThreadIST = NULL;
        }

    if (m_hTimerEvent != NULL)
        {
        CloseHandle(m_hTimerEvent);
        }

#ifdef SDIO_DMA_ENABLED
    SDIO_DeinitDMA();
#endif
    return SD_API_STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::SDHCInitialize - Initialize the the controller
//  Input:
//  Output:
//  Return: SD_API_STATUS
//  Notes:
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::SDHCInitializeImpl()
{
    SD_API_STATUS status = SD_API_STATUS_INSUFFICIENT_RESOURCES; // intermediate status
    DMA_ADAPTER_OBJECT dmaAdapter;
    DWORD         threadID;
    DWORD *pdwSDIOIrq;
    DWORD dwSDIOIrqLen;
    DWORD dwClockRate;

    // allocate the DMA buffer
    dmaAdapter.ObjectSize = sizeof(dmaAdapter);
    dmaAdapter.InterfaceType = Internal;
    dmaAdapter.BusNumber = 0;
    m_pDmaBuffer = (PBYTE)HalAllocateCommonBuffer( &dmaAdapter, CB_DMA_BUFFER, &m_pDmaBufferPhys, FALSE );
    ASSERT(m_pDmaBuffer);

    if( m_pDmaBuffer == NULL )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeInstance:: Error allocating DMA buffer!\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    m_fDriverShutdown = FALSE;
    SoftwareReset(SOFT_RESET_ALL);

    SystemClockOn();

    dwClockRate = MMCSD_CLOCK_INIT;
    SetClockRate(&dwClockRate);

    // deactivate the clock
    //SystemClockOff();

    // convert the SDIO hardware IRQ into a logical SYSINTR value
    DWORD rgdwSDIOIrq[] = { IRQ_MMC1, IRQ_MMC2 };
    if(m_dwSlot == MMCSLOT_1)
      pdwSDIOIrq = &rgdwSDIOIrq[0];
    else
      pdwSDIOIrq = &rgdwSDIOIrq[1];
    dwSDIOIrqLen = sizeof(DWORD);

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, pdwSDIOIrq, dwSDIOIrqLen, &m_dwControllerSysIntr, sizeof(DWORD), NULL))
    {
        // invalid SDIO SYSINTR value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("Error obtaining SDIO SYSINTR value!\n")));
        m_dwControllerSysIntr = (DWORD)SYSINTR_UNDEFINED;
        status = SD_API_STATUS_UNSUCCESSFUL;
        goto cleanUp;
    }

    // allocate the interrupt event for the SDIO/controller interrupt
    m_hControllerISTEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if (NULL == m_hControllerISTEvent)
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    if ( !InterruptInitialize( m_dwControllerSysIntr, m_hControllerISTEvent, NULL, 0 ) )
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    // allocate the interrupt event for card detection
    m_hCardDetectEvent = CreateEvent(NULL, FALSE, FALSE,NULL);

    if (NULL == m_hCardDetectEvent)
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    // create the Controller IST thread
    m_htControllerIST = CreateThread(NULL,
        0,
        CSDIOControllerBase::SDHCControllerIstThread,
        this,
        0,
        &threadID);

    if (NULL == m_htControllerIST) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    // create the card detect IST thread
    m_htCardDetectIST = CreateThread(NULL,
        0,
        CSDIOControllerBase::SDHCCardDetectIstThread,
        this,
        0,
        &threadID);

    if (NULL == m_htCardDetectIST)
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

#ifdef SDIO_PRINT_THREAD

        m_cmdRdIndex = m_cmdWrIndex = 0;
    // allocate the interrupt event for card detection
    m_hPrintEvent = CreateEvent(NULL, FALSE, FALSE,NULL);

    if (NULL == m_hPrintEvent)
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    // create the card detect IST thread
    m_hPrintIST = CreateThread(NULL,
        0,
        CSDIOControllerBase::SDHCPrintThread,
        this,
        0,
        &threadID);

    if (NULL == m_hPrintIST)
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

#endif

    // start timer thread
    m_bDisablePower = FALSE;
    m_hTimerThreadIST = NULL;
    m_hTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hTimerEvent != NULL)
        {
        m_hTimerThreadIST = CreateThread(NULL,
            0,
            CSDIOControllerBase::SDHCPowerTimerThread,
            this,
            0,
            &threadID);
        }

    if (NULL == m_hTimerThreadIST)
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    m_fInitialized = TRUE;

    // on start we need the IST to check the slot for a card
    SetEvent(m_hCardDetectEvent);

#ifdef SDIO_DMA_ENABLED
    SDIO_InitDMA();
#endif

    status = SD_API_STATUS_SUCCESS;

cleanUp:

    if (!SD_API_SUCCESS(status))
    {
        // just call the deinit handler directly to cleanup
        SDHCDeinitializeImpl();
    }
    return status;
}


///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::SDHCSDCancelIoHandlerImpl - io cancel handler
//  Input:  Slot - slot the request is going on
//          pRequest - the request to be cancelled
//
//  Output:
//  Return: TRUE if I/O was cancelled
//  Notes:
//          the HC lock is taken before entering this cancel handler
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN CSDIOControllerBase::SDHCCancelIoHandlerImpl(UCHAR Slot, PSD_BUS_REQUEST pRequest)
{
    // for now, we should never get here because all requests are non-cancelable
    // the hardware supports timeouts so it is impossible for the controller to get stuck
    DEBUGCHK(FALSE);

    return TRUE;
}


SD_API_STATUS CSDIOControllerBase::SDHCBusRequestHandlerImpl_FastPath(PSD_BUS_REQUEST pRequest)
{
    DEBUGCHK(pRequest);

    SD_API_STATUS   status = SD_API_STATUS_FAST_PATH_SUCCESS;
    DWORD            dwIe;
    m_fastPathReq = 1;

    // acquire the device lock to protect from device removal
    SDHCDAcquireHCLock(m_pHCContext);
    //m_fastPathSDMEM = 1;

    // turn the clock on
    UpdateSystemClock(TRUE);

    // Disable SDIO interrupt for Fast path
    dwIe = (DWORD)INREG32(&(m_pbRegisters->MMCHS_IE));
    EnterCriticalSection( &m_critSec );
    SETREG32(&m_pbRegisters->MMCHS_CON, MMCHS_CON_CTPL);

    SETREG32(&(m_pbRegisters->MMCHS_IE) , MMC_INT_EN_MASK);
    CLRREG32(&(m_pbRegisters->MMCHS_ISE) , MMC_INT_EN_MASK);
    OUTREG32(&m_pbRegisters->MMCHS_STAT, 0xFFFFFFFF);


    LeaveCriticalSection( &m_critSec );

    status = SendCommand(pRequest);
    if (!SD_API_SUCCESS(status))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - SendCommand failed - command:0x%02x, SendCommand returned 0x%x\r\n"), pRequest->CommandCode));
        SDHCDReleaseHCLock(m_pHCContext);
        goto cleanUp;      
    }

    {
       DWORD retries = 0;
       status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
 
       // polling end-of-command
       while (!(Read_MMC_STAT() & MMCHS_STAT_CC)) {
           if (retries > SDIO_MAX_LOOP) {
                SDHCDReleaseHCLock(m_pHCContext);
                if(Read_MMC_STAT() & MMCHS_STAT_CTO)
                {
                    status = SD_API_STATUS_RESPONSE_TIMEOUT;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - MMCHS_STAT_CTO\r\n")));
                }
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - Timeout waiting for MMCHS_STAT_CC\r\n")));
                goto cleanUp;      
           }
           retries++;
       }

       status = CommandCompleteHandler_FastPath(pRequest);

       SDHCDReleaseHCLock(m_pHCContext);
    }

cleanUp:


    // Restore SDIO interrupts
    EnterCriticalSection( &m_critSec );

    CLRREG32(&(m_pbRegisters->MMCHS_IE) , MMC_INT_EN_MASK);
    SETREG32(&(m_pbRegisters->MMCHS_IE), (dwIe & (MMCHS_IE_CIRQ | MMC_INT_EN_MASK)));
    SETREG32(&(m_pbRegisters->MMCHS_ISE), (dwIe & (MMCHS_ISE_CIRQ | MMC_INT_EN_MASK )));
    OUTREG32(&m_pbRegisters->MMCHS_STAT, 0xFFFFFFFF);

    LeaveCriticalSection( &m_critSec );

    return status;

}

SD_API_STATUS CSDIOControllerBase::SDHCBusRequestHandlerImpl_NormalPath(PSD_BUS_REQUEST pRequest)
{
    DEBUGCHK(pRequest);

    SD_API_STATUS   status = SD_API_STATUS_FAST_PATH_SUCCESS;
    DEBUGCHK(pRequest);

    SDHCDAcquireHCLock(m_pHCContext);

    status = SendCommand(pRequest);

    if(!SD_API_SUCCESS(status))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - SendCommand failed - command:0x%02x, SendCommand returned 0x%x\r\n"), pRequest->CommandCode));
        SDHCDReleaseHCLock(m_pHCContext);
        goto cleanUp;      
    }
    // we will handle the command response interrupt on another thread
    status = SD_API_STATUS_PENDING;
    SDHCDReleaseHCLock(m_pHCContext);   // really needed?

cleanUp:

    return status;

}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::SDHCBusRequestHandlerImpl - bus request handler
//  Input:  pRequest - the request
//
//  Output:
//  Return: SD_API_STATUS
//  Notes:  The request passed in is marked as uncancelable, this function
//          has the option of making the outstanding request cancelable
//          returns status pending
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::SDHCBusRequestHandlerImpl(PSD_BUS_REQUEST pRequest)
{
    DEBUGCHK(pRequest);

    SD_API_STATUS   status = SD_API_STATUS_FAST_PATH_SUCCESS;
    DEBUGCHK(pRequest);

    if(pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)
    {
       m_fastPathReq = 1;
    }

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHCBusRequestHandler - CMD::[%d]  FastPath %d \n"), pRequest->CommandCode , m_fastPathReq));

    //m_fastPathSDMEM = 0;
    /* Choose fastpath or not based on registry settings for SDIO and SD memory cards */
    if((pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE) && ((!m_fastPathSDIO && m_dwSDIOCard) || (!m_fastPathSDMEM && !m_dwSDIOCard)))
    {
       pRequest->SystemFlags &= ~SD_FAST_PATH_AVAILABLE;
       m_fastPathReq = 0;
    }


    if(pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)
       status = SDHCBusRequestHandlerImpl_FastPath(pRequest);
    else
       status = SDHCBusRequestHandlerImpl_NormalPath(pRequest);

    return status;
}

//  CommandCompleteHandler_FastPath
//  Input:
//  Output: 
//  Notes:  
SD_API_STATUS CSDIOControllerBase::CommandCompleteHandler_FastPath(PSD_BUS_REQUEST pRequest)
{
    DWORD               dwCountStart;
    SD_API_STATUS       status = SD_API_STATUS_PENDING;
    DWORD MMC_STAT_OVERWRITE = 0;


    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("CommandCompleteHandler_FastPath() - got the command response CommandCompleteHandler_FastPath\n")));

    DWORD MMC_STAT = Read_MMC_STAT();
    DWORD MMC_PSTAT = INREG32(&m_pbRegisters->MMCHS_PSTATE);

    if( MMC_PSTAT & MMCHS_PSTAT_DATI )
    {
        if( pRequest->CommandResponse.ResponseType == ResponseR1b )
        {
            DEBUGMSG(SHC_BUSY_STATE_ZONE, (TEXT("Card in busy state after command!  Delaying...\n")));
            // get starting tick count for timeout
            dwCountStart = GetTickCount();

            do 
            {
                MMC_STAT = Read_MMC_STAT();
                MMC_PSTAT = INREG32(&m_pbRegisters->MMCHS_PSTATE);

                // check for card ejection
                if( !SDCardDetect() )
                { 
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Card removed!\r\n")));
                    status = SD_API_STATUS_DEVICE_REMOVED;
                    goto TRANSFER_DONE;
                }

                // check for timeout
                if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Card BUSY timeout!\r\n")));
                    status = SD_API_STATUS_RESPONSE_TIMEOUT;
                    goto TRANSFER_DONE;
                }
            } while( (MMC_PSTAT & MMCHS_PSTAT_DATI) && !( MMC_STAT & ( MMCHS_STAT_CCRC | MMCHS_STAT_CTO | MMCHS_STAT_DCRC | MMCHS_STAT_DTO )) );

            DEBUGMSG(SHC_BUSY_STATE_ZONE, (TEXT("Card exited busy state.\n")));
        }
    }

    if((MMC_STAT & MMCHS_STAT_CCRC ) && (MMC_STAT & MMCHS_STAT_CTO) )// command CRC &Command timeout
    {

        DWORD               dwCountStart;
        DEBUGCHK(sizeof(OMAP_MMCHS_REGS) % sizeof(DWORD) == 0);

        if(MMC_STAT & MMCHS_STAT_CCRC)
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got control command CRC and timeout!\r\n")));
            MMC_STAT_OVERWRITE |= MMCHS_STAT_CCRC;
        }
        else
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got No control command CRC and timeout!\r\n")));
        }

        status = SD_API_STATUS_CRC_ERROR;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_CTO;
        Write_MMC_STAT(MMC_STAT_OVERWRITE);

        // Reset the controller
        SETREG32(&m_pbRegisters->MMCHS_SYSCTL, MMCHS_SYSCTL_SRC);

        // get starting tick count for timeout
        dwCountStart = GetTickCount();

        // Verify that reset has completed.
        while ((INREG32(&m_pbRegisters->MMCHS_SYSCTL) & MMCHS_SYSCTL_SRC))
        {
            if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
            {
                DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SoftwareReset() - exit: TIMEOUT.\n")));
                break;
            }
            Sleep(0);
        }

        goto TRANSFER_DONE;
    }


    if( MMC_STAT & MMCHS_STAT_CCRC ) // command CRC error
    {
        status = SD_API_STATUS_CRC_ERROR;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_CCRC;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got command CRC error!\r\n")));
    }
    if( MMC_STAT & MMCHS_STAT_CTO ) // command response timeout
    {
        status = SD_API_STATUS_RESPONSE_TIMEOUT;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_CTO;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got command response timeout!\r\n")));

    }
    if( MMC_STAT & MMCHS_STAT_DTO ) // data timeout
    {
        status = SD_API_STATUS_RESPONSE_TIMEOUT;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_DTO;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got command response timeout!\r\n")));
    }
    if( MMC_STAT & MMCHS_STAT_DCRC ) // data CRC error
    {
        status = SD_API_STATUS_RESPONSE_TIMEOUT;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_DCRC;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got command response timeout!\r\n")));

    }
    if( MMC_STAT_OVERWRITE ) // clear the status error bits
    {

        Write_MMC_STAT(MMC_STAT_OVERWRITE);
        goto TRANSFER_DONE;
    }

    // get the response information
    if(pRequest->CommandResponse.ResponseType == NoResponse)
    {
        DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("GetCmdResponse returned no response (no response expected)\r\n")));
        status = SD_API_STATUS_SUCCESS;
        goto TRANSFER_DONE;
    }
    else
    {
        status =  GetCommandResponse(pRequest);
        if(!SD_API_SUCCESS(status))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - Error getting response for command:0x%02x\r\n"), pRequest->CommandCode));
            goto TRANSFER_DONE;     
        }
    }

    if (SD_COMMAND != pRequest->TransferClass) // data transfer
    {
        DWORD cbTransfer = TRANSFER_SIZE(pRequest);
        BOOL     fRet;
        BOOL     FastPathMode = FALSE;

        FastPathMode = (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE) ? TRUE : FALSE; 

        switch(pRequest->TransferClass)
        {
        case SD_READ:
                __try {
                    fRet = SDIReceive(pRequest->pBlockBuffer, cbTransfer, FastPathMode);
                }
                __except(SDProcessException(GetExceptionInformation())) {
                    fRet = FALSE;
                }
            if(!fRet)
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - SDIPollingReceive() failed\r\n")));
                status = SD_API_STATUS_DATA_ERROR;
                goto TRANSFER_DONE;
            }
            else
            {
#ifdef DEBUG
                DWORD dwTemp = 0;
                while( dwTemp < cbTransfer && (dwTemp < (HEXBUFSIZE / 2 - 1) ) )
                {
                    szHexBuf[dwTemp*2] = pRequest->pBlockBuffer[dwTemp] / 16;
                    szHexBuf[dwTemp*2+1] = pRequest->pBlockBuffer[dwTemp] % 16;

                    if( szHexBuf[dwTemp*2] < 10 )
                        szHexBuf[dwTemp*2] += '0';
                    else
                        szHexBuf[dwTemp*2] += 'a' - 10;

                    if( szHexBuf[dwTemp*2+1] < 10 )
                        szHexBuf[dwTemp*2+1] += '0';
                    else
                        szHexBuf[dwTemp*2+1] += 'a' - 10;

                    dwTemp++;
                }
                szHexBuf[dwTemp*2] = 0;
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("PollingReceive succesfully received %d bytes\r\n                                     {%S}\r\n"), 
                    cbTransfer, szHexBuf ));
#endif
            }
            break;

        case SD_WRITE:
            {
#ifdef DEBUG
                DWORD dwTemp = 0;
                while( dwTemp < cbTransfer && (dwTemp < (HEXBUFSIZE / 2 - 1) ) )
                {
                    szHexBuf[dwTemp*2] = pRequest->pBlockBuffer[dwTemp] / 16;
                    szHexBuf[dwTemp*2+1] = pRequest->pBlockBuffer[dwTemp] % 16;

                    if( szHexBuf[dwTemp*2] < 10 )
                        szHexBuf[dwTemp*2] += '0';
                    else
                        szHexBuf[dwTemp*2] += 'a' - 10;

                    if( szHexBuf[dwTemp*2+1] < 10 )
                        szHexBuf[dwTemp*2+1] += '0';
                    else
                        szHexBuf[dwTemp*2+1] += 'a' - 10;

                    dwTemp++;
                }
                szHexBuf[dwTemp*2] = 0;
#endif
            }

                __try {
                    fRet = SDITransmit(pRequest->pBlockBuffer, cbTransfer, FastPathMode);
                }
                __except(SDProcessException(GetExceptionInformation())) {
                    fRet = FALSE;
                }

            if( !fRet )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - SDIPollingTransmit() failed\r\n")));
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("PollingTransmit failed to send %d bytes\r\n                                     {%S}\r\n"), 
                    cbTransfer, szHexBuf ));

                status = SD_API_STATUS_DATA_ERROR;
                goto TRANSFER_DONE;
            }
            else
            {
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("PollingTransmit succesfully sent %d bytes\r\n                                     {%S}\r\n"), 
                    cbTransfer, szHexBuf ));
            }

            break;
        }
        if(!m_fCardPresent)
            status = SD_API_STATUS_DEVICE_REMOVED;
        else
            status = SD_API_STATUS_SUCCESS;
    }

TRANSFER_DONE:

    if( status == SD_API_STATUS_SUCCESS )
    {
        if( m_fAppCmdMode )
        {
            m_fAppCmdMode = FALSE;
            DEBUGMSG(SHC_SEND_ZONE, (TEXT("SDHCBusRequestHandler - Switched to Standard Command Mode\n")));
        }
        else if( pRequest && pRequest->CommandCode == 55 )
        {
            m_fAppCmdMode = TRUE;
            DEBUGMSG(SHC_SEND_ZONE, (TEXT("SDHCBusRequestHandler - Switched to Application Specific Command Mode\n")));
        }

        if( pRequest->CommandCode == SD_CMD_MMC_SEND_OPCOND )
        {
            DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (TEXT("SendCommand: Card is recognized as a MMC\r\n") ) );
            m_fMMCMode = TRUE;
        }
    }

    
    // Clear the MMC_STAT register
    MMC_STAT = Read_MMC_STAT();
    Write_MMC_STAT(MMC_STAT); 
    UpdateSystemClock(FALSE);

    if( pRequest != NULL )
    {
        if( MMC_STAT_OVERWRITE ) // clear the status error bits
        {
            if( !SDCardDetect() )
            {
                SetEvent( m_hCardDetectEvent );
            }
        }

        // Update status according to the request type
        if((status == SD_API_STATUS_SUCCESS) && (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)) 
            status = SD_API_STATUS_FAST_PATH_SUCCESS;
    }
    return status;
}


//  CommandCompleteHandler
//  Input:
//  Output:
//  Notes:
SD_API_STATUS CSDIOControllerBase::CommandCompleteHandler()
{
    DWORD               dwCountStart;
    PSD_BUS_REQUEST     pRequest = NULL;       // the request to complete
    SD_API_STATUS       status = SD_API_STATUS_PENDING;
    DWORD MMC_STAT_OVERWRITE = 0;


    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("DataTransferIstThread() - got the command response CommandCompleteHandler\n")));

    // get and lock the current bus request
    if((pRequest = SDHCDGetAndLockCurrentRequest(m_pHCContext, 0)) == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Unable to get/lock current request!\r\n")));
        status = SD_API_STATUS_INVALID_DEVICE_REQUEST;
        goto TRANSFER_DONE;
    }

    DWORD MMC_STAT = Read_MMC_STAT();
    DWORD MMC_PSTAT = INREG32(&m_pbRegisters->MMCHS_PSTATE);

    if( MMC_PSTAT & MMCHS_PSTAT_DATI )
    {
        if( pRequest->CommandResponse.ResponseType == ResponseR1b )
        {
            DEBUGMSG(SHC_BUSY_STATE_ZONE, (TEXT("Card in busy state after command!  Delaying...\n")));
            // get starting tick count for timeout
            dwCountStart = GetTickCount();

            MMC_STAT = Read_MMC_STAT();
            MMC_PSTAT = INREG32(&m_pbRegisters->MMCHS_PSTATE);
            InterruptDone( m_dwControllerSysIntr );
            while( (MMC_PSTAT & MMCHS_PSTAT_DATI) && !( MMC_STAT & ( MMCHS_STAT_CCRC | MMCHS_STAT_CTO | MMCHS_STAT_DCRC | MMCHS_STAT_DTO )) )
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject(m_hControllerISTEvent,2))
                {
                    InterruptDone( m_dwControllerSysIntr );
                }
                MMC_STAT = Read_MMC_STAT();
                MMC_PSTAT = INREG32(&m_pbRegisters->MMCHS_PSTATE);

                // check for card ejection
                if( !SDCardDetect() )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Card removed!\r\n")));
                    status = SD_API_STATUS_DEVICE_REMOVED;
                    goto TRANSFER_DONE;
                }

                // check for timeout
                if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Card BUSY timeout!\r\n")));
                    status = SD_API_STATUS_RESPONSE_TIMEOUT;
                    goto TRANSFER_DONE;
                }
            }

            DEBUGMSG(SHC_BUSY_STATE_ZONE, (TEXT("Card exited busy state.\n")));
        }
    }

    
    if((MMC_STAT & MMCHS_STAT_CCRC ) && (MMC_STAT & MMCHS_STAT_CTO) )// command CRC &Command timeout
    {

        DWORD               dwCountStart;
        DEBUGCHK(sizeof(OMAP_MMCHS_REGS) % sizeof(DWORD) == 0);

        if(MMC_STAT & MMCHS_STAT_CCRC)
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got control command CRC and timeout!\r\n")));
            MMC_STAT_OVERWRITE |= MMCHS_STAT_CCRC;
        }
        else
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got No control command CRC and timeout!\r\n")));
        }

        status = SD_API_STATUS_CRC_ERROR;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_CTO;
        Write_MMC_STAT(MMC_STAT_OVERWRITE);

        // Reset the controller
        SETREG32(&m_pbRegisters->MMCHS_SYSCTL, MMCHS_SYSCTL_SRC);

        // get starting tick count for timeout
        dwCountStart = GetTickCount();

        // Verify that reset has completed.
        while ((INREG32(&m_pbRegisters->MMCHS_SYSCTL) & MMCHS_SYSCTL_SRC))
        {
            if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
            {
                DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SoftwareReset() - exit: TIMEOUT.\n")));
                break;
            }
            Sleep(0);
        }

        goto TRANSFER_DONE;
    }

    if( MMC_STAT & MMCHS_STAT_CCRC ) // command CRC error
    {
        status = SD_API_STATUS_CRC_ERROR;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_CCRC;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got command CRC error!\r\n")));
    }

    if( MMC_STAT & MMCHS_STAT_CTO ) // command response timeout
    {
        status = SD_API_STATUS_RESPONSE_TIMEOUT;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_CTO;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got command response timeout!\r\n")));

    }

    if( MMC_STAT & MMCHS_STAT_DTO ) // data timeout
    {
        status = SD_API_STATUS_RESPONSE_TIMEOUT;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_DTO;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got command response timeout!\r\n")));

    }
    if( MMC_STAT & MMCHS_STAT_DCRC ) // data CRC error
    {
        status = SD_API_STATUS_RESPONSE_TIMEOUT;
        MMC_STAT_OVERWRITE |= MMCHS_STAT_DCRC;
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHControllerIstThread() - Got command response timeout!\r\n")));

    }

    if( MMC_STAT_OVERWRITE ) // clear the status error bits
    {

        Write_MMC_STAT(MMC_STAT_OVERWRITE);
        goto TRANSFER_DONE;
    }

    if(MMC_STAT & MMCHS_STAT_CC ) //command complete
    {
        // get the response information
        if(pRequest->CommandResponse.ResponseType == NoResponse)
        {
            DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("GetCmdResponse returned no response (no response expected)\r\n")));
            status = SD_API_STATUS_SUCCESS;
            goto TRANSFER_DONE;
        }
        else
        {
            status =  GetCommandResponse(pRequest);
            if(!SD_API_SUCCESS(status))
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - Error getting response for command:0x%02x\r\n"), pRequest->CommandCode));
                goto TRANSFER_DONE;     
            }
        }
    }

    if (SD_COMMAND != pRequest->TransferClass) // data transfer
    {
        DWORD cbTransfer = TRANSFER_SIZE(pRequest);
        BOOL     fRet;
        BOOL     FastPathMode = FALSE;

        FastPathMode = ((pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)) ? TRUE : FALSE; 

        switch(pRequest->TransferClass)
        {
        case SD_READ:
                __try {
                    if(FastPathMode)
                    {
                       fRet = SDIReceive(pRequest->pBlockBuffer, cbTransfer, FastPathMode);
                    }
                    else
                    {
                       fRet = SDIReceive(pRequest->pBlockBuffer, cbTransfer);
                    }
                }
                __except(SDProcessException(GetExceptionInformation())) {
                    fRet = FALSE;
                }
            if(!fRet)
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - SDIPollingReceive() failed\r\n")));
                status = SD_API_STATUS_DATA_ERROR;
                goto TRANSFER_DONE;
            }
            else
            {
#ifdef DEBUG
                DWORD dwTemp = 0;
                while( dwTemp < cbTransfer && (dwTemp < (HEXBUFSIZE / 2 - 1) ) )
                {
                    szHexBuf[dwTemp*2] = pRequest->pBlockBuffer[dwTemp] / 16;
                    szHexBuf[dwTemp*2+1] = pRequest->pBlockBuffer[dwTemp] % 16;

                    if( szHexBuf[dwTemp*2] < 10 )
                        szHexBuf[dwTemp*2] += '0';
                    else
                        szHexBuf[dwTemp*2] += 'a' - 10;

                    if( szHexBuf[dwTemp*2+1] < 10 )
                        szHexBuf[dwTemp*2+1] += '0';
                    else
                        szHexBuf[dwTemp*2+1] += 'a' - 10;

                    dwTemp++;
                }
                szHexBuf[dwTemp*2] = 0;
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("PollingReceive succesfully received %d bytes\r\n                                     {%S}\r\n"),
                    cbTransfer, szHexBuf ));

#endif
            }
            break;

        case SD_WRITE:
            {
#ifdef DEBUG
                DWORD dwTemp = 0;
                while( dwTemp < cbTransfer && (dwTemp < (HEXBUFSIZE / 2 - 1) ) )
                {
                    szHexBuf[dwTemp*2] = pRequest->pBlockBuffer[dwTemp] / 16;
                    szHexBuf[dwTemp*2+1] = pRequest->pBlockBuffer[dwTemp] % 16;

                    if( szHexBuf[dwTemp*2] < 10 )
                        szHexBuf[dwTemp*2] += '0';
                    else
                        szHexBuf[dwTemp*2] += 'a' - 10;

                    if( szHexBuf[dwTemp*2+1] < 10 )
                        szHexBuf[dwTemp*2+1] += '0';
                    else
                        szHexBuf[dwTemp*2+1] += 'a' - 10;

                    dwTemp++;
                }
                szHexBuf[dwTemp*2] = 0;
#endif
            }

                __try {
                    if(FastPathMode)
                    {
                       fRet = SDITransmit(pRequest->pBlockBuffer, cbTransfer, FastPathMode);
                    }
                    else
                    {
                       fRet = SDITransmit(pRequest->pBlockBuffer, cbTransfer);
                    }
                }
                __except(SDProcessException(GetExceptionInformation())) {
                    fRet = FALSE;
                }

            if( !fRet )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCBusRequestHandler() - SDIPollingTransmit() failed\r\n")));
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("PollingTransmit failed to send %d bytes\r\n                                     {%S}\r\n"),
                    cbTransfer, szHexBuf ));

                status = SD_API_STATUS_DATA_ERROR;
                goto TRANSFER_DONE;
            }
            else
            {
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("PollingTransmit succesfully sent %d bytes\r\n                                     {%S}\r\n"),
                    cbTransfer, szHexBuf ));

            }

            break;
        }

        if(!m_fCardPresent)
            status = SD_API_STATUS_DEVICE_REMOVED;
        else
            status = SD_API_STATUS_SUCCESS;    }

TRANSFER_DONE:

    if( status == SD_API_STATUS_SUCCESS )
    {
        if( m_fAppCmdMode )
        {
            m_fAppCmdMode = FALSE;
            DEBUGMSG(SHC_SEND_ZONE, (TEXT("SDHCBusRequestHandler - Switched to Standard Command Mode\n")));
        }
        else if( pRequest && pRequest->CommandCode == 55 )
        {
            m_fAppCmdMode = TRUE;
            DEBUGMSG(SHC_SEND_ZONE, (TEXT("SDHCBusRequestHandler - Switched to Application Specific Command Mode\n")));
        }

        if( pRequest->CommandCode == SD_CMD_MMC_SEND_OPCOND )
        {
            DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (TEXT("SendCommand: Card is recognized as a MMC\r\n") ) );
            m_fMMCMode = TRUE;
        }
    }

    if(m_ActualPowerState == D4)
    {

       if( pRequest != NULL )
          SDHCDIndicateBusRequestComplete(m_pHCContext, pRequest, status);
       return TRUE;
    }

    // Clear the MMC_STAT register
    MMC_STAT = Read_MMC_STAT();

    Write_MMC_STAT(MMC_STAT); 
    UpdateSystemClock(FALSE);

    if( pRequest != NULL )
    {
      if( MMC_STAT_OVERWRITE )
      {
        if( !SDCardDetect() )
        {
          SetEvent( m_hCardDetectEvent );
        }
      }

      // Update status according to the request type
      if((status == SD_API_STATUS_SUCCESS) && m_fastPathReq && (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)) 
      {
         status = SD_API_STATUS_FAST_PATH_SUCCESS;
      }

      SDHCDIndicateBusRequestComplete(m_pHCContext, pRequest, status);
    }

    //return status for consistency with fast path..Not used in normal case.
    return status;
}

VOID CSDIOControllerBase::SetSlotPowerState(CEDEVICE_POWER_STATE state)
{

    m_ExternPowerState = (state>D3) ? D4 : D3;
    if (state < D3)
    {
        SystemClockOn();
    }
    else
    {
        SystemClockOff();
    }
}

CEDEVICE_POWER_STATE CSDIOControllerBase::GetSlotPowerState()
{
    return m_ExternPowerState;
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::SDHCSlotOptionHandler - handler for slot option changes
//  Input:  SlotNumber   - the slot the change is being applied to
//          Option       - the option code
//          pData        - data associaSHC with the option
//  Output:
//  Return: SD_API_STATUS
//  Notes:
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::SDHCSlotOptionHandlerImpl(
    UCHAR                 SlotNumber,
    SD_SLOT_OPTION_CODE   Option,
    PVOID                 pData,
    ULONG                 OptionSize)
{
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // status

    SDHCDAcquireHCLock(m_pHCContext);

    switch (Option)
    {
      case SDHCDSetSlotPower:
        {
          //// this platform has 3.2V tied directly to the slot
          DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SHCSDSlotOptionHandler - called - SDHCDSetSlotPower \n")));
        }
        break;

      case SDHCDSetSlotInterface:
        {
            PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;

            DEBUGMSG(SDCARD_ZONE_INIT,
                (TEXT("SHCSDSlotOptionHandler - called - SetSlotInterface : Clock Setting: %d \n"),
                pInterface->ClockRate));
            
            SD_CARD_INTERFACE_EX sdCardInterfaceEx;
            memset(&sdCardInterfaceEx,0, sizeof(sdCardInterfaceEx));
            sdCardInterfaceEx.InterfaceModeEx.bit.sd4Bit = (pInterface->InterfaceMode == SD_INTERFACE_SD_4BIT? 1: 0);
            sdCardInterfaceEx.ClockRate = pInterface->ClockRate;
            sdCardInterfaceEx.InterfaceModeEx.bit.sdWriteProtected = (pInterface->WriteProtected?1:0);
            SetInterface(&sdCardInterfaceEx);

            // Update the argument back.
            pInterface->InterfaceMode = (sdCardInterfaceEx.InterfaceModeEx.bit.sd4Bit!=0?SD_INTERFACE_SD_4BIT:SD_INTERFACE_SD_MMC_1BIT);
            pInterface->ClockRate =  sdCardInterfaceEx.ClockRate;
            pInterface->WriteProtected = (sdCardInterfaceEx.InterfaceModeEx.bit.sdWriteProtected!=0?TRUE:FALSE);
        }
        break;

      case SDHCDSetSlotInterfaceEx: {
            DEBUGCHK(OptionSize == sizeof(SD_CARD_INTERFACE_EX));
            PSD_CARD_INTERFACE_EX pInterface = (PSD_CARD_INTERFACE_EX) pData;

            DEBUGMSG(SDCARD_ZONE_INFO,
                (TEXT("CSDHCSlotBase::SlotOptionHandler: Clock Setting: %d\n"),
                pInterface->ClockRate));
            SetInterface((PSD_CARD_INTERFACE_EX)pInterface);
        }
        break;
      case SDHCDEnableSDIOInterrupts:
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SHCSDSlotOptionHandler - called - EnableSDIOInterrupts : on slot %d  \n"),SlotNumber));
        EnableSDIOInterrupts();
        break;

      case SDHCDSetSlotPowerState:
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SHCSDSlotOptionHandler - called - SetSlotPowerState : on slot %d  \n"),SlotNumber));
        if( pData == NULL || OptionSize != sizeof(CEDEVICE_POWER_STATE) )
        {
          status = SD_API_STATUS_INVALID_PARAMETER;
        }
        else
        {
          PCEDEVICE_POWER_STATE pcps = (PCEDEVICE_POWER_STATE) pData;
          SetSlotPowerState( *pcps );
        }
        break;

      case SDHCDGetSlotPowerState:
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SHCSDSlotOptionHandler - called - GetSlotPowerState : on slot %d  \n"),SlotNumber));
        if( pData == NULL || OptionSize != sizeof(CEDEVICE_POWER_STATE) )
        {
          status = SD_API_STATUS_INVALID_PARAMETER;
        }
        else
        {
          PCEDEVICE_POWER_STATE pcps = (PCEDEVICE_POWER_STATE) pData;
          *pcps = GetSlotPowerState();
        }
        break;

      case SDHCDWakeOnSDIOInterrupts:
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SHCSDSlotOptionHandler - called - WakeOnSDIOInterrupts : on slot %d  \n"),SlotNumber));
        DEBUGCHK(OptionSize == sizeof(BOOL));
        {
          PBOOL pfWake = (PBOOL) pData;
          if ( m_dwWakeupSources & WAKEUP_SDIO )
          {
            DWORD dwCurrentWakeupSources = m_dwWakeupSources & (~WAKEUP_SDIO);
            if (*pfWake) {
              m_dwCurrentWakeupSources |= WAKEUP_SDIO;
            }

            if( m_dwCurrentWakeupSources != dwCurrentWakeupSources )
            {
                KernelIoControl( IOCTL_HAL_ENABLE_WAKE,
                    &m_dwControllerSysIntr,
                    sizeof( m_dwControllerSysIntr ),
                    NULL,
                    0,
                    NULL );
            }
            else
            {
                KernelIoControl( IOCTL_HAL_DISABLE_WAKE,
                    &m_dwControllerSysIntr,
                    sizeof( m_dwControllerSysIntr ),
                    NULL,
                    0,
                    NULL );
            }

            m_dwCurrentWakeupSources = dwCurrentWakeupSources;
          }
          else
          {
            status = SD_API_STATUS_UNSUCCESSFUL;
          }
        }
        break;

      case SDHCDAckSDIOInterrupt:
        AckSDIOInterrupt();
        break;

      case SDHCDDisableSDIOInterrupts:
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SHCSDSlotOptionHandler - called - DisableSDIOInterrupts : on slot %d  \n"),SlotNumber));
        break;

      case SDHCDGetWriteProtectStatus:
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SHCSDSlotOptionHandler - called - SDHCDGetWriteProtectStatus : on slot %d  \n"),SlotNumber));
        {
          PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;
          pInterface->WriteProtected = IsWriteProtected();
        }
        break;

      case SDHCDQueryBlockCapability:
        {
          PSD_HOST_BLOCK_CAPABILITY pBlockCaps =
            (PSD_HOST_BLOCK_CAPABILITY)pData;

          DEBUGMSG(1,
            (TEXT("SHCSDSlotOptionHandler: Read Block Length: %d , Read Blocks: %d\n"),
            pBlockCaps->ReadBlockSize,
            pBlockCaps->ReadBlocks));
          DEBUGMSG(1,
            (TEXT("SHCSDSlotOptionHandler: Write Block Length: %d , Write Blocks: %d\n"),
            pBlockCaps->WriteBlockSize,
            pBlockCaps->WriteBlocks));

          DEBUGMSG(SDCARD_ZONE_INFO,
            (TEXT("SHCSDSlotOptionHandler: Read Block Length: %d , Read Blocks: %d\n"),
            pBlockCaps->ReadBlockSize,
            pBlockCaps->ReadBlocks));
          DEBUGMSG(SDCARD_ZONE_INFO,
            (TEXT("SHCSDSlotOptionHandler: Write Block Length: %d , Write Blocks: %d\n"),
            pBlockCaps->WriteBlockSize,
            pBlockCaps->WriteBlocks));

          if (pBlockCaps->ReadBlockSize < STD_HC_MIN_BLOCK_LENGTH) {
            pBlockCaps->ReadBlockSize = STD_HC_MIN_BLOCK_LENGTH;
          }

          if (pBlockCaps->ReadBlockSize > m_usMaxBlockLen) {
            pBlockCaps->ReadBlockSize = m_usMaxBlockLen;
          }

          if (pBlockCaps->WriteBlockSize < STD_HC_MIN_BLOCK_LENGTH) {
            pBlockCaps->WriteBlockSize = STD_HC_MIN_BLOCK_LENGTH;
          }

          if (pBlockCaps->WriteBlockSize > m_usMaxBlockLen) {
            pBlockCaps->WriteBlockSize = m_usMaxBlockLen;
          }
        }
        break;

      case SDHCDGetSlotInfo:
        if( OptionSize != sizeof(SDCARD_HC_SLOT_INFO) || pData == NULL )
        {
          status = SD_API_STATUS_INVALID_PARAMETER;
        }
        else
        {
          PSDCARD_HC_SLOT_INFO pSlotInfo = (PSDCARD_HC_SLOT_INFO)pData;
          DWORD dwSlotCapabilities = SD_SLOT_SD_1BIT_CAPABLE | SD_SLOT_SDIO_CAPABLE;

          if (!m_Sdio4BitDisable)
              dwSlotCapabilities |= SD_SLOT_SD_4BIT_CAPABLE;

          if (!m_SdMem4BitDisable)
              dwSlotCapabilities |= SD_SLOT_SDMEM_4BIT_CAPABLE;

          SDHCDSetSlotCapabilities( pSlotInfo,dwSlotCapabilities);
          SDHCDSetVoltageWindowMask(pSlotInfo,(SD_VDD_WINDOW_3_1_TO_3_2 |
                                               SD_VDD_WINDOW_3_2_TO_3_3 |
                                               SD_VDD_WINDOW_3_3_TO_3_4 |
                                               SD_VDD_WINDOW_3_4_TO_3_5 |
                                               SD_VDD_WINDOW_3_5_TO_3_6));

          // Set optimal voltage
          SDHCDSetDesiredSlotVoltage(pSlotInfo, SD_VDD_WINDOW_2_9_TO_3_0);

          // Set maximum supported clock rate
          SDHCDSetMaxClockRate(pSlotInfo, m_dwMaxClockRate);

          // set power up delay
          SDHCDSetPowerUpDelay(pSlotInfo, 100);
        }
        break;

      default:
        status = SD_API_STATUS_INVALID_PARAMETER;
        break;
    }

    SDHCDReleaseHCLock(m_pHCContext);
    return status;
}

BOOLEAN CSDIOControllerBase::SDHCCancelIoHandler(
    PSDCARD_HC_CONTEXT pHCContext,
    DWORD Slot,
    PSD_BUS_REQUEST pRequest )
{
    // get our extension
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->SDHCCancelIoHandlerImpl((UCHAR)Slot, pRequest);
}

SD_API_STATUS CSDIOControllerBase::SDHCBusRequestHandler(
    PSDCARD_HC_CONTEXT pHCContext,
    DWORD Slot,
    PSD_BUS_REQUEST pRequest )
{
    // get our extension
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->SDHCBusRequestHandlerImpl(pRequest);
}

SD_API_STATUS CSDIOControllerBase::SDHCSlotOptionHandler(
    PSDCARD_HC_CONTEXT pHCContext,
    DWORD SlotNumber,
    SD_SLOT_OPTION_CODE Option,
    PVOID pData,
    ULONG OptionSize )
{
    // get our extension
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->SDHCSlotOptionHandlerImpl((UCHAR)SlotNumber,
        Option,
        pData,
        OptionSize );
}

SD_API_STATUS CSDIOControllerBase::SDHCDeinitialize(
    PSDCARD_HC_CONTEXT pHCContext )
{
    // get our extension
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->SDHCDeinitializeImpl();
}

SD_API_STATUS CSDIOControllerBase::SDHCInitialize(
    PSDCARD_HC_CONTEXT pHCContext )
{
    // get our extension
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->SDHCInitializeImpl();
}

BOOL CSDIOControllerBase::SetPower(CEDEVICE_POWER_STATE dx)
{
    BOOL rc = FALSE;

    EnterCriticalSection(&m_powerCS);
    if(m_ActualPowerState != dx) switch(dx)
    {
      case D0:
        // simulate a card ejection/insertion
        m_bReinsertTheCard = TRUE;

        // Notify the SD Bus driver of the PowerUp event
        SDHCDPowerUpDown(m_pHCContext, TRUE, FALSE, 0);
        if(!m_fCardPresent)
            SetEvent(m_hCardDetectEvent);

        m_ActualPowerState = dx;
        rc = TRUE;
        break;

      case D4:
        // Notify the SD Bus driver of the PowerDown event
        SDHCDPowerUpDown(m_pHCContext, FALSE, FALSE, 0);

        m_ActualPowerState = dx;
        rc = TRUE;
        break;
    }
    LeaveCriticalSection(&m_powerCS);
    return rc;
}

void CSDIOControllerBase::PowerDown()
{
    // Notify bus driver
    IndicateSlotStateChange(DeviceEjected);
    // Simulate Device removal for PPC Suspend/Resume
    m_fCardPresent = FALSE;
    // get the current request
    PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

    if (pRequest != NULL)
    {
        DEBUGMSG(SDCARD_ZONE_WARN,
            (TEXT("PowerDown - Canceling current request: 0x%08X, command: %d\n"),
             pRequest, pRequest->CommandCode));
        DumpRequest(pRequest);
        IndicateBusRequestComplete(pRequest, SD_API_STATUS_DEVICE_REMOVED);
    }

    SetPower(D4);
    // go to D4 right away
    ResetEvent(m_hTimerEvent);
    m_dwClockCnt = 0;
    m_bDisablePower = TRUE;
    m_ExternPowerState = m_InternPowerState = D4;
    UpdateDevicePowerState();
}

void CSDIOControllerBase::PowerUp()
{
}

//------------------------------------------------------------------------------
//
//  Function:  ContextRestore
//
//  This function restores the context.
//
BOOL 
CSDIOControllerBase::ContextRestore()
{
    SD_API_STATUS      status;              // SD status
    BOOL               fRegisteredWithBusDriver = FALSE;
    BOOL               fHardwareInitialized = FALSE;
    BOOL               bRet = FALSE;

    DEBUGMSG(SHC_RESPONSE_ZONE, (L"CSDIOControllerBase:+ContextRestore\r\n"));

    switch(m_dwMemBase)
    {
      case OMAP_MMCHS1_REGS_PA:
           m_dwSlot = MMCSLOT_1;
           break;
      case OMAP_MMCHS2_REGS_PA:
           m_dwSlot = MMCSLOT_2;
           break;
    }

    if( !InitializeHardware() )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating CD/RW GPIO registers\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto cleanUp;
    }

    fHardwareInitialized = TRUE;

    // Initialize the slot
    SoftwareReset(SOFT_RESET_ALL);
    Sleep(10); // Allow time for card to power down after a device reset

    fRegisteredWithBusDriver = TRUE;

    bRet = TRUE;

cleanUp:
    if ( (bRet == FALSE) && (m_pHCContext) )
    {
        FreeHostContext( fRegisteredWithBusDriver, fHardwareInitialized );
    }
    DEBUGMSG(SHC_RESPONSE_ZONE, (L"CSDIOControllerBase:-ContextRestore\r\n"));

    return bRet;
}

//Function:     GetCommandResponse()
//Description:  Retrieves the response info for the last SDI command
//              issues.
//Notes:
//Returns:      SD_API_STATUS status code.
SD_API_STATUS CSDIOControllerBase::GetCommandResponse(PSD_BUS_REQUEST pRequest)
{
    DWORD  dwRegVal;
    PUCHAR  respBuff;       // response buffer
    DWORD dwRSP;

    dwRegVal = Read_MMC_STAT();

    DEBUGMSG(SHC_RESPONSE_ZONE, (TEXT("GetCommandResponse() - MMC_STAT = 0x%08X.\n"), dwRegVal));


    if ( dwRegVal & (MMCHS_STAT_CC | MMCHS_STAT_CERR | MMCHS_STAT_CCRC))
    {
        respBuff = pRequest->CommandResponse.ResponseBuffer;

        switch(pRequest->CommandResponse.ResponseType)
        {
        case NoResponse:
            break;

        case ResponseR1:
        case ResponseR1b:
            //--- SHORT RESPONSE (48 bits total)---
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | COMMAND_INDEX(6) | CARD_STATUS(32) | CRC7(7) | END_BIT(1) }
            // NOTE: START_BIT and TRANSMISSION_BIT = 0, END_BIT = 1
            //
            // Dummy byte needed by calling function.
            *respBuff = (BYTE)(START_BIT | TRANSMISSION_BIT | pRequest->CommandCode);

            dwRSP = INREG32(&m_pbRegisters->MMCHS_RSP10);

            *(respBuff + 1) = (BYTE)(dwRSP & 0xFF);
            *(respBuff + 2) = (BYTE)(dwRSP >> 8);
            *(respBuff + 3) = (BYTE)(dwRSP >> 16);
            *(respBuff + 4) = (BYTE)(dwRSP >> 24);


            *(respBuff + 5) = (BYTE)(END_RESERVED | END_BIT);

            DEBUGMSG(SHC_RESPONSE_ZONE, (TEXT("GetCommandResponse() - R1 R1b : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n"), *(respBuff + 0),
                *(respBuff + 1), *(respBuff + 2), *(respBuff + 3), *(respBuff + 4), *(respBuff + 5)));
            DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("GetCmdResponse returned [%02x%02x%02x%02x%02x]\r\n"),
                respBuff[0], respBuff[1], respBuff[2], respBuff[3], respBuff[4], respBuff[5] ));

            break;

        case ResponseR3:
        case ResponseR4:
        case ResponseR7:
            DEBUGMSG(SHC_RESPONSE_ZONE, (TEXT("ResponseR3 ResponseR4\n")));
            //--- SHORT RESPONSE (48 bits total)---
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | RESERVED(6) | CARD_STATUS(32) | RESERVED(7) | END_BIT(1) }
            //
            *respBuff = (BYTE)(START_BIT | TRANSMISSION_BIT | START_RESERVED);

            dwRSP = INREG32(&m_pbRegisters->MMCHS_RSP10);

            *(respBuff + 1) = (BYTE)(dwRSP & 0xFF);
            *(respBuff + 2) = (BYTE)(dwRSP >> 8);
            *(respBuff + 3) = (BYTE)(dwRSP >> 16);
            *(respBuff + 4) = (BYTE)(dwRSP >> 24);

            *(respBuff + 5) = (BYTE)(END_RESERVED | END_BIT);

            DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("GetCmdResponse returned [%02x%02x%02x%02x%02x]\r\n"),
                respBuff[0], respBuff[1], respBuff[2], respBuff[3], respBuff[4], respBuff[5] ));

            break;

        case ResponseR5:
        case ResponseR6:
            DEBUGMSG(SHC_RESPONSE_ZONE, (TEXT("ResponseR5 ResponseR6\n")));
            //--- SHORT RESPONSE (48 bits total)---
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | COMMAND_INDEX(6) | RCA(16) | CARD_STATUS(16) | CRC7(7) | END_BIT(1) }
            //
            *respBuff = (BYTE)(START_BIT | TRANSMISSION_BIT | pRequest->CommandCode);

            dwRSP = INREG32(&m_pbRegisters->MMCHS_RSP10);

            *(respBuff + 1) = (BYTE)(dwRSP & 0xFF);
            *(respBuff + 2) = (BYTE)(dwRSP >> 8);
            *(respBuff + 3) = (BYTE)(dwRSP >> 16);
            *(respBuff + 4) = (BYTE)(dwRSP >> 24);

            *(respBuff + 5) = (BYTE)(END_BIT);

            DEBUGMSG(SHC_RESPONSE_ZONE, (TEXT("GetCommandResponse() - R5 R6 : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n"), *(respBuff + 0),
                *(respBuff + 1), *(respBuff + 2), *(respBuff + 3), *(respBuff + 4), *(respBuff + 5)));

            DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("GetCmdResponse returned [%02x%02x%02x%02x%02x]\r\n"),
                respBuff[0], respBuff[1], respBuff[2], respBuff[3], respBuff[4], respBuff[5] ));

            break;

        case ResponseR2:

            dwRSP = INREG32(&m_pbRegisters->MMCHS_RSP10);

            *(respBuff + 0) = (BYTE)(dwRSP & 0xFF);
            *(respBuff + 1) = (BYTE)(dwRSP >> 8);
            *(respBuff + 2) = (BYTE)(dwRSP >> 16);
            *(respBuff + 3) = (BYTE)(dwRSP >> 24);

            dwRSP = INREG32(&m_pbRegisters->MMCHS_RSP32);

            *(respBuff + 4) = (BYTE)(dwRSP & 0xFF);
            *(respBuff + 5) = (BYTE)(dwRSP >> 8);
            *(respBuff + 6) = (BYTE)(dwRSP >> 16);
            *(respBuff + 7) = (BYTE)(dwRSP >> 24);

            dwRSP = INREG32(&m_pbRegisters->MMCHS_RSP54);

            *(respBuff + 8) = (BYTE)(dwRSP & 0xFF);
            *(respBuff + 9) = (BYTE)(dwRSP >> 8);
            *(respBuff + 10) = (BYTE)(dwRSP >> 16);
            *(respBuff + 11) = (BYTE)(dwRSP >> 24);


            dwRSP = INREG32(&m_pbRegisters->MMCHS_RSP76);

            *(respBuff + 12) = (BYTE)(dwRSP & 0xFF);
            *(respBuff + 13) = (BYTE)(dwRSP >> 8);
            *(respBuff + 14) = (BYTE)(dwRSP >> 16);
            *(respBuff + 15) = (BYTE)(dwRSP >> 24);

            DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(TEXT("GetCmdResponse returned [%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x]\r\n"),
                respBuff[0], respBuff[1], respBuff[2], respBuff[3], respBuff[4], respBuff[5], respBuff[6], respBuff[7],
                respBuff[8], respBuff[9], respBuff[10], respBuff[11], respBuff[12], respBuff[13], respBuff[14], respBuff[15]));

            break;

        default:
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("GetCommandResponse() - Unrecognized response type!\r\n")));
            break;
        }
    }
    return SD_API_STATUS_SUCCESS;
}

BOOL CSDIOControllerBase::SDIReceive(PBYTE pBuff, DWORD dwLen)
{
#ifdef SDIO_DMA_ENABLED
    if( m_fDMATransfer )
    {
        return SDIDMAReceive( pBuff, dwLen);
    }
    else
#endif        
    {
        return SDIPollingReceive( pBuff, dwLen );
    }
}

BOOL CSDIOControllerBase::SDIReceive(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode)
{
#ifdef SDIO_DMA_ENABLED
    if( m_fDMATransfer )
    {
        return SDIDMAReceive( pBuff, dwLen, FastPathMode );
    }
    else
#endif        
    {
        return SDIPollingReceive( pBuff, dwLen );
    }
}

BOOL CSDIOControllerBase::SDITransmit(PBYTE pBuff, DWORD dwLen)
{
#ifdef SDIO_DMA_ENABLED
    if( m_fDMATransfer )
    {
        return SDIDMATransmit( pBuff, dwLen);
    }
    else
#endif      
    {
        return SDIPollingTransmit( pBuff, dwLen );
    }
}

BOOL CSDIOControllerBase::SDITransmit(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode)
{
#ifdef SDIO_DMA_ENABLED
    if( m_fDMATransfer )
    {
        return SDIDMATransmit( pBuff, dwLen, FastPathMode );
    }
    else
#endif      
    {
        return SDIPollingTransmit( pBuff, dwLen );
    }
}

#ifdef SDIO_DMA_ENABLED

BOOL CSDIOControllerBase::SDIDMAReceive(PBYTE pBuff, DWORD dwLen)
{
    DWORD dwCountStart;
    DWORD __unaligned *pbuf2 = (DWORD *) pBuff;

    DWORD MMC_STAT = Read_MMC_STAT(); 

    if( MMC_STAT & MMCHS_STAT_DTO )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: STAT register indicates MMC_STAT_DTO error.\n")));
        goto cleanUp;
    }
    if( MMC_STAT & MMCHS_STAT_DCRC ) // DATA CRC Error
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: STAT register indicates MMC_CRC_ERROR_RCVD error.\n")));
        goto cleanUp;
    }
 
    EnterCriticalSection( &m_critSec );
    SETREG32(&m_pbRegisters->MMCHS_IE, MMCHS_IE_TC);
    SETREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_ISE_TC);
    OUTREG32(&m_pbRegisters->MMCHS_STAT, 0xFFFFFFFF);
    LeaveCriticalSection( &m_critSec );
    // Clear interrupt status
    //Write_MMC_STAT(0xFFFFFFFF);

    InterruptDone( m_dwControllerSysIntr );

    // start the DMA
    SDIO_StartInputDMA();

    // get starting tick count for timeout
    dwCountStart = GetTickCount();

    // wait for the SDIO/controller interrupt
    while(TRUE)
    {
        // check for timeout
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: TIMEOUT.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }

        MMC_STAT = Read_MMC_STAT(); 
        if(MMC_STAT & MMCHS_STAT_TC)
        {
            break;
        }
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hControllerISTEvent, 10000)) 
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: Timeout.\n")));

        }

        if(!m_fCardPresent)
        {
           DEBUGMSG(ZONE_ENABLE_INFO,(L"Card has been Removed stopping Input DMA\r\n"));
           break;
        }

        MMC_STAT = Read_MMC_STAT(); 
        if(MMC_STAT & MMCHS_STAT_BRR)
        {
            break;
        }
        if(MMC_STAT & MMCHS_STAT_TC)
        {
            break;
        }
        if( MMC_STAT & MMCHS_STAT_DTO )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: STAT register indicates MMC_STAT_DTO error.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }
        if( MMC_STAT & MMCHS_STAT_DCRC ) // DATA CRC Error
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: STAT register indicates MMC_STAT_DCRC error.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }

        InterruptDone( m_dwControllerSysIntr );
    }

    MMC_STAT = Read_MMC_STAT(); 

    // stop DMA
    SDIO_StopInputDMA();

    Set_MMC_STAT(MMCHS_STAT_TC);

    // finally, copy the data from DMA buffer to the user buffer, which maybe SG buffer, 
    if( !SDPerformSafeCopy( pbuf2, m_pDmaBuffer, dwLen ) )
    {
        goto cleanUp;
    }

    return TRUE;  

cleanUp:

    return FALSE;
}



BOOL CSDIOControllerBase::SDIDMAReceive(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode)
{
    DWORD dwCountStart;
    DWORD __unaligned *pbuf2 = (DWORD *) pBuff;

    DWORD MMC_STAT = Read_MMC_STAT(); 

    if( MMC_STAT & MMCHS_STAT_DTO )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: STAT register indicates MMC_STAT_DTO error.\n")));
        goto cleanUp;
    }
    if( MMC_STAT & MMCHS_STAT_DCRC ) // DATA CRC Error
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: STAT register indicates MMC_CRC_ERROR_RCVD error.\n")));
        goto cleanUp;
    }

        //CLRREG32(&(m_pbRegisters->MMCHS_IE), MMCHS_IE_BRS);
        EnterCriticalSection( &m_critSec );
        SETREG32(&m_pbRegisters->MMCHS_IE, MMCHS_IE_TC);
        CLRREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_ISE_TC);
        OUTREG32(&m_pbRegisters->MMCHS_STAT, 0xFFFFFFFF);
        LeaveCriticalSection( &m_critSec );
        // Clear interrupt status
        //Write_MMC_STAT(0xFFFFFFFF);


    // start the DMA
    SDIO_StartInputDMA();

    // calculate timeout conditions
    dwCountStart = GetTickCount();

    // wait for the SDIO/controller interrupt
    while(1)
    {
        // check for timeout
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: TIMEOUT.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }

           DWORD retries = 0;
           // polling end-of-command
           while (!(Read_MMC_STAT() & MMCHS_STAT_TC)) {
               if (retries > SDIO_MAX_LOOP) {
                   DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("A.S. Timeout while polling DMA Receive complete.\n")));
                   break;      
               }
               retries++;
           }

        if(!m_fCardPresent)
        {

           DEBUGMSG(ZONE_ENABLE_INFO, (L"Card has been Removed stopping Input DMA\r\n"));
           break;
        }

        MMC_STAT = Read_MMC_STAT(); 
        if(MMC_STAT & MMCHS_STAT_BRR)
        {
            break;
        }
        if(MMC_STAT & MMCHS_STAT_TC)
        {
            break;
        }
        if( MMC_STAT & MMCHS_STAT_DTO )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: STAT register indicates MMC_STAT_DTO error.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }
        if( MMC_STAT & MMCHS_STAT_DCRC ) // DATA CRC Error
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMAReceive() - exit: STAT register indicates MMC_STAT_DCRC error.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }

    }

    MMC_STAT = Read_MMC_STAT(); 

    // stop DMA
    SDIO_StopInputDMA();

    Set_MMC_STAT(MMCHS_STAT_TC);

    // finally, copy the data from DMA buffer to the user buffer, which maybe SG buffer, 
    if( !SDPerformSafeCopy( pbuf2, m_pDmaBuffer, dwLen ) )
    {
        goto cleanUp;
    }


    return TRUE;  

cleanUp:

    DEBUGMSG(ZONE_ENABLE_INFO, (L"SDIDMAReceive(%08X, %08X, %08X)-\r\n", INREG32(&m_pbRegisters->MMCHS_STAT), INREG32(&m_pbRegisters->MMCHS_PSTATE), INREG32(&m_pbRegisters->MMCHS_DATA)));
    return FALSE;
}

BOOL CSDIOControllerBase::SDIDMATransmit(PBYTE pBuff, DWORD dwLen)
{
    DWORD dwCountStart;
    DWORD __unaligned *pbuf2 = (DWORD *) pBuff;

    // first copy the data to the DMA buffer, then to the user buffer, which maybe SG buffer,
    if( !SDPerformSafeCopy( m_pDmaBuffer, pbuf2, dwLen ) )
    {
        goto cleanUp;
    }

    DWORD MMC_STAT = Read_MMC_STAT(); 

    DEBUGMSG(ZONE_ENABLE_INFO, (L"SDIDMATransmit(dwLen 0x%x,%08x,%08x,%08x,%08x,%08x,%08x,%08x,%08x)+\r\n", 
      dwLen, pbuf2[0], pbuf2[1], pbuf2[2], pbuf2[3], pbuf2[4], pbuf2[5], pbuf2[6], pbuf2[7])); 

    if( MMC_STAT & MMCHS_STAT_DTO )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMATransmit() - exit: STAT register indicates MMC_STAT_DTO error.\n")));
        goto cleanUp;
    }
    if( MMC_STAT & MMCHS_STAT_DCRC ) // DATA CRC Error
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMATransmit() - exit: STAT register indicates MMC_STAT_DCRC error.\n")));
        goto cleanUp;
    }

    // start the DMA
    EnterCriticalSection( &m_critSec );
    SETREG32(&m_pbRegisters->MMCHS_IE, MMCHS_IE_TC);
    SETREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_ISE_TC);
    OUTREG32(&m_pbRegisters->MMCHS_STAT, 0xFFFFFFFF);
    LeaveCriticalSection( &m_critSec );
    //Write_MMC_STAT(0xFFFFFFFF);

    InterruptDone( m_dwControllerSysIntr );

    SDIO_StartOutputDMA();

    // get starting tick count for timeout
    dwCountStart = GetTickCount();

    // wait for the SDIO/controller interrupt
    while(TRUE)
    {
        // check for timeout
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIDMATransmit() - exit: TIMEOUT.\n")));
            goto cleanUp;
        }
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hControllerISTEvent, 10000)) 
        {
            DEBUGMSG(ZONE_ENABLE_INFO,(TEXT("Timeout...\n")));
        }

        if(!m_fCardPresent)
        {
           DEBUGMSG(ZONE_ENABLE_INFO, (L"Card has been Removed stopping Output DMA\r\n"));
           break;
        }

        MMC_STAT = Read_MMC_STAT(); 
        if(MMC_STAT & MMCHS_STAT_TC)
        {
            break;
        }
        if( MMC_STAT & MMCHS_STAT_DTO )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMATransmit() - exit: STAT register indicates MMC_STAT_DTO error.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }
        if( MMC_STAT & MMCHS_STAT_DCRC ) // DATA CRC Error
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMATransmit() - exit: STAT register indicates MMC_CRC_ERROR_RCVD error.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }
            InterruptDone( m_dwControllerSysIntr );
    }
    MMC_STAT = Read_MMC_STAT();

    // stop DMA
    SDIO_StopOutputDMA();
    Set_MMC_STAT(MMCHS_STAT_TC);
    return TRUE;

cleanUp:

    DEBUGMSG(ZONE_ENABLE_INFO, (L"CSDIOControllerBase::SDIDMATransmit(%08X, %08X)-\r\n", INREG32(&m_pbRegisters->MMCHS_STAT), INREG32(&m_pbRegisters->MMCHS_PSTATE)));
    return FALSE;

}

BOOL CSDIOControllerBase::SDIDMATransmit(PBYTE pBuff, DWORD dwLen, BOOL FastPathMode)
{
    DWORD dwCountStart;
    DWORD __unaligned *pbuf2 = (DWORD *) pBuff;

    // first copy the data to the DMA buffer, then to the user buffer, which maybe SG buffer,
    if( !SDPerformSafeCopy( m_pDmaBuffer, pbuf2, dwLen ) )
    {
        goto cleanUp;
    }

    DWORD MMC_STAT = Read_MMC_STAT(); 

    DEBUGMSG(ZONE_ENABLE_INFO, (L"SDIDMATransmit(dwLen 0x%x,%08x,%08x,%08x,%08x,%08x,%08x,%08x,%08x)+\r\n", 
      dwLen, pbuf2[0], pbuf2[1], pbuf2[2], pbuf2[3], pbuf2[4], pbuf2[5], pbuf2[6], pbuf2[7])); 

    if( MMC_STAT & MMCHS_STAT_DTO )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMATransmit() - exit: STAT register indicates MMC_STAT_DTO error.\n")));
        goto cleanUp;
    }
    if( MMC_STAT & MMCHS_STAT_DCRC ) // DATA CRC Error
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMATransmit() - exit: STAT register indicates MMC_STAT_DCRC error.\n")));
        goto cleanUp;
    }


 //       m_pbRegisters->MMC_IE &= ~MMC_IE_BRS;
        EnterCriticalSection( &m_critSec );
        SETREG32(&m_pbRegisters->MMCHS_IE, MMCHS_IE_TC);
        CLRREG32(&m_pbRegisters->MMCHS_ISE, MMCHS_ISE_TC);
        OUTREG32(&m_pbRegisters->MMCHS_STAT, 0xFFFFFFFF);
        LeaveCriticalSection( &m_critSec );
        //Write_MMC_STAT(0xFFFFFFFF);


    SDIO_StartOutputDMA();

    // get starting tick count for timeout
    dwCountStart = GetTickCount();

    // wait for the SDIO/controller interrupt
    while(1)
    {
        // check for timeout
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIDMATransmit() - exit: TIMEOUT.\n")));
            goto cleanUp;
        }

            DWORD retries = 0;
            // polling end-of-command
            while (!(Read_MMC_STAT() & MMCHS_STAT_TC)) {
                if (retries > SDIO_MAX_LOOP) {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("A.S. Timeout while polling DMA Transmit complete.\n")));
                    break;      
                }
                retries++;
            }


        if(!m_fCardPresent)
        {
           DEBUGMSG(ZONE_ENABLE_INFO,  (L"Card has been Removed stopping Output DMA\r\n"));
           break;
        }

        MMC_STAT = Read_MMC_STAT(); 
        if(MMC_STAT & MMCHS_STAT_TC)
        {
            break;
        }
        if( MMC_STAT & MMCHS_STAT_DTO )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMATransmit() - exit: STAT register indicates MMC_STAT_DTO error.\n")));

            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }
        if( MMC_STAT & MMCHS_STAT_DCRC ) // DATA CRC Error
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDIDMATransmit() - exit: STAT register indicates MMC_CRC_ERROR_RCVD error.\n")));
            // stop DMA
            SDIO_StopInputDMA();
            goto cleanUp;
        }
    }
    MMC_STAT = Read_MMC_STAT();

    // stop DMA
    SDIO_StopOutputDMA();
    Set_MMC_STAT(MMCHS_STAT_TC);

    return TRUE;

cleanUp:
    DEBUGMSG(ZONE_ENABLE_INFO,  (L"CSDIOControllerBase::SDIDMATransmit(%08X, %08X)-\r\n", INREG32(&m_pbRegisters->MMCHS_STAT), INREG32(&m_pbRegisters->MMCHS_PSTATE)));
    return FALSE;
}

#endif

//Function:     SDIPollingReceive()
//Description:
//Notes:        This routine assumes that the caller has already locked
//              the current request and checked for errors.
//Returns:      SD_API_STATUS status code.
BOOL CSDIOControllerBase::SDIPollingReceive(PBYTE pBuff, DWORD dwLen)
{
    DWORD fifoSizeW, blockLengthW; // Almost Full level and block length
    DWORD dwCount1, dwCount2;

    DWORD *pbuf = (DWORD *) pBuff;
    DWORD __unaligned *pbuf2 = (DWORD *) pBuff;
    DWORD cBytesTransferred=0;


    DWORD dwCountStart;

    DEBUGMSG(ZONE_ENABLE_INFO, (TEXT("SDIPollingReceive(0x%x)\n"), dwLen));
    //check the parameters

    DWORD MMC_STAT = Read_MMC_STAT();
    DWORD MMC_PSTAT = INREG32(&m_pbRegisters->MMCHS_PSTATE);

    // for transfers not multiples of block size or using SDIO card, data buffer could be unaligned.
    //  use unaligned buffer pointer pbuf2.
    if ((dwLen % (INREG32(&m_pbRegisters->MMCHS_BLK) & 0xFFFF)) || m_dwSDIOCard)
    {

        DEBUGMSG(ZONE_ENABLE_INFO, (TEXT("SDIPollingReceive Unaligned \n")));

        fifoSizeW = INREG32(&m_pbRegisters->MMCHS_BLK) & 0xFFFF;
        blockLengthW = dwLen / fifoSizeW;

        // get starting tick count for timeout
        dwCountStart = GetTickCount();

        for (dwCount1 = 0; dwCount1 < blockLengthW; dwCount1++)
        {
          // Wait for Block ready for read
          while ((Read_MMC_STAT() & MMCHS_STAT_BRR) != MMCHS_STAT_BRR)
          {
            // check for timeout
            if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
            {
              DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIPollingReceive() - exit: TIMEOUT1, MMCHS_STAT = 0x%x\n"), Read_MMC_STAT()));
              goto READ_ERROR;
            }
          }
          Set_MMC_STAT(MMCHS_STAT_BRR);

          // Get all data from DATA register and write in user buffer
          for (dwCount2 = 0; dwCount2 < (fifoSizeW/sizeof(DWORD)); dwCount2++)
          {
              *pbuf2 = INREG32(&m_pbRegisters->MMCHS_DATA);
              pbuf2++;
          }
          cBytesTransferred += dwCount2*sizeof(DWORD);
          // Copy the remaining bytes if any..
          DWORD dwResidue = fifoSizeW % sizeof(DWORD);
          DWORD dwTempData=0;
        
          switch(dwResidue)
          {
          case 1:
            dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
            pBuff[cBytesTransferred]= (BYTE)dwTempData;
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+1);
            break;

          case 2:
            dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
            pBuff[cBytesTransferred]= (BYTE)dwTempData;
            pBuff[cBytesTransferred+1]= (BYTE)(dwTempData>>8);
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+2);
            break;

          case 3:
            dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
            pBuff[cBytesTransferred]= (BYTE)dwTempData;
            pBuff[cBytesTransferred+1]= (BYTE)(dwTempData>>8);
            pBuff[cBytesTransferred+2]= (BYTE)(dwTempData>>16);
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+3);
            break;
          };

          cBytesTransferred += dwResidue;

        }

        if ( dwLen > cBytesTransferred)  //  check if any data left
        {
           DWORD bytesLeft = dwLen - cBytesTransferred;

           dwCountStart = GetTickCount();

           // Wait for Block ready for read
           while((Read_MMC_STAT() & MMCHS_STAT_BRR) != MMCHS_STAT_BRR)
           {
              // check for timeout
              if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
              {
                 DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIPollingReceive() - exit: TIMEOUT2, MMCHS_STAT = 0x%x\n"), Read_MMC_STAT()));
                 goto READ_ERROR;
              }
           }
           Set_MMC_STAT(MMCHS_STAT_BRR);
           fifoSizeW = bytesLeft / sizeof(DWORD);

           for (dwCount2 = 0; dwCount2 < fifoSizeW; dwCount2++)
           {
               *pbuf2 = INREG32(&m_pbRegisters->MMCHS_DATA) ;
               pbuf2++;
           }
           cBytesTransferred += dwCount2*sizeof(DWORD);
           
           // Copy the remaining bytes if any..
           DWORD dwTempData=0;
           DWORD dwResidue = bytesLeft % sizeof(DWORD);

           switch (dwResidue) 
           {
           case 1:
                dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
                pBuff[cBytesTransferred]= (BYTE)dwTempData;
                pbuf2 = (DWORD*) (pBuff+cBytesTransferred+1);
                break;

           case 2:
                dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
                pBuff[cBytesTransferred]= (BYTE)dwTempData;
                pBuff[cBytesTransferred+1]= (BYTE)(dwTempData>>8);
                pbuf2 = (DWORD*) (pBuff+cBytesTransferred+2);
                break;

           case 3:
                dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
                pBuff[cBytesTransferred]= (BYTE)dwTempData;
                pBuff[cBytesTransferred+1]= (BYTE)(dwTempData>>8);
                pBuff[cBytesTransferred+2]= (BYTE)(dwTempData>>16);
                pbuf2 = (DWORD*) (pBuff+cBytesTransferred+3);
                break;
           };

          cBytesTransferred += dwResidue;

        }
    } 
    else
    {
      fifoSizeW = INREG32(&m_pbRegisters->MMCHS_BLK) & 0xFFFF;
      blockLengthW = dwLen / fifoSizeW;

      DEBUGMSG(ZONE_ENABLE_INFO, (TEXT("SDIPollingReceive Aligned \n")));

      
      for (dwCount1 = 0; dwCount1 < blockLengthW; dwCount1++)
      {
        
        dwCountStart = GetTickCount();
        
        // Wait for Block ready for read
        while(((MMC_STAT = Read_MMC_STAT()) & MMCHS_STAT_BRR) != MMCHS_STAT_BRR)
        {
          DEBUGMSG(ZONE_ENABLE_INFO, (TEXT("SDIPollingReceive() - Retying for BRR = 0x%x\n"), MMC_STAT));

          // check for timeout
          if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
          {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIPollingReceive() - exit: TIMEOUT3, MMCHS_STAT = 0x%x\n"), Read_MMC_STAT()));
            goto READ_ERROR;
          }
        }
        Set_MMC_STAT(MMCHS_STAT_BRR);

        // Get all data from DATA register and write in user buffer
        for (dwCount2 = 0; dwCount2 < (fifoSizeW / sizeof(DWORD)); dwCount2++)
        {
            *pbuf = INREG32(&m_pbRegisters->MMCHS_DATA) ;
            pbuf++;
        }

        cBytesTransferred += dwCount2*sizeof(DWORD);

        // Copy the remaining bytes if any..
        DWORD dwResidue = fifoSizeW % sizeof(DWORD);
        DWORD dwTempData=0;
        
        switch(dwResidue)
        {
        case 1:
            dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
            pBuff[cBytesTransferred]= (BYTE)dwTempData;
            pbuf = (DWORD*) (pBuff+cBytesTransferred+1);
            break;
        case 2:
            dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
            pBuff[cBytesTransferred]= (BYTE)dwTempData;
            pBuff[cBytesTransferred+1]= (BYTE)(dwTempData>>8);
            pbuf = (DWORD*) (pBuff+cBytesTransferred+2);
            break;
        case 3:
            dwTempData = INREG32(&m_pbRegisters->MMCHS_DATA);
            pBuff[cBytesTransferred]= (BYTE)dwTempData;
            pBuff[cBytesTransferred+1]= (BYTE)(dwTempData>>8);
            pBuff[cBytesTransferred+2]= (BYTE)(dwTempData>>16);
            pbuf = (DWORD*) (pBuff+cBytesTransferred+3);
            break;
        };

        cBytesTransferred += dwResidue;
      }
    }

    // get starting tick count for timeout
    dwCountStart = GetTickCount();

    // poll for transfer or command complete
    while (!(Read_MMC_STAT() & MMCHS_STAT_TC))
    {
        // check for timeout
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIPollingReceive() - exit: TIMEOUT4, MMCHS_STAT = 0x%x\n"), Read_MMC_STAT()));
            goto READ_ERROR;
        }
    }

    Set_MMC_STAT(MMCHS_STAT_TC);

    MMC_STAT = Read_MMC_STAT();
    if ( (MMC_STAT & MMCHS_STAT_DCRC) 
        || (MMC_STAT & MMCHS_STAT_DEB)
        || (MMC_STAT & MMCHS_STAT_DTO)
       )
    {

        DEBUGMSG(ZONE_ENABLE_INFO, (TEXT("SDIPollingReceive() - STAT Error = 0x%x\n"), MMC_STAT));

        // Reset the controller
        SETREG32(&m_pbRegisters->MMCHS_SYSCTL, MMCHS_SYSCTL_SRD);

        // get starting tick count for timeout
        dwCountStart = GetTickCount();

        // Verify that reset has completed.
        while ((INREG32(&m_pbRegisters->MMCHS_SYSCTL) & MMCHS_SYSCTL_SRD))
        {
            if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
            {
                DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SoftwareReset() - exit: TIMEOUT.\n")));
                break;
            }
            Sleep(0);
        }

        MMC_STAT = Read_MMC_STAT();
        Write_MMC_STAT(MMC_STAT);
        return FALSE;
    }
    else
    {

        MMC_STAT = Read_MMC_STAT();
        Write_MMC_STAT(MMC_STAT);
        return TRUE;
    }
 
READ_ERROR:

    return FALSE;
}


BOOL CSDIOControllerBase::SDIPollingTransmit(PBYTE pBuff, DWORD dwLen)
{
    DWORD fifoSizeW, blockLengthW; // Almost Full level and block length
    DWORD dwCount1, dwCount2;
    DWORD *pbuf = (DWORD *) pBuff; // short* of buffer
    DWORD __unaligned *pbuf2 = (DWORD *) pBuff;
    DWORD cBytesTransferred=0;

    DWORD dwCountStart;

    // for transfers not multiples of block size or using SDIO card, data buffer could be unaligned.
    //  use unaligned buffer pointer pbuf2.
    if((dwLen % (INREG32(&m_pbRegisters->MMCHS_BLK) & 0xFFFF))
        || m_dwSDIOCard)
    {

        fifoSizeW = INREG32(&m_pbRegisters->MMCHS_BLK) & 0xFFFF;
        blockLengthW = dwLen / fifoSizeW;
        for (dwCount1 = 0; dwCount1 < blockLengthW; dwCount1++)
        {

          // get starting tick count for timeout
          dwCountStart = GetTickCount();

          // poll on write ready here
          while((Read_MMC_STAT() & MMCHS_STAT_BWR) != MMCHS_STAT_BWR)
          {
            if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
            {
              DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIPollingTransmit() - exit: TIMEOUT1, MMCHS_STAT = 0x%x\n"), Read_MMC_STAT()));
              goto WRITE_ERROR;
            }
          }
          Set_MMC_STAT(MMCHS_STAT_BWR);

          DWORD dwTempData=0;
          for (dwCount2 = 0; dwCount2 < (fifoSizeW /sizeof(DWORD)); dwCount2++) // write data to DATA buffer
          {
              OUTREG32(&m_pbRegisters->MMCHS_DATA, *pbuf2++);
          }
          cBytesTransferred += dwCount2*sizeof(DWORD);

          DWORD dwResidue = fifoSizeW % sizeof(DWORD);
        
          switch(dwResidue)
          {

          case 1:
            dwTempData = pBuff[cBytesTransferred];
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+1);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
          case 2:
            dwTempData = pBuff[cBytesTransferred]|pBuff[cBytesTransferred+1]<<8;
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+2);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
          case 3:
            dwTempData = pBuff[cBytesTransferred]|pBuff[cBytesTransferred+1]<<8|
            pBuff[cBytesTransferred+2]<<16;
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+3);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
          };
          cBytesTransferred += dwResidue;
        }

        if ( dwLen > cBytesTransferred)  //  check if any data left
        {
          DWORD bytesLeft = dwLen - cBytesTransferred;

          // get starting tick count for timeout
          dwCountStart = GetTickCount();

          while((Read_MMC_STAT() & MMCHS_STAT_BWR) != MMCHS_STAT_BWR)
          {
            if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
            {
              DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIPollingTransmit() - exit: TIMEOUT2, MMCHS_STAT = 0x%x\n"), Read_MMC_STAT()));
              goto WRITE_ERROR;
            }
          }
          Set_MMC_STAT(MMCHS_STAT_BWR);
          fifoSizeW = bytesLeft / sizeof(DWORD);


          for (dwCount1 = 0; dwCount1 < fifoSizeW; dwCount1++)
          {
              OUTREG32(&m_pbRegisters->MMCHS_DATA, *pbuf2++) ;
          }

          cBytesTransferred += dwCount1*sizeof(DWORD);
          DWORD dwTempData=0;
          DWORD dwResidue = bytesLeft % sizeof(DWORD);
        
          switch(dwResidue)
          {
          case 1:
            dwTempData = pBuff[cBytesTransferred];
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+1);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
          case 2:
            dwTempData = pBuff[cBytesTransferred]|pBuff[cBytesTransferred+1]<<8;
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+2);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
          case 3:
            dwTempData = pBuff[cBytesTransferred]|pBuff[cBytesTransferred+1]<<8|
            pBuff[cBytesTransferred+2]<<16;
            pbuf2 = (DWORD*) (pBuff+cBytesTransferred+3);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
          };
          cBytesTransferred += dwResidue;
        }
    } else
    {
      fifoSizeW = INREG32(&m_pbRegisters->MMCHS_BLK) & 0xFFFF;
      blockLengthW = dwLen / fifoSizeW;
      for (dwCount1 = 0; dwCount1 < blockLengthW; dwCount1++)
      {
        // get starting tick count for timeout
        dwCountStart = GetTickCount();

        // poll on write ready here
        while((Read_MMC_STAT() & MMCHS_STAT_BWR) != MMCHS_STAT_BWR)
        {
          if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
          {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIPollingTransmit() - exit: TIMEOUT3, MMCHS_STAT = 0x%x\n"), Read_MMC_STAT()));
            goto WRITE_ERROR;
          }
        }
        Set_MMC_STAT(MMCHS_STAT_BWR);
       
        for (dwCount2 = 0; dwCount2 < (fifoSizeW /sizeof(DWORD)); dwCount2++) // write data to DATA buffer
        {
            OUTREG32(&m_pbRegisters->MMCHS_DATA, *pbuf++);
        }

        cBytesTransferred += dwCount2*sizeof(DWORD);

        DWORD dwResidue = fifoSizeW % sizeof(DWORD);
        DWORD dwTempData=0;

        switch(dwResidue)
        {
        case 1:
            dwTempData = pBuff[cBytesTransferred];
            pbuf = (DWORD*) (pBuff+cBytesTransferred+1);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
        case 2:
            dwTempData = pBuff[cBytesTransferred]|pBuff[cBytesTransferred+1]<<8;
            pbuf = (DWORD*) (pBuff+cBytesTransferred+2);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
        case 3:
            dwTempData = pBuff[cBytesTransferred]|pBuff[cBytesTransferred+1]<<8|
            pBuff[cBytesTransferred+2]<<16;
            pbuf = (DWORD*) (pBuff+cBytesTransferred+3);
            OUTREG32(&m_pbRegisters->MMCHS_DATA, dwTempData);
            break;
        };

        cBytesTransferred += dwResidue;
      }
    }

    // get starting tick count for timeout
    dwCountStart = GetTickCount();

    // poll for transfer or command complete
    while (!(Read_MMC_STAT() & MMCHS_STAT_TC))
    {
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDIPollingTransmit() - exit: TIMEOUT4, MMCHS_STAT = 0x%x\n"), Read_MMC_STAT()));
            goto WRITE_ERROR;
        }
    }

    // Check if there is no CRC error
    if (!(Read_MMC_STAT() & MMCHS_STAT_DCRC))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

WRITE_ERROR:

    return FALSE;
}

void CSDIOControllerBase:: SDHC_DumpPRCM()
{
    PHYSICAL_ADDRESS pa;
    OMAP_PRCM_CORE_CM_REGS *pPRCMRegs = NULL;
    // Map Power, Reset, and Clock Manager (PRCM) Registers
     pa.QuadPart = OMAP_PRCM_CORE_CM_REGS_PA;
     pPRCMRegs   = (OMAP_PRCM_CORE_CM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CORE_CM_REGS), FALSE);

     RETAILMSG(1, (L"   SDHC Slot_%d  CM_AUTOIDLE1_CORE = 0x%08X\r\n", m_dwSlot, INREG32(&pPRCMRegs->CM_AUTOIDLE1_CORE)   ));
     RETAILMSG(1, (L"   SDHC Slot_%d  CM_FCLKEN1_CORE = 0x%08X\r\n", m_dwSlot, INREG32(&pPRCMRegs->CM_FCLKEN1_CORE)  ));
     RETAILMSG(1, (L"   SDHC Slot_%d  CM_ICLKEN1_CORE = 0x%08X\r\n", m_dwSlot, INREG32(&pPRCMRegs->CM_ICLKEN1_CORE)   ));
     RETAILMSG(1, (L"   SDHC Slot_%d  CM_IDLEST1_CORE = 0x%08X\r\n", m_dwSlot, INREG32(&pPRCMRegs->CM_IDLEST1_CORE)   ));
     RETAILMSG(1, (L"\r\n"));

}
