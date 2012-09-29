//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:  tve_cgms.cpp
//
// -----------------------------------------------------------------------------
// PURPOSE:  tve Cgms/WSS support
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
#include "tve.h"

//-----------------------------------------------------------------------------
//
//  Function: TveSetCgmsWssFor626LinePAL
//
//  Set CGMS F1 WSS data for PAL. 
//
//  Parameters:
//     cgms_f1_wss_data
//           [in] cgms f1 wss data to be set for CGMS WSS.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------   
void TveClass::TveSetCgmsWssFor626LinePAL(UINT32 cgms_f1_wss_data)
{    
    // ETSI EN 300 294 specification describes television systems: 625-line television Wide Screen
    // Signalling (WSS), which is applicable to 625-line PAL and SECAM systems. PAL which is 625
    // vertical lines, 25 frames per second interlaced. PAL in progressive scan, in which the
    // display is non-interlaced. 
    // 
    // CGMS WSS is inserted in lines 23 and 336 for PAL/SECAM. Some countries with PAL standard don't
    // use line 336 for wide screen signaling ( they use only line 23). TVE only sets line 23 for PAL.
    //
    // The wide screen signalling information contains 14 bits information (i.e. cgms_data) on the aspect ratio
    // range of the transmitted signal and its position, on the position of the subtitles and on the camera/film
    // mode. Furthermore signalling for EDTV and for surround sound and copyright are included. Some bits are
    // reserved for future use. Please refer to tve_sdk.h for more descriptions about each bit.
 
    // Left shift 8 bits to feed the cgms data into CC_F1_DATA fields [21:8] in CGMS WSS CON Reg0
    OUTREG32(&m_pTVE->CGMS_WSS_CONT_REG_0, 0x00000001 | ( 0x003fff00 & (cgms_f1_wss_data << 8) ));
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetCgmsWssFor525LineNTSC
//
//  Set CGMS F1 WSS data for NTSC. The TVE shall be in the interlaced mode. 
//
//  Parameters:
//     cgms_f1_wss_data
//           [in] cgms f1 wss data to be set for CGMS WSS.
//
//     cgms_f2_wss_data
//           [in] cgms f2 wss data to be set for CGMS WSS.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void TveClass::TveSetCgmsWssFor525LineNTSC(UINT32 cgms_f1_wss_data, UINT32 cgms_f2_data)
{
    // NTSC, which is 525 vertical lines, 30 frames per second interlaced. NTSC in progressive scan,
    // in which the display is non-interlaced. Please refer to tve_sdk.h for more descriptions about
    // each bit.

    // CGMS WSS is inserted in line 20 and 283 for NTSC. We need to set CGMS_WSS_CONT_REG_0 and
    // CGMS_CONT_REG_1 registers.
    
    // Left shift 8 bits to feed the cgms wss data into CC_F1_DATA fields [21:8] in 
    // CGMS WSS CON Reg0, and  CC_F2_DATA fields [21:8] in CGMS WSS CON Reg1.
    OUTREG32(&m_pTVE->CGMS_WSS_CONT_REG_0, 0x00000001 | ( 0x003fff00 & (cgms_f1_wss_data << 8) ));
  
    OUTREG32(&m_pTVE->CGMS_CONT_REG_1, 0x00000001 | ( 0x003fff00 & (cgms_f2_data << 8) ));
}


