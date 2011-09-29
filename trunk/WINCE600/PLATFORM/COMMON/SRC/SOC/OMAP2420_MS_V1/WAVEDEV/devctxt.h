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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
#pragma once

#define SECONDARYGAINCLASSMAX 4

// number of classes affected by the device gain
#define SECONDARYDEVICEGAINCLASSMAX 2  

// by default attenuation settings are adjusted for 
// Windows CE software mixer.
// However, depending on the hardware design the defaults 
// may need adjustment

class DeviceContext
{
public:
    DeviceContext()
    {
        InitializeListHead(&m_StreamList);
        m_dwGain = 0xFFFF;
        m_dwDefaultStreamGain = 0xFFFF;
        for (int i=0;i<SECONDARYGAINCLASSMAX;i++)
        {
            m_dwSecondaryGainLimit[i]=0xFFFF;
        }

#ifdef PROFILE_MIXER
		m_liPCStart.QuadPart = 0;
		m_liPCTotal.QuadPart = 0;
		QueryPerformanceFrequency(&m_liPCFrequency);
#endif
    }

    virtual BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    PBYTE TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, DWORD *pNumStreams);

    void NewStream(StreamContext *pStreamContext);
    void DeleteStream(StreamContext *pStreamContext);

    DWORD GetGain()
    {
        return m_dwGain;
    }

    DWORD SetGain(DWORD dwGain)
    {
        m_dwGain = dwGain;
        RecalcAllGains();
        return MMSYSERR_NOERROR;
    }

    DWORD GetDefaultStreamGain()
    {
        return m_dwDefaultStreamGain;
    }

    DWORD SetDefaultStreamGain(DWORD dwGain)
    {
        m_dwDefaultStreamGain = dwGain;
        return MMSYSERR_NOERROR;
    }

    DWORD GetSecondaryGainLimit(DWORD GainClass)
    {
        return m_dwSecondaryGainLimit[GainClass];
    }

    DWORD SetSecondaryGainLimit(DWORD GainClass, DWORD Limit)
    {
        if (GainClass>=SECONDARYGAINCLASSMAX)
        {
            return MMSYSERR_ERROR;
        }
        m_dwSecondaryGainLimit[GainClass]=Limit;
        RecalcAllGains();
        return MMSYSERR_NOERROR;
    }

    void RecalcAllGains();

    DWORD OpenStream(LPWAVEOPENDESC lpWOD, DWORD dwFlags, StreamContext **ppStreamContext);
    virtual DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual DWORD GetDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual void StreamReadyToRender(StreamContext *pStreamContext)=0;

    virtual StreamContext *CreateStream(LPWAVEOPENDESC lpWOD)=0;

protected:
    LIST_ENTRY  m_StreamList;         // List of streams rendering to/from this device
    DWORD       m_dwGain;
    DWORD       m_dwDefaultStreamGain;
    DWORD       m_dwSecondaryGainLimit[SECONDARYGAINCLASSMAX];

    static DWORD m_dwStreamAttenMax;
    static DWORD m_dwDeviceAttenMax;
    static DWORD m_dwSecondAttenMax;


#ifdef PROFILE_MIXER
public:
	void StartMixerProfiler()
	{
		// reset Counter for this specific output
		m_liPCTotal.QuadPart = 0;
		QueryPerformanceCounter(&m_liPCStart);
	}
	void StopMixerProfiler( LARGE_INTEGER *pliTotalTime, LARGE_INTEGER *pliMixerTime)
	{
		LARGE_INTEGER liPCStop;
		QueryPerformanceCounter(&liPCStop);
		liPCStop.QuadPart -= m_liPCStart.QuadPart;
		m_liPCStart.QuadPart = liPCStop.QuadPart;
		pliTotalTime->QuadPart = (1000*liPCStop.QuadPart)/m_liPCFrequency.QuadPart;
		pliMixerTime->QuadPart = (1000*m_liPCTotal.QuadPart)/m_liPCFrequency.QuadPart;
	}
	LARGE_INTEGER m_liPCStart;
	LARGE_INTEGER m_liPCTotal;
	LARGE_INTEGER m_liPCFrequency;
#endif
};

class InputDeviceContext : public DeviceContext
{
public:
    StreamContext *CreateStream(LPWAVEOPENDESC lpWOD);
    DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize);
    DWORD GetDevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);
};

class OutputDeviceContext : public DeviceContext
{
public:
    BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    StreamContext *CreateStream(LPWAVEOPENDESC lpWOD);
    DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize);
    DWORD GetDevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);
};


