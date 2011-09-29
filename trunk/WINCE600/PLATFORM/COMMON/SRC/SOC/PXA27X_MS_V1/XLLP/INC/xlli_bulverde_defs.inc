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
;*********************************************************************************
;
;        COPYRIGHT (c) 2002 Intel Corporation
;
;   The information in this file is furnished for informational use only,
;   is subject to change without notice, and should not be construed as
;   a commitment by Intel Corporation. Intel Corporation assumes no
;   responsibility or liability for any errors or inaccuracies that may appear
;   in this document or any software that may be provided in association with
;   this document.
;
;*********************************************************************************
;
;  FILENAME:       xlli_Bulverde_defs.inc (Core processor address definitions)
;
; LAST MODIFIED:   1-Mar-2006
;
;******************************************************************************
;
;
; Include file for PXA27x Processor based
; Cross Platform Low Level Initialization  (XLLI)
;
;******************************************************************************

;     Bit settings
;
xlli_BIT_0      EQU     0x00000001
xlli_BIT_1      EQU     0x00000002
xlli_BIT_2      EQU     0x00000004
xlli_BIT_3      EQU     0x00000008
xlli_BIT_4      EQU     0x00000010
xlli_BIT_5      EQU     0x00000020
xlli_BIT_6      EQU     0x00000040
xlli_BIT_7      EQU     0x00000080
xlli_BIT_8      EQU     0x00000100
xlli_BIT_9      EQU     0x00000200
xlli_BIT_10     EQU     0x00000400
xlli_BIT_11     EQU     0x00000800
xlli_BIT_12     EQU     0x00001000
xlli_BIT_13     EQU     0x00002000
xlli_BIT_14     EQU     0x00004000
xlli_BIT_15     EQU     0x00008000
xlli_BIT_16     EQU     0x00010000
xlli_BIT_17     EQU     0x00020000
xlli_BIT_18     EQU     0x00040000
xlli_BIT_19     EQU     0x00080000
xlli_BIT_20     EQU     0x00100000
xlli_BIT_21     EQU     0x00200000
xlli_BIT_22     EQU     0x00400000
xlli_BIT_23     EQU     0x00800000
xlli_BIT_24     EQU     0x01000000
xlli_BIT_25     EQU     0x02000000
xlli_BIT_26     EQU     0x04000000
xlli_BIT_27     EQU     0x08000000
xlli_BIT_28     EQU     0x10000000
xlli_BIT_29     EQU     0x20000000
xlli_BIT_30     EQU     0x40000000
xlli_BIT_31     EQU     0x80000000

;
;  Processor stepping Values
;
xlli_PXA27x_CP15_A0_Val  EQU (0x69054110)
xlli_PXA27x_CP15_A1_Val  EQU (0x69054111)
xlli_PXA27x_CP15_B0_Val  EQU (0x69054112)
xlli_PXA27x_CP15_B1_Val  EQU (0x69054113)
xlli_PXA27x_CP15_C0_Val  EQU (0x69054114)
xlli_PXA27x_CP15_C5_Val  EQU (0x69054117)

xlli_PXA27x_JTAG_A0_Val  EQU (0x09265013)
xlli_PXA27x_JTAG_A1_Val  EQU (0x19265013)
xlli_PXA27x_JTAG_B0_Val  EQU (0x29265013)
xlli_PXA27x_JTAG_B1_Val  EQU (0x39265013)
xlli_PXA27x_JTAG_C0_Val  EQU (0x49265013)
xlli_PXA27x_JTAG_C5_Val  EQU (0x79265013)

xlli_PXA27x_A0_stepping  EQU (0x0)
xlli_PXA27x_A1_stepping  EQU (0x1)
xlli_PXA27x_B0_stepping  EQU (0x2)
xlli_PXA27x_B1_stepping  EQU (0x3)
xlli_PXA27x_C0_stepping  EQU (0x4)
xlli_PXA27x_C5_stepping  EQU (0x7)


;
; GENERAL PURPOSE I/O (GPIO) base address and register offsets from the base address
;

xlli_GPIOREGS_PHYSICAL_BASE  EQU    0x40E00000

; GPIO register offsets from the base address

xlli_GPLR0_offset    EQU    (0x000)  ; GPIO Level registers
xlli_GPLR1_offset    EQU    (0x004)
xlli_GPLR2_offset    EQU    (0x008)
xlli_GPLR3_offset    EQU    (0x100)

xlli_GPDR0_offset    EQU    (0x00C)  ; GPIO Direction registers
xlli_GPDR1_offset    EQU    (0x010)
xlli_GPDR2_offset    EQU    (0x014)
xlli_GPDR3_offset    EQU    (0x10C)

xlli_GPSR0_offset    EQU    (0x018)  ; GPIO Set registers
xlli_GPSR1_offset    EQU    (0x01C)
xlli_GPSR2_offset    EQU    (0x020)
xlli_GPSR3_offset    EQU    (0x118)

xlli_GPCR0_offset    EQU    (0x024)  ; GPIO Clear registers
xlli_GPCR1_offset    EQU    (0x028)
xlli_GPCR2_offset    EQU    (0x02C)
xlli_GPCR3_offset    EQU    (0x124)

xlli_GAFR0_L_offset  EQU    (0x054)  ; GPIO Alternate function registers (Bits 15:0)
xlli_GAFR0_U_offset  EQU    (0x058)  ; Bits 31:16
xlli_GAFR1_L_offset  EQU    (0x05c)  ; Bits 47:32
xlli_GAFR1_U_offset  EQU    (0x060)  ; Bits 63:48
xlli_GAFR2_L_offset  EQU    (0x064)  ; Bits 79:64
xlli_GAFR2_U_offset  EQU    (0x068)  ; Bits 95:80
xlli_GAFR3_L_offset  EQU    (0x06C)  ; Bits 111:96
xlli_GAFR3_U_offset  EQU    (0x070)  ; Bits 127:112

;
; GPIO register bit masks
;
xlli_GPIO_BIT_FFRXD    EQU      (xlli_BIT_2)
xlli_GPIO_BIT_FFCTS    EQU      (xlli_BIT_3)
xlli_GPIO_BIT_FFDCD    EQU      (xlli_BIT_4)
xlli_GPIO_BIT_FFDSR    EQU      (xlli_BIT_5)
xlli_GPIO_BIT_FFRI     EQU      (xlli_BIT_6)
xlli_GPIO_BIT_FFTXD    EQU      (xlli_BIT_7)
xlli_GPIO_BIT_FFDTR    EQU      (xlli_BIT_8)
xlli_GPIO_BIT_FFRTS    EQU      (xlli_BIT_9)

xlli_GPIO_AF_BIT_FFRXD EQU      (xlli_BIT_4)
xlli_GPIO_AF_BIT_FFCTS EQU      (xlli_BIT_6)
xlli_GPIO_AF_BIT_FFDCD EQU      (xlli_BIT_8)
xlli_GPIO_AF_BIT_FFDSR EQU      (xlli_BIT_10)
xlli_GPIO_AF_BIT_FFRI  EQU      (xlli_BIT_12)
xlli_GPIO_AF_BIT_FFTXD EQU      (xlli_BIT_15)
xlli_GPIO_AF_BIT_FFDTR EQU      (xlli_BIT_17)
xlli_GPIO_AF_BIT_FFRTS EQU      (xlli_BIT_19)

;
; POWER MANAGER base address and register offsets from the base address
;

xlli_PMRCREGS_PHYSICAL_BASE  EQU    0x40F00000 

xlli_PMCR_offset     EQU    (0x00)      ; Power Manager Control Register
xlli_PSSR_offset     EQU    (0x04)      ; Power Manager Sleep Status Register
xlli_PSPR_offset     EQU    (0x08)      ; Power Manager Scratch Pad Register
xlli_PWER_offset     EQU    (0x0C)      ; Power Manager Wake-up Enable Register
xlli_PRER_offset     EQU    (0x10)      ; Power Manager GPIO Rising-edge Detect Enable Register
xlli_PFER_offset     EQU    (0x14)      ; Power Manager GPIO Falling-edge Detect Enable Register
xlli_PEDR_offset     EQU    (0x18)      ; Power Manager GPIO Edge Detect Status Register
xlli_PCFR_offset     EQU    (0x1C)      ; Power Manager General Configuration Register
xlli_PGSR0_offset    EQU    (0x20)      ; Power Manager GPIO Sleep State Register for GP [31-0]
xlli_PGSR1_offset    EQU    (0x24)      ; Power Manager GPIO Sleep State Register for GP [63-32]
xlli_PGSR2_offset    EQU    (0x28)      ; Power Manager GPIO Sleep State Register for GP [95-64]
xlli_PGSR3_offset    EQU    (0x2C)      ; Power Manager GPIO Sleep State Register for GP [120-96]
xlli_RCSR_offset     EQU    (0x30)      ; Reset Controller Status Register
xlli_PSLR_offset     EQU    (0x34)      ; Power Manager Sleep Mode Config Register
xlli_PSTR_offset     EQU    (0x38)      ; Power Manager Standby Mode Config Register
xlli_PSNR_offset     EQU    (0x3C)      ; Power Manager Sense Moce Config Register
xlli_PVCR_offset     EQU    (0x40)      ; Power Manager Voltage Change Control Register
xlli_PKWR_offset     EQU    (0x50)      ; Power Manager Keyboard Wake-up Enable Register
xlli_PKSR_offset     EQU    (0x54)      ; Power Manager Keyboard Edge-Detect Status Register
xlli_PI2DBR_offset   EQU   (0x188)      ; Power I2C Data Buffer Register
xlli_PI2CR_offset    EQU   (0x190)      ; Power I2C Control Register
xlli_PI2SR_offset    EQU   (0x198)      ; Power I2C Status Register
xlli_PI2SAR_offset   EQU   (0x1A0)      ; Power I2C Slave Address Register

;
; POWER MANAGER register bit masks 
;
xlli_PSSR_SSS           EQU    (0x01)   ; Software Sleep Status
xlli_PSSR_BFS           EQU    (0x02)   ; Battery Fault Status
xlli_PSSR_VFS           EQU    (0x04)   ; VCC Fault Status
xlli_PSSR_PH            EQU    (0x10)   ; Peripheral Control Hold
xlli_PSSR_RDH           EQU    (0x20)   ; Read Disable Hold

xlli_PCFR_OPDE          EQU    (0x01)   ; Processor (13MHz) osc power-down enable
xlli_PCFR_FP            EQU    (0x02)   ; Float PCMCIA during sleep modes
xlli_PCFR_FS            EQU    (0x04)   ; Float Static Chip Selects
xlli_PCFR_GPR_EN        EQU    (0x10)   ; GPIO 1 performs GPIO reset
xlli_PCFR_SYSEN_EN      EQU    (0x20)   ; SYS_EN pin
xlli_PCFR_PI2C_EN       EQU    (0x40)   ;
xlli_PCFR_DC_EN         EQU    (0x80)      ; Deep-Sleep Mode
xlli_PCFR_FVC           EQU    (0x400)  ;
xlli_PCFR_L1_EN         EQU    (0x800)  ;
xlli_PCFR_GPROD         EQU    (0x1000) ;
xlli_PCFR_PO            EQU    (0x4000) ;
xlli_PCFR_RO            EQU    (0x8000) ;
xlli_PCFR_USEDBITS      EQU     (xlli_PCFR_OPDE :OR: xlli_PCFR_FP :OR: xlli_PCFR_FS :OR: xlli_PCFR_GPR_EN :OR: \
                                xlli_PCFR_SYSEN_EN :OR: xlli_PCFR_PI2C_EN :OR: xlli_PCFR_DC_EN :OR: xlli_PCFR_FVC :OR: \
                                xlli_PCFR_L1_EN :OR: xlli_PCFR_GPROD :OR: xlli_PCFR_PO :OR: xlli_PCFR_RO)

xlli_PSLR_SL_PI_OFF     EQU   (0x00000000)      ; PI power domain is powered off in sleep and deep sleep mode
xlli_PSLR_SL_PI_RETAIN  EQU   (0x00000004)      ; PI power domain retains state in sleep and deep sleep mode
xlli_PSLR_SL_PI_RUN     EQU   (0x00000008)      ; PI power domain is active with clocks running in sleep mode (DO NOT set for deep-sleep mode)
xlli_PSLR_SL_PI_RSVD    EQU   (0x0000000C)      ; PI power domain reserved bits
xlli_PSLR_SL_R0         EQU   (0x00000100)      ; SRAM Bank 0 retains state in sleep mode
xlli_PSLR_SL_R1         EQU   (0x00000200)      ; SRAM Bank 1 retains state in sleep mode
xlli_PSLR_SL_R2         EQU   (0x00000400)      ; SRAM Bank 2 retains state in sleep mode
xlli_PSLR_SL_R3         EQU   (0x00000800)      ; SRAM Bank 3 retains state in sleep mode
xlli_PSLR_SL_ROD        EQU   (0x00100000)      ; 1 = nRESET_OUT is not asserted upon entry into sleep or deep-sleep mode
xlli_PSLR_IVF           EQU   (0x00400000)      ; 1 = No action taken when nVV_FAULT occurs in sleep mode
xlli_PSLR_PSSD          EQU   (0x00800000)      ; 1 = shorten the wake-up delay if all corresponding power supplies are detected to have powered on
xlli_PSLR_PWR_DEL_MAX   EQU   (0x0f000000)      ; max power supply PWR_EN ramp delay
xlli_PSLR_SYS_DEL_MAX   EQU   (0xf0000000)      ; max system power supply SYS_EN ramp delay
xlli_PSLR_USEDBITS      EQU   (xlli_PSLR_SL_PI_RSVD :OR: xlli_PSLR_SL_R0 :OR: \
                                xlli_PSLR_SL_R1 :OR: xlli_PSLR_SL_R2 :OR: \
                                xlli_PSLR_SL_R3 :OR: xlli_PSLR_SL_ROD :OR: \
                                xlli_PSLR_IVF :OR: xlli_PSLR_PSSD :OR: \
                                xlli_PSLR_PWR_DEL_MAX :OR: xlli_PSLR_SYS_DEL_MAX)
                                
xlli_PWER_WE0     EQU    (0x01)      ; Wake-up Enable GPIO pin 0
xlli_PWER_WE1     EQU    (0x02)      ; Wake-up Enable GPIO pin 1
xlli_PWER_WERTC   EQU    (0x80000000); RTC Standby, Wake-up Enable-

;
; MEMORY CONTROLLER base address and register offsets from the base address 
;

xlli_MEMORY_CONFIG_BASE       EQU   0x48000000

xlli_MDCNFG_offset    EQU     (0x00)
xlli_MDREFR_offset    EQU     (0x04)
xlli_MSC0_offset      EQU     (0x08)
xlli_MSC1_offset      EQU     (0x0C)
xlli_MSC2_offset      EQU     (0x10)
xlli_MECR_offset      EQU     (0x14)
xlli_SXLCR_offset     EQU     (0x18)
xlli_SXCNFG_offset    EQU     (0x1C)
xlli_FLYCNFG_offset   EQU     (0x20)
xlli_SXMRS_offset     EQU     (0x24)                                       
xlli_MCMEM0_offset    EQU     (0x28)
xlli_MCMEM1_offset    EQU     (0x2C)
xlli_MCATT0_offset    EQU     (0x30)
xlli_MCATT1_offset    EQU     (0x34)
xlli_MCIO0_offset     EQU     (0x38)
xlli_MCIO1_offset     EQU     (0x3C)
xlli_MDMRS_offset     EQU     (0x40)
xlli_BOOT_DEF_offset  EQU     (0x44)
xlli_ARB_CNTL_offset  EQU     (0x48)
xlli_BSCNTR0_offset   EQU     (0x4C)
xlli_BSCNTR1_offset   EQU     (0x50)
xlli_LCDBSCNTR_offset EQU     (0x54)
xlli_MDMRSLP_offset   EQU     (0x58)
xlli_BSCNTR2_offset   EQU     (0x5C)
xlli_BSCNTR3_offset   EQU     (0x60)

; Memory Controller bit defs

xlli_MDREFR_K0DB4     EQU     (0x20000000)    ; Sync Static Clock 0 divide by 4 control/status
xlli_MDREFR_K2FREE    EQU     (0x02000000)    ; Set to force SDCLK[2] to be free running
xlli_MDREFR_K1FREE    EQU     (0x01000000)    ; Set to force SDCLK[1] to be free running
xlli_MDREFR_K0FREE    EQU     (0x00800000)    ; Set to force SDCLK[0] to be free running
xlli_MDREFR_SLFRSH    EQU     (0x00400000)    ; Self Refresh Control Status bit
xlli_MDREFR_APD       EQU     (0x00100000)    ; Auto Power Down bit
xlli_MDREFR_K2DB2     EQU     (0x00080000)    ; SDRAM clock pin 2 divide by 2 control/status
xlli_MDREFR_K2RUN     EQU     (0x00040000)    ; SDRAM clock pin 2 run/control status
xlli_MDREFR_K1DB2     EQU     (0x00020000)    ; SDRAM clock pin 1 divide by 2 control/status
xlli_MDREFR_K1RUN     EQU     (0x00010000)    ; SDRAM clock pin 1 run/control status
xlli_MDREFR_E1PIN     EQU     (0x00008000)    ; SDRAM clock Enable pin 1 level control/status
xlli_MDREFR_K0DB2     EQU     (0x00004000)    ; Sync Static Memory Clock divide by 2 control/status
xlli_MDREFR_K0RUN     EQU     (0x00002000)    ; Sync Static Memory Clock Pin 0
xlli_MDREFR_E0PIN     EQU     (0x00000100)    ; SDRAM clock enable pin 0 (Cotulla ONLY!!)

xlli_MDCNFG_DE0       EQU     (0x00000001)    ; SDRAM enable bit for partition 0
xlli_MDCNFG_DE1       EQU     (0x00000002)    ; SDRAM enable bit for partition 1
xlli_MDCNFG_DE2       EQU     (0x00010000)    ; SDRAM enable bit for partition 2
xlli_MDCNFG_DE3       EQU     (0x00020000)    ; SDRAM enable bit for partition 3
xlli_MDCNFG_DWID0     EQU     (0x00000004)    ; SDRAM bus width (clear = 32 bits, set = 16 bits)
xlli_MDCNFG_DWID2     EQU     (0x00040000)    ; SDRAM bus width (clear = 32 bits, set = 16 bits)
xlli_MDCNFG_DCAC0     EQU     (0x00000008)
xlli_MDCNFG_DCAC2     EQU     (0x00080000)

;
; INTERNAL MEMORY CONTROLLER base address and register offsets from the base address 
;

xlli_IMEMORY_CONFIG_BASE      EQU   (0x58000000)

xlli_IMPMCR_offset   EQU     (0x00)       ; Internal Memory Power Manager Control Register
xlli_IMPMSR_offset   EQU     (0x08)       ; Internal Memory Power Management Status Register


;
; UART Definitions
;
xlli_perif_base                 EQU     (0x40000000)    ; Base address of the peripherals
xlli_ffuart_offset              EQU     (0x00100000)    ; Offset to the Full-Feature UART in the peripheral block
xlli_btuart_offset              EQU     (0x00200000)    ; Offset to the BlueTooth UART in the peripheral block
xlli_stuart_offset              EQU     (0x00700000)    ; Offset to the Standard UART in the peripheral block


xlli_uart_thr_offset            EQU     (0x0)       ;DLAB = 0  WO  8bit - Transmit Holding Register
xlli_uart_rbr_offset            EQU     (0x0)       ;DLAB = 0  RO  8bit - Recieve Buffer Register
xlli_uart_dll_offset            EQU     (0x0)       ;DLAB = 1  RW  8bit - Divisor Latch Low Register
xlli_uart_ier_offset            EQU     (0x4)       ;DLAB = 0  RW  8bit - Interrupt Enable Register
xlli_uart_dlh_offset            EQU     (0x4)       ;DLAB = 1  RW  8bit - Divisor Latch High Register
xlli_uart_iir_offset            EQU     (0x8)       ;DLAB = X  RO  8bit - Interrupt Identification Register
xlli_uart_fcr_offset            EQU     (0x8)       ;DLAB = X  WO  8bit - FIFO Control Register
xlli_uart_lcr_offset            EQU     (0xC)       ;DLAB = X  RW  8bit - Line Control Register
xlli_uart_mcr_offset            EQU     (0x10)      ;DLAB = X  RW  8bit - Modem Control Regiser
xlli_uart_lsr_offset            EQU     (0x14)      ;DLAB = X  RO  8bit - Line Status Register
xlli_uart_msr_offset            EQU     (0x18)      ;DLAB = X  RO  8bit - Modem Status Register
xlli_uart_spr_offset            EQU     (0x1C)      ;DLAB = X  RW  8bit - Scratchpad Register
xlli_uart_isr_offset            EQU     (0x20)      ;DLAB = X  RW  8bit - Slow Infrared Select Register
xlli_uart_for_offset            EQU     (0x24)      ;DLAB = X  RO  FIFO Occupancy Register
xlli_uart_abr_offset            EQU     (0x28)      ;DLAB = X  RW  Autobaud Control Register
xlli_uart_acr_offset            EQU     (0x2C)      ;DLAB = X Autobaud Count Register

;
; INTERRUPT CONTROLLER base address and register offsets from the base address
; 
;

xlli_INTERREGS_PHYSICAL_BASE      EQU     (0x40D00000)

xlli_ICIP_offset      EQU     (0x00)   ; Interrupt Controller IRQ Pending Register
xlli_ICMR_offset      EQU     (0x04)   ; Interrupt Controller Mask Register
xlli_ICLR_offset      EQU     (0x08)   ; Interrupt Controller Level Register
xlli_ICFP_offset      EQU     (0x0C)   ; Interrupt Controller FIQ pending Register
xlli_ICPR_offset      EQU     (0x10)   ; Interrupt Controller Pending Register
xlli_ICCR_offset      EQU     (0x14)   ; Interrupt Controller Control Register
xlli_ICHP_offset      EQU     (0x18)   ; Interrupt Controller Highest Priority Reg
xlli_ICMR2_offset     EQU     (0xA0)   ; Interrupt Controller Mask Register 2
xlli_ICLR2_offset     EQU     (0xA4)   ; Interrupt Controller Level Register 2
xlli_ICCR2_offset     EQU     (0xAC)   ; Interrupt Controller Control Register 2

;
; SSP SERIAL PORTS base address and register offsets from the base address
; 

xlli_SSP_PHYSICAL_BASE EQU    (0x40100000)

xlli_SSCR0_1_offset    EQU    (0x00)   ; SSP 1 Control Regsiter 0
xlli_SSCR1_1_offset    EQU    (0x04)   ; SSP 1 Control Register 1
xlli_SSSR_1_offset     EQU    (0x08)   ; SSP 1 Status Register
xlli_SSITR_1_offset    EQU    (0x0C)   ; SSP 1 Interrupt Test Register
xlli_SSDR_1_offset     EQU    (0x10)   ; SSP 1 Data Write Register/Data Read Register

;
; CLOCK REGISTERS base address and register offsets from the base address
; 

xlli_CLKREGS_PHYSICAL_BASE        EQU     (0x41300000)

xlli_CCCR_offset     EQU     (0x00)    ; Core Clock Configuration Register
xlli_CKEN_offset     EQU     (0x04)    ; Clock-Enable Register
xlli_OSCC_offset     EQU     (0x08)    ; Oscillator Configuration Register
xlli_CCSR_offset     EQU     (0x0C)    ; Core Clock Status Register

xlli_CCCR_A_Bit_Mask EQU     (0x1 << 25)  ; "A" bit is bit 25 in CCCR
;
; OS TIMER REGISTERS base address and register offsets from the base address
; 

xlli_OSTREGS_PHYSICAL_BASE        EQU     (0x40A00000)

xlli_OSMR0_offset    EQU     (0x00)    ; OS Timer Match Register 0
xlli_OSMR1_offset    EQU     (0x04)    ; OS Timer Match Register 1
xlli_OSMR2_offset    EQU     (0x08)    ; OS Timer Match Register 2
xlli_OSMR3_offset    EQU     (0x0C)    ; OS Timer Match Register 3

xlli_OSCR0_offset    EQU     (0x10)    ; OS Timer Count Register 0
xlli_OSSR_offset     EQU     (0x14)    ; OS Timer Status Register
xlli_OWER_offset     EQU     (0x18)    ; OS Timer Watchdog Enable Register
xlli_OIER_offset     EQU     (0x1C)    ; OS Timer Interrupt Enable Register

xlli_OSCR4_offset    EQU     (0x40)    ; OS Timer Count Register 4
xlli_OSCR5_offset    EQU     (0x44)    ; OS Timer Count Register 5
xlli_OSCR6_offset    EQU     (0x48)    ; OS Timer Count Register 6
xlli_OSCR7_offset    EQU     (0x4C)    ; OS Timer Count Register 7
xlli_OSCR8_offset    EQU     (0x50)    ; OS Timer Count Register 8
xlli_OSCR9_offset    EQU     (0x54)    ; OS Timer Count Register 9
xlli_OSCR10_offset   EQU     (0x58)    ; OS Timer Count Register 10
xlli_OSCR11_offset   EQU     (0x5C)    ; OS Timer Count Register 11

xlli_OSMR4_offset    EQU     (0x80)    ; OS Timer Match Register 4
xlli_OSMR5_offset    EQU     (0x84)    ; OS Timer Match Register 5
xlli_OSMR6_offset    EQU     (0x88)    ; OS Timer Match Register 6
xlli_OSMR7_offset    EQU     (0x8C)    ; OS Timer Match Register 7
xlli_OSMR8_offset    EQU     (0x90)    ; OS Timer Match Register 8
xlli_OSMR9_offset    EQU     (0x94)    ; OS Timer Match Register 9
xlli_OSMR10_offset   EQU     (0x98)    ; OS Timer Match Register 10
xlli_OSMR11_offset   EQU     (0x9C)    ; OS Timer Match Register 11

xlli_OMCR4_offset    EQU     (0xC0)    ; OS Timer Match Control Register 4
xlli_OMCR5_offset    EQU     (0xC4)    ; OS Timer Match Control Register 5
xlli_OMCR6_offset    EQU     (0xC8)    ; OS Timer Match Control Register 6
xlli_OMCR7_offset    EQU     (0xCC)    ; OS Timer Match Control Register 7
xlli_OMCR8_offset    EQU     (0xD0)    ; OS Timer Match Control Register 8
xlli_OMCR9_offset    EQU     (0xD4)    ; OS Timer Match Control Register 9
xlli_OMCR10_offset   EQU     (0xD8)    ; OS Timer Match Control Register 10
xlli_OMCR11_offset   EQU     (0xDC)    ; OS Timer Match Control Register 11

xlli_OSSR_ALL        EQU     (0xFFF)   ; Match register status "sticky bits"
xlli_OIER_E1         EQU     (0x002)   ; Interrupt enable bit for match register #1

;
; REAL TIME CLOCK (RTC) REGISTERS base address and register offsets from the base address
; 

xlli_RTCREGS_PHYSICAL_BASE   EQU       (0x40900000)

xlli_RCNR_offset     EQU     (0x00)    ; RTC Counter Register
xlli_RTAR_offset     EQU     (0x04)    ; RTC Alarm Register
xlli_RTSR_offset     EQU     (0x08)    ; RTC Status Register
xlli_RTTR_offset     EQU     (0x0C)    ; RTC Timer Trim Register
xlli_RDCR_offset     EQU     (0x10)    ; RTC Day Counter Register
xlli_RYCR_offset     EQU     (0x14)    ; RTC Year Counter Register 
xlli_RDAR1_offset    EQU     (0x18)    ; RTC Day Alarm Register 1
xlli_RYAR1_offset    EQU     (0x1C)    ; RTC Year Alarm Register 2
xlli_RDAR2_offset    EQU     (0x20)    ; RTC Day Alarm Register 2
xlli_RYAR2_offset    EQU     (0x24)    ; RTC Year Alarm Register 2
xlli_SWCR_offset     EQU     (0x28)    ; Stopwatch Counter Register
xlli_SWAR1_offset    EQU     (0x2C)    ; Stopwatch Alarm Register 1
xlli_SWAR2_offset    EQU     (0x30)    ; Stopwatch Alarm Register 2
xlli_PICR_offset     EQU     (0x34)    ; Periodic Interrupt Counter Register
xlli_PIAR_offset     EQU     (0x38)    ; Periodic Interrupt Alarm Register


; Oscillator Controller bit defs

xlli_OSCC_OOK        EQU     (0x01)    ; Oscillator OK bit
xlli_OSCC_OON        EQU     (0x02)    ; Timekeeping (32.768KHz) Osc bit
xlli_OSCC_TOUT_EN    EQU     (0x04)    ; Timekeeping Output enable
xlli_OSCC_PIO_EN     EQU     (0x08)    ; Processor Oscillator Output Enable
xlli_OSCC_CRI        EQU     (0x10)    ; Processor Oscillator Output Enable

;
; Coprocessor 15 data bits
; 

xlli_control_icache  EQU     (0x1000)  ; bit 12 -  i-cache bit
xlli_control_btb     EQU     (0x0800)  ; bit 11 -  btb bit
xlli_control_r       EQU     (0x0200)  ; Bit 9
xlli_control_s       EQU     (0x0100)  ; Bit 8
xlli_control_dcache  EQU     (0x0004)  ; Bit 2  -  d-cache bit
xlli_control_mmu     EQU     (0x0001)  ; Bit 0  -  MMU bit


;
; CP 15 related settings
;

xlli_PID                   EQU     (0x00)
xlli_DACR                  EQU     (0x01)
xlli_CONTROL_DCACHE        EQU     (0x04)
xlli_CONTROL_MINIDATA_01   EQU     (0x10)
xlli_CONTROL_BTB           EQU     (0x800)   ; Brach Target Buffer bit

;
; register bit masks - RCSR
;
xlli_RCSR_HWR         EQU     (0x01)
xlli_RCSR_WDR         EQU     (0x02)
xlli_RCSR_SMR         EQU     (0x04)
xlli_RCSR_GPR         EQU     (0x08)
xlli_RCSR_ALL         EQU     (0xF)


;
;  CPSR Processor constants

xlli_CPSR_Mode_MASK   EQU       (0x0000001F)
xlli_CPSR_Mode_USR    EQU       (0x10)
xlli_CPSR_Mode_FIQ    EQU       (0x11)
xlli_CPSR_Mode_IRQ    EQU       (0x12)
xlli_CPSR_Mode_SVC    EQU       (0x13)
xlli_CPSR_Mode_ABT    EQU       (0x17)
xlli_CPSR_Mode_UND    EQU       (0x1B)
xlli_CPSR_Mode_SYS    EQU       (0x1F)

xlli_CPSR_I_Bit       EQU       (0x80)
xlli_CPSR_F_Bit       EQU       (0x40)


xlli_PWRMODE_SLEEP    EQU       (0x00000003) ; Value for cp14: Reg7 to induce sleep.

      END
