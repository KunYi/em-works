//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//

#ifndef __HWAUDIOBRDG_H__
#define __HWAUDIOBRDG_H__

#include <pm.h>
#include "wavext.h"

//------------------------------------------------------------------------------
//  Defines a callback interface used by CHardwareAudioBridge to copy
// audio data to a buffer to be rendered
//
class CStreamCallback
{
public:
    typedef struct
        {
        DWORD           SampleRate;             // f32.0
        DWORD           InvSampleRate;          // f0.32
        }
    SampleRateEntry_t;

    typedef enum
        {
        k8khz = 0,
        k16khz,
        k44khz,
        k48khz,
        k96khz,     // added 96k sample frequency
        kAudioSampleRateSize
        }
    AudioSampleRate_e;

    //--------------------------------------------------------------------------
    // public methods
    //
public:
    virtual DWORD copy_AudioData(void* pStart, DWORD nSize)              = 0;
    virtual void put_AudioSampleRate(AudioSampleRate_e rate)             = 0;
    virtual DWORD copy_StreamData(HANDLE hContext, void* pStart, DWORD nSize) = 0;
};


//------------------------------------------------------------------------------
//
//  Serves as a base class for all hw audio bridge
//  - defines a common interface so all audio components can communicate with it
//
class CHardwareAudioBridge
{
    //--------------------------------------------------------------------------
    // typedef, enum, structs
    //
public:

    enum StreamType
        {
        kOutput = 0,
        kInput,
        kStreamTypeCount
        };


    //--------------------------------------------------------------------------
    // member variables
    //
protected:

    CEDEVICE_POWER_STATE            m_PowerState;

    BOOL                            m_bBTHeadsetAttached;
    DWORD                           m_dwHdmiAudioAttached;
    DWORD                           m_dwHeadsetCount;
    DWORD                           m_dwSpeakerCount;
    DWORD                           m_dwWideBandCount;

    CStreamCallback                *m_prgStreams[kStreamTypeCount];


    //--------------------------------------------------------------------------
    // constructor/destructor
    //
public:
    CHardwareAudioBridge()
    {
        memset(m_prgStreams, 0, sizeof(CStreamCallback*) * kStreamTypeCount);
        m_PowerState = D4;

        m_dwHeadsetCount = 0;
        m_dwSpeakerCount = 0;
        m_dwWideBandCount = 0;
        m_bBTHeadsetAttached = FALSE;
        m_dwHdmiAudioAttached = FALSE;
    }


    //--------------------------------------------------------------------------
    // public methods
    //
public:

    void put_StreamCallback(StreamType type, CStreamCallback *pCallback)
    {
        m_prgStreams[type] = pCallback;
    }

    //--------------------------------------------------------------------------
    // public virtual methods
    //
public:

    // this should just enable the clocks necessary for the hardware
    //
    virtual void power_On()                            {}

    // this should immediately turn-off all power into the hardware
    //
    virtual void power_Off()                           {}

    virtual BOOL start_AudioPort(StreamType type)                                    { return FALSE; }
    virtual BOOL stop_AudioPort(StreamType type)                                    { return FALSE; }
    virtual BOOL start_Stream(StreamType type, HANDLE hStreamContext) { return FALSE; }
    virtual BOOL start_Stream(StreamType type) { return FALSE; }
    virtual BOOL stop_Stream(StreamType type, HANDLE hStreamContext) { return FALSE; }
    virtual BOOL set_StreamGain(StreamType type, HANDLE hStreamContext, DWORD dwContextData) {return FALSE;}
    virtual BOOL switch_AudioStreamPort(BOOL bPortRequest) {return FALSE;}
    virtual BOOL query_AudioStreamPort() {return FALSE;}
    virtual BOOL enable_I2SClocks(BOOL bClkEnable) {return FALSE;}

    // calling this routine does not necessarily immediately change the
    // power state; ie when audio is currently being rendered
    //
    virtual void request_PowerState(CEDEVICE_POWER_STATE powerState)
    {
        if (powerState == D0 || powerState == D4)
            {
            m_PowerState = powerState;
            }
    }

    // the current power state
    //
    virtual CEDEVICE_POWER_STATE get_CurrentPowerState() const
    {
        return m_PowerState;
    }

    // audio profiles
    //
    virtual void notify_BTHeadsetAttached(BOOL bAttached)
    {
        m_bBTHeadsetAttached = bAttached;
    }

    virtual void notify_HdmiAudioAttached(BOOL bAttached)
    {
        m_dwHdmiAudioAttached = bAttached;
    }

    virtual BOOL enable_Headset(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwHeadsetCount : --m_dwHeadsetCount;
    }

    virtual BOOL enable_AuxHeadset(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwHeadsetCount : --m_dwHeadsetCount;      
    }

    virtual BOOL enable_Speaker(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwSpeakerCount : --m_dwSpeakerCount;
    }

    virtual BOOL enable_WideBand(BOOL bEnable)
    {
        return (bEnable) ? ++m_dwWideBandCount : --m_dwWideBandCount;
    }

    virtual BOOL query_BTHeadsetAttached() const
    {
        return m_bBTHeadsetAttached;
    }

    virtual BOOL query_HdmiAudioAttached() const
    {
        return m_dwHdmiAudioAttached;
    }

    virtual BOOL query_HeadsetEnable() const
    {
        return m_dwHeadsetCount > 0;
    }

    virtual BOOL query_SpeakerEnable() const
    {
        return m_dwSpeakerCount > 0;
    }

    virtual BOOL query_WideBand() const
    {
        return m_dwWideBandCount > 0;
    }

    virtual BOOL query_CarkitEnable() const             { return FALSE; }
};



#endif // __HWAUDIOBRDG_H__
