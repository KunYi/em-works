//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  battdrvr.cpp
//
//  This file implements the PDD layer APIs for battery driver
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <battimpl.h>
#include <Mmsystem.h>
#pragma warning(pop)

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: BatteryPDDInitialize
//
// This function performs the deinit to the battery pdd
//
// Parameters:
//      pszRegistryContext
//          [in] pointer of the reigistry context
//
// Returns:
//      TRUE for success FALSE if else
//
//-----------------------------------------------------------------------------
BOOL WINAPI BatteryPDDInitialize(LPCTSTR pszRegistryContext)
{
    UNREFERENCED_PARAMETER(pszRegistryContext);
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: BatteryPDDDeinitialize
//
// This function performs the deinit to the battery pdd
//
// Parameters:
//      void
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
void WINAPI BatteryPDDDeinitialize(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+BatteryPDDDeinitialize\r\n")));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BatteryPDDDeinitialize\r\n")));
}


//-----------------------------------------------------------------------------
//
// Function: BatteryPDDResume
//
// This function performs hardware-specific battery processing in a thread context 
//      following system resume
//
// Parameters:
//      void
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
void WINAPI BatteryPDDResume(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+BatteryPDDResume\r\n")));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BatteryPDDResume\r\n")));
    
}


//-----------------------------------------------------------------------------
//
// Function: BatteryPDDPowerHandler
//
// This power callback performs hardware-specific processing for the battery driver
//
// Parameters:
//      bOff
//          [in] TRUE if suspending, FALSE if resuming. 
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
void WINAPI BatteryPDDPowerHandler(BOOL bOff)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+BatteryPDDPowerHandler\r\n")));
    UNREFERENCED_PARAMETER(bOff);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BatteryPDDPowerHandler\r\n")));
}


//-----------------------------------------------------------------------------
//
// Function: BatteryPDDGetStatus
//
// This routine obtains the most current battery/power status available on the platform.  It fills 
// in the structures pointed to by its parameters
//
// Parameters:
//      pstatus
//          [in] pointer to the battery status structure
//      pfBatteriesChangedSinceLastCall
//          [in] Pointer to a flag that the function sets to TRUE if the user replaced or changed 
//                  the system's batteries since the last call to this function. 
//
// Returns:
//      TRUE if success FALSE if error
//
//-----------------------------------------------------------------------------
BOOL WINAPI BatteryPDDGetStatus(PSYSTEM_POWER_STATUS_EX2 pstatus, PBOOL pfBatteriesChangedSinceLastCall)
{   
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+BatteryPDDGetStatus\r\n")));
    pstatus->ACLineStatus = AC_LINE_ONLINE;
    pstatus->BatteryFlag = BATTERY_FLAG_NO_BATTERY;
    pstatus->BackupBatteryFlag  =  BATTERY_FLAG_UNKNOWN;
    UNREFERENCED_PARAMETER(pfBatteriesChangedSinceLastCall);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BatteryPDDGetStatus\r\n")));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: BatteryPDDGetLevels
//
// This routine indicates how many battery levels will be reported
// in the BatteryFlag and BackupBatteryFlag fields of the PSYSTEM_POWER_STATUS_EX2
// filed in by BatteryPDDGetStatus().  This number ranges from 0 through 3 --
// see the Platform Builder documentation for details.  The main battery
// level count is reported in the low word of the return value; the count 
// for the backup battery is in the high word.
//
// Parameters:
//      void
//
// Returns:
//      Number of battery levels
//
//-----------------------------------------------------------------------------
long BatteryPDDGetLevels(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+BatteryPDDGetLevels\r\n")));
    LONG lLevels = MAKELONG (3 /* main battery levels   */,  
                             0 /* backup battery levels */);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BatteryPDDGetLevels\r\n")));
    return lLevels;
}


//-----------------------------------------------------------------------------
//
// Function: BatteryPDDSupportsChangeNotification
//
// This routine returns TRUE to indicate that the pfBatteriesChangedSinceLastCall
// value filled in by BatteryPDDGetStatus() is valid.  If there is no way to
// tell that the platform's batteries have been changed this routine should
// return FALSE.
//
// Parameters:
//      void
//
// Returns:
//      false
//
//-----------------------------------------------------------------------------
BOOL BatteryPDDSupportsChangeNotification(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+BatteryPDDSupportsChangeNotification\r\n")));
    BOOL fSupportsChange = FALSE;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-BatteryPDDSupportsChangeNotification\r\n")));
    return fSupportsChange;
}
