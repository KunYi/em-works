//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  Header:  rxhwctx.h
//
//  Provides definitions for SPDIF Receiver
//  module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __RXHWCTX_H
#define __RXHWCTX_H

#if __cplusplus
extern "C" {
#endif

#include "common_regsspdif.h"
#include "common_spdifvs.h"

//-----------------------------------------------------------------------------
// Defines for SPDIFdware capabilities.

#define INCHANNELS    (2)                               

//----- Used for scheduling DMA transfers -----
#define  IN_BUFFER_A        0
#define  IN_BUFFER_B        1

class SpdifRxHwContext
{
protected:    
    SpdifRxHwContext();
    ~SpdifRxHwContext();

private:
    static SpdifRxHwContext *m_pSpdifRxHwContext;
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

    InputDeviceContext *m_pInputDeviceContext;

    // Keep the wave format that upper level prefers
    WAVEFORMATEX    m_WaveFormat; 

    PBYTE m_pInputDMABuffer[NUM_DMA_BUFFERS];
    BOOL  m_bInputDMARunning;


    volatile DWORD  m_dwInputDMAStatus;        // Input DMA channel's status
    UINT8  m_InputDMAChan;                   // Input DMA virtual channel index
    UINT32 m_InputDMALevel;                  // Input DMA watermark level

    CCHANNEL m_CChanel;
    QCHANNEL m_QChanel;
    UCHANNEL m_UChanel;

    PSPDIF_REG m_pSpdifReg;

    RX_INT_COUNTER m_intCounter;
    
    
public:
    static SpdifRxHwContext* GetHwContext();
    DWORD GetNumInputDevices(void);
    BOOL GetControlDetails(PRX_CONTROL_DETAILS pDetails);
    BOOL InitInputDMA(PHYSICAL_ADDRESS *pAddress);
    BOOL DeinitInputDMA();
    BOOL StartInputDMA();
    void StopInputDMA();
    BOOL GetDMARunState();
    UINT8 GetDMAChan() {return m_InputDMAChan;}
    void SetWaveFormat(WAVEFORMATEX *pFmt) {m_WaveFormat = *pFmt;}
    
    DeviceContext *GetInputDeviceContext(UINT DeviceId)
    {
        UNREFERENCED_PARAMETER(DeviceId);
        return m_pInputDeviceContext;
    }

    BOOL Init(DWORD Index, HardwareContext * pHardwareContext);
    BOOL Deinit();

    /*Map/Unmap the DMA buffers*/
    BOOL MapDMABuffers(PBYTE *pVirtDMABufferAddr);
    BOOL UnmapDMABuffers();
    void InterruptThread();

private:
    ULONG TransferInputBuffer(ULONG NumBuf);
    ULONG TransferInputBuffers(DWORD dwDCSR, DWORD checkFirst);

    //BSP LEVEL
    BOOL BSPInitInput(void);
    BOOL BSPSPDIFStartInput(void);
    BOOL BSPSPDIFStopInput(void);
    //BOOL BSPInitOutput(void);
    UINT32 BSPGetFreqMesa(void);

};

#ifdef __cplusplus
}
#endif

#endif // ____RXHWCTX_H
