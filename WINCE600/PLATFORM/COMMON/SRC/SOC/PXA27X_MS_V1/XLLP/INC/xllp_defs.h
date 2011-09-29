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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/******************************************************************************
**
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
**
**  FILENAME:       xllp_defs.h
**
**  PURPOSE:        header file contining common XScale Low Level Primitive
**                  (XLLP) type definitions and preprocessor macros. This as 
**                  a header file that is intended for inclusion within XLLP
**                  library sources, and in sources that invoke XLLP library 
**                  functions.                    
**                  
**
******************************************************************************/

#ifndef __XLLP_DEFS_H__
#define __XLLP_DEFS_H__

/* Typedefs for peripheral register variables */
typedef long                   XLLP_INT32_T,   *P_XLLP_INT32_T;
typedef volatile XLLP_INT32_T  XLLP_VINT32_T,  *P_XLLP_VINT32_T;
typedef unsigned long          XLLP_UINT32_T,  *P_XLLP_UINT32_T;
typedef volatile XLLP_UINT32_T XLLP_VUINT32_T, *P_XLLP_VUINT32_T;

typedef int                    XLLP_INT_T,     *P_XLLP_INT_T;
typedef volatile XLLP_INT_T    XLLP_VINT_T,    *P_XLLP_VINT_T;
typedef unsigned int           XLLP_UINT_T,    *P_XLLP_UINT_T;
typedef volatile XLLP_UINT_T   XLLP_VUINT_T,   *P_XLLP_VUINT_T;

typedef short                  XLLP_INT16_T,   *P_XLLP_INT16_T;
typedef volatile XLLP_INT16_T  XLLP_VINT16_T,  *P_XLLP_VINT16_T;
typedef unsigned short         XLLP_UINT16_T,  *P_XLLP_UINT16_T;
typedef volatile XLLP_UINT16_T XLLP_VUINT16_T, *P_XLLP_VUINT16_T;

typedef char                   XLLP_INT8_T,    *P_XLLP_INT8_T;
typedef volatile XLLP_INT8_T   XLLP_VINT8_T,   *P_XLLP_VINT8_T;
typedef unsigned char          XLLP_UINT8_T,   *P_XLLP_UINT8_T;
typedef volatile XLLP_UINT8_T  XLLP_VUINT8_T,  *P_XLLP_VUINT8_T;

#ifndef NULL
#define NULL 0
#endif

/* XLLP status codes. */
typedef enum
{
    /* The following status codes are common to all XLLP components: */
    XLLP_STATUS_SUCCESS = 0,
    XLLP_STATUS_FAILURE,
    XLLP_STATUS_UNSUPPORTED,
    XLLP_STATUS_WRONG_PARAMETER,
    XLLP_STATUS_TIME_OUT,
    XLLP_STATUS_NO_RESOURCES,
    XLLP_STATUS_HW_ERROR,
    XLLP_STATUS_NO_RESPONSE,
    XLLP_STATUS_DATA_UNDERRUN,
    XLLP_STATUS_DATA_OVERRUN,
    XLLP_STATUS_DATA_ERROR,
    XLLP_STATUS_OPERATION_ABORTED,
    XLLP_STATUS_INVALID_STATE,
    XLLP_STATUS_RESOURCE_ALREADY_EXISTS,
    /* The following status codes are specific to each XLLP component: */
    XLLP_STATUS_NO_DMA_CHANNEL_AVAILABLE,
    XLLP_STATUS_PCCARD_FAILURE,
    XLLP_STATUS_PCCARD_INVALID_SOCKET_NUMBER,
    XLLP_STATUS_PCCARD_INVALID_POINTER_ALLOC,
    XLLP_STATUS_USIM_PARITY_ERROR,
    XLLP_STATUS_USIM_CHAR_NOT_RECEIVED_YET,
    XLLP_STATUS_USIM_ATR_FAILURE,
    XLLP_STATUS_USIM_ATR_NOT_CORRECT,
    XLLP_STATUS_TOUCH_PEN_NOT_DOWN
}XLLP_STATUS_T;

/* typedefs for XLLP API */
typedef enum 
{
    XLLP_FALSE = 0, 
    XLLP_TRUE = 1
} XLLP_BOOL_T;

typedef enum 
{
    XLLP_LO = 0,
    XLLP_HI = 1
}XLLP_LEVEL_T;

typedef enum 
{
    XLLP_OFF = 0,
    XLLP_ON = 1
}XLLP_CONTROL_T;

/* Bit Position Macros */
#define XLLP_BIT_0    ( 1u << 0 )
#define XLLP_BIT_1    ( 1u << 1 )
#define XLLP_BIT_2    ( 1u << 2 )
#define XLLP_BIT_3    ( 1u << 3 )
#define XLLP_BIT_4    ( 1u << 4 )
#define XLLP_BIT_5    ( 1u << 5 )
#define XLLP_BIT_6    ( 1u << 6 )
#define XLLP_BIT_7    ( 1u << 7 )
#define XLLP_BIT_8    ( 1u << 8 )
#define XLLP_BIT_9    ( 1u << 9 )
#define XLLP_BIT_10   ( 1u << 10 )
#define XLLP_BIT_11   ( 1u << 11 )
#define XLLP_BIT_12   ( 1u << 12 )
#define XLLP_BIT_13   ( 1u << 13 )
#define XLLP_BIT_14   ( 1u << 14 )
#define XLLP_BIT_15   ( 1u << 15 )
#define XLLP_BIT_16   ( 1u << 16 )
#define XLLP_BIT_17   ( 1u << 17 )
#define XLLP_BIT_18   ( 1u << 18 )
#define XLLP_BIT_19   ( 1u << 19 )
#define XLLP_BIT_20   ( 1u << 20 )
#define XLLP_BIT_21   ( 1u << 21 )
#define XLLP_BIT_22   ( 1u << 22 )
#define XLLP_BIT_23   ( 1u << 23 )
#define XLLP_BIT_24   ( 1u << 24 )
#define XLLP_BIT_25   ( 1u << 25 )
#define XLLP_BIT_26   ( 1u << 26 )
#define XLLP_BIT_27   ( 1u << 27 )
#define XLLP_BIT_28   ( 1u << 28 )
#define XLLP_BIT_29   ( 1u << 29 )
#define XLLP_BIT_30   ( 1u << 30 )
#define XLLP_BIT_31   ( 1u << 31 )

/* Miscellaneous Macros */
#define XLLP_USEMASK    XLLP_TRUE
#define XLLP_NOMASK     XLLP_FALSE
#define XLLP_SET        1
#define XLLP_CLEAR      0
#define XLLP_ENCODED_ERROR 0xdeadc0deL

#endif //__XLLP_DEFS_H__




