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
//  The hwctxt BSP-level implementation.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "wm8580.h"
#include "ak5702.h"
#include "wavemain.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
static CWM8580 *g_pCWM8580;
static CAK5702 *g_pCAK5702;

static const WCHAR c_szESAIKey[]   = L"Drivers\\BuiltIn\\ESAI";
static const WCHAR c_szAudioProtocol[] = L"AudioProtocol";
static AUDIO_PROTOCOL g_sAudioInputProtocol = AUDIO_PROTOCOL_I2S;
static AUDIO_PROTOCOL g_sAudioOutputProtocol = AUDIO_PROTOCOL_LEFT_ALIGNED;


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


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
BOOL BSPAudioInitCodec()
{
    HKEY hKey = NULL;    
    DWORD dwAudioProtocol;
    DWORD dwSize = sizeof(DWORD);

    //create class for dac
    g_pCWM8580 = new CWM8580;

    if (g_pCWM8580 == NULL)
    {
        goto errexit;
    }

    g_pCWM8580->InitCodec();
    g_pCWM8580->EnableCLKO(); //enable clko for ak5702 input

    
    // create class for adc
    g_pCAK5702 = new CAK5702;

    if (g_pCAK5702 == NULL)
    {
        goto errexit;
    }
    g_pCAK5702->InitAK5702();


    // Allow for registry override of output bus protocol value
    if (RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        c_szESAIKey,
        0,
        0,
        &hKey
        ) == ERROR_SUCCESS)
    {
        

        RegQueryValueEx(
            hKey,
            c_szAudioProtocol,
            NULL,
            NULL,
            (LPBYTE)&dwAudioProtocol,
            &dwSize
        );

        if (dwAudioProtocol == 2){
            g_sAudioOutputProtocol = AUDIO_PROTOCOL_LEFT_ALIGNED;
        }else{
            g_sAudioOutputProtocol = AUDIO_PROTOCOL_NETWORK;
        }          
            
        //RETAILMSG(TRUE,(TEXT("Audio Protocol: reg val(%d) prot(%d)\r\n"),
        //    dwAudioProtocol,g_sAudioOutputProtocol));
        
        RegCloseKey(hKey);
    }
 
    return TRUE;

errexit:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioConfigDAC
//
//  This function configures audio dac.
//
//  Parameters:
//      Audio bus params.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL BSPAudioConfigDAC(DWORD dwSampleRate, DWORD dwBitDepth, 
    DWORD dwChnMask,DWORD dwChnNum, AUDIO_PROTOCOL audioProtocol)
{
    g_pCWM8580->ConfigOutput(dwSampleRate,dwBitDepth,dwChnMask,dwChnNum,audioProtocol);
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAudioConfigADC
//
//  This function configures audio adc.
//
//  Parameters:
//      Audio bus params.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL BSPAudioConfigADC(DWORD dwSampleRate, DWORD dwBitDepth, 
    DWORD dwChnMask,DWORD dwChnNum, AUDIO_PROTOCOL audioProtocol)
{
    g_pCAK5702->ConfigADC(audioProtocol,dwSampleRate,dwBitDepth,
        dwChnNum,dwChnMask,PLL_MCKI_12M_DEF);
    return TRUE;
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
VOID BSPAudioDeinitCodec()
{
    if (g_pCWM8580)
    {
        //g_pCWM8580->SetClockPower(FALSE);
        delete g_pCWM8580;
    }

    if (g_pCAK5702)
    {
        g_pCAK5702->SetCodecPower(AK5702_POWER_FULLOFF);
        delete g_pCAK5702;

    }
    return;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStartDAC
//
//  This function powers on audio codec DAC.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioStartDAC()
{
    g_pCWM8580->EnableDAC();
    return;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStopDAC
//
//  This function powers down audio codec DAC.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioStopDAC()
{
    g_pCWM8580->DisableDAC();
    return;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStartADC
//
//  This function powers on audio codec ADC.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioStartADC()
{
   g_pCAK5702->EnalbeADC();
   return;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStopADC
//
//  This function powers down audio codec ADC.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioStopADC()
{
    g_pCAK5702->DisableADC();
    return;
}



//-----------------------------------------------------------------------------
//
//  Function: BSPAudioSetOutputGain
//
//  This function configures the Stereo DAC's output PGA amplifier gain
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
    g_pCWM8580->SetOutputGain(dwGain,dwGain,dwGain,0x0f);
    return;
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
    BYTE ucVolL, ucVolR;
    ucVolL = (BYTE)((dwGain && 0xffff) >> 8);
    ucVolR = (BYTE)(dwGain >> 24);
    g_pCAK5702->SetInputGain(ucVolL,ucVolR,ucVolL,ucVolR,0XF);
    return;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioGetInputDeviceNum
//
//  Get the number of input device.
//
//  Parameters:
//      none.
//
//  Returns:
//      Number of input device.
//
//-----------------------------------------------------------------------------
UINT8 BSPAudioGetInputDeviceNum()
{
    return 1;

#if 0
#ifdef AUDIO_NOREC
    return 0;
#else
    return 1;
#endif

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

    // Use dynamically allocated memory for the audio DMA buffer (should be multiple of 6).
    static const DMA_BUFFER_REGION_SIZE_NBYTES = (0x1000*12);

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate DMA buffers from external memory
    *pVirtAddr = (PBYTE)HalAllocateCommonBuffer(
                            &Adapter,
                            DMA_BUFFER_REGION_SIZE_NBYTES,
                            pPhysAddr,
                            FALSE);
    
    return DMA_BUFFER_REGION_SIZE_NBYTES;
    
    // Use a statically defined memory region for the audio DMA buffers. This
    // may be either internal or external memory depending upon the address
    // region that is selected.
#if 0
    pPhysAddr->HighPart = 0;
    pPhysAddr->LowPart  = BSP_AUDIO_DMA_BUF_ADDR;

    *pVirtAddr = (PBYTE)MmMapIoSpace(*pPhysAddr, BSP_AUDIO_DMA_BUF_SIZE, FALSE);

    // If virtual mapping succeeded and buffer is being allocated from IRAM
    if ((*pVirtAddr) && (pPhysAddr->LowPart >= IMAGE_WINCE_IRAM_PA_START) && 
        (pPhysAddr->LowPart < (IMAGE_WINCE_IRAM_PA_START+IMAGE_WINCE_IRAM_SIZE)))
    {
        // Hardware constraints require IRAM to be accessed with PAHB or
        // we can incur errors on subsequent accesses to DRAM.  Update
        // the MMU attributes to use backwards-compatible extended
        // small page format.  This format allows us to set the TEX bits
        // to mark the IRAM region for non-shared device (directs
        // access to PAHB)
        //
        // Extended Small Page Format
        // --------------------------
        // BITS[31:12] = ADDRESS (preserved)
        // BITS[11:9] = SBZ (write as zero)
        // BITS[8:6] = TEX  (010b used for non-shared device)
        // BITS[5:4] = AP (preserved)
        // BITS[3:2] = CB (00b used for non-shared device)
        // BITS[1:0] = must be 11b for extended small page
        //
        // VAL = (0x0 << 9) | (0x2 << 6) | (0x0 << 2) | (0x3 << 0) = 0x083
        // MASK = (0x7 << 9) | (0x7 << 6) | (0x3 << 2) | (0x3 << 0) = 0xFCF
        
        if (!VirtualSetAttributes(*pVirtAddr, BSP_AUDIO_DMA_BUF_SIZE, 
            0x083, 0xFCF, NULL))
        {
            ERRORMSG(ZONE_ERROR, (_T("VirtualSetAttributes failed!\r\n")));
            goto cleanUp;
        }
    
        // Flush the TLB since we directly updated the page tables
        CacheRangeFlush(0, 0, CACHE_SYNC_FLUSH_TLB);
    }

cleanUp:
    return BSP_AUDIO_DMA_BUF_SIZE;
#endif    
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
//      phyAddr:
//          [in] The physical address of the buffer
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPDeallocOutputDMABuffer(PBYTE VirtAddr,PHYSICAL_ADDRESS phyAddr)
{
    // Nothing do dealloc since we get output DMA buffer from internal RAM
    static const DMA_BUFFER_REGION_SIZE_NBYTES = (0x1000*12);
    DMA_ADAPTER_OBJECT Adapter;
    
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    HalFreeCommonBuffer(&Adapter, DMA_BUFFER_REGION_SIZE_NBYTES, phyAddr, VirtAddr, FALSE);
    return;
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
    // Use dynamically allocated memory for the audio DMA buffer (2 pages).
    static const DMA_BUFFER_REGION_SIZE_NBYTES = (0x1000 * 2);

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate DMA buffers from external memory
    *pVirtAddr = (PBYTE)HalAllocateCommonBuffer(
                            &Adapter,
                            DMA_BUFFER_REGION_SIZE_NBYTES,
                            pPhysAddr,
                            FALSE);
    
    return DMA_BUFFER_REGION_SIZE_NBYTES;
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
VOID BSPDeallocInputDMABuffer(PBYTE VirtAddr,PHYSICAL_ADDRESS phyAddr)
{
    // Only deallocate the audio DMA buffer memory if it was previously
    // dynamically allocated.
    static const DMA_BUFFER_REGION_SIZE_NBYTES = (0x1000 * 2);
    DMA_ADAPTER_OBJECT Adapter;
    
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    HalFreeCommonBuffer(&Adapter, DMA_BUFFER_REGION_SIZE_NBYTES, phyAddr, VirtAddr, FALSE);
    return;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioIomuxConfig
//
//  This function configures the IOMUX pins required for the ESAI.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPAudioIomuxConfig()
{
    // SCKR
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D2,     DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // FSR
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D3,     DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // HCKR
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D4,     DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // SCKT
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D5,     DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // FST
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D6,     DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // HCKT
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D7,     DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // TX5_RX0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D8,     DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // TX4_RX1
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D9,     DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // TX3_RX2
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_MCLK,   DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // TX2_RX3
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_VSYNC,  DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // TX1
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_HSYNC,  DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
    // TX0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_PIXCLK, DDK_IOMUX_PIN_MUXMODE_ALT3, DDK_IOMUX_PIN_SION_REGULAR);
}


//-----------------------------------------------------------------------------
//
//  Function: BSPESAIEnableClock
//
//  This function is to enable/disable ESAI clock.
//
//  Parameters:
//      index
//          [in] Index specifying the ESAI instance.
//
//      bEnable
//          [in] TRUE if the clock is to be enabled, otherwise FALSE.
//
//  Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
BOOL BSPESAIEnableClock(UINT32 index, BOOL bEnable)
{
    UNREFERENCED_PARAMETER(index);
    
    if (bEnable)
    {
        DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_ESAI, 
                              DDK_CLOCK_GATE_MODE_ENABLED);

        DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_ESAI, 
                              DDK_CLOCK_GATE_MODE_ENABLED);

        return DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_ESAI, 
                                    DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
        DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_ESAI, 
                              DDK_CLOCK_GATE_MODE_DISABLED);

        DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_ESAI, 
                              DDK_CLOCK_GATE_MODE_DISABLED);

        return DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_ESAI, 
                                    DDK_CLOCK_GATE_MODE_DISABLED);
    }
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
UINT8 BSPAudioGetSdmaPriority()
{
    return BSP_SDMA_CHNPRI_AUDIO;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioPowerUp
//
//  This function powers up the ESAI,CODEC
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
    UNREFERENCED_PARAMETER(fullPowerUp);

    return;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioPowerDown
//
//  This function powers down the ESAI, CODEC
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
    UNREFERENCED_PARAMETER(fullPowerOff); 

    return;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioGetOutputProtocol
//
//  This function gets the audio protocol used on ESAI.
//
//  Parameters:
//      None.
//
//  Returns:
//      The audio protocol.
//
//-----------------------------------------------------------------------------
AUDIO_PROTOCOL BSPAudioGetOutputProtocol()
{
    return g_sAudioOutputProtocol;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAudioGetInputProtocol
//
//  This function gets the audio protocol used on ESAI.
//
//  Parameters:
//      None.
//
//  Returns:
//      The audio protocol.
//
//-----------------------------------------------------------------------------
AUDIO_PROTOCOL BSPAudioGetInputProtocol()
{
    return g_sAudioInputProtocol;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPIsSupportedOutputFormat
//
//  Query if the output audio format is supported
//
//  Parameters:
//      pFmtEx: poniter to the audio format.
//
//  Returns:
//      Return TRUE if the format is supported, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPIsSupportedOutputFormat(PWAVEFORMATEXTENSIBLE pFmtEx)
{
    //max channel number is 6
    if(pFmtEx->Format.nChannels > 6){
        return FALSE;
    }

    if((g_sAudioOutputProtocol == AUDIO_PROTOCOL_NETWORK) 
        && (pFmtEx->Format.nSamplesPerSec > 48000)){
        RETAILMSG(TRUE,(TEXT("SampleRate>48K is not supported in network mode\r\n")));
        return FALSE;
    }
    
    if ((pFmtEx->Format.wBitsPerSample == 16) 
         && (pFmtEx->Samples.wValidBitsPerSample == 16)){
        if ((pFmtEx->Format.nSamplesPerSec == 16000)||
            (pFmtEx->Format.nSamplesPerSec == 22050)||
            (pFmtEx->Format.nSamplesPerSec == 32000)||
            (pFmtEx->Format.nSamplesPerSec == 44100)||
            (pFmtEx->Format.nSamplesPerSec == 48000)||
            (pFmtEx->Format.nSamplesPerSec == 64000)||
            (pFmtEx->Format.nSamplesPerSec == 88200)||
            (pFmtEx->Format.nSamplesPerSec == 96000)||
            (pFmtEx->Format.nSamplesPerSec == 128000)||
            (pFmtEx->Format.nSamplesPerSec == 176400)||
            (pFmtEx->Format.nSamplesPerSec == 192000)){
            return TRUE;
        }else{
            return FALSE;
        } 
    }

    if ((pFmtEx->Format.wBitsPerSample == 32) 
        && (pFmtEx->Samples.wValidBitsPerSample == 24)){
        if ((pFmtEx->Format.nSamplesPerSec == 16000)||
            (pFmtEx->Format.nSamplesPerSec == 22050)||
            (pFmtEx->Format.nSamplesPerSec == 32000)||
            (pFmtEx->Format.nSamplesPerSec == 44100)||
            (pFmtEx->Format.nSamplesPerSec == 48000)||
            (pFmtEx->Format.nSamplesPerSec == 64000)||
            (pFmtEx->Format.nSamplesPerSec == 88200)||
            (pFmtEx->Format.nSamplesPerSec == 96000)||
            (pFmtEx->Format.nSamplesPerSec == 128000)||
            (pFmtEx->Format.nSamplesPerSec == 176400)||
            (pFmtEx->Format.nSamplesPerSec == 192000)){
            return TRUE;
        }else{
            return FALSE;
        } 
    }

    return FALSE;
    
}

//-----------------------------------------------------------------------------
//
//  Function: BSPIsSupportedInputFormat
//
//  Query if the input audio format is supported
//
//  Parameters:
//      pFmtEx: poniter to the audio format.
//
//  Returns:
//      Return TRUE if the format is supported, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPIsSupportedInputFormat(PWAVEFORMATEXTENSIBLE pFmtEx)
{
    //max input channel number is 4
    if(pFmtEx->Format.nChannels > 4){
        return FALSE;
    }
    
    if ((pFmtEx->Format.wBitsPerSample == 16) 
         && (pFmtEx->Samples.wValidBitsPerSample == 16)){
        if ((pFmtEx->Format.nSamplesPerSec == 32000)||
            (pFmtEx->Format.nSamplesPerSec == 44100)||
            (pFmtEx->Format.nSamplesPerSec == 48000)||
            (pFmtEx->Format.nSamplesPerSec == 24000)||
            (pFmtEx->Format.nSamplesPerSec == 8000)||
            (pFmtEx->Format.nSamplesPerSec == 12000)||
            (pFmtEx->Format.nSamplesPerSec == 16000)){
            return TRUE;
        }else{
            return FALSE;
        }        
    }

    if ((pFmtEx->Format.wBitsPerSample == 32) 
        && (pFmtEx->Samples.wValidBitsPerSample == 24)){
        if ((pFmtEx->Format.nSamplesPerSec == 32000)||
            (pFmtEx->Format.nSamplesPerSec == 44100)||
            (pFmtEx->Format.nSamplesPerSec == 48000)||
            (pFmtEx->Format.nSamplesPerSec == 24000)||
            (pFmtEx->Format.nSamplesPerSec == 16000)||
            (pFmtEx->Format.nSamplesPerSec == 12000)||
            (pFmtEx->Format.nSamplesPerSec == 8000)){
            return TRUE;
        }else{
            return FALSE;
        } 
    }

    return FALSE;   
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAudioGetTxBitClock
//
//  Get the bit clock rate for playback, this function is mainly used for asrc operation
//
//  Parameters:
//      none.
//
//  Returns:
//      Return the bit clock rate.
//
//-----------------------------------------------------------------------------
UINT32 BSPAudioGetTxBitClock(void)
{
    return g_pCWM8580->GetDACBitClock();
}

