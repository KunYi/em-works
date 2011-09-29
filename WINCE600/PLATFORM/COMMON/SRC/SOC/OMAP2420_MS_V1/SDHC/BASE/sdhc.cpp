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
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

// SDHC controller driver implementation
#pragma optimize("",off)    // DEBUG
#include <SDCardDDK.h>
#include "SDHC.h"
#include <nkintr.h>
#include <pm.h>
#include <twl.h>

//------------------------------------------------------------------------------
//  Local Structures

//Debug the Power IOCTL's
//#define PM_TRACE 0
#define SDCARD_ZONE_POWER          DEBUGZONE(0)

#define DEFAULT_TIMEOUT_VALUE               60000
#define START_BIT                           0x00
#define TRANSMISSION_BIT                    0x00
#define START_RESERVED                      0x3F
#define END_RESERVED                        0xFE
#define END_BIT                             0x01

#define WAIT_FOR_INTERRUPT_TIMEOUT      500             // Time in milliseconds

#define IndicateSlotStateChange(event) \
    SDHCDIndicateSlotStateChange(m_pHCContext, \
    (UCHAR) 0, (event))

#define GetAndLockCurrentRequest() \
    SDHCDGetAndLockCurrentRequest(m_pHCContext, (UCHAR) 0)

#define TRANSFER_IS_WRITE(pRequest)        (SD_WRITE == (pRequest)->TransferClass)
#define TRANSFER_IS_READ(pRequest)         (SD_READ == (pRequest)->TransferClass)
#define TRANSFER_IS_COMMAND_ONLY(pRequest) (SD_COMMAND == (pRequest)->TransferClass)      

#define TRANSFER_SIZE(pRequest)            ((pRequest)->BlockSize * (pRequest)->NumBlocks)

static const GUID DEVICE_IFC_GPIO_GUID;
static const GUID DEVICE_IFC_TWL_GUID;

#ifdef DEBUG
// dump the current request info to the debugger
static 
VOID 
DumpRequest(
            PSD_BUS_REQUEST pRequest
            )
{   
    DEBUGCHK(pRequest);

    DEBUGMSG(SDCARD_ZONE_INIT, (L"DumpCurrentRequest: 0x%08X\r\n", pRequest)); 
    DEBUGMSG(SDCARD_ZONE_INIT, (L"\t Command: %d\r\n",  pRequest->CommandCode));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"\t ResponseType: %d\r\n",  pRequest->CommandResponse.ResponseType)); 
    DEBUGMSG(SDCARD_ZONE_INIT, (L"\t NumBlocks: %d\r\n",  pRequest->NumBlocks)); 
    DEBUGMSG(SDCARD_ZONE_INIT, (L"\t BlockSize: %d\r\n",  pRequest->BlockSize)); 
    DEBUGMSG(SDCARD_ZONE_INIT, (L"\t HCParam: %d\r\n",    pRequest->HCParam)); 

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
    WORD CMD_TYPE;
    WORD ACMD_TYPE;
    WORD MMC_CMD_TYPE;
};

// table of command codes...  at this time only SD/SDIO commands are implemented
const CMD gwaCMD[] =
{
    { 1, 0, 0, 0x0000, 0x2000, 0x0000 }, // CMD 00
    { 0, 0, 1, 0x1000, 0x2000, 0x1040 }, // CMD 01 (known MMC command)
    { 1, 0, 1, 0x1000, 0x2000, 0x1040 }, // CMD 02
    { 1, 0, 1, 0x1000, 0x2000, 0x1040 }, // CMD 03
    { 1, 0, 0, 0x0000, 0x2000, 0x0000 }, // CMD 04
    { 2, 0, 0, 0x1000, 0x2000, 0x1000 }, // CMD 05
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 06
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 07
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 08
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 09
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 10
    { 0, 0, 1, 0xf000, 0x2000, 0xf040 }, // CMD 11 (known MMC command)
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 12
    { 1, 1, 0, 0x2000, 0x3000, 0x2000 }, // CMD 13
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 14
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 15
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 16
    { 1, 1, 0, 0x3000, 0x2000, 0x3000 }, // CMD 17
    { 1, 1, 0, 0x3000, 0x2000, 0x3000 }, // CMD 18
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 19
    { 0, 1, 1, 0x7000, 0x2000, 0x7000 }, // CMD 20 (known MMC command)
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 21
    { 0, 1, 0, 0x2000, 0x3000, 0x2000 }, // CMD 22
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 23 (known MMC command)
    { 1, 1, 0, 0x3000, 0x2000, 0x3000 }, // CMD 24
    { 1, 1, 0, 0x3000, 0x2000, 0x3000 }, // CMD 25
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 26
    { 1, 0, 0, 0x3000, 0x2000, 0x3000 }, // CMD 27
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 28
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 29
    { 1, 0, 0, 0x3000, 0x2000, 0x3000 }, // CMD 30
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 31
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 32
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 33
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 34
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 35
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 36
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 37
    { 1, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 38
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 39 (known MMC command)
    { 1, 1, 1, 0x2000, 0x2000, 0x2040 }, // CMD 40
    { 0, 1, 0, 0x1000, 0x1000, 0x1000 }, // CMD 41 (known MMC command)
    { 1, 1, 0, 0x3000, 0x2000, 0x3000 }, // CMD 42
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 43
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 44
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 45
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 46
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 47
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 48
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 49
    { 0, 1, 0, 0x2000, 0x2000, 0x2000 }, // CMD 50
    { 0, 1, 0, 0xb000, 0x3000, 0xb000 }, // CMD 51 (known MMC command)
    { 2, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 52
    { 2, 0, 0, 0x3000, 0x2000, 0x3000 }, // CMD 53
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 54
    { 1, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 55
    { 1, 0, 0, 0x3000, 0x2000, 0x3000 }, // CMD 56
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 57
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 58
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 59
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 60
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 61
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 62
    { 0, 0, 0, 0x2000, 0x2000, 0x2000 }, // CMD 63
};

CSDIOControllerBase::CSDIOControllerBase()
{
    InitializeCriticalSection( &m_critSec );
    m_fSDIOInterruptInService = FALSE;
    m_fFirstTime = TRUE;
    m_hControllerISTEvent = NULL;
    m_htControllerIST = NULL;
    m_dwControllerSysIntr = SYSINTR_UNDEFINED;
    m_hCardDetectEvent = NULL;
    m_htCardDetectIST = NULL;
    m_dwCardDetectSysIntr = SYSINTR_UNDEFINED;
    m_dwCardDetectSysIRQSize = -1;
    m_fAppCmdMode = FALSE;

    m_vpSDIOReg = NULL;
    m_fCardPresent = FALSE;
    m_fSDIOInterruptsEnabled = FALSE;
    
    m_dwMaxTimeout = DEFAULT_TIMEOUT_VALUE;
    m_bReinsertTheCard = FALSE;
    m_dwWakeupSources = 0;
    m_dwCurrentWakeupSources = 0;
    m_fMMCMode = FALSE;

    m_PowerState = D0;
}

WORD CSDIOControllerBase::Read_MMC_SDIO()
{
    WORD wVal;
    EnterCriticalSection( &m_critSec );
    wVal = INREG16(&m_vpSDIOReg->MMC_SDIO);
    LeaveCriticalSection( &m_critSec );
    return wVal;
}

void CSDIOControllerBase::Write_MMC_SDIO( WORD wVal )
{
    EnterCriticalSection( &m_critSec );
    OUTREG16(&m_vpSDIOReg->MMC_SDIO, wVal);
    LeaveCriticalSection( &m_critSec );
}

WORD CSDIOControllerBase::Read_MMC_STAT()
{
    WORD wVal;
    EnterCriticalSection( &m_critSec );
    wVal = INREG16(&m_vpSDIOReg->MMC_STAT);
    LeaveCriticalSection( &m_critSec );
    return wVal;
}

void CSDIOControllerBase::Write_MMC_STAT( WORD wVal )
{
    EnterCriticalSection( &m_critSec );
    OUTREG16(&m_vpSDIOReg->MMC_STAT,wVal);
    LeaveCriticalSection( &m_critSec );
}

//  Reset the controller.
VOID CSDIOControllerBase::SoftwareReset( BYTE bResetBits )
{
    WORD                bValue;
    DWORD               dwCurrentTickCount;
    DWORD               dwTimeout;
    DWORD               dwCountStart;
    BOOL                fTimeoutOverflow = FALSE;

    DEBUGCHK(sizeof(OMAP2420_SDIO_REGS) % sizeof(WORD) == 0);

    // Reset the controller
    OUTREG16(&m_vpSDIOReg->MMC_SYSC,(0x0002 & bResetBits));

    // calculate timeout conditions
    dwCountStart = GetTickCount();
    dwTimeout = dwCountStart + m_dwMaxTimeout;
    if( dwTimeout < dwCountStart )
        fTimeoutOverflow = TRUE;

    // Verify that reset has completed.
    do {
        bValue = INREG16(&m_vpSDIOReg->MMC_SISS);

        // check for a timeout
        dwCurrentTickCount = GetTickCount();
        if( fTimeoutOverflow ? ( dwTimeout < dwCurrentTickCount && dwCurrentTickCount < dwCountStart )
            : ( dwTimeout < dwCurrentTickCount || dwCurrentTickCount < dwCountStart ) )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SoftwareReset: "
                L"Exit: TIMEOUT.\r\n"
            ));
            break;
        }
    } while (bValue != 0);
}


// Set up the controller according to the interface parameters.
VOID 
CSDIOControllerBase::SetInterface(
                                  PSD_CARD_INTERFACE pInterface
                                  )
{            
    DEBUGCHK(pInterface);

    WORD wRegValue;

    if (SD_INTERFACE_SD_MMC_1BIT == pInterface->InterfaceMode) 
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (L"CSDIOControllerBase::SetInterface: "
            L"Setting for 1 bit mode\r\n"
        ));
        wRegValue = INREG16(&m_vpSDIOReg->MMC_CON);
        OUTREG16(&m_vpSDIOReg->MMC_CON, (wRegValue & (~VALUE_MMC_CON_DW_4BIT)));

        DEBUGMSG(SDCARD_ZONE_INIT, (L"CSDIOControllerBase::SetInterface: "
            L"MMC1_CON value = %X\r\n", INREG16(&m_vpSDIOReg->MMC_CON)
        ));
    } 
    else if (SD_INTERFACE_SD_4BIT == pInterface->InterfaceMode) 
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (L"CSDIOControllerBase::SetInterface: "
            L"Setting for 4 bit mode \r\n"
        ));
        wRegValue = INREG16(&m_vpSDIOReg->MMC_CON);
        OUTREG16(&m_vpSDIOReg->MMC_CON, (wRegValue | (VALUE_MMC_CON_DW_4BIT)));

        DEBUGMSG(SDCARD_ZONE_INIT, (L"CSDIOControllerBase::SetInterface: "
            L"MMC1_CON value = %X\r\n", INREG16(&m_vpSDIOReg->MMC_CON)
        ));
    } 
    else 
    {
        DEBUGCHK(FALSE);
    }

    ClockOff();
    SetClockRate(&pInterface->ClockRate);
    ClockOn();
}


// Enable SDIO Interrupts.
VOID 
CSDIOControllerBase::EnableSDIOInterrupts()
{
    ASSERT( !m_fSDIOInterruptsEnabled );
    DEBUGMSG(SHC_INTERRUPT_ZONE, (L"+CSDHCSlot::EnableSDIOInterrupts\r\n"));

    WORD wRegValue = Read_MMC_SDIO();
    wRegValue |= MMC_IE_EOC;
    Write_MMC_SDIO(wRegValue);
    SETREG16(&m_vpSDIOReg->MMC_IE, MMC_IE_CIRQ);

    m_fSDIOInterruptsEnabled = TRUE;
}


// Acknowledge an SDIO Interrupt.
VOID CSDIOControllerBase::AckSDIOInterrupt(
    )
{   
    ASSERT( m_fSDIOInterruptsEnabled );

    DEBUGMSG(SHC_INTERRUPT_ZONE, (L"+CSDHCSlot::AckSDIOInterrupt\r\n"));

    WORD wRegValue = Read_MMC_STAT();
    Write_MMC_STAT(MMC_STAT_CIRQ);
    wRegValue = Read_MMC_STAT();
    if( wRegValue & MMC_STAT_CIRQ )
    {
        SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceInterrupting);
    }
    else
    {
        SETREG16(&m_vpSDIOReg->MMC_IE, MMC_IE_CIRQ);
        m_fSDIOInterruptInService = FALSE;
    }
}


// Disable SDIO Interrupts.
VOID 
CSDIOControllerBase::DisableSDIOInterrupts()
{            
    ASSERT( m_fSDIOInterruptsEnabled );
    DEBUGMSG(SHC_INTERRUPT_ZONE, (L"+CSDHCSlot::DisableSDIOInterrupts\r\n"));
    
    CLRREG16(&m_vpSDIOReg->MMC_IE, MMC_IE_CIRQ);
    WORD wRegValue = Read_MMC_SDIO();
    wRegValue &= (~MMC_IE_EOC);
    Write_MMC_SDIO(wRegValue);

    m_fSDIOInterruptsEnabled = FALSE;
}


//  Set clock rate based on HC capability
VOID 
CSDIOControllerBase::SetClockRate(PDWORD pdwRate)
{
    const DWORD dwClockRate = *pdwRate;

    // calculate the register value
    WORD wDiv = (WORD)((MMCSD_CLOCK_INPUT + dwClockRate - 1) / dwClockRate);
    DEBUGMSG(SHC_CLOCK_ZONE, (L"CSDIOControllerBase::SetClockRate: "
        L"Actual wDiv = 0x%x  requested:0x%x ", wDiv, *pdwRate
    ));
    // Only 8 bits available for the divider, so mmc base clock / 255 is minimum.
    if ( wDiv > 0x03FF )
        wDiv = 0x03FF;

    DEBUGMSG(SHC_CLOCK_ZONE, (L"wDiv = 0x%x 0x%x", wDiv, *pdwRate
    ));

    // Program the divisor, but leave the rest of the register alone.
    WORD wRegValue = INREG16(&m_vpSDIOReg->MMC_CON);

    OUTREG16(&m_vpSDIOReg->MMC_CON, ((wRegValue & ~0x03FF) | wDiv));

    
    *pdwRate = MMCSD_CLOCK_INPUT / wDiv;
    DEBUGMSG(SHC_CLOCK_ZONE,(L"CSDIOControllerBase::SetClockRate: "
        L"Actual clock rate = 0x%x\n", *pdwRate
    ));
}

//  Turn on the clock
VOID 
CSDIOControllerBase::ClockOn()
{
    // Enable MMC clock - clocks on by default
    DWORD cbRet;
    DWORD regVal = PRCM_FCLKEN1_CORE_EN_MMC;
    KernelIoControl(IOCTL_FCLK1_ENB, (VOID *)&regVal, sizeof(DWORD), (VOID *)&regVal, sizeof(DWORD), &cbRet);
    regVal = PRCM_ICLKEN1_CORE_EN_MMC;
    KernelIoControl(IOCTL_ICLK1_ENB, (VOID *)&regVal, sizeof(DWORD), (VOID *)&regVal, sizeof(DWORD), &cbRet);   
}

// Turn the clock off
VOID 
CSDIOControllerBase::ClockOff()
{
    // disable MMC clock
    DWORD cbRet;
    DWORD regVal = PRCM_FCLKEN1_CORE_EN_MMC;
    KernelIoControl(IOCTL_FCLK1_DIS, (VOID *)&regVal, sizeof(DWORD), (VOID *)&regVal, sizeof(DWORD), &cbRet);
    regVal = PRCM_ICLKEN1_CORE_EN_MMC;
    KernelIoControl(IOCTL_ICLK1_DIS, (VOID *)&regVal, sizeof(DWORD), (VOID *)&regVal, sizeof(DWORD), &cbRet);   
}



// Issues the specified SDI command
SD_API_STATUS
CSDIOControllerBase::SendCommand( PSD_BUS_REQUEST pRequest )
{
    WORD MMC_CMD;
    DWORD dwCurrentTickCount;
    DWORD dwTimeout;
    DWORD dwCountStart;
    BOOL  fTimeoutOverflow = FALSE;
    UINT16 Cmd = pRequest->CommandCode;
    UINT32 Arg = pRequest->CommandArgument;
    UINT16 respType = pRequest->CommandResponse.ResponseType;
    UINT16 TransferClass = pRequest->TransferClass;

    DEBUGMSG(SHC_SEND_ZONE, (L"CSDIOControllerBase::SendCommand: "
        L"Cmd = 0x%x Arg = 0x%x respType = 0x%x TransferClass = 0x%x\r\n", Cmd, Arg, respType, TransferClass
    ));

    if( TransferClass == SD_READ || TransferClass == SD_WRITE )
    {
        DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::SendCommand: "
            L"Cmd=0x%04x, Arg=0x%08x, RespType=0x%04x, Data=0x%x <%dx%d>) starts\r\n", 
            Cmd, Arg, respType, (TransferClass==SD_COMMAND)?FALSE:TRUE, pRequest->NumBlocks, pRequest->BlockSize
        ));
    }
    else
    {
        DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::SendCommand: "
            L"Cmd=0x%04x, Arg=0x%08x, RespType=0x%04x, Data=0x%x) starts\r\n", 
            Cmd, Arg, respType, (TransferClass==SD_COMMAND)?FALSE:TRUE
        ));
    }

    // turn the clock on
    ClockOn();

    WORD MMC_STAT = Read_MMC_STAT();

    if ( MMC_STAT & MMC_STAT_CB )
    {
        DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::SendCommand: "
            L"Card in busy state before command sent!\r\n"
        ));
    }

    if (m_fFirstTime) 
    {
        m_fFirstTime = FALSE;

        // Clear the MMC_STAT register
        Write_MMC_STAT( Read_MMC_STAT() & (~MMC_STAT_CIRQ) );

        // temporarily mask all interrupts
        OUTREG16(&m_vpSDIOReg->MMC_IE, 0x00);

        // send the initialization command
        OUTREG16(&m_vpSDIOReg->MMC_CMD, 0x80);

        // calculate timeout conditions
        dwCountStart = GetTickCount();
        dwTimeout = dwCountStart + m_dwMaxTimeout;
        if( dwTimeout < dwCountStart )
            fTimeoutOverflow = TRUE;

        // poll until command complete
        while( !( Read_MMC_STAT() & ( MMC_IE_EOC | MMC_IE_CTO | MMC_IE_CCRC | MMC_IE_CERR ) ) )
        {
            // check for a timeout
            dwCurrentTickCount = GetTickCount();
            if( fTimeoutOverflow ? ( dwTimeout < dwCurrentTickCount && dwCurrentTickCount < dwCountStart )
                : ( dwTimeout < dwCurrentTickCount || dwCurrentTickCount < dwCountStart ) )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SendCommand: "
                    L"Exit: TIMEOUT while sending INIB.\r\n"
                ));
                ClockOff();
                return SD_API_STATUS_UNSUCCESSFUL;
            }

            // check for card ejection
            if( !SDCardDetect() )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SendCommand: "
                    L"Exit: card ejected while sending INIB.\r\n"
                ));
                ClockOff();
                return SD_API_STATUS_DEVICE_REMOVED;
            }
        }

        // Clear the MMC_STAT register
        Write_MMC_STAT( Read_MMC_STAT() & (~MMC_STAT_CIRQ) );

        // unmask interrupts
        OUTREG16(&m_vpSDIOReg->MMC_IE, (MMC_IE_EOC | MMC_IE_CTO | MMC_IE_CCRC | MMC_IE_CERR | MMC_IE_BRS));           
    }

    // Clear the MMC_STAT register before issues command.
    Write_MMC_STAT( Read_MMC_STAT() & (~MMC_STAT_CIRQ) );

    MMC_CMD = Cmd;
    ASSERT( ( MMC_CMD & 0x003f ) == MMC_CMD );

    if( m_fAppCmdMode )
    {
        ASSERT( gwaCMD[Cmd].ACmd != 0 );
        MMC_CMD |= gwaCMD[Cmd].ACMD_TYPE;
    }
    else
    {
        if( m_fMMCMode )
        {
            ASSERT( gwaCMD[Cmd].Cmd != 0 || gwaCMD[Cmd].MMCCmd != 0 );
            if( gwaCMD[Cmd].MMCCmd )
                MMC_CMD |= gwaCMD[Cmd].MMC_CMD_TYPE;
            else
                MMC_CMD |= gwaCMD[Cmd].CMD_TYPE;
        }
        else
        {
            ASSERT( gwaCMD[Cmd].Cmd != 0 || gwaCMD[Cmd].MMCCmd != 0 );
            MMC_CMD |= gwaCMD[Cmd].CMD_TYPE;
        }
    }

    switch( respType )
    {
    case ResponseR1:                // Short response required
        MMC_CMD |= 0x100;
        break;
    case ResponseR1b:
        MMC_CMD |= 0x900;
        break;
    case ResponseR2:
        MMC_CMD |= 0x200;
        break;
    case ResponseR3:
        MMC_CMD |= 0x300;
        break;
    case ResponseR4:
        MMC_CMD |= 0x400;
        Write_MMC_SDIO( Read_MMC_SDIO() | MMC_IE_CTO ); // disable CRC check in R4 response
        break;
    case ResponseR5:                
        MMC_CMD |= 0x500;
        break;
    case ResponseR6:
        MMC_CMD |= 0x600;
        break;
    }

    if ((Cmd == SD_CMD_IO_RW_DIRECT) || (Cmd == SD_CMD_IO_RW_EXTENDED))
    {
        Write_MMC_SDIO( Read_MMC_SDIO() | ( BIT15 | BIT6 ) );
        MMC_CMD |= 0x0040;
    }

    CLRREG16(&m_vpSDIOReg->MMC_BUF, (MMC_BUF_RXDE|MMC_BUF_TXDE));

    if( TransferClass == SD_READ )
    {
        MMC_CMD |= 0x8000;
        OUTREG16(&m_vpSDIOReg->MMC_BLEN, ((WORD)(( pRequest->BlockSize - 1 ) & 0x7ff)));
        OUTREG16(&m_vpSDIOReg->MMC_NBLK, ((WORD)(( pRequest->NumBlocks - 1 ) & 0x7ff)));
    }
    else if( TransferClass == SD_WRITE )
    {
        MMC_CMD &= ~0x8000;
        OUTREG16(&m_vpSDIOReg->MMC_BLEN, ((WORD)(( pRequest->BlockSize - 1 ) & 0x7ff)));
        OUTREG16(&m_vpSDIOReg->MMC_NBLK, ((WORD)(( pRequest->NumBlocks - 1 ) & 0x7ff)));
    }

    // Program the argument into the argument registers
    OUTREG16(&m_vpSDIOReg->MMC_ARG1, Arg);
    OUTREG16(&m_vpSDIOReg->MMC_ARG2, ((WORD)(Arg >> 16)));

    DEBUGMSG(SHC_SEND_ZONE, (L"CSDIOControllerBase::SendCommand: "
        L"Registers:Command = 0x%x, MMC_ARG1 = 0x%x, MMC_ARG2 = 0x%x\r\n",
        MMC_CMD, m_vpSDIOReg->MMC_ARG1 , m_vpSDIOReg->MMC_ARG2
    ));

    MMC_STAT = Read_MMC_STAT();

    // Issue the command.
    OUTREG16(&m_vpSDIOReg->MMC_CMD, MMC_CMD);
    MMC_STAT = Read_MMC_STAT();

    return SD_API_STATUS_PENDING;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Remove the device instance in the slot
VOID 
CSDIOControllerBase::HandleRemoval(
                                   BOOL fCancelRequest
                                   )
{    
    m_fCardPresent = FALSE;
    m_fMMCMode = FALSE;

    IndicateSlotStateChange(DeviceEjected);

    // turn clock off and remove power from the slot
    ClockOff();
    TurnCardPowerOff();

    // turn off SDIO interrupts
    if( m_fSDIOInterruptsEnabled )
    {
        DisableSDIOInterrupts();
    }

    if (fCancelRequest) {
        // get the current request  
        PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

        if (pRequest != NULL) {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"CSDIOControllerBase::HandleRemoval: "
                L"Card Removal Detected - Canceling current request: 0x%08X, command: %d\r\n", 
                pRequest, pRequest->CommandCode
            ));
            DumpRequest(pRequest);
            IndicateBusRequestComplete(pRequest, SD_API_STATUS_DEVICE_REMOVED);
            
            ClockOff();
        }
    }
}


// Initialize the card
VOID 
CSDIOControllerBase::HandleInsertion()
{
    DWORD dwClockRate = SD_DEFAULT_CARD_ID_CLOCK_RATE;

    m_fCardPresent = TRUE;

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CSDIOControllerBase::HandleInsertion\r\n"));

    // turn power to the card on
    TurnCardPowerOn();

    SetClockRate(&dwClockRate);
    
    ClockOn();

    // indicate device arrival
    IndicateSlotStateChange(DeviceInserted);
}

// Static interrupt routine for the entire controller.
VOID CSDIOControllerBase::HandleCardDetectInterrupt() 
{
    DEBUGMSG(SHC_INTERRUPT_ZONE, (L"+CSDHCSlot::HandleCardDetectInterrupt\r\n"));

    SDHCDAcquireHCLock(m_pHCContext);

    if( SDCardDetect() )
    {
        DEBUGMSG(SHC_INTERRUPT_ZONE, (L"CSDHCSlot::HandleCardDetectInterrupt: "
            L"Card is Inserted!\r\n"
        ));
        
        if( m_fCardPresent == FALSE || m_bReinsertTheCard ) {
            m_bReinsertTheCard = FALSE;
            m_fFirstTime = TRUE;
            HandleInsertion();
        }
    }
    else if( m_fCardPresent ) 
    {
        DEBUGMSG(SHC_INTERRUPT_ZONE, (L"CSDHCSlot::HandleCardDetectInterrupt: "
            L"Card is Removed!\r\n"));
        HandleRemoval(TRUE);
    }

    SDHCDReleaseHCLock(m_pHCContext);
}


#ifdef ENABLE_DEBUG

// Reads from SD Standard Host registers and writes them to the debugger.
VOID CSDIOControllerBase::DumpRegisters()
{
    EnterCriticalSection( &m_critSec );
    DEBUGMSG(SDCARD_ZONE_INIT, (L"+DumpStdHCRegs-------------------------\r\n"));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_CMD 0x%04X \r\n", m_vpSDIOReg->MMC_CMD    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_ARG1 0x%04X \r\n", m_vpSDIOReg->MMC_ARG1  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_ARG2 0x%04X \r\n", m_vpSDIOReg->MMC_ARG2  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_CON  0x%04X \r\n", m_vpSDIOReg->MMC_CON   ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_STAT 0x%04X \r\n", m_vpSDIOReg->MMC_STAT  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_IE 0x%04X \r\n", m_vpSDIOReg->MMC_IE  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_CTO 0x%04X \r\n", m_vpSDIOReg->MMC_CTO    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_DTO 0x%04X \r\n", m_vpSDIOReg->MMC_DTO    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_DATA 0x---- \r\n"/*, m_vpSDIOReg->MMC_DATA*/  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_BLEN 0x%04X \r\n", m_vpSDIOReg->MMC_BLEN  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_NBLK 0x%04X \r\n", m_vpSDIOReg->MMC_NBLK  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_BUF 0x%04X \r\n", m_vpSDIOReg->MMC_BUF    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_SDIO 0x%04X \r\n", m_vpSDIOReg->MMC_SDIO  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_SYSTEST 0x%04X \r\n", m_vpSDIOReg->MMC_SYSTEST    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_REV 0x%04X \r\n", m_vpSDIOReg->MMC_REV    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP0 0x%04X \r\n", m_vpSDIOReg->MMC_RSP0  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP1 0x%04X \r\n", m_vpSDIOReg->MMC_RSP1  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP2 0x%04X \r\n", m_vpSDIOReg->MMC_RSP2  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP3 0x%04X \r\n", m_vpSDIOReg->MMC_RSP3  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP4 0x%04X \r\n", m_vpSDIOReg->MMC_RSP4  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP5 0x%04X \r\n", m_vpSDIOReg->MMC_RSP5  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP6 0x%04X \r\n", m_vpSDIOReg->MMC_RSP6  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP7 0x%04X \r\n", m_vpSDIOReg->MMC_RSP7  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_IOSR 0x%04X \r\n", m_vpSDIOReg->MMC_IOSR  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_SYSC 0x%04X \r\n", m_vpSDIOReg->MMC_SYSC  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_SISS 0x%04X \r\n", m_vpSDIOReg->MMC_SISS  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"-DumpStdHCRegs-------------------------\r\n"));
    LeaveCriticalSection( &m_critSec );
}

#endif

CSDIOControllerBase::~CSDIOControllerBase()
{
    DeleteCriticalSection( &m_critSec );
}

BOOL CSDIOControllerBase::Init( LPCTSTR pszActiveKey )
{
    PSDCARD_HC_CONTEXT pHostContext = NULL; // new HC context
    SD_API_STATUS      status;              // SD status
    HKEY               hKeyDevice = NULL;   // device key
    CReg               regDevice;           // encapsulated device key
    DWORD              dwRet = 0;           // return value    
    BOOL               fRegisteredWithBusDriver = FALSE;
    BOOL               fHardwareInitialized = FALSE;
    PHYSICAL_ADDRESS   pa;
    
    DEBUGMSG(SDCARD_ZONE_INIT, (L"+CSDIOControllerBase::Init: "
        L"Active RegPath: %s\r\n", pszActiveKey
    ));
   
    m_pCurrentRequest = NULL;
    m_pszActiveKey = pszActiveKey; //save the activity key for power mgt
    
    hKeyDevice = OpenDeviceKey(pszActiveKey);
    if ( (hKeyDevice == NULL) || !regDevice.Open(hKeyDevice, NULL) ) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::Init: "
            L"Failed to open device key\r\n"
        ));
        goto EXIT;
    }

    // allocate the context - support for 2 Slots - BUT 1 controller
    status = SDHCDAllocateContext(1, &m_pHCContext);
    if (!SD_API_SUCCESS(status)) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::Init: "
            L"Failed to allocate context : 0x%08X\r\n",
            status
        ));
        goto EXIT;
    }

    // Set our extension
    m_pHCContext->pHCSpecificContext = this;

    if( !GetRegistrySettings(&regDevice) )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::Init: "
            L"Error reading registry settings\r\n"
        ));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT;
    }

    // map hardware memory space
    pa.QuadPart = OMAP2420_MSDI1_REGS_PA;
    m_vpSDIOReg = (OMAP2420_SDIO_REGS*)MmMapIoSpace( pa, sizeof(OMAP2420_SDIO_REGS), FALSE );
    if ( !m_vpSDIOReg )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::Init: "
            L"Error allocating MMC controller register\r\n"
        ));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT;
    }

    pa.QuadPart = OMAP2420_PRCM_REGS_PA;
    m_vpPRCMReg = (OMAP2420_PRCM_REGS*)MmMapIoSpace( pa, sizeof(OMAP2420_PRCM_REGS), FALSE );
    if ( !m_vpPRCMReg )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::Init: "
            L"Error allocating Power Reset and Clock Managment Registers\r\n"
        ));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT;
    }

    
    if( !InitializeHardware() )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::Init: "
            L"Error allocating CD/RW GPIO registers\r\n"
        ));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT;
    }

    // turn the SDHC controller to fully on!
    ClockOn();
    this->m_PowerState = D0;

    fHardwareInitialized = TRUE;

    // Initialize the slot
    SoftwareReset(SOFT_RESET_ALL);
    Sleep(10); // Allow time for card to power down after a device reset
    DumpRegisters();

    // Read SD Host Controller Info from register.
    if (!InterpretCapabilities()) 
    {
        goto EXIT;
    }

    // now register the host controller 
    status = SDHCDRegisterHostController(m_pHCContext);

    if (!SD_API_SUCCESS(status)) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::Init: "
            L"Failed to register host controller: %0x08X\r\n", status
        ));
        goto EXIT;
    }

    fRegisteredWithBusDriver = TRUE;

    // return the controller context
    dwRet = (DWORD) this;

EXIT:
    if (hKeyDevice) RegCloseKey(hKeyDevice);

    if ( (dwRet == 0) && m_pHCContext ) {
        FreeHostContext( fRegisteredWithBusDriver, fHardwareInitialized );
    }

    DEBUGMSG(SDCARD_ZONE_INIT, (L"-CSDIOControllerBase::Init\r\n"));

    return dwRet;
}


// Free the host context and associated resources.
VOID CSDIOControllerBase::FreeHostContext( BOOL fRegisteredWithBusDriver, BOOL fHardwareInitialized )
{
    DEBUGCHK(m_pHCContext);
    ClockOff();

    if (fRegisteredWithBusDriver) {
        // deregister the host controller
        SDHCDDeregisterHostController(m_pHCContext);
    }

    // unmap hardware memory space

    DeinitializeHardware();
    if (m_vpSDIOReg) MmUnmapIoSpace((PVOID)m_vpSDIOReg, sizeof(OMAP2420_SDIO_REGS));
    if (m_vpPRCMReg) MmUnmapIoSpace((PVOID)m_vpPRCMReg, sizeof(OMAP2420_PRCM_REGS));

    m_PowerState = D4;
    
    // cleanup the host context
    SDHCDDeleteContext(m_pHCContext);
}    


BOOL CSDIOControllerBase::IOControl(
   DWORD dwCode, 
   BYTE *pInBuffer, 
   DWORD inSize, 
   BYTE *pOutBuffer, 
   DWORD outSize, 
   DWORD *pOutSize)
{
    BOOL bRetVal = FALSE;
    
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CSDIOControllerBase::IOControl(0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        dwCode, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
    ));

    switch (dwCode) {
    // Power management functions.
    // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
    {
        POWER_CAPABILITIES pc;

        // Check arguments.
        if ( pOutBuffer == NULL || outSize < sizeof(POWER_CAPABILITIES))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::IOControl: "
                L"IOCTL_POWER_CAPABILITIES Invalid parameter.\r\n"
            ));
            break;
        }

        // Clear capabilities structure.
        memset(&pc, 0, sizeof(POWER_CAPABILITIES));

        // Set power capabilities. Supports D0 and D4.
        pc.DeviceDx = DX_MASK(D0)|DX_MASK(D4);

        DEBUGMSG(SDCARD_ZONE_POWER, (L"CSDIOControllerBase::IOControl: "
            L"IOCTL_POWER_CAPABILITIES = 0x%x\r\n", pc.DeviceDx
        ));

        if (CeSafeCopyMemory(pOutBuffer, &pc, sizeof(pc)) == 0)
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::IOControl: "
                L"CeSafeCopyMemory Failed\r\n"
            ));
            break;
        }

        // Update returned data size.
        if (pOutSize)
        {
            *pOutSize = sizeof(pc);
        }
        bRetVal = TRUE;
        break;
    }

    // Indicate if the device is ready to enter a new device power state.
    case IOCTL_POWER_QUERY:
    {
        DEBUGMSG(SDCARD_ZONE_POWER, (L"CSDIOControllerBase::IOControl: "
            L"IOCTL_POWER_QUERY Deprecated Function Called\r\n"
        ));
        bRetVal = FALSE;
        break;
    }

    // Request a change from one device power state to another
    // This driver self-manages it's internal power state by controlling
    // functional and interface clocks as needed in the Read and Write
    // functions rather than waiting for PM to tell it to save power
    // So the set calls below just update the power state variable
    case IOCTL_POWER_SET:
    {
        CEDEVICE_POWER_STATE dxState;

        // Check arguments.
        if (pOutBuffer == NULL || outSize < sizeof(CEDEVICE_POWER_STATE))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::IOControl: "
                L"IOCTL_POWER_SET Invalid parameter.\r\n"
            ));
            break;
        }

        if (CeSafeCopyMemory(&dxState, pOutBuffer, sizeof(dxState)) == 0) break;

        DEBUGMSG(SDCARD_ZONE_POWER, (L"CSDIOControllerBase::IOControl: "
            L"IOCTL_POWER_SET = %d.\r\n", dxState
        ));

        // Check for any valid power state.
        if (VALID_DX(dxState))
        {
            // Power off
            if ( dxState == D4 )
            {
                DEBUGMSG(SDCARD_ZONE_POWER, (L"CSDIOControllerBase::IOControl: "
                    L"IOCTL_POWER_set PowerDown\r\n"
                ));
                
                //SDHCDPowerUpDown(m_pHCContext, FALSE, FALSE, 0);
                this->m_PowerState = dxState;
                this->FreeHostContext(TRUE, TRUE);        
            }
            // Power on.
            else
            {
                DEBUGMSG(SDCARD_ZONE_POWER, (L"CSDIOControllerBase::IOControl: "
                    L"IOCTL_POWER_set PowerUp\r\n"
                ));
                
                this->m_PowerState = dxState;
                this->Init(m_pszActiveKey);
                
                // Notify the SD Bus driver of the PowerUp event
                //SDHCDPowerUpDown(m_pHCContext, TRUE, FALSE, 0);
            }
            bRetVal = TRUE;
        }
        else
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::IOControl: "
                L"IOCTL_POWER_SET invalid power state.\r\n"
            ));
        }
        break;
    }

    // Return the current device power state.
    case IOCTL_POWER_GET:
    {
        // Check arguments.
        if (pOutBuffer == NULL || outSize < sizeof(CEDEVICE_POWER_STATE))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::IOControl: "
                L"IOCTL_POWER_GET Invalid parameter.\r\n"
            ));
            break;
        }

        //Copy current state
        if (CeSafeCopyMemory(pOutBuffer, &this->m_PowerState, sizeof(this->m_PowerState)) == 0)
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::IOControl: "
                L"CeSafeCopyMemory Failed\r\n"
            ));
            break;
        }

        // Update returned data size.
        if (pOutSize)
        {
            *pOutSize = sizeof(this->m_PowerState);
        }

        DEBUGMSG(SDCARD_ZONE_POWER, (L"CSDIOControllerBase::IOControl: "
            L"IOCTL_POWER_GET: %d\r\n", this->m_PowerState
        ));
        bRetVal = TRUE;
        break;
    }

    default:
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::IOControl: "
            L"Unknown IOCTL_xxx(0x%0.8X)\r\n", dwCode
        ));
        break;

    }

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CSDIOControllerBase::IOControl(rc = %d)\r\n", bRetVal));
    return bRetVal;    
}

// Read the registry settings
BOOL CSDIOControllerBase::GetRegistrySettings( CReg *pReg )
{
    BOOL fRet = TRUE;

    DEBUGCHK(pReg);

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
    SDHCDSetHCName(m_pHCContext, L"SDHC");

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

    if (!CeSetThreadPriority(GetCurrentThread(), m_dwSDIOPriority)) {
        DEBUGMSG(SDCARD_ZONE_WARN, (L"CSDIOControllerBase::SDHCControllerIstThreadImpl: "
            L"Warning, failed to set CEThreadPriority\r\n"
        ));
    }

    InterruptDone( m_dwControllerSysIntr );

    while (TRUE) {
        // wait for the SDIO/controller interrupt
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hControllerISTEvent, dwWaitTime)) 
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"CSDIOControllerBase::SDHCControllerIstThreadImpl: "
                L"Wait Failed!\r\n"
            ));
            break;
        }

        if (m_fDriverShutdown) {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"CSDIOControllerBase::SDHCControllerIstThreadImpl: "
                L"Thread exiting!\r\n"
            ));
            break;
        }

        WORD wStat;

        wStat = Read_MMC_STAT() & INREG16(&m_vpSDIOReg->MMC_IE) & 0x7fff;

        if( wStat & (MMC_IE_EOC|MMC_STAT_CERR|MMC_STAT_CCRC|MMC_STAT_CTO) )
        {
            CommandCompleteHandler();
        }

        if( wStat & MMC_STAT_CIRQ )
        {
            ASSERT( m_fSDIOInterruptsEnabled );
            // indicate that the card is interrupting
            DEBUGMSG(SHC_INTERRUPT_ZONE, (L"CSDIOControllerBase::SDHCControllerIstThreadImpl: "
                L"Receibed SDIO interrupt!\r\n"));

            // disable the SDIO interrupt
            CLRREG16(&m_vpSDIOReg->MMC_IE, MMC_IE_CIRQ);
            // notify the SDBusDriver of the SDIO interrupt
            m_fSDIOInterruptInService = TRUE;
            SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceInterrupting);
        }

        InterruptDone( m_dwControllerSysIntr );
    }

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CSDIOControllerBase::SDHCControllerIstThreadImpl\r\n"));
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

    if (m_fInitialized) {
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
            m_dwControllerSysIntr = SYSINTR_UNDEFINED;
        }

        if (m_fCardPresent) {
            // remove device
            HandleRemoval(FALSE);
        }

    }

    // clean up controller IST
    if (NULL != m_htControllerIST) {
        // wake up the IST
        SetEvent(m_hControllerISTEvent);
        // wait for the thread to exit
        WaitForSingleObject(m_htControllerIST, INFINITE); 
        CloseHandle(m_htControllerIST);
        m_htControllerIST = NULL;
    }

    // free controller interrupt event
    if (NULL != m_hControllerISTEvent) {
        CloseHandle(m_hControllerISTEvent);
        m_hControllerISTEvent = NULL;
    }

    // clean up card detect IST
    if (NULL != m_htCardDetectIST) {
        // wake up the IST
        SetEvent(m_hCardDetectEvent);
        // wait for the thread to exit
        WaitForSingleObject(m_htCardDetectIST, INFINITE); 
        CloseHandle(m_htCardDetectIST);
        m_htCardDetectIST = NULL;
    }

    if( m_hParentBus != NULL )
    {
        TWLIntrDisable(m_hParentBus, TWL_INTR_CD1);
        TWLIntrDisable(m_hParentBus, TWL_INTR_DL1);
        TWLIntrDisable(m_hParentBus, TWL_INTR_CD2);
        TWLIntrDisable(m_hParentBus, TWL_INTR_DL2);

        TWLClose(m_hParentBus);
        m_hParentBus = NULL;
    }

    // free card detect interrupt event
    if (NULL != m_hCardDetectEvent) {
        CloseHandle(m_hCardDetectEvent);
        m_hCardDetectEvent = NULL;
    }

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
    DWORD         threadID;
    unsigned long  ulLong;
    WORD wRegValue;
    DWORD *pdwSDIOIrq;
    DWORD dwSDIOIrqLen;

    m_fDriverShutdown = FALSE;
    SoftwareReset(SOFT_RESET_ALL);

    // activate the clock
    ClockOn();

    // --- Setup MMC config register ---
    // Data Bus width 1 bit - MMC/SD mode - Power Up Up/Normal - Little Endian - Clock Divider
    wRegValue = 0;
    wRegValue |= VALUE_MMC_CON_DW_1BIT | VALUE_MMC_CON_MODE_MMCSD | 
        VALUE_MMC_CON_POWERUP_UP | VALUE_MMC_CON_BE_LITTLEENDIAN;

    ulLong = (MMCSD_CLOCK_INPUT / MMCSD_CLOCK_INIT);
    if (MMCSD_CLOCK_INPUT % MMCSD_CLOCK_INIT) ulLong++;
    wRegValue |= (ulLong & MASK_MMC_CON_CLKD);

    OUTREG16(&m_vpSDIOReg->MMC_CON, wRegValue);
    DEBUGMSG(SDCARD_ZONE_INIT, (L"CSDIOControllerBase::SDHCInitializeImpl: "
        L"MMC1_CON set value = %X\r\n", wRegValue
    ));

    OUTREG16(&m_vpSDIOReg->MMC_IE, 0);

    wRegValue = (SDMMC_DEFAULT_ALMOST_EMPTY - 1) | ((SDMMC_DEFAULT_ALMOST_FULL - 1) << 8);
    OUTREG16(&m_vpSDIOReg->MMC_BUF, wRegValue);

    Write_MMC_SDIO(0);


    // --- Set the timeouts ----
    // Set command timeout of 64 clocks, the max according to OMAP1610 and SD specs
    OUTREG16(&m_vpSDIOReg->MMC_CTO, MMC_CTO_CONTROL_MAX);
    OUTREG16(&m_vpSDIOReg->MMC_DTO, MMC_DTO_CONTROL_MAX);

    // deactivate the clock
    ClockOff();

    // convert the SDIO hardware IRQ into a logical SYSINTR value
    DWORD rgdwSDIOIrq[] = { IRQ_MMC };
    pdwSDIOIrq = rgdwSDIOIrq;
    dwSDIOIrqLen = sizeof(rgdwSDIOIrq);

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, pdwSDIOIrq, dwSDIOIrqLen, &m_dwControllerSysIntr, sizeof(DWORD), NULL))
    {
        // invalid SDIO SYSINTR value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDHCInitializeImpl: "
            L"Error obtaining SDIO SYSINTR value!\r\n"
        ));
        m_dwControllerSysIntr = SYSINTR_UNDEFINED;
        status = SD_API_STATUS_UNSUCCESSFUL;
        goto exitInit;
    }

    // allocate the interrupt event for the SDIO/controller interrupt
    m_hControllerISTEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if (NULL == m_hControllerISTEvent) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    if ( !InterruptInitialize( m_dwControllerSysIntr, m_hControllerISTEvent, NULL, 0 ) ) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    // allocate the interrupt event for card detection
    m_hCardDetectEvent = CreateEvent(NULL, FALSE, FALSE,NULL);

    if (NULL == m_hCardDetectEvent) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    // Open parent bus - triton
    m_hParentBus = TWLOpen();
    if ( m_hParentBus == NULL ) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::Init: "
            L"Failed to obtain parent bus handle\r\n"
        ));
        goto exitInit;
    }
 
    // Associate event with TWL TWL_INTR_CD1 interrupt
    if (!TWLSetIntrEvent(m_hParentBus, TWL_INTR_CD1, m_hCardDetectEvent)) {
        goto exitInit;
    }

    // Associate event with TWL TWL_INTR_DL1 interrupt
    if (!TWLSetIntrEvent(m_hParentBus, TWL_INTR_DL1, m_hCardDetectEvent)) {
        goto exitInit;
    }

    // Associate event with TWL TWL_INTR_CD2 interrupt
    if (!TWLSetIntrEvent(m_hParentBus, TWL_INTR_CD2, m_hCardDetectEvent)) {
        goto exitInit;
    }

    // Associate event with TWL TWL_INTR_DL2 interrupt
    if (!TWLSetIntrEvent(m_hParentBus, TWL_INTR_DL2, m_hCardDetectEvent)) {
        goto exitInit;
    }

    // Enable TWL_INTR_CD1 event
    if (!TWLIntrEnable(m_hParentBus, TWL_INTR_CD1)) {
        goto exitInit;
    }

    // Enable TWL_INTR_DL1 event
    if (!TWLIntrEnable(m_hParentBus, TWL_INTR_DL1)) {
        goto exitInit;
    }

    // Enable TWL_INTR_CD2 event
    if (!TWLIntrEnable(m_hParentBus, TWL_INTR_CD2)) {
        goto exitInit;
    }

    // Enable TWL_INTR_DL2 event
    if (!TWLIntrEnable(m_hParentBus, TWL_INTR_DL2)) {
        goto exitInit;
    }

    // create the Controller IST thread
    m_htControllerIST = CreateThread(NULL,
        0,
        CSDIOControllerBase::SDHCControllerIstThread,
        this,
        0,
        &threadID);

    if (NULL == m_htControllerIST) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    // create the card detect IST thread
    m_htCardDetectIST = CreateThread(NULL,
        0,
        CSDIOControllerBase::SDHCCardDetectIstThread,
        this,
        0,
        &threadID);

    if (NULL == m_htCardDetectIST) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    m_fInitialized = TRUE;

    // on start we need the IST to check the slot for a card
    // FIXME Menelaus should detect card SetEvent(m_hCardDetectEvent);

    status = SD_API_STATUS_SUCCESS;

exitInit:

    if (!SD_API_SUCCESS(status)) {
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
BOOLEAN CSDIOControllerBase::SDHCCancelIoHandlerImpl( UCHAR Slot, PSD_BUS_REQUEST pRequest)
{
    // for now, we should never get here because all requests are non-cancelable
    // the hardware supports timeouts so it is impossible for the controller to get stuck
    DEBUGCHK(FALSE);

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::SDHCBusRequestHandlerImpl - bus request ha     ndler 
//  Input:  pRequest - the request
//          
//  Output: 
//  Return: SD_API_STATUS
//  Notes:  The request passed in is marked as uncancelable, this function
//          has the option of making the outstanding request cancelable    
//          returns status pending
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::SDHCBusRequestHandlerImpl( PSD_BUS_REQUEST pRequest ) 
{
    DEBUGCHK(pRequest);

    SD_API_STATUS   status;
    BOOL            fWriteTransferMode = FALSE;

    DEBUGMSG(SHC_SEND_ZONE, (L"CSDIOControllerBase::SDHCBusRequestHandlerImpl: "
        L"CMD: [%d]\r\n", pRequest->CommandCode
    ));

    // acquire the device lock to protect from device removal
    SDHCDAcquireHCLock(m_pHCContext);

    if ( m_pCurrentRequest) {
        IndicateBusRequestComplete(pRequest, SD_API_STATUS_CANCELED);
        m_pCurrentRequest = NULL;
    }

    m_fCurrentRequestFastPath = FALSE;
    m_pCurrentRequest = pRequest ;
    // if no data transfer involved, use FAST PATH
    if (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE && 
            !( SD_COMMAND != pRequest->TransferClass && 
                pRequest->NumBlocks * pRequest->BlockSize >=  NUM_BYTE_FOR_POLLING_MODE)){   // We do fast path here.
        m_fCurrentRequestFastPath = TRUE;
        InterruptMask(m_dwControllerSysIntr,TRUE);
        status = SendCommand(pRequest);
        if ( status == SD_API_STATUS_PENDING ) { // Polling for completion.
            while (m_pCurrentRequest) {
                if( Read_MMC_STAT() & INREG16(&m_vpSDIOReg->MMC_IE) & (MMC_IE_EOC|MMC_STAT_CERR|MMC_STAT_CCRC|MMC_STAT_CTO)) {
                    CommandCompleteHandler();
                }
            }               
            status = FastPathStatus;
        }

        InterruptMask(m_dwControllerSysIntr,FALSE);
        ASSERT(m_fCurrentRequestFastPath);
    }
    else 
    {
        pRequest->SystemFlags &= ~SD_FAST_PATH_AVAILABLE ;
        status = SendCommand(pRequest);
        if(!SD_API_SUCCESS(status))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDHCBusRequestHandlerImpl: "
                L"Error sending command:0x%02x\r\n", pRequest->CommandCode
            ));
            goto EXIT;      
        }
    }


EXIT:
    SDHCDReleaseHCLock(m_pHCContext);
    return status;
}

//  CommandCompleteHandler
//  Input:
//  Output: 
//  Notes:  
BOOL CSDIOControllerBase::CommandCompleteHandler()
{
    DWORD               dwCurrentTickCount;
    DWORD               dwTimeout;
    DWORD               dwCountStart;
    BOOL                fTimeoutOverflow = FALSE;
    PSD_BUS_REQUEST     pRequest = NULL;       // the request to complete
    SD_API_STATUS       status = SD_API_STATUS_PENDING;

    DEBUGMSG(SHC_INTERRUPT_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
        L"Got the command response\r\n"
    ));
    
#ifdef SDIO_INAB_INTERRUPT_WORKAROUND
    if( m_fINABworkaroundEnabled )
    {
        if( m_CurrentState == SendingINAB )
            goto PROCESS_INAB_STATE;
    }
#endif

    // get and lock the current bus request
    if((pRequest = SDHCDGetAndLockCurrentRequest(m_pHCContext, 0)) == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::CommandCompleteHandler: "
            L"Unable to get/lock current request!\r\n"
        ));
        status = SD_API_STATUS_INVALID_DEVICE_REQUEST;
        goto TRANSFER_DONE;
    } 

#ifdef DEBUG
    EnterCriticalSection( &m_critSec );
    DEBUGMSG(SDCARD_ZONE_INIT, (L"+DebugSDHCRegs-------------------------\r\n"));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_CMD 0x%04X \r\n", m_vpSDIOReg->MMC_CMD    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_ARG1 0x%04X \r\n", m_vpSDIOReg->MMC_ARG1  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_ARG2 0x%04X \r\n", m_vpSDIOReg->MMC_ARG2  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_CON  0x%04X \r\n", m_vpSDIOReg->MMC_CON   ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_STAT 0x%04X \r\n", m_vpSDIOReg->MMC_STAT  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_IE 0x%04X \r\n", m_vpSDIOReg->MMC_IE  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_CTO 0x%04X \r\n", m_vpSDIOReg->MMC_CTO    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_DTO 0x%04X \r\n", m_vpSDIOReg->MMC_DTO    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_BLEN 0x%04X \r\n", m_vpSDIOReg->MMC_BLEN  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_NBLK 0x%04X \r\n", m_vpSDIOReg->MMC_NBLK  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_BUF 0x%04X \r\n", m_vpSDIOReg->MMC_BUF    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_SDIO 0x%04X \r\n", m_vpSDIOReg->MMC_SDIO  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_REV 0x%04X \r\n", m_vpSDIOReg->MMC_REV    ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP0 0x%04X \r\n", m_vpSDIOReg->MMC_RSP0  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP1 0x%04X \r\n", m_vpSDIOReg->MMC_RSP1  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP2 0x%04X \r\n", m_vpSDIOReg->MMC_RSP2  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP3 0x%04X \r\n", m_vpSDIOReg->MMC_RSP3  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP4 0x%04X \r\n", m_vpSDIOReg->MMC_RSP4  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP5 0x%04X \r\n", m_vpSDIOReg->MMC_RSP5  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP6 0x%04X \r\n", m_vpSDIOReg->MMC_RSP6  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_RSP7 0x%04X \r\n", m_vpSDIOReg->MMC_RSP7  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_IOSR 0x%04X \r\n", m_vpSDIOReg->MMC_IOSR  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_SYSC 0x%04X \r\n", m_vpSDIOReg->MMC_SYSC  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"MMC_SISS 0x%04X \r\n", m_vpSDIOReg->MMC_SISS  ));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"PRCM_PM_WKEN1 0x%04X \r\n", m_vpPRCMReg->ulPM_WKEN1_CORE));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"PRCM_PM_WKST1 0x%04X \r\n", m_vpPRCMReg->ulPM_WKST1_CORE));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"PRCM_PM_AUTOIDEL1 0x%04X \r\n", m_vpPRCMReg->ulCM_AUTOIDLE1_CORE));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"PRCM_PM_FCLKEN1 0x%04X \r\n", m_vpPRCMReg->ulCM_FCLKEN1_CORE));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"PRCM_PM_ICLKEN1 0x%04X \r\n", m_vpPRCMReg->ulCM_ICLKEN1_CORE));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"-DebugSDHCRegs-------------------------\r\n"));
    LeaveCriticalSection( &m_critSec );
#endif
    
    WORD MMC_STAT = Read_MMC_STAT();
    if( MMC_STAT & MMC_STAT_CB )
    {
        ASSERT( pRequest->CommandResponse.ResponseType == ResponseR1b );

        if( pRequest->CommandResponse.ResponseType == ResponseR1b )
        {
            DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
                L"Card in busy state after command!  Delaying...\r\n"));

            // calculate timeout conditions
            dwCountStart = GetTickCount();
            dwTimeout = dwCountStart + m_dwMaxTimeout;
            if( dwTimeout < dwCountStart )
                fTimeoutOverflow = TRUE;

            do {
                MMC_STAT = Read_MMC_STAT();

                // check for card ejection
                if( !SDCardDetect() )
                { 
                    DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::CommandCompleteHandler: "
                        L"Card removed!\r\n"));
                    status = SD_API_STATUS_DEVICE_REMOVED;
                    goto TRANSFER_DONE;
                }

                // check for a timeout
                dwCurrentTickCount = GetTickCount();
                if( fTimeoutOverflow ? ( dwTimeout < dwCurrentTickCount && dwCurrentTickCount < dwCountStart )
                    : ( dwTimeout < dwCurrentTickCount || dwCurrentTickCount < dwCountStart ) )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::CommandCompleteHandler: "
                        L"Card BUSY timeout!\r\n"));
                    status = SD_API_STATUS_RESPONSE_TIMEOUT;
                    goto TRANSFER_DONE;
                }
            } while( !( MMC_STAT & ( MMC_STAT_EOFB | MMC_STAT_CCRC | MMC_STAT_CTO | MMC_STAT_DCRC | MMC_STAT_DTO ) ) );

            DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
                L"Card exited busy state.\r\n"));
            ASSERT( MMC_STAT & MMC_STAT_EOFB );
            Write_MMC_STAT( MMC_STAT_CB | MMC_STAT_EOFB );
        }
    }
    //    }

    WORD MMC_STAT_OVERWRITE = 0;
    if( MMC_STAT & MMC_STAT_CCRC ) // command CRC error
    {
        status = SD_API_STATUS_CRC_ERROR;
        MMC_STAT_OVERWRITE |= MMC_STAT_CCRC;
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::CommandCompleteHandler: "
            L"Got command CRC error!\r\n"));
    }
    if( MMC_STAT & MMC_STAT_CTO ) // command response timeout
    {
        status = SD_API_STATUS_RESPONSE_TIMEOUT;
        MMC_STAT_OVERWRITE |= MMC_STAT_CTO;
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::CommandCompleteHandler: "
            L"Got command response timeout!\r\n"));
    }

    if( MMC_STAT_OVERWRITE ) // clear the status error bits
    {
        Write_MMC_STAT(MMC_STAT_OVERWRITE);
        goto TRANSFER_DONE;
    }

    // get the response information
    if(pRequest->CommandResponse.ResponseType == NoResponse)
    {
        DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(L"CSDIOControllerBase::CommandCompleteHandler: "
            L"GetCmdResponse returned no response (no response expected)\r\n"));
        goto TRANSFER_DONE;
    }
    else{

        status =  GetCommandResponse(pRequest);

        if(!SD_API_SUCCESS(status))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::CommandCompleteHandler: "
                L"Error getting response for command:0x%02x\r\n", pRequest->CommandCode
            ));
            goto TRANSFER_DONE;     
        }
    }

    if (SD_COMMAND != pRequest->TransferClass) // data transfer
    {
        DWORD cbTransfer = TRANSFER_SIZE(pRequest);
        BOOL     fRet;

        switch(pRequest->TransferClass)
        {
        case SD_READ:
            __try {
                fRet = SDIReceive(pRequest->pBlockBuffer, cbTransfer);
            }
            __except(SDProcessException(GetExceptionInformation())) {
                fRet = FALSE;
            }

            if(!fRet)
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::CommandCompleteHandler: "
                    L"SDIPollingReceive() failed\r\n"
                ));
                goto TRANSFER_DONE;
            }
            else
            {
#ifdef ENABLE_DEBUG
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
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
                    L"PollingReceive succesfully received %d bytes {%S}\r\n", cbTransfer, szHexBuf
                ));
#endif
            }
            break;

        case SD_WRITE:
            {
#ifdef ENABLE_DEBUG
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
                fRet = SDITransmit(pRequest->pBlockBuffer, cbTransfer);
            }
            __except(SDProcessException(GetExceptionInformation())) {
                fRet = FALSE;
            }

            if( !fRet )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::CommandCompleteHandler: "
                    L"SDIPollingTransmit() failed\r\n"));
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
                    L"PollingTransmit failed to send %d bytes {%S}\r\n", cbTransfer, szHexBuf
                ));
                goto TRANSFER_DONE;
            }
            else
            {
                DEBUGMSG (SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
                    L"PollingTransmit succesfully sent %d bytes {%S}\r\n", cbTransfer, szHexBuf
                ));
            }

            break;
        }
        status = SD_API_STATUS_SUCCESS;
    }

TRANSFER_DONE:
    if( pRequest != NULL )
    {
        if ( ( !( m_fAppCmdMode ) ) && 
            ( (pRequest->CommandCode == SD_CMD_IO_RW_DIRECT) || 
            (pRequest->CommandCode == SD_CMD_IO_RW_EXTENDED) ) )
        {
            Write_MMC_SDIO( Read_MMC_SDIO() & ~( BIT15 | BIT6 ) );
        }

        if( pRequest->CommandResponse.ResponseType == ResponseR4 )
        {
            Write_MMC_SDIO( Read_MMC_SDIO() & ~( BIT7 ) );
        }
    }

    if( status == SD_API_STATUS_SUCCESS )
    {
        if( m_fAppCmdMode )
        {
            m_fAppCmdMode = FALSE;
            DEBUGMSG(SHC_SEND_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
                L"Switched to Standard Command Mode\r\n"));
        }
        else if( pRequest && pRequest->CommandCode == 55 )
        {
            m_fAppCmdMode = TRUE;
            DEBUGMSG(SHC_SEND_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
                L"Switched to Application Specific Command Mode\r\n"));
        }

        if( pRequest && pRequest->CommandCode == SD_CMD_MMC_SEND_OPCOND )
        {
            DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::CommandCompleteHandler: "
                L"Card is recognized as a MMC\r\n"
            ));
            m_fMMCMode = TRUE;
        }
    }

    // Clear the MMC_STAT register
    Write_MMC_STAT( Read_MMC_STAT() & (~MMC_STAT_CIRQ) );

    ClockOff();
    if( pRequest != NULL )
    {
        IndicateBusRequestComplete(pRequest, status);
    }
    return TRUE;
}

// give derived class an opportunity to intercept call
VOID CSDIOControllerBase::IndicateBusRequestComplete( 
    PSD_BUS_REQUEST     pRequest,
    SD_API_STATUS       Status)
{
    if ( m_pCurrentRequest == pRequest ) {
        DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,
            (L"CSDIOControllerBase::IndicateBusRequestComplete: "
            L"pRequest = %x, Status = %d\r\n", pRequest,Status)); 
        m_pCurrentRequest = NULL;
        if (m_fCurrentRequestFastPath ) {
            if (Status == SD_API_STATUS_SUCCESS) {
                Status = SD_API_STATUS_FAST_PATH_SUCCESS;
            }
            FastPathStatus = Status ;
        }
    }

    if ( Status != SD_API_STATUS_FAST_PATH_SUCCESS ) {
        SDHCDIndicateBusRequestComplete(m_pHCContext, pRequest, Status);
    }
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

    switch (Option) {

case SDHCDSetSlotPower:
    {
        DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
            L"Called - SDHCDSetSlotPower\r\n"));
        // Nothing to do because this system only operates at the reported 3.2V
    }
    break;

case SDHCDSetSlotInterface:
    {
        PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;

        DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
            L"Called - SetSlotInterface : Clock Setting: %d\r\n", pInterface->ClockRate
        ));

        SetInterface(pInterface);
    }
    break;

case SDHCDEnableSDIOInterrupts:
    DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
        L"Called - EnableSDIOInterrupts : on slot %d\r\n", SlotNumber
    ));
    EnableSDIOInterrupts();
    break;

case SDHCDSetSlotPowerState:
    DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
        L"Called - SetSlotPowerState : on slot %d\r\n", SlotNumber
    ));
    if( pData == NULL || OptionSize != sizeof(CEDEVICE_POWER_STATE) )
    {
        status = SD_API_STATUS_INVALID_PARAMETER;
    }
    else
    {
        PCEDEVICE_POWER_STATE pcps = (PCEDEVICE_POWER_STATE) pData;
        this->m_PowerState = *pcps;
    }
    break;

case SDHCDGetSlotPowerState:
    DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
        L"Called - GetSlotPowerState : on slot %d\r\n", SlotNumber
    ));
    if( pData == NULL || OptionSize != sizeof(CEDEVICE_POWER_STATE) )
    {
        status = SD_API_STATUS_INVALID_PARAMETER;
    }
    else
    {
        PCEDEVICE_POWER_STATE pcps = (PCEDEVICE_POWER_STATE) pData;
        *pcps = this->m_PowerState;
    }
    break;

case SDHCDAckSDIOInterrupt:
    AckSDIOInterrupt();
    break;

case SDHCDDisableSDIOInterrupts:
    DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
        L"Called - DisableSDIOInterrupts : on slot %d\r\n", SlotNumber
    ));
    break;

case SDHCDGetWriteProtectStatus:
    DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
        L"Called - SDHCDGetWriteProtectStatus : on slot %d\r\n", SlotNumber
    )); 
    {
        PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;
        pInterface->WriteProtected = IsWriteProtected();
    }

    break;

case SDHCDQueryBlockCapability:
    {
        PSD_HOST_BLOCK_CAPABILITY pBlockCaps = 
            (PSD_HOST_BLOCK_CAPABILITY)pData;

        DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
            L"Read Block Length: %d , Read Blocks: %d\r\n", pBlockCaps->ReadBlockSize, pBlockCaps->ReadBlocks
        ));
        DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOControllerBase::SDHCSlotOptionHandlerImpl: "
            L"Write Block Length: %d , Write Blocks: %d\r\n", pBlockCaps->WriteBlockSize, pBlockCaps->WriteBlocks
        ));

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

        // set the slot capabilities
        SDHCDSetSlotCapabilities( pSlotInfo, 
                SD_SLOT_SD_1BIT_CAPABLE |
                SD_SLOT_SD_4BIT_CAPABLE |
                SD_SLOT_SDIO_CAPABLE );

        SDHCDSetVoltageWindowMask(pSlotInfo, (SD_VDD_WINDOW_3_0_TO_3_1 | SD_VDD_WINDOW_3_1_TO_3_2)); 


        // Set optimal voltage
        SDHCDSetDesiredSlotVoltage(pSlotInfo, SD_VDD_WINDOW_3_2_TO_3_3);     

        // Set maximum supported clock rate
        SDHCDSetMaxClockRate(pSlotInfo, m_dwMaxClockRate);

        // set power up delay
        SDHCDSetPowerUpDelay(pSlotInfo, 100); 
    }
    break;

default:
    status = SD_API_STATUS_INVALID_PARAMETER;

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

//Function:     GetCommandResponse()
//Description:  Retrieves the response info for the last SDI command
//              issues.
//Notes:        
//Returns:      SD_API_STATUS status code.
SD_API_STATUS 
CSDIOControllerBase::GetCommandResponse(PSD_BUS_REQUEST pRequest)
{
    UINT16  dtmp;
    PUCHAR              respBuff;       // response buffer
    WORD wRSP;

    dtmp = Read_MMC_STAT();

    DEBUGMSG(SHC_RESPONSE_ZONE, (L"CSDIOControllerBase::GetCommandResponse: "
        L"MMC_STAT = 0x%04X.\r\n", dtmp
    ));


    if ( dtmp & MMC_STAT_EOC | dtmp & MMC_STAT_CERR | dtmp & MMC_STAT_CCRC) 
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

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP6);
            *(respBuff + 1) = (BYTE)wRSP;
            *(respBuff + 2) = (BYTE)(wRSP >> 8);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP7);
            *(respBuff + 3) = (BYTE)wRSP;
            *(respBuff + 4) = (BYTE)(wRSP >> 8);


            *(respBuff + 5) = (BYTE)(END_RESERVED | END_BIT);

            DEBUGMSG(SHC_RESPONSE_ZONE, (L"CSDIOControllerBase::GetCommandResponse: "
                L"R1 R1b : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\r\n", *(respBuff + 0), 
                *(respBuff + 1), *(respBuff + 2), *(respBuff + 3), *(respBuff + 4), *(respBuff + 5)
            ));
            DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(L"CSDIOControllerBase::GetCommandResponse: "
                L"Returned [%02x%02x%02x%02x%02x]\r\n",
                respBuff[0], respBuff[1], respBuff[2], respBuff[3], respBuff[4], respBuff[5]
            ));
            break;

        case ResponseR3:
        case ResponseR4:
            DEBUGMSG(SHC_RESPONSE_ZONE, (L"CSDIOControllerBase::GetCommandResponse: "
                L"ResponseR3 ResponseR4\r\n"
            ));
            //--- SHORT RESPONSE (48 bits total)--- 
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | RESERVED(6) | CARD_STATUS(32) | RESERVED(7) | END_BIT(1) }
            //
            *respBuff = (BYTE)(START_BIT | TRANSMISSION_BIT | START_RESERVED);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP6);
            *(respBuff + 1) = (BYTE)wRSP;
            *(respBuff + 2)= (BYTE)(wRSP >> 8);


            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP7);
            *(respBuff + 3) = (BYTE)wRSP;
            *(respBuff + 4) = (BYTE)(wRSP >> 8);

            *(respBuff + 5) = (BYTE)(END_RESERVED | END_BIT);

            DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(L"CSDIOControllerBase::GetCommandResponse: "
                L"Returned [%02x%02x%02x%02x%02x]\r\n", 
                respBuff[0], respBuff[1], respBuff[2], respBuff[3], respBuff[4], respBuff[5]
            ));
            break;

        case ResponseR5:                
        case ResponseR6:
            DEBUGMSG(SHC_RESPONSE_ZONE, (L"CSDIOControllerBase::GetCommandResponse: "
                L"ResponseR5 ResponseR6\r\n"
            ));
            //--- SHORT RESPONSE (48 bits total)--- 
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | COMMAND_INDEX(6) | RCA(16) | CARD_STATUS(16) | CRC7(7) | END_BIT(1) }
            //
            *respBuff = (BYTE)(START_BIT | TRANSMISSION_BIT | pRequest->CommandCode);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP6);
            *(respBuff + 1) = (BYTE)wRSP;
            *(respBuff + 2)= (BYTE)(wRSP >> 8);


            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP7);
            *(respBuff + 3) = (BYTE)wRSP;
            *(respBuff + 4) = (BYTE)(wRSP >> 8);

            *(respBuff + 5) = (BYTE)(END_BIT);

            DEBUGMSG(SHC_RESPONSE_ZONE, (L"CSDIOControllerBase::GetCommandResponse: "

                L"R5 R6 : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \r\n", *(respBuff + 0), 
                *(respBuff + 1), *(respBuff + 2), *(respBuff + 3), *(respBuff + 4), *(respBuff + 5)
            ));

            DEBUGMSG (SHC_SDBUS_INTERACT_ZONE,(L"CSDIOControllerBase::GetCommandResponse: "
                L"Returned [%02x%02x%02x%02x%02x]\r\n", 
                respBuff[0], respBuff[1], respBuff[2], respBuff[3], respBuff[4], respBuff[5]
            ));
            break;

        case ResponseR2:

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP0);
            *respBuff       = (BYTE)wRSP;
            *(respBuff + 1) = (BYTE)(wRSP >> 8);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP1);
            *(respBuff + 2) = (BYTE)wRSP;
            *(respBuff + 3) = (BYTE)(wRSP >> 8);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP2);
            *(respBuff + 4) = (BYTE)wRSP;
            *(respBuff + 5) = (BYTE)(wRSP >> 8);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP3);
            *(respBuff + 6) = (BYTE)wRSP;
            *(respBuff + 7) = (BYTE)(wRSP >> 8);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP4);
            *(respBuff + 8) = (BYTE)wRSP;
            *(respBuff + 9) = (BYTE)(wRSP >> 8);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP5);
            *(respBuff + 10) = (BYTE)wRSP;
            *(respBuff + 11) = (BYTE)(wRSP >> 8);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP6);
            *(respBuff + 12) = (BYTE)wRSP;
            *(respBuff + 13) = (BYTE)(wRSP >> 8);

            wRSP = INREG16(&m_vpSDIOReg->MMC_RSP7);
            *(respBuff + 14) = (BYTE)wRSP;
            *(respBuff + 15) = (BYTE)(wRSP >> 8);

            DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::GetCommandResponse: "
                L"Returned [%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x]\r\n", 
                respBuff[0], respBuff[1], respBuff[2], respBuff[3], respBuff[4], respBuff[5], respBuff[6], respBuff[7], 
                respBuff[8], respBuff[9], respBuff[10], respBuff[11], respBuff[12], respBuff[13], respBuff[14], respBuff[15]
            ));
            break;

        default:
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::GetCommandResponse: "
                L"Unrecognized response type!\r\n"
            ));
            break;
        }
    }
    return SD_API_STATUS_SUCCESS;
}


BOOL CSDIOControllerBase::SDIReceive(PBYTE pBuff, DWORD dwLen)
{
    return SDIPollingReceive( pBuff, dwLen );
}

BOOL CSDIOControllerBase::SDITransmit(PBYTE pBuff, DWORD dwLen)
{
    return SDIPollingTransmit( pBuff, dwLen );
}

//Function:     SDIPollingReceive()
//Description:  
//Notes:        This routine assumes that the caller has already locked
//              the current request and checked for errors.
//Returns:      SD_API_STATUS status code.
BOOL 
CSDIOControllerBase::SDIPollingReceive(PBYTE pBuff, DWORD dwLen)
{
    WORD wFIFOAlmostFullLevel = (((INREG16(&m_vpSDIOReg->MMC_BUF) >> 8) & 0x1F) + 1) * 2;
    BOOL fWaitingForBusyEnd = FALSE;
    DWORD dwCurrentTickCount;
    DWORD dwTimeout;
    DWORD dwCountStart;
    BOOL fTimeoutOverflow = FALSE;

    DEBUGMSG(SHC_RECEIVE_ZONE, (L"+CSDIOControllerBase::SDIPollingReceive: "
        L"dwLen 0x%x\n", dwLen
    ));
    //check the parameters

    WORD MMC_STAT = Read_MMC_STAT();

    if( ( MMC_STAT & MMC_STAT_CB ) && (!(MMC_STAT & MMC_STAT_EOFB)) )
    {
        DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::SDIPollingReceive: "
            L"Card in busy state before data receive!  Delaying...\r\n"
        ));
    }

    // calculate timeout conditions
    dwCountStart = GetTickCount();
    dwTimeout = dwCountStart + m_dwMaxTimeout;
    if( dwTimeout < dwCountStart )
        fTimeoutOverflow = TRUE;

    while( dwLen > 0 )
    {
        // check for a timeout
        dwCurrentTickCount = GetTickCount();
        if( fTimeoutOverflow ? ( dwTimeout < dwCurrentTickCount && dwCurrentTickCount < dwCountStart )
            : ( dwTimeout < dwCurrentTickCount || dwCurrentTickCount < dwCountStart ) )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingReceive: "
                L"Exit: TIMEOUT.\r\n"
            ));
            goto READ_ERROR;
        }

        if( !SDCardDetect() )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingReceive: "
                L"Exit: Card removed.\r\n"
            ));
            goto READ_ERROR;
        }

        if( MMC_STAT & MMC_STAT_DCRC ) // DATA CRC Error
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingReceive: "
                L"Exit: MMC_CRC_ERROR_RCVD.\r\n"
            ));
            goto READ_ERROR;
        }

        if( MMC_STAT & MMC_STAT_DTO ) // DATA Response Timeout
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingReceive: "
                L"Exit: MMC_TIME_OUT_RCVD.\r\n"
            ));
            goto READ_ERROR;
        }

        if( MMC_STAT & ( MMC_STAT_AF | MMC_STAT_BRS ) ) // buffer almost full or block received
        {
            Write_MMC_STAT( MMC_STAT_AF );
            DWORD dwTransferSize = wFIFOAlmostFullLevel;
            if( dwLen < dwTransferSize )
            {
                dwTransferSize = dwLen;
            }

            dwLen = dwLen - dwTransferSize;

            if( ((DWORD)pBuff) % 2 ) // the data buffer is not aligned
            {
                while( dwTransferSize > 1)
                {
                    WORD wData = INREG16(&m_vpSDIOReg->MMC_DATA);
                    BYTE bLo = wData & 0xff;
                    BYTE bHi = ( wData & 0xff00 ) >> 8;
                    *((BYTE*)pBuff) = bLo;
                    *((BYTE*)pBuff+1) = bHi;
                    dwTransferSize -= 2;
                    pBuff += 2;
                }
            }
            else
            {
                while( dwTransferSize > 1)
                {
                    *((WORD*)pBuff) = INREG16(&m_vpSDIOReg->MMC_DATA);
                    dwTransferSize -= 2;
                    pBuff += 2;
                }
            }

            if (dwTransferSize==1)
            {
                *((BYTE*)pBuff) = INREG16(&m_vpSDIOReg->MMC_DATA) & 0xff;
                dwTransferSize -= 1;
                pBuff += 1;
            }

            // recalculate timeout conditions
            dwCountStart = GetTickCount();
            dwTimeout = dwCountStart + m_dwMaxTimeout;
            if( dwTimeout < dwCountStart )
                fTimeoutOverflow = TRUE;
            else
                fTimeoutOverflow = FALSE;
        }

        MMC_STAT = Read_MMC_STAT();
    } // while

    Write_MMC_STAT( MMC_STAT_BRS );

    if( ( MMC_STAT & MMC_STAT_CB ) && (!(MMC_STAT & MMC_STAT_EOFB)) )
    {
        DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::SDIPollingReceive: "
            L"Card in busy state after data receive!  Delaying...\r\n"
        ));
    }

    return TRUE;  

READ_ERROR:
    return FALSE;
}


BOOL 
CSDIOControllerBase::SDIPollingTransmit(PBYTE pBuff, DWORD dwLen)
{
    DWORD dwCurrentTickCount;
    DWORD dwTimeout;
    DWORD dwCountStart;
    BOOL fTimeoutOverflow = FALSE;

    WORD wFIFOAlmostEmptyLevel = ((INREG16(&m_vpSDIOReg->MMC_BUF) & 0x1F) + 1) * 2;

    WORD MMC_STAT = Read_MMC_STAT();

    if( ( MMC_STAT & MMC_STAT_CB ) && (!(MMC_STAT & MMC_STAT_EOFB)) )
    {
        DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::SDIPollingTransmit: "
            L"Card in busy state before data transfer!  Delaying...\r\n"
        ));
    }

    ASSERT( ( MMC_STAT & MMC_STAT_BRS ) == 0 );

    // calculate timeout conditions
    dwCountStart = GetTickCount();
    dwTimeout = dwCountStart + m_dwMaxTimeout;
    if( dwTimeout < dwCountStart )
        fTimeoutOverflow = TRUE;

    while( dwLen > 0 ) 
    {
        // check for a timeout
        dwCurrentTickCount = GetTickCount();
        if( fTimeoutOverflow ? ( dwTimeout < dwCurrentTickCount && dwCurrentTickCount < dwCountStart )
            : ( dwTimeout < dwCurrentTickCount || dwCurrentTickCount < dwCountStart ) )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingTransmit: "
                L"Exit: TIMEOUT.\r\n"
            ));
            goto WRITE_ERROR;
        }

        if( !SDCardDetect() )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingTransmit: "
                L"Exit: Card removed.\r\n"
            ));
            goto WRITE_ERROR;
        }

        if( MMC_STAT & MMC_STAT_DCRC ) // DATA CRC Error
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingTransmit: "
                L"Exit: MMC_CRC_ERROR_RCVD.\r\n"
            ));
            goto WRITE_ERROR;
        }

        if( MMC_STAT & MMC_STAT_DTO ) // DATA Response Timeout
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingTransmit: "
                L"Exit: MMC_TIME_OUT_RCVD.\r\n"
            ));
            goto WRITE_ERROR;
        }

        if( MMC_STAT & MMC_STAT_CB ) // card in BUSY state
        {
            DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::SDIPollingTransmit: "
                L"Card in busy state during data transfer!  Delaying...\r\n"
            ));
            do {
                MMC_STAT = Read_MMC_STAT();

                if( !SDCardDetect() )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingTransmit: "
                        L"Exit: Card removed.\r\n"
                    ));
                    goto WRITE_ERROR;
                }

                // check for a timeout
                dwCurrentTickCount = GetTickCount();
                if( fTimeoutOverflow ? ( dwTimeout < dwCurrentTickCount && dwCurrentTickCount < dwCountStart )
                    : ( dwTimeout < dwCurrentTickCount || dwCurrentTickCount < dwCountStart ) )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingTransmit: "
                        L"Exit: TIMEOUT.\r\n"
                    ));
                    goto WRITE_ERROR;
                }

            } while( !( MMC_STAT & ( MMC_STAT_EOFB | MMC_STAT_DCRC | MMC_STAT_DTO ) ) );

            DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::SDIPollingTransmit: "
                L"Card exited busy state.\r\n"
            ));
            ASSERT( MMC_STAT & MMC_STAT_EOFB );
            Write_MMC_STAT( MMC_STAT_CB | MMC_STAT_EOFB );

            // recalculate timeout conditions
            dwCountStart = GetTickCount();
            dwTimeout = dwCountStart + m_dwMaxTimeout;
            if( dwTimeout < dwCountStart )
                fTimeoutOverflow = TRUE;
            else
                fTimeoutOverflow = FALSE;
        }

        if( MMC_STAT & MMC_STAT_AE ) // Almost empty
        {
            Write_MMC_STAT(MMC_STAT_AE);

            DWORD dwTransferSize = wFIFOAlmostEmptyLevel;
            if( dwLen < dwTransferSize )
            {
                dwTransferSize = dwLen;
            }

            dwLen = dwLen - dwTransferSize;

            if( ((DWORD)pBuff) % 2 ) // if the data buffer is not aligned
            {
                while( dwTransferSize > 1)
                {
                    BYTE bLo = *((BYTE*)pBuff);
                    BYTE bHi = *((BYTE*)pBuff + 1);
                    WORD wData = ( bHi << 8 ) | bLo; // assume little endian
                    OUTREG16(&m_vpSDIOReg->MMC_DATA, wData);
                    dwTransferSize -= 2;
                    pBuff += 2;
                } // while
            }
            else
            {
                while( dwTransferSize > 1)
                {
                    OUTREG16(&m_vpSDIOReg->MMC_DATA,*((WORD*)pBuff));
                    dwTransferSize -= 2;
                    pBuff += 2;
                } // while
            }
            if( dwTransferSize == 1 )
            {
                OUTREG16(&m_vpSDIOReg->MMC_DATA, (WORD)(*((BYTE*)pBuff)));
                dwTransferSize -= 1;
                pBuff += 1;
            }

            // recalculate timeout conditions
            dwCountStart = GetTickCount();
            dwTimeout = dwCountStart + m_dwMaxTimeout;
            if( dwTimeout < dwCountStart )
                fTimeoutOverflow = TRUE;
            else
                fTimeoutOverflow = FALSE;
        }

        MMC_STAT = Read_MMC_STAT();
    } // while

    // recalculate timeout conditions
    dwCountStart = GetTickCount();
    dwTimeout = dwCountStart + m_dwMaxTimeout;
    if( dwTimeout < dwCountStart )
        fTimeoutOverflow = TRUE;
    else
        fTimeoutOverflow = FALSE;

    // Wait for data to be transmitted to the card
    MMC_STAT = Read_MMC_STAT();
    if( !( MMC_STAT & MMC_STAT_BRS ) )
    {
        DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::SDIPollingTransmit: "
            L"Waiting for BRS bit...\r\n"
        ));

        while( !( MMC_STAT & MMC_STAT_BRS ) )
        {
            if( !SDCardDetect() )
            {
                break;
            }

            // check for a timeout
            dwCurrentTickCount = GetTickCount();
            if( fTimeoutOverflow ? ( dwTimeout < dwCurrentTickCount && dwCurrentTickCount < dwCountStart )
                : ( dwTimeout < dwCurrentTickCount || dwCurrentTickCount < dwCountStart ) )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingTransmit: "
                    L"Exit: TIMEOUT.\r\n"
                ));
                goto WRITE_ERROR;
            }

            Sleep(1);
            MMC_STAT = Read_MMC_STAT();
        }
        DEBUGMSG(SHC_SDBUS_INTERACT_ZONE, (L"CSDIOControllerBase::SDIPollingTransmit: "
            L"Got the BRS bit.\r\n"
        ));
    }

    // recalculate timeout conditions
    dwCountStart = GetTickCount();
    dwTimeout = dwCountStart + m_dwMaxTimeout;
    if( dwTimeout < dwCountStart )
        fTimeoutOverflow = TRUE;
    else
        fTimeoutOverflow = FALSE;

    // Wait for card if it is busy?
    if( MMC_STAT & MMC_STAT_CB )
    {
        DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::SDIPollingTransmit: "
            L"Card in busy state after data transfer!  Delaying...\r\n"
        ));

        while( !(MMC_STAT & MMC_STAT_EOFB) )
        {
            if( !SDCardDetect() )
            {
                break;
            }

            // check for a timeout
            dwCurrentTickCount = GetTickCount();
            if( fTimeoutOverflow ? ( dwTimeout < dwCurrentTickCount && dwCurrentTickCount < dwCountStart )
                : ( dwTimeout < dwCurrentTickCount || dwCurrentTickCount < dwCountStart ) )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOControllerBase::SDIPollingTransmit: "
                    L"Exit: TIMEOUT.\r\n"
                ));
                goto WRITE_ERROR;
            }

            Sleep(1);
            MMC_STAT = Read_MMC_STAT();
        } 
        DEBUGMSG(SHC_BUSY_STATE_ZONE, (L"CSDIOControllerBase::SDIPollingTransmit: "
            L"Card exited busy state!\r\n"
        ));
    }

    return TRUE;

WRITE_ERROR:
    return FALSE;
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

#pragma optimize("",on) // debug
