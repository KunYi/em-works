//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

// File:  gpt_priv.h
//
// Private definitions for General Purpose Timer Driver
//
//------------------------------------------------------------------------------

#ifndef __GPT_PRIV_H__
#define __GPT_PRIV_H__

#include "gpt.h"

typedef struct {
    PCSP_GPT_REGS pGPT;
    CRITICAL_SECTION hGptLock;
    DWORD dwGptIntr;
    DWORD dwGptIrq;
    DWORD dwGptIndex;
    HANDLE hGptIntrEvent;
    HANDLE hTimerThread;
    HANDLE hTimerEvent ;
    LPCTSTR TimerEventString;
    timerSrc_c ClkSrc;
    BOOL bCurrentlyOwned;
}CSP_GPT_STRUCT,*PCSP_GPT_STRUCT;

//------------------------------------------------------------------------------
// Functions

DWORD GptInitialize(LPCTSTR pContext);
void GptRelease(PCSP_GPT_STRUCT pController);
BOOL GptTimerCreateEvent(PCSP_GPT_STRUCT pController, LPTSTR);
BOOL GptTimerReleaseEvent(PCSP_GPT_STRUCT pController, LPTSTR);
BOOL GptEnableTimer(PCSP_GPT_STRUCT pController);
void GptDisableTimer(PCSP_GPT_STRUCT pController);
void GptEnableTimerInterrupt(PCSP_GPT_STRUCT pController);
void GptDisableTimerInterrupt(PCSP_GPT_STRUCT pController);
BOOL GptResetTimer(PCSP_GPT_STRUCT pController);
BOOL GptGetTimerCount(PCSP_GPT_STRUCT pController, PDWORD pGptCntValue);
BOOL GptUpdateTimerPeriod(PCSP_GPT_STRUCT pController, DWORD Period);
BOOL GptSetTimerMode(PCSP_GPT_STRUCT pController, timerMode_c TimerMode);
void GptShowClkSrc(PCSP_GPT_STRUCT pController);
void GptChangeClkSrc(PCSP_GPT_STRUCT pController, PGPT_TIMER_SRC_PKT pSetSrcPkt);


#endif   // __GPT_PRIV_H__
