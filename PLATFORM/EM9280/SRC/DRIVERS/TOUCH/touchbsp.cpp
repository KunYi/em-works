//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
// Module Name:
//
//    sdhcbsp.cpp   
//
// Abstract:
//
//    Board-level TOUCH  implmentation.
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////

#include "bsp.h"


#define _MIN_SAMPLE_X_                          0x130
#define _MAX_SAMPLE_X_                          0xf70
#define _MIN_SAMPLE_Y_                          0x0f0
#define _MAX_SAMPLE_Y_                          0xf80

#define _RANGE_SAMPLE_X_                        (_MAX_SAMPLE_X_-_MIN_SAMPLE_X_)
#define _RANGE_SAMPLE_Y_                        (_MAX_SAMPLE_Y_-_MIN_SAMPLE_Y_)

#define _SAMPLE_POINT_X_                        ((_RANGE_SAMPLE_X_)/800)
#define _SAMPLE_POINT_Y_                        ((_RANGE_SAMPLE_Y_)/480)

#define _AD_TORLANCE_X_                         (_SAMPLE_POINT_X_*10)  
#define _AD_TORLANCE_Y_                         (_SAMPLE_POINT_Y_*6)  

BOOL BSPTouchUses5WireType();
//-----------------------------------------------------------------------------
//
// Function:  BSPTouchGetDetIRQ
//
// This function returns the touch detect IRQ
//
// Parameters:
//      index
//         None.
//
// Returns:
//      Physical Detect  IRQ  for touch
//
//-----------------------------------------------------------------------------

DWORD  BSPTouchGetDetIRQ()
{
    return IRQ_LRADC_TOUCH;
}

//-----------------------------------------------------------------------------
//
// Function:  BSPTouchGetChangedIRQ
//
// This function returns the touch Changed  IRQ.
//
// Parameters:
//      index
//        None.
//
// Returns:
//      Physical Changed  IRQ  for touch
//
//-----------------------------------------------------------------------------
DWORD BSPTouchGetChangedIRQ()
{

    if(BSPTouchUses5WireType())
        return IRQ_LRADC_CH6;

    return IRQ_LRADC_CH3;
}


//-----------------------------------------------------------------------------
//
// Function:  BSPTouchGetDeltaCoord
//
// This function returns bsp delta X ,Y coordinate samples
//
// Parameters:
// Parameters:
//      x
//          [out] Points to x delta sample coordinate.
//
//      y
//          [out] Points to y delta sample coordinate.
// Returns:
//       None
//
//-----------------------------------------------------------------------------
void BSPTouchGetDeltaCoord(int * x,int * y)
{
    *x=_AD_TORLANCE_X_;
    *y=_AD_TORLANCE_Y_;
}


//------------------------------------------------------------------------------
//
// Function:     BSPTouchUses5WireType
//
//
// Return Value:
//        This fuction returns TRUE if use 5 wire type  mode otherwise  used 4 wire type  it returns FALSE
//
//------------------------------------------------------------------------------
BOOL BSPTouchUses5WireType()
{
    return FALSE;
}

