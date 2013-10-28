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
;-------------------------------------------------------------------------------
;
;  Header: s3c6410.inc
;
;  This header file defines only those registers required by the startup
;  code. All addresses are based off the physical addresses (PA) defined
;  in s3c6410_base_reg.h (s3c6410_base_reg.inc).
;
;-------------------------------------------------------------------------------

;-------------------------------------------------
; CPU Revision Definition ( S3C6410 Has EVT0, EVT1
;-------------------------------------------------
EVT0        EQU    (36410100)
EVT1        EQU    (36410101)

;-------------------------------------------------
; System Clock Definition
;-------------------------------------------------

CLK_66_5MHZ   EQU    66500000
CLK_133MHZ    EQU    133000000
CLK_150MHZ    EQU    150000000
CLK_200MHZ    EQU    200000000
CLK_266MHZ    EQU    266000000
CLK_300MHZ    EQU    300000000
CLK_400MHZ    EQU    400000000
CLK_450MHZ    EQU    450000000
CLK_532MHZ    EQU    532000000
CLK_600MHZ    EQU    600000000
CLK_666MHZ    EQU    666000000        ; Sync
CLK_667MHZ    EQU    667000000        ; ASync
CLK_798MHZ    EQU    798000000        ; Sync
CLK_800MHZ    EQU    800000000        ; ASync
CLK_900MHZ    EQU    900000000
CLK_1332MHZ   EQU    1332000000

ECLK_96MHZ    EQU    96000000
ECLK_84MHZ    EQU    84666667    ; for IIS 44.1 KHz
ECLK_92MHZ    EQU    92160000    ; for IIS 48 KHz

    GBLA    CPU_REVISION
    GBLA    TARGET_ARM_CLK
    GBLA    S3C6410_ECLK
    GBLL    CHANGE_PLL_CLKDIV_ON_EBOOT
    GBLL    CHANGE_PLL_CLKDIV_ON_KERNEL
    GBLL    CLEAR_DRAM_ON_EBOOT
    GBLL    CLEAR_DRAM_ON_KERNEL

CHANGE_PLL_CLKDIV_ON_EBOOT  SETL    {TRUE}
CHANGE_PLL_CLKDIV_ON_KERNEL SETL    {TRUE}
CLEAR_DRAM_ON_EBOOT         SETL    {FALSE}
CLEAR_DRAM_ON_KERNEL        SETL    {FALSE}


;------------------------------------------------------------------------------
; Define: SYNCMODE
;
; SYNCMODE used to set cpu operation mode to syncronous mode or asyncronous mode
;------------------------------------------------------------------------------

    GBLL    SYNCMODE

SYNCMODE            SETL    {TRUE}

;-------------------------------------------------
; Change CPU Revision (S3C6410 HAS EVT0, EVT1)
;-------------------------------------------------
CPU_REVISION    SETA    EVT1
;-------------------------------------------------

;-------------------------------------------------
; Change TARGET_ARM_CLK definition for StartUp code
;-------------------------------------------------
;TARGET_ARM_CLK    SETA    CLK_66_5MHZ        ; Sync 66.5:66.5:66.5
;TARGET_ARM_CLK    SETA    CLK_133MHZ        ; Sync 133:133:66.5
;TARGET_ARM_CLK    SETA    CLK_266MHZ        ; Sync 266:133:66.5
;TARGET_ARM_CLK    SETA    CLK_400MHZ        ; Sync 400:100:50
;TARGET_ARM_CLK    SETA    CLK_450MHZ        ; Sync 450:150:65
TARGET_ARM_CLK    SETA    CLK_532MHZ        ; Sync 532:133:66.5
;TARGET_ARM_CLK    SETA    CLK_600MHZ        ; Sync 600:150:75
;TARGET_ARM_CLK    SETA    CLK_666MHZ        ; Sync 666:133.2:66.5
;TARGET_ARM_CLK    SETA    CLK_798MHZ        ; Sync 798:133:66.5
;TARGET_ARM_CLK    SETA    CLK_800MHZ        ; Sync 800:133.33:66.66
;TARGET_ARM_CLK    SETA    CLK_900MHZ        ; Sync 900:150:75

FIN        EQU    12000000
;-------------------------------------------------

; Include the base register definitions
; Fout = MDIV*Fin/(PDIV*2^SDIV)
; Fvco = MDIV*Fin/PDIV

    INCLUDE s3c6410_base_regs.inc

;-------------------------------------------------
; Change S3C6410_ECLK definition for EPLL Fout
;-------------------------------------------------
;S3C6410_ECLK    SETA    ECLK_96MHZ
S3C6410_ECLK    SETA    ECLK_84MHZ
;S3C6410_ECLK    SETA    ECLK_92MHZ
;-------------------------------------------------

;-------------------------------------------------
; Set Clock Source : MPLL, APLL
;-------------------------------------------------
; MPLL Setting
    ; 400:100:25 (Asyncronous Mode)
    [ (TARGET_ARM_CLK = CLK_400MHZ)
;Fvco=800MHz, Fout=200MHz
MPLL_MVAL    EQU    (400)
MPLL_PVAL    EQU    (6)
MPLL_SVAL    EQU    (2)
    |
    ; Other Clock use 266Mhz for mDDR in Asynchronous mode
;Fvco=1064MHz, Fout=266MHz
MPLL_MVAL    EQU    (266)
MPLL_PVAL    EQU    (3)
MPLL_SVAL    EQU    (2)
    ]
MPLL_CLK    EQU     (((FIN>>MPLL_SVAL)/MPLL_PVAL)*MPLL_MVAL)    ; MPLL Clock

; APLL Setting
    [ (TARGET_ARM_CLK = CLK_400MHZ)
;Fvco=800MHz, Fout=400MHz
APLL_MVAL    EQU    (400)
APLL_PVAL    EQU    (6)
APLL_SVAL    EQU    (1)
    ]
    [ (TARGET_ARM_CLK = CLK_532MHZ):LOR:(TARGET_ARM_CLK = CLK_266MHZ):LOR:(TARGET_ARM_CLK = CLK_133MHZ):LOR:(TARGET_ARM_CLK = CLK_66_5MHZ)
;Fvco=1064MHz, Fout=532MHz
APLL_MVAL    EQU    (266)
APLL_PVAL    EQU    (3)
APLL_SVAL    EQU    (1)
    ]
    [ (TARGET_ARM_CLK = CLK_600MHZ):LAND:(SYNCMODE)
;Fvco=1200MHz, Fout=600MHz
APLL_MVAL    EQU    (300)
APLL_PVAL    EQU    (3)
APLL_SVAL    EQU    (1)
    ]
    [ (TARGET_ARM_CLK = CLK_666MHZ)
        [ (SYNCMODE):LAND:(CPU_REVISION = EVT1)
;Fvco=1332MHz, Fout=1332MHz
APLL_MVAL    EQU    (333)
APLL_PVAL    EQU    (3)
APLL_SVAL    EQU    (0)
        |
;Fvco=1332MHz, Fout=666MHz
APLL_MVAL    EQU    (333)
APLL_PVAL    EQU    (3)
APLL_SVAL    EQU    (1)
        ]
    ]
    [ (TARGET_ARM_CLK = CLK_798MHZ):LAND:(SYNCMODE)
;Fvco=1596MHz, Fout=798MHz
APLL_MVAL    EQU    (399)
APLL_PVAL    EQU    (3)
APLL_SVAL    EQU    (1)
    ]
    [ (TARGET_ARM_CLK = CLK_800MHZ)
;Fvco=1600MHz, Fout=800MHz
APLL_MVAL    EQU    (400)
APLL_PVAL    EQU    (3)
APLL_SVAL    EQU    (1)
    ]
    [ ((TARGET_ARM_CLK = CLK_900MHZ):LAND:(SYNCMODE)):LOR:(TARGET_ARM_CLK = CLK_450MHZ)
;Fvco=900MHz, Fout=900MHz
APLL_MVAL    EQU    (225)
APLL_PVAL    EQU    (3)
APLL_SVAL    EQU    (0)
    ]
APLL_CLK        EQU    (((FIN>>APLL_SVAL)/APLL_PVAL)*APLL_MVAL)    ; APLL Clock

;-------------------------------------------------
; Set Clock Dividers
;-------------------------------------------------

MPLL_DIV    EQU    (2-1)    ; DOUT_MPLL = MPLL_Fout/2

    [(TARGET_ARM_CLK = CLK_450MHZ):LOR:(TARGET_ARM_CLK = CLK_666MHZ):LOR:(TARGET_ARM_CLK = CLK_266MHZ)
APLL_DIV    EQU    (2-1)    ; ARM_CLK = APLL_CLK/2
    |
    [(TARGET_ARM_CLK = CLK_133MHZ)
APLL_DIV    EQU    (4-1)    ; ARM_CLK = APLL_CLK/4
    |
    [(TARGET_ARM_CLK = CLK_66_5MHZ)
APLL_DIV    EQU    (8-1)    ; ARM_CLK = APLL_CLK/8
    |
APLL_DIV    EQU    (1-1)    ; ARM_CLK = APLL_CLK
    ]
    ]
    ]


    [ (SYNCMODE)   ; Use APLL as Memory Clock Source
        [ (TARGET_ARM_CLK = CLK_532MHZ):LOR:(TARGET_ARM_CLK = CLK_600MHZ):LOR:(TARGET_ARM_CLK = CLK_266MHZ):LOR:(TARGET_ARM_CLK = CLK_133MHZ)
; ARM:AHB:APB = 4:2:1, HCLKx2 = APLL_CLK/2
HCLKx2_DIV  EQU    (2-1)    ; HCLKx2 = APLL_CLK/2
        ]
        [ (TARGET_ARM_CLK = CLK_666MHZ):LAND:(CPU_REVISION = EVT1)     ; This setting requires enabling MISC_CON[19]
HCLKx2_DIV  EQU    (5-1)    ; HCLKx2 = APLL_CLK/5 = 266.4MHz(Hard wired-PreDivider on EVT1)
        ]
        [ (TARGET_ARM_CLK = CLK_798MHZ):LOR:(TARGET_ARM_CLK = CLK_900MHZ):LOR:(TARGET_ARM_CLK = CLK_450MHZ):LOR:(TARGET_ARM_CLK = CLK_800MHZ)
; ARM:AHB:APB = 12:2:1, HCLKx2 = APLL_CLK/3
HCLKx2_DIV  EQU    (3-1)    ; HCLKx2 = APLL_CLK/3
        ]
        [ (TARGET_ARM_CLK = CLK_66_5MHZ)
HCLKx2_DIV  EQU    (4-1)    ; HCLKx2 = APLL_CLK/4
        ]
    | ; Use MPLL as Memory Clock Source
        [ (TARGET_ARM_CLK = CLK_400MHZ):LOR:(TARGET_ARM_CLK = CLK_532MHZ):LOR:(TARGET_ARM_CLK = CLK_666MHZ)
; ARM:AHB:APB = 4:(2:1), HCLKx2 = MPLL
HCLKx2_DIV  EQU    (1-1)    ; HCLKx2 = MPLL_CLK
        ]
    ] ; (SYNCMODE)



HCLK_DIV    EQU    (2-1)    ; AHB_CLK = HCLKx2/2
    [ (TARGET_ARM_CLK = CLK_66_5MHZ)
PCLK_DIV    EQU    (2-1)    ; PCLK = HCLKx2/2
    |
PCLK_DIV    EQU    (4-1)    ; PCLK = HCLKx2/4
    ]

ARM_CLK    EQU    (APLL_CLK/(APLL_DIV+1))

    [ (SYNCMODE)
HCLK    EQU    (APLL_CLK/(HCLKx2_DIV+1)/(HCLK_DIV+1))
    |
HCLK    EQU    (MPLL_CLK/(HCLKx2_DIV+1)/(HCLK_DIV+1))
    ]

; For 532:133:66.5
; Set most Stable Clock as Stratup Clock
;Fvco=1064MHz, Fout=532MHz
Startup_APLL_MVAL    EQU    (266)
Startup_APLL_PVAL    EQU    (3)
Startup_APLL_SVAL    EQU    (1)
Startup_APLL_CLK     EQU    (((FIN>>Startup_APLL_SVAL)/Startup_APLL_PVAL)*Startup_APLL_MVAL)    ; APLL Clock

Startup_APLL_DIV    EQU    (1-1)    ; ARM_CLK = APLL_CLK
Startup_HCLKx2_DIV  EQU    (2-1)    ; HCLKx2 = APLL_CLK/2
Startup_HCLK_DIV    EQU    (2-1)    ; AHB_CLK = HCLKx2/2
Startup_PCLK_DIV    EQU    (4-1)    ; PCLK = HCLKx2/4
Startup_ARM_CLK    EQU    (Startup_APLL_CLK/(Startup_APLL_DIV+1))


    ; EPLL Fout 96 MHz
    [ S3C6410_ECLK = ECLK_96MHZ

;Fout=96MHz
EPLL_MVAL    EQU    (32)
EPLL_PVAL    EQU    (1)
EPLL_SVAL    EQU    (2)
EPLL_KVAL    EQU    (0)

      ]    ; 96 MHz

    ; EPLL Fout 84.666667 MHz
    [ S3C6410_ECLK = ECLK_84MHZ

;Fout=84.67MHz
EPLL_MVAL    EQU    (254)
EPLL_PVAL    EQU    (9)
EPLL_SVAL    EQU    (2)
EPLL_KVAL    EQU    (0)

      ]    ; 84.666667 MHz

    ; EPLL Fout 92,160,000 Hz
    [ S3C6410_ECLK = ECLK_92MHZ

;Fout=92.16MHz
EPLL_MVAL    EQU    (192)
EPLL_PVAL    EQU    (25)
EPLL_SVAL    EQU    (0)
EPLL_KVAL    EQU    (0)

      ]    ; 92.16 MHz

;---------------------------
; CPSR Mode Bit Definition
;---------------------------
Mode_USR    EQU    (0x10)
Mode_FIQ    EQU    (0x11)
Mode_IRQ    EQU    (0x12)
Mode_SVC    EQU    (0x13)
Mode_ABT    EQU    (0x17)
Mode_UND    EQU    (0x1B)
Mode_SYS    EQU    (0x1F)
Mode_MASK    EQU    (0x1F)
NOINT        EQU    (0xC0)
I_Bit        EQU    (0x80)
F_Bit        EQU    (0x40)

;---------------------------
; CP15 Mode Bit Definition
;---------------------------
R1_iA        EQU    (1<<31)
R1_nF        EQU    (1<<30)
R1_VE        EQU    (1<<24)
R1_I        EQU    (1<<12)
R1_BP        EQU    (1<<11)    ; Z bit
R1_C        EQU    (1<<2)
R1_A        EQU    (1<<1)
R1_M        EQU    (1<<0)

;---------------------------
;  Miscellaneous defines
;---------------------------
WORD_SIZE    EQU    (4)
DW8        EQU    (0x0)
DW16        EQU    (0x1)
DW32        EQU    (0x2)
WAIT        EQU    (0x1<<2)
UBLB        EQU    (0x1<<3)

;---------------------------
;
;  SFR Address
;
;---------------------------


;---------------------------
;  SysCon
;---------------------------

APLL_LOCK        EQU    (0x7e00f000)
MPLL_LOCK        EQU    (0x7e00f004)
APLL_CON        EQU    (0x7e00f00c)
MPLL_CON        EQU    (0x7e00f010)
EPLL_CON0        EQU    (0x7e00f014)
EPLL_CON1        EQU    (0x7e00f018)
CLK_SRC            EQU    (0x7e00f01c)
CLK_DIV0        EQU    (0x7e00f020)
CLK_OUT            EQU    (0x7e00f02c)
MEM_SYS_CFG        EQU    (0x7e00f120)
OTHERS            EQU    (0x7e00f900)
RST_STAT        EQU    (0x7E00F904)
INFORM0            EQU    (0x7E00FA00)
INFORM1            EQU    (0x7E00FA04)
INFORM2            EQU    (0x7E00FA08)
INFORM3            EQU    (0x7E00FA0C)

vPWR_CFG        EQU    (0xB2A0F804)
vSLEEP_CFG        EQU    (0xB2A0F818)
vOSC_STABLE        EQU    (0xB2A0F824)
vPWR_STABLE        EQU    (0xB2A0F828)
vRST_STAT        EQU    (0xB2A0F904)
vINFORM0        EQU    (0xB2A0FA00)
vINFORM1        EQU    (0xB2A0FA04)
vINFORM2        EQU    (0xB2A0FA08)
vINFORM3        EQU    (0xB2A0FA0C)

;---------------------------
;  GPIO
;---------------------------
GPACON        EQU    (S3C6410_BASE_REG_PA_GPIO + 0x000)
;GPHCON0    EQU    (S3C6410_BASE_REG_PA_IOPORT + 0x0E0)
;GPHCON1    EQU    (S3C6410_BASE_REG_PA_IOPORT + 0x0E4)
;GPHDAT        EQU    (S3C6410_BASE_REG_PA_IOPORT + 0x0E8)
;GPHPUD        EQU    (S3C6410_BASE_REG_PA_IOPORT + 0x0EC)

;GPH_OneND    EQU    0x2

GPNCON        EQU    (S3C6410_BASE_REG_PA_GPIO + 0x830)
GPNDAT        EQU    (S3C6410_BASE_REG_PA_GPIO + 0x834)
GPNPUD        EQU    (S3C6410_BASE_REG_PA_GPIO + 0x838)

MEM1DRVCON    EQU    (S3C6410_BASE_REG_PA_GPIO + 0x1D4)

;// 0x7F000000 -> 0x92B00000

;vGPHCON0    EQU    (0xB2B08000 + 0x0E0)
;vGPHCON1    EQU    (0xB2B08000 + 0x0E4)
;vGPHDAT    EQU    (0xB2B08000 + 0x0E8)
;vGPHPUD    EQU    (0xB2B08000 + 0x0EC)

vGPNCON                    EQU    (0xB2B08000 + 0x830)
vGPNDAT                    EQU    (0xB2B08000 + 0x834)
vGPNPUD                    EQU    (0xB2B08000 + 0x838)

;vGPNCON        EQU    (0xB2B08830)
vEINT0CON0        EQU    (0xB2B08900)

;---------------------------
;  VIC
;---------------------------
VIC0INTENCLEAR        EQU    (S3C6410_BASE_REG_PA_VIC0 + 0x14)
VIC1INTENCLEAR        EQU    (S3C6410_BASE_REG_PA_VIC1 + 0x14)

vVIC0INTENABLE        EQU     (0xB0600010) ; VIC0
vVIC0INTENCLEAR        EQU     (0xB0600014) ; VIC0

vVIC1INTENABLE        EQU     (0xB0700010) ; VIC1
vVIC1INTENCLEAR        EQU     (0xB0700014) ; VIC1

NFDATA        EQU    0x70200010    ; NAND Flash data register

;-----------------------------------
; Watch Dog Timer
;-----------------------------------
WTCON        EQU    (0x7e004000)

;=================
; SMC
;=================
SMBIDCYR0                EQU    0x70000000
SMBWSTRDR0                EQU    0x70000004
SMBWSTWRR0                EQU    0x70000008
SMBWSTOENR0                EQU    0x7000000c
SMBWSTWENR0                EQU    0x70000010
SMBCR0                    EQU    0x70000014
SMBSR0                    EQU    0x70000018
SMBWSTBRDR0                EQU    0x7000001c

SMBIDCYR1                EQU    0x70000020
SMBWSTRDR1                EQU    0x70000024
SMBWSTWRR1                EQU    0x70000028
SMBWSTOENR1                EQU    0x7000002c
SMBWSTWENR1                EQU    0x70000030
SMBCR1                    EQU    0x70000034
SMBSR1                    EQU    0x70000038
SMBWSTBRDR1                EQU    0x7000003c

SMBIDCYR2                EQU    0x70000040
SMBWSTRDR2                EQU    0x70000044
SMBWSTWRR2                EQU    0x70000048
SMBWSTOENR2                EQU    0x7000004c
SMBWSTWENR2                EQU    0x70000050
SMBCR2                    EQU    0x70000054
SMBSR2                    EQU    0x70000058
SMBWSTBRDR2                EQU    0x7000005c

SMBIDCYR3                EQU    0x70000060
SMBWSTRDR3                EQU    0x70000064
SMBWSTWRR3                EQU    0x70000068
SMBWSTOENR3                EQU    0x7000006c
SMBWSTWENR3                EQU    0x70000070
SMBCR3                    EQU    0x70000074
SMBSR3                    EQU    0x70000078
SMBWSTBRDR3                EQU    0x7000007c

SMBIDCYR4                EQU    0x70000080
SMBWSTRDR4                EQU    0x70000084
SMBWSTWRR4                EQU    0x70000088
SMBWSTOENR4                EQU    0x7000008c
SMBWSTWENR4                EQU    0x70000090
SMBCR4                    EQU    0x70000094
SMBSR4                    EQU    0x70000098
SMBWSTBRDR4                EQU    0x7000009c

SMBIDCYR5                EQU    0x700000a0
SMBWSTRDR5                EQU    0x700000a4
SMBWSTWRR5                EQU    0x700000a8
SMBWSTOENR5                EQU    0x700000ac
SMBWSTWENR5                EQU    0x700000b0
SMBCR5                    EQU    0x700000b4
SMBSR5                    EQU    0x700000b8
SMBWSTBRDR5                EQU    0x700000bc

SMBIDCYR6                EQU    0x700000c0
SMBWSTRDR6                EQU    0x700000c4
SMBWSTWRR6                EQU    0x700000c8
SMBWSTOENR6                EQU    0x700000cc
SMBWSTWENR6                EQU    0x700000d0
SMBCR6                    EQU    0x700000d4
SMBSR6                    EQU    0x700000d8
SMBWSTBRDR6                EQU    0x700000dc

SMBIDCYR7                EQU    0x700000e0
SMBWSTRDR7                EQU    0x700000e4
SMBWSTWRR7                EQU    0x700000e8
SMBWSTOENR7                EQU    0x700000ec
SMBWSTWENR7                EQU    0x700000f0
SMBCR7                    EQU    0x700000f4
SMBSR7                    EQU    0x700000f8
SMBWSTBRDR7                EQU    0x700000fc



;=================
; DMC
;=================
DMC0_BASE            EQU    0x7e000000
DMC1_BASE            EQU    0x7e001000

INDEX_MEMSTAT            EQU    0x0
INDEX_MEMCCMD            EQU    0x4
INDEX_DIRECTCMD            EQU    0x8
INDEX_MEMCFG            EQU    0xc
INDEX_REFRESH            EQU    0x10
INDEX_CASLAT            EQU    0x14
INDEX_T_DQSS            EQU    0x18
INDEX_T_MRD            EQU    0x1c
INDEX_T_RAS            EQU    0x20
INDEX_T_RC            EQU    0x24
INDEX_T_RCD            EQU    0x28
INDEX_T_RFC            EQU    0x2c
INDEX_T_RP            EQU    0x30
INDEX_T_RRD            EQU    0x34
INDEX_T_WR            EQU    0x38
INDEX_T_WTR            EQU    0x3c
INDEX_T_XP            EQU    0x40
INDEX_T_XSR            EQU    0x44
INDEX_T_ESR            EQU    0x48
INDEX_MEMCFG2            EQU    0x4c
INDEX_ID_0_CFG            EQU    0x100
INDEX_ID_1_CFG            EQU    0x104
INDEX_ID_2_CFG            EQU    0x108
INDEX_ID_3_CFG            EQU    0x10c
INDEX_ID_4_CFG            EQU    0x110
INDEX_ID_5_CFG            EQU    0x114
INDEX_ID_6_CFG            EQU    0x118
INDEX_ID_7_CFG            EQU    0x11c
INDEX_ID_8_CFG            EQU    0x120
INDEX_ID_9_CFG            EQU    0x124
INDEX_ID_10_CFG            EQU    0x128
INDEX_ID_11_CFG            EQU    0x12c
INDEX_ID_12_CFG            EQU    0x130
INDEX_ID_13_CFG            EQU    0x134
INDEX_ID_14_CFG            EQU    0x138
INDEX_ID_15_CFG            EQU    0x13c
INDEX_CHIP0_CFG            EQU    0x200
INDEX_CHIP1_CFG            EQU    0x204
INDEX_USER_STAT            EQU    0x300
INDEX_USER_CFG            EQU    0x304

;-------------------------------------------------------------------------------
; Memory Chip direct command
;-------------------------------------------------------------------------------
DMC_NOP0        EQU    0x0c0000
DMC_NOP1        EQU     0x1c0000
DMC_PA0            EQU    0x000000    ; Precharge all
DMC_PA1            EQU     0x100000
DMC_AR0            EQU     0x040000    ; Autorefresh
DMC_AR1            EQU     0x140000
DMC_SDR_MR0        EQU    0x080032    ; MRS, CAS 3, Burst Length 4
DMC_SDR_MR1        EQU    0x180032
DMC_DDR_MR0        EQU     0x080162
DMC_DDR_MR1        EQU     0x180162
DMC_mDDR_MR0        EQU    0x080032    ; CAS 3, Burst Length 4
DMC_mDDR_MR1        EQU     0x180032
DMC_mSDR_EMR0        EQU     0x0a0000    ; EMRS, DS:Full, PASR:Full Array
DMC_mSDR_EMR1        EQU     0x1a0000
DMC_DDR_EMR0        EQU     0x090000
DMC_DDR_EMR1        EQU     0x190000
DMC_mDDR_EMR0        EQU     0x0a0000    ; DS:Full, PASR:Full Array
DMC_mDDR_EMR1        EQU     0x1a0000

    END
