/*
* ============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
* ============================================================================
*/

//--------------------------------------------------------------------------
//
//  File: nand.c
//
//  Platform specific funtionality for the NAND driver.
//

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <ceddk.h>
#include "am33x.h"
#include "am33x_config.h"
#include "am33x_oal_prcm.h"
#include <oal_clock.h>
#include "ceddkex.h"
#include "sdk_padcfg.h"

void GpmcEnable(BOOL on)
{
    if (on == TRUE)
    {
        RequestDevicePads(AM_DEVICE_NAND);
        EnableDeviceClocks(AM_DEVICE_GPMC, TRUE);
        EnableDeviceClocks(AM_DEVICE_ELM, TRUE);
    }
    else
    {
        EnableDeviceClocks(AM_DEVICE_GPMC, FALSE);
        EnableDeviceClocks(AM_DEVICE_ELM, FALSE);
    }

    return;
}

