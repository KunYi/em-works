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
//
// Copyright (c) Intrinsyc Corporation.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File:  hwinit.c
//
#include <windows.h>
#include <oal.h>
#include <omap2420.h>

#define DEVICETRANCIEVER 1      // configure for Device Transceiver

#if DEVICETRANCIEVER
#define USBX_TRX_MODE 3          // 6-pin unidirectional transceiver signaling
#else
#define USBX_TRX_MODE 2         // 3-pin bidirectional (DAT_SE0) mode transceiver
                                // signaling
#endif

//------------------------------------------------------------------------------
BOOL InitOTGTransceiver();

//------------------------------------------------------------------------------
BOOL InitSystemControl()
{
#if 0
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+USBFN:: Initialize System Control\r\n"));
    {
        DWORD temp, mode;
        OMAP2420_SYSC1_REGS *pSysConRegs = OALPAtoUA(OMAP2420_SYSC1_REGS_PA);
    
        temp = INREG32(&pSysConRegs->ulCONTROL_DEVCONF);

        mode = USBX_TRX_MODE;

        // Clear USBT0WRIMODEI. This places the USB Controls in Unidirectional Mode
        temp &= 0xFF3FFFFF;

        // Set the Transceiver Interface Mode for USB Port 0
        if ((mode == 0x01) || (mode == 0x02))
        {
            // Change the mode to Bidirectional.
            temp |= 0x00800000;
        }

        // Make sure the USB Enable signal is being used as an Active-High signal
        temp &= 0xFFFEFFFF;

        // Make sure the USB module standby signal is not asserted
        temp &= 0xFFFF7FFF;

        OUTREG32(&pSysConRegs->ulCONTROL_DEVCONF, temp);
    
    }
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-USBFN:: Initialize System Control\r\n"));
#endif
    return TRUE;
}

//------------------------------------------------------------------------------

BOOL InitOTGController()
{
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+USBFN:: Initialize OTG Controller\r\n"));
    {
        DWORD temp;
        OMAP2420_OTG_REGS *pOTGRegs = OALPAtoUA(OMAP2420_OTG_REGS_PA);
    
        // Reset all USB controllers
        OUTREG32(&pOTGRegs->SYSCON_1, OTG_SYSCON_1_SOFT_RESET);    
        
        // Wait for the reset to complete.
        while ((INREG32(&pOTGRegs->SYSCON_1) & OTG_SYSCON_1_RESET_DONE) == 0);
        
        // Set USBx_TRX_MODE
        // Note that the H4 System's two USB transceivers will only work in
        // one of the three available transceiver modes. The USB OTG transceiver
        // (ISP1301BS) will only work in the 3-pin bidirectional mode. The USB
        // Device Transceiver will only work in the 6-pin unidirectional mode.
        temp= (USBX_TRX_MODE << 24) | (USBX_TRX_MODE << 20) | (USBX_TRX_MODE << 16); 
        OUTREG32(&pOTGRegs->SYSCON_1, temp);
        {
            temp = INREG32(&pOTGRegs->SYSCON_2);
            OUTREG32(&pOTGRegs->SYSCON_2, 0x5B240080);      // USB_DET is not connect correctly, so try OTG for BSESSVLD
        
            // We don't get any interrupts from the USB Device unless this bit
            // is set. Unfortunately, setting this bit also sets the "USB
            // Cable Attached" bit, which remains set regardless of whether or
            // not a cable is actually attached, and thus prevents us from
            // being able to properly tell when a USB cable is connected to or
            // disconnected from the device.
            // The OMAP2420 TRM suggest having an additional hardware mechanism
            // that would allow the driver to detect when a cable is connected
            // and then set the BSESSVLD bit as appropriate. The H4 System
            // design lacks this feature, so we'll have to use a workaround in
            // the USB Driver PDD to compensate for this.        
            OUTREG32(&pOTGRegs->CTRL, OTG_CTRL_BSESSVLD);
        }   
    }
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-USBFN:: Initialize OTG Controller\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------

BOOL InitClockController()
{
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+USBFN:: Initialize USB Clock Mgr\r\n"));
    {
        OMAP2420_PRCM_REGS * pPRCMRegs = OALPAtoUA(OMAP2420_PRCM_REGS_PA);
        
#if 0
        // Disable the USB Interface clock
        CLRREG32(&pPRCMRegs->ulCM_ICLKEN2_CORE,   0x00000001);    // Clear EN_USB
#endif    
        // Configure the USB Interface Clock Speed
        CLRREG32(&pPRCMRegs->ulCM_CLKSEL1_CORE,   0x0E000000);    // Clear clk = L3_CLK/1 (boot mode only)
        SETREG32(&pPRCMRegs->ulCM_CLKSEL1_CORE,   0x08000000);    // Set   clk = L3_CLK/3
    
#if 0    
        // Ensure that the USB Interface clock remains active when the MPU enters Idle Mode.
        CLRREG32(&pPRCMRegs->ulCM_AUTOIDLE2_CORE, 0x00000001);    // Clear AUTO_USB
    
        // Enable the USB Interface clock
        SETREG32(&pPRCMRegs->ulCM_ICLKEN2_CORE,   0x00000001);    // Set   EN_USB
    
        // Enable the USB Functional clock
        SETREG32(&pPRCMRegs->ulCM_FCLKEN2_CORE,   0x00000001);    // Set   EN_USB
    
        // Enable USB Wake-Up
        SETREG32(&pPRCMRegs->ulPM_WKEN2_CORE,     0x00000001);    // Set   EN_USB
#endif        
    }
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-USBFN:: Initialize USB Clock Mgr\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------

BOOL InitializeHardware()
{
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+USBFN:: Initialize Hardware\r\n"));
    // Configure clock controller
    InitClockController();
    // Configure OTG controller
    InitOTGController();
    // Configure System
    InitSystemControl();
    // Done
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-USBFN:: Initialize Hardware\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------

DWORD GetUniqueDeviceID()
{
    /* TODO : need to find unique silicon id
    DWORD dwID0 = 0;

    OMAP2420_DEVICE_ID_REGS *pDeviceIDRegs = OALPAtoUA(OMAP2420_DEVICE_ID_REGS_PA);

    dwID0 = INREG32(&pDeviceIDRegs->OMAP_DIE_ID_0);
    dwID1 = INREG32(&pDeviceIDRegs->OMAP_DIE_ID_1);
    
    return dwID0 ^ dwID1;
    */
    
    return 0x0B5D902F;  // hardcoded id
}

//------------------------------------------------------------------------------

