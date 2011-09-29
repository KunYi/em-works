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
/*

  Copyright(c) 1998,1999 Renesas Technology Corp.

	Module Name:

		cc.h

	Revision History:

		26th April 1999		Released
		25th May   1999		Added FIR definitions
		14th June  1999		Added HD64464's definitions
		28th June  1999		Fixed minor bug
		11th August1999		Add some HD64465 definitions

*/

#ifndef _CC_H_
#define _CC_H_

// Companion Chip (HD64465) module offsets

#define HD64465_STB_SYSTEM_OFFSET					0x0000		// Power down modes & system configuration
#define HD64465_RESERVED							0x1000		// Reserved
#define HD64465_PCMCIA_OFFSET						0x2000		// PCMCIA
#define HD64465_AFE_OFFSET							0x3000		// Modem analog front end
#define HD64465_GPIO_OFFSET							0x4000		// I/O ports
#define HD64465_INTC_OFFSET							0x5000		// Interrupt Controller
#define HD64465_TMR_OFFSET							0x6000		// Timer
#define HD64465_FIR_OFFSET							0x7000		// IrDA/16550(A) UART
#define HD64465_UART_OFFSET							0x8000		// 16550(B) UART
#define HD64465_EMBEDED_SDRAM_OFFSET				0x9000		// Embeded SDRAM
#define HD64465_PARALLEL_OFFSET						0xA000		// Parallel Port
#define HD64465_USB_OFFSET							0xB000		// USB Host Controller
#define HD64465_CODEC_OFFSET						0xC000		// Serial CODEC Interface
#define HD64465_KBC_OFFSET							0xD000		// Keyboard Controller Interface
#define HD64465_ADC_OFFSET							0xE000		// A/D Converter

#if ENABLE_HD64464!=1
// Companion Chip (HD64463) module offsets
#define HD64463_STB_SYSTEM_OFFSET					0x0000		// Power down modes & system configuration 
#define HD64463_LCDC_OFFSET							0x1000		// LCD/CRT 
#define HD64463_PCMCIA_OFFSET						0x2000		// PCMCIA 
#define HD64463_AFE_OFFSET							0x3000		// Modem analog front end 
#define HD64463_GPIO_OFFSET							0x4000		// I/O ports 	
#define HD64463_INTC_OFFSET							0x5000		// Interrupt Controller 
#define HD64463_TMR_OFFSET							0x6000		// Timer 
#define HD64463_FIR_OFFSET							0x7000		// IrDA/16550(A) UART 
#define HD64463_UART0_OFFSET						0x8000		// 16550(B) UART  
#define HD64463_UART1_OFFSET						0x9000		// Embeded SDRAM 
#define HD64463_PARALLEL_OFFSET						0xA000		// Parallel Port 
#define HD64463_USB_OFFSET							0xB000		// USB Host Controller 
#define HD64463_CODEC_OFFSET						0xC000		// Serial CODEC Interface 
#define HD64463_KBC_OFFSET							0xD000		// Keyboard Controller Interface 
#define HD64463_ADC_OFFSET							0xE000		// A/D Converter 
#endif

//
// definitions of System control and power interface (SYS) on the companion chip ( all the system registers are 16 bits wide )
//

#if ENABLE_HD64464!=1
#define CC_SYS_REGBASE_463							(HD64463_BASE + HD64463_STB_SYSTEM_OFFSET)
#endif

#define CC_SYS_REGBASE								(HD64465_BASE + HD64465_STB_SYSTEM_OFFSET)

#define CC_SYS_SMSCR_OFFSET							0x0000
#define CC_SYS_SCONFR_OFFSET						0x0002
#define CC_SYS_SBCR_OFFSET							0x0004
#define CC_SYS_SPCCR_OFFSET							0x0006
#define CC_SYS_SPSRCR_OFFSET						0x0008
#define CC_SYS_SPLLCR_OFFSET						0x000a
#define CC_SYS_SRR_OFFSET							0x000c
#define CC_SYS_STMCR_OFFSET							0x000e
#define CC_SYS_SDID_OFFSET							0x0010
#define CC_SYS_SDPR_OFFSET							0x0ff0

#define CC_SYS_REGSIZE								0x0ff2			// total size of SYSTEM regs in CC ASIC

#define CC_SYS_SMSCR								(CC_SYS_REGBASE + CC_SYS_SMSCR_OFFSET)
#define CC_SYS_SCONFR								(CC_SYS_REGBASE + CC_SYS_SCONFR_OFFSET)
#define CC_SYS_SBCR									(CC_SYS_REGBASE + CC_SYS_SBCR_OFFSET)
#define CC_SYS_SPCCR								(CC_SYS_REGBASE + CC_SYS_SPCCR_OFFSET)
#define CC_SYS_SPSRCR								(CC_SYS_REGBASE + CC_SYS_SPSRCR_OFFSET)
#define CC_SYS_SPLLCR								(CC_SYS_REGBASE + CC_SYS_SPLLCR_OFFSET)
#define CC_SYS_SRR									(CC_SYS_REGBASE + CC_SYS_SRR_OFFSET)
#define CC_SYS_STMCR								(CC_SYS_REGBASE + CC_SYS_STMCR_OFFSET)
#define CC_SYS_SDID									(CC_SYS_REGBASE + CC_SYS_SDID_OFFSET)
#define CC_SYS_SDPR									(CC_SYS_REGBASE + CC_SYS_SDPR_OFFSET)

// System Module Standby Control Register (SMSCR)

#define CC_SYS_SMSCR_LCDST							0x2000			// HD64463 only
#define CC_SYS_SMSCR_UART0ST						0x0800			// HD64463 only
#define CC_SYS_SMSCR_UART1ST						0x0400			// HD64463 only
#define CC_SYS_SMSCR_USBST							0x0080			// HD64463 only

#define CC_SYS_SMSCR_PS2ST							(0x4000) /* HD64465 only 99-08-11 cea */
#define CC_SYS_SMSCR_ADCST							0x1000
#define CC_SYS_SMSCR_UARTST							0x0800
#define CC_SYS_SMSCR_SCDIST							0x0200
#define CC_SYS_SMSCR_PPST							0x0100
#define CC_SYS_SMSCR_PC0ST							0x0040
#define CC_SYS_SMSCR_PC1ST							0x0020
#define CC_SYS_SMSCR_AFEST							0x0010
#define CC_SYS_SMSCR_TM0ST							0x0008
#define CC_SYS_SMSCR_TM1ST							0x0004
#define CC_SYS_SMSCR_IRDAST							0x0002
#define CC_SYS_SMSCR_KBCST							0x0001

// System Configuration Register (SCONFR)

#define CC_SYS_SCONFR_SLS							0x2000			// HD64463 only
#define CC_SYS_SCONFR_LCDCD_DIVID_3					0x0080			// HD64463 only
#define CC_SYS_SCONFR_LCDCD_DIVID_2					0x0040			// HD64463 only
#define CC_SYS_SCONFR_LCDCD_DIVID_1					0x0000			// HD64463 only
#define CC_SYS_SCONFR_ILCDMS						0x0020			// HD64463 only

#define CC_SYS_SCONFR_HWEN							0x1000
#define CC_SYS_SCONFR_HW1							0x0100
#define CC_SYS_SCONFR_HW2							0x0200
#define CC_SYS_SCONFR_HW3							0x0300
#define CC_SYS_SCONFR_HW4							0x0400
#define CC_SYS_SCONFR_HW5							0x0500
#define CC_SYS_SCONFR_HW6							0x0600
#define CC_SYS_SCONFR_HW7							0x0700
#define CC_SYS_SCONFR_HW8							0x0800
#define CC_SYS_SCONFR_HW9							0x0900
#define CC_SYS_SCONFR_HW10							0x0a00
#define CC_SYS_SCONFR_HW11							0x0b00
#define CC_SYS_SCONFR_HW12							0x0c00
#define CC_SYS_SCONFR_HW13							0x0d00
#define CC_SYS_SCONFR_HW14							0x0e00
#define CC_SYS_SCONFR_HW15							0x0f00

#define CC_SYS_SCONFR_USBCKS						0x0020
#define CC_SYS_SCONFR_SCDICKS						0x0010

#define CC_SYS_SCONFR_PPFMS_ECP_EPP					0x000b
#define CC_SYS_SCONFR_PPFMS_ECP						0x0008
#define CC_SYS_SCONFR_PPFMS_EPP						0x0004
#define CC_SYS_SCONFR_PPFMS_SPP						0x0000

#define CC_SYS_SCONFR_KBWUP							0x0002

// System Bus Control Register (SBCR)

#define CC_SYS_SBCR_LCDIG							0x0080			// HD64463 only

#define CC_SYS_SBCR_PDOF							0x8000			// HD64465 only
#define CC_SYS_SBCR_PDIG							0x4000			// HD64465 only
#define CC_SYS_SBCR_PCOF							0x2000			// HD64465 only
#define CC_SYS_SBCR_PCIG							0x1000			// HD64465 only
#define CC_SYS_SBCR_PBOF							0x0800
#define CC_SYS_SBCR_PBIG							0x0400
#define CC_SYS_SBCR_PAOF							0x0200
#define CC_SYS_SBCR_PAIG							0x0100
#define CC_SYS_SBCR_CSPE							0x0040
#define CC_SYS_SBCR_CMDPE							0x0020
#define CC_SYS_SBCR_ADDRPE							0x0010
#define CC_SYS_SBCR_DATAPE							0x0008
#define CC_SYS_SBCR_CPUBIG							0x0004
#define CC_SYS_SBCR_PEOF							0x0002			// HD64465 only
#define CC_SYS_SBCR_PEIG							0x0001			// HD64465 only

// SYSTEM Peripheral Clock Control Register

#define CC_SYS_SPCCR_URT1CLK						0x4000			// HD64463 only
#define CC_SYS_SPCCR_URT0CLK						0x2000			// HD64463 only
#define CC_SYS_SPCCR_LCKOSC							0x0004			// HD64463 only

#define CC_SYS_SPCCR_ADCCLK							0x8000
#define CC_SYS_SPCCR_UARTCLK						0x2000
#define CC_SYS_SPCCR_PPCLK							0x1000
#define CC_SYS_SPCCR_FIRCLK							0x0800
#define CC_SYS_SPCCR_SIRCLK							0x0400
#define CC_SYS_SPCCR_SCDICLK						0x0200
#define CC_SYS_SPCCR_KBCCLK							0x0100
#define CC_SYS_SPCCR_USBCLK							0x0080
#define CC_SYS_SPCCR_AFECLK							0x0040
#define CC_SYS_SPCCR_UCKOSC							0x0002
#define CC_SYS_SPCCR_AFEOSC							0x0001

// System Peripheral S/W Reset Control Register (SPSRCR)

#define CC_SYS_SPSRCR_LCDCSRT						0x2000			// HD64463 only
#define CC_SYS_SPSRCR_UR0SRT						0x0800			// HD64463 only
#define CC_SYS_SPSRCR_UR1SRT						0x0400			// HD64463 only

#define CC_SYS_SPSRCR_SPORST						0x8000
#define CC_SYS_SPSRCR_ADCSRT						0x1000
#define CC_SYS_SPSRCR_UARTSRT						0x0800
#define CC_SYS_SPSRCR_SCDISRT						0x0200
#define CC_SYS_SPSRCR_PPSRT							0x0100
#define CC_SYS_SPSRCR_USBSRT						0x0080
#define CC_SYS_SPSRCR_PC0SRT						0x0040
#define CC_SYS_SPSRCR_PC1RST						0x0020
#define CC_SYS_SPSRCR_AFERST						0x0010
#define CC_SYS_SPSRCR_TM0RST						0x0008
#define CC_SYS_SPSRCR_TM1RST						0x0004
#define CC_SYS_SPSRCR_IRDARST						0x0002
#define CC_SYS_SPSRCR_KBCRST						0x0001

// System PLL Control Register (SPLLCR)

#define CC_SYS_SPLLCR_PLL2SB						0x0020
#define CC_SYS_SPLLCR_PLL1SB						0x0010
#define CC_SYS_SPLLCR_PLL2BP						0x0002
#define CC_SYS_SPLLCR_PLL1BP						0x0001

// System Test Mode Control Register (STMCR)

#define CC_SYS_STMCR_DITST							0x0400
#define CC_SYS_STMCR_DOTST							0x0200
#define CC_SYS_STMCR_AFETST							0x0100
#define CC_SYS_STMCR_PCITST							0x0080
#define CC_SYS_STMCR_SDBST							0x0040
#define CC_SYS_STMCR_USBST							0x0020
#define CC_SYS_STMCR_PLL2TST						0x0010
#define CC_SYS_STMCR_PLL1TST						0x0008
#define CC_SYS_STMCR_URTTST							0x0004
#define CC_SYS_STMCR_ACTST							0x0002
#define CC_SYS_STMCR_DCTST							0x0001

//
// GPIO regs
//

#if ENABLE_HD64464!=1
#define CC_GPIO_REGBASE_463							(HD64463_BASE + HD64463_GPIO_OFFSET)
#endif

#define CC_GPIO_REGBASE								(HD64465_BASE + HD64465_GPIO_OFFSET)

#define CC_GPIO_GPACR_OFFSET						0x0000	// Port A Control Reg Offset Address 
#define CC_GPIO_GPBCR_OFFSET						0x0002	// Port B Control Reg Offset Address 
#define CC_GPIO_GPCCR_OFFSET						0x0004	// Port C Control Reg Offset Address 
#define CC_GPIO_GPDCR_OFFSET						0x0006	// Port D Control Reg Offset Address 
#define CC_GPIO_GPECR_OFFSET						0x0008	// Port E Control Reg Offset Address 

#define CC_GPIO_GPADR_OFFSET						0x0010	// Port A Data Reg Offset Address 
#define CC_GPIO_GPBDR_OFFSET						0x0012	// Port B Data Reg Offset Address 
#define CC_GPIO_GPCDR_OFFSET						0x0014	// Port C Data Reg Offset Address 
#define CC_GPIO_GPDDR_OFFSET						0x0016	// Port D Data Reg Offset Address 
#define CC_GPIO_GPEDR_OFFSET						0x0018	// Port E Data Reg Offset Address 

#define CC_GPIO_GPAICR_OFFSET						0x0020	// Port A Interrupt Control Reg Offset Address 
#define CC_GPIO_GPBICR_OFFSET						0x0022	// Port B Interrupt Control Reg Offset Address 
#define CC_GPIO_GPCICR_OFFSET						0x0024	// Port C Interrupt Control Reg Offset Address 
#define CC_GPIO_GPDICR_OFFSET						0x0026	// Port D Interrupt Control Reg Offset Address 
#define CC_GPIO_GPEICR_OFFSET						0x0028	// Port E Interrupt Control Reg Offset Address 
	
#define CC_GPIO_GPAISR_OFFSET						0x0040	// Port A Interrupt Status Reg Offset Address 
#define CC_GPIO_GPBISR_OFFSET						0x0042	// Port B Interrupt Status Reg Offset Address 
#define CC_GPIO_GPCISR_OFFSET						0x0044	// Port C Interrupt Status Reg Offset Address 
#define CC_GPIO_GPDISR_OFFSET						0x0046	// Port D Interrupt Status Reg Offset Address 
#define CC_GPIO_GPEISR_OFFSET						0x0048	// Port E Interrupt Status Reg Offset Address 

#define CC_GPIO_REGSIZE								0x0050

#define CC_GPIO_GPACR								(CC_GPIO_REGBASE + CC_GPIO_GPACR_OFFSET)	// Port A Control Reg 
#define CC_GPIO_GPBCR								(CC_GPIO_REGBASE + CC_GPIO_GPBCR_OFFSET)	// Port B Control Reg 
#define CC_GPIO_GPCCR								(CC_GPIO_REGBASE + CC_GPIO_GPCCR_OFFSET)	// Port C Control Reg 
#define CC_GPIO_GPDCR								(CC_GPIO_REGBASE + CC_GPIO_GPDCR_OFFSET)	// Port D Control Reg 
#define CC_GPIO_GPECR								(CC_GPIO_REGBASE + CC_GPIO_GPECR_OFFSET)	// Port E Control Reg 

#define CC_GPIO_GPADR								(CC_GPIO_REGBASE + CC_GPIO_GPADR_OFFSET)	// Port A Data Reg 
#define CC_GPIO_GPBDR								(CC_GPIO_REGBASE + CC_GPIO_GPBDR_OFFSET)	// Port B Data Reg 
#define CC_GPIO_GPCDR								(CC_GPIO_REGBASE + CC_GPIO_GPCDR_OFFSET)	// Port C Data Reg 
#define CC_GPIO_GPDDR								(CC_GPIO_REGBASE + CC_GPIO_GPDDR_OFFSET)	// Port D Data Reg 
#define CC_GPIO_GPEDR								(CC_GPIO_REGBASE + CC_GPIO_GPEDR_OFFSET)	// Port E Data Reg 

#define CC_GPIO_GPAICR								(CC_GPIO_REGBASE + CC_GPIO_GPAICR_OFFSET)	// Port A Interrupt Control Reg 
#define CC_GPIO_GPBICR								(CC_GPIO_REGBASE + CC_GPIO_GPBICR_OFFSET)	// Port B Interrupt Control Reg 
#define CC_GPIO_GPCICR								(CC_GPIO_REGBASE + CC_GPIO_GPCICR_OFFSET)	// Port C Interrupt Control Reg 
#define CC_GPIO_GPDICR								(CC_GPIO_REGBASE + CC_GPIO_GPDICR_OFFSET)	// Port D Interrupt Control Reg 
#define CC_GPIO_GPEICR								(CC_GPIO_REGBASE + CC_GPIO_GPEICR_OFFSET)	// Port E Interrupt Control Reg 
	
#define CC_GPIO_GPAISR								(CC_GPIO_REGBASE + CC_GPIO_GPAISR_OFFSET)	// Port A Interrupt Status Reg 
#define CC_GPIO_GPBISR								(CC_GPIO_REGBASE + CC_GPIO_GPBISR_OFFSET)	// Port B Interrupt Status Reg 
#define CC_GPIO_GPCISR								(CC_GPIO_REGBASE + CC_GPIO_GPCISR_OFFSET)	// Port C Interrupt Status Reg 
#define CC_GPIO_GPDISR								(CC_GPIO_REGBASE + CC_GPIO_GPDISR_OFFSET)	// Port D Interrupt Status Reg 
#define CC_GPIO_GPEISR								(CC_GPIO_REGBASE + CC_GPIO_GPEISR_OFFSET)	// Port E Interrupt Status Reg 

//
// The companion chip interrupt controller
//
#if ENABLE_HD64464!=1
#define CC_INTC_REGBASE_463							(HD64463_BASE + HD64463_INTC_OFFSET)
#endif
#define CC_INTC_REGBASE								(HD64465_BASE + HD64465_INTC_OFFSET)
#define CC_INTC_NIRR_OFFSET							0x0000									// Interrupt Request Register Address Offset
#define CC_INTC_NIMR_OFFSET							0x0002									// Interrupt Mask Register Address Offset

#define CC_INTC_NIRR								(CC_INTC_REGBASE + CC_INTC_NIRR_OFFSET)	// Interrupt Request Register Address Offset
#define CC_INTC_NIMR								(CC_INTC_REGBASE + CC_INTC_NIMR_OFFSET)

// Interrupt Request Register (NIRR)

#define CC_INTC_NIRR_PS2KBR							0x8000	/* PS/2 Keyboard interrupt */ /* HD64465 only 99-08-11 cea */
#define CC_INTC_NIRR_PS2MSR							0x0080	/* PS/2 mouse interrupt */ /* HD64465 only 99-08-11 cea */

#define	CC_INTC_NIRR_PCC0R							0x4000	//	PCC0R	interrupt	
#define	CC_INTC_NIRR_PCC1R							0x2000	//	PCC1R	interrupt	
#define	CC_INTC_NIRR_AFER							0x1000	//	AFER	interrupt	
#define	CC_INTC_NIRR_GPIOR							0x0800	//	GPIOR	interrupt	
#define	CC_INTC_NIRR_TMU0R							0x0400	//	TMU0R	interrupt	
#define	CC_INTC_NIRR_TMU1R							0x0200	//	TMU1R	interrupt	
#define	CC_INTC_NIRR_KBCR							0x0100	//	KBDR	interrupt	
#define	CC_INTC_NIRR_IRDAR							0x0040	//	IRDAR	interrupt	
#define	CC_INTC_NIRR_UART0R							0x0020	//	UART0R	interrupt	
#define	CC_INTC_NIRR_UART1R							0x0010	//	UART1R	interrupt (not available in HD64465)	
#define	CC_INTC_NIRR_PPR							0x0008	//	PPR		interrupt	
#define	CC_INTC_NIRR_SCDIR							0x0004	//	SCDIR	interrupt	
#define	CC_INTC_NIRR_USBR							0x0002	//	USBR	interrupt	
#define	CC_INTC_NIRR_ADCR							0x0001	//	ADCR	interrupt	

// Interrupt Mask Register (NIMR)

#define	CC_INTC_NIMR_ALL_MASK						0xffff	//	all	interrupt	mask

#define CC_INTC_NIMR_PS2KBM_MASK					0x8000	/* PS/2 Keyboard mask */ /* HD64465 only 99-08-11 cea */
#define CC_INTC_NIMR_PS2MSM_MASK					0x0080	/* PS/2 mouse mask */ /* HD64465 only 99-08-11 cea */

#define	CC_INTC_NIMR_PCC0M_MASK						0x4000	//	PCC0M	interrupt	mask
#define	CC_INTC_NIMR_PCC1M_MASK						0x2000	//	PCC1M	interrupt	mask
#define	CC_INTC_NIMR_AFEM_MASK						0x1000	//	AFEM	interrupt	mask
#define	CC_INTC_NIMR_GPIOM_MASK						0x0800	//	GPIOM	interrupt	mask
#define	CC_INTC_NIMR_TMU0M_MASK						0x0400	//	TMU0M	interrupt	mask
#define	CC_INTC_NIMR_TMU1M_MASK						0x0200	//	TMU1M	interrupt	mask
#define	CC_INTC_NIMR_KBCM_MASK						0x0100	//	KBDM	interrupt	mask
#define	CC_INTC_NIMR_IRDAM_MASK						0x0040	//	IRDAM	interrupt	mask
#define	CC_INTC_NIMR_UART0M_MASK					0x0020	//	UART0M	interrupt	mask
#define	CC_INTC_NIMR_UART1M_MASK					0x0010	//	UART1M	interrupt	mask
#define	CC_INTC_NIMR_PPM_MASK						0x0008	//	PPM		interrupt	mask
#define	CC_INTC_NIMR_SCDIM_MASK						0x0004	//	SCDIM	interrupt	mask
#define	CC_INTC_NIMR_USBM_MASK						0x0002	//	USBM	interrupt	mask
#define	CC_INTC_NIMR_ADCM_MASK						0x0001	//	ADCM	interrupt	mask

#define CC_INTC_NIMR_PS2KBM_UNMASK					0x7FFF	/* PS/2 Keyboard UNmask */ /* HD64465 only 99-08-11 cea */
#define CC_INTC_NIMR_PS2MSM_UNMASK					0xFF7F	/* PS/2 mouse UNmask */ /* HD64465 only 99-08-11 cea */

#define	CC_INTC_NIMR_PCC0M_UNMASK					0xbfff	//	PCC0M	interrupt	unmask
#define	CC_INTC_NIMR_PCC1M_UNMASK					0xdfff	//	PCC1M	interrupt	unmask
#define	CC_INTC_NIMR_AFEM_UNMASK					0xefff	//	AFEM	interrupt	unmask
#define	CC_INTC_NIMR_GPIOM_UNMASK					0xf7ff	//	GPIOM	interrupt	unmask
#define	CC_INTC_NIMR_TMU0M_UNMASK					0xfbff	//	TMU0M	interrupt	unmask
#define	CC_INTC_NIMR_TMU1M_UNMASK					0xfdff	//	TMU1M	interrupt	unmask
#define	CC_INTC_NIMR_KBCM_UNMASK					0xfeff	//	KBDM	interrupt	unmask
#define	CC_INTC_NIMR_IRDAM_UNMASK					0xffbf	//	IRDAM	interrupt	unmask
#define	CC_INTC_NIMR_UART0M_UNMASK					0xffdf	//	UART0M	interrupt	unmask
#define	CC_INTC_NIMR_UART1M_UNMASK					0xffef	//	UART1M	interrupt	unmask
#define	CC_INTC_NIMR_PPM_UNMASK						0xfff7	//	PPM		interrupt	unmask
#define	CC_INTC_NIMR_SCDIM_UNMASK					0xfffb	//	SCDIM	interrupt	unmask
#define	CC_INTC_NIMR_USBM_UNMASK					0xfffd	//	USBM	interrupt	unmask
#define	CC_INTC_NIMR_ADCM_UNMASK					0xfffe	//	ADCM	interrupt	unmask

//
// definitions of Timer interface (TMR) on the companion chip
// all the timer registers are 16 bits wide
//
#if ENABLE_HD64464!=1
#define CC_TMR_REGBASE_463							(HD64463_BASE + HD64463_TMR_OFFSET)
#endif
#define CC_TMR_REGBASE								(HD64465_BASE + HD64465_TMR_OFFSET)

#define CC_TMR_TCVR1_OFFSET							0x0000			// Constant 1 Offset Address 
#define CC_TMR_TCVR0_OFFSET							0x0002			// Constant 0  Offset Address
#define	CC_TMR_TRVR1_OFFSET							0x0004			// Read Count 1  Offset Address
#define CC_TMR_TRVR0_OFFSET							0x0006			// Read Count 0 Offset Address 
#define CC_TMR_TCR1_OFFSET							0x0008			// Control 1 Offset Address 
#define CC_TMR_TCR0_OFFSET							0x000A			// Control 0 Offset Address 
#define CC_TMR_TIRR_OFFSET							0x000C			// Interrupt Request Offset Address
#define CC_TMR_TIDR_OFFSET							0x000E			// Interrupt Disable Offset Address

#define CC_TMR_REGSIZE								0x0010			// total size of TMR regs in CC ASIC 

#define CC_TMR_TCVR1								(CC_TMR_REGBASE + CC_TMR_TCVR1_OFFSET)	// Constant 1
#define CC_TMR_TCVR0								(CC_TMR_REGBASE + CC_TMR_TCVR0_OFFSET)	// Constant 0
#define CC_TMR_TRVR1								(CC_TMR_REGBASE + CC_TMR_TRVR1_OFFSET)	// Read Count 1
#define CC_TMR_TRVR0								(CC_TMR_REGBASE + CC_TMR_TRVR0_OFFSET)	// Read Count 0
#define CC_TMR_TCR1									(CC_TMR_REGBASE + CC_TMR_TCR1_OFFSET)		// Control 1
#define CC_TMR_TCR0									(CC_TMR_REGBASE + CC_TMR_TCR0_OFFSET)		// Control 0
#define CC_TMR_TIRR									(CC_TMR_REGBASE + CC_TMR_TIRR_OFFSET)		// Interrupt Request
#define CC_TMR_TIDR									(CC_TMR_REGBASE + CC_TMR_TIDR_OFFSET)		// Interrupt Mask

// Timer 1 Control Register (TCR1)

#define CC_TMR_TCR1_EDMA							0x0010
#define CC_TMR_TCR1_ETMO1							0x0008
#define CC_TMR_TCR1_PST1_CKIO						0x0006
#define CC_TMR_TCR1_PST1_CKIO_DIVID_4				0x0004
#define CC_TMR_TCR1_PST1_CKIO_DIVID_8				0x0002
#define CC_TMR_TCR1_PST1_CKIO_DIVID_16				0x0000
#define CC_TMR_TCR1_T1STP							0x0001

// Timer 0 Control Register (TCR1)

#define CC_TMR_TCR0_EADT							0x0010
#define CC_TMR_TCR0_ETMO0							0x0008
#define CC_TMR_TCR0_PST0_CKIO						0x0006
#define CC_TMR_TCR0_PST0_CKIO_DIVID_4				0x0004
#define CC_TMR_TCR0_PST0_CKIO_DIVID_8				0x0002
#define CC_TMR_TCR0_PST0_CKIO_DIVID_16				0x0000
#define CC_TMR_TCR0_T0STP							0x0001

// Timer Interrupt Request Register (TIRR)

#define CC_TMR_TIRR_TMU1R							0x0002
#define CC_TMR_TIRR_TMU0R							0x0001

// Timer Interrupt Disable Register (TIDR)

#define CC_TMR_TIRR_TMU1D							0x0002
#define CC_TMR_TIRR_TMU0D							0x0001

//
// PCMCIA defines
//
#if ENABLE_HD64464!=1
#define CC_PCMCIA_REGBASE_463						(HD64463_BASE + HD64463_PCMCIA_OFFSET)	
#endif
#define CC_PCMCIA_REGBASE							(HD64465_BASE + HD64465_PCMCIA_OFFSET)

// Card 0 defines 

#define CC_PCMCIA_PCC0ISR_OFFSET					0x0000			// Interface Status Register offset address			
#define CC_PCMCIA_PCC0GCR_OFFSET					0x0002			// General Control Register  offset address
#define CC_PCMCIA_PCC0CSCR_OFFSET					0x0004			// Status Change Register offset address
#define CC_PCMCIA_PCC0CSCIER_OFFSET					0x0006			// Status Change Int. En reg offset address
#define CC_PCMCIA_PCC0SCR_OFFSET					0x0008			// Software Control reg offset address

#define CC_PCMCIA_PCCPSR_OFFSET						0x000a			// Serial Power Switch Control reg offset address

#define CC_PCMCIA_PCC0ISR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0ISR_OFFSET)	// Interface Status Register 			
#define CC_PCMCIA_PCC0GCR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0GCR_OFFSET)	// General Control Register 
#define CC_PCMCIA_PCC0CSCR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0CSCR_OFFSET)	// Status Change Register 
#define CC_PCMCIA_PCC0CSCIER						(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0CSCIER_OFFSET)	// Status Change Int. En reg 
#define CC_PCMCIA_PCC0SCR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0SCR_OFFSET)	// Software Control reg 

#define CC_PCMCIA_PCCPSR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCCPSR_OFFSET)		// Serial Power Switch Control reg 

// Card 1 defines 

#define CC_PCMCIA_PCC1ISR_OFFSET					0x0010			// Interface Status Register 			
#define CC_PCMCIA_PCC1GCR_OFFSET					0x0012			// General Control Register 
#define CC_PCMCIA_PCC1CSCR_OFFSET					0x0014			// Status Change Register 
#define CC_PCMCIA_PCC1CSCIER_OFFSET					0x0016			// Status Change Int. En reg 
#define CC_PCMCIA_PCC1SCR_OFFSET					0x0018			// Software Control reg 

#define CC_PCMCIA_PCC1ISR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1ISR_OFFSET)	// Interface Status Register 			
#define CC_PCMCIA_PCC1GCR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1GCR_OFFSET)	// General Control Register 
#define CC_PCMCIA_PCC1CSCR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1CSCR_OFFSET)	// Status Change Register 
#define CC_PCMCIA_PCC1CSCIER						(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1CSCIER_OFFSET)	// Status Change Int. En reg 
#define CC_PCMCIA_PCC1SCR							(CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1SCR_OFFSET)	// Software Control reg 

#define CC_PCMCIA_REGSIZE							0x0020			// as far as these regs can go 

// Interface Status Reg

#define CC_PCMCIA_ISR_IREQ_MASK						0x80	// mask to obtain value of IREQ pin 
#define CC_PCMCIA_ISR_RDY_MASK						0x80	// mask to obtain value of RDY pin 
#define CC_PCMCIA_ISR_WP_MASK						0x40	// mask to obtain value of WP pin 
#define CC_PCMCIA_ISR_VS_MASK						0x30	// mask to obtain value of VS1 and VS2 
#define CC_PCMCIA_ISR_VS2_MASK						0x20	// mask to obtain value of VS2 
#define CC_PCMCIA_ISR_VS1_MASK						0x10	// mask to obtain value of VS1 
#define CC_PCMCIA_ISR_CD_MASK						0x0C	// mask to obtain value of CD1 and CD2 
#define CC_PCMCIA_ISR_CD2_MASK						0x08	// mask to obtain value of CD2 
#define CC_PCMCIA_ISR_CD1_MASK						0x04	// mask to obtain value of CD1 
#define CC_PCMCIA_ISR_BVD_MASK						0x03	// mask to obtain value of BVD pins 
#define CC_PCMCIA_ISR_BVD_NORMAL					0x03	// battery is fine 
#define CC_PCMCIA_ISR_BVD_LOW_GD					0x01	// battery is low but data is OK 
#define CC_PCMCIA_ISR_BVD_LOW_CD					0x02	// battery is low and data is corrupt 
#define CC_PCMCIA_ISR_BVD_DEAD						0x00	// battery is dead 
#define CC_PCMCIA_ISR_SPKR_MASK						0x02	// mask to obtain value of SPK pin 
#define CC_PCMCIA_ISR_STSCH_MASK					0x01	// mask to obtain value of STSCH pin 

// General Control reg

#define CC_PCMCIA_GCR_DRV_ENABLE					0x80	// Enables the PCMCIA card 
#define CC_PCMCIA_GCR_RESET							0x40	// Causes a reset on the card 
#define CC_PCMCIA_GCR_IO_CARD						0x20	// There is an I/O card in this slot 
#define CC_PCMCIA_GCR_MEM_CARD						0x00	// There is a mem card in this slot 
#define CC_PCMCIA_GCR_5V_ENABLE						0x10	// Spcifies to use 3.3V 
#define CC_PCMCIA_GCR_MMOD_32						0x00	// set mode as 32MB mem areas 
#define CC_PCMCIA_GCR_MMOD_16						0x08	// set mode as 16MB mem areas 
#define CC_PCMCIA_GCR_A25_HIGH						0x04	// set A25 High 
#define CC_PCMCIA_GCR_A25_LOW						0x00	// set A25 Low 
#define CC_PCMCIA_GCR_A24_HIGH						0x02	// set A24 High 
#define CC_PCMCIA_GCR_A24_LOW						0x00	// set A24 Low 
#define CC_PCMCIA_GCR_ADDR_MASK						0x06	// Mask A24 and A25 
#define CC_PCMCIA_GCR_REG_HIGH						0x01	// set REG High 
#define CC_PCMCIA_GCR_REG_LOW						0x00	// set REG Low 

// Card Status Change (PCC0CSCR) Reg

#define CC_PCMCIA_CSCR_GEN_CD_INT					0x80	// Generate a CD interrupt 
#define CC_PCMCIA_CSCR_IREQ_INT_REQ					0x20	// IREQ Int has occurred 
#define CC_PCMCIA_CSCR_STSCH_INT_REQ				0x10	// STSCH Int has occurred 
#define CC_PCMCIA_CSCR_CD_INT_REQ					0x08	// CD Int occurred 
#define CC_PCMCIA_CSCR_RDY_INT_REQ					0x04	// RDY Int occurred 
#define CC_PCMCIA_CSCR_BW_INT_REQ					0x02	// Batt warning int occurred 
#define CC_PCMCIA_CSCR_BE_INT_REQ					0x01	// Batt dead int occurred 
#define CC_PCMCIA_CSCR_TPS2206_SEL					0x40	// TPS2206 serial power switch 
#define CC_PCMCIA_CSCR_MIC2563_SEL					0xbf	// MIC2563 select 

// Card Status Change Interrupt Enable Reg

#define CC_PCMCIA_CSCIER_AUTO_GCR					0x80	// Automatically init the GCR 
#define CC_PCMCIA_CSCIER_IREQ_DIS					0x00	// Disable IREQ ints 
#define CC_PCMCIA_CSCIER_IREQ_LEVEL					0x20	// IREQ Level ints 
#define CC_PCMCIA_CSCIER_IREQ_PFE					0x40	// IREQ Falling Edge, Pulse 
#define CC_PCMCIA_CSCIER_IREQ_PRE					0x60	// IREQ Rising Edge Pulse 
#define CC_PCMCIA_CSCIER_IREQ_MASK					0x60	// For masking this value 
#define CC_PCMCIA_CSCIER_STSCH_INT_EN				0x10	// Enable STSCH Interrupts 
#define CC_PCMCIA_CSCIER_CD_INT_EN					0x08	// Enable CD Interrupts 
#define CC_PCMCIA_CSCIER_RDY_INT_EN					0x04	// Enable RDY/BSY Interrupts 
#define CC_PCMCIA_CSCIER_BWE_INT_EN					0x02	// Enable Batt Warning Interrupts 
#define CC_PCMCIA_CSCIER_BDE_INT_EN					0x01	// Enable Batt Dead Interrupts 

// Software control reg

#define CC_PCMCIA_SCR_3V_ENABLE						0x02	// Specifies to use 5V 
#define CC_PCMCIA_SCR_MASK_VCC						0x0c	// Masks out our bits 
#define CC_PCMCIA_SCR_VCC0VPP0						0x04	// Voltage control pin P0VPP0 
#define CC_PCMCIA_SCR_VCC0VPP1						0x08	// Voltage control pin P0VPP1 
#define CC_PCMCIA_SCR_VCC1VPP0						0x04	// Voltage control pin P1VPP0 
#define CC_PCMCIA_SCR_VCC1VPP1						0x08	// Voltage control pin P1VPP1 
#define CC_PCMCIA_SCR_SHDN_ENB						0x10	// Shutdown bit for TPS2206

// Serial Power Switch Control Register

#define CC_PCMCIA_PSR_3V_ENBB						0x80	// Enable BVcc   5V
#define CC_PCMCIA_PSR_5V_ENBB						0x40	// Enable BVcc 3.3V
#define CC_PCMCIA_PSR_BVPP_VCC						0x20	// Enable BVpp 3.3V or 5V
#define CC_PCMCIA_PSR_BVPP_PGM						0x10	// Enable BVpp  12v
#define CC_PCMCIA_PSR_5V_ENBA						0x08	// Enable AVcc 3.3V
#define CC_PCMCIA_PSR_3V_ENBA						0x04	// Enable AVcc   5V
#define CC_PCMCIA_PSR_AVPP_VCC						0x02	// Enable AVpp 3.3V or 5V
#define CC_PCMCIA_PSR_AVPP_PGM						0x01	// Enable AVpp  12V

#define CC_PCMCIA_PSR_AVCC_MASK						0x0c
#define CC_PCMCIA_PSR_AVPP_MASK						0x03
#define CC_PCMCIA_PSR_BVCC_MASK						0xc0
#define CC_PCMCIA_PSR_BVPP_MASK						0x30

#define CC_PCMCIA_PSR_AVCC_AVPP						(CC_PCMCIA_PSR_AVCC_MASK	| CC_PCMCIA_PSR_AVPP_MASK)
#define CC_PCMCIA_PSR_BVCC_BVPP						(CC_PCMCIA_PSR_BVCC_MASK	| CC_PCMCIA_PSR_BVPP_MASK)

//
// definitions for the FIR on the companion chip
//

#define CC_FIR_REGBASE								(HD64465_BASE + HD64465_FIR_OFFSET)

#define CC_FIR_IRRBR_OFFSET							0x0000	// Receiver Buffer Register offset address
#define CC_FIR_IRTBR_OFFSET							0x0000	// Transmitter Buffer Register offset address
#define CC_FIR_IRIER_OFFSET							0x0002	// Interrupt Enable Register offset address
#define CC_FIR_IRIIR_OFFSET							0x0004	// Interrupt Identification Register offset address
#define CC_FIR_IRFCR_OFFSET							0x0004	// FIFO Control Register offset address
#define CC_FIR_IRLCR_OFFSET							0x0006	// Line Control Register offset address
#define CC_FIR_IRMCR_OFFSET							0x0008	// Modem Control Register offset address
#define CC_FIR_IRDLL_OFFSET							0x0000	// Divisor Latch LSB offset address
#define CC_FIR_IRDLM_OFFSET							0x0002	// Divisor Latch MSB offset address
#define CC_FIR_IRLSR_OFFSET							0x000a	// Line Status Register offset address
#define CC_FIR_IRMSR_OFFSET							0x000c	// Modem Status Register offset address
#define CC_FIR_IRSCR_OFFSET							0x000e	// Scratch Pad Register offset address

#define CC_FIR_IMSTCR_OFFSET						0x0100	// Master Control Register offset address
#define CC_FIR_IMSTSR_OFFSET						0x0102	// Master Status Register offset address
#define CC_FIR_IMISCR_OFFSET						0x0102	// Misc. Control Register offset address
#define CC_FIR_IRFR_OFFSET							0x0104	// Rx FIFO Register offset address
#define CC_FIR_ITFR_OFFSET							0x0104	// Tx FIFO Register offset address
#define CC_FIR_ITC1R_OFFSET							0x0106	// Tx Control 1 Register offset address
#define CC_FIR_ITC2R_OFFSET							0x0108	// Tx Control 2 Register offset address
#define CC_FIR_ITSR_OFFSET							0x010a	// Tx Status Register offset address
#define CC_FIR_IRCR_OFFSET							0x010c	// Rx control Register offset address
#define CC_FIR_IRSR_OFFSET							0x010e	// Rx Status Register offset address
#define CC_FIR_IRSTCR_OFFSET						0x010e	// Reset Command Register offset address

#define CC_FIR_IFAR_OFFSET							0x0102	// Frame Address Register offset address
#define CC_FIR_IRBCLR_OFFSET						0x0104	// Rx Byte Count Low Register offset address
#define CC_FIR_IRBCHR_OFFSET						0x0106	// Rx Byte Count High Register offset address
#define CC_FIR_IRRFPLR_OFFSET						0x0108	// Rx Ring Frame Pointer Low Register offset address
#define CC_FIR_IRRFPHR_OFFSET						0x010a	// Rx Ring Frame Pointer High Register offset address
#define CC_FIR_ITBCLR_OFFSET						0x010c	// Tx Byte Count Low Register offset address
#define CC_FIR_ITBCHR_OFFSET						0x010e	// Tx Byte Count High Register offset address

#define CC_FIR_IIRC1R_OFFSET						0x0102	// Infrared Configuration 1 Register offset address
#define CC_FIR_IIRTCR_OFFSET						0x0104	// Infrared Transceiver Control Register offset address
#define CC_FIR_IIRC2R_OFFSET						0x0106	// Infrared Configuration 2 Register offset address
#define CC_FIR_ITMR_OFFSET							0x0108	// Timer Register offset address
#define CC_FIR_IIRC3R_OFFSET						0x010a	// Infrared Configuration 3 Register offset address

#define CC_FIR_DMARP_OFFSET							0x0110	// DMA Data Read Port offset address
#define CC_FIR_DMAWP_OFFSET							0x0110	// DMA Data Write Port offset address
#define CC_FIR_ISIRR_OFFSET							0x0120	// SIR Register offset address
#define CC_FIR_IFIRCR_OFFSET						0x01e0	// FIR Configuration Register offset address
#define CC_FIR_ITMCR_OFFSET							0x01f0	// Timing Control Register offset address

#define CC_FIR_IRRBR								(CC_FIR_REGBASE + CC_FIR_IRRBR_OFFSET)	//Receiver Buffer Register 
#define CC_FIR_IRTBR								(CC_FIR_REGBASE + CC_FIR_IRTBR_OFFSET)	//Transmitter Buffer Register 
#define CC_FIR_IRIER								(CC_FIR_REGBASE + CC_FIR_IRIER_OFFSET)	//Interrupt Enable Register 
#define CC_FIR_IRIIR								(CC_FIR_REGBASE + CC_FIR_IRIIR_OFFSET)	//Interrupt Identification Register 
#define CC_FIR_IRFCR								(CC_FIR_REGBASE + CC_FIR_IRFCR_OFFSET)	//FIFO Control Register 
#define CC_FIR_IRLCR								(CC_FIR_REGBASE + CC_FIR_IRLCR_OFFSET)	//Line Control Register 
#define CC_FIR_IRMCR								(CC_FIR_REGBASE + CC_FIR_IRMCR_OFFSET)	//Modem Control Register 
#define CC_FIR_IRDLL								(CC_FIR_REGBASE + CC_FIR_IRDLL_OFFSET)	//Divisor Latch LSB 
#define CC_FIR_IRDLM								(CC_FIR_REGBASE + CC_FIR_IRDLM_OFFSET)	//Divisor Latch MSB 
#define CC_FIR_IRLSR								(CC_FIR_REGBASE + CC_FIR_IRLSR_OFFSET)	//Line Status Register 
#define CC_FIR_IRMSR								(CC_FIR_REGBASE + CC_FIR_IRMSR_OFFSET)	//Modem Status Register 
#define CC_FIR_IRSCR								(CC_FIR_REGBASE + CC_FIR_IRSCR_OFFSET)	//Scratch Pad Register 

#define CC_FIR_IMSTCR								(CC_FIR_REGBASE + CC_FIR_IMSTCR_OFFSET)	//Master Control Register 
#define CC_FIR_IMSTSR								(CC_FIR_REGBASE + CC_FIR_IMSTSR_OFFSET)	//Master Status Register 
#define CC_FIR_IMISCR								(CC_FIR_REGBASE + CC_FIR_IMISCR_OFFSET)	//Misc. Control Register 
#define CC_FIR_IRFR									(CC_FIR_REGBASE + CC_FIR_IRFR_OFFSET)	//Rx FIFO Register 
#define CC_FIR_ITFR									(CC_FIR_REGBASE + CC_FIR_ITFR_OFFSET)	//Tx FIFO Register 
#define CC_FIR_ITC1R								(CC_FIR_REGBASE + CC_FIR_ITC1R_OFFSET)	//Tx Control 1 Register 
#define CC_FIR_ITC2R								(CC_FIR_REGBASE + CC_FIR_ITC2R_OFFSET)	//Tx Control 2 Register 
#define CC_FIR_ITSR									(CC_FIR_REGBASE + CC_FIR_ITSR_OFFSET)	//Tx Status Register 
#define CC_FIR_IRCR									(CC_FIR_REGBASE + CC_FIR_IRCR_OFFSET)	//Rx control Register 
#define CC_FIR_IRSR									(CC_FIR_REGBASE + CC_FIR_IRSR_OFFSET)	//Rx Status Register 
#define CC_FIR_IRSTCR								(CC_FIR_REGBASE + CC_FIR_IRSTCR_OFFSET)	//Reset Command Register 

#define CC_FIR_IFAR									(CC_FIR_REGBASE + CC_FIR_IFAR_OFFSET)	//Frame Address Register 
#define CC_FIR_IRBCLR								(CC_FIR_REGBASE + CC_FIR_IRBCLR_OFFSET)	//Rx Byte Count Low Register
#define CC_FIR_IRBCHR								(CC_FIR_REGBASE + CC_FIR_IRBCHR_OFFSET)	//Rx Byte Count High Register 
#define CC_FIR_IRRFPLR								(CC_FIR_REGBASE + CC_FIR_IRRFPLR_OFFSET)//Rx Ring Frame Pointer Low Register 
#define CC_FIR_IRRFPHR								(CC_FIR_REGBASE + CC_FIR_IRRFPHR_OFFSET)//Rx Ring Frame Pointer High Register 
#define CC_FIR_ITBCLR								(CC_FIR_REGBASE + CC_FIR_ITBCLR_OFFSET)	//Tx Byte Count Low Register 
#define CC_FIR_ITBCHR								(CC_FIR_REGBASE + CC_FIR_ITBCHR_OFFSET)	//Tx Byte Count High Register 

#define CC_FIR_IIRC1R								(CC_FIR_REGBASE + CC_FIR_IIRC1R_OFFSET)	//Infrared Configuration 1 Register 
#define CC_FIR_IIRTCR								(CC_FIR_REGBASE + CC_FIR_IIRTCR_OFFSET)	//Infrared Transceiver Control Register 
#define CC_FIR_IIRC2R								(CC_FIR_REGBASE + CC_FIR_IIRC2R_OFFSET)	//Infrared Configuration 2 Register 
#define CC_FIR_ITMR									(CC_FIR_REGBASE + CC_FIR_ITMR_OFFSET)	//Timer Register 
#define CC_FIR_IIRC3R								(CC_FIR_REGBASE + CC_FIR_IIRC3R_OFFSET)	//Infrared Configuration 3 Register 

#define CC_FIR_DMARP								(CC_FIR_REGBASE + CC_FIR_DMARP_OFFSET)	//DMA Data Read Port 
#define CC_FIR_DMAWP								(CC_FIR_REGBASE + CC_FIR_DMAWP_OFFSET)	//DMA Data Write Port 
#define CC_FIR_ISIRR								(CC_FIR_REGBASE + CC_FIR_ISIRR_OFFSET)	//SIR Register 
#define CC_FIR_IFIRCR								(CC_FIR_REGBASE + CC_FIR_IFIRCR_OFFSET)	//FIR Configuration Register 
#define CC_FIR_ITMCR								(CC_FIR_REGBASE + CC_FIR_ITMCR_OFFSET)	//Timing Control Register 

//Master Control Register (IMSTCR)
#define CC_FIR_IMSTCR_IEN							0x80
#define CC_FIR_IMSTCR_TXEN							0x40
#define CC_FIR_IMSTCR_RXEN							0x20
#define CC_FIR_IMSTCR_RST_BANK						0xe0
#define CC_FIR_IMSTCR_BANK0							0x00
#define CC_FIR_IMSTCR_BANK1							0x01
#define CC_FIR_IMSTCR_BANK2							0x02

//Master Status Register (IMSTSR)
#define CC_FIR_IMSTSR_TMI							0x40
#define CC_FIR_IMSTSR_TXI							0x20
#define CC_FIR_IMSTSR_RXI							0x10
#define CC_FIR_IMSTSR_IID_RXSC						0x08
#define CC_FIR_IMSTSR_IID_RXDA						0x0a
#define CC_FIR_IMSTSR_IID_TXBE						0x0c
#define CC_FIR_IMSTSR_IID_TXSC						0x0e

//Miscellaneous Control Register (IMISCR)
#define CC_FIR_IMISCR_DCS_TX						0x80
#define CC_FIR_IMISCR_DCS_RX						0x40
#define CC_FIR_IMISCR_DCS_NONE						0x00
#define CC_FIR_IMISCR_ILOOP							0x10

//Tx Control 1 Register(ITC1R)
#define CC_FIR_ITC1R_RTS							0x80
#define CC_FIR_ITC1R_TFRIEN							0x40
#define CC_FIR_ITC1R_TFUIEN							0x20
#define CC_FIR_ITC1R_TFTL							0x10
#define CC_FIR_ITC1R_ADRTS							0x08
#define CC_FIR_ITC1R_ACEOM							0x04
#define CC_FIR_ITC1R_TIDL							0x02
#define CC_FIR_ITC1R_UA								0x01

//Tx Control 2 Register (ITC2R)
#define CC_FIR_ITC2R_SB								0x80
#define CC_FIR_ITC2R_ACRCG							0x40
#define CC_FIR_ITC2R_SIP_NOW						0x20
#define CC_FIR_ITC2R_SIP_AFTER						0x10
#define CC_FIR_ITC2R_NSFP							0x08
#define CC_FIR_ITC2R_EEIL_EOM						0x00
#define CC_FIR_ITC2R_EEIL_16						0x01
#define CC_FIR_ITC2R_EEIL_32						0x02
#define CC_FIR_ITC2R_EEIL_64						0x03
#define CC_FIR_ITC2R_EEIL_128						0x04
#define CC_FIR_ITC2R_EEIL_256						0x05
#define CC_FIR_ITC2R_EEIL_512						0x06
#define CC_FIR_ITC2R_EEIL_1024						0x07

//Tx Status Register (ITSR)
#define CC_FIR_ITSR_TFUR							0x08
#define CC_FIR_ITSR_EOM								0x04
#define CC_FIR_ITSR_TFRDY							0x02
#define CC_FIR_ITSR_EEOM							0x01

//Rx Control Register (IRCR)
#define CC_FIR_IRCR_RFTL							0x80
#define CC_FIR_IRCR_ACRCC							0x40
#define CC_FIR_IRCR_RADM_ALL						0x00
#define CC_FIR_IRCR_RADM_LOHI_FAR					0x10
#define CC_FIR_IRCR_RADM_HI_FAR						0x20
#define CC_FIR_IRCR_SYNIEN							0x08
#define CC_FIR_IRCR_RFRIEN							0x02
#define CC_FIR_IRCR_SCIEN							0x01

//Rx Status Register (IRSR)
#define CC_FIR_IRSR_ABORT							0x80
#define CC_FIR_IRSR_CRCER							0x40
#define CC_FIR_IRSR_RFOVF							0x20
#define CC_FIR_IRSR_EOF								0x10
#define CC_FIR_IRSR_RFEM							0x08
#define CC_FIR_IRSR_SYNC							0x04

//Reset Command Register (IRSTCR)
#define CC_FIR_IRSTCR_RSTC_HUNT						0x10
#define CC_FIR_IRSTCR_RSTC_RXFIFO					0x20
#define CC_FIR_IRSTCR_RSTC_RXSCI					0x30
#define CC_FIR_IRSTCR_RSTC_RXRFP					0x40
#define CC_FIR_IRSTCR_RSTC_UNDERRUN					0x50
#define CC_FIR_IRSTCR_RSTC_TXFIFO					0x60
#define CC_FIR_IRSTCR_RSTC_HW						0x70

//Infrared Configuration 1 Register (IIRC1R)
#define CC_FIR_IIRC1R_IRSPD_1152					0x00
#define CC_FIR_IIRC1R_IRSPD_0756					0x10
#define CC_FIR_IIRC1R_IRSPD_0288					0x20
#define CC_FIR_IIRC1R_IRMOD_HPSIR					0x00
#define CC_FIR_IIRC1R_IRMOD_ASK						0x01
#define CC_FIR_IIRC1R_IRMOD_MIR						0x02
#define CC_FIR_IIRC1R_IRMOD_FIR						0x04

//Infrared Transceiver Control Register (IIRTCR)
#define CC_FIR_IIRTCR_DFREQ							0x20
#define CC_FIR_IIRTCR_MODSEL						0x10
#define CC_FIR_IIRTCR_ECHO							0x08
#define CC_FIR_IIRTCR_TXDF							0x02

//Infrared Configuration 2 Register (IIRC2R)
#define CC_FIR_IIRC2R_CHOP_DISABLE					0x70
#define CC_FIR_IIRC2R_CHOP_EX187					0x74
#define CC_FIR_IIRC2R_CHOP_EX229					0x78
#define CC_FIR_IIRC2R_CHOP_EX208					0x7c
#define CC_FIR_IIRC2R_CHOP_ENABLE_MAX				0xf0
#define CC_FIR_IIRC2R_CHOP_ENABLE_LESS				0xf4
#define CC_FIR_IIRC2R_CHOP_ENABLE_ZERO_BTB42		0xf8
#define CC_FIR_IIRC2R_CHOP_ENABLE_ZERO				0xfc
#define CC_FIR_IIRC2R_DSIRI							0x02
#define CC_FIR_IIRC2R_DFIRI							0x01

//Infrared Configuration 3 Register (IIRC3R)
#define CC_FIR_IIRC3R_SCDIEN						0x80
#define CC_FIR_IIRC3R_SCD							0x40
#define CC_FIR_IIRC3R_TMIEN							0x02
#define CC_FIR_IIRC3R_TMI							0x01

//SIR Register (ISIRR)
#define CC_FIR_ISIRR_SLOOP							0x02
#define CC_FIR_ISIRR_SIRMOD							0x01

//FIR Configuration Register (IFIRCR)
#define CC_FIR_IFIRCR_RX2_PP						0x04
#define CC_FIR_IFIRCR_RX_PP							0x02
#define CC_FIR_IFIRCR_TMODE							0x01

//Timing Control Register (ITMCR)
#define CC_FIR_ITMCR_TMCR_12						0x00
#define CC_FIR_ITMCR_TMCR_25						0x02
#define CC_FIR_ITMCR_TMCR_30						0x03
#define CC_FIR_ITMCR_TMCR_40						0x04
#define CC_FIR_ITMCR_TMCR_50						0x05
#define CC_FIR_ITMCR_TMCR_66						0x06


//
// definitions for the UART on the companion chip
//

#define CC_UART_REGBASE								(HD64465_BASE + HD64465_UART_OFFSET)

#define CC_UART_UTBR_OFFSET								0x0000
#define CC_UART_URBR_OFFSET								0x0000	// Mirrors UTBR
#define CC_UART_UIER_OFFSET								0x0002
#define CC_UART_UIIR_OFFSET								0x0004
#define CC_UART_UFCR_OFFSET								0x0004	// Mirrors UIIR
#define CC_UART_ULCR_OFFSET								0x0006
#define CC_UART_UMCR_OFFSET								0x0008
#define CC_UART_UDLL_OFFSET								0x0000
#define CC_UART_UDLM_OFFSET								0x0002
#define CC_UART_ULSR_OFFSET								0x000a
#define CC_UART_UMSR_OFFSET								0x000c
#define CC_UART_USCR_OFFSET								0x000e

#define CC_16550_REG_STRIDE								2				// each register is spaced 2 bytes apart

#define CC_UART_UTBR				(CC_UART_REGBASE + CC_UART_UTBR_OFFSET)
#define CC_UART_URBR				(CC_UART_REGBASE + CC_UART_URBR_OFFSET)
#define CC_UART_UIER				(CC_UART_REGBASE + CC_UART_UIER_OFFSET)
#define CC_UART_UIIR				(CC_UART_REGBASE + CC_UART_UIIR_OFFSET)
#define CC_UART_UFCR				(CC_UART_REGBASE + CC_UART_UFCR_OFFSET)
#define CC_UART_ULCR				(CC_UART_REGBASE + CC_UART_ULCR_OFFSET)
#define CC_UART_UMCR				(CC_UART_REGBASE + CC_UART_UMCR_OFFSET)
#define CC_UART_UDLL				(CC_UART_REGBASE + CC_UART_UDLL_OFFSET)
#define CC_UART_UDLM				(CC_UART_REGBASE + CC_UART_UDLM_OFFSET)
#define CC_UART_ULSR				(CC_UART_REGBASE + CC_UART_ULSR_OFFSET)
#define CC_UART_UMSR				(CC_UART_REGBASE + CC_UART_UMSR_OFFSET)
#define CC_UART_USCR				(CC_UART_REGBASE + CC_UART_USCR_OFFSET)

//
// Parallel Interface defines,  HD64465 only 99-08-11 cea
//

#define CC_PAR_BASE_REG 			(HD64465_BASE + HD64465_PARALLEL_OFFSET)
#define CC_PAR_DATA_REG 			CC_PAR_BASE_REG + 0x00
#define CC_PAR_STATUS_REG 			CC_PAR_BASE_REG + 0x02
#define CC_PAR_CONTROL_REG			CC_PAR_BASE_REG + 0x04
#define CC_PAR_ECP_DATA_REG			CC_PAR_BASE_REG + 0x10

#define CC_PAR_EXT_CONTROL_REG		CC_PAR_BASE_REG + 0x14

/* CC_PAR_EXT_CONTROL_REG mode bits
 		Bits 7-5 select mode,
             000 = SPP
             001 = PS/2 Parallel mode
             010 = Parallel Port FIFO mode
             011 = ECP Parallel Port mode
             100 = Reserved
             101 = Reserved
             110 = Test mode
             111 = Configuration mode
		Bit 4 Error Interrupt Enable (nErrIntrEn) 0=enable, 1=disable
		Bit 2 Service Interrupt (ServiceIntr) 0=enable, 1=disable
 */
#define CC_PAR_SPP_MODE				(0x14)
#define CC_PAR_PS2_MODE				(0x34)
#define CC_PAR_ECP_MODE				(0x74)
#define CC_PAR_TEST_MODE			(0xC0)


/* The WinCE documentation for the D9000
   parallel port uses some variation in
   the names of the parallel port bits.

   The D9000 uses a 36 pin Centronics connector
   on the debug interface board. This interface
   seems to designed to look like a printer.

   The S1 uses a 25 pin D-type connector. this
   interface is designed to look like a host. To
   make the S1 to behave like a printer is tricky.

  DB25 (S1)                 Cen-36 (D9000)
  DIR Pin#                  DIR Pin# 
   O  1    CC_PAR_STROBE     I  1   DF_PAR_STROBE
  I/O 2-9  (data bus)       I/O 2-9 (data bus)
   I  10   CC_PAR_NACK       O  10  DF_PAR_NACK
   I  11   CC_PAR_BUSY       O  11  DF_PAR_BUSY
   I  12   CC_PAR_PE         O  12  DF_PAR_ERROR
   I  13   CC_PAR_SELECT     O  13  DF_PAR_SELECT
   O  14   CC_PAR_AUTOFD     I  14  DF_PAR_AUTOFD
   I  15   CC_PAR_ERROR      O  32  DF_PAR_NFAULT
   O  16   CC_PAR_INIT       I  31  DF_PAR_INIT
   O  17   CC_PAR_SELECTIN   I  36  DF_PAR_SELECTIN

           CC_PAR_READDIR       DF_PAR_EN
           CC_PAR_IRQEN         DF_PAR_INTR_MASK

--------------------------------------------------

   The WinCE help data base describes a cable
   to connect the development workstation to
   a WinCE target. This is the cable used to 
   connect to the HD64465 parallel port.

 */

// CC_PAR_STATUS_REG bits
#define CC_PAR_BUSY					0x80
#define CC_PAR_NACK					0x40
#define CC_PAR_PE					0x20
#define CC_PAR_SELECT				0x10
#define CC_PAR_ERROR				0x08

// CC_PAR_CONTROL_REG bits
// 		CC_PAR_READDIR,0 = output, 1 = input
#define CC_PAR_READDIR				0x20
#define CC_PAR_IRQEN				0x10
#define CC_PAR_SELECTIN				0x08
#define CC_PAR_INIT					0x04
#define CC_PAR_AUTOFD				0x02
#define CC_PAR_STROBE				0x01

// CC_PAR_EXT_CONTROL_REG bits
#define CC_PAR_NERRINTRE			0x10
#define CC_PAR_SERVICEINTR			0x04
#define CC_PAR_OUT_FULL				0x02
#define CC_PAR_IN_EMPTY				0x01

//
// Serial CODEC Interface defines
//
#if ENABLE_HD64464!=1
#define CC_CODEC_REGBASE_463						(HD64463_BASE + HD64463_CODEC_OFFSET)
#endif
#define CC_CODEC_REGBASE							(HD64465_BASE + HD64465_CODEC_OFFSET)
#define CC_CODEC_REGSIZE							0x0070

#define CC_CODEC_TDR_OFFSET							0x0000
#define CC_CODEC_RDR_OFFSET							0x0004
#define CC_CODEC_CR_OFFSET							0x0008
#define CC_CODEC_SR_OFFSET							0x000C
#define CC_CODEC_FSR_OFFSET							0x0010
#define CC_CODEC_CAR_OFFSET							0x0020
#define CC_CODEC_CDR_OFFSET							0x0024
#define CC_CODEC_PCML_OFFSET						0x0028
#define CC_CODEC_PCMR_OFFSET						0x002C
#define CC_CODEC_LINE1_OFFSET						0x0030
#define CC_CODEC_PCMC_OFFSET						0x0034
#define CC_CODEC_PCMLS_OFFSET						0x0038
#define CC_CODEC_PCMRS_OFFSET						0x003C
#define CC_CODEC_PCMLFE_OFFSET						0x0040
#define CC_CODEC_LINE2_OFFSET						0x0044
#define CC_CODEC_HSET_OFFSET						0x0048
#define CC_CODEC_IOCS_OFFSET						0x004C
#define CC_CODEC_ATIER_OFFSET						0x0050
#define CC_CODEC_ATSR_OFFSET						0x0054
#define CC_CODEC_ARIER_OFFSET						0x0058
#define CC_CODEC_ARSR_OFFSET						0x005C
#define CC_CODEC_ACR_OFFSET							0x0060
#define CC_CODEC_ATAGR_OFFSET						0x0064
#define CC_CODEC_SRAR_OFFSET						0x0068

#define CC_CODEC_TDR								(CC_CODEC_REGBASE + CC_CODEC_TDR_OFFSET)
#define CC_CODEC_RDR								(CC_CODEC_REGBASE + CC_CODEC_RDR_OFFSET)
#define CC_CODEC_CR									(CC_CODEC_REGBASE + CC_CODEC_CR_OFFSET)
#define CC_CODEC_SR									(CC_CODEC_REGBASE + CC_CODEC_SR_OFFSET)
#define CC_CODEC_FSR								(CC_CODEC_REGBASE + CC_CODEC_FSR_OFFSET)
#define CC_CODEC_CAR								(CC_CODEC_REGBASE + CC_CODEC_CAR_OFFSET)
#define CC_CODEC_CDR								(CC_CODEC_REGBASE + CC_CODEC_CDR_OFFSET)
#define CC_CODEC_PCML								(CC_CODEC_REGBASE + CC_CODEC_PCML_OFFSET)
#define CC_CODEC_PCMR								(CC_CODEC_REGBASE + CC_CODEC_PCMR_OFFSET)
#define CC_CODEC_LINE1								(CC_CODEC_REGBASE + CC_CODEC_LINE1_OFFSET)
#define CC_CODEC_PCMC								(CC_CODEC_REGBASE + CC_CODEC_PCMC_OFFSET)
#define CC_CODEC_PCMLS								(CC_CODEC_REGBASE + CC_CODEC_PCMLS_OFFSET)
#define CC_CODEC_PCMRS								(CC_CODEC_REGBASE + CC_CODEC_PCMRS_OFFSET)
#define CC_CODEC_PCMLFE								(CC_CODEC_REGBASE + CC_CODEC_PCMLFE_OFFSET)
#define CC_CODEC_LINE2								(CC_CODEC_REGBASE + CC_CODEC_LINE2_OFFSET)
#define CC_CODEC_HSET								(CC_CODEC_REGBASE + CC_CODEC_HSET_OFFSET)
#define CC_CODEC_IOCS								(CC_CODEC_REGBASE + CC_CODEC_IOCS_OFFSET)
#define CC_CODEC_ATIER								(CC_CODEC_REGBASE + CC_CODEC_ATIER_OFFSET)
#define CC_CODEC_ATSR								(CC_CODEC_REGBASE + CC_CODEC_ATSR_OFFSET)
#define CC_CODEC_ARIER								(CC_CODEC_REGBASE + CC_CODEC_ARIER_OFFSET)
#define CC_CODEC_ARSR								(CC_CODEC_REGBASE + CC_CODEC_ARSR_OFFSET)
#define CC_CODEC_ACR								(CC_CODEC_REGBASE + CC_CODEC_ACR_OFFSET)
#define CC_CODEC_ATAGR								(CC_CODEC_REGBASE + CC_CODEC_ATAGR_OFFSET)
#define CC_CODEC_SRAR								(CC_CODEC_REGBASE + CC_CODEC_SRAR_OFFSET)

// Control Register (CR)

#define CC_CODEC_CR_DMAEN							0x2000
#define CC_CODEC_CR_SL18							0x1000
#define CC_CODEC_CR_CDRT							0x0800
#define CC_CODEC_CR_WMRT							0x0400
#define CC_CODEC_CR_AC97S							0x0200
#define CC_CODEC_CR_SWR								0x0100
#define CC_CODEC_CR_PU								0x0080
#define CC_CODEC_CR_MS								0x0040
#define CC_CODEC_CR_ST								0x0020
#define CC_CODEC_CR_CRE								0x0010
#define CC_CODEC_CR_FTF								0x0008
#define CC_CODEC_CR_TXEN							0x0004
#define CC_CODEC_CR_FRF								0x0002
#define CC_CODEC_CR_RXEN							0x0001

// Status Register (SR)

#define CC_CODEC_SR_IR71							0x4000
#define CC_CODEC_SR_TNF								0x2000

#define CC_CODEC_SR_TFS_E1E0						0x0000		// (FIFO-1, FIFO-0):(EMPTY,		EMPTY)
#define CC_CODEC_SR_TFS_E1N0						0x0800		// (FIFO-1, FIFO-0):(EMPTY,		NOT EMPTY)
#define CC_CODEC_SR_TFS_N1E0						0x1000		// (FIFO-1, FIFO-0):(NOT EMPTY,	EMPTY)
#define CC_CODEC_SR_TFS_N1N0						0x1800		// (FIFO-1, FIFO-0):(NOT EMPTY,	NOT EMPTY)

#define CC_CODEC_SR_TFU								0x0400
#define CC_CODEC_SR_TFO								0x0200
#define CC_CODEC_SR_TDI								0x0100
#define CC_CODEC_SR_RNE								0x0020

#define CC_CODEC_SR_RFS_N1N0						0x0000		// (FIFO-1, FIFO-0):(NOT FULL,	NOT FULL)
#define CC_CODEC_SR_RFS_N1F0						0x0008		// (FIFO-1, FIFO-0):(NOT FULL,	FULL)
#define CC_CODEC_SR_RFS_F1N0						0x0010		// (FIFO-1, FIFO-0):(FULL,		NOT FULL)
#define CC_CODEC_SR_RFS_F1F0						0x0018		// (FIFO-1, FIFO-0):(FULL,		FULL)

#define CC_CODEC_SR_RFU								0x0004
#define CC_CODEC_SR_RFO								0x0002
#define CC_CODEC_SR_RDI								0x0001

// Frequency Select Register (FSR)

#define CC_CODEC_FSR_FS_8000HZ						0x0000
#define CC_CODEC_FSR_FS_9600HZ						0x0001
#define CC_CODEC_FSR_FS_12000HZ						0x0002
#define CC_CODEC_FSR_FS_16000HZ						0x0003
#define CC_CODEC_FSR_FS_24000HZ						0x0004
#define CC_CODEC_FSR_FS_48000HZ						0x0005

// Command/Status Address Register (CAR/CSAR)

#define CC_CODEC_CAR_RW								0x00080000

// AC97 Transmit Interrupt Enable Register (ATIER)

#define CC_CODEC_ATIER_PLTFRQIE						0x20000000
#define CC_CODEC_ATIER_PRTFRQIE						0x10000000
#define CC_CODEC_ATIER_L1TFRQIE						0x08000000
#define CC_CODEC_ATIER_PCTFRQIE						0x04000000
#define CC_CODEC_ATIER_PLSTFRQIE					0x02000000
#define CC_CODEC_ATIER_PRSTFRQIE					0x01000000
#define CC_CODEC_ATIER_PLFETFRQIE					0x00800000
#define CC_CODEC_ATIER_L2TFRQIE						0x00400000
#define CC_CODEC_ATIER_HTTFRQIE						0x00200000
#define CC_CODEC_ATIER_IOCTFRQIE					0x00100000

#define CC_CODEC_ATIER_PLTFOVIE						0x00080000
#define CC_CODEC_ATIER_PRTFOVIE						0x00040000
#define CC_CODEC_ATIER_L1TFOVIE						0x00020000
#define CC_CODEC_ATIER_PCTFOVIE						0x00010000
#define CC_CODEC_ATIER_PLSTFOVIE					0x00008000
#define CC_CODEC_ATIER_PRSTFOVIE					0x00004000
#define CC_CODEC_ATIER_PLFETFOVIE					0x00002000
#define CC_CODEC_ATIER_L2TFOVIE						0x00001000
#define CC_CODEC_ATIER_HTTFOVIE						0x00000800
#define CC_CODEC_ATIER_IOCTFOVIE					0x00000400

#define CC_CODEC_ATIER_PLTFUNIE						0x00000200
#define CC_CODEC_ATIER_PRTFUNIE						0x00000100
#define CC_CODEC_ATIER_L1TFUNIE						0x00000080
#define CC_CODEC_ATIER_PCTFUNIE						0x00000040
#define CC_CODEC_ATIER_PLSTFUNIE					0x00000020
#define CC_CODEC_ATIER_PRSTFUNIE					0x00000010
#define CC_CODEC_ATIER_PLFETFUNIE					0x00000008
#define CC_CODEC_ATIER_L2TFUNIE						0x00000004
#define CC_CODEC_ATIER_HTTFUNIE						0x00000002
#define CC_CODEC_ATIER_IOCTFUNIE					0x00000001

// AC97 TX FIFO Status Register (ATSR)

#define CC_CODEC_ATSR_PLTFRQ						0x20000000
#define CC_CODEC_ATSR_PRTFRQ						0x10000000
#define CC_CODEC_ATSR_L1TFRQ						0x08000000
#define CC_CODEC_ATSR_PCTFRQ						0x04000000
#define CC_CODEC_ATSR_PLSTFRQ						0x02000000
#define CC_CODEC_ATSR_PRSTFRQ						0x01000000
#define CC_CODEC_ATSR_PLFETFRQ						0x00800000
#define CC_CODEC_ATSR_L2TFRQ						0x00400000
#define CC_CODEC_ATSR_HTTFRQ						0x00200000
#define CC_CODEC_ATSR_IOCTFRQ						0x00100000

#define CC_CODEC_ATSR_PLTFOV						0x00080000
#define CC_CODEC_ATSR_PRTFOV						0x00040000
#define CC_CODEC_ATSR_L1TFOV						0x00020000
#define CC_CODEC_ATSR_PCTFOV						0x00010000
#define CC_CODEC_ATSR_PLSTFOV						0x00008000
#define CC_CODEC_ATSR_PRSTFOV						0x00004000
#define CC_CODEC_ATSR_PLFETFOV						0x00002000
#define CC_CODEC_ATSR_L2TFOV						0x00001000
#define CC_CODEC_ATSR_HTTFOV						0x00000800
#define CC_CODEC_ATSR_IOCTFOV						0x00000400

#define CC_CODEC_ATSR_PLTFUN						0x00000200
#define CC_CODEC_ATSR_PRTFUN						0x00000100
#define CC_CODEC_ATSR_L1TFUN						0x00000080
#define CC_CODEC_ATSR_PCTFUN						0x00000040
#define CC_CODEC_ATSR_PLSTFUN						0x00000020
#define CC_CODEC_ATSR_PRSTFUN						0x00000010
#define CC_CODEC_ATSR_PLFETFUN						0x00000008
#define CC_CODEC_ATSR_L2TFUN						0x00000004
#define CC_CODEC_ATSR_HTTFUN						0x00000002
#define CC_CODEC_ATSR_IOCTFUN						0x00000001

// AC97 RX FIFO Interrupt Enable Register (ARIER)

#define CC_CODEC_ARIER_STARYIE						0x00400000
#define CC_CODEC_ARIER_STDRYIE						0x00200000

#define CC_CODEC_ARIER_PLRFRQIE						0x00100000
#define CC_CODEC_ARIER_PRRFRQIE						0x00080000
#define CC_CODEC_ARIER_L1RFRQIE						0x00040000
#define CC_CODEC_ARIER_MICRFRQIE					0x00020000
#define CC_CODEC_ARIER_L2RFRQIE						0x00010000
#define CC_CODEC_ARIER_HTRFRQIE						0x00008000
#define CC_CODEC_ARIER_IOCSRFRQIE					0x00004000

#define CC_CODEC_ARIER_PLRFOVIE						0x00002000
#define CC_CODEC_ARIER_PRRFOVIE						0x00001000
#define CC_CODEC_ARIER_L1RFOVIE						0x00000800
#define CC_CODEC_ARIER_MICRFOVIE					0x00000400
#define CC_CODEC_ARIER_L2RFOVIE						0x00000200
#define CC_CODEC_ARIER_HTRFOVIE						0x00000100
#define CC_CODEC_ARIER_IOCSRFOVIE					0x00000080

#define CC_CODEC_ARIER_PLRFUNIE						0x00000040
#define CC_CODEC_ARIER_PRRFUNIE						0x00000020
#define CC_CODEC_ARIER_L1RFUNIE						0x00000010
#define CC_CODEC_ARIER_MICRFUNIE					0x00000008
#define CC_CODEC_ARIER_L2RFUNIE						0x00000004
#define CC_CODEC_ARIER_HTRFUNIE						0x00000002
#define CC_CODEC_ARIER_IOCSRFUNIE					0x00000001

// AC97 RX FIFO Status Register (ARSR)

#define CC_CODEC_ARSR_STARY							0x00400000
#define CC_CODEC_ARSR_STDRY							0x00200000

#define CC_CODEC_ARSR_PLRFRQ						0x00100000
#define CC_CODEC_ARSR_PRRFRQ						0x00080000
#define CC_CODEC_ARSR_L1RFRQ						0x00040000
#define CC_CODEC_ARSR_MICRFRQ						0x00020000
#define CC_CODEC_ARSR_L2RFRQ						0x00010000
#define CC_CODEC_ARSR_HTRFRQ						0x00008000
#define CC_CODEC_ARSR_IOCSRFRQ						0x00004000

#define CC_CODEC_ARSR_PLRFOV						0x00002000
#define CC_CODEC_ARSR_PRRFOV						0x00001000
#define CC_CODEC_ARSR_L1RFOV						0x00000800
#define CC_CODEC_ARSR_MICRFOV						0x00000400
#define CC_CODEC_ARSR_L2RFOV						0x00000200
#define CC_CODEC_ARSR_HTRFOV						0x00000100
#define CC_CODEC_ARSR_IOCSRFOV						0x00000080

#define CC_CODEC_ARSR_PLRFUN						0x00000040
#define CC_CODEC_ARSR_PRRFUN						0x00000020
#define CC_CODEC_ARSR_L1RFUN						0x00000010
#define CC_CODEC_ARSR_MICRFUN						0x00000008
#define CC_CODEC_ARSR_L2RFUN						0x00000004
#define CC_CODEC_ARSR_HTRFUN						0x00000002
#define CC_CODEC_ARSR_IOCSRFUN						0x00000001

// AC97 Control Register (ACR)

#define CC_CODEC_ACR_VS								0x80000000
#define CC_CODEC_ACR_RXDMA_EN						0x00400000
#define CC_CODEC_ACR_TXDMA_EN						0x00200000
#define CC_CODEC_ACR_FCAF							0x00100000
#define CC_CODEC_ACR_FCDF							0x00080000
#define CC_CODEC_ACR_FSTAF							0x00040000
#define CC_CODEC_ACR_FSTDF							0x00020000
#define CC_CODEC_ACR_FPLTF							0x00010000
#define CC_CODEC_ACR_FPRTF							0x00008000
#define CC_CODEC_ACR_FL1TF							0x00004000
#define CC_CODEC_ACR_FPCTF							0x00002000
#define CC_CODEC_ACR_FPLSTF							0x00001000
#define CC_CODEC_ACR_FPRSTF							0x00000800
#define CC_CODEC_ACR_FPLETF							0x00000400
#define CC_CODEC_ACR_FL2TF							0x00000200
#define CC_CODEC_ACR_FHTF							0x00000100
#define CC_CODEC_ACR_FIOCTF							0x00000080
#define CC_CODEC_ACR_FPLRF							0x00000040
#define CC_CODEC_ACR_FPRRF							0x00000020
#define CC_CODEC_ACR_FL1RF							0x00000010
#define CC_CODEC_ACR_FMRF							0x00000008
#define CC_CODEC_ACR_FL2RF							0x00000004
#define CC_CODEC_ACR_FHRF							0x00000002
#define CC_CODEC_ACR_FIOSRF							0x00000001

//AC97 TAG Register (ATAGR)

#define CC_CODEC_ATAGR_CR							0x80000000

//Slot Request Active Register (SRAR)

#define CC_CODEC_SRAR_SL12RA						0x1000
#define CC_CODEC_SRAR_SL11RA						0x0800
#define CC_CODEC_SRAR_SL10RA						0x0400
#define CC_CODEC_SRAR_SL9RA							0x0200
#define CC_CODEC_SRAR_SL8RA							0x0100
#define CC_CODEC_SRAR_SL7RA							0x0080
#define CC_CODEC_SRAR_SL6RA							0x0040
#define CC_CODEC_SRAR_SL5RA							0x0020
#define CC_CODEC_SRAR_SL4RA							0x0010
#define CC_CODEC_SRAR_SL3RA							0x0008

//
// definitions for the Audio Front End on the companion chip (all the registers are 16 bits wide)
//
#if ENABLE_HD64464!=1
#define CC_AFE_REGBASE_463							(HD64463_BASE + HD64463_AFE_OFFSET)
#endif
#define CC_AFE_REGBASE								(HD64465_BASE + HD64465_AFE_OFFSET)

#define CC_AFE_RXDB0_OFFSET							0x0000		// Receive buffer 0 
#define CC_AFE_RXDB1_OFFSET							0x0000		// Receive buffer 1 (same offset!) 
#define CC_AFE_TXDB0_OFFSET							0x0100		// Transmit buffer 0 
#define CC_AFE_TXDB1_OFFSET							0x0100		// Transmit buffer 1 (same offset!) 
#define CC_AFE_CTR_OFFSET							0x0200		// Control register 
#define CC_AFE_STR_OFFSET							0x0202		// Status register 
#define CC_AFE_RXDR_OFFSET							0x0204		// Receive data register 
#define CC_AFE_TXDR_OFFSET							0x0206		// Transmit data register 

#define CC_AFE_REGSIZE								0x0208		// total size of AFE registers 

#define CC_AFE_RXDB0								(CC_AFE_REGBASE + CC_AFE_RXDB0_OFFSET)	// Receive buffer 0 
#define CC_AFE_RXDB1								(CC_AFE_REGBASE + CC_AFE_RXDB1_OFFSET)	// Receive buffer 1 (same offset!) 
#define CC_AFE_TXDB0								(CC_AFE_REGBASE + CC_AFE_TXDB0_OFFSET)	// Transmit buffer 0 
#define CC_AFE_TXDB1								(CC_AFE_REGBASE + CC_AFE_TXDB1_OFFSET)	// Transmit buffer 1 (same offset!) 
#define CC_AFE_CTR									(CC_AFE_REGBASE + CC_AFE_CTR_OFFSET)		// Control register 
#define CC_AFE_STR									(CC_AFE_REGBASE + CC_AFE_STR_OFFSET)		// Status register 
#define CC_AFE_RXDR									(CC_AFE_REGBASE + CC_AFE_RXDR_OFFSET)		// Receive data register 
#define CC_AFE_TXDR									(CC_AFE_REGBASE + CC_AFE_TXDR_OFFSET)		// Transmit data register 

//
// definitions of DISPLAY interface (DSP) HD64463 BASE
//
#if (ENABLE_HD64464!=1 && (ENABLE_MQ200 !=1))
#define CC_DSP_REGBASE								(HD64463_BASE + HD64463_LCDC_OFFSET)
#define CC_DSP_LCDCBAR_OFFSET						0x0000		// Base Address Register Offset Address 
#define CC_DSP_LCDCLOR_OFFSET						0x0002		// Line address offset register  Offset Address
#define	CC_DSP_LCDCCR_OFFSET						0x0004		// Control Register  Offset Address
#define CC_DSP_LDR1_OFFSET							0x0010		// LCD Display regsiter 1  Offset Address
#define CC_DSP_LDR2_OFFSET							0x0012		// LCD Display register 2  Offset Address
#define CC_DSP_LDHNCR_OFFSET						0x0014		// Num Chars in Horz. reg  Offset Address
#define CC_DSP_LDHNSR_OFFSET						0x0016		// Start Position of Horz. reg  Offset Address
#define CC_DSP_LDVNTR_OFFSET						0x0018		// Total Vertical Lines reg  Offset Address
#define CC_DSP_LDVNDR_OFFSET						0x001A		// Display Vertical Lines reg  Offset Address
#define CC_DSP_LDVSPR_OFFSET						0x001C		// Vertical Synchronous Pos. Reg  Offset Address
#define CC_DSP_LDR3_OFFSET							0x001E		// LCD Display register 3  Offset Address
#define CC_DSP_CRTVTR_OFFSET						0x0020		// CRT Vertical Total Register  Offset Address
#define CC_DSP_CRTVRSR_OFFSET						0x0022		// CRT Vertical Retrace Start Register  Offset Address
#define CC_DSP_CRTVRER_OFFSET						0x0024		// CRT Vertical Retrace End Register  Offset Address
#define CC_DSP_CPTWAR_OFFSET						0x0030		// Color Palette Write Addr Reg  Offset Address
#define CC_DSP_CPTWDR_OFFSET						0x0032		// Color Palette Write Data Reg  Offset Address
#define CC_DSP_CPTRAR_OFFSET						0x0034		// Color Palette Read Address Reg  Offset Address
#define CC_DSP_CPTRDR_OFFSET						0x0036		// Color Palette Read data reg  Offset Address

#define CC_DSP_REGSIZE								0x0038		// total size of DSP ASIC regs 

#define CC_DSP_LCDCBAR								(CC_DSP_REGBASE + CC_DSP_LCDCBAR_OFFSET)	// Base Address Register 
#define CC_DSP_LCDCLOR								(CC_DSP_REGBASE + CC_DSP_LCDCLOR_OFFSET)	// Line address offset register 
#define	CC_DSP_LCDCCR								(CC_DSP_REGBASE + CC_DSP_LCDCCR_OFFSET)	// Control Register 
#define CC_DSP_LDR1									(CC_DSP_REGBASE + CC_DSP_LDR1_OFFSET)		// LCD Display regsiter 1 
#define CC_DSP_LDR2									(CC_DSP_REGBASE + CC_DSP_LDR2_OFFSET)		// LCD Display register 2 
#define CC_DSP_LDHNCR								(CC_DSP_REGBASE + CC_DSP_LDHNCR_OFFSET)	// Num Chars in Horz. reg 
#define CC_DSP_LDHNSR								(CC_DSP_REGBASE + CC_DSP_LDHNSR_OFFSET)	// Start Position of Horz. reg 
#define CC_DSP_LDVNTR								(CC_DSP_REGBASE + CC_DSP_LDVNTR_OFFSET)	// Total Vertical Lines reg 
#define CC_DSP_LDVNDR								(CC_DSP_REGBASE + CC_DSP_LDVNDR_OFFSET)	// Display Vertical Lines reg 
#define CC_DSP_LDVSPR								(CC_DSP_REGBASE + CC_DSP_LDVSPR_OFFSET)	// Vertical Synchronous Pos. Reg 
#define CC_DSP_LDR3									(CC_DSP_REGBASE + CC_DSP_LDR3_OFFSET)		// LCD Display register 3 
#define CC_DSP_CRTVTR								(CC_DSP_REGBASE + CC_DSP_CRTVTR_OFFSET)	// CRT Vertical Total Register 
#define CC_DSP_CRTVRSR								(CC_DSP_REGBASE + CC_DSP_CRTVRSR_OFFSET)	// CRT Vertical Retrace Start Register 
#define CC_DSP_CRTVRER								(CC_DSP_REGBASE + CC_DSP_CRTVRER_OFFSET)	// CRT Vertical Retrace End Register 
#define CC_DSP_CPTWAR								(CC_DSP_REGBASE + CC_DSP_CPTWAR_OFFSET)	// Color Palette Write Addr Reg 
#define CC_DSP_CPTWDR								(CC_DSP_REGBASE + CC_DSP_CPTWDR_OFFSET)	// Color Palette Write Data Reg 
#define CC_DSP_CPTRAR								(CC_DSP_REGBASE + CC_DSP_CPTRAR_OFFSET)	// Color Palette Read Address Reg 
#define CC_DSP_CPTRDR								(CC_DSP_REGBASE + CC_DSP_CPTRDR_OFFSET)	// Color Palette Read data reg 
#endif

// Companion Chip (HD64464) module offsets
#if ENABLE_HD64464==1
#define HD64464_OFFSET								0x03E00000
#define CC_DSP_REGBASE								(HD64464_BASE + HD64464_OFFSET)

#define	CC_DSP_PM_OFFSET							0x00000000
#define	CC_DSP_CC_OFFSET							0x00002000
#define	CC_DSP_MM_OFFSET							0x00004000
#define	CC_DSP_IN_OFFSET							0x00014000
#define	CC_DSP_GC_OFFSET							0x0001E000
#define	CC_DSP_GE_OFFSET							0x00020000
#define	CC_DSP_FP_OFFSET							0x00022000
#define	CC_DSP_CP1_OFFSET							0x00024000
#define	CC_DSP_CP2_OFFSET							0x00026000
#define	CC_DSP_CR_OFFSET							0x0002A000
#define CC_DSP_REGSIZE								0x00030000

#define	CC_DSP_PM									(CC_DSP_REGBASE + CC_DSP_PM_OFFSET)	// Power Management + Clock Generation
#define	CC_DSP_CC									(CC_DSP_REGBASE + CC_DSP_CC_OFFSET)	// CPU Interface
#define	CC_DSP_MM									(CC_DSP_REGBASE + CC_DSP_MM_OFFSET)	// Memory Interface
#define	CC_DSP_IN									(CC_DSP_REGBASE + CC_DSP_IN_OFFSET)	// Interrupt Controller
#define	CC_DSP_GC									(CC_DSP_REGBASE + CC_DSP_GC_OFFSET)	// Graphics Controller 1 and 2
#define	CC_DSP_GE									(CC_DSP_REGBASE + CC_DSP_GE_OFFSET)	// Graphics Engine
#define	CC_DSP_FP									(CC_DSP_REGBASE + CC_DSP_FP_OFFSET)	// Flat Panel Controller
#define	CC_DSP_CP1									(CC_DSP_REGBASE + CC_DSP_CP1_OFFSET)// Color Palette 1
#define	CC_DSP_CP2									(CC_DSP_REGBASE + CC_DSP_CP2_OFFSET)// Color Palette 2
#define	CC_DSP_CR									(CC_DSP_REGBASE + CC_DSP_CR_OFFSET)	// Configuration Registers

#endif

// Companion Chip (MQ200) module offsets
#if ENABLE_MQ200==1
#define MQ200_OFFSET								0x01E00000
#define CC_DSP_REGBASE								(MQ200_BASE + MQ200_OFFSET)

#define	CC_DSP_PM_OFFSET							0x00000000
#define	CC_DSP_CC_OFFSET							0x00002000
#define	CC_DSP_MM_OFFSET							0x00004000
#define	CC_DSP_IN_OFFSET							0x00008000
#define	CC_DSP_GC_OFFSET							0x0000A000
#define	CC_DSP_GE_OFFSET							0x0000C000
#define	CC_DSP_FP_OFFSET							0x0000E000
#define	CC_DSP_CP1_OFFSET							0x00010400//We have to chwck color palette address!
#define	CC_DSP_DC_OFFSET							0x00014000
#define	CC_DSP_PCI_OFFSET							0x00016000
#define	CC_DSP_PSF_OFFSET							0x00018000
#define CC_DSP_REGSIZE								0x0001A000

#define	CC_DSP_PM									(CC_DSP_REGBASE + CC_DSP_PM_OFFSET)	// Power Management + Clock Generation
#define	CC_DSP_CC									(CC_DSP_REGBASE + CC_DSP_CC_OFFSET)	// CPU Interface
#define	CC_DSP_MM									(CC_DSP_REGBASE + CC_DSP_MM_OFFSET)	// Memory Interface
#define	CC_DSP_IN									(CC_DSP_REGBASE + CC_DSP_IN_OFFSET)	// Interrupt Controller
#define	CC_DSP_GC									(CC_DSP_REGBASE + CC_DSP_GC_OFFSET)	// Graphics Controller 1 and 2
#define	CC_DSP_GE									(CC_DSP_REGBASE + CC_DSP_GE_OFFSET)	// Graphics Engine
#define	CC_DSP_FP									(CC_DSP_REGBASE + CC_DSP_FP_OFFSET)	// Flat Panel Controller
#define	CC_DSP_CP1									(CC_DSP_REGBASE + CC_DSP_CP1_OFFSET)// Color Palette 1
#define	CC_DSP_DC									(CC_DSP_REGBASE + CC_DSP_DC_OFFSET)	// Configuration Registers
#define	CC_DSP_PCI									(CC_DSP_REGBASE + CC_DSP_PCI_OFFSET)// PCI Configuration Registers
#define	CC_DSP_PSF									(CC_DSP_REGBASE + CC_DSP_PSF_OFFSET)// PCI Configuration Registers

#endif	//ENABLE_MQ200

#if ENABLE_HD64464!=1
#define CC_KBC_REGBASE_463							(HD64463_BASE + HD64463_KBC_OFFSET)
#endif

#define CC_KBC_REGBASE								(HD64465_BASE + HD64465_KBC_OFFSET)

#define CC_KBC_CR_OFFSET			0x800
#define CC_KBC_SR_OFFSET			0x802

#define CC_KBC_CR					(CC_KBC_REGBASE + CC_KBC_CR_OFFSET)
#define CC_KBC_SR					(CC_KBC_REGBASE + CC_KBC_SR_OFFSET)

#endif
