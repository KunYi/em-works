//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CamBuffer.h
//
//  Class definition for an object to manage multiple buffering
//  in the DMA channel of the CSI controller.
//
//------------------------------------------------------------------------------

#ifndef __CAMBUFFER_H__
#define __CAMBUFFER_H__

//------------------------------------------------------------------------------
// Functions
class CamBuffer
{
    public:
        CamBuffer(DWORD dwBufferSize);
        ~CamBuffer();

        UINT32* PhysAddr();
        UINT32* VirtAddr();
        DWORD BufSize();

    private:
        UINT32 *m_pPhysAddr;
        UINT32 *m_pVirtAddr;
        DWORD m_dwBufSize;
};

#endif  // __CAMBUFFER_H__
