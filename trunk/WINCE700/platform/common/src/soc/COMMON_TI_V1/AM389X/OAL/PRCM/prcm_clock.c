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
#include "am389x_oal_prcm.h"
//#include "omap_prof.h"
#include "am389x.h"
//#include "omap_led.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"

// initialize voltage domain ref count
VddRefCountTable s_VddTable = {
    0,    // kVDD1V_AVS
    0,    // kVDD1V0
    0,    // kVDD1V8
    0,    // kVDD3V3
    0,    // kVDD1V5
    0,    // kVDD0V9
};


//-----------------------------------------------------------------------------
// initialize dpll domain ref count
DpllMap s_DpllTable = {
	{ kVDD_EXT,   0, NULL }, //	kSECURE_32K
	{ kVDD_EXT,   0, NULL }, // kSYS_32K
	{ kVDD_EXT,   0, NULL }, // kTCLKIN
	{ kVDD_EXT,   0, NULL }, // kSYSCLKIN
	{ kVDD_EXT,   0, NULL }, // kMAIN_PLL
	{ kVDD_EXT,   0, NULL }, // kDDR_PLL
	{ kVDD_EXT,   0, NULL }, // kVIDEO_PLL
	{ kVDD_EXT,   0, NULL }, // kAUDIO_PLL
};

//-----------------------------------------------------------------------------
// initialize dpll clkout ref count
//

// FIXME: correct freq to the actual values
DpllClkOutMap s_DpllClkOutTable = {
    { kSECURE_32K, 0,     32000.0 }, 	// kSECURE_32K_CLK
    { kSYS_32K,    0,     32000.0 }, 	// kSYS_32K_CLK
    { kTCLKIN,     0,  27000000.0 },	// kTCLKIN_CLK
    { kSYSCLKIN,   0, 0.0      },	    // kSYSCLKIN_CLK

    { kMAIN_PLL,   0,1000000000.0 },	// kMAIN_PLL_CLK1_CLK
    { kMAIN_PLL,   0,1000000000.0 },	// kMAIN_PLL_CLK2_CLK
    { kMAIN_PLL,   0, 600000000.0 },	// kMAIN_PLL_CLK3_CLK
    { kMAIN_PLL,   0, 500000000.0 },	// kMAIN_PLL_CLK4_CLK
    { kMAIN_PLL,   0, 125000000.0 },	// kMAIN_PLL_CLK5_CLK

    { kDDR_PLL,    0, 800000000.0 },	// kDDR_PLL_CLK1_CLK
    { kDDR_PLL,    0,  48000000.0 },	// kDDR_PLL_CLK2_CLK
    { kDDR_PLL,    0, 400000000.0 },    // kDDR_PLL_CLK3_CLK
     
    { kVIDEO_PLL,  0, 0.0      },		// kVIDEO_PLL_CLK1_CLK
    { kVIDEO_PLL,  0, 0.0      },		// kVIDEO_PLL_CLK2_CLK
    { kVIDEO_PLL,  0, 0.0      },		// kVIDEO_PLL_CLK3_CLK

    { kAUDIO_PLL,  0, 0.0      },		// kAUDIO_PLL_CLK1_CLK
    { kAUDIO_PLL,  0, 0.0      },		// kAUDIO_PLL_CLK2_CLK
    { kAUDIO_PLL,  0, 0.0      },		// kAUDIO_PLL_CLK3_CLK
    { kAUDIO_PLL,  0, 0.0      },		// kAUDIO_PLL_CLK4_CLK
    { kAUDIO_PLL,  0, 0.0      }		// kAUDIO_PLL_CLK5_CLK
// VA - What is kDPLL_CLKOUT_CLK ? What clk domain does it belong to?
//	,{ kUNKNOWN,   0	}		// kDPLL_CLKOUT_CLK External Observation Clock

};

SrcClockMap s_SrcClockTable = {
/* Order of the table elements MUST match the SourceClock_e list */ 	
/*   Parent Clock      RefCount  isDPLL  	  Divisor   clock    */
    {kMAIN_PLL_CLK1_CLK, 	0,  TRUE,   1, kSYS_CLK1_CLK 	}, // (DIV 0x300[2:0] 1,2,...,8 )
    {kSYS_CLK1_CLK,		 	0,  FALSE,  1, kGEM_I_CLK    	}, // (EN  0x420[1])
    {kMAIN_PLL_CLK2_CLK,	0,  TRUE,   1, kSYS_CLK2_CLK	}, // (DIV 0x304[2:0] 1,2,...,8 )
    {kMAIN_PLL_CLK2_CLK,	0,  TRUE,   1, kSYS_CLK23_CLK	}, // (DIV 0x3B0[2:0] 1,2,...,8 )
	{kSYS_CLK23_CLK,		0,  FALSE,  1, kSGX_CLK			}, // (EN  0x920[1])
    {kMAIN_PLL_CLK3_CLK,	0,  TRUE,   1, kSYS_CLK3_CLK		}, // (DIV 0x308[2:0] 1,2,3 )
	{kSYS_CLK3_CLK,			0,  FALSE,  1, kIVAHD0_CLK		}, // (EN  0x620[1])
	{kSYS_CLK3_CLK,			0,  FALSE,  1, kIVAHD0_SL2_CLK	}, // (EN  0x624[1])
	{kSYS_CLK3_CLK,			0,  FALSE,  1, kIVAHD1_CLK		}, // (EN  0x720[1])
	{kSYS_CLK3_CLK,			0,  FALSE,  1, kIVAHD1_SL2_CLK	}, // (EN  0x724[1])
	{kSYS_CLK3_CLK,			0,  FALSE,  1, kIVAHD2_CLK		}, // (EN  0x820[1])
	{kSYS_CLK3_CLK,			0,  FALSE,  1, kIVAHD2_SL2_CLK	}, // (EN  0x824[1])
    {kMAIN_PLL_CLK4_CLK,	0,  TRUE,   1, kSYS_CLK4_CLK		}, // (DIV 0x30C[0] 1,2 )
	{kSYS_CLK4_CLK,			0,  FALSE,  1, kMMU_I_CLK		}, // (EN  0x159C[1])
	{kSYS_CLK4_CLK,			0,  FALSE,  1, kDUCATI_UCACHE_I_CLK}, // ???????
	{kSYS_CLK4_CLK,			0,  FALSE,  1, kTPCC_I_CLK		}, // (EN  0x15F4[1])
	{kSYS_CLK4_CLK,			0,  FALSE,  1, kTPTC0_I_CLK		}, // (EN  0x15F8[1])
	{kSYS_CLK4_CLK,			0,  FALSE,  1, kTPTC1_I_CLK		}, // (EN  0x15FC[1])
	{kSYS_CLK4_CLK,			0,  FALSE,  1, kTPTC2_I_CLK		}, // (EN  0x1600[1])
	{kSYS_CLK4_CLK,			0,  FALSE,  1, kTPTC3_I_CLK		}, // (EN  0x1604[1])
    {kSYS_CLK4_CLK,			0,  FALSE,  2, kSYS_CLK6_CLK		}, // R/O(2) (DIV 0x314[0] 2,4 )
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kMMU_CFG_I_CLK	}, // (EN  0x15A8[1])
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kMAILBOX_I_CLK	}, // (EN  0x1594[1])
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kSPINBOX_I_CLK	}, // (EN  0x1598[1])
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kGEM_VBUSP_I_CLK	}, // 
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kMCSPI1_I_CLK	}, // 
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kUSBOTG_I_CLK	}, // (EN  0x0558[1])
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kGPMC_I_CLK		}, // 
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kGPIO0_CLK		}, // (EN  0x155C[1])
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kGPIO1_CLK		}, // (EN  0x1560[1])
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kSR0_CLK		    }, // (EN  0x1608[1])
	{kSYS_CLK6_CLK,			0,  FALSE,  1, kSR1_CLK		    }, // (EN  0x160C[1])
	{kSYS_CLK4_CLK,			0,  FALSE,  1, kSYS_CLK5_CLK	}, // R/O (1) (DIV 0x310[0] 1,2 )
	{kSYS_CLK5_CLK,			0,  FALSE,  1, kPCIE_CLK		}, // (EN  0x0578[1])
	{kSYS_CLK5_CLK,			0,  FALSE,  1, kSATA_I_CLK		}, // (EN  0x0560[1])
	{kSYS_CLK5_CLK,			0,  FALSE,  1, kEMAC0_I_CLK	}, // (EN  0x15D4[1])
	{kSYS_CLK5_CLK,			0,  FALSE,  1, kEMAC1_I_CLK	}, // (EN  0x15D8[1])
	{kSYS_CLK5_CLK,			0,  FALSE,  1, kDUCATI_I_CLK	}, // (EN  0x0574[1])
	{kSYS_CLK5_CLK,			0,  FALSE,  1,	kGEM_TRC_F_CLK	}, // 
    {kDDR_PLL_CLK3_CLK,		0,  TRUE,   2,	kSYS_CLK8_CLK	}, // (DIV 0x324[2:0] 1,2,3 )
    {kDDR_PLL_CLK2_CLK,		0,  TRUE,   2,	kSYS_CLK10_CLK	}, // (DIV 0x324[2:0] 1,2,3 )
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kUART1_F_CLK	}, //(EN  0x1550[1])
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kUART2_F_CLK	}, //(EN  0x1554[1])
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kUART3_F_CLK	}, //(EN  0x1558[1])
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kSPI_F_CLK		}, //(EN  0x1590[1])
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kI2C0_F_CLK		}, //(EN  0x1564[1])
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kI2C1_F_CLK		}, //(EN  0x1568[1])
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kSDIO_F_CLK		}, //(EN  0x15B0[1]) was MMCHS1
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kSD0_F_CLK		}, //(EN  0x15BC[1])
	{kSYS_CLK10_CLK,		0,  FALSE,  1,	kSD1_F_CLK		}, //(EN  0x15C0[1])
    {kMAIN_PLL_CLK5_CLK,	0,  TRUE,   1,	kSYS_CLK24_CLK	}, // (DIV 0x3B4[2:0] 1,2,3 )
    {kAUDIO_PLL_CLK1_CLK,	0,  TRUE,   1,	kAUDIO_PLL_A_CLK}, // (DIV 0x35C[2:0] 1,2,3 )
	{kSYS_32K_CLK  /*kAUDIO_PLL_A_CLK*/,
	                        0,  TRUE,   1,	kSYS_CLK18_CLK	}, // SEL 0x378[0] 
    {kAUDIO_PLL_CLK3_CLK,	0,  TRUE,   1,	kSYS_CLK20_CLK	}, // (DIV 0x350[2:0] 1,2,3 )
    {kAUDIO_PLL_CLK4_CLK,	0,  TRUE,   1,	kSYS_CLK21_CLK	}, // (DIV 0x354[2:0] 1,2,3 )
    {kAUDIO_PLL_CLK5_CLK,	0,  TRUE,   1,	kSYS_CLK22_CLK	}, // (DIV 0x358[2:0] 1,2,3 )
	{kTCLKIN_CLK /*kSYS_CLK18_CLK, kSYSCLKIN_CLK*/,
							0,  TRUE,   1,	kDMT1_F_CLK		}, // SEL 0x390[1:0] EN  0x1570[1]
	{kTCLKIN_CLK /*kSYS_CLK18_CLK, kSYSCLKIN_CLK*/,
 							0,  TRUE,   1, 	kDMT2_F_CLK		}, // SEL 0x394[1:0] EN  0x1574[1]
	{kTCLKIN_CLK /*kSYS_CLK18_CLK, kSYSCLKIN_CLK*/,
							0,  TRUE,   1, 	kDMT3_F_CLK		}, // SEL 0x398[1:0] EN  0x1578[1]
	{kTCLKIN_CLK /*kSYS_CLK18_CLK, kSYSCLKIN_CLK*/,
							0,  TRUE,   1, 	kDMT4_F_CLK		}, // SEL 0x39C[1:0] EN  0x157C[1]
	{kTCLKIN_CLK /*kSYS_CLK18_CLK, kSYSCLKIN_CLK*/,
							0,  TRUE,   1, 	kDMT5_F_CLK		}, // SEL 0x3A0[1:0]  EN  0x1580[1]
	{kTCLKIN_CLK /*kSYS_CLK18_CLK, kSYSCLKIN_CLK*/,
							0,  TRUE,   1, 	kDMT6_F_CLK		}, // SEL 0x3A4[1:0]  EN  0x1584[1]
	{kTCLKIN_CLK /*kSYS_CLK18_CLK, kSYSCLKIN_CLK*/,
							0,  TRUE,   1, 	kDMT7_F_CLK		}, // SEL 0x3A8[1:0] EN  0x1588[1]
	{kSYS_CLK18_CLK,		0,  FALSE,  1, 	kMMCHSDB1		}, //
	{kSYS_CLK20_CLK /*kSYS_CLK21_CLK, kSYS_CLK22_CLK*/,
 							0,	FALSE,  1, 	kMCASP0_F_CLK	},// SEL 0x37C[1:0]  0x1540[1]
	{kSYS_CLK20_CLK /*kSYS_CLK21_CLK, kSYS_CLK22_CLK*/,
 							0,	FALSE,  1, 	kMCASP1_F_CLK	},// SEL 0x380[1:0] EN  0x1544[1]
	{kSYS_CLK20_CLK /*kSYS_CLK21_CLK, kSYS_CLK22_CLK*/,
 							0,	FALSE,  1,	kMCASP2_F_CLK} // SEL 0x384[1:0]  EN  0x1548[1]
};

ClockDomainLookupEntry s_rgClkDomainLookupTable[] = {
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON_L3_SLOW_CLKSTCTRL)}, //CLKDMN_ALWON_L3_SLOW,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ETHERNET_CLKSTCTRL)     }, //CLKDMN_ETHERNET,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON_L3_FAST_CLKSTCTRL)}, //CLKDMN_L3_FAST_ALWON,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_L3_MED_CLKSTCTRL) }, //CLKDMN_L3_MED_ALWON,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_MMUCFG_CLKSTCTRL)       }, //CLKDMN_MMU_CFG,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_MMU_CLKSTCTRL)          }, //CLKDMN_MMU,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_MPU_CLKSTCTRL)    }, //CLKDMN_MPU,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON_OCMC_0_CLKSTCTRL) }, //CLKDMN_OCMC0,
	{ PWR_ALWAYSON, 0, FALSE, PRCM_OFS(CM_ALWON_OCMC_1_CLKSTCTRL) }, //CLKDMN_OCMC1,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_RTC_CLKSTCTRL)    }, //CLKDMN_RTC,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_SYSCLK4_CLKSTCTRL)}, //CLKDMN_SYSCLK4,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_SYSCLK5_CLKSTCTRL)}, //CLKDMN_SYSCLK5,
	{ PWR_ALWAYSON, 0, TRUE,  PRCM_OFS(CM_ALWON_SYSCLK6_CLKSTCTRL)}, //CLKDMN_SYSCLK6,
// DEFAULT
	{ PWR_DEFAULT,  0, FALSE, PRCM_OFS(CM_DEFAULT_DUCATI_CLKSTCTRL) }, //CLKDMN_DUCATI_INTR,
	{ PWR_DEFAULT,  0, FALSE, PRCM_OFS(CM_DEFAULT_L3_FAST_CLKSTCTRL)}, //CLKDMN_L3_FAST_DEFAULT,
	{ PWR_DEFAULT,  0, FALSE, PRCM_OFS(CM_DEFAULT_L3_MED_CLKSTCTRL) }, //CLKDMN_L3_MED_DEFAULT,
	{ PWR_DEFAULT,  0, FALSE, PRCM_OFS(CM_DEFAULT_PCI_CLKSTCTRL)    }, //CLKDMN_PCI,
	{ PWR_DEFAULT,  0, FALSE, PRCM_OFS(CM_DEFAULT_TPPSS_CLKSTCTRL)  }, //CLKDMN_TPPSS,
	{ PWR_DEFAULT,  0, FALSE, PRCM_OFS(CM_DEFAULT_L3_SLOW_CLKSTCTRL)}, //CLKDMN_L3_SLOW_DEFAULT,
// ACTIVE 
	{ PWR_ACTIVE,   0, FALSE, PRCM_OFS(CM_GEM_CLKSTCTRL)          }, //CLKDMN_GEM,
	{ PWR_ACTIVE,   0, FALSE, PRCM_OFS(CM_HDMI_CLKSTCTRL)         }, //CLKDMN_HDMI_OCP,
	{ PWR_ACTIVE,   0, FALSE, PRCM_OFS(CM_HDDSS_CLKSTCTRL)        }, //CLKDMN_HDDSS_L3,
// IVA & SGX
	{ PWR_IVA0,     0, FALSE, PRCM_OFS(CM_IVAHD0_CLKSTCTRL)       }, //CLKDMN_IVA0,
	{ PWR_IVA1,     0, FALSE, PRCM_OFS(CM_IVAHD1_CLKSTCTRL)       }, //CLKDMN_IVA1,
	{ PWR_IVA2,     0, FALSE, PRCM_OFS(CM_IVAHD2_CLKSTCTRL)       }, //CLKDMN_IVA2,
	{ PWR_SGX,      0, FALSE, PRCM_OFS(CM_SGX_CLKSTCTRL)          }  //CLKDMN_SGX,
//
};

BOOL _ClockHwEnableClkDomain(ClockDomain_e clkDomain, BOOL bEnable)
{
	volatile unsigned int *pclkstctrl;
	if (clkDomain >= CLKDMN_COUNT || clkDomain < 0) return FALSE;

	pclkstctrl = (volatile unsigned int*)((UCHAR*)g_pPrcmRegs +
		                          s_rgClkDomainLookupTable[clkDomain].CLKSTCTRL_REG);

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
    BOOL rc = FALSE;
    UINT parentClock;
    OALMSG(OAL_FUNC, (L"+_ClockHwUpdateParentClock(clockId=%d)\r\n", clockId));

    if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;

    parentClock = s_SrcClockTable[clockId].parentClk;

    switch (clockId) {
        case kSYS_CLK18_CLK:
            switch(parentClock) {
                case kSYS_32K_CLK:     OUTREG32(&g_pPrcmRegs->CM_SYSCLK18_CLKSEL, 1); break;
                case kAUDIO_PLL_A_CLK: OUTREG32(&g_pPrcmRegs->CM_SYSCLK18_CLKSEL, 0); break;
                default: goto cleanUp;
            }
            break;

        case kDMT1_F_CLK:
            switch(parentClock){
                case kTCLKIN_CLK:    OUTREG32(&g_pPrcmRegs->CM_TIMER1_CLKSEL, 0); break;
                case kSYS_CLK18_CLK: OUTREG32(&g_pPrcmRegs->CM_TIMER1_CLKSEL, 1); break;
                case kSYSCLKIN_CLK:  OUTREG32(&g_pPrcmRegs->CM_TIMER1_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kDMT2_F_CLK:
            switch(parentClock){
                case kTCLKIN_CLK:    OUTREG32(&g_pPrcmRegs->CM_TIMER2_CLKSEL, 0); break;
                case kSYS_CLK18_CLK: OUTREG32(&g_pPrcmRegs->CM_TIMER2_CLKSEL, 1); break;
                case kSYSCLKIN_CLK:  OUTREG32(&g_pPrcmRegs->CM_TIMER2_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kDMT3_F_CLK:
            switch(parentClock){
                case kTCLKIN_CLK:    OUTREG32(&g_pPrcmRegs->CM_TIMER3_CLKSEL, 0); break;
                case kSYS_CLK18_CLK: OUTREG32(&g_pPrcmRegs->CM_TIMER3_CLKSEL, 1); break;
                case kSYSCLKIN_CLK:  OUTREG32(&g_pPrcmRegs->CM_TIMER3_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kDMT4_F_CLK:
            switch(parentClock){
                case kTCLKIN_CLK:    OUTREG32(&g_pPrcmRegs->CM_TIMER4_CLKSEL, 0); break;
                case kSYS_CLK18_CLK: OUTREG32(&g_pPrcmRegs->CM_TIMER4_CLKSEL, 1); break;
                case kSYSCLKIN_CLK:  OUTREG32(&g_pPrcmRegs->CM_TIMER4_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kDMT5_F_CLK:
            switch(parentClock){
                case kTCLKIN_CLK:    OUTREG32(&g_pPrcmRegs->CM_TIMER5_CLKSEL, 0); break;
                case kSYS_CLK18_CLK: OUTREG32(&g_pPrcmRegs->CM_TIMER5_CLKSEL, 1); break;
                case kSYSCLKIN_CLK:  OUTREG32(&g_pPrcmRegs->CM_TIMER5_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kDMT6_F_CLK:
            switch(parentClock){
                case kTCLKIN_CLK:    OUTREG32(&g_pPrcmRegs->CM_TIMER6_CLKSEL, 0); break;
                case kSYS_CLK18_CLK: OUTREG32(&g_pPrcmRegs->CM_TIMER6_CLKSEL, 1); break;
                case kSYSCLKIN_CLK:  OUTREG32(&g_pPrcmRegs->CM_TIMER6_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kDMT7_F_CLK:
            switch(parentClock){
                case kTCLKIN_CLK:    OUTREG32(&g_pPrcmRegs->CM_TIMER7_CLKSEL, 0); break;
                case kSYS_CLK18_CLK: OUTREG32(&g_pPrcmRegs->CM_TIMER7_CLKSEL, 1); break;
                case kSYSCLKIN_CLK:  OUTREG32(&g_pPrcmRegs->CM_TIMER7_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kMCASP0_F_CLK:
            switch(parentClock){
                case kSYS_CLK20_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP0_CLKSEL, 0); break;
                case kSYS_CLK21_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP0_CLKSEL, 1); break;
                case kSYS_CLK22_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP0_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kMCASP1_F_CLK:
            switch(parentClock){
                case kSYS_CLK20_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP1_CLKSEL, 0); break;
                case kSYS_CLK21_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP1_CLKSEL, 1); break;
                case kSYS_CLK22_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP1_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

        case kMCASP2_F_CLK:
            switch(parentClock){
                case kSYS_CLK20_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP2_CLKSEL, 0); break;
                case kSYS_CLK21_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP2_CLKSEL, 1); break;
                case kSYS_CLK22_CLK: OUTREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP2_CLKSEL, 2); break;
                default: goto cleanUp;
            }
            break;

		default:
			goto cleanUp;

	}

    rc = TRUE;
            
cleanUp:    
    OALMSG(OAL_FUNC, (L"-_ClockHwUpdateParentClock()=%d\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
int _ClockHwGetDivisor(UINT clockId)
{
	int val = 1;
    switch (clockId) {            
        case kSYS_CLK1_CLK:  val = INREG32(&g_pPrcmRegs->CM_SYSCLK1_CLKSEL)  + 1; break;
        case kSYS_CLK23_CLK: val = INREG32(&g_pPrcmRegs->CM_SYSCLK23_CLKSEL) + 1; break;
        case kSYS_CLK3_CLK:  val = INREG32(&g_pPrcmRegs->CM_SYSCLK3_CLKSEL)  + 1; break;
        case kSYS_CLK4_CLK:  val = INREG32(&g_pPrcmRegs->CM_SYSCLK4_CLKSEL)  + 1; break;
        case kSYS_CLK10_CLK: val = INREG32(&g_pPrcmRegs->CM_SYSCLK10_CLKSEL) + 1; break;
        case kSYS_CLK24_CLK: val = INREG32(&g_pPrcmRegs->CM_SYSCLK24_CLKSEL) + 1; break;
        case kAUDIO_PLL_A_CLK: val = INREG32(&g_pPrcmRegs->CM_APA_CLKSEL)    + 1; break;
        case kSYS_CLK20_CLK: val = INREG32(&g_pPrcmRegs->CM_SYSCLK20_CLKSEL) + 1; break;
        case kSYS_CLK21_CLK: val = INREG32(&g_pPrcmRegs->CM_SYSCLK21_CLKSEL) + 1; break;
        case kSYS_CLK22_CLK: val = INREG32(&g_pPrcmRegs->CM_SYSCLK22_CLKSEL) + 1; break;
		default:
			val = 1;
	} // switch

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

	static int dmt_src[] = {kTCLKIN, kSYS_CLK18_CLK, kSYSCLKIN_CLK, -1};
	static int mcasp_src[] = {kSYS_CLK20_CLK, kSYS_CLK21_CLK, kSYS_CLK22_CLK, -1};

    OALMSG(OAL_FUNC, (L"+ClockInitialize()\r\n"));

    // initialize dpll settings with what's current set in hw

	// Maybe we need to read HW registers and figure out what clocks are enabled
	// Based on that we need to update reference counter ???
	
	
	
    // Save  PER CLKSEL configuration    
    s_SrcClockTable[kSYS_CLK18_CLK].parentClk =
    	(INREG32(&g_pPrcmRegs->CM_SYSCLK18_CLKSEL) & 1) ? kSYS_32K_CLK : kAUDIO_PLL_A_CLK;
	
    s_SrcClockTable[kDMT1_F_CLK].parentClk = dmt_src[INREG32(&g_pPrcmRegs->CM_TIMER1_CLKSEL) & 3];
    s_SrcClockTable[kDMT2_F_CLK].parentClk = dmt_src[INREG32(&g_pPrcmRegs->CM_TIMER2_CLKSEL) & 3];
    s_SrcClockTable[kDMT3_F_CLK].parentClk = dmt_src[INREG32(&g_pPrcmRegs->CM_TIMER3_CLKSEL) & 3];
    s_SrcClockTable[kDMT4_F_CLK].parentClk = dmt_src[INREG32(&g_pPrcmRegs->CM_TIMER4_CLKSEL) & 3];
    s_SrcClockTable[kDMT5_F_CLK].parentClk = dmt_src[INREG32(&g_pPrcmRegs->CM_TIMER5_CLKSEL) & 3];
    s_SrcClockTable[kDMT6_F_CLK].parentClk = dmt_src[INREG32(&g_pPrcmRegs->CM_TIMER6_CLKSEL) & 3];
    s_SrcClockTable[kDMT7_F_CLK].parentClk = dmt_src[INREG32(&g_pPrcmRegs->CM_TIMER7_CLKSEL) & 3];

    s_SrcClockTable[kMCASP0_F_CLK].parentClk = mcasp_src[INREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP0_CLKSEL) & 3];
    s_SrcClockTable[kMCASP1_F_CLK].parentClk = mcasp_src[INREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP1_CLKSEL) & 3];
    s_SrcClockTable[kMCASP2_F_CLK].parentClk = mcasp_src[INREG32(&g_pPrcmRegs->CM_AUDIOCLK_MCASP2_CLKSEL) & 3];

	// TODO: When we set parrent clock in the s_SrcClockTable we also have to update clock domain for fclk
	// for the corresponding device in the device table. So the ref_cnt for the correct domain will be updated on DeviceInitialize
	// do it here 


 	// initilize divizores from HW settings
 	for (i=0; i<kSOURCE_CLOCK_COUNT; i++)
		s_SrcClockTable[i].Divisor = _ClockHwGetDivisor(i);


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
    int reg_value = 0;
    BOOL rc = FALSE;
    
    OALMSG(OAL_FUNC, (L"+PrcmClockSetDivisor (clockId=%d, parentClockId=%d, divisor=%d)\r\n", 
        clockId, parentClockId, divisor) );

    // get sync handle
    Lock(Mutex_Clock);
    
    switch (clockId) {            
        case kSYS_CLK1_CLK:
            if (divisor < 1 || divisor > 8) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK1_CLKSEL, reg_value);
            break;

        case kSYS_CLK23_CLK:
            if (divisor < 1 || divisor > 8) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK23_CLKSEL, reg_value);
            break;

		case kSYS_CLK3_CLK:
			if (divisor < 1 || divisor > 3) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK3_CLKSEL, reg_value);
			break;
	
		case kSYS_CLK4_CLK:
			if (divisor < 1 || divisor > 2) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK4_CLKSEL, reg_value);
			break;
	
		case kSYS_CLK10_CLK:
			if (divisor < 1 || divisor > 8) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK10_CLKSEL, reg_value);
			break;

		case kSYS_CLK24_CLK:
			if (divisor < 1 || divisor > 8) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK24_CLKSEL, reg_value);
			break;

		case kAUDIO_PLL_A_CLK:
			if (divisor < 1 || divisor > 3) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_APA_CLKSEL, reg_value);
			break;
	
		case kSYS_CLK20_CLK:
			if (divisor < 1 || divisor > 8) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK20_CLKSEL, reg_value);
			break;
	
		case kSYS_CLK21_CLK:
			if (divisor < 1 || divisor > 8) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK21_CLKSEL, reg_value);
			break;
	
		case kSYS_CLK22_CLK:
			if (divisor < 1 || divisor > 8) goto cleanUp;
			reg_value = divisor-1;
			OUTREG32(&g_pPrcmRegs->CM_SYSCLK22_CLKSEL, reg_value);
			break;

		default:
			goto cleanUp;
	} // switch

	s_SrcClockTable[clockId].Divisor = divisor;
	rc = TRUE;
	
    // release sync handle    
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
    OALMSG(OAL_FUNC, (L"+PrcmClockSetParent"
        L"(clockId=%d, newParentClockId=%d)\r\n", clockId, newParentClockId));

    if (clockId > kSOURCE_CLOCK_COUNT) goto cleanUp;

    Lock(Mutex_Clock);
    oldParentClockId = s_SrcClockTable[clockId].parentClk;
    switch (clockId){
        case kSYS_CLK18_CLK:
            switch(newParentClockId){
                case kSYS_32K_CLK:
                case kAUDIO_PLL_A_CLK:
                    break;

                default: 
                    goto cleanUp;
            }
            break;
            
		case kDMT1_F_CLK:
		case kDMT2_F_CLK:
		case kDMT3_F_CLK:
		case kDMT4_F_CLK:
		case kDMT5_F_CLK:
		case kDMT6_F_CLK:
		case kDMT7_F_CLK:
			switch(newParentClockId){
                case kTCLKIN_CLK: 
                case kSYS_CLK18_CLK:
                case kSYSCLKIN_CLK:
					break;
			
				default: 
					goto cleanUp;
			}
			break;

        case kMCASP0_F_CLK:
        case kMCASP1_F_CLK:
        case kMCASP2_F_CLK:
            switch(newParentClockId){
                case kSYS_CLK20_CLK:
                case kSYS_CLK21_CLK:
                case kSYS_CLK22_CLK:
                    break;

                default: 
                    goto cleanUp;
            }
            break;
    }

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
    return 27.0f * 1000000.0f;
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

UINT32 PrcmClockGetClockRate(OMAP_CLOCKID clock_id)
{
    float freq=0;
		
OALMSG(1, (L"+PrcmClockGetClockRate(clock_id=%d)\r\n", clock_id));
// TODO: IMPLEMENT	
    return (UINT32)freq;
}



