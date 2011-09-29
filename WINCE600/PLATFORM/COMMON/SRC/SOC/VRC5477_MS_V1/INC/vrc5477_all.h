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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  vrc5477_all.h
//
//  This header file defines VRC5477 register layout and associated constants
//  and types. In this case there is only one common registry layour for
//  all devices.
//
//  Note:  Base addresses are defined in vrc5477_base_regs.h header file.
//
#ifndef __VRC5477_ALL_H
#define __VRC5477_ALL_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

#include <packon.h>

typedef struct {

    UINT32 SDRAM01;                     // 0000
    UINT32 PAD0004;                     // 0004
    UINT32 SDRAM23;                     // 0008
    UINT32 PAD000C;                     // 000C
    UINT32 LDCS0;                       // 0010
    UINT32 PAD0014;                     // 0014
    UINT32 LDCS1;                       // 0018
    UINT32 PAD001C;                     // 001C
    UINT32 LDCS2;                       // 0020
    UINT32 PAD0024[15];                 // 0024
    UINT32 PCIW0;                       // 0060
    UINT32 PAD0064;                     // 0064
    UINT32 PCIW1;                       // 0068
    UINT32 PAD006C;                     // 006C
    UINT32 INTCS;                       // 0070
    UINT32 PAD0074;                     // 0074
    UINT32 BOOTCS;                      // 0078
    UINT32 PAD007C;                     // 007C

    UINT32 PAD0080[20];                 // 0080

    UINT32 IOPCIW0;                     // 00D0
    UINT32 PAD00D4;                     // 00D4
    UINT32 IOPCIW1;                     // 00D8
    UINT32 PAD00DC;                     // 00DC

    UINT32 PAD00E0[8];                  // 00E0

    UINT32 LCNFG;                       // 0100
    UINT32 PAD0104[3];                  // 0104
    UINT32 LCST0;                       // 0110
    UINT32 PAD0114;                     // 0114
    UINT32 LCST1;                       // 0118
    UINT32 PAD011C;                     // 011C
    UINT32 LCST2;                       // 0120
    UINT32 PAD0124[11];                 // 0124
    UINT32 ERRADR;                      // 0150
    UINT32 PAD0154[3];                  // 0154
    UINT32 ERRCS;                       // 0160
    UINT32 PAD0164[3];                  // 0164
    UINT32 BTM;                         // 0170
    UINT32 PAD0174;                     // 0174
    UINT32 BCST;                        // 0178
    UINT32 PAD017C;                     // 017C

    UINT32 PAD0180[16];                 // 0180

    UINT32 REFCTRLL;                    // 01C0
    UINT32 REFCTRLH;                    // 01C4
    UINT32 REFCNTR;                     // 01C8
    UINT32 PAD01CC;                     // 01CC

    UINT32 SPT0CTRLL;                   // 01D0
    UINT32 SPT0CTRLH;                   // 01D4
    UINT32 SPT0CNTR;                    // 01D8
    UINT32 PAD01DC;                     // 01DC

    UINT32 SPT1CTRLL;                   // 01E0
    UINT32 SPT1CTRLH;                   // 01E4
    UINT32 SPT1CNTR;                    // 01E8
    UINT32 PAD01EC;                     // 01EC

    UINT32 WDTCTRLL;                    // 01F0
    UINT32 WDTCTRLH;                    // 01F4
    UINT32 WDTCNTR;                     // 01F8
    UINT32 PAD01FC;                     // 01FC

    UINT16 VID0;                        // 0200
    UINT16 DID0;                        // 0202
    UINT16 PCICMD0;                     // 0204
    UINT16 PCISTS0;                     // 0206
    UINT8  REVID0;                      // 0208
    UINT8  PROGIF0;                     // 0209
    UINT8  SUBCLASS0;                   // 020A
    UINT8  BASECLASS0;                  // 020B
    UINT8  CLSIZ0;                      // 020C
    UINT8  MLTIM0;                      // 020D
    UINT8  HTYPE0;                      // 020E
    UINT8  BIST0;                       // 020F

    UINT32 BARC0;                       // 0210
    UINT32 PAD0214;                     // 0214
    UINT32 BARM010;                     // 0218
    UINT32 PAD021C;                     // 021C
    UINT32 BARM230;                     // 0220
    UINT32 PAD0224[2];                  // 0224
    UINT16 SSVID0;                      // 022C
    UINT16 SSID0;                       // 022E

    UINT32 PAD0230[3];                  // 0230
    UINT8  INTLIN0;                     // 023C
    UINT8  INTPIN0;                     // 023D
    UINT16 PAD023E;                     // 023E

    UINT32 BAR00;                       // 0240
    UINT32 PAD0244;                     // 0244
    UINT32 BAR10;                       // 0248
    UINT32 PAD024C;                     // 024C
    UINT32 BAR20;                       // 0250
    UINT32 PAD0254[11];                 // 0254

    UINT32 BARB0;                       // 0280
    UINT32 PAD0284[3];                  // 0284
    UINT32 BARP00;                      // 0290
    UINT32 PAD0294;                     // 0294
    UINT32 BARP10;                      // 0298
    UINT32 PAD029C[5];                  // 029C

    UINT32 PCISWP0;                     // 02B0
    UINT32 PAD02B4;                     // 02B4
    UINT32 PCIERR0;                     // 02B8
    UINT32 PAD02BC[9];                  // 02BC

    UINT32 PCICTL0L;                    // 02E0
    UINT32 PCICTL0H;                    // 02E4
    UINT32 PCIARB0L;                    // 02E8
    UINT32 PCIARB0H;                    // 02EC

    UINT32 PCIINIT00;                   // 02F0
    UINT32 PAD02F4;                     // 02F4
    UINT32 PCIINIT10;                   // 02F8
    UINT32 PAD02FC;                     // 02FC

    UINT32 DMACTRL0L;                   // 0300
    UINT32 DMACTRL0H;                   // 0304
    UINT32 DMASRCA0;                    // 0308
    UINT32 PAD030C;                     // 030C
    UINT32 DMADESA0;                    // 0310
    UINT32 PAD0314;                     // 0314
    UINT32 DMANXAP0;                    // 0318
    UINT32 PAD031C;                     // 031C

    UINT32 DMACTRL1L;                   // 0320
    UINT32 DMACTRL1H;                   // 0324
    UINT32 DMASRCA1;                    // 0328
    UINT32 PAD032C;                     // 032C
    UINT32 DMADESA1;                    // 0330
    UINT32 PAD0334;                     // 0334
    UINT32 DMANXAP1;                    // 0338
    UINT32 PAD033C;                     // 033C

    UINT32 DMACTRL2L;                   // 0340
    UINT32 DMACTRL2H;                   // 0344
    UINT32 DMASRCA2;                    // 0348
    UINT32 PAD034C;                     // 034C
    UINT32 DMADESA2;                    // 0350
    UINT32 PAD0354;                     // 0354
    UINT32 DMANXAP2;                    // 0358
    UINT32 PAD035C;                     // 035C

    UINT32 DMACTRL3L;                   // 0360
    UINT32 DMACTRL3H;                   // 0364
    UINT32 DMASRCA3;                    // 0368
    UINT32 PAD036C;                     // 036C
    UINT32 DMADESA3;                    // 0370
    UINT32 PAD0374;                     // 0374
    UINT32 DMANXAP3;                    // 0378
    UINT32 PAD037C;                     // 037C
    
    UINT32 DMACTRL4L;                   // 0380
    UINT32 DMACTRL4H;                   // 0384
    UINT32 DMASRCA4;                    // 0388
    UINT32 PAD038C;                     // 038C
    UINT32 DMADESA4;                    // 0390
    UINT32 PAD0394;                     // 0394
    UINT32 DMANXAP4;                    // 0398
    UINT32 PAD039C;                     // 039C

    UINT32 DMACTRL6L;                   // 03A0
    UINT32 DMACTRL6H;                   // 03A4
    UINT32 DMASRCA6;                    // 03A8
    UINT32 PAD03AC;                     // 03AC
    UINT32 DMADESA6;                    // 03B0
    UINT32 PAD03B4;                     // 03B4
    UINT32 DMANXAP6;                    // 03B8
    UINT32 PAD03BC;                     // 03BC

    UINT32 PAD03C0[16];                 // 03C0

    UINT32 INTCTRL0;                    // 0400
    UINT32 INTCTRL1;                    // 0404
    UINT32 INTCTRL2;                    // 0408
    UINT32 INTCTRL3;                    // 040C
    UINT32 PAD0410[4];                  // 0410

    UINT32 INT0STAT;                    // 0420
    UINT32 PAD0424;                     // 0424
    UINT32 INT1STAT;                    // 0428
    UINT32 PAD042C;                     // 042C
    UINT32 INT2STAT;                    // 0430
    UINT32 PAD0434;                     // 0434
    UINT32 INT3STAT;                    // 0438
    UINT32 PAD043C;                     // 043C
    UINT32 INT4STAT;                    // 0440
    UINT32 PAD0444[3];                  // 0444
    UINT32 NMISTAT;                     // 0450
    UINT32 PAD0454[5];                  // 0454
    UINT32 INTCLR32;                    // 0468
    UINT32 PAD046C;                     // 046C

    UINT32 INTPPES0;                    // 0470
    UINT32 PAD0474;                     // 0474
    UINT32 INTPPES1;                    // 0478
    UINT32 PAD047C;                     // 047C

    UINT32 CPUSTAT;                     // 0480
    UINT32 PAD0484;                     // 0484
    UINT32 BUSCTRL;                     // 0488
    UINT32 PAD048C;                     // 048C

    UINT32 PAD0490[92];                 // 0490    

    UINT16 VID1;                        // 0600
    UINT16 DID1;                        // 0602
    UINT16 PCICMD1;                     // 0604
    UINT16 PCISTS1;                     // 0606
    UINT8  REVID1;                      // 0608
    UINT8  PROGIF1;                     // 0609
    UINT8  SUBCLASS1;                   // 060A
    UINT8  BASECLASS1;                  // 060B
    UINT8  CLSIZ1;                      // 060C
    UINT8  MLTIM1;                      // 060D
    UINT8  HTYPE1;                      // 060E
    UINT8  BIST1;                       // 060F

    UINT32 BARC1;                       // 0610
    UINT32 PAD0614;                     // 0614
    UINT32 BARM011;                     // 0618
    UINT32 PAD061C;                     // 061C
    UINT32 BARM231;                     // 0620
    UINT32 PAD0624[2];                  // 0624
    UINT16 SSVID1;                      // 062C
    UINT16 SSID1;                       // 062E

    UINT32 PAD0630[3];                  // 0630
    UINT8  INTLIN1;                     // 063C
    UINT8  INTPIN1;                     // 063D
    UINT16 PAD063E;                     // 063E

    UINT32 BAR01;                       // 0640
    UINT32 PAD0644;                     // 0644
    UINT32 BAR11;                       // 0648
    UINT32 PAD064C;                     // 064C
    UINT32 BAR21;                       // 0650
    UINT32 PAD0654[11];                 // 0654

    UINT32 BARB1;                       // 0680
    UINT32 PAD0684[3];                  // 0684
    UINT32 BARP01;                      // 0690
    UINT32 PAD0694;                     // 0694
    UINT32 BARP11;                      // 0698
    UINT32 PAD069C[5];                  // 069C

    UINT32 PCISWP1;                     // 06B0
    UINT32 PAD06B4;                     // 06B4
    UINT32 PCIERR1;                     // 06B8
    UINT32 PAD06BC[9];                  // 06BC

    UINT32 PCICTL1L;                    // 06E0
    UINT32 PCICTL1H;                    // 06E4
    UINT32 PCIARB1L;                    // 06E8
    UINT32 PCIARB1H;                    // 06EC

    UINT32 PCIINIT01;                   // 06F0
    UINT32 PAD06F4;                     // 06F4
    UINT32 PCIINIT11;                   // 06F8
    UINT32 PAD06FC;                     // 06FC

    UINT32 MEMCTRL;                     // 0700
    UINT32 PAD0704;                     // 0704
    UINT32 MEMTCTRL;                    // 0708
    UINT32 PAD070C[13];                 // 070C

    UINT32 PIBRST;                      // 0740
    UINT32 PAD0744;                     // 0744
    UINT32 PIBRDY;                      // 0748
    UINT32 PAD074C;                     // 074C
    UINT32 PIBMISC;                     // 0750
    UINT32 PAD0754[43];                 // 0754

    UINT32 PAD0800[3584];               // 0800

    UINT32 GIUDIR;                      // 4000
    UINT32 PAD4004;                     // 4004
    UINT32 GIUPIOD;                     // 4008
    UINT32 PAD400C;                     // 400C
    UINT32 GIUINSTAT;                   // 4010
    UINT32 PAD4014;                     // 4014
    UINT32 GIUINTEN;                    // 4018
    UINT32 PAD401C;                     // 401C
    UINT32 GIUINTTYP;                   // 4020
    UINT32 PAD4024;                     // 4024
    UINT32 GIUINTALSEL;                 // 4028
    UINT32 PAD402C;                     // 402C
    UINT32 GIUINTHTSEL;                 // 4030
    UINT32 PAD4034[3];                  // 4034
    UINT32 GIUFUNSEL;                   // 4040
    UINT32 PAD4044[47];                 // 4044

    UINT32 GPT0VAL;                     // 4100
    UINT32 GPT0CTRL;                    // 4104
    UINT32 GPT0CNTR;                    // 4108
    UINT32 PAD410C;                     // 410C

    UINT32 GPT1VAL;                     // 4110
    UINT32 GPT1CTRL;                    // 4114
    UINT32 GPT1CNTR;                    // 4118
    UINT32 PAD411C;                     // 411C

    UINT32 GPT2VAL;                     // 4120
    UINT32 GPT2CTRL;                    // 4124
    UINT32 GPT2CNTR;                    // 4128
    UINT32 PAD412C;                     // 412C

    UINT32 GPT3VAL;                     // 4130
    UINT32 GPT3CTRL;                    // 4134
    UINT32 GPT3CNTR;                    // 4138
    UINT32 PAD413C;                     // 413C

    UINT32 PAD4140[48];                 // 4140

    union {                             // 4200
        UINT8 UARTRBR0;
        UINT8 UARTTHR0;
        UINT8 UARTDLL0;
    };
    UINT8 PAD4201[7];                   // 4201
    union {                             // 4208
        UINT8 UARTIER0;
        UINT8 UARTDLM0;
    };
    UINT8 PAD4209[7];                   // 4209
    union {                             // 4210
        UINT8 UARTIIR0;
        UINT8 UARTFCR0;
    };
    UINT8 PAD4211[7];                   // 4211
    UINT8 UARTLCR0;                     // 4218
    UINT8 PAD4219[7];                   // 4219
    UINT8 UARTMCR0;                     // 4220
    UINT8 PAD4221[7];                   // 4221
    UINT8 UARTLSR0;                     // 4228
    UINT8 PAD4229[7];                   // 4229
    UINT8 UARTMSR0;                     // 4230
    UINT8 PAD4231[7];                   // 4231
    UINT8 UARTSCR0;                     // 4238
    UINT8 PAD4239[7];                   // 4239

    union {                             // 4240
        UINT8 UARTRBR1;
        UINT8 UARTTHR1;
        UINT8 UARTDLL1;
    };
    UINT8 PAD4241[7];                   // 4241
    union {                             // 4248
        UINT8 UARTIER1;
        UINT8 UARTDLM1;
    };
    UINT8 PAD4249[7];                   // 4249
    union {                             // 4250
        UINT8 UARTIIR1;
        UINT8 UARTFCR1;
    };
    UINT8 PAD4251[7];                   // 4251
    UINT8 UARTLCR1;                     // 4258
    UINT8 PAD4259[7];                   // 4259
    UINT8 UARTMCR1;                     // 4260
    UINT8 PAD4261[7];                   // 4261
    UINT8 UARTLSR1;                     // 4268
    UINT8 PAD4269[7];                   // 4269
    UINT8 UARTMSR1;                     // 4270
    UINT8 PAD4271[7];                   // 4271
    UINT8 UARTSCR1;                     // 4278
    UINT8 PAD4279[7];                   // 4279


    UINT32 PAD4280[32];                 // 4280

    UINT32 SIO0DATA0;                   // 4300
    UINT32 PAD4304;                     // 4304
    UINT32 SIO0DATA1;                   // 4308
    UINT32 PAD430C;                     // 430C
    UINT32 SIO0DATA2;                   // 4310
    UINT32 PAD4314;                     // 4314
    UINT32 SIO0DATA3;                   // 4318
    UINT32 PAD431C;                     // 431C
    UINT32 SIO0DATA4;                   // 4320
    UINT32 PAD4324;                     // 4324
    UINT32 SIO0TRSCNT;                  // 4328
    UINT32 PAD432C;                     // 432C
    UINT32 SIO0INTCLR;                  // 4330
    UINT32 PAD4334[3];                  // 4334

    UINT32 SIO1TXDATA0;                 // 4340
    UINT32 PAD4344;                     // 4344
    UINT32 SIO1TXDATA1;                 // 4348
    UINT32 PAD434C;                     // 434C
    UINT32 SIO1RXDATA0;                 // 4350
    UINT32 PAD4354;                     // 4354
    UINT32 SIO1RXDATA1;                 // 4358
    UINT32 PAD435C;                     // 435C
    UINT32 SIO1CTRL;                    // 4360
    UINT32 PAD4364;                     // 4364
    UINT32 SIO1INTCLR;                  // 4368
    UINT32 PAD436C;                     // 436C

    UINT32 PAD4370[36];                 // 4370

    UINT8  IICC0;                       // 4400
    UINT8  PAD4401[7];                  // 4401
    UINT8  IICS0;                       // 4408
    UINT8  PAD4409[7];                  // 4409
    UINT8  IICCL0;                      // 4410
    UINT8  PAD4411[7];                  // 4411
    UINT8  IICSVA0;                     // 4418
    UINT8  PAD4419[7];                  // 4419
    UINT8  IIC0;                        // 4420
    UINT8  PAD4421[31];                 // 4421
    UINT8  PIOC0;                       // 4440
    UINT8  PAD4441[191];                // 4441

} VRC5477_REGS;

#include <packoff.h>

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------
// Offset addresses
//------------------------------------------------------------------------------

#define VRC5477_REG_OA_SDRAM01      0x0000
#define VRC5477_REG_OA_SDRAM23      0x0008
#define VRC5477_REG_OA_MEMCTRL      0x00C0
#define VRC5477_REG_OA_MEMTCTRL     0x00C8

#define VRC5477_REG_OA_REFCTRL      0x01C0
#define VRC5477_REG_OA_REFCTRH      0x01C4

//------------------------------------------------------------------------------
// UART
//------------------------------------------------------------------------------

#define UART_LSR_RFERR              (1 << 7)
#define UART_LSR_THRE               (1 << 5)
#define UART_LSR_DR                 (1 << 0)

#define UART_FCR_EN                 (1 << 0)    // Receive/Transmit FIFO enable
#define UART_FCR_RRST               (1 << 1)    // Receive FIFO clear
#define UART_FCR_TRST               (1 << 2)    // Transmit FIFO clear

#define UART_LCR_DLAB               (1 << 7)    // Divisor latch access
#define UART_LCR_EP                 (1 << 4)    // Even parity
#define UART_LCR_PE                 (1 << 3)    // Parity enable
#define UART_LCR_2STP               (1 << 2)    // 2-stop bits
#define UART_LCR_8BIT               (3 << 0)    // 8-bit

#define UART_MCR_RTS                (1 << 1)    // Modem Control RTS#
#define UART_MCR_DTR                (1 << 0)    // Modem Control DTR#

#define UART_LSR_RFERR              (1 << 7)    // Error Detection
#define UART_LSR_THRE               (1 << 5)    // Trans Hold Registry Empty
#define UART_LSR_RDR                (1 << 0)    // Receive Data Ready

//------------------------------------------------------------------------------
// PCI Bus
//------------------------------------------------------------------------------

#define PCI_CTRL_CRST               (1 << 31)

#define PCI_INIT_TYPE_IO            (1 << 1)
#define PCI_INIT_TYPE_MEM           (3 << 1)
#define PCI_INIT_TYPE_CFG           (5 << 1)
#define PCI_INIT_TYPE_MASK          (7 << 1)
#define PCI_INIT_ADDR_MASK          0xFFE00000

#define PCI_INIT_TYPE0              (0 << 9)
#define PCI_INIT_TYPE1              (1 << 9)

//------------------------------------------------------------------------------

#endif
