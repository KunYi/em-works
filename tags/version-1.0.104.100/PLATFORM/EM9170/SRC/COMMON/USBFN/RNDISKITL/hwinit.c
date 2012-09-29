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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
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
#include "common_usbcommon.h"

extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);
extern VOID OALClockDisableAllGatedClock(VOID);

static PCSP_IOMUX_REGS pUSBIOMUX = NULL;
extern BSP_ARGS *g_pBSPArgs;

void GetRNDISMACAddress(UINT16 mac[3])
{
    //memcpy( mac, g_EbootCFG.mac, 6);
    memcpy( mac, g_pBSPArgs->kitl.mac, 6);
    mac[0] |= 2;
}

//-----------------------------------------------------------------------------
//
//  Function: InitializeMux
//
//  This function is to configure the IOMUX
//  can safely share access.
//
//  Parameters:
//     speed - the speed of the device to configure as OTG can be configured as 
//     HS or FS .  Currently only HS is used and as such the parameter is not used.
//
//  Returns:
//      TRUE - success, FALSE - failure.
//
//-----------------------------------------------------------------------------
static BOOL InitializeMux()
{
    // IOMUX is not needed as internal UTMI PHY is used.
    return TRUE;
}

BOOL controllerSetup(CSP_USB_REGS* regs)
{
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
    while ((INREG32(&regs->OTG.USBCMD) & 0x1) == 0x1);  
    // reset the usb otg   
    temp = (DWORD *)&cmd;
    *temp = INREG32(&regs->OTG.USBCMD);
    cmd.RST = 1;
    OUTREG32(&regs->OTG.USBCMD, *temp);
    while ((INREG32(&regs->OTG.USBCMD) & 0x1<<1) == (0x1<<1));
        
    // Set USB Mode 
    temp=(DWORD *)&mode;
    *temp=2;
    OUTREG32(&regs->OTG.USBMODE, *temp);    
    while ((INREG32(&regs->OTG.USBMODE) != 2));
    //mode.SLOM=1;        // 2.3 hardware and later
    //OUTREG32(&regs->OTG.USBMODE, *temp);  

     // Reset all interrupts           
    temp  =(DWORD*)&status;
    *temp=0;            
    OUTREG32(&regs->OTG.USBSTS, *temp);
      
    //set UTMI
    temp=(DWORD *)&portsc;
    *temp=INREG32(&regs->OTG.PORTSC);
    // Set PORTSC 
    if( portsc.PTS != 0)
    {
        portsc.PTS=0;       //Set Transeiver to UTMI               
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

    USB_CTRL_T ctrl;
    temp=(DWORD *)&ctrl;
    *temp = INREG32(&regs->USB_CTRL);

    ctrl.OUIE = 0;      // Not meaningful since UTMI transceiver is selected
    ctrl.OPM = 1;       // OTG Power Mask
    ctrl.OWIE = 1;

    SETREG32(&regs->USB_CTRL, *temp);

    {
        USB_PORTSC_T portsc;
        temp=(DWORD *)&portsc;
        *temp=INREG32(&regs->OTG.PORTSC);
        // Set PORTSC 
        portsc.PTS=0;       //Set Transeiver to UTMI        
        // first mask out the PTS field, then set all the above bits
        INSREG32(&regs->OTG.PORTSC,(3<<30),*temp);
    }   

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
    {
        DWORD dwCurSec,timeOut;
        timeOut = 1; //delay 1 s.
        dwCurSec = OEMEthGetSecs();
        while((OEMEthGetSecs() - dwCurSec) <= timeOut);
    }

    controllerSetup( regs);
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
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_USBOTG, DDK_CLOCK_GATE_MODE_ENABLED);
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
    pUSBIOMUX = (PCSP_IOMUX_REGS) OALPAtoUA(CSP_BASE_REG_PA_IOMUXC);
    
    if (USBClockInit() && 
         InitializeMux()  && InitializeTransceiver(regs))
        return TRUE;

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
    DWORD temp;
    CSP_USB_REGS* pUSBRegs = (CSP_USB_REGS*)baseMem;

    if(blEnable)
    {
        temp = INREG32(&pUSBRegs->USB_CTRL);
        temp |= (1<<15);
    }
    else
    {
        temp = INREG32(&pUSBRegs->USB_CTRL);
        temp &= (~(1<<15));
    }
    OUTREG32(&pUSBRegs->USB_CTRL, temp);
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
    CSP_USB_REG* pUSBReg = (CSP_USB_REG*)baseMem;
    CLRREG32(&pUSBReg->SBUSCFG, 0x7);
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
