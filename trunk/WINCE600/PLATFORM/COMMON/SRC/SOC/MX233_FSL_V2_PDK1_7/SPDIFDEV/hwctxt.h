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
//
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

//-----------------------------------------------------------------------------
//
//  File : HWCTXT.H
//
//  Platform dependent code for the SPDIF driver.
//
//-----------------------------------------------------------------------------

#ifndef __HWCTXT_H
#define __HWCTXT_H

#if __cplusplus
extern "C" {
#endif

#include <ceddk.h> // Needed for typedef of PHYSICAL_ADDRESS
#include "rxhwctx.h"
#include "txhwctx.h"


//-----------------------------------------------------------------------------
// Defines for audio hardware capabilities.


#define OUTCHANNELS   (2)    // SPDIF has two output channels.
//#define BITSPERSAMPLE (24)    // SPDIF uses 24 bits/sample

//#define INSAMPLERATE  (48000)    // Voice CODEC input at 48 kHz.
//#define OUTSAMPLERATE (48000)    // Stereo DAC output at 48 kHz.


// Define the size of each audio data word. The valid choices are:
//
//     INT8    for 8 bits/word
//     INT16   for 16 bits/word
//     INT32   for 24 bits/word
//
// The correct value to use depends upon the SPDIF hardware
//
// The word size that is defined here is also used to set the corresponding
// SDMA transfer sizes.
//
#if 1
typedef INT32 HWSAMPLE;

typedef HWSAMPLE *PHWSAMPLE;
#endif

typedef enum {
    SPDIF_PWR_STATE_OFF,
    SPDIF_PWR_STATE_STANDBY,
    SPDIF_PWR_STATE_ON
} SPDIF_PWR_STATE;



//----- Used to track DMA controllers status -----
#define DMA_CLEAR           0x00000000
#define DMA_DONEA           0x00000002
#define DMA_DONEB           0x00000004
#define DMA_BIU             0x00000008 // Determines which buffer is in use:
                                       // A=0, B=1.
#define DMA_STOP            0x00000010 // Stop DMA after buffers exhausted

//----- Used for scheduling DMA transfers -----
#define BUFFER_A            0
#define BUFFER_B            1

//----- Used for scheduling DMA transfers -----
#define  OUT_BUFFER_A       0
#define  OUT_BUFFER_B       1

#define AUDIO_REGKEY_PREFIX TEXT("Drivers\\BuiltIn\\SPDIF")

#define SPDIF_CONFIG_TX_ENABLED   0x00000001
#define SPDIF_CONFIG_RX_ENABLED   0x00000002

//------------Globals---------------------

//------------------- Board Support Package Interface ----------------------

class HardwareContext
{
private:
    SpdifRxHwContext *m_pSpdifRxHwContext;
    SpdifTxHwContext *m_pSpdifTxHwContext;
    static HardwareContext * m_pHardwareContext;
    PSPDIF_REG m_pSpdifReg;
    PHYSICAL_ADDRESS m_PhysDMABufferAddr;

    DWORD m_dwSysintrSPDIF;
    
    HANDLE m_hInterrupt;                // Handle to Audio Interrupt event.
    HANDLE m_hInterruptThread;          // Handle to thread which waits on
                                             // an audio interrupt event.
    DWORD m_DriverIndex;
    CRITICAL_SECTION m_Lock;
    BOOL m_bInitialized;

    BOOL m_bPowerdown;  // Flag to indicate if PowerDown() has been called.

    SPDIF_PWR_STATE m_CodecPwrState;
                                                   
    PBYTE m_pVirtDMABufferAddr;
    UINT32 m_Config;

    
    static const int NUM_DMA_BUFFERS = 2; // For DMA transfers to/from the
                                              // spdif hardware we need to allocate
                                              // 2 buffers and we just alternate
                                              // between them. Note that we allocate
                                              // separate pairs of DMA buffers for
                                              // RX/TX 
    
    static const int INTR_PRIORITY = 249; // Default interrupt thread priority.
    static const unsigned short INTR_PRIORITY_REGKEY[12];
    

protected:
     HardwareContext();
    ~HardwareContext();    
    
public:
    static HardwareContext* GetHwContext(DWORD Index);

    void Lock()   {EnterCriticalSection(&m_Lock);}
    void Unlock() {LeaveCriticalSection(&m_Lock);}

    DWORD GetNumInputDevices() {return m_pSpdifRxHwContext ? m_pSpdifRxHwContext->GetNumInputDevices() : 0;}
    DWORD GetNumOutputDevices() {return m_pSpdifTxHwContext ? m_pSpdifTxHwContext->GetNumOutputDevices() : 0;}
    DWORD GetNumMixerDevices() {return 1;}

    SpdifRxHwContext *GetRxHwContext() {return m_pSpdifRxHwContext;}
    SpdifTxHwContext *GetTxHwContext() {return m_pSpdifTxHwContext;}

    PSPDIF_REG GetSPDIFReg() {return m_pSpdifReg;}
    DeviceContext *GetInputDeviceContext(UINT DeviceId);
    DeviceContext *GetOutputDeviceContext(UINT DeviceId);

    BOOL TxEnabled() {return (m_Config & SPDIF_CONFIG_TX_ENABLED) >> 0;}
    BOOL RxEnabled() {return (m_Config & SPDIF_CONFIG_RX_ENABLED) >> 1;}

    void DumpHwRegs();
    
    BOOL Init(DWORD Index);                                        
    BOOL Deinit();

    void PowerUp();
    void PowerDown();

    BOOL StartOutputDMA();
    void  StopOutputDMA();

    BOOL StartInputDMA();
    void  StopInputDMA();
    void InterruptThread();

private:

    BOOL InitDMA(PHYSICAL_ADDRESS address);
    BOOL InitInterruptThread();
    BOOL MapRegisters();
    BOOL UnmapRegisters();
    BOOL MapDMABuffers();
    BOOL UnmapDMABuffers();

    DWORD GetInterruptThreadPriority();

    //------------------- Board Support Package Interface ----------------------
    BOOL BSPInit();
    BOOL BSPDeinit();

    void BSPAllocDMABuffers(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysicalAddress);
    void BSPDeallocDMABuffers(PVOID virtualAddress);
    BOOL BSPRequestIrqs(VOID);

    //void BSPAudioInitCodec(AUDIO_BUS bus);
    //void BSPAudioSetCodecPower(AUDIO_PWR_STATE state);
    
    void BSPSPDIFPowerUp();
    void BSPSPDIFPowerDown();
    void BSPSPDIFGetConfig(UINT32 *pConfig);

};


extern DWORD CALLBACK CallInterruptThread(HardwareContext *pHWContext);
//extern DWORD CALLBACK BSPAudioDisableDelayHandler(HardwareContext *pHWContext);
//--------------------------------- Externs ------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __HWCTXT_H
