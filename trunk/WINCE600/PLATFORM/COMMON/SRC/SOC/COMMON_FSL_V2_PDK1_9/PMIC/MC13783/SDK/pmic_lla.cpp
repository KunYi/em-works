//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pmic_lla.cpp
//
//  This file contains the PMIC Lower Level Access SDK interface that is used by
//  applications and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

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

extern HANDLE hPMI;

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

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param = index;

    // Need to check if index is valid here.

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param,
              sizeof(param), &content, sizeof(content), NULL, NULL);

    *reg = content & 0xFFFFFF;

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

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = index;
    param.data = reg;
    param.mask = mask;

    ret = DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
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
