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
;-------------------------------------------------------------------------------
;
; Portions Copyright (c) Texas Instruments.  All rights reserved.
;
;-------------------------------------------------------------------------------
;
;  File:  hwinit.s
;
;  Hardware initialization routine for OMAP5912 boards. The code contains
;  references to board hardware specific initialization constants. It is
;  intended to be included by from other assembler file.
;
;  Constants used in configuration are defined in bsp_XXX_cfg.inc files
;  located in platform\%_TGTPLAT%\src\inc directory.
;
;  We also expect that the actual DPLL and its associated clock tree
;  frequencies including EMIF (memory interface Traffic Controller), CPU,
;  and peripheral are lower than the final and may even be in by-pass.
;
;  In normal situation this code is called from IPL/EBOOT code (for system with NOR flash).
;  This code can also be included from Windows CE OS startup code when
;  original TI EBOOT is used (it sets clocks back to 12/13MHz).
;

        ;---------------------------------------------------------------
        ; Set SVC mode & disable IRQ/FIQ
        ;---------------------------------------------------------------

        mrs     r0, cpsr                        ; Get current mode bits.
        bic     r0, r0, #0x1F                   ; Clear mode bits.
        orr     r0, r0, #0xD3                   ; Disable IRQs/FIQs, SVC mode
        msr     cpsr_c, r0                      ; Enter supervisor mode


        ;---------------------------------------------------------------
        ; Initialize cache & CP15 control register
        ;---------------------------------------------------------------
        ; Even if in most cases caches are disabled at this moment
        ; first clean both data & instruction cache, then enable
        ; I-cache. We also clear system and rom protection bits
        ; as they are enabled after reset/ROM code.

        ; Clean & invalidate D cache
10      mrc     p15, 0, r15, c7, c14, 3
        bne     %b10

        ; Invalidate I cache
        mov     r0, #0
        mcr     p15, 0, r0, c7, c5, 0

        ; Set CP15 control bits register
        mrc     p15, 0, r0, c1, c0, 0
        orr     r0, r0, #(1 :SHL: 12)           ; I-cache
        bic     r0, r0, #(1 :SHL: 9)            ; ROM protection
        bic     r0, r0, #(1 :SHL: 8)            ; system protection
        bic     r0, r0, #(1 :SHL: 3)            ; write buffer
        bic     r0, r0, #(1 :SHL: 2)            ; D-cache
        orr     r0, r0, #(1 :SHL: 1)            ; alignment fault
        mcr     p15, 0, r0, c1, c0, 0

        ;---------------------------------------------------------------
        ; Initialize TIPB Bridges
        ;---------------------------------------------------------------

        ldr     r0, =OMAP5912_TIPBPRIV_REGS_PA
        ldr     r1, =BSP_TIPBPRIV_RHEA_CNTL
        strh    r1, [r0, #OMAP5912_TIPB_RHEA_CNTL_REGS_OA]

        ldr     r0, =OMAP5912_TIPBPUBL_REGS_PA
        ldr     r1, =BSP_TIPBPUBL_RHEA_CNTL
        strh    r1, [r0, #OMAP5912_TIPB_RHEA_CNTL_REGS_OA]

        ;---------------------------------------------------------------
        ; Disable Watchdog Timer
        ;---------------------------------------------------------------

        ldr     r0, =OMAP5912_WDOG_REGS_PA
        ldr     r1, =OMAP5912_WDOG_DISABLE_SEQ1
        strh    r1, [r0, #OMAP5912_WDOG_TIMER_MODE]
        ldr     r1, =OMAP5912_WDOG_DISABLE_SEQ2
        strh    r1, [r0, #OMAP5912_WDOG_TIMER_MODE]


        ;---------------------------------------------------------------
        ; Disable 19s Watchdog Timer
        ;---------------------------------------------------------------

        ldr     r0, =OMAP5912_19SWATCHDOG_REGS_PA

        ldr     r1, =OMAP5912_19SWATCHDOG_DISABLE_SEQ1
        str     r1, [r0, #OMAP5912_19SWATCHDOG_MODE]

101
        ldr     r1, [r0,#OMAP5912_19SWATCHDOG_STATUS]
        tst     r1, #(1 :SHL: 4)
        bne 	%b101

        ldr     r1, =OMAP5912_19SWATCHDOG_DISABLE_SEQ2
        str     r1, [r0, #OMAP5912_19SWATCHDOG_MODE]

105
		ldr 	r1, [r0, #OMAP5912_19SWATCHDOG_STATUS]
        tst     r1, #(1 :SHL: 4)
        bne 	%b105


        ;---------------------------------------------------------------
        ; Initialize EMIFS (flash & debug board controller)
        ;---------------------------------------------------------------

        ; Initialize base register
        ldr     r0, =OMAP5912_EMIF_REGS_PA

        ldr     r1, =BSP_EMIFS_CONFIG
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_CONFIG_REGS_OA]
        
        ldr     r1, =BSP_EMIFS_CFG_0
        movs    r1, r1
        beq     %F12
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_CFG_0_REGS_OA]
        ldr     r1, =BSP_EMIFS_ACFG_0
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_ACFG_0_I_REGS_OA]
12      ldr     r1, =BSP_EMIFS_CFG_1
        movs    r1, r1
        beq     %F14
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_CFG_1_REGS_OA]
        ldr     r1, =BSP_EMIFS_ACFG_1
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_ACFG_1_I_REGS_OA]
14      ldr     r1, =BSP_EMIFS_CFG_2
        movs    r1, r1
        beq     %F16
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_CFG_2_REGS_OA]
        ldr     r1, =BSP_EMIFS_ACFG_2
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_ACFG_2_I_REGS_OA]
16      ldr     r1, =BSP_EMIFS_CFG_3
        movs    r1, r1
        beq     %F18
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_CFG_3_REGS_OA]
        ldr     r1, =BSP_EMIFS_ACFG_3
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_ACFG_3_I_REGS_OA]
18      ldr     r1, =BSP_DYN_WAIT
        movs    r1, r1
        beq     %F19
        str     r1, [r0, #OMAP5912_EMIF_EMIFS_FL_CFG_DYN_WAIT_REGS_OA]
19


        ;---------------------------------------------------------------
        ; Initialize EMIFF (DRAM controller)
        ;---------------------------------------------------------------
        ; Code can run from DRAM memory, so for critical parts
        ; we must be sure that code is running from cache memory.
        ; Of course code must be smaller than I cache...
        ; We also assume that actual TC frequency is lower than final
        ; (otherwise we should change first TC frequency and then
        ; intialize EMIFF).
        ;
        ;

        ; First get cache info
        mrc     p15, 0, r0, c0, c0, 1

        ; Get I cache line size as (1 << (b[1..0] + 3))
        and     r3, r0, #3
        mov     r2, #1
        add     r3, r3, #3
        mov     r2, r2, lsl r3
        sub     r3, r2, #1

        ; Now get first and last instruction addresses
        add     r0, pc, #(IC1START - . + 8)
        add     r1, pc, #(IC1END - . + 8)

        ; Make sure that address is cache line aligned
        add     r0, r0, r3
        bic     r0, r0, r3

        ; Avoid doing too much
        sub     r1, r1, #4

        ; Prefetch instructions
20      mcr     p15, 0, r0, c7, c13, 1
        add     r0, r0, r2
        cmp     r0, r1
        bls     %B20

        ; Initialize base register
        ldr     r0, =OMAP5912_EMIF_REGS_PA

        ; Read all constants in front (we must avoid D cache miss)
        ldr     r4, =BSP_EMIFF_OPERATION
        ldr     r5, =BSP_EMIFF_CONFIG
        ldr     r6, =BSP_EMIFF_MRS
        ldr     r7, =BSP_EMIFF_EMRS1
        ldr     r8, =BSP_EMIFF_DELAY
        ldr     r9, =BSP_EMIFF_CONFIG2

        ; First code should wait > 0.2 msec after power up
        mov     r1, r8
30      subs    r1, r1, #1
        bne     %B30

IC1START

        ; Set SDRAM operating mode and type
        str     r4, [r0, #OMAP5912_EMIF_EMIFF_OPERATION_REGS_OA]

        ; Set config register with AR disabled
        bic     r1, r5, #(0x3 :SHL: 2)
        str     r1, [r0, #OMAP5912_EMIF_EMIFF_CONFIG_REGS_OA]

        ; Set clock enable high, then NOP
        mov     r1, #0x07       ; CKE high
        str     r1, [r0, #OMAP5912_EMIF_EMIFF_MANUAL_CMD_REGS_OA]
        mov     r1, #0x00       ; NOP
        str     r1, [r0, #OMAP5912_EMIF_EMIFF_MANUAL_CMD_REGS_OA]

        ; Delay for SDRAM initialization
        mov     r1, r8
40      subs    r1, r1, #1
        bne     %B40

        ; Issue precharge and auto refresh commands
        mov     r1, #0x01       ; Precharge
        str     r1, [r0, #OMAP5912_EMIF_EMIFF_MANUAL_CMD_REGS_OA]
        mov     r1, #0x02       ; Auto Refresh
        str     r1, [r0, #OMAP5912_EMIF_EMIFF_MANUAL_CMD_REGS_OA]
        str     r1, [r0, #OMAP5912_EMIF_EMIFF_MANUAL_CMD_REGS_OA]

        ; Write the MRS value
        str     r6, [r0, #OMAP5912_EMIF_EMIFF_MRS_NEW_REGS_OA]

        ; Write the EMRS1 value
        str     r7, [r0, #OMAP5912_EMIF_EMIFF_EMRS1_REGS_OA]

        ; Set config register with final value
        str     r5, [r0, #OMAP5912_EMIF_EMIFF_CONFIG_REGS_OA]

        ; Set config register with final value
        str     r9, [r0, #OMAP5912_EMIF_EMIFF_CONFIG2_REGS_OA]

        ; Delay for SDRAM initialization
        mov     r1, r8
50      subs    r1, r1, #1
        bne     %B50

IC1END

        ; Set EMIFF priority
        ldr     r1, =BSP_EMIFF_PRIORITY
        str     r1, [r0, #OMAP5912_EMIF_EMIFF_PRIORITY_REGS_OA]

        ;---------------------------------------------------------------
        ; Initialize OCPT1/OCPT2
        ;---------------------------------------------------------------

        ldr     r0, =OMAP5912_EMIF_REGS_PA

        ldr     r1, =BSP_EMIF_OCPT1_PRIOR
        str     r1, [r0, #OMAP5912_EMIF_OCPT1_PRIOR_REGS_OA]
        ldr     r1, =BSP_EMIF_OCPT1_PTOR1
        str     r1, [r0, #OMAP5912_EMIF_OCPT1_PTOR1_REGS_OA]
        ldr     r1, =BSP_EMIF_OCPT1_PTOR2
        str     r1, [r0, #OMAP5912_EMIF_OCPT1_PTOR2_REGS_OA]
        ldr     r1, =BSP_EMIF_OCPT1_PTOR3
        str     r1, [r0, #OMAP5912_EMIF_OCPT1_PTOR3_REGS_OA]
        ldr     r1, =BSP_EMIF_OCPT1_ATOR
        str     r1, [r0, #OMAP5912_EMIF_OCPT1_ATOR_REGS_OA]

        ldr     r1, =BSP_EMIF_OCPT2_PRIOR
        str     r1, [r0, #OMAP5912_EMIF_OCPT2_PRIOR_REGS_OA]
        ldr     r1, =BSP_EMIF_OCPT2_PTOR1
        str     r1, [r0, #OMAP5912_EMIF_OCPT2_PTOR1_REGS_OA]
        ldr     r1, =BSP_EMIF_OCPT2_PTOR2
        str     r1, [r0, #OMAP5912_EMIF_OCPT2_PTOR2_REGS_OA]
        ldr     r1, =BSP_EMIF_OCPT2_PTOR3
        str     r1, [r0, #OMAP5912_EMIF_OCPT2_PTOR3_REGS_OA]
        ldr     r1, =BSP_EMIF_OCPT2_ATOR
        str     r1, [r0, #OMAP5912_EMIF_OCPT2_ATOR_REGS_OA]

        ldr     r1, =BSP_EMIF_OCPT_CONFIG
        str     r1, [r0, #OMAP5912_EMIF_OCPT_CONFIG_REGS_OA]

        ;---------------------------------------------------------------
        ; Initialize OCPI
        ;---------------------------------------------------------------

        ldr     r0, =OMAP5912_OCPI_REGS_PA

        ldr     r1, =BSP_OCPI_PROTECT
        str     r1, [r0, #OMAP5912_OCPI_PROTECT_REGS_OA]
        ldr     r1, =BSP_OCPI_SECURE
        str     r1, [r0, #OMAP5912_OCPI_SECURE_REGS_OA]

        ;---------------------------------------------------------------
        ; Initialize UPLD
        ;---------------------------------------------------------------

        ; Initialize base register
        ldr     r0, =OMAP5912_ULPD_REGS_PA

        ldr     r1, =BSP_ULPD_SOFT_REQ
        strh    r1, [r0, #OMAP5912_ULPD_SOFT_REQ]
        ldr     r1, =BSP_ULPD_SOFT_DISABLE_REQ
        strh    r1, [r0, #OMAP5912_ULPD_SOFT_DISABLE_REQ]
        ldr     r1, =BSP_ULPD_CAM_CLK_CTRL
        strh    r1, [r0, #OMAP5912_ULPD_CAM_CLK_CTRL]
        ldr     r1, =BSP_ULPD_POWER_CTRL
        strh    r1, [r0, #OMAP5912_ULPD_POWER_CTRL]
        ldr     r1, =BSP_ULDP_RF_SETUP
        strh    r1, [r0, #OMAP5912_ULPD_RF_SETUP]
        ldr     r1, =BSP_ULDP_VTCXO_SETUP
        strh    r1, [r0, #OMAP5912_ULPD_VTCXO_SETUP]
        ldr     r1, =BSP_ULDP_SLICER_SETUP
        strh    r1, [r0, #OMAP5912_ULPD_SLICER_SETUP]

        ;---------------------------------------------------------------
        ; Initialize CLKM
        ;---------------------------------------------------------------
        ; Code will run from DRAM memory and there can be time before
        ; new clock will be stabilised and it is safe to access DRAM.
        ; To avoid any problem we make sure that critical code is
        ; run from I cache.
        ;

        ; First get cache info
        mrc     p15, 0, r0, c0, c0, 1

        ; Get I cache line size as (1 << (b[1..0] + 3))
        and     r3, r0, #3
        add     r3, r3, #3
        mov     r2, #1
        mov     r2, r2, lsl r3
        sub     r3, r2, #1

        ; Now get first and last instruction addresses
        add     r0, pc, #(IC2START - . + 8)
        add     r1, pc, #(IC2END - . + 8)

        ; Make sure that address is cache line aligned
        add     r0, r0, r3
        bic     r0, r0, r3

        ; Avoid doing too much
        sub     r1, r1, #4

        ; Prefetch instructions
60      mcr     p15, 0, r0, c7, c13, 1
        add     r0, r0, r2
        cmp     r0, r1
        bls     %B60


        ; Initialize base register
        ldr     r0, =OMAP5912_CLKM_REGS_PA

        ; Remove reset from external peripherals
        ldrh    r1, [r0, #OMAP5912_CLKM_RSTCT2_REGS_OA]
        orr     r1, r1, #(1 :SHL: 0)
        strh    r1, [r0, #OMAP5912_CLKM_RSTCT2_REGS_OA]

        ; Read all constants in front (we must avoid D cache miss)
        ldr     r4, =BSP_CLKM_SYSST
        ldr     r5, =BSP_CLKM_CKCTL
        ldr     r6, =BSP_CLKM_DPLL1_CTL
        ldr     r7, =BSP_CLKM_DELAY

IC2START

        ; Set the new divisors
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        strh    r5, [r0, #OMAP5912_CLKM_CKCTL_REGS_OA]
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop

        ; Set the new mode
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        strh    r4, [r0, #OMAP5912_CLKM_SYSST_REGS_OA]
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop

        ; Get DPLL1_CTL register address to r1
        add     r1, r0, #OMAP5912_CLKM_DPLL1_CTL_REGS_OA

        ; Set DPLL divisor/multiplier value
        strh    r6, [r1]

        ; Check if requesting that PLL be enabled
        tst     r6, #(1 :SHL: 4)
        beq     %F80

        ; Wait for the PLL lock
70      ldrh    r2, [r1]
        tst     r2, #(1 :SHL: 0)
        beq     %B70
80

        ; Delay for new clock stabilitation
        mov     r1, r7
90      subs    r1, r1, #1
        bne     %B90

IC2END
        ; Initialize remaining CLKM registers

        ldr     r1, =BSP_CLKM_IDLECT1
        strh    r1, [r0, #OMAP5912_CLKM_IDLECT1_REGS_OA]
        ldr     r1, =BSP_CLKM_IDLECT2
        strh    r1, [r0, #OMAP5912_CLKM_IDLECT2_REGS_OA]
        ldr     r1, =BSP_CLKM_IDLECT3
        strh    r1, [r0, #OMAP5912_CLKM_IDLECT3_REGS_OA]

        ldr     r1, =BSP_CLKM_EWUPCT
        strh    r1, [r0, #OMAP5912_CLKM_EWUPCT_REGS_OA]
        ldr     r1, =BSP_CLKM_RSTCT1
        strh    r1, [r0, #OMAP5912_CLKM_RSTCT1_REGS_OA]
        ldr     r1, =BSP_CLKM_RSTCT2
        strh    r1, [r0, #OMAP5912_CLKM_RSTCT2_REGS_OA]
        ldr     r1, =BSP_CLKM_CKOUT1
        strh    r1, [r0, #OMAP5912_CLKM_CKOUT1_REGS_OA]
        ldr     r1, =BSP_CLKM_CKOUT2
        strh    r1, [r0, #OMAP5912_CLKM_CKOUT2_REGS_OA]


        ;---------------------------------------------------------------
        ; Configure OMAP5912 SoC multiplexing&pullup/pulldown
        ;---------------------------------------------------------------

        ldr     r0, =OMAP5912_CONFIG_REGS_PA

        ldr     r1, =BSP_CONF_MUX_CTRL6
        str     r1, [r0, #OMAP5912_CONFIG_FUNC_MUX_CTRL_6]
        ldr     r1, =BSP_CONF_MUX_CTRL7
        str     r1, [r0, #OMAP5912_CONFIG_FUNC_MUX_CTRL_7]
        ldr     r1, =BSP_CONF_MUX_CTRL9
        str     r1, [r0, #OMAP5912_CONFIG_FUNC_MUX_CTRL_9]
        ldr     r1, =BSP_CONF_MUX_CTRLA
        str     r1, [r0, #OMAP5912_CONFIG_FUNC_MUX_CTRL_A]
        ldr     r1, =BSP_CONF_MUX_CTRLB
        str     r1, [r0, #OMAP5912_CONFIG_FUNC_MUX_CTRL_B]
        ldr     r1, =BSP_CONF_MUX_CTRL10
        str     r1, [r0, #OMAP5912_CONFIG_FUNC_MUX_CTRL_10]

        ldr     r1, =BSP_CONF_PUPD_SEL2
        str     r1, [r0, #OMAP5912_CONFIG_PU_PD_SEL_2]
        ldr     r1, =BSP_CONF_DWN_CTRL2
        str     r1, [r0, #OMAP5912_CONFIG_PULL_DWN_CTRL_2]

        ldr     r1, =BSP_CONF_PUPD_SEL3
        str     r1, [r0, #OMAP5912_CONFIG_PU_PD_SEL_3]
        ldr     r1, =BSP_CONF_DWN_CTRL3
        str     r1, [r0, #OMAP5912_CONFIG_PULL_DWN_CTRL_3]

        ldr     r1, =0x0000EAEF
        str     r1, [r0, #OMAP5912_CONFIG_COMP_MODE_CTRL_0] ; enable all setting

        ;---------------------------------------------------------------
        ; Set DMA to OMAP3.2 mode
        ;---------------------------------------------------------------

        ldr     r0, =OMAP5912_DMA_REGS_PA
        ldr     r1, [r0, #OMAP5912_DMA_GSCR_REGS_OA]
        orr     r1, r1,  #(1:SHL:0)
        orr     r1, r1,  #(1:SHL:3)
        str     r1, [r0, #OMAP5912_DMA_GSCR_REGS_OA]

        ;---------------------------------------------------------------
        ; Reset USB modules
        ;---------------------------------------------------------------

        ldr     r0, =OMAP5912_OTG_REGS_PA
        ldr     r1, =BSP_OTG_SYSCON_1
        str     r1, [r0, #OMAP5912_OTG_SYSCON_1_REGS_OA]
        ldr     r1, =BSP_OTG_SYSCON_2
        str     r1, [r0, #OMAP5912_OTG_SYSCON_2_REGS_OA]
        
;-------------------------------------------------------------------------------

        END
