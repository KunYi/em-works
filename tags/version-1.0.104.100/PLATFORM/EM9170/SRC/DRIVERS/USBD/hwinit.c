//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  hwinit.c
//  This file contains the functions used to configure the MUX, CPLD and initialize 
//  the USB registers for the USB client driver
//
//
#pragma optimize( "", off )

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include "common_usbname.h"
#include "common_usbcommon.h"

#define HIGH_SPEED      1
#define FULL_SPEED      2


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
// Description: This function configures the IOMUX for USB OTG Core. Senna OTG
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

#ifdef	EM9170
    // CS&ZHL APR-16-2011: setup VBUS
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D11,								// pin name
									DDK_IOMUX_PIN_MUXMODE_ALT5,			// select GPIO4[9] 
									DDK_IOMUX_PIN_SION_REGULAR);			// no SION option on this pin

    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D11,						
									 DDK_IOMUX_PAD_SLEW_SLOW,				// -> the same electrical config as PDK1_7
									 DDK_IOMUX_PAD_DRIVE_HIGH,
									 DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
									 DDK_IOMUX_PAD_PULL_DOWN_100K,
									 DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
									 DDK_IOMUX_PAD_VOLTAGE_3V3);
	
	//DDKGpioSetConfig(..) -> ..\COMMON_FSL_V2_PDK1_7\INC\common_ddk.h
    DDKGpioSetConfig(DDK_GPIO_PORT4,						//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
								9,													//gpio number within a gpio group = 0..31
								DDK_GPIO_DIR_OUT,					//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
								DDK_GPIO_INTR_NONE);				//gpio interrupt = level, edge,none, etc 
	//write 0/1 -> gpio#
    DDKGpioWriteDataPin(DDK_GPIO_PORT4, 9, 1);		//USB_OTG_PWR = 1: deassert VBUS

#else	//->PDK1_7
    // Deassert VBUS
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_A, DDK_IOMUX_PIN_MUXMODE_ALT0, 
                        DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_A,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_HIGH,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_DOWN_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKGpioSetConfig(DDK_GPIO_PORT1, 0, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT1, 0, 1);
#endif	//EM9170
    
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
    DWORD r = TRUE;
    DWORD *temp;
  
    USB_HCSPARAMS_T hcs;
    USB_USBCMD_T cmd;
    USB_USBMODE_T mode;
    USB_PORTSC_T port;
    
    UNREFERENCED_PARAMETER(speed);
      
    // Enable power for OTG on port 1
    temp = (DWORD *)&hcs;
    *temp = INREG32(&regs->OTG.HCSPARAMS);
        
    if (hcs.PPC) {
        WORD USB_OTG_PORTSC1_Write_MASK = (0x1<<1)+(0x1<<3)+(0x1<<5);   
        *temp = INREG32(&regs->OTG.PORTSC[0]);
        *temp &= ~USB_OTG_PORTSC1_Write_MASK;
        *temp &= ~(1<<12);  // PP in device mode should be off, PORT_POWER
        // Seems SETREG32 will not cause problem here, but is ugly to understand, change to OUTREG32
        OUTREG32(&regs->OTG.PORTSC[0], *temp);
    }
        
    // reset the USB OTG port       
    SETREG32(&regs->OTG.USBCMD, (1<<1));
    while((INREG32(&regs->OTG.USBCMD) & (0x1<<1)) == (0x1<<1))
    {
        Sleep(100);
    }

    // Set USB Mode 
    temp=(DWORD *)&mode;
    *temp=0;

    mode.CM = 2;
    mode.SLOM = 1;        // 2.3 hardware and later
    OUTREG32(&regs->OTG.USBMODE, *temp);       

    // otg_itc_setup(0);
    // Eric : we don't know if this is necessary
    temp=(DWORD *)&cmd;
    *temp=INREG32(&regs->OTG.USBCMD);
    cmd.ITC=0;
    OUTREG32(&regs->OTG.USBCMD, *temp);
        
    // otg_frindex_setup(1);
    OUTREG32(&regs->OTG.FRINDEX, 1);

    // otg_set_configflag_on();
    OUTREG32(&regs->OTG.CONFIGFLAG, 1);
       
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
        DWORD * tempPortsc;
        tempPortsc=(DWORD *)&portsc;
        *tempPortsc=INREG32(&regs->OTG.PORTSC);
        // Set PORTSC 
        portsc.PTS=0;       //UTMI Transeiver
#ifdef FORCE_FULLSPEED
        portsc.PFSC=1;  //Port Force Full Speed Connect, test only
#endif
        
        //portsc.WKDC = 1;    // Wake on disconnect enable
        //portsc.WKCN = 1;    // Wake on connect cable

        *tempPortsc|=(2<<10);
        INSREG32(&regs->OTG.PORTSC,(3<<30),*tempPortsc);
    }

    //Sleep(5000); // Failed

    //usb_otg_utmi_interrupt_enable() && usb_otg_power_mask_enable()
    {
        USB_CTRL_T ctrl;
        DWORD * tempCtrl;
        tempCtrl=(DWORD *)&ctrl;
        *tempCtrl=INREG32(&regs->USB_CTRL);

        ctrl.OUIE=0;        // OUIE -- OTG ULPI interrupt enable
        ctrl.OPM=1;     // OTG Power Mask
        ctrl.OWIE = 1;
        SETREG32(&regs->USB_CTRL, *tempCtrl);
    }

    // otg_power_on_port1()
    {
        USB_HCSPARAMS_T hcs1;
        DWORD * tempData;
        tempData=(DWORD *)&hcs1;
        *tempData=INREG32(&regs->OTG.HCSPARAMS);

        if (hcs1.PPC) {
            USB_PORTSC_T portsc;
            tempData=(DWORD *)&portsc;
            *tempData=0;
            portsc.CSC=1; portsc.PEC=1; portsc.OCC=1;
            *tempData=(~*(tempData))&INREG32(&regs->OTG.PORTSC);
            portsc.PP=0;  // don't need to set this in device mode PORT_POWER
            OUTREG32(&regs->OTG.PORTSC, *tempData);
        }                   
    }

    // usb cmd
    {
        // USB_USBCMD_T cmd;
        DWORD * tempCmd;
        tempCmd=(DWORD *)&cmd;
        *tempCmd=INREG32(&regs->OTG.USBCMD);
        cmd.ITC=0x08;
        SETREG32(&regs->OTG.USBCMD, *tempCmd);
    }
    
    // bus config
    {
        USB_SBUSCFG_T buscfg;
        DWORD * tempBuscfg;
        tempBuscfg=(DWORD *)&buscfg;
        *tempBuscfg=INREG32(&regs->OTG.SBUSCFG);
        buscfg.AHBBRST=0;
        OUTREG32(&regs->OTG.SBUSCFG, *tempBuscfg);
    }

    DEBUGMSG(1, (TEXT("InitializeTransceiver done\r\n")));
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

	// CS&ZHL JUN-8-2011: debug only
    RETAILMSG(1, (TEXT("USB HardwareInitialize -> 0x%x\r\n"), (DWORD)regs));

    deviceSpeed = SelectUSBCore(&sel);
    if (deviceSpeed == 0)
        return FALSE;
    if (USBClockInit() && InitializeMux(deviceSpeed)  && 
            InitializeTransceiver(regs,deviceSpeed))
        return TRUE;
    else 
        return FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function: HardwarePullupDP
//
//  This function is being called by CSP code to pull up the D+ if necessary.
//
//  Parameters:
//     regs - Pointer to the 3 USB Core registers.
//     
//  Returns:
//     TRUE - success, FALSE - failure
//
//-----------------------------------------------------------------------------
BOOL HardwarePullupDP(CSP_USB_REGS * regs)
{
    BOOL fSuccess = FALSE;
    USB_PORTSC_T portsc;

    DWORD * temp_portsc=(DWORD*)&portsc;
    *temp_portsc=INREG32(&regs->OTG.PORTSC[0]);
    if (portsc.PTS==2) {
        // Stop USB Controller first
        USB_USBCMD_T cmd;           
        DWORD * temp = (DWORD *)&cmd;
        *temp = INREG32(&regs->OTG.USBCMD);
        cmd.RS = 0;
        OUTREG32(&regs->OTG.USBCMD, *temp);
        while ((INREG32(&regs->OTG.USBCMD) & 0x1) == 0x1){
            //RETAILMSG(1, (TEXT("USBCMD= 0x%x\r\n"), (INREG32(&(*regs)->OTG.USBCMD))));
            Sleep(100);
        }


//        fSuccess = HardwarePullupDP1504(regs);

//        DEBUGMSG(ZONE_FUNCTION, (L"ISP1504 High Speed used with success %d\r\n", fSuccess));
        // Start USB Controller first
        if (fSuccess)
        {
            temp = (DWORD *)&cmd;
            *temp = INREG32(&regs->OTG.USBCMD);
            cmd.RS = 1;
            OUTREG32(&regs->OTG.USBCMD, *temp);
            while ((INREG32(&regs->OTG.USBCMD) & 0x1) == 0x0){
                //RETAILMSG(1, (TEXT("USBCMD= 0x%x\r\n"), (INREG32(&(*regs)->OTG.USBCMD))));
                Sleep(100);
            }
        }
    }
    else {
        RETAILMSG(1, (L"ISP1301 Full Speed is not implemented\r\n"));
        DEBUGCHK(FALSE);
    }

    //DumpUSBRegs(regs, 2);
    return fSuccess;
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

#pragma optimize( "", on )
