;------------------------------------------------------------------------------
;
;   Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------

    IF :DEF: BSP_BOARD_RED  
    ;
    ; mDDR on red board
    ;
    ldr     r1, =CSP_BASE_REG_PA_ESDCTL
    ldr     r2, =CSP_BASE_MEM_PA_CSD0

    ; Configure for mDDR
    ldr     r0, =0x4
    str     r0, [r1, #ESDRAMC_ESDMISC_OFFSET]

    ; Wait for more than 200 us
    ldr     r3, =0x10000
wait
    subs    r3, r3, #1
    bne     wait

    ; Set precharge command
    ldr     r0, =0x92100000
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; Access SDRAM with A10 high to precharge all banks
    ldr     r0, =0x0
    strb    r0, [r2, #0x400]

    ; Set autorefresh command
    ldr     r0, =0xA2100000
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; Use writes to refresh all banks of SDRAM
    ldr     r0, =0x0
    strb    r0, [r2]
    strb    r0, [r2]

    ; Set load mode command
    ldr     r0, =0xB2100000
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; Mode Register: BL=8, CAS=3
    ldr     r0, =0x0
    strb    r0, [r2, #0x33]

    ; Extended Mode Register
    ldr     r3, =0x1
    ldr     r0, =0x0
    strb    r0, [r2, r3, LSL #24]

    ; Set normal mode command
    ; ROW: 13, COL: 10, DSIZ: 16bit D[15:0], refresh: 8192cycles/64ms
    ldr     r0, =0x83216080
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; Set PWDT to 64 clocks which is the most aggressive setting
    ldr     r0, =0x83216880
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; Configure timing parameters
    ; Optimized based on datasheet HYx18M1G16x_BF_rev100.pdf
    ldr     r0, =0x00295729
    str     r0, [r1, #ESDRAMC_ESDCFG0_OFFSET]

    ; Dummy write to set DQS low (needed in MX31, not sure about MX25)
    ldr     r0, =0x0
    str     r0, [r2]
   
    ELSE
    ;
    ; DDR2 on blue board
    ;
    ldr     r1, =CSP_BASE_REG_PA_ESDCTL ; Set ESDCTL SRRAm Controler base address in r1 register
    ldr     r2, =CSP_BASE_MEM_PA_CSD0   ; Set CSD0 memory base address in r2 register   
    
    ; Configure for DDR2
    ldr     r0, =0x24C
    str     r0, [r1, #ESDRAMC_ESDMISC_OFFSET]

    ;----------------------------------------
    ; Configure DDR2 memory on CSD0
    ;----------------------------------------
    ldr     r2, =CSP_BASE_MEM_PA_CSD0

    ; Configure timing parameters
    ldr     r0, =0x0076E83F
    str     r0, [r1, #ESDRAMC_ESDCFG0_OFFSET]

    ; Set precharge command
    ;
    ;   DSIZ - 16-bit align to D[15:0] (1 << 16)        = 0x00010000
    ;   COL - 10 column addresses (2 << 20)             = 0x00200000
    ;   ROW - 13 Row addresses (2 << 24)                = 0x02000000
    ;   SP - User mode access (0 << 27)                 = 0x00000000
    ;   SMODE - Precharge command (1 << 28)             = 0x10000000
    ;   SDE - Enable controller (1 << 31)               = 0x80000000
    ;                                                   ------------
    ;                                                     0x92210000
    ldr     r0, =0x92210000
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; Access SDRAM with A10 high to precharge all banks
    ldr     r0, =0x0
    strb    r0, [r2, #0x400]

    ; Set load mode command
    ;
    ;   DSIZ - 16-bit align to D[15:0] (1 << 16)        = 0x00010000
    ;   COL - 10 column addresses (2 << 20)             = 0x00200000
    ;   ROW - 13 Row addresses (2 << 24)                = 0x02000000
    ;   SP - User mode access (0 << 27)                 = 0x00000000
    ;   SMODE - Load mode command (3 << 28)             = 0x30000000
    ;   SDE - Enable controller (1 << 31)               = 0x80000000
    ;                                                   ------------
    ;                                                     0xB2210000
    ldr     r0, =0xB2210000
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; DDR2 EMR2
    ldr     r0, =0x0
    ldr     r3, =0x82000000
    strb    r0, [r3]
    
    ; DDR2 EMR3
    ldr     r0, =0x0
    ldr     r3, =0x83000000
    strb    r0, [r3]
    
    ; DDR2 EMR1: enable DLL, disable /DQS
    ldr     r0, =0x0 
    ldr     r3, =0x81000400
    strb    r0, [r3]
    
    ; DDR2 MR: reset DLL, BL=8, CL=3
    ldr     r0, =0x0
    strb    r0, [r2, #0x333]

    ; Set precharge command
    ;
    ;   DSIZ - 16-bit align to D[15:0] (1 << 16)        = 0x00010000
    ;   COL - 10 column addresses (2 << 20)             = 0x00200000
    ;   ROW - 13 Row addresses (2 << 24)                = 0x02000000
    ;   SP - User mode access (0 << 27)                 = 0x00000000
    ;   SMODE - Precharge command (1 << 28)             = 0x10000000
    ;   SDE - Enable controller (1 << 31)               = 0x80000000
    ;                                                   ------------
    ;                                                     0x92210000
    ldr     r0, =0x92210000
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; Access SDRAM with A10 high to precharge all banks
    ldr     r0, =0x0
    strb    r0, [r2, #0x400]

    ; Set autorefresh command
    ;
    ;   DSIZ - 16-bit align to D[15:0] (1 << 16)        = 0x00010000
    ;   COL - 10 column addresses (2 << 20)             = 0x00200000
    ;   ROW - 13 Row addresses (2 << 24)                = 0x02000000
    ;   SP - User mode access (0 << 27)                 = 0x00000000
    ;   SMODE - Autorefresh command (2 << 28)           = 0x20000000
    ;   SDE - Enable controller (1 << 31)               = 0x80000000
    ;                                                   ------------
    ;                                                     0xA2210000
    ldr     r0, =0xA2210000
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; Use writes to refresh all banks of SDRAM
    ldr     r0, =0x0
    strb    r0, [r2]
    strb    r0, [r2]

    ; Set load mode command
    ;
    ;   DSIZ - 16-bit align to D[15:0] (1 << 16)        = 0x00010000
    ;   COL - 10 column addresses (2 << 20)             = 0x00200000
    ;   ROW - 13 Row addresses (2 << 24)                = 0x02000000
    ;   SP - User mode access (0 << 27)                 = 0x00000000
    ;   SMODE - Load mode command (3 << 28)             = 0x30000000
    ;   SDE - Enable controller (1 << 31)               = 0x80000000
    ;                                                   ------------
    ;                                                     0xB2210000
    ldr     r0, =0xB2210000
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ; DDR2 MR: end DLL reset, BL=8, CL=3
    ldr     r0, =0x0
    strb    r0, [r2, #0x233]

    ; Hold for more than 200 cycles
    ldr     r0, =0x100
hold
    subs    r0, r0, #1
    bne     hold
    
    ; DDR2 EMR1: OCD calibration default
    ldr     r0, =0x0
    ldr     r3, =0x81000780
    strb    r0, [r3]
    
    ; DDR2 EMR1: OCD calibration exit, enable DLL, disable /DQS
    ldr     r0, =0x0
    ldr     r3, =0x81000400
    strb    r0, [r3]

    ; Set normal mode command
    ;
    ;   PRCT - Precharge timer disabled (0 << 5)        = 0x00000000
    ;   BL - Burst of 8 for SDR/DDR (1 << 7)            = 0x00000080
    ;   FP - No full page mode (0 << 8)                 = 0x00000000
    ;   PWDT - Power down timeout disabled (3 << 10)    = 0x00000000
    ;   SREFR - 4 rows refreshed each clock (3 << 13)   = 0x00006000
    ;   DSIZ - 16-bit align to D[15:0] (1 << 16)        = 0x00010000
    ;   COL - 10 column addresses (2 << 20)             = 0x00200000
    ;   ROW - 13 Row addresses (2 << 24)                = 0x02000000
    ;   SP - User mode access (0 << 27)                 = 0x00000000
    ;   SMODE - Normal mode command (0 << 28)           = 0x00000000
    ;   SDE - Enable controller (1 << 31)               = 0x80000000
    ;                                                   ------------
    ;                                                     0x82216080
    ldr     r0, =0x82216080
    str     r0, [r1, #ESDRAMC_ESDCTL0_OFFSET]

    ENDIF
    
    END
