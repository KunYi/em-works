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
//------------------------------------------------------------------------------
//
//  Header:  s3c6410_cam.h
//
//  Defines the Camera Interface control registers and definitions.
//
#ifndef __S3C6410_CAM_H
#define __S3C6410_CAM_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

typedef struct {
    UINT32         CISRCFMT;        // 00
    UINT32         CIWDOFST;        // 04
    UINT32         CIGCTRL;        // 08
    UINT32         CIFCTRL1;            // 0c
    UINT32         CIFCTRL2;            // 10
    UINT32         CIDOWSFT2;            // 14    
    UINT32         CICOYSA1;        // 18
    UINT32         CICOYSA2;        // 1c
    UINT32         CICOYSA3;        // 20
    UINT32         CICOYSA4;        // 24
    UINT32         CICOCBSA1;        // 28
    UINT32         CICOCBSA2;        // 2c
    UINT32         CICOCBSA3;        // 30
    UINT32         CICOCBSA4;        // 34
    UINT32         CICOCRSA1;        // 38
    UINT32         CICOCRSA2;        // 3c
    UINT32         CICOCRSA3;        // 40
    UINT32         CICOCRSA4;        // 44
    UINT32         CICOTRGFMT;        // 48
    UINT32         CICOCTRL;        // 4c
    UINT32         CICOSCPRERATIO;    // 50
    UINT32         CICOSCPREDST;    // 54
    UINT32         CICOSCCTRL;        // 58
    UINT32         CICOTAREA;        // 5c
    UINT32         PAD4;            // 60
    UINT32         CICOSTATUS;        // 64
    UINT32         PAD5;            // 68
    UINT32         CIPRYSA1;        // 6c
    UINT32         CIPRYSA2;        // 70
    UINT32         CIPRYSA3;        // 74
    UINT32         CIPRYSA4;        // 78
    UINT32         CIPRCBSA1;        // 7C
    UINT32         CIPRCBSA2;        // 80
    UINT32         CIPRCBSA3;        // 84
    UINT32         CIPRCBSA4;        // 88
    UINT32         CIPRCRSA1;        // 8C
    UINT32         CIPRCRSA2;        // 90
    UINT32         CIPRCRSA3;        // 94
    UINT32         CIPRCRSA4;        // 98        
    UINT32         CIPRTRGFMT;        // 9c
    UINT32         CIPRCTRL;        // A0
    UINT32         CIPRSCPRERATIO;    // A4
    UINT32         CIPRSCPREDST;    // A8
    UINT32         CIPRSCCTRL;        // AC
    UINT32         CIPRTAREA;        // B0
    UINT32         PAD6;            // B4
    UINT32         CIPRSTATUS;        // B8
    UINT32         PAD7;            // BC
    UINT32         CIIMGCPT;        // C0
    UINT32         CICPTSEQ;        // C4
    UINT32         PAD8;            // C8
    UINT32         PAD9;            // CC
    UINT32         CIIMGEFF;        // D0
    UINT32         MSCOY0SA;        // D4
    UINT32         MSCOCB0SA;        // D8
    UINT32         MSCOCR0SA;        // DC
    UINT32         MSCOY0END;        // E0
    UINT32         MSCOCB0END;        // E4
    UINT32         MSCOCR0END;        // E8
    UINT32         MSCOYOFF;        // EC
    UINT32         MSCOCBOFF;        // F0
    UINT32         MSCOCROFF;        // F4
    UINT32         MSCOWIDTH;        // F8
    UINT32         MSCOCTRL;        // FC
    UINT32         MSPRY0SA;        // 100
    UINT32         MSPRCB0SA;        // 104
    UINT32         MSPRCR0SA;        // 108
    UINT32         MSPRY0END;        // 10C
    UINT32         MSPRCB0END;        // 110
    UINT32         MSPRCR0END;        // 114
    UINT32         MSPRYOFF;        // 118
    UINT32         MSPRCBOFF;        // 11C
    UINT32         MSPRCROFF;        // 120
    UINT32         MSPRWIDTH;        // 124
    UINT32         CIMSCTRL;        // 128
    UINT32        CICOSCOSY;        // 12C
    UINT32      CICOSCOSCB;        // 130
    UINT32        CICOSCOSCR;        // 134
    UINT32        CIPRSCOSY;        // 138
    UINT32        CIPRSCOSCB;        // 13C
    UINT32        CIPRSCOSCR;        // 140
    
}S3C6410_CAMIF_REG,*PS3C6410_CAMIF_REG;
//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif // __S3C6410_CAM_H
