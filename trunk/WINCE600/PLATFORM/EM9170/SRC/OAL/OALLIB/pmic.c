//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  pmic.c
//
//  This file contains the OAL support code for the PMIC interface.
//
//-----------------------------------------------------------------------------

#include <bsp.h>
#include "mc34704.h"

//-----------------------------------------------------------------------------
// External Functions

extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Functions


//-----------------------------------------------------------------------------
//
//  Function: OALPmicInit
//
//  Initializes the PMIC interface for OAL communication.
//
//  Parameters:
//      None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL OALPmicInit(void)
{
    BOOL rc = TRUE;

	//
	// CS&ZHL JUN-2-2011: iMX257PDK has a MC34704 as power managment IC, not EM9170
	//
#ifdef		IMX257PDK_MC34704
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_ENABLED);
    
    PmicOpen(); 
    PmicEnable(TRUE);

    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_DISABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_DISABLED);
#endif		//IMX257PDK_MC34704

	return rc;
}
