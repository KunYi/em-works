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
/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:    HWCTXT.CPP

Abstract:               Platform dependent code for the mixing audio driver.

Notes:                  The following file contains all the hardware specific code
for the mixing audio driver.  This code's primary responsibilities
are:

* Initialize audio hardware (including codec chip)
* Schedule DMA operations (move data from/to buffers)
* Handle audio interrupts

All other tasks (mixing, volume control, etc.) are handled by the "upper"
layers of this driver.
*/
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2010 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include <wavedbg.h>
#include <nkintr.h>
#include <ceddk.h>
#include "wavemain.h"


//-----------------------------------------------------------------------------
// External Functions
extern UINT32 GetSsiBaseRegAddr(UINT32 index);
extern UINT32 GetAudmuxBaseRegAddr();
extern DWORD GetSdmaChannelIRQ(UINT32 chan);

extern UINT8 BSPAudioGetInputDeviceNum();
extern UINT16 BSPAllocOutputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr);
extern UINT16 BSPAllocInputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr);
extern VOID BSPDeallocOutputDMABuffer(PBYTE VirtAddr);
extern VOID BSPDeallocInputDMABuffer(PBYTE VirtAddr);
extern BOOL BSPAudioInitCodec();
extern VOID BSPAudioDeinitCodec();
extern VOID BSPAudioStartDAC();
extern VOID BSPAudioStopDAC();
extern VOID BSPAudioStartADC();
extern VOID BSPAudioStopADC();
extern VOID BSPAudioSelectADCSource(DWORD nIndex);
extern BOOL BSPAudioSetCodecLoopback(BOOL bEnable);
extern VOID BSPAudioSetOutputGain(DWORD dwGain);
extern VOID BSPAudioSetInputGain(DWORD dwGain);
extern VOID BSPAudioIomuxConfig();
extern BOOL BSPSsiEnableClock(UINT32 index, BOOL bEnable);
extern BOOL BSPAudmuxEnableClock(BOOL bEnable);
extern UINT8 BSPAudioGetSdmaPriority();
extern VOID BSPAudioPowerUp(const BOOL fullPowerUp);
extern VOID BSPAudioPowerDown(const BOOL fullPowerOff);
extern AUDIO_PROTOCOL BSPAudioGetProtocol();
extern AUDMUX_INTERNAL_PORT BSPGetAudmuxIntPort();
extern AUDMUX_EXTERNAL_PORT BSPGetAudmuxExtPort();
extern void BSPSetSpeakerEnable(BOOL bEnable);
extern void BSPAudioHandleHPDetectIRQ();
extern BOOL BSPIsHeadphoneDetectSupported();
extern DWORD BSPGetHPDetectIRQ();
extern WORD BSPGetBitsPerSample();


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

// Define the SSI transmitter low water and receiver high watermarks. These
// watermark levels are used to trigger DMA requests to the core so that
// additional data can be added to the FIFO (for transmit) or data can be
// read from the FIFO (for receive).
//
// The valid values are from 1 to 8 (the SSI FIFO depth). Note that 0 is not
// a valid value for either the transmitter or receiver watermark levels and
// setting the watermark level to zero will prevent the SSI from functioning
// correctly.
//
// The proper watermark level to be used for a particular platform may need to
// be determined through performance testing and tuning. The key requirement is
// that both transmitter underrun and receiver overrun conditions must be
// avoided. Otherwise audio data will be lost. There is even the possibility
// of the left/right audio channels being randomly switched due to the nature
// of how the SSI hardware handles underrun and overrun conditions.
//
// See also the definition of audioDMAPageSize (in hwctxt.h) which defines
// the size in bytes of each DMA buffer. The size of the DMA buffer along with
// the SSI FIFO watermark levels are the two performance tuning parameters
// that are available for avoiding SSI FIFO underrun and overrun errors.
//
#define SSI_SFCSR_TX_WATERMARK      4
#define SSI_SFCSR_RX_WATERMARK      4

#define SSI_FIFO_DEPTH      8

//----- Used to track DMA controllers status -----
#define DMA_CLEAR           0x00000000
#define DMA_DONEA           0x00000002
#define DMA_DONEB           0x00000004

//----- Used for scheduling DMA transfers -----
#define BUFFER_A            0
#define BUFFER_B            1


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
HardwareContext *g_pHWContext=NULL;
WORD g_BitsPerSample;    // support 16/24 bit


//-----------------------------------------------------------------------------
// Local Variables
static UINT32 PlaybackDisableDelayMsec = 1000;
static UINT32 RecordDisableDelayMsec = 1000;

static BOOL g_saveOutputDMARunning = FALSE;
static BOOL g_saveInputDMARunning  = FALSE;


BOOL HardwareContext::CreateHWContext(DWORD Index)
{
    if (g_pHWContext)
    {
        return TRUE;
    }

    g_pHWContext = new HardwareContext;
    if (!g_pHWContext)
    {
        return FALSE;
    }

    return g_pHWContext->Init(Index);
}

HardwareContext::HardwareContext():
m_InputDeviceContext(),
m_OutputDeviceContext()
{
    InitializeCriticalSection(&m_Lock);
    m_Initialized = FALSE;
    
    m_NumInputDevices = BSPAudioGetInputDeviceNum();

    m_hAudioHPDetectIntrEvent = NULL;
    m_hAudioHPDetectIntrThread = NULL;
}

HardwareContext::~HardwareContext()
{
    DeleteCriticalSection(&m_Lock);
}


BOOL HardwareContext::Init(DWORD Index)
{
    if (m_Initialized)
    {
        return FALSE;
    }

    //
    // Initialize the state/status variables
    //
    m_PowerUpState      = TRUE;
    
    m_DriverIndex       = Index;
    m_InPowerHandler    = FALSE;
    m_InputDMARunning   = FALSE;
    m_OutputDMARunning  = FALSE;
    m_NumForcedSpeaker  = 0;

    m_dwOutputGain = 0xFFFFFFFF;
    m_dwInputGain  = 0xFFFFFFFF;
    m_fInputMute  = FALSE;
    m_fOutputMute = FALSE;

    m_OutputDMAStatus = DMA_CLEAR;
    m_InputDMAStatus  = DMA_CLEAR;
    
    m_bCanStopOutputDMA   = TRUE;
    m_bSSITXUnderrunError = FALSE;
    m_bSSIRXOverrunError = FALSE;
    m_delayBypass         = FALSE;
    m_bCodecLoopbackState = FALSE;

    m_InputDeviceContext.SetBaseSampleRate(INPUT_SAMPLERATE);
    m_OutputDeviceContext.SetBaseSampleRate(OUTPUT_SAMPLERATE);
    g_BitsPerSample = BSPGetBitsPerSample();

    //
    // Initialize hardware
    //
    PHYSICAL_ADDRESS phyAddr;

    // Map audio peripherals
    phyAddr.QuadPart = GetSsiBaseRegAddr(2);
    m_pSSI = (PCSP_SSI_REG) MmMapIoSpace(phyAddr, sizeof(CSP_SSI_REG), FALSE);
    if (m_pSSI == NULL)
    {
        ERRMSG("MmMapIoSpace failed for SSI");
        goto exit;
    }

    phyAddr.QuadPart = GetAudmuxBaseRegAddr();
    m_pAUDMUX= (PCSP_AUDMUX_REG) MmMapIoSpace(phyAddr, sizeof(CSP_AUDMUX_REG), FALSE);
    if (m_pAUDMUX == NULL)
    {
        ERRMSG("MmMapIoSpace failed for AUDMUX");
        goto exit;
    }

    // Initialize audio Codec
    if (!BSPAudioInitCodec())
    {
        ERRMSG("Initialize Audio Codec failed");
        goto exit;
    }

    // Initialize SSI
    InitSsi(BSPAudioGetProtocol());

    // We need to init AUDMUX last (only after both SSI and Codec 
    // have been properly configured) to avoid any possible signal 
    // conflicts due to any previous SSI and/or Codec settings.
    //InitAudmux(AUDMUX_PORT2, AUDMUX_PORT4);
    InitAudmux(BSPGetAudmuxIntPort(), BSPGetAudmuxExtPort());

    // Initialize the Ouput DMA chanel
    InitOutputDMA();

    // Initialize the Input DMA chanel
    InitInputDMA();

    // Setup dma_pages
    m_DMAOutPageVirtAddr[BUFFER_A] = m_DMAOutBufVirtAddr;
    m_DMAOutPageVirtAddr[BUFFER_B] = m_DMAOutBufVirtAddr + m_OutputDMAPageSize;

    m_DMAInPageVirtAddr[BUFFER_A] = m_DMAInBufVirtAddr;
    m_DMAInPageVirtAddr[BUFFER_B] = m_DMAInBufVirtAddr + m_InputDMAPageSize;

    // Initialize the mixer controls in mixerdrv.cpp
    InitMixerControls();

    // Note that the audio driver interrupt handler will only run if
    // m_Initialized is TRUE so we must initialize the state variable
    // here before trying to create the IST thread.
    m_Initialized = TRUE;

    // Initialize interrupt thread
    if (!InitInterruptThread())
    {
        ERRMSG("Failed to initialize output interrupt thread");
        goto exit;
    }


    if (!InitDisableDelayThread())
    {
        ERRMSG("Failed to initialize disable delay thread");
        goto exit;
    }

    // Initialize interrupt thread for Headphone detect if supported
    if(BSPIsHeadphoneDetectSupported())
    {
        if (!InitHPDetectInterruptThread())
        {
            ERRMSG("Failed to initialize headphone detect interrupt thread");
            goto exit;
        }
    }
    
exit:
    
    return m_Initialized;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:               Deinit()

Description:    Deinitializest the hardware: disables DMA channel(s),
                                clears any pending interrupts, powers down the audio
                                codec chip, etc.

Returns:                Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::Deinit()
{
    m_Initialized = FALSE;

    // Wake up the threads so that they can clean up 
    // and then terminate (now that m_Initialize is FALSE).
    if (m_hStopOutputDMAEvent != NULL)
    {
        SetEvent(m_hStopOutputDMAEvent);
    }
    if (m_hAudioIntrEvent != NULL)
    {
        SetEvent(m_hAudioIntrEvent);
    }
    
    // If Headphone detect is supported, cleanup related event and thread
    // that were created in init function
    if(BSPIsHeadphoneDetectSupported())
    {    
        if (m_hAudioHPDetectIntrEvent != NULL)
        {
            SetEvent(m_hAudioHPDetectIntrEvent);
        }        
        // Release headphone detect SYSINTR
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_AudioHPDetectSysIntr, sizeof(DWORD),
                        NULL, 0, NULL);
        m_AudioHPDetectSysIntr = (DWORD)SYSINTR_UNDEFINED;

        // Kill the headphone detect IST thread
        if (m_hAudioHPDetectIntrThread)
        {
            CloseHandle(m_hAudioHPDetectIntrThread);
            m_hAudioHPDetectIntrThread = NULL;
        }
    }

    // Free DMA buffers
    BSPDeallocOutputDMABuffer(m_DMAOutBufVirtAddr);
    BSPDeallocInputDMABuffer(m_DMAInBufVirtAddr);

    // Deinit audio codec
    BSPAudioDeinitCodec();

    // Unmap audio peripherals
    if (m_pSSI)
    {
        MmUnmapIoSpace(m_pSSI, sizeof(CSP_SSI_REG));
        m_pSSI = NULL;
    }

    if (m_pAUDMUX)
    {
        MmUnmapIoSpace(m_pAUDMUX, sizeof(CSP_AUDMUX_REG));
        m_pAUDMUX = NULL;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: InitSsi
//
//  This function initializes the specified audio port for input/output.
//
//  Parameters:
//      The SSI configuration to be used.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::InitSsi(AUDIO_PROTOCOL protocol)
{
    UINT32 ssi_i2s_mode;
    UINT32 ssi_stcr_tsckp_mode;
    UINT32 ssi_stcr_tfsi_mode;
    UINT32 ssi_stccr_dc_mode;
    UINT32 ssi_scr_net_mode;
    UINT32 ssi_stcr_tfsl_mode;
    UINT32 ssi_stccr_wl;    

    if (protocol == AUDIO_PROTOCOL_NETWORK)
    {
        ssi_i2s_mode        = SSI_SCR_I2S_MODE_NORMAL;
        ssi_stcr_tsckp_mode = SSI_STCR_TSCKP_RISING_EDGE;
        ssi_stcr_tfsi_mode  = SSI_STCR_TFSI_ACTIVE_HIGH;
        ssi_stccr_dc_mode   = 3;
        ssi_scr_net_mode    = SSI_SCR_NET_ENABLE;
        ssi_stcr_tfsl_mode  = SSI_STCR_TFSL_1BIT;
    }
    else // (protocol == AUDIO_PROTOCOL_I2S)
    {
        ssi_i2s_mode        = SSI_SCR_I2S_MODE_SLAVE;
        ssi_stcr_tsckp_mode = SSI_STCR_TSCKP_FALLING_EDGE;
        ssi_stcr_tfsi_mode  = SSI_STCR_TFSI_ACTIVE_LOW;
        ssi_stccr_dc_mode   = 1;
        ssi_scr_net_mode    = SSI_SCR_NET_DISABLE;
        ssi_stcr_tfsl_mode  = SSI_STCR_TFSL_1WORD;
    }

    // Get sample size
    switch (g_BitsPerSample)
    {
    case 8:
        ssi_stccr_wl = SSI_STCCR_WL_8BIT;
        break;

    case 24:
    case 32:
        ssi_stccr_wl = SSI_STCCR_WL_24BIT;
        break;

    default:
        ssi_stccr_wl = SSI_STCCR_WL_16BIT;
    }

    // Enable SSI clocks before we access the SSI registers.
    BSPSsiEnableClock(2, TRUE);

    // Disable the SSI transmitter and receiver.
    CLRREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_RE) | CSP_BITFMASK(SSI_SCR_TE));

    // Also mask all transmitter and receiver timeslots until we are ready
    // to begin a new audio I/O operation.
    OUTREG32(&m_pSSI->STMSK, 0xffffffff);
    OUTREG32(&m_pSSI->SRMSK, 0xffffffff);

    // Next disable the SSI so that we can reconfigure the items that
    // can only be changed when the SSI is disabled.
    //
    // We should do this as a separate write from disabling the SSI
    // transmitter and receiver in order to give the SSI sufficient
    // time to properly complete the previous operation.
    CLRREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_SSIEN));

    // Disable all SSI interrupts and DMA requests.
    OUTREG32(&m_pSSI->SIER, 0);

    // Also disable the transmit and receive FIFOs so that we can configure
    // the DMA watermark levels later.
    OUTREG32(&m_pSSI->STCR, 0);
    OUTREG32(&m_pSSI->SRCR, 0);

    // We can now begin reconfiguring the SSI for synchronous 4-wire mode.
    // Since we are using synchronous mode, we only need to configure the
    // SSI transmitter clock in the following stages and there is no need to
    // separately set the SSI receiver clock configuration.
    //
    // The SSI is configured for I2S master/slave mode here or just "normal"
    // mode if NETWORK_MODE was selected for the audio bus above. All clock
    // configuration settings for the selected master/slave mode will be done
    // later.
    //
    // The SSI SYS_CLK clock output is also disabled here since we do not need
    // it (even if the SSI is operated in master mode since the slave device
    // just needs to receive the framesync and bitclock signals).
    //
    // We never use 2-channel mode (where both FIFO0 and FIFO1 are active) so
    // that is disabled here as well.
    OUTREG32(&m_pSSI->SCR, 
        CSP_BITFVAL(SSI_SCR_CLK_IST, SSI_SCR_CLK_IST_HIGH)      |
        CSP_BITFVAL(SSI_SCR_TCH_EN, SSI_SCR_TCH_EN_2CHAN_OFF)   |
        CSP_BITFVAL(SSI_SCR_SYS_CLK_EN, SSI_SCR_SYS_CLK_EN_OFF) |
        CSP_BITFVAL(SSI_SCR_I2S_MODE, ssi_i2s_mode)             |
        CSP_BITFVAL(SSI_SCR_SYN, SSI_SCR_SYN_SYNC)              |
        CSP_BITFVAL(SSI_SCR_NET, ssi_scr_net_mode));
 
    // Next, continue with the configuration of the SSI transmitter for
    // audio output/playback.
    //
    // To match the PMIC's capabilities, the SSI transmitter must transmit
    // 16-bit words MSB-first. Only FIFO0 is needed for single channel
    // operation but we leave it disabled until we are actually ready to
    // start an audio I/O operation.
    //
    // Also, the SSI transmitter's clock and framesync signals are configured
    // here for the slave mode of operation. We will change this later if it
    // turns out that the SSI is to be operated in bus master mode.
    //
    // Note that we must configure the transmitter's clock settings even if we
    // never use this SSI for audio playback because we are using synchronous
    // mode where the receiver shares the same clock configuration as the
    // transmitter.
    OUTREG32(&m_pSSI->STCR,
        CSP_BITFVAL(SSI_STCR_TXBIT0, SSI_STCR_TXBIT0_LSB_ALIGNED) |
        CSP_BITFVAL(SSI_STCR_TFEN1,  SSI_STCR_TFEN1_DISABLE)      |
        CSP_BITFVAL(SSI_STCR_TFEN0,  SSI_STCR_TFEN0_DISABLE)      |
        CSP_BITFVAL(SSI_STCR_TFDIR,  SSI_STCR_TFDIR_EXTERNAL)     |
        CSP_BITFVAL(SSI_STCR_TXDIR,  SSI_STCR_TXDIR_EXTERNAL)     |
        CSP_BITFVAL(SSI_STCR_TSHFD,  SSI_STCR_TSHFD_MSB_FIRST)    |
        CSP_BITFVAL(SSI_STCR_TSCKP,  ssi_stcr_tsckp_mode)         |
        CSP_BITFVAL(SSI_STCR_TFSI,   ssi_stcr_tfsi_mode)          |
        CSP_BITFVAL(SSI_STCR_TFSL,   ssi_stcr_tfsl_mode)          |
        CSP_BITFVAL(SSI_STCR_TEFS,   SSI_STCR_TEFS_EARLY)
    );
 
    // Now configure the SSI receiver for audio input/recording.
    //
    // To match the PMIC's capabilities, the SSI receiver must receive 
    // 16-bit words MSB-first. Only FIFO0 is needed for single channel
    // operation but we leave it disabled until we are actually ready to
    // start an audio I/O operation.
    //
    // Also, we skip configuring the SSI receiver's clock settings here because
    // we're using synchronous mode and we've already configured the SSI's
    // transmitter clock configuration above. The receiver will simply share
    // the same clock configuration as the transmitter.
    //
    // Note that it is harmless to also configure the SSI receiver even if we
    // never use this SSI for audio recording.
    OUTREG32(&m_pSSI->SRCR,
        CSP_BITFVAL(SSI_SRCR_RXBIT0, SSI_SRCR_RXBIT0_LSB_ALIGNED) |
        CSP_BITFVAL(SSI_SRCR_RFEN1,  SSI_SRCR_RFEN1_DISABLE)      |
        CSP_BITFVAL(SSI_SRCR_RFEN0,  SSI_SRCR_RFEN0_DISABLE)      |
        CSP_BITFVAL(SSI_SRCR_RSHFD,  SSI_SRCR_RSHFD_MSB_FIRST)
    );

    // Set the Receive FIFO empty and Transmit FIFO full watermark levels.
    //
    // We set the watermark levels for both FIFO0 and FIFO1 here even though
    // we will only use FIFO0 because all of the watermark levels must be set
    // to a valid value (regardless of whether the FIFO is used or not) in
    // order for the SSI to operate properly.
    OUTREG32(&m_pSSI->SFCSR,
        CSP_BITFVAL(SSI_SFCSR_RFWM0, SSI_SFCSR_RX_WATERMARK) |
        CSP_BITFVAL(SSI_SFCSR_TFWM0, SSI_SFCSR_TX_WATERMARK) |
        CSP_BITFVAL(SSI_SFCSR_RFWM1, SSI_SFCSR_RX_WATERMARK) |
        CSP_BITFVAL(SSI_SFCSR_TFWM1, SSI_SFCSR_TX_WATERMARK));

    // Configure the SSI transmit clock for slave mode operation where
    // the internal divider and prescaler parameters are irrelevant. The
    // key settings are DC to select words/frame and WL for bits/word to
    // match the Codec's configuration.
    OUTREG32(&m_pSSI->STCCR,
        CSP_BITFVAL(SSI_STCCR_DIV2, SSI_STCCR_DIV2_BYPASS)    |
        CSP_BITFVAL(SSI_STCCR_PSR, SSI_STCCR_PSR_DIV8_BYPASS) |
        CSP_BITFVAL(SSI_STCCR_WL, ssi_stccr_wl)               |
        CSP_BITFVAL(SSI_STCCR_DC, ssi_stccr_dc_mode)          |
        CSP_BITFVAL(SSI_STCCR_PM, 0));

    // Leave the SSI in a disabled state until we are actually ready to perform
    // audio playback or recording.
    //
    // Also note that we leave all of the transmit and receive timeslots masked
    // at this time and only unmask the required timeslots when we call either
    // BSPAudioStartSsiOutput() or BSPAudioStartSsiInput().

    // Disable SSI clocks to minimize power consumption.
    BSPSsiEnableClock(2, FALSE);
}


//-----------------------------------------------------------------------------
//
//  Function: InitAudmux
//
//  This function configures the Audio MUX to connect/disconnect the SSI
//  to the external power management IC.
//
//  Parameters:
//      intPort [in]   Audio MUX internal port for connection to SSI.
//      extPort [in]   Audio MUX external port for connection to PMIC.
//      bMaster [in]   Boolean flag to select SSI Master (if TRUE) or
//                     PMIC Master (if FALSE) modes.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::InitAudmux(AUDMUX_INTERNAL_PORT intPort, AUDMUX_EXTERNAL_PORT extPort)
{
    PUINT32 pPTCR, pPDCR;

    // Enable AUDMUX clock
    BSPAudmuxEnableClock(TRUE);
    
    // Configure the Audio MUX based upon that Codec is acting as the bus master.
    // 
    // We also configure the both internal and external port for synchronous 4-wire
    // operation in normal mode (which is what we actually need to support the SSI 
    // and Codec for either network or I2S modes).
    //
    // Note that we only configure the transmit framesync and bitclock here
    // because we are using synchronous mode and the receiver clock
    // settings will be determined by the transmitter settings.

    // Get pointers to the Audio MUX internal port registers.
    pPTCR = &m_pAUDMUX->PTCR1 + intPort*2;
    pPDCR = &m_pAUDMUX->PDCR1 + intPort*2;

    // All clock signals for the internal port are all output signals for
    // Codec master mode.
    OUTREG32(pPTCR,
        CSP_BITFVAL(AUDMUX_PTCR_TFSDIR, AUDMUX_PTCR_TFSDIR_OUTPUT)   |
        CSP_BITFVAL(AUDMUX_PTCR_TFSEL, extPort)                      |
        CSP_BITFVAL(AUDMUX_PTCR_TCLKDIR, AUDMUX_PTCR_TCLKDIR_OUTPUT) |
        CSP_BITFVAL(AUDMUX_PTCR_TCSEL, extPort)                      |
        CSP_BITFVAL(AUDMUX_PTCR_SYN, AUDMUX_PTCR_SYN_SYNC));

    OUTREG32(pPDCR,
        CSP_BITFVAL(AUDMUX_PDCR_RXDSEL, extPort)                    |
        CSP_BITFVAL(AUDMUX_PDCR_TXRXEN, AUDMUX_PDCR_TXRXEN_NO_SWAP) |
        CSP_BITFVAL(AUDMUX_PDCR_MODE,   AUDMUX_PDCR_MODE_NORMAL));

    // Get pointers to the Audio MUX external port registers.
    pPTCR = &m_pAUDMUX->PTCR1 + extPort*2;
    pPDCR = &m_pAUDMUX->PDCR1 + extPort*2;

    // All clock signals for the external port are input signals for
    // Codec master mode.
    OUTREG32(pPTCR,
        CSP_BITFVAL(AUDMUX_PTCR_TFSDIR,  AUDMUX_PTCR_TFSDIR_INPUT)  |
        CSP_BITFVAL(AUDMUX_PTCR_TCLKDIR, AUDMUX_PTCR_TCLKDIR_INPUT) |
        CSP_BITFVAL(AUDMUX_PTCR_SYN, AUDMUX_PTCR_SYN_SYNC));

    OUTREG32(pPDCR,
        CSP_BITFVAL(AUDMUX_PDCR_RXDSEL, intPort)                    |
        CSP_BITFVAL(AUDMUX_PDCR_TXRXEN, AUDMUX_PDCR_TXRXEN_NO_SWAP) |
        CSP_BITFVAL(AUDMUX_PDCR_MODE,   AUDMUX_PDCR_MODE_NORMAL));

    BSPAudioIomuxConfig();

    // Disable AUDMUX clock
    BSPAudmuxEnableClock(FALSE);

    return;
}


//--------------------------------------------------------------------------
//
//  Name: InitDMAChannel
//
//  Description: Intialize all the things about a DMA channel
//
//  Parameters: UCHAR ucDMAChannel : Index of the DMA channel to init
//              ULONG ulPhysDMAAddr : Physical address of the DMA buffer
//                                    in memory
//              USHORT usBufferSize : Number of bytes in a DMA buffer
//
//  Returns: none
//
//  Note:
//
//--------------------------------------------------------------------------
BOOL HardwareContext::InitOutputDMA()
{
    UINT16 sampleSize = g_BitsPerSample;
    UINT16 dmaBufferSize;
    DDK_DMA_ACCESS dataWidth;

    // Allocate a block of virtual memory (physically contiguous) for the
    // DMA buffers. Note that a platform-specific API call is used here to
    // allow for the DMA buffers to be allocated from different memory regions
    // (e.g., internal vs. external memory).
    dmaBufferSize = BSPAllocOutputDMABuffer(&m_DMAOutBufVirtAddr, &m_DMAOutBufPhysAddr);

    if (m_DMAOutBufVirtAddr == NULL)
    {
        ERRMSG("InitOutputDMA: DMA buffer page allocation failed");
        return FALSE;
    }

    m_OutputDMAPageSize = dmaBufferSize / AUDIO_DMA_NUMBER_PAGES;

    // Open DMA channel
    m_OutputDMAChan = DDKSdmaOpenChan(DDK_DMA_REQ_SSI2_TX0, 
                                      BSPAudioGetSdmaPriority());
    if (!m_OutputDMAChan) 
    {
        ERRMSG("InitOutputDMA: DDKSdmaOpenChan failed");
        return FALSE;
    }

    // Allocate a DMA chain for the virtual channel
    if (!DDKSdmaAllocChain(m_OutputDMAChan, AUDIO_DMA_NUMBER_PAGES))
    {
        ERRMSG("InitOutputDMA: DDKSdmaAllocChain failed");
        return FALSE;
    }

    // Configure the SDMA buffer descriptors. We currently use 2 contiguous DMA
    // pages as a circular queue.
    //
    // If the audio data stream is not big enough to fill a DMA buffer (i.e., we
    // are performing the last transfer), then we will resize the data transfer
    // at that time.
    //
    switch (sampleSize)
    {
    case 8:
        dataWidth = DDK_DMA_ACCESS_8BIT;
        break;

    case 16:
        dataWidth = DDK_DMA_ACCESS_16BIT;
        break;

    case 24:
        dataWidth = DDK_DMA_ACCESS_24BIT;
        break;

    case 32:
        dataWidth = DDK_DMA_ACCESS_32BIT;
        break;

    default:
        ERRMSG("InitOutputDMA: Invalid sample size");
        return FALSE;
    }
    
    if (!DDKSdmaSetBufDesc(m_OutputDMAChan,
                           BUFFER_A,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT,
                           m_DMAOutBufPhysAddr.LowPart,
                           0,
                           dataWidth,
                           m_OutputDMAPageSize))
    {
        ERRMSG("InitOutputDMA: BUFFER_A DDKSdmaSetBufDesc failed");
        return FALSE;
    }

    if (!DDKSdmaSetBufDesc(m_OutputDMAChan,
                           BUFFER_B,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT | DDK_DMA_FLAGS_WRAP,
                           m_DMAOutBufPhysAddr.LowPart + m_OutputDMAPageSize,
                           0,
                           dataWidth,
                           m_OutputDMAPageSize))
    {
        ERRMSG("InitOutputDMA: BUFFER_B DDKSdmaSetBufDesc failed");
        return FALSE;
    }

    // If no record device, request systintr here, otherwise 
    // it should be done in InitInputDMA(), combining both 
    // output and input DMA channel IRQs to request one systintr.
    if (m_NumInputDevices == 0)
    {
        // Request sysintr
        UINT32 dmaIrq = GetSdmaChannelIRQ(m_OutputDMAChan);
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
                             &dmaIrq,
                             sizeof(UINT32),
                             &m_AudioSysintr,
                             sizeof(DWORD),
                             NULL))
        {
            ERRMSG("InitOutputDMA: Sysintr request failed");
            return FALSE;
        }
    }
    
    return TRUE;
}

//--------------------------------------------------------------------------
//
//  Name: InitDMAChannel
//
//  Description: Intialize all the things about a DMA channel
//
//  Parameters: UCHAR ucDMAChannel : Index of the DMA channel to init
//              ULONG ulPhysDMAAddr : Physical address of the DMA buffer
//                                    in memory
//              USHORT usBufferSize : Number of bytes in a DMA buffer
//
//  Returns: none
//
//  Note:
//
//--------------------------------------------------------------------------
BOOL HardwareContext::InitInputDMA()
{
    // Stub out if no record device
    if (m_NumInputDevices == 0)
    {
        return TRUE;
    }
    
    UINT16 sampleSize = g_BitsPerSample;
    UINT16 dmaBufferSize;
    DDK_DMA_ACCESS dataWidth;
    
    // Allocate a block of virtual memory (physically contiguous) for the
    // DMA buffers. Note that a platform-specific API call is used here to
    // allow for the DMA buffers to be allocated from different memory regions
    // (e.g., internal vs. external memory).
    dmaBufferSize = BSPAllocInputDMABuffer(&m_DMAInBufVirtAddr, &m_DMAInBufPhysAddr);

    if (m_DMAInBufVirtAddr == NULL)
    {
        ERRMSG("InitInputDMA: DMA buffer page allocation failed");
        return FALSE;
    }

    m_InputDMAPageSize = dmaBufferSize / AUDIO_DMA_NUMBER_PAGES;
    
    // Open DMA channel
    m_InputDMAChan = DDKSdmaOpenChan(DDK_DMA_REQ_SSI2_RX0, 
                                     BSPAudioGetSdmaPriority());
    if (!m_InputDMAChan) 
    {
        ERRMSG("InitInputDMA: DDKSdmaOpenChan failed");
        return FALSE;
    }

    // Allocate a DMA chain for the virtual channel
    if (!DDKSdmaAllocChain(m_InputDMAChan, AUDIO_DMA_NUMBER_PAGES))
    {
        ERRMSG("InitInputDMA: DDKSdmaAllocChain failed");
        return FALSE;
    }

    // Configure the SDMA buffer descriptors. We currently use 2 contiguous DMA
    // pages as a circular queue.
    //
    // If the audio data stream is not big enough to fill a DMA buffer (i.e., we
    // are performing the last transfer), then we will resize the data transfer
    // at that time.
    //
    switch (sampleSize)
    {
    case 8:
        dataWidth = DDK_DMA_ACCESS_8BIT;
        break;

    case 16:
        dataWidth = DDK_DMA_ACCESS_16BIT;
        break;

    case 24:
        dataWidth = DDK_DMA_ACCESS_24BIT;
        break;

    case 32:
        dataWidth = DDK_DMA_ACCESS_32BIT;
        break;

    default:
        ERRMSG("InitInputDMA: Invalid sample size");
        return FALSE;
    }
    
    if (!DDKSdmaSetBufDesc(m_InputDMAChan,
                           BUFFER_A,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT,
                           m_DMAInBufPhysAddr.LowPart,
                           0,
                           dataWidth,
                           m_InputDMAPageSize))
    {
        ERRMSG("InitInputDMA: BUFFER_A DDKSdmaSetBufDesc failed");
        return FALSE;
    }

    if (!DDKSdmaSetBufDesc(m_InputDMAChan,
                           BUFFER_B,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT | DDK_DMA_FLAGS_WRAP,
                           m_DMAInBufPhysAddr.LowPart + m_InputDMAPageSize,
                           0,
                           dataWidth,
                           m_InputDMAPageSize))
    {
        ERRMSG("InitInputDMA: BUFFER_B DDKSdmaSetBufDesc failed");
        return FALSE;
    }

    // Combine input and output DMA channel IRQs together 
    // to request one systintr, so we can handle interrupts 
    // from both channels in one thread.
    UINT32 aIrqs[4];
    aIrqs[0] = (UINT32) -1;
    aIrqs[1] = 0;
    aIrqs[2] = GetSdmaChannelIRQ(m_InputDMAChan);
    aIrqs[3] = GetSdmaChannelIRQ(m_OutputDMAChan);
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
                         aIrqs,
                         sizeof(aIrqs),
                         &m_AudioSysintr,
                         sizeof(m_AudioSysintr),
                         NULL))
    {
        ERRMSG("InitInputDMA: Sysintr request failed");
        return FALSE;
    }

    return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:               InitInterruptThread()

Description:    Initializes the IST for handling DMA interrupts.

Returns:                Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitInterruptThread()
{
    m_hAudioIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hAudioIntrEvent)
    {
        return FALSE;
    }

    if (!InterruptInitialize(m_AudioSysintr, m_hAudioIntrEvent, NULL, 0))
    {
        return FALSE;
    }

    m_hAudioIntrThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                       0,
                                       (LPTHREAD_START_ROUTINE)CallInterruptThread,
                                       this,
                                       0,
                                       NULL);
    if (!m_hAudioIntrThread)
    {
        return FALSE;
    }

    // Bump up the priority since the interrupt must be serviced immediately.
    CeSetThreadPriority(m_hAudioIntrThread, GetInterruptThreadPriority());

    return TRUE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       InitHPDetectInterruptThread()

Description:    Initializes the IST for handling Headphone detect
                interrupt (via GPIO)

Returns:        Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitHPDetectInterruptThread()
{
    // Request sysintr
    UINT32 HPDetectIrq = BSPGetHPDetectIRQ();
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
                         &HPDetectIrq,
                         sizeof(UINT32),
                         &m_AudioHPDetectSysIntr,
                         sizeof(DWORD),
                         NULL))
    {
        ERRMSG("InitHPDetectInterruptThread: Sysintr request failed");
        return FALSE;
    } 

    // Create a synchronization event object
    m_hAudioHPDetectIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hAudioHPDetectIntrEvent)
    {
        return FALSE;
    }

    // Associate the event to the system interrupt
    if (!InterruptInitialize(m_AudioHPDetectSysIntr, m_hAudioHPDetectIntrEvent, NULL, 0))
    {
        return FALSE;
    }

    // Create a separate thread which will be signalled on HP detect/remove event
    m_hAudioHPDetectIntrThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                       0,
                                       (LPTHREAD_START_ROUTINE)CallHPDetectInterruptThread,
                                       this,
                                       0,
                                       NULL);
    if (!m_hAudioHPDetectIntrThread)
    {
        return FALSE;
    }

    return TRUE;
}


BOOL HardwareContext::InitDisableDelayThread()
{
    m_DACDelayDisableTimerID = NULL;
    m_ADCDelayDisableTimerID = NULL;
    m_hAudioDelayedDisableThread = NULL;

    // Create delayed disable event and thread
    if ((PlaybackDisableDelayMsec > 0) || (RecordDisableDelayMsec > 0))
    {
        m_hDACDelayDisableEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_hADCDelayDisableEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_hStopOutputDMAEvent    = CreateEvent(NULL, FALSE, FALSE, NULL);

        if ((m_hDACDelayDisableEvent == NULL) || 
            (m_hADCDelayDisableEvent == NULL) || 
            (m_hStopOutputDMAEvent == NULL))
        {
            ERRMSG("Unable to create delay events");
            return FALSE;
        }

        h_AudioDelayEvents[0] = m_hStopOutputDMAEvent;
        h_AudioDelayEvents[1] = m_hDACDelayDisableEvent;
        h_AudioDelayEvents[2] = m_hADCDelayDisableEvent;

        m_hAudioDelayedDisableThread =
            CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                         0,
                         (LPTHREAD_START_ROUTINE)CallDisableDelayThread,
                         this,
                         0,
                         NULL);

        if (!m_hAudioDelayedDisableThread)
        {
            ERRMSG("Unable to create audio CODEC delayed disable timer event handling thread");
            return FALSE;
        }
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStartCodecOutput
//
//  This function configures the audio chip to start audio output.
//
//  Parameters:
//      bus
//          [in] Specifies the audio bus to be used for audio output.
//
//      path
//          [in] Specifies the audio path to be used for audio output.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::StartCodec(AUDIO_BUS audioBus)
{
    if (audioBus == AUDIO_BUS_DAC)
    {
        BSPAudioStartDAC();
    }
    else // (audioBus == AUDIO_BUS_ADC)
    {
        BSPAudioStartADC();
    }
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStopCodecOutput
//
//  This function configures the audio chip to stop audio output.
//
//  Parameters:
//      bus
//          [in] Specifies the audio bus being used for audio output.
//
//      path
//          [in] Specifies the audio path being used for audio output.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopCodec(AUDIO_BUS audioBus)
{
    UINT32 uDelayMsec;
    HANDLE *phDelayEvent;
    MMRESULT *phDelayTimerID;

    if (audioBus == AUDIO_BUS_DAC)
    {
        uDelayMsec     = PlaybackDisableDelayMsec;
        phDelayEvent   = &m_hDACDelayDisableEvent;
        phDelayTimerID = &m_DACDelayDisableTimerID;
    }
    else // (audioBus == AUDIO_BUS_ADC)
    {
        uDelayMsec     = RecordDisableDelayMsec;
        phDelayEvent   = &m_hADCDelayDisableEvent;
        phDelayTimerID = &m_ADCDelayDisableTimerID;
    }

    // Note that we want to immediately disable the Codec
    // if we're in the process of powering down.
    if ((uDelayMsec > 0) &&
        (m_hAudioDelayedDisableThread != NULL) &&
        (*phDelayEvent != NULL) &&
        !m_delayBypass)
    {
        if (*phDelayTimerID != NULL)
        {
            // Cancel previous timer so that we can restart 
            // it later from the beginning.
            timeKillEvent(*phDelayTimerID);
            *phDelayTimerID = NULL;
        }

        // Start a timed delay. If the timer expires without us
        // starting another audio playback or record operation, 
        // then we should really disable the audio CODEC hardware.
        *phDelayTimerID = timeSetEvent(uDelayMsec,
                                      1,
                                      (LPTIMECALLBACK)*phDelayEvent,
                                      NULL,
                                      TIME_ONESHOT | TIME_CALLBACK_EVENT_SET);
    }
    else
    {
        // We want to immediately disable the Codec
        if (audioBus == AUDIO_BUS_DAC)
        {
            BSPAudioStopDAC();
        }
        else // (audioBus == AUDIO_BUS_ADC)
        {
            BSPAudioStopADC();
        }
    }
}

//-----------------------------------------------------------------------------
//
//  Function: StartSsiOutput
//
//  This function configures SSI to start audio output.
//
//  Parameters:
//      pSSI 
//          [in] Points to SSI to be configured.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::StartSsi(AUDIO_BUS audioBus)
{
    // Enable SSI clocks before we access the SSI registers.
    BSPSsiEnableClock(2, TRUE);
    BSPAudmuxEnableClock(TRUE);

    // As per audio bus type
    switch (audioBus)
    {
    case AUDIO_BUS_DAC:
        // Enable the SSI and the transmit FIFO0 so that we can 
        // fill it up before we enable the transmitter later.
        SETREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_SSIEN));
        SETREG32(&m_pSSI->STCR, CSP_BITFMASK(SSI_STCR_TFEN0));

        // Prefill the transmit FIFO with silence so that there is something 
        // ready to go when we enable the transmitter in the next step.
        for (unsigned i = 0; i < SSI_FIFO_DEPTH; i++)
        {
            OUTREG32(&m_pSSI->STX0, 0);
        }

        // Enable only the first two transmit timeslots 
        // (for the left and right audio channels).
        OUTREG32(&m_pSSI->STMSK, ~0x3);

        // The SSI has already been properly configured, we just need to 
        // enable the SSI transmitter DMA request and the transmitter to 
        // begin the audio output/playback process.
        SETREG32(&m_pSSI->SIER, CSP_BITFMASK(SSI_SIER_TDMAE));
        // ENGcm09220
        while((EXTREG32BF(&m_pSSI->SCR, SSI_SCR_RE) == 1) &&
              (EXTREG32BF(&m_pSSI->SISR, SSI_SISR_RFS) == SSI_SISR_RFS_FS_NOT_OCCUR));
        // End of ENGcm09220
        
        SETREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_TE));

        break;

    case AUDIO_BUS_ADC:
        // Start off by enabling the SSI. This must be done first.
        SETREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_SSIEN));

        // Next, enable the SSI receiver DMA request and RX FIFO0.
        SETREG32(&m_pSSI->SIER, CSP_BITFMASK(SSI_SIER_RFF0_EN));
        SETREG32(&m_pSSI->SRCR, CSP_BITFMASK(SSI_SRCR_RFEN0));

        // Unmask only the first two receive timeslots (for the primary and
        // secondary audio recording channels).
        OUTREG32(&m_pSSI->SRMSK, ~0x3);

        // The SSI has already been properly configured, we just need to enable
        // the SSI receiver DMA request and the receiver to begin the audio
        // input/recording process.
        SETREG32(&m_pSSI->SIER, CSP_BITFMASK(SSI_SIER_RDMAE));
        SETREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_RE));

        break;
    }
}

//-----------------------------------------------------------------------------
//
//  Function: StopSsiOutput
//
//  This function configures SSI to stop audio output.
//
//  Parameters:
//      pSSI 
//          [in] Points to SSI to be configured.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopSsi(AUDIO_BUS audioBus)
{
    // As per audio bus type
    switch (audioBus)
    {
    case AUDIO_BUS_DAC:
        // ENGcm09222
        while((EXTREG32BF(&m_pSSI->SCR, SSI_SCR_TE) == 1) &&
              (EXTREG32BF(&m_pSSI->SISR, SSI_SISR_TFS) == SSI_SISR_TFS_FS_NOT_OCCUR));
        // End of ENGcm09222
        
        // ENGcm09220
        while((EXTREG32BF(&m_pSSI->SCR, SSI_SCR_RE) == 1) &&
              (EXTREG32BF(&m_pSSI->SISR, SSI_SISR_RFS) == SSI_SISR_RFS_FS_NOT_OCCUR));
        // End of ENGcm09220
        
        // Start by disabling the SSI transmitter.
        CLRREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_TE));
        
        // Disable the SSI transmit DMA request.
        CLRREG32(&m_pSSI->SIER, CSP_BITFMASK(SSI_SIER_TDMAE));

        // Disable the transmit FIFO0 and mask all transmit timeslots.
        CLRREG32(&m_pSSI->STCR, CSP_BITFMASK(SSI_STCR_TFEN0));
        OUTREG32(&m_pSSI->STMSK, 0xffffffff);

        break;

    case AUDIO_BUS_ADC:    
        // Start by disabling the SSI receiver.
        CLRREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_RE));
        
        // Disable the SSI receive DMA request
        CLRREG32(&m_pSSI->SIER, CSP_BITFMASK(SSI_SIER_RDMAE));

        // Disable the receive FIFO0 and mask all receive timeslots.
        CLRREG32(&m_pSSI->SRCR, CSP_BITFMASK(SSI_SRCR_RFEN0));
        OUTREG32(&m_pSSI->SRMSK, 0xffffffff);

        break;
    }
    
    // Turn off the entire SSI only if both trasmitter and 
    // receiver are disabled. 
    if ((EXTREG32BF(&m_pSSI->SCR, SSI_SCR_RE) == 0) && 
        (EXTREG32BF(&m_pSSI->SCR, SSI_SCR_TE) == 0))
    {
        // Completely disable the SSI.
        CLRREG32(&m_pSSI->SCR, CSP_BITFMASK(SSI_SCR_SSIEN));

        // Disable SSI clocks to minimize power consumption.
        BSPAudmuxEnableClock(FALSE);
        BSPSsiEnableClock(2, FALSE);
    }
}

//-----------------------------------------------------------------------------
//
//  Function: StartOutputDMA
//
//  This function starts outputting the sound data to the audio codec
//  chip via the DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::StartOutputDMA()
{
    ULONG nTransferred = 0;

    // Can not start output if loopback is working
    if (m_bCodecLoopbackState)
    {
        return FALSE;
    }
    if (!m_PowerUpState)
    {
        return FALSE;
    }
    
    // Must acquire critical section Lock() here so that we don't have
    // a race condition with StopOutputDMA() and the interrupt handler.
    Lock();

    // Terminate the existing DAC disable timer since we're about
    // to start another playback operation.
    if (m_DACDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_DACDelayDisableTimerID);
        m_DACDelayDisableTimerID = NULL;
    }

    // Reinitialize audio output if it is not already active.
    if (!m_OutputDMARunning || m_bSSITXUnderrunError)
    {
        // Initialize output DMA state variables. Note that if we get here
        // because m_bSSITXUnderrunError is still TRUE, then we must have
        // ran out of additional audio data in the interrupt thread.
        // Therefore, it is correct here to reinitialize everything just as
        // if the DMA was not running.
        m_OutputDMAStatus  = DMA_DONEA | DMA_DONEB;
        m_OutputDMARunning = TRUE;

        // Prime the output DMA buffers.
        nTransferred = TransferOutputBuffers();

        // Check if any of the DMA buffers have been filled with new audio
        // data. Do not enable the DMA controller if there is no data in
        // any of the DMA buffers (this is normally an unusual condition
        // but it may occur so let's handle it properly).
        if (nTransferred > 0)
        {
            // Allocate a set of DMA buffer descriptors and reset the
            // state of the output DMA transfer list. This call must be
            // matched by a later call to DDKSdmaStopChan(m_OutputDMAChan,
            // FALSE) in StopOutputDMA() to free up all of the allocated
            // resources.
            //
            // Note that the second "watermark" argument to the function
            // DDKSdmaInitChain() is actually in bytes whereas the variable
            // "m_OutputDMALevel" is given in SSI FIFO slots or words.
            // Therefore, we have to multiply by the audio data word size
            // to ensure that we are properly refilling the SSI TX FIFO.
            //
            if(!m_bSSITXUnderrunError &&
               !DDKSdmaInitChain(m_OutputDMAChan, 
                                 SSI_SFCSR_TX_WATERMARK * ((g_BitsPerSample==16)?sizeof(HWSAMPLE):3)))
            {
                ERRMSG("StartOutputDMA: Unable to initialize output DMA channel\r\n");

                m_OutputDMARunning  = FALSE;
                m_bCanStopOutputDMA = TRUE;
                
                return FALSE;
            }

            // Start the output DMA channel. This must be done before we 
            // start SSI and Codec because we have to make sure that audio
            // data is already available before that.
            DDKSdmaStartChan(m_OutputDMAChan);
        }
        else
        {
            m_OutputDMARunning = FALSE;
            m_bCanStopOutputDMA = TRUE;
            goto EXIT;
         }

        // Check to see if we need to complete the handling of the last
        // SSI TX underrun error which was flagged in interrupt thread.
        //
        // This takes care of the unusual situation whereby the last
        // playback operation ended with an SSI TX underrun error but
        // we are now about to resume playback without having called
        // StopOutputDMA() in between.
        if (m_bSSITXUnderrunError)
        {
            // In this case, the SSI has just been disabled and it does
            // not need to be completely reinitialized.
            //
            // Reenable the SSI here if we disabled it during the last
            // interrupt thread call where we also had to handle a TX
            // underrun error and there was no more audio data available
            // to continue.
            StartSsi(AUDIO_BUS_DAC);

            // We have now completed the recovery from the most recent
            // SSI TX underrun error.
            m_bSSITXUnderrunError = FALSE;
        }
        else
        {
            // Enable the audio output hardware. This must be done after 
            // we have already activated the output DMA channel (by calling
            // DDKSdmaStartChan()). Otherwise, there is a race condition 
            // between how fast the initial transmit FIFO will be emptied 
            // and whether the DMA controller will be ready yet to handle 
            // the first interrupt from the FIFO.
            //
            // Turn on the codec and amps first. This must be done before 
            // starting SSI output because there are required delays involved
            // in enabling some of the Codec components. These required delays
            // will cause an SSI FIFO underrun error if the SSI transmitter 
            // is already running before the Codec components have been enabled.
            StartCodec(AUDIO_BUS_DAC);

            // Then start the SSI transmitter
            StartSsi(AUDIO_BUS_DAC);
        }
    }
    else
    {
        // Try to transfer in case we've got a buffer spare and waiting
        nTransferred = TransferOutputBuffers();
    }

    // This will be set to TRUE only by the audio driver IST at the
    // end of the playback operation.
    m_bCanStopOutputDMA = FALSE;

    EXIT:
    Unlock();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: StopOutputDMA
//
//  This function stops any DMA activity on the output channel.
//
//  This function can only be called after Lock() has already been called. A
//  matching Unlock() call must also be made after this function has finished.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopOutputDMA()
{
    Lock();
    
    // If the output DMA is running, stop it
    if (m_OutputDMARunning)
    {
        // Reset output DMA state variables
        m_OutputDMAStatus  = DMA_CLEAR;
        m_OutputDMARunning = FALSE;

        // Also reset the SSI TX underrun error flag here since we are about
        // to shutdown the current audio setup. The audio hardware will be
        // fully reinitialized when StartOutputDMA() is next called so we no
        // longer care if an SSI TX underrun error was previously flagged for
        // the current audio playback operation.
        m_bSSITXUnderrunError = FALSE;

        // Disable audio output hardware
        StopCodec(AUDIO_BUS_DAC);
        StopSsi(AUDIO_BUS_DAC);

        // Kill the output DMA channel
        DDKSdmaStopChan(m_OutputDMAChan, FALSE);
    }

    Unlock();
}

//-----------------------------------------------------------------------------
//
//  Function:  StartInputDMA
//
//  This function starts inputting the sound data from the audio codec
//  chip via the DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::StartInputDMA()
{
    // Can not start output if loopback is working
    if (m_bCodecLoopbackState)
    {
        RETAILMSG(TRUE, (L"StartInputDMA: Recording can not work "
                         L"when loopback is working\r\n"));
        return FALSE;
    }
    
    if (!m_PowerUpState)
    {
        return FALSE;
    }

    // Must acquire critical section Lock() here so that we don't have a race
    // condition with StopInputDMA() and the interrupt handler.
    Lock();

    // Terminate the existing ADC disable timer since we're about
    // to start another recording operation.
    if (m_ADCDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_ADCDelayDisableTimerID);
        m_ADCDelayDisableTimerID = NULL;
    }

    // Reinitialize audio input if it is not already active
    if(!m_InputDMARunning || m_bSSIRXOverrunError)
    {
        // Initialize the input DMA state variables
        m_InputDMARunning = TRUE;
        m_InputDMAStatus  = (DWORD)~(DMA_DONEA | DMA_DONEB);

        // Reset the state of the input DMA chain.
        //
        // Note that the second "watermark" argument to DDKSdmaInitChain()
        // is actually in bytes whereas "m_InputDMALevel" is given in
        // SSI FIFO slots or words. Therefore, we have to multiply by the
        // audio data word size to ensure that we are properly emptying
        // the SSI RX FIFO.
        //
        if(!m_bSSIRXOverrunError && !DDKSdmaInitChain(m_InputDMAChan,
                             SSI_SFCSR_RX_WATERMARK * ((g_BitsPerSample==16)?sizeof(HWSAMPLE):3)))
        {
            ERRMSG("StartInputDMA: Unable to initialize input DMA channel!\r\n");

            // Must reset the m_InputDMARunning state variable to FALSE
            // if we encounter an error here.
            m_InputDMARunning = FALSE;

            return FALSE;
        }

        // Start the input DMA
        DDKSdmaStartChan(m_InputDMAChan);

        if (m_bSSIRXOverrunError)
        {
            // In this case, the SSI has just been disabled and it does
            // not need to be completely reinitialized.
            //
            // Reenable the SSI here if we disabled it during the last
            // interrupt thread call where we also had to handle a RX
            // overrun error and there was no more audio data available
            // to continue.
            StartSsi(AUDIO_BUS_ADC);

            // We have now completed the recovery from the most recent
            // SSI RX overrun error.
            m_bSSIRXOverrunError = FALSE;
        }else{

            // Enable audio input hardware. This must be done after we activate the
            // input DMA channel because the audio hardware will immediately start
            // recording when this call is made.
            //
            StartSsi(AUDIO_BUS_ADC);
            StartCodec(AUDIO_BUS_ADC);
        }    
    }

    Unlock();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  StopInputDMA
//
//  This function stops any DMA activity on the input channel.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopInputDMA()
{
    Lock();
    
    // If the input DMA is running, stop it
    if (m_InputDMARunning)
    {
        // Reset input DMA state variables
        m_InputDMAStatus  = DMA_CLEAR;
        m_InputDMARunning = FALSE;
        m_bSSIRXOverrunError = FALSE;

        // Disable audio input hardware
        StopCodec(AUDIO_BUS_ADC);
        StopSsi(AUDIO_BUS_ADC);

        // Kill the input DMA channel
        DDKSdmaStopChan(m_InputDMAChan, FALSE);
    }

    Unlock();
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:               TransferOutputBuffer()

Description:    Retrieves the next "mixed" audio buffer of data to
                                DMA into the output channel.

Returns:                Number of bytes needing to be transferred.
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferOutputBuffers()
{
    ULONG BytesTransferred_A = 0;
    ULONG BytesTransferred_B = 0;

    if (m_OutputDMAStatus & DMA_DONEA)
    {
        // DMA buffer A is free, try to refill it.
        BytesTransferred_A = TransferBuffer(AUDIO_BUS_DAC, BUFFER_A);

        // Only mark the DMA buffer as being in use if some data was actually
        // transferred.
        if (BytesTransferred_A > 0)
        {
            m_OutputDMAStatus &= ~DMA_DONEA;
        }
    }

    if (m_OutputDMAStatus & DMA_DONEB)
    {
        // DMA buffer B is free, try to refill it.
        //
        // Note that we can refill both DMA buffers at once if both DMA
        // buffers are free and there is sufficient data.
        //
        BytesTransferred_B = TransferBuffer(AUDIO_BUS_DAC, BUFFER_B);

        if (BytesTransferred_B > 0)
        {
            m_OutputDMAStatus &= ~DMA_DONEB;
        }
    }

    return BytesTransferred_A + BytesTransferred_B;
}

ULONG HardwareContext::TransferInputBuffers(UINT8 checkFirst)
{
    ULONG BytesTransferred = 0;

    // We will mark the input DMA buffer as being in use again regardless
    // of whether or not the data was completely copied out to an
    // application-supplied data buffer. The input DMA buffer must be
    // immediately reused because the audio recording operation is still
    // running.
    //
    if (checkFirst == BUFFER_A)
    {
        if (m_InputDMAStatus & DMA_DONEA)
        {
            BytesTransferred = TransferBuffer(AUDIO_BUS_ADC, BUFFER_A);
            m_InputDMAStatus &= ~DMA_DONEA;
        }

        if (m_InputDMAStatus & DMA_DONEB)
        {
            BytesTransferred += TransferBuffer(AUDIO_BUS_ADC, BUFFER_B);
            m_InputDMAStatus &= ~DMA_DONEB;
        }
    }
    else // (checkFirst == IN_BUFFER_B)
    {
        if (m_InputDMAStatus & DMA_DONEB)
        {
            BytesTransferred = TransferBuffer(AUDIO_BUS_ADC, BUFFER_B);
            m_InputDMAStatus &= ~DMA_DONEB;
        }

        if (m_InputDMAStatus & DMA_DONEA)
        {
            BytesTransferred += TransferBuffer(AUDIO_BUS_ADC, BUFFER_A);
            m_InputDMAStatus &= ~DMA_DONEA;
        }
    }

    return BytesTransferred;
}

ULONG HardwareContext::TransferBuffer(AUDIO_BUS audioBus, UINT8 NumBuf)
{
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart;
    PBYTE pBufferEnd;
    PBYTE pBufferLast;

    if (audioBus == AUDIO_BUS_DAC)
    {
        pBufferStart = m_DMAOutPageVirtAddr[NumBuf];
        pBufferEnd = pBufferStart + m_OutputDMAPageSize;

        TRANSFER_STATUS TransferStatus = {0};
        pBufferLast = m_OutputDeviceContext.TransferBuffer(pBufferStart, 
                                                           pBufferEnd, 
                                                           &TransferStatus);
        
        BytesTransferred = pBufferLast - pBufferStart;
        if(pBufferLast != pBufferEnd)
        {
            // Enable if you need to clear the rest of the DMA buffer
            //
            // Probably want to do something better than clearing out remaining
            // buffer samples. DC output by replicating last sample or
            // some type of fade out would be better.
            StreamContext::ClearBuffer(pBufferLast, pBufferEnd);
        }
    }
    else // (audioBus == AUDIO_BUS_ADC)
    {
        pBufferStart = m_DMAInPageVirtAddr[NumBuf];
        pBufferEnd = pBufferStart + m_InputDMAPageSize;
        
        pBufferLast = m_InputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd, NULL);
        
        BytesTransferred = pBufferLast - pBufferStart;
    }

    return BytesTransferred;
}


void HardwareContext::InterruptThread()
{
    // Loop repeatedly to handle each audio interrupt event until we
    // deinitialize the audio driver.
    while (m_Initialized)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(m_hAudioIntrEvent, INFINITE);

        // Immediately terminate this thread if Deinit() has been called to
        // unload the audio driver. Otherwise, handle the audio interrupt
        // event.
        if (!m_Initialized)
        {
            break;
        }

        // Grab the audio driver lock to avoid race conditions with the
        // StartOutputDMA() and StopOutputDMA() functions.
        Lock();

        // Handle DMA output (i.e., audio playback) if it is active.
        if (m_OutputDMARunning)
        {
            OutputIntrRoutine();
        }

        // Handle DMA input (i.e., audio recording) if it is active.
        if (m_InputDMARunning)
        {
            InputIntrRoutine();
        }

        // No need to call InterruptDone since we are using SDMA interrupts
        Unlock();
    }

    // Close the event handle
    if (m_hAudioIntrEvent != NULL)
    {
        CloseHandle(m_hAudioIntrEvent);
        m_hAudioIntrEvent = NULL;
    }
}

void HardwareContext::HPDetectInterruptThread()
{
    // While the audio driver is initialized, wait for the HP detect
    // event to occur
    while (m_Initialized)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(m_hAudioHPDetectIntrEvent, INFINITE);

        // Immediately terminate this thread if Deinit() has been called to
        // unload the audio driver. Otherwise, handle the audio interrupt
        // event.
        if (!m_Initialized)
        {
            // Signal interrupt done just in case an interrupt actually
            // happened in the mean time
            InterruptDone(m_AudioHPDetectSysIntr);
            break;
        }

        // Resolve the HP detect event.  The HP may be plugged/unplugged.
        // This needs to be resolved and appropriate action needs to be taken.
        BSPAudioHandleHPDetectIRQ();

        // Signal the kernel that interrupt processing is done and re-enable
        // for next interrupt
        InterruptDone(m_AudioHPDetectSysIntr);
    }

    // Close the event handle
    if (m_hAudioHPDetectIntrEvent != NULL)
    {
        CloseHandle(m_hAudioHPDetectIntrEvent);
        m_hAudioHPDetectIntrEvent = NULL;
    }
}

void HardwareContext::OutputIntrRoutine()
{
    UINT32 bufDescStatus[AUDIO_DMA_NUMBER_PAGES];
    UINT32 nTransferred;

    // Check for SSI transmitter underrun errors. These are
    // bad and can result in unpredictable swapping of the
    // left/right audio channels if we do not recover from
    // it properly.
    if (EXTREG32BF(&m_pSSI->SISR, SSI_SISR_TUE0))
    {
        DEBUGMSG(ZONE_ERROR, (_T("SSI TX underrun\r\n")));

        // Flag the SSI TX underrun error so that we can recover
        // from it later.
        INSREG32BF(&m_pSSI->SISR, SSI_SISR_TUE0, 1);
        //m_bSSITXUnderrunError = TRUE;
    }

    // Get the transfer status of the DMA output buffers.
    if (!DDKSdmaGetChainStatus(m_OutputDMAChan, bufDescStatus))
    {
        ERRMSG("Could not retrieve output buffer status");
    }

    if (bufDescStatus[BUFFER_A] & DDK_DMA_FLAGS_ERROR)
    {
        ERRMSG("Error flag set in output BUFFER_A descriptor");
    }

    if (bufDescStatus[BUFFER_B] & DDK_DMA_FLAGS_ERROR)
    {
        ERRMSG("Error flag set in output BUFFER_B descriptor");
    }

    // Set DMA status bits according to retrieved transfer status
    if (!(bufDescStatus[BUFFER_A] & DDK_DMA_FLAGS_BUSY))
    {
        // The data in DMA buffer A has been transmitted and the
        // buffer may now be reused.
        m_OutputDMAStatus |= DMA_DONEA;
        DDKSdmaClearBufDescStatus(m_OutputDMAChan, BUFFER_A);
    }

    if (!(bufDescStatus[BUFFER_B] & DDK_DMA_FLAGS_BUSY))
    {
        // The data in DMA buffer B has been transmitted and the
        // buffer may now be reused.
        m_OutputDMAStatus |= DMA_DONEB;
        DDKSdmaClearBufDescStatus(m_OutputDMAChan, BUFFER_B);
    }

    // Check to see if an SSI TX underrun error has just occurred
    // and, if necessary, recover from it.
    if (m_bSSITXUnderrunError)
    {
        // Start the SSI TX underrun error recovery by disabling
        // the SSI. This will also flush out any stray data that
        // may still be in the TX FIFO. Note that this is possible
        // because an earlier DMA operation may have refilled the
        // TX FIFO (we only get interrupts here when a DMA buffer
        // has been emptied).
        //
        StopSsi(AUDIO_BUS_DAC);

        // Next stop all output DMA channel activity and flush out
        // both DMA buffers.
        DDKSdmaStopChan(m_OutputDMAChan, TRUE);
        m_OutputDMAStatus = DMA_DONEA | DMA_DONEB;
    }

    // Try to refill any empty output DMA buffers with new data.
    nTransferred = TransferOutputBuffers();

    // Don't restart the DMA unless we actually have some data
    // in at least one of the DMA buffers.
    // if (nTransferred > 0)
    if (!(m_OutputDMAStatus & DMA_DONEA) || !(m_OutputDMAStatus & DMA_DONEB))
    {
        // SDMA will disable itself on buffer underrun.
        // Force channel to be enabled since we now have
        // at least one buffer to be processed.
        DDKSdmaStartChan(m_OutputDMAChan);

        // Also reenable the SSI here if we disabled it above when
        // we began our handling of the TX underrun error.
        if (m_bSSITXUnderrunError)
        {
            StartSsi(AUDIO_BUS_DAC);

            // We have now completed the recovery from the most
            // recent SSI TX underrun error.
            m_bSSITXUnderrunError = FALSE;
        }
    }
    else
    {
        // The upper audio driver layer has no more data, so we can
        // terminate the current DMA operation because all DMA
        // buffers are empty.
        //
        // Note that we signal the DisableDelayThread() to actually 
        // make the call to StopOutputDMA() so that we can ensure that
        // this IST thread will never have it's priority changed.
        //
        // Currently the MMTIMER library can temporarily change the 
        // priority of a calling thread and StopOutputDMA() does 
        // make calls to the MMTIMER library. Therefore, we do not
        // want this IST thread making any direct MMTIMER calls.
        //
        m_bCanStopOutputDMA = TRUE;
        SetEvent(m_hStopOutputDMAEvent);
    }
}


void HardwareContext::InputIntrRoutine()
{
    static UINT8 checkFirst = BUFFER_A;
    UINT32 bufDescStatus[AUDIO_DMA_NUMBER_PAGES];
    UINT32 nTransferred;

    // Indicate SSI RX overrun
    if (m_InputDeviceContext.m_dwStreamCount < 16){ 
        //we don't deal with it if too many input streams opened
        if (EXTREG32BF(&m_pSSI->SISR, SSI_SISR_ROE0))
        {
            DEBUGMSG(ZONE_ERROR, (_T("SSI RX overrun\r\n")));

            // We do not handle RX overrun error and instead 
            // just simply clear the error bit.
            INSREG32BF(&m_pSSI->SISR, SSI_SISR_ROE0, 1);
            m_bSSIRXOverrunError = TRUE;
        }
    }

    // Get the transfer status of the DMA input buffers.
    if (!DDKSdmaGetChainStatus(m_InputDMAChan, bufDescStatus))
    {
        ERRMSG("Could not retrieve input buffer status");
    }

    if (bufDescStatus[BUFFER_A] & DDK_DMA_FLAGS_ERROR)
    {
        ERRMSG("Error flag set in input BUFFER_A descriptor");
    }

    if (bufDescStatus[BUFFER_B] & DDK_DMA_FLAGS_ERROR)
    {
        ERRMSG("Error flag set in input BUFFER_B");
    }

    // Set DMA status bits according to retrieved transfer status.
    if (!(bufDescStatus[BUFFER_A] & DDK_DMA_FLAGS_BUSY))
    {
        // Mark input buffer A as having new audio data.
        m_InputDMAStatus |= DMA_DONEA;
        DDKSdmaClearBufDescStatus(m_InputDMAChan, BUFFER_A);
    }

    if (!(bufDescStatus[BUFFER_B] & DDK_DMA_FLAGS_BUSY))
    {
        // Mark input buffer B as having new audio data.
        m_InputDMAStatus |= DMA_DONEB;
        DDKSdmaClearBufDescStatus(m_InputDMAChan, BUFFER_B);
    }

    if ((m_InputDMAStatus & (DMA_DONEA | DMA_DONEB)) == 0)
    {
        // If running here, we must encounter a dummy interrupt, 
        // of which the data has been transferred within the previous 
        // interrupt handling that services another  input buffer. 
        // So we want to skip the data tranfer below.
        return;
    }

    // Check to see if an SSI RX overrun error has just occurred
    // and, if necessary, recover from it.
    if (m_bSSIRXOverrunError)
    {
        StopSsi(AUDIO_BUS_ADC);

        // Next stop all input DMA channel activity and flush out
        // both DMA buffers.
        DDKSdmaStopChan(m_InputDMAChan, TRUE);
    }

    // Try to empty out the input DMA buffers by copying the data
    // up to the application-supplied data buffers.
    //
    // Note that we require the higher-level audio application
    // to provide enough data buffers to transfer all of the new
    // data from the input DMA buffers. Otherwise, we will be
    // forced to discard the data and just continue recording.
    //
    nTransferred = TransferInputBuffers(checkFirst);

    if (nTransferred > 0)
    {
        // Update the checkFirst flag to mark the first input DMA
        // buffer that should be transferred in order to maintain the
        // correct ABABABAB... input DMA buffer sequencing.
        //
        // Note that except for the 2 cases that we check for below, we
        // do not need to change the current value of the checkFirst
        // flag. Here are some pseudocode blocks to explain why:
        //
        //     if ((checkFirst == BUFFER_A) &&
        //         (m_InputDMAStatus & DMA_DONEA) &&
        //         (m_InputDMAStatus & DMA_DONEB))
        //     {
        //         // This means that both input DMA buffers A and
        //         // B were full and that the data was copied out
        //         // during the handling of this interrupt. So we
        //         // can just continue the input DMA operation in
        //         // the same state.
        //     }
        //
        //     if ((checkFirst == BUFFER_A) &&
        //         !(m_InputDMAStatus & DMA_DONEA) &&
        //         !(m_InputDMAStatus & DMA_DONEB))
        //     {
        //         // This is an invalid or error condition that
        //         // should never happen because it indicates that
        //         // we got an input DMA interrupt but neither
        //         // input buffer was full.
        //     }
        //
        //     if ((checkFirst == BUFFER_A) &&
        //         !(m_InputDMAStatus & DMA_DONEA) &&
        //         (m_InputDMAStatus & DMA_DONEB))
        //     {
        //         // This is an invalid or error condition that
        //         // should never happen because it indicates that
        //         // we've lost our expected ABABABAB... buffer
        //         // sequencing.
        //     }
        //
        //     if ((checkFirst == BUFFER_A) &&
        //         (m_InputDMAStatus & DMA_DONEA) &&
        //         !(m_InputDMAStatus & DMA_DONEB))
        //     {
        //         // This is the only scenario that we need to
        //         // check for (see the code below).
        //     }
        //
        // The same logic can be applied for the cases where the
        // checkFirst flag is equal to BUFFER_B.
        //
        if ((checkFirst == BUFFER_A) &&
            (m_InputDMAStatus & DMA_DONEA) &&
            !(m_InputDMAStatus & DMA_DONEB))
        {
            // Input DMA buffer A was full and just transferred but
            // input DMA buffer B was not yet full so we must check
            // it first when handling the next input DMA interrupt.
            checkFirst = BUFFER_B;
        }
        else if ((checkFirst == BUFFER_B) &&
                 (m_InputDMAStatus & DMA_DONEB) &&
                 !(m_InputDMAStatus & DMA_DONEA))
        {
            // Input DMA buffer B was full and just transferred but
            // input DMA buffer A was not yet full so we must check
            // it first when handling the next input DMA interrupt.
            checkFirst = BUFFER_A;
        }

        // Force the input DMA channel to be enabled again since we now
        // have at least one input DMA buffer that's ready to be used.
        DDKSdmaStartChan(m_InputDMAChan);
        
        // Also reenable the SSI here if we disabled it above when
        // we began our handling of the RX overrun error.
        if (m_bSSIRXOverrunError)
        {
            StartSsi(AUDIO_BUS_ADC);

            // We have now completed the recovery from the most
            // recent SSI RX overrun error.
            m_bSSIRXOverrunError = FALSE;
        }    
    }
    else
    {
        StopInputDMA();
    }
}


void HardwareContext::DisableDelayThread()
{
    // We want to loop endlessly here just waiting to process timer timeout
    // events that were created by calling timeSetEvent() in StopCodec().
    while (m_Initialized)
    {
        DWORD index = WaitForMultipleObjects(AUDIO_DELAY_EVENTS,
                                             h_AudioDelayEvents,
                                             FALSE,
                                             INFINITE);

        // Immediately terminate this thread if Deinit() has been called to
        // unload the audio driver. Otherwise, handle the appropriate event.
        if (!m_Initialized)
        {
            break;
        }
        else if (index == WAIT_OBJECT_0)
        {
            // The audio IST has signalled us to terminate the current
            // playback operation.
            Lock();

            // Don't call StopOutputDMA() if a new playback operation has
            // been started in the meantime.
            if (m_bCanStopOutputDMA)
            {
                StopOutputDMA();
                m_bCanStopOutputDMA = FALSE;
            }

            Unlock();
        }
        else if (index == (WAIT_OBJECT_0 + 1))
        {
            // Must first acquire the critical section to avoid race conditions
            // with StartCodec() and the interrupt handler.
            Lock();

            if (!m_OutputDMARunning && !m_bCodecLoopbackState)
            {
                // The timer has expired without another output audio stream
                // being activated so we can really disable the DAC now.
                // But if the loopback mode be enabled, can not stop the DAC.
                BSPAudioStopDAC();
            }
            else
            {
                // Another audio output operation has been started, 
                // so we should skip disabling the DAC.
                DEBUGMSG(ZONE_TEST, (_T("Skip disabling the DAC\r\n")));
            }

            m_DACDelayDisableTimerID = NULL;

            Unlock();
        }
        else if (index == (WAIT_OBJECT_0 + 2))
        {
            // Must first acquire the critical section to avoid race conditions
            // with StartCodec() and the interrupt handler.
            Lock();

            if (!m_InputDMARunning)
            {
                // The timer has expired without another input audio stream
                // being activated so we can really disable the ADC now.
                BSPAudioStopADC();
            }
            else
            {
                // Another audio input operation has been started, 
                // so we should skip disabling the ADC.
                DEBUGMSG(ZONE_TEST, (_T("Skip disabling the ADC\r\n")));
            }

            m_ADCDelayDisableTimerID = NULL;

            Unlock();
        }
    }

    // Close all delayed event handles
    if (m_hStopOutputDMAEvent != NULL)
    {
        CloseHandle(m_hStopOutputDMAEvent);
        m_hStopOutputDMAEvent = NULL;
    }

    if (m_hDACDelayDisableEvent!= NULL)
    {
        CloseHandle(m_hDACDelayDisableEvent);
        m_hDACDelayDisableEvent = NULL;
    }

    if (m_hADCDelayDisableEvent!= NULL)
    {
        CloseHandle(m_hADCDelayDisableEvent);
        m_hADCDelayDisableEvent = NULL;
    }
}


void CallInterruptThread(HardwareContext *pHWContext)
{
    pHWContext->InterruptThread();
}


void CallDisableDelayThread(HardwareContext *pHWContext)
{
    pHWContext->DisableDelayThread();
}

void CallHPDetectInterruptThread(HardwareContext *pHWContext)
{
    pHWContext->HPDetectInterruptThread();
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
//
// Gain control functions
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
#define SW_GAIN     0
inline void HardwareContext::UpdateOutputGain()
{
#if (SW_GAIN)
    m_OutputDeviceContext.SetGain(m_fOutputMute ? 0 : m_dwOutputGain);
#else
    BSPAudioSetOutputGain(m_fOutputMute ? 0 : m_dwOutputGain);
#endif
}

inline void HardwareContext::UpdateInputGain()
{
#if (SW_GAIN)
    m_InputDeviceContext.SetGain(m_fInputMute ? 0 : m_dwInputGain);
#else
    BSPAudioSetInputGain(m_fInputMute ? 0 : m_dwInputGain);
#endif
}

MMRESULT HardwareContext::SetOutputGain (DWORD dwGain)
{
    m_dwOutputGain = dwGain;
    UpdateOutputGain();
    return MMSYSERR_NOERROR;
}

MMRESULT HardwareContext::SetOutputMute (BOOL fMute)
{
    m_fOutputMute = fMute;
    UpdateOutputGain();
    return MMSYSERR_NOERROR;
}

DWORD HardwareContext::GetOutputGain (void)
{
    return m_dwOutputGain;
}

BOOL HardwareContext::GetOutputMute (void)
{
    return m_fOutputMute;
}

BOOL HardwareContext::GetInputMute (void)
{
    return m_fInputMute;
}

MMRESULT HardwareContext::SetInputMute (BOOL fMute)
{
    m_fInputMute = fMute;
    UpdateInputGain();
    return MMSYSERR_NOERROR;
}

DWORD HardwareContext::GetInputGain (void)
{
    return m_dwInputGain;
}

MMRESULT HardwareContext::SetInputGain (DWORD dwGain)
{
    m_dwInputGain = dwGain;
    UpdateInputGain();
    return MMSYSERR_NOERROR;
}

// Control the hardware speaker enable
void HardwareContext::SetSpeakerEnable(BOOL bEnable)
{
    BSPSetSpeakerEnable(bEnable);
    return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// RecalcSpeakerEnable decides whether to enable the speaker or not.
// For now, it only looks at the m_bForceSpeaker variable, but it could
// also look at whether the headset is plugged in
// and/or whether we're in a voice call. Some mechanism would
// need to be implemented to inform the wave driver of changes in the state of
// these variables however.
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

void HardwareContext::RecalcSpeakerEnable()
{
    SetSpeakerEnable(m_NumForcedSpeaker);
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// SetForceSpeaker is called from the device context to update the state of the
// m_bForceSpeaker variable.
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

DWORD HardwareContext::ForceSpeaker( BOOL bForceSpeaker )
{
    // If m_NumForcedSpeaker is non-zero, audio should be routed to an
    // external speaker (if hw permits).
    if (bForceSpeaker)
    {
        m_NumForcedSpeaker++;
        if (m_NumForcedSpeaker==1)
        {
            RecalcSpeakerEnable();
        }else   m_NumForcedSpeaker--;
    }
    else
    {
        m_NumForcedSpeaker--;
        if (m_NumForcedSpeaker==0)
        {
            RecalcSpeakerEnable();
        }else   m_NumForcedSpeaker++;
    }

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
//  Function: PmControlMessage
//
//  Power management routine.
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

BOOL
HardwareContext::PmControlMessage (
                                  DWORD  dwCode,
                                  PBYTE  pBufIn,
                                  DWORD  dwLenIn,
                                  PBYTE  pBufOut,
                                  DWORD  dwLenOut,
                                  PDWORD pdwActualOut)
{
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);
    
    BOOL bRetVal = FALSE;

    switch (dwCode)
    {
    // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
        {
            PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;

            // Check arguments.
            if ( ppc && (dwLenOut >= sizeof(POWER_CAPABILITIES)) && pdwActualOut )
            {
                // Clear capabilities structure.
                memset( ppc, 0, sizeof(POWER_CAPABILITIES) );

                // Set power capabilities. Supports D0 and D4.
                ppc->DeviceDx = DX_MASK(D0)|DX_MASK(D4);

                DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_CAPABILITIES = 0x%x\r\n"), ppc->DeviceDx));

                // Update returned data size.
                *pdwActualOut = sizeof(*ppc);
                bRetVal = TRUE;
            }
            else
            {
                DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: Invalid parameter.\r\n" ) ) );
            }
            break;
        }

        // Indicate if the device is ready to enter a new device power state.
    case IOCTL_POWER_QUERY:
        {
            PCEDEVICE_POWER_STATE pDxState = (PCEDEVICE_POWER_STATE)pBufOut;

            // Check arguments.
            if (pDxState && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE)) && pdwActualOut)
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_QUERY = %d\r\n"), *pDxState));

                // Check for any valid power state.
                if (VALID_DX (*pDxState))
                {
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    bRetVal = TRUE;
                }
                else
                {
                    DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_QUERY invalid power state.\r\n" ) ) );
                }
            }
            else
            {
                DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_QUERY invalid parameter.\r\n" ) ) );
            }
            break;
        }

        // Request a change from one device power state to another.
    case IOCTL_POWER_SET:
        {
            PCEDEVICE_POWER_STATE pDxState = (PCEDEVICE_POWER_STATE)pBufOut;

            // Check arguments.
            if (pDxState && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE)))
            {
                DEBUGMSG(ZONE_FUNCTION, ( TEXT( "WAVE: IOCTL_POWER_SET = %d.\r\n"), *pDxState));

                // Check for any valid power state.
                if (VALID_DX(*pDxState))
                {
                    Lock();

                    // Power off.
                    if ( *pDxState == D4 )
                    {
                        PowerDown();
                    }
                    // Power on.
                    else
                    {
                        PowerUp();
                    }

                    m_DxState = *pDxState;

                    Unlock();

                    bRetVal = TRUE;
                }
                else
                {
                    DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_SET invalid power state.\r\n" ) ) );
                }
            }
            else
            {
                DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_SET invalid parameter.\r\n" ) ) );
            }

            break;
        }

        // Return the current device power state.
    case IOCTL_POWER_GET:
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_GET -- not supported!\r\n")));
            break;
        }

    default:
        DEBUGMSG(ZONE_WARN, (TEXT("WAVE: Unknown IOCTL_xxx(0x%0.8X) \r\n"), dwCode));
        break;
    }

    return bRetVal;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerUp
//
//  This function powers up the audio codec chip.  Note that the audio CODEC
//  chip is ONLY powered up when the user wishes to play or record.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void HardwareContext::PowerUp()
{
    m_PowerUpState = TRUE;
    
    BSPAudioPowerUp(TRUE);

    // Restart the audio I/O operations that were suspended when PowerDown()
    // was called.
    if (g_saveOutputDMARunning)
    {
        StartOutputDMA();
    }

    if (g_saveInputDMARunning)
    {
        StartInputDMA();
    }

    BSPAudioPowerUp(FALSE);

    m_delayBypass = FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function:  PowerDown
//
//  This function powers down the audio codec chip.  If the input/output
//  channels are muted, then this function powers down the appropriate
//  components of the audio codec chip in order to conserve battery power.
//
//  The PowerDown() method will power down the entire audio chip while the
//  PowerDown(const DWORD channel) allows for the independent powerdown of
//  the input and/or output components to allow a finer degree of control.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::PowerDown()
{
    // Note that for this special case, we do not need to grab the critical
    // section via Lock() and Unlock() before calling StopOutputDMA() and
    // StopInputDMA(). The powerdown thread runs at a high enough priority
    // that all other threads will be blocked while it runs. Having the
    // Lock() and Unlock() calls here may actually cause potential race
    // condition problems during the shutdown process.
    //
    // Furthermore, we will not resume operation until the hardware is powered
    // up again. At that point the hardware and audio driver state will be
    // reinitialized again. So at this point, we can just go ahead and shut
    // everything down.

    // We need cancel the existing DAC/ADC disable timer and  immidiately 
    // stop DAC/ADC when power down. 
    if (m_DACDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_DACDelayDisableTimerID);
        m_DACDelayDisableTimerID = NULL;
        BSPAudioStopDAC();
    }

    if (m_ADCDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_ADCDelayDisableTimerID);
        m_ADCDelayDisableTimerID = NULL;
        BSPAudioStopADC();
    }

    // This tells the low-level PMIC driver to immediately terminate any
    // pending timer delays.
    BSPAudioPowerDown(FALSE);

    // Set this flag to allow special handling for the powerdown procedure.
    // The most important thing we want to avoid when powering down is using
    // any sort of timer or other type of delay mechanism. We need to bypass
    // anything like that and simply shutdown the hardware components when
    // performing a powerdown operation.
    m_delayBypass = TRUE;

    // Save state for correct powerup handling.
    g_saveOutputDMARunning = m_OutputDMARunning;
    g_saveInputDMARunning  = m_InputDMARunning;
    m_PowerUpState = FALSE;

    // Request all active audio-related DMA channels to stop.
    StopOutputDMA();
    StopInputDMA();

    // Request audio devices to power down.
    BSPAudioPowerDown(TRUE);
}

DWORD HardwareContext::GetDriverRegValue(LPWSTR ValueName, DWORD DefaultValue)
{
    HKEY DevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);

    if (DevKey)
    {
        DWORD ValueLength = sizeof(DWORD);
        RegQueryValueEx(
                       DevKey,
                       ValueName,
                       NULL,
                       NULL,
                       (PUCHAR)&DefaultValue,
                       &ValueLength);

        RegCloseKey(DevKey);
    }

    return DefaultValue;
}

void HardwareContext::SetDriverRegValue(LPWSTR ValueName, DWORD Value)
{
    HKEY DevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);

    if (DevKey)
    {
        RegSetValueEx(
                     DevKey,
                     ValueName,
                     NULL,
                     REG_DWORD,
                     (PUCHAR)&Value,
                     sizeof(DWORD)
                     );

        RegCloseKey(DevKey);
    }

    return;
}

BOOL HardwareContext::IsSupportedOutputFormat(LPWAVEFORMATEX lpFormat)
{
    BOOL bRet = FALSE;

    // Stub out here
    UNREFERENCED_PARAMETER(lpFormat);

    return bRet;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
//
// Mixer support funcions
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
BOOL HardwareContext::SelectInputSource(DWORD nIndex)
{
    Lock();

    // We do not want to change input source when record path is working
    if (m_InputDMARunning)
    {
        RETAILMSG(TRUE, (L"Can not change input source when "
                         L"recording is running\r\n"));
        return FALSE;
    }

    BSPAudioSelectADCSource(nIndex);
    
    Unlock();

    return TRUE;
}

BOOL HardwareContext::SetCodecLoopback(BOOL bEnable)
{
    BOOL bRet;
    
    Lock();

    // We need to stop output and input if they are running
    if (m_OutputDMARunning)
    {
        RETAILMSG(TRUE, (L"SetCodecLoopback: Playback is running\r\n"));

        // Stop output without delay
        m_delayBypass = TRUE;
        StopOutputDMA();
        m_delayBypass = FALSE;
    }

    if (m_InputDMARunning)
    {
        RETAILMSG(TRUE, (L"SetCodecLoopback: Recording is running\r\n"));

        // Stop input without delay
        m_delayBypass = TRUE;
        StopInputDMA();
        m_delayBypass = FALSE;
    }

    bRet = BSPAudioSetCodecLoopback(bEnable);

    // Save state if loopback was successfully set, so that StartOuputDMA() 
    // and StartInputDMA() can check the state of loopback.
    if (bRet)
    {
        m_bCodecLoopbackState = bEnable;
    }
    
    Unlock();

    return bRet;
}

