//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: hw_dcp_common.h
//  Brief data co-processor interface
//
//
/////////////////////////////////////////////////////////////////////////////////

#if !defined( DDI_DCP_COMMON_H )
#define DDI_DCP_COMMON_H 1

#define DCP_GET_ANY_CHANNEL 0x0000ffff

#define DCP_HANDLE_SHIFT    28
#define CHANNEL_FROM_HANDLE(dcpHandle) ((dcpHandle >> (DCP_HANDLE_SHIFT)) & 0x00000003)
#define DCP_HANDLE_MASK (0x0fffffff)

#define DCP_VMI_CHANNEL 0

typedef UINT32 DCPHandle_t;

typedef enum _CSCFormat_t
{
    DCP_RGB16_565,
    DCP_RGB24,
    DCP_YUV422,
    DCP_YUV420
} CSCFormat_t;

#endif
