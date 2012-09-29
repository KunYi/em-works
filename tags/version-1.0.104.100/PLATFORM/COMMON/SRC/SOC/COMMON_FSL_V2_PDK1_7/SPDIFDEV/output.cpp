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
//--------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//--------------------------------------------------------------------------------


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
    UNREFERENCED_PARAMETER(dwMultiplier);
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


PBYTE OutputStreamContextSPDIF32::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    DWORD dwSrcBytes = m_lpCurrDataEnd - m_lpCurrData;
    DWORD dwDstBytes = pBufferEnd - pBuffer;
    DWORD dwBytesToCopy = min(dwSrcBytes,dwDstBytes);

#ifdef DEBUG
    static int count = 0;
    DWORD i = 0;
#endif    

    memcpy(pBuffer,m_lpCurrData,dwBytesToCopy);

#ifdef DEBUG
    #define SAMPLES (100)
    if (count++ % SAMPLES == 1)
    {
         DEBUGMSG(ZONE_TEST, (_T("pBuffer = 0x%x, m_lpCurrData = 0x%x, Bytes = %d, count = %d \r\n"), pBuffer, m_lpCurrData, dwBytesToCopy, count));  
         for (i=0; i<4; i++)
             DEBUGMSG(ZONE_TEST, (_T("data[%d] = 0x%x \r\n"), i, *((DWORD*)pBuffer + i)));       
    }
#endif

    m_dwByteCount += dwBytesToCopy;
    m_lpCurrData += dwBytesToCopy;

    pBufferLast = pBuffer+dwBytesToCopy;
    
    return pBufferLast;
}

PBYTE OutputStreamContextSPDIF16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
#ifdef DEBUG
    #define SAMPLES (100)
    static int count = 0;
    DWORD i = 0;
#endif

    PBYTE pCurrData = m_lpCurrData;
    pBufferLast = pBuffer;

    while ((pBuffer < pBufferEnd) && (pCurrData < m_lpCurrDataEnd))
    {
        //data stored in bit 23-8
        ((HWSAMPLE *)pBuffer)[0] = *((UINT16*)pCurrData + 0) << 8; 
        
        ((HWSAMPLE *)pBuffer)[1] = *((UINT16*)pCurrData + 1) << 8;
        
        pBuffer += 2*sizeof(HWSAMPLE);
        pCurrData += 4;             
    }

#ifdef DEBUG
    if (count++ % SAMPLES == 1)
    {
         DEBUGMSG(ZONE_TEST, (_T("pBuffer = 0x%x, m_lpCurrData = 0x%x, Bytes = %d, count = %d \r\n"), pBufferLast, m_lpCurrData, pCurrData - m_lpCurrData, count));  
         for (i=0; i<4; i++)
             DEBUGMSG(ZONE_TEST, (_T("data[%d] = 0x%x \r\n"), i, *((DWORD*)pBufferLast + i)));       
    }
#endif

    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    
    return pBuffer;
}

PBYTE OutputStreamContextSPDIF24::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
#ifdef DEBUG
    #define SAMPLES (100)
    static int count = 0;
    DWORD i = 0;
#endif

    PBYTE pCurrData = m_lpCurrData;
    pBufferLast = pBuffer;

    while ((pBuffer < pBufferEnd) && (pCurrData < m_lpCurrDataEnd))
    {
        ((HWSAMPLE *)pBuffer)[0] =  pCurrData[0] | (pCurrData[1] << 8) | (pCurrData[2] << 16);

        pCurrData += 3;  
        ((HWSAMPLE *)pBuffer)[1] = pCurrData[0] | (pCurrData[1] << 8) | (pCurrData[2] << 16);
        pCurrData += 3;  
        
        pBuffer += 2*sizeof(HWSAMPLE);                
    }

#ifdef DEBUG
    if (count++ % SAMPLES == 1)
    {
         DEBUGMSG(ZONE_TEST, (_T("pBuffer = 0x%x, m_lpCurrData = 0x%x, Bytes = %d, count = %d \r\n"), pBufferLast, m_lpCurrData, pCurrData - m_lpCurrData, count));  
         for (i=0; i<4; i++)
             DEBUGMSG(ZONE_TEST, (_T("data[%d] = 0x%x \r\n"), i, *((DWORD*)pBufferLast + i)));  
    }
#endif

    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    
    return pBuffer;
}

