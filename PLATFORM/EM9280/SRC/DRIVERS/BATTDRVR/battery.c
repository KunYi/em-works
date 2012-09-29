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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  battery.c
//
//  Provides the implementation for BSP-specific battdrvr support.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include <windev.h>
#include <devload.h>
#include <battimpl.h>
#include <battery.h>
#include "mx28_irq.h"
#include "ioctl_cfg.h"
#include "csp.h"
#include "Pmu.h"
#include "hw_lradc.h"
#include "regspower.h"

//-----------------------------------------------------------------------------
// Defines
// These are read from the registry
#define DEFAULT_BATTERY_MAX_VOLTAGE     4200
#define DEFAULT_BATTERY_VOLTAGE_HIGH    3700
#define DEFAULT_BATTERY_VOLTAGE_LOW     3200

#define BATTERY_LOW                     2400
#define BATTERY_HIGH                    4300
#define BATTERY_BAD                     3500

#define UPDATE_BATTERY_INFO_COUNT       20
#define COMPLETE_CHARGE_BATTERY_VOLTAGE 4080
//#define MAX_CHARGE_BATTERY_VOLTAGE      3200
#define MAX_SAFE_CURRENT_LEVEL          0x38     //700mA  
#define MIN_CURRENT_LEVEL               0x7      //80mA   
//#define MAX_CURRENT_LEVEL_WALL_MAX      0x18     //300mA  
//#define MAX_CURRENT_LEVEL_WALL_MIN      0x10     //200mA  
#define POWER_OFF_HOLD_TIME             2000000
#define ABSOLUTE_TEMPERATURE_VALUE      273
#define HIGH_TEMPERATURE_VALUE          353
#define MID_TEMPERATURE_VALUE           338
#define DIETEMPSTOPCHARGER              343
#define VDD5V_MAX                       5200
#define VDD5V_MIN                       0
#define VDD5V_FULL_VALID                4800

#ifdef EM9283		//LQK :Jul-12-2012
	#define MAX_CHARGE_BATTERY_VOLTAGE      3000
	#define MAX_CURRENT_LEVEL_WALL_MAX      0x28    //500mA  
	#define MAX_CURRENT_LEVEL_WALL_MIN      0x10    //200mA  
	#define MAX_CURRENT_LEVEL_USB_MAX      0x18		//300mA  
	#define MAX_CURRENT_LEVEL_USB_MIN      0x10     //200mA  
	// LQK:Jul-25-2012
	#define pv_HWregPOWER      CSP_BASE_REG_PA_POWER
#else
	#define MAX_CHARGE_BATTERY_VOLTAGE      3200
	#define MAX_CURRENT_LEVEL_WALL_MAX      0x18     //300mA  
	#define MAX_CURRENT_LEVEL_WALL_MIN      0x10     //200mA  
#endif
#define BATTERY_MAX_VOLTAGE_TEXT     TEXT("MaxBatteryVoltage")
#define BATTERY_VOLTAGE_HIGH_TEXT    TEXT("BatteryVoltageHighLevel")
#define BATTERY_VOLTAGE_LOW_TEXT     TEXT("BatteryVoltageLowLevel")

//-----------------------------------------------------------------------------
// Local Variables

static UINT32 gBatteryVoltage        = 0x0;
static BOOL gfACOnline               = FALSE;
static BOOL gfBatteryCharging        = FALSE;
static DWORD gdwBatteryMaxVoltage    = 0;
static DWORD gdwBatteryVoltageHigh   = 0;
static DWORD gdwBatteryVoltageLow    = 0;
static DWORD dwUpdateVoltageTime     = 18;
static DWORD gdwDieTemperature       = 0;
static DWORD gdwPSwitchStartTime     = 0;
static LPTSTR g_5VDetectEventName    = TEXT("5V_Detect");
static HANDLE g_h5VIntEvent          = NULL;
static HANDLE g_h5VIntEventThread    = NULL;
static HANDLE g_hPwrLRADC            = NULL;
//LQK:Jul-16-2012
static UINT32 gPowerSource			 =1;		//1->USB, 0->wall charger

DWORD SysIntr5V         = 0;
PVOID pv_HWregDIGCTL    = NULL;
PVOID pv_HWregPINCTL    = NULL;
BOOL gbChargerFlag      = FALSE;
BOOL gbMaxCharger       = FALSE;
BOOL gbBattery          = FALSE;
BOOL gbBattNoConnected  = FALSE;
static CRITICAL_SECTION BatteryStatusAccess;
SYSTEM_POWER_STATUS_EX2 gPowerStatus;

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// Local Functions
BOOL WINAPI BatteryDrvrGetStatus( PSYSTEM_POWER_STATUS_EX2,PBOOL );
static VOID UpdateBatteryStatus(VOID);
BOOL BatteryStopCharger(VOID);
BOOL BatterySetCharger(DWORD voltage,DWORD current);
BOOL BatteryChargeDetectThreadProc(LPVOID lpParam);


//-----------------------------------------------------------------------------
//
//  Function: PowerGetDieTemperature
//
//  This routine get the die temperature for battery module.
//
//  Parameters:
//  Returns:
//          returns the die temperature.
//
//-----------------------------------------------------------------------------
UINT32 BattGetDieTemperature()
{
    UINT32 rtn = 0;

    while((rtn <= ABSOLUTE_TEMPERATURE_VALUE) || (rtn >= HIGH_TEMPERATURE_VALUE))    
        rtn = LRADCEnableDieMeasurement(g_hPwrLRADC);
    //RETAILMSG(TRUE,(TEXT("rtn = %d\r\n"),rtn));
    return rtn;
}
//-----------------------------------------------------------------------------
//
//  Function: BattGetVDD5V
//
//  This routine get the VDD5V for battery module.
//
//  Parameters:
//  Returns:
//          returns the VDD5V voltage.
//
//-----------------------------------------------------------------------------
UINT32 BattGetVDD5V()
{
    UINT32 rtn = 0;

    while((rtn <= VDD5V_MIN) || (rtn >= VDD5V_MAX))    
        rtn = LRADCEnableVDD5VMeasurement(g_hPwrLRADC);
    //RETAILMSG(TRUE,(TEXT("rtn = %d\r\n"),rtn));
    return rtn;
}

//-----------------------------------------------------------------------------
//
//  Function:  IsBatteryGood
//
//  This function is used to detect whether there is a battery.
//
//  Parameters:
//          
//  Returns:
//          TRUE if success,and FALSE if fail.
//
//-----------------------------------------------------------------------------
BOOL IsBatteryGood()
{
    UINT32 BattVoltage = 0;
    
    PmuGetBatteryVoltage(&BattVoltage);
    if((BattVoltage > BATTERY_LOW) && (BattVoltage < BATTERY_HIGH))
        return TRUE;

    return FALSE;

}

//-----------------------------------------------------------------------------
//
//  Function: UpdateBatteryStatus
//
//  This function fills in the structures pointed to by its parameters
//  and returns TRUE if successful.  If there's an error, it returns FALSE
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
static VOID UpdateBatteryStatus(VOID)
{
    PSYSTEM_POWER_STATUS_EX2 ppowerstatus = &gPowerStatus;
    UINT32 u32MaxChargeCurrent = MAX_CURRENT_LEVEL_WALL_MIN;
    PMU_POWER_SUPPLY_MODE PowerMode;

	//JLY05-2012:LQK
#ifdef EM9283
	BOOL bSetCharger;
#endif
    
    //RETAILMSG(1,(TEXT("UpdateBatteryStatus ++\r\n")));

    // Update power status structure
    EnterCriticalSection(&BatteryStatusAccess);
 
    //DumpPowerRegisters();
    PmuPowerGetSupplyMode(&PowerMode);
    //RETAILMSG(1,(TEXT("Battery PmuPowerSupplyMode = %d ++\r\n"), PowerMode));
    
    if(PowerMode == PMU_POWER_SUPPLY_5V)
        gfACOnline = TRUE; 
    else
        gfACOnline = FALSE;

	//JLY05-2012:LQK
#ifndef EM9283
    // range ~3000 - 4200 mV
    PmuGetBatteryChargingStatus(&gfBatteryCharging);
#endif   //EM9283

    //RETAILMSG(1,(TEXT("Battery PmuGetBatteryChargingStatus = %d ++\r\n"), gfBatteryCharging)); 

    ppowerstatus->ACLineStatus = gfACOnline ? AC_LINE_ONLINE : AC_LINE_OFFLINE;

    //RETAILMSG(1,(TEXT("+++gfBatteryCharging = %d\r\n"),gfBatteryCharging));

    if(dwUpdateVoltageTime >= UPDATE_BATTERY_INFO_COUNT)
    {
        if(BattGetVDD5V() >= VDD5V_FULL_VALID)
		{
#ifndef EM9283	//LQK:Jul-12-2012
			u32MaxChargeCurrent = MAX_CURRENT_LEVEL_WALL_MAX;
#else
			if(gPowerSource)
			{
				u32MaxChargeCurrent = MAX_CURRENT_LEVEL_USB_MAX;
			}
			else
			{
				u32MaxChargeCurrent = MAX_CURRENT_LEVEL_WALL_MAX;
			}
			//RETAILMSG(1, (TEXT("PowerSource: %d\r\n"), gPowerSource ));
#endif
		}
		
        gdwDieTemperature = BattGetDieTemperature();
        //if temperature > 65C,set the charge current to 200mA.
		// LQK:Jul-12-2012
        if(gdwDieTemperature > MID_TEMPERATURE_VALUE)
            u32MaxChargeCurrent = MAX_CURRENT_LEVEL_WALL_MIN;
        BatteryStopCharger();
        gbChargerFlag = FALSE;
        dwUpdateVoltageTime = 0;
        Sleep(500);
        
        PmuGetBatteryVoltage(&gBatteryVoltage);     // Should return a value in
        //RETAILMSG(1,(TEXT("!!!Battery gBatteryVoltage = %d\r\n"),gBatteryVoltage));
        //RETAILMSG(1,(TEXT("gdwDieTemperature = %d\r\n"),gdwDieTemperature - 273));
	// JLY05-2012: LQK
#ifdef EM9283
        if(gfACOnline && gBatteryVoltage <= gdwBatteryMaxVoltage )
        {
            if(gBatteryVoltage <= MAX_CHARGE_BATTERY_VOLTAGE )
            {
                bSetCharger = BatterySetCharger(gBatteryVoltage,MIN_CURRENT_LEVEL);
                gbMaxCharger = FALSE;
               // gbChargerFlag = TRUE;
            }
            else 
            {
                bSetCharger = BatterySetCharger(gBatteryVoltage,u32MaxChargeCurrent);
                gbMaxCharger = TRUE;
                //gbChargerFlag = TRUE;
            }
			Sleep(100);

			if( bSetCharger )
			{
				PmuGetBatteryChargingStatus(&gbChargerFlag);
				if( !gbChargerFlag )
					BatteryStopCharger();
			}

			//RETAILMSG(1, (TEXT("Battery Charing Falg: %d, BatteryVoltage: %d\r\n"), gbChargerFlag, gBatteryVoltage));

        }
#else
        if(gfACOnline && gBatteryVoltage <= gdwBatteryMaxVoltage)
        {
            if(gBatteryVoltage <= MAX_CHARGE_BATTERY_VOLTAGE )
            {
                BatterySetCharger(gBatteryVoltage,MIN_CURRENT_LEVEL);
                gbMaxCharger = FALSE;
                gbChargerFlag = TRUE;
            }
            else 
            {
                BatterySetCharger(gBatteryVoltage,u32MaxChargeCurrent);
                gbMaxCharger = TRUE;
                gbChargerFlag = TRUE;
            }
        }
#endif     //EM9283

        if(gfACOnline && (gBatteryVoltage >= ppowerstatus->BatteryVoltage))
        {
             if(ppowerstatus->BatteryLifePercent <=(BYTE)((gBatteryVoltage - gdwBatteryVoltageLow)*100/(COMPLETE_CHARGE_BATTERY_VOLTAGE - gdwBatteryVoltageLow)))
                ppowerstatus->BatteryLifePercent =(BYTE)( (gBatteryVoltage - gdwBatteryVoltageLow)*100/(COMPLETE_CHARGE_BATTERY_VOLTAGE - gdwBatteryVoltageLow));
             
        }
        
        else if((!gfACOnline) && (gBatteryVoltage <= ppowerstatus->BatteryVoltage))
        {           
            if(ppowerstatus->BatteryLifePercent >=(BYTE)((gBatteryVoltage - gdwBatteryVoltageLow)*100/(COMPLETE_CHARGE_BATTERY_VOLTAGE - gdwBatteryVoltageLow)))
                ppowerstatus->BatteryLifePercent =(BYTE)((gBatteryVoltage - gdwBatteryVoltageLow)*100/(COMPLETE_CHARGE_BATTERY_VOLTAGE - gdwBatteryVoltageLow));
        }
    
        if(gfACOnline && (gBatteryVoltage >= COMPLETE_CHARGE_BATTERY_VOLTAGE))              
            ppowerstatus->BatteryLifePercent = 100;

        if(ppowerstatus->BatteryLifePercent == 100)
            gbChargerFlag = FALSE;
        
        if(gBatteryVoltage < gdwBatteryVoltageLow)
            ppowerstatus->BatteryLifePercent = 0;    
    
        ppowerstatus->BatteryVoltage            = gBatteryVoltage;
    }

    if(ppowerstatus->BatteryLifePercent >= 65)
        ppowerstatus->BatteryFlag           = BATTERY_FLAG_HIGH;
    else
    {
        if(ppowerstatus->BatteryLifePercent >= 20)
            ppowerstatus->BatteryFlag       = BATTERY_FLAG_LOW;
        else
            ppowerstatus->BatteryFlag       = BATTERY_FLAG_CRITICAL;
    }

    ppowerstatus->Reserved1                 = 0;
    ppowerstatus->BatteryLifeTime           = BATTERY_LIFE_UNKNOWN;
    ppowerstatus->BatteryFullLifeTime       = BATTERY_LIFE_UNKNOWN;

    ppowerstatus->Reserved2                  = 0;
    ppowerstatus->BackupBatteryFlag          = BATTERY_FLAG_UNKNOWN;
    ppowerstatus->BackupBatteryLifePercent   = BATTERY_PERCENTAGE_UNKNOWN;
    ppowerstatus->Reserved3                  = 0;
    ppowerstatus->BackupBatteryLifeTime      = BATTERY_LIFE_UNKNOWN;
    ppowerstatus->BackupBatteryFullLifeTime  = BATTERY_LIFE_UNKNOWN;

    ppowerstatus->BatteryChemistry           = BATTERY_CHEMISTRY_LION;
    ppowerstatus->BatteryCurrent             = 0;
    ppowerstatus->BatteryAverageCurrent      = 0;
    ppowerstatus->BatteryAverageInterval     = 0;
    ppowerstatus->BatterymAHourConsumed      = 0;
    ppowerstatus->BatteryTemperature         = 0;
    ppowerstatus->BackupBatteryVoltage       = 0;

    if(gbChargerFlag)
        ppowerstatus->BatteryFlag    = BATTERY_FLAG_CHARGING;
    
    LeaveCriticalSection(&BatteryStatusAccess);
    //RETAILMSG(1,(TEXT("UpdateBatteryStatus --\r\n")));

}


//-----------------------------------------------------------------------------
//
//  Function: BatteryPDDInitialize
//
//  This function initializes the Battery module
//
//  Parameters:
//          pszRegistryContext
//              [in]  registry handle to the Battery module
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL WINAPI
BatteryPDDInitialize(LPCTSTR pszRegistryContext)
{
    PSYSTEM_POWER_STATUS_EX2 ppowerstatus = &gPowerStatus;
    HKEY hKey = NULL;    
    //POWER_INITVALUES PowerInitVal;
    //DWORD irq = 0;
    TCHAR lpDevName[] = TEXT("LDC1:");
    UINT32 rtn;
    ULONG Status = 0;
    UINT32 SampleInterval;
    PHYSICAL_ADDRESS phyAddr;

	// debug only
    RETAILMSG(1, (TEXT("->BatteryPDDInitialize\r\n")));

    if (pv_HWregDIGCTL == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_DIGCTL;
        pv_HWregDIGCTL = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregDIGCTL == NULL)
        {
            RETAILMSG(1, (TEXT("InitializeTransceiver::MmMapIoSpace failed for pv_HWregDIGCTL\r\n")));
        }
    }
    
    if (pv_HWregPINCTL == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_PINCTRL;
        pv_HWregPINCTL = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregPINCTL == NULL)
        {
            RETAILMSG(1, (TEXT("InitializeTransceiver::MmMapIoSpace failed for pv_HWregPINCTL\r\n")));
        }
    }

    //RETAILMSG(1,(TEXT("BatteryPDDInitialize ++\r\n")));
    
    // Initialize the global variables
    gdwBatteryMaxVoltage          = DEFAULT_BATTERY_MAX_VOLTAGE;
    gdwBatteryVoltageHigh         = DEFAULT_BATTERY_VOLTAGE_HIGH;
    gdwBatteryVoltageLow          = DEFAULT_BATTERY_VOLTAGE_LOW;

    InitializeCriticalSection(&BatteryStatusAccess);

    memset(&gPowerStatus,0,sizeof(gPowerStatus));

    // get registry values, if present, for the voltage thresholds
    hKey = OpenDeviceKey(pszRegistryContext);
    if(hKey != NULL)
    {
        DWORD dwSize, dwStatus, dwType, dwValue;

        dwSize = sizeof(dwValue);
        dwStatus = RegQueryValueEx(hKey, BATTERY_MAX_VOLTAGE_TEXT, NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD)
        {
            gdwBatteryMaxVoltage = (DWORD) dwValue;
        }

        dwSize = sizeof(dwValue);
        dwStatus = RegQueryValueEx(hKey, BATTERY_VOLTAGE_HIGH_TEXT, NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD)
        {
            gdwBatteryVoltageHigh = (DWORD) dwValue;
        }

        dwSize = sizeof(dwValue);
        dwStatus = RegQueryValueEx(hKey, BATTERY_VOLTAGE_LOW_TEXT, NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_DWORD)
        {
            gdwBatteryVoltageLow = (DWORD) dwValue;
        }

        RegCloseKey(hKey);
    }

    //Open the handlle to the LRADC module
    g_hPwrLRADC = LRADCOpenHandle((LPCWSTR)lpDevName);
    if(!g_hPwrLRADC)
    {
        //Error opening the Handle of LRADC module
        ERRORMSG(1, (_T("PowerInit :: LRADCOpenHandle failed with error %d!"),GetLastError()));
      
    }

    SampleInterval = 200;

    PmuGetBatteryVoltage(&gBatteryVoltage);
    RETAILMSG(1,(TEXT("Deteced gBatteryVoltage = %d\r\n"),gBatteryVoltage));
	
	//LQK:Jul-16-2012
	PmuGetPowerSource( &gPowerSource );

    //if(IsBatteryGood() && (gBatteryVoltage < COMPLETE_CHARGE_BATTERY_VOLTAGE))
	//JLY05-2012: LQK lqk 2010-5-25 
	/*if( IsBatteryGood() )
        gbBattery = TRUE;*/

	//Lqk:Jul-25-2012
	PmuIsBatteryAttach( &gbBattery );

    if(gBatteryVoltage == 0)
        gbBattNoConnected = TRUE;
     // Start up the battery measurements
    rtn = LRADCEnableBatteryMeasurement(g_hPwrLRADC,LRADC_DELAY_TRIGGER3,(UINT16)SampleInterval);
    if(rtn != 0)
    {
        return rtn;
    }

    // Initialize the battery monitor.
    // Acquire the delay trigger assigned for battery monitoring and
    // parse the init structure for the sampling interval.
    // Set up the battery monitor and return if unsuccessful.
    if((Status = PmuInitBatteryMonitor()) != TRUE)
        return Status;

    // Create an event for the 5V detect event
    g_h5VIntEvent = CreateEvent(NULL, FALSE, FALSE, g_5VDetectEventName);
    if(g_h5VIntEvent == NULL)
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to create 5V interrupt event\r\n")));
        return FALSE;
    }

    g_h5VIntEventThread = CreateThread(0, 0, 
                                    (LPTHREAD_START_ROUTINE)BatteryChargeDetectThreadProc, 
                                    NULL, 0, NULL);
    if (g_h5VIntEventThread == NULL)
    {
        RETAILMSG(1, (TEXT("hChargeDetectThread : CreateThread Failed!\r\n")));
    }

    // for Priority of BatteryThreadProc
    CeSetThreadPriority(g_h5VIntEventThread, CE_THREAD_PRIO_256_HIGHEST);
 
    //PmuGetBatteryVoltage(&gBatteryVoltage);
    //RETAILMSG(1,(TEXT("---gBatteryVoltage = %d\r\n"),gBatteryVoltage));

    ppowerstatus->BatteryLifePercent =(BYTE)( (gBatteryVoltage - gdwBatteryVoltageLow)*100/(COMPLETE_CHARGE_BATTERY_VOLTAGE - gdwBatteryVoltageLow));
    ppowerstatus->BatteryVoltage = gBatteryVoltage;

	// debug only
    RETAILMSG(1,(TEXT("!!!Detected Battery Voltage = %d, gbBattery = %d\r\n"),gBatteryVoltage,gbBattery));
    RETAILMSG(1, (TEXT("<-BatteryPDDInitialize\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BatteryPDDDeinitialize
//
//  This function initializes the Battery module
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void WINAPI
BatteryPDDDeinitialize(void)
{
    SETFNAME(_T("BatteryPDDDeinitialize"));

    DEBUGMSG(ZONE_PDD, (_T("%s: invoked\r\n"), pszFname));

    //Close Event handle if open
    if(g_hPwrLRADC)
        LRADCCloseHandle(g_hPwrLRADC);
    
    if(g_h5VIntEvent)
        CloseHandle(g_h5VIntEvent);

    //Close the thread handle if exists
    if(g_h5VIntEventThread)
        CloseHandle(g_h5VIntEventThread);  

    DeleteCriticalSection(&BatteryStatusAccess);
}

//-----------------------------------------------------------------------------
//
//  Function: BatteryPDDResume
//
//  This function performs hardware-specific battery processing after system resume
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void WINAPI
BatteryPDDResume(void)
{
    SETFNAME(_T("BatteryPDDResume"));
    //RETAILMSG(1,(TEXT("BatteryPDDResume ++\r\n")));

    DEBUGMSG(ZONE_PDD, (_T("%s: invoked\r\n"), pszFname));
}

//-----------------------------------------------------------------------------
//
//  Function: BatteryPDDPowerHandler
//
//  This function performs hardware-specific processing for the battery driver
//
//  Parameters:
//          pszRegistryContext
//              [in]  TRUE for Power ON and FALSE for Power Off
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
void WINAPI
BatteryPDDPowerHandler(BOOL bOff)
{
    SETFNAME(_T("BatteryPDDPowerHandler"));

    UNREFERENCED_PARAMETER(bOff);

    DEBUGMSG(ZONE_PDD | ZONE_RESUME, (_T("%s: invoked w/ bOff %d\r\n"), pszFname, bOff));
}

//-----------------------------------------------------------------------------
//
//  Function: BatteryPDDGetStatus
//
//  This routine obtains the most current battery/power status available
//  on the platform.  It fills in the structures pointed to by its parameters
//  and returns TRUE if successful.  If there's an error, it returns FALSE.
//
//  Parameters:
//          pstatus
//              [out]  pointer to PSYSTEM_POWER_STATUS_EX2 structure
//          pfBatteriesChangedSinceLastCall
//              [out]  pointer to Boolean value which indicates a change
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL WINAPI
BatteryPDDGetStatus(
    PSYSTEM_POWER_STATUS_EX2 pstatus,
    PBOOL pfBatteriesChangedSinceLastCall
    )
{
    BOOL fRetVal = FALSE;
    PMU_POWER_SUPPLY_MODE PowerMode;
    SETFNAME(_T("BatteryPDDGetStatus"));


    //RETAILMSG(1,(TEXT("BatteryPDDGetStatus ++\r\n")));

    DEBUGMSG(ZONE_PDD, (_T("%s: invoked w/ pstatus 0x%08x, pfChange 0x%08x\r\n"),
                        pszFname, pstatus, pfBatteriesChangedSinceLastCall));

    if(pstatus)
    {

        if(gbBattery)
        {
            dwUpdateVoltageTime++;
            UpdateBatteryStatus();
            memcpy(pstatus,&gPowerStatus,sizeof(gPowerStatus));
        }
        else
        {
            PmuPowerGetSupplyMode(&PowerMode);           
            //RETAILMSG(1,(TEXT("PmuPowerSupplyMode %d --\r\n"), PowerMode));
            
            if(PowerMode == PMU_POWER_SUPPLY_5V)
                gfACOnline = TRUE; 
            else 
                gfACOnline = FALSE;

            pstatus->ACLineStatus = gfACOnline ? AC_LINE_ONLINE : AC_LINE_OFFLINE;
#ifdef EM9283 //LQK: Aug 21, 2012
			pstatus->BatteryFlag = BATTERY_FLAG_NO_BATTERY;
			pstatus->BatteryLifePercent = 0;
#else
			pstatus->BatteryFlag = BATTERY_FLAG_HIGH;
			pstatus->BatteryLifePercent = 100;
#endif //ifdef EM9283
            pstatus->BackupBatteryFlag  =  BATTERY_FLAG_UNKNOWN;
            pstatus->BackupBatteryLifePercent = BATTERY_PERCENTAGE_UNKNOWN;
            pstatus->BatteryVoltage = 0; 
            pstatus->BatteryCurrent = 0;
            pstatus->BatteryAverageCurrent = 0;
            pstatus->BatterymAHourConsumed = (DWORD)-1;
            pstatus->BatteryTemperature = 0;
            pstatus->BackupBatteryVoltage = 0;
            pstatus->BatteryChemistry = BATTERY_CHEMISTRY_UNKNOWN;

            if(gbBattNoConnected == TRUE)
            {        
                pstatus->BatteryLifePercent   = 0;
#ifndef EM9283		//LQK:Jul-12-2012 
                RETAILMSG(1,(TEXT("Warning:Battery is lost,please connect battery and reboot!!!\r\n")));    
#endif
            }
        }
        
        *pfBatteriesChangedSinceLastCall = FALSE;
        fRetVal = TRUE;
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    //RETAILMSG(1,(TEXT("BatteryPDDGetStatus --\r\n")));
    return fRetVal;
}

//-----------------------------------------------------------------------------
//
//  Function: BatteryPDDGetLevels
//
// This routine indicates how many battery levels will be reported
// in the BatteryFlag and BackupBatteryFlag fields of the PSYSTEM_POWER_STATUS_EX2
// filed in by BatteryPDDGetStatus().  This number ranges from 0 through 3 --
// see the Platform Builder documentation for details.  The main battery
// level count is reported in the low word of the return value; the count
// for the backup battery is in the high word.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns a long value which represent the battery levels.
//
//-----------------------------------------------------------------------------
LONG
BatteryPDDGetLevels(void)
{
    LONG lLevels;
    SETFNAME(_T("BatteryPDDPowerHandler"));

    //RETAILMSG(1,(TEXT("BatteryPDDGetLevels ++\r\n")));

    lLevels = MAKELONG (3 /* main battery levels   */,
                        0 /* backup battery levels */);

    DEBUGMSG(ZONE_PDD, (_T("%s: returning %u (%d main levels, %d backup levels)\r\n"),
                        pszFname, lLevels, LOWORD(lLevels), HIWORD(lLevels)));

    //RETAILMSG(1,(TEXT("BatteryPDDGetLevels --\r\n")));

    return lLevels;
}

//-----------------------------------------------------------------------------
//
//  Function: BatteryPDDGetLevels
//
// This routine returns TRUE to indicate that the pfBatteriesChangedSinceLastCall
// value filled in by BatteryPDDGetStatus() is valid.  If there is no way to
// tell that the platform's batteries have been changed this routine should
// return FALSE.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL
BatteryPDDSupportsChangeNotification(void)
{
    BOOL fSupportsChange;
    SETFNAME(_T("BatteryPDDPowerHandler"));

    //RETAILMSG(1,(TEXT("BatteryPDDSupportsChangeNotification ++\r\n")));
    fSupportsChange = FALSE;

    DEBUGMSG(ZONE_PDD, (_T("%s: returning %d\r\n"), pszFname, fSupportsChange));
    //RETAILMSG(1,(TEXT("BatteryPDDSupportsChangeNotification --\r\n")));

    return fSupportsChange;
}

//-----------------------------------------------------------------------------
//
//  Function: BatterySetCharger
//
//  This routine set the PMU to start charging battery.
//
//  Parameters:
//          voltage:current battery voltage.
//          current:the charging current.
//  Returns:
//          returns TRUE if success,FALSE if fail.
//
//-----------------------------------------------------------------------------
BOOL BatterySetCharger(DWORD voltage,DWORD current)
{
    if(gdwDieTemperature > DIETEMPSTOPCHARGER)  //if die temperature > 70C
    {
        RETAILMSG(TRUE, (TEXT("WARNING:The SOC DIE temperature is %d C ,too high,stop charging...\r\n"),gdwDieTemperature - ABSOLUTE_TEMPERATURE_VALUE));
        return FALSE;
    }

    if (current > MAX_SAFE_CURRENT_LEVEL)  
    {
        ERRORMSG(TRUE, (TEXT("BSPBattdrvrSetCharger fail, the current set = %d is so large\r\n"), current));
        return FALSE;
    }

    if ((voltage > gdwBatteryMaxVoltage))  
    {
        ERRORMSG(TRUE, (TEXT("BSPBattdrvrSetCharger fail, the voltage set = %d is so large\r\n"), voltage));
        return FALSE;
    }

    PmuSetCharger(current);

    return TRUE;
   
}

//-----------------------------------------------------------------------------
//
//  Function: BatteryStopCharger
//
//  This routine set the PMU to stop charging battery.
//
//  Parameters:
//          None.
//  Returns:
//          returns TRUE if success,FALSE if fail.
//
//-----------------------------------------------------------------------------
BOOL BatteryStopCharger(VOID)
{
    PmuStopCharger();
    return TRUE;
}
//-----------------------------------------------------------------------------
//
//  Function: BatteryChargeDetectThreadProc
//
//  This routine wait for the event to update battery status.
//
//  Parameters:
//          lpParam.
//  Returns:
//          returns.
//
//-----------------------------------------------------------------------------
BOOL BatteryChargeDetectThreadProc(LPVOID lpParam)
{
    static HANDLE hUseractivity = NULL;
    hUseractivity = CreateEvent(NULL, FALSE, FALSE,L"PowerManager/ActivityTimer/UserActivity" );

    UNREFERENCED_PARAMETER(lpParam);
    //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    for(;;)
    {
        if (WaitForSingleObject(g_h5VIntEvent, INFINITE) == WAIT_OBJECT_0)
        {
            //RETAILMSG(TRUE, (TEXT("BatteryChargeDetectThreadProc h5VIntEvent!\r\n")));
            dwUpdateVoltageTime = UPDATE_BATTERY_INFO_COUNT;
            UpdateBatteryStatus();
            // Power source change should be treated as User activity.
            SetEvent(hUseractivity);            
        }
    }
}

