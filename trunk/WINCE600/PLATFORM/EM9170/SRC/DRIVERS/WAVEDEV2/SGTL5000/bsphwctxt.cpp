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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: bsphwctxt.cpp
//
//  The wavedev2 hwctxt BSP-level implementation.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "mx25_ddk.h"
#include "wavemain.h"
#include "sgtl5000codec.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
static SGTL5000Codec *g_pSGTL5000Codec;


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStartDAC
//
//  This function configures the codec for duplex mode operation and powers on
//  audio codec for playback operation
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioStartDAC(VOID)
{
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_PLAYBACK_ON); 
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStopDAC
//
//  This function stop playback operation
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioStopDAC(VOID)
{
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_PLAYBACK_STOP);
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStartADC
//
//  This function configures the codec for duplex mode operation and powers on
//  audio codec for record operation
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioStartADC(VOID)
{
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_RECORD_ON);
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStopADC
//
//  This function stop record operation
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioStopADC(VOID)
{ 
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_RECORD_STOP);
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSetSpeakerEnable
//
//  the WaveDev2 architecture can send a waveoutmessage to enable the
//  audio output stream to play out of the external speaker.  This function sets
//
//  Parameters:
//      bEnable
//          [in] enable/disable Speaker
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void BSPSetSpeakerEnable(BOOL bEnable)
{
    if (bEnable)
    {
        // Set a flag indicating the codec that force speaker flag is set
        g_pSGTL5000Codec->SGTL5000ForceSpeaker(ENABLE);
    }
    else
    {
        // Update the force speaker flag
        g_pSGTL5000Codec->SGTL5000ForceSpeaker(DISABLE);
    }

    // Audio playback will be done via speakers or headphones based on 
    // MM_WOM_FORCESPEAKER flag and whether the Headphones are plugged in
    g_pSGTL5000Codec->SGTL5000SpeakerHeadphoneSelection();
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAudioSelectADCSource
//
//  This function selects the input source into audio codec ADC.
//
//  Parameters:
//      nIndex
//          [in] Input source index. 0 - MIC, 1 - Line In.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioSelectADCSource(DWORD nIndex)
{
    if (nIndex == 0)
    {
        g_pSGTL5000Codec->SGTL5000SetADCInput(ADC_INPUT_MIC);
    }
    else
    {
        g_pSGTL5000Codec->SGTL5000SetADCInput(ADC_INPUT_LINEIN);
    }
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioSetCodecLoopback
//
//  This function turns on/off codec loopback function.
//
//  Parameters:
//      bEnable
//          [out] TRUE for on, FALSE for OFF.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL BSPAudioSetCodecLoopback(BOOL bEnable)
{
    // Not supported on SGTL5000
    UNREFERENCED_PARAMETER(bEnable);
    return true;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioSetOutputGain
//
//  This function configures the Stereo DAC output digital volume
//  level based upon the volume level that Windows is currently requesting.
//
//  Parameters:
//      dwGain
//          [in] The desired output volume level.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioSetOutputGain(DWORD dwGain)
{
    #define DB_MAX  (0)
    #define DB_MIN  (-90)
    #define NUM_ENTRY ((DB_MAX-DB_MIN) * 2)
    
    DWORD lowGain = (dwGain & 0xFFFF);   
    DWORD highGain = (dwGain & 0xFFFF0000) >> 16;
    BYTE lVol, rVol;
    
    double fDb; 
     
    // Map linear volume(0xFFFF - 0x0) to DB( DB_MAX - DB_MIN) 
    if (lowGain == 0)
    {
        lVol = (BYTE)DAC_VAL_MAX;
    }
    else
    {
        // About 7-10db will cause voice double heared by ear 
        fDb = log((lowGain*1.0)/0xFFFF) * 25;  
        lVol = (BYTE)(((DB_MAX - fDb)*NUM_ENTRY)/(DB_MAX-DB_MIN) + DAC_VAL_MIN) ;
    }
    
    if (lowGain == highGain)
    {
        rVol = lVol;
    }
    else
    { 
        if (highGain == 0)
        {
            rVol = (BYTE)DAC_VAL_MAX;
        }
        else
        {
            fDb = log((highGain*1.0)/0xFFFF) * 25; 
            rVol = (BYTE)(((DB_MAX - fDb)*NUM_ENTRY)/(DB_MAX-DB_MIN) + DAC_VAL_MIN);
         }
     }
    
    g_pSGTL5000Codec->SGTL5000SetDACVolume(lVol, rVol);
}



//-----------------------------------------------------------------------------
//
//  Function: BSPAudioSetInputGain
//
//  This function configures the Stereo ADC's digital input gain
//  level based upon the volume level that Windows is currently requesting.
//
//  Parameters:
//      dwGain
//          [in] The desired input volume level.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioSetInputGain(DWORD dwGain)
{    
    DWORD lowGain = (dwGain & 0xFFFF);   
    DWORD highGain = (dwGain & 0xFFFF0000) >> 16;
    BYTE lVol, rVol;

    lVol = (BYTE)(lowGain >> 12);
    rVol = (BYTE)(highGain >> 12);
    
    g_pSGTL5000Codec->SGTL5000SetADCVolume(lVol, rVol);
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioGetInputDeviceNum
//
//  Get device number capabile of recording
//
//  Parameters:
//      None
//
//  Returns:
//      UINT8 Device number
//
//-----------------------------------------------------------------------------
UINT8 BSPAudioGetInputDeviceNum(VOID)
{
#ifdef AUDIO_NOREC
    return 0;
#else
    return 1;
#endif
}

    
//-----------------------------------------------------------------------------
//
//  Function: BSPAllocOutputDMABuffer
//
//  Allocate the audio output DMA buffers.
//
//  Parameters:
//      pVirtAddr
//          [out] The virtual address of the buffer.
//
//      pPhysAddr
//          [out] The physical address of the buffer.
//
//  Returns:
//      The buffer size in bytes.
//
//-----------------------------------------------------------------------------
UINT16 BSPAllocOutputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr)
{
#ifdef BSP_AUDIO_DMA_BUF_ADDR
    // Use a statically defined memory region for the audio DMA buffers. This
    // may be either internal or external memory depending upon the address
    // region that is selected.

    pPhysAddr->HighPart = 0;
    pPhysAddr->LowPart  = BSP_AUDIO_DMA_BUF_ADDR;

    *pVirtAddr = (PBYTE)MmMapIoSpace(*pPhysAddr, BSP_AUDIO_DMA_BUF_SIZE, FALSE);
#else
    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);
    // Allocate the input DMA buffers (same size as output) from external memory
    *pVirtAddr = (PBYTE)HalAllocateCommonBuffer(
                  &Adapter,
                  BSP_AUDIO_DMA_BUF_SIZE,
                  pPhysAddr,
                  FALSE);
#endif

    return BSP_AUDIO_DMA_BUF_SIZE;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPDeallocOutputDMABuffer
//
//  Deallocate the audio output DMA buffers.
//
//  Parameters:
//      VirtAddr
//          [in] The virtual address of the buffer.
//
//  Returns:
//      The buffer size in bytes.
//
//-----------------------------------------------------------------------------
VOID BSPDeallocOutputDMABuffer(PBYTE VirtAddr)
{
    // Nothing do dealloc since we get output DMA buffer from internal RAM
    UNREFERENCED_PARAMETER(VirtAddr);
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAllocInputDMABuffer
//
//  Allocate the audio input DMA buffers.
//
//  Parameters:
//      pVirtAddr
//          [out] The virtual address of the buffer.
//
//      pPhysAddr
//          [out] The physical address of the buffer.
//
//  Returns:
//      The buffer size in bytes.
//
//-----------------------------------------------------------------------------
UINT16 BSPAllocInputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr)
{
    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate the input DMA buffers (same size as output) from external memory
    *pVirtAddr = (PBYTE)HalAllocateCommonBuffer(
                  &Adapter,
                  BSP_AUDIO_DMA_BUF_SIZE,
                  pPhysAddr,
                  FALSE);
    
    return BSP_AUDIO_DMA_BUF_SIZE;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPDeallocOutputDMABuffer
//
//  Deallocate the audio output DMA buffers.
//
//  Parameters:
//      VirtAddr
//          [in] The virtual address of the buffer.
//
//  Returns:
//      The buffer size in bytes.
//
//-----------------------------------------------------------------------------
VOID BSPDeallocInputDMABuffer(PBYTE VirtAddr)
{
    // Only deallocate the audio DMA buffer memory if it was previously
    // dynamically allocated.
    PHYSICAL_ADDRESS phyAddr;

    // Logical address parameter is ignored
    phyAddr.QuadPart = 0;
    
    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    HalFreeCommonBuffer(&Adapter, BSP_AUDIO_DMA_BUF_SIZE, phyAddr, VirtAddr, FALSE);
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioIomuxConfig
//
//  This function configures the IOMUX pins required for the AUDMUX.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioIomuxConfig(VOID)
{
    // Must configure the IOMUX pins for connecting the Audio MUX
    // external port to off-chip peripherals such as the audio codec. Without
    // this step, we will not be able to connect the Audio MUX to the
    // audio codec audio buses.

    // The key here is to configure the 4-wire interface (framesync, bitclock,
    // TX, and RX) through the IOMUX by enabling the functional/normal mode.
    // We leave the input/output direction control for all of the I/O pins up
    // to the Audio MUX.
    //
    // Configure both the RX and TX pins here
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_EB0, DDK_IOMUX_PIN_MUXMODE_ALT4, 
     DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_EB1, DDK_IOMUX_PIN_MUXMODE_ALT4, 
     DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_OE, DDK_IOMUX_PIN_MUXMODE_ALT4, 
     DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_RW, DDK_IOMUX_PIN_MUXMODE_ALT4, 
     DDK_IOMUX_PIN_SION_REGULAR);
}


//-----------------------------------------------------------------------------
//
//  Function: BSPSsiEnableClock
//
//  This function is to enable/disable SSI clock.
//
//  Parameters:
//      index
//          [in] Index specifying the SSI instance.
//
//      bEnable
//          [in] TRUE if the clock is to be enabled, otherwise FALSE.
//
//  Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
// TODO Add CLKO gating here
BOOL BSPSsiEnableClock(UINT32 index, BOOL bEnable)
{
    DDK_CLOCK_GATE_INDEX clockGateIndex;

    switch (index)
    {
        case 1:
            clockGateIndex = DDK_CLOCK_GATE_INDEX_SSI1;
            break;
            
        case 2:
            clockGateIndex = DDK_CLOCK_GATE_INDEX_SSI2;
            break;

        default:
            return FALSE;
    }

    if (bEnable)
    {
        return DDKClockSetGatingMode(clockGateIndex, 
                                     DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
        return DDKClockSetGatingMode(clockGateIndex, 
                                     DDK_CLOCK_GATE_MODE_DISABLED);
    }
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudmuxEnableClock
//
//  This function is to enable/disable AUDMUX clock.
//
//  Parameters:
//      bEnable
//          [in] TRUE if the clock is to be enabled, otherwise FALSE.
//
//  Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
BOOL BSPAudmuxEnableClock(BOOL bEnable)
{
    if (bEnable)
    {
        return DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AUDMUX, 
                                     DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
        return DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AUDMUX, 
                                     DDK_CLOCK_GATE_MODE_DISABLED);
    }
   
    //UNREFERENCED_PARAMETER(bEnable);
}
//-----------------------------------------------------------------------------
//
//  Function: BSPGetAudmuxIntPort
//
//  This function is to get Audmux internal port
//
//  Parameters:
//     None
//
//  Returns:
//      return the Audmux internal port
//
//-----------------------------------------------------------------------------
AUDMUX_INTERNAL_PORT BSPGetAudmuxIntPort(VOID)
{
    return AUDMUX_PORT2;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPGetAudmuxExtPort
//
//  This function is to get Audmux external port
//
//  Parameters:
//     None
//
//  Returns:
//      return the Audmux external port
//
//-----------------------------------------------------------------------------
AUDMUX_EXTERNAL_PORT BSPGetAudmuxExtPort(VOID)
{
    return AUDMUX_PORT4;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioGetSdmaPriority
//
//  This function returns the priority of the SDMA channel used by Audio driver.
//
//  Parameters:
//      None.
//
//  Returns:
//      The priority of the SDMA channel.
//
//-----------------------------------------------------------------------------
UINT8 BSPAudioGetSdmaPriority(VOID)
{
    return BSP_SDMA_CHNPRI_AUDIO;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioPowerUp
//
//  This function powers up the SSI, AUDMUX, and external audio chip.
//
//  Parameters:
//      fullPowerUp
//          [in] Flag of full power-up.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioPowerUp(const BOOL fullPowerUp)
{
    if (fullPowerUp)
    {
        g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_CODEC_ON);
    }
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioPowerDown
//
//  This function powers down the SSI, AUDMUX, and external audio chip.
//
//  Parameters:
//      fullPowerOff
//          [in] Flag of full power-down.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioPowerDown(const BOOL fullPowerOff)
{
    if (fullPowerOff)
    {
        g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_CODEC_OFF);
    }
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioGetProtocol
//
//  This function gets the audio protocol used on SSI.
//
//  Parameters:
//      None.
//
//  Returns:
//      The audio protocol.
//
//-----------------------------------------------------------------------------
AUDIO_PROTOCOL BSPAudioGetProtocol(VOID)
{
    return AUDIO_PROTOCOL_I2S;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioDeinitCodec
//
//  This function de-initializes audio codec.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
VOID BSPAudioDeinitCodec(VOID)
{
    if (g_pSGTL5000Codec)
    {
        g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_CODEC_OFF);        
        delete g_pSGTL5000Codec;
    }
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioInitCodec
//
//  This function initializes audio codec.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL BSPAudioInitCodec(VOID)
{
    g_pSGTL5000Codec = new SGTL5000Codec;

    if (g_pSGTL5000Codec == NULL)
    {
        ERRORMSG(ZONE_ERROR, (L"BSPAudioInitCodec: Init SGTL5000Codec class failed\r\n"));
        goto cleanUp;
    }

    DDKClockSetCKO(1, DDK_CLOCK_CKO_SRC_UNGATED_IPG_CLK, 5);
    // SGTL5000 Initialize
    g_pSGTL5000Codec->SGTL5000CodecInit();
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_CODEC_ON);

    return TRUE;

cleanUp:
    BSPAudioDeinitCodec();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPIsHeadphoneDetectSupported
//
//  This function returns whether the iMX platform supports HP detection
//
//  Parameters:
//      None
//
//  Returns:
//      Bool - True or False
//
//-----------------------------------------------------------------------------
BOOL BSPIsHeadphoneDetectSupported()
{
    // Feature not implemented/supported    
    return FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPGetHPDetectIRQ
//
//  This function returns the IRQ number for the GPIO connected to headphone 
//  detect
//
//  Parameters:
//      None
//
//  Returns:
//      IRQ number.
//
//-----------------------------------------------------------------------------
DWORD BSPGetHPDetectIRQ()
{
    // Feature not implemented/supported
    return 0;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAudioHandleHPDetectIRQ
//
//  This function handles the HP detect IRQ on the GPIO pin.  This is NOT
//  implemented/supported on this MX platform

//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioHandleHPDetectIRQ(VOID)
{
    // Feature not implemented/supported
}

//-----------------------------------------------------------------------------
//
//  Function: BSPGetBitsPerSample
//
//  This function returns BitsPerSample supported on this platform
//
//  Parameters:
//      None.
//
//  Returns:
//      BitsPerSample.
//
//-----------------------------------------------------------------------------
WORD BSPGetBitsPerSample(VOID)
{
    WORD BitsPerSample;

    BitsPerSample = BITSPERSAMPLE;

    return BitsPerSample;
}

