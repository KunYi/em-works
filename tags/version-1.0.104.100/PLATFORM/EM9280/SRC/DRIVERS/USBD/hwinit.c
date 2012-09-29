//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  hwinit.c
//  This file contains the functions used to configure the MUX, CPLD and initialize
//  the USB registers for the USB client driver
//
//------------------------------------------------------------------------------

#pragma optimize( "", off )

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include "mx28_usbname.h"
#include "mx28_usbcommon.h"
#include "mx28_ddk.h"
#include "common_usbcommon.h"

#define HIGH_SPEED      1
#define FULL_SPEED      2

extern void BSPUsbPhyStartUp(void);
extern void BSPUSBSwitchModulePower(BOOL bOn);

//-----------------------------------------------------------------------------
//
//  Function: BSPGetUSBControllerType
//
//  This is used by the public common CSP library to be used to return the USB controller
//  currently using.
//
//  Parameters:
//      None.
//
//  Returns:
//      USB_SEL_OTG, USB_SEL_H2 or USB_SEL_H1. In this case, it would be USB_SEL_OTG
//
//-----------------------------------------------------------------------------
WORD BSPGetUSBControllerType(void)
{
    return USB_SEL_OTG;
}

//------------------------------------------------------------------------------
// Function: InitializeMux
//
// Description: This function configures the IOMUX for USB OTG Core. Ringo OTG
//              takes its internal UTMI+ trasceiver as default, so only OTG_PWR
//              and OTG_OC pins are configured.
//
// Parameters:
//     speed
//         [IN] The speed of the device to configure as OTG can be configured as
//              HS or FS. Currently only HS is used and as such the parameter
//              is not used.
//
// Returns:
//     TRUE for success, FALSE if failure.
//
//------------------------------------------------------------------------------
static BOOL InitializeMux(int speed)
{
    UNREFERENCED_PARAMETER(speed);

    // On OTG port usbfn driver may be switched from host driver,
    // in that case VBUS may still be configured as 5V, need to
    // do a forcefully shutdown here
#ifdef	EM9280
	// use GPIO1_1 as USB0_PWR_EN
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D1, DDK_IOMUX_MODE_GPIO); 
    DDKGpioEnableDataPin(DDK_IOMUX_LCD_D1, 1);
    DDKGpioWriteDataPin(DDK_IOMUX_LCD_D1, 0); // turn off VBUS
#else	// -> EM9283 or iMX28EVK
#ifdef EM9283
	DDKIomuxSetPinMux(DDK_IOMUX_LCD_D0, DDK_IOMUX_MODE_GPIO); 
	DDKGpioEnableDataPin(DDK_IOMUX_LCD_D0, 1);
	DDKGpioWriteDataPin(DDK_IOMUX_LCD_D0, 0); // turn off VBUS
#else
	DDKIomuxSetPinMux(DDK_IOMUX_AUART2_TX_1, DDK_IOMUX_MODE_GPIO); 
    DDKGpioEnableDataPin(DDK_IOMUX_AUART2_TX_1, 1);
    DDKGpioWriteDataPin(DDK_IOMUX_AUART2_TX_1, 0); // turn off VBUS
#endif   //EM9283
#endif	//EM9280

    return TRUE;
}

//------------------------------------------------------------------------------
// Function: SelectUSBCore
//
// Description: This function is to configure the CPLD to corresponding USB core
//
// Parameters:
//     sel
//         [OUT] The value in which USB core to configure. Since FS OTG doesn't
//               need to be supported, this parameter is not used.
//
// Returns:
//     Speed of the USB core - HIGH_SPEED or FULL_SPEED
//
//------------------------------------------------------------------------------
static int SelectUSBCore(WORD * sel)
{
    int CoreSpeed;

    UNREFERENCED_PARAMETER(sel);

    {
        DEBUGMSG(ZONE_FUNCTION, (L"High speed USB client\r\n"));
        CoreSpeed = HIGH_SPEED;
    }

    return CoreSpeed;
}

//-----------------------------------------------------------------------------
//
//  Function: InitializeTransceiver
//
//  This function is to configure the USB core
//
//  Parameters:
//     regs - pointer to the registers for all three USB core
//     speed - the speed of the OTG USB Core to be configured.
//     if full speed, it is using the ISP1301, otherwise it is ISP1504
//
//  Returns:
//     TRUE - success, FALSE - failure
//
//-----------------------------------------------------------------------------
static BOOL InitializeTransceiver(CSP_USB_REGS* regs, int speed)
{
    DWORD r=TRUE;
    DWORD * temp;
    USB_USBMODE_T mode;
    USB_PORTSC_T port;

    UNREFERENCED_PARAMETER(speed);
    RETAILMSG(FALSE, (TEXT("InitializeTransceiver++\r\n")));

    BSPUsbPhyStartUp();

    BSPUSBSwitchModulePower(TRUE);

    // reset the USB OTG port
    SETREG32(&regs->OTG.USBCMD, (1<<1));
    while((INREG32(&regs->OTG.USBCMD) & (0x1<<1)) == (0x1<<1))
    {
        Sleep(100);
    }

    {
        // Set USB Mode
        temp = (DWORD *)&mode;
        *temp=0;

        mode.CM = 2;
        //mode.SLOM=1;                // 2.3 hardware and later
        OUTREG32(&regs->OTG.USBMODE, *temp);
    }

    // Port reset
    temp = (DWORD *)&port;
    *temp = INREG32(&regs->OTG.PORTSC[0]);
    while (port.PR) {
        *temp = 0;
        port.PR = 1;
        CLRREG32(&regs->OTG.PORTSC[0], *temp);
        *temp = INREG32(&regs->OTG.PORTSC[0]);
    }

    {
        USB_PORTSC_T portsc;
        temp = (DWORD *)&portsc;
        *temp = INREG32(&regs->OTG.PORTSC);
        // Set PORTSC
        portsc.PTS = 0;       //UTMI Transeiver
        //portsc.PFSC=1;        //Port Force Full Speed Connect, test only

        *temp |= (2<<10);
        INSREG32(&regs->OTG.PORTSC,(3<<30),*temp);
    }

    // usb cmd
    {
        USB_USBCMD_T cmd;
        temp = (DWORD *)&cmd;
        *temp = INREG32(&regs->OTG.USBCMD);
        cmd.ITC = 0x08;
        SETREG32(&regs->OTG.USBCMD, *temp);
    }

    // The Bus Config is set in "BSPUsbSetBusConfig"

    RETAILMSG(FALSE, (L"InitializeTransceiver--\r\n"));

    return r;
}

//------------------------------------------------------------------------------
// Function: HardwareInitialize
//
// Description: This function is being called by SOC code to initialize the
//              transceiver and USB core.
//
// Parameters:
//     regs
//         [IN] Points to USB related registers mapped to virtual address
//
// Returns:
//     TRUE for success, FALSE if failure
//
//------------------------------------------------------------------------------
BOOL HardwareInitialize(CSP_USB_REGS * regs)
{
    int deviceSpeed;
    WORD sel;

    deviceSpeed = SelectUSBCore(&sel);
    if (deviceSpeed == 0)
        return FALSE;
    RETAILMSG(FALSE, (L"HardwareInitialize\r\n"));
    if (USBClockInit() && InitializeMux(deviceSpeed)  &&
        InitializeTransceiver(regs,deviceSpeed))
        return TRUE;
    else
        return FALSE;
}
//------------------------------------------------------------------------------
// Function: GetSysIntr
//
// Description: This function is to return the SYSINTR value being used by OTG
//              USB core
//
// Parameters:
//     NONE
//
// Returns:
//     SYSINTR of USB OTG being used
//
//------------------------------------------------------------------------------
DWORD GetSysIntr(void)
{
    return SYSINTR_USBOTG;
}

#pragma optimize( "", on )
