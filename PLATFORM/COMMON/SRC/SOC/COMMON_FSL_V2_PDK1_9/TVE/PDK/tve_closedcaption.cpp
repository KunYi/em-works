//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:  tve_closedcaption.cpp
//
// -----------------------------------------------------------------------------
// PURPOSE:  tve closed caption support.
//
// -----------------------------------------------------------------------------
//

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_tve.h"

#include "Ipu_common.h"
#include "di.h"
#include "tve.h"
#include "tve_sdk.h"


//-----------------------------------------------------------------------------
//
//  Function: TveSetClosedCaption
//
//  Set closed caption0 data.
//
//  Parameters:
//      cc_f1_odd_field_data
//            [in] closed caption1 data to set.
//      cc_f2_even_field_data
//            [in] closed caption2 data to set.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL TveClass::TveSetClosedCaption(TVECC cc_f1_odd_field_data, TVECC cc_f2_even_field_data)
{
    UINT32 data;
   
    // Enable TVE CC bits in Interrupt Control Register
    OUTREG32(&m_pTVE->INT_CONT_REG, 
              CSP_BITFVAL( TVE_CC_F1_DONE_IEN, TVE_CC_F1_DONE_IEN_ENABLE)   |
              CSP_BITFVAL( TVE_CC_F2_DONE_IEN, TVE_CC_F2_DONE_IEN_ENABLE));
        
  
    // Set closed caption data to reg0 and reg1        
    data = CSP_BITFVAL( TVE_CC_F1_EN, TVE_CC_F1_EN_ENABLE)                  |
           CSP_BITFVAL( TVE_CC_F1_DATA1, cc_f1_odd_field_data.cc_data_low)  |   // cc1_data[6:0]
           CSP_BITFVAL( TVE_CC_F1_DATA2, cc_f1_odd_field_data.cc_data_high);       // cc1_data[14:
    
    // Enable closed caption level boost a factor of 1.7 when use in RGB TV output mode
    // This CC_BOOST_EN bit setting applies both for data in CC_CONT_REG_0 and CC_CONT_REG_1.
    if (m_eTVOutputMode == TV_OUT_COMPONENT_RGB)
        data |=  CSP_BITFVAL( TVE_CC_BOOST_EN, TVE_CC_BOOST_EN_ENABLE);
              
    OUTREG32(&m_pTVE->CC_CONT_REG_0, data);
    
    OUTREG32(&m_pTVE->CC_CONT_REG_1,
              CSP_BITFVAL( TVE_CC_F2_EN, TVE_CC_F2_EN_ENABLE)                   | 
              CSP_BITFVAL( TVE_CC_F2_DATA1, cc_f2_even_field_data.cc_data_low)  |  // cc2_data[6:0]
              CSP_BITFVAL( TVE_CC_F2_DATA2, cc_f2_even_field_data.cc_data_high));    // cc2_data[14:8]
 
    return TRUE;             
}



