//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  tve.cpp
//
//  Provides SOC driver implementation for TV encoder V2 chip.
//

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)
#include <math.h>

#include "common_macros.h"
#include "common_tvev2.h"
#include "tve.h"

//------------------------------------------------------------------------------
// External Functions

extern "C" DWORD CSPTVEGetBaseAddr();
extern "C" DWORD CSPTVEGetIRQ();
extern "C" BOOL  CSPTVESetClockGatingMode(BOOL bTVEClockEnable);
extern "C" BOOL  CSPTVESetClock(BOOL bHD);
//-----------------------------------------------------------------------------
//
// Function: TveClass
//
// TVE class constructor.  Calls TveInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
TveClass::TveClass(void)
{
    TveInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~TveClass
//
// The destructor for TveClass.  Calls TveDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
TveClass::~TveClass(void)
{
    TveDeinit();
}


//------------------------------------------------------------------------------
//
// Function: TveInit
//
// This function initializes the TVE encoder, including memory mapping, 
// interrupt, etc.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TveClass::TveInit()
{
    BOOL retVal = TRUE;
    DWORD dwIrq;
    PHYSICAL_ADDRESS phyAddr;

    // Memory mapping for TVE registers         
    phyAddr.QuadPart = CSPTVEGetBaseAddr();
    m_pTVE = (PTVEV2_REGS)MmMapIoSpace(phyAddr, sizeof(PTVEV2_REGS), FALSE);
    if(m_pTVE == NULL) 
    {   
        ERRORMSG(1, (TEXT("%s: TVE Memory Mapping failed\r\n"), __WFUNCTION__));
        goto cleanUp;
    }   
        
    // Create the TVE interrupt events 
    m_hTveIntrEvent = CreateEvent(NULL, FALSE, FALSE, TVE_INTR_EVENT);
    if (m_hTveIntrEvent == NULL)
    {
        ERRORMSG(1,
            (TEXT("%s: CreateEvent for TVE Interrupt failed\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }
    
    // Create TVE interrupt events
    m_hTveFieldEvent  = CreateEvent(NULL, FALSE, FALSE, TVE_FIELD_EVENT_NAME);
    m_hTveFrameEvent  = CreateEvent(NULL, FALSE, FALSE, TVE_FRAME_EVENT_NAME);
    m_hTveCgmsF2Event = CreateEvent(NULL, FALSE, FALSE, TVE_CGMS_F2_EVENT_NAME);
    m_hTveCgmsF1Event = CreateEvent(NULL, FALSE, FALSE, TVE_CGMS_F1_EVENT_NAME);
    m_hTveCcF2Event   = CreateEvent(NULL, FALSE, FALSE, TVE_CC_F2_EVENT_NAME);
    m_hTveCcF1Event   = CreateEvent(NULL, FALSE, FALSE, TVE_CC_F1_EVENT_NAME);
    m_hTveCdMonEvent  = CreateEvent(NULL, FALSE, FALSE, TVE_CD_MON_EVENT_NAME);
    m_hTveCdSmEvent   = CreateEvent(NULL, FALSE, FALSE, TVE_CD_SM_EVENT_NAME);
    m_hTveCdLmEvent   = CreateEvent(NULL, FALSE, FALSE, TVE_CD_LM_EVENT_NAME);

    if ((m_hTveFieldEvent == NULL)  || (m_hTveFrameEvent == NULL) || (m_hTveCgmsF2Event == NULL) ||
        (m_hTveCgmsF1Event == NULL) || (m_hTveCcF2Event == NULL)  || (m_hTveCcF1Event == NULL)   ||
        (m_hTveCdMonEvent == NULL)  || (m_hTveCdSmEvent == NULL)  || (m_hTveCdLmEvent == NULL))
    {
        ERRORMSG(1, (TEXT("%s: Create events for TVE HW interrupt failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp; 
    }
    
    // Get kernel to translate IRQ -> System Interrupt ID
    dwIrq = CSPTVEGetIRQ();
    if(!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(DWORD), &dwSysIntr, sizeof(DWORD), NULL)) 
    {   
        ERRORMSG(1, (TEXT("%s: Request SYSINTR failed (TVE Interrupt)!\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }

    if(!InterruptInitialize(dwSysIntr, m_hTveIntrEvent, NULL, 0)) 
    {
        ERRORMSG(1, (TEXT("%s: Interrupt initialization failed (TVE Interrupt)!\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }

    
    return retVal;

cleanUp:
    TveDeinit();
    
    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TveEnable
//
// This function enables the TVE encoder.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TveClass::TveEnable()
{   
    BOOL retVal = TRUE;
    
    // The following settings are more related TVE internal working mode. 
    // So we don't ask customers to configure them.
    // Here are default settings. You can change to other working modes here if necesary.
    // 
    TVE_CABLE_DETECTION_MODE tveCDMode  = TVE_CD_TRIGGER_IN_FUNC; 
    TVE_INPUT_DATA_SOURCE data_source   = IPU_VIDEO_DATA_BUS1; // TVEv2 will use IPU DI1
    TVE_INPUT_VIDEO_FORM video_form     = VIDEO_YCBCR422;
    DWORD tveClock = TVE_S216_CLK; // default is for SD.
    
    // Check if the current mode is for SD or HD
    if ((m_eTVStd == TV_STAND_NTSC) || 
        (m_eTVStd == TV_STAND_PALM) ||
        (m_eTVStd == TV_STAND_PALN) ||
        (m_eTVStd == TV_STAND_PAL))
    {
         m_bHD = FALSE;
    }
    else
    {
         // update tveClock for HD
         tveClock = TVE_S297_CLK;
         m_bHD = TRUE;
    }
    
    // Set PLL3 clock for HDTV or SDTV.
    TveSetClock(tveClock);
    
    // To save power, enable TVE Clock Gating only when TV is enabled.
    retVal = CSPTVESetClockGatingMode(TRUE);
    if(retVal != TRUE) 
    {
        ERRORMSG(1, (TEXT("%s: TVE clock gating failed\r\n"), __WFUNCTION__));
        TVE_FUNCTION_EXIT();
        goto cleanUp;
    } 
 
    // Create IST thread to receive hardware interrupts
    bIntrThreadLoop = TRUE;
    hTveIST = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TveIST, NULL, 0, NULL);
    if(hTveIST == NULL) 
    {
        ERRORMSG(1, (TEXT("%s: Create thread for TVE interrupt failed!\r\n"),  __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }
 
    // Get TVDAC sample rate by the current clock
    TVE_TVDAC_SAMPLING_RATE sampleRate = TveGetTVDACSampleRateByClock(tveClock);
    if (sampleRate == SAMPLING_RATE_NONE)
    {
        ERRORMSG(1, (_T("Invaild sample rate!\r\n")));
        retVal = FALSE;
        goto cleanUp;
    }
    
    // Setup TVE Common Register 
    TveConfigureCommonReg(sampleRate, data_source, video_form);

    // Setup TVE Interrupt Control Register
    TveConfigureInterruptCtrlReg();
    
    // Setup TVE Luma Control Registers
  //  TveConfigureLumaCtrlReg();
        
    // Setup TVE Chroma Control register
    TveConfigureChromaCtrlReg();
    
    int data = 0;
    data = CSP_BITFVAL( TVEV2_DATA_CLIP_USR, 1);
    OUTREG32(&m_pTVE->CSC_USR_CONT_REG_0, data);  
    
    // Set TVE cable detection using TVE_CD_TRIGGER_IN_FUNC as a default setting.
    if (!TveSetCableDetection(tveCDMode))
    {
        ERRORMSG(1, (TEXT("%s: Set TVE Cable Detection failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }
    
    
#if 0  
    // FIXME: TVE HW has a bug to detect the cables
    
    // Check if the cables are connected as we expected
    if (!TveIsCableDetected())
    {
        DEBUGMSG(1, (TEXT("%s: TVE Cable is not connected!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }
#endif
    
    // Setup the cables detect directly to enable cable detect status bits and interrupt bits.
    OUTREG32(&m_pTVE->STAT_REG, 0x01777FF9);                            
    SETREG32(&m_pTVE->INT_CONT_REG, CSP_BITFVAL( TVEV2_CD_LM_IEN, TVEV2_CD_LM_IEN_ENABLE) |
                                    CSP_BITFVAL( TVEV2_CD_SM_IEN, TVEV2_CD_SM_IEN_ENABLE));
    
    // TVE initialization
    OUTREG32(&m_pTVE->RESERVED_REG_4, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_3, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_2, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_1, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_0, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_5, 0x00000000);
    
    // TveDumpRegs();
    
    return retVal;
    
cleanUp:
    TveDeinit();
    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TveDisable
//
// This function disable the TVE out encoder.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TveClass::TveDisable()
{
    // Give the IST thread a chance to perform any cleanup.
    if (hTveIST) 
    {
        bIntrThreadLoop = FALSE;
        SetEvent(m_hTveIntrEvent);
        CloseHandle(hTveIST);
        hTveIST = NULL;
    }
    
    // Clear up interrupt/CD control registers and stop TV encoder
    if(m_pTVE != 0) 
    {
        OUTREG32(&m_pTVE->INT_CONT_REG, 0);
        OUTREG32(&m_pTVE->CD_CONT_REG, 0);
    
        // Disable the TVE power to stop TV encoder
        OUTREG32(&m_pTVE->COM_CONF_REG, 0);
    }

    // Disable TVE Clock Gating
    CSPTVESetClockGatingMode(FALSE); 
    
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: TveDeinit
//
// This function deinitializes the TVE out encoder.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TveClass::TveDeinit()
{
   
    if(m_hTveIntrEvent)
    {
        CloseHandle(m_hTveIntrEvent); 
        m_hTveIntrEvent = NULL;
    }
    
    if (m_hTveFieldEvent)
    {
        CloseHandle(m_hTveFieldEvent);
        m_hTveFieldEvent = NULL;
    }
    
    if (m_hTveFrameEvent)
    {
        CloseHandle(m_hTveFrameEvent);
        m_hTveFrameEvent = NULL;
    }
    
    if (m_hTveCgmsF2Event)
    {
        CloseHandle(m_hTveCgmsF2Event);
        m_hTveCgmsF2Event = NULL;
    }
    
    if (m_hTveCgmsF1Event)
    {
        CloseHandle(m_hTveCgmsF1Event);
        m_hTveCgmsF1Event = NULL;
    }
    
    if (m_hTveCcF2Event)
    {
        CloseHandle(m_hTveCcF2Event);
        m_hTveCcF2Event = NULL;
    }
    
    if (m_hTveCcF1Event)
    { 
        CloseHandle(m_hTveCcF1Event);
        m_hTveCcF1Event = NULL;
    }
    
    if (m_hTveCdMonEvent)
    { 
        CloseHandle(m_hTveCdMonEvent);
        m_hTveCdMonEvent = NULL;
    }
    
    if (m_hTveCdSmEvent)
    {
        CloseHandle(m_hTveCdSmEvent);
        m_hTveCdSmEvent = NULL;
    }
    
    if (m_hTveCdLmEvent)
    {
        CloseHandle(m_hTveCdLmEvent);
        m_hTveCdLmEvent = NULL;
    }
    
    // Disable TVE
    TveDisable();
    
    // Do unmap memory
    if(m_pTVE != 0) 
    {
        MmUnmapIoSpace((PTVEV2_REGS)m_pTVE, sizeof(PTVEV2_REGS));
        m_pTVE = NULL;
    }
    
    return TRUE;
}



//-----------------------------------------------------------------------------
//
//  Function: TveConfigureCommonReg
//
//  This function is to configure the TVE common configuration register. 
//
//  Parameters:
//
//     sampling_rate
//           [in] Set TVE sampling rate
//
//     data_source
//           [in] video data comes from which source
//     
//     video_form
//           [in] Video form: YUV422 or YUV444 
//  
//
//  Returns:
//     None.
//
//-----------------------------------------------------------------------------
void TveClass::TveConfigureCommonReg(TVE_TVDAC_SAMPLING_RATE sampling_rate, TVE_INPUT_DATA_SOURCE data_source,
                                     TVE_INPUT_VIDEO_FORM video_form)
{
    int data=0;
    
    // Enable the Common Configuration Register
    data = CSP_BITFVAL( TVEV2_TVE_EN, TVEV2_TVE_EN_ENABLE)                |
           CSP_BITFVAL( TVEV2_TVDAC_SAMP_RATE, sampling_rate)             |                                                                                
           CSP_BITFVAL( TVEV2_IPU_CLK_EN, TVEV2_IPU_CLK_EN_ENABLE)        |
           CSP_BITFVAL( TVEV2_DATA_SOURCE_SEL, data_source)               |
           CSP_BITFVAL( TVEV2_INP_VIDEO_FORM, video_form);
 /*
    // Enable progressive to interlaced conversion. Valid only in for interlaced output mode. 
    // Must be 0 for progressive output mode      
    if ((m_eTVStd == TV_STAND_720P60)  || (m_eTVStd == TV_STAND_720P30)  ||
        (m_eTVStd == TV_STAND_720P25)  || (m_eTVStd == TV_STAND_720P24)  ||
        (m_eTVStd == TV_STAND_1080P30) || (m_eTVStd == TV_STAND_1080P25) ||
        (m_eTVStd == TV_STAND_1080P24))
    {
         data |= CSP_BITFVAL( TVEV2_P2I_CONV_EN, TVEV2_P2I_CONV_EN_DISABLE);   
    
    }
    else
    {
         data |= CSP_BITFVAL( TVEV2_P2I_CONV_EN, TVEV2_P2I_CONV_EN_ENABLE);   
    }
 */
    
    data |= CSP_BITFVAL( TVEV2_TV_STAND, m_eTVStd)                                  |
            CSP_BITFVAL( TVEV2_TV_OUT_MODE, m_eTVOutputMode)                        | 
            CSP_BITFVAL( TVEV2_SYNC_CH_0_EN, TVEV2_SYNC_CH_0_EN_ENABLE );
                     
    // reset the COM_CONF_REG register with the new settings
    OUTREG32(&m_pTVE->COM_CONF_REG, data);
    
}


//-----------------------------------------------------------------------------
//
//  Function: TveConfigureInterruptCtrlReg
//
//  This function is to configure the TVE Interrupt Control Register. 
//
//  Parameters:
//
//     None.  
//
//  Returns:
//     None.
//
//-----------------------------------------------------------------------------
void TveClass::TveConfigureInterruptCtrlReg()
{
    int data=0;
    
    data = CSP_BITFVAL( TVEV2_CD_LM_IEN, TVEV2_CD_LM_IEN_ENABLE)                 |
           CSP_BITFVAL( TVEV2_CD_SM_IEN, TVEV2_CD_SM_IEN_ENABLE)                 |
           CSP_BITFVAL( TVEV2_CD_MON_END_IEN, TVEV2_CD_MON_END_IEN_ENABLE)       |
           CSP_BITFVAL( TVEV2_TVE_FIELD_END_IEN, TVEV2_TVE_FIELD_END_IEN_ENABLE) |
           CSP_BITFVAL( TVEV2_TVE_FRAME_END_IEN, TVEV2_TVE_FRAME_END_IEN_ENABLE) |
           CSP_BITFVAL( TVEV2_SA_MEAS_END_IEN, TVEV2_SA_MEAS_END_IEN_ENABLE);
           
    if (m_bHD)
    {
        // Enable HD related, disable SD related
        data |= CSP_BITFVAL( TVEV2_CC_SD_F1_DONE_IEN, TVEV2_CC_SD_F1_DONE_IEN_DISABLE)        |
                CSP_BITFVAL( TVEV2_CC_SD_F2_DONE_IEN, TVEV2_CC_SD_F2_DONE_IEN_DISABLE)        |
                CSP_BITFVAL( TVEV2_CGMS_SD_F1_DONE_IEN, TVEV2_CGMS_SD_F1_DONE_IEN_DISABLE)    |
                CSP_BITFVAL( TVEV2_CGMS_SD_F2_DONE_IEN, TVEV2_CGMS_SD_F2_DONE_IEN_DISABLE)    |
                CSP_BITFVAL( TVEV2_WSS_SD_DONE_IEN, TVEV2_WSS_SD_DONE_IEN_DISABLE)            |
                CSP_BITFVAL( TVEV2_CGMS_HD_A_F1_DONE_IEN, TVEV2_CGMS_HD_A_F1_DONE_IEN_ENABLE) |
                CSP_BITFVAL( TVEV2_CGMS_HD_A_F2_DONE_IEN, TVEV2_CGMS_HD_A_F2_DONE_IEN_ENABLE) |
                CSP_BITFVAL( TVEV2_CGMS_HD_B_F1_DONE_IEN, TVEV2_CGMS_HD_B_F1_DONE_IEN_ENABLE) |
                CSP_BITFVAL( TVEV2_CGMS_HD_B_F2_DONE_IEN, TVEV2_CGMS_HD_B_F2_DONE_IEN_ENABLE);
    }
    else
    {
        // Enable SD related, disable HD related
        data |= CSP_BITFVAL( TVEV2_CC_SD_F1_DONE_IEN, TVEV2_CC_SD_F1_DONE_IEN_ENABLE)          |
                CSP_BITFVAL( TVEV2_CC_SD_F2_DONE_IEN, TVEV2_CC_SD_F2_DONE_IEN_ENABLE)          |
                CSP_BITFVAL( TVEV2_CGMS_SD_F1_DONE_IEN, TVEV2_CGMS_SD_F1_DONE_IEN_ENABLE)      |
                CSP_BITFVAL( TVEV2_CGMS_SD_F2_DONE_IEN, TVEV2_CGMS_SD_F2_DONE_IEN_ENABLE)      |
                CSP_BITFVAL( TVEV2_WSS_SD_DONE_IEN, TVEV2_WSS_SD_DONE_IEN_ENABLE)              |
                CSP_BITFVAL( TVEV2_CGMS_HD_A_F1_DONE_IEN, TVEV2_CGMS_HD_A_F1_DONE_IEN_DISABLE) |
                CSP_BITFVAL( TVEV2_CGMS_HD_A_F2_DONE_IEN, TVEV2_CGMS_HD_A_F2_DONE_IEN_DISABLE) |
                CSP_BITFVAL( TVEV2_CGMS_HD_B_F1_DONE_IEN, TVEV2_CGMS_HD_B_F1_DONE_IEN_DISABLE) |
                CSP_BITFVAL( TVEV2_CGMS_HD_B_F2_DONE_IEN, TVEV2_CGMS_HD_B_F2_DONE_IEN_DISABLE);
    }
    
    // reset the INT_CONT_REG register with the new settings
    OUTREG32(&m_pTVE->INT_CONT_REG, data);
    
}


//-----------------------------------------------------------------------------
//
//  Function: TveConfigureLumaCtrlReg
//
//  This function is to configure the TVE Luma Control Registers. 
//
//  Parameters:
//
//     None.  
//
//  Returns:
//     None.
//
//-----------------------------------------------------------------------------
void TveClass::TveConfigureLumaCtrlReg()
{
    int data=0;
    
    // supplementary filter
    data = CSP_BITFVAL( TVEV2_LUMA_FILT_USR_MODE_EN, 0);
    OUTREG32(&m_pTVE->USER_MODE_CONT_REG, data);  
    
    // reset the LUMA_FILT_CONT_REG_0 register with the new settings
    data = CSP_BITFVAL( TVEV2_DEFLICK_EN, TVEV2_DEFLICK_EN_ENABLE)                | 
           CSP_BITFVAL( TVEV2_DEFLICK_MEAS_WIN, TVEV2_DEFLICK_MEAS_WIN_1_PIX_WIN) |
           CSP_BITFVAL( TVEV2_DEFLICK_COEF, TVEV2_DEFLICK_COEF_0_125) |
           CSP_BITFVAL( TVEV2_DEFLICK_LOW_THRESH, 1) |
           CSP_BITFVAL( TVEV2_DEFLICK_MID_THRESH, 9) | 
           CSP_BITFVAL( TVEV2_DEFLICK_HIGH_THRESH, 9); 
    OUTREG32(&m_pTVE->LUMA_FILT_CONT_REG_0, data);
    
    
    // reset the LUMA_FILT_CONT_REG_1 register with the new settings
    data = CSP_BITFVAL( TVEV2_V_SHARP_EN, TVEV2_V_SHARP_EN_ENABLE)  | 
           CSP_BITFVAL( TVEV2_V_SHARP_COEF, TVEV2_V_SHARP_COEF_0_125);
    OUTREG32(&m_pTVE->LUMA_FILT_CONT_REG_1, data);
    
    
    // reset the LUMA_FILT_CONT_REG_2 register with the new settings
    data = CSP_BITFVAL( TVEV2_H_SHARP_EN, TVEV2_H_SHARP_EN_ENABLE)  | 
           CSP_BITFVAL( TVEV2_H_SHARP_COEF, TVEV2_H_SHARP_COEF_0_125) | 
           CSP_BITFVAL( TVEV2_H_SHARP_LOW_THRESH, 1) | 
           CSP_BITFVAL( TVEV2_H_SHARP_HIGH_THRESH, 10);   
            
    OUTREG32(&m_pTVE->LUMA_FILT_CONT_REG_2, data);
    
    
    // reset the LUMA_FILT_CONT_REG_3 register with the new settings
    data = CSP_BITFVAL( TVEV2_DERING_EN, TVEV2_DERING_EN_ENABLE) | 
           CSP_BITFVAL( TVEV2_DERING_COEF, TVEV2_DERING_COEF_0_125) |
           CSP_BITFVAL( TVEV2_DERING_LOW_THRESH, 1) |
           CSP_BITFVAL( TVEV2_DERING_MID_THRESH, 4) | 
           CSP_BITFVAL( TVEV2_DERING_HIGH_THRESH, 10);     
    
    if (m_eTVStd == TV_STAND_NTSC) 
    {
         data |= CSP_BITFVAL( TVEV2_SUPP_FILTER_TYPE, TVEV2_SUPP_FILTER_TYPE_NTSC_NOTCH_FILT);
    }   
    else if ((m_eTVStd == TV_STAND_PALM) || (m_eTVStd == TV_STAND_PALN) || (m_eTVStd == TV_STAND_PAL))   
    {
         data |= CSP_BITFVAL( TVEV2_SUPP_FILTER_TYPE, TVEV2_SUPP_FILTER_TYPE_PAL_NOTCH_FILT);
    }   
    else
    {
        data |= CSP_BITFVAL( TVEV2_SUPP_FILTER_TYPE, TVEV2_SUPP_FILTER_TYPE_LOWPASS_FILT);
    }
    
    OUTREG32(&m_pTVE->LUMA_FILT_CONT_REG_3, data);
}



//-----------------------------------------------------------------------------
//
//  Function: TveConfigureChromaCtrlReg
//
//  This function is to configure the TVE Chroma Control Register. 
//
//  Parameters:
//
//     None.  
//
//  Returns:
//     None.
//
//-----------------------------------------------------------------------------
void TveClass::TveConfigureChromaCtrlReg()
{
    int data=0;
    
    if (m_bHD)
    {
        data = CSP_BITFVAL( TVEV2_CHROMA_V_FILT_EN, TVEV2_CHROMA_V_FILT_EN_DISABLE) | // Must be 0 for HD mode
               CSP_BITFVAL( TVEV2_CHROMA_BW, TVEV2_CHROMA_BW_4);  // default setting: 16.5 MHz for HD mode and YCbCr4:2:2 input format        
    }
    else
    {  
     
        if (m_eTVStd == TV_STAND_NTSC) 
        {
            data = CSP_BITFVAL( TVEV2_CHROMA_BW, TVEV2_CHROMA_BW_1) |
                   CSP_BITFVAL( TVEV2_CHROMA_V_FILT_EN, TVEV2_CHROMA_V_FILT_EN_ENABLE);  // Enable vertical chroma filter for SD
        }
               
    }
    
    // reset the CHROMA_CONT_REG register with the new settings
    OUTREG32(&m_pTVE->CHROMA_CONT_REG, data);
    
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetClock
//
//  This function sets TVE clock.
//
//  Parameters:
//     clock_frequency
//           [in] clock frequency to set
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL TveClass::TveSetClock(DWORD clock_frequency)
{
    // The current DDKCSP Clock API just provides 216MHz and 297MHz modes, 
    // we can use m_bHD for setting the clock.
    UNREFERENCED_PARAMETER(clock_frequency);
    
    if (!CSPTVESetClock(m_bHD))
    {
            DEBUGMSG(1, (TEXT("%s: CSPTVESetClock failed!\r\n"), __WFUNCTION__));
            return FALSE;
    }
        
    return TRUE;   
}


//------------------------------------------------------------------------------
//
//  Function:  TveIST
//
//  This function is the TVE IST thread.
//  
//  Parameters:  
//      LPVOID lpParameter
//
//  Returns:  
//      None
// 
//------------------------------------------------------------------------------
void WINAPI TveClass::TveIST(LPVOID lpParameter)
{
    TveClass *pTVE = (TveClass *)lpParameter;

    
    pTVE->TveISTLoop(INFINITE);
    return;
}

//-----------------------------------------------------------------------------
//
// Function: TveISTLoop
//
// This function is the interrupt handler for the TVE events.
// It waits for an interrupt from the TVE, and signals the appropriate
// event associated with the interrupt.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for an interrupt.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void TveClass::TveISTLoop(UINT32 timeout)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(timeout);
    
#if 0
    DWORD statReg;
    
    CeSetThreadPriority(GetCurrentThread(), 97);

    while (bIntrThreadLoop)
    {
        WaitForSingleObject(m_hTveIntrEvent, timeout);  
        
        if(!bIntrThreadLoop)
            break;
            
        statReg = INREG32(&m_pTVE->STAT_REG);
       
        if (statReg & 0x02000000) // field end
        {
            // clear interrupt
            SETREG32(&m_pTVE->STAT_REG, 0x02000000);
            SetEvent(m_hTveFieldEvent);
        }
   
        if (statReg & 0x01000000) // Frame end
        {
            // clear interrupt
            SETREG32(&m_pTVE->STAT_REG, 0x01000000);
            SetEvent(m_hTveFrameEvent);
        }
    
        if (statReg & 0x00000080) // CGMS F2 done
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000080);
           SetEvent(m_hTveCgmsF2Event);
        }
        
        if (statReg & 0x00000040) // CGMS F1 done
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000040);
           SetEvent(m_hTveCgmsF1Event);
        }
        
        if (statReg & 0x00000020) // CC F2 done
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000020);
           SetEvent(m_hTveCcF2Event);
        }
        
        if (statReg & 0x00000010) // CC F1 done
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000010);
           SetEvent(m_hTveCcF1Event);
        }
        
        if (statReg & 0x00000004) // CD Monitoring end
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000004);
           SetEvent(m_hTveCdMonEvent);
        }
        
        if (statReg & 0x00000004) //C D Short detected
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000004);
           SetEvent(m_hTveCdMonEvent);
        }
        
        if (statReg & 0x00000002) // CD Short detected
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000002);
           SetEvent(m_hTveCdSmEvent);
        }
        
        if (statReg & 0x00000001) // CD Cable detected
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000001);
           SetEvent(m_hTveCdLmEvent);
        }
        
        // Kernel call to unmask the interrupt so that it can be signalled again
        InterruptDone(dwSysIntr);
    }
#endif
    return;
}



void TveClass::TveDumpRegs()
{
    int i;
    
    UINT32* ptr = (UINT32 *)&m_pTVE->COM_CONF_REG;
    RETAILMSG (1, (TEXT("\n\nDump TVE Registers \n")));
    for (i = 0; i < 14; i++) // 56 registers
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), 
                      (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
   
}
  
