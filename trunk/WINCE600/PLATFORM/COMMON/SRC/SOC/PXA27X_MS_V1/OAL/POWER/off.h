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

/******************************************************************************
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
**
**  FILENAME:   off.h
**
**  PURPOSE:    Contains constants and structures needed for off.c
**
**
******************************************************************************/

#ifndef __OFF_H__
#define __OFF_H__

////////////////////////////////////////////////
//
//  Structure Definitions
//

typedef struct  OFF_SLEEP_DATA_S
{
    // Standard data save area must be first entry in structure.
    // data saved when entering sleep
    struct XLLP_PM_SLEEP_SAVE_DATA_S       standardDataSaveArea;

    // sleep params passed to xllp layer
    struct XLLP_PM_ENTER_SLEEP_PARAMS_S    sleepParameters;
};

////////////////////////////////////////////////
//
//  Functions defined in off.c
//
void InitSleepParams(struct OFF_SLEEP_DATA_S* pSleepData);

#endif  // __OFF_H__
