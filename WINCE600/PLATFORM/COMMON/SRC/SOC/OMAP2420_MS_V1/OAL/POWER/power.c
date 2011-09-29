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
/* ***********************************************************
* THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY, 
* INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
* COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE.  TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET 
* POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR 
* YOUR USE OF THE PROGRAM.
*
* IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL, CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY 
* THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT 
* OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM.  EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF 
* REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF 
* USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF 
* YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS (U.S.$500).
*
* Unless otherwise stated, the Program written and copyrighted by Texas Instruments is distributed as "freeware".  You may, 
* only under TI's copyright in the Program, use and modify the Program without any charge or restriction.  You may 
* distribute to third parties, provided that you transfer a copy of this license to the third party and the third party 
* agrees to these terms by its first use of the Program. You must reproduce the copyright notice and any other legend of 
* ownership on each copy or partial copy, of the Program.
*
* You acknowledge and agree that the Program contains copyrighted material, trade secrets and other TI proprietary 
* information and is protected by copyright laws, international copyright treaties, and trade secret laws, as 
* well as other intellectual property laws.  To protect TI's rights in the Program, you agree not to decompile, reverse 
* engineer, disassemble or otherwise translate any object code versions of the Program to a human-readable form.  You agree 
* that in no event will you alter, remove or destroy any copyright notice included in the Program.  TI reserves all 
* rights not specifically granted under this license. Except as specifically provided herein, nothing in this agreement 
* shall be construed as conferring by implication, estoppel, or otherwise, upon you, any license or other right under any 
* TI patents, copyrights or trade secrets.
*
* You may not use the Program in non-TI devices.
* ********************************************************* */
//
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  File:  power.c
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <omap2420.h>
#include <bsp.h>

//Assembly routines for context save/restore
VOID OALCPUIdle();
VOID OALCPUIdle_sz();


//Map Public region of SRAM for sleep routines
//Must use cached address range or an exception is thrown
#define OMAP2420_SRAM_API_SUSPEND (OALPAtoCA(0x4020F800))
//#define OMAP2420_SRAM_API_SUSPEND (OALPAtoCA(0xA0100000))

enum
{
    ulINTC_MIR0 = 0,
    ulINTC_MIR1,
    ulINTC_MIR2,
    ulCM_FCLKEN_WKUP,
    ulCM_ICLKEN_WKUP,
    ulGPIO_DATAOUT,
    ulGPIO1_OE,
    ulPADCONF_UART1_RX,
    ulGPIO3_OE,
    ulPADCONF_SPI1_NCS2,
    ulPM_WKEN_WKUP,
    ulGPIO_FALLINGDETECT,
    ulGPIO_SYSCONFIG,
    ulGPIO_WAKEUPENABLE,
    ulGPIO_DEBOUNCINGTIME,
    ulGPIO_DEBOUNCENABLE,
    ulPM_WKEN1_CORE,
    ulPM_WKEN2_CORE,
    ulCM_AUTOIDLE_PLL,
    ulPRCM_CLKSRC_CTRL,
    ulPRCM_CLKSSETUP,
    ulPM_PWSTCTRL_CORE,
    ulPM_WKDEP_MPU,
    ulPM_WKDEP_DSP,
    ulPM_WKDEP_GFX,
    ulGPIO_IRQENABLE1,
    ulPM_PWSTCTRL_MPU,
    ulPM_PWSTCTRL_DSP,
    ulPM_PWSTCTRL_GFX,
    ulSDRC_POWER,
    ulSMS_SYSCONFIG,
    ulSDRC_SYSCONFIG,
    ulGPMC_SYSCONFIG,
    ulCM_CLKSTCTRL_MPU,
    ulCM_CLKSTCTRL_DSP,
    ulCM_CLKSTCTRL_GFX,
    ulCM_CLKSTCTRL_CORE,
    ulCM_AUTOIDLE_DSP,
    ulCM_AUTOIDLE1_CORE,
    ulCM_AUTOIDLE2_CORE,
    ulCM_AUTOIDLE3_CORE,
    ulCM_AUTOIDLE4_CORE,
    ulCM_AUTOIDLE_WKUP,
    ulCM_FCLKEN1_CORE,
    ulCM_FCLKEN2_CORE,

    MAX_REGISTER_CONTEXT
};
UINT32 ulaRegisterContext[MAX_REGISTER_CONTEXT];


//------------------------------------------------------------------------------
//
//  Function:  OEMPowerOff
//
//  Called when the system is to transition to it's lowest power mode (off)
//
VOID OEMPowerOff()
{
    void (*suspend_func_ptr)();
    int suspend_func_size;
    OAL_KITL_ARGS *pArgs;
    void *pIDCODE_reg;
    //Declare all register pointers
    static CPLD_REGS *pCpldRegs;
    static OMAP2420_MPUINTC_REGS *pIntcRegs;
    static OMAP2420_GPIO_REGS *pGPIO1Regs;
    static OMAP2420_GPIO_REGS *pGPIO2Regs;
    static OMAP2420_GPIO_REGS *pGPIO3Regs;
    static OMAP2420_GPIO_REGS *pGPIO4Regs;
    static OMAP2420_PRCM_REGS *pPRCMReg;
    static OMAP2420_CONTROL_PADCONF_REGS *pPadConfRegs;
    static OMAP2420_SMS_REGS *pSMSRegs;
    static OMAP2420_SDRC_REGS *pSDRCRegs;
    static OMAP2420_GPMC_REGS *pGPMCRegs;
    static OMAP2420_SYSC1_REGS *pSYSC1Regs;
    UINT32 new_value = 0x00000000;
    
    //Initialize all pointers
    pCpldRegs    = (CPLD_REGS *) OALPAtoUA(BSP_CPLD_REGS_PA);
    pIntcRegs    = (OMAP2420_MPUINTC_REGS *) OALPAtoUA(OMAP2420_INTC_MPU_REGS_PA);
    pGPIO1Regs   = (OMAP2420_GPIO_REGS *) OALPAtoUA(OMAP2420_GPIO1_REGS_PA);
    pGPIO2Regs   = (OMAP2420_GPIO_REGS *) OALPAtoUA(OMAP2420_GPIO2_REGS_PA);
    pGPIO3Regs   = (OMAP2420_GPIO_REGS *) OALPAtoUA(OMAP2420_GPIO3_REGS_PA);
    pGPIO4Regs   = (OMAP2420_GPIO_REGS *) OALPAtoUA(OMAP2420_GPIO4_REGS_PA);
    pPRCMReg     = (OMAP2420_PRCM_REGS *) OALPAtoUA(OMAP2420_PRCM_REGS_PA);
    pPadConfRegs = (OMAP2420_CONTROL_PADCONF_REGS *) OALPAtoUA(OMAP2420_SYSC1_REGS_PA+0x30);
    pSMSRegs     = (OMAP2420_SMS_REGS *) OALPAtoUA(OMAP2420_SMS_REGS_PA);
    pSDRCRegs    = (OMAP2420_SDRC_REGS *) OALPAtoUA(OMAP2420_SDRC_REGS_PA);
    pGPMCRegs    = (OMAP2420_GPMC_REGS *) OALPAtoUA(OMAP2420_GPMC_REGS_PA);
    pSYSC1Regs   = (OMAP2420_SYSC1_REGS *) OALPAtoUA(OMAP2420_SYSC1_REGS_PA);

    //Debug Start
    //OUTREG16(&pCpldRegs->LED, 1);
    OALMSG(1, (L"+OEMPowerOff\r\n"));
    g_oalWakeSource = SYSWAKE_UNKNOWN;

    //The IDCODE value is 0x0B5D902F for OMAP242x ES1.0.
    //The IDCODE value is 0x1B5D902F for OMAP242x ES2.0.
    //The IDCODE value is 0x2B5D902F for OMAP242x ES2.05.
    //The IDCODE value is 0x3B5D902F for OMAP242x ES2.1 *** Tested PM on this ***
    //The IDCODE value is 0x4B5D902F for OMAP242x ES2.1.1.
    pIDCODE_reg = OALPAtoUA(0x48014204);
    OALMSG(1, (L"OEMPowerOff: IDCODE_reg (%x)\r\n", *(int*)pIDCODE_reg));
    
    // Make sure that KITL is powered off
    pArgs = (OAL_KITL_ARGS*)OALArgsQuery(OAL_ARGS_QUERY_KITL);
    if ((pArgs->flags & OAL_KITL_FLAGS_ENABLED) != 0)
    {
        OALKitlPowerOff();
        OALMSG(1, (L"OEMPowerOff: KITL Disabled\r\n"));
    }

    //Backup all used registers
    ulaRegisterContext[ulINTC_MIR0] = pIntcRegs->ulINTC_MIR0; //restored
    ulaRegisterContext[ulINTC_MIR1] = pIntcRegs->ulINTC_MIR1; //restored
    ulaRegisterContext[ulINTC_MIR2] = pIntcRegs->ulINTC_MIR2; //restored
    ulaRegisterContext[ulCM_FCLKEN_WKUP] = pPRCMReg->ulCM_FCLKEN_WKUP; //restored
    ulaRegisterContext[ulCM_ICLKEN_WKUP] = pPRCMReg->ulCM_ICLKEN_WKUP; //restored
    ulaRegisterContext[ulGPIO_DATAOUT] = pGPIO1Regs->ulGPIO_DATAOUT; //restored
    ulaRegisterContext[ulGPIO1_OE] = pGPIO1Regs->ulGPIO_OE; //restored
    ulaRegisterContext[ulPADCONF_UART1_RX] = pPadConfRegs->ulPADCONF_UART1_RX; //restored
    ulaRegisterContext[ulGPIO3_OE] = pGPIO3Regs->ulGPIO_OE; //restored
    ulaRegisterContext[ulPADCONF_SPI1_NCS2] = pPadConfRegs->ulPADCONF_SPI1_NCS2; //restored
    ulaRegisterContext[ulPM_WKEN_WKUP] = pPRCMReg->ulPM_WKEN_WKUP; //restored
    ulaRegisterContext[ulGPIO_FALLINGDETECT] = pGPIO3Regs->ulGPIO_FALLINGDETECT; //restored
    ulaRegisterContext[ulGPIO_SYSCONFIG] = pGPIO3Regs->ulGPIO_SYSCONFIG; //restored
    ulaRegisterContext[ulGPIO_WAKEUPENABLE] = pGPIO3Regs->ulGPIO_WAKEUPENABLE; //restored
    ulaRegisterContext[ulGPIO_DEBOUNCINGTIME] = pGPIO3Regs->ulGPIO_DEBOUNCINGTIME; //restored
    ulaRegisterContext[ulGPIO_DEBOUNCENABLE] = pGPIO3Regs->ulGPIO_DEBOUNCENABLE; //restored
    ulaRegisterContext[ulPM_WKEN1_CORE] = pPRCMReg->ulPM_WKEN1_CORE; //restored
    ulaRegisterContext[ulPM_WKEN2_CORE] = pPRCMReg->ulPM_WKEN2_CORE; //restored
    ulaRegisterContext[ulCM_AUTOIDLE_PLL] = pPRCMReg->ulCM_AUTOIDLE_PLL; //restored
    ulaRegisterContext[ulPRCM_CLKSRC_CTRL] = pPRCMReg->ulPRCM_CLKSRC_CTRL; //restored
    ulaRegisterContext[ulPRCM_CLKSSETUP] = pPRCMReg->ulPRCM_CLKSSETUP; //restored
    ulaRegisterContext[ulPM_PWSTCTRL_CORE] = pPRCMReg->ulPM_PWSTCTRL_CORE; //restored
    ulaRegisterContext[ulPM_WKDEP_MPU] = pPRCMReg->ulPM_WKDEP_MPU; //restored
    ulaRegisterContext[ulPM_WKDEP_DSP] = pPRCMReg->ulPM_WKDEP_DSP; //restored
    ulaRegisterContext[ulPM_WKDEP_GFX] = pPRCMReg->ulPM_WKDEP_GFX; //restored
    ulaRegisterContext[ulGPIO_IRQENABLE1] = pGPIO3Regs->ulGPIO_IRQENABLE1; //restored
    ulaRegisterContext[ulPM_PWSTCTRL_MPU] = pPRCMReg->ulPM_PWSTCTRL_MPU; //restored
    ulaRegisterContext[ulPM_PWSTCTRL_DSP] = pPRCMReg->ulPM_PWSTCTRL_DSP; //restored
    ulaRegisterContext[ulPM_PWSTCTRL_GFX] = pPRCMReg->ulPM_PWSTCTRL_GFX; //restored
    ulaRegisterContext[ulSDRC_POWER] = pSDRCRegs->ulSDRC_POWER;         //restored
    ulaRegisterContext[ulSMS_SYSCONFIG] = pSMSRegs->ulSMS_SYSCONFIG;    //restored
    ulaRegisterContext[ulSDRC_SYSCONFIG] = pSDRCRegs->ulSDRC_SYSCONFIG; //restored
    ulaRegisterContext[ulGPMC_SYSCONFIG] = pGPMCRegs->ulGPMC_SYSCONFIG; //restored
    ulaRegisterContext[ulCM_CLKSTCTRL_MPU] = pPRCMReg->ulCM_CLKSTCTRL_MPU; //restored
    ulaRegisterContext[ulCM_CLKSTCTRL_DSP] = pPRCMReg->ulCM_CLKSTCTRL_DSP; //restored
    ulaRegisterContext[ulCM_CLKSTCTRL_GFX] = pPRCMReg->ulCM_CLKSTCTRL_GFX;  //restored
    ulaRegisterContext[ulCM_CLKSTCTRL_CORE] = pPRCMReg->ulCM_CLKSTCTRL_CORE; //restored
    ulaRegisterContext[ulCM_AUTOIDLE_DSP] = pPRCMReg->ulCM_AUTOIDLE_DSP;     //restored
    ulaRegisterContext[ulCM_AUTOIDLE1_CORE] = pPRCMReg->ulCM_AUTOIDLE1_CORE; //restored
    ulaRegisterContext[ulCM_AUTOIDLE2_CORE] = pPRCMReg->ulCM_AUTOIDLE2_CORE; //restored
    ulaRegisterContext[ulCM_AUTOIDLE3_CORE] = pPRCMReg->ulCM_AUTOIDLE3_CORE; //restored
    ulaRegisterContext[ulCM_AUTOIDLE4_CORE] = pPRCMReg->ulCM_AUTOIDLE4_CORE; //restored
    ulaRegisterContext[ulCM_AUTOIDLE_WKUP] = pPRCMReg->ulCM_AUTOIDLE_WKUP; //restored
    ulaRegisterContext[ulCM_FCLKEN1_CORE] = pPRCMReg->ulCM_FCLKEN1_CORE; //restored
    ulaRegisterContext[ulCM_FCLKEN2_CORE] = pPRCMReg->ulCM_FCLKEN2_CORE; //restored


    //Mask all interrupts
    pIntcRegs->ulINTC_MIR_SET0 = OMAP2420_MPUINTC_MASKALL;
    pIntcRegs->ulINTC_MIR_SET1 = OMAP2420_MPUINTC_MASKALL;
    pIntcRegs->ulINTC_MIR_SET2 = OMAP2420_MPUINTC_MASKALL;
    //Clear isr
    pIntcRegs->ulINTC_ISR_CLEAR0 = OMAP2420_MPUINTC_MASKALL;
    pIntcRegs->ulINTC_ISR_CLEAR1 = OMAP2420_MPUINTC_MASKALL;
    pIntcRegs->ulINTC_ISR_CLEAR2 = OMAP2420_MPUINTC_MASKALL;
    //Clear current pending isr
    pIntcRegs->ulINTC_CONTROL = 0x3;

    //functional_clock_control(PRCM_GPIOS, PRCM_ENABLE);
   	pPRCMReg->ulCM_FCLKEN_WKUP |= PRCM_FCLKEN_WKUP_EN_GPIOS;
    //interface_clock_control(PRCM_GPIOS, PRCM_ENABLE);
	pPRCMReg->ulCM_ICLKEN_WKUP |= PRCM_ICLKEN_WKUP_EN_GPIOS;

    //This sets GPIO 12 to drive low
    pGPIO1Regs->ulGPIO_DATAOUT &= 0xFFFFEFFF; 

    //This sets GPIO 12 as an output
    pGPIO1Regs->ulGPIO_OE &= 0xFFFFEFFF; 

    //Set up GPIO 12 pin mux
    //first clear bits for pin P21
    pPadConfRegs->ulPADCONF_UART1_RX &= 0xFF00FFFF;

    //now set appropriate bits for P21 mode 3, pulldown enabled   
    pPadConfRegs->ulPADCONF_UART1_RX |= 0x000B0000; 

    //Set GPIO 88 as an input.
    pGPIO3Regs->ulGPIO_OE |= 0x01000000;

    //GPIO 88 pin mux.
    //first clear bits for pin T19
    pPadConfRegs->ulPADCONF_SPI1_NCS2 &= 0xFF00FFFF;

    //now set appropriate bits for T19 mode 3, pullup/pulldown disabled
    pPadConfRegs->ulPADCONF_SPI1_NCS2 |= 0x00030000;

    //GPIO 89 (S7) will be used to wake up the device
    //wakeup_event_controller
    pPRCMReg->ulPM_WKEN_WKUP |= 0x00000004;

    //This sets GPIO 89 as an input
    pGPIO3Regs->ulGPIO_OE |= 0x02000000;

    //This will enable the wakeup on the falling edge of GPIO 89
    pGPIO3Regs->ulGPIO_FALLINGDETECT |= 0x02000000;

    //Enable wake up capabilities for all GPIO3
    pGPIO3Regs->ulGPIO_SYSCONFIG |= 0x00000004;

    //Enable wake up for GPIO 89
    pGPIO3Regs->ulGPIO_WAKEUPENABLE |= 0x02000000; 

    //Set GPIO 3 for smart idle.
    pGPIO3Regs->ulGPIO_SYSCONFIG |= 0x00000010;

    //Set debouncing time for GPIO 89. 
    pGPIO3Regs->ulGPIO_DEBOUNCINGTIME = 0x000000FF;

    //Enable debouncing for GPIO 89. 
    pGPIO3Regs->ulGPIO_DEBOUNCENABLE |= 0x02000000;

    //GPIO 89 pin mux.
    //first clear bits for pin R19
    pPadConfRegs->ulPADCONF_SPI1_NCS2 &= 0x00FFFFFF;
    //now set appropriate bits for R19 mode 3, pullup enabled
    pPadConfRegs->ulPADCONF_SPI1_NCS2 |= 0x1B000000;

    //Unmask GPIO 89 IRQ - IRQ31 for GPIO3 will wake the system
    new_value = IRQ_GPIO_0 + 89;
    OALIntrEnableIrqs(1, &new_value);

//========================================//
//= At this point the GPIO 89 is set     =//
//= up to wakeup the MPU.  On the SDP    =//
//= User Interface Module this is        =//
//= keyboard S7.                         =//
//========================================//      

    //Disable CORE wakeup events because we will use a WKUP domain wakeup event.   
    pPRCMReg->ulPM_WKEN1_CORE = 0x00000000;
    pPRCMReg->ulPM_WKEN2_CORE = 0x00000000;

    // Clear WKUP domain reset status register
    pPRCMReg->ulRM_RSTST_WKUP = 0x0000007B;
    pPRCMReg->ulRM_RSTST_MPU  = 0x0000000F; 

    //prepare_for_idle
    //apll_54M_clock_auto_control(PRCM_APLL_ENABLE);
    new_value = pPRCMReg->ulCM_AUTOIDLE_PLL & 0xFFFFFF3F;
    pPRCMReg->ulCM_AUTOIDLE_PLL = new_value | PRCM_CM_AUTOIDLE_PLL_AUTO_54M_EN;

    //apll_96M_clock_auto_control(PRCM_APLL_ENABLE);
    new_value = pPRCMReg->ulCM_AUTOIDLE_PLL & 0xFFFFFFF3;
    pPRCMReg->ulCM_AUTOIDLE_PLL = new_value | PRCM_CM_AUTOIDLE_PLL_AUTO_96M_EN;

    //dpll_clock_auto_control(PRCM_DPLL_ENABLE);
    new_value = pPRCMReg->ulCM_AUTOIDLE_PLL & 0xFFFFFFFC;
    pPRCMReg->ulCM_AUTOIDLE_PLL = new_value | PRCM_CM_AUTOIDLE_PLL_AUTO_DPLL_EN;

    //setup_external_clock_control(PRCM_OFF_WHEN_RETENTION_OR_OFF); 
    new_value = pPRCMReg->ulPRCM_CLKSRC_CTRL & 0xFFFFFFE7;
    pPRCMReg->ulPRCM_CLKSRC_CTRL = new_value | PRCM_CLKSRC_CTRL_OFF_WHEN_RETENTION_OR_OFF;

    //set_system_clock_setup_time(0x0000);
    pPRCMReg->ulPRCM_CLKSSETUP = 0;

    //SKIPPED:setup_voltage_scaling_auto(PRCM_L1, PRCM_L1, 0x0000);
    //SKIPPED: emulation_tools_control(PRCM_DISABLE); 

    //set_core_memory_retention_state
    new_value = pPRCMReg->ulPM_PWSTCTRL_CORE & 0xFFFFFFC7;
    pPRCMReg->ulPM_PWSTCTRL_CORE = new_value | PRCM_PM_PWSTCTRL_CORE_MEM3RETSTATE_RETAIN |
        PRCM_PM_PWSTCTRL_CORE_MEM2RETSTATE_RETAIN | PRCM_PM_PWSTCTRL_CORE_MEM1RETSTATE_RETAIN;

      //MPU dependant on WKUP domain
    pPRCMReg->ulPM_WKDEP_MPU = 0x00000010;
    pPRCMReg->ulPM_WKDEP_DSP = 0x00000000;
    pPRCMReg->ulPM_WKDEP_GFX = 0x00000000;
    //END prepare_for_idle

    //Need to clear IRQ Status or will not enter idle.
    pGPIO3Regs->ulGPIO_IRQSTATUS1 = 0x02000000;
    pGPIO3Regs->ulGPIO_IRQSTATUS2 = 0x02000000;

    //enable GPIO3 interrupt
    pGPIO3Regs->ulGPIO_IRQENABLE1 = 0x02000000;

    //enter_idle_mode(PRCM_CHIP, PRCM_DORMANT, PRCM_AUTO);
    //set_powermode(PRCM_DORMANT)
    //set_domain_powerstate(PRCM_MPU,  PRCM_RETENTION);   
    new_value = pPRCMReg->ulPM_PWSTCTRL_MPU & 0xFFFFFFF8;
    pPRCMReg->ulPM_PWSTCTRL_MPU = new_value | 0x00000005;
    //NOTE: The current boot method is Overlay Boot rather than context restore
    //So the processor ROM code does not excecute to be able to restore
    //For now, we use a PRCM_RETENTION state for the MPU instead of PRCM_DORMANT
    //which allows us to retain logic when we sleep (LOGICRETSTATE) and return here

    //set_domain_powerstate(PRCM_DSP,  PRCM_OFF);
    pPRCMReg->ulPM_PWSTCTRL_DSP |= 0x00000003;

    //set_domain_powerstate(PRCM_GFX,  PRCM_OFF);
    pPRCMReg->ulPM_PWSTCTRL_GFX |= 0x00000003;

    //set_domain_powerstate(PRCM_CORE, PRCM_RETENTION);
    new_value = pPRCMReg->ulPM_PWSTCTRL_CORE & 0xFFFFFFFC;
    pPRCMReg->ulPM_PWSTCTRL_CORE = new_value | 0x00000001;
   
    //SKIPPED: force_standby_usb();

    //sdram_self_refresh_on_idle_req(PRCM_ENABLE);
    pSDRCRegs->ulSDRC_POWER |= 0x00000040;
    //set_smartidle_smartstandby
    pSMSRegs->ulSMS_SYSCONFIG |= 0x00000010;
    pSDRCRegs->ulSDRC_SYSCONFIG |= 0x00000010;
    pGPMCRegs->ulGPMC_SYSCONFIG |= 0x00000010;

    //clock_auto_control(PRCM_CHIP, PRCM_ENABLE);
    pPRCMReg->ulCM_CLKSTCTRL_MPU = 0x00000001;  //set AutoState for the MPU clock
    pPRCMReg->ulCM_CLKSTCTRL_DSP = 0x00000101;  //set AutoState for the DSP and IVA clocks
    pPRCMReg->ulCM_CLKSTCTRL_GFX = 0x00000001;  //set AutoState for the GFX clock
    pPRCMReg->ulCM_CLKSTCTRL_CORE = 0x00000007;  //set AutoState for the DSS, L3, and L4 clocks

    //set_domain_autoidle(PRCM_CHIP);
    pPRCMReg->ulCM_AUTOIDLE_DSP = 0x00000002;  //set the AutoIdle for the DSP IPI interface clock
    pPRCMReg->ulCM_AUTOIDLE1_CORE = 0xFFFFFFF9;  //set AutoIdle all interface clocks in CORE
    pPRCMReg->ulCM_AUTOIDLE2_CORE = 0x00000007;
    pPRCMReg->ulCM_AUTOIDLE3_CORE = 0x00000007;
    pPRCMReg->ulCM_AUTOIDLE4_CORE = 0x0000001F;
    pPRCMReg->ulCM_AUTOIDLE_WKUP = 0x0000003F;  //set AutoIdle for all WKUP clocks

    pPRCMReg->ulCM_FCLKEN1_CORE,  0x00000000;
    pPRCMReg->ulCM_FCLKEN2_CORE,  0x00000000;



    // **************************  BEFORE SLEEP  **********************************
    //OUTREG16(&pCpldRegs->LED, 3);
    
	//We copy the assembler sleep/wakeup routines to SRAM.
    //These routines should be in SRAM as that's the only
    //memory the MPU can see when it wakes up.
    //IMPORTANT: Drivers using the SRAM should be aware that it's contents
    //will be modified after a sleep (ie. if display driver used it as a frame buffer)
    //Drivers should save/restore the SRAM contents when it gets the power IOCTL's D0-D4
    suspend_func_ptr = (void*)(OMAP2420_SRAM_API_SUSPEND);
    suspend_func_size = (UINT32)OALCPUIdle_sz - (UINT32)OALCPUIdle + 256;  //include constants
    memcpy(suspend_func_ptr, OALCPUIdle, suspend_func_size );

    OALMSG(1, (L"OEMPowerOff: suspend_func_size (%d)\r\n", suspend_func_size));
    OALMSG(1, (L"OEMPowerOff: suspend_func_ptr (%x)\r\n", suspend_func_ptr));
    OALMSG(1, (L"OEMPowerOff: Val:suspend_func_ptr (%x)\r\n", *(int*)suspend_func_ptr));
    OALMSG(1, (L"OEMPowerOff: OALCPUIdle (%x)\r\n", OALCPUIdle));
    OALMSG(1, (L"OEMPowerOff: Val:OALCPUIdle (%x)\r\n", *(int*)OALCPUIdle));

    //IMPORTANT: Need to Clear TLB, I and D cache before a jump to cached SRAM
    OALClearDTLB();
    OALClearITLB();
    OALFlushDCache();
    OALFlushICache();
    //Call routines to save context and execute WFI (wait for interrupt) instruction
    (*suspend_func_ptr)();

    //OUTREG16(&pCpldRegs->LED, 4);
    // **************************  AFTER SLEEP  **********************************

    OALMSG(1, (L"OEMPowerOff: Back from sleep\r\n"));

    // Clear WKUP domain reset status registers
    pPRCMReg->ulRM_RSTST_WKUP = 0x0000007B;
    pPRCMReg->ulRM_RSTST_MPU  = 0x0000000F; 
    //Clear CORE domain wakeup status registers
    pPRCMReg->ulPM_WKST1_CORE = 0x04667FF8;
    pPRCMReg->ulPM_WKST2_CORE = 0x00000005;

    //Check for wake event
    OALMSG(1, (L"OEMPowerOff: pPRCMReg->ulPM_WKST_WKUP (%x)\r\n", pPRCMReg->ulPM_WKST_WKUP ));
    if(4 == pPRCMReg->ulPM_WKST_WKUP)
    {
        //GPIO Wake up
        g_oalWakeSource = SYSWAKE_POWER_BUTTON;
        OUTREG32(&pPRCMReg->ulPM_WKST_WKUP, 0x00000005); //Clear MPU interrupt status 
    }

    //Restore SDRC regs used
    pSDRCRegs->ulSDRC_POWER = ulaRegisterContext[ulSDRC_POWER];
    pSMSRegs->ulSMS_SYSCONFIG = ulaRegisterContext[ulSMS_SYSCONFIG];
    pSDRCRegs->ulSDRC_SYSCONFIG = ulaRegisterContext[ulSDRC_SYSCONFIG];
    pGPMCRegs->ulGPMC_SYSCONFIG = ulaRegisterContext[ulGPMC_SYSCONFIG];

    //Restore Clocks
    pPRCMReg->ulCM_FCLKEN_WKUP = ulaRegisterContext[ulCM_FCLKEN_WKUP];
    pPRCMReg->ulCM_ICLKEN_WKUP = ulaRegisterContext[ulCM_ICLKEN_WKUP];
    pPRCMReg->ulCM_CLKSTCTRL_MPU = ulaRegisterContext[ulCM_CLKSTCTRL_MPU];
    pPRCMReg->ulCM_CLKSTCTRL_DSP = ulaRegisterContext[ulCM_CLKSTCTRL_DSP];
    pPRCMReg->ulCM_CLKSTCTRL_GFX = ulaRegisterContext[ulCM_CLKSTCTRL_GFX];
    pPRCMReg->ulCM_CLKSTCTRL_CORE = ulaRegisterContext[ulCM_CLKSTCTRL_CORE];
    pPRCMReg->ulCM_AUTOIDLE_DSP = ulaRegisterContext[ulCM_AUTOIDLE_DSP];
    pPRCMReg->ulCM_AUTOIDLE1_CORE = ulaRegisterContext[ulCM_AUTOIDLE1_CORE];
    pPRCMReg->ulCM_AUTOIDLE2_CORE = ulaRegisterContext[ulCM_AUTOIDLE2_CORE];
    pPRCMReg->ulCM_AUTOIDLE3_CORE = ulaRegisterContext[ulCM_AUTOIDLE3_CORE];
    pPRCMReg->ulCM_AUTOIDLE4_CORE = ulaRegisterContext[ulCM_AUTOIDLE4_CORE];
    pPRCMReg->ulCM_AUTOIDLE_WKUP = ulaRegisterContext[ulCM_AUTOIDLE_WKUP];
    pPRCMReg->ulCM_FCLKEN1_CORE = ulaRegisterContext[ulCM_FCLKEN1_CORE];
    pPRCMReg->ulCM_FCLKEN2_CORE = ulaRegisterContext[ulCM_FCLKEN2_CORE];

    //Restore GPIO and wake regs
    pGPIO1Regs->ulGPIO_DATAOUT = ulaRegisterContext[ulGPIO_DATAOUT];
    pGPIO1Regs->ulGPIO_OE = ulaRegisterContext[ulGPIO1_OE];
    pPadConfRegs->ulPADCONF_UART1_RX = ulaRegisterContext[ulPADCONF_UART1_RX];
    pGPIO3Regs->ulGPIO_OE = ulaRegisterContext[ulGPIO3_OE];
    pPadConfRegs->ulPADCONF_SPI1_NCS2 = ulaRegisterContext[ulPADCONF_SPI1_NCS2];
    pPRCMReg->ulPM_WKEN_WKUP = ulaRegisterContext[ulPM_WKEN_WKUP];
    pGPIO3Regs->ulGPIO_FALLINGDETECT = ulaRegisterContext[ulGPIO_FALLINGDETECT];
    pGPIO3Regs->ulGPIO_SYSCONFIG = ulaRegisterContext[ulGPIO_SYSCONFIG];
    pGPIO3Regs->ulGPIO_WAKEUPENABLE = ulaRegisterContext[ulGPIO_WAKEUPENABLE];
    pGPIO3Regs->ulGPIO_DEBOUNCINGTIME = ulaRegisterContext[ulGPIO_DEBOUNCINGTIME];
    pGPIO3Regs->ulGPIO_DEBOUNCENABLE = ulaRegisterContext[ulGPIO_DEBOUNCENABLE];
    pPRCMReg->ulPM_WKEN1_CORE = ulaRegisterContext[ulPM_WKEN1_CORE];
    pPRCMReg->ulPM_WKEN2_CORE = ulaRegisterContext[ulPM_WKEN2_CORE];
    pPRCMReg->ulCM_AUTOIDLE_PLL = ulaRegisterContext[ulCM_AUTOIDLE_PLL];
    pPRCMReg->ulPRCM_CLKSRC_CTRL = ulaRegisterContext[ulPRCM_CLKSRC_CTRL];
    pPRCMReg->ulPRCM_CLKSSETUP = ulaRegisterContext[ulPRCM_CLKSSETUP];
    pPRCMReg->ulPM_PWSTCTRL_CORE = ulaRegisterContext[ulPM_PWSTCTRL_CORE];
    pPRCMReg->ulPM_WKDEP_MPU = ulaRegisterContext[ulPM_WKDEP_MPU];
    pPRCMReg->ulPM_WKDEP_DSP = ulaRegisterContext[ulPM_WKDEP_DSP];
    pPRCMReg->ulPM_WKDEP_GFX = ulaRegisterContext[ulPM_WKDEP_GFX];
    pGPIO3Regs->ulGPIO_IRQENABLE1 = ulaRegisterContext[ulGPIO_IRQENABLE1];

    //Restore domain states
    pPRCMReg->ulPM_PWSTCTRL_MPU = ulaRegisterContext[ulPM_PWSTCTRL_MPU];
    pPRCMReg->ulPM_PWSTCTRL_DSP = ulaRegisterContext[ulPM_PWSTCTRL_DSP];
    pPRCMReg->ulPM_PWSTCTRL_GFX = ulaRegisterContext[ulPM_PWSTCTRL_GFX];

    // Reinitialize KITL
    if ((pArgs->flags & OAL_KITL_FLAGS_ENABLED) != 0)
    {
        OALKitlPowerOn();
    }

    //Restore interrupt masks
    pIntcRegs->ulINTC_MIR0 = ulaRegisterContext[ulINTC_MIR0];
    pIntcRegs->ulINTC_MIR1 = ulaRegisterContext[ulINTC_MIR1];
    pIntcRegs->ulINTC_MIR2 = ulaRegisterContext[ulINTC_MIR2];

    OALMSG(1, (L"-OEMPowerOff\r\n"));
}




//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalPresuspend
//
BOOL OALIoCtlHalPresuspend(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
)
{
	OALMSG(1, (L"OALIoCtlHalPresuspend\r\n"));
    return TRUE;
}



