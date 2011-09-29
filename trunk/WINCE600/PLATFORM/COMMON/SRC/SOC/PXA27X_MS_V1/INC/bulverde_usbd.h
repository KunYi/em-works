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
//  Header: bulverde_usbd.h
//
//  Defines the USB device controller CPU register layout and definitions.
//
#ifndef __BULVERDE_USBD_H
#define __BULVERDE_USBD_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: BULVERDE_USBD_REG    
//
//  Defines the USB device control register block.
//

typedef struct
{
    UINT32    udc_cr;
    UINT32    udc_icr0;
    UINT32    udc_icr1;
    UINT32    udc_isr0;
    UINT32    udc_isr1;
    UINT32    udc_fnr;
    UINT32    udc_otgicr;
    UINT32    udc_otgisr;
    UINT32    up2ocr;
    UINT32    up3ocr;
    UINT32    rsvd0[54];  //0x4060_0028 - 0x4060_00FF

    UINT32    udc_csr0;
    UINT32    udc_csrA;
    UINT32    udc_csrB;
    UINT32    udc_csrC;
    UINT32    udc_csrD;
    UINT32    udc_csrE;
    UINT32    udc_csrF;
    UINT32    udc_csrG;
    UINT32    udc_csrH;
    UINT32    udc_csrI;
    UINT32    udc_csrJ;
    UINT32    udc_csrK;
    UINT32    udc_csrL;     
    UINT32    udc_csrM;     
    UINT32    udc_csrN; 
    
    UINT32    udc_csrP;
    UINT32    udc_csrQ;
    UINT32    udc_csrR;
    UINT32    udc_csrS;
    UINT32    udc_csrT;
    UINT32    udc_csrU;
    UINT32    udc_csrV;
    UINT32    udc_csrW;
    UINT32    udc_csrX;
    UINT32    rsvd1[40];  //0x4060_0160 - 0x4060_01FF
     
    UINT32    udc_bcr0;
    UINT32    udc_bcrA;
    UINT32    udc_bcrB;
    UINT32    udc_bcrC;
    UINT32    udc_bcrD;
    UINT32    udc_bcrE;
    UINT32    udc_bcrF;
    UINT32    udc_bcrG;
    UINT32    udc_bcrH;
    UINT32    udc_bcrI;
    UINT32    udc_bcrJ;
    UINT32    udc_bcrK;
    UINT32    udc_bcrL;     
    UINT32    udc_bcrM;     
    UINT32    udc_bcrN; 
    
    UINT32    udc_bcrP;
    UINT32    udc_bcrQ;
    UINT32    udc_bcrR;
    UINT32    udc_bcrS;
    UINT32    udc_bcrT;
    UINT32    udc_bcrU;
    UINT32    udc_bcrV;
    UINT32    udc_bcrW;
    UINT32    udc_bcrX;
    UINT32    rsvd2[40];  //0x4060_0260 - 0x4060_02FF

    UINT32    udc_dr0;
    UINT32    udc_drA;
    UINT32    udc_drB;
    UINT32    udc_drC;
    UINT32    udc_drD;
    UINT32    udc_drE;
    UINT32    udc_drF;
    UINT32    udc_drG;
    UINT32    udc_drH;
    UINT32    udc_drI;
    UINT32    udc_drJ;
    UINT32    udc_drK;
    UINT32    udc_drL;     
    UINT32    udc_drM;     
    UINT32    udc_drN; 
    
    UINT32    udc_drP;
    UINT32    udc_drQ;
    UINT32    udc_drR;
    UINT32    udc_drS;
    UINT32    udc_drT;
    UINT32    udc_drU;
    UINT32    udc_drV;
    UINT32    udc_drW;
    UINT32    udc_drX;
    UINT32    rsvd3[40];  //0x4060_0360 - 0x4060_03FF
    
    UINT32    udc_cr0;
    UINT32    udc_crA;
    UINT32    udc_crB;
    UINT32    udc_crC;
    UINT32    udc_crD;
    UINT32    udc_crE;
    UINT32    udc_crF;
    UINT32    udc_crG;
    UINT32    udc_crH;
    UINT32    udc_crI;
    UINT32    udc_crJ;
    UINT32    udc_crK;
    UINT32    udc_crL;     
    UINT32    udc_crM;     
    UINT32    udc_crN; 
    
    UINT32    udc_crP;
    UINT32    udc_crQ;
    UINT32    udc_crR;
    UINT32    udc_crS;
    UINT32    udc_crT;
    UINT32    udc_crU;
    UINT32    udc_crV;
    UINT32    udc_crW;
    UINT32    udc_crX;
    UINT32    rsvd4[40];  //0x4060_0460 - 0x4060_04FF

    //Note:  0x4060_0500 - 0x406F_FFFF is reserved.

} BULVERDE_USBD_REG, *PBULVERDE_USBD_REG;

typedef union _UDCCR {
    struct {
        DWORD   UDE:1;
        DWORD   UDA:1;
        DWORD   UDR:1;
        DWORD   EMCE:1;
        DWORD   SMAC:1;
        DWORD   AAISN:3;
        DWORD   AIN:3;
        DWORD   ACN:2;
        DWORD   Reserved1:3;
        DWORD   DWRE:1;
        DWORD   :11;
        DWORD   BHNP:1;
        DWORD   AHNP:1;
        DWORD   AALTHNP:1;
        DWORD   OEN:1;
    } bit;
    DWORD ulValue;
} UDCCR, *PUDCCR;

// For Interrupt Status Register and Interrut Control Register.
#define UDCISR1_IRCC    0x80000000
#define UDCISR1_IRSOF   0x40000000
#define UDCISR1_IRRU    0x20000000
#define UDCISR1_IRSU    0x10000000
#define UDCISR1_IRRS    0x08000000

// Endpoint Interrupt After shift.
#define EPINT_PACKET_COMPLETE   0x1
#define EPINT_FIFO_ERROR        0x2


typedef union _UDCFNR {
    struct {
        DWORD FN:11;
        DWORD Reserved:21;
    } bit;
    DWORD ulValue;
} UDCFNR, *PUDCFNR;

typedef union _UDCCSR {
    struct {
        DWORD OPC:1;
        DWORD IPR:1;
        DWORD FTF:1;
        DWORD DME:1;
        DWORD SST:1;
        DWORD FST:1;
        DWORD RNE:1;
        DWORD SA:1;
        DWORD AREN:1;
        DWORD ACM:1;
        DWORD Reserved:24;
    } ep0bit;
    struct {
        DWORD FS:1;
        DWORD PC:1;
        DWORD TRN:1;
        DWORD DME:1;
        DWORD SST:1;
        DWORD FST:1;
        DWORD BNE_BNF:1;
        DWORD SP:1;
        DWORD FEF:1;
        DWORD DPE:1;
        DWORD Reserved:22;
    } epbit;
    DWORD ulValue;
} UDCCSR, *PUDCCSR;

typedef union _UDCBCR {
    struct {
        DWORD BC:10;
        DWORD Reserved:22;
    } bit;
    DWORD   ulReserved;
} UDCBCR, *PUDCBCR;

typedef union _UDCCRAX {
    struct {
        DWORD EE:1;
        DWORD DE:1;
        DWORD MPS:10;
        DWORD ED:1;
        DWORD ET:2;
        DWORD EN:4;
        DWORD AISN:3;
        DWORD ISN:3;
        DWORD CN:2;
        DWORD Reserved:5;
    } bit;
    DWORD ulValue;
} UDCCRAX, *PUDCCRAX;

typedef union _UDCISR1 {
    struct {
        DWORD IRQ:2;
        DWORD IRR:2;
        DWORD IRS:2;
        DWORD IRT:2;
        DWORD IRU:2;
        DWORD IRV:2;
        DWORD IRW:2;
        DWORD IRX:2;
        DWORD Reserved:11;
        DWORD IRRS:1;
        DWORD IRSU:1;
        DWORD IRRU:1;
        DWORD IRSOF:1;
        DWORD IRCC:1;
    } bit;
    DWORD ulValue;
} UDCISR1, *PUDCISR1;

typedef union _UDCICR1 {
    struct {
        DWORD IEQ:2;
        DWORD IER:2;
        DWORD IES:2;
        DWORD IET:2;
        DWORD IEU:2;
        DWORD IEV:2;
        DWORD IEW:2;
        DWORD IEX:2;
        DWORD Reserved:11;
        DWORD IERS:1;
        DWORD IESU:1;
        DWORD IERU:1;
        DWORD IESOF:1;
        DWORD IECC:1;
    } bit;
    DWORD ulValue;
} UDCICR1, *PUDCICR1;
//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
