//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header:  tve.h
//
//  Provides definitions for the TV encoder chip.
//
//------------------------------------------------------------------------------

#ifndef __TVE_H
#define __TVE_H

#include "tve_sdk.h"

// TVE Events 

#define TVE_INTR_EVENT                   L"Tve Interrupt Event"
  
#define TVE_FIELD_EVENT_NAME             L"Tve Field Event"
#define TVE_FRAME_EVENT_NAME             L"Tve Frame Event"
#define TVE_CGMS_F2_EVENT_NAME           L"Tve CGMS F2 Event"
#define TVE_CGMS_F1_EVENT_NAME           L"Tve CGMS F1 Event"
#define TVE_CC_F2_EVENT_NAME             L"Tve CC F2 Event"
#define TVE_CC_F1_EVENT_NAME             L"Tve CC F1 Event"
#define TVE_CD_MON_EVENT_NAME            L"Tve CD Mon Event"
#define TVE_CD_SM_EVENT_NAME             L"Tve CD SM Event"
#define TVE_CD_LM_EVENT_NAME             L"Tve CD LM Event"

static BOOL bIntrThreadLoop;             // Interrupt thread loop flag
static HANDLE hTveIST = NULL;
static DWORD dwSysIntr;                  // System Interrupt ID

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define TVE_WAIT_TIMEOUT                    1000

#define TVE_S216_CLK                        216  // 216MHz
#define TVE_S108_CLK                        108  // 108MHz
#define TVE_S54_CLK                         54   // 54MHz
#define TVE_S27_CLK                         27   // 27MHz
 
//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------


#define TVE_FUNCTION_ENTRY() \
    DEBUGMSG(1, (TEXT("++%s\r\n"), __WFUNCTION__))
#define TVE_FUNCTION_EXIT() \
    DEBUGMSG(1, (TEXT("--%s\r\n"), __WFUNCTION__))


typedef enum {
    TVE_CD_TRIGGER_IN_MANUAL = 0, // "on demand" ( on the trigger control generated by software), when CD_TRIG_MODE is 1.
    TVE_CD_TRIGGER_IN_STANDBY,    // standby mode, when TV_OUT_MODE is 000 and CD_TRIG_MODE is 0.
    TVE_CD_TRIGGER_IN_FUNC,       // normal operation mode, when TV_OUT_MODE is 000 and CD_TRIG_MODE is 0.
} TVE_CABLE_DETECTION_MODE;

typedef enum {
    SAMPLING_RATE_216MHZ = 0,
    SAMPLING_RATE_108MHZ = 1,
    SAMPLING_RATE_54MHZ  = 2,
    SAMPLING_RATE_27MHZ  = 3,
    SAMPLING_RATE_NONE   = -1  // invaild sampleing rate
} TVE_TVDAC_SAMPLING_RATE;


//-------------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------------
class TveClass
{

    public:
    
        // tve.cpp
        TveClass();
        ~TveClass();

        BOOL TveInit(void);
        BOOL TveDeinit(void);
        BOOL TveEnable(void);
        BOOL TveDisable(void);
        
        // tve_cgms.cpp
        void TveSetCgmsWssFor626LinePAL(UINT32 cgms_f1_wss_data);
        void TveSetCgmsWssFor525LineNTSC(UINT32 cgms_f1_wss_data, UINT32 cgms_f2_data);

        // tve_closedcaption.cpp
        BOOL TveSetClosedCaption(TVECC cc_f1_odd_field_data, TVECC cc_f2_even_field_data);
        
        
        PTVE_REGS           m_pTVE;
        TVE_TV_OUT_MODE     m_eTVOutputMode; 
        TVE_TV_STAND        m_eTVStd; 
        TVE_TV_RES_SIZE     m_eTVResSize; 
       
        
    private:
        
        BOOL TveConfigureCommonReg(TVE_TV_STAND tv_stand, TVE_TV_OUT_MODE tv_out_mode, TVE_TVDAC_SAMPLING_RATE sampling_rate);
        static void TveIST(LPVOID lpParameter);

        // tve.cpp
        BOOL TveSetClock(DWORD clock_frequency);
        void TveDumpRegs();
        
        // tve_cd.cpp
        BOOL TveSetCableDetection(TVE_CABLE_DETECTION_MODE cd_mode, TVE_TV_STAND tv_stand, TVE_TV_OUT_MODE tv_out_mode);
        BOOL TveIsCableDetected(TVE_TV_OUT_MODE tv_out_mode);

        // tve_tvdac.cpp
        TVE_TVDAC_SAMPLING_RATE TveGetTVDACSampleRateByClock(DWORD tveClock);
        void TveSetClockBySamplingRate(TVE_TVDAC_SAMPLING_RATE sampleRate);
        
        HANDLE m_hTveIntrEvent;    // TVE Interrupt Event

        HANDLE m_hTveFieldEvent;   // TVE Field Event
        HANDLE m_hTveFrameEvent;   // TVE Frame Event
        HANDLE m_hTveCgmsF2Event;  // TVE CGMS F2 Event 
        HANDLE m_hTveCgmsF1Event;  // TVE CGMS F1 Event
        HANDLE m_hTveCcF2Event;    // TVE CC F2 Event
        HANDLE m_hTveCcF1Event;    // TVE CC F1 Event
        HANDLE m_hTveCdMonEvent;   // TVE CD MON Event
        HANDLE m_hTveCdSmEvent;    // TVE CD SM Event
        HANDLE m_hTveCdLmEvent;    // TVE CD LM Event

        
};


#endif // __TVE_H


