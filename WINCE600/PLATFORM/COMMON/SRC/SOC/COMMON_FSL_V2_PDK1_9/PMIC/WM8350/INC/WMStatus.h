/*******************************************************************************
 *
 * Copyright (c) 2003-2007 Wolfson Microelectronics plc.  All rights reserved.
 *
 * This software as well as any related documentation may only be used or
 * copied in accordance with the terms of the Wolfson Microelectronics plc
 * agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
 *
 * The information in this file is furnished for informational use only,
 * is subject to change without notice, and should not be construed as a
 * commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
 * assumes no responsibility or liability for any errors or inaccuracies that
 * may appear in the software or any related documentation.
 *
 * Except as permitted by the agreement(s), no part of the software or any
 * related documentation may be reproduced, stored in a retrieval system, or
 * transmitted in any form or by any means without the express written
 * consent of Wolfson Microelectronics plc.
 *                                                                         *//**
 * @file   WMStatus.h
 * @brief  Status code definitions
 *
 * This file contains mask definitions for the status codes returned by the
 * drivers for the Wolfson devices.
 *
 * @version $Id: WMStatus.h 483 2007-05-04 19:02:20Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMSTATUS_H__
#define __WMSTATUS_H__

/*
 * Include files
 */

/*
 * Definitions
 */

/**
 * @def WM_SUCCESS( _status )
 *      Checks if @a _status is a success status code.
 */
#define WM_SUCCESS( _status )   ( WMS_RETURN_BASE == (_status & WMS_MASK) )

/**
 * @def WM_ERROR( _status )
 *      Checks if @a _status is an error status code.
 */
#define WM_ERROR( _status )     ( ! ( WM_SUCCESS( _status ) ) )

/**
 * The status values.
 * @b Note: If you add a new status value, make sure you also update WMStatusText
 * in WMStatus.c.
 */
typedef enum tag_WM_STATUS
{
    /** Successful return */
    WMS_SUCCESS             = 0xDDDD0000,
    WMS_RETURN_BASE         = WMS_SUCCESS,
    /** Successful boolean TRUE */
    WMS_RETURN_TRUE         = WMS_RETURN_BASE + 0x0001,
    /** Successful boolean FALSE */
    WMS_RETURN_FALSE        = WMS_RETURN_BASE + 0x0002,

    WMS_ERROR_BASE          = 0xEEEE0000,

    /** Device occupied */
    WMS_DEVICE_BUSY         = WMS_ERROR_BASE + 0x0001,
    /** A read timed out */
    WMS_DATA_TIMED_OUT      = WMS_ERROR_BASE + 0x0002,
    /** Didn't get lock before timeout */
    WMS_LOCK_TIMED_OUT      = WMS_ERROR_BASE + 0x0003,
    /** Nothing coming across the AC link */
    WMS_ACLINK_NOT_ACTIVE   = WMS_ERROR_BASE + 0x0004,
    /** Codec ready bit not set on link */
    WMS_CODEC_NOT_READY     = WMS_ERROR_BASE + 0x0005,
    /** No Wolfson device, or support not compiled */
    WMS_NO_SUPPORTED_DEVICE = WMS_ERROR_BASE + 0x0006,
    /** No such register */
    WMS_REG_NOT_PRESENT     = WMS_ERROR_BASE + 0x0007,
    /** Couldn't allocate a resource */
    WMS_RESOURCE_FAIL       = WMS_ERROR_BASE + 0x0010,
    /** An invalid parameter was passed in */
    WMS_INVALID_PARAMETER   = WMS_ERROR_BASE + 0x0011,
    /** The command is not supported */
    WMS_UNSUPPORTED         = WMS_ERROR_BASE + 0x0012,
    /** General hardware error */
    WMS_HW_ERROR            = WMS_ERROR_BASE + 0x0013,
    /** General error */
    WMS_FAILURE             = WMS_ERROR_BASE + 0x0014,
    /** Invalid state */
    WMS_WRONG_MODE          = WMS_ERROR_BASE + 0x0015,
    /** Resource already exists, or other conflict */
    WMS_RESOURCE_CONFLICT   = WMS_ERROR_BASE + 0x0016,
    /** The given channel does not exist */
    WMS_INVALID_CHANNEL     = WMS_ERROR_BASE + 0x0017,
    /** The resource is null */
    WMS_RESOURCE_NULL       = WMS_ERROR_BASE + 0x0018,
    /** An unknown driver message */
    WMS_UNKNOWN_MESSAGE     = WMS_ERROR_BASE + 0x0019,
    /** The profile is not supported */
    WMS_UNKNOWN_PROFILE     = WMS_ERROR_BASE + 0x001A,
    /** the event did not occur before timeout*/
    WMS_EVENT_TIMED_OUT     = WMS_ERROR_BASE + 0x001B,
    /** A parameter was out of range */
    WMS_OUT_OF_RANGE        = WMS_ERROR_BASE + 0x001C,
    /** The function has not been implemented */
    WMS_NOT_IMPLEMENTED     = WMS_ERROR_BASE + 0x001F,

    /** The data received wasn't what was expected */
    WMS_UNEXPECTED_DATA     = WMS_ERROR_BASE + 0x0020,
    /** Pen not down so X or Y reading invalid */
    WMS_NO_PEN_DOWN         = WMS_ERROR_BASE + 0x0021,
    /** Touch samples too noisy - can't get valid reading */
    WMS_SCATTERED_SAMPLES   = WMS_ERROR_BASE + 0x0022,
    /** ADC not currently generating data */
    WMS_AUXADC_INACTIVE     = WMS_ERROR_BASE + 0x0023,
    /** No ADC data received */
    WMS_NO_DATA             = WMS_ERROR_BASE + 0x0024,

    /** An invalid WM_AUDIO_SIGNAL was used */
    WMS_INVALID_SIGNAL      = WMS_ERROR_BASE + 0x0030,

    /** The register cannot be shadowed */
    WMS_UNABLE_TO_SHADOW    = WMS_ERROR_BASE + 0x0040,
    /** Read error: The register has not been set => value unknown */
    WMS_SHADOW_UNINTIALISED = WMS_ERROR_BASE + 0x0041,

    /* For SUCCESS/ERROR check */
    WMS_MASK                = 0xFFFF0000
} WM_STATUS;

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * Function: WMStatusText
 *
 * Purpose:
 * This function returns a textual representation of the status value - useful
 * for reporting errors.
 *
 * Parameters:
 *      status      status code.
 *
 * Returns:      const TCHAR *
 *      Textual representation of error code.
 ----------------------------------------------------------------------------*/
const TCHAR *WMStatusText( WM_STATUS status );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    /* __WMSTATUS_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
