//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  nandbsp.h
//
//  Contains definitions for FMD impletation of the SoC NAND flash controller 
//  and NAND memory device.
//
//------------------------------------------------------------------------------
#ifndef __NANDBSP_H__
#define __NANDBSP_H__

#ifdef BSP_NAND_K9LBG08U0M
    #include "K9LBG08U0M.h"
#elif BSP_NAND_K9LBG08U0D
    #include "K9LBG08U0D.h"    
#elif BSP_NAND_K9F2G08U0A			//CS&ZHL APR-22-2011: supporting EM9170
    #include "K9F2G08U0A.h"    
#elif BSP_NAND_K9F1G08U0A			//CS&ZHL APR-27-2011: supporting EM9170
    #include "K9F1G08U0A.h"    
#else
    #include "K9LAG08U0M.h"
#endif

#endif    // __NANDBSP_H__
