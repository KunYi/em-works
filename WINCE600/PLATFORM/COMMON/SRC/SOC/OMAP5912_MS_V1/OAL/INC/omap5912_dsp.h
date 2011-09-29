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
//  Header:  omap5912_dsp.h
//

#ifndef __OMAP5912_DSP_H
#define __OMAP5912_DSP_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 PREFETCH;
    UINT32 WALKING_ST_REG;
    UINT32 CONTROL_REG;
    UINT32 FAULT_AD_H_REG;
    UINT32 FAULT_AD_L_REG;
    UINT32 FAULT_ST_REG;
    UINT32 IT_ACK_REG;
    UINT32 TTB_H_REG;
    UINT32 TTB_L_REG;
    UINT32 LOCK_REG;
    UINT32 LD_TLB_REG;
    UINT32 CAM_H_REG;
    UINT32 CAM_L_REG;
    UINT32 RAM_H_REG;
    UINT32 RAM_L_REG;
    UINT32 GFLUSH_REG;
    UINT32 FLUSH_ENTRY_REG;
    UINT32 READ_CAM_H_REG;
    UINT32 READ_CAM_L_REG;
    UINT32 READ_RAM_H_REG;
    UINT32 READ_RAM_L_REG;
    UINT32 DSPMMU_IDLE_CTRL;
} OMAP5912_DSP_MMU_REGS;

//------------------------------------------------------------------------------

#endif // __OMAP5912_DSP_H
