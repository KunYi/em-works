//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  wm8580.h
//
//  Definition of Multichannel Audio CODEC WM8580.
//
//------------------------------------------------------------------------------
#ifndef __WM8580_H
#define __WM8580_H

#include "audiotypes.h"

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Types


typedef enum
{
    WM8580_DAC_1 = 1,
    WM8580_DAC_2 = 2,
    WM8580_DAC_3 = 4,
    WM8580_DAC_MASTER_ALL = 8
}WM8580_DAC_MASK;

typedef enum
{
    WM8580_OUTPUT_CHN1  = 1,
    WM8580_OUTPUT_CHN2 = 2,
    WM8580_OUTPUT_CHN3 = 4,
    WM8580_OUTPUT_CHN4 = 8,
    WM8580_OUTPUT_CHN5 = 16,
    WM8580_OUTPUT_CHN6 = 32
}WM8580_OUTPUT_CHN_MASK;




//-----------------------------------------------------------------------------
// Classes
class CWM8580
{
private:
    HANDLE m_hCSPI;
    UINT8 m_deviceID1;
    UINT8 m_deviceID2;

    DWORD m_dwInputSampleRate;
    DWORD m_dwInputBitDepth;

    DWORD m_dwOutputSampleRate;
    DWORD m_dwOutputBitDepth;
    DWORD m_dwOutputChnMask;   //mask for chn1,chn2,chn3,chn4,chn5,chn6
    AUDIO_PROTOCOL m_audioProtocol;

    DWORD m_dwDACMask;  // mask for dac1, dac2, dac3

    BOOL m_bDACPowerOn;
    BOOL m_bADCPowerOn;

    BOOL m_bCLKOEnable;
    UINT8 ReadRegister(DWORD reg);
    BOOL WriteRegister(DWORD reg, DWORD val);

public:
    CWM8580 ();
    ~CWM8580 ();
    BOOL InitCodec();
    BOOL GetDeviceID();
    BOOL ConfigOutput(DWORD dwSampleRate, DWORD dwBitDepth, 
    DWORD dwChnMask,DWORD dwChnNum,AUDIO_PROTOCOL audioProtocal);
    BOOL EnableDAC();
    BOOL DisableDAC();
    BOOL SetOutputGain(DWORD dwGainDAC1, DWORD dwGainDAC2,
        DWORD dwGainDAC3, DWORD dwMask);
    BOOL SetOutputMute(BOOL bMute,DWORD dwMask);

    UINT32 GetDACBitClock(void);
    BOOL EnableCLKO(void);

};

#endif  // __WM8580_H
