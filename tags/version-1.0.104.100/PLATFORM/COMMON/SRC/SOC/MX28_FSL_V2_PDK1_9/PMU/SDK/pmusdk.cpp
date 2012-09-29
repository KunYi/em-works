//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include "ceddk.h"
#pragma warning(pop)

#include "csp.h"
#include "pmu.h"
#include "pmu_ioctl.h"

//-----------------------------------------------------------------------------
// Global Variables
HANDLE hPMI;

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

DBGPARAM dpCurSettings = {
    _T("PMU"),
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
        TEXT("Info"), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN // ulZoneMask
};

#endif  // DEBUG

//-----------------------------------------------------------------------------
//
//  Function: PmuInitBatteryMonitor
//
//  This routine initializes the Battery monitor for battery module.
//
//  Parameters:
//         None
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuInitBatteryMonitor(void)
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_BATT_MONITOR_INIT,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_BATT_MONITOR_INIT failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;

}

//-----------------------------------------------------------------------------
//
//  Function: PmuGetBatteryVoltage
//
//  This routine returns the current Battery voltage.
//
//  Parameters:
//         [Out] battery voltage.
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuGetBatteryVoltage(UINT32 *pBattVol)
{

    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_BATT_GET_VOLTAGE,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pBattVol,                   // out buffer
        sizeof(UINT32),             // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_BATT_GET_VOLTAGE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;
}



//-----------------------------------------------------------------------------
//
//  Function:  PmuSetCharger
//
//  This function is used to set the charger
//
//  Parameters:
//          current
//              [in] The current value of charger
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuSetCharger(DWORD current)
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_BATT_CHARGE_SET,   // I/O control code
        &current,                       // in buffer
        sizeof(DWORD),                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_BATT_CHARGE_SET failed!\r\n"), __WFUNCTION__));
        return FALSE;
    } 

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  PmuStopCharger
//
//  This function is used to stop the charger
//
//  Parameters:
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuStopCharger()
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_BATT_CHARGE_STOP,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_BATT_CHARGE_STOP failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    return TRUE;

}

//-----------------------------------------------------------------------------
//
//  Function: PmurGetBatteryChargingStatus
//
//  This routine returns the Battery charging status.
//
//  Parameters:
//          [Out] Changing states, TURE is Battery in charging, FALSE is Battery not in charging.
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL PmuGetBatteryChargingStatus(BOOL *bChargStas )
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_BATT_CHARGE_GET_STATUS,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        bChargStas,                 // out buffer
        sizeof(BOOL),               // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_BATT_CHARGE_GET_STATUS failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;

}

//-----------------------------------------------------------------------------
//
//  Function:  PmuSetVddd
//
// This function sets the VDDD value and VDDD brownout level specified by the
// input parameters. If the new brownout level is equal to the current setting
// it'll only update the VDDD setting. If the new brownout level is less than
// the current setting, it will update the VDDD brownout first and then the VDDD.
// Otherwise, it will update the VDDD first and then the brownout. This
// arrangement is intended to prevent from false VDDD brownout. This function
// will not return until the output VDDD stable.
//
//  Parameters:
//          NewTarget
//             [in] Vddd voltage in millivolts
//
//          NewBrownout
//             [in]  Vddd brownout in millivolts
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------

BOOL PmuSetVddd(UINT32 NewTargetmV,  UINT32 NewBrownoutmV )
{
    POWER_RAIL_VALUE VdddValue;
    VdddValue.TargetmV = NewTargetmV;
    VdddValue.BrownOutmV = NewBrownoutmV;
    
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_VDDD_SET_VOLTAGE,   // I/O control code
        &VdddValue,                   // in buffer
        sizeof(POWER_RAIL_VALUE),     // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_VDDD_SET_VOLTAGE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    } 

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  PmuGetVddd
//
//  This function returns the present values of the VDDD voltage in millivolts
//
//  Parameters:
//      [Out] Vddd voltage in millivolts
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuGetVddd(UINT32 *VdddmV )
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_VDDD_GET_VOLTAGE,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        VdddmV,                     // out buffer
        sizeof(UINT32),             // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_VDDD_GET_VOLTAGE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }  
    return TRUE;

}

//-----------------------------------------------------------------------------
//
//  Function:  PmuGetVdddBrownont
//
//  This function returns the present values of the VDDD brownout in millivolts
//
//  Parameters:
//      [Out] Vddd Brownout voltage in millivolts
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE    
//
//-----------------------------------------------------------------------------
BOOL PmuGetVdddBrownont(UINT32 *VdddBo)
{
   
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_VDDD_GET_BRNOUT,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        VdddBo,                     // out buffer
        sizeof(UINT32),             // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_FET_SET_MODE failed!\r\n"), __WFUNCTION__));
        return 0;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: PmuSetFets
//
//  This routine set the Fets mode, 
//
//  Parameters:
//          bFetsMode
//              [in]  Fets mode
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuSetFets(PMU_POWER_FETSSET bFetsMode)
{

    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_FET_SET_MODE,     // I/O control code
        &bFetsMode,                 // in buffer
        sizeof(UINT32),             // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_FET_SET_MODE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;    
}

//-----------------------------------------------------------------------------
//
//  Function:PmuPowerGetSupplyMode
//
//  This routine checks if the 5V supply is present.
//
//  Parameters:
//          [Out] Supply mode 5V/Battery.
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuPowerGetSupplyMode(PMU_POWER_SUPPLY_MODE *PowerMode)
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_POWER_SUPPLY_MODE_GET,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        (UINT32*)PowerMode,         // out buffer
        sizeof(UINT32),             // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_POWER_THERMAL_SET failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:PmuPowerThermalSetTemp
//
//  This routine Set Thermal reset temperature .
//
//  Parameters:
//          [In] Reset temperature value (115degc to  150 degc)
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuPowerThermalSetTemp(PMU_POWER_THERMAL_TEMP uTemp)
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_POWER_THERMAL_SET,   // I/O control code
        &uTemp,                       // in buffer
        sizeof(UINT32),             // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_POWER_THERMAL_SET failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;

}

//-----------------------------------------------------------------------------
//
//  Function:PmuPowerThermalGetTemp
//
//  This routine Get Thermal reset temperature .
//
//  Parameters:
//          [Out] Reset temperature value (115 degc to  150 degc)
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------

BOOL PmuPowerThermalGetTemp(PMU_POWER_THERMAL_TEMP *uTemp)
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_POWER_THERMAL_GET,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        (UINT32*)uTemp,             // out buffer
        sizeof(UINT32),             // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_POWER_THERMAL_GET failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:PmuPowerThermalResetEnable
//
//  This routine Enable Thermal module .
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuPowerThermalResetEnable(void)
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_POWER_THERMAL_ENABLE,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,             // out buffer
        0,             // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_POWER_THERMAL_ENABLE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:PmuPowerThermalResetEnable
//
//  This routine Enable Thermal module .
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE
//
//-----------------------------------------------------------------------------
BOOL PmuPowerThermalResetDisable(void)
{
    if (!DeviceIoControl(hPMI,      // file handle to the driver
        PMU_IOCTL_POWER_THERMAL_DISABLE,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,             // out buffer
        0,             // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PMU_IOCTL_POWER_THERMAL_DISABLE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;
}

//LQK:Jul-12-2012
BOOL PmuGetPowerSource(UINT32 *powerSource)
{
	if (!DeviceIoControl(hPMI,      // file handle to the driver
		PMU_IOCTL_GET_POWER_SOURCE,   // I/O control code
		NULL,                       // in buffer
		NULL,                          // in buffer size
		powerSource,                       // out buffer
		sizeof(UINT32),                          // out buffer size
		NULL,                       // pointer to number of bytes returned
		NULL))                      // ignored (=NULL)
	{
		DEBUGMSG(ZONE_ERROR,
			(TEXT("%s: PMU_IOCTL_GET_POWER_SOURCE failed!\r\n"), __WFUNCTION__));
		return FALSE;
	} 

	return TRUE;
}

//LQK:Jul-25-2012
BOOL PmuIsBatteryAttach(BOOL *bIsBatteryAttach )
{
	if (!DeviceIoControl(hPMI,      // file handle to the driver
		PMU_IOCTL_BATTERY_ATTACH,   // I/O control code
		NULL,                       // in buffer
		0,                          // in buffer size
		bIsBatteryAttach,           // out buffer
		sizeof(BOOL),               // out buffer size
		NULL,                       // pointer to number of bytes returned
		NULL))                      // ignored (=NULL)
	{
		DEBUGMSG(ZONE_ERROR,
			(TEXT("%s: PMU_IOCTL_BATTERY_ATTACH failed!\r\n"), __WFUNCTION__));
		return FALSE;
	}
	return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the PMIC DDK module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the PMIC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
extern "C"
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HMODULE)hInstDll);
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS ATTACH TO PMIC SDK *****\r\n")));

            DisableThreadLibraryCalls((HMODULE) hInstDll);

            hPMI = CreateFile(TEXT("PMI1:"),
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_RANDOM_ACCESS,
                              NULL);
            if ((hPMI == NULL) || (hPMI == INVALID_HANDLE_VALUE))
            {
                ERRORMSG(TRUE, (_T("Failed in createFile() for PMI1:\r\n")));
                return FALSE;
            }

            break;

        case DLL_PROCESS_DETACH:
            if ((hPMI != NULL) && (hPMI != INVALID_HANDLE_VALUE))
            {
                CloseHandle(hPMI);
            }

            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM PMIC SDK *****\r\n")));
            break;
    }

    // return TRUE for success
    return TRUE;
}

