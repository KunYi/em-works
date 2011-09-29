//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "common_tvev2.h"
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
    
    if (m_bHD)
    {
        switch (tveClock)
        {
            case TVE_S297_CLK:
                sampleRate = HD_SAMPLING_RATE_297MHZ;
                break;
           // case TVE_S148_5_CLK:
           //   sampleRate = HD_SAMPLING_RATE_148_5MHZ;  
           //   break;
            default:
                ERRORMSG(1, (_T("Invaild TVEV2 HD clock, get HD clock failed!\r\n")));
                break;      
        }

    }
    else
    {
        switch (tveClock)
        {
            case TVE_S216_CLK:  
                sampleRate = SD_SAMPLING_RATE_216MHZ; 
                break;
                                  
            case TVE_S108_CLK:
                sampleRate = SD_SAMPLING_RATE_108MHZ;
                break;
                                    
            case TVE_S54_CLK:
                sampleRate = SD_SAMPLING_RATE_54MHZ;
                break;
                                     
            case TVE_S27_CLK:
                sampleRate = SD_SAMPLING_RATE_27MHZ;
                break;
            default:
                ERRORMSG(1, (_T("Invaild TVE SD clock, get SD clock failed!!\r\n")));
                break;                         
        }
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
    if (m_bHD)
    {
        switch (sampleRate)
        {
            case HD_SAMPLING_RATE_297MHZ:
                TveSetClock(TVE_S297_CLK);
                break;
           // case HD_SAMPLING_RATE_148_5MHZ:
           //     TveSetClock(TVE_S148_5_CLK);  
           //     break;
            default:
                ERRORMSG(1, (_T("Invaild sample rate, set HD clock failed!\r\n")));
                break;      
        }

    }
    else
    {
        switch (sampleRate)
        {
            case SD_SAMPLING_RATE_216MHZ:
                TveSetClock(TVE_S216_CLK);
                break;
            case SD_SAMPLING_RATE_108MHZ:
                TveSetClock(TVE_S108_CLK);  
                break;
            case SD_SAMPLING_RATE_54MHZ:
                TveSetClock(TVE_S54_CLK); 
                break;
            case SD_SAMPLING_RATE_27MHZ:
                TveSetClock(TVE_S27_CLK);
                break;
            default:
                ERRORMSG(1, (_T("Invaild sample rate, set SD clock failed!\r\n")));
                break;      
        }
    }


}

