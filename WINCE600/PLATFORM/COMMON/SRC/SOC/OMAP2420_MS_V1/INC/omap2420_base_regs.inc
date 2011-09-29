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
;------------------------------------------------------------------------------
;
;  Header:  omap2420_base_regs.inc
;
;  This header file defines the addresses of the base registers for
;  the System on Chip (SoC) components.
;

;------------------------------------------------------------------------------
;  Device ID
;------------------------------------------------------------------------------
OMAP2420_DEVICE_ID_REGS_PA      EQU     0x48014218

;------------------------------------------------------------------------------
;  Configuration Unit
;------------------------------------------------------------------------------

OMAP2420_CONFIG_REGS_PA         EQU     0x48000000
OMAP2420_REGS_SIZE              EQU     0x08000000

;//------------------------------------------------------------------------------
;//  Power, Reset, Clock Management module
;//------------------------------------------------------------------------------

OMAP2420_PRCM_REGS_PA            EQU 0x48008000

;//------------------------------------------------------------------------------
;//  UART Units
;//------------------------------------------------------------------------------

OMAP2420_UART1_REGS_PA           EQU 0x4806A000
OMAP2420_UART2_REGS_PA           EQU 0x4806C000
OMAP2420_UART3_REGS_PA           EQU 0x4806E000

;//------------------------------------------------------------------------------
;//  USB 
;//------------------------------------------------------------------------------

OMAP2420_USBOT1_REGS_PA           EQU 0x4805E000

;//------------------------------------------------------------------------------
;//  USB OTG Controller
;//------------------------------------------------------------------------------

OMAP2420_OTG_REGS_PA             EQU 0x4805E300

;//------------------------------------------------------------------------------
;//  USB Device Controller
;//------------------------------------------------------------------------------

OMAP2420_USBD_REGS_PA            EQU 0x4805E200

;//------------------------------------------------------------------------------
;//  Camera SubSystem
;//------------------------------------------------------------------------------

OMAP2420_CAMSUB_REGS_PA             EQU 0x48052000
OMAP2420_CAMCORE_REGS_PA            EQU 0x48052400
OMAP2420_CAMDMA_REGS_PA             EQU 0x48052800
OMAP2420_CAMMMU_REGS_PA             EQU 0x48052C00

;//------------------------------------------------------------------------------
;//  IVA MMU base addresss
;//------------------------------------------------------------------------------
OMAP2420_IVA_MMU_REGS_PA            EQU 0x5D000000

;//------------------------------------------------------------------------------
;//  HDQ/1Wire Controller
;//------------------------------------------------------------------------------

OMAP2420_HDQ_1WIRE_REGS_PA       EQU 0x480B2000

;//------------------------------------------------------------------------------
;//  32kHz Timer Units
;//------------------------------------------------------------------------------

OMAP2420_TIMER32K_REGS_PA        EQU 0x48004000

;//
;// General Purpose Timers.
;//

OMAP2420_GPTIMER1_REGS_PA   EQU 0x48028000
OMAP2420_GPTIMER2_REGS_PA   EQU 0x4802A000
OMAP2420_GPTIMER3_REGS_PA   EQU 0x48078000
OMAP2420_GPTIMER4_REGS_PA   EQU 0x4807A000
OMAP2420_GPTIMER5_REGS_PA   EQU 0x4807C000
OMAP2420_GPTIMER6_REGS_PA   EQU 0x4807E000
OMAP2420_GPTIMER7_REGS_PA   EQU 0x48080000
OMAP2420_GPTIMER8_REGS_PA   EQU 0x48082000
OMAP2420_GPTIMER9_REGS_PA   EQU 0x48084000
OMAP2420_GPTIMER10_REGS_PA  EQU 0x48086000
OMAP2420_GPTIMER11_REGS_PA  EQU 0x48088000
OMAP2420_GPTIMER12_REGS_PA  EQU 0x4808A000

;//------------------------------------------------------------------------------
;//  USB Host Controller
;//------------------------------------------------------------------------------

OMAP2420_USBH_REGS_PA            EQU 0x4805E000

;//------------------------------------------------------------------------------
;//  EAC Controller
;//------------------------------------------------------------------------------

OMAP2420_EAC_REGS_PA             EQU 0x48090000


;//------------------------------------------------------------------------------
;//  Mailbox registers
;//------------------------------------------------------------------------------

OMAP2420_MLB1_REGS_PA             EQU 0x48094000
OMAP2420_MLB2_REGS_PA             EQU 0x48095000

;//------------------------------------------------------------------------------
;//  DSP MMU Controller
;//------------------------------------------------------------------------------

OMAP2420_DSP_MMU_REGS_PA         EQU 0x5A000000

;//------------------------------------------------------------------------------
;//  GPIO Controllers
;//------------------------------------------------------------------------------

OMAP2420_GPIO1_REGS_PA           EQU 0x48018000
OMAP2420_GPIO2_REGS_PA           EQU 0x4801A000
OMAP2420_GPIO3_REGS_PA           EQU 0x4801C000
OMAP2420_GPIO4_REGS_PA           EQU 0x4801E000

;//------------------------------------------------------------------------------
;//  OCPx Controllers
;//------------------------------------------------------------------------------

OMAP2420_OCP_REG_PA              EQU 0x5C060000

;//------------------------------------------------------------------------------
;//  Timer Units
;//------------------------------------------------------------------------------

OMAP2420_TIMER1_REGS_PA          EQU 0x48028000
OMAP2420_TIMER2_REGS_PA          EQU 0x4802A000
OMAP2420_TIMER3_REGS_PA          EQU 0x48078000
OMAP2420_TIMER4_REGS_PA          EQU 0x4807A000
OMAP2420_TIMER5_REGS_PA          EQU 0x4807C000
OMAP2420_TIMER6_REGS_PA          EQU 0x4807E000
OMAP2420_TIMER7_REGS_PA          EQU 0x48080000
OMAP2420_TIMER8_REGS_PA          EQU 0x48082000
OMAP2420_TIMER9_REGS_PA          EQU 0x48084000
OMAP2420_TIMER10_REGS_PA         EQU 0x48086000
OMAP2420_TIMER11_REGS_PA         EQU 0x48088000
OMAP2420_TIMER12_REGS_PA         EQU 0x4808A000

;//------------------------------------------------------------------------------
;//  Watchdog Unit
;//------------------------------------------------------------------------------

OMAP2420_WDOG1_REGS_PA           EQU 0x48020000
OMAP2420_WDOG2_REGS_PA           EQU 0x48022000
OMAP2420_WDOG3_REGS_PA           EQU 0x48024000
OMAP2420_WDOG4_REGS_PA           EQU 0x48026000

;//------------------------------------------------------------------------------
;//  Interrupt Units
;//------------------------------------------------------------------------------

OMAP2420_INTC_MPU_REGS_PA        EQU 0x480FE000
OMAP2420_INTC_IVA_REGS_PA        EQU 0x40000000

;//------------------------------------------------------------------------------
;//  I2C Controller
;//------------------------------------------------------------------------------

OMAP2420_I2C1_REGS_PA             EQU 0x48070000
OMAP2420_I2C2_REGS_PA             EQU 0x48072000

;//------------------------------------------------------------------------------
;//  System and DSP DMA Controllers
;//------------------------------------------------------------------------------

;// The registers 0ffset 0x00 - 0x7C are common to all the DMA's
;// So the logical channel starts at offset 0x80.

OMAP2420_SDMA_REGS_PA       EQU 0x48056000
;//
;// SDMA
;//
OMAP2420_DMA_LCD_REGS_PA       EQU 0x48056080

;// Two IOMA values are available (0xF80000 & 0xFC0000). 
;// We need to identify the IOMA used.
OMAP2420_DDMA_REGS_PA               EQU (0xF80000 + 0xC000) 

;//------------------------------------------------------------------------------
;//  Display Subsystem
;//------------------------------------------------------------------------------

OMAP2420_DISS1_REGS_PA          EQU 0x48050000
OMAP2420_DISC1_REGS_PA          EQU 0x48050400
OMAP2420_RFBI1_REGS_PA          EQU 0x48050800
OMAP2420_VENC1_REGS_PA          EQU 0x48050C00

;//------------------------------------------------------------------------------
;//  McBSP base addresses
;//  (see file omap2420_McBSP.inc for offset definitions for these base addresses)
;//------------------------------------------------------------------------------

OMAP2420_MCBSP1_REGS_PA        EQU 0x48074000
OMAP2420_MCBSP2_REGS_PA        EQU 0x48076000

;//------------------------------------------------------------------------------
;//  SRAM embedded memory
;//------------------------------------------------------------------------------

OMAP2420_SRAM_SIZE          EQU (640*1024)
OMAP2420_SRAM_PA            EQU 0x40206000

;//------------------------------------------------------------------------------
;//  Control Pad Configuration Module Register base address
;//  (see file ctrlpadconf.inc for offset definitions for this base address)
;//------------------------------------------------------------------------------

OMAP2420_SYSC1_REGS_PA             EQU 0x48000000

;//------------------------------------------------------------------------------
;//  GPMC Module Register base address
;//  (see file omap2420_gpmc.inc for offset definitions for this base address)
;//------------------------------------------------------------------------------

OMAP2420_GPMC_REGS_PA            EQU 0x6800A000

;//------------------------------------------------------------------------------
;//  SDRAM module register base address
;//  (see file omap2420_sdram.inc for offset definitions for these base addresses)
;//------------------------------------------------------------------------------

OMAP2420_SMS_REGS_PA             EQU 0x68008000
OMAP2420_SDRC_REGS_PA            EQU 0x68009000

;//
;// Frame Adjustment Counter Register (FAC)
;// (see file omap24240_timer.h for offset definitions)
;//

OMAP2420_FAC_REGS_PA            EQU 0x48092000

;//------------------------------------------------------------------------------
;//  McSPI base addresses
;//  (see file omap2420_McSPI.h for offset definitions for these base addresses)
;//------------------------------------------------------------------------------

OMAP2420_MCSPI1_REGS_PA        EQU 0x48098000 
OMAP2420_MCSPI2_REGS_PA        EQU 0x4809A000

;//------------------------------------------------------------------------------
;//  DSP subsystem (IPI module, dealing with OMAP 24xx memory space) base address
;//  (see file omap2420_dsp.h for offset definitions for this base address)
;//------------------------------------------------------------------------------

OMAP2420_DSP_IPI_REGS_PA      EQU 0x59000000

;//------------------------------------------------------------------------------
;//  vlynq module, see file omap2420_vlynq.h for offset definitions for this base address
;//------------------------------------------------------------------------------

OMAP2420_VLYNQFUNC_REGS_PA    EQU 0x67FFFE00

;//------------------------------------------------------------------------------
;//  SSI Controller, see file omap2420_SSI.h for offset definitions
;//------------------------------------------------------------------------------

OMAP2420_SSI_REGS_PA          EQU 0x48058000
OMAP2420_GDD1_REGS_PA         EQU 0x48059000
OMAP2420_SST1_REGS_PA         EQU 0x4805A000
OMAP2420_SSR1_REGS_PA         EQU 0x4805A800
OMAP2420_SST2_REGS_PA         EQU 0x4805B000
OMAP2420_SSR2_REGS_PA         EQU 0x4805B800


;//------------------------------------------------------------------------------
                        END
