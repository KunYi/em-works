//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx25_usb.h
//
//  Provides definitions for usb module based on Freescale MX25 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX25_USB_H
#define __MX25_USB_H

#include <common_usb.h>

#if __cplusplus
extern "C" {
#endif

typedef struct {
    //****************OTG Registers ********************************************
                                //      off DEV OTG SPH MPH 
    UINT32 ID;                  // RO   0   *   *   *   *   Identification Register
    UINT32 HWGENERAL;           // RO   4   *   *   *   *   General Hardware Parameters
    UINT32 HWHOST;              // RO   8       *   *   *   Host Hardware Parameters
    UINT32 HWDEVICE;            // RO   C   *   *           Device Hardware Parameters
    UINT32 HWTXBUF;             // RO   0x10    *   *   *   *   TX Buffer Hardware Parameters
    UINT32 HWRXBUF;             // RO   0x14    *   *   *   *   RX Buffer Hardware Parameters 
    UINT32 RESERVED1[30];       //      0x18-0x8c
    UINT32 SBUSCFG;             //      0x90                Control for the System Bus Interface
    UINT32 RESERVED2[27];       //      0x94-0xfc 
    UINT8   CAPLENGTH;          // RO   0x100*  *   *   *   Capability Register Length
    UINT8   RESERVED3;          //      0x101
    UINT16 HCIVERSION;          // RO   0x102   *   *   *   Host Interface Version Number
    UINT32 HCSPARAMS;           // RO   0x104   *   *   *   Host Control Structural Parameters
    UINT32 HCCPARAMS;           // RO   0x108   *   *   *   Host Control Capability Parameters
    UINT32 RESERVED4[5];        //      0x10C-0x11f
    UINT16 DCIVERSION;          // RO   0x120*  *           Device Interface Version Number
    UINT16 RESERVED5;           //      0x122
    UINT32 DCCPARAMS;           // RO   0x124*  *           Device Control CapabilityParameters
    UINT32 RESERVED6[6];        //      0x128-0x13C
    UINT32 USBCMD;              //      0x140*  *   *   *   USB Command
    UINT32 USBSTS;              //      0x144*  *   *   *   USB Status
    UINT32 USBINTR;             //      0x148*  *   *   *   USB Interrupt Enable
    UINT32 FRINDEX;             //      0x14C*  *   *   *   USB Frame Index; Read/Write in host mode, Read in device mode
    UINT32 CTRLDSSENGMENT;      // RO   0x150               This register is not used in this implementation.
    union {
        UINT32 PERIODICLISTBASE;//      0x154   *   *   *   Frame List Base Address
        UINT32 USBADR;          //          *   *           USB Device Address
    }T_154H;                    // Read/Write (Writes must be DWord Writes)
    union {
        UINT32 ASYNCLISTADDR;   //      0x158   *   *   *   Next Asynchronous List Address
        UINT32 ENDPOINTLISTADDR;//          *   *           Address at Endpoint list in memory
    }T_158H;                    // Read/Write (Writes must be DWord Writes)
    UINT32 ASYNCTTSTS;          // 0x15C            *   Asynchronous Buffer Status For Embedded TT.
    UINT32 BURSTSIZE;           //      0x160*  *   *   *   Programmable Burst Size.
    UINT32 TXFILLTUNING;        //      0x164   *   *   *   Host Transmit Pre-Buffer Packet Tuning
    UINT32 TXTTFILLTUNING;      // 0x168                *   Host TT Transmit Pre-Buffer Packet Tuning
    UINT32 RESERVED7;           //      0x16C
    UINT32 ULPI_VIEWPORT;       //      0x170
    UINT32 RESERVED8;        //      0x174-0x178

    //The NAK interrupt is enabled to workaround the issue where the Device controller does not ACK 
    //the repeated status stages but NAKs them. ENDPTNAK and ENDPTNAKEN are the registers that are used to implement the workaround.
    UINT32 ENDPTNAK;            //  0x178
    UINT32 ENDPTNAKEN;          // 0x17c
    UINT32 CONFIGFLAG;          // RO   0x180   *   *   *   Configured Flag Register, This register is not used in this implementation. A read from this register returns a constant of a 00000001h to indicate that all port routings default to this host controller.
    UINT32 PORTSC[8];           //      0x184-0x1A0
                                //          *   *   *   *   Port Status/Control X
    UINT32 OTGSC;               //      0x1A4   *           On-The-Go(OTG) Status and Control
    UINT32 USBMODE;             //      0x1A8*  *   *   *   USB Device Mode
    UINT32 ENDPTSETUPSTAT;      //      0x1AC*  *           Endpoint Setup Status
    UINT32 ENDPTPRIME;          //      0x1B0*  *           Endpoint Initialization
    UINT32 ENDPTFLUSH;          //      0x1B4*  *           Endpoint De-Initialize
    UINT32 ENDPTSTATUS;         //      0x1B8*  *           Endpoint Status
    UINT32 ENDPTCOMPLETE;       //      0x1BC*  *           Endpoint Complete
    UINT32 ENDPTCTRL0;          //      0x1C0*  *           Endpoint Control 0
    UINT32 ENDPTCTRL    [15];   //      0x1C4-0x1FC
                                //          *   *           Endpoint Control 1-15

} CSP_USB_REG, * PCSP_USB_REG;

typedef volatile struct {
    CSP_USB_REG OTG;                    //  0
    UINT32 RESERVED[128];               //  0x200
    CSP_USB_REG H2;                     //  0x400
    UINT32       USB_CTRL;              //  0x600
    UINT32       USB_OTG_MIRROR;        //  0x604
    UINT32       USB_UTMI_CTRL;         //  0x608
    UINT32       USB_UTMI_CTRLTEST;     //  0x60C
} CSP_USB_REGS, * PCSP_USB_REGS;

typedef struct _USB_SBUSCFG {
    unsigned int AHBBRST:3;     // This field selects the burst signal of the AMBA master interface.
    unsigned int RESERVED:29;   // Reserved. These bits are reserved and should be set to zero.
}USB_SBUSCFG_T;

typedef struct _USB_CTRL {
    unsigned int OOCS:1;      // OTG OverCurrent State
                              // This bit is to show the OverCurrent state
                              // 1 = The otg has OverCurrent event
                              // 0 = The otg has no OverCurrent event
    unsigned int H2OCS:1;     // Host 2 OverCurrent State
                              // This bit is to show the OverCurrent state
                              // 1 = The host 2 has OverCurrent event
                              // 0 = The host 2 has no OverCurrent event
    unsigned int UH2OCPOL:1;  // HOST2 OverCurrent Polarity
                              // This bit selects the polarity of OverCurrent
                              // 1 = High Active(default)
                              // 0 = Low Active
    unsigned int OTGOCPOL:1;  // OTG OverCurrent Polarity
                              // This bit selects the polarity of OverCurrent
                              // 1 = High Active(default)
                              // 0 = Low Active
                              // "OCPOL" bit in TO 1.0
    unsigned int USBTE:1;     // USB Transceiver Enable
                              // This bit controls the USB transceiver for HOST 2
                              // 1 = USB transceiver for HOST 2 is enabled
                              // 0 = USB transceiver for HOST 2 is disabled(default)
    unsigned int H2SDT:1;     // Host 2 Serial TLL Disable
                              // This bit controls whether or not the Serial Transceiver Link Logic(FS-TLL)
                              // is enabled for the serial interface of Host Port 2
                              // 1 = Serial TLL is disabled
                              // 0 = Serial TLL is enabled(default)
    unsigned int IDPUEDWN:1;  // Ipp_pue_pulldwn_dpdm
                              // This bit is used to give a software control to force the signal
                              // ipp_pue_pulldwn_dpdm to the FULL-speed PHY: USBT
                              // 1 = Software enables this signal
                              // 0 = Software does not control this signal(default)    
    unsigned int IDPUEUP:1;   // Ipp_pue_pullup_dp
                              // This bit is used to give a software control to force the signal
                              // ipp_pue_pullup_dp to the FULL-speed PHY: USBT
                              // 1 = Software enables this signal
                              // 0 = Software does not control this signal(default)
    unsigned int IDPUIDP:1;   // Ipp_puimpel_pullup_dp
                              // This bit is used to give a software control to force the signal
                              // ipp_puimpel_pullup_dp to the FULL-speed PHY: USBT
                              // 1 = Software enables this signal
                              // 0 = Software does not control this signal(default)
    unsigned int XCSH2:1;     // Xcvr Clock Select for Host Port 2
                              // This bit is used to give a software control to force the host port 2 to use
                              // the internal 60MHz clock. It is when the PHY's 60MHz clock has not been
                              // supplied to the host port 2 but the host port 2 is in the xcvr supply clock
                              // mode.
                              // 1 = Force to use the internal 60MHz clock
                              // 0 = No need to use the internal 60MHz clock(default)
    unsigned int XCSO:1;      // Xcvr Clock Select for OTG Port 2
                              // This bit is used to give a software control to force the otg port to use the
                              // internal 60MHz clock. It is when the PHY's 60MHz clock has not been supplied
                              // to the otg port but the otg port is in the xcvr supply clock mode.
                              // 1 = Force to use the internal 60MHz clock
                              // 0 = No need to use the internal 60MHz clock(default)              
    unsigned int OTGPP:1;     // OTG Power Polarity
                              // The power polarity bit controls the polarity of pwr output signal
                              // 1 = High active
                              // 0 = Low active    
                              // "PP" bit in TO 1.0
    unsigned int UH2AHBLOCK:1;  
    unsigned int OTGAHBLOCK:1;
    unsigned int OTGIDWE:1;   // otg_id_wakeup_en
    unsigned int OTGVBUSWE:1; // otg_vbus_wakeup_en    
    unsigned int H2PM:1;      // Host 2 Power Mask
                              // The power mask bit controls whether or not the external Vbus Power and OverCurrent
                              // detection are active for the Host 2 port
                              // 1 = The USBPWR and OC pins are not used by the Host 2 core
                              // 0 = The USBPWR pin will assert with the Host 2 core's Vbus power Enable and the
                              // assertion of the OC input will be reported to the Host 2 core.
    unsigned int HBURST:1;    // hburst override function, override all INCR transfer to INCR8 transfer
    unsigned int UH2PP:1;     // USB Host 2 Power Polarity
                              // The power polarity bit controls the polarity of pwr output signal
                              // 1 = High active
                              // 0 = Low active    
    unsigned int H2WIE:1;     // Host 2 Wake-up Interrupt Enable
                              // This bit enables or disables the Host 2 wake-up interrupt. Disabling the interrupt 
                              // also clears the Interrupt request bit. Wake-up interrupt enable should be turned off 
                              // after receiving a wake-up interrupt and turned on again prior to going in suspend mode 
                              // 1 = Interrupt Enabled
                              // 0 = Interrupt Disabled
    unsigned int H2UIE:1;     // Host 2 ULPI interrupt enable
                              // Controls whether or not interrupts from the ULPI transceiver will trigger the wake-up 
                              // logic. This bit is only meaningfull when a ULPI transceiver is selected.
                              // 1 = ULPI transceiver interrupts activate the wake-up logic
                              // 0 = ULPI transceiver interrupts are ignored by the wakeup logic.
    unsigned int H2SIC:2;     // Host 2 Serial Interface Configuration
                              // Controls the interface type of the Host 2 port when used with a serial transceiver. 
                              // This bit field allows for configuring the serial interface for Single Ended or Differential 
                              // operation combined with Bidirectional or Unidirectional operation.
                              // 0 0 Differential / Unidirectional (6-wire)
                              // 0 1 Differential / Bidirectional (4-wire)
                              // 1 0 Single Ended / Unidirectional (6-wire)
                              // 1 1 Single Ended / Bidirectional (3-wire)
    unsigned int H2WIR:1;     // Host 2 Wake-up Interrupt Request(RO)
                              // Indicates a pending Wake-up request on Host port 2. This bit is cleared by disabling the 
                              // interrupt. The interrupt must be disabled for at least 2 clock cycles of the standby clock.
                              // 1 = Wake-up interrupt received
                              // 0 = No Wake-up interrupt received                  
    unsigned int OPM:1;       // OTG Power Mask
                              // The power mask bit controls whether or not the external Vbus Power and OverCurrent detection
                              // are active for the OTG port.
                              // 1 = The USBPWR and OC pins are not used by the OTG core.
                              // 0 = The USBPWR pin will assert with the OTG core.s Vbus power Enable and the assertion of the OC input will be reported to the OTG core.
    unsigned int R2:2;        // Reserved. These bits are reserved and should be set to zero.
    unsigned int OWIE:1;      // OTG Wake-up Interrupt Enable
                              // This bit enables or disables the OTG wake-up interrupt. Disabling the interrupt also clears the Interrupt request bit. Wake-up interrupt enable should be turned off after receiving a wake-up
                              // interrupt and turned on again prior to going in suspend mode
                              // 1 = Interrupt Enabled
                              // 0 = Interrupt Disabled
    unsigned int OUIE:1;      // OUIE -- OTG ULPI interrupt enable
                              // Controls whether or not interrupts from the ULPI transceiver will trigger the wake-up logic. This bit is only meaningfull when a ULPI transceiver is selected.
                              // 1 = ULPI transceiver interrupts activate the wake-up logic
                              // 0 = ULPI transceiver interrupts are ignored by the wakeup logic.
    unsigned int OSIC:2;      // OTG Serial Interface Configuration
                              // Controls the interface type of the OTG port when used with a serial transceiver. This bit field allows for configuring the serial interface for Single Ended or Differential operation combined
                              // with Bidirectional or Unidirectional operation. Table 21-2 describes the available settings. The reset value of OSIC depends on the state of the signals .__ADD__. and .__ADD__.
                              // during reset.
                              // Bit 30 Bit 29 Interface Type
                              // 0 0 Differential / Unidirectional (6-wire)
                              // 0 1 Differential / Bidirectional (4-wire)
                              // 1 0 Single Ended / Unidirectional (6-wire)
                              // 1 1 Single Ended / Bidirectional (3-wire)
    unsigned int OWIR:1;      // OTG Wake up Interrupt Request (RO)
                              // This bit indicates that a wake-up interrupt request is received on the OTG port. This bit is cleared by disabling the wake-up interrupt.
                              // 1 = Wake-up Interrupt Request received
                              // 0 = No Wake-up detected
}USB_CTRL_T;

typedef struct _USB_OTG_MIRROR {
    unsigned int IDDIG:1;     // OTG ID-pin Status
                              // This bit indicates to the USB core whether it should operate as A-device or as B-device
                              // 1 = ID pin is High -- Operate as B-device
                              // 0 = ID pin is Low -- Operate as A-device
    unsigned int ASESVLD:1;   // A session Valid
                              // This bit must be set when a valid .A-Session. level is detected on Vbus.
                              // 1 = A Session is Valid (0.8V < Vbus < 2.0V)
                              // 0 = Session is not valid for A-device.
    unsigned int BSESVLD:1;   // B Session Valid
                              // B session valid should be set when the transceiver reports B-session Valid.
                              // 1 = B Session is Valid (0.8V < Vbus < 4.0V)
                              // 0 = B Session is not valid (Vbus < 0.8V)
    unsigned int VBUSVAL:1;   // Vbus Valid
                              // The USB driver sets this bit when the transceiver reports Vbus Valid.
                              // 1 = Vbus is Valid (Vbus > 4.4V)
                              // 0 = Vbus Invalid (Vbus < 4.4V)
    unsigned int SESEND:1;    // B device Session End
                              // This bit is set by the USB driver when the PHY reports a Session End condition.
                              // 1 = Session End (0.2V < Vbus < 0.8V)
                              // 0 = Session Active
    unsigned int RESERVED:3;  // Reserved. These bits are reserved and should be set to zero.
}USB_OTG_MIRROR_T;

typedef struct _USB_PHY_CONTROL {
    unsigned int UTMIVSTATUS:8;        // UTMI VStatus
                                       // This bit indicates the data present at the Vendor Control register (Read-only)
    unsigned int UTMIVCONTROLDATA:8;   // UTMI VControl Data
                                       // This bit indicates output data to be written into the Vendor Control Reg
    unsigned int UTMIVCONTROL:4;       // UTMI VControl
                                       // This bit indicates Vendor defined control used to address the Vendor control registers
    unsigned int UTMIVCONLOADM:1;      // UTMI VControlLoadM
                                       // This bit enable signal to load the Vendor Control Reg from the VControl pin
                                       // 1 = Enable
                                       // 0 = Disable 
    unsigned int UTMIHOSTPORT:1;       // UTMI Host Port
                                       // This indicates to the UTM of it acts as a downstream or upstream facing port.
                                       // 1 = Host side
                                       // 0 = Device side
    unsigned int UTMILSFE:1;           // UTMI Line State Filter Enable
                                       // This enable filtering of LineState to account for skew between D+/D- signals
                                       // 1 = Enable
                                       // 0 = Disable
    unsigned int UTMIEVDO:1;           // UTMI Ext Vbus Divider Option
                                       // This enables off chip resistor divider for vbus
                                       // 1 = Enable
                                       // 0 = Disable
    unsigned int UTMIUSBEN:1;          // UTMI USB Enable
                                       // This indicates to enable the USB UTMI PHY
                                       // 1 = Enable
                                       // 0 = Disable
    unsigned int UTMIRESET:1;          // UTMI Reset
                                       // This is to let the software give a reset to the PHY, if the reset from the UTMI core is not stable.
                                       // 1 = Reset
                                       // 0 = No reset
    unsigned int UTMISUSPENDM:1;       // UTMI Suspend
                                       // This is to let the software give a suspend to the PHY, if the suspend from the UTMI core is not stable.
                                       // 1 = No Suspend
                                       // 0 = Suspend
    unsigned int UTMICLKVALID:1;       // UTMI Clock Valid
                                       // This indicates to the UTM of it the clk from the PHY is within the spec (read-only)
                                       // 1 = Yes, the clock is valid
                                       // 0 = No, the clock is invalid
    unsigned int RESERVED:4;           // Reserved. These bits are reserved and should be set to zero.                                                                                                                                                             
}USB_PHY_CONTROL_T;

typedef struct _USB_PHY_CONTROL_TEST {
    unsigned int UTMIFT:1;             // UTMI Function Test
                                       // This field enables PHY to use the ipt_scan_clk instead of the internally generated PLL clocks.
                                       // 1 = Enable
                                       // 0 = Disable
    unsigned int UTMINFC:1;            // UTMI No Frequency Check
                                       // This cause PHY to disregard the UTMI spec for clock accuracy
                                       // 1 = Disregard
                                       // 0 = No disregard
    unsigned int UTMILB:1;             // UTMI Loopback
                                       // This is enable the PHY to receive its own packeds
                                       // 1 = Enable
                                       // 0 = Disable
    unsigned int UTMITM:2;             // UTMI Test Mode
                                       // This is indicates different testmode. Pls refer to the UTMI PHY SPEC
                                                                              
    unsigned int RESERVED:27;          // Reserved. These bits are reserved and should be set to zero.                                                                          
}USB_PHY_CONTROL_TEST_T;

typedef struct {
    // pfnUSBPowerDown - function pointer for platform to call during power down.
    // pfnUSBPowerUp - function pointer for platform to call during power up.
    // Parameter: 1) regs - USB registers
    //            2) pUSBCoreClk - pointer to boolean to indicate the status of USB Core Clk
    //               if it is on or off. Platform is responsible to update this value if they change
    //               the status of USBCoreClk. [TRUE - USBCoreClk ON,  FALSE - USBCoreClk OFF]
    //            3) pPanicMode - pointer to boolean to indicate the status of panic mode
    //               if it is on or off. Platform is responsible to update this value if they change
    //               the status of panic mode. [TRUE - PanicMode ON,  FALSE - PanicMode OFF]      
    void (*pfnUSBPowerDown)(CSP_USB_REGS *regs, BOOL *pUSBCoreClk);
    void (*pfnUSBPowerUp)(CSP_USB_REGS *regs, BOOL *pUSBCoreClk);
    // pfnUSBSetPhyPowerMode - function pointer for platform to call when they want to suspend/resume the PHY
    // Parameter: 1) regs - USB registers
    //            2) bResume - TRUE - request to resume, FALSE - request suspend
    void (*pfnUSBSetPhyPowerMode)(CSP_USB_REGS *regs, BOOL bResume);
} BSP_USB_CALLBACK_FNS;

#ifdef __cplusplus
}
#endif

#endif // __MX25_USB_H
