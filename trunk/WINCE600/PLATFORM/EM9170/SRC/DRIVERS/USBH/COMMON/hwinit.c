//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

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
#include "mx25_usb.h"
#include "common_usbname.h"
#include "common_usbcommon.h"
#include <..\USBD\OS\oscheckkitl.c>

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
     
    return;
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
	// Set the USB OTG power pin
#ifdef EM9170
	//
	// CS&ZHL APR-16-2011: supporting EM9170 config
	//
#ifdef BSP_CPU_TO1
    // Pullup VBUS
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

#else
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
	//
	// USB_OTG_OC signal has 2 inputs, so Input selection should be made
	//
	DDKIomuxSelectInput(DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_USB_OC, 0);		//select pin D10 as true input signal

#endif	// BSP_CPU_TO1
#else	// -> PDK1_7
	// Set the USB OTG power pin
#ifdef BSP_CPU_TO1
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
    
#else
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

#endif	//BSP_CPU_TO1
#endif	//EM9170
}
  
//------------------------------------------------------------------------------
// Function: InitializeHost2Mux
//
// Description: This function is to configure the IOMUX for USB HOST 2 Core
//
// Parameters:
//     NULL
//     
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void InitializeHost2Mux()
{
#ifdef EM9170
	//
	// CS&ZHL APR-16-2011: EM9170 USB Host Port2 use fixed 5V supply, 
	//										pin xxx_D9 and xx_D8 are used as GPIO18 and GPIO19 respectively
	//
    // Set the USB OC pin
    //DDKIomuxSetPinMux(DDK_IOMUX_PIN_D8, DDK_IOMUX_PIN_MUXMODE_ALT6,
    //                    DDK_IOMUX_PIN_SION_REGULAR); 

	//
	// CS&ZHL JUN-9-2011: should use pull-up config for USB_OC
	//
	//DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D8,
    //                     DDK_IOMUX_PAD_SLEW_SLOW,
    //                     DDK_IOMUX_PAD_DRIVE_NORMAL,
    //                     DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
    //                     DDK_IOMUX_PAD_PULL_UP_100K,
    //                     DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
    //                     DDK_IOMUX_PAD_VOLTAGE_3V3);

	//
	// CS&ZHL AUG-10-2011: The pin D8 is used as GPIO19 or ISA_CS1# in EM9170, 
	//                                      暂时安排USBH2_OC到PWM1_PWMO管脚 
	//
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_PWM, DDK_IOMUX_PIN_MUXMODE_ALT6, DDK_IOMUX_PIN_SION_REGULAR); 
	DDKIomuxSetPadConfig(DDK_IOMUX_PAD_PWM,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_UP_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

#ifdef BSP_CPU_TO1
#else
	//RETAILMSG(1, (L"InitializeHost2Mux::USBH2_OC <- pin-D8 in EM9170\r\n"));
    //DDKIomuxSelectInput(DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_USB_OC, 0x0);		// 0x1: route to pin-PWM, 0x0: to pin-D8;
	RETAILMSG(1, (L"InitializeHost2Mux::USBH2_OC <- pin-PWM in EM9170\r\n"));
    DDKIomuxSelectInput(DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_USB_OC, 0x1);		// 0x1: route to pin-PWM, 0x0: to pin-D8;
#endif
#else	// -> iMX25PDK
    // Set the USB Host 2 power pin
    // Use it as GPIO
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D9, DDK_IOMUX_PIN_MUXMODE_ALT5,
                        DDK_IOMUX_PIN_SION_REGULAR);
    
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D9,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_DOWN_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_DISABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    
    DDKGpioSetConfig(DDK_GPIO_PORT4, 11, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT4, 11, 1);		// CS&ZHL JUN-9-2011: Active High for RT9702

    // Set the USB OC pin
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D8, DDK_IOMUX_PIN_MUXMODE_ALT6,
                        DDK_IOMUX_PIN_SION_REGULAR); 

	//DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D8,
    //                     DDK_IOMUX_PAD_SLEW_SLOW,
    //                     DDK_IOMUX_PAD_DRIVE_NORMAL,
    //                     DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
    //                     DDK_IOMUX_PAD_PULL_DOWN_100K,
    //                     DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
    //                     DDK_IOMUX_PAD_VOLTAGE_3V3);
	//
	// CS&ZHL JUN-9-2011: should use pull-up config for USB_OC
	//
	DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D8,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_UP_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

#ifdef BSP_CPU_TO1
#else
    DDKIomuxSelectInput(DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_USB_OC, 0x0);
#endif

    // Set the USB OR BT pin as GPIO
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_A21, DDK_IOMUX_PIN_MUXMODE_ALT5,
                        DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_A21,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_HIGH,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_DOWN_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    //  Set it to 0 to select the USB
    DDKGpioSetConfig(DDK_GPIO_PORT2, 7, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT2, 7, 0);

#endif	//EM9170
}
    
//------------------------------------------------------------------------------
// Function: InitializeMux
//
// This function is to configure the IOMUX for USB Host Cores
//
// Parameters:
//     sel
//         [IN] Selection of the USB cores (0 - OTG, 1 - HOST2)
//     
// Returns:
//     offset of the USB core register to be configured.
//
//------------------------------------------------------------------------------
static DWORD InitializeMux(int sel)
{
    DWORD off = offset(CSP_USB_REGS, H2) + offset(CSP_USB_REG, CAPLENGTH);
     
    if (sel == 0) 
	{   // OTG
        InitializeOTGMux();
        off = offset(CSP_USB_REG, CAPLENGTH);
		//RETAILMSG(1, (L"InitializeMux::USB OTG Offset=0x%x\r\n", off));
    }
    else 
	{
        InitializeHost2Mux(); // HOST 2
        off = offset(CSP_USB_REGS, H2) + offset(CSP_USB_REG, CAPLENGTH);
		//RETAILMSG(1, (L"InitializeMux::USB Host2 Offset=0x%x\r\n", off));
    }       
    
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
//               core, sel = 1 means USB High Speed HOST 2 core.
//     
// Returns:
//     Speed of the USB core - HIGH_SPEED or FULL_SPEED
//
//------------------------------------------------------------------------------
static int SelectUSBCore(WORD * sel)
{
    int CoreSpeed;
    
#if (USB_HOST_MODE == 0)||(USB_HOST_MODE == 1)
    // OTG port, be sure to turn off the USB function driver.   
    *sel = 0;
#if (USB_HOST_MODE == 0)  // OTG Full Speed - not implemented here
    CoreSpeed = FULL_SPEED;
#else                     // OTG High Speed - Internal transceiver(default)
    CoreSpeed = HIGH_SPEED;
    DEBUGMSG(ZONE_INIT, (L"High Speed USB OTG Host\r\n"));
#endif
#else                     // High Speed USB Host 2
    *sel = 1;        
    CoreSpeed = HIGH_SPEED;
    
    DEBUGMSG(ZONE_INIT, (L"High Speed USB Host Port 2\r\n"));
    
#endif  
    return CoreSpeed;
}

//-----------------------------------------------------------------------------
//
//  Function: DumpDeviceState
//
//  This function is to dump the device status
//
//  Parameters:
//     state - pointer to PORTSC register
//     
//  Returns:
//     NULL
//
//-----------------------------------------------------------------------------
#ifdef DEBUG
static void DumpDeviceState( USB_PORTSC_T * state)  
{ 
    if (state->CCS)
        DEBUGMSG(1, (L"\t\tCurrent Connect Status: Attached\r\n"));
    if (state->CSC)
        DEBUGMSG (1, (L"\t\tConnect Status Change: Changed\r\n"));
    if (state->PE)
        DEBUGMSG (1, (L"\t\tPort Enabled\r\n"));
    if (state->PEC)
        DEBUGMSG (1, (L"\t\tPort Enable/Disable Change\r\n"));
    if (state->OCA)
        DEBUGMSG (1, (L"\t\tOver-current Active\r\n"));
    if (state->OCC)
        DEBUGMSG (1, (L"\t\tOver-current Change\r\n"));
    if (state->FPR)
        DEBUGMSG (1, (L"\t\tForce Port Resume\r\n"));
    if (state->SUSP)
        DEBUGMSG (1, (L"\t\tSuspend\r\n"));
    if (state->PR)
        DEBUGMSG (1, (L"\t\tPort Reset\r\n"));
    if (state->HSP)
        DEBUGMSG (1, (L"\t\tHigh-Speed Port \r\n"));
    
    DEBUGMSG (1, (L"\t\tLine Status: %x", state->LS));
    switch (state->LS) 
    {
        case 0:
            DEBUGMSG (1, (L"\t\t\tSE0\r\n"));
            break;
        case 1:
            DEBUGMSG (1, (L"\t\t\tJ-state\r\n"));
            break;
        case 2:
            DEBUGMSG (1, (L"\t\t\tK-state\r\n"));
            break;
        default:
            DEBUGMSG (1, (L"\t\t\tUndefined\r\n"));
            break;
    }

    if (state->PP)
        DEBUGMSG (1, (L"\t\t??? Should be 0 for device\r\n"));
    if (state->PO)
        DEBUGMSG (1, (L"\t\tPort Owner\r\n"));
    if (state->PIC) {
        DEBUGMSG (1, (L"\t\tPort Indicator Control"));
        switch (state->PIC) {
            case 1:
                DEBUGMSG (1, (L"\t\t\tAmber\r\n"));
                break;
            case 2:
                DEBUGMSG (1, (L"\t\t\tGreen\r\n"));
                break;
          
            default:
                DEBUGMSG (1, (L"\t\t\tUndefined\r\n"));
                break;
        }
    }   
    if (state->PTC) 
        DEBUGMSG (1, (L"\t\tPort Test Control: %x\r\n", state->PTC));
        
    if (state->WKCN) 
        DEBUGMSG (1, (L"\t\tWake on Connect Enable (WKCNNT_E)\r\n"));
                        
    if (state->WKDC) 
        DEBUGMSG (1, (L"\t\tWake on Disconnect Enable (WKDSCNNT_E) \r\n"));

    if (state->WKOC) 
        DEBUGMSG (1, (L"\t\tWake on Over-current Enable (WKOC_E) \r\n"));
    
    if (state->PHCD) 
        DEBUGMSG (1, (L"\t\tPHY Low Power Suspend - Clock Disable (PLPSCD) \r\n"));

    if (state->PFSC) 
        DEBUGMSG (1, (L"\t\tPort Force Full Speed Connect \r\n"));

    DEBUGMSG (1, (L"\t\tPort Speed: %x->", state->PSPD));
    switch (state->PSPD) 
    {
        case 0:
            DEBUGMSG (1, (L"\t\t\tFull Speed\r\n"));
            break;
        case 1:
            DEBUGMSG (1, (L"\t\t\tLow Speed\r\n"));
            break;
        case 2:
            DEBUGMSG (1, (L"\t\t\tHigh Speed\r\n"));
            break;
        
        default:
            DEBUGMSG (1, (L"\t\t\tUndefined\r\n"));
            break;
    }
    DEBUGMSG (1, (L"\t\tParallel Transceiver Width:%x->", state->PTW));
    if (state->PTW) 
        DEBUGMSG (1, (L"\t\t\t16 bits\r\n"));
    else
        DEBUGMSG (1, (L"\t\t\t8 bits\r\n"));
    if (state->STS) 
        DEBUGMSG (1, (L"\t\tSerial Transceiver Select \r\n"));
    
    DEBUGMSG (1, (L"\t\tParallel Transceiver Select:%x->", state->PTS));
    switch (state->PTS) {
        case 0:
            DEBUGMSG (1, (L"\t\t\tUTMI/UTMI+\r\n"));
            break;
        case 1:
            DEBUGMSG (1, (L"\t\t\tPhilips Classic\r\n"));
            break;
        case 2:
            DEBUGMSG (1, (L"\t\t\tULPI\r\n"));
            break;
        case 3:
            DEBUGMSG (1, (L"\t\t\tSerial/1.1 PHY (FS Only)\r\n"));
            break;
        default:
            DEBUGMSG (1, (L"\t\t\tUndefined\r\n"));
            break;
    }                   
}

//-----------------------------------------------------------------------------
//
//  Function: DumpUSBRegs
//
//  This function is to dump the USB Register detail
//
//  Parameters:
//     regs - Pointer to 3 USB Core Register
//     sel - 0: H2, 1: H1, 2: OTG
//     
//  Returns:
//     NULL
//
//-----------------------------------------------------------------------------
static void DumpUSBRegs(PCSP_USB_REGS regs, WORD sel)
{    
    CSP_USB_REG *pReg;
    
    if (sel == 0)
        RETAILMSG (1, (L"Dump OTG Regs\r\n"));
    else
        RETAILMSG (1, (L"Dump H2 Regs\r\n"));
    
    if (sel == 0)
        pReg = (PCSP_USB_REG)(&(regs->OTG));
    else
        pReg=(PCSP_USB_REG)(&(regs->H2));

    DEBUGMSG (1,(L"\tID(%xh)=%x\r\n",offset(CSP_USB_REG,ID), INREG32(&pReg->ID)));
    DEBUGMSG (1,(L"\tHWGENERAL(%xh)=%x\r\n",offset(CSP_USB_REG,HWGENERAL),INREG32(&pReg->HWGENERAL)));
    DEBUGMSG (1,(L"\tHWHOST(%xh)=%x\r\n",offset(CSP_USB_REG,HWHOST),INREG32(&pReg->HWHOST)));
    DEBUGMSG (1,(L"\tHWDEVICE(%xh)=%x\r\n",offset(CSP_USB_REG,HWDEVICE),INREG32(&pReg->HWDEVICE)));
    DEBUGMSG (1,(L"\tHWTXBUF(%xh)=%x\r\n",offset(CSP_USB_REG,HWTXBUF),INREG32(&pReg->HWTXBUF)));
    DEBUGMSG (1,(L"\tHWRXBUF(%xh)=%x\r\n",offset(CSP_USB_REG,HWRXBUF),INREG32(&pReg->HWRXBUF)));
    DEBUGMSG (1,(L"\tCAPLENGTH(%xh)=%x\r\n",offset(CSP_USB_REG,CAPLENGTH),INREG8(&pReg->CAPLENGTH)));
    DEBUGMSG (1,(L"\tHCIVERSION(%xh)=%x\r\n",offset(CSP_USB_REG,HCIVERSION),INREG16(&pReg->HCIVERSION)));
    DEBUGMSG (1,(L"\tHCSPARAMS(%xh)=%x\r\n",offset(CSP_USB_REG,HCSPARAMS),INREG32(&pReg->HCSPARAMS)));
    DEBUGMSG (1,(L"\tHCCPARAMS(%xh)=%x\r\n",offset(CSP_USB_REG,HCCPARAMS),INREG32(&pReg->HCCPARAMS)));
    DEBUGMSG (1,(L"\tDCIVERSION(%xh)=%x\r\n",offset(CSP_USB_REG,DCIVERSION),INREG16(&pReg->DCIVERSION)));
    DEBUGMSG (1,(L"\tDCCPARAMS(%xh)=%x\r\n",offset(CSP_USB_REG,DCCPARAMS),INREG32(&pReg->DCCPARAMS)));
    DEBUGMSG (1,(L"\tUSBCMD(%xh)=%x\r\n",offset(CSP_USB_REG,USBCMD),INREG32(&pReg->USBCMD)));
    DEBUGMSG (1,(L"\tUSBSTS(%xh)=%x\r\n",offset(CSP_USB_REG,USBSTS),INREG32(&pReg->USBSTS)));
    DEBUGMSG (1,(L"\tUSBINTR(%xh)=%x\r\n",offset(CSP_USB_REG,USBINTR),INREG32(&pReg->USBINTR)));
    DEBUGMSG (1,(L"\tPORTSC(%xh)[0]=%x\r\n",offset(CSP_USB_REG,PORTSC[0]),INREG32(&pReg->PORTSC[0])));
    {
        USB_PORTSC_T state;
        DWORD * temp=(DWORD*)&state;
        *temp=INREG32(&pReg->PORTSC[0]);
        DumpDeviceState( & state);
    }
    DEBUGMSG (1,(L"\tOTGSC(%xh)=%x\r\n",offset(CSP_USB_REG,OTGSC),INREG32(&pReg->OTGSC)));
    DEBUGMSG (1,(L"\tUSBMODE(%xh)=%x\r\n",offset(CSP_USB_REG,USBMODE),INREG32(&pReg->USBMODE)));
    DEBUGMSG (1,(L"\tULPI_VIEWPORT(%xh)=%x\r\n",offset(CSP_USB_REG,ULPI_VIEWPORT),INREG32(&pReg->ULPI_VIEWPORT)));

    DEBUGMSG (1,(L"\t*********************\r\n"));
    DEBUGMSG (1,(L"\tAddress of ASYNCLISTADDR at 0x%x\r\n", &pReg->T_158H.ASYNCLISTADDR));
    DEBUGMSG (1,(L"\tASYNCLISTADDR(%xh)=%x\r\n",offset(CSP_USB_REG,T_158H),INREG32(&pReg->T_158H.ASYNCLISTADDR)));
    DEBUGMSG (1,(L"\tENDPTSETUPSTAT(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTSETUPSTAT),INREG32(&pReg->ENDPTSETUPSTAT)));
    DEBUGMSG (1,(L"\tENDPTPRIME(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTPRIME),INREG32(&pReg->ENDPTPRIME)));
    DEBUGMSG (1,(L"\tENDPTFLUSH(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTFLUSH),INREG32(&pReg->ENDPTFLUSH)));
    DEBUGMSG (1,(L"\tENDPTSTATUS(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTSTATUS),INREG32(&pReg->ENDPTSTATUS)));
    DEBUGMSG (1,(L"\tENDPTCOMPLETE(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTCOMPLETE),INREG32(&pReg->ENDPTCOMPLETE)));
    DEBUGMSG (1,(L"\tENDPTCTRL0(%xh)=%x\r\n",offset(CSP_USB_REG,ENDPTCTRL0),INREG32(&pReg->ENDPTCTRL0)));
    DEBUGMSG (1,(L"\t*********************\r\n"));
    
    DEBUGMSG (1,(L"\tUSB_CTRL(%xh)=%x\r\n",offset(CSP_USB_REGS,USB_CTRL),INREG32(&regs->USB_CTRL)));
    DEBUGMSG (1,(L"\tUSB_OTG_MIRROR(%xh)=%x\r\n",offset(CSP_USB_REGS,USB_OTG_MIRROR),INREG32(&regs->USB_OTG_MIRROR)));
}

#endif

//------------------------------------------------------------------------------
//
//  Function: ConfigH2
//
//  This function is to configure the USB H2 Core.
//
//  Parameters:
//     pRegs - Pointer to 2 USB Core(USB OTG and HOST 2 ports) Registers
//     
//  Returns:
//     NULL
//
//------------------------------------------------------------------------------
static void ConfigH2(CSP_USB_REGS *pRegs)
{
    USB_PORTSC_T portsc;
    USB_CTRL_T   ctrl;
    USB_USBCMD_T cmd;
    DWORD *temp;
            
    //RETAILMSG(1, (TEXT("ConfigUSBH2 -> Stop the controller first\r\n")));		//CS&ZHL JUN-8-2011: debug only
    // Stop the controller first
    {
        temp = (DWORD *)&cmd;
        *temp = INREG32(&pRegs->H2.USBCMD);
        cmd.RS = 0;
        OUTREG32(&pRegs->H2.USBCMD, *temp);
        while ((INREG32(&pRegs->H2.USBCMD) & 0x1) == 0x1){
             Sleep(100);
        }
    }

    // Do a reset first no matter what
    {
        temp = (DWORD *)&cmd;
        *temp = INREG32(&pRegs->H2.USBCMD);
        cmd.RST = 1;
        OUTREG32(&pRegs->H2.USBCMD, *temp);
        while ((INREG32(&pRegs->H2.USBCMD) & 0x1<<1) == (0x1<<1)){
             Sleep(100);
        }

    }
            
    //usb_hs2_serial_interface_configure
    temp  = (DWORD *)&portsc;
    *temp = INREG32(&pRegs->H2.PORTSC);
    portsc.PTS = 0x3;
    OUTREG32(&pRegs->H2.PORTSC, *temp);

    //usb_h2_interrupt_enable
    temp =  (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.H2WIE = 1;
    OUTREG32(&pRegs->USB_CTRL, *temp);

    //Host 2 Serial Interface Configuration
    temp =  (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.H2SIC = 0;
    OUTREG32(&pRegs->USB_CTRL, *temp);

    //Host 2 pulldwn_dpdm must be set to enable
    temp =  (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.IDPUEDWN = 1;
    OUTREG32(&pRegs->USB_CTRL, *temp);

    // usb_h2_OverCurrent_Polarity
    temp =  (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
#ifdef BSP_CPU_TO1
    ctrl.OTGOCPOL = 0;		// TO 1.0 "OCPOL" in TO 1.1 defined as "OTGOCPOL", ugly looking
#else    
    //ctrl.UH2OCPOL = 0;		// Low Active
    ctrl.UH2OCPOL = 1;		// High Active
	RETAILMSG(1, (TEXT("ConfigUSBH2::polarity of USBH2 overcurrent is HIGH active\r\n")));		//CS&ZHL AUG-11-2011: debug only
#endif
    OUTREG32(&pRegs->USB_CTRL, *temp);


    //usb_h2_power_mask_enable
    temp =  (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
#ifdef BSP_CPU_TO1
    ctrl.H2PM = 0;
#else
    ctrl.H2PM = 1;		//The USBPWR and OC signals are not used by the host core
	RETAILMSG(1, (TEXT("ConfigUSBH2::The USBPWR and OC pins are not used by USBH2\r\n")));		//CS&ZHL JUN-8-2011: debug only
#endif
    OUTREG32(&pRegs->USB_CTRL, *temp);

    // usb_h2_tll_disable
    temp =  (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.H2SDT = 1;
    OUTREG32(&pRegs->USB_CTRL, *temp);

    // usb_transceiver_enable(internal PHY)
    temp=(DWORD *)&ctrl; 
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.USBTE=1;    
    OUTREG32(&pRegs->USB_CTRL, *temp);


    // USB Host Power Polarity Active High
    temp=(DWORD *)&ctrl; 
    *temp = INREG32(&pRegs->USB_CTRL);
#ifdef BSP_CPU_TO1
    ctrl.OTGPP=1;  // this was changed to 0, need to investigate 
                   // TO 1.0 "PP" in TO 1.1 defined as "OTGPP", ugly looking
#else
    ctrl.UH2PP=0;
#endif
    OUTREG32(&pRegs->USB_CTRL, *temp);

    //RETAILMSG(1, (TEXT("ConfigUSBH2 -> BSPUsbSetBusConfig(...)\r\n")));		//CS&ZHL JUN-8-2011: debug only
    BSPUsbSetBusConfig((PUCHAR)(&pRegs->H2));

    return;
}

//------------------------------------------------------------------------------
// Function: ConfigOTG
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
static void ConfigOTG(CSP_USB_REGS *pRegs, int speed)
{
    USB_PORTSC_T portsc;
    USB_CTRL_T   ctrl;
    USB_USBMODE_T mode;
    USB_USBCMD_T cmd;   
    DWORD *temp;   
    
    UNREFERENCED_PARAMETER(speed);   

    // Stop the controller first
    {
        temp = (DWORD *)&cmd;
        *temp = INREG32(&pRegs->OTG.USBCMD);
        cmd.RS = 0;
        OUTREG32(&pRegs->OTG.USBCMD, *temp);
        while ((INREG32(&pRegs->OTG.USBCMD) & 0x1) == 0x1){
             Sleep(100);
        }
    }

    // Do a reset first no matter what
    {
        temp = (DWORD *)&cmd;
        *temp = INREG32(&pRegs->OTG.USBCMD);
        cmd.RST = 1;
        OUTREG32(&pRegs->OTG.USBCMD, *temp);
        while ((INREG32(&pRegs->OTG.USBCMD) & 0x1<<1) == (0x1<<1)){
             Sleep(100);
        }
    }

    temp  = (DWORD *)&portsc;
    *temp = INREG32(&pRegs->OTG.PORTSC);
    portsc.PTS = 0x0;     // UTMI/UTMI+ transceiver for Senna
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

     // otg_setmode
    temp = (DWORD *)&cmd;
    *temp = INREG32(&pRegs->OTG.USBCMD);
    cmd.RST = 1;
    OUTREG32(&pRegs->OTG.USBCMD, *temp);

    while (INREG32(&pRegs->OTG.USBCMD)& (0x1 << 1));

     temp = (DWORD *)&mode;
    *temp = INREG32(&pRegs->OTG.USBMODE);
    mode.CM = 0x3;
    OUTREG32(&pRegs->OTG.USBMODE, *temp);

    Sleep(10);
    if ((INREG32(&pRegs->OTG.USBMODE)& 0x3) != 0x3)
    {
        return;
    }

    BSPUsbSetBusConfig((PUCHAR)(&pRegs->OTG));

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
        DEBUGMSG (1, (TEXT("Host does not control power\r\n")));

 #ifdef OTG_TEST_MODE 
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

     // Disable async and periodic schedule
    // Disable async and periodic schedule
    temp = (DWORD *)&cmd;
    *temp = 0;
    *temp = INREG32(&pRegs->OTG.USBCMD);
    *temp = *temp & ~0x30;
    OUTREG32(&pRegs->OTG.USBCMD, *temp);

 
    // Go to suspend mode, please
    temp  = (DWORD *)&portsc;
    *temp = 0;
    *temp = INREG32(&pRegs->OTG.PORTSC);
    *temp  |= 0x00000080;
    OUTREG32(&pRegs->OTG.PORTSC, *temp);

    // Set to TEST_MODE, please
    temp  = (DWORD *)&portsc;
    *temp = 0;
    *temp = INREG32(&pRegs->OTG.PORTSC);
    portsc.PTC = 0x4; // PTC = 4 -> test pattern
    OUTREG32(&pRegs->OTG.PORTSC, *temp);

    // Set run/stop bit now
    temp = (DWORD *)&cmd;
    *temp = 0;
    *temp = INREG32(&pRegs->OTG.USBCMD);
    cmd.RS = 1;
    OUTREG32(&pRegs->OTG.USBCMD, *temp);

    while(1);

#else

#ifdef BSP_CPU_TO1
    // the following code was added, need investigation
#if 0   // should examine
    temp =  (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.OTGOCPOL = 0;
    OUTREG32(&pRegs->USB_CTRL, *temp);
    
    temp=(DWORD *)&ctrl; 
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.OTGPP=0;
    OUTREG32(&pRegs->USB_CTRL, *temp);
#endif
#else
    // OTG_OverCurrent_Polarity
    temp =  (DWORD *)&ctrl;
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.OTGOCPOL = 0;
    OUTREG32(&pRegs->USB_CTRL, *temp);
    
    temp=(DWORD *)&ctrl; 
    *temp = INREG32(&pRegs->USB_CTRL);
    ctrl.OTGPP=0;
    OUTREG32(&pRegs->USB_CTRL, *temp);
#endif

    temp = (DWORD *)&cmd;
    *temp = INREG32(&pRegs->OTG.USBCMD);
    cmd.RS = 0;
    OUTREG32(&pRegs->OTG.USBCMD, *temp);
    
    // Handshake to wait for Halted
    while ((INREG32(&pRegs->OTG.USBSTS) & 0x1000) != 0x1000);

#endif

     // Enable interrupts
    OUTREG32(&pRegs->OTG.USBINTR, 0x5ff);
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
BOOL InitializeTransceiver(PCSP_USB_REGS* regs, DWORD * phyregs, DWORD* dwOffset,
                           DWORD * sysintr, DWORD dwOTGSupport, TCHAR *pOTGGroup)
{
    DWORD dwRet = TRUE, speed, temp;
    int off, irq;
    WORD sel = 0;
    WORD StringSize = 0;
    
    StringSize = sizeof(gszOTGGroup) / sizeof(TCHAR);
    StringCchCopy(gszOTGGroup,StringSize,pOTGGroup);
    gdwOTGSupport = dwOTGSupport;

    dwRet = SelectUSBCore(&sel);
    speed = dwRet;
    gSel = sel;
    
    if(sel == 0)
    {
        //only OTG port need to check
        if(FslUfnIsUSBKitlEnable())
        {
            RETAILMSG(1,(_T("USB Host load failure because Usb Kitl Enabled\r\n")));
            return FALSE;
        }
    }
    
    off = InitializeMux(sel);  
    USBClockInit();
    
    if (sel == 0) {
        ConfigOTG(*regs, speed);
        irq = IRQ_USB_OTG;
    }
    else {
        ConfigH2(*regs);
        irq = IRQ_USB_HOST;
    }

    if (irq == IRQ_USB_OTG)
    {
        INT32 aIrqs[3];
        aIrqs[0] = -1;
        aIrqs[1] = OAL_INTR_TRANSLATE;
        aIrqs[2] = IRQ_USB_OTG;
        KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(aIrqs), sysintr, sizeof(DWORD), NULL);
    }
    else
        KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD), sysintr, sizeof(DWORD), NULL);      
        
    DEBUGMSG(1, (TEXT("InitializeTransceiver: IRQ=%d, sysIntr=%d\r\n"), irq, *sysintr));
       
    {
        USB_USBMODE_T mode;
        DWORD * ptemp=(DWORD *)&mode;
        
        // Set USB Mode 
        *ptemp = 0;
        mode.CM = 3;      // Host 

 
        if (sel == 0)
            OUTREG32(&(*regs)->OTG.USBMODE, *ptemp);
        else
            OUTREG32(&(*regs)->H2.USBMODE, *ptemp);

    }

    // power on port
    {
        if (sel == 0)
        {
            DWORD *ptemp;
            USB_HCSPARAMS_T hcs;
            ptemp=(DWORD *)&hcs;
            *ptemp=INREG32(&(*regs)->OTG.HCSPARAMS);

            if (hcs.PPC) 
            {
                USB_PORTSC_T portsc;
                DWORD * temp2= (DWORD *)&portsc;

                *temp2 = INREG32(&(*regs)->OTG.PORTSC);
                portsc.PP = 1;
                SETREG32(&(*regs)->H2.PORTSC, *temp2);
            }   
        }
        else if (sel == 1)
        {
            DWORD *ptemp;
            USB_HCSPARAMS_T hcs;
            ptemp=(DWORD *)&hcs;
            *ptemp=INREG32(&(*regs)->H2.HCSPARAMS);

            if (hcs.PPC) 
            {
                USB_PORTSC_T portsc;
                DWORD * temp2= (DWORD *)&portsc;

                *temp2 = INREG32(&(*regs)->H2.PORTSC);
                portsc.PP = 1;
                SETREG32(&(*regs)->H2.PORTSC, *temp2);
            }   
        }

    }
    
    gRegs = (PCSP_USB_REGS)(*regs);
    gSel = sel;

    temp=*(DWORD*)regs;
    temp+=off;
    *(DWORD*)regs=temp;
    *phyregs+=off;
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
    USB_CTRL_T ctrl;
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
        pReg = (PCSP_USB_REG)(&(gRegs->H2));
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
        // currently, for IC issue, we can't enable WKCN & WKOC for HOST2,
        // otherwise, an unexpected interrupt will be issued
        if(portsc.CCS == 0)
        {
            if(gSel == 0)
            {
                portsc.WKCN = 1;
                portsc.WKOC = 1;
                portsc.WKDC = 0;        
            }
        }
        else
        {
            if(gSel == 0)
            {
#if defined USB_WAKEUP_CNANDDN
                portsc.WKDC = 1;
                portsc.WKCN = 0;
#else
                portsc.WKDC = 0;
                portsc.WKCN = 0;
#endif
                portsc.WKOC = 1;
            }
        }
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
    
    temp = (DWORD *)&ctrl;

    if (bEnable)
        *temp = INREG32(&gRegs->USB_CTRL);
    else
        *temp = 0;

    switch (gSel)
    {
    case 0:
        ctrl.OWIE = 1;
        ctrl.OUIE = 1;     
        break;
    case 1:
        ctrl.H2WIE = 1;
#if defined USB_WAKEUP_CNANDDN        
        ctrl.H2UIE = 1; 
#else
        ctrl.H2UIE = 0;
#endif
        break;
    default:
        break;
    }


    if (bEnable)
        OUTREG32(&gRegs->USB_CTRL, *temp);
    else
        CLRREG32(&gRegs->USB_CTRL, *temp);

     return;
}

//------------------------------------------------------------------------------
//
//  Function: BSPUsbCheckWakeUp
//
//  This function is called by CSP to clear the wakeup interrupt bit in USBCONTROL. According to
//  Senna specification, disable the wake-up enable bit also clear the interrupt request bit.
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
    volatile DWORD  *temp;
    USB_CTRL_T ctrl;
    BOOL fWakeUp = FALSE;

    temp = (DWORD *)&ctrl;
    *temp = INREG32(&gRegs->USB_CTRL);

 
    switch (gSel) {
    case 0:
        if( ctrl.OWIR == 1)
        {
            *temp = 0;
            fWakeUp = TRUE;
            ctrl.OWIE = 1;
        }
        break;
    case 1:
        if (ctrl.H2WIR == 1)
        {
            *temp = 0;
            fWakeUp = TRUE;
            ctrl.H2WIE = 1;
        }
  
    default:
        break;
    }

    if (fWakeUp)
        CLRREG32(&gRegs->USB_CTRL, *temp);  

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
    return FALSE;
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
    if (gSel == 0)
    {
        // Don't do anything for OTG
        return;
    }

	//
	// CS&ZHL JUN-7-2011: Only PDK can control USB Host2's Vbus
	//
#ifdef		EM9170
    UNREFERENCED_PARAMETER(blOn);

#else		// -> iMX25PDK
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D9, DDK_IOMUX_PIN_MUXMODE_ALT5,
                        DDK_IOMUX_PIN_SION_REGULAR);
    
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D9,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_DOWN_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_DISABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKGpioSetConfig(DDK_GPIO_PORT4, 11, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);

    if (blOn)
    {
        // turn on vbus
        DDKGpioWriteDataPin(DDK_GPIO_PORT4, 11, 1);
    }
    else
    {
        // turn off vbus
        DDKGpioWriteDataPin(DDK_GPIO_PORT4, 11, 0);
    }
#endif		//EM9170
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
    // We can't close VBUS for MX25 during system Resume (Power Up)
    // Otherwise there will be a Overcurrent Condition happenning,
    // Causing Serial Phy mal-functioning
    return FALSE;
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

