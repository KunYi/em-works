/*---------------------------------------------------------------------------
* Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/


/*
 *  File:     hwinit.c
 *  Purpose:  Platform-specific host driver initialisation and configuration
 *            Pin muxing
 *            CPLD configuration
 *            transceiver, wakeup, Vbus configuration
 *
 *  Functions:  BSPUsbhCheckConfigPower  - verify power available for devices
 *              BSPUsbSetWakeUp          - setup wakeup sources for host port
 *              InitializeTransceiver    - any setup required for attached transceiver   
 *              SetPHYPowerMgmt          - suspend or resume the transceiver
 */

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include <mx28_usb.h>
#include "common_usbname.h"
#include "common_usbcommon.h"
#include <..\USBD\OS\oscheckkitl.c>

#pragma warning(disable: 4115 4201 4204 4214 4100)

#define HIGH_SPEED      1
#define FULL_SPEED      2

PCSP_USB_REGS gRegs; 
WORD gSel;

volatile UINT32 *gQueueHead;
#define QUEUE_HEAD_SIZE 0x200

// Really needs this
TCHAR gszOTGGroup[30];
DWORD gdwOTGSupport;
BOOL BSPUsbCheckWakeUp(void);
extern void BSPUsbSetBusConfig(PUCHAR baseMem);
extern void BSPUSBSwitchModulePower(BOOL bOn);
extern void BSPUsbPhyStartUp(void);
extern void BSPUsbPhyRegDump(void);
extern void DumpUSBRegs(PUCHAR baseMem);
extern WORD BSPGetUSBControllerType(void);
void BSPUSBHostVbusControl(BOOL blOn);

//-----------------------------------------------------------------------------
//
//  Function: PreSetPHYPowerMgmt
//
//  This function is to set configure before set the transceiver to suspend or resume mode.
//
//  Parameters:
//     N/A
//     
//  Returns:
//     NULL
//
//-----------------------------------------------------------------------------
void PreSetPHYPowerMgmt()
{
    // This function need to be called only when system resume and USB_WAKEUP_NONE is defined
}

//-----------------------------------------------------------------------------
//
//  Function: SetPHYPowerMgmt
//
//  This function is to configure the transceiver to suspend or resume mode.
//
//  Parameters:
//     fSuspend - TRUE : Suspend request, FALSE : Resume request
//     blSysSus - TRUE : System Suspend , FALSE : USB Low Power mode
//     
//  Returns:
//     NULL
//
//-----------------------------------------------------------------------------
void SetPHYPowerMgmt(BOOL fSuspend, BOOL blSysSus)
{
    UNREFERENCED_PARAMETER(fSuspend);
    UNREFERENCED_PARAMETER(blSysSus);
}

//------------------------------------------------------------------------------
// Function: InitializeOTGMux
//
// Description: This function is to configure the IOMUX for USB OTG Core
//
// Parameters:
//     NULL
//     
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void InitializeOTGMux()
{
}
  
//------------------------------------------------------------------------------
// Function: InitializeHost1Mux
//
// Description: This function is to configure the IOMUX for USB OTG Core
//
// Parameters:
//     NULL
//     
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void InitializeHost1Mux()
{
}

//------------------------------------------------------------------------------
// Function: InitializeMux
//
// This function is to configure the IOMUX for USB Host Cores
//
// Parameters:
//     sel
//         [IN] Selection of the USB cores (0 - OTG, 1 - HOST1)
//     
// Returns:
//     offset of the USB core register to be configured.
//
//------------------------------------------------------------------------------
static DWORD InitializeMux(int sel)
{
    DWORD off = offset(CSP_USB_REG, CAPLENGTH);
     
    if (sel == 0) 
    {   
        // OTG
        InitializeOTGMux();
    }
    else if(sel == 1)
    {
        // HOST1
        InitializeHost1Mux();
    }       
    
    // In MX28, We plan to use a union instead of a array to 
    off = offset(CSP_USB_REG, CAPLENGTH);
    return off;
}

//------------------------------------------------------------------------------
// Function: SelectUSBCore
//
// Description: This function is to select the corresponding USB core type 
//              according to the  micro USB_HOST_MODE. 
//
// Parameters:
//     sel
//         [OUT] the value returned which indicates the type of the USB core
//              (USB OTG or USB High Speed HOST port 2). sel = 0 means USB OTG 
//               core, sel = 1 means USB High Speed HOST 2 core. sel = 2 means USB
//               HIGH Speed Host 1 core.
//     
// Returns:
//     Speed of the USB core - HIGH_SPEED or FULL_SPEED
//
//------------------------------------------------------------------------------
static int SelectUSBCore(WORD * sel)
{
    int CoreSpeed;
    
    // OTG port, be sure to turn off the USB function driver.   
#if (USB_HOST_MODE == 0)
    // USBOTG
    *sel = 0;
#endif
#if (USB_HOST_MODE == 1)
    // USBH1
    *sel = 1;
#endif
    CoreSpeed = HIGH_SPEED;
    DEBUGMSG(ZONE_INIT, (L"High Speed USB OTG Host\r\n"));
    return CoreSpeed;
}


//------------------------------------------------------------------------------
// Function: ConfigEHCI
//
// Description: This function is to configure the USB OTG Core.
//
// Parameters:
//     pRegs
//         [IN] Pointer to 2 USB Core Registers
//     
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void ConfigEHCI(CSP_USB_REGS *pRegs, int speed)
{
    CSP_USB_REG* pReg = (PCSP_USB_REG)(&(pRegs->OTG));
    USB_PORTSC_T portsc;
    USB_USBMODE_T mode;
    USB_USBCMD_T cmd;   
    DWORD *temp;   
    
    UNREFERENCED_PARAMETER(speed);   

    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        pReg = (PCSP_USB_REG)(&(pRegs->OTG));
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        pReg = (PCSP_USB_REG)(&(pRegs->H1));
    }

    BSPUsbPhyStartUp();
    BSPUSBSwitchModulePower(TRUE);

    // Stop the controller first
    {
        temp = (DWORD *)&cmd;
        *temp = INREG32(&pReg->USBCMD);
        cmd.RS = 0;
        OUTREG32(&pReg->USBCMD, *temp);
        while ((INREG32(&pReg->USBCMD) & 0x1) == 0x1){
             Sleep(100);
        }
    }

    // Do a reset first no matter what
    {
        temp = (DWORD *)&cmd;
        *temp = INREG32(&pReg->USBCMD);
        cmd.RST = 1;
        OUTREG32(&pReg->USBCMD, *temp);
        while ((INREG32(&pReg->USBCMD) & 0x1<<1) == (0x1<<1)){
             Sleep(100);
        }
    }

    temp  = (DWORD *)&portsc;
    *temp = INREG32(&pReg->PORTSC);
    portsc.PTS = 0x0;     // UTMI/UTMI+ transceiver
    OUTREG32(&pReg->PORTSC, *temp);

    // set mode
    temp = (DWORD *)&cmd;
    *temp = INREG32(&pReg->USBCMD);
    cmd.RST = 1;
    OUTREG32(&pReg->USBCMD, *temp);
    while (INREG32(&pReg->USBCMD) & (0x1 << 1));

    temp = (DWORD *)&mode;
    *temp = INREG32(&pReg->USBMODE);
    mode.CM = 0x3;
    OUTREG32(&pReg->USBMODE, *temp);

    Sleep(10);
    if ((INREG32(&pReg->USBMODE)& 0x3) != 0x3)
    {
        return;
    }

    BSPUsbSetBusConfig((PUCHAR)pRegs);

    // H1_power_on_port1
    if (INREG32(&pReg->HCSPARAMS) &(0x1 << 4))
    {
        DWORD mask = (0x1<<1) + (0x1<<3)+(0x1<<5);
        CLRREG32(&pReg->PORTSC, mask);
        temp = (DWORD *)&portsc;
        *temp = INREG32(&pReg->PORTSC);
        portsc.PP = 1;      
        OUTREG32(&pReg->PORTSC, *temp);
    }
    else
        DEBUGMSG (1, (TEXT("Host does not control power\r\n")));

    temp = (DWORD *)&cmd;
    *temp = INREG32(&pReg->USBCMD);
    cmd.RS=0;
    OUTREG32(&pReg->USBCMD, *temp);
    
    // Handshake to wait for Halted
    while ((INREG32(&pReg->USBSTS) & 0x1000) != 0x1000);

     // Enable interrupts
    OUTREG32(&pReg->USBINTR, 0x5ff);
    return;
}

//-----------------------------------------------------------------------------
//
//  Function: gfnGetOTGGroup
//
//  This function is to return the OTG Group 
//
//  Parameters:
//     NULL
//     
//  Returns:
//     the OTG Group Name, that is used for creating the mode switching event semaphore
//     the value is reading from registry
//
//-----------------------------------------------------------------------------
TCHAR *gfnGetOTGGroup(void)
{
    return (gszOTGGroup);
}

//-----------------------------------------------------------------------------
//
//  Function: gfnIsOTGSupport
//
//  This function is whether to have OTG support
//
//  Parameters:
//     NULL
//     
//  Returns:
//     0 - Not OTG Support, 1 - OTG Support
//
//-----------------------------------------------------------------------------
DWORD gfnIsOTGSupport(void)
{
    return gdwOTGSupport;
}

//------------------------------------------------------------------------------
//
//  Function: InitializeTransceiver
//
//  This function is to configure the USB Transceiver, register the interrupt
//
//  Parameters:
//     NULL
//     
//  Returns:
//     TRUE - success, FALSE - failure
//
//------------------------------------------------------------------------------
BOOL InitializeTransceiver(PVOID* context, DWORD * phyregs, DWORD* dwOffset,
                           DWORD * sysintr, DWORD dwOTGSupport, TCHAR *pOTGGroup)
{
    DWORD dwRet = TRUE, speed, temp;
    int off, irq = 0;
    WORD sel = 0;
    WORD StringSize = 0;
    PCSP_USB_REGS* regs = (PCSP_USB_REGS*)context;
    PCSP_USB_REG pReg = (PCSP_USB_REG)(&((*regs)->OTG));

    StringSize = sizeof(gszOTGGroup) / sizeof(TCHAR);
    StringCchCopy(gszOTGGroup,StringSize,pOTGGroup);
    gdwOTGSupport = dwOTGSupport;

    dwRet = SelectUSBCore(&sel);
    speed = dwRet;
    gSel = sel;
    
    if(sel == 0)
    {
        //only OTG port need to check KITL
        if(FslUfnIsUSBKitlEnable())
        {
            RETAILMSG(1,(_T("USB Host load failure because Usb Kitl Enabled\r\n")));
            return FALSE;
        }
    }
    
    {
        off = InitializeMux(sel);  
        USBClockInit();
        
        ConfigEHCI(*regs, speed);

        if (sel == 0) {
            irq = IRQ_USB0;
        }
        else if(sel == 1)
        {
            irq = IRQ_USB1;
        }
    }

    if (irq == IRQ_USB0)
    {
        INT32 aIrqs[3];
        aIrqs[0] = -1;
#ifdef GENERAL_OTG
        // We now have 2 Sets of OTG mechanism
        // 1. traditional i.MX implementaion, where XVC, UFN and HCD driver are working simultaneously, blocked each other
        // by blocking mutex events. In this case we should use "OAL_INTR_TRANSLATE" to map USB IRQ to same SYSINTR in different module
        // 2. more general MSFT otg implementaion, where OTG driver is the father of UFN and HCD. The latter 2 are loaded by OTG dirver
        // and don't work simultaneously. In this implementaion a chain interrupt mechanism is used. Via giisr.dll, OTG driver only keeps
        // monitoring OTGSC related interrupt. If a USB IRQ happens not because of OTGSC, the SYSINTR is generated in a chainning method.
        // In this case we should use "OAL_INTR_FORCE_STATIC" in order for UFN/HCD getting different SYSINTR number so that interrupt chainning
        // can be acheieved through OTG and UFN/HCD
        // Select either "OAL_INTR_FORCE_STATIC" or "OAL_INTR_TRANSLATE" don't have impact on pure UFN/HCD configuration
        //
        // "GENERAL_OTG" is defined only in porject where the 2nd OTG mechanism is used, such as MX23/MX28

        aIrqs[1] = OAL_INTR_FORCE_STATIC;
#else
        aIrqs[1] = OAL_INTR_TRANSLATE;
#endif
        aIrqs[2] = IRQ_USB0;
        KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(aIrqs), sysintr, sizeof(DWORD), NULL);
    }
    else
        KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD), sysintr, sizeof(DWORD), NULL);      
        
    DEBUGMSG(1, (TEXT("InitializeTransceiver: IRQ=%d, sysIntr=%d\r\n"), irq, *sysintr));
       
    if (sel == 0)
    {
        pReg = (PCSP_USB_REG)(&((*regs)->OTG));
    }
    else if (sel == 1)
    {
        pReg = (PCSP_USB_REG)(&((*regs)->H1));
    }

    {
        USB_USBMODE_T mode;
        DWORD * ptemp=(DWORD *)&mode;
        
        // Set USB Mode 
        *ptemp = 0;
        mode.CM = 3;      // Host 
 
        OUTREG32(&pReg->USBMODE, *ptemp);
    }

    // power on port
    {
        DWORD *ptemp;
        USB_HCSPARAMS_T hcs;
        ptemp=(DWORD *)&hcs;
        *ptemp=INREG32(&pReg->HCSPARAMS);

        if (hcs.PPC) 
        {
            USB_PORTSC_T portsc;
            DWORD * temp2= (DWORD *)&portsc;

            *temp2 = INREG32(&pReg->PORTSC);
            portsc.PP = 1;
            SETREG32(&pReg->PORTSC, *temp2);
        }   
    }

    gRegs = (PCSP_USB_REGS)(*regs);
    gSel = sel;

    // It is necessary here to check if PHCD was set to 1, This happens
    // on OTG implementation, when unloading UFN driver, PHCD will be set, 
    // so we need to check and make sure PHCD is 0 here
    {
        USB_PORTSC_T portsc;
        DWORD* tempPsc = (DWORD*)&portsc;

        *tempPsc = INREG32(&pReg->PORTSC);
        if (portsc.PHCD == 1)
        {
            portsc.PHCD = 0;
            OUTREG32(&pReg->PORTSC, *tempPsc);
        }
    }

    BSPUSBHostVbusControl(TRUE);

    temp = *(DWORD*)regs;
    temp += off;
    *(DWORD*)regs = temp;
    *phyregs += off;
    *dwOffset = off;

    return dwRet;
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPUsbCheckConfigPower
//
//  Check power required by specific device configuration and return whether it
//  can be supported on this platform.  For CEPC, this is trivial, just limit
//  to the 500mA requirement of USB.  For battery powered devices, this could
//  be more sophisticated, taking into account current battery status or other 
//  info.
//
// Parameters:
//      bPort
//          [in] Port number
//
//      dwCfgPower
//          [in] Power required by configuration in mA.
//
//      dwTotalPower
//          [in] Total power currently in use on port in mA.
//
// Returns:
//      Return TRUE if configuration can be supported, FALSE if not.
//
//-----------------------------------------------------------------------------
BOOL BSPUsbhCheckConfigPower(UCHAR bPort, DWORD dwCfgPower, DWORD dwTotalPower)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(bPort);

    return ((dwCfgPower + dwTotalPower) > 500) ? FALSE : TRUE;
}

#ifdef DISABLE_DETACH_WAKEUP
#undef DISABLE_DETACH_WAKEUP
#endif
 //-----------------------------------------------------------------------------
//
//  Function: BSPUsbSetWakeUp
//
//  This function is to enable/disable the wakeup interrupt bit in USBCONTROL
//
//  Parameters:
//     bEnable - TRUE : Enable, FALSE : Disable
//     
//  Returns:
//     NULL
//
//-----------------------------------------------------------------------------
void BSPUsbSetWakeUp(BOOL bEnable)
{
    // Access the USB Control Register
    volatile DWORD  *temp;
    // USB_CTRL_T ctrl;
    USB_PORTSC_T portsc;
    CSP_USB_REG *pReg; 

    // still need to check it first before proceed
    BSPUsbCheckWakeUp();
        
    switch (gSel) 
    {
    case 0:
        pReg = (PCSP_USB_REG)(&(gRegs->OTG));
        break;
    case 1:
        pReg = (PCSP_USB_REG)(&(gRegs->H1));
        break;
    default:
        pReg = (PCSP_USB_REG)(&(gRegs->OTG));
        break;
    }

    temp = (DWORD *)&portsc;
    *temp = INREG32(&pReg->PORTSC[0]);
    // If Current Connect Status = 1, we should not set WKCN or it would 
    // wake up right away.  With this we can enable wake up on attach
    if (bEnable)
    {
        //portsc.WKDC & portsc.WDCN is mutex, means at the same time, only one
        //can be set to 1, so we need to set WKDC or WKCN according to current
        //connect status
        if(portsc.CCS == 0)
        {
            portsc.WKDC = 0;
            portsc.WKCN = 1;
        }
        else
        {
#if defined USB_WAKEUP_CNANDDN
            portsc.WKDC = 1;
            portsc.WKCN = 0;
#else
            portsc.WKDC = 0;
            portsc.WKCN = 0;
#endif
        }

        portsc.WKOC = 1;          
    }
    else
    {
        portsc.WKOC = 0;
        portsc.WKDC = 0;
        portsc.WKCN = 0;
    }
    // This delay is very important, if no delay here, a ULPI interrupt will be issued
    // immediately and system can't suspend at all.
    StallExecution(10000);
    OUTREG32(&pReg->PORTSC[0], *temp);
    
    return;
}

//------------------------------------------------------------------------------
//
//  Function: BSPUsbCheckWakeUp
//
//  This function is called by CSP to clear the wakeup interrupt bit in USBCONTROL. According to
//  Ringo specification, disable the wake-up enable bit also clear the interrupt request bit.
//  This wake-up interrupt enable should be disable after receiving a wakeup request.
//
//  Parameters:
//     NULL
//     
//  Returns:
//     TRUE - there is a wakeup.  FALSE - no wakeup is set.
//
//------------------------------------------------------------------------------
BOOL BSPUsbCheckWakeUp(void)
{
    // Access the USB Control Register
    // volatile DWORD  *temp;
    // USB_CTRL_T ctrl;
    BOOL fWakeUp = FALSE;

    return fWakeUp;
}

//------------------------------------------------------------------------------
//
//  Function: BSPUsbHostLowPowerModeEnable
//
//  This function is called by CSP to get the information about if we need enable
//  Low Power Mode, enable means close PHY clock, disable means keep PHY clock
//
//  Parameters:
//     NULL
//     
//  Returns:
//     TRUE - enable.  FALSE - disable.
//
//------------------------------------------------------------------------------
BOOL BSPUsbHostLowPowerModeEnable(void)
{
    return TRUE;
}

//------------------------------------------------------------------------------
// Function: BSPUSBHostVbusControl
//
// This function is to control USB VBus
//
// Parameters:
//     blOn
//         [IN] Pull UP/DOWN USB VBUS 
//     
// Returns:
//     
//
//------------------------------------------------------------------------------
void BSPUSBHostVbusControl(BOOL blOn)
{
    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
#ifdef	EM9280
		// use GPIO1_1 as USB0_PWR_EN
		DDKIomuxSetPinMux(DDK_IOMUX_LCD_D1, DDK_IOMUX_MODE_GPIO); 
		DDKGpioEnableDataPin(DDK_IOMUX_LCD_D1, 1);

		if (blOn)
        {
            DDKGpioWriteDataPin(DDK_IOMUX_LCD_D1, 1); // turn on VBUS
        }
        else
        {
            DDKGpioWriteDataPin(DDK_IOMUX_LCD_D1, 0); // turn off VBUS
        }
#else	// -> EM9283 or iMX28EVK
#ifdef EM9283
		DDKIomuxSetPinMux(DDK_IOMUX_LCD_D0, DDK_IOMUX_MODE_GPIO); 
		DDKGpioEnableDataPin(DDK_IOMUX_LCD_D0, 1);

		if (blOn)
		{
			DDKGpioWriteDataPin(DDK_IOMUX_LCD_D0, 1);
		}
		else
		{
			DDKGpioWriteDataPin(DDK_IOMUX_LCD_D0, 0);
		}
#else
        DDKIomuxSetPinMux(DDK_IOMUX_AUART2_TX_1, DDK_IOMUX_MODE_GPIO); 
        DDKGpioEnableDataPin(DDK_IOMUX_AUART2_TX_1, 1);

        if (blOn)
        {
            DDKGpioWriteDataPin(DDK_IOMUX_AUART2_TX_1, 1);
        }
        else
        {
            DDKGpioWriteDataPin(DDK_IOMUX_AUART2_TX_1, 0);
        }
#endif  //EM9283
#endif	//EM9280
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
#ifdef	EM9280
		//
		// USB HOST port has no VBUS control in EM9280
		//
#else	// -> EM9283 or iMX28EVK
#ifdef  EM9283
		DDKIomuxSetPinMux(DDK_IOMUX_LCD_D1, DDK_IOMUX_MODE_GPIO); 
		DDKGpioEnableDataPin(DDK_IOMUX_LCD_D1, 1);

		if (blOn)
		{
			DDKGpioWriteDataPin(DDK_IOMUX_LCD_D1, 1);
		}
		else
		{
			DDKGpioWriteDataPin(DDK_IOMUX_LCD_D1, 0);
		}
#else
        DDKIomuxSetPinMux(DDK_IOMUX_AUART2_RX_1, DDK_IOMUX_MODE_GPIO); 
        DDKGpioEnableDataPin(DDK_IOMUX_AUART2_RX_1, 1);

        if (blOn)
        {
            DDKGpioWriteDataPin(DDK_IOMUX_AUART2_RX_1, 1);
        }
        else
        {
            DDKGpioWriteDataPin(DDK_IOMUX_AUART2_RX_1, 0);
        }
#endif  //EM9283
#endif	//EM9280
    }
}


//------------------------------------------------------------------------------
// Function: BSPNeedCloseVBus
//
// This function is used to control if we need to do a VBUS off-on Cycle on public
// code druing resuem (power-up) opertion
//
// Parameters:
//          N.A
//     
// Returns:
//      TRUE - public code need to close VBUS and open it again 
//      FALSE - don't need to do so
//     
//
//------------------------------------------------------------------------------
BOOL BSPNeedCloseVBus(void)
{
    return TRUE;
}


//------------------------------------------------------------------------------
// Function: IsLowPowerSameAsSuspend
//
// This function is used to return a bool value to indicate if additional process
// is need to proper handle LowPower mode and Suspend mode
//
// Parameters:
//          blSuspend -- if current state is suspend or not
//     
// Returns:
//      TRUE - public code need to close VBUS and open it again 
//      FALSE - don't need to do so
//     
//
//------------------------------------------------------------------------------
BOOL IsLowPowerSameAsSuspend(BOOL blSuspend)
{
    if(blSuspend)
    {
#if defined USB_WAKEUP_CNANDDN || defined USB_WAKEUP_CN
        return TRUE;
#else
        return FALSE;
#endif
    }
    else
    {
        return FALSE;
    }
}
