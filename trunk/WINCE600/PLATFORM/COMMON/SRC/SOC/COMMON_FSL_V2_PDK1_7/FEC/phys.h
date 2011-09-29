//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  phys.h
//
//  Implementation of FEC Driver
//
//  This file defines interfaces for external PHY(s)
//
//------------------------------------------------------------------------------

#ifndef _SRC_DRIVERS_PHYS_H

#define _SRC_DRIVERS_PHYS_H

//#include "fec.h"

// AMD AM79C874 phy

// Specific register definitions for the Am79c874
#define MII_AM79C874_MFR       16  /* Miscellaneous Feature Register */
#define MII_AM79C874_ICSR      17  /* Interrupt/Status Register      */
#define MII_AM79C874_DR        18  /* Diagnostic Register            */
#define MII_AM79C874_PMLR      19  /* Power and Loopback Register    */
#define MII_AM79C874_MCR       21  /* ModeControl Register           */
#define MII_AM79C874_DC        23  /* Disconnect Counter             */
#define MII_AM79C874_REC       24  /* Recieve Error Counter          */

FEC_PHY_CMD Am79c874Config[] = {
    {MII_READ_COMMAND(MII_REG_CR), FECParseMIICr},
    {MII_READ_COMMAND(MII_REG_ANAR), FECParseMIIAnar},
    {MII_READ_COMMAND(MII_AM79C874_DR), FECParseAm79c874Dr},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD Am79c874Startup[] = {
    {MII_WRITE_COMMAND(MII_AM79C874_ICSR, 0xff00), NULL},
    {MII_WRITE_COMMAND(MII_REG_CR, 0x1200), NULL},
    {MII_READ_COMMAND(MII_REG_SR), FECParseMIISr},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD Am79c874Ackint[] = {
    {MII_READ_COMMAND(MII_REG_SR), FECParseMIISr},
    {MII_READ_COMMAND(MII_AM79C874_DR), FECParseAm79c874Dr},
    {MII_READ_COMMAND(MII_AM79C874_ICSR), NULL},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD Am79c874Shutdown[] = {
    {MII_WRITE_COMMAND(MII_AM79C874_ICSR, 0x0000), NULL},
    {FEC_MII_END, NULL}
};

FEC_PHY_INFO Am79c874Info = {
    0x0022561b,
    TEXT("AM79C874"),
    Am79c874Config,
    Am79c874Startup,
    Am79c874Ackint,
    Am79c874Shutdown
};

// SMCS LAN8700 phy

// Specific register definitions for the SMCS LAN8700

#define MII_LAN8700_SRR        16  /* Silicon Revision Register                */
#define MII_LAN8700_MCSR       17  /* Mode Control/Status Register         */
#define MII_LAN8700_SMR        18  /* Special Modes  Register                  */
#define MII_LAN8700_SECR       26  /* Symbol Error Counter Register        */
#define MII_LAN8700_CSIR       27  /* Control/Status Indication Register    */
#define MII_LAN8700_SITC       28  /* Special Internal Testbility  Register   */
#define MII_LAN8700_ICR        29  /* Interrupt Sources Control  Register   */
#define MII_LAN8700_IMR        30  /* Interrupt Mark Register                    */
#define MII_LAN8700_SCSR       31  /* PHY Special Control/Status Register  */


FEC_PHY_CMD LAN8700Config[] = {
    {MII_READ_COMMAND(MII_REG_CR),       FECParseMIICr      },
    {MII_READ_COMMAND(MII_REG_ANAR),     FECParseMIIAnar    },
    {MII_READ_COMMAND(MII_LAN8700_SCSR), FECParseLAN8700SCSR},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD LAN8700Startup[] = {
    {MII_WRITE_COMMAND(MII_LAN8700_IMR, 0x00fe), NULL         },
    {MII_WRITE_COMMAND(MII_REG_CR,      0x1200), NULL         },
    {MII_READ_COMMAND(MII_REG_SR),               FECParseMIISr},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD LAN8700Ackint[] = {
    {MII_READ_COMMAND(MII_REG_SR),      FECParseMIISr},
    {MII_READ_COMMAND(MII_LAN8700_ICR), NULL         },
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD LAN8700Shutdown[] = {
    {MII_WRITE_COMMAND(MII_LAN8700_IMR, 0x0000), NULL},
    {FEC_MII_END, NULL}
};

FEC_PHY_INFO LAN8700Info = {
    0x7c0c3,
    TEXT("LAN8700"),
    LAN8700Config,
    LAN8700Startup,
    LAN8700Ackint,
    LAN8700Shutdown
};

#define MII_DP83640_PHYSTS          16  
#define MII_DP83640_MICR            17  
#define MII_DP83640_MISR            18  
#define MII_DP83640_PAGESEL         19  
#define MII_DP83640_FCSR            20 
#define MII_DP83640_RECR            21  
#define MII_DP83640_PCSR            22
#define MII_DP83640_RBR             23  
#define MII_DP83640_LEDCR           24  
#define MII_DP83640_PHYCR           25  
#define MII_DP83640_10BTSCR         26  
#define MII_DP83640_CDCTRL1         27  
#define MII_DP83640_PHYCR2          28  
#define MII_DP83640_EDCR            29    
#define MII_DP83640_PCFCR           31  


FEC_PHY_CMD DP83640Config[] = {
    {MII_READ_COMMAND(MII_REG_CR),       FECParseMIICr      },
    {MII_READ_COMMAND(MII_REG_ANAR),     FECParseMIIAnar    },
    {MII_READ_COMMAND(MII_DP83640_PHYSTS), FECParseDP83640PHYSTS},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD DP83640Startup[] = {
    {MII_WRITE_COMMAND(MII_DP83640_MICR, 0x0002), NULL         },
    {MII_WRITE_COMMAND(MII_REG_CR,      0x1200), NULL         },
    {MII_READ_COMMAND(MII_REG_SR),               FECParseMIISr},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD DP83640Ackint[] = {
    {MII_READ_COMMAND(MII_REG_SR),      FECParseMIISr},
    {MII_READ_COMMAND(MII_DP83640_MISR), NULL},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD DP83640Shutdown[] = {
    {MII_WRITE_COMMAND(MII_DP83640_MICR, 0x0000), NULL},
    {FEC_MII_END, NULL}
};

FEC_PHY_INFO DP83640Info = {
    0x20005ce1,
    TEXT("DP83640"),
    DP83640Config,
    DP83640Startup,
    DP83640Ackint,
    DP83640Shutdown
};

//
// CS&ZHL JUN-2-2011: add DM9161A
//
#define MII_DM9161_CR				16				// DM9161 specified config regsiter
#define MII_DM9161_SR				17				// DM9161 specified state regsiter
#define MII_DM9161_IR				21				// interrupt register  

#define	MII_DM9161_SR_100FDX			(1 << 15)
#define	MII_DM9161_SR_100HDX			(1 << 14)
#define	MII_DM9161_SR_10FDX			(1 << 13)
#define	MII_DM9161_SR_10HDX			(1 << 12)

FEC_PHY_CMD DM9161Config[] = {
    {MII_READ_COMMAND(MII_REG_CR),			FECParseMIICr      },
    {MII_READ_COMMAND(MII_REG_ANAR),		FECParseMIIAnar    },
    {MII_READ_COMMAND(MII_DM9161_SR),	FECParseDM9161State},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD DM9161Startup[] = {
    {MII_WRITE_COMMAND(MII_REG_CR, 0x1200),	NULL         },
    {MII_READ_COMMAND(MII_REG_SR),					FECParseMIISr},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD DM9161Ackint[] = {
    {MII_READ_COMMAND(MII_REG_SR),			FECParseMIISr},
    {MII_READ_COMMAND(MII_DM9161_IR),		NULL},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD DM9161Shutdown[] = {						// DM9161 is always on, nothing to do
    {FEC_MII_END, NULL}
};

FEC_PHY_INFO DM9161AInfo = {
    0x0181b8a0, 
    TEXT("DM9161A"),
    DM9161Config,
    DM9161Startup,
    DM9161Ackint,
    DM9161Shutdown
};

// CS&ZHL JLY-20-2011: supporting DM9161C
FEC_PHY_INFO DM9161CInfo = {
    0x0181b8b1, 
    TEXT("DM9161C"),
    DM9161Config,
    DM9161Startup,
    DM9161Ackint,
    DM9161Shutdown
};


FEC_PHY_CMD PHYCmdCfg[] = {
    {MII_READ_COMMAND(MII_REG_CR), FECDispPHYCfg},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD PHYCmdLink[] = {
    {MII_READ_COMMAND(MII_REG_CR), FECParsePHYLink},
    {FEC_MII_END, NULL}
};


FEC_PHY_CMD PHYCmdSuspend[] = {
    {MII_WRITE_COMMAND(MII_REG_CR,      0x800),  NULL         },
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD PHYCmdResume[] = {
    {MII_WRITE_COMMAND(MII_REG_CR,      0x1200), NULL         },
    {FEC_MII_END, NULL}
};


// Now we can  support Am79c874 and LAN8700, if other PHY(s) need to be supported,
// add them to this array
FEC_PHY_INFO *PhyInfo[] = {
    &Am79c874Info,
    &LAN8700Info,
    &DP83640Info,
	&DM9161AInfo,
	&DM9161CInfo,
    NULL
};

#endif //  _SRC_DRIVERS_PHYS_H
