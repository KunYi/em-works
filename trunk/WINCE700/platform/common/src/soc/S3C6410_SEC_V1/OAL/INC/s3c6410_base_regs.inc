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
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;

;;
;;  Header: s3c6410_base_regs.inc
;;
;;  This header file defines the Physical Addresses (PA) of
;;  the base registers for the System on Chip (SoC) components.
;;

;;
;;  NAMING CONVENTIONS
;;
;;  CPU_BASE_REG_ is the standard prefix for CPU base registers.
;;
;;  Memory ranges are accessed using physical, uncached, or cached addresses,
;;  depending on the system state. The following abbreviations are used for
;;  each addressing type:
;;
;;      PA - physical address
;;      UA - uncached virtual address
;;      CA - cached virtual address
;;
;;  The naming convention for CPU base registers is:
;;
;;      CPU_BASE_REG_<ADDRTYPE>_<SUBSYSTEM>
;;
;;



;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_SROMCON
;
; Locates the SROM Controller register block.
;
S3C6410_BASE_REG_PA_SROMCON        EQU    (0x70000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_ONENAND
;
; Locates the OneNAND Controller register block.
;
S3C6410_BASE_REG_PA_ONENAND        EQU    (0x70100000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_NFCON
;
; Locates the NAND Flash Controller register block.
;
S3C6410_BASE_REG_PA_NFCON        EQU    (0x70200000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_CFCON
;
; Locates the Compact Flash Controller register block.
;
S3C6410_BASE_REG_PA_CFCON        EQU    (0x70300000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_TZIC0
;
; Locates the Trust Zone Interrupt Controller 0 register block.
;
S3C6410_BASE_REG_PA_TZIC0        EQU    (0x71000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_TZIC1
;
; Locates the Trust Zone Interrupt Controller 1 register block.
;
S3C6410_BASE_REG_PA_TZIC1        EQU    (0x71100000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_VIC0
;
; Locates the Vectored Interrupt Controller 0 register block.
;
S3C6410_BASE_REG_PA_VIC0        EQU    (0x71200000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_VIC1
;
; Locates the Vectored Interrupt Controller 1 register block.
;
S3C6410_BASE_REG_PA_VIC1        EQU    (0x71300000)

; ETB to be defined...
;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_FIMG_3DSE
;
; Locates the FIMG 3DSE register block.
;
S3C6410_BASE_REG_PA_FIMG_3DSE        EQU    (0x72000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_HOSTIF
;
; Locates the Indirect HOST Interface register block.
;
S3C6410_BASE_REG_PA_HOSTIF        EQU    (0x74000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_MSMIFBM
;
; Locates the Modem Interface SRAM Buffer Memory block.
;
S3C6410_BASE_REG_PA_MSMIF_BM        EQU    (0x74100000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_MSMIF
;
; Locates the Modem Interface register block.
;
S3C6410_BASE_REG_PA_MSMIF_SFR        EQU    (0x74108000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_USBHOST
;
; Locates the USB Host Controller register block.
;
S3C6410_BASE_REG_PA_USBHOST        EQU    (0x74300000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_MDPIF
;
; Locates the MDP Interface register block.
;
S3C6410_BASE_REG_PA_MDPIF        EQU    (0x74400000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_DMA0
;
; Locates the DMA Controller 0 register block.
;
S3C6410_BASE_REG_PA_DMA0        EQU    (0x75000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_DMA1
;
; Locates the DMA Controller 1 register block.
;
S3C6410_BASE_REG_PA_DMA1        EQU    (0x75100000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_2DGRAPHICS
;
; Locates the 2D Graphics register block.
;
S3C6410_BASE_REG_PA_2DGRAPHICS        EQU    (0x76100000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_TVENC
;
; Locates the TV Encoder register block.
;
S3C6410_BASE_REG_PA_TVENC        EQU    (0x76200000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_TVSC
;
; Locates the TV Scaler register block.
;
S3C6410_BASE_REG_PA_TVSC        EQU    (0x76300000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_POST
;
; Locates the Post Processor register block.
;
S3C6410_BASE_REG_PA_POST        EQU    (0x77000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_DISPLAY
;
; Locates the Display Controller register block.
;
S3C6410_BASE_REG_PA_DISPLAY        EQU    (0x77100000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_ROTATOR
;
; Locates the Rotator register block.
;
S3C6410_BASE_REG_PA_ROTATOR        EQU    (0x77200000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_CAMIF
;
; Locates the Camera Interface register block.
;
S3C6410_BASE_REG_PA_CAMIF        EQU    (0x78000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_JPEG
;
; Locates the JPEG Codec register block.
;
S3C6410_BASE_REG_PA_JPEG        EQU    (0x78800000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_USBOTG_LINK
;
; Locates the USB OTG Link Core register block.
;
S3C6410_BASE_REG_PA_USBOTG_LINK        EQU    (0x7C000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_USBOTG_PHY
;
; Locates the USB OTG Phy Control register block.
;
S3C6410_BASE_REG_PA_USBOTG_PHY        EQU    (0x7C100000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_HSMMC0
;
; Locates the High Speed MMC Controller 0 register block.
;
S3C6410_BASE_REG_PA_HSMMC0        EQU    (0x7C200000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_HSMMC1
;
; Locates the High Speed MMC Controller 1 register block.
;
S3C6410_BASE_REG_PA_HSMMC1        EQU    (0x7C300000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_HSMMC2
;
; Locates the High Speed MMC Controller 2 register block.
;
S3C6410_BASE_REG_PA_HSMMC2        EQU    (0x7C400000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_SECURITY
;
; Locates the Security Subsystem Config register block.
;
S3C6410_BASE_REG_PA_SECURITY        EQU    (0x7D000000)

; AEX, DES, HASH, FIFO, SDMA... to be defined

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_DMC0
;
; Locates the DRAM Controller 0 register block.
;
S3C6410_BASE_REG_PA_DMC0        EQU    (0x7E000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_DMC1
;
; Locates the DRAM Controller 1 register block.
;
S3C6410_BASE_REG_PA_DMC1        EQU    (0x7E001000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_MFC
;
; Locates the Multi Format Video Codec register block.
;
S3C6410_BASE_REG_PA_MFC            EQU    (0x7E002000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_WATCHDOG
;
; Locates the Watch-Dog Timer register block.
;
S3C6410_BASE_REG_PA_WATCHDOG        EQU    (0x7E004000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_RTC
;
; Locates the Real Time Clock register block.
;
S3C6410_BASE_REG_PA_RTC            EQU    (0x7E005000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_MIPITX
;
; Locates the MIPI HSI Interface TX register block.
;
S3C6410_BASE_REG_PA_MIPITX        EQU    (0x7E006000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_MIPIRX
;
; Locates the MIPI HSI Interface RX register block.
;
S3C6410_BASE_REG_PA_MIPIRX        EQU    (0x7E007000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_KEYPAD
;
; Locates the Keypad Interface register block.
;
S3C6410_BASE_REG_PA_KEYPAD        EQU    (0x7E00A000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_ADC
;
; Locates the ADC / Touch Screen Interface register block.
;
S3C6410_BASE_REG_PA_ADC            EQU    (0x7E00B000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_ETM
;
; Locates the ETM register block.
;
S3C6410_BASE_REG_PA_ETM            EQU    (0x7E00C000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_KEY
;
; Locates the ADC / Touch Screen Interface register block.
;
S3C6410_BASE_REG_PA_KEY            EQU    (0x7E00D000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_CHIPID
;
; Locates the Chip ID register block.
;
S3C6410_BASE_REG_PA_CHIPID        EQU    (0x7E00E000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_SYSCON
;
; Locates the System Controller register block.
;
S3C6410_BASE_REG_PA_SYSCON        EQU    (0x7E00F000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_TZPC
;
; Locates the TZPC register block.
;
S3C6410_BASE_REG_PA_TZPC        EQU    (0x7F000000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_AC97
;
; Locates the AC97 Controller register block.
;
S3C6410_BASE_REG_PA_AC97        EQU    (0x7F001000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_IIS0
;
; Locates the IIS-Bus Interface Ch0 register block.
;
S3C6410_BASE_REG_PA_IIS0        EQU    (0x7F002000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_IIS1
;
; Locates the IIS-Bus Interface Ch1 register block.
;
S3C6410_BASE_REG_PA_IIS1        EQU    (0x7F003000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_IICBUS
;
; Locates the IIC-Bus Interface register block.
;
S3C6410_BASE_REG_PA_IICBUS        EQU    (0x7F004000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_UART
;
; Locates the UART register block.
;
;S3C6410_BASE_REG_PA_UART        EQU    (0x7F005000)
S3C6410_BASE_REG_PA_UART0        EQU    (0x7F005000)
S3C6410_BASE_REG_PA_UART1        EQU    (0x7F005400)
S3C6410_BASE_REG_PA_UART2        EQU    (0x7F005800)
S3C6410_BASE_REG_PA_UART3        EQU    (0x7F005C00)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_PWM
;
; Locates the PWM Timer register block.
;
S3C6410_BASE_REG_PA_PWM            EQU    (0x7F006000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_IRDA
;
; Locates the IrDA Controller register block.
;
S3C6410_BASE_REG_PA_IRDA        EQU    (0x7F007000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_GPIO
;
; Locates the GPIO register block.
;
S3C6410_BASE_REG_PA_GPIO        EQU    (0x7F008000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_PCM0
;
; Locates the PCM Audio Interface Ch 0 register block.
;
S3C6410_BASE_REG_PA_PCM0        EQU    (0x7F009000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_PCM1
;
; Locates the PCM Audio Interface Ch 1 register block.
;
S3C6410_BASE_REG_PA_PCM1        EQU    (0x7F00A000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_SPI0
;
; Locates the Serial Periphral Interface Ch 0 register block.
;
S3C6410_BASE_REG_PA_SPI0        EQU    (0x7F00B000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_SPI1
;
; Locates the Serial Periphral Interface Ch 1 register block.
;
S3C6410_BASE_REG_PA_SPI1        EQU    (0x7F00C000)

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_I2S_40
;
; Locates the I2S version 4.0 Bus Interface register block.
;
S3C6410_BASE_REG_PA_I2S_40        EQU    0x7F00D000

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_SSS
;
; Locates the SSS register block.
;
S3C6410_BASE_REG_PA_SSS            EQU    0x7F00E000

;--------------------------------------------------------------
;
; Define:  S3C6410_BASE_REG_PA_IIC1
;
; Locates the IIC Bus Interface Ch 1 register block.
;
S3C6410_BASE_REG_PA_IIC1        EQU    0x7F00F000

    END
