;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;++
;
; Module Name:
;
;    bvd1regs.inc
;
; Abstract:
;
;    Intel Bulverde CPU register addresses and register field definitions.
;
; Environment:
;
; Revision History:
;
;--
;
; Copyright © 2002-2003 Intel Corp.
;

    IF  !:DEF: _bvd1regs_inc_
_bvd1regs_inc_    EQU 1


; ////////////////////////////////////////////////////
; /* DEVICE BASE ADDRESSES GROUPED BY FUNCTIONALITY */
; ////////////////////////////////////////////////////

;
; Internal Memory - Storage (256 KB)
;
BULVERDE_BASE_REG_PA_IMSTORAGE                   EQU 0x5C000000

;
; Internal Memory - Control (12 B)
;
BULVERDE_BASE_REG_PA_IMCONTROL                   EQU 0x58000000

;
; Camera Peripheral
;
BULVERDE_BASE_REG_PA_CAMERA                      EQU 0x50000000

;
; USB Host
;
BULVERDE_BASE_REG_PA_USBH                        EQU 0x4C000000

;
;  MEMC
;
BULVERDE_BASE_REG_PA_MEMC                        EQU 0x48000000

;
; LCDC
;
BULVERDE_BASE_REG_PA_LCD                         EQU 0x44000000

;
; Peripheral registers base - DMAC, UART[3:1/SIR], I2S/C, AC97, USBC, FIR, 
; RTC, OST, PWM, INTC, GPIO, PWRMAN/RESETC, SSP[3:1], MMC, CLKMAN, BB, KEYPAD, 
; USIM, MEMSTICK
;
BULVERDE_BASE_REG_PA_PERIPH                      EQU 0x40000000

;
; PCMCIA Slots 0,1
;
BULVERDE_BASE_REG_PA_PCMCIA_S0_IO                EQU 0x20000000
BULVERDE_BASE_REG_PA_PCMCIA_S0_ATTR              EQU 0x28000000
BULVERDE_BASE_REG_PA_PCMCIA_S0_CMN               EQU 0x2C000000
BULVERDE_BASE_REG_PA_PCMCIA_S1_IO                EQU 0x30000000
BULVERDE_BASE_REG_PA_PCMCIA_S1_ATTR              EQU 0x38000000
BULVERDE_BASE_REG_PA_PCMCIA_S1_CMN               EQU 0x3C000000


; /////////////////////////////////////////////////////////////////////////////////////////
; /* Peripheral register offsets */
; /////////////////////////////////////////////////////////////////////////////////////////


;//
;//  mmr-perifs
;//
DMAC_OFFSET                     EQU     0x0             ; DMA CONTROLLER
FFUART_OFFSET                   EQU     0x00100000      ; Full-Feature UART
BTUART_OFFSET                   EQU     0x00200000      ; BlueTooth UART
I2C_OFFSET                      EQU     0x00300000      ; I2C
I2S_OFFSET                      EQU     0x00400000      ; I2S
AC97_OFFSET                     EQU     0x00500000      ; AC97
UDC_OFFSET                      EQU     0x00600000      ; UDC (usb client)
STUART_OFFSET                   EQU     0x00700000      ; Standard UART
FIR_OFFSET                      EQU     0x00800000      ; FIR
RTC_OFFSET                      EQU     0x00900000      ; real time clock
OST_OFFSET                      EQU     0x00A00000      ; OS Timer
PWM0_2_OFFSET                   EQU     0x00B00000      ; PWM 0 (pulse-width mod)
PWM1_3_OFFSET                   EQU     0x00C00000      ; PWM 1 (pulse-width mod)
INTC_OFFSET                     EQU     0x00D00000      ; Interrupt controller
GPIO_OFFSET                     EQU     0x00E00000      ; GPIO
PWR_OFFSET                      EQU     0x00F00000      ; Power Manager and Reset Control
SSP1_OFFSET                     EQU     0x01000000      ; SSP 1
MMC_OFFSET                      EQU     0x01100000      ; MMC
CLKMGR_OFFSET                   EQU     0x01300000      ; Clock Manager
BB_OFFSET                       EQU     0x01400000      ; Baseband Interface
KEYPAD_OFFSET                   EQU     0x01500000      ; Keypad Interface
USIM_OFFSET                     EQU     0x01600000      ; USIM
SSP2_OFFSET                     EQU     0x01700000      ; SSP 2
MEMSTK_OFFSET                   EQU     0x01800000      ; Memory Stick
SSP3_OFFSET                     EQU     0x01900000      ; SSP 3

; /////////////////////////////////////////////////////////////////////////////////////////
; /* Relevant register-specific offsets */
; /////////////////////////////////////////////////////////////////////////////////////////

;//
;// MEMC
;//
MDCNFG_OFFSET                   EQU     0x0
MDREFR_OFFSET                   EQU     0x4
MSC0_OFFSET                     EQU     0x8
MSC1_OFFSET                     EQU     0xC
MSC2_OFFSET                     EQU     0x10
MECR_OFFSET                     EQU     0x14
SXCNFG_OFFSET                   EQU     0x1C
FLYCNFG_OFFSET                  EQU     0x20
MCMEM0_OFFSET                   EQU     0x28
MCMEM1_OFFSET                   EQU     0x2C
MCATT0_OFFSET                   EQU     0x30
MCATT1_OFFSET                   EQU     0x34
MCIO0_OFFSET                    EQU     0x38
MCIO1_OFFSET                    EQU     0x3C
MDMRS_OFFSET                    EQU     0x40
BOOT_DEF_OFFSET                 EQU     0x44
ARB_CNTL_OFFSET                 EQU     0x48
BSCNTR0_OFFSET                  EQU     0x4C
BSCNTR1_OFFSET                  EQU     0x50
LCDBSCNTR_OFFSET                EQU     0x54
MDMRSLP_OFFSET                  EQU     0x58
BSCNTR2_OFFSET                  EQU     0x5C
BSCNTR3_OFFSET                  EQU     0x60

;//
;// FULL-FEATURE UART
;//
FF_THR_OFFSET                   EQU     0x0       ;DLAB = 0  WO  8bit - Transmit Holding Register
FF_RBR_OFFSET                   EQU     0x0       ;DLAB = 0  RO  8bit - Receive Buffer Register
FF_DLL_OFFSET                   EQU     0x0       ;DLAB = 1  RW  8bit - Divisor Latch Low Register
FF_IER_OFFSET                   EQU     0x4       ;DLAB = 0  RW  8bit - Interrupt Enable Register
FF_DLH_OFFSET                   EQU     0x4       ;DLAB = 1  RW  8bit - Divisor Latch High Register
FF_IIR_OFFSET                   EQU     0x8       ;DLAB = X  RO  8bit - Interrupt Identification Register
FF_FCR_OFFSET                   EQU     0x8       ;DLAB = X  WO  8bit - FIFO Control Register
FF_LCR_OFFSET                   EQU     0xC       ;DLAB = X  RW  8bit - Line Control Register
FF_MCR_OFFSET                   EQU     0x10      ;DLAB = X  RW  8bit - Modem Control Regiser
FF_LSR_OFFSET                   EQU     0x14      ;DLAB = X  RO  8bit - Line Status Register
FF_MSR_OFFSET                   EQU     0x18      ;DLAB = X  RO  8bit - Modem Status Register
FF_SPR_OFFSET                   EQU     0x1C      ;DLAB = X  RW  8bit - Scratchpad Register
FF_ISR_OFFSET                   EQU     0x20      ;DLAB = X  RW  8bit - Slow Infrared Select Register
FF_FOR_OFFSET                   EQU     0x24      ;DLAB = X  RO  FIFO Occupancy Register
FF_ABR_OFFSET                   EQU     0x28      ;DLAB = X  RW  Autobaud Control Register
FF_ACR_OFFSET                   EQU     0x2C      ;DLAB = X Autobaud Count Register

;//
;// BLUETOOTH UART
;//
BT_THR_OFFSET                   EQU     0x0       ;DLAB = 0  WO  8bit - Transmit Holding Register
BT_RBR_OFFSET                   EQU     0x0       ;DLAB = 0  RO  8bit - Receive Buffer Register
BT_DLL_OFFSET                   EQU     0x0       ;DLAB = 1  RW  8bit - Divisor Latch Low Register
BT_IER_OFFSET                   EQU     0x4       ;DLAB = 0  RW  8bit - Interrupt Enable Register
BT_DLH_OFFSET                   EQU     0x4       ;DLAB = 1  RW  8bit - Divisor Latch High Register
BT_IIR_OFFSET                   EQU     0x8       ;DLAB = X  RO  8bit - Interrupt Identification Register
BT_FCR_OFFSET                   EQU     0x8       ;DLAB = X  WO  8bit - FIFO Control Register
BT_LCR_OFFSET                   EQU     0xC       ;DLAB = X  RW  8bit - Line Control Register
BT_MCR_OFFSET                   EQU     0x10      ;DLAB = X  RW  8bit - Modem Control Regiser
BT_LSR_OFFSET                   EQU     0x14      ;DLAB = X  RO  8bit - Line Status Register
BT_MSR_OFFSET                   EQU     0x18      ;DLAB = X  RO  8bit - Modem Status Register
BT_SPR_OFFSET                   EQU     0x1C      ;DLAB = X  RW  8bit - Scratchpad Register
BT_ISR_OFFSET                   EQU     0x20      ;DLAB = X  RW  8bit - Slow Infrared Select Register
BT_FOR_OFFSET                   EQU     0x24      ;DLAB = X  RO  FIFO Occupancy Register
BT_ABR_OFFSET                   EQU     0x28      ;DLAB = X  RW  Autobaud Control Register
BT_ACR_OFFSET                   EQU     0x2C      ;DLAB = X Autobaud Count Register

;//
;// STANDARD UART
;//
ST_THR_OFFSET                   EQU     0x0       ;DLAB = 0  WO  8bit - Transmit Holding Register
ST_RBR_OFFSET                   EQU     0x0       ;DLAB = 0  RO  8bit - Receive Buffer Register
ST_DLL_OFFSET                   EQU     0x0       ;DLAB = 1  RW  8bit - Divisor Latch Low Register
ST_IER_OFFSET                   EQU     0x4       ;DLAB = 0  RW  8bit - Interrupt Enable Register
ST_DLH_OFFSET                   EQU     0x4       ;DLAB = 1  RW  8bit - Divisor Latch High Register
ST_IIR_OFFSET                   EQU     0x8       ;DLAB = X  RO  8bit - Interrupt Identification Register
ST_FCR_OFFSET                   EQU     0x8       ;DLAB = X  WO  8bit - FIFO Control Register
ST_LCR_OFFSET                   EQU     0xC       ;DLAB = X  RW  8bit - Line Control Register
ST_MCR_OFFSET                   EQU     0x10      ;DLAB = X  RW  8bit - Modem Control Regiser
ST_LSR_OFFSET                   EQU     0x14      ;DLAB = X  RO  8bit - Line Status Register
ST_MSR_OFFSET                   EQU     0x18      ;DLAB = X  RO  8bit - Modem Status Register
ST_SPR_OFFSET                   EQU     0x1C      ;DLAB = X  RW  8bit - Scratchpad Register
ST_ISR_OFFSET                   EQU     0x20      ;DLAB = X  RW  8bit - Slow Infrared Select Register
ST_FOR_OFFSET                   EQU     0x24      ;DLAB = X  RO  FIFO Occupancy Register
ST_ABR_OFFSET                   EQU     0x28      ;DLAB = X  RW  Autobaud Control Register
ST_ACR_OFFSET                   EQU     0x2C      ;DLAB = X Autobaud Count Register

;//
;// RTC
;//
RCNR_OFFSET                     EQU     0x0       ;RTC count register
RTAR_OFFSET                     EQU     0x4       ;RTC alarm register
RTSR_OFFSET                     EQU     0x8       ;RTC status register
RTTR_OFFSET                     EQU     0xC       ;RTC timer trim register
RDCR_OFFSET                     EQU     0x10      ;RTC Day Counter
RYCR_OFFSET                     EQU     0x14      ;RTC Year Counter
RDAR1_OFFSET                    EQU     0x18      ;RTC Day Alarm 1
RYAR1_OFFSET                    EQU     0x1C      ;RTC Year Alarm 1
RDAR2_OFFSET                    EQU     0x20      ;RTC Day Alarm 2
RYAR2_OFFSET                    EQU     0x24      ;RTC Year Alarm 2
SWCR_OFFSET                     EQU     0x28      ;RTC Stopwatch Counter
SWAR1_OFFSET                    EQU     0x2C      ;RTC Stopwatch Alarm 1
SWAR2_OFFSET                    EQU     0x30      ;RTC Stopwatch Alarm 2
PICR_OFFSET                     EQU     0x34      ;RTC Periodic Interrupt Counter
PIAR_OFFSET                     EQU     0x38      ;RTC Periodic Interrupt Alarm


;//
;// OST (OS TIMER)
;//
OSMR0_OFFSET                    EQU     0x0       ;OS timer match register 0
OSMR1_OFFSET                    EQU     0x4       ;OS timer match register 1
OSMR2_OFFSET                    EQU     0x8       ;OS timer match register 2
OSMR3_OFFSET                    EQU     0xC       ;OS timer match register 3
OSCR0_OFFSET                    EQU     0x10      ;OS timer counter register 0
OSSR_OFFSET                     EQU     0x14      ;OS timer status register
OWER_OFFSET                     EQU     0x18      ;OS timer watchdog enable register
OIER_OFFSET                     EQU     0x1C      ;OS timer interrupt enable register
OSCR4_OFFSET                    EQU     0x40
OSCR5_OFFSET                    EQU     0x44
OSCR6_OFFSET                    EQU     0x48
OSCR7_OFFSET                    EQU     0x4C
OSCR8_OFFSET                    EQU     0x50
OSCR9_OFFSET                    EQU     0x54
OSCR10_OFFSET                   EQU     0x58
OSCR11_OFFSET                   EQU     0x5C
OSMR4_OFFSET                    EQU     0x80
OSMR5_OFFSET                    EQU     0x84
OSMR6_OFFSET                    EQU     0x88
OSMR7_OFFSET                    EQU     0x8C
OSMR8_OFFSET                    EQU     0x90
OSMR9_OFFSET                    EQU     0x94
OSMR10_OFFSET                   EQU     0x98
OSMR11_OFFSET                   EQU     0x9C
OMCR4_OFFSET                    EQU     0xC0
OMCR5_OFFSET                    EQU     0xC4
OMCR6_OFFSET                    EQU     0xC8
OMCR7_OFFSET                    EQU     0xCC
OMCR8_OFFSET                    EQU     0xD0
OMCR9_OFFSET                    EQU     0xD4
OMCR10_OFFSET                   EQU     0xD8
OMCR11_OFFSET                   EQU     0xDC


;//
;// INTC (INTERRUPT CONTROLLER) - Memory-mapped addresses (can also use c-proc for most of these)
;//
ICIP_OFFSET                     EQU     0x0       ;Interrupt controller IRQ pending register
ICMR_OFFSET                     EQU     0x4       ;Interrupt controller mask register
ICLR_OFFSET                     EQU     0x8       ;Interrupt controller level register
ICFP_OFFSET                     EQU     0xC       ;Interrupt controller FIQ pending register
ICPR_OFFSET                     EQU     0x10      ;Interrupt controller pending register
ICCR_OFFSET                     EQU     0x14      ;Interrupt controller control register
ICHP_OFFSET                     EQU     0x18      ;Interrupt controller Highest Priority register
IPR0_OFFSET                     EQU     0x1C      ;Interrupt controller Priority registerS [31:0]
IPR1_OFFSET                     EQU     0x20
IPR2_OFFSET                     EQU     0x24
IPR3_OFFSET                     EQU     0x28
IPR4_OFFSET                     EQU     0x2C
IPR5_OFFSET                     EQU     0x30
IPR6_OFFSET                     EQU     0x34
IPR7_OFFSET                     EQU     0x38
IPR8_OFFSET                     EQU     0x3C
IPR9_OFFSET                     EQU     0x40
IPR10_OFFSET                    EQU     0x44
IPR11_OFFSET                    EQU     0x48
IPR12_OFFSET                    EQU     0x4C
IPR13_OFFSET                    EQU     0x50
IPR14_OFFSET                    EQU     0x54
IPR15_OFFSET                    EQU     0x58
IPR16_OFFSET                    EQU     0x5C
IPR17_OFFSET                    EQU     0x60
IPR18_OFFSET                    EQU     0x64
IPR19_OFFSET                    EQU     0x68
IPR20_OFFSET                    EQU     0x6C
IPR21_OFFSET                    EQU     0x70
IPR22_OFFSET                    EQU     0x74
IPR23_OFFSET                    EQU     0x78
IPR24_OFFSET                    EQU     0x7C
IPR25_OFFSET                    EQU     0x80
IPR26_OFFSET                    EQU     0x84
IPR27_OFFSET                    EQU     0x88
IPR28_OFFSET                    EQU     0x8C
IPR29_OFFSET                    EQU     0x90
IPR30_OFFSET                    EQU     0x94
IPR31_OFFSET                    EQU     0x98

;//
;// GPIO
;//
GPLR0_OFFSET                    EQU     0x0       ;GPIO pin-level register 31:0
GPLR1_OFFSET                    EQU     0x4       ;GPIO pin-level register 63:32
GPLR2_OFFSET                    EQU     0x8       ;GPIO pin-level register 95:64
GPDR0_OFFSET                    EQU     0xC       ;GPIO pin-direction register 31:0
GPDR1_OFFSET                    EQU     0x10      ;GPIO pin-direction register 63:32
GPDR2_OFFSET                    EQU     0x14      ;GPIO pin-direction register 95:64
GPSR0_OFFSET                    EQU     0x18      ;GPIO pin output set register 31:0
GPSR1_OFFSET                    EQU     0x1C      ;GPIO pin output set register 63:32
GPSR2_OFFSET                    EQU     0x20      ;GPIO pin output set register 95:64
GPCR0_OFFSET                    EQU     0x24      ;GPIO pin output clear register 31:0
GPCR1_OFFSET                    EQU     0x28      ;GPIO pin output clear register 63:32
GPCR2_OFFSET                    EQU     0x2C      ;GPIO pin output clear register 95:64
GRER0_OFFSET                    EQU     0x30      ;GPIO rising edge detect register 31:0
GRER1_OFFSET                    EQU     0x34      ;GPIO rising edge detect register 63:32
GRER2_OFFSET                    EQU     0x38      ;GPIO rising edge detect register 95:64
GFER0_OFFSET                    EQU     0x3C      ;GPIO falling edge detect register 31:0
GFER1_OFFSET                    EQU     0x40      ;GPIO falling edge detect register 63:32
GFER2_OFFSET                    EQU     0x44      ;GPIO falling edge detect register 95:64
GEDR0_OFFSET                    EQU     0x48      ;GPIO edge detect status register 31:0
GEDR1_OFFSET                    EQU     0x4C      ;GPIO edge detect status register 63:32
GEDR2_OFFSET                    EQU     0x50      ;GPIO edge detect status register 95:64
GAFR0_L_OFFSET                  EQU     0x54      ;GPIO alternate funciton select register 15:0
GAFR0_U_OFFSET                  EQU     0x58      ;GPIO alternate function select register 31:16
GAFR1_L_OFFSET                  EQU     0x5C      ;GPIO alternate function select register 47:32
GAFR1_U_OFFSET                  EQU     0x60      ;GPIO alternate function select register 63:48
GAFR2_L_OFFSET                  EQU     0x64      ;GPIO alternate function select register 79:64
GAFR2_U_OFFSET                  EQU     0x68      ;GPIO alternate function select register 95:80
GAFR3_L_OFFSET                  EQU     0x6C      ;GPIO alternate function select register 111:96
GAFR3_U_OFFSET                  EQU     0x70      ;GPIO alternate function select register 120:112
GPLR3_OFFSET                    EQU     0x100     ;GPIO pin-level register 120:96
GPDR3_OFFSET                    EQU     0x10C     ;GPIO pin-direction register 120:96
GPSR3_OFFSET                    EQU     0x118     ;GPIO pin output set register 120:96
GPCR3_OFFSET                    EQU     0x124     ;GPIO pin output clear register 120:96
GRER3_OFFSET                    EQU     0x130     ;GPIO rising edge detect register 120:96
GFER3_OFFSET                    EQU     0x13C     ;GPIO falling edge detect register 120:96
GEDR3_OFFSET                    EQU     0x148     ;GPIO edge detect status register 120:96


;//
;// POWER MANAGER & RESET CONTROLLER
;//
PMCR_OFFSET                     EQU     0x0       ;Power manager control register
PSSR_OFFSET                     EQU     0x4       ;Power manager sleep status register
PSPR_OFFSET                     EQU     0x8       ;Power manager scratch pad register
PWER_OFFSET                     EQU     0xC       ;Power manager wake-up enable register
PRER_OFFSET                     EQU     0x10      ;Power manager GPIO rising edge detect enable register
PFER_OFFSET                     EQU     0x14      ;Power manager GPIO falling edge detect enable register
PEDR_OFFSET                     EQU     0x18      ;Power manager GPIO edge detect status register
PCFR_OFFSET                     EQU     0x1C      ;Power manager general configuration register
PGSR0_OFFSET                    EQU     0x20      ;Power manager GPIO sleep state register for GPIO 31:0
PGSR1_OFFSET                    EQU     0x24      ;Power manager GPIO sleep state register for GPIO 63:32
PGSR2_OFFSET                    EQU     0x28      ;Power manager GPIO sleep state register for GPIO 95:64
PGSR3_OFFSET                    EQU     0x2C      ;Power manager GPIO sleep state register for GPIO 120:96
RCSR_OFFSET                     EQU     0x30      ; **Reset controller status register**
PSLR_OFFSET                     EQU     0x34      ;Power manager Sleep Mode Config
PSTR_OFFSET                     EQU     0x38      ;Power manager Standby Mode Config
PSNR_OFFSET                     EQU     0x3C      ;Power manager Sense Mode Config
PVCR_OFFSET                     EQU     0x40      ;Power manager Voltage Change Control
PCMD0_OFFSET                    EQU     0x80      ;Power manager I2C Command[31:0]
PCMD1_OFFSET                    EQU     0x84
PCMD2_OFFSET                    EQU     0x88
PCMD3_OFFSET                    EQU     0x8C
PCMD4_OFFSET                    EQU     0x90
PCMD5_OFFSET                    EQU     0x94
PCMD6_OFFSET                    EQU     0x98
PCMD7_OFFSET                    EQU     0x9C
PCMD8_OFFSET                    EQU     0xA0
PCMD9_OFFSET                    EQU     0xA4
PCMD10_OFFSET                   EQU     0xA8
PCMD11_OFFSET                   EQU     0xAC
PCMD12_OFFSET                   EQU     0xB0
PCMD13_OFFSET                   EQU     0xB4
PCMD14_OFFSET                   EQU     0xB8
PCMD15_OFFSET                   EQU     0xBC
PCMD16_OFFSET                   EQU     0xC0
PCMD17_OFFSET                   EQU     0xC4
PCMD18_OFFSET                   EQU     0xC8
PCMD19_OFFSET                   EQU     0xCC
PCMD20_OFFSET                   EQU     0xD0
PCMD21_OFFSET                   EQU     0xD4
PCMD22_OFFSET                   EQU     0xD8
PCMD23_OFFSET                   EQU     0xDC
PCMD24_OFFSET                   EQU     0xE0
PCMD25_OFFSET                   EQU     0xE4
PCMD26_OFFSET                   EQU     0xE8
PCMD27_OFFSET                   EQU     0xEC
PCMD28_OFFSET                   EQU     0xF0
PCMD29_OFFSET                   EQU     0xF4
PCMD30_OFFSET                   EQU     0xF8
PCMD31_OFFSET                   EQU     0xFC
PIBMR_OFFSET                    EQU     0x180     ;Power manager I2C Bus Monitor
PIDBR_OFFSET                    EQU     0x188     ;Power manager I2C Data Buffer
PI2CR_OFFSET                    EQU     0x190     ;Power manager I2C Control
PISR_OFFSET                     EQU     0x198     ;Power manager I2C Status
PISAR_OFFSET                    EQU     0x1A0     ;Power manager I2C Slave Address


;//
;// CLK MANAGER
;//
CCCR_OFFSET                     EQU     0x0       ;Core Clock Configuration Register
CKEN_OFFSET                     EQU     0x4       ;Clock Enable Register
OSCC_OFFSET                     EQU     0x8       ;Oscillator Configuration Register
CCSR_OFFSET                     EQU     0xC       ;Core Clock Status


; /////////////////////////////////////////////////////////////////////////////////////////
; /* Peripheral-specific base addresses */
; /////////////////////////////////////////////////////////////////////////////////////////

BULVERDE_BASE_REG_PA_FFUART     EQU     (BULVERDE_BASE_REG_PA_PERIPH + FFUART_OFFSET)
BULVERDE_BASE_REG_PA_BTUART     EQU     (BULVERDE_BASE_REG_PA_PERIPH + BTUART_OFFSET)
BULVERDE_BASE_REG_PA_STUART     EQU     (BULVERDE_BASE_REG_PA_PERIPH + STUART_OFFSET)
BULVERDE_BASE_REG_PA_RTC        EQU     (BULVERDE_BASE_REG_PA_PERIPH + RTC_OFFSET)
BULVERDE_BASE_REG_PA_OST        EQU     (BULVERDE_BASE_REG_PA_PERIPH + OST_OFFSET)
BULVERDE_BASE_REG_PA_INTC       EQU     (BULVERDE_BASE_REG_PA_PERIPH + INTC_OFFSET)
BULVERDE_BASE_REG_PA_GPIO       EQU     (BULVERDE_BASE_REG_PA_PERIPH + GPIO_OFFSET)
BULVERDE_BASE_REG_PA_PWR        EQU     (BULVERDE_BASE_REG_PA_PERIPH + PWR_OFFSET)
BULVERDE_BASE_REG_PA_CLKMGR     EQU     (BULVERDE_BASE_REG_PA_PERIPH + CLKMGR_OFFSET)


RCSR_ALL                        EQU     0x1F
Mode_SVC                        EQU     0x13
Mode_USR                        EQU     0x10
NoIntsMask                      EQU     0x000000C0
IRQIntsMask                     EQU     0x7F      ; 0=enabled, 1=disabled
IrqFiqEnable                    EQU     0xFFFFFF3F

;
; FLASH constants
;
K3_128Mb_DEVCODE                EQU     0x8806
J3_128Mb_DEVCODE                EQU     0x18
L3_128Mb_DEVCODE                EQU     0x880C

;
; Reset Controller Status Register bit defines
;
RCSR_HARD_RESET                 EQU     (0x1)
RCSR_WDOG_RESET                 EQU     (0x1 << 1)
RCSR_SLEEP_RESET                EQU     (0x1 << 2)
RCSR_GPIO_RESET                 EQU     (0x1 << 3)
PSSR_VALID_MASK                 EQU     (0x3F)
PSSR_RDH                        EQU     (0x1 << 5)
PSSR_PH                         EQU     (0x1 << 4)

;
; Clock Manager Defs
;
OSCC_OOK                        EQU     (0x1)
OSCC_OON                        EQU     (0x1 << 1)
OSCC_TOUT_EN                    EQU     (0x1 << 2)
OSCC_PIO_EN                     EQU     (0x1 << 3)
OSCC_CRI                        EQU     (0x1 << 4)
CKEN_DEFAULT                    EQU     0x00400200        ; MEMC, OST clocked.  Rest OFF

;
;  Power Manager Defs
;
PCFR_OPDE                       EQU     (0x1)
PCFR_FP                         EQU     (0x1 << 1)
PCFR_FS                         EQU     (0x1 << 2)
PCFR_GPR_EN                     EQU     (0x1 << 4)
PCFR_SYSEN_EN                   EQU     (0x1 << 5)
PCFR_PI2C_EN                    EQU     (0x1 << 6)
PCFR_DC_EN                      EQU     (0x1 << 7)
PCFR_FVC                        EQU     (0x1 << 10)
PCFR_L1_EN                      EQU     (0x1 << 11)
PCFR_GP_ROD                     EQU     (0x1 << 12)
PWER_WE0                        EQU     (0x1)
PWER_WE1                        EQU     (0x1 << 1)
PWER_WBB                        EQU     (0x1 << 25)
PWER_WEUSBC                     EQU     (0x1 << 26)
PWER_WEUSBH0                    EQU     (0x1 << 27)
PWER_WEUSBH1                    EQU     (0x1 << 28)
PWER_WEP1                       EQU     (0x1 << 30)
PWER_WERTC                      EQU     (0x1 << 31)
PMCR_BIDAE                      EQU     (0x1)
PMCR_BIDAS                      EQU     (0x1 << 1)
PMCR_VIDAE                      EQU     (0x1 << 2)
PMCR_VIDAS                      EQU     (0x1 << 3)
PMCR_IAS                        EQU     (0x1 << 4)
PMCR_INTRS                      EQU     (0x1 << 5)

;
;  Bits used for Memory Controller Init
;
; register bit masks - mdcnfg
MDCNFG_DE0                      EQU     (0x1 << 0)
MDCNFG_DE1                      EQU     (0x1 << 1)
MDCNFG_DWID0                    EQU     (0x1 << 2)
MDCNFG_DCAC0                    EQU     (0x3 << 3)
MDCNFG_DRAC0                    EQU     (0x3 << 5)
MDCNFG_DNB0                     EQU     (0x1 << 7)
MDCNFG_DTC0                     EQU     (0x3 << 8)
MDCNFG_DADDR0                   EQU     (0x1 << 10)
MDCNFG_DLATCH0                  EQU     (0x1 << 11)
MDCNFG_RESERVED0                EQU     (0xF << 12)
MDCNFG_DE2                      EQU     (0x1 << 16)
MDCNFG_DE3                      EQU     (0x1 << 17)
MDCNFG_DWID2                    EQU     (0x1 << 18)
MDCNFG_DCAC2                    EQU     (0x3 << 19)
MDCNFG_DRAC2                    EQU     (0x3 << 21)
MDCNFG_DNB2                     EQU     (0x1 << 23)
MDCNFG_DTC2                     EQU     (0x3 << 24)
MDCNFG_DADDR2                   EQU     (0x1 << 26)
MDCNFG_DLATCH2                  EQU     (0x1 << 27)
MDCNFG_RESERVED2                EQU     (0xF << 28)

MDREFR_E0PIN                    EQU     0x00001000
MDREFR_K0RUN                    EQU     0x00002000
MDREFR_K1RUN                    EQU     0x00010000
MDREFR_K2RUN                    EQU     0x00040000
MDREFR_SLFRSH                   EQU     0x00400000
MDREFR_E1PIN                    EQU     0x00008000
MDREFR_K1DB2                    EQU     0x00020000    ; run SDCLK[1] @ .5(MClk)
MDREFR_K0DB2                    EQU     0x00004000
MDREFR_K0DB4                    EQU     0x20000000    ; run SDCLK[0] @ .25(MemClk)
MDREFR_K0FREE                   EQU     0x00800000
MDREFR_K1FREE                   EQU     0x01000000
MDREFR_K2FREE                   EQU     0x02000000
MDREFR_APD                      EQU     0x00100000
BANK_SHIFT                      EQU     20

;
;   Bits used for the HWConfig Reg (aka PowerManager.ScratchPad)
;
HWConfig_RESET                  EQU     (0x1  :SHL:  0)
HWCONFIG_DEFAULT                EQU     (0x01155046)
HWConfig_BootromPM              EQU     (0x1  :SHL:  12)
PAGEMODE_ON                     EQU     (0x1)

;
;   Bits used for CP 15
;
CONTROL_MMU                     EQU     0x00000001

    ENDIF ; !:DEF: _bvd1regs_inc_
    
    END
