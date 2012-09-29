//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   pmic_lla.cpp
/// @brief  WM8350 PMIC low-level access functions.
///
/// This file contains the PMIC Lower Level Access SDK interface that is used by
/// applications and other drivers to access registers of the PMIC.
///
/// @version $Id: pmic_lla.cpp 682 2007-06-30 16:43:46Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
///-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include <Devload.h>
#include <ceddk.h>
#include "common_macros.h"
#include "pmic_lla.h"
#include "pmic_ioctl.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

extern "C" HANDLE               hPMI;           // Our global handle to the PMIC driver
extern "C" WM_DEVICE_HANDLE     g_hWMDevice;    // Our global Wolfson device handle

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;

#endif

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: PmicRegisterRead
//
// This function reads a register in PMIC.
//
// Parameters:
//      index
//           [in] the index of the register.
//      reg
//           [out] the content read out from the register.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
PMIC_STATUS
PmicRegisterRead(unsigned char index, UINT32* reg)
{
    UINT32 param;
    UINT32 content = 0;
    BOOL ret;

    //DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param = index;

    // Need to check if index is valid here.

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param,
              sizeof(param), &content, sizeof(content), NULL, NULL);

    *reg = content & 0xFFFFFF;

    //DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}

//-----------------------------------------------------------------------------
//
// Function: PmicRegisterWrite
//
// This function writes a register in PMIC.
//
// Parameters:
//      index
//           [in] the index of the register.
//      reg
//           [in] the content to be written to the register.
//      mask
//           [in] indicates the valid bits in the "reg" parameter.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
PMIC_STATUS
PmicRegisterWrite(unsigned char index, UINT32 reg, UINT32 mask)
{
    BOOL ret;
    PMIC_PARAM_LLA_WRITE_REG param;

    //DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = index;
    param.data = reg;
    param.mask = mask;

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL);

    //DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}


//-----------------------------------------------------------------------------
//
// Function: PmicGetSiliconRev
//
// This function returns the silicon revision reported by the current chip.
//
// Parameters:
//      None
//
// Returns:
//      Chip revision (WM_REV_A, WM_REV_B, etc), or WM_REV_UNKNOWN if not
//      available.
//
//-----------------------------------------------------------------------------
WM_CHIPREV PmicGetSiliconRev()
{
    WM_CHIPREV revision;

    revision = WMGetDeviceRev( g_hWMDevice );

    return revision;
}


//-----------------------------------------------------------------------------
//
// Function: PmicDumpAllRegs
//
// This function prints out the current register settings on the PMIC to the
// debug console.
//
// Parameters:
//      None
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
PMIC_STATUS
PmicDumpAllRegs()
{
    WM_STATUS   wmStatus;
    PMIC_STATUS pmStatus;

    wmStatus = WMDumpRegs( g_hWMDevice );
    pmStatus = WMStatusToPmicStatus( wmStatus );
    return pmStatus;
}



//-----------------------------------------------------------------------------
//
//  Function: BSPPmicBlockControlIF
//
//  Locks the control interface.  NB: this will prevent any control of the
//  PMIC or anything else on the same control bus, so should be used sparingly
//  and with caution.  It is only needed if the bus state needs to be rigorously
//  controlled for a short while.
//
//  In particular, it is not needed during normal operation - the PMIC code
//  handles all locking and unlocking internally.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE if successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicBlockControlIF()
{
    BOOL ret;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_BLOCK_CONTROL_IF, NULL,
              0, NULL, 0, NULL, NULL);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}

//-----------------------------------------------------------------------------
//
//  Function: BSPPmicUnblockControlIF
//
//  Unlocks the control interface after BSPPmicBlockControlIF, allowing normal
//  operation to resume.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE if successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicUnblockControlIF()
{
    BOOL ret;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_UNBLOCK_CONTROL_IF, NULL,
              0, NULL, 0, NULL, NULL);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}

//-----------------------------------------------------------------------------
//
// Function: PmicInterruptRegister
//
// This function writes a register in PMIC.
//
// Parameters:
//      int_id
//           [in] the interrupt id.
//      event_name
//           [in] the name of the event.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
PMIC_STATUS
PmicInterruptRegister(PMIC_INT_ID int_id, LPTSTR event_name)
{
    BOOL ret;
    PMIC_PARAM_INT_REGISTER param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.int_id = int_id;
    param.event_name = event_name;

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_INT_REGISTER, &param,
              sizeof(param), NULL, 0, NULL, NULL);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}


//-----------------------------------------------------------------------------
//
// Function: PmicInterruptDeregister
//
// This function writes a register in PMIC.
//
// Parameters:
//      int_id
//           [in] the interrupt id.
//      name
//           [in] the name of the event.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
PMIC_STATUS
PmicInterruptDeregister(PMIC_INT_ID int_id)
{
    BOOL ret;
    UINT32 param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param = int_id;

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_INT_DEREGISTER, &param,
              sizeof(param), NULL, 0, NULL, NULL);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}


//-----------------------------------------------------------------------------
//
// Function: PmicInterruptHandlingComplete
//
// This function writes a register in PMIC.
//
// Parameters:
//      int_id
//           [in] the interrupt id.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
PMIC_STATUS
PmicInterruptHandlingComplete(PMIC_INT_ID int_id)
{
    BOOL ret;
    UINT32 param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param = int_id;

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_INT_COMPLETE, &param,
              sizeof(param), NULL, 0, NULL, NULL);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}


//-----------------------------------------------------------------------------
//
// Function: PmicInterruptDisable
//
// This function disables an interrupt
//
// Parameters:
//      int_id
//           [in] the interrupt id.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
PMIC_STATUS
PmicInterruptDisable(PMIC_INT_ID int_id)
{
    BOOL ret;
    UINT32 param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param = int_id;

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_INT_DISABLE, &param,
              sizeof(param), NULL, 0, NULL, NULL);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}


//-----------------------------------------------------------------------------
//
// Function: PmicInterruptEnable
//
// This function enables an interrupt
//
// Parameters:
//      int_id
//           [in] the interrupt id.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
PMIC_STATUS
PmicInterruptEnable(PMIC_INT_ID int_id)
{
    BOOL ret;
    UINT32 param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param = int_id;

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_INT_ENABLE, &param,
              sizeof(param), NULL, 0, NULL, NULL);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    if (ret)
    {
        return PMIC_SUCCESS;
    }
    else
    {
        return PMIC_ERROR;
    }
}

/*******************************************************************************
 * Function: WMStatusToPmicStatus                                          *//**
 *
 * @brief   Returns the PMIC_STATUS for the WM_STATUS.
 *
 * @param   status      The status to convert.
 *
 * @return  The corresponding PMIC_STATUS.
 ******************************************************************************/
extern "C" PMIC_STATUS WMStatusToPmicStatus( WM_STATUS status )
{
    switch ( status )
    {
    case WMS_SUCCESS:
        return PMIC_SUCCESS;
    case WMS_INVALID_PARAMETER:
        return PMIC_PARAMETER_ERROR;
    case WMS_UNSUPPORTED:
        return PMIC_NOT_SUPPORTED;
    case WMS_RESOURCE_FAIL:
        return PMIC_MALLOC_ERROR;
    default:
        return PMIC_ERROR;
    }
}
