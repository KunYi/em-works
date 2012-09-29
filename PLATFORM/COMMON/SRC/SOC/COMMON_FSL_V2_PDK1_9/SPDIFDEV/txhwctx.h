//------------------------------------------------------------------------------
//
//  Copyright (C) 2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Header:  txhwctx.h
//
//  Provides definitions for SPDIF Transmitter
//  module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __TXHWCTX_H
#define __TXHWCTX_H

#if __cplusplus
extern "C" {
#endif

#include "common_spdif.h"

class SpdifTxHwContext
{
protected:    
    SpdifTxHwContext();
    ~SpdifTxHwContext();

private:
    static SpdifTxHwContext *m_pSpdifTxHwContext;
    HardwareContext * m_pHardwareContext;

    BOOL m_bInitialized;

    //CRITICAL_SECTION m_Lock;

    static const int NUM_DMA_BUFFERS = 2; // For DMA transfers to/from the
                                          // audio hardware we need to allocate
                                          // 2 buffers and we just alternate
                                          // between them. Note that we allocate
                                          // separate pairs of DMA buffers for
                                          // output/playback and for
                                          // input/recording.

    OutputDeviceContext *m_pOutputDeviceContext;

    // Keep the wave format that upper level prefers
    WAVEFORMATEX    m_WaveFormat; 
    
    //BOOL m_bPowerdown;  // Flag to indicate if PowerDown() has been called.

    PBYTE m_pOutputDMABuffer[NUM_DMA_BUFFERS];
    BOOL  m_bOutputDMARunning;


    volatile DWORD  m_dwOutputDMAStatus;        // Output DMA channel's status
    UINT8  m_OutputDMAChan;                   // Output DMA virtual channel index
    UINT32 m_OutputDMALevel;                  // Output DMA watermark level


    CCHANNEL m_CChanel;

    PSPDIF_REG m_pSpdifReg;

    TX_INT_COUNTER m_intCounter;

    PBYTE m_pTxFifoPrefill;
    int m_iTxFifoPrefill; 
    
public:
    static SpdifTxHwContext* GetHwContext();

    //void Lock()   {EnterCriticalSection(&m_Lock);}
    //void Unlock() {LeaveCriticalSection(&m_Lock);}

    DWORD GetNumOutputDevices(void);
    BOOL SetControlDetails(PTX_CONTROL_DETAILS pDetails);

    BOOL InitOutputDMA(PHYSICAL_ADDRESS *pAddress);
    BOOL DeinitOutputDMA();
    BOOL StartOutputDMA();
    void StopOutputDMA();
    BOOL GetDMARunState();
    UINT8 GetDMAChan() {return m_OutputDMAChan;}
    void SetWaveFormat(WAVEFORMATEX *pFmt) {m_WaveFormat = *pFmt;}
    

    DeviceContext *GetOutputDeviceContext(UINT DeviceId)
    {
        UNREFERENCED_PARAMETER(DeviceId);
        return m_pOutputDeviceContext;
    }

    BOOL Init(DWORD Index, HardwareContext * pHardwareContext);
    BOOL Deinit();

    /*Map/Unmap the DMA buffers*/
    BOOL MapDMABuffers(PBYTE *pVirtDMABufferAddr);
    BOOL UnmapDMABuffers();

    void InterruptThread();

private:
    ULONG TransferOutputBuffer(ULONG NumBuf);
    ULONG TransferOutputBuffers(DWORD dwDCSR);

    //BSP LEVEL
   
    BOOL BSPSPDIFStartOutput(void);
    BOOL BSPSPDIFStopOutput(void);

    BOOL BSPInitOutput();
};

#ifdef __cplusplus
}
#endif

#endif // ____TXHWCTX_H
