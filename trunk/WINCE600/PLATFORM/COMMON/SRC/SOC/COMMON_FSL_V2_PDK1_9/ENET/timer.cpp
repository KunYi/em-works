//------------------------------------------------------------------------------
//
//  Copyright (C) 2010£¬ Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  ptp.c
//
//  Implementation of PTP basic function
//
//  This file implements the 1588  timer basic function  for ENET .
//
//-----------------------------------------------------------------------------
#include "common_macros.h"
#include "enet.h"


extern PVOID pv_HWregENET0;
extern PVOID pv_HWregENET1;

#define DEFS_ONE_SECOND_COUNT (1000*1000*1000)

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: ENETClass::InitTimer
// 
// This function is the init the PTP timer
//
// Parameters:
//        None
//  
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::InitTimer()
{
     
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +InitTimer\r\n")));
    
    HW_ENET_MAC_ATIME_EVT_PERIOD_WR(index,1000000000); // set period of 1 second (1*10^9 ns)
       
    HW_ENET_MAC_ATIME_INC_WR(index,0x00001919);  // set INC and CORR = 20 (40 for 25MHz, 20 for 50MHz, 10 for 100MHz, 8 for 125MHz, ...)

    BW_ENET_MAC_ATIME_CTRL_ENABLE(index,1);

    BW_ENET_MAC_ATIME_CTRL_FRC_SLAVE(index,index);//for MAC 0 timer for master  MAC1  timer for slave select

    BW_ENET_MAC_ATIME_CTRL_EVT_PERIOD_ENA(index,1);  // enable periodical events

    BW_ENET_MAC_ATIME_CTRL_EVT_PERIOD_RST(index,1);  // reset timer at each period event (enable modulo)

    BW_ENET_MAC_ATIME_CTRL_PIN_PERIOD_ENA(index,1);   // enable external pin (frc_evt_period)

    CurTimer.sec=0;
    CurTimer.nsecs=0;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: -InitTimer\r\n"),index));
    
    return;

}

//------------------------------------------------------------------------------
//
// Function: ENETClass::UpdateCurrentTimer
// 
// This function is Update  the PTP timer (sencond and nonsecond) 
//
// Parameters:
//        None
//  
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::UpdateCurrentTimer(void)
{   
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +UpdateCurrentTimer\r\n")));
    EnterCriticalSection (&gENETTimerCs);

    CurTimer.sec++;
    BW_ENET_MAC_ATIME_CTRL_CAPTURE(index,1);
    
    // Get the ns value from TSM_ATIME register
    CurTimer.nsecs = HW_ENET_MAC_ATIME_RD(index);
    
    LeaveCriticalSection (&gENETTimerCs);
  
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: Timestamp:  %d  %d   \r\n"),index, CurTimer.sec,CurTimer.nsecs));
}


//------------------------------------------------------------------------------
//
// Function: ENETClass::GetPTPTxTimer
// 
// This function is Get PTP frame Timer 
//
// Parameters:
//        pTxTimer
//        point to TxTimer
//  
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::GetPTPTxTimer(TimeRepr *pTxTimer)
{
    EnterCriticalSection (&gENETTimerCs);
    pTxTimer->sec=CurTimer.sec; 

    // Get the ns value from TSM_ATIME register
    pTxTimer->nsecs= HW_ENET_MAC_TS_TIMESTAMP_RD(index);
    
    LeaveCriticalSection (&gENETTimerCs);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: GetPTPTxTimer Timestamp:%d %d\r\n"), index,pTxTimer->sec, pTxTimer->nsecs));

}

//------------------------------------------------------------------------------
//
// Function: ENETClass::SetPTPTimer
// 
// This function is Set PTP frame Timer 
//
// Parameters:
//        pTimer
//        point to Timer
//  
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::SetPTPTimer(TimeRepr *pTimer)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: ++ SetPTPTimer\r\n")));
    CeSetThreadPriority(GetCurrentThread(), 0);
    
    EnterCriticalSection (&gENETTimerCs);
  
    int tempns=0,setns=0;
    RETAILMSG(0, (TEXT("ENET%x: GetPTPTxTimer Timestamp: %d %d\r\n"),index,CurTimer.sec,pTimer->sec));
    CurTimer.sec=pTimer->sec+ CurTimer.sec;
    
    BW_ENET_MAC_ATIME_CTRL_CAPTURE(index,1);
    
    tempns=HW_ENET_MAC_ATIME_RD(index);
    
    RETAILMSG(0, (TEXT("ENET%x: GetPTPTxTimer Timestamp: %x\r\n"),index, tempns));

    BW_ENET_MAC_ATIME_CTRL_CAPTURE(index,0);
    
    setns=tempns+pTimer->nsecs+10575;
    if(setns>DEFS_ONE_SECOND_COUNT)
    {
        HW_ENET_MAC_ATIME_WR(index,setns-DEFS_ONE_SECOND_COUNT);
        CurTimer.sec=CurTimer.sec+1;
    }else
     HW_ENET_MAC_ATIME_WR(index,setns);
    
    LeaveCriticalSection (&gENETTimerCs);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x:SetPTPTimer: Timestamp:%x %x\r\n"),index, pTimer->sec, pTimer->nsecs));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::GetPTPRxTimer
// 
// This function is Get PTP frame Rx Timer 
//
// Parameters:
//        pTimer
//                  point to Timer
//        Timestamp
//                  Timestamp of the Enhanced uDMA Receive Buffer Descriptor            
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::GetPTPRxTimer(RxTimeRepr *pRxTimer, ULONG Timestamp, DWORD sequenceId, DWORD messageType)
{

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ProcessPTPTxTimer GetPTPRxTimer\r\n")));
    EnterCriticalSection (&gENETTimerCs);
    
    pRxTimer->sec=CurTimer.sec;
    // Get the ns value from TSM_ATIME register
    pRxTimer->nsecs= Timestamp;
    pRxTimer->sequenceId= sequenceId;
    pRxTimer->messageType= messageType;
    LeaveCriticalSection (&gENETTimerCs);
 
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: GetPTPRxTimer: Timestamp:%x %x\r\n"),index, pRxTimer->sec, pRxTimer->nsecs));
 
}