//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  nand_gpmi.c
//
//
//
//-----------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include "nand_hal.h"
#pragma warning(pop)
#include "csp.h"
#include "nand_gpmi.h"
#include "regsdigctl.h"
////////////////////////////////////////////////////////////////////////////////
// Definitions
//
// These values are used to initialize the GPMI NAND pins.
////////////////////////////////////////////////////////////////////////////////

#define MAX_GPMI_CLK_FREQUENCY_kHZ (120000)
#define FLASH_BUSY_TIMEOUT          10000000  //!< Busy Timeout time in nsec. (10msec)
#define FLASH_BUSY_TIMEOUT_DIV_4096 ((FLASH_BUSY_TIMEOUT + 4095) / 4096)
#define DDI_NAND_HAL_GPMI_SOFT_RESET_LATENCY    (1)
#define DDI_NAND_HAL_GPMI_SOFT_RESET_TIMEOUT    (5000)

#define NAND_GPMI_TIMING0(AddSetup, DataSetup, DataHold) \
    (BF_GPMI_TIMING0_ADDRESS_SETUP(AddSetup) | \
     BF_GPMI_TIMING0_DATA_HOLD(DataHold) | \
     BF_GPMI_TIMING0_DATA_SETUP(DataSetup))

//! Default structure that should be more than safe for initial reads.
const NANDTiming zFailsafeTimings =
{
    100,          //!< Data Setup (ns)
    80,          //!< Data Hold (ns)
    120,          //!< Address Setup (ns)
    10            //!< DSAMPLE_TIME (ns)
};

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////

static INT FindGpmiCycles(UINT32 u32NandTime_ns, UINT32 u32GpmiPeriod_ns);

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
void GPMI_Reset()
{
    UINT32 musecs;
    
    RETAILMSG(1, (_T("reset GPMI module\r\n")));
    
    // Prepare for soft-reset by making sure that SFTRST is not currently
    // asserted.  Also clear CLKGATE so we can wait for its assertion below.
    HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
    
    // Wait at least a microsecond for SFTRST to deassert. In actuality, we
    // need to wait 3 GPMI clocks, but this is much easier to implement.
    
    musecs = HW_DIGCTL_MICROSECONDS_RD();
    while(HW_DIGCTL_MICROSECONDS_RD() - musecs < DDI_NAND_HAL_GPMI_SOFT_RESET_LATENCY);
    
    musecs = HW_DIGCTL_MICROSECONDS_RD();
    while (HW_GPMI_CTRL0.B.SFTRST)
    {
        if(HW_DIGCTL_MICROSECONDS_RD() - musecs > DDI_NAND_HAL_GPMI_SOFT_RESET_TIMEOUT)
        {
            ERRORMSG(TRUE, (_T("Wait for SFTRST low fail\r\n")));
            break;   
        }       
    }
    
    // Also clear CLKGATE so we can wait for its assertion below.
    HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);
    
    // Now soft-reset the hardware.
    HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_SFTRST);
    
    // Poll until clock is in the gated state before subsequently
    // clearing soft reset and clock gate.
    musecs = HW_DIGCTL_MICROSECONDS_RD();
    while (!HW_GPMI_CTRL0.B.CLKGATE)
    {
        if(HW_DIGCTL_MICROSECONDS_RD() - musecs > DDI_NAND_HAL_GPMI_SOFT_RESET_TIMEOUT)
        {
            ERRORMSG(TRUE, (_T("Wait for CLKGATE high fail\r\n")));
            break;   
        }  
    }
    
    // bring GPMI_CTRL0 out of reset
    HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
    
    // Wait at least a microsecond for SFTRST to deassert. In actuality, we
    // need to wait 3 GPMI clocks, but this is much easier to implement.
    musecs = HW_DIGCTL_MICROSECONDS_RD();
    while(HW_DIGCTL_MICROSECONDS_RD() - musecs < DDI_NAND_HAL_GPMI_SOFT_RESET_LATENCY);
    
    while (HW_GPMI_CTRL0.B.SFTRST)
    {
        if(HW_DIGCTL_MICROSECONDS_RD() - musecs > DDI_NAND_HAL_GPMI_SOFT_RESET_TIMEOUT)
        {
            ERRORMSG(TRUE, (_T("Wait for SFTRST low fail\r\n")));
            break;   
        }  
    }
    
    HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);
    
    // Poll until clock is in the NON-gated state before returning.
    while (HW_GPMI_CTRL0.B.CLKGATE)
    {
        if(HW_DIGCTL_MICROSECONDS_RD() - musecs > DDI_NAND_HAL_GPMI_SOFT_RESET_TIMEOUT)
        {
            ERRORMSG(TRUE, (_T("Wait for CLKGATE low fail\r\n")));
            break;   
        }  
    }
}

BOOL GPMI_Init()
{
    UINT32 frequency , rootfreq, u32Div;
    BOOL status = FALSE;

    // Can't boot from NAND if GPMI block is not present
    if (!(HW_GPMI_STAT_RD() & BM_GPMI_STAT_PRESENT)) {
        RETAILMSG(0, (TEXT("-NAND_GPMIEnable: BM_GPMI_STAT_PRESENT\r\n")));
        return FALSE;
    }

    // Bump GPMI_CLK frequency up to the maximum.
    frequency = MAX_GPMI_CLK_FREQUENCY_kHZ;
    //status = DDKClockSetGpmiClk(&frequency, TRUE);

    DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_IO, &rootfreq);

    u32Div = rootfreq / (frequency*1000) + 1;
    if(u32Div != 0)
        status = DDKClockConfigBaud(DDK_CLOCK_SIGNAL_GPMI_CLK, DDK_CLOCK_BAUD_SOURCE_REF_IO, u32Div );
    if (status != TRUE)
    {
        return status;
    }
    //DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, FALSE);

    GPMI_Reset();

    // Use the failsafe timings and default 24MHz clock
    GPMI_SetTiming((NANDTiming *)&zFailsafeTimings, 0);

    // Put GPMI in NAND mode, disable DEVICE reset, and make certain
    // polarity is active high, sample on GPMI clock
    HW_GPMI_CTRL1_WR(
        BF_GPMI_CTRL1_DEV_RESET(BV_GPMI_CTRL1_DEV_RESET__DISABLED) |
        BF_GPMI_CTRL1_ATA_IRQRDY_POLARITY(BV_GPMI_CTRL1_ATA_IRQRDY_POLARITY__ACTIVEHIGH) |
        BW_GPMI_CTRL1_GPMI_MODE(BV_GPMI_CTRL1_GPMI_MODE__NAND));
    HW_GPMI_CTRL1_SET(BM_GPMI_CTRL1_BCH_MODE);
    
    return TRUE;
}

static INT FindGpmiCycles(UINT32 u32NandTime_ns, UINT32 u32GpmiPeriod_ns)
{
    return (u32NandTime_ns + u32GpmiPeriod_ns - 1) / u32GpmiPeriod_ns;
}

VOID GPMI_SetTiming(NANDTiming * pNewNANDTiming, UINT32 u32GpmiPeriod_ns)
{
    NANDTiming * pNANDTiming = (NANDTiming *) pNewNANDTiming;
    
    // If u32GpmiPeriod is passed in as 0, we get the current GPMI_CLK frequency
    // and compute the period in ns.
    if (u32GpmiPeriod_ns == 0)
    {
        UINT32 freq_kHz = MAX_GPMI_CLK_FREQUENCY_kHZ; 
        u32GpmiPeriod_ns = 1000000 / freq_kHz;
    }

    {
        UINT32 u32AddressSetup ;
        UINT32 u32DataSetup ;
        UINT32 u32DataHold ;
        UINT32 u32DataSampleTime ;
        UINT32 u32BusyTimeout ;

        u32AddressSetup = FindGpmiCycles(pNANDTiming->AddressSetup, u32GpmiPeriod_ns);
        u32DataSetup = FindGpmiCycles(pNANDTiming->DataSetup, u32GpmiPeriod_ns);
        u32DataHold = FindGpmiCycles(pNANDTiming->DataHold, u32GpmiPeriod_ns);
        u32DataSampleTime = FindGpmiCycles((pNANDTiming->DataSample+ (u32GpmiPeriod_ns >> 2)), (u32GpmiPeriod_ns>>1)) - 1;

        HW_GPMI_TIMING0_WR(NAND_GPMI_TIMING0(u32AddressSetup, u32DataSetup, u32DataHold));

        BW_GPMI_CTRL1_RDN_DELAY(u32DataSampleTime);
        u32BusyTimeout = FindGpmiCycles(FLASH_BUSY_TIMEOUT_DIV_4096, u32GpmiPeriod_ns);

        // Number of cycles / 4096.
        HW_GPMI_TIMING1_WR( BF_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT(u32BusyTimeout));
        HW_GPMI_TIMING2_WR(0x02020101);
    }
}

