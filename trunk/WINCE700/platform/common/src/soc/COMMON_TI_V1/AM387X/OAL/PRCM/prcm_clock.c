// All rights reserved ADENEO EMBEDDED 2010
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//  File: prcm_clock.c
//
#include "omap.h"
#include "am387x_oal_prcm.h"
//#include "omap_prof.h"
#include "am387x.h"
//#include "omap_led.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"

// initialize voltage domain ref count
VddRefCountTable s_VddTable = {
    0,    // kARM_L,
    0,    // kARM_M,
    0,    // kCORE_L,
    0,    // kCORE_M,
    0,    // kACTIVE_L,
    0,    // kACTIVE_M,
    0,    // kIVAHD_L,
    0     // kIVAHD_M,
};


//-----------------------------------------------------------------------------
// initialize dpll domain ref count
DpllMap s_DpllTable = {
	{ kVDD_EXT,   0, NULL }, //	kAD_PLL,
	{ kVDD_EXT,   0, NULL }, // kDSP_PLL,
	{ kVDD_EXT,   0, NULL }, // kDSS_PLL,
	{ kVDD_EXT,   0, NULL }, // kISS_PLL,
	{ kVDD_EXT,   0, NULL }, // kIVA_PLL,
	{ kVDD_EXT,   0, NULL }, // kL3_PLL,
	{ kVDD_EXT,   0, NULL }, // kSGX_PLL,
	{ kVDD_EXT,   0, NULL }, // kUSB_PLL,
	{ kVDD_EXT,   0, NULL }, //	kDDR_PLL,
	{ kVDD_EXT,   0, NULL }, //	kVIDEO0_PLL,
	{ kVDD_EXT,   0, NULL }, //	kHDMI_PLL,
	{ kVDD_EXT,   0, NULL }, //	kVIDEO1_PLL,
	{ kVDD_EXT,   0, NULL }, //	kAUDIO_PLL,
	{ kVDD_EXT,   0, NULL }  //	kUNKNOWN_PLL,

};

//-----------------------------------------------------------------------------
// initialize dpll clkout ref count
//

// FIXME: correct freq to the actual values
DpllClkOutMap s_DpllClkOutTable = {
	{ kUNKNOWN_PLL, 0,     32768.0},	//kRCOSC_32K_CK,				
	{ kUNKNOWN_PLL, 0,     32768.0},	//kSYS_32K_CLKIN_CK,		
	{ kUNKNOWN_PLL, 0,     32768.0},	//kTCLKIN_CK,				
	{ kUNKNOWN_PLL, 0,  20000000.0},	//kOSC0_CLKIN_CK,			
	{ kUNKNOWN_PLL, 0,  22579000.0},	//kOSC1_CLKIN_CK,			
	{ kUNKNOWN_PLL, 0,     32768.0},	//kRTC_DIVIDER_CK,		
	{ kUNKNOWN_PLL, 0,  20000000.0},	//kOSC1_X1_CK,			
	{ kUNKNOWN_PLL, 0,  27000000.0},	//kXREF0_CK,				
	{ kUNKNOWN_PLL, 0,  27000000.0},	//kXREF1_CK,				
	{ kUNKNOWN_PLL, 0,  27000000.0},	//kXREF2_CK,				
	{ kUNKNOWN_PLL, 0,  82000000.0},	//kTSI0_DCK_CK,			
	{ kUNKNOWN_PLL, 0,  82000000.0},	//kTSI1_DCK_CK,			
	{ kUNKNOWN_PLL, 0,  50000000.0},	//kEXTERNAL_CK,			
	{ kUNKNOWN_PLL, 0,  22579000.0},	//kATL0_CLK_CK,			
	{ kUNKNOWN_PLL, 0,  22579000.0},	//kATL1_CLK_CK,			
	{ kUNKNOWN_PLL, 0,  22579000.0},	//kATL2_CLK_CK,			
	{ kUNKNOWN_PLL, 0,  22579000.0},	//kATL3_CLK_CK,			
	{ kUNKNOWN_PLL, 0, 600000000.0},	//kARM_DPLL_CK,			
	{ kUNKNOWN_PLL, 0, 500000000.0},	//kDSP_DPLL_CK,			
	{ kUNKNOWN_PLL, 0, 200000000.0},	//kSGX_DPLL_CK,			
	{ kUNKNOWN_PLL, 0, 425000000.0},	//kHDVICP_DPLL_CK,		 
	{ kUNKNOWN_PLL, 0, 220000000.0},	//kL3_DPLL_CK,			
	{ kUNKNOWN_PLL, 0, 400000000.0},	//kISS_DPLL_CK,			
	{ kUNKNOWN_PLL, 0, 200000000.0},	//kHDVPSS_DPLL_CK,		
	{ kUNKNOWN_PLL, 0, 166000000.0},	//kDVI_CH1_CK,			
	{ kUNKNOWN_PLL, 0, 166000000.0},	//kDVI_CH2_CK,			
	{ kUNKNOWN_PLL, 0, 166000000.0},	//kDVI_CH3_CK,			
	{ kUNKNOWN_PLL, 0, 166000000.0},	//kDVI_CH4_CK,			
	{ kUNKNOWN_PLL, 0, 162000000.0},	//kDVOI1_CK,				
	{ kUNKNOWN_PLL, 0, 162000000.0},	//kDVOI2_CK,				
	{ kUNKNOWN_PLL, 0, 960000000.0},	//kUSB_DPLL_CK,			
	{ kUNKNOWN_PLL, 0, 333000000.0},	//kDDR_DPLL_CK,			
	{ kUNKNOWN_PLL, 0,  54000000.0},	//kVIDEO0_DPLL_CK,		
	{ kUNKNOWN_PLL, 0, 148500000.0},	//kVIDEO1_DPLL_CK,		
	{ kUNKNOWN_PLL, 0, 186000000.0},	//kHDMI_DPLL_CK,          
	{ kUNKNOWN_PLL, 0, 165000000.0},	//kVIDEO_M_PCLK_CK,       
	{ kUNKNOWN_PLL, 0, 185625000.0},	//kHDMI_PHY_TCLK_CK,      
	{ kUNKNOWN_PLL, 0, 192000000.0},	//kAUDIO_DPLL_CK,			
	{ kUNKNOWN_PLL, 0, 100000000.0},	//kLJCB_SERDESP_CK,		
	{ kUNKNOWN_PLL, 0, 100000000.0},	//kLJCB_SERDESN_CK,		
	{ kUNKNOWN_PLL, 0,  50000000.0},	//kPCIESS_50M_CK,			
	{ kUNKNOWN_PLL, 0, 125000000.0},	//kPCIESS_125M_CK,		
	{ kUNKNOWN_PLL, 0, 100000000.0},	//kSATASSP_CK,			
	{ kUNKNOWN_PLL, 0, 100000000.0},	//kSATASSN_CK,			
	{ kUNKNOWN_PLL, 0,  20000000.0},	//kSATASS_20M_CK,			
	{ kUNKNOWN_PLL, 0,  50000000.0},	//kSATASS_50M_CK,			
	{ kUNKNOWN_PLL, 0, 125000000.0}		//kSATASS_125M_CK,		
};

static UINT arm_dpll_clkin_sel[] = { 2, kOSC0_CLKIN_CK, kRTC_DIVIDER_CK};
static UINT osc_sel[]            = { 2, kOSC0_CLKIN_CK, kOSC1_CLKIN_CK }; 
static UINT uart_fck_sel[]       = { 3, kSYSCLK8_CK, kSYSCLK10_CK, kSYSCLK6_CK };
static UINT securess_fck_sel[]   = { 2, kISS_DPLL_D2_CK, kUSB_DPLL_DIV5_CK};
static UINT hdmi_dpll_muxout_sel[]={ 2, kHDMI_DPLL_CK,  kVIDEO_M_PCLK_CK};
static UINT video012_dpll_muxout_sel[]={ 3, kVIDEO0_DPLL_CK, kHDMI_DPLL_MUXOUT_CK, kVIDEO1_DPLL_CK};
static UINT sysclk16_sel[]       = { 2, kSYSCLK16_D1MUX_CK, kSYSCLK16_B3MUX_CK };
static UINT sysclk14_sel[]       = { 3, kSYSCLK16_B3MUX_CK, kVIDEO0_DPLL_CLKIN_CK, kSYSCLK14_C1MUX_CK};
static UINT hd_venc_g_ck_sel[]   = { 2, kVIDEO1_DPLL_CK, kHDMI_DPLL_MUXOUT_CK };
static UINT tppss_stc0_fck_sel[] = { 4, kSYSCLK14_CK, kXREF0_CK, kXREF1_CK, kXREF2_CK};
static UINT audio_dpll_clk2_ck_sel[]={ 4, kISS_DPLL_D2_CK, kISS_DPLL_CK, kTSI0_DCK_CK, kTSI1_DCK_CK };
static UINT audio_prcm_clkin_ck_sel[]={2, kRTC_DIVIDER_CK, kSYS_32K_CLKIN_CK};
static UINT sysclk18_sel[]       = { 2, kAUDIO_PRCM_CLKIN_CK, kAUDIO_DPLL_CLK1_CK };
static UINT sysclk20_1_2_sel[]   = { 3, kSYSCLK20_CK, kSYSCLK21_CK, kSYSCLK22_CK };
static UINT mcbsp_fck_sel[]      = { 5, kAUDIO_PRCM1_OUT_CK, kXREF0_CK, kXREF1_CK, kXREF2_CK, kOSC1_X1_CK}; 
static UINT hdmi_i2s_fck_sel[]   = { 5, kHDMI_I2S_CK, kXREF0_CK, kXREF1_CK, kXREF2_CK, kOSC1_X1_CK };
static UINT atl_fck_sel[]        = { 3, kSYSCLK19_CK, kAUDIO_DPLL_CK, kVIDEO012_DPLL_MUXOUT_CK };
static UINT timer_clk_sel[]      = { 7, kSYSCLK18_CK, kXREF0_CK, kXREF1_CK, kXREF2_CK,
                                              kOSC0_CLKIN_CK, kOSC1_X1_CK, kTCLKIN_CK };
static UINT mcasp_auxclk_mux0_sel[]={ 2, kMLB_ICK, kMLBP_ICK };
static UINT mcasp345_aux_sel[]   = { 7,  kMCASP_AUXCLK_MUX0_CK, kAUDIO_DPLL_CK, kVIDEO012_DPLL_MUXOUT_CK,
                                        kXREF0_CK, kXREF1_CK, kXREF2_CK, kOSC1_X1_CK };
static UINT mcasp_ah_sel[]       = { 8, kXREF0_CK, kXREF1_CK, kXREF2_CK, kOSC1_X1_CK,
                                       kATL0_CLK_CK, kATL1_CLK_CK, kATL2_CLK_CK, kATL3_CLK_CK};
static UINT rmii_refclk_sel[]    = { 5, kVIDEO0_DPLL_CK, kVIDEO1_DPLL_CK, kAUDIO_DPLL_CK, kHDMI_DPLL_MUXOUT_CK, kL3_DPLL_CK };
static UINT gmac0to1_mux_sel[]   = { 2, kSATASS_50M_CK, kEXTERNAL_CK };
static UINT wdt0to1_fclk_mux_sel[] = { 2, kRTC_DIVIDER_CK, kRCOSC_32K_CK }; 
static UINT clkout0to3_mux_sel[] = { 4, kDSP_DPLL_CK, kHDVICP_DPLL_CK, kVIDEO0_DPLL_CK, kRTC_DIVIDER_CK };
static UINT clkout_mux_sel[]     = { 11, kCLKOUT_PRCM_CK, kSATASS_125M_CK, kPCIESS_125M_CK,
                                        kHDVPSS_PROC_D2_FCK, kISS_DPLL_D2_CK, kL3_DPLL_CK,
                                        kOSC0_CLKIN_CK, kOSC1_CLKIN_CK, kMPU_CK,
										kSGX_DPLL_CK, kRCOSC_32K_CK };

static UINT div_1_16[]           = {1, 2, 4, 8, 16, NOCLOCK, NOCLOCK, NOCLOCK }; 
static UINT div_1_8[] = {1,2,3,4,5,6,7,8,NOCLOCK};
static UINT div_1_2_22[] = {1,2,22,NOCLOCK,NOCLOCK,NOCLOCK,NOCLOCK,NOCLOCK};

SrcClockMap s_SrcClockTable = {
/* Order of the table elements MUST match the SourceClock_e list */ 	
/*   Parent Clock      RefCount  isDPLL  	  Divisor   clock    */
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kDCAN0_FCK,  NONE, 0, 0, 0, NULL	},
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kDCAN1_FCK,  NONE, 0, 0, 0, NULL	},
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kSR0_FCK,	NONE, 0, 0, 0, NULL	},
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kSR1_FCK,	NONE, 0, 0, 0, NULL	},
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kSR2_FCK,	NONE, 0, 0, 0, NULL	},
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kSR3_FCK,	NONE, 0, 0, 0, NULL	},
	{kOSC0_CLKIN_CK,    0,  FALSE,  1, kARM_DPLL_CLKIN_CK,
			SEL_PLLSS,	PLLSS_OFS(ARM_CLKSRC), 0, 1, arm_dpll_clkin_sel },
	{kARM_DPLL_CK,      0,  FALSE,  1, kMPU_CK,     NONE, 0, 0, 0, NULL	},
	{kDSP_DPLL_CK,		0,  FALSE,	1, kGEM_FCK,    NONE, 0, 0, 0, NULL	},
	{kSGX_DPLL_CK,		0,  FALSE,  1, kSYSCLK23_CK,
			DIV_PRCM,	PRCM_OFS(CM_SYSCLK23_CLKSEL), 0, 7, div_1_8     },
	{kSYSCLK23_CK,		0,  FALSE,  1, kSGX_CK,     NONE, 0, 0, 0, NULL	}, // en-0x0920[1:0]
	{kSYSCLK23_CK,		0,  FALSE,  1, kSGX_SYS_CK, NONE, 0, 0, 0, NULL	},
	{kSYSCLK23_CK,		0,  FALSE,  1, kSGX_MEM_CK, NONE, 0, 0, 0, NULL	},
	{kSYSCLK3_CK,		0,  FALSE,  1, kSYSCLK3_CK,
			DIV_PRCM,	PRCM_OFS(CM_SYSCLK3_CLKSEL), 0, 7, div_1_8   	},
	{kSYSCLK3_CK,		0,  FALSE,  1, kIVAHD0_CK,  NONE, 0, 0, 0, NULL	}, // en-0x0620[1:0]
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kL3_DPLL_CLKIN_CK,
			SEL_PLLSS,	PLLSS_OFS(OSC_SRC), 0, 1, osc_sel  	            },
	{kL3_DPLL_CK,		0,  FALSE,  1, kSYSCLK4_CK, NONE, 0, 0, 0, NULL	},
	{kSYSCLK4_CK,		0,  FALSE,	1, kL3_FAST_ICK,NONE, 0, 0, 0, NULL	}, // en-0x15E4[1:0]
	{kSYSCLK4_CK,		0,  FALSE,  1, kIVAHD0_ICK, NONE, 0, 0, 0, NULL	},
	{kSYSCLK4_CK,		0,  FALSE,  1, kEXP_SLOT_ICK,NONE, 0, 0, 0, NULL},
	{kSYSCLK4_CK,		0,  FALSE,  1, kMMU_ICK,    NONE, 0, 0, 0, NULL	}, // en-0x159C[1:0]
	{kSYSCLK4_CK,		0,  FALSE,  1, kGEM_ICK,    NONE, 0, 0, 0, NULL	}, // en-0x0420[1:0]
	{kSYSCLK4_CK,		0,  FALSE,  1, kTPTC0_ICK,  NONE, 0, 0, 0, NULL	}, // en-0x15F8[1:0]
	{kSYSCLK4_CK,		0,  FALSE,  1, kTPTC1_ICK,  NONE, 0, 0, 0, NULL	}, // en-0x15FC[1:0]
	{kSYSCLK4_CK,		0,  FALSE,  1, kTPTC2_ICK,  NONE, 0, 0, 0, NULL	}, // en-0x1600[1:0]
	{kSYSCLK4_CK,		0,  FALSE,  1, kTPTC3_ICK,  NONE, 0, 0, 0, NULL	}, // en-0x1604[1:0]
	{kSYSCLK4_CK,		0,  FALSE,	1, kIVAHD0_SL2_ICK,NONE, 0, 0, 0, NULL}, // en-0x0624[1:0]
	{kSYSCLK4_CK,		0,  FALSE,  1, kTPCC_ICK,   NONE, 0, 0, 0, NULL	}, // en-0x15F4[1:0]
	{kSYSCLK4_CK,		0,  FALSE,  1, kFDIF_ICK,   NONE, 0, 0, 0, NULL	},
	{kSYSCLK4_CK,	    0,  FALSE,  1, kHDVPSS_L3_ICK,NONE,0,0, 0, NULL },
	{kSYSCLK4_CK,		0,  FALSE,  1, kSYSCLK5_CK, NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,  1, kL3_MED_ICK, NONE, 0, 0, 0, NULL	}, // en-0x15E4[1:0]
	{kSYSCLK5_CK,		0,  FALSE,  1, kL4_FAST_ICK,NONE, 0, 0, 0, NULL	}, // en-0x15E8[1:0]
	{kSYSCLK5_CK,		0,  FALSE,  1, kSECURESS_ICK,NONE,0, 0, 0, NULL	}, // en-0x15C8[1:0]
	{kSYSCLK5_CK,		0,  FALSE,  1, kTPGSW_ICK,  NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,	1, kSATA_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x0560[1:0]
	{kSYSCLK5_CK,		0,  FALSE,  1, kPCIE_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x0510[1:0]
	{kSYSCLK5_CK,		0,  FALSE,  1, kVCP2_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x15B8[1:0]
	{kSYSCLK5_CK,		0,  FALSE,  1, kMLB_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x1574[1:0]
	{kSYSCLK5_CK,		0,  FALSE,  1, kFDIF_FCK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,  1, kDAP_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,  1, kOCMC_RAM_ICK,NONE,0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,	1, kMMCHS2_ICK, NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,  1, kCPSW_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x1404[1:0]
	{kSYSCLK5_CK,		0,  FALSE,  1, kGEM_TRC_FCK,NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,  1, kMCASP3_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,  1, kMCASP4_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,  1, kMCASP5_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK5_CK,		0,  FALSE,  1, kHDVPSS_L4_ICK,NONE,0,0, 0, NULL	},
	{kSYSCLK4_CK,		0,  FALSE,  2, kSYSCLK6_CK,	NONE, 0, 0, 0, NULL	}, // div-0x0314[0] 2/4 read only set to 2 
	{kSYSCLK6_CK,		0,  FALSE,  1, kL3_SLOW_ICK,NONE, 0, 0, 0, NULL	}, // en-0x15E4[1:0]
	{kSYSCLK6_CK,		0,  FALSE,	1, kL4_SLOW_ICK,NONE, 0, 0, 0, NULL	}, // en-0x15EC[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kUART0_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kUART1_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kUART2_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kUART3_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kUART4_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kUART5_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,	1, kI2C0_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kI2C1_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kI2C2_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kI2C3_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMCSPI0_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMCSPI1_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMCSPI2_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMCSPI3_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kSDIO_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,	1, kGPT1_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPT2_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPT3_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPT4_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPT5_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPT6_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPT7_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,	1, kGPT8_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPIO0_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x155C[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPIO1_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x1560[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kPRCM_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,	    0,  FALSE,  1, kSMARTCARD0_ICK,NONE,0,0,0, NULL	}, // en-0x15BC[1:0]
	{kSYSCLK6_CK,	    0,  FALSE,  1, kSMARTCARD1_ICK,NONE,0,0,0, NULL	}, // en-0x15C0[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kMCASP0_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMCASP1_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMCASP2_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,	1, kMCBSP_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kGPMC_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x15D0[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kHDMI_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMLBP_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x1574[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kWDT0_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x158C[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kWDT1_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,	    0,  FALSE,  1, kSYNC_TIMER_ICK,NONE,0,0,0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,	1, kPATA_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x1578[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kMAILBOX_ICK,NONE, 0, 0, 0, NULL	}, // en-0x1594[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kSPINBOX_ICK,NONE, 0, 0, 0, NULL	}, // en-0x1598[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kSR0_ICK	,	NONE, 0, 0, 0, NULL	}, // en-0x1608[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kSR1_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x160C[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kSR2_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x1610[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kSR3_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x1614[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kUSB_ICK,	NONE, 0, 0, 0, NULL	}, // en-0x0558[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kMMU_CFG_ICK,NONE, 0, 0, 0, NULL	}, // en-0x15A8[1:0]
	{kSYSCLK6_CK,		0,  FALSE,	1, kP1500_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kELM_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMMCHS0_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kMMCHS1_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kATL_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kFDIF_ENB_CK,NONE, 0, 0, 0, NULL	}, // en-0x0724[1:0]
	{kSYSCLK6_CK,		0,  FALSE,  1, kRTC_C32K_ICK,NONE,0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,	1, kDCAN0_ICK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK6_CK,		0,  FALSE,  1, kDCAN1_ICK,	NONE, 0, 0, 0, NULL	},
	{kISS_DPLL_CK,		0,  FALSE,  1, kISS_ICK,	NONE, 0, 0, 0, NULL	},
	{kISS_DPLL_CK,		0,  FALSE,  1, kTPPSS_TSO_ICK,	NONE, 0, 0, 0, NULL	},
	{kISS_DPLL_CK,		0,  FALSE,  2, kISS_DPLL_D2_CK,	NONE, 0, 0, 0, NULL	}, // fixed div=2
	{kISS_DPLL_D2_CK,	0,  FALSE,  1, kDUCATI_ICK,	NONE, 0, 0, 0, NULL	},     // en-0x0574[1:0]
	{kHDVPSS_DPLL_CK,	0,  FALSE,  1, kHDVPSS_PROC_FCK,	NONE, 0, 0, 0, NULL	},
	{kHDVPSS_DPLL_CK,	0,  FALSE,  2, kHDVPSS_PROC_D2_FCK,	NONE, 0, 0, 0, NULL	}, // fixed div=2
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kUSB_DPLL_CLKIN_CK,	NONE, 0, 0, 0, NULL	},
	{kUSB_DPLL_CK,		0,  FALSE,	1, kUSB_PHY0_RCLK_ICK,	NONE, 0, 0, 0, NULL	},
	{kUSB_DPLL_CK,		0,  FALSE,  1, kUSB_PHY1_RCLK_ICK,	NONE, 0, 0, 0, NULL	},
	{kUSB_DPLL_CK,		0,  FALSE,  5, kUSB_DPLL_DIV5_CK,	NONE, 0, 0, 0, NULL	}, // fixed div=5
	{kUSB_DPLL_DIV5_CK,	0,  FALSE,  1, kSYSCLK10_CK,
			DIV_PRCM,	PRCM_OFS(CM_SYSCLK10_CLKSEL), 0, 7, div_1_8   	},
	{kSYSCLK10_CK,		0,  FALSE,  1, kMCSPI0_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x1590[1:0]
	{kSYSCLK10_CK,		0,  FALSE,  1, kMCSPI1_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x1590[1:0]
	{kSYSCLK10_CK,		0,  FALSE,  1, kMCSPI2_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x1590[1:0]
	{kSYSCLK10_CK,		0,  FALSE,	1, kMCSPI3_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x1590[1:0]
	{kSYSCLK10_CK,		0,  FALSE,  1, kI2C02_CK,	NONE, 0, 0, 0, NULL	}, // en-0x1564[1:0]
	{kSYSCLK10_CK,		0,  FALSE,  1, kI2C13_CK,	NONE, 0, 0, 0, NULL	}, // en-0x1568[1:0]
	{kI2C02_CK,			0,  FALSE,  1, kI2C0_FCK,	NONE, 0, 0, 0, NULL	},
	{kI2C13_CK,			0,  FALSE,  1, kI2C1_FCK,	NONE, 0, 0, 0, NULL	},
	{kI2C02_CK,			0,  FALSE,  1, kI2C2_FCK,	NONE, 0, 0, 0, NULL	},
	{kI2C13_CK,			0,  FALSE,  1, kI2C3_FCK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK10_CK,		0,  FALSE,  1, kUART0_FCK,	NONE, 0, 0, 0, NULL	}, // en-1550[1:0] 
	{kSYSCLK10_CK,		0,  FALSE,  1, kUART1_FCK,	NONE, 0, 0, 0, NULL	}, // en-1554[1:0] 
	{kSYSCLK10_CK,		0,  FALSE,  1, kUART2_FCK,	NONE, 0, 0, 0, NULL	}, // en-1558[1:0] 
	{kSYSCLK10_CK,		0,  FALSE,  1, kHDMI_CEC_DCC_FCK,NONE, 0, 0, 0, NULL},
	{kUSB_DPLL_DIV5_CK,	0,  FALSE,  1, kSYSCLK8_CK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK8_CK,		0,  FALSE,  1, kMMCHS0_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x161C[1:0]
	{kSYSCLK8_CK,		0,  FALSE,  1, kMMCHS1_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x1620[1:0]
	{kSYSCLK8_CK,		0,  FALSE,  1, kMMCHS2_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x1624[1:0]
	{kSYSCLK8_CK,		0,  FALSE,  1, kUART3_FCK,
			SEL_PLLSS,	PLLSS_OFS(McBSP_UART_CLKSRC), 3, 3, uart_fck_sel }, // en-0x1580[1:0];
	{kSYSCLK8_CK,		0,  FALSE,	1, kUART4_FCK,
			SEL_PLLSS,	PLLSS_OFS(McBSP_UART_CLKSRC), 5, 3, uart_fck_sel }, // en-0x1584[1:0];
	{kSYSCLK8_CK,		0,  FALSE,  1, kUART5_FCK,
			SEL_PLLSS,	PLLSS_OFS(McBSP_UART_CLKSRC), 7, 3, uart_fck_sel }, // en-0x1588[1:0];
	{kSYSCLK8_CK,		0,  FALSE,  1, kSECURESS_FCK,
			SEL_PLLSS,	PLLSS_OFS(SECSS_CLK_SRC),     0, 1, securess_fck_sel }, // en-0x15C8[1:0];
	{kSECURESS_FCK,		0,  FALSE,  1, kTPPSS_FCK,	NONE, 0, 0, 0, NULL	},
	{kUSB_DPLL_DIV5_CK,	0,  FALSE,  2, kCSI2_PHY_FCK,NONE,0, 0, 0, NULL }, // fixed div=2
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kDDR_DPLL_CLKIN_CK,	NONE, 0, 0, 0, NULL	},
	{kDDR_DPLL_CK,		0,  FALSE,  1, kDDR0_PHY_FCK,	NONE, 0, 0, 0, NULL	}, // en-PLLSS_PREGS->EMIF_CLK_GATE[0] fixed div=1
	{kDDR_DPLL_CK,		0,  FALSE,  1, kDDR1_PHY_FCK,	NONE, 0, 0, 0, NULL	}, // en-PLLSS_PREGS->EMIF_CLK_GATE[1] fixed div=1
	{kDDR_DPLL_CK,		0,  FALSE,  2, kDDR0_HALF_FCK,	NONE, 0, 0, 0, NULL	}, // en-PLLSS_PREGS->EMIF_CLK_GATE[0] fixed div=2
	{kDDR_DPLL_CK,		0,  FALSE,  2, kDDR1_HALF_FCK,	NONE, 0, 0, 0, NULL	}, // en-PLLSS_PREGS->EMIF_CLK_GATE[1] fixed div=2
	{kDDR0_HALF_FCK,	0,  FALSE,  1, kDDR0_PHY_D2_FCK,NONE, 0, 0, 0, NULL	},
	{kDDR0_HALF_FCK,	0,  FALSE,  1, kDDR0_EMIF_FCK,	NONE, 0, 0, 0, NULL	},
	{kDDR0_HALF_FCK,	0,  FALSE,  1, kDMM_PHY_FCK,	NONE, 0, 0, 0, NULL	},
	{kDDR1_HALF_FCK,	0,  FALSE,  1, kDDR1_PHY_D2_FCK,NONE, 0, 0, 0, NULL	},
	{kDDR1_HALF_FCK,	0,  FALSE,  1, kDDR1_EMIF_FCK,	NONE, 0, 0, 0, NULL	},

	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kVIDEO0_DPLL_CLKIN_CK, SEL_PLLSS, PLLSS_OFS(OSC_SRC), 16, 1, osc_sel },
	{kOSC0_CLKIN_CK,	0,  FALSE,	1, kVIDEO1_DPLL_CLKIN_CK, SEL_PLLSS, PLLSS_OFS(OSC_SRC), 17, 1, osc_sel },
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kHDMI_DPLL_CLKIN_CK,   SEL_PLLSS, PLLSS_OFS(OSC_SRC), 18, 1, osc_sel },
	{kHDMI_DPLL_CK,		0,  FALSE,  1, kHDMI_DPLL_MUXOUT_CK,
			SEL_PLLSS,	PLLSS_OFS(VIDEO_PLL_CLKSRC), 0, 1, hdmi_dpll_muxout_sel },
	{kVIDEO0_DPLL_CK	,0,FALSE,  1, kVIDEO012_DPLL_MUXOUT_CK,
			SEL_PLLSS,	PLLSS_OFS(VIDEO_PLL_CLKSRC), 8, 3, video012_dpll_muxout_sel },
	{kVIDEO0_DPLL_CK,	0,  FALSE,  1, kSYSCLK16_D1MUX_CK,
			DIV_PRCM,	PRCM_OFS(CM_PV0D1_CLKSEL), 0, 7, div_1_8   	},
	{kHDMI_DPLL_MUXOUT_CK,0,FALSE,  1, kSYSCLK16_B3MUX_CK,
			DIV_PRCM,	PRCM_OFS(CM_PV2B3_CLKSEL), 0, 3, div_1_2_22 },
	{kVIDEO1_DPLL_CK,	0,  FALSE,  1, kSYSCLK14_C1MUX_CK,
			DIV_PRCM,	PRCM_OFS(CM_PV1C1_CLKSEL), 0, 3, div_1_2_22 },
	{kSYSCLK16_D1MUX_CK,			0,  FALSE,  1, kSYSCLK16_CK,
			SEL_PRCM,	PRCM_OFS(CM_SYSCLK16_CLKSEL), 0, 1, sysclk16_sel },
	{kSYSCLK16_CK,		0,  FALSE,  1, kTPPSS_STC1_FCK,	NONE, 0, 0, 0, NULL	},
	{kVIDEO1_DPLL_CK,	0,  FALSE,  1, kHD_VENC_G_CK,
			SEL_PLLSS,	PLLSS_OFS(VIDEO_PLL_CLKSRC), 24, 1, hd_venc_g_ck_sel },
	{kHDMI_DPLL_MUXOUT_CK,0,FALSE,  1, kHD_VENC_D_CK,	NONE, 0, 0, 0, NULL	},
	{kHD_VENC_D_CK,		0,  FALSE,  1, kHDMI_PHY_GCLK_CK,NONE,0, 0, 0, NULL	},
	{kVIDEO0_DPLL_CK,	0,  FALSE,  1, kSD_VENC_CK,		NONE, 0, 0, 0, NULL	},
	{kSYSCLK16_B3MUX_CK,0,  FALSE,  1, kSYSCLK14_CK,
		SEL_PRCM,	PRCM_OFS(CM_SYSCLK14_CLKSEL), 0, 3, sysclk14_sel},
	{kSYSCLK14_CK,		0,  FALSE,  1, kTPPSS_STC0_FCK,
		SEL_PLLSS,	PLLSS_OFS(VIDEO_PLL_CLKSRC), 16, 3, tppss_stc0_fck_sel},
	{kISS_DPLL_D2_CK, 	0,  FALSE,  1, kAUDIO_DPLL_CLK2_CK,
		SEL_PLLSS,	PLLSS_OFS(VIDEO_PLL_CLKSRC), 18, 3, audio_dpll_clk2_ck_sel},
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kAUDIO_DPLL_CLKIN_CK, SEL_PLLSS, PLLSS_OFS(OSC_SRC), 24, 1, osc_sel},
	{kRTC_DIVIDER_CK, 	0,  FALSE,  1, kAUDIO_DPLL_CLK1_CK,
		DIV_PRCM,	PRCM_OFS(CM_RTCDIVA_CLKSEL), 0, 7, div_1_8},
	{kRTC_DIVIDER_CK,	0,  FALSE,  1, kAUDIO_PRCM_CLKIN_CK,
		SEL_PLLSS,	PLLSS_OFS(SYSCLK_18_SRC), 0, 1, audio_prcm_clkin_ck_sel},
	{kAUDIO_PRCM_CLKIN_CK,0,FALSE,  1, kSYSCLK18_CK,
		SEL_PRCM,	PRCM_OFS(CM_SYSCLK18_CLKSEL), 0, 1, sysclk18_sel},

	{kSYSCLK18_CK,		0,  FALSE,  1, kRTC_C32K_FCK,	NONE, 0, 0, 0, NULL },
	{kAUDIO_DPLL_CLK2_CK,0, FALSE,  1, kSYSCLK19_CK,
		DIV_PRCM,	PRCM_OFS(CM_SYSCLK19_CLKSEL), 0, 7, div_1_8},
	{kAUDIO_DPLL_CK,	0,  FALSE,  1, kSYSCLK20_CK,
		DIV_PRCM,	PRCM_OFS(CM_SYSCLK20_CLKSEL), 0, 7, div_1_8},
	{kVIDEO012_DPLL_MUXOUT_CK,0,FALSE, 1, kSYSCLK21_CK,
		DIV_PRCM,	PRCM_OFS(CM_SYSCLK21_CLKSEL), 0, 7, div_1_8},
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kSYSCLK22_CK,	NONE, 0, 0, 0, NULL },
	{kSYSCLK20_CK,		0,  FALSE,  1, kAUDIO_PRCM1_OUT_CK,
		SEL_PRCM,	PRCM_OFS(CM_AUDIOCLK_MCBSP_CLKSEL), 0, 1, sysclk20_1_2_sel}, // en-0x154C[1..0];
	{kAUDIO_PRCM1_OUT_CK,0, FALSE,  1, kMCBSP_FCK,
		SEL_PLLSS,	PLLSS_OFS(McBSP_UART_CLKSRC), 0, 7, mcbsp_fck_sel},
	{kSYSCLK20_CK,		0,  FALSE,  1, kMCASP0_FCK,
		SEL_PRCM,	PRCM_OFS(CM_AUDIOCLK_MCASP0_CLKSEL), 0, 1, sysclk20_1_2_sel}, // en-0x1540[1..0];
	{kSYSCLK20_CK,		0,  FALSE,  1, kMCASP1_FCK,
		SEL_PRCM,	PRCM_OFS(CM_AUDIOCLK_MCASP1_CLKSEL), 0, 1, sysclk20_1_2_sel}, // en-0x1544[1..0];
	{kSYSCLK20_CK,		0,  FALSE,  1, kMCASP2_FCK,
		SEL_PRCM,	PRCM_OFS(CM_AUDIOCLK_MCASP2_CLKSEL), 0, 1, sysclk20_1_2_sel}, // en-0x1548[1..0];
	{kSYSCLK20_CK,		0,  FALSE,  1, kHDMI_I2S_CK,
		SEL_PRCM,	PRCM_OFS(CM_HDMI_CLKSEL), 0, 1, sysclk20_1_2_sel}, // en-0x0824[1..0];
	{kHDMI_I2S_CK,		0,  FALSE,  1, kHDMI_I2S_FCK,
		SEL_PLLSS,	PLLSS_OFS(HDMI_I2S_CLKSRC), 0, 7, hdmi_i2s_fck_sel },
	{kSYSCLK19_CK,		0,  FALSE,  1, kTPPSS_TSO_FCK,	NONE, 0, 0, 0, NULL	}, // en-0x554[1..0]
	{kSYSCLK19_CK,		0,  FALSE,  1, kATL_FCK,
		SEL_PLLSS,	PLLSS_OFS(MLB_ATL_CLKSRC), 16, 3, atl_fck_sel },
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPIO0_DBCK,		NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPIO1_DBCK,		NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kRTC_FCK,		NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kMMCHS0_DBCK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kMMCHS1_DBCK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kMMCHS2_DBCK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kSYNC_TIMER_FCK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kBANDGAPS_FCK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kARM_OPER_FCK,	NONE, 0, 0, 0, NULL	},
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPT1_FCK,
		SEL_PLLSS,	PLLSS_OFS(TIMER_CLK_CHANGE), 3, 7, timer_clk_sel },
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPT2_FCK,
		SEL_PLLSS,	PLLSS_OFS(TIMER_CLK_CHANGE), 6, 7, timer_clk_sel },
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPT3_FCK,
		SEL_PLLSS,	PLLSS_OFS(TIMER_CLK_CHANGE), 9, 7, timer_clk_sel },
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPT4_FCK,
		SEL_PLLSS,	PLLSS_OFS(TIMER_CLK_CHANGE), 16, 7, timer_clk_sel },
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPT5_FCK,
		SEL_PLLSS,	PLLSS_OFS(TIMER_CLK_CHANGE), 19, 7, timer_clk_sel },
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPT6_FCK,
		SEL_PLLSS,	PLLSS_OFS(TIMER_CLK_CHANGE), 22, 7, timer_clk_sel },
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPT7_FCK,
		SEL_PLLSS,	PLLSS_OFS(TIMER_CLK_CHANGE), 25, 7, timer_clk_sel },
	{kSYSCLK18_CK,		0,  FALSE,  1, kGPT8_FCK,
		SEL_PLLSS,	PLLSS_OFS(TIMER_CLK_CHANGE), 0, 7, timer_clk_sel },
	{kMLB_ICK,			0,  FALSE,  1, kMCASP_AUXCLK_MUX0_CK,
		SEL_PLLSS,	PLLSS_OFS(MLB_ATL_CLKSRC), 0, 1, mcasp_auxclk_mux0_sel },
	{kMCASP_AUXCLK_MUX0_CK,0,FALSE, 1, kMCASP3_FCK, SEL_PLLSS,	PLLSS_OFS(McASP345_AUX_CLKSRC), 0, 7, mcasp345_aux_sel},
	{kMCASP_AUXCLK_MUX0_CK,0,FALSE, 1, kMCASP4_FCK, SEL_PLLSS,	PLLSS_OFS(McASP345_AUX_CLKSRC), 8, 7, mcasp345_aux_sel},
	{kMCASP_AUXCLK_MUX0_CK,0,FALSE, 1, kMCASP5_FCK, SEL_PLLSS,	PLLSS_OFS(McASP345_AUX_CLKSRC), 16, 7, mcasp345_aux_sel},
	{kXREF0_CK,	0,  FALSE,  1, kMCASP0_AHX_CK, SEL_PLLSS,	PLLSS_OFS(McASP_AHCLK_CLKSRC), 0, 7, mcasp_ah_sel},
	{kXREF0_CK,	0,  FALSE,  1, kMCASP0_AHR_CK, SEL_PLLSS,	PLLSS_OFS(McASP_AHCLK_CLKSRC), 3, 7, mcasp_ah_sel},
	{kXREF0_CK,	0,  FALSE,  1, kMCASP1_AHX_CK, SEL_PLLSS,	PLLSS_OFS(McASP_AHCLK_CLKSRC), 6, 7, mcasp_ah_sel},
	{kXREF0_CK,	0,  FALSE,  1, kMCASP1_AHR_CK, SEL_PLLSS,	PLLSS_OFS(McASP_AHCLK_CLKSRC), 9, 7, mcasp_ah_sel},
	{kXREF0_CK,	0,  FALSE,  1, kMCASP2_AHX_CK, SEL_PLLSS,	PLLSS_OFS(McASP_AHCLK_CLKSRC), 16, 7, mcasp_ah_sel},
	{kXREF0_CK,	0,  FALSE,  1, kMCASP3_AHX_CK, SEL_PLLSS,	PLLSS_OFS(McASP_AHCLK_CLKSRC), 19, 7, mcasp_ah_sel},
	{kXREF0_CK,	0,  FALSE,  1, kMCASP4_AHX_CK, SEL_PLLSS,	PLLSS_OFS(McASP_AHCLK_CLKSRC), 22, 7, mcasp_ah_sel},
	{kXREF0_CK,	0,  FALSE,  1, kMCASP5_AHX_CK, SEL_PLLSS,	PLLSS_OFS(McASP_AHCLK_CLKSRC), 25, 7, mcasp_ah_sel},
	{kVIDEO0_DPLL_CK,	0,  FALSE,  1, kCPTS_RFT_CLK_CK, SEL_PLLSS,	PLLSS_OFS(RMII_REFCLK_SRC), 1, 7, rmii_refclk_sel},
	{kOSC0_CLKIN_CK,	0,  FALSE,  1, kPCIESS_20M_CK,	NONE, 0, 0, 0, NULL	},
	{kLJCB_SERDESP_CK,	0,  FALSE,  1, kPCIESSP_CK,		NONE, 0, 0, 0, NULL	},
	{kLJCB_SERDESN_CK,	0,  FALSE,  1, kPCIESSN_CK,		NONE, 0, 0, 0, NULL	},
	{kSATASS_50M_CK,	0,  FALSE,  1, kEMAC_RMII_FCK,
		 SEL_PLLSS,	PLLSS_OFS(RMII_REFCLK_SRC), 0, 1, gmac0to1_mux_sel},
	{kSATASS_125M_CK,	0,  FALSE,  1, kEMAC_GMII_FCK,	NONE, 0, 0, 0, NULL	},
	{kRTC_DIVIDER_CK,	0,  FALSE,  1, kWDT0_FCK,
		 SEL_PLLSS,	PLLSS_OFS(WDT0_SRC), 0, 1, wdt0to1_fclk_mux_sel}, // sel PLLSS_PREGS->WDT0_SRC[0]
	{kRCOSC_32K_CK,		0,  FALSE,  1, kWDT1_FCK,		NONE, 0, 0, 0, NULL	},
	{kDSP_DPLL_CK,		0,  FALSE,  1, kCLKOUT_PRCM_MUX_CK,
		SEL_PRCM,	PRCM_OFS(CM_CLKOUT_CTRL), 0, 3, clkout0to3_mux_sel},
	{kCLKOUT_PRCM_MUX_CK,0, FALSE,  1, kCLKOUT_PRCM_CK,
		DIV_PRCM,	PRCM_OFS(CM_CLKOUT_CTRL), 3, 7, div_1_16},
	{kCLKOUT_PRCM_CK,	0,  FALSE,  1, kSYS_CLKOUT0,
		SEL_PLLSS,	PLLSS_OFS(CLKOUT_MUX), 0, 15, clkout_mux_sel }, // PLLSS_PREGS->CLKOUT_MUX[3..0]   - clkout0
	{kCLKOUT_PRCM_CK,	0,  FALSE,  1, kSYS_CLKOUT1,
		SEL_PLLSS,	PLLSS_OFS(CLKOUT_MUX), 16, 15, clkout_mux_sel }, // PLLSS_PREGS->CLKOUT_MUX[19..16] - clkout1
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ClockDomainLookupEntry s_rgClkDomainLookupTable[] = {
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON2_L3_MED_CLKSTCTRL)	}, //CLKDMN_ALWON2_L3_MED,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON2_PCI_CLKSTCTRL)		}, //CLKDMN_ALWON2_PCIE,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON2_TPPSS_CLKSTCTRL)	}, //CLKDMN_ALWON2_TPPSS,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON2_USB_CLKCTRL)		}, //CLKDMN_ALWON2_USB,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON2_L3_SLOW_CLKSTCTRL) }, //CLKDMN_ALWON2_L3_SLOW,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON2_L3_FAST_CLKSTCTRL) }, //CLKDMN_ALWON2_L3_FAST,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON2_MC_CLKSTCTRL)		}, //CLKDMN__ALWON2_MC,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_SYSCLK6_CLKSTCTRL)	}, //CLKDMN_SYSCLK6,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_SYSCLK4_CLKSTCTRL)	}, //CLKDMN_SYSCLK4,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON_L3_SLOW_CLKSTCTRL)  }, //CLKDMN_ALWON_L3_SLOW,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ETHERNET_CLKSTCTRL)       }, //CLKDMN_ALWON_ETHERNET,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_L3_MED_CLKSTCTRL)   }, //CLKDMN_ALWON_L3_MED,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_MMU_CLKSTCTRL)			}, //CLKDMN_MMU,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_MMUCFG_CLKSTCTRL)			}, //CLKDMN_MMU_CFG,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON_OCMC_0_CLKSTCTRL)   }, //CLKDMN_ALWON_OCMC_0,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON_VCP_CLKSTCTRL)      }, //CLKDMN_ALWON_VCP,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_MPU_CLKSTCTRL)		}, //CLKDMN_ALWON_MPU,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_RTC_CLKSTCTRL)		}, //CLKDMN_ALWON_RTC,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON_L3_FAST_CLKSTCTRL)  }, //CLKDMN_ALWON_L3_FAST,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_DSP_CLKSTCTRL)			}, //CLKDMN_GEM,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_HDVICP_CLKSTCTRL)			}, //CLKDMN_HDVICP,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ISP_CLKSTCTRL)			}, //CLKDMN_ISP,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_HDVPSS_CLKSTCTRL)			}, //CLKDMN_HDVPSS,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_GFX_CLKSTCTRL)			} //CLKDMN_GFX,
};																	   

BOOL _ClockHwEnableClkDomain(ClockDomain_e clkDomain, BOOL bEnable)
{
	volatile unsigned int *pclkstctrl;


	if (clkDomain >= CLKDMN_COUNT || clkDomain < 0) return FALSE;

	pclkstctrl = (volatile unsigned int*)((UCHAR*)g_pPrcmRegs +
		                          s_rgClkDomainLookupTable[clkDomain].CLKSTCTRL_REG);

    OALMSG(OAL_FUNC,(L"_ClockHwEnableClkDomain: clkDomain=%d, %d, %08X\r\n",
	                    clkDomain, bEnable, pclkstctrl));
	OUTREG32(pclkstctrl, (bEnable)? 0x2 : 0x1); 
	return TRUE;	
}

BOOL ClockEnableClkDomain(ClockDomain_e clkDomain, BOOL bEnable)
{
	if (clkDomain >= CLKDMN_COUNT || clkDomain < 0) return FALSE;

	if (bEnable == TRUE){
		s_rgClkDomainLookupTable[clkDomain].refCount++;
		if (s_rgClkDomainLookupTable[clkDomain].refCount == 1 &&
			s_rgClkDomainLookupTable[clkDomain].ro == FALSE){
			_ClockHwEnableClkDomain(clkDomain, bEnable);
		}
	} else if (s_rgClkDomainLookupTable[clkDomain].refCount > 0){
		s_rgClkDomainLookupTable[clkDomain].refCount--;
		if (s_rgClkDomainLookupTable[clkDomain].refCount == 0 &&
			s_rgClkDomainLookupTable[clkDomain].ro == FALSE){
			_ClockHwEnableClkDomain(clkDomain, bEnable);
		}
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
static BOOL _ClockHwUpdateParentClock( UINT clockId )
// There are several clocks which have multiple parent clocks. The function 
// sets the HW register to connect the clock to a parent accordingly to the
// s_SrcClockTable[clockId].parentClk 
{
	volatile unsigned int *reg_addr;
    BOOL rc = FALSE;
    UINT parentClock;
	UINT reg_val;
	UINT parClkInx;

    OALMSG(OAL_FUNC, (L"+_ClockHwUpdateParentClock(clockId=%d)\r\n", clockId));


    if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;                  // clockId out of range
	if (s_SrcClockTable[clockId].sel_regT != SEL_PRCM &&
		s_SrcClockTable[clockId].sel_regT != SEL_PLLSS) goto cleanUp; // clock doesn't have a mux register

    parentClock = s_SrcClockTable[clockId].parentClk;
	// search for the parentClock in the parents table 
	for (parClkInx=0; parClkInx<s_SrcClockTable[clockId].parents[0]; parClkInx++)
		if (s_SrcClockTable[clockId].parents[parClkInx+1] == parentClock)
			break;

	if (parClkInx >= s_SrcClockTable[clockId].parents[0])
		goto cleanUp; // we haven't found the parent

	// read the sel register
	if (s_SrcClockTable[clockId].sel_regT == SEL_PRCM)
		reg_addr = (volatile unsigned int*)((UCHAR*)g_pPrcmRegs + s_SrcClockTable[clockId].SEL_REG);
	else
		reg_addr = (volatile unsigned int*)((UCHAR*)g_pPllssRegs + s_SrcClockTable[clockId].SEL_REG);

	reg_val = INREG32(reg_addr);
	reg_val &= ~(s_SrcClockTable[clockId].mask << s_SrcClockTable[clockId].shift);
	reg_val |= (parClkInx & s_SrcClockTable[clockId].mask) << s_SrcClockTable[clockId].shift;
	OUTREG32(reg_addr, reg_val);

    rc = TRUE;
            
cleanUp:    
    OALMSG(OAL_FUNC, (L"-_ClockHwUpdateParentClock()=%d\r\n", rc));
    return rc;
}

static UINT _ClockHwGetParentClock( UINT clockId )
// If clock has multiple parents (has sel register) we read the register and return  
// the corresponding parent clockId, otherwise NOCLOCK 
{
	volatile unsigned int *reg_addr;
    UINT parentClock = NOCLOCK;
	UINT reg_val;
//	int  parClkInx;

    if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;                  // clockId out of range
	if (s_SrcClockTable[clockId].sel_regT != SEL_PRCM &&
		s_SrcClockTable[clockId].sel_regT != SEL_PLLSS) goto cleanUp; // clock doesn't have a mux register

	// read the sel register
	if (s_SrcClockTable[clockId].sel_regT == SEL_PRCM)
		reg_addr = (volatile unsigned int*)((UCHAR*)g_pPrcmRegs + s_SrcClockTable[clockId].SEL_REG);
	else
		reg_addr = (volatile unsigned int*)((UCHAR*)g_pPllssRegs + s_SrcClockTable[clockId].SEL_REG);

	reg_val = INREG32(reg_addr);
	reg_val = (reg_val >> s_SrcClockTable[clockId].shift) & s_SrcClockTable[clockId].mask;

	parentClock = s_SrcClockTable[clockId].parents[reg_val+1];

cleanUp:
	return parentClock;
}

//-----------------------------------------------------------------------------
int _ClockHwGetDivisor(UINT clockId)
{
	volatile unsigned int *reg_addr;
	int val = 1;
//	int max_ind; 
	UINT32   reg_val;

	if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;                  // clockId out of range
	// we assume div regs are only in the PRCM module 
	if (s_SrcClockTable[clockId].sel_regT != DIV_PRCM) goto cleanUp; // clock doesn't have a div register
	
	reg_addr = (volatile unsigned int*)((UCHAR*)g_pPrcmRegs + s_SrcClockTable[clockId].SEL_REG);
	reg_val = INREG32(reg_addr);
	reg_val = (reg_val >> s_SrcClockTable[clockId].shift) & s_SrcClockTable[clockId].mask;
		
	val = s_SrcClockTable[clockId].parents[reg_val];
	if (val == NOCLOCK)
		val = 1;

cleanUp:
	return val;	
}



static BOOL _ClockUpdateDpllOutput(int dpllClkId, BOOL bEnable)
{
    int addRef;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+_ClockUpdateDpllOutput"
            L"(srcClkId=%d, bEnable=%d)\r\n", dpllClkId, bEnable));

    if ((UINT)dpllClkId > kDPLL_CLKOUT_COUNT) goto cleanUp;

    addRef = (bEnable != FALSE) ? 1 : -1;

    if ((s_DpllClkOutTable[dpllClkId].refCount == 0 && bEnable == TRUE) ||
        (s_DpllClkOutTable[dpllClkId].refCount == 1 && bEnable == FALSE))
        {
        int dpllId = s_DpllClkOutTable[dpllClkId].dpllDomain;

#if 0 //VA: Don't know yet how to map DPLL and voltage domains 
        // update vdd refCount
        if ((s_DpllTable[dpllId].refCount == 0 && bEnable == TRUE) ||
            (s_DpllTable[dpllId].refCount == 1 && bEnable == FALSE))
            {
            // increment vdd refCount
            int vddId = s_DpllTable[dpllId].vddDomain;            
            s_VddTable[vddId] += addRef;   
            }
#endif
        // incrment dpll refCount
        s_DpllTable[dpllId].refCount += addRef;
        }

    // increment dpllClkSrc refCount
    s_DpllClkOutTable[dpllClkId].refCount += addRef;    

cleanUp:    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-_ClockUpdateDpllOutput()=%d\r\n", TRUE));
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL ClockInitialize()
{
    BOOL rc = TRUE;
    UINT i;
	int  hw_div;
	UINT parentClkId;

    OALMSG(OAL_FUNC, (L"+ClockInitialize()\r\n"));

    // initialize dpll settings with what's current set in hw

	// Maybe we need to read HW registers and figure out what clocks are enabled
	// Based on that we need to update reference counter ???
	
	for (i=0; i<kSOURCE_CLOCK_COUNT; i++){
		parentClkId = _ClockHwGetParentClock(i);
		if (parentClkId == NOCLOCK)
			continue;
		s_SrcClockTable[i].parentClk = parentClkId;
	}
	
	// TODO: When we set parrent clock in the s_SrcClockTable we also have to update clock domain for fclk
	// for the corresponding device in the device table. So the ref_cnt for the correct domain will be updated on DeviceInitialize
	// do it here 

 	// initilize divisors from HW settings
	for (i=0; i<kSOURCE_CLOCK_COUNT; i++){
		hw_div = _ClockHwGetDivisor(i);
		// we update only those clocks that retuns non 1, because there are some clocks which
		// have fixed divisors already set in the table, but function cannot read HW reg and return 1
		// we don't want to corrupt the value in the table for those clocks 
		if (hw_div > 1)
			s_SrcClockTable[i].Divisor = hw_div;
	}

    OALMSG(OAL_FUNC, (L"-ClockInitialize()=%d\r\n", rc));
    return rc;    
}

//-----------------------------------------------------------------------------
// The function walks through the clock tree and updates reference counters 
// TODO: update the clock domain for the device as well (ref counters too)
BOOL ClockUpdateParentClock( int srcClkId, BOOL bEnable )
{
    BOOL rc = FALSE;
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"+ClockUpdateParentClock (srcClkId=%d, bEnable=%d)\r\n",
                srcClkId, bEnable));

    if ((UINT)srcClkId > kSOURCE_CLOCK_COUNT)
		goto cleanUp;
    if ((s_SrcClockTable[srcClkId].refCount == 0 && bEnable == TRUE) ||
        (s_SrcClockTable[srcClkId].refCount == 1 && bEnable == FALSE)){         
        if (s_SrcClockTable[srcClkId].bIsDpllSrcClk == TRUE){
            _ClockUpdateDpllOutput(s_SrcClockTable[srcClkId].parentClk, bEnable);
        } else {
            ClockUpdateParentClock(s_SrcClockTable[srcClkId].parentClk, bEnable);
        }
    }
    s_SrcClockTable[srcClkId].refCount += (bEnable != FALSE) ? 1 : -1;
    
    rc = TRUE;
cleanUp:    
    if (!g_bSingleThreaded)
        OALMSG(OAL_FUNC, (L"-ClockUpdateParentClock()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL PrcmClockSetDivisor( UINT clockId, UINT parentClockId, UINT divisor )
// divisor - actual value of parent_freq/clock_freq
// function finds the corresponding to the divisor register's value and set it.
// The divisor is stored in the  s_SrcClockTable 
{
	volatile unsigned int *reg_addr;
	int		val = 1;
	UINT32  reg_val;
    BOOL rc = FALSE;
    
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDivisor (clockId=%d, parentClockId=%d, divisor=%d)\r\n", 
        clockId, parentClockId, divisor) );

    Lock(Mutex_Clock);

	if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;                  // clockId out of range
	// we assume div regs are only in the PRCM module 
	if (s_SrcClockTable[clockId].sel_regT != DIV_PRCM) goto cleanUp; // clock doesn't have a div register

	// lookup for divisor index
	for (val=0; s_SrcClockTable[clockId].parents[val] != NOCLOCK; val++){ 
		if (s_SrcClockTable[clockId].parents[val] == divisor)
			break;
	}

	if (s_SrcClockTable[clockId].parents[val] == NOCLOCK) goto cleanUp;

	// update register
	reg_addr = (volatile unsigned int*)((UCHAR*)g_pPrcmRegs + s_SrcClockTable[clockId].SEL_REG);
	reg_val = INREG32(reg_addr);
	reg_val = (reg_val >> s_SrcClockTable[clockId].shift) & s_SrcClockTable[clockId].mask | val;
	OUTREG32(reg_addr, reg_val);

	s_SrcClockTable[clockId].Divisor = divisor;
	rc = TRUE;
	
cleanUp:	
    Unlock(Mutex_Clock);

    OALMSG(OAL_FUNC, (L"-PrcmClockSetDivisor()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL PrcmClockSetParent(UINT clockId, UINT newParentClockId )
{    
    BOOL rc = FALSE;
    UINT oldParentClockId;
	UINT parClkInx;

    OALMSG(OAL_FUNC, (L"+PrcmClockSetParent"
        L"(clockId=%d, newParentClockId=%d)\r\n", clockId, newParentClockId));

    if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;

    Lock(Mutex_Clock);
    oldParentClockId = s_SrcClockTable[clockId].parentClk;

	// if the newParentClockId isn't in the parents for the clockId goto cleanUpL	
	if (s_SrcClockTable[clockId].sel_regT != SEL_PRCM &&
		s_SrcClockTable[clockId].sel_regT != SEL_PLLSS) goto cleanUpL; // clock doesn't have a mux register

	// search for the parentClock in the parents table 
	for (parClkInx=0; parClkInx < s_SrcClockTable[clockId].parents[0]; parClkInx++)
		if (s_SrcClockTable[clockId].parents[parClkInx+1] == newParentClockId)
			break;

	if (parClkInx >= s_SrcClockTable[clockId].parents[0])
		goto cleanUpL; // we haven't found the parent

    // check if clock is enabled.  If so then release old src clocks
    // and enable new ones
    if (s_SrcClockTable[clockId].refCount > 0){
        ClockUpdateParentClock(newParentClockId, TRUE);
        ClockUpdateParentClock(oldParentClockId, FALSE);
    }

    // update state tree
    s_SrcClockTable[clockId].parentClk = newParentClockId;
    rc = _ClockHwUpdateParentClock(clockId);

    // release sync handle    
cleanUpL:    
    Unlock(Mutex_Clock);
            
cleanUp:    
    OALMSG(OAL_FUNC, (L"-PrcmClockSetParent()=%d\r\n", TRUE));
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL PrcmClockGetParentClockRefcount(UINT clockId, UINT nLevel, LONG *pRefCount )
{
    BOOL rc = FALSE;
    OALMSG(OAL_FUNC, (L"+PrcmClockGetParentClockRefcount (clockId=%d, nLevel=%d)\r\n", clockId, nLevel));

    switch (nLevel){
        case 1:
            if (clockId < kSOURCE_CLOCK_COUNT){
                *pRefCount = s_SrcClockTable[clockId].refCount;
                rc = TRUE;
            }
            break;

        case 2:
            if (clockId < kDPLL_CLKOUT_COUNT){
                *pRefCount = s_DpllClkOutTable[clockId].refCount;
                rc = TRUE;
            }
            break;

        case 3:
            if (clockId < kDPLL_COUNT){
                *pRefCount = s_DpllTable[clockId].refCount;
                rc = TRUE;
            }
            break;

        case 4:
            if (clockId < kVDD_COUNT){
                *pRefCount = s_VddTable[clockId];
                rc = TRUE;
            }
            break;
        }

    OALMSG(OAL_FUNC, (L"+PrcmClockGetParentClockRefcount()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL PrcmClockGetParentClockInfo(UINT clockId, UINT nLevel, SourceClockInfo_t *pInfo)
{
    BOOL rc = FALSE;
    UINT parentClock;
    OALMSG(OAL_FUNC, (L"+PrcmClockGetParentClockInfo"
        L"(clockId=%d, nLevel=%d, pInfo=0x%08X)\r\n", clockId, nLevel, pInfo));

    // get parent clock
    switch (nLevel) {
        case 1:            
            if (clockId >= kSOURCE_CLOCK_COUNT)
				goto cleanUp;
            parentClock = s_SrcClockTable[clockId].parentClk;
            if (s_SrcClockTable[clockId].bIsDpllSrcClk)
				++nLevel;                    
            break;

        case 2:
            if (clockId >= kDPLL_CLKOUT_COUNT)
				goto cleanUp;
            parentClock = s_DpllClkOutTable[clockId].dpllDomain;
            ++nLevel;                    
            break;

        case 3:
            if (clockId >= kDPLL_COUNT)
                goto cleanUp;
            parentClock = s_DpllTable[clockId].vddDomain;
            ++nLevel;                    
            break;

        default:
            goto cleanUp;
        }


    // get parent information
    pInfo->nLevel = nLevel;
    pInfo->clockId = parentClock;
    rc = PrcmClockGetParentClockRefcount(parentClock, nLevel, &pInfo->refCount);

cleanUp:    
    OALMSG(OAL_FUNC, (L"+PrcmClockGetParentClockInfo()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
BOOL PrcmClockSetDpllState(IOCTL_PRCM_CLOCK_SET_DPLLSTATE_IN *pInfo )
{
    OALMSG(1 /*OAL_FUNC*/, (L"+PrcmClockSetDpllState(pInfo=0x%08X)\r\n", pInfo));
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL PrcmClockSetDpllFrequency(UINT dpllId, UINT m,
    UINT n, UINT freqSel, UINT outputDivisor  )
{
    OALMSG(1/*OAL_FUNC*/, (L"+PrcmClockSetDpllFrequency"
        L"(dpllId=%d, m=%d, n=%d, freqSel=%d)\r\n", dpllId, m, n, freqSel  ));

    return TRUE;
}    

//-----------------------------------------------------------------------------
BOOL PrcmClockSetDpllAutoIdleState( UINT dpllId, UINT dpllAutoidleState )
{
    OALMSG(1/*OAL_FUNC*/, (L"+PrcmClockSetDpllAutoIdleState"
        L"(dpllId=%d, dpllAutoidleState=0x%08X)\r\n", 
        dpllId, dpllAutoidleState));

    return TRUE;
}

//------------------------------------------------------------------------------
BOOL PrcmClockSetExternalClockRequestMode( UINT extClkReqMode )
// TODO: What is that for?????
{    
    Lock(Mutex_Clock);
    OALMSG(1/*OAL_FUNC*/, (L"+-PrcmClockSetExternalClockRequestMode"
        L"(extClkReqMode=%d)\r\n", extClkReqMode));
    Unlock(Mutex_Clock);
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL PrcmClockSetSystemClockSetupTime( USHORT  setupTime )
{    
    UNREFERENCED_PARAMETER(setupTime);

    // acquire sync handle   
 	// Lock(Mutex_Clock);

    // release sync handle    
    // Unlock(Mutex_Clock);
    
    return TRUE;
}

//-----------------------------------------------------------------------------
float PrcmClockGetSystemClockFrequency()
{
    return 20.0f * 1000000.0f;
}

#if 0
//-----------------------------------------------------------------------------
UINT PrcmClockGetDpllState( UINT dpllId )
{
    UINT val = 0; 
    OALMSG(OAL_FUNC, (L"+PrcmClockGetDpllState(dpllId=%d)\r\n", dpllId));
    return val;
}

//-----------------------------------------------------------------------------
VOID PrcmClockRestoreDpllState( UINT dpll ){}
void DumpPrcmRegs(){}
#endif

extern  UINT32 g_oalIoCtlClockSpeed;

UINT32 PrcmClockGetClockRate(OMAP_CLOCKID clock_id)
{
    float freq=0;
		
    OALMSG(1, (L"+PrcmClockGetClockRate(clock_id=%d)\r\n", clock_id));

    switch(clock_id)
    {
        case MPU_CLK:                
            freq = (float) g_oalIoCtlClockSpeed;
        break;
    }

    return (UINT32)freq;
}



