;------------------------------------------------------------------------------
;
;   Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   FILE:   xldr.s
;   
;   Provides support for booting from an SD device connected to the 
;   ESDHC controller. This file was meant to come out of I2C EEPROM, so it has to 
;   enumrate and initialize SD.
;
;------------------------------------------------------------------------------
    INCLUDE armmacros.s
    INCLUDE mx25_base_regs.inc
    INCLUDE mx25_base_mem.inc
    INCLUDE mx25_crm.inc
    INCLUDE mx25_esdramc.inc
    INCLUDE image_cfg.inc


;
; ARM constants
;
ARM_CPSR_PRECISE            EQU     (1 << 8)
ARM_CPSR_IRQDISABLE         EQU     (1 << 7)
ARM_CPSR_FIQDISABLE         EQU     (1 << 6)
ARM_CPSR_MODE_SVC           EQU     0x13
ARM_CTRL_ICACHE             EQU     (1 << 12)
ARM_CTRL_DCACHE             EQU     (1 << 2)
ARM_CTRL_MMU                EQU     (1 << 0)
ARM_CTRL_VECTORS            EQU     (1 << 13)
ARM_CACR_FULL               EQU     0x3

; CRM Lock flags
MPLL_LOCK_FLAG              EQU     0x00008000
UPLL_LOCK_FLAG              EQU     0x00008000

; ESDHC1 CRM CGCR1 Enable bit
CRM_ESDHC1_EN               EQU     (1 << 13)

;// ESDHC registers
DSADDR      EQU     CSP_BASE_REG_PA_ESDHC1+0x0000
BLKATTR     EQU     CSP_BASE_REG_PA_ESDHC1+0x0004
CMDARG      EQU     CSP_BASE_REG_PA_ESDHC1+0x0008
XFERTYP     EQU     CSP_BASE_REG_PA_ESDHC1+0x000C
CMDRSP0     EQU     CSP_BASE_REG_PA_ESDHC1+0x0010
CMDRSP1     EQU     CSP_BASE_REG_PA_ESDHC1+0x0014
CMDRSP2     EQU     CSP_BASE_REG_PA_ESDHC1+0x0018
CMDRSP3     EQU     CSP_BASE_REG_PA_ESDHC1+0x001C
DATPORT     EQU     CSP_BASE_REG_PA_ESDHC1+0x0020
PRSSTAT     EQU     CSP_BASE_REG_PA_ESDHC1+0x0024
PROCTL      EQU     CSP_BASE_REG_PA_ESDHC1+0x0028
SYSCTL     EQU     CSP_BASE_REG_PA_ESDHC1+0x002C
IRQSTAT     EQU     CSP_BASE_REG_PA_ESDHC1+0x0030
IRQSTATEN     EQU     CSP_BASE_REG_PA_ESDHC1+0x0034
IRQSIGEN     EQU     CSP_BASE_REG_PA_ESDHC1+0x0038
AUTOC12ERR     EQU     CSP_BASE_REG_PA_ESDHC1+0x003C
HOSTCAPBLT     EQU     CSP_BASE_REG_PA_ESDHC1+0x0040
WML     EQU     CSP_BASE_REG_PA_ESDHC1+0x0044
FEVT     EQU     CSP_BASE_REG_PA_ESDHC1+0x0050
HOSTVER     EQU     CSP_BASE_REG_PA_ESDHC1+0x00FC

;// ROM code stores the address mode of the card at this location in IRAM
IRAM_CARD_ADDR_MODE    EQU     0x780018fc

    OPT 2                                       ; disable listing
    INCLUDE kxarm.h
    OPT 1                                       ; reenable listing
   
    TEXTAREA

    ; romimage needs pTOC. give it one.
pTOC    DCD -1
    EXPORT pTOC
    


;******************************************************************************
;*
;* FUNCTION:    StartUp
;*
;* DESCRIPTION: System bootstrap function
;*
;* PARAMETERS:  None
;*
;* RETURNS:     None
;*
;******************************************************************************
 
    STARTUPTEXT
    LEAF_ENTRY StartUp
    
    
    ;----------------------------------------
    ; Configure CRM (Clock Register Module)
    ;----------------------------------------    

    ldr     r1, =CSP_BASE_REG_PA_CRM

    ; Configure MCU PLL
    ;
    ;   MCUPLL  = (CKIH) * 2 * (MFI + MFN/(MFD+1)) / (PDF+1)
    ;           = 24 MHz * 2 * (11 + 1/(11+1)) / (0+1)
    ;           = 532 MHz
    ;
    ;   MFN = 1 = (1 << 0)                          = 0x00000001
    ;   MFI = 11 = (11 << 10)                       = 0x00002C00
    ;   MFD = 11 = (11 << 16)                       = 0x000B0000
    ;   PD = 0 = (0 << 26)                          = 0x00000000
    ;   BRMO = first order = (1 << 31)              = 0x80000000
    ;                                               ------------
    ;                                                 0x800B2C01
    ldr     r0, =0x800B2C01
    str     r0, [r1, #CRM_MPCTL_OFFSET]

    ; Configure USB PLL
    ;
    ;   USBPLL  = (CKIH) * 2 * (MFI + MFN/(MFD+1)) / (PDF+1)
    ;           = 24 MHz * 2 * (5 + 0/(0+1)) / (0+1)
    ;           = 240 MHz
    ;
    ;   MFN = 0 = (0 << 0)                          = 0x00000000
    ;   MFI = 5 = (5 << 10)                         = 0x00001400
    ;   MFD = 0 = (0 << 16)                         = 0x00000000
    ;   PD = 0 = (0 << 26)                          = 0x00000000
    ;   BRMO = first order = (1 << 31)              = 0x80000000
    ;                                               ------------
    ;                                                 0x10001400
    ldr     r0, =0x10001400
    str     r0, [r1, #CRM_UPCTL_OFFSET]
    
    ; Configure Clock Control Register
    ;   Configure ARM and AHB Clock
    ;
    ;   ARM Clock 399Mhz = MCUPLL * DIVGEN / ARM_DIVIDER
    ;                    = 532Mhz * 0.75   / 1
    ;                    = 399 Mhz
    ;       ARM_DIVIDER (0 << 30)   = 0x00000000
    ;       ARM_SRC     (1 << 14)   = 0x00004000
    ;
    ;   AHB Clock 133Mhz = ARMCLK / AHB_DIVIDER
    ;                    = 399Mhz / 3
    ;                    = 133 Mhz
    ;       AHB_DIVIDER (2 << 28)   = 0x20000000
    ;
    ;   USB Clock 60Mhz =  UPLL  / USB_DIVIDER
    ;                   = 240Mhz / 4
    ;                   = 60Mhz
    ;       USB_DIVIDER (3 << 16)   = 0x00030000
    ;
    ;   USB PLL Enable (0 << 23)    = 0x00000000
    ;   MPLL and UPLL reset (3<<26) = 0x0C000000
    ;   LP CTL (0<<24)              = 0x00000000
    ;                               -------------
    ;                                 0x2C034000
    ldr     r0, =0x2C034000
    str     r0, [r1, #CRM_CCTL_OFFSET]

    ; Now wait the MPLL is reset, so wait on the MPLL lock bit
WaitForMPLL
    ldr     r0, [r1, #CRM_MPCTL_OFFSET]
    tst     r0, #MPLL_LOCK_FLAG
    beq     WaitForMPLL
    
    ; Now wait the UPLL is reset, so wait on the UPLL lock bit
WaitForUPLL
    ldr     r0, [r1, #CRM_UPCTL_OFFSET]
    tst     r0, #UPLL_LOCK_FLAG
    beq     WaitForUPLL


    ;   Misc Control Register
    ;       Default value is correct 0x4300 0000
    ;       PER CLK MUX = HCLK Always >> So PER Clock input already 133Mhz
    ;       USB CLK MUX = UPLL Output
    ;       CLK OUTPUT  = Disable
    

    ;----------------------------------------
    ; Disable all clock except the EMI
    ;----------------------------------------   
    ;   The EMI clock used by the XLDR in Clock Gating Control Register 0
    ;   is not turned off. We only need RAM and ESDHC 
    ;       Enable AHB hclk_emi             = 0x00080000
    ;       Enable AHB hclk_esdhc1          = 0x00200000
    ;       Enable AHB ipg_per_esdhc2       = 0x00000008
    ;                                       ------------
    ;                                         0x00280008
    ldr     r0, =0xFFFFFFFF
    str     r0, [r1, #CRM_CGCR0_OFFSET]

    ;   Clock Gating Control Register 1
    ;       Disable all IPG Clock except for ipg_clk_esdhc1 (1<<13)  = 0x00002000
    ;
    ldr     r0, =0xFFFFFFFF
    str     r0, [r1, #CRM_CGCR1_OFFSET]

    ;   Clock Gating Control Register 2
    ;       Disable all IPG Clock except for ESDHC1 (0 << 0)  = 0x00000000
    ;
    ldr     r0, =0xFFFFFFFF
    str     r0, [r1, #CRM_CGCR2_OFFSET]
    
    ;------------------------------------------------------------
    ;configure AIPSs
    ;------------------------------------------------------------
    mov r1,#0
    ldr r2,=0x77777777
    
    ldr r0,=0x43f00000
    str r1,[r0,#0x40]       ;setmem  0x43f00040 0x00000000  32
    str r1,[r0,#0x44]       ;setmem  0x43f00044 0x00000000  32
    str r1,[r0,#0x48]       ;setmem  0x43f00048 0x00000000  32
    str r1,[r0,#0x4C]       ;setmem  0x43f0004C 0x00000000  32
    str r1,[r0,#0x50]       ;setmem  0x43f00050 0x00000000  32  
    str r2,[r0,#0x0]        ;setmem  0x43f00000 0x77777777  32
    str r2,[r0,#0x4]        ;setmem  0x43f00004 0x77777777  32  
    ldr r0,=0x53f00000      
    str r1,[r0,#0x40]       ;setmem  0x53f00040 0x00000000  32
    str r1,[r0,#0x44]       ;setmem  0x53f00044 0x00000000  32
    str r1,[r0,#0x48]       ;setmem  0x53f00048 0x00000000  32
    str r1,[r0,#0x4C]       ;setmem  0x53f0004C 0x00000000  32  
    str r1,[r0,#0x50]       ;setmem  0x53f00050 0x00000000  32
    str r2,[r0,#0x0]        ;setmem  0x53f00000 0x77777777  32  
    str r2,[r0,#0x4]        ;setmem  0x53f00004 0x77777777  32


    ;; now we do the external RAM init. it's located in another file because it's shared among all the XLDRs
    INCLUDE xldr_sdram_init.inc

    IMPORT SDMMC_go
    b   SDMMC_go

    LTORG
    
    END


