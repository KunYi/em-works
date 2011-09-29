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
// Copyright (c) Texas Instruments Corporation.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  Header:  omap5912_hdq_1wire.h
//
#ifndef __OMAP5912_HDQ_1WIRE_H
#define __OMAP5912_HDQ_1WIRE_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 TX_DATA;                 // 0000
    UINT32 RX_RECEIVE_BUFFER_REG;   // 0004
    UINT32 CNTL_STATUS_REG;         // 0008
    UINT32 INTERRUPT_STATUS;        // 000c
} OMAP5912_HDQ_1WIRE_REGS;

//------------------------------------------------------------------------------

#define OMAP5912_HDQ_SINGE_BIT               (1<<7)
#define OMAP5912_HDQ_INTERRUPT_MASK          (1<<6)
#define OMAP5912_HDQ_POWER_MODE              (1<<5)
#define OMAP5912_HDQ_GO_BIT                  (1<<4)
#define OMAP5912_HDQ_PRESENCE_DETECT         (1<<4)
#define OMAP5912_HDQ_INIT_PULSE              (1<<2)
#define OMAP5912_HDQ_READ_WRITE_BIT          (1<<1)
#define OMAP5912_HDQ_MODE_BIT                (1<<0)

#define OMAP5912_HDQ_TX_COMPLETE             (1<<2)
#define OMAP5912_HDQ_READ_COMPLETE           (1<<1)
#define OMAP5912_HDQ_PRESENCE_TIMEOUT        (1<<0)

//------------------------------------------------------------------------------
    
#endif
