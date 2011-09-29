//-----------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  voltctrl.cpp
//
//  Provides PMIC voltage control support for the DVFC driver.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include "bsp.h"

#include "mc34704.h"

//-----------------------------------------------------------------------------
// Types
typedef struct
{
    UINT32 mV;
    BYTE voltCode;
    INT8 signedVoltCode;  
} DVFC_VOLT_CONVERSION;


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
// MC34704 can be adjusted in 2.5% voltage offsets from the nominal voltage.
// To calculate the number of steps, we must determine the percentage 
// difference from the nominal and the calculate the number of 2.5% steps.
#define DVFS_NOMINAL_VOLTAGE_mV     1450
#define DVFS_NOMINAL_VOLTAGE_uV     (DVFS_NOMINAL_VOLTAGE_mV * 1000)
#define DVFS_STEP_VOLTAGE_uV        (DVFS_NOMINAL_VOLTAGE_uV * 25 / 1000)
#define DVFS_MAX_VOLTAGE_uV         (DVFS_NOMINAL_VOLTAGE_uV * 1175 / 1000)
#define DVFS_MIN_VOLTAGE_uV         (DVFS_NOMINAL_VOLTAGE_uV * 80 / 100)

#define DVFS_VOLTCODE_SIGNED(voltCode) (voltCode & 0x8 ? voltCode | 0xF0 : voltCode)

#define DVFC_NUM_VOLT

//-----------------------------------------------------------------------------
// Local Variables
static INT8 g_prevSignedVoltCode;
static DVFC_VOLT_CONVERSION g_voltConvUp;
static DVFC_VOLT_CONVERSION g_voltConvDown;


//-----------------------------------------------------------------------------
//
//  Function:  DvfcInitVoltageSupplies
//
//  This function updates the core supply voltage for the system. 
//
//  Parameters:
//      None
//
//  Returns:
//      TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DvfcInitVoltageSupplies(VOID)
{
    BOOL rc = FALSE;
    BYTE voltCode;
    
    if (!PmicOpen())
    {
        ERRORMSG(TRUE, (_T("PmicOpen failed!\r\n")));
        goto cleanUp;
    }
    

    // Query the current voltage setpoints
    if (!PmicRegulatorGetVoltageLevel(MC34704_REGULATOR3, &voltCode))
    {
        ERRORMSG(TRUE, (_T("PmicRegulatorGetVoltageLevel failed!\r\n")));
        goto cleanUp;
    }

    g_prevSignedVoltCode = DVFS_VOLTCODE_SIGNED(voltCode);

    rc = TRUE;
    
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  DvfcConvertVoltage
//
//  This function converts the core supply voltage for the system. 
//
//  Parameters:
//      mV
//          [in]  Specifies the core voltage in mV.
//
//  Returns:
//      Returns the delay is usec that is required to reach the specified
//      voltage setting.  INFINITE is returned if an error has occurred while
//      attempting to set the new voltage.
//
//-----------------------------------------------------------------------------
BYTE DvfcConvertVoltage(UINT32 mV)
{
    UINT32 uV = mV * 1000;
    BYTE voltCode = 0;
    UINT32 uVnew;

    if (uV <= (DVFS_NOMINAL_VOLTAGE_uV - DVFS_STEP_VOLTAGE_uV))
    {
        uVnew = DVFS_NOMINAL_VOLTAGE_uV - DVFS_STEP_VOLTAGE_uV;
        for (voltCode = 0xF ; voltCode > 0x8; voltCode--)
        {
            uVnew -= DVFS_STEP_VOLTAGE_uV;
            if (uVnew < uV) break;
        }        
    }
    else if (uV > DVFS_NOMINAL_VOLTAGE_uV)
    {
        uVnew = DVFS_NOMINAL_VOLTAGE_uV + DVFS_STEP_VOLTAGE_uV;
        for (voltCode = 0x1 ; voltCode < 0x7; voltCode++)
        {
            if (uVnew >= uV) break;
            uVnew += DVFS_STEP_VOLTAGE_uV;
        }        
    }

    return voltCode;
}


//-----------------------------------------------------------------------------
//
//  Function:  DvfcUpdateSupplyVoltage
//
//  This function updates the core supply voltage for the system. 
//
//  Parameters:
//      mV
//          [in]  Specifies the core voltage in mV.
//
//      dvs
//          [in]  Specifies the value for the DVS signals that can be optionally
//                used to select a PMIC voltage setting.
//
//      domain
//          [in] Specifies DVFC domain.
//
//  Returns:
//      Returns the delay is usec that is required to reach the specified
//      voltage setting.  INFINITE is returned if an error has occurred while
//      attempting to set the new voltage.
//
//-----------------------------------------------------------------------------
UINT32 DvfcUpdateSupplyVoltage(UINT32 mV, UINT32 dvs, DDK_DVFC_DOMAIN domain)
{
    UINT32 usDelay = INFINITE;
    BYTE newVoltCode;
    INT8 signedVoltCode;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dvs);
    UNREFERENCED_PARAMETER(domain);

    if (g_voltConvUp.mV == mV)
    {
        newVoltCode = g_voltConvUp.voltCode;
        signedVoltCode = g_voltConvUp.signedVoltCode;
    }
    else if (g_voltConvDown.mV == mV)
    {
        newVoltCode = g_voltConvDown.voltCode;
        signedVoltCode = g_voltConvDown.signedVoltCode;
    }
    else
    {
        newVoltCode = DvfcConvertVoltage(mV);
        signedVoltCode = DVFS_VOLTCODE_SIGNED(newVoltCode);        
    }
        
    // Update the voltage setpoint
    if (!PmicRegulatorSetVoltageLevel(MC34704_REGULATOR3, newVoltCode))
    {
        goto cleanUp;
    }


    // If we are raising the voltage, return the applicable delay
    if (signedVoltCode > g_prevSignedVoltCode)
    {
        usDelay = (signedVoltCode - g_prevSignedVoltCode) * MC34704_DVS_STEP_DELAY;
        g_voltConvUp.mV = mV;
        g_voltConvUp.voltCode = newVoltCode;
        g_voltConvUp.signedVoltCode = signedVoltCode;
    }
    else
    {
        usDelay = 0;
        g_voltConvDown.mV = mV;
        g_voltConvDown.voltCode = newVoltCode;
        g_voltConvDown.signedVoltCode = signedVoltCode;
    }

    g_prevSignedVoltCode = signedVoltCode;

cleanUp:

    return usDelay;
}

