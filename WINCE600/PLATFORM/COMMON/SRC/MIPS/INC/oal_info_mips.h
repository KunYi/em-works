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
//  File:  oal_info_mips.h
//
#ifndef __OAL_INFO_MIPS_H
#define __OAL_INFO_MIPS_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define OAL_MIPS_COMP_ID_NONE   0
#define OAL_MIPS_COMP_ID_MIPS   1
#define OAL_MIPS_COMP_ID_AMD    3

typedef union {
    struct {
        UINT32 revision:8;
        UINT32 processorId:8;
        UINT32 companyId:8;
        UINT32 option:8;
    };
    UINT32 value;
} OAL_MIPS_PROCESSOR_ID;

UINT32 OALMIPSGetProcessorId();

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
