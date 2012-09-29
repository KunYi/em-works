//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:  tve_tvdac.cpp
//
// PURPOSE: TVDAC related 
//

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_tve.h"
#include "tve.h"

//-----------------------------------------------------------------------------
//
//  Function: TveGetTVDACSampleRateByClock
//
//  This function gets a TVDAC sample rate using a TVE clock
//
//  Parameters:
//     tveClock
//           [in] a TVE clock
//
//
//  Returns:
//      Returns a vaild TVDAC sample rate if successful, 
//      otherwise returns SAMPLING_RATE_NONE.
//
//-----------------------------------------------------------------------------
TVE_TVDAC_SAMPLING_RATE TveClass::TveGetTVDACSampleRateByClock(DWORD tveClock)
{
    TVE_TVDAC_SAMPLING_RATE sampleRate = SAMPLING_RATE_NONE;
    
    switch (tveClock)
    {
        case TVE_S216_CLK:  
            sampleRate = SAMPLING_RATE_216MHZ;
            break;
                                  
        case TVE_S108_CLK:
            sampleRate = SAMPLING_RATE_108MHZ;
            break;
                                    
        case TVE_S54_CLK:
            sampleRate = SAMPLING_RATE_54MHZ;
            break;
                                     
        case TVE_S27_CLK:
            sampleRate = SAMPLING_RATE_27MHZ;
            break;
        default:
            ERRORMSG(1, (_T("Invaild TVE clock!\r\n")));
            break;                         
    }
    
    return sampleRate;
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetClockBySamplingRate
//
//  This function sets TVE clock by TVDAC sampling rate.
//
//  Parameters:
//     sampleRate
//           [in] TVDAC sample rate
//
//  Returns:
//     None.
//
//-----------------------------------------------------------------------------
void TveClass::TveSetClockBySamplingRate(TVE_TVDAC_SAMPLING_RATE sampleRate)
{
    // Set TVE clock based on TVDAC sampling rate
    switch (sampleRate)
    {
        case SAMPLING_RATE_216MHZ:
            TveSetClock(TVE_S216_CLK);
            break;
        case SAMPLING_RATE_108MHZ:
            TveSetClock(TVE_S108_CLK);  
            break;
        case SAMPLING_RATE_54MHZ:
            TveSetClock(TVE_S54_CLK); 
            break;
        case SAMPLING_RATE_27MHZ:
            TveSetClock(TVE_S27_CLK);
            break;
        default:
            ERRORMSG(1, (_T("Invaild sample rate, set clock failed!\r\n")));
            break;      
    }

}

