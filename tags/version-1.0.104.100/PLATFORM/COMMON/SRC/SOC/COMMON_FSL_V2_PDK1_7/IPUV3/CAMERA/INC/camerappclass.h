//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CameraPPClass.h
//
//  Common definitions for CameraPP module
//
//------------------------------------------------------------------------------
//

#ifndef __CAMERAPPCLASS_H__
#define __CAMERAPPCLASS_H__

//------------------------------------------------------------------------------
// Defines

#define CAMERAPP_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CAMERAPP_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Functions
class CameraPPClass
{
    public:
        CameraPPClass();
        ~CameraPPClass();

        BOOL CameraPPInit(void);
        void CameraPPDeinit(void);
        BOOL CameraPPOpenHandle(void);
        BOOL CameraPPCloseHandle(void);     
        BOOL CameraPPConfigure(pSMFCConfigData,pPpConfigData);
        // for the issue: new buffer manager
        void CameraPPSetBufferManager(CamBufferManager *);
        BOOL CameraPPAllocateBuffers(CamBufferManager *,ULONG, ULONG);
        BOOL CameraPPDeleteBuffers(CamBufferManager *);
        UINT32* CameraPPGetBufFilled();
        UINT32 CameraPPGetFrameCount();
        BOOL CameraPPStopChannel(void);
        BOOL CameraPPStartChannel(void);
        BOOL RegisterBuffer(CamBufferManager *, LPVOID pVirtualAddress, UINT32 pPhysicalAddress);
        BOOL UnregisterBuffer(CamBufferManager *, LPVOID pVirtualAddress, UINT32 pPhysicalAddress);
        BOOL Enqueue(CamBufferManager *);
        BOOL UnregisterAllBuffers();
        BOOL CameraPPSetInputBufPhyAddr(ULONG,UINT32*[]);
        UINT32* CameraPPGetInputBufPhyAddr(ULONG);

        // Event variables
        HANDLE m_hCameraPPEOFEvent;
    private:

        BOOL CameraPPInputConfigure(pSMFCConfigData,pIdmaChannel);
        BOOL CameraPPOutputConfigure(pPpConfigData);
        static void CameraPPWorkerThread(LPVOID);
        void CameraPPWorkerLoop(UINT32);

        HANDLE m_hPP;
        HANDLE m_hCameraPPWorkerThread;
        HANDLE m_hExitCameraPPWorkerThread;        
        BOOL   m_bCameraPPRunning;
        CRITICAL_SECTION m_csCameraPPStopping;
        
        // Frame count variables
        UINT32  m_iCameraPPFrameCount;
        
        //*****************************
        // Buffer Management Members
        //*****************************        
        UINT32 m_iCameraPPNumBuffers;
        UINT32* m_pCameraPPInputBufPhyAddr[NUM_PIN_BUFFER_MAX];
        CamBufferManager *m_pCameraPPBufferManager;

        // Event variables
        HANDLE m_hCameraPPInputBufEvent[NUM_PIN_BUFFER_MAX];

        LARGE_INTEGER m_lpFrequency;
        LARGE_INTEGER m_lpPerformanceCount_start;
        LARGE_INTEGER m_lpPerformanceCount_end;
        
};
#endif  // __CAMERAPPCLASS_H__
