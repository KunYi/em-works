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
//
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  File:  omap2420_eac.h
//
//  This header file is comprised of EAC module register details defined as 
//  structures and macros for configuring and controlling EAC module.

#ifndef __OMAP2420_EAC_H
#define __OMAP2420_EAC_H

//------------------------------------------------------------------------------

// Base Address : OMAP2420_EAC_REGS_PA (defined in omap2420_base_regs.h as 0x48090000)

typedef struct __EACREGS__
{
   volatile unsigned short usCPCFR1;  //offset 0x0, Codec configuration1
   unsigned short usRESERVED_0x2;
   volatile unsigned short usCPCFR2;  //offset 0x4, Codec configuration2
   unsigned short usRESERVED_0x6;
   volatile unsigned short usCPCFR3;  //offset 0x8, Codec configuration3
   unsigned short usRESERVED_0xA;
   volatile unsigned short usCPCFR4;  //offset 0xC, Codec configuration4
   unsigned short usRESERVED_0xE;
   volatile unsigned short usCPTCTL;  //offset 0x10, Codec i/f ctrl & stat
   unsigned short usRESERVED_0x12;
   volatile unsigned short usCPTTADR; //offset 0x14, Codec i/f address
   unsigned short usRESERVED_0x16;
   volatile unsigned short usCPTDATL; //offset 0x18, Codec i/f data(L)
   unsigned short usRESERVED_0x1A;
   volatile unsigned short usCPTDATH; //offset 0x1C, Codec i/f data(H)
   unsigned short usRESERVED_0x1E;
   volatile unsigned short usCPTVSLL; //offset 0x20, Codec i/f val time slots(L)
   unsigned short usRESERVED_0x22;
   volatile unsigned short usCPTVSLH; //offset 0x24, Codec i/f val time slots(H)
   unsigned short usRESERVED_1[12];
   volatile unsigned short usMPCTR;   //offset 0x40, Modem control
   unsigned short usRESERVED_0x42;
   volatile unsigned short usMPMCCFR; //offset 0x44, Modem main channel config
   unsigned short usRESERVED_2[12];
   volatile unsigned short usBPCTR;   //offset 0x60, Bluetooth control
   unsigned short usRESERVED_0x62;
   volatile unsigned short usBPMCCFR; //offset 0x64, BTH main chnl port config
   unsigned short usRESERVED_3[12];
   volatile unsigned short usAMSCFR;  //offset 0x80, Audio mixer switch config
   unsigned short usRESERVED_0x82;
   volatile unsigned short usAMVCTR; //offset 0x84, Audio master volume ctrl
   unsigned short usRESERVED_0x86;
   volatile unsigned short usAM1VCTR; //offset 0x88, Audio mixer1 volume ctrl
   unsigned short usRESERVED_0x8A;
   volatile unsigned short usAM2VCTR; //offset 0x8C, Audio mixer2 volume ctrl
   unsigned short usRESERVED_0x8E;
   volatile unsigned short usAM3VCTR; //offset 0x90, Audio mixer3 volume ctrl
   unsigned short usRESERVED_0x92;
   volatile unsigned short usASTCTR; //offset 0x94, Audio side tone ctrl
   unsigned short usRESERVED_0x96;
   volatile unsigned short usAPD1LCR;//offset 0x98, Audio pk detector1 left chnl
   unsigned short usRESERVED_0x9A;
   volatile unsigned short usAPD1RCR;//offset 0x9C, Audio pk det1 right channel
   unsigned short usRESERVED_0x9E;
   volatile unsigned short usAPD2LCR;//offset 0xA0, Audio pk det2 left channel
   unsigned short usRESERVED_0xA2;
   volatile unsigned short usAPD2RCR;//offset 0xA4, Audio pk det2 right channel
   unsigned short usRESERVED_0xA6;
   volatile unsigned short usAPD3LCR;//offset 0xA8, Audio pk det3 left channel
   unsigned short usRESERVED_0xAA;
   volatile unsigned short usAPD3RCR;//offset 0xAC, Audio pk det3 right channel
   unsigned short usRESERVED_0xAE;
   volatile unsigned short usAPD4R; //offset 0xB0, Audio pk detector4
   unsigned short usRESERVED_0xB2;
   volatile unsigned short usADWR; //offset 0xB4, Audio DMA write data reg
   unsigned short usRESERVED_0xB6;
   volatile unsigned short usADRDR; //offset 0xB8, Audio DMA read data reg
   unsigned short usRESERVED_0xBA;
   volatile unsigned short usAGCFR; //offset 0xBC, Audio global config
   unsigned short usRESERVED_0xBE;
   volatile unsigned short usAGCTR; //offset 0xC0, Audio global control
   unsigned short usRESERVED_0xC2;
   volatile unsigned short usAGCFR2; //offset 0xC4, Audio global config, rev2
   unsigned short usRESERVED_0xC6;
   volatile unsigned short usAGCFR3; //offset 0xC8, Audio global config, rev2
   unsigned short usRESERVED_0xCA;
   volatile unsigned short usMBPDMACTR; //offset 0xCC, Modem/Bluetooth ports DMA
                                        // Channel control registers
   unsigned short usRESERVED_0xCE;
   volatile unsigned short usMPDDMARR; //offset 0xD0, Modem port dnlnk DMA read
   unsigned short usRESERVED_0xD2;
   volatile unsigned short usMPDDMAWR; //offset 0xD4, Modem port dnlnk DMA write
   unsigned short usRESERVED_0xD6;
   volatile unsigned short usMPUDMARR; //offset 0xD8, Modem port uplnk DMA read
   unsigned short usRESERVED_0xDA;
   unsigned short usRESERVED_0xDC;
   unsigned short usRESERVED_0xDE;
   volatile unsigned short usMPUDMAWR; //offset 0xE0, Modem port uplnk DMA write
   unsigned short usRESERVED_0xE2;
   volatile unsigned short usBPDDMARR; //offset 0xE4, Bluetooth dwnlnk DMA read
   unsigned short usRESERVED_0xE6;
   volatile unsigned short usBPDDMAWR; //offset 0xE8, Bluetooth dwnlnk DMA write
   unsigned short usRESERVED_0xEA;
   volatile unsigned short usBPUDMARR; //offset 0xEC, Bluetooth uplnk DMA read
   unsigned short usRESERVED_0xEE;
   volatile unsigned short usBPUDMAWR; //offset 0xF0, Bluetooth uplnk DMA write
   unsigned short usRESERVED_4[7];
   volatile unsigned short usVERSION_NUMBER; //offset 0x100, IP revision code
   unsigned short usRESERVED_0x102;
   volatile unsigned short usSYSCONFIG; //offset 0x104, Enbl ctrl of OCP params
   unsigned short usRESERVED_0x106;
   volatile unsigned short usSYSSTATUS; //offset 0x108, Provides status info
}
OMAP2420_EAC_REGS,*pEACREGS;


#endif
