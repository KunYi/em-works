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
// ----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
// ----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

#include "wavemain.h"

// Init input m_DeltaT with (HWSampleRate/SampleRate) calculated in 24.8 fixed
// point form. Note that we need to hold the result in a 64-bit value until
// we're done shifting, since the result of the multiply will overflow 32 bits
// for sample rates greater than or equal to the hardware's sample rate.
DWORD InputStreamContext::SetRate(DWORD dwMultiplier)
{
    UNREFERENCED_PARAMETER(dwMultiplier);
    return MMSYSERR_NOTSUPPORTED;
}

DWORD InputStreamContext::Stop()
{
    // Stop the stream
    WaveStreamContext::Stop();

    // Return any partially filled buffers to the client
    if ((m_lpWaveHdrCurrent) && (m_lpWaveHdrCurrent->dwBytesRecorded>0))
    {
        GetNextBuffer();
    }

    return MMSYSERR_NOERROR;
}

PBYTE InputStreamContext::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    UNREFERENCED_PARAMETER(pBufferLast);
    DWORD dwDstBytes = m_lpCurrDataEnd - m_lpCurrData;
    DWORD dwSrcBytes = pBufferEnd - pBuffer;
    DWORD dwBytesToCopy = min(dwSrcBytes,dwDstBytes);

#ifdef DEBUG
    #define SAMPLES (100)
    static int count = 0;
    DWORD i = 0;

    if (count++ % SAMPLES == 1)
    {
         DEBUGMSG(ZONE_TEST, (_T("pBuffer = 0x%x, m_lpCurrData = 0x%x, Bytes = %d, count = %d \r\n"), pBuffer, m_lpCurrData, dwBytesToCopy, count));  
         for (i=0; i<4; i++)
             DEBUGMSG(ZONE_TEST, (_T("data[%d] = 0x%x \r\n"), i, *((DWORD*)pBuffer + i)));       
    }
#endif

    memcpy(m_lpCurrData, pBuffer, dwBytesToCopy);

    m_lpWaveHdrCurrent->dwBytesRecorded += dwBytesToCopy;
    m_dwByteCount += dwBytesToCopy;
    m_lpCurrData += dwBytesToCopy;
    pBuffer += dwBytesToCopy; 

    
    return  pBuffer;
}

PBYTE InputStreamContextSPDIF32::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    UNREFERENCED_PARAMETER(pBufferLast);
    DWORD dwDstBytes = m_lpCurrDataEnd - m_lpCurrData;
    DWORD dwSrcBytes = pBufferEnd - pBuffer;
    DWORD dwBytesToCopy = min(dwSrcBytes,dwDstBytes);

#ifdef DEBUG
    #define SAMPLES (100)
    static int count = 0;
    DWORD i = 0;

    if (count++ % SAMPLES == 1)
    {
         DEBUGMSG(ZONE_TEST, (_T("pBuffer = 0x%x, m_lpCurrData = 0x%x, Bytes = %d, count = %d \r\n"), pBuffer, m_lpCurrData, dwBytesToCopy, count));  
         for (i=0; i<4; i++)
             DEBUGMSG(ZONE_TEST, (_T("data[%d] = 0x%x \r\n"), i, *((DWORD*)pBuffer + i)));  
    }
#endif    

    memcpy(m_lpCurrData, pBuffer, dwBytesToCopy);

    m_lpWaveHdrCurrent->dwBytesRecorded += dwBytesToCopy;
    m_dwByteCount += dwBytesToCopy;
    m_lpCurrData += dwBytesToCopy;
    pBuffer += dwBytesToCopy; 

    return  pBuffer;
}

PBYTE InputStreamContextSPDIF16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
#ifdef DEBUG
    #define SAMPLES (100)
    static int count = 0;
    DWORD i = 0;
#endif    
  
    PBYTE pCurrData = m_lpCurrData;
    //PBYTE pBuffer0 = pBuffer; // for debug
    pBufferLast = pBuffer;

    while ((pBuffer < pBufferEnd) && (pCurrData < m_lpCurrDataEnd))
    {
        //data stored in bit 23-8
        pCurrData[0] = pBuffer[1];
        pCurrData[1] = pBuffer[2];
        
        pBuffer += sizeof(HWSAMPLE);
        pCurrData += 2; 

        pCurrData[0] = pBuffer[1];
        pCurrData[1] = pBuffer[2];
        
        pBuffer += sizeof(HWSAMPLE);
        pCurrData += 2; 

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
    m_lpWaveHdrCurrent->dwBytesRecorded += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
   
    return pBuffer;
}

PBYTE InputStreamContextSPDIF24::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
#ifdef DEBUG
    #define SAMPLES (100)
    static int count = 0;
    DWORD i = 0;
#endif

    PBYTE pCurrData = m_lpCurrData;
    //PBYTE pBuffer0 = pBuffer; // for debug
    pBufferLast = pBuffer;

    while ((pBuffer < pBufferEnd) && (pCurrData < m_lpCurrDataEnd))
    {
        pCurrData[0] =  pBuffer[0] ;
        pCurrData[1] =  pBuffer[1] ;
        pCurrData[2] =  pBuffer[2] ;

        pCurrData += 3;  
        pBuffer += sizeof(HWSAMPLE);  

        pCurrData[0] =  pBuffer[0] ;
        pCurrData[1] =  pBuffer[1] ;
        pCurrData[2] =  pBuffer[2] ;

        pCurrData += 3;  
        pBuffer += sizeof(HWSAMPLE);
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
    m_lpWaveHdrCurrent->dwBytesRecorded += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
     
    return pBuffer;
}
