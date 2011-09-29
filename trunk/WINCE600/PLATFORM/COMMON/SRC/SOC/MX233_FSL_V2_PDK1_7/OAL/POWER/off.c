//------------------------------------------------------------------------------
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
// NOTE: stubs are being used - this isn't done
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  off.c
//
//  This file provides the capabilities to suspend the system and controlling
//  wake sources.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <cmnintrin.h>
#include "poweroff.h"
#pragma warning(pop)

#include "csp.h"
#include "mx233_timrot.h"
#include "regsicoll.h"

// Need to disable function pointer cast warnings.  We use memcpy to move 
// the bus scaling calibration routine into IRAM and then cast this address 
// as a function pointer.
#pragma warning(disable: 4054 4055)

//------------------------------------------------------------------------------
// External Functions
extern UINT32 GetPowerOffIRAMSize(VOID);

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregPINCTRL;
extern PVOID pv_HWregICOLL;
extern PVOID pv_HWregTIMROT;
extern PVOID pv_HWregDIGCTL;
extern PVOID pv_HWregPOWER;
extern PVOID pv_HWregCLKCTRL;
extern PVOID pv_HWregGPMI;
extern PVOID pv_HWregAPBH;
extern PVOID pv_HWregAPBX;
extern PVOID pv_HWregUSBPhy;
extern PVOID pv_HWRegPWM;
extern PVOID pv_HWregAUDIOIN;
extern PVOID pv_HWregAUDIOOUT;
extern PVOID pv_HWregBCH;
extern PVOID pv_HWregLRADC;
extern PVOID pv_HWregLCDIF;
extern PVOID pv_HWregDRAM;

extern CSP_RTC_REGS *pRtcReg;

extern FUNC_POWER_CONTROL pControlFunc;

//------------------------------------------------------------------------------
// Defines



//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
static void SuspendTheSystem(VOID);


//------------------------------------------------------------------------------
// Local Variables
typedef struct _SOC_POWEROFF_BACKUP
{
    volatile DWORD CLKCTRL_PLLCTRL0;
    volatile DWORD CLKCTRL_PIX;
    volatile DWORD CLKCTRL_HBUS;
    volatile DWORD CLKCTRL_SSP;
    volatile DWORD CLKCTRL_GPMI;
    volatile DWORD CLKCTRL_SPDIF;
    volatile DWORD CLKCTRL_IR;
    volatile DWORD CLKCTRL_SAIF;
    volatile DWORD CLKCTRL_TV;
    volatile DWORD CLKCTRL_FRAC;    
    volatile DWORD CLKCTRL_FRAC1;
    volatile DWORD CLKCTRL_CLKSEQ;
    volatile DWORD CLKCTRL_XTAL;
    volatile DWORD GPMI_CTRL0;
    volatile DWORD GPMI_CTRL1;
    volatile DWORD GPMI_TIMING0;
    volatile DWORD GPMI_TIMING1;
    volatile DWORD APBH_CTRL0;
    volatile DWORD APBH_CTRL1;
    volatile DWORD APBH_CTRL2;
    volatile DWORD APBX_CTRL0;
    volatile DWORD APBX_CTRL1;
    volatile DWORD APBX_CTRL2;
    volatile DWORD PWM_CTRL;  
    volatile DWORD RTC_CTRL;    
    volatile DWORD LRADC_CTRL0;  
    volatile DWORD LRADC_CTRL1; 
    volatile DWORD LRADC_CTRL2;  
    volatile DWORD LRADC_CTRL3;
    volatile DWORD LRADC_CTRL4;
    
}SOC_POWEROFF_BACKUP, *PSOC_POWEROFF_BACKUP;
static SOC_POWEROFF_BACKUP SocBackupReg;

BOOL bAudioInSoftReset = FALSE;
BOOL bAudioInClkGate = FALSE;
BOOL bAudioOutSoftReset = FALSE;
BOOL bAudioOutClkGate = FALSE;

BOOL bPwrDnRightAdc = FALSE;
BOOL bPwrDnDac = FALSE;
BOOL bPwrDnAdc = FALSE;
BOOL bPwrDnHeadphone = FALSE;


//------------------------------------------------------------------------------
// Local Functions
static void SOCPowerOffReg(void);
static void SOCPowerOnReg(void);
static void SOCPowerOffCPURate(void);
static void SOCPowerOnCPURate(void);
static void SuspendMe(void);

//-----------------------------------------------------------------------------
//
// Function: OEMPowerOff
//
// Called when the system is to transition to it's lowest power mode.  This
// function stores off some of the important registers (not really needed for
// DSM since registers will not lose state in DSM).
//
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void OEMPowerOff()
{

    UINT32 registerStore[IRQ_RESERVED_95+1+DDK_GPIO_BANK3+1];
    UINT32 sysIntr, irq;
    UINT32 line, powersts;
    BOOL PowerState = TRUE;

    OALMSG(OAL_FUNC,(_T("OEMPowerOff!!\r\n")));  
 
    // Call board-level power off function
    BSPPowerOff();

    //Leave suspend if 5v power supply 
    powersts = HW_POWER_STS_RD();
    if((powersts & BM_POWER_STS_VBUSVALID) != 0) return;

    // Reset the wake source global
    g_oalWakeSource = SYSWAKE_UNKNOWN;

    //Save state of GPIO interrupt
    registerStore[IRQ_RESERVED_95 + 1]= HW_PINCTRL_IRQEN0_RD();
    registerStore[IRQ_RESERVED_95 + 2]= HW_PINCTRL_IRQEN1_RD();
    registerStore[IRQ_RESERVED_95 + 3]= HW_PINCTRL_IRQEN2_RD();

    //Mask all interrupt source
    HW_PINCTRL_IRQEN0_CLR(0xFFFFFFFF);
    HW_PINCTRL_IRQEN1_CLR(0xFFFFFFFF);    
    HW_PINCTRL_IRQEN2_CLR(0xFFFFFFFF);
    
    // Save state of enabled interrupts and Mask interrupt
    for(irq = IRQ_DEBUG_UART; irq <= IRQ_RESERVED_95; irq++ )
    {
        registerStore[irq] = HW_ICOLL_INTERRUPTn_RD(irq);
        HW_ICOLL_INTERRUPTn_CLR(irq, BM_ICOLL_INTERRUPTn_ENABLE);
    }

    // backup SOC registers and power down them
    SOCPowerOffReg();

    // Now enable interrupt if it was enabled as wakeup source
    for (sysIntr = SYSINTR_DEVICES; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
    {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) continue;
        
        // Enable it as wakeup source interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);

        // Set flag to indicate we have at least one wake source
        PowerState = FALSE;
    }
/*
    for(irq = IRQ_DEBUG_UART; irq <= IRQ_RESERVED_95; irq++ )
    {
        if( HW_ICOLL_INTERRUPTn_RD(irq) != 0)    
        OALMSG(1,(_T("Wakeup source =%d\r\n"), irq));    
    }
*/
    if (!PowerState)
    {
        // Switch off power for KITL device
        KITLIoctl(IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

        // Place the system in suspend state and wait for an interrupt.
        OALMSG(1, 
            (_T("INFO: OEMPowerOff entering suspend.\r\n")));

        // Decrease ARM/AHB rate 
        SOCPowerOffCPURate();

        OALStall(1000);

        //Suspend 
        SuspendMe();

        // Increase ARM/AHB rate 
        SOCPowerOnCPURate();

        // Get interrupt source
        irq = HW_ICOLL_STAT_RD() & BM_ICOLL_STAT_VECTOR_NUMBER;

        // If valid wake interrupt is pending

        // GPIO0 special case
        if(irq == IRQ_GPIO0)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(HW_PINCTRL_IRQSTAT0_RD() & HW_PINCTRL_IRQEN0_RD());

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO0_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }

        // GPIO1 special case
        else if(irq == IRQ_GPIO1)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(HW_PINCTRL_IRQSTAT1_RD() & HW_PINCTRL_IRQEN1_RD());

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO1_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }

        // GPIO2 special case
        else if(irq == IRQ_GPIO2)
        { 
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(HW_PINCTRL_IRQSTAT2_RD() & HW_PINCTRL_IRQEN2_RD());

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO2_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }

        // Chance of board-level subordinate interrupt controller
        irq = BSPIntrActiveIrq(irq);
        
        //OALMSG(1, (_T("INFO: OEMPowerOff leaving suspend.  IRQ = %d\r\n"), irq));

        while((HW_POWER_STS_RD() & BM_POWER_STS_PSWITCH) != 0);
        OALStall(10000);
        if(irq == IRQ_VDD5V)
            HW_POWER_CTRL_CLR(BM_POWER_CTRL_PSWITCH_IRQ);
        
        // Map the irq to a SYSINTR
        g_oalWakeSource = OALIntrTranslateIrq(irq);
        
    }
    else
    {
        OALMSG(1, (_T("INFO: No wake sources for OEMPowerOff, system will resume...\r\n")));
    }

    // Restore register state
    SOCPowerOnReg();

    // Restore state of enabled interrupts
    for(irq = IRQ_DEBUG_UART; irq <= IRQ_RESERVED_95; irq++ )
    {
        HW_ICOLL_INTERRUPTn_CLR(irq, 0xFFFFFFFF);
        HW_ICOLL_INTERRUPTn_SET(irq, registerStore[irq]);
    }

    HW_PINCTRL_IRQEN0_SET(registerStore[IRQ_RESERVED_95 + 1]);
    HW_PINCTRL_IRQEN1_SET(registerStore[IRQ_RESERVED_95 + 2]);
    HW_PINCTRL_IRQEN2_SET(registerStore[IRQ_RESERVED_95 + 3]);


    // Switch on power for KITL device
    PowerState = TRUE;
    KITLIoctl (IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

    // Do platform dependent power on actions
    BSPPowerOn();

}

//------------------------------------------------------------------------------
//
// Function:     SuspendTheSystem
//
// Description:  
//
VOID SuspendTheSystem(VOID)
{

    DWORD Reg_Data;

    //DDR EnterSelfrefreshMode
    //HW_DRAM_CTL16_SET(BF_DRAM_CTL16_LOWPOWER_CONTROL(2));
    HW_DRAM_CTL16_SET(1<<9);
    HW_DRAM_CTL16_SET(1<<17);

    //enable pswitch IRQ
    HW_POWER_CTRL_CLR(BM_POWER_CTRL_PSWITCH_IRQ_SRC);
    
    //Gating EMI CLock 
    HW_CLKCTRL_EMI_SET(BM_CLKCTRL_EMI_CLKGATE);
  
    //--------------------------------------------------------------------------
    // Voltage rails
    //--------------------------------------------------------------------------
      
    // Reduce the VDDIO (3.050 volt)
    HW_POWER_VDDIOCTRL_WR(HW_POWER_VDDIOCTRL_RD() | BM_POWER_VDDIOCTRL_BO_OFFSET);
    HW_POWER_VDDIOCTRL_WR((HW_POWER_VDDIOCTRL_RD() & ~BM_POWER_VDDIOCTRL_TRG)|10);
    // need to wait more than 15 microsecond before the DC_OK is valid  
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 
    
    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    // Reduce VDDA 1.725volt
    HW_POWER_VDDACTRL_WR(HW_POWER_VDDACTRL_RD() | BM_POWER_VDDACTRL_BO_OFFSET);
    HW_POWER_VDDACTRL_WR((HW_POWER_VDDACTRL_RD() & ~BM_POWER_VDDACTRL_TRG) | 9);
    // need to wait more than 15 microsecond before the DC_OK is valid
    
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    // Reduce VDDD 1.000 volt
    HW_POWER_VDDDCTRL_WR(HW_POWER_VDDDCTRL_RD() | BM_POWER_VDDDCTRL_BO_OFFSET);
    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_TRG) | 8);

    // need to wait more than 15 microsecond before the DC_OK is valid
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    //--------------------------------------------------------------------------
    // optimize the DCDC loop gain
    //--------------------------------------------------------------------------     
    HW_POWER_LOOPCTRL_WR((HW_POWER_LOOPCTRL_RD() & ~BM_POWER_LOOPCTRL_EN_RCSCALE));
    HW_POWER_LOOPCTRL_WR((HW_POWER_LOOPCTRL_RD() & ~BM_POWER_LOOPCTRL_DC_R) | (2<<BP_POWER_LOOPCTRL_DC_R));
    
    // half the fets
    HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_DOUBLE_FETS); 
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_HALF_FETS);
    
    HW_POWER_LOOPCTRL_CLR(BM_POWER_LOOPCTRL_CM_HYST_THRESH);   
    HW_POWER_LOOPCTRL_CLR(BM_POWER_LOOPCTRL_EN_CM_HYST);
    HW_POWER_LOOPCTRL_CLR(BM_POWER_LOOPCTRL_EN_DF_HYST);
    // enable PFM
    HW_POWER_LOOPCTRL_SET(BM_POWER_LOOPCTRL_HYST_SIGN);
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_EN_DC_PFM);

    //POWER: Good for about 40uA.
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_LESSANA_I);    
    HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_ILIMIT_EQ_ZERO);   
    

    //Disable PLL
    HW_CLKCTRL_PLLCTRL0_CLR(BM_CLKCTRL_PLLCTRL0_POWER);

    // Power off ...
    _MoveToCoprocessor(0, 15, 0, 7, 0, 4);

    //Enable PLL
    HW_CLKCTRL_PLLCTRL0_SET(BM_CLKCTRL_PLLCTRL0_POWER);
    
    //--------------------------------------------------------------------------
    // restore the DCDC parameter
    //--------------------------------------------------------------------------     

    HW_POWER_5VCTRL_CLR(BM_POWER_5VCTRL_ILIMIT_EQ_ZERO); 
    HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_LESSANA_I);

    HW_POWER_MINPWR_CLR (BM_POWER_MINPWR_EN_DC_PFM);
    HW_POWER_LOOPCTRL_CLR(BM_POWER_LOOPCTRL_HYST_SIGN);
    HW_POWER_LOOPCTRL_SET(BM_POWER_LOOPCTRL_EN_DF_HYST);
    HW_POWER_LOOPCTRL_SET(BM_POWER_LOOPCTRL_EN_CM_HYST);
    HW_POWER_LOOPCTRL_SET(BM_POWER_LOOPCTRL_CM_HYST_THRESH);

    // Clear half the fets
    HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_HALF_FETS);
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_DOUBLE_FETS);   
    
    HW_POWER_LOOPCTRL_WR((HW_POWER_LOOPCTRL_RD() & ~BM_POWER_LOOPCTRL_DC_R) |
                         (2<<BP_POWER_LOOPCTRL_DC_R));
    HW_POWER_LOOPCTRL_WR((HW_POWER_LOOPCTRL_RD() & ~BM_POWER_LOOPCTRL_EN_RCSCALE) |
                        (3 << BP_POWER_LOOPCTRL_EN_RCSCALE));


    //--------------------------------------------------------------------------
    // Voltage rails
    //--------------------------------------------------------------------------
 
    // Restore VDDD 1.525 volt
    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_TRG) | 29 );
    // need to wait more than 15 microsecond before the DC_OK is valid
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));
    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_BO_OFFSET) |
                         (4 << BP_POWER_VDDDCTRL_BO_OFFSET));

    // Restore VDDA 1.800 volt
    HW_POWER_VDDACTRL_WR((HW_POWER_VDDACTRL_RD() & ~BM_POWER_VDDACTRL_TRG)|12);
    // need to wait more than 15 microsecond before the DC_OK is valid
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));
    HW_POWER_VDDACTRL_WR((HW_POWER_VDDACTRL_RD() & ~BM_POWER_VDDACTRL_BO_OFFSET) |
                        (4 << BP_POWER_VDDACTRL_BO_OFFSET));

    // Restore the VDDIO 3.30 volt
    HW_POWER_VDDIOCTRL_WR((HW_POWER_VDDIOCTRL_RD() & ~BM_POWER_VDDIOCTRL_TRG)|20);
    // need to wait more than 15 microsecond before the DC_OK is valid
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));
    HW_POWER_VDDIOCTRL_WR((HW_POWER_VDDIOCTRL_RD() & ~BM_POWER_VDDIOCTRL_BO_OFFSET) |
                          (4 << BP_POWER_VDDIOCTRL_BO_OFFSET));

    
    //Ungating EMI CLock 
    HW_CLKCTRL_EMI_CLR(BM_CLKCTRL_EMI_CLKGATE);
    
    //LeaveSelfrefreshMode
    //HW_DRAM_CTL16_CLR(BF_DRAM_CTL16_LOWPOWER_CONTROL(2));
    HW_DRAM_CTL16_CLR(1<<17);
    HW_DRAM_CTL16_CLR(1<<9);    

}


//------------------------------------------------------------------------------
//
// Function:     SuspendMe
//
// Description:  
//
static VOID SuspendMe(VOID)
{
    FUNC_POWER_CONTROL pFuncTotalSuspend;
    UINT32 IRAMSize;
    pFuncTotalSuspend=pControlFunc;
    IRAMSize = GetPowerOffIRAMSize();
  
    memcpy((void *) pFuncTotalSuspend, (void *) SuspendTheSystem, IRAMSize);

    //--------------------------------------------------------------------------     
    //Shut down stuff
    //--------------------------------------------------------------------------   
    // gate the UART clock, We wont be gating any DEBUGMSG/RETAILMSG 
    HW_CLKCTRL_XTAL_SET ( BM_CLKCTRL_XTAL_UART_CLK_GATE);

    // gate timrot block clock
    HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);

    HW_BCH_CTRL_SET(BM_BCH_CTRL_CLKGATE);
    HW_DIGCTL_CTRL_SET(BM_DIGCTL_CTRL_USB_CLKGATE);

    // run the suspend code from the OCRAM
    // Put the CPU to lowest power mode (Wait for interrupt )
    pFuncTotalSuspend();

    //Enable XTAL 24MHz
    //HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_PWD_XTAL24);

    HW_BCH_CTRL_CLR(BM_BCH_CTRL_CLKGATE);
    HW_DIGCTL_CTRL_CLR(BM_DIGCTL_CTRL_USB_CLKGATE);

    HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);
    
    // ungate the UART clock, We will be able to use RETAILMSG,DEBUGMSG now
    HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_UART_CLK_GATE);

}


//-----------------------------------------------------------------------------
//
// Function: SOCPowerOffReg
//
// Called from OEMPowerOff when the system is to transition to it's lowest power
// mode.  This function stores off some of the important registers and turn off 
// clkgate where ever possible
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
static void SOCPowerOffReg()
{
    //OALMSG(1, (L"SOCPowerOffReg \r\n"));

    // Backup the clock control register for following peripherals
    SocBackupReg.CLKCTRL_SSP       = HW_CLKCTRL_SSP_RD();
    SocBackupReg.CLKCTRL_GPMI      = HW_CLKCTRL_GPMI_RD();
    SocBackupReg.CLKCTRL_IR        = HW_CLKCTRL_IR_RD();
    SocBackupReg.CLKCTRL_PLLCTRL0 = HW_CLKCTRL_PLLCTRL0_RD();
    SocBackupReg.CLKCTRL_HBUS     = HW_CLKCTRL_HBUS_RD();

    // Backup General Purpose Media Interface registers
    SocBackupReg.GPMI_CTRL0    = HW_GPMI_CTRL0_RD();
    SocBackupReg.GPMI_CTRL1    = HW_GPMI_CTRL1_RD();
    SocBackupReg.GPMI_TIMING0  = HW_GPMI_TIMING0_RD();
    SocBackupReg.GPMI_TIMING1  = HW_GPMI_TIMING1_RD();

    // Backup APBH interface
    SocBackupReg.APBH_CTRL0    = HW_APBH_CTRL0_RD();
    SocBackupReg.APBH_CTRL1    = HW_APBH_CTRL1_RD();
    SocBackupReg.APBH_CTRL2    = HW_APBH_CTRL2_RD();

    // Backup APBX interface
    SocBackupReg.APBX_CTRL0    = HW_APBX_CTRL0_RD();
    SocBackupReg.APBX_CTRL1    = HW_APBX_CTRL1_RD();
    SocBackupReg.APBX_CTRL2    = HW_APBX_CTRL2_RD();

   // Backup PWM  
   SocBackupReg.PWM_CTRL    = HW_PWM_CTRL_RD();   
   
    // Power down APBH blcok
    HW_APBH_CTRL1_CLR(0xffffffff);
    HW_APBH_CTRL2_CLR(0xffffffff);
    HW_APBH_CTRL0_SET(BM_APBH_CTRL0_SFTRST);
    HW_APBH_CTRL0_SET(BM_APBH_CTRL0_CLKGATE);

    // POWER down APBX block
    HW_APBX_CTRL1_CLR(0xffffffff);
    HW_APBX_CTRL2_CLR(0xffffffff);
    HW_APBX_CTRL0_SET(BM_APBX_CTRL0_SFTRST);
    HW_APBX_CTRL0_SET(BM_APBX_CTRL0_CLKGATE);
   
    //Backup CLKSEQ
    SocBackupReg.CLKCTRL_CLKSEQ = HW_CLKCTRL_CLKSEQ_RD();

    HW_CLKCTRL_CLKSEQ_SET(BP_CLKCTRL_CLKSEQ_BYPASS_ETM |
                          BP_CLKCTRL_CLKSEQ_BYPASS_SSP |
                          BP_CLKCTRL_CLKSEQ_BYPASS_GPMI |
                          BP_CLKCTRL_CLKSEQ_BYPASS_IR |
                          BP_CLKCTRL_CLKSEQ_BYPASS_PIX|
                          BP_CLKCTRL_CLKSEQ_BYPASS_SAIF);

    HW_CLKCTRL_SSP_SET(BM_CLKCTRL_SSP_CLKGATE);
    HW_CLKCTRL_GPMI_SET(BM_CLKCTRL_GPMI_CLKGATE);
    HW_CLKCTRL_IR_SET(BM_CLKCTRL_IR_CLKGATE);


    //Gating IO, PIX, VID Clock
    SocBackupReg.CLKCTRL_FRAC = HW_CLKCTRL_FRAC_RD();
    SocBackupReg.CLKCTRL_FRAC1 = HW_CLKCTRL_FRAC1_RD();
    HW_CLKCTRL_FRAC_SET(BM_CLKCTRL_FRAC_CLKGATEIO | BM_CLKCTRL_FRAC_CLKGATEPIX );
    HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEVID );
    
    //Backup GPMI control register
    HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_SFTRST);
    HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_CLKGATE);

    // Backup XTAL clock control register
    SocBackupReg.CLKCTRL_XTAL   = HW_CLKCTRL_XTAL_RD();

    HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_FILT_CLK24M_GATE |
                        BM_CLKCTRL_XTAL_PWM_CLK24M_GATE  |
                        BM_CLKCTRL_XTAL_DRI_CLK24M_GATE);

    //Backup RTC control register state
    SocBackupReg.RTC_CTRL = INREG32(&pRtcReg->CTRL[BASEREG]);
    // Disable the watchdog
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_WATCHDOGEN); 

}
//-----------------------------------------------------------------------------
//
// Function: SOCPowerOnReg
//
// Called from OEMPowerOff when the system is to transition to it's resume mode
// 
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
static void SOCPowerOnReg()
{
    OALMSG(0, (L"SOCPowerOnReg \r\n"));

    HW_CLKCTRL_PLLCTRL0_WR(SocBackupReg.CLKCTRL_PLLCTRL0);

    //restore Watchdog state to Enable
    if((SocBackupReg.RTC_CTRL & BM_RTC_CTRL_WATCHDOGEN) != 0)
        OUTREG32(&pRtcReg->CTRL[SETREG],BM_RTC_CTRL_WATCHDOGEN);

    //Restore XTAL clock control register
    HW_CLKCTRL_XTAL_WR(SocBackupReg.CLKCTRL_XTAL);

    //Restore IO, PIX, VID gating state
    HW_CLKCTRL_FRAC_WR(SocBackupReg.CLKCTRL_FRAC);
    HW_CLKCTRL_FRAC1_WR(SocBackupReg.CLKCTRL_FRAC1);
    //HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEVID );

    HW_CLKCTRL_GPMI_WR(SocBackupReg.CLKCTRL_GPMI);
    HW_CLKCTRL_SSP_WR(SocBackupReg.CLKCTRL_SSP);
    HW_CLKCTRL_IR_WR(SocBackupReg.CLKCTRL_IR);

    HW_CLKCTRL_CLKSEQ_WR(SocBackupReg.CLKCTRL_CLKSEQ);


    //restore APBH state
    HW_APBH_CTRL0_CLR(BM_APBH_CTRL0_SFTRST);
    for(; (HW_APBH_CTRL0_RD() & BM_APBH_CTRL0_SFTRST) != 0;);

    HW_APBH_CTRL0_CLR(BM_APBH_CTRL0_CLKGATE);
    for(; (HW_APBH_CTRL0_RD() & BM_APBH_CTRL0_CLKGATE) != 0;);

    HW_APBH_CTRL0_WR(SocBackupReg.APBH_CTRL0);
    HW_APBH_CTRL1_WR(SocBackupReg.APBH_CTRL1);
    HW_APBH_CTRL2_WR(SocBackupReg.APBH_CTRL2);

    //restore APBX state
    HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_SFTRST);
    for(; (HW_APBX_CTRL0_RD() & BM_APBX_CTRL0_SFTRST) != 0;);

    HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_CLKGATE);
    for(; (HW_APBX_CTRL0_RD() & BM_APBX_CTRL0_CLKGATE) != 0;);

    HW_APBX_CTRL0_WR(SocBackupReg.APBX_CTRL0);
    HW_APBX_CTRL1_WR(SocBackupReg.APBX_CTRL1);
    HW_APBX_CTRL2_WR(SocBackupReg.APBX_CTRL2);

    //
    HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
    for(; (HW_GPMI_CTRL0_RD() & BM_GPMI_CTRL0_SFTRST) != 0;);

    HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);
    for(; (HW_GPMI_CTRL0_RD() & BM_GPMI_CTRL0_CLKGATE) != 0;);

    HW_GPMI_CTRL0_WR(SocBackupReg.GPMI_CTRL0 & ~BM_GPMI_CTRL0_RUN);
    HW_GPMI_CTRL1_WR(SocBackupReg.GPMI_CTRL1);
    HW_GPMI_TIMING0_WR(SocBackupReg.GPMI_TIMING0);
    HW_GPMI_TIMING1_WR(SocBackupReg.GPMI_TIMING1);
    
    OALStall(10000);

}
//------------------------------------------------------------------------------
//
// Function:     SOCPowerOffCPURate
//
// Description:  
//
static VOID SOCPowerOffCPURate(VOID)
{
    //
    // let CPU sink the xtal clock
    //
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
  
    for(; (HW_CLKCTRL_CPU_RD() & BM_CLKCTRL_CPU_BUSY_REF_XTAL) != 0;);
    OALStall(100);
  
    HW_CLKCTRL_FRAC_SET(BM_CLKCTRL_FRAC_CLKGATECPU);

    // config CLK_CPU driver run in lowest frequency
    HW_CLKCTRL_CPU_WR( BF_CLKCTRL_CPU_DIV_CPU(1)         | 
                       BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)    | 
                       BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)     | 
                       BF_CLKCTRL_CPU_DIV_XTAL(1023)           | 
                       BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0));


    // config CLK_HBUS for lowest frequency
    HW_CLKCTRL_HBUS_WR(BF_CLKCTRL_HBUS_DIV(1)                 | 
                        BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)         | 
                        BF_CLKCTRL_HBUS_SLOW_DIV(5)            | 
                        BF_CLKCTRL_HBUS_AUTO_SLOW_MODE(1)      | 
                        BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(1) | 
                        BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(1)  | 
                        BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(1)   | 
                        BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(1) | 
                        BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(1)     | 
                        BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(1)    |
                        BF_CLKCTRL_HBUS_PXP_AS_ENABLE(1)         |
                        BF_CLKCTRL_HBUS_DCP_AS_ENABLE(1));

    // XBUS
    BF_CLR(CLKCTRL_XBUS,DIV_FRAC_EN);
  
    //Set the divider
    HW_CLKCTRL_XBUS.B.DIV = 64;
    OALStall(100);
}
//------------------------------------------------------------------------------
//
// Function:     SOCPowerOnCPURate
//
// Description:  
//
static VOID SOCPowerOnCPURate(VOID)
{
    DWORD RegFRAC;
    UINT32 x;

    //OALMSG(1, (L"SOCPowerOnCPURate++ \r\n"));

    //
    // let CPU sink the xtal clock
    //
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    //
    // Turn on PLL
    //
    HW_CLKCTRL_PLLCTRL0_SET(BM_CLKCTRL_PLLCTRL0_POWER);
    //for(x=0; x++ != 0x1000; ) ;
    OALStall(10);

    // CPU running in High setpoint
    HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_CLKGATECPU);
    HW_CLKCTRL_FRAC_WR((HW_CLKCTRL_FRAC_RD() & ~BM_CLKCTRL_FRAC_CPUFRAC) |\
                        BF_CLKCTRL_FRAC_CPUFRAC(19));

    RegFRAC = HW_CLKCTRL_FRAC_RD() & BM_CLKCTRL_FRAC_CPU_STABLE;
    for(; (HW_CLKCTRL_FRAC_RD() ^ RegFRAC) == 0; ) ;

    // config CLK_CPU driver for High setpoint
    HW_CLKCTRL_CPU_WR((BF_CLKCTRL_CPU_DIV_CPU(1)            |
                       BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)    |
                       BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)     | 
                       BF_CLKCTRL_CPU_DIV_XTAL(1)           | 
                       BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0)));


    // config CLK_HBUSfor High setpoint
    HW_CLKCTRL_HBUS_WR((BF_CLKCTRL_HBUS_DIV(3)                 | 
                        BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)         | 
                        BF_CLKCTRL_HBUS_SLOW_DIV(0)            | 
                        BF_CLKCTRL_HBUS_AUTO_SLOW_MODE(0)      | 
                        BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(0) | 
                        BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(0)  | 
                        BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(0)   | 
                        BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(0) | 
                        BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(0)     | 
                        BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(0))    |
                        BF_CLKCTRL_HBUS_PXP_AS_ENABLE(0)         |
                        BF_CLKCTRL_HBUS_DCP_AS_ENABLE(0));

    // XBUS
    BF_CLR(CLKCTRL_XBUS,DIV_FRAC_EN);

    //Set the divider
    HW_CLKCTRL_XBUS.B.DIV = 1;
    OALStall(100);

    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
    for(x=0; x++ != 0x1000; ) ;

    //OALMSG(0, (L"SOCPowerOnCPURate-- \r\n"));
}

