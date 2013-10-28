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
//  Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
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

// Specific register definitions for the SMCS LAN8700
#define MII_LAN8700_SRR        16  /* Silicon Revision Register            */
#define MII_LAN8700_MCSR       17  /* Mode Control/Status Register         */
#define MII_LAN8700_SMR        18  /* Special Modes  Register              */
#define MII_LAN8700_SECR       26  /* Symbol Error Counter Register        */
#define MII_LAN8700_CSIR       27  /* Control/Status Indication Register   */
#define MII_LAN8700_SITC       28  /* Special Internal Testbility  Register*/
#define MII_LAN8700_ICR        29  /* Interrupt Sources Control  Register  */
#define MII_LAN8700_IMR        30  /* Interrupt Mark Register              */
#define MII_LAN8700_SCSR       31  /* PHY Special Control/Status Register  */

//8700 control register (basic) bits definition
#define LAN8700_BASICCONTROL_RESET                      0x8000
#define LAN8700_BASICCONTROL_LOOPBACK                   0x4000
#define LAN8700_BASICCONTROL_SPEEDSELECT                0x2000
#define LAN8700_BASICCONTROL_AN_ENABLE                  0x1000
#define LAN8700_BASICCONTROL_POWERDOWN                  0x0800
#define LAN8700_BASICCONTROL_ISOLATE                    0x0400
#define LAN8700_BASICCONTROL_RESTART_AN                 0x0200
#define LAN8700_BASICCONTROL_DUPLEXMODE                 0x0100
#define LAN8700_BASICCONTROL_COLLISSION_TEST            0x0080

//8700 status register (basic) bits definition
#define LAN8700_BASICSTATUS_100BASE_T4                  0x8000
#define LAN8700_BASICSTATUS_100BASE_TX_FULLDUPLEX       0x4000
#define LAN8700_BASICSTATUS_100BASE_TX_HALFDUPLEX       0x2000
#define LAN8700_BASICSTATUS_10BASE_T_FULLDUPLEX         0x1000
#define LAN8700_BASICSTATUS_10BASE_TX_HALFDUPLEX        0x0800
#define LAN8700_BASICSTATUS_AN_COMPLETE                 0x0020
#define LAN8700_BASICSTATUS_REMOTEFAULT                 0x0010
#define LAN8700_BASICSTATUS_AN_ABILITY                  0x0008
#define LAN8700_BASICSTATUS_LINKSTATUS                  0x0004
#define LAN8700_BASICSTATUS_JABBER_DETECT               0x0002
#define LAN8700_BASICSTATUS_EXTENDEDCAPABILITY          0x0001

//8700 Auto-Negotiation Advertisement Register bits definition
#define LAN8700_AN_ADVERTISEMENT_NEXTPAGE               0x8000
#define LAN8700_AN_ADVERTISEMENT_REMOTEFAULT            0x2000
#define LAN8700_AN_ADVERTISEMENT_PAUSEOPERATION         0x0C00
#define LAN8700_AN_ADVERTISEMENT_100BASET4              0x0200
#define LAN8700_AN_ADVERTISEMENT_100BASE_TX_FULLDUPLEX  0x0100
#define LAN8700_AN_ADVERTISEMENT_100BASE_TX             0x0080
#define LAN8700_AN_ADVERTISEMENT_10BASE_T_FULLDUPLEX    0x0040
#define LAN8700_AN_ADVERTISEMENT_10BASE_T               0x0020


FEC_PHY_CMD LAN8700Config[] = {
    {MII_READ_COMMAND(MII_REG_CR), FECParseMIICr},
    {MII_READ_COMMAND(MII_REG_ANAR), FECParseMIIAnar},
    {MII_READ_COMMAND(MII_REG_SR), FECParseLAN8700SR2},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD LAN8700Startup[] = {
    {MII_WRITE_COMMAND(MII_LAN8700_IMR, 0x00FE), NULL},
    {MII_WRITE_COMMAND(MII_REG_CR, 0x1200), NULL},
    {MII_READ_COMMAND(MII_REG_SR), FECParseMIISr},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD LAN8700Ackint[] = {
    {MII_READ_COMMAND(MII_REG_SR), FECParseMIISr},
    {MII_READ_COMMAND(MII_LAN8700_ICR), FECParseLAN8700Isr},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD LAN8700Shutdown[] = {
    {MII_WRITE_COMMAND(MII_LAN8700_IMR, 0x0000), NULL},
    {FEC_MII_END, NULL}
};

//For LAN8700 Hardware with Revision number 3
FEC_PHY_INFO LAN8700Info = {
    0x7c0c3,
    TEXT("LAN8700"),
    LAN8700Config,
    LAN8700Startup,
    LAN8700Ackint,
    LAN8700Shutdown,
};

//For LAN8700 Hardware with Revision number 4
FEC_PHY_INFO LAN8700Info_Rev1 = {
    0x7c0c4,
    TEXT("LAN8700"),
    LAN8700Config,
    LAN8700Startup,
    LAN8700Ackint,
    LAN8700Shutdown,
};

FEC_PHY_CMD PHYCmdCfg[] = {
    {MII_READ_COMMAND(MII_REG_CR), FECDispPHYCfg},
    {FEC_MII_END, NULL}
};

FEC_PHY_CMD PHYCmdLink[] = {
    {MII_READ_COMMAND(MII_REG_SR), FECParsePHYLink},
    {FEC_MII_END, NULL}
};

// Now we only support LAN8700, if other PHY(s) need to be supported,
// add them to this array
FEC_PHY_INFO *PhyInfo[] = {
    &LAN8700Info,
    &LAN8700Info_Rev1,
    NULL
};

#endif //  _SRC_DRIVERS_PHYS_H
