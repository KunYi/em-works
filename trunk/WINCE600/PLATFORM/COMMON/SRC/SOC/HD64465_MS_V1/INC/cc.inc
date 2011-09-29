;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
;
;
;  Copyright(c) 1998,1999 Renesas Technology Corp.
;
;	Module Name:
;
;		cc.inc
;
;	Revision History:
;
;		26th April 1999		Released
;		25th May   1999		Added FIR definitions
;		14th June  1999		Added HD64464's definitions
;		28th June  1999		Fixed minor bug
;


; Companion Chip (HD64465) module offsets

HD64465_STB_SYSTEM_OFFSET				.equ	h'0000		; Power down modes & system configuration
HD64465_RESERVED						.equ	h'1000		; Reserved
HD64465_PCMCIA_OFFSET					.equ	h'2000		; PCMCIA
HD64465_AFE_OFFSET						.equ	h'3000		; Modem analog front end
HD64465_GPIO_OFFSET						.equ	h'4000		; I/O ports
HD64465_INTC_OFFSET						.equ	h'5000		; Interrupt Controller
HD64465_TMR_OFFSET						.equ	h'6000		; Timer
HD64465_FIR_OFFSET						.equ	h'7000		; IrDA/16550(A) UART
HD64465_UART_OFFSET						.equ	h'8000		; 16550(B) UART
HD64465_EMBEDED_SDRAM_OFFSET			.equ	h'9000		; Embeded SDRAM
HD64465_PARALLEL_OFFSET					.equ	h'A000		; Parallel Port
HD64465_USB_OFFSET						.equ	h'B000		; USB Host Controller
HD64465_CODEC_OFFSET					.equ	h'C000		; Serial CODEC Interface
HD64465_KBC_OFFSET						.equ	h'D000		; Keyboard Controller Interface
HD64465_ADC_OFFSET						.equ	h'E000		; A/D Converter


; Companion Chip (HD64463) module offsets
	.aif ENABLE_HD64464 eq h'00
HD64463_STB_SYSTEM_OFFSET				.equ	h'0000		; Power down modes & system configuration 
HD64463_LCDC_OFFSET						.equ	h'1000		; LCD/CRT 
HD64463_PCMCIA_OFFSET					.equ	h'2000		; PCMCIA 
HD64463_AFE_OFFSET						.equ	h'3000		; Modem analog front end 
HD64463_GPIO_OFFSET						.equ	h'4000		; I/O ports 	
HD64463_INTC_OFFSET						.equ	h'5000		; Interrupt Controller 
HD64463_TMR_OFFSET						.equ	h'6000		; Timer 
HD64463_FIR_OFFSET						.equ	h'7000		; IrDA/16550(A) UART 
HD64463_UART0_OFFSET					.equ	h'8000		; 16550(B) UART  
HD64463_UART1_OFFSET					.equ	h'9000		; Embeded SDRAM 
HD64463_PARALLEL_OFFSET					.equ	h'A000		; Parallel Port 
HD64463_USB_OFFSET						.equ	h'B000		; USB Host Controller 
HD64463_CODEC_OFFSET					.equ	h'C000		; Serial CODEC Interface 
HD64463_KBC_OFFSET						.equ	h'D000		; Keyboard Controller Interface 
HD64463_ADC_OFFSET						.equ	h'E000		; A/D Converter 
	.aendi

;
; definitions of System control and power interface (SYS) on the companion chip ( all the system registers are 16 bits wide )
;

	.aif ENABLE_HD64464 eq h'00
CC_SYS_REGBASE_463						.equ	HD64463_BASE + HD64463_STB_SYSTEM_OFFSET
	.aendi
CC_SYS_REGBASE							.equ	HD64465_BASE + HD64465_STB_SYSTEM_OFFSET

CC_SYS_SMSCR_OFFSET						.equ	h'0000
CC_SYS_SCONFR_OFFSET					.equ	h'0002
CC_SYS_SBCR_OFFSET						.equ	h'0004
CC_SYS_SPCCR_OFFSET						.equ	h'0006
CC_SYS_SPSRCR_OFFSET					.equ	h'0008
CC_SYS_SPLLCR_OFFSET					.equ	h'000a
CC_SYS_SRR_OFFSET						.equ	h'000c
CC_SYS_STMCR_OFFSET						.equ	h'000e
CC_SYS_SDID_OFFSET						.equ	h'0010
CC_SYS_SDPR_OFFSET						.equ	h'0ff0

CC_SYS_REGSIZE							.equ	h'0ff2			; total size of SYSTEM regs in CC ASIC

CC_SYS_SMSCR							.equ	CC_SYS_REGBASE + CC_SYS_SMSCR_OFFSET
CC_SYS_SCONFR							.equ	CC_SYS_REGBASE + CC_SYS_SCONFR_OFFSET
CC_SYS_SBCR								.equ	CC_SYS_REGBASE + CC_SYS_SBCR_OFFSET
CC_SYS_SPCCR							.equ	CC_SYS_REGBASE + CC_SYS_SPCCR_OFFSET
CC_SYS_SPSRCR							.equ	CC_SYS_REGBASE + CC_SYS_SPSRCR_OFFSET
CC_SYS_SPLLCR							.equ	CC_SYS_REGBASE + CC_SYS_SPLLCR_OFFSET
CC_SYS_SRR								.equ	CC_SYS_REGBASE + CC_SYS_SRR_OFFSET
CC_SYS_STMCR							.equ	CC_SYS_REGBASE + CC_SYS_STMCR_OFFSET
CC_SYS_SDID								.equ	CC_SYS_REGBASE + CC_SYS_SDID_OFFSET
CC_SYS_SDPR								.equ	CC_SYS_REGBASE + CC_SYS_SDPR_OFFSET

; System Module Standby Control Register (SMSCR)

CC_SYS_SMSCR_LCDST						.equ	h'2000			; HD64463 only
CC_SYS_SMSCR_UART0ST					.equ	h'0800			; HD64463 only
CC_SYS_SMSCR_UART1ST					.equ	h'0400			; HD64463 only
CC_SYS_SMSCR_USBST						.equ	h'0080			; HD64463 only

CC_SYS_SMSCR_PS2ST						.equ	h'4000			; HD64465 only 99-08-11 cea

CC_SYS_SMSCR_ADCST						.equ	h'1000
CC_SYS_SMSCR_UARTST						.equ	h'0800
CC_SYS_SMSCR_SCDIST						.equ	h'0200
CC_SYS_SMSCR_PPST						.equ	h'0100
CC_SYS_SMSCR_PC0ST						.equ	h'0040
CC_SYS_SMSCR_PC1ST						.equ	h'0020
CC_SYS_SMSCR_AFEST						.equ	h'0010
CC_SYS_SMSCR_TM0ST						.equ	h'0008
CC_SYS_SMSCR_TM1ST						.equ	h'0004
CC_SYS_SMSCR_IRDAST						.equ	h'0002
CC_SYS_SMSCR_KBCST						.equ	h'0001

; System Configuration Register (SCONFR)

CC_SYS_SCONFR_SLS						.equ	h'2000			; HD64463 only
CC_SYS_SCONFR_LCDCD_DIVID_3				.equ	h'0080			; HD64463 only
CC_SYS_SCONFR_LCDCD_DIVID_2				.equ	h'0040			; HD64463 only
CC_SYS_SCONFR_LCDCD_DIVID_1				.equ	h'0000			; HD64463 only
CC_SYS_SCONFR_ILCDMS					.equ	h'0020			; HD64463 only

CC_SYS_SCONFR_HWEN						.equ	h'1000
CC_SYS_SCONFR_HW1						.equ	h'0100
CC_SYS_SCONFR_HW2						.equ	h'0200
CC_SYS_SCONFR_HW3						.equ	h'0300
CC_SYS_SCONFR_HW4						.equ	h'0400
CC_SYS_SCONFR_HW5						.equ	h'0500
CC_SYS_SCONFR_HW6						.equ	h'0600
CC_SYS_SCONFR_HW7						.equ	h'0700
CC_SYS_SCONFR_HW8						.equ	h'0800
CC_SYS_SCONFR_HW9						.equ	h'0900
CC_SYS_SCONFR_HW10						.equ	h'0a00
CC_SYS_SCONFR_HW11						.equ	h'0b00
CC_SYS_SCONFR_HW12						.equ	h'0c00
CC_SYS_SCONFR_HW13						.equ	h'0d00
CC_SYS_SCONFR_HW14						.equ	h'0e00
CC_SYS_SCONFR_HW15						.equ	h'0f00

CC_SYS_SCONFR_USBCKS					.equ	h'0020
CC_SYS_SCONFR_SCDICKS					.equ	h'0010

CC_SYS_SCONFR_PPFMS_ECP_EPP				.equ	h'000b
CC_SYS_SCONFR_PPFMS_ECP					.equ	h'0008
CC_SYS_SCONFR_PPFMS_EPP					.equ	h'0004
CC_SYS_SCONFR_PPFMS_SPP					.equ	h'0000

CC_SYS_SCONFR_KBWUP						.equ	h'0002

; System Bus Control Register (SBCR)

CC_SYS_SBCR_LCDIG						.equ	h'0080			; HD64463 only

CC_SYS_SBCR_PDOF						.equ	h'8000			; HD64465 only
CC_SYS_SBCR_PDIG						.equ	h'4000			; HD64465 only
CC_SYS_SBCR_PCOF						.equ	h'2000			; HD64465 only
CC_SYS_SBCR_PCIG						.equ	h'1000			; HD64465 only
CC_SYS_SBCR_PBOF						.equ	h'0800
CC_SYS_SBCR_PBIG						.equ	h'0400
CC_SYS_SBCR_PAOF						.equ	h'0200
CC_SYS_SBCR_PAIG						.equ	h'0100
CC_SYS_SBCR_CSPE						.equ	h'0040
CC_SYS_SBCR_CMDPE						.equ	h'0020
CC_SYS_SBCR_ADDRPE						.equ	h'0010
CC_SYS_SBCR_DATAPE						.equ	h'0008
CC_SYS_SBCR_CPUBIG						.equ	h'0004
CC_SYS_SBCR_PEOF						.equ	h'0002			; HD64465 only
CC_SYS_SBCR_PEIG						.equ	h'0001			; HD64465 only

; SYSTEM Peripheral Clock Control Register

CC_SYS_SPCCR_URT1CLK					.equ	h'4000			; HD64463 only
CC_SYS_SPCCR_URT0CLK					.equ	h'2000			; HD64463 only
CC_SYS_SPCCR_LCKOSC						.equ	h'0004			; HD64463 only

CC_SYS_SPCCR_ADCCLK						.equ	h'8000
CC_SYS_SPCCR_UARTCLK					.equ	h'2000
CC_SYS_SPCCR_PPCLK						.equ	h'1000
CC_SYS_SPCCR_FIRCLK						.equ	h'0800
CC_SYS_SPCCR_SIRCLK						.equ	h'0400
CC_SYS_SPCCR_SCDICLK					.equ	h'0200
CC_SYS_SPCCR_KBCCLK						.equ	h'0100
CC_SYS_SPCCR_USBCLK						.equ	h'0080
CC_SYS_SPCCR_AFECLK						.equ	h'0040
CC_SYS_SPCCR_UCKOSC						.equ	h'0002
CC_SYS_SPCCR_AFEOSC						.equ	h'0001

; System Peripheral S/W Reset Control Register (SPSRCR)

CC_SYS_SPSRCR_LCDCSRT					.equ	h'2000			; HD64463 only
CC_SYS_SPSRCR_UR0SRT					.equ	h'0800			; HD64463 only
CC_SYS_SPSRCR_UR1SRT					.equ	h'0400			; HD64463 only

CC_SYS_SPSRCR_SPORST					.equ	h'8000
CC_SYS_SPSRCR_ADCSRT					.equ	h'1000
CC_SYS_SPSRCR_UARTSRT					.equ	h'0800
CC_SYS_SPSRCR_SCDISRT					.equ	h'0200
CC_SYS_SPSRCR_PPSRT						.equ	h'0100
CC_SYS_SPSRCR_USBSRT					.equ	h'0080
CC_SYS_SPSRCR_PC0SRT					.equ	h'0040
CC_SYS_SPSRCR_PC1RST					.equ	h'0020
CC_SYS_SPSRCR_AFERST					.equ	h'0010
CC_SYS_SPSRCR_TM0RST					.equ	h'0008
CC_SYS_SPSRCR_TM1RST					.equ	h'0004
CC_SYS_SPSRCR_IRDARST					.equ	h'0002
CC_SYS_SPSRCR_KBCRST					.equ	h'0001

; System PLL Control Register (SPLLCR)

CC_SYS_SPLLCR_PLL2SB					.equ	h'0020
CC_SYS_SPLLCR_PLL1SB					.equ	h'0010
CC_SYS_SPLLCR_PLL2BP					.equ	h'0002
CC_SYS_SPLLCR_PLL1BP					.equ	h'0001

; System Test Mode Control Register (STMCR)

CC_SYS_STMCR_DITST						.equ	h'0400
CC_SYS_STMCR_DOTST						.equ	h'0200
CC_SYS_STMCR_AFETST						.equ	h'0100
CC_SYS_STMCR_PCITST						.equ	h'0080
CC_SYS_STMCR_SDBST						.equ	h'0040
CC_SYS_STMCR_USBST						.equ	h'0020
CC_SYS_STMCR_PLL2TST					.equ	h'0010
CC_SYS_STMCR_PLL1TST					.equ	h'0008
CC_SYS_STMCR_URTTST						.equ	h'0004
CC_SYS_STMCR_ACTST						.equ	h'0002
CC_SYS_STMCR_DCTST						.equ	h'0001

;
; GPIO regs
;

	.aif ENABLE_HD64464 eq h'00
CC_GPIO_REGBASE_463						.equ	HD64463_BASE + HD64463_GPIO_OFFSET
	.aendi
CC_GPIO_REGBASE							.equ	HD64465_BASE + HD64465_GPIO_OFFSET

CC_GPIO_GPACR_OFFSET					.equ	h'0000	; Port A Control Reg Offset Address 
CC_GPIO_GPBCR_OFFSET					.equ	h'0002	; Port B Control Reg Offset Address 
CC_GPIO_GPCCR_OFFSET					.equ	h'0004	; Port C Control Reg Offset Address 
CC_GPIO_GPDCR_OFFSET					.equ	h'0006	; Port D Control Reg Offset Address 
CC_GPIO_GPECR_OFFSET					.equ	h'0008	; Port E Control Reg Offset Address 

CC_GPIO_GPADR_OFFSET					.equ	h'0010	; Port A Data Reg Offset Address 
CC_GPIO_GPBDR_OFFSET					.equ	h'0012	; Port B Data Reg Offset Address 
CC_GPIO_GPCDR_OFFSET					.equ	h'0014	; Port C Data Reg Offset Address 
CC_GPIO_GPDDR_OFFSET					.equ	h'0016	; Port D Data Reg Offset Address 
CC_GPIO_GPEDR_OFFSET					.equ	h'0018	; Port E Data Reg Offset Address 

CC_GPIO_GPAICR_OFFSET					.equ	h'0020	; Port A Interrupt Control Reg Offset Address 
CC_GPIO_GPBICR_OFFSET					.equ	h'0022	; Port B Interrupt Control Reg Offset Address 
CC_GPIO_GPCICR_OFFSET					.equ	h'0024	; Port C Interrupt Control Reg Offset Address 
CC_GPIO_GPDICR_OFFSET					.equ	h'0026	; Port D Interrupt Control Reg Offset Address 
CC_GPIO_GPEICR_OFFSET					.equ	h'0028	; Port E Interrupt Control Reg Offset Address 
	
CC_GPIO_GPAISR_OFFSET					.equ	h'0040	; Port A Interrupt Status Reg Offset Address 
CC_GPIO_GPBISR_OFFSET					.equ	h'0042	; Port B Interrupt Status Reg Offset Address 
CC_GPIO_GPCISR_OFFSET					.equ	h'0044	; Port C Interrupt Status Reg Offset Address 
CC_GPIO_GPDISR_OFFSET					.equ	h'0046	; Port D Interrupt Status Reg Offset Address 
CC_GPIO_GPEISR_OFFSET					.equ	h'0048	; Port E Interrupt Status Reg Offset Address 

CC_GPIO_REGSIZE							.equ	h'0050

CC_GPIO_GPACR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPACR_OFFSET	; Port A Control Reg 
CC_GPIO_GPBCR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPBCR_OFFSET	; Port B Control Reg 
CC_GPIO_GPCCR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPCCR_OFFSET	; Port C Control Reg 
CC_GPIO_GPDCR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPDCR_OFFSET	; Port D Control Reg 
CC_GPIO_GPECR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPECR_OFFSET	; Port E Control Reg 

CC_GPIO_GPADR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPADR_OFFSET	; Port A Data Reg 
CC_GPIO_GPBDR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPBDR_OFFSET	; Port B Data Reg 
CC_GPIO_GPCDR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPCDR_OFFSET	; Port C Data Reg 
CC_GPIO_GPDDR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPDDR_OFFSET	; Port D Data Reg 
CC_GPIO_GPEDR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPEDR_OFFSET	; Port E Data Reg 

CC_GPIO_GPAICR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPAICR_OFFSET	; Port A Interrupt Control Reg 
CC_GPIO_GPBICR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPBICR_OFFSET	; Port B Interrupt Control Reg 
CC_GPIO_GPCICR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPCICR_OFFSET	; Port C Interrupt Control Reg 
CC_GPIO_GPDICR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPDICR_OFFSET	; Port D Interrupt Control Reg 
CC_GPIO_GPEICR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPEICR_OFFSET	; Port E Interrupt Control Reg 
	
CC_GPIO_GPAISR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPAISR_OFFSET	; Port A Interrupt Status Reg 
CC_GPIO_GPBISR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPBISR_OFFSET	; Port B Interrupt Status Reg 
CC_GPIO_GPCISR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPCISR_OFFSET	; Port C Interrupt Status Reg 
CC_GPIO_GPDISR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPDISR_OFFSET	; Port D Interrupt Status Reg 
CC_GPIO_GPEISR							.equ	CC_GPIO_REGBASE + CC_GPIO_GPEISR_OFFSET	; Port E Interrupt Status Reg 

;
; The companion chip interrupt controller
;

	.aif ENABLE_HD64464 eq h'00
CC_INTC_REGBASE_463						.equ	HD64463_BASE + HD64463_INTC_OFFSET
	.aendi
CC_INTC_REGBASE							.equ	HD64465_BASE + HD64465_INTC_OFFSET
CC_INTC_NIRR_OFFSET						.equ	h'0000									; Interrupt Request Register Address Offset
CC_INTC_NIMR_OFFSET						.equ	h'0002									; Interrupt Mask Register Address Offset

CC_INTC_NIRR							.equ	CC_INTC_REGBASE + CC_INTC_NIRR_OFFSET	; Interrupt Request Register Address Offset
CC_INTC_NIMR							.equ	CC_INTC_REGBASE + CC_INTC_NIMR_OFFSET

; Interrupt Request Register (NIRR)

CC_INTC_NIRR_PS2KBR		.equ	0x8000		; PS/2 Keyboard interrupt HD64465 only 99-08-11 cea
CC_INTC_NIRR_PS2MSR		.equ	0x0080		; PS/2 mouse interrupt    HD64465 only 99-08-11 cea

CC_INTC_NIRR_PCC0R						.equ	h'4000	;	PCC0R	interrupt	
CC_INTC_NIRR_PCC1R						.equ	h'2000	;	PCC1R	interrupt	
CC_INTC_NIRR_AFER						.equ	h'1000	;	AFER	interrupt	
CC_INTC_NIRR_GPIOR						.equ	h'0800	;	GPIOR	interrupt	
CC_INTC_NIRR_TMU0R						.equ	h'0400	;	TMU0R	interrupt	
CC_INTC_NIRR_TMU1R						.equ	h'0200	;	TMU1R	interrupt	
CC_INTC_NIRR_KBCR						.equ	h'0100	;	KBDR	interrupt	
CC_INTC_NIRR_IRDAR						.equ	h'0040	;	IRDAR	interrupt	
CC_INTC_NIRR_UART0R						.equ	h'0020	;	UART0R	interrupt	
CC_INTC_NIRR_UART1R						.equ	h'0010	;	UART1R	interrupt	
CC_INTC_NIRR_PPR						.equ	h'0008	;	PPR		interrupt	
CC_INTC_NIRR_SCDIR						.equ	h'0004	;	SCDIR	interrupt	
CC_INTC_NIRR_USBR						.equ	h'0002	;	USBR	interrupt	
CC_INTC_NIRR_ADCR						.equ	h'0001	;	ADCR	interrupt	

; Interrupt Mask Register (NIMR)

CC_INTC_NIMR_ALL_MASK					.equ	h'ffff	;	all	interrupt	mask

CC_INTC_NIMR_PS2KBM_MASK	.equ	0x8000		; PS/2 Keyboard mask HD64465 only 99-08-11 cea
CC_INTC_NIMR_PS2MSM_MASK	.equ	0x0080		; PS/2 mouse mask    HD64465 only 99-08-11 cea

CC_INTC_NIMR_PCC0M_MASK					.equ	h'4000	;	PCC0M	interrupt	mask
CC_INTC_NIMR_PCC1M_MASK					.equ	h'2000	;	PCC1M	interrupt	mask
CC_INTC_NIMR_AFEM_MASK					.equ	h'1000	;	AFEM	interrupt	mask
CC_INTC_NIMR_GPIOM_MASK					.equ	h'0800	;	GPIOM	interrupt	mask
CC_INTC_NIMR_TMU0M_MASK					.equ	h'0400	;	TMU0M	interrupt	mask
CC_INTC_NIMR_TMU1M_MASK					.equ	h'0200	;	TMU1M	interrupt	mask
CC_INTC_NIMR_KBCM_MASK					.equ	h'0100	;	KBDM	interrupt	mask
CC_INTC_NIMR_IRDAM_MASK					.equ	h'0040	;	IRDAM	interrupt	mask
CC_INTC_NIMR_UART0M_MASK				.equ	h'0020	;	UART0M	interrupt	mask
CC_INTC_NIMR_UART1M_MASK				.equ	h'0010	;	UART1M	interrupt	mask
CC_INTC_NIMR_PPM_MASK					.equ	h'0008	;	PPM		interrupt	mask
CC_INTC_NIMR_SCDIM_MASK					.equ	h'0004	;	SCDIM	interrupt	mask
CC_INTC_NIMR_USBM_MASK					.equ	h'0002	;	USBM	interrupt	mask
CC_INTC_NIMR_ADCM_MASK					.equ	h'0001	;	ADCM	interrupt	mask

CC_INTC_NIMR_PS2KBM_UNMASK	.equ	0x7FFF		; PS/2 Keyboard UNmask HD64465 only 99-08-11 cea
CC_INTC_NIMR_PS2MSM_UNMASK	.equ	0xFF7F		; PS/2 mouse UNmask    HD64465 only 99-08-11 cea

CC_INTC_NIMR_PCC0M_UNMASK				.equ	h'bfff	;	PCC0M	interrupt	unmask
CC_INTC_NIMR_PCC1M_UNMASK				.equ	h'dfff	;	PCC1M	interrupt	unmask
CC_INTC_NIMR_AFEM_UNMASK				.equ	h'efff	;	AFEM	interrupt	unmask
CC_INTC_NIMR_GPIOM_UNMASK				.equ	h'f7ff	;	GPIOM	interrupt	unmask
CC_INTC_NIMR_TMU0M_UNMASK				.equ	h'fbff	;	TMU0M	interrupt	unmask
CC_INTC_NIMR_TMU1M_UNMASK				.equ	h'fdff	;	TMU1M	interrupt	unmask
CC_INTC_NIMR_KBCM_UNMASK				.equ	h'feff	;	KBDM	interrupt	unmask
CC_INTC_NIMR_IRDAM_UNMASK				.equ	h'ffbf	;	IRDAM	interrupt	unmask
CC_INTC_NIMR_UART0M_UNMASK				.equ	h'ffdf	;	UART0M	interrupt	unmask
CC_INTC_NIMR_UART1M_UNMASK				.equ	h'ffef	;	UART1M	interrupt	unmask
CC_INTC_NIMR_PPM_UNMASK					.equ	h'fff7	;	PPM		interrupt	unmask
CC_INTC_NIMR_SCDIM_UNMASK				.equ	h'fffb	;	SCDIM	interrupt	unmask
CC_INTC_NIMR_USBM_UNMASK				.equ	h'fffd	;	USBM	interrupt	unmask
CC_INTC_NIMR_ADCM_UNMASK				.equ	h'fffe	;	ADCM	interrupt	unmask

;
; definitions of Timer interface (TMR) on the companion chip
; all the timer registers are 16 bits wide
;

	.aif ENABLE_HD64464 eq h'00
CC_TMR_REGBASE_463						.equ	HD64463_BASE + HD64463_TMR_OFFSET
	.aendi
CC_TMR_REGBASE							.equ	HD64465_BASE + HD64465_TMR_OFFSET

CC_TMR_TCVR1_OFFSET						.equ	h'0000			; Constant 1 Offset Address 
CC_TMR_TCVR0_OFFSET						.equ	h'0002			; Constant 0  Offset Address
CC_TMR_TRVR1_OFFSET						.equ	h'0004			; Read Count 1  Offset Address
CC_TMR_TRVR0_OFFSET						.equ	h'0006			; Read Count 0 Offset Address 
CC_TMR_TCR1_OFFSET						.equ	h'0008			; Control 1 Offset Address 
CC_TMR_TCR0_OFFSET						.equ	h'000A			; Control 0 Offset Address 
CC_TMR_TIRR_OFFSET						.equ	h'000C			; Interrupt Request Offset Address
CC_TMR_TIDR_OFFSET						.equ	h'000E			; Interrupt Disable Offset Address

CC_TMR_REGSIZE							.equ	h'0010			; total size of TMR regs in CC ASIC 

CC_TMR_TCVR1							.equ	CC_TMR_REGBASE + CC_TMR_TCVR1_OFFSET	; Constant 1
CC_TMR_TCVR0							.equ	CC_TMR_REGBASE + CC_TMR_TCVR0_OFFSET	; Constant 0
CC_TMR_TRVR1							.equ	CC_TMR_REGBASE + CC_TMR_TRVR1_OFFSET	; Read Count 1
CC_TMR_TRVR0							.equ	CC_TMR_REGBASE + CC_TMR_TRVR0_OFFSET	; Read Count 0
CC_TMR_TCR1								.equ	CC_TMR_REGBASE + CC_TMR_TCR1_OFFSET		; Control 1
CC_TMR_TCR0								.equ	CC_TMR_REGBASE + CC_TMR_TCR0_OFFSET		; Control 0
CC_TMR_TIRR								.equ	CC_TMR_REGBASE + CC_TMR_TIRR_OFFSET		; Interrupt Request
CC_TMR_TIDR								.equ	CC_TMR_REGBASE + CC_TMR_TIDR_OFFSET		; Interrupt Mask

; Timer 1 Control Register (TCR1)

CC_TMR_TCR1_EDMA						.equ	h'0010
CC_TMR_TCR1_ETMO1						.equ	h'0008
CC_TMR_TCR1_PST1_CKIO					.equ	h'0006
CC_TMR_TCR1_PST1_CKIO_DIVID_4			.equ	h'0004
CC_TMR_TCR1_PST1_CKIO_DIVID_8			.equ	h'0002
CC_TMR_TCR1_PST1_CKIO_DIVID_16			.equ	h'0000
CC_TMR_TCR1_T1STP						.equ	h'0001

; Timer 0 Control Register (TCR1)

CC_TMR_TCR0_EADT						.equ	h'0010
CC_TMR_TCR0_ETMO0						.equ	h'0008
CC_TMR_TCR0_PST0_CKIO					.equ	h'0006
CC_TMR_TCR0_PST0_CKIO_DIVID_4			.equ	h'0004
CC_TMR_TCR0_PST0_CKIO_DIVID_8			.equ	h'0002
CC_TMR_TCR0_PST0_CKIO_DIVID_16			.equ	h'0000
CC_TMR_TCR0_T0STP						.equ	h'0001

; Timer Interrupt Request Register (TIRR)

CC_TMR_TIRR_TMU1R						.equ	h'0002
CC_TMR_TIRR_TMU0R						.equ	h'0001

; Timer Interrupt Disable Register (TIDR)

CC_TMR_TIRR_TMU1D						.equ	h'0002
CC_TMR_TIRR_TMU0D						.equ	h'0001

;
; PCMCIA defines
;

	.aif ENABLE_HD64464 eq h'00
CC_PCMCIA_REGBASE_463					.equ	HD64463_BASE + HD64463_PCMCIA_OFFSET	
	.aendi
CC_PCMCIA_REGBASE						.equ	HD64465_BASE + HD64465_PCMCIA_OFFSET	

; Card 0 defines 

CC_PCMCIA_PCC0ISR_OFFSET				.equ	h'0000			; Interface Status Register offset address			
CC_PCMCIA_PCC0GCR_OFFSET				.equ	h'0002			; General Control Register  offset address
CC_PCMCIA_PCC0CSCR_OFFSET				.equ	h'0004			; Status Change Register offset address
CC_PCMCIA_PCC0CSCIER_OFFSET				.equ	h'0006			; Status Change Int. En reg offset address
CC_PCMCIA_PCC0SCR_OFFSET				.equ	h'0008			; Software Control reg offset address

CC_PCMCIA_PCCPSR_OFFSET					.equ	h'000a			; Serial Power Switch Control reg offset address

CC_PCMCIA_PCC0ISR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0ISR_OFFSET	; Interface Status Register 			
CC_PCMCIA_PCC0GCR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0GCR_OFFSET	; General Control Register 
CC_PCMCIA_PCC0CSCR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0CSCR_OFFSET	; Status Change Register 
CC_PCMCIA_PCC0CSCIER					.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0CSCIER_OFFSET	; Status Change Int. En reg 
CC_PCMCIA_PCC0SCR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC0SCR_OFFSET	; Software Control reg 

CC_PCMCIA_PCCPSR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCCPSR_OFFSET		; Serial Power Switch Control reg 

; Card 1 defines 

CC_PCMCIA_PCC1ISR_OFFSET				.equ	h'0010			; Interface Status Register 			
CC_PCMCIA_PCC1GCR_OFFSET				.equ	h'0012			; General Control Register 
CC_PCMCIA_PCC1CSCR_OFFSET				.equ	h'0014			; Status Change Register 
CC_PCMCIA_PCC1CSCIER_OFFSET				.equ	h'0016			; Status Change Int. En reg 
CC_PCMCIA_PCC1SCR_OFFSET				.equ	h'0018			; Software Control reg 

CC_PCMCIA_PCC1ISR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1ISR_OFFSET	; Interface Status Register 			
CC_PCMCIA_PCC1GCR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1GCR_OFFSET	; General Control Register 
CC_PCMCIA_PCC1CSCR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1CSCR_OFFSET	; Status Change Register 
CC_PCMCIA_PCC1CSCIER					.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1CSCIER_OFFSET	; Status Change Int. En reg 
CC_PCMCIA_PCC1SCR						.equ	CC_PCMCIA_REGBASE + CC_PCMCIA_PCC1SCR_OFFSET	; Software Control reg 

CC_PCMCIA_REGSIZE						.equ	h'0020			; as far as these regs can go 

; Interface Status Reg

CC_PCMCIA_ISR_IREQ_MASK					.equ	h'80	; mask to obtain value of IREQ pin 
CC_PCMCIA_ISR_RDY_MASK					.equ	h'80	; mask to obtain value of RDY pin 
CC_PCMCIA_ISR_WP_MASK					.equ	h'40	; mask to obtain value of WP pin 
CC_PCMCIA_ISR_VS_MASK					.equ	h'30	; mask to obtain value of VS1 and VS2 
CC_PCMCIA_ISR_VS2_MASK					.equ	h'20	; mask to obtain value of VS2 
CC_PCMCIA_ISR_VS1_MASK					.equ	h'10	; mask to obtain value of VS1 
CC_PCMCIA_ISR_CD_MASK					.equ	h'0C	; mask to obtain value of CD1 and CD2 
CC_PCMCIA_ISR_CD2_MASK					.equ	h'08	; mask to obtain value of CD2 
CC_PCMCIA_ISR_CD1_MASK					.equ	h'04	; mask to obtain value of CD1 
CC_PCMCIA_ISR_BVD_MASK					.equ	h'03	; mask to obtain value of BVD pins 
CC_PCMCIA_ISR_BVD_NORMAL				.equ	h'03	; battery is fine 
CC_PCMCIA_ISR_BVD_LOW_GD				.equ	h'01	; battery is low but data is OK 
CC_PCMCIA_ISR_BVD_LOW_CD				.equ	h'02	; battery is low and data is corrupt 
CC_PCMCIA_ISR_BVD_DEAD					.equ	h'00	; battery is dead 
CC_PCMCIA_ISR_SPKR_MASK					.equ	h'02	; mask to obtain value of SPK pin 
CC_PCMCIA_ISR_STSCH_MASK				.equ	h'01	; mask to obtain value of STSCH pin 

; General Control reg

CC_PCMCIA_GCR_DRV_ENABLE				.equ	h'80	; Enables the PCMCIA card 
CC_PCMCIA_GCR_RESET						.equ	h'40	; Causes a reset on the card 
CC_PCMCIA_GCR_IO_CARD					.equ	h'20	; There is an I/O card in this slot 
CC_PCMCIA_GCR_MEM_CARD					.equ	h'00	; There is a mem card in this slot 
CC_PCMCIA_GCR_5V_ENABLE					.equ	h'10	; Spcifies to use 3.3V 
CC_PCMCIA_GCR_MMOD_32					.equ	h'00	; set mode as 32MB mem areas 
CC_PCMCIA_GCR_MMOD_16					.equ	h'08	; set mode as 16MB mem areas 
CC_PCMCIA_GCR_A25_HIGH					.equ	h'04	; set A25 High 
CC_PCMCIA_GCR_A25_LOW					.equ	h'00	; set A25 Low 
CC_PCMCIA_GCR_A24_HIGH					.equ	h'02	; set A24 High 
CC_PCMCIA_GCR_A24_LOW					.equ	h'00	; set A24 Low 
CC_PCMCIA_GCR_ADDR_MASK					.equ	h'06	; Mask A24 and A25 
CC_PCMCIA_GCR_REG_HIGH					.equ	h'01	; set REG High 
CC_PCMCIA_GCR_REG_LOW					.equ	h'00	; set REG Low 

; Card Status Change (PCC0CSCR) Reg

CC_PCMCIA_CSCR_GEN_CD_INT				.equ	h'80	; Generate a CD interrupt 
CC_PCMCIA_CSCR_IREQ_INT_REQ				.equ	h'20	; IREQ Int has occurred 
CC_PCMCIA_CSCR_STSCH_INT_REQ			.equ	h'10	; STSCH Int has occurred 
CC_PCMCIA_CSCR_CD_INT_REQ				.equ	h'08	; CD Int occurred 
CC_PCMCIA_CSCR_RDY_INT_REQ				.equ	h'04	; RDY Int occurred 
CC_PCMCIA_CSCR_BW_INT_REQ				.equ	h'02	; Batt warning int occurred 
CC_PCMCIA_CSCR_BE_INT_REQ				.equ	h'01	; Batt dead int occurred 
CC_PCMCIA_CSCR_TPS2206_SEL				.equ	h'40	; TPS2206 serial power switch 
CC_PCMCIA_CSCR_MIC2563_SEL				.equ	h'bf	; MIC2563 select 

; Card Status Change Interrupt Enable Reg

CC_PCMCIA_CSCIER_AUTO_GCR				.equ	h'80	; Automatically init the GCR 
CC_PCMCIA_CSCIER_IREQ_DIS				.equ	h'00	; Disable IREQ ints 
CC_PCMCIA_CSCIER_IREQ_LEVEL				.equ	h'20	; IREQ Level ints 
CC_PCMCIA_CSCIER_IREQ_PFE				.equ	h'40	; IREQ Falling Edge, Pulse 
CC_PCMCIA_CSCIER_IREQ_PRE				.equ	h'60	; IREQ Rising Edge Pulse 
CC_PCMCIA_CSCIER_IREQ_MASK				.equ	h'60	; For masking this value 
CC_PCMCIA_CSCIER_STSCH_INT_EN			.equ	h'10	; Enable STSCH Interrupts 
CC_PCMCIA_CSCIER_CD_INT_EN				.equ	h'08	; Enable CD Interrupts 
CC_PCMCIA_CSCIER_RDY_INT_EN				.equ	h'04	; Enable RDY/BSY Interrupts 
CC_PCMCIA_CSCIER_BWE_INT_EN				.equ	h'02	; Enable Batt Warning Interrupts 
CC_PCMCIA_CSCIER_BDE_INT_EN				.equ	h'01	; Enable Batt Dead Interrupts 

; Software control reg

CC_PCMCIA_SCR_3V_ENABLE					.equ	h'02	; Specifies to use 5V 
CC_PCMCIA_SCR_MASK_VCC					.equ	h'0c	; Masks out our bits 
CC_PCMCIA_SCR_VCC0VPP0					.equ	h'04	; Voltage control pin P0VPP0 
CC_PCMCIA_SCR_VCC0VPP1					.equ	h'08	; Voltage control pin P0VPP1 
CC_PCMCIA_SCR_VCC1VPP0					.equ	h'04	; Voltage control pin P1VPP0 
CC_PCMCIA_SCR_VCC1VPP1					.equ	h'08	; Voltage control pin P1VPP1 
CC_PCMCIA_SCR_SHDN_ENB					.equ	h'10	; Shutdown bit for TPS2206

; Serial Power Switch Control Register

CC_PCMCIA_PSR_5V_ENBB					.equ	h'80	; Enable BVcc   5V
CC_PCMCIA_PSR_3V_ENBB					.equ	h'40	; Enable BVcc 3.3V
CC_PCMCIA_PSR_BVPP_VCC					.equ	h'20	; Enable BVpp 3.3V or 5V
CC_PCMCIA_PSR_BVPP_PGM					.equ	h'10	; Enable BVpp  12v
CC_PCMCIA_PSR_3V_ENBA					.equ	h'08	; Enable AVcc 3.3V
CC_PCMCIA_PSR_5V_ENBA					.equ	h'04	; Enable AVcc   5V
CC_PCMCIA_PSR_AVPP_VCC					.equ	h'02	; Enable AVpp 3.3V or 5V
CC_PCMCIA_PSR_AVPP_PGM					.equ	h'01	; Enable AVpp  12V

CC_PCMCIA_PSR_AVCC_MASK					.equ	h'0c
CC_PCMCIA_PSR_AVPP_MASK					.equ	h'03
CC_PCMCIA_PSR_BVCC_MASK					.equ	h'c0
CC_PCMCIA_PSR_BVPP_MASK					.equ	h'30

CC_PCMCIA_PSR_AVCC_AVPP					.equ	CC_PCMCIA_PSR_AVCC_MASK	| CC_PCMCIA_PSR_AVPP_MASK
CC_PCMCIA_PSR_BVCC_BVPP					.equ	CC_PCMCIA_PSR_BVCC_MASK	| CC_PCMCIA_PSR_BVPP_MASK

;
; definitions for the FIR on the companion chip
;

CC_FIR_REGBASE							.equ	HD64465_BASE + HD64465_FIR_OFFSET

CC_FIR_IRRBR_OFFSET						.equ	0x0000	; Receiver Buffer Register offset address
CC_FIR_IRTBR_OFFSET						.equ	0x0000	; Transmitter Buffer Register offset address
CC_FIR_IRIER_OFFSET						.equ	0x0002	; Interrupt Enable Register offset address
CC_FIR_IRIIR_OFFSET						.equ	0x0004	; Interrupt Identification Register offset address
CC_FIR_IRFCR_OFFSET						.equ	0x0004	; FIFO Control Register offset address
CC_FIR_IRLCR_OFFSET						.equ	0x0006	; Line Control Register offset address
CC_FIR_IRMCR_OFFSET						.equ	0x0008	; Modem Control Register offset address
CC_FIR_IRDLL_OFFSET						.equ	0x0000	; Divisor Latch LSB offset address
CC_FIR_IRDLM_OFFSET						.equ	0x0002	; Divisor Latch MSB offset address
CC_FIR_IRLSR_OFFSET						.equ	0x000a	; Line Status Register offset address
CC_FIR_IRMSR_OFFSET						.equ	0x000c	; Modem Status Register offset address
CC_FIR_IRSCR_OFFSET						.equ	0x000e	; Scratch Pad Register offset address

CC_FIR_IMSTCR_OFFSET					.equ	0x0100	; Master Control Register offset address
CC_FIR_IMSTSR_OFFSET					.equ	0x0102	; Master Status Register offset address
CC_FIR_IMISCR_OFFSET					.equ	0x0102	; Misc. Control Register offset address
CC_FIR_IRFR_OFFSET						.equ	0x0104	; Rx FIFO Register offset address
CC_FIR_ITFR_OFFSET						.equ	0x0104	; Tx FIFO Register offset address
CC_FIR_ITC1R_OFFSET						.equ	0x0106	; Tx Control 1 Register offset address
CC_FIR_ITC2R_OFFSET						.equ	0x0108	; Tx Control 2 Register offset address
CC_FIR_ITSR_OFFSET						.equ	0x010a	; Tx Status Register offset address
CC_FIR_IRCR_OFFSET						.equ	0x010c	; Rx control Register offset address
CC_FIR_IRSR_OFFSET						.equ	0x010e	; Rx Status Register offset address
CC_FIR_IRSTCR_OFFSET					.equ	0x010e	; Reset Command Register offset address

CC_FIR_IFAR_OFFSET						.equ	0x0102	; Frame Address Register offset address
CC_FIR_IRBCLR_OFFSET					.equ	0x0104	; Rx Byte Count Low Register offset address
CC_FIR_IRBCHR_OFFSET					.equ	0x0106	; Rx Byte Count High Register offset address
CC_FIR_IRRFPLR_OFFSET					.equ	0x0108	; Rx Ring Frame Pointer Low Register offset address
CC_FIR_IRRFPHR_OFFSET					.equ	0x010a	; Rx Ring Frame Pointer High Register offset address
CC_FIR_ITBCLR_OFFSET					.equ	0x010c	; Tx Byte Count Low Register offset address
CC_FIR_ITBCHR_OFFSET					.equ	0x010e	; Tx Byte Count High Register offset address

CC_FIR_IIRC1R_OFFSET					.equ	0x0102	; Infrared Configuration 1 Register offset address
CC_FIR_IIRTCR_OFFSET					.equ	0x0104	; Infrared Transceiver Control Register offset address
CC_FIR_IIRC2R_OFFSET					.equ	0x0106	; Infrared Configuration 2 Register offset address
CC_FIR_ITMR_OFFSET						.equ	0x0108	; Timer Register offset address
CC_FIR_IIRC3R_OFFSET					.equ	0x010a	; Infrared Configuration 3 Register offset address

CC_FIR_DMARP_OFFSET						.equ	0x0110	; DMA Data Read Port offset address
CC_FIR_DMAWP_OFFSET						.equ	0x0110	; DMA Data Write Port offset address
CC_FIR_ISIRR_OFFSET						.equ	0x0120	; SIR Register offset address
CC_FIR_IFIRCR_OFFSET					.equ	0x01e0	; FIR Configuration Register offset address
CC_FIR_ITMCR_OFFSET						.equ	0x01f0	; Timing Control Register offset address

CC_FIR_IRRBR							.equ	(CC_FIR_REGBASE + CC_FIR_IRRBR_OFFSET)	;Receiver Buffer Register 
CC_FIR_IRTBR							.equ	(CC_FIR_REGBASE + CC_FIR_IRTBR_OFFSET)	;Transmitter Buffer Register 
CC_FIR_IRIER							.equ	(CC_FIR_REGBASE + CC_FIR_IRIER_OFFSET)	;Interrupt Enable Register 
CC_FIR_IRIIR							.equ	(CC_FIR_REGBASE + CC_FIR_IRIIR_OFFSET)	;Interrupt Identification Register 
CC_FIR_IRFCR							.equ	(CC_FIR_REGBASE + CC_FIR_IRFCR_OFFSET)	;FIFO Control Register 
CC_FIR_IRLCR							.equ	(CC_FIR_REGBASE + CC_FIR_IRLCR_OFFSET)	;Line Control Register 
CC_FIR_IRMCR							.equ	(CC_FIR_REGBASE + CC_FIR_IRMCR_OFFSET)	;Modem Control Register 
CC_FIR_IRDLL							.equ	(CC_FIR_REGBASE + CC_FIR_IRDLL_OFFSET)	;Divisor Latch LSB 
CC_FIR_IRDLM							.equ	(CC_FIR_REGBASE + CC_FIR_IRDLM_OFFSET)	;Divisor Latch MSB 
CC_FIR_IRLSR							.equ	(CC_FIR_REGBASE + CC_FIR_IRLSR_OFFSET)	;Line Status Register 
CC_FIR_IRMSR							.equ	(CC_FIR_REGBASE + CC_FIR_IRMSR_OFFSET)	;Modem Status Register 
CC_FIR_IRSCR							.equ	(CC_FIR_REGBASE + CC_FIR_IRSCR_OFFSET)	;Scratch Pad Register 

CC_FIR_IMSTCR							.equ	(CC_FIR_REGBASE + CC_FIR_IMSTCR_OFFSET)	;Master Control Register 
CC_FIR_IMSTSR							.equ	(CC_FIR_REGBASE + CC_FIR_IMSTSR_OFFSET)	;Master Status Register 
CC_FIR_IMISCR							.equ	(CC_FIR_REGBASE + CC_FIR_IMISCR_OFFSET)	;Misc. Control Register 
CC_FIR_IRFR								.equ	(CC_FIR_REGBASE + CC_FIR_IRFR_OFFSET)	;Rx FIFO Register 
CC_FIR_ITFR								.equ	(CC_FIR_REGBASE + CC_FIR_ITFR_OFFSET)	;Tx FIFO Register 
CC_FIR_ITC1R							.equ	(CC_FIR_REGBASE + CC_FIR_ITC1R_OFFSET)	;Tx Control 1 Register 
CC_FIR_ITC2R							.equ	(CC_FIR_REGBASE + CC_FIR_ITC2R_OFFSET)	;Tx Control 2 Register 
CC_FIR_ITSR								.equ	(CC_FIR_REGBASE + CC_FIR_ITSR_OFFSET)	;Tx Status Register 
CC_FIR_IRCR								.equ	(CC_FIR_REGBASE + CC_FIR_IRCR_OFFSET)	;Rx control Register 
CC_FIR_IRSR								.equ	(CC_FIR_REGBASE + CC_FIR_IRSR_OFFSET)	;Rx Status Register 
CC_FIR_IRSTCR							.equ	(CC_FIR_REGBASE + CC_FIR_IRSTCR_OFFSET)	;Reset Command Register 

CC_FIR_IFAR								.equ	(CC_FIR_REGBASE + CC_FIR_IFAR_OFFSET)	;Frame Address Register 
CC_FIR_IRBCLR							.equ	(CC_FIR_REGBASE + CC_FIR_IRBCLR_OFFSET)	;Rx Byte Count Low Register
CC_FIR_IRBCHR							.equ	(CC_FIR_REGBASE + CC_FIR_IRBCHR_OFFSET)	;Rx Byte Count High Register 
CC_FIR_IRRFPLR							.equ	(CC_FIR_REGBASE + CC_FIR_IRRFPLR_OFFSET);Rx Ring Frame Pointer Low Register 
CC_FIR_IRRFPHR							.equ	(CC_FIR_REGBASE + CC_FIR_IRRFPHR_OFFSET);Rx Ring Frame Pointer High Register 
CC_FIR_ITBCLR							.equ	(CC_FIR_REGBASE + CC_FIR_ITBCLR_OFFSET)	;Tx Byte Count Low Register 
CC_FIR_ITBCHR							.equ	(CC_FIR_REGBASE + CC_FIR_ITBCHR_OFFSET)	;Tx Byte Count High Register 

CC_FIR_IIRC1R							.equ	(CC_FIR_REGBASE + CC_FIR_IIRC1R_OFFSET)	;Infrared Configuration 1 Register 
CC_FIR_IIRTCR							.equ	(CC_FIR_REGBASE + CC_FIR_IIRTCR_OFFSET)	;Infrared Transceiver Control Register 
CC_FIR_IIRC2R							.equ	(CC_FIR_REGBASE + CC_FIR_IIRC2R_OFFSET)	;Infrared Configuration 2 Register 
CC_FIR_ITMR								.equ	(CC_FIR_REGBASE + CC_FIR_ITMR_OFFSET)	;Timer Register 
CC_FIR_IIRC3R							.equ	(CC_FIR_REGBASE + CC_FIR_IIRC3R_OFFSET)	;Infrared Configuration 3 Register 

CC_FIR_DMARP							.equ	(CC_FIR_REGBASE + CC_FIR_DMARP_OFFSET)	;DMA Data Read Port 
CC_FIR_DMAWP							.equ	(CC_FIR_REGBASE + CC_FIR_DMAWP_OFFSET)	;DMA Data Write Port 
CC_FIR_ISIRR							.equ	(CC_FIR_REGBASE + CC_FIR_ISIRR_OFFSET)	;SIR Register 
CC_FIR_IFIRCR							.equ	(CC_FIR_REGBASE + CC_FIR_IFIRCR_OFFSET)	;FIR Configuration Register 
CC_FIR_ITMCR							.equ	(CC_FIR_REGBASE + CC_FIR_ITMCR_OFFSET)	;Timing Control Register 

;Master Control Register (IMSTCR)
CC_FIR_IMSTCR_IEN						.equ	0x80
CC_FIR_IMSTCR_TXEN						.equ	0x40
CC_FIR_IMSTCR_RXEN						.equ	0x20
CC_FIR_IMSTCR_RST_BANK					.equ	0xe0
CC_FIR_IMSTCR_BANK0						.equ	0x00
CC_FIR_IMSTCR_BANK1						.equ	0x01
CC_FIR_IMSTCR_BANK2						.equ	0x02

;Master Status Register (IMSTSR)
CC_FIR_IMSTSR_TMI						.equ	0x40
CC_FIR_IMSTSR_TXI						.equ	0x20
CC_FIR_IMSTSR_RXI						.equ	0x10
CC_FIR_IMSTSR_IID_RXSC					.equ	0x08
CC_FIR_IMSTSR_IID_RXDA					.equ	0x0a
CC_FIR_IMSTSR_IID_TXBE					.equ	0x0c
CC_FIR_IMSTSR_IID_TXSC					.equ	0x0e

;Miscellaneous Control Register (IMISCR)
CC_FIR_IMISCR_DCS_TX					.equ	0x80
CC_FIR_IMISCR_DCS_RX					.equ	0x40
CC_FIR_IMISCR_DCS_NONE					.equ	0x00
CC_FIR_IMISCR_ILOOP						.equ	0x10

;Tx Control 1 Register(ITC1R)
CC_FIR_ITC1R_RTS						.equ	0x80
CC_FIR_ITC1R_TFRIEN						.equ	0x40
CC_FIR_ITC1R_TFUIEN						.equ	0x20
CC_FIR_ITC1R_TFTL						.equ	0x10
CC_FIR_ITC1R_ADRTS						.equ	0x08
CC_FIR_ITC1R_ACEOM						.equ	0x04
CC_FIR_ITC1R_TIDL						.equ	0x02
CC_FIR_ITC1R_UA							.equ	0x01

;Tx Control 2 Register (ITC2R)
CC_FIR_ITC2R_SB							.equ	0x80
CC_FIR_ITC2R_ACRCG						.equ	0x40
CC_FIR_ITC2R_SIP_NOW					.equ	0x20
CC_FIR_ITC2R_SIP_AFTER					.equ	0x10
CC_FIR_ITC2R_NSFP						.equ	0x08
CC_FIR_ITC2R_EEIL_EOM					.equ	0x00
CC_FIR_ITC2R_EEIL_16					.equ	0x01
CC_FIR_ITC2R_EEIL_32					.equ	0x02
CC_FIR_ITC2R_EEIL_64					.equ	0x03
CC_FIR_ITC2R_EEIL_128					.equ	0x04
CC_FIR_ITC2R_EEIL_256					.equ	0x05
CC_FIR_ITC2R_EEIL_512					.equ	0x06
CC_FIR_ITC2R_EEIL_1024					.equ	0x07

;Tx Status Register (ITSR)
CC_FIR_ITSR_TFUR						.equ	0x08
CC_FIR_ITSR_EOM							.equ	0x04
CC_FIR_ITSR_TFRDY						.equ	0x02
CC_FIR_ITSR_EEOM						.equ	0x01

;Rx Control Register (IRCR)
CC_FIR_IRCR_RFTL						.equ	0x80
CC_FIR_IRCR_ACRCC						.equ	0x40
CC_FIR_IRCR_RADM_ALL					.equ	0x00
CC_FIR_IRCR_RADM_LOHI_FAR				.equ	0x10
CC_FIR_IRCR_RADM_HI_FAR					.equ	0x20
CC_FIR_IRCR_SYNIEN						.equ	0x08
CC_FIR_IRCR_RFRIEN						.equ	0x02
CC_FIR_IRCR_SCIEN						.equ	0x01

;Rx Status Register (IRSR)
CC_FIR_IRSR_ABORT						.equ	0x80
CC_FIR_IRSR_CRCER						.equ	0x40
CC_FIR_IRSR_RFOVF						.equ	0x20
CC_FIR_IRSR_EOF							.equ	0x10
CC_FIR_IRSR_RFEM						.equ	0x08
CC_FIR_IRSR_SYNC						.equ	0x04

;Reset Command Register (IRSTCR)
CC_FIR_IRSTCR_RSTC_HUNT					.equ	0x10
CC_FIR_IRSTCR_RSTC_RXFIFO				.equ	0x20
CC_FIR_IRSTCR_RSTC_RXSCI				.equ	0x30
CC_FIR_IRSTCR_RSTC_RXRFP				.equ	0x40
CC_FIR_IRSTCR_RSTC_UNDERRUN				.equ	0x50
CC_FIR_IRSTCR_RSTC_TXFIFO				.equ	0x60
CC_FIR_IRSTCR_RSTC_HW					.equ	0x70

;Infrared Configuration 1 Register (IIRC1R)
CC_FIR_IIRC1R_IRSPD_1152				.equ	0x00
CC_FIR_IIRC1R_IRSPD_0756				.equ	0x10
CC_FIR_IIRC1R_IRSPD_0288				.equ	0x20
CC_FIR_IIRC1R_IRMOD_HPSIR				.equ	0x00
CC_FIR_IIRC1R_IRMOD_ASK					.equ	0x01
CC_FIR_IIRC1R_IRMOD_MIR					.equ	0x02
CC_FIR_IIRC1R_IRMOD_FIR					.equ	0x04

;Infrared Transceiver Control Register (IIRTCR)
CC_FIR_IIRTCR_DFREQ						.equ	0x20
CC_FIR_IIRTCR_MODSEL					.equ	0x10
CC_FIR_IIRTCR_ECHO						.equ	0x08
CC_FIR_IIRTCR_TXDF						.equ	0x02

;Infrared Configuration 2 Register (IIRC2R)
CC_FIR_IIRC2R_CHOP_DISABLE				.equ	0x70
CC_FIR_IIRC2R_CHOP_EX187				.equ	0x74
CC_FIR_IIRC2R_CHOP_EX229				.equ	0x78
CC_FIR_IIRC2R_CHOP_EX208				.equ	0x7c
CC_FIR_IIRC2R_CHOP_ENABLE_MAX			.equ	0xf0
CC_FIR_IIRC2R_CHOP_ENABLE_LESS			.equ	0xf4
CC_FIR_IIRC2R_CHOP_ENABLE_ZERO_BTB42	.equ	0xf8
CC_FIR_IIRC2R_CHOP_ENABLE_ZERO			.equ	0xfc
CC_FIR_IIRC2R_DSIRI						.equ	0x02
CC_FIR_IIRC2R_DFIRI						.equ	0x01

;Infrared Configuration 3 Register (IIRC3R)
CC_FIR_IIRC3R_SCDIEN					.equ	0x80
CC_FIR_IIRC3R_SCD						.equ	0x40
CC_FIR_IIRC3R_TMIEN						.equ	0x02
CC_FIR_IIRC3R_TMI						.equ	0x01

;SIR Register (ISIRR)
CC_FIR_ISIRR_SLOOP						.equ	0x02
CC_FIR_ISIRR_SIRMOD						.equ	0x01

;FIR Configuration Register (IFIRCR)
CC_FIR_IFIRCR_RX2_PP					.equ	0x04
CC_FIR_IFIRCR_RX_PP						.equ	0x02
CC_FIR_IFIRCR_TMODE						.equ	0x01

;Timing Control Register (ITMCR)
CC_FIR_ITMCR_TMCR_12					.equ	0x00
CC_FIR_ITMCR_TMCR_25					.equ	0x02
CC_FIR_ITMCR_TMCR_30					.equ	0x03
CC_FIR_ITMCR_TMCR_40					.equ	0x04
CC_FIR_ITMCR_TMCR_50					.equ	0x05
CC_FIR_ITMCR_TMCR_66					.equ	0x06


;
; definitions for the UART on the companion chip
;

CC_UART_REGBASE							.equ	HD64465_BASE + HD64465_UART_OFFSET

CC_UART_UTBR_OFFSET						.equ	0x0000
CC_UART_URBR_OFFSET						.equ	0x0000	// Mirrors UTBR
CC_UART_UIER_OFFSET						.equ	0x0002
CC_UART_UIIR_OFFSET						.equ	0x0004
CC_UART_UFCR_OFFSET						.equ	0x0004	// Mirrors UIIR
CC_UART_ULCR_OFFSET						.equ	0x0006
CC_UART_UMCR_OFFSET						.equ	0x0008
CC_UART_UDLL_OFFSET						.equ	0x0000
CC_UART_UDLM_OFFSET						.equ	0x0002
CC_UART_ULSR_OFFSET						.equ	0x000a
CC_UART_UMSR_OFFSET						.equ	0x000c
CC_UART_USCR_OFFSET						.equ	0x000e

CC_16550_REG_STRIDE						.equ	2				// each register is spaced 2 bytes apart

CC_UART_UTBR		.equ	(CC_UART_REGBASE + CC_UART_UTBR_OFFSET)
CC_UART_URBR		.equ	(CC_UART_REGBASE + CC_UART_URBR_OFFSET)
CC_UART_UIER		.equ	(CC_UART_REGBASE + CC_UART_UIER_OFFSET)
CC_UART_UIIR		.equ	(CC_UART_REGBASE + CC_UART_UIIR_OFFSET)
CC_UART_UFCR		.equ	(CC_UART_REGBASE + CC_UART_UFCR_OFFSET)
CC_UART_ULCR		.equ	(CC_UART_REGBASE + CC_UART_ULCR_OFFSET)
CC_UART_UMCR		.equ	(CC_UART_REGBASE + CC_UART_UMCR_OFFSET)
CC_UART_UDLL		.equ	(CC_UART_REGBASE + CC_UART_UDLL_OFFSET)
CC_UART_UDLM		.equ	(CC_UART_REGBASE + CC_UART_UDLM_OFFSET)
CC_UART_ULSR		.equ	(CC_UART_REGBASE + CC_UART_ULSR_OFFSET)
CC_UART_UMSR		.equ	(CC_UART_REGBASE + CC_UART_UMSR_OFFSET)
CC_UART_USCR		.equ	(CC_UART_REGBASE + CC_UART_USCR_OFFSET)

//
// Parallel Interface defines,  HD64465 only 99-08-11 cea
//

CC_PAR_BASE_REG 			.equ	(HD64465_BASE + HD64465_PARALLEL_OFFSET)
CC_PAR_DATA_REG 			.equ	CC_PAR_BASE_REG + 0x00
CC_PAR_STATUS_REG 			.equ	CC_PAR_BASE_REG + 0x02
CC_PAR_CONTROL_REG			.equ	CC_PAR_BASE_REG + 0x04
CC_PAR_ECP_DATA_REG			.equ	CC_PAR_BASE_REG + 0x10

CC_PAR_EXT_CONTROL_REG		.equ	CC_PAR_BASE_REG + 0x14

; CC_PAR_EXT_CONTROL_REG mode bits
; 		Bits 7-5 select mode,
;             000 = SPP
;             001 = PS/2 Parallel mode
;             010 = Parallel Port FIFO mode
;             011 = ECP Parallel Port mode
;             100 = Reserved
;             101 = Reserved
;             110 = Test mode
;             111 = Configuration mode
;		Bit 4 Error Interrupt Enable (nErrIntrEn) 0=enable, 1=disable
;		Bit 2 Service Interrupt (ServiceIntr) 0=enable, 1=disable
;
CC_PAR_SPP_MODE				.equ	(0x14)
CC_PAR_PS2_MODE				.equ	(0x34)
CC_PAR_ECP_MODE				.equ	(0x74)
CC_PAR_TEST_MODE			.equ	(0xC0)

; The WinCE documentation for the D9000
;   parallel port uses some variation in
;   the names of the parallel port bits.
;
;   The D9000 uses a 36 pin Centronics connector
;   on the debug interface board. This interface
;   seems to designed to look like a printer.
;
;   The S1 uses a 25 pin D-type connector. this
;   interface is designed to look like a host. To
;   make the S1 to behave like a printer is tricky.
;
;  DB25 (S1)                 Cen-36 (D9000)
;  DIR Pin#                  DIR Pin# 
;   O  1    CC_PAR_STROBE     I  1   DF_PAR_STROBE
;  I/O 2-9  (data bus)       I/O 2-9 (data bus)
;   I  10   CC_PAR_NACK       O  10  DF_PAR_NACK
;   I  11   CC_PAR_BUSY       O  11  DF_PAR_BUSY
;   I  12   CC_PAR_PE         O  12  DF_PAR_ERROR
;   I  13   CC_PAR_SELECT     O  13  DF_PAR_SELECT
;   O  14   CC_PAR_AUTOFD     I  14  DF_PAR_AUTOFD
;   I  15   CC_PAR_ERROR      O  32  DF_PAR_NFAULT
;   O  16   CC_PAR_INIT       I  31  DF_PAR_INIT
;   O  17   CC_PAR_SELECTIN   I  36  DF_PAR_SELECTIN
;
;           CC_PAR_READDIR       DF_PAR_EN
;           CC_PAR_IRQEN         DF_PAR_INTR_MASK
;
;--------------------------------------------------
;
;   The WinCE help data base describes a cable
;   to connect the development workstation to
;   a WinCE target. This is the cable used to 
;   connect to the HD64465 parallel port.
;
;

; CC_PAR_STATUS_REG bits
CC_PAR_BUSY					.equ	0x80
CC_PAR_NACK					.equ	0x40
CC_PAR_PE					.equ	0x20
CC_PAR_SELECT				.equ	0x10
CC_PAR_ERROR				.equ	0x08

; CC_PAR_CONTROL_REG bits
; 		CC_PAR_READDIR,0 = output, 1 = input
CC_PAR_READDIR				.equ	0x20
CC_PAR_IRQEN				.equ	0x10
CC_PAR_SELECTIN				.equ	0x08
CC_PAR_INIT					.equ	0x04
CC_PAR_AUTOFD				.equ	0x02
CC_PAR_STROBE				.equ	0x01

; CC_PAR_EXT_CONTROL_REG bits
CC_PAR_NERRINTRE			.equ	0x10
CC_PAR_SERVICEINTR			.equ	0x04
CC_PAR_OUT_FULL				.equ	0x02
CC_PAR_IN_EMPTY				.equ	0x01

;
; Serial CODEC Interface defines
;

	.aif ENABLE_HD64464 eq h'00
CC_CODEC_REGBASE_463					.equ	HD64463_BASE + HD64463_CODEC_OFFSET
	.aendi
CC_CODEC_REGBASE						.equ	HD64465_BASE + HD64465_CODEC_OFFSET
CC_CODEC_REGSIZE						.equ	h'0070

CC_CODEC_TDR_OFFSET						.equ	h'0000
CC_CODEC_RDR_OFFSET						.equ	h'0004
CC_CODEC_CR_OFFSET						.equ	h'0008
CC_CODEC_SR_OFFSET						.equ	h'000C
CC_CODEC_FSR_OFFSET						.equ	h'0010
CC_CODEC_CAR_OFFSET						.equ	h'0020
CC_CODEC_CDR_OFFSET						.equ	h'0024
CC_CODEC_PCML_OFFSET					.equ	h'0028
CC_CODEC_PCMR_OFFSET					.equ	h'002C
CC_CODEC_LINE1_OFFSET					.equ	h'0030
CC_CODEC_PCMC_OFFSET					.equ	h'0034
CC_CODEC_PCMLS_OFFSET					.equ	h'0038
CC_CODEC_PCMRS_OFFSET					.equ	h'003C
CC_CODEC_PCMLFE_OFFSET					.equ	h'0040
CC_CODEC_LINE2_OFFSET					.equ	h'0044
CC_CODEC_HSET_OFFSET					.equ	h'0048
CC_CODEC_IOCS_OFFSET					.equ	h'004C
CC_CODEC_ATIER_OFFSET					.equ	h'0050
CC_CODEC_ATSR_OFFSET					.equ	h'0054
CC_CODEC_ARIER_OFFSET					.equ	h'0058
CC_CODEC_ARSR_OFFSET					.equ	h'005C
CC_CODEC_ACR_OFFSET						.equ	h'0060
CC_CODEC_ATAGR_OFFSET					.equ	h'0064
CC_CODEC_SRAR_OFFSET					.equ	h'0068

CC_CODEC_TDR							.equ	CC_CODEC_REGBASE + CC_CODEC_TDR_OFFSET
CC_CODEC_RDR							.equ	CC_CODEC_REGBASE + CC_CODEC_RDR_OFFSET
CC_CODEC_CR								.equ	CC_CODEC_REGBASE + CC_CODEC_CR_OFFSET
CC_CODEC_SR								.equ	CC_CODEC_REGBASE + CC_CODEC_SR_OFFSET
CC_CODEC_FSR							.equ	CC_CODEC_REGBASE + CC_CODEC_FSR_OFFSET
CC_CODEC_CAR							.equ	CC_CODEC_REGBASE + CC_CODEC_CAR_OFFSET
CC_CODEC_CDR							.equ	CC_CODEC_REGBASE + CC_CODEC_CDR_OFFSET
CC_CODEC_PCML							.equ	CC_CODEC_REGBASE + CC_CODEC_PCML_OFFSET
CC_CODEC_PCMR							.equ	CC_CODEC_REGBASE + CC_CODEC_PCMR_OFFSET
CC_CODEC_LINE1							.equ	CC_CODEC_REGBASE + CC_CODEC_LINE1_OFFSET
CC_CODEC_PCMC							.equ	CC_CODEC_REGBASE + CC_CODEC_PCMC_OFFSET
CC_CODEC_PCMLS							.equ	CC_CODEC_REGBASE + CC_CODEC_PCMLS_OFFSET
CC_CODEC_PCMRS							.equ	CC_CODEC_REGBASE + CC_CODEC_PCMRS_OFFSET
CC_CODEC_PCMLFE							.equ	CC_CODEC_REGBASE + CC_CODEC_PCMLFE_OFFSET
CC_CODEC_LINE2							.equ	CC_CODEC_REGBASE + CC_CODEC_LINE2_OFFSET
CC_CODEC_HSET							.equ	CC_CODEC_REGBASE + CC_CODEC_HSET_OFFSET
CC_CODEC_IOCS							.equ	CC_CODEC_REGBASE + CC_CODEC_IOCS_OFFSET
CC_CODEC_ATIER							.equ	CC_CODEC_REGBASE + CC_CODEC_ATIER_OFFSET
CC_CODEC_ATSR							.equ	CC_CODEC_REGBASE + CC_CODEC_ATSR_OFFSET
CC_CODEC_ARIER							.equ	CC_CODEC_REGBASE + CC_CODEC_ARIER_OFFSET
CC_CODEC_ARSR							.equ	CC_CODEC_REGBASE + CC_CODEC_ARSR_OFFSET
CC_CODEC_ACR							.equ	CC_CODEC_REGBASE + CC_CODEC_ACR_OFFSET
CC_CODEC_ATAGR							.equ	CC_CODEC_REGBASE + CC_CODEC_ATAGR_OFFSET
CC_CODEC_SRAR							.equ	CC_CODEC_REGBASE + CC_CODEC_SRAR_OFFSET

; Control Register (CR)

CC_CODEC_CR_DMAEN						.equ	h'2000
CC_CODEC_CR_SL18						.equ	h'1000
CC_CODEC_CR_CDRT						.equ	h'0800
CC_CODEC_CR_WMRT						.equ	h'0400
CC_CODEC_CR_AC97S						.equ	h'0200
CC_CODEC_CR_SWR							.equ	h'0100
CC_CODEC_CR_PU							.equ	h'0080
CC_CODEC_CR_MS							.equ	h'0040
CC_CODEC_CR_ST							.equ	h'0020
CC_CODEC_CR_CRE							.equ	h'0010
CC_CODEC_CR_FTF							.equ	h'0008
CC_CODEC_CR_TXEN						.equ	h'0004
CC_CODEC_CR_FRF							.equ	h'0002
CC_CODEC_CR_RXEN						.equ	h'0001

; Status Register (SR)

CC_CODEC_SR_IR71						.equ	h'4000
CC_CODEC_SR_TNF							.equ	h'2000

CC_CODEC_SR_TFS_E1E0					.equ	h'0000		; (FIFO-1, FIFO-0):(EMPTY,		EMPTY)
CC_CODEC_SR_TFS_E1N0					.equ	h'0800		; (FIFO-1, FIFO-0):(EMPTY,		NOT EMPTY)
CC_CODEC_SR_TFS_N1E0					.equ	h'1000		; (FIFO-1, FIFO-0):(NOT EMPTY,	EMPTY)
CC_CODEC_SR_TFS_N1N0					.equ	h'1800		; (FIFO-1, FIFO-0):(NOT EMPTY,	NOT EMPTY)

CC_CODEC_SR_TFU							.equ	h'0400
CC_CODEC_SR_TFO							.equ	h'0200
CC_CODEC_SR_TDI							.equ	h'0100
CC_CODEC_SR_RNE							.equ	h'0020

CC_CODEC_SR_RFS_N1N0					.equ	h'0000		; (FIFO-1, FIFO-0):(NOT FULL,	NOT FULL)
CC_CODEC_SR_RFS_N1F0					.equ	h'0008		; (FIFO-1, FIFO-0):(NOT FULL,	FULL)
CC_CODEC_SR_RFS_F1N0					.equ	h'0010		; (FIFO-1, FIFO-0):(FULL,		NOT FULL)
CC_CODEC_SR_RFS_F1F0					.equ	h'0018		; (FIFO-1, FIFO-0):(FULL,		FULL)

CC_CODEC_SR_RFU							.equ	h'0004
CC_CODEC_SR_RFO							.equ	h'0002
CC_CODEC_SR_RDI							.equ	h'0001

; Frequency Select Register (FSR)

CC_CODEC_FSR_FS_8000HZ					.equ	h'0000
CC_CODEC_FSR_FS_9600HZ					.equ	h'0001
CC_CODEC_FSR_FS_12000HZ					.equ	h'0002
CC_CODEC_FSR_FS_16000HZ					.equ	h'0003
CC_CODEC_FSR_FS_24000HZ					.equ	h'0004
CC_CODEC_FSR_FS_48000HZ					.equ	h'0005

; Command/Status Address Register (CAR/CSAR)

CC_CODEC_CAR_RW							.equ	h'00080000

; AC97 Transmit Interrupt Enable Register (ATIER)

CC_CODEC_ATIER_PLTFRQIE					.equ	h'20000000
CC_CODEC_ATIER_PRTFRQIE					.equ	h'10000000
CC_CODEC_ATIER_L1TFRQIE					.equ	h'08000000
CC_CODEC_ATIER_PCTFRQIE					.equ	h'04000000
CC_CODEC_ATIER_PLSTFRQIE				.equ	h'02000000
CC_CODEC_ATIER_PRSTFRQIE				.equ	h'01000000
CC_CODEC_ATIER_PLFETFRQIE				.equ	h'00800000
CC_CODEC_ATIER_L2TFRQIE					.equ	h'00400000
CC_CODEC_ATIER_HTTFRQIE					.equ	h'00200000
CC_CODEC_ATIER_IOCTFRQIE				.equ	h'00100000

CC_CODEC_ATIER_PLTFOVIE					.equ	h'00080000
CC_CODEC_ATIER_PRTFOVIE					.equ	h'00040000
CC_CODEC_ATIER_L1TFOVIE					.equ	h'00020000
CC_CODEC_ATIER_PCTFOVIE					.equ	h'00010000
CC_CODEC_ATIER_PLSTFOVIE				.equ	h'00008000
CC_CODEC_ATIER_PRSTFOVIE				.equ	h'00004000
CC_CODEC_ATIER_PLFETFOVIE				.equ	h'00002000
CC_CODEC_ATIER_L2TFOVIE					.equ	h'00001000
CC_CODEC_ATIER_HTTFOVIE					.equ	h'00000800
CC_CODEC_ATIER_IOCTFOVIE				.equ	h'00000400

CC_CODEC_ATIER_PLTFUNIE					.equ	h'00000200
CC_CODEC_ATIER_PRTFUNIE					.equ	h'00000100
CC_CODEC_ATIER_L1TFUNIE					.equ	h'00000080
CC_CODEC_ATIER_PCTFUNIE					.equ	h'00000040
CC_CODEC_ATIER_PLSTFUNIE				.equ	h'00000020
CC_CODEC_ATIER_PRSTFUNIE				.equ	h'00000010
CC_CODEC_ATIER_PLFETFUNIE				.equ	h'00000008
CC_CODEC_ATIER_L2TFUNIE					.equ	h'00000004
CC_CODEC_ATIER_HTTFUNIE					.equ	h'00000002
CC_CODEC_ATIER_IOCTFUNIE				.equ	h'00000001

; AC97 TX FIFO Status Register (ATSR)

CC_CODEC_ATSR_PLTFRQ					.equ	h'20000000
CC_CODEC_ATSR_PRTFRQ					.equ	h'10000000
CC_CODEC_ATSR_L1TFRQ					.equ	h'08000000
CC_CODEC_ATSR_PCTFRQ					.equ	h'04000000
CC_CODEC_ATSR_PLSTFRQ					.equ	h'02000000
CC_CODEC_ATSR_PRSTFRQ					.equ	h'01000000
CC_CODEC_ATSR_PLFETFRQ					.equ	h'00800000
CC_CODEC_ATSR_L2TFRQ					.equ	h'00400000
CC_CODEC_ATSR_HTTFRQ					.equ	h'00200000
CC_CODEC_ATSR_IOCTFRQ					.equ	h'00100000

CC_CODEC_ATSR_PLTFOV					.equ	h'00080000
CC_CODEC_ATSR_PRTFOV					.equ	h'00040000
CC_CODEC_ATSR_L1TFOV					.equ	h'00020000
CC_CODEC_ATSR_PCTFOV					.equ	h'00010000
CC_CODEC_ATSR_PLSTFOV					.equ	h'00008000
CC_CODEC_ATSR_PRSTFOV					.equ	h'00004000
CC_CODEC_ATSR_PLFETFOV					.equ	h'00002000
CC_CODEC_ATSR_L2TFOV					.equ	h'00001000
CC_CODEC_ATSR_HTTFOV					.equ	h'00000800
CC_CODEC_ATSR_IOCTFOV					.equ	h'00000400

CC_CODEC_ATSR_PLTFUN					.equ	h'00000200
CC_CODEC_ATSR_PRTFUN					.equ	h'00000100
CC_CODEC_ATSR_L1TFUN					.equ	h'00000080
CC_CODEC_ATSR_PCTFUN					.equ	h'00000040
CC_CODEC_ATSR_PLSTFUN					.equ	h'00000020
CC_CODEC_ATSR_PRSTFUN					.equ	h'00000010
CC_CODEC_ATSR_PLFETFUN					.equ	h'00000008
CC_CODEC_ATSR_L2TFUN					.equ	h'00000004
CC_CODEC_ATSR_HTTFUN					.equ	h'00000002
CC_CODEC_ATSR_IOCTFUN					.equ	h'00000001

; AC97 RX FIFO Interrupt Enable Register (ARIER)

CC_CODEC_ARIER_STARYIE					.equ	h'00400000
CC_CODEC_ARIER_STDRYIE					.equ	h'00200000

CC_CODEC_ARIER_PLRFRQIE					.equ	h'00100000
CC_CODEC_ARIER_PRRFRQIE					.equ	h'00080000
CC_CODEC_ARIER_L1RFRQIE					.equ	h'00040000
CC_CODEC_ARIER_MICRFRQIE				.equ	h'00020000
CC_CODEC_ARIER_L2RFRQIE					.equ	h'00010000
CC_CODEC_ARIER_HTRFRQIE					.equ	h'00008000
CC_CODEC_ARIER_IOCSRFRQIE				.equ	h'00004000

CC_CODEC_ARIER_PLRFOVIE					.equ	h'00002000
CC_CODEC_ARIER_PRRFOVIE					.equ	h'00001000
CC_CODEC_ARIER_L1RFOVIE					.equ	h'00000800
CC_CODEC_ARIER_MICRFOVIE				.equ	h'00000400
CC_CODEC_ARIER_L2RFOVIE					.equ	h'00000200
CC_CODEC_ARIER_HTRFOVIE					.equ	h'00000100
CC_CODEC_ARIER_IOCSRFOVIE				.equ	h'00000080

CC_CODEC_ARIER_PLRFUNIE					.equ	h'00000040
CC_CODEC_ARIER_PRRFUNIE					.equ	h'00000020
CC_CODEC_ARIER_L1RFUNIE					.equ	h'00000010
CC_CODEC_ARIER_MICRFUNIE				.equ	h'00000008
CC_CODEC_ARIER_L2RFUNIE					.equ	h'00000004
CC_CODEC_ARIER_HTRFUNIE					.equ	h'00000002
CC_CODEC_ARIER_IOCSRFUNIE				.equ	h'00000001

; AC97 RX FIFO Status Register (ARSR)

CC_CODEC_ARSR_STARY						.equ	h'00400000
CC_CODEC_ARSR_STDRY						.equ	h'00200000

CC_CODEC_ARSR_PLRFRQ					.equ	h'00100000
CC_CODEC_ARSR_PRRFRQ					.equ	h'00080000
CC_CODEC_ARSR_L1RFRQ					.equ	h'00040000
CC_CODEC_ARSR_MICRFRQ					.equ	h'00020000
CC_CODEC_ARSR_L2RFRQ					.equ	h'00010000
CC_CODEC_ARSR_HTRFRQ					.equ	h'00008000
CC_CODEC_ARSR_IOCSRFRQ					.equ	h'00004000

CC_CODEC_ARSR_PLRFOV					.equ	h'00002000
CC_CODEC_ARSR_PRRFOV					.equ	h'00001000
CC_CODEC_ARSR_L1RFOV					.equ	h'00000800
CC_CODEC_ARSR_MICRFOV					.equ	h'00000400
CC_CODEC_ARSR_L2RFOV					.equ	h'00000200
CC_CODEC_ARSR_HTRFOV					.equ	h'00000100
CC_CODEC_ARSR_IOCSRFOV					.equ	h'00000080

CC_CODEC_ARSR_PLRFUN					.equ	h'00000040
CC_CODEC_ARSR_PRRFUN					.equ	h'00000020
CC_CODEC_ARSR_L1RFUN					.equ	h'00000010
CC_CODEC_ARSR_MICRFUN					.equ	h'00000008
CC_CODEC_ARSR_L2RFUN					.equ	h'00000004
CC_CODEC_ARSR_HTRFUN					.equ	h'00000002
CC_CODEC_ARSR_IOCSRFUN					.equ	h'00000001

; AC97 Control Register (ACR)

CC_CODEC_ACR_VS							.equ	h'80000000
CC_CODEC_ACR_RXDMA_EN					.equ	h'00400000
CC_CODEC_ACR_TXDMA_EN					.equ	h'00200000
CC_CODEC_ACR_FCAF						.equ	h'00100000
CC_CODEC_ACR_FCDF						.equ	h'00080000
CC_CODEC_ACR_FSTAF						.equ	h'00040000
CC_CODEC_ACR_FSTDF						.equ	h'00020000
CC_CODEC_ACR_FPLTF						.equ	h'00010000
CC_CODEC_ACR_FPRTF						.equ	h'00008000
CC_CODEC_ACR_FL1TF						.equ	h'00004000
CC_CODEC_ACR_FPCTF						.equ	h'00002000
CC_CODEC_ACR_FPLSTF						.equ	h'00001000
CC_CODEC_ACR_FPRSTF						.equ	h'00000800
CC_CODEC_ACR_FPLETF						.equ	h'00000400
CC_CODEC_ACR_FL2TF						.equ	h'00000200
CC_CODEC_ACR_FHTF						.equ	h'00000100
CC_CODEC_ACR_FIOCTF						.equ	h'00000080
CC_CODEC_ACR_FPLRF						.equ	h'00000040
CC_CODEC_ACR_FPRRF						.equ	h'00000020
CC_CODEC_ACR_FL1RF						.equ	h'00000010
CC_CODEC_ACR_FMRF						.equ	h'00000008
CC_CODEC_ACR_FL2RF						.equ	h'00000004
CC_CODEC_ACR_FHRF						.equ	h'00000002
CC_CODEC_ACR_FIOSRF						.equ	h'00000001

;AC97 TAG Register (ATAGR)

CC_CODEC_ATAGR_CR						.equ	h'80000000

;Slot Request Active Register (SRAR)

CC_CODEC_SRAR_SL12RA					.equ	h'1000
CC_CODEC_SRAR_SL11RA					.equ	h'0800
CC_CODEC_SRAR_SL10RA					.equ	h'0400
CC_CODEC_SRAR_SL9RA						.equ	h'0200
CC_CODEC_SRAR_SL8RA						.equ	h'0100
CC_CODEC_SRAR_SL7RA						.equ	h'0080
CC_CODEC_SRAR_SL6RA						.equ	h'0040
CC_CODEC_SRAR_SL5RA						.equ	h'0020
CC_CODEC_SRAR_SL4RA						.equ	h'0010
CC_CODEC_SRAR_SL3RA						.equ	h'0008

;
; definitions for the Audio Front End on the companion chip (all the registers are 16 bits wide)
;

	.aif ENABLE_HD64464 eq h'00
CC_AFE_REGBASE_463						.equ	HD64463_BASE + HD64463_AFE_OFFSET
	.aendi
CC_AFE_REGBASE							.equ	HD64465_BASE + HD64465_AFE_OFFSET

CC_AFE_RXDB0_OFFSET						.equ	h'0000		; Receive buffer 0 
CC_AFE_RXDB1_OFFSET						.equ	h'0000		; Receive buffer 1 (same offset!) 
CC_AFE_TXDB0_OFFSET						.equ	h'0100		; Transmit buffer 0 
CC_AFE_TXDB1_OFFSET						.equ	h'0100		; Transmit buffer 1 (same offset!) 
CC_AFE_CTR_OFFSET						.equ	h'0200		; Control register 
CC_AFE_STR_OFFSET						.equ	h'0202		; Status register 
CC_AFE_RXDR_OFFSET						.equ	h'0204		; Receive data register 
CC_AFE_TXDR_OFFSET						.equ	h'0206		; Transmit data register 

CC_AFE_REGSIZE							.equ	h'0208		; total size of AFE registers 

CC_AFE_RXDB0							.equ	CC_AFE_REGBASE + CC_AFE_RXDB0_OFFSET	; Receive buffer 0 
CC_AFE_RXDB1							.equ	CC_AFE_REGBASE + CC_AFE_RXDB1_OFFSET	; Receive buffer 1 (same offset!) 
CC_AFE_TXDB0							.equ	CC_AFE_REGBASE + CC_AFE_TXDB0_OFFSET	; Transmit buffer 0 
CC_AFE_TXDB1							.equ	CC_AFE_REGBASE + CC_AFE_TXDB1_OFFSET	; Transmit buffer 1 (same offset!) 
CC_AFE_CTR								.equ	CC_AFE_REGBASE + CC_AFE_CTR_OFFSET		; Control register 
CC_AFE_STR								.equ	CC_AFE_REGBASE + CC_AFE_STR_OFFSET		; Status register 
CC_AFE_RXDR								.equ	CC_AFE_REGBASE + CC_AFE_RXDR_OFFSET		; Receive data register 
CC_AFE_TXDR								.equ	CC_AFE_REGBASE + CC_AFE_TXDR_OFFSET		; Transmit data register 

;
; definitions of DISPLAY interface (DSP) HD64463 BASE
;
	.aif ENABLE_HD64464 eq h'00
CC_DSP_REGBASE							.equ	HD64463_BASE + HD64463_LCDC_OFFSET

CC_DSP_LCDCBAR_OFFSET					.equ	h'0000		; Base Address Register Offset Address 
CC_DSP_LCDCLOR_OFFSET					.equ	h'0002		; Line address offset register  Offset Address
CC_DSP_LCDCCR_OFFSET					.equ	h'0004		; Control Register  Offset Address
CC_DSP_LDR1_OFFSET						.equ	h'0010		; LCD Display regsiter 1  Offset Address
CC_DSP_LDR2_OFFSET						.equ	h'0012		; LCD Display register 2  Offset Address
CC_DSP_LDHNCR_OFFSET					.equ	h'0014		; Num Chars in Horz. reg  Offset Address
CC_DSP_LDHNSR_OFFSET					.equ	h'0016		; Start Position of Horz. reg  Offset Address
CC_DSP_LDVNTR_OFFSET					.equ	h'0018		; Total Vertical Lines reg  Offset Address
CC_DSP_LDVNDR_OFFSET					.equ	h'001A		; Display Vertical Lines reg  Offset Address
CC_DSP_LDVSPR_OFFSET					.equ	h'001C		; Vertical Synchronous Pos. Reg  Offset Address
CC_DSP_LDR3_OFFSET						.equ	h'001E		; LCD Display register 3  Offset Address
CC_DSP_CRTVTR_OFFSET					.equ	h'0020		; CRT Vertical Total Register  Offset Address
CC_DSP_CRTVRSR_OFFSET					.equ	h'0022		; CRT Vertical Retrace Start Register  Offset Address
CC_DSP_CRTVRER_OFFSET					.equ	h'0024		; CRT Vertical Retrace End Register  Offset Address
CC_DSP_CPTWAR_OFFSET					.equ	h'0030		; Color Palette Write Addr Reg  Offset Address
CC_DSP_CPTWDR_OFFSET					.equ	h'0032		; Color Palette Write Data Reg  Offset Address
CC_DSP_CPTRAR_OFFSET					.equ	h'0034		; Color Palette Read Address Reg  Offset Address
CC_DSP_CPTRDR_OFFSET					.equ	h'0036		; Color Palette Read data reg  Offset Address

CC_DSP_REGSIZE							.equ	h'0038		; total size of DSP ASIC regs 

CC_DSP_LCDCBAR							.equ	CC_DSP_REGBASE + CC_DSP_LCDCBAR_OFFSET	; Base Address Register 
CC_DSP_LCDCLOR							.equ	CC_DSP_REGBASE + CC_DSP_LCDCLOR_OFFSET	; Line address offset register 
CC_DSP_LCDCCR							.equ	CC_DSP_REGBASE + CC_DSP_LCDCCR_OFFSET	; Control Register 
CC_DSP_LDR1								.equ	CC_DSP_REGBASE + CC_DSP_LDR1_OFFSET		; LCD Display regsiter 1 
CC_DSP_LDR2								.equ	CC_DSP_REGBASE + CC_DSP_LDR2_OFFSET		; LCD Display register 2 
CC_DSP_LDHNCR							.equ	CC_DSP_REGBASE + CC_DSP_LDHNCR_OFFSET	; Num Chars in Horz. reg 
CC_DSP_LDHNSR							.equ	CC_DSP_REGBASE + CC_DSP_LDHNSR_OFFSET	; Start Position of Horz. reg 
CC_DSP_LDVNTR							.equ	CC_DSP_REGBASE + CC_DSP_LDVNTR_OFFSET	; Total Vertical Lines reg 
CC_DSP_LDVNDR							.equ	CC_DSP_REGBASE + CC_DSP_LDVNDR_OFFSET	; Display Vertical Lines reg 
CC_DSP_LDVSPR							.equ	CC_DSP_REGBASE + CC_DSP_LDVSPR_OFFSET	; Vertical Synchronous Pos. Reg 
CC_DSP_LDR3								.equ	CC_DSP_REGBASE + CC_DSP_LDR3_OFFSET		; LCD Display register 3 
CC_DSP_CRTVTR							.equ	CC_DSP_REGBASE + CC_DSP_CRTVTR_OFFSET	; CRT Vertical Total Register 
CC_DSP_CRTVRSR							.equ	CC_DSP_REGBASE + CC_DSP_CRTVRSR_OFFSET	; CRT Vertical Retrace Start Register 
CC_DSP_CRTVRER							.equ	CC_DSP_REGBASE + CC_DSP_CRTVRER_OFFSET	; CRT Vertical Retrace End Register 
CC_DSP_CPTWAR							.equ	CC_DSP_REGBASE + CC_DSP_CPTWAR_OFFSET	; Color Palette Write Addr Reg 
CC_DSP_CPTWDR							.equ	CC_DSP_REGBASE + CC_DSP_CPTWDR_OFFSET	; Color Palette Write Data Reg 
CC_DSP_CPTRAR							.equ	CC_DSP_REGBASE + CC_DSP_CPTRAR_OFFSET	; Color Palette Read Address Reg 
CC_DSP_CPTRDR							.equ	CC_DSP_REGBASE + CC_DSP_CPTRDR_OFFSET	; Color Palette Read data reg 
	.aendi

	.aif ENABLE_HD64464 eq h'01
HD64464_OFFSET							.equ	h'03E00000
CC_DSP_REGBASE							.equ	(HD64464_BASE + HD64464_OFFSET)

CC_DSP_PM_OFFSET						.equ	h'00000000
CC_DSP_CC_OFFSET						.equ	h'00002000
CC_DSP_MM_OFFSET						.equ	h'00004000
CC_DSP_IN_OFFSET						.equ	h'00014000
CC_DSP_GC_OFFSET						.equ	h'0001E000
CC_DSP_GE_OFFSET						.equ	h'00020000
CC_DSP_FP_OFFSET						.equ	h'00022000
CC_DSP_CP1_OFFSET						.equ	h'00024000
CC_DSP_CP2_OFFSET						.equ	h'00026000
CC_DSP_CR_OFFSET						.equ	h'0002A000
CC_DSP_REGSIZE							.equ	h'00030000

CC_DSP_PM								.equ	(CC_DSP_REGBASE + CC_DSP_PM_OFFSET)	; Power Management + Clock Generation
CC_DSP_CC								.equ	(CC_DSP_REGBASE + CC_DSP_CC_OFFSET)	; CPU Interface
CC_DSP_MM								.equ	(CC_DSP_REGBASE + CC_DSP_MM_OFFSET)	; Memory Interface
CC_DSP_IN								.equ	(CC_DSP_REGBASE + CC_DSP_IN_OFFSET)	; Interrupt Controller
CC_DSP_GC								.equ	(CC_DSP_REGBASE + CC_DSP_GC_OFFSET)	; Graphics Controller 1 and 2
CC_DSP_GE								.equ	(CC_DSP_REGBASE + CC_DSP_GE_OFFSET)	; Graphics Engine
CC_DSP_FP								.equ	(CC_DSP_REGBASE + CC_DSP_FP_OFFSET)	; Flat Panel Controller
CC_DSP_CP1								.equ	(CC_DSP_REGBASE + CC_DSP_CP1_OFFSET); Color Palette 1
CC_DSP_CP2								.equ	(CC_DSP_REGBASE + CC_DSP_CP2_OFFSET); Color Palette 2
CC_DSP_CR								.equ	(CC_DSP_REGBASE + CC_DSP_CR_OFFSET)	; Configuration Registers
	.aendi

	.aif ENABLE_HD64464 eq h'01
CC_KBC_REGBASE							.equ	(HD64465_BASE + HD64465_KBC_OFFSET)

CC_KBC_CR_OFFSET						.equ	0x800
CC_KBC_SR_OFFSET						.equ	0x802

CC_KBC_CR								.equ	(CC_KBC_REGBASE + CC_KBC_CR_OFFSET)
CC_KBC_SR								.equ	(CC_KBC_REGBASE + CC_KBC_SR_OFFSET)
	.aendi
