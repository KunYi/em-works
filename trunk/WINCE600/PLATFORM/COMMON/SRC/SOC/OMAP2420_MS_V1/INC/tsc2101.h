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
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
//
// Copyright 2003 by Texas Instruments Incorporated. All rights reserved.
// Property of Texas Instruments Incorporated. Restricted rights to use,
// duplicate or disclose this code are granted through contract.
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
//
// Contains the internal definitions, variables, and function prototypes for
// the TSC2101 codec.
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

#ifndef __TSC2101_h__
#define __TSC2101_h__


#define TSC2101_READ							0x8000
#define TSC2101_ENBKL_GPIO_RD					0x0400
#define TSC2101_RESET		 				    0xBB00
// Codec TSC2101 memory map

// MCLK frequency
#define MCLK_16M								16000000
#define MCLK_13M7								13700000
#define MCLK_12M								12000000
#define MCLK_9M6								9600000

//Page 0
#define TSC2101_DATA_X_POS					    0x0000
#define TSC2101_DATA_Y_POS					    0x0020
#define TSC2101_DATA_Z1						    0x0040
#define TSC2101_DATA_Z2						    0x0060
#define TSC2101_DATA_BAT					    0x00A0
#define TSC2101_DATA_AUX1					    0x00E0
#define TSC2101_DATA_AUX2					    0x0100
#define TSC2101_DATA_TEMP1					    0x0120
#define TSC2101_DATA_TEMP2					    0x0140

//Page1
#define TSC2101_CTRL_TSC_ADC				    0x0800
#define TSC2101_CTRL_STATUS 				    0x0820
#define TSC2101_CTRL_BUFFER 				    0x0840
#define TSC2101_CTRL_REF	 				    0x0840
#define TSC2101_CTRL_RESET	 				    0x0860
#define TSC2101_CTRL_CONFIG 				    0x0880
#define TSC2101_CTRL_TEMP_MAX 				    0x08A0
#define TSC2101_CTRL_TEMP_MIN 				    0x08C0
#define TSC2101_CTRL_AUX1_MAX 				    0x08E0
#define TSC2101_CTRL_AUX1_MIN 				    0x0900
#define TSC2101_CTRL_AUX2_MAX 				    0x0920
#define TSC2101_CTRL_AUX2_MIN 				    0x0940
#define TSC2101_CTRL_MEASUR 				    0x0960
#define TSC2101_CTRL_DELAY	 				    0x0980
#define TSC2101_CTRL_RESET_DEV 				    0x09A0

//Page2
#define TSC2101_AUDCTRL_1	 				    0x1000
#define TSC2101_AUDCTRL_HEADSET	 				0x1020
#define TSC2101_AUDCTRL_DAC		 				0x1040
#define TSC2101_AUDCTRL_MIXER	 				0x1060
#define TSC2101_AUDCTRL_2	 					0x1080
#define TSC2101_AUDCTRL_POWER	 				0x10A0
#define TSC2101_AUDCTRL_3	 					0x10C0

//Audio Bass Boost Coefficients
#define TSC2101_AUDCTRL_LN0	 					0x10E0
#define TSC2101_AUDCTRL_LN1	 					0x1100
#define TSC2101_AUDCTRL_LN2	 					0x1120
#define TSC2101_AUDCTRL_LN3	 					0x1140
#define TSC2101_AUDCTRL_LN4	 					0x1160
#define TSC2101_AUDCTRL_LN5	 					0x1180
#define TSC2101_AUDCTRL_LD1	 					0x11A0
#define TSC2101_AUDCTRL_LD2	 					0x11C0
#define TSC2101_AUDCTRL_LD4	 					0x11E0
#define TSC2101_AUDCTRL_LD5	 					0x1200
#define TSC2101_AUDCTRL_RN0	 					0x1220
#define TSC2101_AUDCTRL_RN1	 					0x1240
#define TSC2101_AUDCTRL_RN2	 					0x1260
#define TSC2101_AUDCTRL_RN3	 					0x1280
#define TSC2101_AUDCTRL_RN4	 					0x12A0
#define TSC2101_AUDCTRL_RN5	 					0x12C0
#define TSC2101_AUDCTRL_RD1	 					0x12E0
#define TSC2101_AUDCTRL_RD2	 					0x1300
#define TSC2101_AUDCTRL_RD4	 					0x1320
#define TSC2101_AUDCTRL_RD5	 					0x1340

#define TSC2101_AUDCTRL_PLL0	 				0x1360
#define TSC2101_AUDCTRL_PLL1	 				0x1380
#define TSC2101_AUDCTRL_4	 					0x13A0
#define TSC2101_AUDCTRL_HANDSET					0x13C0
#define TSC2101_AUDCTRL_CELL 					0x13E0
#define TSC2101_AUDCTRL_5	 					0x1400
#define TSC2101_AUDCTRL_6	 					0x1420
#define TSC2101_AUDCTRL_7	 					0x1440
#define TSC2101_AUDCTRL_GPIO				    0x1460
#define TSC2101_AUDCTRL_AGC					    0x1480
#define TSC2101_AUDCTRL_PWR_STATUS			    0x14A0
#define TSC2101_AUDCTRL_MIC_AGC					0x14C0
#define TSC2101_AUDCTRL_CELL_AGC				0x14E0

//Page3
#define TSC2101_BUFCTRL_0						0x1800

/* Bit position */
#define TSC2101_BIT(ARG)    ((0x01)<<(ARG))

/* Field masks for Audio Control 1 */
#define AC1_ADCHPF(ARG)     (((ARG) & 0x03) << 14)
#define AC1_WLEN(ARG)       (((ARG) & 0x03) << 10)
#define AC1_DATFM(ARG)      (((ARG) & 0x03) << 8)
#define AC1_DACFS(ARG)      (((ARG) & 0x07) << 3)
#define AC1_ADCFS(ARG)      (((ARG) & 0x07))

/* Field masks for TSC2101_HEADSET_GAIN_CTRL */
#define HGC_ADMUT_HED       TSC2101_BIT(15)
#define HGC_ADPGA_HED(ARG)  (((ARG) & 0x7F) << 8)
#define HGC_AGCTG_HED(ARG)  (((ARG) & 0x07) << 5)
#define HGC_AGCTC_HED(ARG)  (((ARG) & 0x0F) << 1)
#define HGC_AGCEN_HED       (0x01)

/* Field masks for TSC2101_DAC_GAIN_CTRL */
#define DGC_DALMU           TSC2101_BIT(15)
#define DGC_DALVL(ARG)      (((ARG) & 0x7F) << 8)
#define DGC_DARMU           TSC2101_BIT(7)
#define DGC_DARVL(ARG)      (((ARG) & 0x7F))

/* Field masks for TSC2101_MIXER_PGA_CTRL */
#define MPC_ASTMU           TSC2101_BIT(15)
#define MPC_ASTG(ARG)       (((ARG) & 0x7F) << 8)
#define MPC_MICSEL(ARG)     (((ARG) & 0x07) << 5)
#define MPC_MICADC          TSC2101_BIT(4)
#define MPC_CPADC           TSC2101_BIT(3)
#define MPC_ASTGF           (0x01)

/* Field formats for TSC2101_AUDIO_CTRL_2 */
#define AC2_KCLEN           TSC2101_BIT(15)
#define AC2_KCLAC(ARG)      (((ARG) & 0x07) << 12)
#define AC2_APGASS          TSC2101_BIT(11)
#define AC2_KCLFRQ(ARG)     (((ARG) & 0x07) << 8)
#define AC2_KCLLN(ARG)      (((ARG) & 0x0F) << 4)
#define AC2_DLGAF           TSC2101_BIT(3)
#define AC2_DRGAF           TSC2101_BIT(2)
#define AC2_DASTC           TSC2101_BIT(1)
#define AC2_ADGAF           (0x01)

/* Field masks for TSC2101_CODEC_POWER_CTRL */
#define CPC_MBIAS_HND       TSC2101_BIT(15)
#define CPC_MBIAS_HED       TSC2101_BIT(14)
#define CPC_ASTPWD          TSC2101_BIT(13)
#define CPC_SP1PWDN         TSC2101_BIT(12)
#define CPC_SP2PWDN         TSC2101_BIT(11)
#define CPC_DAPWDN          TSC2101_BIT(10)
#define CPC_ADPWDN          TSC2101_BIT(9)
#define CPC_VGPWDN          TSC2101_BIT(8)
#define CPC_COPWDN          TSC2101_BIT(7)
#define CPC_LSPWDN          TSC2101_BIT(6)
#define CPC_ADPWDF          TSC2101_BIT(5)
#define CPC_LDAPWDF         TSC2101_BIT(4)
#define CPC_RDAPWDF         TSC2101_BIT(3)
#define CPC_ASTPWF          TSC2101_BIT(2)
#define CPC_BASSBC          TSC2101_BIT(1)
#define CPC_DEEMPF          (0x01)

/* Field masks for TSC2101_AUDIO_CTRL_3 */
#define AC3_DMSVOL(ARG)     (((ARG) & 0x03) << 14)
#define AC3_REFFS           TSC2101_BIT(13)
#define AC3_DAXFM           TSC2101_BIT(12)
#define AC3_SLVMS           TSC2101_BIT(11)
#define AC3_ADCOVF          TSC2101_BIT(8)
#define AC3_DALOVF          TSC2101_BIT(7)
#define AC3_DAROVF          TSC2101_BIT(6)
#define AC3_CLPST           TSC2101_BIT(3)
#define AC3_REVID(ARG)      (((ARG) & 0x07))

/* Field masks for TSC2101_PLL_PROG_1 */
#define PLL1_PLLSEL         TSC2101_BIT(15)
#define PLL1_QVAL(ARG)      (((ARG) & 0x0F) << 11)
#define PLL1_PVAL(ARG)      (((ARG) & 0x07) << 8)
#define PLL1_I_VAL(ARG)     (((ARG) & 0x3F) << 2)

/* Field masks of TSC2101_PLL_PROG_2 */
#define PLL2_D_VAL(ARG)     (((ARG) & 0x3FFF) << 2)

/* Field masks for TSC2101_AUDIO_CTRL_4 */
#define AC4_ADSTPD          TSC2101_BIT(15)
#define AC4_DASTPD          TSC2101_BIT(14)
#define AC4_ASSTPD          TSC2101_BIT(13)
#define AC4_CISTPD          TSC2101_BIT(12)
#define AC4_BISTPD          TSC2101_BIT(11)
#define AC4_AGCHYS(ARG)     (((ARG) & 0x03) << 9)
#define AC4_MB_HED(ARG)     (((ARG) & 0x03) << 7)
#define AC4_MB_HND          TSC2101_BIT(6)
#define AC4_SCPFL           TSC2101_BIT(1)

/* Field masks settings for TSC2101_HANDSET_GAIN_CTRL */
#define HNGC_ADMUT_HND      TSC2101_BIT(15)
#define HNGC_ADPGA_HND(ARG) (((ARG) & 0x7F) << 8)
#define HNGC_AGCTG_HND(ARG) (((ARG) & 0x07) << 5)
#define HNGC_AGCTC_HND(ARG) (((ARG) & 0x0F) << 1)
#define HNGC_AGCEN_HND      (0x01)

/* Field masks settings for TSC2101_BUZZER_GAIN_CTRL */
#define BGC_MUT_CP          TSC2101_BIT(15)
#define BGC_CPGA(ARG)       (((ARG) & 0x7F) << 8)
#define BGC_CPGF            TSC2101_BIT(7)
#define BGC_MUT_BU          TSC2101_BIT(6)
#define BGC_BPGA(ARG)       (((ARG) & 0x0F) << 2)
#define BGC_BUGF            TSC2101_BIT(1)

/* Field masks settings for TSC2101_AUDIO_CTRL_5 */
#define AC5_DIFFIN          TSC2101_BIT(15)
#define AC5_DAC2SPK1(ARG)   (((ARG) & 0x03) << 13)
#define AC5_AST2SPK1        TSC2101_BIT(12)
#define AC5_BUZ2SPK1        TSC2101_BIT(11)
#define AC5_KCL2SPK1        TSC2101_BIT(10)
#define AC5_CPI2SPK1        TSC2101_BIT(9)
#define AC5_DAC2SPK2(ARG)   (((ARG) & 0x03) << 7)
#define AC5_AST2SPK2        TSC2101_BIT(6)
#define AC5_BUZ2SPK2        TSC2101_BIT(5)
#define AC5_KCL2SPK2        TSC2101_BIT(4)
#define AC5_CPI2SPK2        TSC2101_BIT(3)
#define AC5_MUTSPK1         TSC2101_BIT(2)
#define AC5_MUTSPK2         TSC2101_BIT(1)
#define AC5_HDSCPTC         (0x01)

/* Field masks settings for TSC2101_AUDIO_CTRL_6 */
#define AC6_SPL2LSK         TSC2101_BIT(15)
#define AC6_AST2LSK         TSC2101_BIT(14)
#define AC6_BUZ2LSK         TSC2101_BIT(13)
#define AC6_KCL2LSK         TSC2101_BIT(12)
#define AC6_CPI2LSK         TSC2101_BIT(11)
#define AC6_MIC2CPO         TSC2101_BIT(10)
#define AC6_SPL2CPO         TSC2101_BIT(9)
#define AC6_SPR2CPO         TSC2101_BIT(8)
#define AC6_MUTLSPK         TSC2101_BIT(7)
#define AC6_MUTSPK2         TSC2101_BIT(6)
#define AC6_LDSCPTC         TSC2101_BIT(5)
#define AC6_VGNDSCPTC       TSC2101_BIT(4)
#define AC6_CAPINTF         TSC2101_BIT(3)

/* Field masks settings for TSC2101_AUDIO_CTRL_7 */
#define AC7_DETECT          TSC2101_BIT(15)
#define AC7_HESTYPE(ARG)    (((ARG) & 0x03) << 13)
#define AC7_HDDETFL         TSC2101_BIT(12)
#define AC7_BDETFL          TSC2101_BIT(11)
#define AC7_HDDEBNPG(ARG)   (((ARG) & 0x03) << 9)
#define AC7_BDEBNPG(ARG)    (((ARG) & 0x03) << 6)
#define AC7_DGPIO2          TSC2101_BIT(4)
#define AC7_DGPIO1          TSC2101_BIT(3)
#define AC7_CLKGPIO2        TSC2101_BIT(2)
#define AC7_ADWSF(ARG)      (((ARG) & 0x03))

/* Field masks settings for TSC2101_GPIO_CTRL */
#define GC_GPO2EN           TSC2101_BIT(15)
#define GC_GPO2SG           TSC2101_BIT(14)
#define GC_GPI2EN           TSC2101_BIT(13)
#define GC_GPI2SGF          TSC2101_BIT(12)
#define GC_GPO1EN           TSC2101_BIT(11)
#define GC_GPO1SG           TSC2101_BIT(10)
#define GC_GPI1EN           TSC2101_BIT(9)
#define GC_GPI1SGF          TSC2101_BIT(8)

/* Field masks for TSC2101_AGC_CTRL */
#define AC_AGCNF_CELL       TSC2101_BIT(14)
#define AC_AGCNL(ARG)       (((ARG) & 0x07) << 11)
#define AC_AGCHYS_CELL(ARG) (((ARG) & 0x03) << 9)
#define AC_CLPST_CELL       TSC2101_BIT(8)
#define AC_AGCTG_CELL(ARG)  (((ARG) & 0x07) << 5)
#define AC_AGCTC_CELL(ARG)  (((ARG) & 0x0F) << 1)
#define AC_AGCEN_CELL       (0x01)

/* Field masks for TSC2101_POWERDOWN_STS */
#define PS_SPK1FL            TSC2101_BIT(15)
#define PS_SPK2FL            TSC2101_BIT(14)
#define PS_HNDFL             TSC2101_BIT(13)
#define PS_VGNDFL            TSC2101_BIT(12)
#define PS_LSPKFL            TSC2101_BIT(11)
#define PS_CELLFL            TSC2101_BIT(10)
#define PS_PSEQ              TSC2101_BIT(5)
#define PS_PSTIME            TSC2101_BIT(4)

/* Field masks for Register Mic AGC Control */
#define MAC_MMPGA(ARG)       (((ARG) & 0x7F) << 9)
#define MAC_MDEBNS(ARG)      (((ARG) & 0x07) << 6)
#define MAC_MDEBSN(ARG)      (((ARG) & 0x07) << 3)

/* Field masks for Register Cellphone AGC Control */
#define CAC_CMPGA(ARG)       (((ARG) & 0x7F) << 9)
#define CAC_CDEBNS(ARG)      (((ARG) & 0x07) << 6)
#define CAC_CDEBSN(ARG)      (((ARG) & 0x07) << 3)

#endif // __TSC2101_h__

