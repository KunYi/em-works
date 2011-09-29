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

#define VOLSHIFT (32-BITSPERSAMPLE)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG

#define ZONE_AC        DEBUGZONE(0)
#define ZONE_PARAMS     DEBUGZONE(1)
#define ZONE_VERBOSE    DEBUGZONE(2)
#define ZONE_IRQ        DEBUGZONE(3)
#define ZONE_WODM       DEBUGZONE(4)
#define ZONE_WIDM       DEBUGZONE(5)
#define ZONE_PDD        DEBUGZONE(6)
#define ZONE_MDD        DEBUGZONE(7)
#define ZONE_DMA        DEBUGZONE(8)
#define ZONE_MISC       DEBUGZONE(9)
#define ZONE_MIDI       DEBUGZONE(10)
#define ZONE_MODEM      DEBUGZONE(11)
#define ZONE_POWER      DEBUGZONE(12)
#define ZONE_FUNCTION   DEBUGZONE(13)
#define ZONE_WARN       DEBUGZONE(14)
#define ZONE_ERROR      DEBUGZONE(15)

#endif

#ifdef __cplusplus
}
#endif

// Define the format of the m_Delta value. By default, we'll use
// 17.15 format. This may seem weird, but there's a problem using
// 16.16 format with the SRC code-- in the following equation where
// we use the fractional part to calculate the interpolated sample:
//        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
// If CurrT is a 16-bit fraction value, and if (CurrSamp0-PrevSamp0) overflow a
// signed 16-bit number (which they can), then the result can overflow a 32-bit
// value and cause a click.

// We could use 64-bit math to overcome that, but it would be a few extra instructions in the inner loop.
//
// On the other hand, we want as much accuracy in the fractional part as possible,
// as that determines how close we are to the target rate during sample rate conversion.
// Therefore, using 17.15 format is the best we can do.

#define DELTAINT  (17)                // was 24
#define DELTAFRAC (32 - DELTAINT)

// Define 1.0 value for Delta
#define DELTA_OVERFLOW (1<<DELTAFRAC) // was 0x100

class StreamContext
{
public:
    LIST_ENTRY  m_Link;               // Link into list of streams

    StreamContext() {};
    virtual ~StreamContext() {};

    LONG AddRef();
    LONG Release();

    virtual HRESULT Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags);
    virtual DWORD Close();
    virtual DWORD GetPos(PMMTIME pmmt);

    virtual DWORD Run();
    virtual DWORD Stop();
    virtual DWORD Reset();
    virtual PBYTE Render(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)=0;

    BOOL StillPlaying() {return (m_lpWaveHdrHead!=NULL);}
    DWORD GetByteCount() {return m_dwByteCount;}
    WAVEFORMATEX *GetWaveFormat() {return &m_WaveFormat;}
    DeviceContext *GetDeviceContext() { return m_pDeviceContext; }

    void DoDriverCallback(UINT msg, DWORD dwParam1, DWORD dwParam2)
    {
        m_pfnCallback(m_hWave,msg,m_dwInstance,dwParam1,dwParam2);
    }
    virtual void DoCallbackReturnBuffer(LPWAVEHDR lpHdr)
    {
        DoDriverCallback(WOM_DONE,(DWORD)lpHdr,0);
    }
    virtual void DoCallbackStreamOpened()
    {
        g_pHWContext->OutputStreamOpened();
        DEBUGMSG(ZONE_WODM && ZONE_VERBOSE,(TEXT("WODM: DoCallbackStreamOpened\r\n")));
        DoDriverCallback(WOM_OPEN,0,0);
    }
    virtual void DoCallbackStreamClosed()
    {
        g_pHWContext->OutputStreamClosed();
        DEBUGMSG(ZONE_WODM && ZONE_VERBOSE,(TEXT("WODM: DoCallbackStreamClosed\r\n")));
        DoDriverCallback(WOM_CLOSE,0,0);
    }

    virtual DWORD QueueBuffer(LPWAVEHDR lpWaveHdr);
    PBYTE GetNextBuffer();

    // Default implementation
    void ReturnBuffer(LPWAVEHDR lpHdr)
    {
        lpHdr->dwFlags &= ~WHDR_INQUEUE;
        lpHdr->dwFlags |= WHDR_DONE;
        DEBUGMSG(ZONE_WODM && ZONE_VERBOSE,(TEXT("WODM: DoCallbackStreamClosed\r\n")));
        DoCallbackReturnBuffer(lpHdr);
    }

    DWORD GetGain()
    {
        return m_dwGain;
    }

    DWORD SetGain(DWORD dwGain)
    {
        m_dwGain = dwGain;
        GainChange();
        return MMSYSERR_NOERROR;
    }

    DWORD SetSecondaryGainClass(DWORD GainClass)
    {
        if (GainClass>=SECONDARYGAINCLASSMAX)
        {
            return MMSYSERR_ERROR;
        }
        m_SecondaryGainClass=GainClass;
        GainChange();
        return MMSYSERR_NOERROR;
    }

    DWORD MapGain(DWORD Gain);

    virtual void GainChange()
    {
        m_fxpGain = MapGain(m_dwGain);
    }

    static void ClearBuffer(PBYTE pStart, PBYTE pEnd) {memset(pStart,0,pEnd-pStart);}

    DWORD BreakLoop();

    DWORD ForceSpeaker (BOOL bForceSpeaker);

protected:
    LONG                m_RefCount;
    BOOL                m_bRunning;         // Is stream running or stopped

    DWORD               m_dwFlags;          // allocation flags
    HWAVE               m_hWave;            // handle for stream
    DRVCALLBACK*        m_pfnCallback;      // client's callback
    DWORD               m_dwInstance;       // client's instance data

    WAVEFORMATEX        m_WaveFormat;       // Format of wave data

    LPWAVEHDR           m_lpWaveHdrHead;
    LPWAVEHDR           m_lpWaveHdrCurrent;
    LPWAVEHDR           m_lpWaveHdrTail;
    PBYTE               m_lpCurrData;       // position in current buffer
    PBYTE               m_lpCurrDataEnd;    // end of current buffer

    DWORD               m_dwByteCount;      // byte count since last reset

    DeviceContext *     m_pDeviceContext;   // Device which this stream renders to

    // Loopcount shouldn't really be here, since it's really for wave output only, but it makes things easier
    DWORD               m_dwLoopCount;      // Number of times left through loop

    DWORD               m_dwGain;
    DWORD               m_SecondaryGainClass;
    DWORD               m_fxpGain;

    BOOL                m_bForceSpeaker;
};

class WaveStreamContext : public StreamContext
{
public:
    virtual HRESULT Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags);

    DWORD GetRate(DWORD *pdwMultiplier);
    virtual DWORD SetRate(DWORD dwMultiplier) = 0;
    PBYTE Render(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
    virtual PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)=0;

protected:
    PCM_TYPE            m_SampleType;       // Enum of sample type, e.g. M8, M16, S8, S16
    ULONG               m_SampleSize;       // # of bytes per sample in client buffer
    DWORD               m_DeltaT;           // Sample rate conversion factor
    DWORD               m_dwMultiplier;
    LONG                m_PrevSamp[OUTCHANNELS];
    LONG                m_CurrSamp[OUTCHANNELS];
    LONG                m_CurrT;
};

class InputStreamContext : public WaveStreamContext
{
public:
    HRESULT Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags);
    DWORD SetRate(DWORD dwMultiplier);
    DWORD Stop();   // On input, stop has special handling
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);

    virtual void DoCallbackReturnBuffer(LPWAVEHDR lpHdr)
    {
        DEBUGMSG(ZONE_WIDM && ZONE_VERBOSE,(TEXT("WIDM: DoCallbackReturnBuffer\r\n")));
        DoDriverCallback(WIM_DATA,(DWORD)lpHdr,0);
    }
    virtual void DoCallbackStreamOpened()
    {
        g_pHWContext->InputStreamOpened();
        DEBUGMSG(ZONE_WIDM && ZONE_VERBOSE,(TEXT("WIDM: DoCallbackStreamOpened\r\n")));
        DoDriverCallback(WIM_OPEN,0,0);
    }
    virtual void DoCallbackStreamClosed()
    {
        g_pHWContext->InputStreamClosed();
        DEBUGMSG(ZONE_WIDM && ZONE_VERBOSE,(TEXT("WIDM: DoCallbackStreamOpened\r\n")));
        DoDriverCallback(WIM_CLOSE,0,0);
    }
};

class OutputStreamContext : public WaveStreamContext
{
public:
    HRESULT Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags);
    DWORD Reset();

    DWORD SetRate(DWORD dwMultiplier);
};

class OutputStreamContextM8 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
};
class OutputStreamContextM16 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
};
class OutputStreamContextS8 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
};
class OutputStreamContextS16 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
};

// CAutoLock helper class
class CAutoLock
{
public:
    CAutoLock (CRITICAL_SECTION * cs)
    {
        pcs = cs;
        EnterCriticalSection(pcs);
    }
    ~CAutoLock ()
    {
        LeaveCriticalSection(pcs);
    }
private:
    CRITICAL_SECTION * pcs;
};

