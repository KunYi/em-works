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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_edio.c
//
//  This file contains a DDK interface for the EDIO module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4202 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "mxarm11.h"

//-----------------------------------------------------------------------------
// External Functions
extern void EdioLock(void);
extern void EdioUnlock(void);


//-----------------------------------------------------------------------------
// External Variables
extern PCSP_EDIO_REGS g_pEDIO;


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function: DDKEdioSetConfig
//
//  Sets the EDIO configration (direction and interrupt) for the specified pin.
//
//  Parameters:
//      pin
//          [in] EDIO pin [0-7].
//
//      dir
//          [in] Direction for the pin.
//
//      intr
//          [in] Interrupt configuration for the pin.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKEdioSetConfig(UINT8 pin, DDK_EDIO_DIR dir, DDK_EDIO_INTR intr)
{
    BOOL rc = FALSE;
    
    // Make sure EDIO virtual mapping exists
    if (g_pEDIO == NULL)
    {
        ERRORMSG(TRUE, (_T("EDIO null pointer!\r\n")));
        goto cleanUp;
    }

    // Validate pin
    if (pin >= EDIO_PINS_PER_PORT)
    {
        ERRORMSG(TRUE, (_T("Invalid EDIO pin!\r\n")));
        goto cleanUp;
    }
    
    // Acquire lock for shared register access to EDIO registers
    EdioLock();

    // Configure direction
    INSREG16(&g_pEDIO->EPDDR, EDIO_PIN_MASK(pin), (dir << pin));

    // Configure interrupt logic
    INSREG16(&g_pEDIO->EPPAR, EDIO_EPPAR_MASK(pin), EDIO_EPPAR_VAL(intr, pin));

    // Release lock for shared register access to EDIO registers
    EdioUnlock();

    rc = TRUE;

cleanUp:

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKEdioWriteData
//
//  Writes the EDIO port data to the specified pins.
//
//  Parameters:
//      portMask
//          [in] Bit mask for data port pins to be written.
//
//      data
//          [in] Data to be written.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//
//-----------------------------------------------------------------------------
BOOL DDKEdioWriteData(UINT8 portMask, UINT8 data)
{
    BOOL rc = FALSE;
    
    // Make sure EDIO virtual mapping exists
    if (g_pEDIO == NULL)
    {
        ERRORMSG(TRUE, (_T("EDIO null pointer!\r\n")));
        goto cleanUp;
    }
    
    // Make sure data bits fall within mask
    data &= portMask;
    
    // Acquire lock for shared register access to EDIO registers
    EdioLock();

    INSREG16(&g_pEDIO->EPDR, portMask, data);

    // Release lock for shared register access to EDIO registers
    EdioUnlock();

    rc = TRUE;

cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKEdioWriteDataPin
//
//  Writes the EDIO port data to the specified pin.
//
//  Parameters:
//      pin
//          [in] EDIO pin [0-7].
//
//      data
//           [in] Data to be written [0 or 1].
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKEdioWriteDataPin(UINT8 pin, UINT8 data)
{
    if (pin >= EDIO_PINS_PER_PORT)
    {
        ERRORMSG(TRUE, (_T("Invalid EDIO pin!\r\n")));
        return FALSE;
    }

    return DDKEdioWriteData(EDIO_PIN_MASK(pin), (data << pin));
}


//-----------------------------------------------------------------------------
//
//  Function: DDKEdioReadData
//
//  Reads the EDIO port data for the specified pins.
//
//  Parameters:
//      portMask
//          [in] Bit mask for data port pins to be read.
//
//      pData
//          [out] Points to buffer for data read.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKEdioReadData(UINT8 portMask, UINT8 *pData)
{
    BOOL rc = FALSE;
    
    // Make sure EDIO virtual mapping exists
    if (g_pEDIO == NULL)
    {
        ERRORMSG(TRUE, (_T("EDIO null pointer!\r\n")));
        goto cleanUp;
    }
    
    // Validate buffer
    if (pData == NULL)
    {
        ERRORMSG(1, (_T("NULL input parameter!\r\n")));
        goto cleanUp;
    }
    
    *pData = portMask & ((UINT8) INREG16(&g_pEDIO->EPDR));
    
    rc = TRUE;

cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKEdioReadDataPin
//
//  Reads the EDIO port data from the specified pin.
//
//  Parameters:
//      pin
//          [in] EDIO pin [0-7].
//
//      pData
//          [out] Points to buffer for data read.  Data will be shifted to LSB.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKEdioReadDataPin(UINT8 pin, UINT8 * pData)
{
    BOOL rc = FALSE;
    
    // Make sure EDIO virtual mapping exists
    if (g_pEDIO == NULL)
    {
        ERRORMSG(TRUE, (_T("EDIO null pointer!\r\n")));
        goto cleanUp;
    }

    // Validate pin
    if (pin >= EDIO_PINS_PER_PORT)
    {
        ERRORMSG(TRUE, (_T("Invalid EDIO pin!\r\n")));
        goto cleanUp;
    }
    
    if (!DDKEdioReadData(EDIO_PIN_MASK(pin), pData))
    {
        ERRORMSG(TRUE, (_T("DDKEdioReadData failed!\r\n")));
        goto cleanUp;
    }

    // Shift data to LSB
    *pData >>= pin;

    rc = TRUE;
    
cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKEdioReadIntr
//
//  Reads the EDIO port interrupt status for the specified pins.
//
//  Parameters:
//      portMask
//          [in] Bit mask for interrupt status pins to be read.
//
//      pStatus
//          [out] Points to buffer for interrupt status.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKEdioReadIntr(UINT8 portMask, UINT8 *pStatus)
{
    BOOL rc = FALSE;
    
    // Make sure EDIO virtual mapping exists
    if (g_pEDIO == NULL)
    {
        ERRORMSG(TRUE, (_T("EDIO null pointer!\r\n")));
        goto cleanUp;
    }
    
    // Validate buffer
    if (pStatus == NULL)
    {
        ERRORMSG(1, (_T("NULL input parameter!\r\n")));
        goto cleanUp;
    }
    
    *pStatus = portMask & ((UINT8) INREG16(&g_pEDIO->EPFR));
    
    rc = TRUE;

cleanUp:

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKEdioReadIntrPin
//
//  Reads the EDIO port interrupt status from the specified pin.
//
//  Parameters:
//      pin
//          [in] EDIO pin [0-7].
//
//      pStatus
//          [out] Points to buffer for interrupt status.  Status will be 
//          shifted to LSB.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKEdioReadIntrPin(UINT8 pin, UINT8 * pStatus)
{
    BOOL rc = FALSE;
    
    // Make sure EDIO virtual mapping exists
    if (g_pEDIO == NULL)
    {
        ERRORMSG(TRUE, (_T("EDIO null pointer!\r\n")));
        goto cleanUp;
    }

    // Validate pin
    if (pin >= EDIO_PINS_PER_PORT)
    {
        ERRORMSG(TRUE, (_T("Invalid EDIO pin!\r\n")));
        goto cleanUp;
    }
    
    if (!DDKEdioReadIntr(EDIO_PIN_MASK(pin), pStatus))
    {
        ERRORMSG(TRUE, (_T("DDKEdioReadData failed!\r\n")));
        goto cleanUp;
    }

    // Shift data to LSB
    *pStatus >>= pin;

    rc = TRUE;
    
cleanUp:

    return rc;
    
}


//-----------------------------------------------------------------------------
//
//  Function: DDKEdioClearIntrPin
//
//  Clears the EDIO interrupt status for the specified pin.
//
//  Parameters:
//      pin
//          [in] EDIO pin [0-7].
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKEdioClearIntrPin(UINT8 pin)
{
    BOOL rc = FALSE;
    
    // Make sure EDIO virtual mapping exists
    if (g_pEDIO == NULL)
    {
        ERRORMSG(TRUE, (_T("EDIO null pointer!\r\n")));
        goto cleanUp;
    }

    // Validate pin
    if (pin >= EDIO_PINS_PER_PORT)
    {
        ERRORMSG(TRUE, (_T("Invalid EDIO pin!\r\n")));
        goto cleanUp;
    }
    
    // EPFR is write-1-clear
    SETREG16(&g_pEDIO->EPFR, EDIO_PIN_MASK(pin));

    rc = TRUE;

cleanUp:

    return rc;
    
}



