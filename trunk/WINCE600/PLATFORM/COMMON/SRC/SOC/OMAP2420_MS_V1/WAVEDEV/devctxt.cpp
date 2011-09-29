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

#include "wavemain.h"


//------------------------------------------------------------------------------
//
//  Function: DeviceContext::IsSupportedFormat
//  
//  check if wave format is supported 
//

BOOL 
DeviceContext::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    if (lpFormat->wFormatTag != WAVE_FORMAT_PCM)
        return FALSE;

    if (  (lpFormat->nChannels!=1) && (lpFormat->nChannels!=2) )
        return FALSE;

    if (  (lpFormat->wBitsPerSample!=8) && (lpFormat->wBitsPerSample!=16) )
        return FALSE;

    if (lpFormat->nSamplesPerSec < 100 || 
        lpFormat->nSamplesPerSec > 48000)
        return FALSE;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: OutputDeviceContext::IsSupportedFormat
//  
//  We also support MIDI on output
//

BOOL 
OutputDeviceContext::IsSupportedFormat(LPWAVEFORMATEX lpFormat)
{
    if (lpFormat->wFormatTag == WAVE_FORMAT_MIDI)
    {
        return TRUE;
    }

    return DeviceContext::IsSupportedFormat(lpFormat);
}

//------------------------------------------------------------------------------
//
//  Function: DeviceContext::NewStream
//  
//  add new stream to list of open streams
//  Assumes lock is taken
//

void 
DeviceContext::NewStream(StreamContext *pStreamContext)
{
    InsertTailList(&m_StreamList,&pStreamContext->m_Link);
}

//------------------------------------------------------------------------------
//
//  Function: DeviceContext::DeleteStream
//  
//  free stream context
//  Assumes lock is taken
//

void 
DeviceContext::DeleteStream(StreamContext *pStreamContext)
{
    RemoveEntryList(&pStreamContext->m_Link);
}

//------------------------------------------------------------------------------
//
//  Function: DeviceContext::TransferBuffer
//  
//  Returns # of samples of output buffer filled
//  Assumes that g_pHWContext->Lock already held.
//

PBYTE 
DeviceContext::TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, DWORD *pNumStreams)
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

//------------------------------------------------------------------------------
//
//  Function: DeviceContext::RecalcAllGains
//  
//
//

void 
DeviceContext::RecalcAllGains()
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

//------------------------------------------------------------------------------
//
//  Function: OutputDeviceContext::StreamReadyToRender
//  
//  start playback of wave output stream
//

void 
OutputDeviceContext::StreamReadyToRender(StreamContext *pStreamContext)
{
    g_pHWContext->StartOutputDMA();
    return;
}

//------------------------------------------------------------------------------
//
//  Function: InputDeviceContext::StreamReadyToRender
//  
//  start capture of wave input stream
//

void 
InputDeviceContext::StreamReadyToRender(StreamContext *pStreamContext)
{
    g_pHWContext->StartInputDMA();
    return;
}

//------------------------------------------------------------------------------
//
//  Function: OutputDeviceContext::GetDevCaps
//  
//  return wave output device caps
//
//

DWORD 
OutputDeviceContext::GetDevCaps(LPVOID pCaps, DWORD dwSize)
{
    static const WAVEOUTCAPS wc =
    {
        MM_MICROSOFT,
        24,
        0x0001,
        TEXT("Audio Output"),
        WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_4M08 |
        WAVE_FORMAT_1S08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4S08 |
        WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M16 |
        WAVE_FORMAT_1S16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S16,
        1,
        0,
        WAVECAPS_VOLUME | WAVECAPS_PLAYBACKRATE
    };

    if (dwSize > sizeof(wc))
        dwSize = sizeof(wc);

    memcpy( pCaps, &wc, dwSize);

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
//  Function: InputDeviceContext::GetDevCaps
//  
//  return wave input device caps
//

DWORD
InputDeviceContext::GetDevCaps(LPVOID pCaps, DWORD dwSize)
{
    static const WAVEINCAPS wc =
    {
        MM_MICROSOFT,
        23,
        0x0001,
        TEXT("Audio Input"),
        WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_4M08 |
        WAVE_FORMAT_1S08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4S08 |
        WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M16 |
        WAVE_FORMAT_1S16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S16,
        1,
        0
    };

    if (dwSize > sizeof(wc))
        dwSize = sizeof(wc);

    memcpy( pCaps, &wc, dwSize);

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
//  Function: GetExtDevCaps
//  
//  return extended device caps
//

DWORD 
OutputDeviceContext::GetExtDevCaps(LPVOID pCaps, DWORD dwSize)
{
    static const WAVEOUTEXTCAPS wec =
    {
        0x0000FFFF,                         // max number of hw-mixed streams
        0x0000FFFF,                         // available HW streams
        0,                                  // preferred sample rate for software mixer (0 indicates no preference)
// dev note: this value prevents the Windows CE software mixer from
// allocating mixer memory. This driver does all mixing internally (was 0)
        6,                                  // preferred buffer size for software mixer (0 indicates no preference)
        0,                                  // preferred number of buffers for software mixer (0 indicates no preference)
        8000,                               // minimum sample rate for a hw-mixed stream
        48000                               // maximum sample rate for a hw-mixed stream
    };

    if (dwSize > sizeof(wec))
    {
        dwSize = sizeof(wec);
    }

    memcpy( pCaps, &wec, dwSize);

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
//  Function: 
//  
//
//

DWORD
InputDeviceContext::GetExtDevCaps(LPVOID pCaps, DWORD dwSize)
{
    return MMSYSERR_NOTSUPPORTED;
}

//------------------------------------------------------------------------------
//
//  Function: InputDeviceContext::CreateStream
//  
//  Create input stream context. 
//

StreamContext *
InputDeviceContext::CreateStream(LPWAVEOPENDESC lpWOD)
{
    return new InputStreamContext;
}

//------------------------------------------------------------------------------
//
//  Function: OutputDeviceContext::CreateStream
//  
//  Create StreamContext for wave output based on format parameters.
//

StreamContext *
OutputDeviceContext::CreateStream(LPWAVEOPENDESC lpWOD)
{
    LPWAVEFORMATEX lpFormat=lpWOD->lpFormat;

    if (lpWOD->lpFormat->wFormatTag == WAVE_FORMAT_MIDI)
    {
        return new CMidiStream;
    }

    if (lpFormat->nChannels==1)
    {
        if (lpFormat->wBitsPerSample==8)
        {
            return new OutputStreamContextM8;
        }
        else
        {
            return new OutputStreamContextM16;
        }
    }
    else
    {
        if (lpFormat->wBitsPerSample==8)
        {
            return new OutputStreamContextS8;
        }
        else
        {
            return new OutputStreamContextS16;
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function: OpenStream
//  
//  Check format parameters and create stream context if possible.
//

DWORD 
DeviceContext::OpenStream(LPWAVEOPENDESC lpWOD, DWORD dwFlags, 
                          StreamContext **ppStreamContext)
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

