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
//  Copyright (C) 2008-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)

#include <windows.h>
#include <types.h>
#include <winddi.h>
#include <emul.h>
#include <ceddk.h>

#include <ddkmacro.h>
#include <ddkreg.h>

#include <ddrawi.h>
#include <ddgpe.h>
#include <ddhfuncs.h>

#include <strsafe.h>

#pragma warning(pop)

#include "common_macros.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"

#if defined(USE_C2D_ROUTINES)
#include "c2d_api.h"
#include "c2d_mutex.h"

// Macros
#define C2D_CALL(f)               \
{                                 \
    status = f;                   \
    if (status != C2D_STATUS_OK)  \
        return status;            \
}

#define ON_ERROR_EXIT_AND_LOG(f, msg)       \
{                                           \
    status = f;                             \
    if (status != C2D_STATUS_OK)            \
    {                                       \
        C2D_EXIT;                           \
        ERRORMSG(1, (msg));                 \
        return C2DStatusToScode(status);    \
    }                                       \
}                                      

#define C2D_UNKNOWNFORMAT C2D_COLOR_8888_RGBA
const C2D_COLORFORMAT EDDGPEPixelFormatToC2DFormat[] = {
            C2D_COLOR_A1,       //ddgpePixelFormat_1bpp = 0,    
            C2D_UNKNOWNFORMAT,  //ddgpePixelFormat_2bpp,
            C2D_COLOR_A4,       //ddgpePixelFormat_4bpp,
            C2D_COLOR_8,        //ddgpePixelFormat_8bpp,
            C2D_COLOR_0565,        //ddgpePixelFormat_565,
            C2D_COLOR_1555,  //ddgpePixelFormat_5551,
            C2D_COLOR_4444,  //ddgpePixelFormat_4444,
            C2D_COLOR_1555,  //ddgpePixelFormat_5550,
            C2D_COLOR_888,  //ddgpePixelFormat_8880,
            C2D_COLOR_8888,  //ddgpePixelFormat_8888,
            C2D_COLOR_YUY2,  //ddgpePixelFormat_YUYV422,
            C2D_COLOR_UYVY,  //ddgpePixelFormat_UYVY422,
            C2D_COLOR_YUY2,  //ddgpePixelFormat_YUY2422,
            C2D_UNKNOWNFORMAT,  //ddgpePixelFormat_YV12,
            C2D_UNKNOWNFORMAT,  //ddgpePixelFormat_15bppGeneric,
            C2D_UNKNOWNFORMAT,  //ddgpePixelFormat_16bppGeneric,
            C2D_COLOR_8888,  //ddgpePixelFormat_24bppGeneric,
            C2D_COLOR_8888,     //ddgpePixelFormat_32bppGeneric,
            C2D_UNKNOWNFORMAT   //ddgpePixelFormat_UnknownFormat,
};
#endif //#if defined(USE_C2D_ROUTINES)

#include "display.h"
#include "ddipu.h"
