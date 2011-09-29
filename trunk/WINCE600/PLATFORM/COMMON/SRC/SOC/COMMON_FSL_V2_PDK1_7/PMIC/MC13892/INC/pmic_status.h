/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/

#ifndef _PMIC_STATUS_H
#define _PMIC_STATUS_H

/*!
 * @file pmic_status.h
 * @brief PMIC APIs return code definition.
 *
 * @ingroup PMIC_CORE
 */

/*!
 * @enum PMIC_STATUS
 * @brief Define return values for all PMIC APIs.
 *
 * These return values are used by all of the PMIC APIs.
 *
 * @ingroup PMIC
 */
typedef enum
{
    PMIC_SUCCESS = 0,     /*!< The requested operation was successfully
                               completed.                                     */
    PMIC_ERROR = -1,      /*!< The requested operation could not be completed
                               due to an error.                               */
    PMIC_PARAMETER_ERROR = -2,      /*!< The requested operation failed because
                                         one or more of the parameters was
                                         invalid.                             */
    PMIC_NOT_SUPPORTED = -3,        /*!< The requested operation could not be
                                         completed because the PMIC hardware
                                         does not support it. */
    PMIC_CLIENT_NBOVERFLOW = -4,    /*!< The requested operation could not be
                                         completed because there are too many 
                                         PMIC client requests */
    PMIC_MALLOC_ERROR = -5,         /*!< Error in malloc function             */
    PMIC_UNSUBSCRIBE_ERROR = -6,    /*!< Error in un-subscribe event          */
    PMIC_EVENT_NOT_SUBSCRIBED = -7, /*!< Event occur and not subscribed       */
    PMIC_EVENT_CALL_BACK = -8,      /*!< Error - bad call back                */  
} PMIC_STATUS;

#endif /* _PMIC_STATUS_H */
