//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: hw_dcp_csc.c
//  Brief data co-processor interface
//
//
/////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)
#include "hw_dcp.h"

extern PVOID pv_HWregDCP;

HRESULT dcp_hw_RunCSC()
{
    HW_DCP_CSCCTRL0_SET(BM_DCP_CSCCTRL0_ENABLE);

    return ERROR_SUCCESS;
}

HRESULT dcp_hw_Rotate(BOOL Enable)
{
    if (Enable != FALSE) 
    {
        HW_DCP_CSCCTRL0_SET(BM_DCP_CSCCTRL0_ROTATE);
    }
    else
    {
        HW_DCP_CSCCTRL0_CLR(BM_DCP_CSCCTRL0_ROTATE);
    }

    return ERROR_SUCCESS;
}

HRESULT dcp_hw_SetScale(UINT32 InputWidth, UINT32 InputHeight, UINT32 OutputWidth, UINT32 OutputHeight)
{
    INT32 ScaleInt;
    INT32 ScaleFrac;
    HRESULT Status;

    if ((OutputWidth != InputWidth) || (OutputHeight != InputHeight))
    {
        HW_DCP_CSCCTRL0_SET(BM_DCP_CSCCTRL0_SCALE);

        ScaleInt = InputWidth / OutputWidth;
        // We can't downscale greater than 2
        if(ScaleInt >= 3)
        {
            Status = ERROR_INVALID_DATA;
            ERRORMSG (1, (L"DCP can't downscale greater than 2!\r\n"));
            ASSERT(0);
            return Status;
        }

        ScaleFrac = InputWidth % OutputWidth;

        HW_DCP_CSCXSCALE_WR((ScaleInt<<24) | (ScaleFrac <<12) | OutputWidth );

        ScaleInt = InputHeight / OutputHeight;
        // We can't downscale greater than 2
        if(ScaleInt >= 3)
        {
            Status = ERROR_INVALID_DATA;
            ERRORMSG (1, (L"DCP can't downscale greater than 2!\r\n"));
            ASSERT(0);
            return Status;
        }

        ScaleFrac = InputHeight % OutputHeight;

        HW_DCP_CSCYSCALE_WR((ScaleInt<<24) | (ScaleFrac << 12) | OutputHeight);

    }
    else
    {
        HW_DCP_CSCCTRL0_CLR(BM_DCP_CSCCTRL0_SCALE);
    }


    return ERROR_SUCCESS;
}

HRESULT dcp_hw_SetOutputSize(UINT32 Width, UINT32 Height)
{
    HW_DCP_CSCOUTBUFPARAM_WR((Height << 12) | Width);

    return ERROR_SUCCESS;
}

HRESULT dcp_hw_SetInputSize(UINT32 Width, UINT32 Height)
{
    HW_DCP_CSCINBUFPARAM_WR((Height << 12) | Width);

    return ERROR_SUCCESS;
}

HRESULT dcp_hw_SetOutputBuffer(void *Buffer)
{
    HW_DCP_CSCRGB_WR((reg32_t)Buffer);

    return ERROR_SUCCESS;
}

HRESULT dcp_hw_SetInputBuffer(void *Y, void *U, void *V)
{
    HW_DCP_CSCLUMA_WR ((reg32_t)Y);
    HW_DCP_CSCCHROMAU_WR((reg32_t)U);
    HW_DCP_CSCCHROMAV_WR((reg32_t)V);

    return ERROR_SUCCESS;
}

HRESULT dcp_hw_SetOutputFormat(CSCFormat_t Format)
{
    HRESULT Status;

    Status = ERROR_SUCCESS;
    switch (Format)
    {
        case DCP_RGB16_565:
            BW_DCP_CSCCTRL0_RGB_FORMAT(BV_DCP_CSCCTRL0_RGB_FORMAT__RGB16_565);
            break;
        case DCP_RGB24:
            BW_DCP_CSCCTRL0_RGB_FORMAT(BV_DCP_CSCCTRL0_RGB_FORMAT__RGB24);
            break;
        case DCP_YUV422:
            BW_DCP_CSCCTRL0_RGB_FORMAT(BV_DCP_CSCCTRL0_RGB_FORMAT__YUV422I);
            break;
        default:
            Status = ERROR_INVALID_DATA;
            break;
    }

    return Status;
}

HRESULT dcp_hw_SetInputFormat(CSCFormat_t Format)
{
    HRESULT Status;

    Status = ERROR_SUCCESS;
    switch (Format)
    {
        case DCP_YUV422:
            BW_DCP_CSCCTRL0_YUV_FORMAT(BV_DCP_CSCCTRL0_YUV_FORMAT__YUV422);
            break;
        case DCP_YUV420:
            BW_DCP_CSCCTRL0_YUV_FORMAT(BV_DCP_CSCCTRL0_YUV_FORMAT__YUV420);
            break;
        default:
            Status = ERROR_INVALID_DATA;
            break;
    }

    return Status;
}


HRESULT dcp_hw_CSCInterruptEnable(BOOL Enable)
{
    if (Enable)
    {
        HW_DCP_CTRL_SET(BM_DCP_CTRL_CSC_INTERRUPT_ENABLE);
    }
    else
    {
        HW_DCP_CTRL_CLR(BM_DCP_CTRL_CSC_INTERRUPT_ENABLE);
    }
        
    return ERROR_SUCCESS;
}

HRESULT dcp_hw_CSCInterruptClear()
{
    HW_DCP_STAT_CLR(BM_DCP_STAT_CSCIRQ);

    return ERROR_SUCCESS;
}

HRESULT dcp_hw_CSCSetPriority(UINT32 Priority)
{
    HW_DCP_CHANNELCTRL_SET(BF_DCP_CHANNELCTRL_CSC_PRIORITY(Priority));
    return ERROR_SUCCESS;
}




