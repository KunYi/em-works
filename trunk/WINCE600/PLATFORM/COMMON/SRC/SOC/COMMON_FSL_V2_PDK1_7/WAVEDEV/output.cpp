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
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include "wavemain.h"

HRESULT OutputStreamContext::Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags)
{
    HRESULT Result;

    Result = WaveStreamContext::Open(pDeviceContext, lpWOD, dwFlags);

    if (Result==MMSYSERR_NOERROR)
    {
        // Note: Output streams should be initialized in the run state.
        Run();
    }

    return Result;
}

DWORD OutputStreamContext::Reset()
{
    HRESULT Result;

    Result = WaveStreamContext::Reset();

    if (Result==MMSYSERR_NOERROR)
    {
        // Note: Output streams should be reset to the run state.
        Run();
    }

    return Result;
};

// Init m_DeltaT with (SampleRate/HWSampleRate) calculated in the appropriate fixed point form.
// Note that we need to hold the result in a 64-bit value until we're done shifting,
// since the result of the multiply will overflow 32 bits for sample rates greater than
// or equal to the hardware's sample rate.
DWORD OutputStreamContext::SetRate(DWORD dwMultiplier)
{
    UINT64 Delta;

    m_dwMultiplier = dwMultiplier;

    // The following code makes a bunch of assumptions on why we don't overflow a 64-bit value:
    // nSamplesPerSec is < 20 bits (e.g. no input data rates greater than ~1 MHz!)
    // m_dwMultiplier is < 24 bits (e.g. no speedup beyond 256x)
    // INVSAMPLERATE is < 20 bits (e.g. no playback rates less than 4kHz)

    Delta = (m_WaveFormat.nSamplesPerSec * m_dwMultiplier);
    Delta = (Delta * INVSAMPLERATE);    // < Note: These two lines are out of order to preserve accuracy
    Delta >>= 16;                       // <
    Delta >>= DELTAINT;  // Convert to x.x format (e.g. 17.15)

    m_DeltaT = (DWORD)Delta;
    return MMSYSERR_NOERROR;
}

// Originally, this code used to be in each renderer, and each one would call GetNextBuffer as needed.
// Pulling this code out of each low level renderer allows the inner loop to be in a leaf routine (ie no
// subroutine calls out of that routine), which helps the compiler optimize the inner loop.
PBYTE WaveStreamContext::Render(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    if (!m_bRunning || !m_lpCurrData)
    {
        return pBuffer;
    }

    while (pBuffer < pBufferEnd)
    {
        while (m_lpCurrData>=m_lpCurrDataEnd)
        {
            if (!GetNextBuffer())
            {
                return pBuffer;
            }
        }

        PREFAST_SUPPRESS(6320, "Generic exception handler");
        _try
        {
            pBuffer = Render2(pBuffer,pBufferEnd,pBufferLast);
        }
        _except (EXCEPTION_EXECUTE_HANDLER)
        {
            ERRORMSG(ZONE_ERROR, (TEXT("EXCEPTION IN IST for stream 0x%x, ")
                                  TEXT("buffer 0x%x!!!!\r\n"),
                                  this, m_lpCurrData));

            // Pretend we finished reading the application buffer
            m_lpCurrData=m_lpCurrDataEnd;
        }
    }

    return pBuffer;
}

PBYTE OutputStreamContextM8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain = m_fxpGain;

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 = (LONG)pSampleSrc->m8.sample;
            CurrSamp0 = (CurrSamp0 - 128) << 8;
            pCurrData+=1;
        }

        LONG OutSamp0;
        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;

#if (OUTCHANNELS==2)
        LONG OutSamp1;
        OutSamp1=OutSamp0;
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;
        pBuffer += 2*sizeof(HWSAMPLE);
#else
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        pBuffer += sizeof(HWSAMPLE);
#endif
    }

    Exit:

    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}

PBYTE OutputStreamContextM16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain = m_fxpGain;
    LONG OutSamp0;

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 = (LONG)pSampleSrc->m16.sample;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;
        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

#if (OUTCHANNELS==2)
        LONG OutSamp1;
        OutSamp1=OutSamp0;
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;
        pBuffer += 2*sizeof(HWSAMPLE);
#else
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        pBuffer += sizeof(HWSAMPLE);
#endif
    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}

#if (OUTCHANNELS==2)
PBYTE OutputStreamContextS8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG CurrSamp1 = m_CurrSamp[1];
    LONG PrevSamp0 = m_PrevSamp[0];
    LONG PrevSamp1 = m_PrevSamp[1];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain = m_fxpGain;
    LONG OutSamp0;
    LONG OutSamp1;

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;
            PrevSamp1 = CurrSamp1;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s8.sample_left;
            CurrSamp0 = (CurrSamp0 - 128) << 8;
            CurrSamp1 = (LONG)pSampleSrc->s8.sample_right;
            CurrSamp1 = (CurrSamp1 - 128) << 8;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;

        OutSamp1 = PrevSamp1 + (((CurrSamp1 - PrevSamp1) * CurrT) >> DELTAFRAC);
        OutSamp1 = (OutSamp1 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;
        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;

        pBuffer += 2*sizeof(HWSAMPLE);

    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_PrevSamp[1] = PrevSamp1;
    m_CurrSamp[0] = CurrSamp0;
    m_CurrSamp[1] = CurrSamp1;
    return pBuffer;
}

PBYTE OutputStreamContextS16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    if (m_DeltaT == (1<<DELTAFRAC) && m_fxpGain == (1<<VOLSHIFT))
    {
        LONG lCopyTo = m_lpCurrDataEnd - m_lpCurrData;
        LONG lCopyFrom = pBufferEnd - pBuffer;
        LONG lCopy = min(lCopyTo, lCopyFrom);
        memcpy(pBuffer, m_lpCurrData, lCopy);
        pBuffer += lCopy;
        m_dwByteCount += lCopy;
        m_lpCurrData += lCopy;
    }
    else
    {
        LONG CurrT = m_CurrT;
        LONG DeltaT = m_DeltaT;
        LONG CurrSamp0 = m_CurrSamp[0];
        LONG CurrSamp1 = m_CurrSamp[1];
        LONG PrevSamp0 = m_PrevSamp[0];
        LONG PrevSamp1 = m_PrevSamp[1];
        PBYTE pCurrData = m_lpCurrData;
        PBYTE pCurrDataEnd = m_lpCurrDataEnd;
        LONG fxpGain = m_fxpGain;
        LONG OutSamp0;
        LONG OutSamp1;

        while (pBuffer < pBufferEnd)
        {
            while (CurrT >= DELTA_OVERFLOW)
            {
                if (pCurrData>=pCurrDataEnd)
                {
                    goto Exit;
                }

                CurrT -= DELTA_OVERFLOW;

                PrevSamp0 = CurrSamp0;
                PrevSamp1 = CurrSamp1;

                PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
                CurrSamp0 = (LONG)pSampleSrc->s16.sample_left;
                CurrSamp1 = (LONG)pSampleSrc->s16.sample_right;
                pCurrData+=4;
            }

            OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
            OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
            OutSamp1 = PrevSamp1 + (((CurrSamp1 - PrevSamp1) * CurrT) >> DELTAFRAC);
            OutSamp1 = (OutSamp1 * fxpGain) >> VOLSHIFT;
            CurrT += DeltaT;
            // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

            if (pBuffer < pBufferLast)
            {
                OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
                OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
    #if USE_MIX_SATURATE
                // Handle saturation
                if (OutSamp0>AUDIO_SAMPLE_MAX)
                {
                    OutSamp0=AUDIO_SAMPLE_MAX;
                }
                else if (OutSamp0<AUDIO_SAMPLE_MIN)
                {
                    OutSamp0=AUDIO_SAMPLE_MIN;
                }
                if (OutSamp1>AUDIO_SAMPLE_MAX)
                {
                    OutSamp1=AUDIO_SAMPLE_MAX;
                }
                else if (OutSamp1<AUDIO_SAMPLE_MIN)
                {
                    OutSamp1=AUDIO_SAMPLE_MIN;
                }
    #endif
            }
            ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
            ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;

            pBuffer += 2*sizeof(HWSAMPLE);
        }

        Exit:
        m_dwByteCount += (pCurrData - m_lpCurrData);
        m_lpCurrData = pCurrData;
        m_CurrT = CurrT;
        m_PrevSamp[0] = PrevSamp0;
        m_PrevSamp[1] = PrevSamp1;
        m_CurrSamp[0] = CurrSamp0;
        m_CurrSamp[1] = CurrSamp1;
    }
    return pBuffer;
}

#else

PBYTE OutputStreamContextS8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain = m_fxpGain;
    LONG OutSamp0;

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s8.sample_left;
            CurrSamp0 += (LONG)pSampleSrc->s8.sample_right;
            CurrSamp0 = (CurrSamp0 - 256) << 7;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;
        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += *(HWSAMPLE *)pBuffer;
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        *(HWSAMPLE *)pBuffer = (HWSAMPLE)OutSamp0;

        pBuffer += sizeof(HWSAMPLE);
    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}

PBYTE OutputStreamContextS16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain = m_fxpGain;
    LONG OutSamp0;

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= DELTA_OVERFLOW)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= DELTA_OVERFLOW;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s16.sample_left;
            CurrSamp0 += (LONG)pSampleSrc->s16.sample_right;
            CurrSamp0 = CurrSamp0>>1;
            pCurrData+=4;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> DELTAFRAC);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;
        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += *(HWSAMPLE *)pBuffer;
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        *(HWSAMPLE *)pBuffer = (HWSAMPLE)OutSamp0;

        pBuffer += sizeof(HWSAMPLE);

    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}
#endif
