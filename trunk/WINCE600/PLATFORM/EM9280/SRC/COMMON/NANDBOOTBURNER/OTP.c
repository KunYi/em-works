//-----------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  otp.c
//
//  Contains otp support functions.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include "bsp.h"
#include "otp.h"
#include "pmu.h"
#pragma warning(pop)

//-----------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregOTP;

#ifdef NAND_PDD
extern PVOID pv_HWregCLKCTRL;
extern PVOID pv_HWregPOWER;
#else
//-----------------------------------------------------------------------------
// Global Variables
PVOID pv_HWregCLKCTRL = NULL;
PVOID pv_HWregPOWER = NULL;
#endif	//NAND_PDD

void DelayMs(INT32 ms)
{
    int i,j;
    
    for(i=0; i<ms; i++)
    {
        for(j=0; j<0x10000; j++);
    }
}
//-----------------------------------------------------------------------------
//
//  Function:  InitOTP
//
//  This function initialize necessary reg pointers for OTP operations.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void InitOTP(void)
{
#ifdef  NAND_PDD
    if (pv_HWregCLKCTRL == NULL)
    {
		pv_HWregCLKCTRL = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_CLKCTRL, FALSE);
	}

    if (pv_HWregPOWER == NULL)
    {
		pv_HWregPOWER = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_POWER, FALSE);
	}

#else	// -> for Flash driver
    PHYSICAL_ADDRESS phyAddr;

    if (pv_HWregCLKCTRL == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_CLKCTRL;

        // Map peripheral physical address to virtual address
        pv_HWregCLKCTRL = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregCLKCTRL == NULL)
        {
            ERRORMSG(1, (_T("ClkAlloc::MmMapIoSpace failed!\r\n")));
            return;
        }
    }

    if (pv_HWregPOWER == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_POWER;

        // Map peripheral physical address to virtual address
        pv_HWregPOWER = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregPOWER == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace failed!\r\n")));
            return;
        }
    }
#endif	//NAND_PDD
}
//-----------------------------------------------------------------------------
//
//  Function:  DeInitOTP
//
//  This function deinitialize necessary reg pointers for OTP operations.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DeInitOTP(void)
{
#ifdef  NAND_PDD
    pv_HWregCLKCTRL = NULL;
    pv_HWregPOWER   = NULL;

#else	// -> for Flash driver
    if (pv_HWregCLKCTRL)
    {
        MmUnmapIoSpace(pv_HWregCLKCTRL, 0x1000);
        pv_HWregCLKCTRL = NULL;
    }

    if (pv_HWregPOWER)
    {
        MmUnmapIoSpace(pv_HWregPOWER, 0x1000);
        pv_HWregPOWER = NULL;
    }
#endif	//NAND_PDD
}


//---------------------------------------------------------------------------------------
//
// Function:  ClockSetHclkDiv
//
// This Function Set PClk to HClk divide ratio
// HClk = PClk/UINT32Div
// The HClk frequency will be the result of the PClk divided by UINT32Div.
//
// Parameters:
//      UINT32Div     - Ratio of PClk to HClk
//      bDivFracEn - Enable divider to be a fractional ratio.
// Returns:
//         Returns TRUE if the function is success , else return FALSE.
//
//---------------------------------------------------------------------------------------
BOOL ClockSetHclkDiv(UINT32 UINT32Div, BOOL bDivFracEn)
{
    BOOL rc = FALSE;

    // Cannot set divider to zero.
    if( UINT32Div == 0 )
    {
        ERRORMSG(1, (_T("ClockSetHclkDiv: Devide By Zero\r\n")));
        goto Cleanup;
    }
    // Return if busy with another divider change.
    if( HW_CLKCTRL_HBUS.B.ASM_BUSY )
    {
        ERRORMSG(1, (_T("ClockSetHclkDiv: CLK_DIV BUSY\r\n")));
        goto Cleanup;
    }
    // Set the divider type.
    if( bDivFracEn )
    {
       // Use the divider as a fractional divider.
       BF_SET(CLKCTRL_HBUS,DIV_FRAC_EN);
    }
    else
    {
       // Use the divider as an integer divider.
       BF_CLR(CLKCTRL_HBUS,DIV_FRAC_EN);
    }
    // Set divider value.
    HW_CLKCTRL_HBUS.B.DIV = UINT32Div;

    rc = TRUE;

Cleanup:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerSetVddioValue
//
//  This function sets Vddio value
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void PowerSetVddioValue(UINT16 u16Vddio_mV)
{
    UINT16 u16Vddio_Set;

    // Convert mV to register setting
    u16Vddio_Set = (UINT16)((u16Vddio_mV - VDDIO_BASE_MV)/25);
    //RETAILMSG(TRUE, (L"Vddio value is %d.\r\n",u16Vddio_Set));
    
    if(u16Vddio_Set <= 0x1f)
        BW_POWER_VDDIOCTRL_TRG(u16Vddio_Set);
}

void CheckOtpStatus()
{
    DelayMs(100);
        
    //Second, check if HW_OCOTP_CTRL_BUSY 
    while((HW_OCOTP_CTRL_RD() & BM_OCOTP_CTRL_BUSY));

    //Clear HW_OCOTP_CTRL_ERROR is set
    if(BF_OCOTP_CTRL_ERROR(HW_OCOTP_CTRL_RD())){
        BW_OCOTP_CTRL_ERROR(BF_OCOTP_CTRL_ERROR(0));
    } 
    
    DelayMs(100);
}


void SetHClock4OtpWrite(clk_settings_t * savedSettings)
{
    UINT32 cpuRef, cpuFreq;    
    
    savedSettings->hclkDiv = HW_CLKCTRL_HBUS.B.DIV;
    //RETAILMSG(TRUE, (L"Current Hclk divider is %d.\r\n",savedSettings->hclkDiv));  
    savedSettings->hclkFracEn = HW_CLKCTRL_HBUS.B.DIV_FRAC_EN;
    
    // get cpu divisor and frac en.
    if (HW_CLKCTRL_CLKSEQ.B.BYPASS_CPU)
    {
       savedSettings->cpuDiv = HW_CLKCTRL_CPU.B.DIV_XTAL;
       savedSettings->cpuFracEn = HW_CLKCTRL_CPU.B.DIV_XTAL_FRAC_EN;
       // get hclk to 24 MHz with fractional divides off.
       HW_CLKCTRL_CPU.B.DIV_XTAL = 1;
       HW_CLKCTRL_CPU.B.DIV_XTAL_FRAC_EN = 0;
       HW_CLKCTRL_HBUS.B.DIV = 1;
       HW_CLKCTRL_HBUS.B.DIV_FRAC_EN = 0;
    }
    else
    {
       // 
       savedSettings->cpuDiv = HW_CLKCTRL_CPU.B.DIV_CPU;
       savedSettings->cpuFracEn = HW_CLKCTRL_CPU.B.DIV_CPU_FRAC_EN;
       // get hclk to about 24 MHz.  Compute cpu clk, use hclk int divide.
       // clkctrl frac cpu clock gate will be zero.
       cpuRef = (480 * 18)/(HW_CLKCTRL_FRAC0.B.CPUFRAC);//what is 480?
       if (savedSettings->cpuFracEn)
       {
          cpuFreq = (savedSettings->cpuDiv * cpuRef) / 1024;
       }
       else
       {
          cpuFreq = cpuRef / savedSettings->cpuDiv;
       }
       
       //RETAILMSG(TRUE, (L"Hclk freq is %d.\r\n",cpuFreq));
       HW_CLKCTRL_HBUS.B.DIV_FRAC_EN = 0;
       HW_CLKCTRL_HBUS.B.DIV = cpuFreq / 24 +1;  // div by 24 MHz to get divider.
       //RETAILMSG(TRUE, (L"New Hclk divider is %d.\r\n",cpuFreq / 24 +1));
    }
    // measurements on wave files indicate it takes 3-4 uSec before hclk is 
    // stable at the new freq.
    DelayMs(10);

    //First, set HCLK to 24 MHz
    /*DDKClockGetFreq(DDK_CLOCK_SIGNAL_P_CLK, &dwRootFreq);
    RETAILMSG(TRUE, (L"Pclk freq is %d.\r\n",dwRootFreq));
    RETAILMSG(TRUE, (L"pOtp->OtpData is %d.\r\n",pOtp->OtpData));
    RETAILMSG(TRUE, (L"pOtp->OtpAddr is %d.\r\n",pOtp->OtpAddr));
    
    if(dwRootFreq > OTP_PROGRAM_FREQ){
        dwOriHClkDiv = HW_CLKCTRL_HBUS_RD();
        RETAILMSG(TRUE, (L"Current Hclk divider is %d.\r\n",dwOriHClkDiv));            
        dwHClkDiv = dwRootFreq / OTP_PROGRAM_FREQ + 1;
        RETAILMSG(TRUE, (L"New Hclk divider is %d.\r\n",dwHClkDiv));
        if(!ClockSetHclkDiv(dwHClkDiv,FALSE))
        {
            ERRORMSG(1, (_T("Failed to set Hclk divider.\r\n")));
            return FALSE;
        }  
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_H_CLK, &dwRootFreq);
        RETAILMSG(TRUE, (L"Hclk freq is %d.\r\n",dwRootFreq));
        DelayMs(500);
    }*/  
    
}

// put things back like they were.  
void RestoreClks(clk_settings_t * savedSettings)
{
   // currently, the brazo board has the old clk ctrl block, and clock is 
   // fixed at 24 MHz, hence this hack.
   if (HW_CLKCTRL_CLKSEQ.B.BYPASS_CPU)
   {
      HW_CLKCTRL_CPU.B.DIV_XTAL = savedSettings->cpuDiv;
      HW_CLKCTRL_CPU.B.DIV_XTAL_FRAC_EN = savedSettings->cpuFracEn; 
   }
   else 
   {
      HW_CLKCTRL_CPU.B.DIV_CPU = savedSettings->cpuDiv;
      HW_CLKCTRL_CPU.B.DIV_CPU_FRAC_EN = savedSettings->cpuFracEn; 
   }
   
   HW_CLKCTRL_HBUS.B.DIV = savedSettings->hclkDiv;
   HW_CLKCTRL_HBUS.B.DIV_FRAC_EN = savedSettings->hclkFracEn;

   // measurements on wave files indicate it takes 3-4 uSec before hclk is 
   // stable at the new freq.
   DelayMs(10);   
}

//-----------------------------------------------------------------------------
//
//  Function:  OTPRead
//
//  This function read OTP value
//
//  Parameters:
//      POtpProgram pOtp: specify the OTP reg address and get the read value
//
//  Returns:
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OTPRead(POtpProgram pOtp)
{
    // make sure we can access banks
    CheckOtpStatus();
        
    //Set RD_BANK_OPEN 
    BW_OCOTP_CTRL_RD_BANK_OPEN(1);
    
    //Make sure RD_BANK_OPEN is set
    //while((HW_OCOTP_CTRL_RD() & BM_OCOTP_CTRL_BUSY));
    CheckOtpStatus();
        
    //read data
    //pOtp->OtpData = HW_OCOTP_CUSTn_RD(0);
    //RETAILMSG(TRUE, (L"pOtp->OtpData is 0x%x.\r\n",pOtp->OtpData));
    pOtp->OtpData = (*(volatile hw_ocotp_data_t *) (REGS_OCOTP_BASE+pOtp->OtpAddr)).U;
    RETAILMSG(TRUE, (L"The value of Otp address 0x%x is 0x%x.\r\n", pOtp->OtpAddr, pOtp->OtpData));
    
    //Clear RD_BANK_OPEN
    BW_OCOTP_CTRL_RD_BANK_OPEN(0);
    CheckOtpStatus();
      
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  OTPProgram
//
//  This function write OTP value
//
//  Parameters:
//      POtpProgram pOtp: specify the OTP reg address and set the value to write.
//
//  Returns:
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OTPProgram(POtpProgram pOtp)
{
    clk_settings_t savedSettings;
    BYTE savedVddio;
    OtpProgram VerifyOtp;
    
    VerifyOtp.OtpAddr = pOtp->OtpAddr;
    RETAILMSG(TRUE, (L"To program Otp address 0x%x with data 0x%x.\r\n", pOtp->OtpAddr, pOtp->OtpData));    
    
    // make sure we can access banks
    CheckOtpStatus();    

    //Make sure we can restore original setting later.
    savedVddio = (BYTE)(HW_POWER_VDDIOCTRL_RD() & BM_POWER_VDDIOCTRL_TRG);
    
    PowerSetVddioValue(OTP_PROGRAM_VOLTAGE);
    
    SetHClock4OtpWrite(&savedSettings);

    while (HW_OCOTP_CTRL_RD() & BM_OCOTP_CTRL_RD_BANK_OPEN){
        HW_OCOTP_CTRL_CLR(BM_OCOTP_CTRL_RD_BANK_OPEN);
        DelayMs(100);
    }
    
    CheckOtpStatus();
    
    //RETAILMSG(TRUE, (L"Lock bits reg is 0x%x.\r\n",HW_OCOTP_LOCK_RD()));
    //BF_OCOTP_LOCK_CUST0(0);
    //RETAILMSG(TRUE, (L"Lock bits reg is 0x%x.\r\n",HW_OCOTP_LOCK_RD()));
    
    //Third, set otp address
    //Enable writes to reg (use the unlock key)
    //Here we need calculate the offset of OTP registers array based on reg32, 
    //i.e. the offset of HW_OCOTP_CUST should be 0
    pOtp->OtpAddr = pOtp->OtpAddr/0x10-2;
    HW_OCOTP_CTRL_WR(OCOTP_WR_UNLOCK_KEY_VALUE | (pOtp->OtpAddr & BM_OCOTP_CTRL_ADDR));    
    //Write data
    HW_OCOTP_DATA_WR(pOtp->OtpData);
    
    //Wait till operation finishes    
    CheckOtpStatus();    
    
    OTPRead(&VerifyOtp);   
    if((VerifyOtp.OtpData & pOtp->OtpData) != pOtp->OtpData){
        ERRORMSG(TRUE, (_T("Programming OTP failed! expected: 0x%x, actual: 0x%x.\r\n"), pOtp->OtpData, HW_OCOTP_CUSTn_RD(pOtp->OtpAddr)));
    }
    else{
        RETAILMSG(TRUE, (_T("Programming and verifying OTP finished successfully!\r\n")));
    }     
    
    // now restore VDDIO and clocks
    RestoreClks(&savedSettings);

    BW_POWER_VDDIOCTRL_TRG(savedVddio);

    //Wait till operation finishes    
    CheckOtpStatus();    
    return TRUE;
}