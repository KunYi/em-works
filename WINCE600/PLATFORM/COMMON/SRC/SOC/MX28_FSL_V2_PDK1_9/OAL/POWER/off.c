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
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
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
#include "mx28_timrot.h"
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
extern PVOID pv_HWregUSBPhy0;
extern PVOID pv_HWregUSBPhy1;
extern PVOID pv_HWRegPWM;
extern PVOID pv_HWregBCH;
//extern PVOID pv_HWregLRADC;
extern PVOID pv_HWregLCDIF;
extern PVOID pv_HWregDRAM;
extern PVOID pv_HWregRTC;

extern FUNC_POWER_CONTROL pControlFunc;

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
typedef struct _SOC_POWEROFF_BACKUP
{
    volatile DWORD CLKCTRL_PLL0CTRL0;
    volatile DWORD CLKCTRL_PLL1CTRL0;
    volatile DWORD CLKCTRL_PLL2CTRL0;    
    volatile DWORD CLKCTRL_XTAL;    
    volatile DWORD CLKCTRL_SSP0;
    volatile DWORD CLKCTRL_SSP1;
    volatile DWORD CLKCTRL_SSP2;
    volatile DWORD CLKCTRL_SSP3;    
    volatile DWORD CLKCTRL_GPMI;
    volatile DWORD CLKCTRL_SPDIF;
    volatile DWORD CLKCTRL_SAIF0;
    volatile DWORD CLKCTRL_SAIF1;
    volatile DWORD CLKCTRL_DIS_LCDIF;    
    volatile DWORD CLKCTRL_ETM;
    volatile DWORD CLKCTRL_ENET;    
    volatile DWORD CLKCTRL_HSADC;
    volatile DWORD CLKCTRL_FLEXCAN;    
    volatile DWORD CLKCTRL_FRAC0;    
    volatile DWORD CLKCTRL_FRAC1;
    volatile DWORD CLKCTRL_CLKSEQ;

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
    volatile DWORD BCH_CTRL;    
    volatile DWORD USBPHY0_CTRL;
    volatile DWORD USBPHY0_PWD;
    volatile DWORD USBPHY1_CTRL;
    volatile DWORD USBPHY1_PWD;    
    volatile DWORD DIGCTL_CTRL; 
    volatile DWORD CLKCTRL_HBUS; 
    volatile DWORD CLKCTRL_XBUS;
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
static void SuspendTheSystem(VOID);

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

    UINT32 registerStore[IRQ_GPIO0 + 1 + DDK_GPIO_BANK4 + 1];
    UINT32 sysIntr, irq;
    UINT32 line, powersts;
    BOOL PowerState = FALSE;

    OALMSG(OAL_FUNC,(_T("OEMPowerOff!!!\r\n")));  
 
    // Call board-level power off function
    BSPPowerOff();

    // Leave suspend if 5v power supply 
    powersts = HW_POWER_STS_RD();
    if((powersts & BM_POWER_STS_VBUSVALID0) != 0) 
    {
        return;
    }
    
    // Switch off power for KITL device
    KITLIoctl(IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);
    PowerState = TRUE;
    

    // Reset the wake source global
    g_oalWakeSource = SYSWAKE_UNKNOWN;

    //Save state of GPIO interrupt
    registerStore[IRQ_GPIO0 + 1]= HW_PINCTRL_IRQEN0_RD();
    registerStore[IRQ_GPIO0 + 2]= HW_PINCTRL_IRQEN1_RD();
    registerStore[IRQ_GPIO0 + 3]= HW_PINCTRL_IRQEN2_RD();
    registerStore[IRQ_GPIO0 + 4]= HW_PINCTRL_IRQEN3_RD();
    registerStore[IRQ_GPIO0 + 5]= HW_PINCTRL_IRQEN4_RD();

    //Mask all interrupt source
    HW_PINCTRL_IRQEN0_CLR(0xFFFFFFFF);
    HW_PINCTRL_IRQEN1_CLR(0xFFFFFFFF);    
    HW_PINCTRL_IRQEN2_CLR(0xFFFFFFFF);
    HW_PINCTRL_IRQEN3_CLR(0xFFFFFFFF);
    HW_PINCTRL_IRQEN4_CLR(0xFFFFFFFF);
    
    // Save state of enabled interrupts and Mask interrupt
    for(irq = IRQ_BATT_BROWNOUT; irq <= IRQ_GPIO0; irq++ )
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
        if (!OALPowerWakeSource(sysIntr)) 
        {
            continue;
        }
        // Enable it as wakeup source interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);

        // Set flag to indicate we have at least one wake source
        PowerState = FALSE;
    }
/*
    for(irq = IRQ_BATT_BROWNOUT; irq <= IRQ_GPIO0; irq++ )
    {
        if( HW_ICOLL_INTERRUPTn_RD(irq) != 0)    
        OALMSG(1,(_T("Wakeup source =%d\r\n"), irq));    
    }
*/
    if (!PowerState)
    {
        
        // Place the system in suspend state and wait for an interrupt.
        OALMSG(1, (_T("INFO: OEMPowerOff entering suspend.\r\n")));

        // Decrease ARM/AHB rate 
        SOCPowerOffCPURate();

		//OALMSG(1, (_T("INFO: entering suspend-->\r\n")));
        //Suspend 
        SuspendMe();
        OALMSG(1, (_T("INFO: OEMPowerOff wakeUp...\r\n")));
        // Increase ARM/AHB rate 
        SOCPowerOnCPURate();
		
        // Get interrupt source
        irq = HW_ICOLL_STAT_RD() & BM_ICOLL_STAT_VECTOR_NUMBER;

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
        
        // GPIO3 special case
        else if(irq == IRQ_GPIO3)
        { 
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(HW_PINCTRL_IRQSTAT3_RD() & HW_PINCTRL_IRQEN3_RD());

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO3_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }
        
        // GPIO4 special case
        else if(irq == IRQ_GPIO4)
        { 
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(HW_PINCTRL_IRQSTAT4_RD() & HW_PINCTRL_IRQEN4_RD());

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO4_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }

        // Chance of board-level subordinate interrupt controller
        irq = BSPIntrActiveIrq(irq);
        
        OALMSG(1, (_T("INFO: OEMPowerOff leaving suspend.  IRQ = %d\r\n"), irq));

        if(irq == IRQ_VDD5V)
        {
            HW_POWER_CTRL_CLR(BM_POWER_CTRL_PSWITCH_IRQ);
        }
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
    for(irq = IRQ_BATT_BROWNOUT; irq <= IRQ_GPIO0; irq++ )
    {
        HW_ICOLL_INTERRUPTn_CLR(irq, 0xFFFFFFFF);
        HW_ICOLL_INTERRUPTn_SET(irq, registerStore[irq]);
    }

    HW_PINCTRL_IRQEN0_SET(registerStore[IRQ_GPIO0 + 1]);
    HW_PINCTRL_IRQEN1_SET(registerStore[IRQ_GPIO0 + 2]);
    HW_PINCTRL_IRQEN2_SET(registerStore[IRQ_GPIO0 + 3]);
    HW_PINCTRL_IRQEN3_SET(registerStore[IRQ_GPIO0 + 4]);
    HW_PINCTRL_IRQEN4_SET(registerStore[IRQ_GPIO0 + 5]);

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
//------------------------------------------------------------------------------
VOID SuspendTheSystem(VOID)
{

    DWORD Reg_Data;
    DWORD Reg_VDDD, Reg_VDDIO, Reg_VDDA;
    
    //DDR Enter Low PowerMode
    //Bit [4] = Controls memory power-down mode (Mode 1).
    //Bit [3] = Controls memory power-down with memory clock gating mode (Mode 2).
    //Bit [2] = Controls memory self-refresh mode (Mode 3).
    //Bit [1] = Controls memory self-refresh with memory clock gating mode (Mode 4).
    //Bit [0] = Controls memory self-refresh with memory and controller clock gating mode (Mode 5).
    //Set DDR2 enter mode 4
    HW_DRAM_CTL22_SET(BF_DRAM_CTL22_LOWPOWER_CONTROL(2));

    //Gating EMI CLock 
    HW_CLKCTRL_EMI_SET(BM_CLKCTRL_EMI_CLKGATE);

    //Change PAD type to GPIO to save power
    HW_PINCTRL_EMI_DS_CTRL.B.DDR_MODE = 1;
    for(Reg_Data = 0; Reg_Data < 20; Reg_Data++)
    {
        Reg_Data = Reg_Data;
    }
    //--------------------------------------------------------------------------
    // Voltage rails
    //--------------------------------------------------------------------------
      
    // Reduce the VDDIO (3.050 volt)
    Reg_VDDIO = HW_POWER_VDDIOCTRL.B.TRG;
    HW_POWER_VDDIOCTRL_WR((HW_POWER_VDDIOCTRL_RD() & ~BM_POWER_VDDIOCTRL_TRG) | VDDIOVolt2Reg(3050));
    // need to wait more than 15 microsecond before the DC_OK is valid  
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 
    
    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    // Reduce VDDA 1.725volt
    Reg_VDDA = HW_POWER_VDDACTRL.B.TRG;
    HW_POWER_VDDACTRL_WR((HW_POWER_VDDACTRL_RD() & ~BM_POWER_VDDACTRL_TRG) | VDDAVolt2Reg(1725));
    // need to wait more than 15 microsecond before the DC_OK is valid
    
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    // Reduce VDDD 1.100 volt
    Reg_VDDD = HW_POWER_VDDDCTRL.B.TRG;    
    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_TRG) | VDDDVolt2Reg(1100));

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
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_HALF_FETS);
    
    HW_POWER_LOOPCTRL_CLR(BM_POWER_LOOPCTRL_CM_HYST_THRESH);   
    HW_POWER_LOOPCTRL_CLR(BM_POWER_LOOPCTRL_EN_CM_HYST);
    HW_POWER_LOOPCTRL_CLR(BM_POWER_LOOPCTRL_EN_DF_HYST);
    // enable PFM
    HW_POWER_LOOPCTRL_SET(BM_POWER_LOOPCTRL_HYST_SIGN);
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_EN_DC_PFM);

    //POWER: Good for about 40uA.
    BF_SET(POWER_MINPWR, LESSANA_I);
    
    BF_SET(POWER_5VCTRL, ILIMIT_EQ_ZERO);   

    //Gated PLL0
    HW_CLKCTRL_PLL0CTRL0_CLR(BM_CLKCTRL_PLL0CTRL0_POWER);

    // Power off ...
    _MoveToCoprocessor(0, 15, 0, 7, 0, 4);

    //Enable PLL
    HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_POWER);
   
    //--------------------------------------------------------------------------
    // restore the DCDC parameter
    //--------------------------------------------------------------------------     

    BF_CLR(POWER_5VCTRL, ILIMIT_EQ_ZERO);
    BF_CLR(POWER_MINPWR, LESSANA_I);

    HW_POWER_MINPWR_CLR (BM_POWER_MINPWR_EN_DC_PFM);
    HW_POWER_LOOPCTRL_CLR(BM_POWER_LOOPCTRL_HYST_SIGN);
    HW_POWER_LOOPCTRL_SET(BM_POWER_LOOPCTRL_EN_DF_HYST);
    HW_POWER_LOOPCTRL_SET(BM_POWER_LOOPCTRL_EN_CM_HYST);
    HW_POWER_LOOPCTRL_SET(BM_POWER_LOOPCTRL_CM_HYST_THRESH);

    // Half fet is already set?
    HW_POWER_LOOPCTRL_WR((HW_POWER_LOOPCTRL_RD() & ~BM_POWER_LOOPCTRL_DC_R) |
                         (2<<BP_POWER_LOOPCTRL_DC_R));
    HW_POWER_LOOPCTRL_WR((HW_POWER_LOOPCTRL_RD() & ~BM_POWER_LOOPCTRL_EN_RCSCALE) |
                        (3 << BP_POWER_LOOPCTRL_EN_RCSCALE));

    //--------------------------------------------------------------------------
    // Voltage rails
    //--------------------------------------------------------------------------
 
    // Restore VDDD  volt
    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_TRG) | Reg_VDDD);
    // need to wait more than 15 microsecond before the DC_OK is valid
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    // Restore VDDA 1.800 volt
    HW_POWER_VDDACTRL_WR((HW_POWER_VDDACTRL_RD() & ~BM_POWER_VDDACTRL_TRG) | Reg_VDDA);
    // need to wait more than 15 microsecond before the DC_OK is valid
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    // Restore the VDDIO 3.55 volt
    HW_POWER_VDDIOCTRL_WR((HW_POWER_VDDIOCTRL_RD() & ~BM_POWER_VDDIOCTRL_TRG) | Reg_VDDIO);
    // need to wait more than 15 microsecond before the DC_OK is valid
    Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
    for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 15;); 

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    //Change PAD type to DDR2
    HW_PINCTRL_EMI_DS_CTRL.B.DDR_MODE = 3;
    for(Reg_Data = 0; Reg_Data < 20; Reg_Data++)
    {
        Reg_Data = Reg_Data;
    }

    //Ungating EMI CLock 
    HW_CLKCTRL_EMI_CLR(BM_CLKCTRL_EMI_CLKGATE);
    
    //Leave DDR Lower Power Mode
    HW_DRAM_CTL22_CLR(BF_DRAM_CTL22_LOWPOWER_CONTROL(2)); 

}


//------------------------------------------------------------------------------
//
// Function:     SuspendMe
//
// Description:  
//------------------------------------------------------------------------------
static VOID SuspendMe(VOID)
{
    FUNC_POWER_CONTROL pFuncTotalSuspend;
    UINT32 IRAMSize;
    pFuncTotalSuspend=pControlFunc;
    IRAMSize = GetPowerOffIRAMSize();

    memcpy((void *) pFuncTotalSuspend, (void *) SuspendTheSystem, IRAMSize);
  

    //Switch 24MHz clock to oscillator
    //HW_POWER_MINPWR_SET(BM_POWER_MINPWR_ENABLE_OSC);
    //OALstall(100);
    //HW_POWER_MINPWR_SET(BM_POWER_MINPWR_SELECT_OSC);
    //Disable XTAL 24MHz
    //HW_POWER_MINPWR_SET(BM_POWER_MINPWR_PWD_XTAL24);

    // run the suspend code from the OCRAM
    // Put the CPU to lowest power mode (Wait for interrupt )
    pFuncTotalSuspend();

    //Enable XTAL 24MHz
    //HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_PWD_XTAL24);

    //Switch 24MHz clock back to oscillator
    //HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_SELECT_OSC);    
    //HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_ENABLE_OSC);
    

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
/*
    // Backup PWM  
    SocBackupReg.PWM_CTRL           = HW_PWM_CTRL_RD();  
    HW_PWM_CTRL_SET(BM_PWM_CTRL_SFTRST);    
    HW_PWM_CTRL_SET(BM_PWM_CTRL_CLKGATE);

    // Backup GPMI registers
    SocBackupReg.GPMI_CTRL0        = HW_GPMI_CTRL0_RD();
    SocBackupReg.GPMI_CTRL1        = HW_GPMI_CTRL1_RD();
    SocBackupReg.GPMI_TIMING0      = HW_GPMI_TIMING0_RD();
    SocBackupReg.GPMI_TIMING1      = HW_GPMI_TIMING1_RD();
    //Gating GPMI
    HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_SFTRST);
    HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_CLKGATE);    

    //Backup RTC control register state
    SocBackupReg.RTC_CTRL = HW_RTC_CTRL_RD();
    // Disable the watchdog
    HW_RTC_CTRL_CLR(BM_RTC_CTRL_WATCHDOGEN);     

    //Power Down BCH
    SocBackupReg.APBX_CTRL2  = HW_BCH_CTRL_RD();  
    HW_BCH_CTRL_SET(BM_BCH_CTRL_CLKGATE);
   
    // Backup APBH interface
    SocBackupReg.APBH_CTRL0        = HW_APBH_CTRL0_RD();
    SocBackupReg.APBH_CTRL1        = HW_APBH_CTRL1_RD();
    SocBackupReg.APBH_CTRL2        = HW_APBH_CTRL2_RD();

    // Backup APBX interface
    SocBackupReg.APBX_CTRL0        = HW_APBX_CTRL0_RD();
    SocBackupReg.APBX_CTRL1        = HW_APBX_CTRL1_RD();
    SocBackupReg.APBX_CTRL2        = HW_APBX_CTRL2_RD();

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

    
    //--------------------------------------------------------------------------      
    //power down all USB modules 
    //--------------------------------------------------------------------------

    SocBackupReg.DIGCTL_CTRL = HW_DIGCTL_CTRL_RD();
    HW_DIGCTL_CTRL_CLR(BM_DIGCTL_CTRL_USB0_CLKGATE); 
    HW_DIGCTL_CTRL_CLR(BM_DIGCTL_CTRL_USB1_CLKGATE); 

    //USB0
    SocBackupReg.USBPHY0_CTRL = HW_USBPHY_CTRL_RD(0);
    SocBackupReg.USBPHY0_PWD = HW_USBPHY_PWD_RD(0);

    HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_SFTRST);
    HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_CLKGATE);   

    //Put down USB0
    HW_USBPHY_PWD_SET(0,BM_USBPHY_PWD_RXPWDRX | 
                      BM_USBPHY_PWD_RXPWDDIFF | 
                      BM_USBPHY_PWD_RXPWD1PT1 | 
                      BM_USBPHY_PWD_RXPWDENV | 
                      BM_USBPHY_PWD_TXPWDV2I | 
                      BM_USBPHY_PWD_TXPWDIBIAS | 
                      BM_USBPHY_PWD_TXPWDFS);
    
    HW_DIGCTL_CTRL_SET(BM_DIGCTL_CTRL_USB0_CLKGATE);        
    
    HW_USBPHY_CTRL_SET(0,BM_USBPHY_CTRL_SFTRST);
    HW_USBPHY_CTRL_SET(0,BM_USBPHY_CTRL_CLKGATE); 

    //USB1
    SocBackupReg.USBPHY1_CTRL = HW_USBPHY_CTRL_RD(1);
    SocBackupReg.USBPHY1_PWD = HW_USBPHY_PWD_RD(1);

    HW_USBPHY_CTRL_CLR(1,BM_USBPHY_CTRL_SFTRST);
    HW_USBPHY_CTRL_CLR(1,BM_USBPHY_CTRL_CLKGATE);   

    //Put down USB1
    HW_USBPHY_PWD_SET(1,BM_USBPHY_PWD_RXPWDRX | 
                      BM_USBPHY_PWD_RXPWDDIFF | 
                      BM_USBPHY_PWD_RXPWD1PT1 | 
                      BM_USBPHY_PWD_RXPWDENV | 
                      BM_USBPHY_PWD_TXPWDV2I | 
                      BM_USBPHY_PWD_TXPWDIBIAS | 
                      BM_USBPHY_PWD_TXPWDFS);
    
    HW_DIGCTL_CTRL_SET(BM_DIGCTL_CTRL_USB1_CLKGATE);        
    
    HW_USBPHY_CTRL_SET(1,BM_USBPHY_CTRL_SFTRST);
    HW_USBPHY_CTRL_SET(1,BM_USBPHY_CTRL_CLKGATE); 


    // Backup the clock control register for following peripherals
    SocBackupReg.CLKCTRL_SSP0       = HW_CLKCTRL_SSP0_RD();
    SocBackupReg.CLKCTRL_SSP1       = HW_CLKCTRL_SSP1_RD();
    SocBackupReg.CLKCTRL_SSP2       = HW_CLKCTRL_SSP2_RD();
    SocBackupReg.CLKCTRL_SSP3       = HW_CLKCTRL_SSP3_RD();
    SocBackupReg.CLKCTRL_GPMI       = HW_CLKCTRL_GPMI_RD();
    SocBackupReg.CLKCTRL_SPDIF      = HW_CLKCTRL_SPDIF_RD();
    SocBackupReg.CLKCTRL_SAIF0      = HW_CLKCTRL_SAIF0_RD();
    SocBackupReg.CLKCTRL_SAIF1      = HW_CLKCTRL_SAIF1_RD();
    SocBackupReg.CLKCTRL_DIS_LCDIF  = HW_CLKCTRL_DIS_LCDIF_RD();
    SocBackupReg.CLKCTRL_ETM        = HW_CLKCTRL_ETM_RD();
    SocBackupReg.CLKCTRL_ENET       = HW_CLKCTRL_ENET_RD();
    SocBackupReg.CLKCTRL_FLEXCAN    = HW_CLKCTRL_FLEXCAN_RD();
    SocBackupReg.CLKCTRL_XTAL       = HW_CLKCTRL_XTAL_RD();

    //Gated Clock
    HW_CLKCTRL_SSP0_SET(BM_CLKCTRL_SSP0_CLKGATE);
    HW_CLKCTRL_SSP1_SET(BM_CLKCTRL_SSP1_CLKGATE);
    HW_CLKCTRL_SSP2_SET(BM_CLKCTRL_SSP2_CLKGATE);
    HW_CLKCTRL_SSP3_SET(BM_CLKCTRL_SSP3_CLKGATE);    
    HW_CLKCTRL_GPMI_SET(BM_CLKCTRL_GPMI_CLKGATE);
    HW_CLKCTRL_SPDIF_SET(BM_CLKCTRL_SPDIF_CLKGATE);
    HW_CLKCTRL_SAIF0_SET(BM_CLKCTRL_SAIF0_CLKGATE);
    HW_CLKCTRL_SAIF1_SET(BM_CLKCTRL_SAIF1_CLKGATE);
    HW_CLKCTRL_DIS_LCDIF_SET(BM_CLKCTRL_DIS_LCDIF_CLKGATE);    
    HW_CLKCTRL_ETM_SET(BM_CLKCTRL_ETM_CLKGATE);    
    HW_CLKCTRL_ENET_SET(BM_CLKCTRL_ENET_DISABLE);    
    HW_CLKCTRL_FLEXCAN_SET(BM_CLKCTRL_FLEXCAN_STOP_CAN0 |
                           BM_CLKCTRL_FLEXCAN_STOP_CAN0 );    

    HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);    
    HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_UART_CLK_GATE);
    HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);


    //Backup CLKSEQ
    SocBackupReg.CLKCTRL_CLKSEQ = HW_CLKCTRL_CLKSEQ_RD();

    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF |
                          BM_CLKCTRL_CLKSEQ_BYPASS_ETM |
                          BM_CLKCTRL_CLKSEQ_BYPASS_SSP0 |
                          BM_CLKCTRL_CLKSEQ_BYPASS_SSP1 |
                          BM_CLKCTRL_CLKSEQ_BYPASS_SSP2 |
                          BM_CLKCTRL_CLKSEQ_BYPASS_SSP3 |
                          BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);

    //Gating IO0, IO1, PIX, GPMI, HSADC Clock
    SocBackupReg.CLKCTRL_FRAC0  = HW_CLKCTRL_FRAC0_RD();
    SocBackupReg.CLKCTRL_FRAC1 = HW_CLKCTRL_FRAC1_RD();
    
    HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATEIO0 | 
                         BM_CLKCTRL_FRAC0_CLKGATEIO1);
    HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEPIX | 
                         BM_CLKCTRL_FRAC1_CLKGATEGPMI | 
                         BM_CLKCTRL_FRAC1_CLKGATEHSADC);
*/
    //Backup  PLLx Register
    SocBackupReg.CLKCTRL_PLL0CTRL0  = HW_CLKCTRL_PLL0CTRL0_RD();
    SocBackupReg.CLKCTRL_PLL1CTRL0  = HW_CLKCTRL_PLL1CTRL0_RD();    
    SocBackupReg.CLKCTRL_PLL2CTRL0  = HW_CLKCTRL_PLL2CTRL0_RD();

    //Disable USB0 Clock
    HW_CLKCTRL_PLL0CTRL0_CLR(BM_CLKCTRL_PLL0CTRL0_EN_USB_CLKS);
    //Disbale USB1 and PLL1 Clock
    HW_CLKCTRL_PLL1CTRL0_CLR(BM_CLKCTRL_PLL1CTRL0_EN_USB_CLKS |
                             BM_CLKCTRL_PLL1CTRL0_POWER);    
    //Disable ENET PLL and PLL2 Clock
    HW_CLKCTRL_PLL2CTRL0_SET(BM_CLKCTRL_PLL2CTRL0_CLKGATE);
    HW_CLKCTRL_PLL2CTRL0_CLR(BM_CLKCTRL_PLL2CTRL0_POWER);    

    //Backup RTC control register state
    SocBackupReg.RTC_CTRL = HW_RTC_CTRL_RD();
    //Disable watchdog
    HW_RTC_CTRL_CLR(BM_RTC_CTRL_WATCHDOGEN);
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

    //Restore watchdog state
    if((SocBackupReg.RTC_CTRL & BM_RTC_CTRL_WATCHDOGEN) != 0)
        HW_RTC_CTRL_SET(BM_RTC_CTRL_WATCHDOGEN);
    
    //Restore PLL0
    if(SocBackupReg.CLKCTRL_PLL0CTRL0 & BM_CLKCTRL_PLL0CTRL0_EN_USB_CLKS)
        HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_EN_USB_CLKS);

    //Restore PLL1    
    if(SocBackupReg.CLKCTRL_PLL1CTRL0 & BM_CLKCTRL_PLL1CTRL0_EN_USB_CLKS )
        HW_CLKCTRL_PLL1CTRL0_SET(BM_CLKCTRL_PLL1CTRL0_EN_USB_CLKS);
    
    if(SocBackupReg.CLKCTRL_PLL1CTRL0 & BM_CLKCTRL_PLL1CTRL0_POWER )
        HW_CLKCTRL_PLL1CTRL0_SET(BM_CLKCTRL_PLL1CTRL0_POWER);

    //Restore PLL2
    if(!(SocBackupReg.CLKCTRL_PLL2CTRL0 & BM_CLKCTRL_PLL2CTRL0_CLKGATE))
        HW_CLKCTRL_PLL2CTRL0_CLR(BM_CLKCTRL_PLL2CTRL0_CLKGATE);
    
    if(SocBackupReg.CLKCTRL_PLL2CTRL0 & BM_CLKCTRL_PLL2CTRL0_POWER )
        HW_CLKCTRL_PLL2CTRL0_SET(BM_CLKCTRL_PLL2CTRL0_POWER);
/*
    //Restore IO0, IO1, PIX, GPMI, HSADC Clock
    if(!(SocBackupReg.CLKCTRL_FRAC0 & BM_CLKCTRL_FRAC0_CLKGATEIO0))
        HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEIO0);    
    
    if(!(SocBackupReg.CLKCTRL_FRAC0 & BM_CLKCTRL_FRAC0_CLKGATEIO1))
        HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEIO1);

    if(!(SocBackupReg.CLKCTRL_FRAC1 & BM_CLKCTRL_FRAC1_CLKGATEPIX))
        HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEPIX);    
    
    if(!(SocBackupReg.CLKCTRL_FRAC1 & BM_CLKCTRL_FRAC1_CLKGATEGPMI))
        HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEGPMI);
    
    if(!(SocBackupReg.CLKCTRL_FRAC1 & BM_CLKCTRL_FRAC1_CLKGATEHSADC))
        HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEHSADC);

    //Restore  CLKSEQ
    HW_CLKCTRL_CLKSEQ_WR(SocBackupReg.CLKCTRL_CLKSEQ);
    */
/*    
    if(!(SocBackupReg.CLKCTRL_CLKSEQ & BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF))
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);

    if(!(SocBackupReg.CLKCTRL_CLKSEQ & BM_CLKCTRL_CLKSEQ_BYPASS_ETM))
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_ETM);

    if(!(SocBackupReg.CLKCTRL_CLKSEQ & BM_CLKCTRL_CLKSEQ_BYPASS_SSP0))
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP0);    

    if(!(SocBackupReg.CLKCTRL_CLKSEQ & BM_CLKCTRL_CLKSEQ_BYPASS_SSP1))
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP1);

    if(!(SocBackupReg.CLKCTRL_CLKSEQ & BM_CLKCTRL_CLKSEQ_BYPASS_SSP2))
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP2);

    if(!(SocBackupReg.CLKCTRL_CLKSEQ & BM_CLKCTRL_CLKSEQ_BYPASS_SSP3))
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP3);
    
    if(!(SocBackupReg.CLKCTRL_CLKSEQ & BM_CLKCTRL_CLKSEQ_BYPASS_GPMI))
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);
*/
  /*
    //
    HW_CLKCTRL_SSP0_WR(SocBackupReg.CLKCTRL_SSP0);
    HW_CLKCTRL_SSP1_WR(SocBackupReg.CLKCTRL_SSP1);
    HW_CLKCTRL_SSP2_WR(SocBackupReg.CLKCTRL_SSP2);
    HW_CLKCTRL_SSP3_WR(SocBackupReg.CLKCTRL_SSP3);
    HW_CLKCTRL_GPMI_WR(SocBackupReg.CLKCTRL_GPMI);
    HW_CLKCTRL_SPDIF_WR(SocBackupReg.CLKCTRL_SPDIF);
    HW_CLKCTRL_SAIF0_WR(SocBackupReg.CLKCTRL_SAIF0);
    HW_CLKCTRL_SAIF1_WR(SocBackupReg.CLKCTRL_SAIF1);
    HW_CLKCTRL_DIS_LCDIF_WR(SocBackupReg.CLKCTRL_DIS_LCDIF);
    HW_CLKCTRL_ETM_WR(SocBackupReg.CLKCTRL_ETM);
    HW_CLKCTRL_ENET_WR(SocBackupReg.CLKCTRL_ENET);    
    HW_CLKCTRL_FLEXCAN_WR(SocBackupReg.CLKCTRL_FLEXCAN);
    
    if(!(SocBackupReg.CLKCTRL_XTAL & BM_CLKCTRL_XTAL_PWM_CLK24M_GATE))
         HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);



    //--------------------------------------------------------------------------     
    //Restore down all USB modules 
    //--------------------------------------------------------------------------  
    
    if(!(SocBackupReg.DIGCTL_CTRL & BM_DIGCTL_CTRL_USB0_CLKGATE))    
        HW_DIGCTL_CTRL_CLR(BM_DIGCTL_CTRL_USB0_CLKGATE); 
    
    if(!(SocBackupReg.DIGCTL_CTRL & BM_DIGCTL_CTRL_USB1_CLKGATE))    
        HW_DIGCTL_CTRL_CLR(BM_DIGCTL_CTRL_USB1_CLKGATE); 


    //USB0
    if(!(SocBackupReg.USBPHY0_CTRL & BM_USBPHY_CTRL_SFTRST) 
        ||!(SocBackupReg.USBPHY0_CTRL & BM_USBPHY_CTRL_SFTRST) )
    {
        HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_SFTRST);
        HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_CLKGATE);   
    }
    
    HW_USBPHY_PWD_WR(0,SocBackupReg.USBPHY0_PWD);

    //USB1
    if(!(SocBackupReg.USBPHY1_CTRL & BM_USBPHY_CTRL_SFTRST) 
        ||!(SocBackupReg.USBPHY1_CTRL & BM_USBPHY_CTRL_SFTRST) )
    {
        HW_USBPHY_CTRL_CLR(1,BM_USBPHY_CTRL_SFTRST);
        HW_USBPHY_CTRL_CLR(1,BM_USBPHY_CTRL_CLKGATE);   
    }
    
    HW_USBPHY_PWD_WR(1, SocBackupReg.USBPHY1_PWD);

    //--------------------------------------------------------------------------     
    //Restore Audio Power save 
    //--------------------------------------------------------------------------   

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
    

    //restore BCH
    if(!(SocBackupReg.BCH_CTRL & BM_BCH_CTRL_CLKGATE));  
        HW_BCH_CTRL_CLR(BM_BCH_CTRL_CLKGATE);

    //restore Watchdog state to Enable
    if(SocBackupReg.RTC_CTRL & BM_RTC_CTRL_WATCHDOGEN)
        HW_RTC_CTRL_SET(BM_RTC_CTRL_WATCHDOGEN);

    // Restore PWM  
    if(!(SocBackupReg.PWM_CTRL & BM_PWM_CTRL_SFTRST));  
    {
        HW_PWM_CTRL_CLR(BM_PWM_CTRL_SFTRST);    
        HW_PWM_CTRL_CLR(BM_PWM_CTRL_CLKGATE);
    }

    //restore GPMI
    HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
    for(; (HW_GPMI_CTRL0_RD() & BM_GPMI_CTRL0_SFTRST) != 0;);

    HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);
    for(; (HW_GPMI_CTRL0_RD() & BM_GPMI_CTRL0_CLKGATE) != 0;);

    HW_GPMI_CTRL0_WR(SocBackupReg.GPMI_CTRL0 & ~BM_GPMI_CTRL0_RUN);
    HW_GPMI_CTRL1_WR(SocBackupReg.GPMI_CTRL1);
    HW_GPMI_TIMING0_WR(SocBackupReg.GPMI_TIMING0);
    HW_GPMI_TIMING1_WR(SocBackupReg.GPMI_TIMING1);
  */ 
    //OALStall(10000);

  
}

//------------------------------------------------------------------------------
//
// Function:     SOCPowerOffCPURate
//
// Description:  
//------------------------------------------------------------------------------
static VOID SOCPowerOffCPURate(VOID)
{
    SocBackupReg.CLKCTRL_HBUS =  HW_CLKCTRL_HBUS_RD();
    SocBackupReg.CLKCTRL_XBUS =  HW_CLKCTRL_XBUS_RD();
    //
    // let CPU sink the xtal clock
    //
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
  
    for(; (HW_CLKCTRL_CPU_RD() & BM_CLKCTRL_CPU_BUSY_REF_XTAL) != 0;);
    OALStall(100);

    //Gating CPU_REF
    HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATECPU);

    // config CLK_CPU driver run in lowest frequency
    HW_CLKCTRL_CPU_WR( BF_CLKCTRL_CPU_DIV_CPU(1)         | 
                       BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0) | 
                       BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)  | 
                       BF_CLKCTRL_CPU_DIV_XTAL(1023)     | 
                       BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0));


    // config CLK_HBUS for lowest frequency
    HW_CLKCTRL_HBUS_WR(BF_CLKCTRL_HBUS_DIV(1023)               | 
                        BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)         | 
                        BF_CLKCTRL_HBUS_SLOW_DIV(32)           |
                        BF_CLKCTRL_HBUS_AUTO_CLEAR_DIV_ENABLE(1) |                        
                        BF_CLKCTRL_HBUS_ASM_ENABLE(1)          | 
                        BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(1) | 
                        BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(1)  | 
                        BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(1)   | 
                        BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(1) | 
                        BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(1)     | 
                        BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(1)     |
                        BF_CLKCTRL_HBUS_ASM_EMIPORT_AS_ENABLE(1) |                        
                        BF_CLKCTRL_HBUS_PXP_AS_ENABLE(1)         |
                        BF_CLKCTRL_HBUS_DCP_AS_ENABLE(1));

    // XBUS
    HW_CLKCTRL_HBUS_WR(BF_CLKCTRL_XBUS_AUTO_CLEAR_DIV_ENABLE(1)| 
                       BF_CLKCTRL_XBUS_DIV_FRAC_EN(0) | 
                       BF_CLKCTRL_XBUS_DIV(1023));
	OALStall(100); 

}

//------------------------------------------------------------------------------
//
// Function:     SOCPowerOnCPURate
//
// Description:  
//------------------------------------------------------------------------------
static VOID SOCPowerOnCPURate(VOID)
{
    //OALMSG(1, (L"SOCPowerOnCPURate++ \r\n"));
	
    //Ungating CPU_REF clock
    HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATECPU);

	//OALMSG(1, (_T("INFO: OEMPowerOff wakeUp...\r\n")));

    // config CLK_CPU driver for High setpoint
    HW_CLKCTRL_CPU_WR((BF_CLKCTRL_CPU_DIV_CPU(1)            |
                       BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)    |
                       BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)     | 
                       BF_CLKCTRL_CPU_DIV_XTAL(1)           | 
                       BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0)));


    // restore HBus XBus Setting
	
    HW_CLKCTRL_HBUS_WR(SocBackupReg.CLKCTRL_HBUS);

    HW_CLKCTRL_XBUS_WR(SocBackupReg.CLKCTRL_XBUS);

	
    //let CPU sink the CPU_REF clock
    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
	
	OALStall(100);
	//OALMSG(0, (L"SOCPowerOnCPURate-- \r\n"));
    
}

