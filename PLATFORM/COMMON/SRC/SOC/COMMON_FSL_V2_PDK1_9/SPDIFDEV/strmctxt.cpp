//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------
#include "wavemain.h"

HRESULT StreamContext::Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags)
{
    m_RefCount = 1;
    m_pDeviceContext = pDeviceContext;
    m_pfnCallback = (DRVCALLBACK *)lpWOD->dwCallback;
    m_dwInstance  = lpWOD->dwInstance;
    m_hWave       = lpWOD->hWave;
    m_dwFlags     = dwFlags;
    m_bRunning    = FALSE;

    // If it's a PCMWAVEFORMAT struct, it's smaller than a WAVEFORMATEX struct (it doesn't have the cbSize field),
    // so don't copy too much or we risk a fault if the structure is located on the end of a page.
    // All other non-PCM wave formats share the WAVEFORMATEX base structure
    // Note: I don't keep around anything after the cbSize of the WAVEFORMATEX struct so that I don't need to
    // worry about allocating additional space. If we need to keep this info around in the future, we can either
    // allocate it dynamically here, or keep the information in any derived format-specific classes.
    DWORD dwSize;
    WAVEFORMATEX *pwfx = lpWOD->lpFormat;
    if (pwfx->wFormatTag == WAVE_FORMAT_PCM)
    {
        dwSize = sizeof(PCMWAVEFORMAT);
        m_WaveFormat.cbSize = 0;
    }
    else //WAVE_FORMAT_WMASPDIF
    {
        dwSize = sizeof(WAVEFORMATEX);
    }

    memcpy(&m_WaveFormat,pwfx,dwSize);

    m_lpWaveHdrHead    = NULL;
    m_lpWaveHdrTail    = NULL;
    m_lpWaveHdrCurrent = NULL;
    m_lpCurrData       = NULL;
    m_lpCurrDataEnd    = NULL;
    m_dwByteCount      = 0;
    m_dwLoopCount = 0;

    m_SecondaryGainClass=0;
    SetGain(pDeviceContext->GetDefaultStreamGain()); // Set gain to default value

    // DEBUGMSG(1, (TEXT("Opening stream 0x%x\r\n"),this));

    // Add stream to list. This will start playback.
    pDeviceContext->NewStream(this);

    DoCallbackStreamOpened();

    return S_OK;
}

DWORD StreamContext::Close()
{
    if (StillPlaying())
    {
        return WAVERR_STILLPLAYING;
    }

    // DEBUGMSG(1, (TEXT("Closing stream 0x%x\r\n"),this));
    DoCallbackStreamClosed();

    return MMSYSERR_NOERROR;
}

// Assumes lock is taken
LONG StreamContext::AddRef()
{
    LONG RefCount = ++m_RefCount;
//    DEBUGMSG(1, (TEXT("AddRef stream 0x%x, RefCount=%d\r\n"),this,RefCount));
    return RefCount;
}

// Assumes lock is taken
LONG StreamContext::Release()
{
    LONG RefCount = --m_RefCount;

//    DEBUGMSG(1, (TEXT("Releasing stream 0x%x, RefCount=%d\r\n"),this,RefCount));
    if (RefCount==0)
    {
        // DEBUGMSG(1, (TEXT("Deleting stream 0x%x\r\n"),this));
        // Only remove stream from list when all refcounts are gone.
        m_pDeviceContext->DeleteStream(this);
        delete this;
    }
    return RefCount;
}

DWORD StreamContext::QueueBuffer(LPWAVEHDR lpWaveHdr)
{
    if (!(lpWaveHdr->dwFlags & WHDR_PREPARED))
    {
        return WAVERR_UNPREPARED;
    }

    lpWaveHdr->dwFlags |= WHDR_INQUEUE;
    lpWaveHdr->dwFlags &= ~WHDR_DONE;
    lpWaveHdr->lpNext=NULL;
    lpWaveHdr->dwBytesRecorded=0;

    if (!m_lpWaveHdrHead)
    {
        m_lpWaveHdrHead = lpWaveHdr;
    }
    else
    {
        m_lpWaveHdrTail->lpNext=lpWaveHdr;
    }

    m_lpWaveHdrTail=lpWaveHdr;

    // Note: Even if head & tail are valid, current may be NULL if we're in the middle of
    // a loop and ran out of data. So, we need to check specifically against current to
    // decide if we need to initialize it.
    if (!m_lpWaveHdrCurrent)
    {
        m_lpWaveHdrCurrent = lpWaveHdr;
        m_lpCurrData    = (PBYTE)lpWaveHdr->lpData;
        m_lpCurrDataEnd = (PBYTE)lpWaveHdr->lpData + lpWaveHdr->dwBufferLength;
        if (lpWaveHdr->dwFlags & WHDR_BEGINLOOP)    // if this is the start of a loop block
        {
            m_dwLoopCount = lpWaveHdr->dwLoops;     // save # of loops
        }
    }

    if (m_bRunning)
    {
        m_pDeviceContext->StreamReadyToRender(this);
    }

    return MMSYSERR_NOERROR;
}

// Note: I've found that when we return used buffers, the wave manager may
// call back into the wave driver in the same thread context to close the stream when
// we return the last buffer.
// If it wasn't the last buffer, the close call will return MMSYSERR_STILLPLAYING.
// However, if it was the last buffer, the close will proceed, and the
// stream may be deleted out from under us. Note that a Lock won't help us here,
// since we're in the same thread which already owns the lock.
// The solution to this is the AddRef/Release use on the stream context, which keeps it
// around if we're acessing it, even if it's closed.

// Assumes lock is taken
PBYTE StreamContext::GetNextBuffer()
{
    LPWAVEHDR lpOldHdr;
    LPWAVEHDR lpNewHdr;

    // Get a pointer to the current buffer which is now done being processed
    lpOldHdr=m_lpWaveHdrCurrent;

    if (!lpOldHdr)
    {
        return NULL;
    }

    // Are we in a loop
    // Note: a loopcount of 1 means we're not really in a loop
    if (m_dwLoopCount>1)
    {
        // We're in a loop!
        if (lpOldHdr->dwFlags & WHDR_ENDLOOP)
        {
           // In loop, last buffer
            // If dwLoopCount was set to INFINITE, loop forever
            // (Note: this is not explicitly in the wave driver API spec)
            if (m_dwLoopCount!=INFINITE)
            {
           m_dwLoopCount--;                    // decrement loop count
            }
           lpNewHdr=m_lpWaveHdrHead;           // go back to start of loop
        }
        else
        {
           // In loop, intermediate buffer
           lpNewHdr=lpOldHdr->lpNext;          // just go to next buffer in loop block
        }

        lpOldHdr=NULL;
    }
    else
    {
        // Not in a loop; return old buffer and get new buffer
        lpNewHdr=lpOldHdr->lpNext;

        m_lpWaveHdrHead = lpNewHdr;           // reset list head
        if (!lpNewHdr)
        {
            m_lpWaveHdrTail=NULL;             // no new buffer, reset tail to NULL
        }
        else if (lpNewHdr->dwFlags & WHDR_BEGINLOOP)    // if new buffer is start of a loop block
        {
            m_dwLoopCount=lpNewHdr->dwLoops;  // save # of loops
        }
    }

    m_lpWaveHdrCurrent=lpNewHdr;              // save current buffer pointer

    if (lpNewHdr)
    {
        m_lpCurrData    = (PBYTE)lpNewHdr->lpData;  // reinitialize data pointer
        m_lpCurrDataEnd = m_lpCurrData + lpNewHdr->dwBufferLength;
    }
    else
    {
        m_lpCurrData  = NULL;
        m_lpCurrDataEnd = NULL;
    }

    // Return the old buffer
    // This may cause the stream to be destroyed, so make sure that any calls to this function
    // are within an AddRef/Release block
    if (lpOldHdr)
    {
        ReturnBuffer(lpOldHdr);
    }

    return m_lpCurrData;
}

DWORD StreamContext::BreakLoop()
{
    AddRef();

    if (m_dwLoopCount>0)
    {
        m_dwLoopCount = 0;

        LPWAVEHDR lpHdr;
        while (m_lpWaveHdrHead!=m_lpWaveHdrCurrent)
        {
            lpHdr = m_lpWaveHdrHead;
            if (lpHdr != NULL)  
                m_lpWaveHdrHead = lpHdr->lpNext;
            if (m_lpWaveHdrHead==NULL)
            {
                m_lpWaveHdrTail=NULL;
            }
            ReturnBuffer(lpHdr);
        }
    }

    Release();

    return MMSYSERR_NOERROR;
}

// Gain table
// Calculated as: 0x10000 * exp(dBGain/20), for dBGain from 0 to -63
// Sample code to generate using VC++
//
//    #include "stdafx.h"
//    #include "math.h"
//
//    const int NumEntries = 64;
//    const double fdBMin = -100;
//
//    int main(int argc, char* argv[])
//    {
//      const double fNumEntries = ((double)(NumEntries-1));
//      for (int i=0;i<NumEntries;i++)
//      {
//          double fVol = fdBMin * ((double)(i)) / fNumEntries;
//          double fMulVal = exp(fVol/20);
//          unsigned long MulVal = (unsigned long)(fMulVal * (double)0x10000);
//          printf("0x%04x, // %d: %f dB\n",MulVal,i,fVol);
//      }
//      return 0;
//    }
//

const DWORD GainMap[] =
{
0x10000, // 0: 0.000000 dB
0xec77, // 1: -1.587302 dB
0xda6d, // 2: -3.174603 dB
0xc9c2, // 3: -4.761905 dB
0xba5d, // 4: -6.349206 dB
0xac25, // 5: -7.936508 dB
0x9f03, // 6: -9.523810 dB
0x92e1, // 7: -11.111111 dB
0x87ac, // 8: -12.698413 dB
0x7d52, // 9: -14.285714 dB
0x73c2, // 10: -15.873016 dB
0x6aed, // 11: -17.460317 dB
0x62c5, // 12: -19.047619 dB
0x5b3b, // 13: -20.634921 dB
0x5445, // 14: -22.222222 dB
0x4dd7, // 15: -23.809524 dB
0x47e7, // 16: -25.396825 dB
0x426b, // 17: -26.984127 dB
0x3d59, // 18: -28.571429 dB
0x38ab, // 19: -30.158730 dB
0x3458, // 20: -31.746032 dB
0x305a, // 21: -33.333333 dB
0x2ca9, // 22: -34.920635 dB
0x2941, // 23: -36.507937 dB
0x261b, // 24: -38.095238 dB
0x2333, // 25: -39.682540 dB
0x2083, // 26: -41.269841 dB
0x1e08, // 27: -42.857143 dB
0x1bbe, // 28: -44.444444 dB
0x19a0, // 29: -46.031746 dB
0x17ab, // 30: -47.619048 dB
0x15dd, // 31: -49.206349 dB
0x1432, // 32: -50.793651 dB
0x12a7, // 33: -52.380952 dB
0x113b, // 34: -53.968254 dB
0x0fea, // 35: -55.555556 dB
0x0eb3, // 36: -57.142857 dB
0x0d94, // 37: -58.730159 dB
0x0c8b, // 38: -60.317460 dB
0x0b96, // 39: -61.904762 dB
0x0ab4, // 40: -63.492063 dB
0x09e3, // 41: -65.079365 dB
0x0921, // 42: -66.666667 dB
0x086f, // 43: -68.253968 dB
0x07ca, // 44: -69.841270 dB
0x0732, // 45: -71.428571 dB
0x06a6, // 46: -73.015873 dB
0x0624, // 47: -74.603175 dB
0x05ac, // 48: -76.190476 dB
0x053d, // 49: -77.777778 dB
0x04d7, // 50: -79.365079 dB
0x0478, // 51: -80.952381 dB
0x0421, // 52: -82.539683 dB
0x03d0, // 53: -84.126984 dB
0x0386, // 54: -85.714286 dB
0x0341, // 55: -87.301587 dB
0x0301, // 56: -88.888889 dB
0x02c6, // 57: -90.476190 dB
0x0290, // 58: -92.063492 dB
0x025e, // 59: -93.650794 dB
0x0230, // 60: -95.238095 dB
0x0205, // 61: -96.825397 dB
0x01de, // 62: -98.412698 dB
0x01b9, // 63: -100.000000 dB
};

DWORD StreamContext::MapGain(DWORD Gain)
{
    DWORD TotalGain = Gain & 0xFFFF;
    DWORD SecondaryGain = m_pDeviceContext->GetSecondaryGainLimit(m_SecondaryGainClass) & 0xFFFF;

    if (m_SecondaryGainClass < SECONDARYDEVICEGAINCLASSMAX)
    {
        // Apply device gain
        DWORD DeviceGain = m_pDeviceContext->GetGain() & 0xFFFF;
        TotalGain *= DeviceGain;
        TotalGain += 0xFFFF;  // Round up
        TotalGain >>= 16;     // Shift to lowest 16 bits
    }

    // Apply secondary gain
    TotalGain *= SecondaryGain;
    TotalGain += 0xFFFF;  // Round up
    TotalGain >>= 16;     // Shift to lowest 16 bits

    // Special case 0 as totally muted
    if (TotalGain==0)
    {
        return 0;
    }

    // Convert to index into table
    DWORD Index = 63 - (TotalGain>>10);
    return GainMap[Index];
}

DWORD StreamContext::GetPos(PMMTIME pmmt)
{
    switch (pmmt->wType)
    {

    case TIME_SAMPLES:
        pmmt->u.sample = (m_dwByteCount * 8) /
                         (m_WaveFormat.nChannels * m_WaveFormat.wBitsPerSample);
        break;

    case TIME_MS:
        if (m_WaveFormat.nAvgBytesPerSec != 0)
        {
            pmmt->u.ms = (m_dwByteCount * 1000) / m_WaveFormat.nAvgBytesPerSec;
            break;
        }
       else// If we don't know avg bytes per sec, return TIME_BYTES
        {
            pmmt->wType = TIME_BYTES;
            pmmt->u.cb = m_dwByteCount;
        }
       break;
        

    default:
        // Anything else, return TIME_BYTES instead.
        pmmt->wType = TIME_BYTES;
        pmmt->u.cb = m_dwByteCount;
      break;

        // Fall through to TIME_BYTES
    case TIME_BYTES:
        pmmt->u.cb = m_dwByteCount;
      break;
    }

    return MMSYSERR_NOERROR;
}

HRESULT WaveStreamContext::Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags)
{
    HRESULT Result;
    Result = StreamContext::Open(pDeviceContext,lpWOD,dwFlags);
    if (FAILED(Result))
    {
        return Result;
    }
#if 0
    if (m_WaveFormat.Format.wBitsPerSample == 8)
    {
        if (m_WaveFormat.Format.nChannels == 1)
        {
            m_SampleType = PCM_TYPE_M8;
            m_SampleSize = 1;
        }
        else
        {
            m_SampleType = PCM_TYPE_S8;
            m_SampleSize = 2;
        }
    }
    else
    {
        if (m_WaveFormat.Format.nChannels == 1)
        {
            m_SampleType = PCM_TYPE_M16;
            m_SampleSize = 2;
        }
        else
        {
            m_SampleType = PCM_TYPE_S16;
            m_SampleSize = 4;
        }
    }
#endif
    SetRate(0x10000);
#if 0
    int i;
    for (i=0;i<OUTCHANNELS;i++)
    {
        m_PrevSamp[i] = 0;
        m_CurrSamp[i] = 0;
    }
    m_CurrT    = 0x200;   // Initializing to this ensures we get the 1st sample.
#endif
    return S_OK;
}

DWORD WaveStreamContext::GetRate(DWORD *pdwMultiplier)
{
    *pdwMultiplier = m_dwMultiplier;
    return MMSYSERR_NOERROR;
}

DWORD StreamContext::Run()
{
    m_bRunning=TRUE;
    if (m_lpCurrData)
    {
        m_pDeviceContext->StreamReadyToRender(this);
    }

    return MMSYSERR_NOERROR;
}


DWORD StreamContext::Stop()
{
    m_bRunning=FALSE;
    return MMSYSERR_NOERROR;
}

DWORD StreamContext::Reset()
{
    AddRef();

    // Stop stream for now.
    Stop();

    m_lpWaveHdrCurrent  = NULL;
    m_lpCurrData       = NULL;
    m_lpCurrDataEnd    = NULL;
    m_dwByteCount      = 0;
    m_dwLoopCount      = 0;

    LPWAVEHDR lpHdr;
    while (m_lpWaveHdrHead)
    {
        lpHdr = m_lpWaveHdrHead;
        m_lpWaveHdrHead = lpHdr->lpNext;
        if (m_lpWaveHdrHead==NULL)
        {
            m_lpWaveHdrTail=NULL;
        }
        ReturnBuffer(lpHdr);
    }

    Release();

    return MMSYSERR_NOERROR;
}

