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
//  File: sdcontroller.cpp
//
//

#include <windows.h>
#include <initguid.h>
#include "SDController.h"
#include <nkintr.h>
// needed for CPU revision constants
#include <oalex.h>

//#define RETAIL_MSG_ENABLED  1
#define DEBOUNCE_TIME           100     // 100ms debounce timeout

//for NETRA: BIT-6 CINS_ENABLE;  BIT-7 CREM_ENABLE
#define MMC_CD_INT_EN_MASK      (MMCHS_IE_CREM|MMCHS_IE_CINS) // 0x000000C0

//------------------------------------------------------------------------------
//  Global variables

CSDIOController::CSDIOController()
    : CSDIOControllerBase()
{
    eSDHCCDIntr = SDHC_INTR_DISABLED;
}

CSDIOController::~CSDIOController()
{
}


//-----------------------------------------------------------------------------
BOOL CSDIOController::InitializeInterrupt()
{
    return TRUE;
}


BOOL CSDIOController::InitializeWPDetect(void)
{
    return TRUE;
}

BOOL CSDIOController::DeInitializeWPDetect(void)
{
    return TRUE;
}

// Is this card write protected?
BOOL CSDIOController::IsWriteProtected()
{
    DWORD MMC_PSTATE = INREG32(&m_pbRegisters->MMCHS_PSTATE);

    return ( (MMC_PSTATE & MMCHS_PSTAT_WP) >> MMCHS_PSTAT_WP_SHIFT );
}

// Is the card present?
//          CDP
//       | 0   1
//---------------
//      0| F   T
// CINS  |
//      1| T   F
BOOL CSDIOController::SDCardDetect()
{
    DWORD MMC_CON    = INREG32(&m_pbRegisters->MMCHS_CON);
    DWORD MMC_PSTATE = INREG32(&m_pbRegisters->MMCHS_PSTATE);

    return ( ((MMC_CON & MMCHS_CON_CDP) >> MMCHS_CON_CDP_SHIFT) !=
             ((MMC_PSTATE & MMCHS_PSTAT_CINS) >> MMCHS_PSTAT_CINS_SHIFT) );
}

//-----------------------------------------------------------------------------
BOOL CSDIOController::InitializeCardDetect()
{
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL CSDIOController::DeInitializeCardDetect()
{
    return TRUE;
}


typedef volatile struct
{
    UINT32 SD_HL_REV;
    UINT32 SD_HL_HWINFO;
    UINT32 SD_HL_SYSCONFIG;
} AM33X_MMCHS_EXT_REGS;


//-----------------------------------------------------------------------------
BOOL CSDIOController::InitializeHardware()
{
    DWORD               dwCountStart;

    volatile UINT32  *pCM_PER_MMC0_CLKCTRL;
#if RETAIL_MSG_ENABLED
    PHYSICAL_ADDRESS physicalAddress;
    AM33X_MMCHS_EXT_REGS *pExtRegs = NULL;
    AM33X_SYSC_PADCONFS_REGS *pPadRegs = NULL;

    RETAILMSG(1,  (L">>>>> +InitializeHardware\r\n")); 
#endif

    // Turn on SDIO clk (if needed)
    // +++FIXME:  should have used (ref sdhc.cpp)
    //              m_dwDeviceID = SOCGetSDHCDeviceBySlot(m_dwSlot);
    //            and access CM_PER_MMC(0/1/2)_CLKCTRL accordingly
    
    // in Netra/Centaurus/Subarctic manually enable clock here. 
    EnableDeviceClocks(SOCGetSDHCDeviceBySlot(m_dwSlot), TRUE);

#if RETAIL_MSG_ENABLED
    // Print netra sdhc ext regs
    physicalAddress.HighPart = 0;
    physicalAddress.LowPart  = AM33X_MMCHS0_REGS_PA;
    pExtRegs = (AM33X_MMCHS_EXT_REGS *)MmMapIoSpace(physicalAddress, sizeof(AM33X_MMCHS_EXT_REGS), FALSE);

    RETAILMSG(1,  (L">>>>> AM33X_MMCHS_EXT_REGS(0x%08X): Rev=0x%08X HL_HWINFO=0x%08X HL_SYSC=0x%08X\r\n", 
        pExtRegs, pExtRegs->SD_HL_REV, pExtRegs->SD_HL_HWINFO, pExtRegs->SD_HL_SYSCONFIG ));

    // Print SD PAD configs
    physicalAddress.HighPart = 0;
    physicalAddress.LowPart  = AM33X_SYSC_PADCONFS_REGS_PA;
	pPadRegs = (AM33X_SYSC_PADCONFS_REGS *)MmMapIoSpace(physicalAddress, sizeof(AM33X_SYSC_PADCONFS_REGS), FALSE);

    RETAILMSG(1,  (L">>>>> AM33X_SYSC_PADCONFS_REGS(0x%08X): \r\n", pPadRegs ));
    RETAILMSG(1,  (L"      MMC_CLK        (0x%08X)  0x%08X \r\n", 
        &(pPadRegs->CONF_MMC0_CLK), pPadRegs->CONF_MMC0_CLK ));
    RETAILMSG(1,  (L"      MMC_CMD        (0x%08X)  0x%08X \r\n", 
        &(pPadRegs->CONF_MMC0_CMD), pPadRegs->CONF_MMC0_CMD ));
    RETAILMSG(1,  (L"      MMC_DAT0       (0x%08X)  0x%08X \r\n", 
        &(pPadRegs->CONF_MMC0_DAT0), pPadRegs->CONF_MMC0_DAT0 ));
    RETAILMSG(1,  (L"      MMC_DAT1       (0x%08X)  0x%08X \r\n", 
        &(pPadRegs->CONF_MMC0_DAT1), pPadRegs->CONF_MMC0_DAT1));
    RETAILMSG(1,  (L"      MMC_DAT2       (0x%08X)  0x%08X \r\n", 
        &(pPadRegs->CONF_MMC0_DAT2), pPadRegs->CONF_MMC0_DAT2 ));
    RETAILMSG(1,  (L"      MMC_DAT3       (0x%08X)  0x%08X \r\n", 
        &(pPadRegs->CONF_MMC0_DAT3), pPadRegs->CONF_MMC0_DAT3 ));

#endif

    // Reset the controller
    OUTREG32(&m_pbRegisters->MMCHS_SYSCONFIG, MMCHS_SYSCONFIG_SOFTRESET);

    // calculate timeout conditions
    dwCountStart = GetTickCount();

    // Verify that reset has completed.
    while (!(INREG32(&m_pbRegisters->MMCHS_SYSSTATUS) & MMCHS_SYSSTATUS_RESETDONE))
    {
        Sleep(0);

        // check for timeout
        if (GetTickCount() - dwCountStart > m_dwMaxTimeout)
        {
            DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("InitializeModule() - exit: TIMEOUT.\r\n")));
            goto cleanUp;
        }
    }

    InitializeCardDetect();
    InitializeWPDetect();
#if RETAIL_MSG_ENABLED
    RETAILMSG(1,  (L">>>>> -InitializeHardware\r\n")); 
#endif
    return TRUE;
cleanUp:
    RETAILMSG(1,  (L"-InitializeHardware: Error\r\n")); 
    return FALSE;
}

void CSDIOController::DeinitializeHardware()
{
    DeInitializeCardDetect();
    DeInitializeWPDetect();
}

//-----------------------------------------------------------------------------
//  SDHCCardDetectIstThreadImpl - card detect IST thread for driver
//  Input:  lpParameter - pController - controller instance
//  Output:
//  Return: Thread exit status
DWORD CSDIOController::SDHCCardDetectIstThreadImpl()
{
    // check for the card already inserted when the driver is loaded
    if(SDCardDetect())
    {
        CardInterrupt(TRUE);
    }

#if RETAIL_MSG_ENABLED
    RETAILMSG(1, (L"SDHCCardDetectIstThreadImpl: Thread not needed in NETRA.  Exit.\r\n"));
#endif

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------
VOID CSDIOController::PreparePowerChange(CEDEVICE_POWER_STATE curPowerState, BOOL bInPowerHandler)
{
    UNREFERENCED_PARAMETER(bInPowerHandler);

    #ifdef MMCHS1_VDDS_WORKAROUND
        if (m_ActualPowerState != curPowerState)
        switch (curPowerState)
        {
            case D0:
            case D1:
            case D2:
                if (m_dwSlot == MMCSLOT_1)
                {
                    // Make sure VDDS stable bit is cleared before enabling the power for slot1
                    //m_pSyscGeneralRegs->CONTROL_PBIAS_LITE &= (~0x00000003);
                } 
                //else if (m_dwSlot == MMCSLOT_2)
                //{
                //    m_pSyscGeneralRegs->CONTROL_DEVCONF1 &= ~(1 << 6);  // clear MMCSDIO2ADPCLKISEL bit
                //}
                break;

            case D3:
            case D4:
                break;
       }
    #endif
}

VOID CSDIOController::PostPowerChange(CEDEVICE_POWER_STATE curPowerState, BOOL bInPowerHandler)
{
	UNREFERENCED_PARAMETER(bInPowerHandler);

    #ifdef MMCHS1_VDDS_WORKAROUND
        if (m_ActualPowerState != curPowerState)
        switch (curPowerState)
        {
            case D0:
            case D1:
            case D2:
                if (m_dwSlot == MMCSLOT_1)
                {
					/*
                    UINT32 dwPBiasValue = DEFAULT_PBIAS_VALUE;

                    if (m_dwCPURev == CPU_FAMILY_35XX_REVISION_ES_1_0)   // ES 1.0
                        dwPBiasValue = 0x00000003;
                    else if (m_dwCPURev == CPU_FAMILY_35XX_REVISION_ES_2_0) // ES 2.0
                        dwPBiasValue = 0x00000002;
                    else if (m_dwCPURev == CPU_FAMILY_35XX_REVISION_ES_2_1) // ES 2.1
                        #ifdef MMCHS1_LOW_VOLTAGE
                            dwPBiasValue = 0x00000002;
                        #else
                            dwPBiasValue = 0x00000003;
                        #endif
                    else
                        dwPBiasValue = DEFAULT_PBIAS_VALUE;

					if (bInPowerHandler)
						StallExecution(100000);
					else
	                    Sleep(100);
                    // Workaround to make the MMC slot 1 work
                    //m_pSyscGeneralRegs->CONTROL_PBIAS_LITE |= dwPBiasValue;
                    //m_pSyscGeneralRegs->CONTROL_DEVCONF1 |= (1 << 24); // set reserved bit???
					*/
                }
                //else if (m_dwSlot == MMCSLOT_2)
                //{
                //    Sleep(100);
                //    m_pSyscGeneralRegs->CONTROL_DEVCONF1 |= (1 << 6); // set MMCSDIO2ADPCLKISEL bit
                //}
                break;

            case D3:
            case D4:
                #if 0
                #ifndef MMCHS1_LOW_VOLTAGE
                    // clear MMC1_ACTIVE_OVERWRITE before the device goes in retention or to OFF
                    if (m_dwSlot == MMCSLOT_1 && m_dwCPURev == CPU_FAMILY_35XX_REVISION_ES_2_1)
                        m_pSyscGeneralRegs->CONTROL_DEVCONF1 &= ~(1 << 31); // clear reserved bit???
                #endif
                #endif
                break;
        }
    #endif
}

VOID CSDIOController::TurnCardPowerOn()
{
    SetSlotPowerState( D0 );
}

VOID CSDIOController::TurnCardPowerOff()
{
    SetSlotPowerState( D4 );
}

CSDIOControllerBase *CreateSDIOController()
{
    return new CSDIOController();
}

//-----------------------------------------------------------------------------
// Enable SDHC Card Detect Interrupts.
VOID CSDIOController::EnableSDHCCDInterrupts()
{
    EnterCriticalSection( &m_critSec );
    SETREG32(&m_pbRegisters->MMCHS_ISE, MMC_CD_INT_EN_MASK);
    SETREG32(&m_pbRegisters->MMCHS_IE,  MMC_CD_INT_EN_MASK);
    eSDHCCDIntr = SDHC_MMC_INTR_ENABLED;
    LeaveCriticalSection( &m_critSec );
}

//-----------------------------------------------------------------------------
// Disable SDHC Card Detect Interrupts.
void CSDIOController::DisableSDHCCDInterrupts()
{
    EnterCriticalSection( &m_critSec );
    SETREG32(&m_pbRegisters->MMCHS_ISE, ~MMC_CD_INT_EN_MASK);
    SETREG32(&m_pbRegisters->MMCHS_IE,  ~MMC_CD_INT_EN_MASK);
    eSDHCCDIntr = SDHC_INTR_DISABLED;
    LeaveCriticalSection( &m_critSec );
}

void CSDIOController::Write_MMC_CD_STAT( DWORD dwVal )
{
    EnterCriticalSection( &m_critSec );
    dwVal &= (MMCHS_STAT_CREM|MMCHS_STAT_CINS);
    OUTREG32(&m_pbRegisters->MMCHS_STAT, dwVal);
    LeaveCriticalSection( &m_critSec );
}

void CSDIOController::SetCDPolarity()
{
    return;
}

