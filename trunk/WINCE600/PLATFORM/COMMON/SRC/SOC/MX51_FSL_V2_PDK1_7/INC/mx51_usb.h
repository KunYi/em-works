//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx51_usb.h
//
//  Provides definitions for usb module based on Freescale MX51 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX51_USB_H
#define __MX51_USB_H

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
    //UINT32 HWTTTXBUF;           //      0x18                *   TT-TX Buffer Hardware Parameters
    //UINT32 HWTTRXBUF;           //      0x1C                *   TT-RX Buffer Hardware Parameters
    UINT32 RESERVED1[26];       //      0x18-0x7c
    UINT32 GPTIMER0LD;          //   0x80
    UINT32 GPTIMER0CTRL;        //  0x84
    UINT32 GPTIMER1LD;          //   0x88
    UINT32 GPTIMER1CTRL;        //  0x8c
    UINT32 RESERVED2[28];       // 0x90-0xfc
    UINT8  CAPLENGTH;           // RO   0x100*  *   *   *   Capability Register Length
    UINT8  RESERVED3;           //      0x101
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
    CSP_USB_REG H1;                     //  0x200
    CSP_USB_REG H2;                     //  0x400
    CSP_USB_REG H3;                     //  0x600
    UINT32      USB_CTRL;              //  0x800
    UINT32      USB_OTG_MIRROR;       //  0x804
    UINT32      USB_PHY_CTRL_0;       // 0x808
    UINT32      USB_PHY_CTRL_1;       // 0x80c
    UINT32      USB_CTRL_1;           // 0x810
    UINT32      USB_UH2_CTRL;         // 0x814
    UINT32      USB_UH3_CTRL;         // 0x818
} CSP_USB_REGS, * PCSP_USB_REGS;

typedef struct _USB_CTRL {
    unsigned int BPE:1;     // Bypass Enable
                            // This bit enables/disables the USB Bypass function.
                            // 1 = Bypass active -- USB signals from Host port 1 are routed to the OTG port.
                            // 0 = Bypass inactive -- Normal mode operation.(default)
    unsigned int OTCKOEN:1;  //OTG ULPI PHY clock output enable
                            // 1 = CLK output disable
                            // 0 = CLK output enable
    unsigned int RESERVED:2; //Reserved. These bits are reserved and should be set to zero.
    unsigned int H1DISFSTLL:1; // host 1 serial TLL disable
                            // 1 = serial TLL is disable
                            // 0 = serial TLL is enable (default)
    unsigned int H1ICEN:1; // host 1 IC USB enable
                            // 1 = IC USB enable
                            // 0 = IC USB disable
    unsigned int H1HSTLL:1; // host 1 ULPI TLL enable
                            // 1 = ULPI TLL is enable
                            // 0 = ULPI TLL is disable (default)
    unsigned int OHSTLL:1; // OTG ULPI TLL enable
                            // 1 = ULPI TLL is enable
                            // 0 = ULPI TLL is disable (default)
    unsigned int H1PM:1;        //Host 1 Power Mask
                            // The power mask bit controls whether or not the external Vbus Power and OverCurrent detection are active for the Host 2 port.
                            // 1 = The USBPWR and OC pins are not used by the Host 1 core.
                            // 0 = The USBPWR pin will assert with the Host 1 core's Vbus power Enable and the assertion of the OC input will be reported to the Host 2 core.
    unsigned int H1BPVAL:2; //HOST 1 Bypass Value
                            //This field contains the status of the RxDP and RxDM inputs to the HOST1 core when Bypass mode is enabled. Bit 10 controls RxDB, bit 9 controls RxDm.  
    unsigned int H1WIE:1;       //Host 1 Wake-up Interrupt Enable
                            // This bit enables or disables the Host 1 wake-up interrupt. Disabling the interrupt also clears the Interrupt request bit. Wake-up interrupt enable should be turned off after receiving a
                            // wake-up interrupt and turned on again prior to going in suspend mode 
                            // 1 = Interrupt Enabled
                            // 0 = Interrupt Disabled
    unsigned int H1UIE:1;       // Host 2 ULPI interrupt enable
                            // Controls whether or not interrupts from the ULPI transceiver will trigger the wake-up logic. This bit is only meaningfull when a ULPI transceiver is selected.
                            // 1 = ULPI transceiver interrupts activate the wake-up logic
                            // 0 = ULPI transceiver interrupts are ignored by the wakeup logic.
    unsigned int H1SIC:2;       //Host 1 Serial Interface Configuration
                            // Controls the interface type of the Host 1 port when used with a serial transceiver. This bit field allows for configuring the serial interface for Single Ended or Differential operation combined
                            // with Bidirectional or Unidirectional operation.
                            // 0 0 Differential / Unidirectional (6-wire)
                            // 0 1 Differential / Bidirectional (4-wire)
                            // 1 0 Single Ended / Unidirectional (6-wire)
                            // 1 1 Single Ended / Bidirectional (3-wire)
    unsigned int H1WIR:1;       // Host 1 Wake-up Interrupt Request(RO)
                            // Indicates a pending Wake-up request on Host port 1. This bit is cleared by disabling the interrupt. The interrupt must be disabled for at least 2 clock cycles of the standby clock.
                            // 1 = Wake-up interrupt received
                            // 0 = No Wake-up interrupt received
    unsigned int ICTPC:1;

    unsigned int H1TCKOEN:1;// Host1 ULPI PHY clock output enable
                            // 1 = CLK output disable
                            // 0 = CLK output enable
    unsigned int UBPCKE:1; // By pass clock enable
                            // 1 = Bypass Host1 ulpi clock to OTG port
                            // 0 = Bypass OTG ulpi clock to Host1 port
    unsigned int ICTPIE:1; // IC USB TP interrupt enable
                            // 1 = IC USB TP interrupt enable
                            // 0 = IC USB TP interrupt disable
    unsigned int ICVSEL:3; // Host1 IC-USB voltage select
                           // Select the voltage for IC USB
                           // 001 = 1.8V class
                           // 010 = 3.0V class
    unsigned int ICVOL:1; // Host1 IC-USB voltage status
                           // Indicates the voltage status of IC-USB
                           // 1 = 3.0V class
                           // 0 = 1.8V class
    unsigned int OPM:1;     //OTG Power Mask
                            // The power mask bit controls whether or not the external Vbus Power and OverCurrent detection are active for the OTG port.
                            // 1 = The USBPWR and OC pins are not used by the OTG core.
                            // 0 = The USBPWR pin will assert with the OTG core.s Vbus power Enable and the assertion of the OC input will be reported to the OTG core.
    unsigned int OBPVAL:2;      //OTG Bypass Value
                            // This field contains the status of the RxDP and RxDM inputs to the OTG core when Bypass mode is enabled. Bit 26 controls RxDB, bit 25 controls RxDm.
    unsigned int OWIE:1;        // OTG Wake-up Interrupt Enable
                            // This bit enables or disables the OTG wake-up interrupt. Disabling the interrupt also clears the Interrupt request bit. Wake-up interrupt enable should be turned off after receiving a wake-up
                            // interrupt and turned on again prior to going in suspend mode
                            // 1 = Interrupt Enabled
                            // 0 = Interrupt Disabled
    unsigned int OUIE:1;        // OUIE -- OTG ULPI interrupt enable
                            // Controls whether or not interrupts from the ULPI transceiver will trigger the wake-up logic. This bit is only meaningfull when a ULPI transceiver is selected.
                            // 1 = ULPI transceiver interrupts activate the wake-up logic
                            // 0 = ULPI transceiver interrupts are ignored by the wakeup logic.
    unsigned int OSIC:2;        //OTG Serial Interface Configuration
                            // Controls the interface type of the OTG port when used with a serial transceiver. This bit field allows for configuring the serial interface for Single Ended or Differential operation combined
                            // with Bidirectional or Unidirectional operation. Table 21-2 describes the available settings. The reset value of OSIC depends on the state of the signals .__ADD__. and .__ADD__.
                            // during reset.
                            // Bit 30 Bit 29 Interface Type
                            // 0 0 Differential / Unidirectional (6-wire)
                            // 0 1 Differential / Bidirectional (4-wire)
                            // 1 0 Single Ended / Unidirectional (6-wire)
                            // 1 1 Single Ended / Bidirectional (3-wire)
    unsigned int OWIR:1;        //OTG Wake up Interrupt Request (RO)
                            // This bit indicates that a wake-up interrupt request is received on the OTG port. This bit is cleared by disabling the wake-up interrupt.
                            // 1 = Wake-up Interrupt Request received
                            // 0 = No Wake-up detected
}USB_CTRL_T;

typedef struct _USB_OTG_MIRROR {
    unsigned int IDDIG:1;       // OTG ID-pin Status
                            // This bit indicates to the USB core whether it should operate as A-device or as B-device
                            // 1 = ID pin is High -- Operate as B-device
                            // 0 = ID pin is Low -- Operate as A-device
    unsigned int ASESVLD:1; // A session Valid
                            // This bit must be set when a valid .A-Session. level is detected on Vbus.
                            // 1 = A Session is Valid (0.8V < Vbus < 2.0V)
                            // 0 = Session is not valid for A-device.
    unsigned int BSESVLD:1; //B Session Valid
                            // B session valid should be set when the transceiver reports B-session Valid.
                            // 1 = B Session is Valid (0.8V < Vbus < 4.0V)
                            // 0 = B Session is not valid (Vbus < 0.8V)
    unsigned int VBUSVAL:1; // Vbus Valid
                            // The USB driver sets this bit when the transceiver reports Vbus Valid.
                            // 1 = Vbus is Valid (Vbus > 4.4V)
                            // 0 = Vbus Invalid (Vbus < 4.4V)
    unsigned int SESEND:1;  // B device Session End
                            // This bit is set by the USB driver when the PHY reports a Session End condition.
                            // 1 = Session End (0.2V < Vbus < 0.8V)
                            // 0 = Session Active
    unsigned int RESERVED:2;    //Reserved. These bits are reserved and should be set to zero.
    unsigned int ULPIPHYCLK:1; //OTG ULPI PHY Clock on dectection
                            // This is a readonly status bit, indicted the external USB ULPI PHY clock is on and sent to module.
                            // 1 = The OTG ulpi input clock is on
                            // 0 = The OTG ulpi input clock is off 
    unsigned int UTMIPHYCLK:1; //OTG UTMI PHY Clock on detection
                            // This is a readonly status bit, indicted the external USB UTMI PHY clock is on and sent to module.
                            // 1 = The OTG ulpi input clock is on
                            // 0 = The OTG ulpi input clock is off 
}USB_OTG_MIRROR_T;

typedef struct _USB_PHY_CTRL_0 {
    unsigned int CHRGDET_INT_FLG:1;// chrgdet detected interrupt flag
                            //Charger detected interrupt flag. This bit will be cleared when CHRGDET_INT_EN is '0'
                            // 1 = charger detected interrupt occurred
                            // 0 = no charger detected interrupt
    unsigned int CHRGDET_INT_EN:1; // chrgdet detected interrupt enable
                            // Charger detected interrupt enable.
                            // 1 = Enable the charger detected interrupt
                            // 0 = Disable the charger detected interrupt
    unsigned int CHRGDET:1; // ChipIdea UTMI PHY chrgdet
                            // Charger detector output. Note: Maximum response time = 1us
                            // 1 = When a charger is detected
                            // 0 = When a Host is detected
    unsigned int PWR_POL:1;// OTG power Pin polarity
                            // The polarity of OTG Power pin(to enable external PMIC to drive VBUS)
                            // 1 = High active
                            // 0 = Low active
    unsigned int H1_Xcvr_clk_sel:1;//Select the clock source of the xcvr_clk
                            // Select the clock source of the xcvr_clk for HOST1 CORE.
                            // 1 = ipg_clk_60Mhz is selected
                            // 0 = Depends on which PHY is connected (Serial PHY: ipg_clk_60Mhz; ULPI PHY: 
                            // ipp_ind_otg_clk; UTMI PHY: sie_clock)
    unsigned int RESERVED1:2;//Reserved. These bits are reserved and should be set to zero.
    
    unsigned int OTG_Xcvr_clk_sel:1;// Select the clock source of the xcvr_clk
                            // Select the clock source of the xcvr_clk for OTG CORE.
                            // 1 = ipg_clk_60Mhz is selected, incase of fslsserialmode for UTMI PHY mode, this bit must be setUniversal Serial Bus OTG HOST3 (USBOH3)
                            // 0 = Depends on which PHY is connected (Serial PHY: ipg_clk_60Mhz; ULPI PHY: 
                            // ipp_ind_otg_clk; UTMI PHY: sie_clock)
    
    unsigned int OTG_Over_Cur_Dis:1;// OTG Disable Overcurrent Event
                            // Disable the OTG overcurrent event.
                            // 1 = Disable the Overcurrent event
                            // 0 = Enable the overcurrent event

    unsigned int OTG_Over_Cur_Pol:1; // OTG Polarity of Overcurrent
                            // The polarity of OTG port overcurrent event.
                            // 1 = Low active
                            // 0 = High active

    unsigned int Utmi_on_clock:1;// UTMI PHY On clock
                            // Allows the system clocks clockout to be available even if suspend is asserted.
                            // 1 = Enable
                            // 0 = Disable

    unsigned int Reset:1;// UTMI PHY Reset
                            // Active High. Reset the UTMI PHY
                            // 1 = Reset the PHY
                            // 0 = Inactive

    unsigned int SUSPENDM:1;// UTMI PHY Suspend
                            // Active Low. Set PHY into suspend mode
                            // 1 = Disable
                            // 0 = Enable
    //unsigned int RESERVED2:2;//Reserved. These bits are reserved and should be set to zero.

    unsigned int OTG_USER_WAKEUP_EN:1;// User wakeup event enable
                            // Active High. Enable the software wakeup event.
                            // 1 = Enable
                            // 0 = Disable

    unsigned int OTG_USER_WAKEUP:1;// User wakeup event
                            // Active High. Write '1'  to trigger wake up event.
                            // 1 = trigger wake up event
                            // 0 = Inactive

    unsigned int VSTATUS:8;// ChipIdeat UTMI PHY Vstatus
                            // Vendor Status - ChipIdea defined 8-bit parallel output.

    unsigned int CHGRDETON:1;// ChipIdea UTMI PHY chgrdeton
                            // Active High. Charger Detector Power On Control. This pin controls the internal current mirrors used 
                            // for charger detection. Must be asserted to '1'. at least 20us before chgrdeten going to '1'

    unsigned int CHGRDETEN:1;// ChipIdea UTMI PHY chgrdeten
                            // Active High. Enable Charger Detector. This pin must be asserted 300us (or more) after chgrdeton 
                            // going to '1'

    unsigned int CONF3:1;// ChipIdea UTMI PHY CONF3
                            // During non-driving mode (opmode[1:0] = 2¡¯b01) if conf3 = 1 the 15Kohm pull-down resistors will be 
                            // connected (if dppulldown = dnpulldown = '1'). Sshould be tied to  '0' for device only applications.

    unsigned int CONF2:1;// ChipIdea UTMI CONF2
                            // Active High. When asserted will turn on OTG comparators during suspend(suspend = ¡®0¡¯).
                            // 1 = will turn on OTG comparators during suspend
                            // 0 = Inactive

    unsigned int VCONTROL:4;// Chipidea UTMI PHY Vcontrol

    unsigned int VLOAD:1;// Chipidea UTMI PHY Vload
                            // Assertion of this signal loads the Vendor Control register. Active low.
                            // 1 = Inactive
                            // 0 = loads the Vendor Control register
}USB_PHY_CTRL_0_T;

typedef struct _USB_PHY_CTRL_1 {
    unsigned int plldivvalue:2;//  Selects between 12Mhz, 19.2Mhz or 24Mhz reference clock
                            // 00 = sysclock uses 12Mhz
                            // 01 = sysclock uses 19.2Mhz
                            // 10 = sysclock uses 24Mhz
                            // 11 = reserved

    unsigned int extcal:5;// Controls calibration value externally. Valid when calbp = '1'
    unsigned int calbp:1;// Enables calibration bypass for both dp and dn lines
    unsigned int preemdepth:1;// HS driver pre-emphasis depthenpre, preemdepth
                            // 00 = I (I = 17.78mA)
                            // 01 = I + 5%
                            // 10 = I + 10%
                            // 11 = I + 20%
    unsigned int enpre:1;// HS driver pre-emphasis enable
    unsigned int lsrftsel:2;// LS driver rise/fall time control
                            // 00 = Nominal LS rise time-30%
                            // 01 = Nominal LS rise time
                            // 10 = Nominal LS rise time
                            // 11 = Nominal LS rise time+30%
    unsigned int fsrftsel:2;// FS driver rise/fall time control
                            // 00 = Nominal FS rise time-30%
                            // 01 = Nominal FS rise time
                            // 10 = Nominal FS rise time
                            // 11 = Nominal FS rise time+30%
    unsigned int icpctrl:2;// PLL charge pump current control
                            // 00 = Icp (Icp = 40uA)
                            // 01 = Icp * 0.5
                            // 10 = Icp * 1.5
                            // 11 = Icp * 2
    unsigned int fstunevsel:3;// Reference voltage control for Calibration circuit
                            // 000 = (46)/100*vbg (533.6mV)
                            // 001 = (46+1)/100*vbg (545.2mV)
                            // 010 = (46+2)/100*vbg (556.8mV)
                            // 011 = (46+3)/100*vbg (568.4mV)
                            // 100 = (46+4)/100*vbg (580mV)
                            // 101 = (46+5)/100*vbg (591.6mV)
                            // 110 = (46+6)/100*vbg (603.2mV)
                            // 111 = (46+7)/100*vbg (614.8mV)

    unsigned int hstedvsel:2;// Reference voltage for high speed transmission envelope detector
                            // 00 = vbg*(9)/100 (104.4mV)
                            // 01 = vbg*(9+1)/100 (116mV)
                            // 10 = vbg*(9+2)/100 (127.6mV)
                            // 11 = vbg*(9+3)/100 (139.2mV)

    unsigned int hsdedvsel:2;// Reference voltage for high speed disconnect envelope detector
                            // 00 = (46+2)/100*vbg (556.8mV)
                            // 01 = (46+3)/100*vbg (568.4mV)
                            // 10 = (46+4)/100*vbg (580mV)
                            // 11 = (46+5)/100*vbg (591.6mV)

    unsigned int hsdrvslope:4;// HS driver slope control
                            // The HS Driver may have its rise/fall times controlled using the hsdrvslope[3:0] control pins. 
                            // Additional charge is injected, so the HS driver Rise/Fall times change (RC constant changes). Rise/Fall 
                            // times: Depends on the Package, PCB... Correct value result from Silicon tests

    unsigned int hsdrvamplitude:2;// HS driver amplitude control
                            // 00 = I (I = 17.78mA)
                            // 01 = I + 2.5%
                            // 10 = I + 5%
                            // 11 = I + 7.5%

    unsigned int hsdrvtimingn:2;// HS driver timing control for NMOS
                            // 00 = 2x
                            // 01 = 4x
                            // 10 = 6x
                            // 11 = 8x

    unsigned int hsdrvtimingp:1;// HS driver timing control for PMOS
                            // 1 = 8x
                            // 0 = 2x  
}USB_PHY_CTRL_1_T;

typedef struct _USB_CTRL_1 {
    unsigned int RESERVED1:24;     // reserved
    unsigned int otg_ext_clk_en:1;  //otg select the clock which comes from external PHY or internal PLL
                            // This bit must be set to 1 before setting OTG core into ULPI mode. It should be clear to 0 when setting 
                            // OTG Core into FSserial mode
                            // 1 = Select the clock which comes from external PHY
                            // 0 = Select the clock which comes from internal PLL

    unsigned int uh1_ext_clk_en:1;  //Host 1 select the clock which comes from external PHY or internal PLL
                            // This bit must be set to 1 before setting Host1 core into ULPI mode. It should be clear to 0 when setting 
                            // Host1 Core into FS serial mode
                            // 1 = Select the clock which comes from external PHY
                            // 0 = Select the clock which comes from internal PLL
    unsigned int uh2_ext_clk_en:1;  //Host 2 select the clock which comes from external PHY or internal PLL
                            // This bit must be set to 1 before setting Host2 core into ULPI mode. It should be clear to 0 when setting 
                            // Host2 Core into FS serial mode
                            // 1 = Select the clock which comes from external PHY
                            // 0 = Select the clock which comes from internal PLL
    unsigned int uh3_ext_clk_en:1;  //Host 3 select the clock which comes from external PHY or internal PLL
                            // This bit must be set to 1 before setting Host3 core into ULPI mode. It should be clear to 0 when setting 
                            // Host3 Core into FS serial mode
                            // 1 = Select the clock which comes from external PHY
                            // 0 = Select the clock which comes from internal PLL
    unsigned int RESERVED2:4;     // reserved
}USB_CTRL_1_T;

typedef struct _USB_CTRL_2 {
    unsigned int RESERVED1:1; //Reserved. These bits are reserved and should be set to zero.
    unsigned int H2DISFSTLL:1; // host 2 serial TLL disable
                            // 1 = serial TLL is disable
                            // 0 = serial TLL is enable (default)
    unsigned int H2ICEN:1; // host 2 IC USB enable
                            // 1 = IC USB enable
                            // 0 = IC USB disable
    unsigned int H2HSTLL:1; // host 2 ULPI TLL enable
                            // 1 = ULPI TLL is enable
                            // 0 = ULPI TLL is disable (default)
    unsigned int H2PM:1;        //Host 2 Power Mask
                            // The power mask bit controls whether or not the external Vbus Power and OverCurrent detection are active for the Host 2 port.
                            // 1 = The USBPWR and OC pins are not used by the Host 1 core.
                            // 0 = The USBPWR pin will assert with the Host 1 core's Vbus power Enable and the assertion of the OC input will be reported to the Host 2 core.
    unsigned int H2BPVAL:2; //HOST 2 Bypass Value
                            //This field contains the status of the RxDP and RxDM inputs to the HOST1 core when Bypass mode is enabled. Bit 10 controls RxDB, bit 9 controls RxDm.  
    unsigned int H2WIE:1;       //Host 2 Wake-up Interrupt Enable
                            // This bit enables or disables the Host 1 wake-up interrupt. Disabling the interrupt also clears the Interrupt request bit. Wake-up interrupt enable should be turned off after receiving a
                            // wake-up interrupt and turned on again prior to going in suspend mode 
                            // 1 = Interrupt Enabled
                            // 0 = Interrupt Disabled
    unsigned int H2UIE:1;       // Host 2 ULPI interrupt enable
                            // Controls whether or not interrupts from the ULPI transceiver will trigger the wake-up logic. This bit is only meaningfull when a ULPI transceiver is selected.
                            // 1 = ULPI transceiver interrupts activate the wake-up logic
                            // 0 = ULPI transceiver interrupts are ignored by the wakeup logic.
    unsigned int H2SIC:2;       //Host 2 Serial Interface Configuration
                            // Controls the interface type of the Host 1 port when used with a serial transceiver. This bit field allows for configuring the serial interface for Single Ended or Differential operation combined
                            // with Bidirectional or Unidirectional operation.
                            // 0 0 Differential / Unidirectional (6-wire)
                            // 0 1 Differential / Bidirectional (4-wire)
                            // 1 0 Single Ended / Unidirectional (6-wire)
                            // 1 1 Single Ended / Bidirectional (3-wire)
    
    unsigned int ICTPC:1;

    unsigned int H2TCKOEN:1;// Host2 ULPI PHY clock output enable
                            // 1 = CLK output disable
                            // 0 = CLK output enable
    unsigned int ICTPIE:1; // IC USB TP interrupt enable
                            // 1 = IC USB TP interrupt enable
                            // 0 = IC USB TP interrupt disable
    unsigned int ICVSEL:3; // Host2 IC-USB voltage select
                           // Select the voltage for IC USB
                           // 001 = 1.8V class
                           // 010 = 3.0V class
    unsigned int H2WIR:1;       // Host 2 Wake-up Interrupt Request(RO)
                            // Indicates a pending Wake-up request on Host port 1. This bit is cleared by disabling the interrupt. The interrupt must be disabled for at least 2 clock cycles of the standby clock.
                            // 1 = Wake-up interrupt received
                            // 0 = No Wake-up interrupt received
    unsigned int ICVOL:1; // Host2 IC-USB voltage status
                           // Indicates the voltage status of IC-USB
                           // 1 = 3.0V class
                           // 0 = 1.8V class
    unsigned int RESERVED2:13;     //Reserved. These bits are reserved and should be set to zero.
}USB_CTRL_2_T;

typedef struct _USB_CTRL_3 {
    unsigned int RESERVED1:1; //Reserved. These bits are reserved and should be set to zero.
    unsigned int H3DISFSTLL:1; // host 3 serial TLL disable
                            // 1 = serial TLL is disable
                            // 0 = serial TLL is enable (default)
    unsigned int H3ICEN:1; // host 3 IC USB enable
                            // 1 = IC USB enable
                            // 0 = IC USB disable
    unsigned int H3HSTLL:1; // host 3 ULPI TLL enable
                            // 1 = ULPI TLL is enable
                            // 0 = ULPI TLL is disable (default)
    unsigned int H3PM:1;        //Host 3 Power Mask
                            // The power mask bit controls whether or not the external Vbus Power and OverCurrent detection are active for the Host 2 port.
                            // 1 = The USBPWR and OC pins are not used by the Host 1 core.
                            // 0 = The USBPWR pin will assert with the Host 1 core's Vbus power Enable and the assertion of the OC input will be reported to the Host 2 core.
    unsigned int H3BPVAL:2; //HOST 3 Bypass Value
                            //This field contains the status of the RxDP and RxDM inputs to the HOST1 core when Bypass mode is enabled. Bit 10 controls RxDB, bit 9 controls RxDm.  
    unsigned int H3WIE:1;       //Host 3 Wake-up Interrupt Enable
                            // This bit enables or disables the Host 1 wake-up interrupt. Disabling the interrupt also clears the Interrupt request bit. Wake-up interrupt enable should be turned off after receiving a
                            // wake-up interrupt and turned on again prior to going in suspend mode 
                            // 1 = Interrupt Enabled
                            // 0 = Interrupt Disabled
    unsigned int H3UIE:1;       // Host 3 ULPI interrupt enable
                            // Controls whether or not interrupts from the ULPI transceiver will trigger the wake-up logic. This bit is only meaningfull when a ULPI transceiver is selected.
                            // 1 = ULPI transceiver interrupts activate the wake-up logic
                            // 0 = ULPI transceiver interrupts are ignored by the wakeup logic.
    unsigned int H3SIC:2;       //Host 3 Serial Interface Configuration
                            // Controls the interface type of the Host 1 port when used with a serial transceiver. This bit field allows for configuring the serial interface for Single Ended or Differential operation combined
                            // with Bidirectional or Unidirectional operation.
                            // 0 0 Differential / Unidirectional (6-wire)
                            // 0 1 Differential / Bidirectional (4-wire)
                            // 1 0 Single Ended / Unidirectional (6-wire)
                            // 1 1 Single Ended / Bidirectional (3-wire)
    
    unsigned int ICTPC:1;

    unsigned int H3TCKOEN:1;// Host 3 ULPI PHY clock output enable
                            // 1 = CLK output disable
                            // 0 = CLK output enable
    unsigned int ICTPIE:1; // IC USB TP interrupt enable
                            // 1 = IC USB TP interrupt enable
                            // 0 = IC USB TP interrupt disable
    unsigned int ICVSEL:3; // Host3 IC-USB voltage select
                           // Select the voltage for IC USB
                           // 001 = 1.8V class
                           // 010 = 3.0V class
    unsigned int H3WIR:1;       // Host 3 Wake-up Interrupt Request(RO)
                            // Indicates a pending Wake-up request on Host port 1. This bit is cleared by disabling the interrupt. The interrupt must be disabled for at least 2 clock cycles of the standby clock.
                            // 1 = Wake-up interrupt received
                            // 0 = No Wake-up interrupt received
    unsigned int ICVOL:1; // Host3 IC-USB voltage status
                           // Indicates the voltage status of IC-USB
                           // 1 = 3.0V class
                           // 0 = 1.8V class
    unsigned int RESERVED2:13;     //Reserved. These bits are reserved and should be set to zero.
}USB_CTRL_3_T;

typedef struct {
    // pfnUSBPowerDown - function pointer for platform to call during power down.
    // pfnUSBPowerUp - function pointer for platform to call during power up.
    // Parameter: 1) regs - USB registers
    //            2) pUSBCoreClk - pointer to boolean to indicate the status of USB Core Clk
    //               if it is on or off. Platform is responsible to update this value if they change
    //               the status of USBCoreClk. [TRUE - USBCoreClk ON,  FALSE - USBCoreClk OFF]
    //            3) pPanicMode - pointer to boolean to indicate the status of panic mode
    //               if it is on or off. Platform is responsible to update this value if they change
    //               the status of panic mode. [TRUE - PanicMode ON,  FALSE - USBCoreClk OFF]      
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

#endif // __MX51_USB_H
