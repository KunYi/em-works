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
#include "xllp_defs.h"
#include "xllp_serialization.h"
#include "xllp_clkmgr.h"

#ifndef XLLP_LCD
#define XLLP_LCD

//
// LCD Controller Register Definitions, associated data structures and bit macros
//

typedef struct 
{
	unsigned short palette[512];
}LCD_PALETTE;

typedef struct  
{
	unsigned int FDADR;		// Pointer to next frame descriptor (Physical address)
	unsigned int FSADR;		// Pointer to the data (Physical address)
	unsigned int FIDR;		// Frame descriptor ID
	unsigned int LDCMD;		// DMA command
	unsigned int PHYSADDR;  // PHYSADDR contains the physical address of this descriptor.
}LCD_FRAME_DESCRIPTOR;

typedef struct
{
	unsigned int sscr0;		
	unsigned int sscr1;
	unsigned int sssr;
	unsigned int ssitr;
	unsigned int ssdr;
	unsigned int rsvd1[5];
	unsigned int ssto;
	unsigned int sspsp;
	unsigned int sstsa;
	unsigned int ssrsa;
	unsigned int sstss;
	unsigned int ssacd;
} XLLP_SSPREGS_T , *P_XLLP_SSPREGS_T;

typedef struct  
{
	unsigned int LCCR0;			// 0x4400 0000
	unsigned int LCCR1;			// 0x4400 0004
	unsigned int LCCR2;			// 0x4400 0008
	unsigned int LCCR3;			// 0x4400 000C
	unsigned int LCCR4;			// 0x4400 0010
	unsigned int LCCR5;			// 0x4400 0014
	unsigned int reserved0[2];	// 0x4400 0018
	unsigned int FBR0;			// 0x4400 0020
	unsigned int FBR1;			// 0x4400 0024
	unsigned int FBR2;			// 0x4400 0028
	unsigned int FBR3;			// 0x4400 002C
	unsigned int FBR4;			// 0x4400 0030
	unsigned int LCSR1;			// 0x4400 0034
	unsigned int LCSR0;			// 0x4400 0038
	unsigned int LIIDR;			// 0x4400 003C
	unsigned int TRGBR;			// 0x4400 0040
	unsigned int TCR;			// 0x4400 0044
	unsigned int reserved1[2];  // 0x4400 0048
	unsigned int OVL1C1;		// 0x4400 0050
	unsigned int reserved2[3];	// 0x4400 0054
	unsigned int OVL1C2;		// 0x4400 0060
	unsigned int reserved3[3];	// 0x4400 0064
	unsigned int OVL2C1;		// 0x4400 0070
	unsigned int reserved4[3];	// 0x4400 0074
	unsigned int OVL2C2;		// 0x4400 0080
	unsigned int reserved5[3];	// 0x4400 0084
	unsigned int CCR;			// 0x4400 0090
	unsigned int reserved6[27];	// 0x4400 0094
	unsigned int CMDCR;			// 0x4400 0100
	unsigned int PRSR;			// 0x4400 0104
	unsigned int reserved7[2];	// 0x4400 0108
	unsigned int FBR5;			// 0x4400 0110
	unsigned int FBR6;			// 0x4400 0114
	unsigned int reserved8[58]; // 0x4400 0118
	unsigned int FDADR0;		// 0x4400 0200
	unsigned int FSADR0;		// 0x4400 0204
	unsigned int FIDR0;			// 0x4400 0208
	unsigned int LDCMD0;		// 0x4400 020C
	unsigned int FDADR1;		// 0x4400 0210
	unsigned int FSADR1;		// 0x4400 0214
	unsigned int FIDR1;			// 0x4400 0218
	unsigned int LDCMD1;		// 0x4400 021C
	unsigned int FDADR2;		// 0x4400 0220
	unsigned int FSADR2;		// 0x4400 0224
	unsigned int FIDR2;			// 0x4400 0228
	unsigned int LDCMD2;		// 0x4400 022C
	unsigned int FDADR3;		// 0x4400 0230
	unsigned int FSADR3;		// 0x4400 0234
	unsigned int FIDR3;			// 0x4400 0238
	unsigned int LDCMD3;		// 0x4400 023C
	unsigned int FDADR4;		// 0x4400 0240
	unsigned int FSADR4;		// 0x4400 0244
	unsigned int FIDR4;			// 0x4400 0248
	unsigned int LDCMD4;		// 0x4400 024C
	unsigned int FDADR5;		// 0x4400 0250
	unsigned int FSADR5;		// 0x4400 0254
	unsigned int FIDR5;			// 0x4400 0258
	unsigned int LDCMD5;		// 0x4400 025C
	unsigned int FDADR6;		// 0x4400 0260
	unsigned int FSADR6;		// 0x4400 0264
	unsigned int FIDR6;			// 0x4400 0268
	unsigned int LDCMD6;		// 0x4400 026C

}LCDRegs;

typedef struct
{
	unsigned long OverlayHeight;
	unsigned long OverlayWidth;
	unsigned long X_Position;
	unsigned long Y_Position;
	unsigned long Format;
	unsigned long DegradeBaseFrame;
	unsigned long CH1;	
	unsigned long CH2_Y;
	unsigned long CH3_Cb;
	unsigned long CH4_Cr;
	unsigned long OverlayBPP;
	unsigned long TmpBPP;
	unsigned long ch1_size;
	unsigned long ch2_size;
	unsigned long ch3_size;
	unsigned long ch4_size;
} XLLP_OVERLAY_T, *P_XLLP_OVERLAY_T;

typedef struct
{
	XLLP_VUINT32_T *GPIO;					
	XLLP_VUINT32_T *CLKMan;					
	XLLP_VUINT32_T *LCDC;
	XLLP_VUINT32_T *SSP;
	XLLP_VUINT32_T *OST;
	unsigned long DisplayType;
	unsigned long FrameBufferWidth;
	unsigned long FrameBufferHeight;
	unsigned long FrameBufferSize;
	unsigned long PaletteSize;
	unsigned long BPP;
	unsigned long PixelDataFormat;
	unsigned long CurrentPage;
	// frame buffers
	unsigned long _FRAME_BUFFER_BASE_PHYSICAL;				
	unsigned long _PALETTE_BUFFER_BASE_PHYSICAL;
        unsigned long _OVERLAY1_CHANNEL_BASE_PHYSICAL;
	unsigned long _OVERLAY2_Y_CHANNEL_BASE_PHYSICAL;
	unsigned long _OVERLAY2_Cb_CHANNEL_BASE_PHYSICAL;
	unsigned long _OVERLAY2_Cr_CHANNEL_BASE_PHYSICAL;
	// frame descriptors
	unsigned long _DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_PHYSICAL;	
	unsigned long _DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_PHYSICAL;
	unsigned long _DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_PHYSICAL;
	unsigned long _PALETTE_FRAME_DESCRIPTOR_BASE_PHYSICAL;
	unsigned long _DMA_CHANNEL_2_Y_FRAME_DESCRIPTOR_BASE_PHYSICAL;
	unsigned long _DMA_CHANNEL_3_Cb_FRAME_DESCRIPTOR_BASE_PHYSICAL;
	unsigned long _DMA_CHANNEL_4_Cr_FRAME_DESCRIPTOR_BASE_PHYSICAL;
	volatile LCD_FRAME_DESCRIPTOR *frameDescriptorCh0fd1;
	volatile LCD_FRAME_DESCRIPTOR *frameDescriptorCh0fd2;
	volatile LCD_FRAME_DESCRIPTOR *frameDescriptorCh1;
	volatile LCD_FRAME_DESCRIPTOR *frameDescriptorPalette;
	volatile LCD_FRAME_DESCRIPTOR *frameDescriptorTemp;
	volatile LCD_FRAME_DESCRIPTOR *frameDescriptorCh2_YCbCr_Y; 
	volatile LCD_FRAME_DESCRIPTOR *frameDescriptorCh3_YCbCr_Cb;
	volatile LCD_FRAME_DESCRIPTOR *frameDescriptorCh4_YCbCr_Cr;

} XLLP_LCD_T, *P_XLLP_LCD_T;

#define NONE		0

// Toshiba 640x480 LTM04C380K display
#define LTM04C380K	1

// Sharp 220x176 LQ64D341 display
#define LQ64D341	2

// Lubbock 640x480 passive
#define LM8V31		4		

// Sharp 240x320 Native Portrait QVGA display
#define LTM035A776C 5

// Sharp LS022Q8DD06 Sharp 240 x 320 for ZOAR
#define LS022Q8DD06 6

// Use the following for base frame values
#define BPP_1 			0x001
#define BPP_2 			0x002
#define BPP_4 			0x004
#define BPP_8 			0x008
#define BPP_16 			0x010
#define BPP_18 			0x020
#define BPP_18_PACKED 	0x040
#define BPP_19 			0x080
#define BPP_19_PACKED	0x100
#define BPP_24 			0x200
#define BPP_25 			0x400

// Use the following for overlay values
#define O_BPP_4				0x2
#define O_BPP_8 			0x3
#define O_BPP_16 			0x4
#define O_BPP_18 			0x5
#define O_BPP_18_PACKED 	0x6
#define O_BPP_19 			0x7
#define O_BPP_19_PACKED		0x8
#define O_BPP_24 			0x9
#define O_BPP_25 			0xA




// Use the following for PixelDataFormat values
#define PDFOR_00		0x0
#define PDFOR_01		0x1
#define PDFOR_10		0x2
#define PDFOR_11		0x3

// Use the following for SuspendType
#define Suspend_Graceful 	0
#define Suspend_Immediate 	1

#define CLK_SSP3 0x00000010
#define CLK_LCD  0x00010000
#define CLK_SRAM 0x00100000

#define INTERNAL_MEMORY_START 0x5C000000
#define INTERNAL_MEMORY_END	  (INTERNAL_MEMORY_START + 0x0003FFFF)

#define LTM04C380K_PIXEL_CLOCK_FREQUENCY 2518	// 25.18 MHz
#define LM8V31_PIXEL_CLOCK_FREQUENCY	  454	// 4.54 MHz
#define LQ64D341_PIXEL_CLOCK_FREQUENCY	  385	// 3.85 MHz
#define LTM035A776C_PIXEL_CLOCK_FREQUENCY 910   // 9.10 MHz
#define LS022Q8DD06_PIXEL_CLOCK_FREQUENCY 540   // 5.40 MHz


// Use the following for configuring overlay 2 format
#define FORMAT_RGB			0x0
#define FORMAT_PACKED_444	0x1
#define FORMAT_PLANAR_444	0x2
#define FORMAT_PLANAR_422	0x3
#define FORMAT_PLANAR_420	0x4

// Public API functions
#ifdef __cplusplus
extern "C" {
#endif
XLLP_STATUS_T XllpLCDInit(P_XLLP_LCD_T pXllpLCD);
XLLP_STATUS_T XllpLCD_Overlay2_Enable(P_XLLP_LCD_T pXllpLCD, P_XLLP_OVERLAY_T pXllpOverlay);
void XllpLCD_Overlay2_Disable(P_XLLP_LCD_T pXllpLCD, P_XLLP_OVERLAY_T pXllpOverlay);
XLLP_STATUS_T XllpLCD_Overlay1_Enable(P_XLLP_LCD_T pXllpLCD, P_XLLP_OVERLAY_T pXllpOverlay);
void XllpLCD_Overlay1_Disable(P_XLLP_LCD_T pXllpLCD, P_XLLP_OVERLAY_T pXllpOverlay);
void XllpLCDLoadPalette(P_XLLP_LCD_T pXllpLCD);
void XllpLCDSuspend(P_XLLP_LCD_T pXllpLCD, int SuspendType);
void XllpLCDResume(P_XLLP_LCD_T pXllpLCD);
void XllpLCDSetDisplayPage(P_XLLP_LCD_T pXllpLCD, int page);
void XllpLCD_DMALength(P_XLLP_OVERLAY_T pXllpOverlay);
#ifdef __cplusplus
}
#endif

//

// Private API functions
void LCDSetupGPIOs(P_XLLP_LCD_T pXllpLCD);
void LCDInitController(P_XLLP_LCD_T pXllpLCD);
void LCDClearStatusReg(P_XLLP_LCD_T pXllpLCD);
void LCDEnableController(P_XLLP_LCD_T pXllpLCD);
void LCDInitController(P_XLLP_LCD_T pXllpLCD);


//
// LCD Controller Control Register 0 (LCCR0)
//
#define LCD_ENB		0x00000001
#define LCD_CMS		0x00000002
#define LCD_SDS		0x00000004
#define LCD_LDM		0x00000008
#define LCD_SFM		0x00000010
#define LCD_IUM		0x00000020
#define LCD_EFM		0x00000040
#define LCD_PAS		0x00000080
#define LCD_BLE		0x00000100
#define LCD_DPD		0x00000200
#define LCD_DIS		0x00000400
#define LCD_QDM		0x00000800
#define LCD_PDD(n)	((n) << 12)
#define LCD_BM		0x00100000
#define LCD_OUM		0x00200000
#define LCD_LCDT	0x00400000
#define LCD_RDSTM	0x00800000
#define LCD_CMDIM	0x01000000
#define LCD_OUC		0x02000000
#define LCD_LDDALT	0x04000000

//
// LCD Controller Control Register 1 (LCCR1)
//
#define LCD_PPL(n)	((n))
#define LCD_HSW(n)	((n) << 10)
#define LCD_ELW(n)	((n) << 16)
#define LCD_BLW(n)	((n) << 24)

//
// LCD Controller Control Register 2 (LCCR2)
//
#define LCD_LPP(n)	((n))
#define LCD_VSW(n)	((n) << 10)
#define LCD_EFW(n)	((n) << 16)
#define LCD_BFW(n)	((n) << 24)

//
// LCD Controller Control Register 3 (LCCR3)
//
#define LCD_PCD(n)		((n))
#define LCD_ACB(n)		((n) << 8)
#define LCD_API(n)		((n) << 16)
#define LCD_VSP			0x00100000
#define LCD_HSP			0x00200000
#define LCD_PCP			0x00400000
#define LCD_OEP			0x00800000
#define LCD_DPC			0x08000000
#define LCD_BPP(n)		((((n) & 0x7) << 24) | (((n) & 0x8) << 26))
#define LCD_PDFOR(n)	((n) << 30)

//
// LCD Controller Control Register 4 (LCCR4)
//
#define LCD_K1(n)		((n))
#define LCD_K2(n)		((n) << 3)
#define LCD_K3(n)		((n) << 6)
#define LCD_PAL_FOR(n)	((n) << 15)
#define LCD_SENSE_PCD_CHG_EN	(1 << 25)
#define LCD_SENSE_MD_PCD(x)		((x) << 17)

//
// LCD Controller Control Register 5 (LCCR5)
//
#define LCD_SOFM1		0x00000001
#define LCD_SOFM2		0x00000002
#define LCD_SOFM3		0x00000004
#define LCD_SOFM4		0x00000008
#define LCD_SOFM5		0x00000010
#define LCD_SOFM6		0x00000020
#define LCD_EOFM1		0x00000100
#define LCD_EOFM2		0x00000200
#define LCD_EOFM3		0x00000400
#define LCD_EOFM4		0x00000800
#define LCD_EOFM5		0x00001000
#define LCD_EOFM6		0x00002000
#define LCD_BSM1		0x00010000
#define LCD_BSM2		0x00020000
#define LCD_BSM3		0x00040000
#define LCD_BSM4		0x00080000
#define LCD_BSM5		0x00100000
#define LCD_BSM6		0x00200000
#define LCD_IUM1		0x01000000
#define LCD_IUM2		0x02000000
#define LCD_IUM3		0x04000000
#define LCD_IUM4		0x08000000
#define LCD_IUM5		0x10000000
#define LCD_IUM6		0x20000000

//
// LCD Controller Overlay Control Register OVL1C1
//
#define LCD_PPL1(n)		((n))
#define LCD_LPO1(n)		((n) << 10)
#define LCD_BPP1(n)		((n) << 20)
#define LCD_O1EN		0x80000000

//
// LCD Controller Overlay Control Register OVL1C2
//
#define LCD_O1XPOS(n)	((n))
#define LCD_01YPOS(n)	((n) << 10)

//
// LCD Controller Overlay Control Register OVL2C1
//
#define LCD_PPL2(n)		((n))
#define LCD_LPO2(n)		((n) << 10)
#define LCD_BPP2(n)		((n) << 20)
#define LCD_O2EN		0x80000000

//
// LCD Controller Overlay Control Register OVL2C2
//
#define LCD_O2XPOS(n)	((n))
#define LCD_O2YPOS(n)	((n) << 10)
#define LCD_FOR(n)		((n) << 20)

//
// LCD Controller Cursor Control Register (CCR)
//
#define LCD_CURMS(n)	((n))
#define LCD_CXPOS(n)	((n) << 5)
#define LCD_CYPOS(n)	((n) << 15)
#define LCD_CEN			0x80000000

//
// LCD Controller Command Control Register (CMDCR)
//
#define LCD_SYNC_CNT(n)	((n))

//
// LCD Controller Panel Read Status Register (PRSR)
//
#define LCD_DATA(n)		((n) & 0xff)
#define LCD_A0			0x00000100
#define LCD_ST_OK		0x00000200
#define LCD_CON_ST		0x00000400

//
// LCD Controller Status Register (LCSR0)
//
#define LCD_LDD			0x00000001
#define LCD_SOF0		0x00000002
#define LCD_BER			0x00000004
#define LCD_ABC			0x00000008
#define LCD_IU0			0x00000010
#define LCD_IU1			0x00000020
#define LCD_OU			0x00000040
#define LCD_QD			0x00000080
#define LCD_EOF0		0x00000100
#define LCD_BS0			0x00000200
#define LCD_SINT		0x00000400
#define LCD_RD_ST		0x00000800
#define LCD_CMD_INTR	0x00001000
#define LCD_BER_CH(n)	(((n) & 0x7FFFFFFF) >> 28)

//
// LCD Controller Status Register (LCSR1)
//
#define LCD_SOF1	0x00000001
#define LCD_SOF2	0x00000002
#define LCD_SOF3	0x00000004
#define LCD_SOF4	0x00000008
#define LCD_SOF5	0x00000010
#define LCD_SOF6	0x00000020
#define LCD_EOF1	0x00000100
#define LCD_EOF2	0x00000200
#define LCD_EOF3	0x00000400
#define LCD_EOF4	0x00000800
#define LCD_EOF5	0x00001000
#define LCD_EOF6	0x00002000
#define LCD_BS1		0x00010000
#define LCD_BS2		0x00020000
#define LCD_BS3		0x00040000
#define LCD_BS4		0x00080000
#define LCD_BS5		0x00100000
#define LCD_BS6		0x00200000
#define LCD_IU2		0x02000000
#define LCD_IU3		0x04000000
#define LCD_IU4		0x08000000
#define LCD_IU5		0x10000000
#define LCD_IU6		0x20000000

//
// LCD Controller Interrupt ID Register (LIIDR)
//
#define LCD_IFrameID(n) ((n))

//
// LCD Controller TMED RGB Seed Register (TRGBR)
//
#define LCD_TRS(n) ((n))
#define LCD_TGS(n) ((n) << 8)
#define LCD_TBS(n) ((n) << 16)

//
// LCD Controller TMED Control Register (TCR)
//
#define LCD_TM2S	0x00000001
#define LCD_TM1S	0x00000002
#define LCD_TM2En	0x00000004
#define LCD_TM1En	0x00000008
#define LCD_TVBS(n)	((n) << 4)
#define LCD_THBS(n)	((n) << 8)
#define LCD_TSCS(n)	((n) << 12)
#define LCD_TED		0x00004000

//
// LCD Controller DMA Frame Descriptor Address Registers (FDADRx)
//
#define LCD_FDADR(n)	((n) & 0xFFFFFFF0)

//
// LCD Controller DMA Frame Source Address Registers (FSADRx)
//
#define LCD_FSADR(n)	((n) & 0xFFFFFFF8)

//
// LCD Controller DMA Frame ID Registers (FIDRx)
//
#define LCD_FIDR(n)		((n) & 0xFFFFFFF8)

//
// LCD Controller DMA Command Registers (LDCMDx)
//
#define LCD_Len(n)		((n))
#define LCD_EOFInt		0x00200000
#define LCD_SOFInt		0x00400000
#define LCD_Pal			0x04000000

//
// LCD Controller DMA Frame Branch Registers (FBRx)
//
#define LCD_BRA			0x00000001
#define LCD_BINT		0x00000002
#define LCD_SrcAddr(n)	((n) << 4)

#endif
