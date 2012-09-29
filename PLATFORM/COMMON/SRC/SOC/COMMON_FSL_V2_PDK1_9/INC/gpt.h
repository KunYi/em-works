//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:  gpt.h
//
// Public definitions for General Purpose Timer Driver
//
//------------------------------------------------------------------------------
#ifndef __GPT_H__
#define __GPT_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Types

// GPT timer mode
typedef enum timerMode{
    timerModePeriodic,
    timerModeFreeRunning
} timerMode_c;

// GPT timer source
typedef enum timerSrc{
    GPT_NOCLK,
    GPT_IPGCLK,
    GPT_HIGHCLK,
    GPT_EXTCLK,
    GPT_32KCLK
} timerSrc_c;

// GPT set timer mode and period 
typedef struct
{
    timerMode_c timerMode;
    UINT32 period;
    timerSrc_c timerSrc;
} GPT_Config, *pGPT_Config;

// GPT timer source packet
typedef struct
{
    timerSrc_c timerSrc;
} GPT_TIMER_SRC_PKT, *PGPT_TIMER_SRC_PKT;
//------------------------------------------------------------------------------
// Functions

HANDLE GptOpenHandle(LPCWSTR lpDevName);
BOOL GptCloseHandle(HANDLE);
HANDLE GptCreateTimerEvent(HANDLE, LPTSTR);
BOOL  GptReleaseTimerEvent(HANDLE, LPTSTR);
BOOL  GptStart(HANDLE, pGPT_Config);
BOOL  GptResume(HANDLE);
BOOL  GptStop(HANDLE);
BOOL  GptGetCounterValue(HANDLE,PDWORD);
BOOL  GptShowTimerSrc(HANDLE hGpt);
//------------------------------------------------------------------------------
// Defines
#define GPT_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define GPT_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT                     0
#define ZONEID_DEINIT                   1
#define ZONEID_IOCTL                    2

#define ZONEID_INFO                     12
#define ZONEID_FUNCTION                 13
#define ZONEID_WARN                     14
#define ZONEID_ERROR                    15

// Debug zone masks
#define ZONEMASK_INIT                   (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT                 (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL                  (1<<ZONEID_IOCTL)

#define ZONEMASK_INFO                   (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION               (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN                   (1<<ZONEID_WARN)
#define ZONEMASK_ERROR                  (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
//#define ZONE_INIT                       DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT                     DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL                      DEBUGZONE(ZONEID_IOCTL)


#define ZONE_INFO                       DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION                   DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN                       DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR                      DEBUGZONE(ZONEID_ERROR)
#endif

// IOCTL to create GPT timer event handle
#define GPT_IOCTL_TIMER_CREATE_EVENT    1
// IOCTL to close GPT timer event handle
#define GPT_IOCTL_TIMER_RELEASE_EVENT   2
// IOCTL to start the GPT timer
#define GPT_IOCTL_TIMER_START           3
// IOCTL to stop the GPT timer
#define GPT_IOCTL_TIMER_STOP            4
// IOCTL to set the delay between
// interrupts for the GPT timer in ms
#define GPT_IOCTL_TIMER_UPDATE_PERIOD   5
// To obtain GPT main counter value
#define GPT_IOCTL_TIMER_GET_COUNT       6
// To resume(enable)the GPT timer from stop
#define GPT_IOCTL_TIMER_RESUME          7
#define GPT_IOCTL_TIMER_CHANGE_CLKSRC   8
#define GPT_IOCTL_TIMER_SHOW_CLKSRC     9

#ifdef __cplusplus
}
#endif

#endif   // __GPT_H__
