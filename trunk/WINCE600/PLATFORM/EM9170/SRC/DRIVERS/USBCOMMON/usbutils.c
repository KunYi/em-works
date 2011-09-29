//-----------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
// File:      
//     USBUtils.c
// Purpose:   
//     Maintains misc utils for specific IP
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include "common_usbcommon.h"

extern WORD BSPGetUSBControllerType(void);
//------------------------------------------------------------------------------
// Function: PowerDownSchemeExist
// 
// Description: This function is called to check if we can stop the system clock
//              and PHY clock, it is determined by IC capability, if VBUS and ID
//              interrupt without clock can wake up the system, we can enter 
//              low power mode, else we can't
//
//------------------------------------------------------------------------------
BOOL PowerDownSchemeExist(void)
{
    // Get TO version
    // IN MX25, 0x00 TO1 
    DWORD dwSiVer;

    if (!KernelIoControl(IOCTL_HAL_QUERY_SI_VERSION, NULL, 0,
        &dwSiVer, sizeof(dwSiVer), NULL))
    {
        ERRORMSG(1, (_T("Cannot obtain the silicon version!\r\n")));
        return FALSE;
    }

    if (dwSiVer == 0)
    {
        // TO1 don't support
        return FALSE;
    }
    else
    {
        return TRUE;
    }
    // TODO need to examine What TO2 indicate
}
//------------------------------------------------------------------------------
// Function: BSPUsbXvrEnableVBUSIntr
// 
// Description: This function is called to enable/disable VBUS interrupt
//     blEnable
//         [IN] TRUE - enable, FALSE - disable
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbXvrEnableVBUSIntr(PUCHAR baseMem, BOOL blEnable)
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


//------------------------------------------------------------------------------
// Function: BSPUsbXvrEnableIDIntr
// 
// Description: This function is called to enable/disable ID interrupt
//
//     blEnable
//         [IN] TRUE - enable, FALSE - disable
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbXvrEnableIDIntr(PUCHAR baseMem, BOOL blEnable)
{
    DWORD temp;
    CSP_USB_REGS* pUSBRegs = (CSP_USB_REGS*)baseMem;

    if(blEnable)
    {
        temp = INREG32(&pUSBRegs->USB_CTRL);
        temp |= (1<<14);
    }
    else
    {
        temp = INREG32(&pUSBRegs->USB_CTRL);
        temp &= (~(1<<14));
    }
    OUTREG32(&pUSBRegs->USB_CTRL, temp);
}

//------------------------------------------------------------------------------
// Function: BSPUsbPhyEnterLowPowerMode
// 
// Description: This function is called to enable/disable interrupt available 
//              in low power mode
//
//     blEnable
//         [IN] TRUE - enable, FALSE - disable
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbPhyEnterLowPowerMode(PUCHAR baseMem, BOOL blEnable)
{
    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        BSPUsbXvrEnableVBUSIntr(baseMem, blEnable);
        BSPUsbXvrEnableIDIntr(baseMem, blEnable);
    }
    // for HSH2, we have ULPI interrupt
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
    // Should clarify with IC if MX25 need to use this
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
    // only register USB interrupt as system wakeup source when macro
    // USB_WAKEUP_CNANDCN or USB_WAKEUP_CN is defined. 
    if(dwIOCTL == IOCTL_HAL_ENABLE_WAKE)
    {
#if defined USB_WAKEUP_CNANDDN || defined USB_WAKEUP_CN
        KernelIoControl(dwIOCTL, pSysIntr, dwLength, NULL, 0, NULL);
#endif
    }
    else if(dwIOCTL == IOCTL_HAL_DISABLE_WAKE)
    {
        KernelIoControl(dwIOCTL, pSysIntr, dwLength, NULL, 0, NULL);
    }
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

//-----------------------------------------------------------------------------
//
//  Function: BSPUsbPhyStartUp
//  
//  Description : This function configure PHY to working mode
//
//------------------------------------------------------------------------------
void BSPUsbPhyStartUp(void)
{
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

//-----------------------------------------------------------------------------
//
//  Function: BSPUsbPhyRegDump
//  
//  Description : This function dumps PHY Control Regs for some specific IC
//
//------------------------------------------------------------------------------
void BSPUsbPhyRegDump(void)
{
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
//  Example : DumpUSBRegs((PUCHAR)(m_capBase - m_dwOffset))
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
//  Function: BSPUsbPhyExit
//  
//  Description : not necessary in this BSP
//
//------------------------------------------------------------------------------
void BSPUsbPhyExit(void)
{
}

//-----------------------------------------------------------------------------
//
//  Function: BSPUSBSwitchModulePower
//  
//  Description : This function switch Power to USB Module 
//
//------------------------------------------------------------------------------
void BSPUSBSwitchModulePower(BOOL bOn)
{
    UNREFERENCED_PARAMETER(bOn);
}

//-----------------------------------------------------------------------------
//  Function: BSPHostDisconDetect
//
//  Description : This function switch USBPHY HS disconnection detect for host
//
//------------------------------------------------------------------------------
void BSPHostDisconDetect(BOOL bOn)
{
    UNREFERENCED_PARAMETER(bOn);
}

//------------------------------------------------------------------------------
// Function: BSPUsbhPutPhySuspend
// 
// Description: This function is not implemented on MX25
//
// Parameters: 
//
// Return: 
//------------------------------------------------------------------------------
void BSPUsbhPutPhySuspend(PUCHAR baseMem, BOOL bOn)
{
    UNREFERENCED_PARAMETER(baseMem);
    UNREFERENCED_PARAMETER(bOn);
}

//------------------------------------------------------------------------------
// Function: BSPUsbhCheckPhyResume
// 
// Description: return FALSE on MX25, ignore this logic
//
// Parameters: 
//
// Return: 
//------------------------------------------------------------------------------
BOOL BSPUsbhCheckPhyResume(void)
{
    return FALSE;
}

//------------------------------------------------------------------------------
// Function: IsResetNeed
// 
// Description: return FALSE except mx233, ignore this logic
//
// Parameters: 
//
// Return: 
//------------------------------------------------------------------------------
BOOL IsResetNeed(void)
{
    return FALSE;
}
