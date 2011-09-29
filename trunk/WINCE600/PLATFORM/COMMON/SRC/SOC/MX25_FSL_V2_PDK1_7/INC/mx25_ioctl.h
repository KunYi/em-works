//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx25_ioctl.h
//
//  Provides definitions for the IOCTL functions for the MX25 SoC.
//
//------------------------------------------------------------------------------

#ifndef __MX25_IOCTL_H__
#define __MX25_IOCTL_H__


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
enum {
    POR_RESET,
    RESET_IN_RESET,
    WDOG_RESET,
    SOFT_RESET,
    JTAG_RESET,
    UNKNOWN_RESET
};

#define IOCTL_HAL_GET_RESET_CAUSE \
    CTL_CODE(FILE_DEVICE_HAL, 2100, METHOD_BUFFERED, FILE_ANY_ACCESS)


#endif // __MX25_IOCTL_H__
