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
//  File:  feature.cpp
//
#include <windows.h>
#include <winnt.h>
#include <oal.h>
#include <algorithm>

#define DBG_FEATURE_QUERY        (FALSE)

//------------------------------------------------------------------------------
//
//  Function:  OALIsProcessorFeaturePresent
//
//  Called to determine the processor's supported feature set.
//
extern "C" BOOL OALIsProcessorFeaturePresent(DWORD feature)
{
    // We Do Not check CPU ID Code
    // Already Knows Feature of S3C6410

    BOOL bRet = FALSE;

    switch(feature)
    {
    case PF_ARM_V4:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_V4) = %d\r\n", bRet));
        break;
    case PF_ARM_V5:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_V5) = %d\r\n", bRet));
        break;
    case PF_ARM_V6:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_V6) = %d\r\n", bRet));
        break;
    case PF_ARM_V7:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_V7) = %d\r\n", bRet));
        break;
    case PF_ARM_THUMB:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_THUMB) = %d\r\n", bRet));
        break;
    case PF_ARM_JAZELLE:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_JAZELLE) = %d\r\n", bRet));
        break;
    case PF_ARM_DSP:        //(Deprecated)
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_DSP) = %d\r\n", bRet));
        break;
    case PF_ARM_MOVE_CP:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_MOVE_CP) = %d\r\n", bRet));
        break;
    case PF_ARM_VFP10:        //(Deprecated)
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_VFP10) = %d\r\n", bRet));
        break;
    case PF_ARM_MPU:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_MPU) = %d\r\n", bRet));
        break;
    case PF_ARM_WRITE_BUFFER:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_WRITE_BUFFER) = %d\r\n", bRet));
        break;
    case PF_ARM_MBX:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_MBX) = %d\r\n", bRet));
        break;
    case PF_ARM_L2CACHE:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_L2CACHE) = %d\r\n", bRet));
        break;
    case PF_ARM_PHYSICALLY_TAGGED_CACHE:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_PHYSICALLY_TAGGED_CACHE) = %d\r\n", bRet));
        break;
    case PF_ARM_VFP_SINGLE_PRECISION:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_VFP_SINGLE_PRECISION) = %d\r\n", bRet));
        break;
    case PF_ARM_VFP_DOUBLE_PRECISION:    // VFPv2 (VFP11)
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_VFP_DOUBLE_PRECISION) = %d\r\n", bRet));
        break;
    case PF_ARM_ITCM:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_ITCM) = %d\r\n", bRet));
        break;
    case PF_ARM_DTCM:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_DTCM) = %d\r\n", bRet));
        break;
    case PF_ARM_UNIFIED_CACHE:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_UNIFIED_CACHE) = %d\r\n", bRet));
        break;
    case PF_ARM_WRITE_BACK_CACHE:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_WRITE_BACK_CACHE) = %d\r\n", bRet));
        break;
    case PF_ARM_CACHE_CAN_BE_LOCKED_DOWN:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_CACHE_CAN_BE_LOCKED_DOWN) = %d\r\n", bRet));
        break;
    case PF_ARM_L2CACHE_MEMORY_MAPPED:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_L2CACHE_MEMORY_MAPPED) = %d\r\n", bRet));
        break;
    case PF_ARM_L2CACHE_COPROC:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_L2CACHE_COPROC) = %d\r\n", bRet));
        break;
    case PF_ARM_THUMB2:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_THUMB2) = %d\r\n", bRet));
        break;
    case PF_ARM_T2EE:        // (Thumb-2 Execution Environment)
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_T2EE) = %d\r\n", bRet));
        break;
    case PF_ARM_VFP3:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_VFP3) = %d\r\n", bRet));
        break;
    case PF_ARM_NEON:
        bRet = FALSE;            // FALSE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_NEON) = %d\r\n", bRet));
        break;
    case PF_ARM_UNALIGNED_ACCESS:
        bRet = TRUE;            // TRUE
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(PF_ARM_UNALIGNED_ACCESS) = %d\r\n", bRet));
        break;
    case PF_ARM_INTEL_XSCALE:    // Not Supported
    case PF_ARM_INTEL_PMU    :    // Not Supported
    case PF_ARM_INTEL_WMMX:    // Not Supported
        bRet = FALSE;
        break;
    default:
        bRet = FALSE;
        OALMSG(DBG_FEATURE_QUERY, (L"OALIsProcessorFeaturePresent(Unknown:0x%08x) = %d\r\n", feature, bRet));
        break;
    }

    return bRet;
}

//------------------------------------------------------------------------------

