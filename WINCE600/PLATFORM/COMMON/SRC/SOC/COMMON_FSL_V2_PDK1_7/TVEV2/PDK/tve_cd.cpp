//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:  tve_cd.cpp
//
// PURPOSE: TVE cable detection 
//

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_tvev2.h"
#include "tve.h"

//-----------------------------------------------------------------------------
//
//  Function: TveIsCableDetected
//
//  Check if the TVE cables are detected.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------   
BOOL TveClass::TveIsCableDetected()
{
    UINT32 statReg = 0;
    BOOL bRet = FALSE;
    BOOL bChan0 = FALSE, bChan1 = FALSE, bChan2 = FALSE;       
/*
    // check CD connection events
    if (m_hTveCdLmEvent)
    {
        if(WAIT_TIMEOUT == WaitForSingleObject(m_hTveCdLmEvent, TVE_WAIT_TIMEOUT))
        {   
            ERRORMSG(TRUE, (TEXT("Check CD LM event failed on timeout!\r\n")));
            return bRet;
        }
    }
    
    if (m_hTveCdSmEvent)
    {
        if(WAIT_TIMEOUT == WaitForSingleObject(m_hTveCdSmEvent, TVE_WAIT_TIMEOUT))
        {   
            ERRORMSG(TRUE, (TEXT("Check CD SM event failed on timeout!\r\n")));
            return bRet;
        }
    }
  */ 
    // Read the TVE Status register
    statReg = INREG32(&m_pTVE->STAT_REG);
    
    // determine CD connection depending on 8 TV OUT Modes
    switch (m_eTVOutputMode) {
        case TV_OUT_DISABLE:
            break;
            
        case TV_OUT_COMPOSITE_CH0:
            // Channel 0 LOADED
            if ((statReg & 0x00001100) == 0x00000100)
            {
                // Channel0 connector is loaded
                DEBUGMSG(1, (TEXT("%s: Channel0 connector is loaded! \r\n"), __WFUNCTION__));
                bChan0 = TRUE;
            }
            else if ((statReg & 0x00001100) == 0x00001100)
            {
                // Channel0 connector is shorted
                DEBUGMSG(1, (TEXT("%s: Channel0 connector is shorted! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            } else
            {
                // Channel0 connector is unconnected
                DEBUGMSG(1, (TEXT("%s: Channel0 is unconnected! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            }
            
            // Channel 1 should be unconnected
            if ((statReg & 0x00002200) == 0x00000200)
            {
                // Channel 1 connector is loaded
                DEBUGMSG(1, (TEXT("%s: Channel 1 connector is loaded! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            else if ((statReg & 0x00002200) == 0x00002200)
            {
                // Channel 1 connector is shorted
                DEBUGMSG(1, (TEXT("%s: Channel 1 connector is shorted! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            else
            {
                // Channel 1 connector is unconnected
                DEBUGMSG(1, (TEXT("%s: Channel1 connector is unconnected, correct! \r\n"), __WFUNCTION__));
                bChan1 = TRUE;
            }
            
            // Channel2 should be unconnected 
            if ((statReg & 0x00004400) == 0x00000400)
            {
                // Channel2 is loaded
                DEBUGMSG(1, (TEXT("%s: Channel2 connector is loaded! \r\n"), __WFUNCTION__));
                bChan2 = FALSE;
            }
            else if ((statReg & 0x00004400) == 0x00004400)
            {
                // Channel2 is shorted
                DEBUGMSG(1, (TEXT("%s: Channel2 is shorted, failed! \r\n"), __WFUNCTION__));
                bChan2 = FALSE;
            }
            else
            {
                // Channel2 is unconnected.
                DEBUGMSG(1, (TEXT("%s: Channel2 is unconnected! \r\n"), __WFUNCTION__));
           
                // If channel2 is unconnected (which is what we are looking for),  we will set bChan2 = TRUE.
                bChan2 = TRUE; 
            }
            
            if (bChan0 & bChan1 & bChan2) 
                bRet = TRUE;
            else
                bRet = FALSE;
          
            break;
            
        case TV_OUT_COMPOSITE_CH2:
            // Channel 0 should be unconnected
            if ((statReg & 0x00001100) == 0x00000100)
            {
                // Channel0 connector is loaded
                DEBUGMSG(1, (TEXT("%s: Channel0 connector is loaded! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            }
            else if ((statReg & 0x00001100) == 0x00001100)
            {
                // Channel0 connector is shorted
                DEBUGMSG(1, (TEXT("%s: Channel0 connector is shorted! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            } else
            {
                // Channel0 connector is unconnected
                DEBUGMSG(1, (TEXT("%s: Channel0 is unconnected, correct! \r\n"), __WFUNCTION__));
                
                // If channel 0 is unconnected (which is what we are looking for),  we will set bChan0 = TRUE.
                bChan0 = TRUE;
            }
            
            // Channel 1 should be unconnected
            if ((statReg & 0x00002200) == 0x00000200)
            {
                // Channel 1 connector is loaded
                DEBUGMSG(1, (TEXT("%s: Channel 1 connector is loaded! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            else if ((statReg & 0x00002200) == 0x00002200)
            {
                // Channel 1 connector is shorted
                DEBUGMSG(1, (TEXT("%s: Channel 1 connector is shorted! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            else
            {
                // Channel 1 connector is unconnected
                DEBUGMSG(1, (TEXT("%s: Channel1 connector is unconnected, correct! \r\n"), __WFUNCTION__));
                
                // If channel 1 is unconnected (which is we are looking for),  we will set bChan1 = TRUE.
                bChan1 = TRUE;
            }
            
            // Channel2 LOADED 
            if ((statReg & 0x00004400) == 0x00000400)
            {
                // Channel2 is loaded
                DEBUGMSG(1, (TEXT("%s: Channel2 connector is loaded! \r\n"), __WFUNCTION__));
                bChan2 = TRUE;
            }
            else if ((statReg & 0x00004400) == 0x00004400)
            {
                // Channel2 is shorted
                DEBUGMSG(1, (TEXT("%s: Channel2 is shorted, failed! \r\n"), __WFUNCTION__));
                bChan2 = FALSE;
            }
            else
            {
                // Channel2 is unconnected.
                DEBUGMSG(1, (TEXT("%s: Channel2 is unconnected! \r\n"), __WFUNCTION__));
                bChan2 = FALSE; 
            }
            
            if (bChan0 & bChan1 & bChan2) 
                bRet = TRUE;
            else
                bRet = FALSE;
          
            break;
            
        case TV_OUT_COMPOSITE_CH0_CH2:
            // CVBS LOADED
            if ((statReg & 0x00001100) == 0x00000100)
            {
                // CVBS connector is loaded
                DEBUGMSG(1, (TEXT("%s: CVBS connector is loaded! \r\n"), __WFUNCTION__));
                bChan0 = TRUE;
            }
            else if ((statReg & 0x00001100) == 0x00001100)
            {
                // CVBS connector is shorted
                DEBUGMSG(1, (TEXT("%s: CVBS connector is shorted! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            } else
            {
                // CVBS connector is unconnected
                DEBUGMSG(1, (TEXT("%s: CVBS connector is unconnected! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            }
            
            // Channel 1 should be unconnected
            if ((statReg & 0x00002200) == 0x00000200)
            {
                // Channel 1 connector is loaded
                DEBUGMSG(1, (TEXT("%s: Channel 1 connector is loaded! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            else if ((statReg & 0x00002200) == 0x00002200)
            {
                // Channel 1 connector is shorted
                DEBUGMSG(1, (TEXT("%s: Channel 1 connector is shorted! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            else
            {
                // Channel 1 connector is unconnected
                DEBUGMSG(1, (TEXT("%s: Channel1 connector is unconnected, correct! \r\n"), __WFUNCTION__));
                
                // If channel 1 is unconnected (which is we are looking for),  we will set bChan1 = TRUE.
                bChan1 = TRUE;
            }
            
            // CBVS LOADED 
            if ((statReg & 0x00004400) == 0x00000400)
            {
                // CVBS is loaded
                DEBUGMSG(1, (TEXT("%s: CVBS connector is loaded! \r\n"), __WFUNCTION__));
                bChan2 = TRUE;
            }
            else if ((statReg & 0x00004400) == 0x00004400)
            {
                // CBVS is shorted
                DEBUGMSG(1, (TEXT("%s: CBVS is shorted, failed! \r\n"), __WFUNCTION__));
                bChan2 = FALSE;
            }
            else
            {
                // CVBS is unconnected.
                DEBUGMSG(1, (TEXT("%s: CVBS is unconnected! \r\n"), __WFUNCTION__));
                bChan2 = FALSE; 
            }
            
            if (bChan0 & bChan1 & bChan2) 
                bRet = TRUE;
            else
                bRet = FALSE;
          
            break;
            
        case TV_OUT_SVIDEO_CH0_CH1:
            // Y LOADED
            if ((statReg & 0x00001100) == 0x00000100)
            {
                // Y connector is loaded
                DEBUGMSG(1, (TEXT("%s: Y connector is loaded! \r\n"), __WFUNCTION__));
                bChan0 = TRUE;
            }
            else if ((statReg & 0x00001100) == 0x00001100)
            {
                // Y connector is shorted
                DEBUGMSG(1, (TEXT("%s: Y connector is shorted! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            } else
            {
                // Y connector is unconnected
                DEBUGMSG(1, (TEXT("%s: Y connector is unconnected! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            }
            
            // C LOADED
            if ((statReg & 0x00002200) == 0x00000200)
            {
                // C connector is loaded
                DEBUGMSG(1, (TEXT("%s: C connector is loaded! \r\n"), __WFUNCTION__));
                bChan1 = TRUE;
            }
            else if ((statReg & 0x00002200) == 0x00002200)
            {
                // C connector is shorted
                DEBUGMSG(1, (TEXT("%s: R or Cr or C connector is shorted! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            else
            {
                // C connector is unconnected
                DEBUGMSG(1, (TEXT("%s: R or Cr or C connector is unconnected! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            
            // Channel 2 should be unconnected for S-Video (Y/C)
            if ((statReg & 0x00004400) == 0x00000400)
            {
                // channel 2 is loaded
                DEBUGMSG(1, (TEXT("%s: Channel 2 connector is loaded, failed! \r\n"), __WFUNCTION__));
                bChan2 = FALSE;
            }
            else if ((statReg & 0x00004400) == 0x00004400)
            {
                // Channel 2 is shorted
                DEBUGMSG(1, (TEXT("%s: Channel 2 is shorted, failed! \r\n"), __WFUNCTION__));
                bChan2 = FALSE;
            }
            else
            {
                // Channel 2 is unconnected. S-video only uses channel 0 and chanel 1.
                DEBUGMSG(1, (TEXT("%s: Channel 2 is unconnected, correct! \r\n"), __WFUNCTION__));
                
                // If channel 2 is unconnected (which is what we are looking for),  we will set bChan2 = TRUE.
                bChan2 = TRUE; 
            }
            
            if (bChan0 & bChan1 & bChan2) 
                bRet = TRUE;
            else
                bRet = FALSE;
          
            break;
            
        case TV_OUT_SVIDEO_CH0_CH1_COMPOSITE_CH2:
        case TV_OUT_COMPONENT_YPRPB:
        case TV_OUT_COMPONENT_RGB:
            
            // G or Y LOADED
            if ((statReg & 0x00001100) == 0x00000100)
            {
                // G or Y connector is loaded
                DEBUGMSG(1, (TEXT("%s: G or Y connector is loaded! \r\n"), __WFUNCTION__));
                bChan0 = TRUE;
            }
            else if ((statReg & 0x00001100) == 0x00001100)
            {
                // G or Y connector is shorted
                DEBUGMSG(1, (TEXT("%s: G or Y connector is shorted! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            } else
            {
                // G or Y connector is unconnected
                DEBUGMSG(1, (TEXT("%s: G or Y connector is unconnected! \r\n"), __WFUNCTION__));
                bChan0 = FALSE;
            }
            
            // R or Cr or C LOADED
            if ((statReg & 0x00002200) == 0x00000200)
            {
                // R or Cr or C connector is loaded
                DEBUGMSG(1, (TEXT("%s: R or Cr or C connector is loaded! \r\n"), __WFUNCTION__));
                bChan1 = TRUE;
            }
            else if ((statReg & 0x00002200) == 0x00002200)
            {
                // R or Cr or C connector is shorted
                DEBUGMSG(1, (TEXT("%s: R or Cr or C connector is shorted! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            else
            {
                // R or Cr or C connector is unconnected
                DEBUGMSG(1, (TEXT("%s: R or Cr or C connector is unconnected! \r\n"), __WFUNCTION__));
                bChan1 = FALSE;
            }
            
            // B or Cb or CVBS LOADED
            if ((statReg & 0x00004400) == 0x00000400)
            {
                // B or Cb or CVBS connector is loaded
                DEBUGMSG(1, (TEXT("%s: B or Cb or CVBS connector is loaded! \r\n"), __WFUNCTION__));
                bChan2 = TRUE;
            }
            else if ((statReg & 0x00004400) == 0x00004400)
            {
                // B or Cb or CVBS connector is shorted
                DEBUGMSG(1, (TEXT("%s: B or Cb or CVBS connector is shorted! \r\n"), __WFUNCTION__));
                bChan2 = FALSE;
            }
            else
            {
                // B or Cb or CVBS connector is unconnected
                DEBUGMSG(1, (TEXT("%s: B or Cb or CVBS connector is unconnected! \r\n"), __WFUNCTION__));
                bChan2 = FALSE;
            }
            
            if (bChan0 & bChan1 & bChan2)
                bRet = TRUE;
            else
                bRet = FALSE;
            
            break;
        
        default:
            return bRet;        
    
    }
    
    SetEvent(m_hTveIntrEvent); 
    
    // Clear interrupt
    SETREG32(&m_pTVE->STAT_REG, 0x00000003);
    
    return bRet;

}

//-----------------------------------------------------------------------------
//
//  Function: TveSetCableDetection
//
//  This function sets cable detection configuration.
//
//  Parameters:
//     cd_mode
//           [in] cable detection mode to set
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

BOOL TveClass::TveSetCableDetection(TVE_CABLE_DETECTION_MODE cd_mode)
{
    int data = 0;
   
    switch (cd_mode) {
        case TVE_CD_TRIGGER_IN_MANUAL:
               
            // configure the CD module to trigger in manual mode (used in test mode also)
            data = CSP_BITFVAL( TVEV2_CD_EN, TVEV2_CD_EN_ENABLE)                     |  // enable cable detection
                   CSP_BITFVAL( TVEV2_CD_TRIG_MODE, TVEV2_CD_TRIG_MODE_AUTO)         |  // TVEV2_CD_TRIG_MODE_MANUAL, Manual (single
                                                                                        // shot) trigger,set cd_trig_mode = 1    
                   CSP_BITFVAL( TVEV2_CD_MON_PER, TVEV2_CD_MON_PER_7)                |  // Default setting: 8 fields (SD) or 8 frame (HD) for normal operation 
                   CSP_BITFVAL( TVEV2_CD_CH_0_REF_LVL, TVEV2_CD_CH_0_REF_LVL_LUMA)   |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_CH_1_REF_LVL, TVEV2_CD_CH_1_REF_LVL_LUMA)   |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_CH_2_REF_LVL, TVEV2_CD_CH_2_REF_LVL_LUMA)   |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_REF_MODE, TVEV2_CD_REF_MODE_MANUAL)         |  // Manual mode when the reference voltage is 
                                                                                        // set according to the CD_CH_#_REF_LVL. 
                   CSP_BITFVAL( TVEV2_CD_CH_0_LM_EN, TVEV2_CD_CH_0_LM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_1_LM_EN, TVEV2_CD_CH_1_LM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_2_LM_EN, TVEV2_CD_CH_2_LM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_0_SM_EN, TVEV2_CD_CH_0_SM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_1_SM_EN, TVEV2_CD_CH_1_SM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_2_SM_EN, TVEV2_CD_CH_2_SM_EN_ENABLE);
                
            break;
            
        case TVE_CD_TRIGGER_IN_STANDBY:
            // configure the CD module to trigger in standby mode, and automatic trigger
            // (used in test mode also)
              
            // configure the CD module to trigger in auto mode (used in test mode also)
            
            data = CSP_BITFVAL( TVEV2_CD_EN, TVEV2_CD_EN_ENABLE)                     |  // enable cable detection
                   CSP_BITFVAL( TVEV2_CD_TRIG_MODE, TVEV2_CD_TRIG_MODE_MANUAL)       |  // Manual mode, set cd_trig_mode = 1
                   CSP_BITFVAL( TVEV2_CD_MON_PER, TVEV2_CD_MON_PER_7)                |  // 2.48 seconds for cable detection monitoring period 
                                                                                        // in standby mode
                   CSP_BITFVAL( TVEV2_CD_CH_0_REF_LVL, TVEV2_CD_CH_0_REF_LVL_LUMA)   |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_CH_1_REF_LVL, TVEV2_CD_CH_1_REF_LVL_LUMA)   |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_CH_2_REF_LVL, TVEV2_CD_CH_2_REF_LVL_LUMA)   |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_REF_MODE, TVEV2_CD_REF_MODE_AUTO)           |  // Automatic mode                   
                   CSP_BITFVAL( TVEV2_CD_CH_0_LM_EN, TVEV2_CD_CH_0_LM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_1_LM_EN, TVEV2_CD_CH_1_LM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_2_LM_EN, TVEV2_CD_CH_2_LM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_0_SM_EN, TVEV2_CD_CH_0_SM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_1_SM_EN, TVEV2_CD_CH_1_SM_EN_ENABLE)     |
                   CSP_BITFVAL( TVEV2_CD_CH_2_SM_EN, TVEV2_CD_CH_2_SM_EN_ENABLE);
                   
            break;
            
        case TVE_CD_TRIGGER_IN_FUNC:
            // configure the CD module to trigger in func (normal performation) mode , and automatic trigger 
            // (used in test mode also)
              
            // configure the CD module to trigger in manual mode (used in test mode also)
            data = CSP_BITFVAL( TVEV2_CD_EN, TVEV2_CD_EN_ENABLE)                      |  // enable cable detection
                   CSP_BITFVAL( TVEV2_CD_TRIG_MODE, TVEV2_CD_TRIG_MODE_MANUAL)        |  // set cd_trig_mode = 1 for manual
                   CSP_BITFVAL( TVEV2_CD_MON_PER, TVEV2_CD_MON_PER_7)                 |  // Default setting: 8 fields (SD) or 8 frame (HD) for normal operation 
                   CSP_BITFVAL( TVEV2_CD_CH_0_REF_LVL, TVEV2_CD_CH_0_REF_LVL_LUMA)    |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_CH_1_REF_LVL, TVEV2_CD_CH_1_REF_LVL_LUMA)    |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_CH_2_REF_LVL, TVEV2_CD_CH_2_REF_LVL_LUMA)    |  // the ref level could be set to LUMA
                   CSP_BITFVAL( TVEV2_CD_REF_MODE, TVEV2_CD_REF_MODE_MANUAL)          |  // Manual mode when the reference voltage is 
                                                                                         // set according to the CD_CH_#_REF_LVL. 
                   CSP_BITFVAL( TVEV2_CD_CH_0_LM_EN, TVEV2_CD_CH_0_LM_EN_ENABLE)      |
                   CSP_BITFVAL( TVEV2_CD_CH_1_LM_EN, TVEV2_CD_CH_1_LM_EN_ENABLE)      |
                   CSP_BITFVAL( TVEV2_CD_CH_2_LM_EN, TVEV2_CD_CH_2_LM_EN_ENABLE)      |
                   CSP_BITFVAL( TVEV2_CD_CH_0_SM_EN, TVEV2_CD_CH_0_SM_EN_ENABLE)      |
                   CSP_BITFVAL( TVEV2_CD_CH_1_SM_EN, TVEV2_CD_CH_1_SM_EN_ENABLE)      |
                   CSP_BITFVAL( TVEV2_CD_CH_2_SM_EN, TVEV2_CD_CH_2_SM_EN_ENABLE);
           
            break;
          
        default:
            ERRORMSG(1, (_T("Mode: Unkown CD Mode is specified!\r\n")));
            return FALSE;
           
    }
    
    // Set CD control register
    OUTREG32(&m_pTVE->CD_CONT_REG, data);
    
    return TRUE;
}


