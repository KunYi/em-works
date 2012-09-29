//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:  tve_vbi.cpp
//
// -----------------------------------------------------------------------------
// PURPOSE:  TVE Closed Caption, CGMS and WSS support.
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
#include "common_tvev2.h"

#include "Ipu_common.h"
#include "di.h"
#include "tve.h"
#include "tve_sdk.h"




//-----------------------------------------------------------------------------
//
//  Function: TveSetVBIControlReg
//
//  Set default configuration to VBI data control register
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void TveClass::TveSetVBIControlReg()
{

    UINT32 data = 0;
  
    // Setup VBI data control register, VBI_DATA_CONT_REG     
    data = CSP_BITFVAL( TVEV2_CC_SD_F1_EN, TVEV2_CC_SD_F1_EN_ENABLE)                  |
           CSP_BITFVAL( TVEV2_CC_SD_F2_EN, TVEV2_CC_SD_F2_EN_ENABLE)                  |
           CSP_BITFVAL( TVEV2_CGMS_SD_F1_EN, TVEV2_CGMS_SD_F1_EN_ENABLE)              |
           CSP_BITFVAL( TVEV2_CGMS_SD_F2_EN, TVEV2_CGMS_SD_F2_EN_ENABLE)              |
           CSP_BITFVAL( TVEV2_CGMS_SD_SW_CRC_EN, TVEV2_CGMS_SD_SW_CRC_EN_DISABLE)     |  // SD CRC value is calculated by HW
           CSP_BITFVAL( TVEV2_WSS_SD_EN, TVEV2_WSS_SD_EN_ENABLE)                      |
           CSP_BITFVAL( TVEV2_CGMS_HD_A_F1_EN, TVEV2_CGMS_HD_A_F1_EN_ENABLE)          |
           CSP_BITFVAL( TVEV2_CGMS_HD_A_F2_EN, TVEV2_CGMS_HD_A_F2_EN_ENABLE)          |
           CSP_BITFVAL( TVEV2_CGMS_HD_A_SW_CRC_EN, TVEV2_CGMS_HD_A_SW_CRC_EN_DISABLE) |  // HD A CRC value is calculated by HW
           CSP_BITFVAL( TVEV2_CGMS_HD_B_F1_EN, TVEV2_CGMS_HD_B_F1_EN_ENABLE)          |
           CSP_BITFVAL( TVEV2_CGMS_HD_B_F2_EN, TVEV2_CGMS_HD_B_F2_EN_ENABLE)          |
           CSP_BITFVAL( TVEV2_CGMS_HD_B_SW_CRC_EN, TVEV2_CGMS_HD_B_SW_CRC_EN_DISABLE);   // HD B CRC value is calculated by HW
           
    // Enable SD closed caption level boost a factor of 1.7 when use in RGB TV output mode
    if (m_eTVOutputMode == TV_OUT_COMPONENT_RGB)
        data |=  CSP_BITFVAL( TVEV2_CC_SD_BOOST_EN, TVEV2_CC_SD_BOOST_EN_ENABLE);
    else
        data |=  CSP_BITFVAL( TVEV2_CC_SD_BOOST_EN, TVEV2_CC_SD_BOOST_EN_DISABLE);
     
              
    OUTREG32(&m_pTVE->VBI_DATA_CONT_REG, data);
                 
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetClosedCaption
//
//  Set SD closed caption data.
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
    
    // Enable TVE CC bits in Interrupt Control Register
    OUTREG32(&m_pTVE->INT_CONT_REG, 
              CSP_BITFVAL( TVEV2_CC_SD_F1_DONE_IEN, TVEV2_CC_SD_F1_DONE_IEN_ENABLE)   |
              CSP_BITFVAL( TVEV2_CC_SD_F2_DONE_IEN, TVEV2_CC_SD_F2_DONE_IEN_ENABLE));
        
    // Set SD closed caption F1 data to VBI_DATA_REG_2      
    OUTREG32(&m_pTVE->VBI_DATA_REG_2,
              CSP_BITFVAL( TVEV2_CC_SD_F1_DATA1, cc_f1_odd_field_data.cc_data_low) |   // CC_SD_CGMS_HD_B_F1_DATA_0[6:0]
              CSP_BITFVAL( TVEV2_CC_SD_F1_DATA2, cc_f1_odd_field_data.cc_data_high));  // CC_SD_CGMS_HD_B_F1_DATA_0[14:8]
 
    // Set SD closed caption F1 data to VBI_DATA_REG_6      
    OUTREG32(&m_pTVE->VBI_DATA_REG_6,
              CSP_BITFVAL( TVEV2_CC_SD_F2_DATA1, cc_f2_even_field_data.cc_data_low) |  // CC_SD_CGMS_HD_B_F2_DATA_0[6:0]
              CSP_BITFVAL( TVEV2_CC_SD_F2_DATA2, cc_f2_even_field_data.cc_data_high)); // CC_SD_CGMS_HD_B_F2_DATA_0[14:8]
 
 
    return TRUE;             
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetCgmsWssFor625LinePALPAL
//
//  Set SD WSS data for PAL. 
//
//  Parameters:
//     cgms_f1_wss_data
//           [in] cgms f1 wss data to be set for CGMS WSS.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------   
void TveClass::TveSetCgmsWssFor625LinePAL(UINT32 cgms_f1_wss_data)
{    
    // ETSI EN 300 294 specification describes television systems: 625-line television Wide Screen
    // Signalling (WSS), which is applicable to 625-line PAL and SECAM systems. PAL which is 625
    // vertical lines, 25 frames per second interlaced. PAL in progressive scan, in which the
    // display is non-interlaced. 
    // 
    // CGMS WSS is inserted in lines 23 and 336 for PAL/SECAM. Some countries with PAL standard don't
    // use line 336 for wide screen signaling ( they use only line 23). TVEV2 only sets line 23 for PAL.
    //
    // The wide screen signalling information contains 14 bits information (i.e. cgms_data) on the aspect ratio
    // range of the transmitted signal and its position, on the position of the subtitles and on the camera/film
    // mode. Furthermore signalling for EDTV and for surround sound and copyright are included. Some bits are
    // reserved for future use. Please refer to tve_sdk.h for more descriptions about each bit.
 
    OUTREG32(&m_pTVE->VBI_DATA_REG_3, CSP_BITFVAL( TVEV2_WSS_SD_DATA, cgms_f1_wss_data));
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetCgmsWssFor525LineNTSC
//
//  Set WSS data for NTSC. The TVE shall be in the interlaced mode. 
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
    // in which the display is non-interlaced. Please refer to tvev2_sdk.h for more descriptions about
    // each bit.

    // CGMS WSS is inserted in line 20 and 283 for NTSC.
    
    OUTREG32(&m_pTVE->VBI_DATA_REG_0, CSP_BITFVAL( TVEV2_CGMS_SD_HD_A_F1_DATA, cgms_f1_wss_data));
    OUTREG32(&m_pTVE->VBI_DATA_REG_1, CSP_BITFVAL( TVEV2_CGMS_SD_HD_A_F2_DATA, cgms_f2_data));
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetHDSCgmsAData
//
//  Set VBI HD CGMS Type A Data.
//
//  Parameters:
//      cgms_hda_f1_data
//            [in] HD CGMS Type A Data for field1 to set.
//
//      cgms_hda_f2_data
//            [in] HD CGMS Type A Data for field2 to set.
//
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void TveClass::TveSetHDCgmsAData(UINT32 cgms_hda_f1_data, UINT32 cgms_hda_f2_data)
{
    
    OUTREG32(&m_pTVE->VBI_DATA_REG_0, CSP_BITFVAL(TVEV2_CGMS_SD_HD_A_F1_DATA, cgms_hda_f1_data ));
    OUTREG32(&m_pTVE->VBI_DATA_REG_1, CSP_BITFVAL(TVEV2_CGMS_SD_HD_A_F2_DATA, cgms_hda_f2_data ));
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetHDCgmsBHeader
//
//  Set VBI HD CGMS Type B header values.
//
//  Parameters:
//      cgms_hdb_f1_header
//            [in] HD CGMS Type B header for field1 data to set.
//
//      cgms_hdb_f2_header
//            [in] HD CGMS Type B header for field2 data to set.
//
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void TveClass::TveSetHDCgmsBHeader(UINT32 cgms_hdb_f1_header, UINT32 cgms_hdb_f2_header)
{
    // Insert header data without overwrite other fields
    INSREG32BF(&m_pTVE->VBI_DATA_CONT_REG, TVEV2_CGMS_HD_B_F1_HEADER, cgms_hdb_f1_header );
    INSREG32BF(&m_pTVE->VBI_DATA_CONT_REG, TVEV2_CGMS_HD_B_F2_HEADER, cgms_hdb_f2_header );
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetHDCgmsBData
//
//  Set VBI HD CGMS Type B Header and Data.
//
//  Parameters:
//      pCgmsInfo
//            [in] a point to TVEHDCgmsBInfo
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void TveClass::TveSetHDCgmsBData(TVEHDCgmsBInfo *pCgmsInfo)
{
    // Setup HD CGMS Type B headers
    TveSetHDCgmsBHeader(pCgmsInfo->f1_header, pCgmsInfo->f2_header);
    
    // Setup HD CGMS Type B data 
    OUTREG32(&m_pTVE->VBI_DATA_REG_2, CSP_BITFVAL(TVEV2_CC_SD_CGMS_HD_B_F1_DATA_0, pCgmsInfo->f1_data_0 ));
    OUTREG32(&m_pTVE->VBI_DATA_REG_3, CSP_BITFVAL(TVEV2_WSS_SD_CGMS_HD_B_F1_DATA_1, pCgmsInfo->f1_data_1 ));
    OUTREG32(&m_pTVE->VBI_DATA_REG_4, CSP_BITFVAL(TVEV2_CGMS_HD_B_F1_DATA_2, pCgmsInfo->f1_data_2 ));
    OUTREG32(&m_pTVE->VBI_DATA_REG_5, CSP_BITFVAL(TVEV2_CGMS_HD_B_F1_DATA_3, pCgmsInfo->f1_data_3 ));

    OUTREG32(&m_pTVE->VBI_DATA_REG_6, CSP_BITFVAL(TVEV2_CC_SD_CGMS_HD_B_F2_DATA_0, pCgmsInfo->f2_data_0 ));
    OUTREG32(&m_pTVE->VBI_DATA_REG_7, CSP_BITFVAL(TVEV2_CGMS_HD_B_F2_DATA_1, pCgmsInfo->f2_data_1 ));
    OUTREG32(&m_pTVE->VBI_DATA_REG_8, CSP_BITFVAL(TVEV2_CGMS_HD_B_F2_DATA_2, pCgmsInfo->f2_data_2 ));
    OUTREG32(&m_pTVE->VBI_DATA_REG_9, CSP_BITFVAL(TVEV2_CGMS_HD_B_F2_DATA_3, pCgmsInfo->f2_data_3 ));

}



