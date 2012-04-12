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
//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  hwinit.c
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4214)
#include <windows.h>
#include <oal.h>
#include "bsp.h"
#include <ethdbg.h>

extern BSP_ARGS *g_pBSPArgs;

void GetRNDISMACAddress(UINT16 mac[3])
{
     memcpy( mac, g_pBSPArgs->kitl.mac, 6);
     mac[0] |= 2;
}

BOOL controllerSetup(CSP_USB_REGS* regs)
{
    //UNREFERENCED_PARAMETER(regs);


    USB_USBMODE_T mode;
    DWORD * temp;

    USB_USBCMD_T cmd;
    USB_USBSTS_T status;
    USB_PORTSC_T portsc;

    //Stop the controller first
    temp = (DWORD *)&cmd;
    *temp = INREG32(&regs->OTG.USBCMD);
    cmd.RS = 0;
    OUTREG32(&regs->OTG.USBCMD, *temp);  
    while ((INREG32(&regs->OTG.USBCMD) & 0x1) == 0x1) ;

    // reset the usb otg
    temp = (DWORD *)&cmd;
    *temp = INREG32(&regs->OTG.USBCMD);
    cmd.RST = 1;
    OUTREG32(&regs->OTG.USBCMD, *temp);
    while ((INREG32(&regs->OTG.USBCMD) & 0x1<<1) == (0x1<<1)) ;

    //
    // Set USB Mode
    temp=(DWORD *)&mode;
    *temp=2;
    OUTREG32(&regs->OTG.USBMODE, *temp);
    while ((INREG32(&regs->OTG.USBMODE) != 2)) ;

    // Reset all interrupts
    temp  =(DWORD*)&status;
    *temp=0;
    OUTREG32(&regs->OTG.USBSTS, *temp);

    //
    //set UTMI
    temp=(DWORD *)&portsc;
    *temp=INREG32(&regs->OTG.PORTSC);
    // Set PORTSC
    if( portsc.PTS != 0)
    {
        portsc.PTS=0;                   //Set Transeiver to UTMI
        INSREG32(&regs->OTG.PORTSC,(3<<30),*temp);
    }
    OUTREG32(&regs->OTG.USBINTR,  0x157);

    return TRUE;

}


//-----------------------------------------------------------------------------
//
//  Function: InitializeTransceiver
//
//  This function is to configure the USB core
//
//  Parameters:
//     regs - pointer to the registers of the OTG USB core
//     speed - the speed of the OTG USB Core to be configured.
//
//  Returns:
//     TRUE - success, FALSE - failure
//
//-----------------------------------------------------------------------------
static BOOL InitializeTransceiver(CSP_USB_REGS* regs)
{
    
    DWORD r=TRUE;
    DWORD * temp;
    PVOID pv_HWregPOWER;
    PVOID pv_HWregUSBPhy0;
    PVOID pv_HWregUSBPhy1;

    pv_HWregPOWER = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_POWER);

    pv_HWregUSBPhy0 = (PVOID) OALPAtoCA(CSP_BASE_REG_PA_USBPHY0);

    pv_HWregUSBPhy1 = (PVOID) OALPAtoCA(CSP_BASE_REG_PA_USBPHY1);


    HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_SFTRST);
    HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_CLKGATE);

    // Soft Reset the PHY
    HW_USBPHY_CTRL_SET(0,BM_USBPHY_CTRL_SFTRST);
    OALStall(10000);

    //clear the soft reset
    HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_SFTRST);
    OALStall(10000);
    
    HW_USBPHY_CTRL_CLR(0,BM_USBPHY_CTRL_CLKGATE);
    OALStall(10000);
 

    // Clear the power down bits
    HW_USBPHY_PWD(0).U = 0;

    // clear to run clocks
    HW_USBPHY_DEBUG(0).B.CLKGATE = 0;

    // Not a host, so disable the host 15 KOhm pulldown resistors
    // on D+ and D-.
    HW_USBPHY_DEBUG(0).B.ENHSTPULLDOWN = 0x3;
    HW_USBPHY_DEBUG(0).B.HSTPULLDOWN   = 0x0;

    // reset the usb otg
    {
        USB_USBCMD_T cmd;
        temp = (DWORD *)&cmd;
        *temp = INREG32(&regs->OTG.USBCMD);
        cmd.RST = 1;
        OUTREG32(&regs->OTG.USBCMD, *temp);
        while ((INREG32(&regs->OTG.USBCMD) & 0x1<<1) == (0x1<<1)){
            //RETAILMSG(1, (TEXT("USBCMD= 0x%x\r\n"), (INREG32(&(*regs)->OTG.USBCMD))));
        }
    }
    //HW_POWER_CTRL_CLR(BM_POWER_CTRL_CLKGATE);
    HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_PWRUP_VBUS_CMPS);

    {
        // When using USB Serial Download, we should add 1s delay 
        // for success kitl connection afterwards
        DWORD dwCurSec,timeOut;
        timeOut = 1; //delay 1 s.
        dwCurSec = OEMEthGetSecs();
        while((OEMEthGetSecs() - dwCurSec) <= timeOut);
    }

    controllerSetup( regs);

    OALMSG(0, (L"PHY CTRL 0x%x\r\n", HW_USBPHY_CTRL_RD(0)));
    OALMSG(0, (L"PHY Power 0x%x\r\n", HW_USBPHY_PWD_RD(0)));
    OALMSG(0, (L"PHY TX 0x%x\r\n", HW_USBPHY_TX_RD(0)));
    OALMSG(0, (L"PHY RX 0x%x\r\n", HW_USBPHY_RX_RD(0)));
    OALMSG(0, (L"PHY Status 0x%x\r\n", HW_USBPHY_STATUS_RD(0)));
    OALMSG(0, (L"PHY Debug 0x%x\r\n", HW_USBPHY_DEBUG_RD(0)));
    OALMSG(0, (L"UTMI Debug 0 0x%x\r\n", HW_USBPHY_DEBUG0_STATUS_RD(0)));
    OALMSG(0, (L"UTMI Debug 1 0x%x\r\n", HW_USBPHY_DEBUG1_RD(0)));

 
    return r;

}

//-------------------------------------------------------------
//
// Function: USBClockInit
//
// Init and start the USB Core Clock
//
// Parameter: NULL
//
// Return:
//   TRUE - success to start the clock
//   FALSE - fail to start the clock
//
//-------------------------------------------------------------
BOOL USBClockInit(void)
{
    PVOID pv_HWregCLKCTRL;
    PVOID pv_HWregDIGCTL;

    pv_HWregCLKCTRL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_CLKCTRL);
    pv_HWregDIGCTL  = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_DIGCTL);

    // Enable 8-phase PLL outputs for USB PHY
    HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_EN_USB_CLKS);

    // if gated, ungate clock to enable block
    HW_DIGCTL_CTRL.B.USB0_CLKGATE = 0;

    OALMSG(0, (L"PLL CTRL 0x%x 0x%x\r\n",  CSP_BASE_REG_PA_CLKCTRL, HW_CLKCTRL_PLL0CTRL0_RD()));
    OALMSG(0, (L"DIG CTRL 0x%x 0x%x \r\n", CSP_BASE_REG_PA_DIGCTL, HW_DIGCTL_CTRL_RD()));

    return TRUE;    
}

//------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Function: HardwareInitialize
//
//  This function is being called by CSP code to initialize the transceiver, USB core
//  and CPLD
//
//  Parameters:
//     regs - Pointer to the 3 USB Core registers.
//
//  Returns:
//     TRUE - success, FALSE - failure
//
//-----------------------------------------------------------------------------

BOOL HardwareInitialize(CSP_USB_REGS * regs)
{
    OALMSG(0, (L"\r\n++HardwareInitialize\r\n"));

    //pv_HWregUSBPhy0 = (PVOID)OALPAtoUA(CSP_BASE_REG_PA_USBPHY0);
    if (USBClockInit() && InitializeTransceiver(regs))
        return TRUE;

    OALMSG(0, (L"-- ERROR :: HardwareInitialize\r\n"));
    return FALSE;

}

//------------------------------------------------------------------------------
// Function: PowerDownSchemeExist
// 
// Description: This function is called to check if we can stop the system clock
//              and PHY clock, it is determined by IC capability, if VBUS and ID
//              interrupt without clock can wake up the system, we can enter 
//              low power mode, else we can't
//
//              while in USB KITL, we always return FALSE
//------------------------------------------------------------------------------
BOOL PowerDownSchemeExist(void)
{
    return FALSE;
}

//------------------------------------------------------------------------------
// Function: BSPUsbfnDetachNeedRSSwitch
// 
// Description: This function is called to check if we need to close and reopen 
//              USBCMD.RS to put port back to original mode
//              needed in MX28
//
//------------------------------------------------------------------------------
BOOL BSPUsbfnDetachNeedRSSwitch(void)
{
    return FALSE;
}

//------------------------------------------------------------------------------
// Function: BSPUsbPhyEnterLowPowerMode
// 
// Description: This function is called to enable/disable VBUS interrupt
//     blEnable
//         [IN] TRUE - enable, FALSE - disable
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbPhyEnterLowPowerMode(PUCHAR baseMem, BOOL blEnable)
{
    UNREFERENCED_PARAMETER(baseMem);
    UNREFERENCED_PARAMETER(blEnable);
}

//-----------------------------------------------------------------------------
//
//  Function: BSPUsbResetPHY
//
//  This is called by the public common CSP library to reset USB phy.
//
//  Parameters:
//      regs - Pointer to the 3 USB Core registers.
//
//  Returns:
//      
//
//-----------------------------------------------------------------------------
void BSPUsbResetPHY(CSP_USB_REGS * regs)
{
    UNREFERENCED_PARAMETER(regs);     
}

//------------------------------------------------------------------------------
// Function: BSPUsbSetBusConfig
// 
// Description: This function is called to set proper AHB BURST Mode
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbSetBusConfig(PUCHAR baseMem)
{
    // MX25 Set to '0'
    UNREFERENCED_PARAMETER(baseMem);
}

//------------------------------------------------------------------------------
// Function: BSPUsbSetCurrentLimitation
// 
// Description: This function is called to set proper current limitaion 
//
// Parameters 
//      bLimitOn : BOOL, true to turn on current limitation, false to turn off limitation
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbSetCurrentLimitation(BOOL bLimitOn)
{
    UNREFERENCED_PARAMETER(bLimitOn);
}

//------------------------------------------------------------------------------
// Function: BSPUSBInterruptControl
//
// This function is used to register USB interrupt as system wakeup source or NOT
//
// Parameters:
//          dwIOCTL  -- IOCTL_HAL_ENABLE_WAKE or IOCTL_HAL_DISABLE_WAKE
//          pSysIntr -- pointer to sys interrupt
//          dwLength -- size of pSysIntr
//     
// Returns:
//      N/A
//     
//
//------------------------------------------------------------------------------
void BSPUSBInterruptControl(DWORD dwIOCTL, PVOID pSysIntr, DWORD dwLength)
{
    UNREFERENCED_PARAMETER(dwIOCTL);
    UNREFERENCED_PARAMETER(pSysIntr);
    UNREFERENCED_PARAMETER(dwLength);
    return;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPPhyShowDevDiscon
//  
//  Description : This function checks USBPHY if disconnect happens in peripheral mode
//
//  Retrun : TRUE means disconnect
//           FALSE means not disconnect
//
//------------------------------------------------------------------------------
BOOL BSPPhyShowDevDiscon(void)
{
    return FALSE;
}

#ifndef DEBUG
//----------------------------------------------------------------
// 
//  Function:  DumpUSBRegs
//
//  Dump all the registers in MX31 usb memory map
//
//  Parameter:
//      pUSBDRegs - pointer to usb memory map
//
//  Return
//     NULL
//
//----------------------------------------------------------------
void DumpUSBRegs(PUCHAR baseMem)
{
    UNREFERENCED_PARAMETER(baseMem);
}
#endif
//-----------------------------------------------------------------------------
//
//  Function: BSPUsbPhyRegDump
//  
//  Description : This function dumps PHY Control Regs for some specific IC
//
//------------------------------------------------------------------------------
void BSPUsbPhyRegDump(void)
{
    return;
}
//----------------------------------------------------------------
// 
//  Function:  DumpUsbExternalRegs
//
//  Dump external registers, such as clock, power, iomux, etc
//
//  Parameter:
//
//  Return
//     NULL
//
//----------------------------------------------------------------
void DumpUsbExternalRegs(void)
{
    return;
}
