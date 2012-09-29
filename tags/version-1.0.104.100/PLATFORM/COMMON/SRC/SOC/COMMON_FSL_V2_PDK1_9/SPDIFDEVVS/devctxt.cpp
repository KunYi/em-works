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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

#include "wavemain.h"

BOOL DeviceContext::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{

#if 0

//Set to PCM to test SPDIF PCM mode
   if (lpFormat->wFormatTag != WAVE_FORMAT_PCM)
       return FALSE;

   if (  (lpFormat->nChannels!=1) && (lpFormat->nChannels!=2) )
       return FALSE;

   if (  (lpFormat->wBitsPerSample!=8) && (lpFormat->wBitsPerSample!=16) )
       return FALSE;

   if (lpFormat->nSamplesPerSec < 100 || lpFormat->nSamplesPerSec > 96000)
       return FALSE;

#else
    if ((lpFormat->wFormatTag != WAVE_FORMAT_WMASPDIF) &&
         (lpFormat->wFormatTag != WAVE_FORMAT_PCM))
        return FALSE;
        
    if (lpFormat->nChannels != OUTCHANNELS)
        return FALSE;

    if ((lpFormat->wBitsPerSample != 16) &&
        (lpFormat->wBitsPerSample != 24) &&
        (lpFormat->wBitsPerSample != 32))
        return FALSE;

#endif

    return TRUE;
}

BOOL OutputDeviceContext::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    if ((lpFormat->nSamplesPerSec != 32000) && 
       (lpFormat->nSamplesPerSec != 48000) && 
       (lpFormat->nSamplesPerSec != 44100))
       return FALSE;
       
    return DeviceContext::IsSupportedFormat(lpFormat);
}

BOOL InputDeviceContext::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    // support for 8K~96K
    if (lpFormat->nSamplesPerSec < 8000 || lpFormat->nSamplesPerSec > 96000)
        return FALSE;
        
    return DeviceContext::IsSupportedFormat(lpFormat);
}


// Assumes lock is taken
void DeviceContext::NewStream(StreamContext *pStreamContext)
{
    InsertTailList(&m_StreamList,&pStreamContext->m_Link);
}

// Assumes lock is taken
void DeviceContext::DeleteStream(StreamContext *pStreamContext)
{
    RemoveEntryList(&pStreamContext->m_Link);
}

// Returns # of samples of output buffer filled
// Assumes that g_pHWContext->Lock already held.
PBYTE DeviceContext::TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, DWORD *pNumStreams)
{
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;
    PBYTE pBufferLastThis;
    PBYTE pBufferLast=pBuffer;
    DWORD NumStreams=0;

    pListEntry = m_StreamList.Flink;
    while (pListEntry != &m_StreamList)
    {
        // Get a pointer to the stream context
        pStreamContext = CONTAINING_RECORD(pListEntry,StreamContext,m_Link);

        // Note: The stream context may be closed and removed from the list inside
        // of Render, and the context may be freed as soon as we call Release.
        // Therefore we need to grab the next Flink first in case the
        // entry disappears out from under us.
        pListEntry = pListEntry->Flink;

        // Render buffers
        pStreamContext->AddRef();
        pBufferLastThis = pStreamContext->Render(pBuffer, pBufferEnd, pBufferLast);
        pStreamContext->Release();
        if (pBufferLastThis>pBuffer)
        {
            NumStreams++;
        }
        if (pBufferLast < pBufferLastThis)
        {
            pBufferLast = pBufferLastThis;
        }
    }

    if (pNumStreams)
    {
        *pNumStreams=NumStreams;
    }
    return pBufferLast;
}

void DeviceContext::RecalcAllGains()
{
    PLIST_ENTRY pListEntry;
    StreamContext *pStreamContext;

    for (pListEntry = m_StreamList.Flink;
        pListEntry != &m_StreamList;
        pListEntry = pListEntry->Flink)
    {
        pStreamContext = CONTAINING_RECORD(pListEntry,StreamContext,m_Link);
        pStreamContext->GainChange();
    }
    return;
}


void OutputDeviceContext::StreamReadyToRender(StreamContext *pStreamContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pStreamContext);
    g_pHWContext->GetTxHwContext()->SetWaveFormat(pStreamContext->GetWaveFormat());
    g_pHWContext->StartOutputDMA();
    return;
}

void InputDeviceContext::StreamReadyToRender(StreamContext *pStreamContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pStreamContext);
    g_pHWContext->GetRxHwContext()->SetWaveFormat(pStreamContext->GetWaveFormat());
    g_pHWContext->StartInputDMA();
    return;
}




DWORD OutputDeviceContext::GetDevCaps(LPVOID pCaps, DWORD dwSize)
{
    static const WAVEOUTCAPS wc =
    {
        MM_MICROSOFT,           // Manufacturer ID.
        25,                     // Product ID.
        0x0001,                 // Driver version.
        TEXT("SPDIF Output"),   // Product name.
                                // Supported formats.
        WAVE_FORMAT_WMASPDIF,
        OUTCHANNELS,            // Device supports (2 channel) output.
        0,                      // Reserved.
        0  // Optional functionality. The driver does not support capability like volume, playrate
    };

    if(dwSize > sizeof(wc))
    {
        dwSize = sizeof(wc);
        memcpy( pCaps, &wc, dwSize);
    }
    else
        memcpy( pCaps, &wc, sizeof(wc));
    return MMSYSERR_NOERROR;
}

DWORD InputDeviceContext::GetDevCaps(LPVOID pCaps, DWORD dwSize)
{
    static const WAVEINCAPS wc =
    {
        MM_MICROSOFT,        // Manufacturer ID.
        23,                  // Product ID.
        0x0001,              // Driver version.
        TEXT("SPDIF Input"), // Product name.
                             // Supported formats.
        WAVE_FORMAT_WMASPDIF,
        INCHANNELS,          // See hwctxt.h for the number of input channels.
        0                    // Reserved.
    };

    if(dwSize > sizeof(wc))
    {
        dwSize = sizeof(wc);
        memcpy( pCaps, &wc, dwSize);
    }
    else
        memcpy( pCaps, &wc, sizeof(wc));

    return MMSYSERR_NOERROR;
}


DWORD OutputDeviceContext::GetExtDevCaps(LPVOID pCaps, DWORD dwSize)
{
    static const WAVEOUTEXTCAPS wec =
    {
        0x00000001, // Max number of hw-mixed streams.
        0x00000001, // Available HW streams.
        0,          // Preferred sample rate for software mixer (0 indicates no
                    // preference).
        0,          // Preferred buffer size for software mixer (0 indicates no
                    // preference).
        0,          // Preferred number of buffers for software mixer (0
                    // indicates no preference).
        32000,       // Minimum sample rate for a hw-mixed stream (Hz).
        48000       // Maximum sample rate for a hw-mixed stream (Hz).
    };

    if(dwSize > sizeof(wec))
    {
        dwSize = sizeof(wec);
        memcpy( pCaps, &wec, dwSize);
    }
    else
        memcpy( pCaps, &wec, sizeof(wec));

    return MMSYSERR_NOERROR;
}

DWORD InputDeviceContext::GetExtDevCaps(LPVOID pCaps, DWORD dwSize)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCaps);
    UNREFERENCED_PARAMETER(dwSize);
    return MMSYSERR_NOTSUPPORTED;
}


StreamContext *InputDeviceContext::CreateStream(LPWAVEOPENDESC lpWOD)
{
    LPWAVEFORMATEX lpFormat=lpWOD->lpFormat;

    if ((lpFormat->wFormatTag == WAVE_FORMAT_PCM) ||
       (lpFormat->wFormatTag == WAVE_FORMAT_WMASPDIF))
    {
        if (lpFormat->nChannels == 2)
        {
            if (lpFormat->wBitsPerSample == 16)
            {
                return new InputStreamContextSPDIF16;
            }
            else if (lpFormat->wBitsPerSample == 24)
            {
                return new InputStreamContextSPDIF24;
            }
            else if (lpFormat->wBitsPerSample == 32)
            {
                return new InputStreamContextSPDIF32;
            }
        }
    }
    
    return NULL;
}


StreamContext *OutputDeviceContext::CreateStream(LPWAVEOPENDESC lpWOD)
{
    LPWAVEFORMATEX lpFormat=lpWOD->lpFormat;

    if ((lpFormat->wFormatTag == WAVE_FORMAT_PCM) ||
       (lpFormat->wFormatTag == WAVE_FORMAT_WMASPDIF))
    {
        if (lpFormat->nChannels == 2)
        {
            if (lpFormat->wBitsPerSample == 16)
            {
                return new OutputStreamContextSPDIF16;
            }
            else if (lpFormat->wBitsPerSample == 24)
            {
                return new OutputStreamContextSPDIF24;
            }
            else if (lpFormat->wBitsPerSample == 32)
            {
                return new OutputStreamContextSPDIF32;
            }
        }
    }
    
    return NULL;
}

DWORD DeviceContext::OpenStream(LPWAVEOPENDESC lpWOD, DWORD dwFlags, StreamContext **ppStreamContext)
{
    HRESULT Result;
    StreamContext *pStreamContext;

    if (lpWOD->lpFormat==NULL)
    {
        return WAVERR_BADFORMAT;
    }

    if (!IsSupportedFormat(lpWOD->lpFormat))
    {
        return WAVERR_BADFORMAT;
    }
 
    // Query format support only - don't actually open device?
    if (dwFlags & WAVE_FORMAT_QUERY)
    {
        return MMSYSERR_NOERROR;
    }

    pStreamContext = CreateStream(lpWOD);
    if (!pStreamContext)
    {
        return MMSYSERR_NOMEM;
    }

    Result = pStreamContext->Open(this,lpWOD,dwFlags);
    if (FAILED(Result))
    {
        delete pStreamContext;
        return MMSYSERR_ERROR;
    }

    *ppStreamContext=pStreamContext;
    return MMSYSERR_NOERROR;
}
