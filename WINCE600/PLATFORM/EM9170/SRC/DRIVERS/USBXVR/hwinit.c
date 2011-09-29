//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// File:     
//     hwinit.c
// Purpose:  
//     Platform-specific routines for OTG transceiver/controller initialisation
//     and control, including suspend/resume of transceiver. Includes set-up of
//     IOMUX and Vbus drivers for OTG port.
// Functions:
//     BOOL InitializeOTGTransceiver(PCSP_USB_REGS* regs, BOOL IsHost)
//
//     Various platform-specific functionality is added through callback 
//     functions which are supplied via generic interface routine:
//         void RegisterCallback(BSP_USB_CALLBACK_FNS *pfn)
// 
//     Those callbacks include:
//         pfn->pfnUSBPowerDown        - called on system power suspend
//         pfn->pfnUSBPowerUp          - called on system power restore
//         pfn->pfnUSBSetPhyPowerMode  - called to set PHY to lower power mode
//               
//     In this implementation phy/usb-clock/vcore is set to lower-power mode 
//     whenever idle in transceiver mode, so nothing needs to be done on 
//     PowerUp/PowerDown)
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include "MX25_usb.h"
#include "common_usbname.h"
#include "common_usbcommon.h"

#ifdef DEBUG
#ifdef ZONE_FUNCTION
#undef ZONE_FUNCTION
#endif
#define ZONE_FUNCTION         DEBUGZONE(3)

DBGPARAM dpCurSettings = {
    _T("USBXVR"),
    {
        _T("Error"), _T("Warning"), _T("Init"), _T("Function"),
        _T("Comments"), _T(""), _T(""), _T(""),
        _T(""), _T(""), _T(""), _T(""), 
        _T(""), _T(""), _T(""), _T("")
    },
    0xffff//0x5
};
#endif // DEBUG

#define USB_HOST_MODE 1             // OTG high speed

static void BSPUsbXvrSetPhyPowerMode(CSP_USB_REGS *pUSBRegs, BOOL bResume);
extern void BSPUsbSetBusConfig(PUCHAR baseMem);

//------------------------------------------------------------------------------
// Function: BSPGetUSBControllerType
//
// Description: This function is to return the USB Core Controller type in 
//              WORD format. This is called by CSP public code when accessing 
//              the type of controller it is using.
//
// Parameters:
//     NULL
//     
// Returns:
//     USB_SEL_OTG(See definition in file mx25_usbcommon.h)
//
//------------------------------------------------------------------------------
WORD BSPGetUSBControllerType(void)
{
    return USB_SEL_OTG;
}

//------------------------------------------------------------------------------
// Function: InitializeOTGMux
//
// Description: This function configures the IOMUX for USB OTG Core. Senna OTG
//              takes its internal UTMI+ trasceiver as default, so only OTG_PWR
//              and OTG_OC pins are configured.        
//
// Parameters:
//     NULL
//     
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void InitializeOTGMux(void)
{
#if 1
#ifdef EM9170
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
    DDKGpioWriteDataPin(DDK_GPIO_PORT4, 9, 0);		//USB_OTG_PWR = 0: assert VBUS
#else //-> PDK1_7
    // Pullup VBUS
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
    DDKGpioWriteDataPin(DDK_GPIO_PORT1, 0, 0);
#endif //EM9170
    return;
#else
#ifdef EM9170
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D11,										// pin name
									DDK_IOMUX_PIN_MUXMODE_ALT6,					// select USB_OTG_PWR 
									DDK_IOMUX_PIN_SION_REGULAR);

    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D11,
										 DDK_IOMUX_PAD_SLEW_SLOW,					// -> the same electrical config as PDK1_7
										 DDK_IOMUX_PAD_DRIVE_NORMAL,
										 DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
										 DDK_IOMUX_PAD_PULL_DOWN_100K,
										 DDK_IOMUX_PAD_HYSTERESIS_DISABLE,
										 DDK_IOMUX_PAD_VOLTAGE_3V3);

    // Set the USB OC pin
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D10,										// pin name
									DDK_IOMUX_PIN_MUXMODE_ALT6,					// select USB_OTG_OC 
									DDK_IOMUX_PIN_SION_REGULAR);

    DDKIomuxSetPadConfig(DDK_IOMUX_PIN_D10,
										 DDK_IOMUX_PAD_SLEW_SLOW,
										 DDK_IOMUX_PAD_DRIVE_NORMAL,
										 DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
										 DDK_IOMUX_PAD_PULL_DOWN_100K,
										 DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
										 DDK_IOMUX_PAD_VOLTAGE_3V3);
#else // -> PDK1_7
    // Set the USB OTG power pin
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_A, DDK_IOMUX_PIN_MUXMODE_ALT2, 
                        DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_A,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_DOWN_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_DISABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // Set the USB OC pin
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_B, DDK_IOMUX_PIN_MUXMODE_ALT2,
                        DDK_IOMUX_PIN_SION_REGULAR); 
    DDKIomuxSetPadConfig(DDK_IOMUX_PIN_GPIO_B,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_DOWN_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
#endif //EM9170
    return;
#endif
}
   
//------------------------------------------------------------------------------
// Function: USBOTGVbusControl
// Description: This function configures the VBUS pin for USB OTG Core. Senna OTG
//              has a negative logic
//     
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
void USBOTGVbusControl(BOOL fOn)
{
#ifdef EM9170
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

    // Negative Logic
    if (fOn)
    {
        DDKGpioWriteDataPin(DDK_GPIO_PORT4, 9, 0);
    }
    else
    {
        DDKGpioWriteDataPin(DDK_GPIO_PORT4, 9, 1);
    }
#else // -> PDK1_7
    // SET Pin Configuration
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

    // Negative Logic
    if (fOn)
    {
        DDKGpioWriteDataPin(DDK_GPIO_PORT1, 0, 0);
    }
    else
    {
        DDKGpioWriteDataPin(DDK_GPIO_PORT1, 0, 1);
    }
#endif //EM9170
}
//------------------------------------------------------------------------------
// Function: InitializeMux
//
// Description: This function is to configure the IOMUX for USB OTG Core
//
// Parameters:
//     NULL
//     
// Returns:
//     Offset of the configured USB Core register.
//
//------------------------------------------------------------------------------
static DWORD InitializeMux(void)
{
    DWORD off;

    InitializeOTGMux();
    off = offset(CSP_USB_REG, CAPLENGTH);

    return off;
}

//------------------------------------------------------------------------------
// Function: DumpDeviceState
//
// Description: This function is used to dump the device status
//
// Parameters:
//     state
//         [IN] Points to USB PORTSC register
//     
//  Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void DumpDeviceState( USB_PORTSC_T * state)  
{
    if (state->CCS)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tCurrent Connect Status: Attached\r\n"));
    if (state->CSC)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tConnect Status Change: Changed\r\n"));
    if (state->PE)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tPort Enabled\r\n"));
    if (state->PEC)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tPort Enable/Disable Change\r\n"));
    if (state->OCA)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tOver-current Active\r\n"));
    if (state->OCC)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tOver-current Change\r\n"));
    if (state->FPR)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tForce Port Resume\r\n"));
    if (state->SUSP)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tSuspend\r\n"));
    if (state->PR)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tPort Reset\r\n"));
    if (state->HSP)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tHigh-Speed Port \r\n"));
    
    DEBUGMSG(ZONE_FUNCTION, (L"\t\tLine Status: %x", state->LS));
    switch (state->LS) {
        case 0:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tSE0\r\n"));
            break;
        case 1:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tJ-state\r\n"));
            break;
        case 2:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tK-state\r\n"));
            break;
        case 3: 
        default:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tUndefined\r\n"));
            break;
    }

    if (state->PP)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\t??? Should be 0 for device\r\n"));
    if (state->PO)
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tPort Owner\r\n"));
    if (state->PIC) {
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tPort Indicator Control"));
        switch (state->PIC) {
            case 1:
                DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tAmber\r\n"));
                break;
            case 2:
                DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tGreen\r\n"));
                break;
            case 3:
            default:
                DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tUndefined\r\n"));
                break;
        }
    }   
    if (state->PTC) 
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tPort Test Control: %x\r\n", state->PTC));
        
    if (state->WKCN) 
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tWake on Connect Enable (WKCNNT_E)\r\n"));
                        
    if (state->WKDC) 
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tWake on Disconnect Enable (WKDSCNNT_E) \r\n"));

    if (state->WKOC) 
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tWake on Over-current Enable (WKOC_E) \r\n"));
    
    if (state->PHCD) 
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tPHY Low Power Suspend - Clock Disable (PLPSCD) \r\n"));

    if (state->PFSC) 
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tPort Force Full Speed Connect \r\n"));

    DEBUGMSG(ZONE_FUNCTION, (L"\t\tPort Speed: %x->", state->PSPD));
    switch (state->PSPD) {
        case 0:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tFull Speed\r\n"));
            break;
        case 1:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tLow Speed\r\n"));
            break;
        case 2:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tHigh Speed\r\n"));
            break;
        case 3:
        default:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tUndefined\r\n"));
            break;
    }
    DEBUGMSG(ZONE_FUNCTION, (L"\t\tParallel Transceiver Width:%x->", state->PTW));
    if (state->PTW) 
        DEBUGMSG(ZONE_FUNCTION, (L"\t\t\t16 bits\r\n"));
    else
        DEBUGMSG(ZONE_FUNCTION, (L"\t\t\t8 bits\r\n"));
    if (state->STS) 
        DEBUGMSG(ZONE_FUNCTION, (L"\t\tSerial Transceiver Select \r\n"));
    
    DEBUGMSG(ZONE_FUNCTION, (L"\t\tParallel Transceiver Select:%x->", state->PTS));
    switch (state->PTS) {
        case 0:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tUTMI/UTMI+\r\n"));
            break;
        case 1:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tPhilips Classic\r\n"));
            break;
        case 2:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tULPI\r\n"));
            break;
        case 3:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tSerial/1.1 PHY (FS Only)\r\n"));
            break;
        default:
            DEBUGMSG(ZONE_FUNCTION, (L"\t\t\tUndefined\r\n"));
            break;
    }                   
}

#ifdef DEBUG
//------------------------------------------------------------------------------
// Function: DumpUSBRegs
//
// Description: This function is used to dump the USB OTG Register in detail
//
// Parameters:
//     regs
//         [IN] Points to USB related registers mapped to virtual address
//     
//  Returns:
//     NULL
//
//------------------------------------------------------------------------------
void DumpUSBRegs(PCSP_USB_REGS regs)
{
    static USHORT first = 1;
    CSP_USB_REG *pReg;
    
    RETAILMSG(1, (L"Dump OTG Regs\r\n"));
    pReg = (PCSP_USB_REG)(&(regs->OTG));

    if (first) {
        RETAILMSG(1,(L"\tID(%xh)=%x\r\n",offset(CSP_USB_REG,ID), INREG32(&pReg->ID)));
        RETAILMSG(1,(L"\tHWGENERAL(%xh)=%x\r\n",offset(CSP_USB_REG,HWGENERAL),INREG32(&pReg->HWGENERAL)));
        RETAILMSG(1,(L"\tHWHOST(%xh)=%x\r\n",offset(CSP_USB_REG,HWHOST),INREG32(&pReg->HWHOST)));
        RETAILMSG(1,(L"\tHWDEVICE(%xh)=%x\r\n",offset(CSP_USB_REG,HWDEVICE),INREG32(&pReg->HWDEVICE)));
        RETAILMSG(1,(L"\tHWTXBUF(%xh)=%x\r\n",offset(CSP_USB_REG,HWTXBUF),INREG32(&pReg->HWTXBUF)));
        RETAILMSG(1,(L"\tHWRXBUF(%xh)=%x\r\n",offset(CSP_USB_REG,HWRXBUF),INREG32(&pReg->HWRXBUF)));
        RETAILMSG(1,(L"\tCAPLENGTH(%xh)=%x\r\n",offset(CSP_USB_REG,CAPLENGTH),INREG8(&pReg->CAPLENGTH)));
        RETAILMSG(1,(L"\tHCIVERSION(%xh)=%x\r\n",offset(CSP_USB_REG,HCIVERSION),INREG16(&pReg->HCIVERSION)));
        RETAILMSG(1,(L"\tHCSPARAMS(%xh)=%x\r\n",offset(CSP_USB_REG,HCSPARAMS),INREG32(&pReg->HCSPARAMS)));
        RETAILMSG(1,(L"\tHCCPARAMS(%xh)=%x\r\n",offset(CSP_USB_REG,HCCPARAMS),INREG32(&pReg->HCCPARAMS)));
        RETAILMSG(1,(L"\tDCIVERSION(%xh)=%x\r\n",offset(CSP_USB_REG,DCIVERSION),INREG16(&pReg->DCIVERSION)));
        RETAILMSG(1,(L"\tDCCPARAMS(%xh)=%x\r\n",offset(CSP_USB_REG,DCCPARAMS),INREG32(&pReg->DCCPARAMS)));
        first=0;
    }   
    RETAILMSG(1,(L"\tUSBCMD(%xh)=%x\r\n",offset(CSP_USB_REG,USBCMD),INREG32(&pReg->USBCMD)));
    RETAILMSG(1,(L"\tUSBSTS(%xh)=%x\r\n",offset(CSP_USB_REG,USBSTS),INREG32(&pReg->USBSTS)));
    RETAILMSG(1,(L"\tUSBINTR(%xh)=%x\r\n",offset(CSP_USB_REG,USBINTR),INREG32(&pReg->USBINTR)));
    RETAILMSG(1,(L"\tPORTSC(%xh)[0]=%x\r\n",offset(CSP_USB_REG,PORTSC[0]),INREG32(&pReg->PORTSC[0])));
    {
        USB_PORTSC_T state;
        DWORD * temp=(DWORD*)&state;
        *temp=INREG32(&pReg->PORTSC[0]);
        DumpDeviceState( & state);
    }
    RETAILMSG(1,(L"\tOTGSC(%xh)=%x\r\n",offset(CSP_USB_REG,OTGSC),INREG32(&pReg->OTGSC)));
    RETAILMSG(1,(L"\tUSBMODE(%xh)=%x\r\n",offset(CSP_USB_REG,USBMODE),INREG32(&pReg->USBMODE)));
    RETAILMSG(1,(L"\tULPI_VIEWPORT(%xh)=%x\r\n",offset(CSP_USB_REG,ULPI_VIEWPORT),INREG32(&pReg->ULPI_VIEWPORT)));

    RETAILMSG(1,(L"\t*********************\r\n"));
    RETAILMSG(1,(L"\tAddress of ASYNCLISTADDR at 0x%x\r\n", &pReg->T_158H.ASYNCLISTADDR));
    RETAILMSG(1,(L"\tASYNCLISTADDR(%xh)=%x\r\n",offset(CSP_USB_REG,T_158H),INREG32(&pReg->T_158H.ASYNCLISTADDR)));
    RETAILMSG(1,(L"\tENDPTSETUPSTAT(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTSETUPSTAT),INREG32(&pReg->ENDPTSETUPSTAT)));
    RETAILMSG(1,(L"\tENDPTPRIME(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTPRIME),INREG32(&pReg->ENDPTPRIME)));
    RETAILMSG(1,(L"\tENDPTFLUSH(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTFLUSH),INREG32(&pReg->ENDPTFLUSH)));
    RETAILMSG(1,(L"\tENDPTSTATUS(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTSTATUS),INREG32(&pReg->ENDPTSTATUS)));
    RETAILMSG(1,(L"\tENDPTCOMPLETE(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTCOMPLETE),INREG32(&pReg->ENDPTCOMPLETE)));
    RETAILMSG(1,(L"\tENDPTCTRL0(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTCTRL0),INREG32(&pReg->ENDPTCTRL0)));
    RETAILMSG(1,(L"\t*********************\r\n"));
    
    RETAILMSG(1,(L"\tUSB_CTRL(%xh)=%x\r\n",offset(CSP_USB_REGS,USB_CTRL),INREG32(&regs->USB_CTRL)));
    RETAILMSG(1,(L"\tUSB_OTG_MIRROR(%xh)=%x\r\n",offset(CSP_USB_REGS,USB_OTG_MIRROR),INREG32(&regs->USB_OTG_MIRROR)));

}
#endif // if debug

//------------------------------------------------------------------------------
// Function: ConfigOTGHost
//
// Description: This function configures host functionality on the USB OTG 
//              Core.
//
// Parameters:
//     pRegs
//         [IN] Points to USB related registers mapped to virtual address     
//     
//  Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void ConfigOTGHost(CSP_USB_REGS *pRegs)
{
    USB_PORTSC_T portsc;
    USB_CTRL_T   ctrl;
    USB_USBMODE_T mode;
    USB_USBCMD_T cmd;
    DWORD *temp;
            

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+ConfigOTGHost\r\n")));
    
    // UTMI/UTMI+ transceiver for Senna
    temp  = (DWORD *)&portsc;
    *temp = INREG32(&pRegs->OTG.PORTSC);
    portsc.PTS = 0x0;
    OUTREG32(&pRegs->OTG.PORTSC, *temp);

    // usb_otg_interrupt_enable
    temp = (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.OWIE = 1;
    OUTREG32(&pRegs->USB_CTRL, *temp);

    // usb_otg_power_mask_enable
    temp = (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.OPM = 1;
    OUTREG32(&pRegs->USB_CTRL, *temp);

    // Rest the OTG controller and set mode to HOST mode.
    temp = (DWORD *)&cmd;
    *temp = INREG32(&pRegs->OTG.USBCMD);
    cmd.RST = 1;
    OUTREG32(&pRegs->OTG.USBCMD, *temp);
    
    while (INREG32(&pRegs->OTG.USBCMD)& (0x1 << 1))
    {
        Sleep(1000);
    }

    temp = (DWORD *)&mode;
    *temp = INREG32(&pRegs->OTG.USBMODE);
    mode.CM = 0x3;
    OUTREG32(&pRegs->OTG.USBMODE, *temp);

    Sleep(10);
    if ((INREG32(&pRegs->OTG.USBMODE)& 0x3) != 0x3)
    {
        return;
    }

    // otg_power_on_port1
    if (INREG32(&pRegs->OTG.HCSPARAMS) &(0x1 << 4))
    {
        DWORD mask = (0x1<<1) + (0x1<<3)+(0x1<<5);
        CLRREG32(&pRegs->OTG.PORTSC, mask);
        temp = (DWORD *)&portsc;
        *temp = INREG32(&pRegs->OTG.PORTSC);
        portsc.PP = 1;      
        OUTREG32(&pRegs->OTG.PORTSC, *temp);
    }
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Host does not control power\r\n")));

    //otg_controller_reset
    temp=(DWORD *)&cmd; 
    *temp=0;
    cmd.RST=1;  
    CLRREG32(&pRegs->OTG.USBCMD, *temp);
    *temp = INREG32(&pRegs->OTG.USBCMD);
    cmd.RST=1;
    OUTREG32(&pRegs->OTG.USBCMD, *temp);
    while (INREG32(&pRegs->OTG.USBCMD) & (0x1 << 1));

    temp = (DWORD *)&mode;
    *temp = INREG32(&pRegs->OTG.USBMODE);
    mode.CM = 0x3;
    OUTREG32(&pRegs->OTG.USBMODE, *temp);

    Sleep(10);  
    while ((INREG32(&pRegs->OTG.USBMODE)& 0x3) != 0x3);

    BSPUsbSetBusConfig((PUCHAR)(&pRegs->OTG));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-ConfigOTGHost\r\n")));
}

//------------------------------------------------------------------------------
// Function: ConfigOTGClient
//
// Description: This function configures client functionality on the USB OTG 
//              Core.
//
// Parameters:
//     pRegs
//         [IN] Points to USB related registers mapped to virtual address 
//     
//  Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void ConfigOTGClient(CSP_USB_REGS *regs)
{
    DWORD *temp;
  
    USB_HCSPARAMS_T hcs;
    //USB_USBCMD_T cmd;
    USB_USBMODE_T mode;
    USB_PORTSC_T port;
    
      
    // Enable power for OTG on port 1
    temp = (DWORD *)&hcs;
    *temp = INREG32(&regs->OTG.HCSPARAMS);
        
    if (hcs.PPC) {
        WORD USB_OTG_PORTSC1_Write_MASK = (0x1<<1)+(0x1<<3)+(0x1<<5);   
        *temp = INREG32(&regs->OTG.PORTSC[0]);
        *temp &= ~USB_OTG_PORTSC1_Write_MASK;
        *temp |= 1<<12;
        
        // Note: SETREG32 reads and ORs extra bits from temp. So what's done
        // above doesn't seem to do as intended (seems it wanted to leave 1, 3, 
        // and 5 as they were; they are write-to-clear). I haven't changed this 
        // because of code lock-down             
        SETREG32(&regs->OTG.PORTSC[0], *temp);
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
        
        portsc.WKDC = 1;    // Wake on disconnect enable
        portsc.WKCN = 1;    // Wake on connect cable

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
            portsc.PP=1;
            SETREG32(&regs->OTG.PORTSC, *tempData);
        }                   
    }

    // usb cmd
    {
        USB_USBCMD_T cmd;
        DWORD * tempCmd;
        tempCmd=(DWORD *)&cmd;
        *tempCmd=INREG32(&regs->OTG.USBCMD);
        cmd.ITC=0x08;
        SETREG32(&regs->OTG.USBCMD, *tempCmd);
    }
    
    // bus config
    BSPUsbSetBusConfig((PUCHAR)(&regs->OTG));
    
    DEBUGMSG(1, (TEXT("ConfigOTGClient is done\r\n")));
}

//------------------------------------------------------------------------------
// Function: InitializeOTGTransceiver
//
// This function is to configure the OTG USB Core.
//
// Parameters:
//     pRegs
//         [IN] Points to USB related registers mapped to virtual address
//     IsHost
//         [IN] If TRUE, OTG is configured as host. If FALSE, OTG is 
//              configured as client
//     
// Returns:
//     TRUE: Success
//     FALSE: Failure
//
//------------------------------------------------------------------------------
BOOL InitializeOTGTransceiver(PCSP_USB_REGS* regs, BOOL IsHost)
{
    // Clear USB Interrupt enable registers
    RETAILMSG(1, (L"Config XVC as %s\r\n", IsHost?L"Host":L"Dev"));
    OUTREG32(&(*regs)->OTG.USBINTR, 0);

    USBClockInit();
    
    if (IsHost) 
    {
        // Stop the controller first. According to EHCI spec, the controller
        // must be stopped (i.e. USBSTS.HCH = 1) before reset.

        // Pull Up VBUS
        USBOTGVbusControl(TRUE);

        {
            USB_USBCMD_T cmd;           
            USB_USBSTS_T sts;
            USB_USBMODE_T mode;
            DWORD * temp = (DWORD *)&cmd;
            DWORD * temp2 = (DWORD*)&sts;
            DWORD *pTmpMode = (DWORD*)&mode;
            int nAttempts = 0;

            *temp = INREG32(&(*regs)->OTG.USBCMD);
            cmd.RS = 0;
            OUTREG32(&(*regs)->OTG.USBCMD, *temp);

            *pTmpMode = INREG32(&(*regs)->OTG.USBMODE);
            if ( mode.CM == 0x3 )
            {
                *temp2 = INREG32(&(*regs)->OTG.USBSTS);
                while ( (sts.HCH == 0) && (nAttempts++ < 50) )
                {
                    Sleep(10);
                    *temp2 = INREG32(&(*regs)->OTG.USBSTS);
                }
            }
        }

        // Do a reset first no matter what, wait until the reset operation is
        // finished.
        {
            USB_USBCMD_T cmd;           
            DWORD * temp = (DWORD *)&cmd;

            *temp = INREG32(&(*regs)->OTG.USBCMD);

            cmd.RST = 1;
            OUTREG32(&(*regs)->OTG.USBCMD, *temp);

            *temp = INREG32(&(*regs)->OTG.USBCMD);

            while ( cmd.RST )
            {
                Sleep(10);
                *temp = INREG32(&(*regs)->OTG.USBCMD);
            }
        }

        // actually set the mode to HOST now
        {
            USB_USBMODE_T mode;
            DWORD *temp = (DWORD *)&mode;
            temp = (DWORD *)&mode;
            *temp = INREG32(&(*regs)->OTG.USBMODE);
            mode.CM = 0x3;
            OUTREG32(&(*regs)->OTG.USBMODE, *temp);
        }

        ConfigOTGHost(*regs);

        {
            USB_USBMODE_T mode;
            DWORD * temp=(DWORD *)&mode;
        
            // Set USB Mode 
            *temp=0;
            //mode.CM=3;      // Host 

            OUTREG32(&(*regs)->OTG.USBMODE, *temp);
        }

        // power on port
        {
            DWORD *temp;
            USB_HCSPARAMS_T hcs;
            temp=(DWORD *)&hcs;
            *temp=INREG32(&(*regs)->OTG.HCSPARAMS);

            if (hcs.PPC) 
            {
                USB_PORTSC_T portsc;
                DWORD * temp2= (DWORD *)&portsc;

                *temp2 = INREG32(&(*regs)->OTG.PORTSC);
                portsc.PP = 1;
                SETREG32(&(*regs)->OTG.PORTSC, *temp2);
            }   

            // InitializeMux();  
        }
    }
    else
    {
        // Pull Down VBUS
        USBOTGVbusControl(FALSE);

        ConfigOTGClient(*regs);

        // Reset all interrupts
        {
            USB_USBSTS_T temp;
            DWORD *t=(DWORD*)&temp;
            *t=0;
            temp.UI=1;  temp.UEI=1; temp.PCI=1; temp.FRI=1; 
            temp.SEI=1; temp.AAI=1; temp.URI=1; temp.SRI=1;
            temp.SLI=1; temp.ULPII=1;
            OUTREG32(&(*regs)->OTG.USBSTS, *t);
        }
        
        OUTREG32(&(*regs)->OTG.USBINTR, 0x10157);
    }

    // Last to do: Enable the ID Pin Interrupt in OTGSC
    {
        USB_OTGSC_T temp;
        DWORD       *t = (DWORD *)&temp;

        *t = INREG32(&(*regs)->OTG.OTGSC);
        temp.IDIE = 1;
        OUTREG32(&(*regs)->OTG.OTGSC, *t);
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: GetSysIntr
//
//  Description: This function is to return the SYSINTR value of the USB OTG 
//               Core.
//
//  Parameters:
//     NULL
//     
//  Returns:
//     SYSINTR_USBOTG(defined in file bsp_cfg.h)
//
//------------------------------------------------------------------------------
DWORD GetSysIntr(DWORD irq)
{
    INT32 aIrqs[3];
    DWORD sysintr;

    aIrqs[0] = -1;
    aIrqs[1] = OAL_INTR_TRANSLATE;
    aIrqs[2] = irq;
        
    KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(aIrqs), &sysintr, sizeof(DWORD), NULL);
    
    return sysintr;
}


//------------------------------------------------------------------------------
// Function: BSPUsbXvrSetPhyPowerMode
// 
// Description: This function is called by platform when the platform wants to 
// suspend/resume the PHY.
//
// Parameters: 
//     regs
//         [IN] Points to USB related registers mapped to virtual address
//     bResume
//         [IN] TRUE - request to resume, FALSE - request suspend
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
static void BSPUsbXvrSetPhyPowerMode(CSP_USB_REGS *pUSBRegs, BOOL bResume)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("BSPUsbXvrSetPhyPowerMode bResume = %d\r\n"),bResume));
    if (bResume)
    {
        if (INREG32(&pUSBRegs->OTG.PORTSC[0]) & 0x800000)
        {
            // Clear the PHCD
            CLRREG32(&pUSBRegs->OTG.PORTSC[0], INREG32(&pUSBRegs->OTG.PORTSC[0]) & 0x800000);
            // Force Resume bit(sets FPR bit)
            OUTREG32(&pUSBRegs->OTG.PORTSC[0], INREG32(&pUSBRegs->OTG.PORTSC[0])|0x40);
        }
    }
    else
    {
        USB_PORTSC_T portsc;
        DWORD *temp = (DWORD *)&portsc;

        *temp = INREG32(&(pUSBRegs)->OTG.PORTSC);
        portsc.PHCD = 1;
        OUTREG32(&(pUSBRegs)->OTG.PORTSC, *temp);
    }
}


//------------------------------------------------------------------------------
// Function: RegisterCallback
//
// Description: This function will be called by XVC to register the callback 
// function during the XVC_Init.
// 
// Parameters: 
//      Pointer to USB Callback function pointers
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void RegisterCallback(BSP_USB_CALLBACK_FNS *pfn)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+RegisterCallback\r\n")));
    if (pfn)
    {
        pfn->pfnUSBPowerDown = NULL;
        pfn->pfnUSBPowerUp = NULL;
        //pfn->pfnUSBPowerUp = BSPUsbXvrPowerUp;
        //pfn->pfnUSBPowerDown = BSPUsbXvrPowerDown;
        pfn->pfnUSBSetPhyPowerMode = BSPUsbXvrSetPhyPowerMode;
        DEBUGMSG(ZONE_FUNCTION, (TEXT("RegisterCallback for XVC\r\n")));
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-RegisterCallback\r\n")));
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
    DWORD temp;
    // 0x608 bit 24 = 0
    temp = INREG32(&regs->USB_UTMI_CTRL);
    temp &= ~(1<<24);
    OUTREG32(&regs->USB_UTMI_CTRL, temp);
    
    // 0x608 bit 24 = 1
    temp = INREG32(&regs->USB_UTMI_CTRL);
    temp |= (1<<24);
    OUTREG32(&regs->USB_UTMI_CTRL, temp);
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSetPHYEnable
//
//  This is called by the public common CSP library to disable/enable USB phy.
//
//  Parameters:
//      regs - Pointer to the 3 USB Core registers.
//      blEnalbe - TURE:  enable PHY
//                 FALSE: disalbe PHY
//
//  Returns:
//      
//
//-----------------------------------------------------------------------------
void BSPUsbSetPHYEnable(CSP_USB_REGS * regs, BOOL blEnable)
{
    DWORD dwUTMICTL;
    if(blEnable)
    {
        dwUTMICTL = INREG32(&(regs)->USB_UTMI_CTRL);
        dwUTMICTL = dwUTMICTL | (1<<24);
        OUTREG32(&(regs)->USB_UTMI_CTRL, dwUTMICTL);
    }
    else
    {
        dwUTMICTL = INREG32(&(regs)->USB_UTMI_CTRL);
        dwUTMICTL = dwUTMICTL & (~(1<<24));
        OUTREG32(&(regs)->USB_UTMI_CTRL, dwUTMICTL);
    }
}

