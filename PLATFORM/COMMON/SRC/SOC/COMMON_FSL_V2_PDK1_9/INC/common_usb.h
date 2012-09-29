//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_usb.h
//
//  Provides definitions for common register for usb module.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_USB_H
#define __COMMON_USB_H


#if __cplusplus
extern "C" {
#endif

typedef struct _USB_ID {
    unsigned int ID:8;          // Configuration number. This number is set to 0x05
                                // and indicates that the peripheral is the ARC USB-HS OTG
                                //High-Speed USB On-The-Go USB 2.0 core.
    unsigned int NID:8;         // Ones complement version of ID[5:0].
    unsigned int REVISION:8;    // Revision number of the core
    unsigned int RESERVED:8;    // These bits are reserved and should be set to zero
} USB_ID_T;

typedef struct _USB_HWGENERAL {
    unsigned int RT:1;          // VUSB_HS_RESET_TYPE
    unsigned int CLKC:2;        // VUSB_HS_CLOCK_CONFIGURATION
    unsigned int BWT:1;         // Reserved for internal testing.
    unsigned int PHYW:2;        // VUSB_HS_PHY16_8
    unsigned int PHYM:3;        // VUSB_HS_PHY_TYPE
    unsigned int SM:1;          // VUSB_HS_PHY_SERIAL
    unsigned int RESERVED:22;   // Reserved. These bits are reserved and should be set to zero.
} USB_HWGENERAL_T;

typedef struct _USB_HWHOST {
    unsigned int HC:1;          // VUSB_HS_HOST
    unsigned int NPORT:3;       // VUSB_HS_NUM_PORT-1
    unsigned int RESERVED:12;   // Reserved. These bits are reserved and should be set to zero.
    unsigned int TTASY:8;       // VUSB_HS_TT_ASYNC_CONTEXTS
    unsigned int TTPER:8;       // VUSB_HS_TT_PERIODIC_CONTEXTS
} USB_HWHOST_T;

typedef struct _USB_HWDEVICE {
    unsigned int DC:1;          // device capable; [VUSB_HS_DEV /= 0]
    unsigned int DEVEP:5;       // VUSB_HS_DEV_EP
    unsigned int RESERVED:26;   // Reserved. These bits are reserved and should be set to zero.
} USB_HWDEVICE_T;

typedef struct _USB_HWTXBUF {
    unsigned int TCBURST:8;     // VUSB_HS_TX_BURST
    unsigned int TXADD:8;       // VUSB_HS_TX_ADD
    unsigned int TXCHANADD:8;   // VUSB_HS_TX_CHAN_ADD
    unsigned int TXLC:1;        // VUSB_HS_TX_LOCAL_CONTEXT_REGISTERS
}USB_HWTXBUF_T;


typedef struct _USB_HWRXBUF {
    unsigned int RXBURST:8;     // VUSB_HS_RX_BURST
    unsigned int RXADD:8;       // VUSB_HS_RX_ADD
    unsigned int RESERVED:16;   // Reserved. These bits are reserved and should be set to zero.
}USB_HWRXBUF_T;

typedef struct _USB_HCSPARAMS { //Port steering logic capabilities are described in this register.
    unsigned int N_PORTS:4;     // Number of downstream ports. This field specifies the number of physical downstream ports implemented on this host
                                // controller. The value of this field determines how many port registers are addressable in the Operational Register. Valid
                                // values are in the range of 1h to Fh. A zero in this field is undefined.
                                // The number of ports for a host implementation is parameterizable from 1 to 8. This field will always be 1 for
                                // device only implementation.
    unsigned int PPC:1;         // Port Power Control. This field indicates whether the host controller implementation includes port power control. A one
                                // indicates the ports have port power switches. A zero indicates the ports do not have port power switches. The value of this field
                                // affects the functionality of the Port Power field in each port status and control register.
    unsigned int RESERVED1:3;   // Reserved. These bits are reserved and should be set to zero.
    unsigned int N_PPC:4;       // Number of Ports per Companion Controller. This field indicates the number of ports supported per internal Companion
                                // Controller. It is used to indicate the port routing configuration to the system software.
                                // For example, if N_PORTS has a value of 6 and N_CC has a value of 2 then N_PCC could have a value of 3. The convention
                                // is that the first N_PCC ports are assumed to be routed to companion controller 1, the next N_PCC ports to companion
                                // controller 2, etc. In the previous example, the N_PCC could have been 4, where the first 4 are routed to companion
                                // controller 1 and the last two are routed to companion controller 2.
                                // The number in this field must be consistent with N_PORTS and N_CC.
                                // In this implementation this field will always be "0".
    unsigned int N_CC:4;        // Number of Companion Controller (N_CC). This field indicates the number of companion controllers associated with this
                                // USB2.0 host controller.
                                // A zero in this field indicates there are no internal Companion Controllers. Port-ownership hand-off is not supported.
                                // A value larger than zero in this field indicates there are companion USB1.1 host controller(s). Port-ownership hand-offs
                                // are supported. High, Full- and Low-speed devices are supported on the host controller root ports.
                                // In this implementation this field will always be "0".
    unsigned int PI:1;          // Port Indicators (P INDICATOR). This bit indicates whether the ports support port indicator control. When set to one, the port
                                // status and control registers include a read/writeable field for controlling the state of the port indicator.
    unsigned int RESERVED2:3;   // Reserved. These bits are reserved and should be set to zero.
    unsigned int N_PTT:4;       // Number of Ports per Transaction Translator (N_PTT). This field indicates the number of ports assigned to each transaction
                                // translator within the USB2.0 host controller.
                                // For Multi-Port Host this field will always equal N_PORTS. For all other implementations, N_PTT = .0000..
                                // This in a non-EHCI field to support embedded TT.
    unsigned int N_TT:4;        // Number of Transaction Translators (N_TT). This field indicates the number of embedded transaction translators
                                // associated with the USB2.0 host controller.
                                // For Multi-Port Host this field will always equal .0001.. For all other implementions, N_TT = .0000..
    unsigned int RESERVED3:4;   // Reserved. These bits are reserved and should be set to zero.
}USB_HCSPARAMS_T ;  //EHCI Compliant with extensions

typedef struct _USB_HCCPARAMS { //This register identifies multiple mode control (time-base bit functionality) addressing capability.
    unsigned int ADC:1;     // 64-bit Addressing Capability. This field will always be "0". No 64-bit addressing capability is supported.
    unsigned int PFL:1;     // Programmable Frame List Flag. If this bit is set to zero, then the system software must use a frame list length of 1024 elements with this host
                            // controller. The USBCMD register Frame List Size field is a read-only register and must be set to zero.
                            // If set to a one, then the system software can specify and use a smaller frame list and configure the host controller via the USBCMD register Frame List Size
                            // field. The frame list must always be aligned on a 4K-page boundary. This requirement ensures that the frame list is always physically contiguous.
                            // This field will always be "1".
    unsigned int ASP:1;     //  Asynchronous Schedule Park Capability. Default = 1. If this bit is set to a one, then the host controller supports the park feature for high-speed queue
                            // heads in the Asynchronous Schedule. The feature can be disabled or enabled and set to a specific level by using the Asynchronous Schedule Park Mode
                            // Enable and Asynchronous Schedule Park Mode Count fields in the USBCMD register.
                            // This field will always be "1"
    unsigned int R:1;           // Reserved. These bits are reserved and should be set to zero.
    unsigned int IST:4;     // Isochronous Scheduling Threshold. Default = implementation dependent. This field indicates, relative to the current position of the executing host
                            // controller, where software can reliably update the isochronous schedule. When bit [7] is zero, the value of the least significant 3 bits indicates the
                            // number of micro-frames a host controller can hold a set of isochronous data structures (one or more) before flushing the state. When bit [7] is a one, then
                            // host software assumes the host controller may cache an isochronous data structure for an entire frame.
                            // This field will always be "0".
    unsigned int EECP:8;        // EHCI Extended Capabilities Pointer. Default = 0. This optional field indicates the existence of a capabilities list. A value of 00h indicates no
                            // extended capabilities are implemented. A non-zero value in this register indicates the offset in PCI configuration space of the first EHCI extended
                            // capability. The pointer value must be 40h or greater if implemented to maintain the consistency of the PCI header defined for this class of device.
                            // For this implementation this field is always "0".
    unsigned int RESERVED:16; // Reserved. These bits are reserved and should be set to zero.
}USB_HCCPARAMS_T ; //EHCI Compliant

typedef struct _USB_DCCPARAMS { //This register identifies multiple mode control (time-base bit functionality) addressing capability.
    unsigned int DEN:5;     // Device Endpoint Number. This field indicates the number of endpoints built into the device controller. If this controller is not device capable, then this field
                            // will be zero. Valid values are 0-16.
    unsigned int R:2;           //Reserved. These bits are reserved and should be set to zero.
    unsigned int DC:1;          // Device Capable. When this bit is 1, this controller is capable of operating as a USB 2.0 device.
    unsigned int HC:1;          //Host Capable. When this bit is 1, this controller is capable of operating as an EHCI compatible USB 2.0 host controller.
    unsigned int RESERVED:23;    // Reserved. These bits are reserved and should be set to zero. 
} USB_DCCPARAMS_T; // Non-EHCI

typedef struct _USB_USBCMD {
    unsigned int RS:1;          //Run/Stop (RS) . Read/Write. Default 0b. 1=Run. 0=Stop.
                            // Host Controller:
                            // When set to a 1, the Host Controller proceeds with the execution of the schedule. The Host Controller continues execution as long as this
                            // bit is set to a one. When this bit is set to 0, the Host Controller completes the current transaction on the USB and then halts. The HC
                            // Halted bit in the status register indicates when the Host Controller has finished the transaction and has entered the stopped state. Software
                            // should not write a one to this field unless the host controller is in the Halted state (i.e. HCHalted in the USBSTS register is a one).
                            // Device Controller:
                            // Writing a one to this bit will cause the device controller to enable a pull-up on D+ and initiate an attach event. This control bit is not
                            // directly connected to the pull-up enable, as the pull-up will become disabled upon transitioning into high-speed mode. Software should
                            // use this bit to prevent an attach event before the device controller has been properly initialized. Writing a 0 to this will cause a detach event.
    unsigned int RST:1;     //Controller Reset (RESET) Read/Write.
                            // Software uses this bit to reset the controller. This bit is set to zero by the Host/Device Controller when the reset
                            // process is complete. Software cannot terminate the reset process early by writing a zero to this register.
                            // Host Controller:
                            // When software writes a one to this bit, the Host Controller resets its internal pipelines, timers, counters, state machines etc. to their initial value. Any
                            // transaction currently in progress on USB is immediately terminated. A USB reset is not driven on downstream ports. Software should not set this bit to a
                            // one when the HCHalted bit in the USBSTS register is a zero. Attempting to reset an actively running host controller will result in undefined behavior.
                            // Device Controller:
                            // When software writes a one to this bit, the Device Controller resets its internal pipelines, timers, counters, state machines etc. to their initial value. Writing a
                            // one to this bit when the device is in the attached state is not recommended, since the effect on an attached host is undefined. In order to ensure that the
                            // device is not in an attached state before initiating a device controller reset, all primed endpoints should be flushed and the USBCMD Run/Stop bit should be
                            // set to 0.
    unsigned int FS:2;          //Frame List Size (Read/Write or Read Only). Default 000b. 
                            // This field is Read/Write only if Programmable Frame List Flag in the HCCPARAMS registers is set to one. This field specifies the size of the
                            // frame list that controls which bits in the Frame Index Register should be used for the Frame List Current index. Note that this field is made up from USBCMD
                            //bits 15, 3 and 2.
                            // Valuesmeaning
                            // 0001024 elements (4096 bytes) Default value
                            // 001512 elements (2048 bytes)
                            // 010256 elements (1024 bytes)
                            // 011128 elements (512 bytes)
                            // 10064 elements (256 bytes)
                            // 10132 elements (128 bytes)
                            // 11016 elements (64 bytes)
                            // 1118 elements (32 bytes)
                            //Only the host controller uses this field.
    unsigned int PSE:1;     //Periodic Schedule Enable Read/Write. Default Ob. 
                            // This bit controls whether the host controller skips processing the Periodic Schedule.
                            // Valuesmeaning
                            // 0 Do not process the Periodic Schedule
                            // 1 Use the PERIODICLISTBASE register to access the PeriodicSchedule.
                            // Only the host controller uses this bit.
    unsigned int ASE:1;     //Asynchronous Schedule Enable Read/Write. Default 0b. 
                            // This bit controls whether the host controller skips processing the Asynchronous Schedule.
                            // Valuesmeaning
                            // 0 Do not process the Asynchronous Schedule.
                            // 1 Use the ASYNCLISTADDR register to access theAsynchronous Schedule.
                            // Only the host controller uses this bit.
    unsigned int IAA:1;     //Interrupt on Async Advance Doorbell Read/Write. 
                            // This bit is used as a doorbell by software to tell the host controller to issue an interrupt the next time
                            // it advances asynchronous schedule. Software must write a 1 to this bit to ring the doorbell.
                            // When the host controller has evicted all appropriate cached schedule states, it sets the Interrupt on Async Advance status bit in the USBSTS register. If the
                            // Interrupt on Sync Advance Enable bit in the USBINTR register is one, then the host controller will assert an interrupt at the next interrupt threshold.
                            // The host controller sets this bit to zero after it has set the Interrupt on Sync Advance status bit in the USBSTS register to one. Software should not write
                            // a one to this bit when the asynchronous schedule is inactive. Doing so will yield undefined results.
                            // This bit is only used in host mode. Writing a one to this bit when device mode is selected will have undefined results.
    unsigned int LR:1;          //Light Host/Device Controller Reset (OPTIONAL) 〞 Read Only. 
                            // Not Implemented. This field will always be "0".

    unsigned int ASP:2;     //Asynchronous Schedule Park Mode Count (OPTIONAL) . Read/Write. 
                            //If the Asynchronous Park Capability bit in the HCCPARAMS register is a one, then this field defaults to 3h and is R/W. Otherwise it defaults to zero and is RO. It contains
                            // a count of the number of successive transactions the host controller is allowed to execute from a high-speed queue head on the Asynchronous schedule before
                            // continuing traversal of the Asynchronous schedule. See Section 4.10.3.2 for full operational details. Valid values are 1h to 3h. Software must not write a zero to this bit
                            // when Park Mode Enable is a one as this will result in undefined behavior. 
                            //This field is set to 3h in this implementation.
    unsigned int R1:1;          //Reserved. These bits are reserved and should be set to zero.
    unsigned int ASPE:1;        //Asynchronous Schedule Park Mode Enable (OPTIONAL) . Read/Write.
                            // If the Asynchronous Park Capability bit in the HCCPARAMS register is a one, then this bit defaults to a 1h and is R/W. Otherwise the bit must be a zero and is RO.
                            // Software uses this bit to enable or disable Park mode. When this bit is one, Park mode is enabled. When this bit is a zero, Park mode is disabled.
                            // This field is set to "1" in this implementation.
    unsigned int ATDTW:1;       //Add dTD TripWire 每 Read/Write. [device mode only]
                            // This bit is used as a semaphore to ensure the to proper addition of a new dTD to an active (primed)
                            // endpoint's linked list. This bit is set and cleared by software. This bit shall also be cleared by hardware when is state machine is hazard region for which
                            // adding a dTD to a primed endpoint may go unrecognized. For more information on the use of this bit, see the Device Operational Model section of
                            // the ARC USB-HS OTG High-Speed USB On-The-Go DEV reference manual.
    unsigned int SUTW:1;        // Setup TripWire 每 Read/Write. [device mode only] 
                            // This bit is used as a semaphore to ensure that the setup data payload of 8 bytes is extracted from
                            // a QH by the DCD without being corrupted. If the setup lockout mode is off (See USBMODE) then there exists a hazard when new setup data arrives while the
                            // DCD is copying the setup data payload from the QH for a previous setup packet. This bit is set and cleared by software and will be cleared by hardware
                            // when a hazard exists. For more information on the use of this bit, see the Device Operational Model section of the ARC USB-HS OTG High-Speed USB
                            // On-The-Go DEV reference manual.
    unsigned int R2:1;          //Reserved. These bits are reserved and should be set to zero.
    unsigned int FS2:1;     //

    unsigned int ITC:8;     //Interrupt Threshold Control Read/Write. Default 08h. 
                            //The system software uses this field to set the maximum rate at which the host/device
                            // controller will issue interrupts. ITC contains the maximum interrupt interval measured in micro-frames. Valid values are shown below.
                            // ValueMaximum Interrupt Interval
                            // 00h Immediate (no threshold)
                            // 01h 1 micro-frame
                            // 02h 2 micro-frames
                            // 04h 4 micro-frames
                            // 08h 8 micro-frames
                            // 10h 16 micro-frames
                            // 20h 32 micro-frames
                            // 40h 64 micro-frames
    unsigned int RESERVED:8;    //Reserved. These bits are reserved and should be set to zero.
    
}USB_USBCMD_T;

typedef struct _USB_USBSTS{
    unsigned int UI:1;          //USB Interrupt (USBINT) R/WC. 
                            // This bit is set by the Host/Device Controller when the cause of an interrupt is a completion of a USB transaction where the
                            // Transfer Descriptor (TD) has an interrupt on complete (IOC) bit set. This bit is also set by the Host/Device Controller when a short packet is
                            // detected. A short packet is when the actual number of bytes received was less than the expected number of bytes.
    unsigned int UEI:1;     //USB Error Interrupt (USBERRINT) R/WC. 
                            // When completion of a USB transaction results in an error condition, this bit is set by the Host/Device
                            // Controller. This bit is set along with the USBINT bit, if the TD on which the error interrupt occurred also had its interrupt on complete (IOC) bit set
                            // See Section (Reference Host Operation Model: Transfer/Transaction Based Interrupt "C i.e. 4.15.1 in EHCI Enhanced Host Controller Interface Specification
                            // for Universal Serial Bus, Revision 0.95, November 2000, Intel Corporation. http://www.intel.com) for a complete list of host error interrupt conditions.
                            // See section Device Error Matrix in the ARC USB-HS OTG High-Speed USB On-The-Go DEV reference manual.
                            // The device controller detects resume signaling only.
    unsigned int PCI:1;     //Port Change Detect R/WC. 
                            // The Host Controller sets this bit to a one when on any port a Connect Status occurs, a Port Enable/Disable Change occurs, or
                            // the Force Port Resume bit is set as the result of a J-K transition on the suspended port.
                            // The Device Controller sets this bit to a one when the port controller enters the full or high-speed operational state. When the port controller exits the full or
                            // high-speed operation states due to Reset or Suspend events, the notification mechanisms are the USB Reset Received bit and the DCSuspend bits respectively.
                            // This bit is not EHCI compatible.
    unsigned int FRI:1;     // Frame List Rollover R/WC. 
                            // The Host Controller sets this bit to a one when the Frame List Index rolls over from its maximum value to zero. The exact value
                            // at which the rollover occurs depends on the frame list size. For example. If the frame list size (as programmed in the Frame List Size field of the USBCMD
                            // register) is 1024, the Frame Index Register rolls over every time FRINDEX [1 3] toggles. Similarly, if the size is 512, the Host Controller sets this bit to a one
                            // every time FHINDEX [12] toggles.
                            // Only used by the host controller.
    unsigned int SEI:1;     //System Error R/WC. 
                            // This bit is not used in this implementation and will always be set to "0".
    unsigned int AAI:1;     //Interrupt on Async Advance R/WC. 0=Default. 
                            // System software can force the host controller to issue an interrupt the next time the host controller
                            // advances the asynchronous schedule by writing a one to the Interrupt on Async Advance Doorbell bit in the USBCMD register. This status bit indicates the
                            // assertion of that interrupt source.
                            //Only used by the host controller.
    unsigned int URI:1;     // USB Reset Received "C R/WC. 0=Default. 
                            // When the device controller detects a USB Reset and enters the default state, this bit will be set to a one. Software
                            // can write a 1 to this bit to clear the USB Reset Received status bit.
                            // Only used by the device controller.
    unsigned int SRI:1;     //SOF Received "C R/WC. 0=Default. 
                            // When the device controller detects a Start Of (micro) Frame, this bit will be set to a one. When a SOF is extremely late, the
                            // device controller will automatically set this bit to indicate that an SOF was expected. Therefore, this bit will be set roughly every 1ms in device FS mode
                            // and every 125ms in HS mode and will be synchronized to the actual SOF that is received.
                            // Since the device controller is initialized to FS before connect, this bit will be set at an interval of 1ms during the prelude to connect and chirp.
                            // In host mode, this bit will be set every 125us and can be used by host controller driver as a time base.
                            // Software writes a 1 to this bit to clear it.
                            // This is a non-EHCI status bit.
                            
    unsigned int SLI:1;         //DCSuspend "C R/WC. 0=Default. 
                            // When a device controller enters a suspend state from an active state, this bit will be set to a one. The device controller
                            // clears the bit upon exiting from a suspend state.
                            // Only used by the device controller.
    unsigned int R1:1;          //Reserved. These bits are reserved and should be set to zero.
    unsigned int ULPII:1;       //ULPI Interrupt "C R/WC. 0=Default.
                            // When the ULPI Viewport is present in the design, an event completion will set this interrupt.
                            // Used by both host & device controller. Only present in designs where configuration constant VUSB_HS_PHY_ULPI = 1.
    unsigned int R2:1;          //Reserved. These bits are reserved and should be set to zero.
    unsigned int HCH:1;     // HCHaIted Read Only. 1=Default. 
                            // This bit is a zero whenever the Run/Stop bit is a one. The Host Controller sets this bit to one after it has stopped
                            // executing because of the Run/Stop bit being set to 0, either by software or by the Host Controller hardware (e.g. internal error).
                            // Only used by the host controller.
    unsigned int RCL:1;     // Reclamation Read Only. 0=Default. 
                            // This is a read-only status bit used to detect an empty asynchronous schedule.
                            // Only used by the host controller.
    unsigned int PS:1;          //Periodic Schedule Status Read Only. 0=Default. 
                            // This bit reports the current real status of the Periodic Schedule. When set to zero the periodic
                            // schedule is disabled, and if set to one the status is enabled. The Host Controller is not required to immediately disable or enable the Periodic
                            // Schedule when software transitions the Periodic Schedule Enable bit in the USBCMD register. When this bit and the Periodic Schedule Enable bit are the
                            // same value, the Periodic Schedule is either enabled (1) or disabled (0).
                            // Only used by the host controller.
    unsigned int AS:1;          //Asynchronous Schedule Status 〞 Read Only. 0=Default. 
                            // This bit reports the current real status of the Asynchronous Schedule. When set to zero the
                            // asynchronous schedule status is disabled and if set to one the status is enabled. The Host Controller is not required to immediately disable or enable
                            // the Asynchronous Schedule when software transitions the Asynchronous Schedule Enable bit in the USBCMD register. When this bit and the
                            // Asynchronous Schedule Enable bit are the same value, the Asynchronous Schedule is either enabled (1) or disabled (0).
                            // Only used by the host controller.

    unsigned int NAKI:1;    //NAK Interrupt bit 

    unsigned int RESERVED:15;   //Reserved. These bits are reserved and should be set to zero.
}USB_USBSTS_T;


typedef struct _USB_USBINTR{
    unsigned int UE:1;          //USB Interrupt Enable
                            // When this bit is a one, and the USBINT bit in the USBSTS register is a one, the host/device controller will issue an
                            // interrupt at the next interrupt threshold. The interrupt is acknowledged by software clearing the USBINT bit.
    unsigned int UEE:1;     // USB Error Interrupt Enable
                            // When this bit is a one, and the USBERRINT bit in the USBSTS register is a one, the host controller will issue an interrupt at the
                            // next interrupt threshold. The interrupt is acknowledged by software clearing the USBERRINT bit in the USBSTS register.
    unsigned int PCE:1;     //Port Change Detect Enable
                            // When this bit is a one, and the Port Change Detect bit in the USBSTS register is a one, the host/device controller will issue
                            // an interrupt. The interrupt is acknowledged by software clearing the Port Change Detect bit.
    unsigned int FRE:1;     // Frame List Rollover Enable
                            // When this bit is a one, and the Frame List Rollover bit in the USBSTS register is a one, the host controller will issue an
                            // interrupt. The interrupt is acknowledged by software clearing the Frame List Rollover bit.
                            // Only used by the host controller.
    unsigned int SEE:1;     //System Error Enable
                            // When this bit is a one, and the System Error bit in the USBSTS register is a one, the host/device controller will issue an
                            // interrupt. The interrupt is acknowledged by software clearing the System Error bit.
    unsigned int AAE:1;     //Interrupt on Async Advance Enable
                            // When this bit is a one, and the Interrupt on Async Advance bit in the USBSTS register is a one, the host controller will issue
                            // an interrupt at the next interrupt threshold. The interrupt is acknowledged by software clearing the Interrupt on Async Advance bit.
                            // Only used by the host controller.
    unsigned int URE:1;     //USB Reset Enable
                            //When this bit is a one, and the USB Reset Received bit in the USBSTS register is a one, the device controller will issue an
                            // interrupt. The interrupt is acknowledged by software clearing the USB Reset Received bit.
                            //Only used by the device controller.
    unsigned int SRE:1;     // SOF Received Enable
                            // When this bit is a one, and the SOF Received bit in the USBSTS register is a one, the device controller will issue an
                            // interrupt. The interrupt is acknowledged by software clearing the SOF Received bit.
    unsigned int SLE:1;     //Sleep Enable 
                            // When this bit is a one, and the DCSuspend bit in the USBSTS register transitions, the device controller will issue an interrupt.
                            // The interrupt is acknowledged by software writing a one to the DCSuspend bit.
                            // Only used by the device controller.
    unsigned int R:1;           //Reserved. These bits are reserved and should be set to zero.
    unsigned int ULPIE:1;       //ULPI Enable 
                            // When this bit is a one, and the ULPI Interrupt bit in the USBSTS register transitions, the controller will issue and interrupt. The
                            // interrupt is acknowledged by software writing a one to the ULPI Interrupt bit.
                            // Used by both host & device controller. Only present in designs where configuration constant VUSB_HS_PHY_ULPI = 1.
    unsigned int R1:5;           //Reserved. These bits are reserved and should be set to zero.
    //The NAK interrupt is enabled to workaround the issue where the Device controller does not ACK 
    //the repeated status stages but NAKs them. ENDPTNAK and ENDPTNAKEN are the registers that are used to implement the workaround.
    unsigned int NAKE:1;          //Asynchronous Schedule Status 〞 Read Only. 0=Default. 

    unsigned int RESERVED:15; //Reserved. These bits are reserved and should be set to zero.
}USB_USBINTR_T;

typedef struct _USB_BURSTSIZE {
    unsigned int RXPBURST:8;    //Programmable RX Burst Length. (Read/Write) Default is the constant VUSB_HS_RX_BURST. 
                            // This register represents the maximum length of a the burst in 32-bit words while moving data from the USB bus to system memory.
    unsigned int TXPBURST:8;    // Programmable TX Burst Length. (Read/Write) Default is the constant VUSB_HS_TX_BURST. 
                            // This register represents the maximum length of a the burst in 32-bit words while moving data from system memory to the USB bus.
    unsigned int RESERVED:24; //Reserved. These bits are reserved and should be set to zero.
}USB_BURSTSIZE_T;       //Read/Write (Writes must be DWord Writes)

typedef struct _USB_TXFILLTUNING {//The fields in this register control performance tuning associated with how the host controller posts data to
                                     //the TX latency FIFO before moving the data onto the USB bus. The specific areas of performance include
                                     //the how much data to post into the FIFO and an estimate for how long that operation should take in the target system.    unsigned int TXSCHOH:8;
    unsigned int TXSCHEALTH:5;
    unsigned int RESERVED:3;    //Reserved. These bits are reserved and should be set to zero.
    unsigned int TXFIFOTHRES:6;
    unsigned int RESERVED2:10;  //Reserved. These bits are reserved and should be set to zero.
}USB_TXFILLTUNING_T;    // Default:00020000h Read/Write (Writes must be DWord Writes)


typedef struct _USB_ULPI_VIEWPORT { //The register provides indirect access to the ULPI PHY register set. Although the core performs access to
                                        // the ULPI PHY register set, there may be extraordinary circumstances where software may need direct access.
    unsigned int ULPIDATWR:8;   //ULPI Data Write . Read/Write. 
                            // When a write operation is commanded, the data to be sent is written to this field.
    unsigned int ULPIDATRD:8; //ULPI Data Read . Read Only. 
                            // After a read operation completes, the result is placed in this field
    unsigned int ULPIADDR:8;    //ULPI Data Address . Read/Write. 
                            // When a read or write operation is commanded, the address of the operation is written to  this field.
    unsigned int ULPIPORT:3;    // ULPI Port Number "C Read/Write. For the wakeup or read/write operation to be executed, this value selects the port number the ULPI PHY is
                            // attached to in the multi-port host. The range is 0 to 7. This field should always be written as a 0 for the non-multi port products.
    unsigned int ULPISS:1;      //ULPI Sync State "C Read Only. 
                            // (1) "C Normal Sync. State. (0) In another state (ie. carkit, serial, low power) This bit represents the state of the ULPI
                            // interface. Before reading this bit, the ULPIPORT field should be set accordingly if used with the multi-port host. Otherwise, this field should
                            // always remain 0.
    unsigned int R:1;           //Reserved. These bits are reserved and should be set to zero.
    unsigned int ULPIRW:1;  // ULPI Read/Write Control "C Read/Write. 
                            // (0) "C Read; (1) "C Write. This bit selects between running a read or write operation.
    unsigned int ULPIRUN:1; // ULPI Read/Write Run "C Read/Write. 
                            // Writing the '1' to this bit will begin the read/write operation. The bit will automatically transition to 0 after the
                            // read/write is complete. Once this bit is set, the driver can not set it back to'0'.
                            // Note: The driver must never executue a wakeup and a read/write operation at the same time.
    unsigned int ULPIWU:1;  //ULPI Wakeup "C Read/Write. 
                            // Writing the'1' to this bit will begin the wakeup operation. The bit will automatically transition to 0 after the
                            // wakeup is complete. Once this bit is set, the driver can not set it back to '0'.
}USB_ULPI_VIEWPORT_T;

typedef struct _USB_PORTSC { 
    unsigned int CCS:1; //Current Connect Status Read Only.
                        // In Host Mode:
                        // 1=Device is present on port. 
                        // 0=No device is present. Default = 0.
                        // This value reflects the current state of the port, and may not correspond directly to the
                        // event that caused the Connect Status Change bit (Bit 1) to be set. This field is zero if Port Power(PP) is zero in host mode.
                        // In Device Mode:
                        // 1=Attached. 
                        // 0=Not Attached. Default=0. 
                        // A one indicates that the device successfully attached and is operating in either high speed or full speed as
                        // indicated by the High Speed Port bit in this register. A zero indicates that the device did not attach successfully or was forcibly disconnected by the software
                        // writing a zero to the Run bit in the USBCMD register. It does not state the device being disconnected or suspended   
    unsigned int CSC:1; //Connect Status Change R/WC. 
                        // 1 =Change in Current Connect Status.
                        // 0=No change. Default 0.
                        // In Host Mode:
                        // Indicates a change has occurred in the port's Current Connect Status. The host/device controller sets this bit for all changes to the port device connect
                        // status, even if system software has not cleared an existing connect status change. For example, the insertion status changes twice before system
                        // software has cleared the changed condition, hub hardware will be 'setting' an already-set bit (i.e., the bit will remain set). Software clears this bit by writing a
                        // one to it.
                        // This field is zero if Port Power(PP) is zero in host mode.
                        // In Device Mode:
                        // This bit is undefined in device controller mode.
    unsigned int PE:1;      //Port Enabled/Disabled Read/Write. 1=Enable. 0=Disable. Default 0.
                        // In Host Mode:
                        //Ports can only be enabled by the host controller as a part of the reset and enable. Software cannot enable a port by writing a one to this field. Ports can
                        // be disabled by either a fault condition (disconnect event or other fault condition) or by the host software. Note that the bit status does not change until the port
                        // state actually changes. There may be a delay in disabling or enabling a port due to other host controller and bus events.
                        // When the port is disabled, (0b) downstream propagation of data is blocked except for reset.
                        // This field is zero if Port Power(PP) is zero in host mode.
                        // In Device Mode:
                        // The device port is always enabled. (This bit will be one)
    unsigned int PEC:1; //Port Enable/Disable Change R/WC. 
                        // 1=Port enabled/disabled status has changed. 
                        // 0=No change. Default = 0.
                        // In Host Mode:
                        // For the root hub, this bit gets set to a one only when a port is disabled due to disconnect on the port or due to the appropriate conditions existing at the EOF2
                        // point (See Chapter 11 of the USB Specification). Software clears this by writing a one to it.
                        // This field is zero if Port Power(PP) is zero.
                        // In Device mode:
                        // The device port is always enabled. (This bit will be zero).
    unsigned int OCA:1; //Over-current Active Read Only. Default 0. 
                        // 1=This port currently has an over-current condition. 
                        // 0=This port does not have an over-current condition.
                        // This bit will automatically transition from one to zero when the over current condition is removed.
                        // For host/OTG implementations the user can provide over-current detection to the vbus_pwr_fault input for this condition.
                        // For device-only implementations this bit shall always be 0.
    unsigned int OCC:1; // Over-current Change R/WC. Default=0. 
                        // 1=This bit gets set to one when there is a change to Over-current Active. Software clears this bit by writing a
                        //  one to this bit position. For host/OTG implementations the user can provide over-current detection to
                        // the vbus_pwr_fault input for this condition.
                        //For device-only implementations this bit shall always be 0.
    unsigned int FPR:1; //Force Port Resume Read/Write. 1= Resume detected/driven on port. 0=No
                        //resume (K-state) detected/driven on port. Default = 0.
                        //In Host Mode:
                        // Software sets this bit to one to drive resume signaling. The Host Controller sets this bit to one if a J-to-K transition is detected while the port is in the Suspend
                        // state. When this bit transitions to a one because a J-to-K transition is detected, the Port Change Detect bit in the USBSTS register is also set to one. This bit
                        // will automatically change to zero after the resume sequence is complete. This behavior is different from EHCI where the host controller driver is required to
                        // set this bit to a zero after the rsume duration is timed in the driver.
                        // Note that when the Host controller owns the port, the resume sequence follows the defined sequence documented in the USB Specification Revision 2.0. The
                        // resume signaling (Full-speed 'K') is driven on the port as long as this bit remains a one. This bit will remain a one until the port has switched to the
                        // high-speed idle. Writing a zero has no affect because the port controller will time the resume operation clear the bit the port control state switches to HS or
                        // FS idle.
                        // This field is zero if Port Power(PP) is zero in host mode.
                        // This bit is not-EHCI compatible.
                        // In Device mode:
                        // After the device has been in Suspend State for 5ms or more, software must set this bit to one to drive resume signaling before clearing. The Device Controller
                        // will set this bit to one if a J-to-K transition is detected while the port is in the Suspend state. The bit will be cleared when the device returns to normal
                        // operation. Also, when this bit transitions to a one because a J-to-K transition detected, the Port Change Detect bit in the USBSTS register is also set to one.
    unsigned int SUSP:1;    //In Host Mode: Read/Write. 1=Port in suspend state. 0=Port not in suspend state. Default=0.
                        // Port Enabled Bit and Suspend bit of this register define the port states as follows:
                        // Bits [Port Enabled, Suspend]Port State
                        // 0x Disable
                        // 10 Enable
                        // 11 Suspend
                        // When in suspend state, downstream propagation of data is blocked on this port, except for port reset. The blocking occurs at the end of the current transaction
                        // if a transaction was in progress when this bit was written to 1. In the suspend state, the port is sensitive to resume detection. Note that the bit status does not
                        // change until the port is suspended and that there may be a delay in suspending a port if there is a transaction currently in progress on the USB.
                        // The host controller will unconditionally set this bit to zero when software sets the Force Port Resume bit to zero. The host controller ignores a write of zero to this bit.
                        // If host software sets this bit to a one when the port is not enabled (i.e. Port enabled bit is a zero) the results are undefined.
                        // This field is zero if Port Power(PP) is zero in host mode.
                        // In Device Mode: Read Only. 1=Port in suspend state. 0=Port not in suspend state. Default=0.
                        // In device mode this bit is a read only status bit.
                        
    unsigned int PR:1;      //Port Reset
                        // This field is zero if Port Power(PP) is zero. 
                        // In Host Mode: Read/Write. 1=Port is in Reset. 0=Port is not in Reset. Default 0.
                        // When software writes a one to this bit the bus-reset sequence as defined in the USB Specification Revision 2.0 is started. This bit will automatically change to
                        // zero after the reset sequence is complete. This behavior is different from EHCI where the host controller driver is required to set this bit to a zero after the reset
                        // duration is timed in the driver.
                        // In Device Mode: This bit is a read only status bit. Device reset from the USB bus is also indicated in the USBSTS register.
    unsigned int HSP:1; //High-Speed Port Read Only. Default = 0b.
                        // When the bit is one, the host/device connected to the port is in high-speed mode and if set to zero, the host/device connected to the port is not in a
                        // high-speed mode.
                        // Note: HSP is redundant with PSPD(27:26) but will remain in the design for compatibility.
                        //This bit is not defined in the EHCI specification.
    unsigned int LS:2;      //Line Status Read Only. 
                        // These bits reflect the current logical levels of the D+(bit 11) and D- (bit 10) signal lines. The encoding of the bits are:
                        //  Bits [11:10]Meaning
                        //  00b SE0
                        //  10b J-state
                        //  01b K-state
                        //  11b Undefined
                        // In host mode, the use of linestate by the host controller driver is not necessary (unlike EHCI), because the port controller state machine and the port routing
                        // manage the connection of LS and FS.
                        // In device mode, the use of linestate by the device controller driver is not necessary
    unsigned int PP:1;      //Port Power (PP) Read/Write or Read Only. 
                        // The function of this bit depends on the value of the Port Power Switching (PPC) field in the HCSPARAMS
                        // register. The behavior is as follows:
                        //  PPC         PP      Operation
                        //  0b      0b      Read Only A device controller with no OTG capability does not have port power control switches.
                        //  1b      1b/0b C  RW. Host/OTG controller requires port power control switches. This bit represents the current setting of the switch (0=off, 1=on).
                        // When power is not available on a port (i.e. PP equals a 0), the port is non-functional and will not report attaches, detaches, etc.
                        // When an over-current condition is detected on a powered port and PPC is a one, the PP bit in each affected port may be transitioned by the host controller
                        // driver from a one to a zero (removing power from the port).
                        // This feature is implemented in the host/OTG controller (PPC = 1).In a device only implementation port power control is not necessary, thus PPC and PP =0
    unsigned int PO:1;      //Port Owner Read/Write. Default = 0. 
                        // This bit unconditionally goes to a 0 when the configured bit in the CONFIGFLAG register makes a 0 to 1 transition.
                        // This bit unconditionally goes to 1 whenever the Configured bit is zero System software uses this field to release ownership of the port to a selected host
                        // controller (in the event that the attached device is not a high-speed device).
                        // Software writes a one to this bit when the attached device is not a high-speed device. A one in this bit means that an internal companion controller owns and
                        // controls the port.
                        // Port owner handoff is not implemented in this design, therefore this bit will always be 0.
    unsigned int PIC:2; //Port Indicator Control Read/Write. Default = Ob. 
                        // Writing to this field has no effect if the P_INDICATOR bit in the HCSPARAMS register is a zero. If
                        // P_INDICATOR bit is a one, then the bit is:
                        //  00b Port indicators are off
                        //  01b Amber
                        //  10b Green
                        //  11b Undefined
                        // Refer to the USB Specification Revision 2.0 Universal Serial Bus Specification, Revision 2.0, April 2000, Compaq, Hewlett-Packard, Intel, Lucent, Microsoft,
                        // NEC, Philips. http://www.usb.org for a description on how these bits are to be used.
                        // This field is output from the controller as signals port_ind_ctl_1 & port_ind_ctl_0 for use by an external led driving circuit
                        
    unsigned int PTC:4; //Port Test Control Read/Write. Default = 0000b. 
                        // Any other value than zero indicates that the port is operating in test mode.
                        // ValueSpecific Test
                        //  0000bTEST_MODE_DISABLE
                        //  0001bJ_ STATE
                        //  0010bK_STATE
                        //  0011bSE0 (host) / NAK (device)
                        //  0100bPacket
                        //  0101bFORCE_ENABLE_HS
                        //  0110bFORCE_ENABLE_FS
                        //  0111bFORCE_ENABLE_LS
                        //  1000b to 1111bReserved
                        // Refer to Chapter 7 of the USB Specification Revision 2.0 Universal Serial Bus Specification, Revision 2.0, April 2000, Compaq, Hewlett-Packard, Intel,
                        // Lucent, Microsoft, NEC, Philips. http://www.usb.org for details on each test mode.
                        // The FORCE_ENABLE_FS and FORCE ENABLE_LS are extensions to the test mode support specified in the EHCI specification. Writing the PTC field to any
                        // of the FORCE_ENABLE_{HS/FS/LS} values will force the port into the connected and enabled state at the selected speed. Writing the PTC field back
                        // to TEST_MODE_DISABLE will allow the port state machines to progress normally from that point.
                        // Note: Low speed operations are not supported as a peripheral device
    unsigned int WKCN:1;    //Wake on Connect Enable (WKCNNT_E) Read/Write. Default=0b. 
                        //Writing this bit to a one enables the port to be sensitive to device connects as wake-up events.
                        // This field is zero if Port Power(PP) is zero or in device mode.
                        // This bit is output from the controller as signal pwrctl_wake_dscnnt_en (OTG/host core only) for use by an external power control circuit. 
    unsigned int WKDC:1;   //Wake on Disconnect Enable (WKDSCNNT_E) Read/Write. Default=0b.
                        //Writing this bit to a one enables the port to be sensitive to device disconnects as wake-up events.
                        // This field is zero if Port Power(PP) is zero or in device mode.
                        // This bit is output from the controller as signal pwrctl_wake_dscnnt_en (OTG/host core only) for use by an external power control circuit
    unsigned int WKOC:1;    //Wake on Over-current Enable (WKOC_E) Read/Write.Default = 0b.
                        // Writing this bit to a one enables the port to be sensitive to over-current conditions as wake-up events.
                        // This field is zero if Port Power(PP) is zero. 
                        // This bit is output from the controller as signal pwrctl_wake_ovrcurr_en (OTG/host core only) for use by an external power control circuit.
    unsigned int PHCD:1;    // PHY Low Power Suspend - Clock Disable (PLPSCD) "C Read/Write. Default= 0b. 
                        // Writing this bit to a 1b will disable the PHY clock. Writing a 0b enables it. Reading this bit will indicate the status of the PHY clock. NOTE: The PHY clock
                        // cannot be disabled if it is being used as the system clock. In device mode, The PHY can be put into Low Power Suspend "C Clock Disable
                        // when the device is not running (USBCMD Run/Stop=0b) or the host has signaled suspend (PORTSC SUSPEND=1b). Low power suspend will be
                        // cleared automatically when the host has signaled resume if using a circuit similar to that in . Before forcing a resume from the device, the device controller
                        // driver must clear this bit.
                        // In host mode, the PHY can be put into Low Power Suspend 每 Clock Disable when the downstream device has been put into suspend mode or when no
                        // downstream device is connected. Low power suspend is completely under the control of software.
                        // See for more discussion on clock disable and power down issues.
                        // This bit is not defined in the EHCI specification.
                        
    unsigned int PFSC:1;    //Port Force Full Speed Connect "C Read/Write. Default = 0b. 
                        // Writing this bit to a 1b will force the port to only connect at Full Speed. It disables the chirp
                        // sequence that allows the port to identify itself as High Speed. This is useful for testing FS configurations with a HS host, hub or device.
                        // This bit is not defined in the EHCI specification.
                        // This bit is for debugging purposes.
    unsigned int R:1;       //Reserved. These bits are reserved and should be set to zero.
    unsigned int PSPD:2;    //Port Speed 每 Read Only.  
                        // This register field indicates the speed at which the port is operating. For HS mode operation in the host controller and HS/FS
                        // operation in the device controller the port routing steers data to the Protocol engine. For FS and LS mode operation in the host controller, the port routing
                        // steers data to the Protocol Engine w/ Embedded Transaction Translator.
                        // 00 "C Full Speed
                        // 01 "C Low Speed
                        // 10 "C High Speed
                        //This bit is not defined in the EHCI specification.
    unsigned int PTW:1; //Parallel Transceiver Width "C Read/Write. 
                        // This register bit is used in conjunction with the configuration constant VUSB_HS_PHY8_16 to control
                        // whether the data bus width of the UTMI transceiver interface. If VUSB_HS_PHY8_16 is set for 0 or 1 then this bit is read only. If
                        // VUSB_HS_PHY8_16 is 2 or 3 then this bit is read/write. This bit is reset to 1 if VUSB_HS_PHY8_16 selects a default UTMI interface width of 16-bits else it is
                        // reset to 0.
                        // Writing this bit to 0 selects the 8-bit [60MHz] UTMI interface.
                        // Writing this bit to 1 selects the 16-bit [30MHz] UTMI interface.
                        // This bit has no effect if the Philips or Serial interfaces are selected.
                        // This bit is not defined in the EHCI specification.
    unsigned int STS:1; //Serial Transceiver Select 每 Read/Write. 
                        // This register bit is used in conjunction with the configuration constant VUSB_HS_PHY_SERIAL to control
                        // whether the parallel or serial transceiver interface is selected for FS and LS operation. The Serial Interface Engine can be used in combination with the
                        // UTMI+ or ULPI physical interface to provide FS/LS signaling instead of the parallel interface. If VUSB_HS_PHY_SERIAL is set for 0 or 1 then this bit is
                        // read only. If VUSB_HS_PHY_SERIAL is 3 or 4 then this bit is read/write. This bit has no effect unless Parallal Transceiver Select is set to UTMI+ or ULPI.
                        // The Philips and Serial/1.1 physical interface will use the Serial Interface Engine for FS/LS signaling regardless of this bit value.
                        // Note: This bit is reserved for future operation and is a placeholder adding dynamic use of the serial engine in accord with UMTI+ and ULPI
                        // characterization logic.
                        // This bit is not defined in the EHCI specification.
    unsigned int PTS:2; //Parallel Transceiver Select 每 Read/Write. 
                        // This register bit pair is used in conjunction with the configuration constant VUSB_HS_PHY_TYPE to control
                        // which parallel transceiver interface is selected. If VUSB_HS_PHY_TYPE is set for 0,1,2, or 3 then this bit is read only. If VUSB_HS_PHY_TYPE is 3,4,5, or 6
                        // then this bit is read/write.
                        // This field is reset to:
                        // 00 if VUSB_HS_PHY_TYPE = 0,4 "C UTMI/UTMI+
                        // 01 if VUSB_HS_PHY_TYPE = 1,5 "C Philips Classic
                        // 10 if VUSB_HS_PHY_TYPE = 2,6 "C ULPI
                        // 11 if VUSB_HS_PHY_TYPE = 3,7 "C Serial/1.1 PHY (FS Only) 
}USB_PORTSC_T;


typedef struct _USB_OTGSC { 
    unsigned int VD:1;      //VBUS_Discharge "C Read/Write. Setting this bit causes VBus to discharge through a resistor.
    unsigned int VC:1;      //VBUS Charge "C Read/Write. Setting this bit causes the VBus line to be charged. This is used for VBus pulsing during SRP.
    unsigned int R1:1;      //Reserved. These bits are reserved and should be set to zero.
    unsigned int OT:1;      //OTG Termination "C Read/Write.This bit must be set when the OTG device is in device mode, this controls the pulldown on DM.
    unsigned int DP:1;      //Data Pulsing "C Read/Write.Setting this bit causes the pullup on DP to be asserted for data pulsing during SRP.
    unsigned int IDPU:1;    //ID Pullup "C Read/Write This bit provide control over the ID pull-up resister ; 0 = off, 1 = on [default]. When this bit is 0, the ID input will not be sampled.
    unsigned int R2:2;      //Reserved. These bits are reserved and should be set to zero.

    unsigned int ID:1;      //USB ID "C Read Only. 0 = A device, 1 = B device
    unsigned int AVV:1; //A VBus Valid "C Read Only. Indicates VBus is above the A VBus valid threshold.
    unsigned int ASV:1; //A Session Valid "C Read Only. Indicates VBus is above the A session valid threshold.
    unsigned int BSV:1; //B Session Valid "C Read Only. Indicates VBus is above the B session valid threshold.
    unsigned int BSE:1; //B Session End "C Read Only. Indicates VBus is below the B session end threshold.
    unsigned int _1MST:1;   // 1 milisecond timer toggle - Read Only. This bit toggles once per millisecond.
    unsigned int DPS:1; //Data Bus Pulsing Status "C Read Only. A '1' indicates data bus pulsing is being detected on the port.
    unsigned int R3:1;      //Reserved. These bits are reserved and should be set to zero.

    unsigned int IDIS:1;    // USB ID Interrupt Status "C Read/Write. This bit is set when a change on the ID input has been detected. Software must write a one to clear this bit.
    unsigned int AVVIS:1;   // A VBus Valid Interrupt Status "C Read/Write to Clear. This bit is set when VBus has either risen above or fallen below the VBus valid threshold (4.4 VDC) on an A device. Software must write a one to clear this bit.
    unsigned int ASVIS:1;   // A Session Valid Interrupt Status "C Read/Write to Clear. This bit is set when VBus has either risen above or fallen below the A session valid threshold (0.8 VDC). Software must write a one to clear this bit.
    unsigned int BSVIS:1;   // B Session Valid Interrupt Status "C Read/Write to Clear. This bit is set when VBus has either risen above or fallen below the B session valid threshold (0.8 VDC). Software must write a one to clear this bit.
    unsigned int BSEIS:1;   // B Session End Interrupt Status "C Read/Write to Clear.This bit is set when VBus has fallen below the B session end threshold.Software must write a one to clear this bit
    unsigned int _1MSS:1;   // 1 milisecond timer Interrupt Status "C Read/Write to Clear.This bit is set once every millisecond. Software must write a one to clear this bit.
    unsigned int DPIS:1;    //Data Pulse Interrupt Status "C Read/Write to Clear.
                        // This bit is set when data bus pulsing occurs on DP or DM. Data bus pulsing is only detected when USBMODE.CM = Host (11) and PORTSC(0).PortPower =
                        // Off (0).
                        // Software must write a one to clear this bit.
    unsigned int R4:1;      //Reserved. These bits are reserved and should be set to zero.

    unsigned int IDIE:1;    //USB ID Interrupt Enable "C Read/Write.Setting this bit enables the USB ID interrupt.
    unsigned int AVVIE:1;   // A VBus Valid Interrupt Enable "C Read/Write.Setting this bit enables the A VBus valid interrupt.
    unsigned int ASVIE:1;   //A Session Valid Interrupt Enable "C Read/Write. Setting this bit enables the A session valid interrupt.
    unsigned int BSVIE:1;   // B Session Valid Interrupt Enable "C Read/Write. Setting this bit enables the B session valid interrupt.
    unsigned int BSEIE:1;   //B Session End Interrupt Enable "C Read/Write. Setting this bit enables the B session end interrupt.
    unsigned int _1MSE:1;   // 1 milisecond timer Interrupt Enable "C Read/Write
    unsigned int DPIE:1;    //Data Pulse Interrupt Enable
    unsigned int R5:1;      //Reserved. These bits are reserved and should be set to zero.
}USB_OTGSC_T;

typedef struct _USB_USBMODE { 
    unsigned int CM:2;      //Controller Mode "C R/WO. Controller mode is defaulted to the proper mode for
                        // host only and device only implementations. For those designs that contain both   host & device capability, the controller will default to an idle state and will need
                        // to be initialized to the desired operating mode after reset. For combination host/device controllers, this register can only be written once after reset. If it is
                        // necessary to switch modes, software must reset the controller by writing to the RESET bit in the USBCMD register before reprogramming this register.
                        // Bit Meaning
                        // 00 Idle [Default for combination host/device]
                        // 01 Reserved
                        // 10 Device Controller [Default for device only controller]
                        // 11 Host Controller [Default for host only controller]
    unsigned int ES:1;      // Endian Select 每 Read/Write. This bit can change the byte alignment of the transfer buffers to match the host microprocessor. The bit fields in the
                        // microprocessor interface and the data structures are unaffected by the value of this bit because they are based upon the 32-bit word.
                        // Bit Meaning
                        // 0 Little Endian [Default]
                        // 1 Big Endian
    unsigned int SLOM:1;    // Setup Lockout Mode. 
                        // In device mode, this bit controls behavior of the setup lock mechanism. See Control Endpoint Operation Model.
                        // 0 每 Setup Lockouts On (default); 1 每 Setup Lockouts Off (DCD requires use of Setup Data Buffer Tripwire in USBCMD)
    unsigned int SDIS:1;    //Stream Disable Mode. (0 每 Inactive [default] ; 1 每 Active)
                        // Device Mode: Setting to a '1' disables double priming on both RX and TX for low bandwidth systems. This mode ensures that when the RX and TX buffers
                        // are sufficient to contain an entire packet that the standared double buffering scheme is disabled to prevent overruns/underruns in bandwidth limited
                        //systems. Note: In High Speed Mode, all packets received will be responded to with a NYET handshake when stream disable is active.
                        // Host Mode: Setting to a '1' ensures that overruns/underruns of the latency FIFO are eliminated for low bandwidth systems where the RX and TX buffers
                        // are sufficient to contain the entire packet. Enabling stream disable also has the effect of ensuring the the TX latency is filled to capacity before the packet is
                        // lanched onto the USB.
                        // Note: Time duration to pre-fill the FIFO becomes significate when stream disable is active. See TXFILLTUNING and TXTTFILLTUNING [MPH Only] to
                        // characterize the adjustments needed for the scheduler when using this feature.
                        // Note: The use of this feature substantially limits of the overall USB performance that can be achived.
    unsigned int r:27;      // Reserved. These bits are reserved and should be set to zero.                     
}USB_USBMODE_T;

typedef struct _USB_ENDPTPRIME { 
    unsigned int PERB:16;   // Prime Endpoint Receive Buffer 每 R/WS. 
                        // For each endpoint, a corresponding bit is used to request a buffer prepare for a receive operation for when a USB
                        // host initiates a USB OUT transaction. Software should write a one to the corresponding bit whenever posting a new transfer descriptor to an endpoint.
                        // Hardware will automatically use this bit to begin parsing for a new transfer descriptor from the queue head and prepare a receive buffer. Hardware will
                        // clear this bit when the associated endpoint(s) is (are) successfully primed.
                        // Note: These bits will be momentarily set by hardware during hardware re-priming operations when a dTD is retired, and the dQH is updated.
                        // Bit 15 每 Endpoint #15
                        // Bit 1 每 Endpoint #1
                        // Bit 0 每 Endpoint #0
    unsigned int PETB:16;   //Prime Endpoint Transmit Buffer 每 R/WS. 
                        // For each endpoint a corresponding bit is used to request that a buffer prepared for a transmit operation in order to
                        // respond to a USB IN/INTERRUPT transaction. Software should write a one to the corresponding bit when posting a new transfer descriptor to an endpoint.
                        // Hardware will automatically use this bit to begin parsing for a new transfer descriptor from the queue head and prepare a transmit buffer. Hardware will
                        // clear this bit when the associated endpoint(s) is (are) successfully primed. 
                        // Note: These bits will be momentarily set by hardware during hardware re-priming operations when a dTD is retired, and the dQH is updated.
                        // PETB[15] 每 Endpoint #15
                        // PETB[1] 每 Endpoint #1
                        // PETB[0] 每 Endpoint #0
}USB_ENDPTPRIME_T;

typedef struct _USB_ENDPTFLUSH { 
    unsigned int FERB:16;   // Flush Endpoint Receive Buffer 每 R/WS. 
                        //Writing a one to a bit(s) will cause the associated endpoint(s) to clear any primed buffers. If a packet is in
                        // progress for one of the associated endpoints, then that transfer will continue until completion. Hardware will clear this register after the endpoint flush
                        // operation is successful.
                        // Bit 15 每 Endpoint #15
                        // Bit 1 每 Endpoint #1
                        // Bit 0 每 Endpoint #0
    unsigned int FETB:16;   //Flush Endpoint Transmit Buffer 每 R/WS. 
                        // Writing a one to a bit(s) in this register will cause the associated endpoint(s) to clear any primed buffers. If a
                        // packet is in progress for one of the associated endpoints, then that transfer will continue until completion. Hardware will clear this register after the endpoint
                        // flush operation is successful.
                        // FETB[15] 每 Endpoint #15
                        // FETB[1] 每 Endpoint #1
                        // FETB[0] 每 Endpoint #0
}USB_ENDPTFLUSH_T;

typedef struct _USB_ENDPTSTAT { 
    unsigned int ERBR:16;   // Endpoint Receive Buffer Ready -- Read Only. 
                        // One bit for each endpoint indicates status of the respective endpoint buffer. This bit is set to a one by the
                        // hardware as a response to receiving a command from a corresponding bit in the ENDPTPRIME register. There will always be a delay between setting a bit in the
                        // ENDPTPRIME register and endpoint indicating ready. This delay time varies based upon the current USB traffic and the number of bits set in the
                        // ENDPTPRIME register. Buffer ready is cleared by USB reset, by the USB DMA system, or through the ENDPTFLUSH register.
                        // Note: These bits will be momentarily cleared by hardware during hardware endpoint re-priming operations when a dTD is retired, and the dQH is updated.
                        // ERBR[15]每 Endpoint #15
                        // ERBR[1]每 Endpoint #1
                        // ERBR[0]每 Endpoint #0
    unsigned int ETBR:16;   // Endpoint Transmit Buffer Ready -- Read Only. 
                        // One bit for each endpoint indicates status of the respective endpoint buffer. This bit is set to a one by the
                        // hardware as a response to receiving a command from a corresponding bit in the ENDPTPRIME register. There will always be a delay between setting a bit in the
                        // ENDPTPRIME register and endpoint indicating ready. This delay time varies based upon the current USB traffic and the number of bits set in the
                        // ENDPTPRIME register. Buffer ready is cleared by USB reset, by the USB DMA system, or through the ENDPTFLUSH register.
                        // Note: These bits will be momentarily cleared by hardware during hardware endpoint re-priming operations when a dTD is retired, and the dQH is updated.
                        // ETBR[15]每 Endpoint #15
                        // ETBR[1]每 Endpoint #1
                        // ETBR[0]每 Endpoint #0
}USB_ENDPTSTAT_T;

typedef struct _USB_ENDPTCOMPLETE { 
    unsigned int ERCE:16;   // Endpoint Receive Complete Event 每 RW/C. 
                        // Each bit indicates a received event (OUT/SETUP) occurred and software should read the corresponding
                        // endpoint queue to determine the transfer status. If the corresponding IOC bit is set in the Transfer Descriptor, then this bit will be set simultaneously with the
                        // USBINT. Writing a one will clear the corresponding bit in this register.
                        // ERCE[15]每 Endpoint #15
                        // ERCE[1]每 Endpoint #1
                        // ERCE[0]每 Endpoint #0
    unsigned int ETCE:16;   // Endpoint Transmit Complete Event 每 R/WC. 
                        // Each bit indicates a transmit event (IN/INTERRUPT) occurred and software should read the corresponding
                        //endpoint queue to determine the endpoint status. If the corresponding IOC bit is set in the Transfer Descriptor, then this bit will be set simultaneously with the
                        // USBINT. Writing a one will clear the corresponding bit in this register.
                        // ETCE[15]每 Endpoint #15
                        // ETCE[1]每 Endpoint #1
                        // ETCE[0]每 Endpoint #0
}USB_ENDPTCOMPLETE_T;

//The NAK interrupt is enabled to workaround the issue where the Device controller does not ACK 
//the repeated status stages but NAKs them. ENDPTNAK and ENDPTNAKEN are the registers that are used to implement the workaround.

typedef struct _USB_ENDPTNAKEN { 
    unsigned int EPRNE:16;   //Each bit is an enaable bit for te
                            //corresponding RX Endpoint NAK bit. If this bit is set and the corresponding RX
                            //Endpoint NAK bit is set, the NAK interrupt bit is set.
                            //Bit 15 ?Endpoint #15
                            //Bit 1 ?Endpoint #1
                            //Bit 0 ?Endpoint #0
    unsigned int EPTNE:16;   //Each bit is an enaable bit for te
                            //corresponding TX Endpoint NAK bit. If this bit is set and the corresponding TX
                            //Endpoint NAK bit is set, the NAK interrupt bit is set.
                            //EPTNE[15] ?Endpoint #15
                            //EPTNE[1] ?Endpoint #1
                            //EPTNE[0] ?Endpoint #0
}USB_ENDPTNAKEN_T;

typedef struct _USB_ENDPTNAK { 
    unsigned int EPRN:16;  
                        //Each RX endpoint has one bit. The bit set when the
                        //device sends a NAK handshake on received OUT or PING token for the
                        //corresponding endpoint.
                        //Bit 15 ?Endpoint #15
                        //Bit 1 ?Endpoint #1
                        //Bit 0 ?Endpoint #0
    unsigned int EPTN:16; 
                        //Each TX endpoint has one bit. The bit set when the
                        //device sends a NAK handshake on received IN token for the corresponding
                        //endpoint.
                        //EPTN[15] Endpoint #15
                        //EPTN[1]  Endpoint #1
                        //EPTN[0]  Endpoint #0
}USB_ENDPTNAK_T;
typedef struct _USB_ENDPTCTRL0 { 
    unsigned int RXS:1; // RX Endpoint Stall 每 Read/Write
                        // 0 每 End Point OK. [Default]
                        // 1 每 End Point Stalled
                        // Software can write a one to this bit to force the endpoint to return a STALL handshake to the Host. It will continue returning STALL until the bit is cleared
                        // by software or it will automatically be cleared upon receipt of a new SETUP request.
    unsigned int R1:1;      //Reserved. These bits are reserved and should be set to zero.
    unsigned int RXT:2; //RX Endpoint Type 每 Read/Write 00 每 Control. Endpoint0 is fixed as a Control End Point.
    unsigned int R2:3;      //Reserved. These bits are reserved and should be set to zero.
    unsigned int RXE:1; //RX Endpoint Enable 1 每 Enabled. Endpoint0 is always enabled.
    unsigned int R3:8;      //Reserved. These bits are reserved and should be set to zero.
    unsigned int TXS:1; //TX Endpoint Stall 每 Read/Write
                        // 0 每 End Point OK [Default]
                        // 1 每 End Point Stalled
                        // Software can write a one to this bit to force the endpoint to return a STALL handshake to the Host. It will continue returning STALL until the bit is cleared
                        // by software or it will automatically be cleared upon receipt of a new SETUP request.
    unsigned int R4:1;      //Reserved. These bits are reserved and should be set to zero.
    unsigned int TXT:2; //TX Endpoint Type 每 Read/Write 00 -Control Endpoint. 0 is fixed as a Control End Point.
    unsigned int R5:3;      //Reserved. These bits are reserved and should be set to zero.
    unsigned int TXE:1; //TX Endpoint Enable 1 - Enabled Endpoint0 is always enabled.
    unsigned int R6:8;      //Reserved. These bits are reserved and should be set to zero.
}USB_ENDPTCTRL0_T;

typedef struct _USB_ENDPTCTRL { 
    unsigned int RXS:1; // RX Endpoint Stall 每 Read/Write
                        // 0 每 End Point OK. [Default]
                        // 1 每 End Point Stalled
                        // This bit will be set automatically upon receipt of a SETUP request if this Endpoint is not configured as a Control Endpoint. It will be cleared
                        // automatically upon receipt a SETUP request if this Endpoint is configured as a Control Endpoint,
                        // Software can write a one to this bit to force the endpoint to return a STALL handshake to the Host. It will continue to returning STALL until this bit is either
                        // cleared by software or automatically cleared as above,
    unsigned int RXD:1; // RX Endpoint Data Sink 每 Read/Write 每 TBD
                        // 0 每 Dual Port Memory Buffer/DMA Engine [Default]
                        // Should always be written as zero.
    unsigned int RXT:2; // RX Endpoint Type 每 Read/Write 
                        // 00 每 Control
                        // 01 每 Isochronous
                        // 10 每 Bulk
                        // 11 每 Interrupt
    unsigned int R1:1;      //Reserved. These bits are reserved and should be set to zero.
    unsigned int RXI:1; //RX Data Toggle Inhibit
                        // 0 每 Disabled [Default]
                        // 1 每 Enabled
                        // This bit is only used for test and should always be written as zero. Writing a one to this bit will cause this endpoint to ignore the data toggle sequence and
                        // always accept data packet regardless of their data PID.
    unsigned int RXR:1; //RX Data Toggle Reset (WS) 
                        // Write 1 每 Reset PID Sequence
                        // Whenever a configuration event is received for this Endpoint, software must write a one to this bit in order to synchronize the data PID's between the host and device.
    unsigned int RXE:1; // RX Endpoint Enable
                        // 0 每 Disabled [Default]
                        // 1 每 Enabled
                        
                        // An Endpoint should be enabled only after it has been configured.
    unsigned int R2:8;      //Reserved. These bits are reserved and should be set to zero.
    
    unsigned int TXS:1; //TX Endpoint Stall 每 Read/Write
                        // 0 每 End Point OK [Default]
                        // 1 每 End Point Stalled
                        //This bit will be set automatically upon receipt of a SETUP request if this Endpoint is not configured as a Control Endpoint. It will be cleared
                        // automatically upon receipt of a SETUP request if this Endpoint is configured as a Control Endpoint.
                        // Software can write a one to this bit to force the endpoint to return a STALL handshake to the Host. It will continue to returning STALL until this bit is either
                        // cleared by software or automatically cleared as above.
    unsigned int TXD:1; // TX Endpoint Data Source 每 Read/Write 0 每 Dual Port Memory Buffer/DMA Engine [DEFAULT], Should always be written as 0.
    unsigned int TXT:2; // TX Endpoint Type 每 Read/Write
                        // 00 每 Control
                        // 01 每 Isochronous
                        // 10 每 Bulk
                        // 11 每 Interrupt
    unsigned int R3:1;      //Reserved. These bits are reserved and should be set to zero.
    unsigned int TXI:1; //TX Data Toggle Inhibit
                        // 0 每 PID Sequencing Enabled. [Default]
                        // 1 每 PID Sequencing Disabled.
                        // This bit is only used for test and should always be written as zero. Writing a one to this bit will cause this endpoint to ignore the data toggle sequence and
                        // always transmit DATA0 for a data packet.
    unsigned int TXR:1; //TX Data Toggle Reset (WS)
                        // Write 1 每 Reset PID Sequence
                        // Whenever a configuration event is received for this Endpoint, software must write a one to this bit in order to synchronize the data PID's between the Host and device.
    unsigned int TXE:1; //TX Endpoint Enable
                        // 0 每 Disabled [Default]
                        // 1 每 Enabled
                        // An Endpoint should be enabled only after it has been configured.

    unsigned int R4:8;      //Reserved. These bits are reserved and should be set to zero.
}USB_ENDPTCTRL_T;



#define FIELD_OFFSET(type, field)    ((LONG)(LONG_PTR)&(((type *)0)->field))
#define offset(s, f)        FIELD_OFFSET(s, f)
#define fieldsize(s, f)     sizeof(((s*)0)->f)
#define dimof(x)            (sizeof(x)/sizeof(x[0]))

//------------------------------------------------------------------------------
//
//  Function:  ReadRegistryParams
//
//  This function initializes driver default settings from registry based on
//  table passed as argument.
//
#define PARAM_DWORD             1
#define PARAM_STRING            2
#define PARAM_MULTIDWORD        3
#define PARAM_BIN               4

typedef struct {
    LPTSTR name;
    DWORD  type;
    BOOL   required;
    DWORD  offset;
    DWORD  size;
    PVOID  pDefault;
} DEVICE_REGISTRY_PARAM;

DWORD GetDeviceRegistryParams(
    LPCWSTR szContext, VOID *pBase, DWORD count, 
    const DEVICE_REGISTRY_PARAM params[]
);

typedef struct {
    UINT32 ClockGatingMask;  // Mask for USB Clock
    UINT32 ClockGatingLock;  // USB Lock
} BSP_USB_CLOCK_GATING;

#define USBFunctionObjectName                     TEXT("USBFunc")
#define USBXcvrObjectName                         TEXT("USBXCVR")
#define USBHostObjectName                         TEXT("USBHost")


#ifdef __cplusplus
}
#endif

#endif // __COMMON_USB_H
