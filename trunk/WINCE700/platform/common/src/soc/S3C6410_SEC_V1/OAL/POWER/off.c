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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// Copyright (c) Samsung Electronics. Co. LTD. All rights reserved.

/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

    off.c

Abstract:

    control Power Off(Suspend, Sleep)/ Power On(Resume, Wakeup)

Functions:

    FMD_Init,

Notes:

--*/


#include <windows.h>
#include <pmplatform.h>
#include <oal.h>

#include <s3c6410.h>
#include "soc_cfg.h"

//#define SLEEP_AGING_TEST

// Vector Address Table Initialize Function in "\OAL\INTR\intr.c"
extern void VICTableInit(void);
// Initialization Function of SystemTimer in "\OAL\TIMER\timer.c"
extern void InitSystemTimer(UINT32 countsPerSysTick);

extern UINT32 g_oalWakeMask;

volatile UINT32 g_oalWakeSource = SYSWAKE_UNKNOWN;

static UINT32 g_aSleepSave_VIC[100];
static UINT32 g_aSleepSave_GPIO[100];
static UINT32 g_aSleepSave_SysCon[100];
static UINT32 g_aSleepSave_DMACon0[100];
static UINT32 g_aSleepSave_DMACon1[100];
//static UINT32 g_LastWakeupStatus = 0;

static void OEMInitializeSystemTimer(UINT32 msecPerSysTick, UINT32 countsPerMSec, UINT32 countsMargin)
{
    UINT32 countsPerSysTick;

    // Validate Input parameters
    countsPerSysTick = msecPerSysTick * countsPerMSec;

    InitSystemTimer(countsPerSysTick);
}

static void S3C6410_SaveState_VIC(void *pVIC0, void *pVIC1, UINT32 *pBuffer)
{
    volatile S3C6410_VIC_REG *pVIC0Reg;
    volatile S3C6410_VIC_REG *pVIC1Reg;

    pVIC0Reg = (S3C6410_VIC_REG *)pVIC0;
    pVIC1Reg = (S3C6410_VIC_REG *)pVIC1;

    *pBuffer++ = pVIC0Reg->VICINTSELECT;
    *pBuffer++ = pVIC0Reg->VICINTENABLE;
    *pBuffer++ = pVIC0Reg->VICSOFTINT;
    *pBuffer++ = pVIC1Reg->VICINTSELECT;
    *pBuffer++ = pVIC1Reg->VICINTENABLE;
    *pBuffer++ = pVIC1Reg->VICSOFTINT;

    // Do not Save VICVECTADDRXX
    // Reinitialize Vector Address Table with VICTableInit()
}

static void S3C6410_RestoreState_VIC(void *pVIC0, void *pVIC1, UINT32 *pBuffer)
{
    volatile S3C6410_VIC_REG *pVIC0Reg;
    volatile S3C6410_VIC_REG *pVIC1Reg;

    pVIC0Reg = (S3C6410_VIC_REG *)pVIC0;
    pVIC1Reg = (S3C6410_VIC_REG *)pVIC1;

    pVIC0Reg->VICINTSELECT = *pBuffer++;
    pVIC0Reg->VICINTENABLE = *pBuffer++;
    pVIC0Reg->VICSOFTINT = *pBuffer++;
    pVIC1Reg->VICINTSELECT = *pBuffer++;
    pVIC1Reg->VICINTENABLE = *pBuffer++;
    pVIC1Reg->VICSOFTINT = *pBuffer++;

    // Do not Restore VICVECTADDRXX
    // Reinitialize Vector Address Table with VICTableInit()
    VICTableInit();
}

static void S3C6410_SaveState_GPIO(void *pGPIO, UINT32 *pBuffer)
{
    volatile S3C6410_GPIO_REG *pGPIOReg;

    pGPIOReg = (S3C6410_GPIO_REG *)pGPIO;

    // GPIO A
    *pBuffer++ = pGPIOReg->GPACON;        // 000
    *pBuffer++ = pGPIOReg->GPADAT;        // 004
    *pBuffer++ = pGPIOReg->GPAPUD;        // 008
    //GPACONSLP;                        // 00c
    //GPAPUDSLP;                        // 010

    // GPIO B
    *pBuffer++ = pGPIOReg->GPBCON;        // 020
    *pBuffer++ = pGPIOReg->GPBDAT;        // 024
    *pBuffer++ = pGPIOReg->GPBPUD;        // 028
    //GPBCONSLP;                        // 02c
    //GPBPUDSLP;                        // 030

    // GPIO C
    *pBuffer++ = pGPIOReg->GPCCON;        // 040
    *pBuffer++ = pGPIOReg->GPCDAT;        // 044
    *pBuffer++ = pGPIOReg->GPCPUD;        // 048
    //GPCCONSLP;                        // 04c
    //GPCPUDSLP;                        // 050

    // GPIO D
    *pBuffer++ = pGPIOReg->GPDCON;        // 060
    *pBuffer++ = pGPIOReg->GPDDAT;        // 064
    *pBuffer++ = pGPIOReg->GPDPUD;        // 068
    //GPDCONSLP;                        // 06c
    //GPDPUDSLP;                        // 070

    // GPIO E
    *pBuffer++ = pGPIOReg->GPECON;        // 080
    *pBuffer++ = pGPIOReg->GPEDAT;        // 084
    *pBuffer++ = pGPIOReg->GPEPUD;        // 088
    //GPECONSLP;                        // 08c
    //GPEPUDSLP;                        // 090

    // GPIO F
    *pBuffer++ = pGPIOReg->GPFCON;        // 0a0
    *pBuffer++ = pGPIOReg->GPFDAT;        // 0a4
    *pBuffer++ = pGPIOReg->GPFPUD;        // 0a8
    //GPFCONSLP;                        // 0ac
    //GPFPUDSLP;                        // 0b0

    // GPIO G
    *pBuffer++ = pGPIOReg->GPGCON;        // 0c0
    *pBuffer++ = pGPIOReg->GPGDAT;        // 0c4
    *pBuffer++ = pGPIOReg->GPGPUD;        // 0c8
    //GPGCONSLP;                        // 0cc
    //GPGPUDSLP;                        // 0d0

    // GPIO H
    *pBuffer++ = pGPIOReg->GPHCON0;    // 0e0
    *pBuffer++ = pGPIOReg->GPHCON1;    // 0e4
    *pBuffer++ = pGPIOReg->GPHDAT;        // 0e8
    *pBuffer++ = pGPIOReg->GPHPUD;        // 0ec
    //GPHCONSLP;                        // 0f0
    //GPHPUDSLP;                        // 0f4

    // GPIO I
    *pBuffer++ = pGPIOReg->GPICON;        // 100
    *pBuffer++ = pGPIOReg->GPIDAT;        // 104
    *pBuffer++ = pGPIOReg->GPIPUD;        // 108
    //GPICONSLP;                        // 10c
    //GPIPUDSLP;                        // 110

    // GPIO J
    *pBuffer++ = pGPIOReg->GPJCON;        // 120
    *pBuffer++ = pGPIOReg->GPJDAT;        // 124
    *pBuffer++ = pGPIOReg->GPJPUD;        // 128
    //GPJCONSLP;                        // 12c
    //GPJPUDSLP;                        // 130

    // GPIO K
    *pBuffer++ = pGPIOReg->GPKCON0;    // 800
    *pBuffer++ = pGPIOReg->GPKCON1;    // 804
    *pBuffer++ = pGPIOReg->GPKDAT;        // 808
    *pBuffer++ = pGPIOReg->GPKPUD;        // 80c

    // GPIO L
    *pBuffer++ = pGPIOReg->GPLCON0;    // 810
    *pBuffer++ = pGPIOReg->GPLCON1;    // 814
    *pBuffer++ = pGPIOReg->GPLDAT;        // 818
    *pBuffer++ = pGPIOReg->GPLPUD;        // 81c

    // GPIO M
    *pBuffer++ = pGPIOReg->GPMCON;        // 820
    *pBuffer++ = pGPIOReg->GPMDAT;        // 824
    *pBuffer++ = pGPIOReg->GPMPUD;        // 828

    // GPIO N
    *pBuffer++ = pGPIOReg->GPNCON;        // 830
    *pBuffer++ = pGPIOReg->GPNDAT;        // 834
    *pBuffer++ = pGPIOReg->GPNPUD;        // 838

    // GPIO O
    *pBuffer++ = pGPIOReg->GPOCON;        // 140
    *pBuffer++ = pGPIOReg->GPODAT;        // 144
    *pBuffer++ = pGPIOReg->GPOPUD;        // 148
    //GPOCONSLP;                        // 14c
    //GPOPUDSLP;                        // 150

    // GPIO P
    *pBuffer++ = pGPIOReg->GPPCON;        // 160
    *pBuffer++ = pGPIOReg->GPPDAT;        // 164
    *pBuffer++ = pGPIOReg->GPPPUD;        // 168
    //GPPCONSLP;                        // 16c
    //GPPPUDSLP;                        // 170

    // GPIO Q
    *pBuffer++ = pGPIOReg->GPQCON;        // 180
    *pBuffer++ = pGPIOReg->GPQDAT;        // 184
    *pBuffer++ = pGPIOReg->GPQPUD;        // 188
    //GPQCONSLP;                        // 18c
    //GPQPUDSLP;                        // 190

    // Special Port Control
    *pBuffer++ = pGPIOReg->SPCON;            // 1a0
    //SPCONSLP;                                // 880    // Configure for Sleep Mode

    // Memory Port Control
    *pBuffer++ = pGPIOReg->MEM0CONSTOP;    // 1b0
    *pBuffer++ = pGPIOReg->MEM1CONSTOP;    // 1b4

    *pBuffer++ = pGPIOReg->MEM0CONSLP0;    // 1c0
    *pBuffer++ = pGPIOReg->MEM0CONSLP1;    // 1c4
    *pBuffer++ = pGPIOReg->MEM1CONSLP;        // 1c8

    *pBuffer++ = pGPIOReg->MEM0DRVCON;    // 1d0
    *pBuffer++ = pGPIOReg->MEM1DRVCON;    // 1d4

    // External Interrupt
    *pBuffer++ = pGPIOReg->EINT12CON;        // 200
    *pBuffer++ = pGPIOReg->EINT34CON;        // 204
    *pBuffer++ = pGPIOReg->EINT56CON;        // 208
    *pBuffer++ = pGPIOReg->EINT78CON;        // 20c
    *pBuffer++ = pGPIOReg->EINT9CON;        // 210

    *pBuffer++ = pGPIOReg->EINT12FLTCON;    // 220
    *pBuffer++ = pGPIOReg->EINT34FLTCON;    // 224
    *pBuffer++ = pGPIOReg->EINT56FLTCON;    // 228
    *pBuffer++ = pGPIOReg->EINT78FLTCON;    // 22c
    *pBuffer++ = pGPIOReg->EINT9FLTCON;        // 230

    *pBuffer++ = pGPIOReg->EINT12MASK;        // 240
    *pBuffer++ = pGPIOReg->EINT34MASK;        // 244
    *pBuffer++ = pGPIOReg->EINT56MASK;        // 248
    *pBuffer++ = pGPIOReg->EINT78MASK;        // 24c
    *pBuffer++ = pGPIOReg->EINT9MASK;        // 250

    //EINT12PEND;                            // 260    // Do not Save Pending Bit
    //EINT34PEND;                            // 264
    //EINT56PEND;                            // 268
    //EINT78PEND;                            // 26c
    //EINT9PEND;                            // 270

    *pBuffer++ = pGPIOReg->PRIORITY;        // 280
    //SERVICE;                                // 284    // Do not Save Read Only Register (But Check before enter sleep...)
    //SERVICEPEND;                            // 288    // Do not Save Pending Bit

    // External Interrupt Group 0
    *pBuffer++ = pGPIOReg->EINT0CON0;        // 900
    *pBuffer++ = pGPIOReg->EINT0CON1;        // 904

    *pBuffer++ = pGPIOReg->EINT0FLTCON0;    // 910
    *pBuffer++ = pGPIOReg->EINT0FLTCON1;    // 914
    *pBuffer++ = pGPIOReg->EINT0FLTCON2;    // 918
    *pBuffer++ = pGPIOReg->EINT0FLTCON3;    // 91c

    *pBuffer++ = pGPIOReg->EINT0MASK;        // 920
    //EINT0PEND;                            // 924    // Do not Save Pending Bit

    // Sleep Mode Pad Configure Register
    //SLPEN;                                    // 930    // Configure for Sleep Mode
}

static void S3C6410_RestoreState_GPIO(void *pGPIO, UINT32 *pBuffer)
{
    volatile S3C6410_GPIO_REG *pGPIOReg;

    pGPIOReg = (S3C6410_GPIO_REG *)pGPIO;

    // GPIO A
    pGPIOReg->GPACON = *pBuffer++;        // 000
    pGPIOReg->GPADAT = *pBuffer++;        // 004
    pGPIOReg->GPAPUD = *pBuffer++;        // 008
    //GPACONSLP;                        // 00c
    //GPAPUDSLP;                        // 010

    // GPIO B
    pGPIOReg->GPBCON = *pBuffer++;        // 020
    pGPIOReg->GPBDAT = *pBuffer++;        // 024
    pGPIOReg->GPBPUD = *pBuffer++;        // 028
    //GPBCONSLP;                        // 02c
    //GPBPUDSLP;                        // 030

    // GPIO C
    pGPIOReg->GPCCON = *pBuffer++;        // 040
    pGPIOReg->GPCDAT = *pBuffer++;        // 044
    pGPIOReg->GPCPUD = *pBuffer++;        // 048
    //GPCCONSLP;                        // 04c
    //GPCPUDSLP;                        // 050

    // GPIO D
    pGPIOReg->GPDCON = *pBuffer++;        // 060
    pGPIOReg->GPDDAT = *pBuffer++;        // 064
    pGPIOReg->GPDPUD = *pBuffer++;        // 068
    //GPDCONSLP;                        // 06c
    //GPDPUDSLP;                        // 070

    // GPIO E
    pGPIOReg->GPECON = *pBuffer++;        // 080
    pGPIOReg->GPEDAT = *pBuffer++;        // 084
    pGPIOReg->GPEPUD = *pBuffer++;        // 088
    //GPECONSLP;                        // 08c
    //GPEPUDSLP;                        // 090

    // GPIO F
    pGPIOReg->GPFCON = *pBuffer++;        // 0a0
    pGPIOReg->GPFDAT = *pBuffer++;        // 0a4
    pGPIOReg->GPFPUD = *pBuffer++;        // 0a8
    //GPFCONSLP;                        // 0ac
    //GPFPUDSLP;                        // 0b0

    // GPIO G
    pGPIOReg->GPGCON = *pBuffer++;        // 0c0
    pGPIOReg->GPGDAT = *pBuffer++;        // 0c4
    pGPIOReg->GPGPUD = *pBuffer++;        // 0c8
    //GPGCONSLP;                        // 0cc
    //GPGPUDSLP;                        // 0d0

    // GPIO H
    pGPIOReg->GPHCON0 = *pBuffer++;    // 0e0
    pGPIOReg->GPHCON1 = *pBuffer++;    // 0e4
    pGPIOReg->GPHDAT = *pBuffer++;        // 0e8
    pGPIOReg->GPHPUD = *pBuffer++;        // 0ec
    //GPHCONSLP;                        // 0f0
    //GPHPUDSLP;                        // 0f4

    // GPIO I
    pGPIOReg->GPICON = *pBuffer++;        // 100
    pGPIOReg->GPIDAT = *pBuffer++;        // 104
    pGPIOReg->GPIPUD = *pBuffer++;        // 108
    //GPICONSLP;                        // 10c
    //GPIPUDSLP;                        // 110

    // GPIO J
    pGPIOReg->GPJCON = *pBuffer++;        // 120
    pGPIOReg->GPJDAT = *pBuffer++;        // 124
    pGPIOReg->GPJPUD = *pBuffer++;        // 128
    //GPJCONSLP;                        // 12c
    //GPJPUDSLP;                        // 130

    // GPIO K
    pGPIOReg->GPKCON0 = *pBuffer++;    // 800
    pGPIOReg->GPKCON1 = *pBuffer++;    // 804
    pGPIOReg->GPKDAT = *pBuffer++;        // 808
    pGPIOReg->GPKPUD = *pBuffer++;        // 80c

    // GPIO L
    pGPIOReg->GPLCON0 = *pBuffer++;    // 810
    pGPIOReg->GPLCON1 = *pBuffer++;    // 814
    pGPIOReg->GPLDAT = *pBuffer++;        // 818
    pGPIOReg->GPLPUD = *pBuffer++;        // 81c

    // GPIO M
    pGPIOReg->GPMCON = *pBuffer++;        // 820
    pGPIOReg->GPMDAT = *pBuffer++;        // 824
    pGPIOReg->GPMPUD = *pBuffer++;        // 828

    // GPIO N
    pGPIOReg->GPNCON = *pBuffer++;        // 830
    pGPIOReg->GPNDAT = *pBuffer++;        // 834
    pGPIOReg->GPNPUD = *pBuffer++;        // 838

    // GPIO O
    pGPIOReg->GPOCON = *pBuffer++;        // 140
    pGPIOReg->GPODAT = *pBuffer++;        // 144
    pGPIOReg->GPOPUD = *pBuffer++;        // 148
    //GPOCONSLP;                        // 14c
    //GPOPUDSLP;                        // 150

    // GPIO P
    pGPIOReg->GPPCON = *pBuffer++;        // 160
    pGPIOReg->GPPDAT = *pBuffer++;        // 164
    pGPIOReg->GPPPUD = *pBuffer++;        // 168
    //GPPCONSLP;                        // 16c
    //GPPPUDSLP;                        // 170

    // GPIO Q
    pGPIOReg->GPQCON = *pBuffer++;        // 180
    pGPIOReg->GPQDAT = *pBuffer++;        // 184
    pGPIOReg->GPQPUD = *pBuffer++;        // 188
    //GPQCONSLP;                        // 18c
    //GPQPUDSLP;                        // 190

    // Special Port Control
    pGPIOReg->SPCON = *pBuffer++;            // 1a0
    //SPCONSLP;                                // 880    // Configure for Sleep Mode

    // Memory Port Control
    pGPIOReg->MEM0CONSTOP = *pBuffer++;    // 1b0
    pGPIOReg->MEM1CONSTOP = *pBuffer++;    // 1b4

    pGPIOReg->MEM0CONSLP0 = *pBuffer++;    // 1c0
    pGPIOReg->MEM0CONSLP1 = *pBuffer++;    // 1c4
    pGPIOReg->MEM1CONSLP = *pBuffer++;        // 1c8

    pGPIOReg->MEM0DRVCON = *pBuffer++;    // 1d0
    pGPIOReg->MEM1DRVCON = *pBuffer++;    // 1d4

    // External Interrupt
    pGPIOReg->EINT12CON = *pBuffer++;        // 200
    pGPIOReg->EINT34CON = *pBuffer++;        // 204
    pGPIOReg->EINT56CON = *pBuffer++;        // 208
    pGPIOReg->EINT78CON = *pBuffer++;        // 20c
    pGPIOReg->EINT9CON = *pBuffer++;        // 210

    pGPIOReg->EINT12FLTCON = *pBuffer++;    // 220
    pGPIOReg->EINT34FLTCON = *pBuffer++;    // 224
    pGPIOReg->EINT56FLTCON = *pBuffer++;    // 228
    pGPIOReg->EINT78FLTCON = *pBuffer++;    // 22c
    pGPIOReg->EINT9FLTCON = *pBuffer++;        // 230

    pGPIOReg->EINT12MASK = *pBuffer++;        // 240
    pGPIOReg->EINT34MASK = *pBuffer++;        // 244
    pGPIOReg->EINT56MASK = *pBuffer++;        // 248
    pGPIOReg->EINT78MASK = *pBuffer++;        // 24c
    pGPIOReg->EINT9MASK = *pBuffer++;        // 250

    //EINT12PEND;                            // 260    // Do not Save Pending Bit
    //EINT34PEND;                            // 264
    //EINT56PEND;                            // 268
    //EINT78PEND;                            // 26c
    //EINT9PEND;                            // 270

    pGPIOReg->PRIORITY = *pBuffer++;        // 280
    //SERVICE;                                // 284    // Do not Save Read Only Register (But Check before enter sleep...)
    //SERVICEPEND;                            // 288    // Do not Save Pending Bit

    // External Interrupt Group 0
    pGPIOReg->EINT0CON0 = *pBuffer++;        // 900
    pGPIOReg->EINT0CON1 = *pBuffer++;        // 904

    pGPIOReg->EINT0FLTCON0 = *pBuffer++;    // 910
    pGPIOReg->EINT0FLTCON1 = *pBuffer++;    // 914
    pGPIOReg->EINT0FLTCON2 = *pBuffer++;    // 918
    pGPIOReg->EINT0FLTCON3 = *pBuffer++;    // 91c

    pGPIOReg->EINT0MASK = *pBuffer++;        // 920
    //EINT0PEND;                            // 924    // Do not Save Pending Bit

    // Sleep Mode Pad Configure Register
    //SLPEN;                                    // 930    // Configure for Sleep Mode

}

static void S3C6410_SaveState_SysCon(void *pSysCon, UINT32 *pBuffer)
{
    volatile S3C6410_SYSCON_REG *pSysConReg;

    pSysConReg = (S3C6410_SYSCON_REG *)pSysCon;

    //APLL_LOCK;        // Reconfiguration
    //MPLL_LOCK;        // Reconfiguration
    //EPLL_LOCK;        // Reconfiguration
    //APLL_CON;            // Reconfiguration
    //MPLL_CON;            // Reconfiguration
    //EPLL_CON0;        // Reconfiguration
    //EPLL_CON1;        // Reconfiguration

    *pBuffer++ = pSysConReg->CLK_SRC;
    *pBuffer++ = pSysConReg->CLK_DIV0;
    *pBuffer++ = pSysConReg->CLK_DIV1;
    *pBuffer++ = pSysConReg->CLK_DIV2;
    *pBuffer++ = pSysConReg->CLK_OUT;

    *pBuffer++ = pSysConReg->HCLK_GATE;
    *pBuffer++ = pSysConReg->PCLK_GATE;
    *pBuffer++ = pSysConReg->SCLK_GATE;

    *pBuffer++ = pSysConReg->AHB_CON0;
    *pBuffer++ = pSysConReg->AHB_CON1;
    *pBuffer++ = pSysConReg->AHB_CON2;
    *pBuffer++ = pSysConReg->SDMA_SEL;

    //SW_RST;            // Reset Trigger
    //SYS_ID;            // Read Only

    *pBuffer++ = pSysConReg->MEM_SYS_CFG;
    *pBuffer++ = pSysConReg->QOS_OVERRIDE0;
    *pBuffer++ = pSysConReg->QOS_OVERRIDE1;
    //MEM_CFG_STAT;    // Read Only

    //PWR_CFG;            // Retension
    //EINT_MASK;        // Retension
    *pBuffer++ = pSysConReg->NORMAL_CFG;    // Retension, But H/W Problem
    //STOP_CFG;            // Retension
    //SLEEP_CFG;        // Retension

    //OSC_FREQ;            // Retension
    //OSC_STABLE;        // Retension
    //PWR_STABLE;        // Retension
    //FPC_STABLE;        // Retension
    //MTC_STABLE;        // Retension

    //OTHERS;            // Retension
    //RST_STAT;            // Retension, Read Only
    //WAKEUP_STAT;        // Retension
    //BLK_PWR_STAT;        // Retension, Read Only

    //INFORM0;            // Retension
    //INFORM1;            // Retension
    //INFORM2;            // Retension
    //INFORM3;            // Retension
    //INFORM4;            // Retension
    //INFORM5;            // Retension
    //INFORM6;            // Retension
    //INFORM7;            // Retension
}

static void S3C6410_RestoreState_SysCon(void *pSysCon, UINT32 *pBuffer)
{
    volatile S3C6410_SYSCON_REG *pSysConReg;

    pSysConReg = (S3C6410_SYSCON_REG *)pSysCon;
    //APLL_LOCK;        // Reconfiguration
    //MPLL_LOCK;        // Reconfiguration
    //EPLL_LOCK;        // Reconfiguration
    //APLL_CON;            // Reconfiguration
    //MPLL_CON;            // Reconfiguration
    //EPLL_CON0;        // Reconfiguration
    //EPLL_CON1;        // Reconfiguration

    pSysConReg->CLK_SRC = *pBuffer++;
    pSysConReg->CLK_DIV0 = *pBuffer++;
    pSysConReg->CLK_DIV1 = *pBuffer++;
    pSysConReg->CLK_DIV2 = *pBuffer++;
    pSysConReg->CLK_OUT = *pBuffer++;

    pSysConReg->HCLK_GATE = *pBuffer++;
    pSysConReg->PCLK_GATE = *pBuffer++;
    pSysConReg->SCLK_GATE = *pBuffer++;

    pSysConReg->AHB_CON0 = *pBuffer++;
    pSysConReg->AHB_CON1 = *pBuffer++;
    pSysConReg->AHB_CON2 = *pBuffer++;
    pSysConReg->SDMA_SEL = *pBuffer++;

    //SW_RST;            // Reset Trigger
    //SYS_ID;            // Read Only

    pSysConReg->MEM_SYS_CFG = *pBuffer++;
    pSysConReg->QOS_OVERRIDE0 = *pBuffer++;
    pSysConReg->QOS_OVERRIDE1 = *pBuffer++;
    //MEM_CFG_STAT;    // Read Only

    //PWR_CFG;            // Retension
    //EINT_MASK;        // Retension
    pSysConReg->NORMAL_CFG = *pBuffer++;    // Retension, But H/W Problem
    //STOP_CFG;            // Retension
    //SLEEP_CFG;        // Retension

    //OSC_FREQ;            // Retension
    //OSC_STABLE;        // Retension
    //PWR_STABLE;        // Retension
    //FPC_STABLE;        // Retension
    //MTC_STABLE;        // Retension

    //OTHERS;            // Retension
    //RST_STAT;            // Retension, Read Only
    //WAKEUP_STAT;        // Retension
    //BLK_PWR_STAT;        // Retension, Read Only

    //INFORM0;            // Retension
    //INFORM1;            // Retension
    //INFORM2;            // Retension
    //INFORM3;            // Retension
    //INFORM4;            // Retension
    //INFORM5;            // Retension
    //INFORM6;            // Retension
    //INFORM7;            // Retension
}

static void S3C6410_SaveState_DMACon(void *pDMAC, UINT32 *pBuffer)
{
    volatile S3C6410_DMAC_REG *pDMACReg;

    pDMACReg = (S3C6410_DMAC_REG *)pDMAC;

    //DMACIntStatus;            // Read-Only
    //DMACIntTCStatus;        // Read-Only
    //DMACIntTCClear;        // Clear Register
    //DMACIntErrStatus;        // Read-Only
    //DMACIntErrClear;        // Clear Register
    //DMACRawIntTCStatus;    // Read-Only
    //DMACRawIntErrStatus;    // Read-Only
    //DMACEnbldChns;        // Read-Only

    //DMACSoftBReq;            // No use
    //DMACSoftSReq;            // No use
    //DMACSoftLBReq;        // No use
    //DMACSoftLSReq;        // No use

    *pBuffer++ = pDMACReg->DMACConfiguration;
    *pBuffer++ = pDMACReg->DMACSync;

    *pBuffer++ = pDMACReg->DMACC0SrcAddr;
    *pBuffer++ = pDMACReg->DMACC0DestAddr;
    *pBuffer++ = pDMACReg->DMACC0LLI;
    *pBuffer++ = pDMACReg->DMACC0Control0;
    *pBuffer++ = pDMACReg->DMACC0Control1;
    *pBuffer++ = pDMACReg->DMACC0Configuration;

    *pBuffer++ = pDMACReg->DMACC1SrcAddr;
    *pBuffer++ = pDMACReg->DMACC1DestAddr;
    *pBuffer++ = pDMACReg->DMACC1LLI;
    *pBuffer++ = pDMACReg->DMACC1Control0;
    *pBuffer++ = pDMACReg->DMACC1Control1;
    *pBuffer++ = pDMACReg->DMACC1Configuration;

    *pBuffer++ = pDMACReg->DMACC2SrcAddr;
    *pBuffer++ = pDMACReg->DMACC2DestAddr;
    *pBuffer++ = pDMACReg->DMACC2LLI;
    *pBuffer++ = pDMACReg->DMACC2Control0;
    *pBuffer++ = pDMACReg->DMACC2Control1;
    *pBuffer++ = pDMACReg->DMACC2Configuration;

    *pBuffer++ = pDMACReg->DMACC3SrcAddr;
    *pBuffer++ = pDMACReg->DMACC3DestAddr;
    *pBuffer++ = pDMACReg->DMACC3LLI;
    *pBuffer++ = pDMACReg->DMACC3Control0;
    *pBuffer++ = pDMACReg->DMACC3Control1;
    *pBuffer++ = pDMACReg->DMACC3Configuration;

    *pBuffer++ = pDMACReg->DMACC4SrcAddr;
    *pBuffer++ = pDMACReg->DMACC4DestAddr;
    *pBuffer++ = pDMACReg->DMACC4LLI;
    *pBuffer++ = pDMACReg->DMACC4Control0;
    *pBuffer++ = pDMACReg->DMACC4Control1;
    *pBuffer++ = pDMACReg->DMACC4Configuration;

    *pBuffer++ = pDMACReg->DMACC5SrcAddr;
    *pBuffer++ = pDMACReg->DMACC5DestAddr;
    *pBuffer++ = pDMACReg->DMACC5LLI;
    *pBuffer++ = pDMACReg->DMACC5Control0;
    *pBuffer++ = pDMACReg->DMACC5Control1;
    *pBuffer++ = pDMACReg->DMACC5Configuration;

    *pBuffer++ = pDMACReg->DMACC6SrcAddr;
    *pBuffer++ = pDMACReg->DMACC6DestAddr;
    *pBuffer++ = pDMACReg->DMACC6LLI;
    *pBuffer++ = pDMACReg->DMACC6Control0;
    *pBuffer++ = pDMACReg->DMACC6Control1;
    *pBuffer++ = pDMACReg->DMACC6Configuration;

    *pBuffer++ = pDMACReg->DMACC7SrcAddr;
    *pBuffer++ = pDMACReg->DMACC7DestAddr;
    *pBuffer++ = pDMACReg->DMACC7LLI;
    *pBuffer++ = pDMACReg->DMACC7Control0;
    *pBuffer++ = pDMACReg->DMACC7Control1;
    *pBuffer++ = pDMACReg->DMACC7Configuration;
}

static void S3C6410_RestoreState_DMACon(void *pDMAC, UINT32 *pBuffer)
{
    volatile S3C6410_DMAC_REG *pDMACReg;

    pDMACReg = (S3C6410_DMAC_REG *)pDMAC;

    //DMACIntStatus;            // Read-Only
    //DMACIntTCStatus;        // Read-Only
    //DMACIntTCClear;        // Clear Register
    //DMACIntErrStatus;        // Read-Only
    //DMACIntErrClear;        // Clear Register
    //DMACRawIntTCStatus;    // Read-Only
    //DMACRawIntErrStatus;    // Read-Only
    //DMACEnbldChns;        // Read-Only

    //DMACSoftBReq;            // No use
    //DMACSoftSReq;            // No use
    //DMACSoftLBReq;        // No use
    //DMACSoftLSReq;        // No use

    pDMACReg->DMACConfiguration = *pBuffer++;
    pDMACReg->DMACSync = *pBuffer++;

    pDMACReg->DMACC0SrcAddr = *pBuffer++;
    pDMACReg->DMACC0DestAddr = *pBuffer++;
    pDMACReg->DMACC0LLI = *pBuffer++;
    pDMACReg->DMACC0Control0 = *pBuffer++;
    pDMACReg->DMACC0Control1 = *pBuffer++;
    pDMACReg->DMACC0Configuration = *pBuffer++;

    pDMACReg->DMACC1SrcAddr = *pBuffer++;
    pDMACReg->DMACC1DestAddr = *pBuffer++;
    pDMACReg->DMACC1LLI = *pBuffer++;
    pDMACReg->DMACC1Control0 = *pBuffer++;
    pDMACReg->DMACC1Control1 = *pBuffer++;
    pDMACReg->DMACC1Configuration = *pBuffer++;

    pDMACReg->DMACC2SrcAddr = *pBuffer++;
    pDMACReg->DMACC2DestAddr = *pBuffer++;
    pDMACReg->DMACC2LLI = *pBuffer++;
    pDMACReg->DMACC2Control0 = *pBuffer++;
    pDMACReg->DMACC2Control1 = *pBuffer++;
    pDMACReg->DMACC2Configuration = *pBuffer++;

    pDMACReg->DMACC3SrcAddr = *pBuffer++;
    pDMACReg->DMACC3DestAddr = *pBuffer++;
    pDMACReg->DMACC3LLI = *pBuffer++;
    pDMACReg->DMACC3Control0 = *pBuffer++;
    pDMACReg->DMACC3Control1 = *pBuffer++;
    pDMACReg->DMACC3Configuration = *pBuffer++;

    pDMACReg->DMACC4SrcAddr = *pBuffer++;
    pDMACReg->DMACC4DestAddr = *pBuffer++;
    pDMACReg->DMACC4LLI = *pBuffer++;
    pDMACReg->DMACC4Control0 = *pBuffer++;
    pDMACReg->DMACC4Control1 = *pBuffer++;
    pDMACReg->DMACC4Configuration = *pBuffer++;

    pDMACReg->DMACC5SrcAddr = *pBuffer++;
    pDMACReg->DMACC5DestAddr = *pBuffer++;
    pDMACReg->DMACC5LLI = *pBuffer++;
    pDMACReg->DMACC5Control0 = *pBuffer++;
    pDMACReg->DMACC5Control1 = *pBuffer++;
    pDMACReg->DMACC5Configuration = *pBuffer++;

    pDMACReg->DMACC6SrcAddr = *pBuffer++;
    pDMACReg->DMACC6DestAddr = *pBuffer++;
    pDMACReg->DMACC6LLI = *pBuffer++;
    pDMACReg->DMACC6Control0 = *pBuffer++;
    pDMACReg->DMACC6Control1 = *pBuffer++;
    pDMACReg->DMACC6Configuration = *pBuffer++;

    pDMACReg->DMACC7SrcAddr = *pBuffer++;
    pDMACReg->DMACC7DestAddr = *pBuffer++;
    pDMACReg->DMACC7LLI = *pBuffer++;
    pDMACReg->DMACC7Control0 = *pBuffer++;
    pDMACReg->DMACC7Control1 = *pBuffer++;
    pDMACReg->DMACC7Configuration = *pBuffer++;
}

//--------------------------------------------------------------------
//48MHz clock source for usb host1.1, IrDA, hsmmc, spi is shared with otg phy clock.
//So, initialization and reset of otg phy shoud be done on initial booting time.
//--------------------------------------------------------------------
void InitializeOTGCLK(void)
{
    volatile S3C6410_SYSCON_REG *pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);
    volatile OTG_PHY_REG *pOtgPhyReg = (OTG_PHY_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_USBOTG_PHY, FALSE);

    pSysConReg->HCLK_GATE |= (1<<20);

    pSysConReg->OTHERS |= (1<<16);

    pOtgPhyReg->OPHYPWR = 0x0;  // OTG block, & Analog bock in PHY2.0 power up, normal operation
    pOtgPhyReg->OPHYCLK = 0x20; // Externel clock/oscillator, 48MHz reference clock for PLL
    pOtgPhyReg->ORSTCON = 0x1;
    OALStall_ms(1);
    pOtgPhyReg->ORSTCON = 0x0;
    OALStall_ms(1);

    pSysConReg->HCLK_GATE &= ~(1<<20);

}

//------------------------------------------------------------------------------
//
// Function:     OEMPowerOff
//
// Description:  Called when the system is to transition to it's lowest  power mode (off)
//
//
void OEMPowerOff()
{
    volatile S3C6410_SYSCON_REG *pSysConReg;
    volatile S3C6410_GPIO_REG *pGPIOReg;
    volatile S3C6410_VIC_REG *pVIC0Reg;
    volatile S3C6410_VIC_REG *pVIC1Reg;
    volatile S3C6410_DMAC_REG *pDMAC0Reg;
    volatile S3C6410_DMAC_REG *pDMAC1Reg;
    volatile OTG_PHY_REG *pOtgPhyReg;
    
    OAL_KITL_ARGS *pArgs;
    BOOL PowerStateOn;
    int nIndex = 0;

    OALMSG(TRUE, (L"[OEM] ++OEMPowerOff()"));

     // Make sure that KITL is powered off
    pArgs = (OAL_KITL_ARGS*)OALArgsQuery(OAL_ARGS_QUERY_KITL);
    if (pArgs && ((pArgs->flags & OAL_KITL_FLAGS_ENABLED) != 0))
    {
        PowerStateOn = FALSE;
        KITLIoctl (IOCTL_KITL_POWER_CALL, &PowerStateOn, sizeof(PowerStateOn), NULL, 0, NULL);

        OALMSG(OAL_VERBOSE, (L"OEMPowerOff: KITL Disabled\r\n"));
    }

    //-----------------------------
    // Prepare Specific Actions for Sleep
    //-----------------------------
    BSPPowerOff();

    //------------------------------
    // Prepare CPU Entering Sleep Mode
    //------------------------------

    //----------------
    // Map SFR Address
    //----------------
    pSysConReg = (S3C6410_SYSCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SYSCON, FALSE);
    pGPIOReg = (S3C6410_GPIO_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_GPIO, FALSE);
    pVIC0Reg = (S3C6410_VIC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_VIC0, FALSE);
    pVIC1Reg = (S3C6410_VIC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_VIC1, FALSE);
    pDMAC0Reg = (S3C6410_DMAC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_DMA0, FALSE);
    pDMAC1Reg = (S3C6410_DMAC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_DMA1, FALSE);
    pOtgPhyReg = (OTG_PHY_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_USBOTG_PHY, FALSE);


    //------------------
    // Save VIC Registers
    //------------------
    S3C6410_SaveState_VIC((void *)pVIC0Reg, (void *)pVIC1Reg, g_aSleepSave_VIC);

    // Disable All Interrupt
    pVIC0Reg->VICINTENCLEAR = 0xFFFFFFFF;
    pVIC1Reg->VICINTENCLEAR = 0xFFFFFFFF;
    pVIC0Reg->VICSOFTINTCLEAR = 0xFFFFFFFF;
    pVIC1Reg->VICSOFTINTCLEAR = 0xFFFFFFFF;

    //--------------------
    // Save DMAC Registers
    //--------------------
    S3C6410_SaveState_DMACon((void *)pDMAC0Reg, g_aSleepSave_DMACon0);
    S3C6410_SaveState_DMACon((void *)pDMAC1Reg, g_aSleepSave_DMACon1);

    //------------------
    // Save GPIO Register
    //------------------
    S3C6410_SaveState_GPIO((void *)pGPIOReg, g_aSleepSave_GPIO);

    //--------------------
    // Save SysCon Register
    //--------------------
    S3C6410_SaveState_SysCon((void *)pSysConReg, g_aSleepSave_SysCon);

    //---------------------------------------------------------------------------
    // Unmask Clock Gating for All IPsand Block Power turn On for the IPs not going to sleep
    //---------------------------------------------------------------------------
    // HCLK_IROM, HCLK_MEM1, HCLK_MEM0, HCLK_MFC Should be Always On for power Mode (Something coupled with BUS operation)
    //pSysConReg->HCLK_GATE |= ((1<<25)|(1<<22)|(1<<21)|(1<<0));
    pSysConReg->HCLK_GATE = 0xFFFFFFFF;
    pSysConReg->PCLK_GATE = 0xFFFFFFFF;
    pSysConReg->SCLK_GATE = 0xFFFFFFFF;
    // Turn On All Block Block Power
    pSysConReg->NORMAL_CFG = 0xFFFFFF00;

    // Wait for Block Power Stable
    while((pSysConReg->BLK_PWR_STAT & 0x7E) != 0x7E);

    //----------------------------
    // Wake Up Source Configuration
    //----------------------------
//    S3C6410_WakeUpSource_Configure();

    //-------------------------------
    // Extra work for Entering Sleep Mode
    //-------------------------------

    // USB Power Control
    pSysConReg->OTHERS &= ~(1<<16);    // USB Signal Mask Clear
    pGPIOReg->SPCON |= (1<<3);            // USB Tranceiver PAD to Suspend

    #ifdef _IROM_SDMMC_
    // Sleep Mode Pad Configuration. HSJANG 070926. SLPEN must be 0 to change cpcon value for reading OM.
    #else
    // Sleep Mode Pad Configuration
    pGPIOReg->SLPEN = 0x2;    // Controlled by SLPEN Bit (You Should Clear SLPEN Bit in Wake Up Process...)
    #endif

    //-----------------------
    // CPU Entering Sleep Mode
    //-----------------------

    OALCPUPowerOff();    // Now in Sleep

    //----------------------------
    // CPU Wake Up from Sleep Mode
    //----------------------------

    // Restore SysCon Register
    S3C6410_RestoreState_SysCon((void *)pSysConReg, g_aSleepSave_SysCon);

    // Restore GPIO Register
    S3C6410_RestoreState_GPIO((void *)pGPIOReg, g_aSleepSave_GPIO);

    #ifdef _IROM_SDMMC_
    // Sleep Mode Pad Configuration. HSJANG 070926. SLPEN must be 0 to change cpcon value for reading OM.
    #else
    // Sleep Mode Pad Configuration
    pGPIOReg->SLPEN = 0x2;    // Clear SLPEN Bit for Pad back to Normal Mode
    #endif

    //-----------------------
    // Restore DMAC Registers
    //-----------------------
    S3C6410_RestoreState_DMACon((void *)pDMAC0Reg, g_aSleepSave_DMACon0);
    S3C6410_RestoreState_DMACon((void *)pDMAC1Reg, g_aSleepSave_DMACon1);

    // Restore VIC Registers
    S3C6410_RestoreState_VIC((void *)pVIC0Reg, (void *)pVIC1Reg, g_aSleepSave_VIC);

    // UART Debug Port Initialize
    OEMInitDebugSerial();

    // Disable Vectored Interrupt Mode on CP15
    System_DisableVIC();

    // Enable Branch Prediction on CP15
    System_EnableBP();

    // Enable IRQ Interrupt on CP15
    System_EnableIRQ();

    // Enable FIQ Interrupt on CP15
    System_EnableFIQ();

    // Initialize System Timer
    OEMInitializeSystemTimer(RESCHED_PERIOD, OEM_COUNT_1MS, 0);

    // USB Power Control
    InitializeOTGCLK();              // pll_powerdown, suspend mode

    pGPIOReg->SPCON &= ~(1<<3);        // USB Tranceiver PAD to Normal

    //--------------------------------------
    // Post Processing Specific Actions for Wake Up
    //--------------------------------------
    BSPPowerOn();

    // Reinitialize KITL
    if (pArgs && ((pArgs->flags & OAL_KITL_FLAGS_ENABLED) != 0))
    {
        PowerStateOn = TRUE;
        KITLIoctl (IOCTL_KITL_POWER_CALL, &PowerStateOn, sizeof(PowerStateOn), NULL, 0, NULL);
    }

    OALMSG(TRUE, (L"[OEM] --OEMPowerOff()"));
}

//-----------------------------------------------------------------------------
//
// Function:     OALIoCtlHalEnableWake
//
// Called when a IOCTL_HAL_ENABLE_WAKE interrupt occurs.  This function will
// set the requested interrupt as a wake source.
//
// Parameters:
//      code
//           [in] IOCTL_HAL_ENABLE_WAKE.
//
//      pInpBuffer
//           [in] The interrupt that is being set as a wake source.
//
//      inpSize
//           [in] The size of a UINT32.
//
//      pOutBuffer
//           [out] Ignored.
//
//      outSize
//           [in] Ignored.
//
//      pOutSize
//           [out] Ignored.
//
// Returns:
//      TRUE if the IOCTL was successfully processed.  FALSE if an error
//      occurs while processing the IOCTL.
//
//-----------------------------------------------------------------------------
BOOL
OALIoCtlHalEnableWake(UINT32 code, VOID* pInpBuffer, UINT32 inpSize,
                      VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    UINT32 sysIntr;
    BOOL rc = FALSE;
    UINT32 count = 1;
    const UINT32 *irq;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_FUNC&&OAL_POWER, (_T("+OALIoCtlHalEnableWake\r\n")));

    if (pInpBuffer == NULL || inpSize < sizeof(UINT32)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_ENABLE_WAKE invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    sysIntr = *(UINT32*)pInpBuffer;

    // Retrieve list of irqs for the specified SYSINTR
    if (!OALIntrTranslateSysIntr(sysIntr, &count, &irq))
    {
        OALMSG(OAL_ERROR,
           (_T("ERROR: OALIntrTranslateSysIntr failed for IOCTL_HAL_ENABLE_WAKE,\r\n")));
        goto cleanUp;
    }

    // If the IRQ returned is not defined, we cannot set a wakeup mask.
    if (*irq == OAL_INTR_IRQ_UNDEFINED)
    {
        OALMSG(OAL_ERROR,
           (_T("ERROR: OALIntrTranslateSysIntr returned an UNDEFINED IRQ\r\n")));
        goto cleanUp;
    }
    
    // Set mask
    switch (*irq)
    {
    
    case IRQ_RTC_ALARM:
        g_oalWakeMask &= ~(1 << 10);
        break;
    case IRQ_HSMMC1:
    case IRQ_HSMMC0:
    case IRQ_HSITX:
    case IRQ_HSIRX:
    case IRQ_ADC:
    case IRQ_RTC_TIC:
    case IRQ_MSM:
    case IRQ_KEYPAD:
    default:
        OALMSG(OAL_FUNC&&OAL_POWER,
           (_T("Not Supported\r\n")));
        goto cleanUp;
        break;
    }

   rc = TRUE;

cleanUp:
    OALMSG(OAL_FUNC&&OAL_POWER, (_T("-OALIoCtlHalEnableWake (rc = %d)\r\n"), rc));

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalDisableWake
//
BOOL OALIoCtlHalDisableWake(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    UINT32 sysIntr;
    UINT32 count = 1;
    const UINT32 *irq;

    OALMSG(OAL_POWER&&OAL_FUNC, (
        L"+OALIoCtlHalDisableWake(sysIntr = %d)\r\n", *(UINT32*)pInpBuffer
    ));
    
    if (pInpBuffer == NULL || inpSize < sizeof(UINT32)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        goto cleanUp;
    }

    sysIntr = *(UINT32*)pInpBuffer;
    if (sysIntr < SYSINTR_DEVICES || sysIntr >= SYSINTR_MAXIMUM) {
        OALMSG(OAL_ERROR, (L"ERROR: OALIoCtlHalDisableWake: ",
            L"Invalid SYSINTR value %d\r\n", sysIntr
        ));
        NKSetLastError(ERROR_INVALID_PARAMETER);
        goto cleanUp;
    }                    

    // Retrieve list of irqs for the specified SYSINTR
    if (!OALIntrTranslateSysIntr(sysIntr, &count, &irq))
    {
        OALMSG(OAL_ERROR,
           (_T("ERROR: OALIntrTranslateSysIntr failed for IOCTL_HAL_ENABLE_WAKE,\r\n")));
        goto cleanUp;
    }

    // If the IRQ returned is not defined, we cannot set a wakeup mask.
    if (*irq == OAL_INTR_IRQ_UNDEFINED)
    {
        OALMSG(OAL_ERROR,
           (_T("ERROR: OALIntrTranslateSysIntr returned an UNDEFINED IRQ\r\n")));
        goto cleanUp;
    }

    // Set mask
    switch (*irq)
    {
    
    case IRQ_RTC_ALARM:
        g_oalWakeMask |= (1 << 10);
        break;
    case IRQ_HSMMC1:
    case IRQ_HSMMC0:
    case IRQ_HSITX:
    case IRQ_HSIRX:
    case IRQ_ADC:
    case IRQ_RTC_TIC:
    case IRQ_MSM:
    case IRQ_KEYPAD:
    default:
       OALMSG(OAL_FUNC&&OAL_POWER,
           (_T("Not supported\r\n")));
        goto cleanUp;
        break;
    }

    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_POWER&&OAL_FUNC, (L"+OALIoCtlHalDisableWake(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlGetWakeSource
//
BOOL OALIoCtlHalGetWakeSource(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{
    BOOL rc = FALSE;
    
    OALMSG(OAL_POWER&&OAL_FUNC, (L"+OALIoCtlHalGetWakeSource\r\n"));

    if (pOutSize) {
        *pOutSize = sizeof(UINT32);
    }

    if (pOutBuffer == NULL && outSize > 0) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        goto cleanUp;
    }

    if (pOutBuffer == NULL || outSize < sizeof(UINT32)) {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto cleanUp;
    }

    *(UINT32*)pOutBuffer = g_oalWakeSource;

    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_POWER&&OAL_FUNC, (
        L"+OALIoCtlHalGetWakeSource(rc = %d, sysIntr = %d)\r\n", 
        rc, *(UINT32*)pOutBuffer
    ));
    return rc;
}

//------------------------------------------------------------------------------
