//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pmic_adc.cpp
//
//  This file contains the PMIC ADC (A/D Converter, including Touch Screen) SDK 
//  interface that is used by applications and other drivers to access registers 
//  of the PMIC.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "pmic_lla.h"
#include "pmic_ioctl.h"
#include "pmic_adc.h"

#include "regs.h"
#include "regs_adc.h"


//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
extern HANDLE hPMI;

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

extern DBGPARAM dpCurSettings;

#endif  // DEBUG

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: PmicADCInit
//
// This function initializes PMIC ADC's resources.
//
// Parameters:
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCInit(void)
{

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicADCDeinit
//
// This function uninitializes the PMIC ADC's resources.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PmicADCDeinit(void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
}

//------------------------------------------------------------------------------
//
// Function: PmicADCSetComparatorThresholds
//
// This function sets WHIGH and WLOW for automatic ADC result comparators.  
//
// Parameters:
//
//      whigh 
//              [in]  a high comparator threshold (<0x3FF). 
//      wlow 
//              [in]  a low comparator threshold (<0x3FF). 
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCSetComparatorThresholds(UINT16 whigh, UINT16 wlow)
{
    PMIC_STATUS rc = PMIC_SUCCESS;
    UINT16 threshold[2];


    if (whigh > MC13783_ADC_MAX_COMPARATOR_LEVEL)
        whigh = MC13783_ADC_MAX_COMPARATOR_LEVEL;

    if (whigh <= wlow)
    {
        ERRORMSG(1, (_T("PmicADCSetComparatorThresholds Invalid Parameter!\r\n")));
        rc = PMIC_PARAMETER_ERROR;
    }
    else if (( whigh < 0x3FF )&&(wlow < 0x3FF))
    {
        threshold[0] = whigh;
        threshold[1] = wlow;

        DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

        if (!DeviceIoControl(hPMI, PMIC_IOCTL_ADC_SET_CMPTR_TRHLD, threshold,
                    sizeof(threshold), NULL, 0, NULL, NULL))
        {
            ERRORMSG(1, (_T("PMIC_IOCTL_ADC_SET_CMPTR_TRHLD failed!\r\n")));
            rc = PMIC_ERROR;
        }
    }
    else
    {
        ERRORMSG(1, (_T("Parameter out of range!\r\n")));
        rc = PMIC_PARAMETER_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetSingleChannelOneSample
//
// This function gets one channel one sample.
//
// Parameters
//
//      channel
//              [in]    a selected channel.
//      pResult
//              [out]   pointer to  the sampled value.
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCGetSingleChannelOneSample(UINT16 channels, UINT16* pResult)
{
    PMIC_STATUS rc = PMIC_SUCCESS;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (!DeviceIoControl(hPMI, PMIC_IOCTL_ADC_GET_SGL_CH_1SPL, &channels,
                sizeof(channels), pResult, sizeof(UINT16), NULL, NULL))
    {
        ERRORMSG(1, (_T("PMIC_IOCTL_ADC_GET_SGL_CH_1SPL failed!\r\n")));
        rc = PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetSingleChannelEightSamples
//
// This function gets one channel eight samples.
//
// Parameters:
//
//  channels
//      [in]  a selected channel. 
//  pResult
//      [out] pointer to  the sampled values (up to 8 sampled values).
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCGetSingleChannelEightSamples(UINT16 channels, UINT16* pResult)
{
    PMIC_STATUS rc = PMIC_SUCCESS;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (!DeviceIoControl(hPMI, PMIC_IOCTL_ADC_GET_SGL_CH_8SPL, &channels,
                sizeof(channels), pResult, sizeof(UINT16), NULL, NULL))
    {
        ERRORMSG(1, (_T("PMIC_IOCTL_ADC_GET_SGL_CH_8SPL failed!\r\n")));
        rc = PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetMultipleChannelsSamples
//
// This function gets one sample for multiple channels.
//
// Parameters:
//
//      channels
//      [in] selected  channels (up to 16 channels).
//      pResult
//      [out] pointer to  the sampled values (up to 16 sampled values).
//            The returned results will be ordered according to the channel
//            selected.
//
// Returns:
//      Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCGetMultipleChannelsSamples(UINT16 channels, UINT16* pResult)
{
    PMIC_STATUS rc = PMIC_SUCCESS;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (!DeviceIoControl(hPMI, PMIC_IOCTL_ADC_GET_MUL_CH_SPL, &channels,
                sizeof(channels), pResult, sizeof(UINT16), NULL, NULL))
    {
        ERRORMSG(1, (_T("PMIC_IOCTL_ADC_GET_MUL_CH_SPL failed!\r\n")));
        rc = PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetHandsetCurrent
//
// This function gets handset battery current measurement values.
//
// Parameters:
//
//      mode
//              [in]     a ADC converter mode: ADC_8CHAN_1X or ADC_1CHAN_8X.
//      pResult
//              [out]    pointer to the handset battery current measurement
//                       value(s).
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCGetHandsetCurrent(PMIC_ADC_CONVERTOR_MODE mode,
                                     UINT16* pResult)
{
    PMIC_STATUS rc = PMIC_SUCCESS;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (!DeviceIoControl(hPMI, PMIC_IOCTL_ADC_GET_HS_CURRENT, &mode,
                sizeof(mode), pResult, sizeof(UINT16), NULL, NULL))
    {
        ERRORMSG(1, (_T("PMIC_IOCTL_ADC_GET_HS_CURRENT failed!\r\n")));
        rc = PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCTouchRead
//
// This function reads 3 pairs of  touch screen sample.
//
// Parameters:
//      x
//          [out] X-coordinate of the point.
//      y
//          [out] Y-coordinate of the point.
//
// Returns:
//      Status.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCTouchRead(UINT16* x, UINT16* y)
{
    PMIC_STATUS rc = PMIC_SUCCESS;
    UINT16 xy[8] = {0,0,0,0,0,0,0,0};

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (!DeviceIoControl(hPMI, PMIC_IOCTL_ADC_TOUCH_READ, NULL, 0,
          xy, sizeof(xy), NULL, NULL))
    {
        ERRORMSG(1, (_T("PMIC_IOCTL_ADC_TOUCH_READ failed!\r\n")));
        rc = PMIC_ERROR;
    }
    else
    {
        for (int i=0; i<6; i++)
        {
            if (i<3)
                *x++ = xy[i];
            else
                *y++ = xy[i];
        }
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s() 0x%x, 0x%x\r\n"), __WFUNCTION__, *x, *y));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: PmicTouchStandby
//
//  This function causes the PMIC touch screen controller to enter standby
//  mode and wait for the next pen down condition.
//
//  Parameters:
//      intEna
//          [in] pen down interrupt enable.
//
//  Returns:
//      Status.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCTouchStandby(BOOL intEna)
{
    PMIC_STATUS rc = PMIC_SUCCESS;
    PMIC_TOUCH_MODE mode;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (intEna)
        mode = TM_INTERRUPT;
    else
        mode = TM_INACTIVE;

    if (!DeviceIoControl(hPMI, PMIC_IOCTL_ADC_SET_MODE, &mode, sizeof(mode),
          NULL, 0, NULL, NULL))
    {
        ERRORMSG(1, (_T("PMIC_IOCTL_ADC_SET_MODE failed!\r\n")));
        rc = PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return rc;
}
