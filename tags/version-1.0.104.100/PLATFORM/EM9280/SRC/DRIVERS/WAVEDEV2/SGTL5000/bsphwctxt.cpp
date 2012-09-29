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
//  Copyright (C) 2009 - 2010, Freescale Semiconductor, Inc. All Rights Reserved.
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
#pragma warning(push)
#pragma warning(disable: 4201)
#include <windows.h>
#pragma warning(pop)
#include <nkintr.h>
#include <ceddk.h>
#include "wavemain.h"
#include "bsp.h"
#include "sgtl5000codec.h"
#include "regsdigctl.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
// used for setting up the DMA Descriptor
#define BUFFER_A            0
#define BUFFER_B            1

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
static SGTL5000Codec *g_pSGTL5000Codec;
UINT32 EnterSuspend = 0;
PVOID pv_HWregPOWER = NULL;


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioInitCodec
//
//  This function initializes SAIF and audio codec. SAIF0 is TX(master) and SAIF1 is RX(slave), and 
//  external codec is slave; MCLK/LRCLK/BITCLK are output.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::BSPAudioInitCodec()
{
    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregDIGCTL;
    UINT32 u32Div;

    DEBUGMSG(ZONE_FUNCTION, (_T("+HardwareContext::BSPAudioInitCodec\n")));

    // Open SAIF0/SAIF1 clock
    DDKClockSetGatingMode(DDK_CLOCK_GATE_SAIF0_CLK, FALSE);
    DDKClockSetGatingMode(DDK_CLOCK_GATE_SAIF1_CLK, FALSE);
    // Sample rate = 44100, SAIF CLK = 22.5792MHz, Oversample Base Rate Multiplier = 512
    u32Div = 0x0C0B;  
    DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SAIF0, DDK_CLOCK_BAUD_SOURCE_REF_PLL, u32Div);
    DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SAIF1, DDK_CLOCK_BAUD_SOURCE_REF_PLL, u32Div);

    // Init SAIF0(AUDIOOUT)
    // Reset SAIF0
    HW_SAIF_CTRL_SET(0, BM_SAIF_CTRL_SFTRST);

    // Wait for some time
    HW_SAIF_CTRL_RD(0);

    // In order to configure SAIF0,  we must first clear the CLKGATE and SFTRST bit
    HW_SAIF_CTRL_CLR(0, BM_SAIF_CTRL_SFTRST);

    // Wait for some time
    HW_SAIF_CTRL_RD(0);

    // Clear the clock gate for normal operation
    HW_SAIF_CTRL_CLR(0, BM_SAIF_CTRL_CLKGATE);

    // Wait for some time
    HW_SAIF_CTRL_RD(0);

    // 16bit sample
    HW_SAIF_CTRL_WR(0, BF_SAIF_CTRL_SFTRST(0)                    |
                       BF_SAIF_CTRL_CLKGATE(0)                   |
                       BF_SAIF_CTRL_BITCLK_MULT_RATE(0)          |
                       BF_SAIF_CTRL_BITCLK_BASE_RATE(0)          |
                       BF_SAIF_CTRL_FIFO_ERROR_IRQ_EN(0)         |
                       BF_SAIF_CTRL_FIFO_SERVICE_IRQ_EN(0)       |
                       BF_SAIF_CTRL_DMAWAIT_COUNT(0)             |
                       BF_SAIF_CTRL_CHANNEL_NUM_SELECT(0)        |
                       BF_SAIF_CTRL_BIT_ORDER(0)                 |
                       BF_SAIF_CTRL_DELAY(1)                     |
                       BF_SAIF_CTRL_JUSTIFY(0)                   |
                       BF_SAIF_CTRL_LRCLK_POLARITY(0)            |
                       BF_SAIF_CTRL_BITCLK_EDGE(0)               |
                       BF_SAIF_CTRL_WORD_LENGTH(0)               |
                       BF_SAIF_CTRL_BITCLK_48XFS_ENABLE(0)       |
                       BF_SAIF_CTRL_SLAVE_MODE(0)                |
                       BF_SAIF_CTRL_READ_MODE(0)                 |
                       BF_SAIF_CTRL_RUN(1));


    // Init SAIF1(AUDIOIN)
    // Reset SAIF1
    HW_SAIF_CTRL_SET(1, BM_SAIF_CTRL_SFTRST);

    // Wait for some time
    HW_SAIF_CTRL_RD(1);

    // In order to configure SAIF0,  we must first clear the CLKGATE and SFTRST bit
    HW_SAIF_CTRL_CLR(1, BM_SAIF_CTRL_SFTRST);

    // Wait for some time
    HW_SAIF_CTRL_RD(1);

    // Clear the clock gate for normal operation
    HW_SAIF_CTRL_CLR(1, BM_SAIF_CTRL_CLKGATE);

    // Wait for some time
    HW_SAIF_CTRL_RD(1);

    // 16bit sample
    HW_SAIF_CTRL_WR(1, BF_SAIF_CTRL_SFTRST(0)                    |
                       BF_SAIF_CTRL_CLKGATE(0)                   |
                       BF_SAIF_CTRL_BITCLK_MULT_RATE(0)          |
                       BF_SAIF_CTRL_BITCLK_BASE_RATE(0)          |
                       BF_SAIF_CTRL_FIFO_ERROR_IRQ_EN(0)         |
                       BF_SAIF_CTRL_FIFO_SERVICE_IRQ_EN(0)       |
                       BF_SAIF_CTRL_DMAWAIT_COUNT(0)             |
                       BF_SAIF_CTRL_CHANNEL_NUM_SELECT(0)        |
                       BF_SAIF_CTRL_BIT_ORDER(0)                 |
                       BF_SAIF_CTRL_DELAY(1)                     |
                       BF_SAIF_CTRL_JUSTIFY(0)                   |
                       BF_SAIF_CTRL_LRCLK_POLARITY(0)            |
                       BF_SAIF_CTRL_BITCLK_EDGE(0)               |
                       BF_SAIF_CTRL_WORD_LENGTH(0)               |
                       BF_SAIF_CTRL_BITCLK_48XFS_ENABLE(0)       |
                       BF_SAIF_CTRL_SLAVE_MODE(1)                |
                       BF_SAIF_CTRL_READ_MODE(1)                 |
                       BF_SAIF_CTRL_RUN(1));


    BSPAudioIomuxConfig();

    // SAIF0 clock pins(MCLK/LRCLK/BITCLK) are selected for both SAIF0 and SAIF1 input clock pins
    phyAddr.QuadPart = CSP_BASE_REG_PA_DIGCTL;
    pv_HWregDIGCTL = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
    if(pv_HWregDIGCTL)
    {   
        HW_DIGCTL_CTRL_CLR(BF_DIGCTL_CTRL_SAIF_CLKMUX_SEL(3));
        HW_DIGCTL_CTRL_SET(BF_DIGCTL_CTRL_SAIF_CLKMUX_SEL(2));

        MmUnmapIoSpace(pv_HWregDIGCTL,0x1000);
        pv_HWregDIGCTL = NULL;
    }
    
    // Initialize SGTL5000 audio codec
    g_pSGTL5000Codec = new SGTL5000Codec;

    if (g_pSGTL5000Codec == NULL)
    {
        ERRORMSG(ZONE_ERROR, (L"BSPAudioInitCodec: Init SGTL5000Codec class failed\r\n"));
        return FALSE;
    }

    // SGTL5000 Initialize
    g_pSGTL5000Codec->SGTL5000CodecInit();
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_CODEC_ON);

    if (pv_HWregPOWER == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_POWER;

        // Map peripheral physical address to virtual address
        pv_HWregPOWER = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregPOWER == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace failed!\r\n")));
            return FALSE;
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("-HardwareContext::BSPAudioInit\n")));
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
VOID HardwareContext:: BSPAudioDeinitCodec()
{
    if (g_pSGTL5000Codec)
    {
        g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_CODEC_OFF);        
        delete g_pSGTL5000Codec;
    }

    if (pv_HWregPOWER != NULL)
    {
        // UnMap peripheral physical address to virtual address
        MmUnmapIoSpace(pv_HWregPOWER, 0x1000);
    }
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
VOID HardwareContext::BSPAudioStartDAC()
{
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_PLAYBACK_ON); 

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
VOID HardwareContext::BSPAudioStopDAC()
{
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_PLAYBACK_STOP);

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
VOID HardwareContext:: BSPAudioStartADC()
{
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_RECORD_ON);

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
VOID HardwareContext:: BSPAudioStopADC()
{
    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_RECORD_STOP);

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
VOID HardwareContext:: BSPAudioSelectADCSource(DWORD nIndex)
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
BOOL HardwareContext:: BSPAudioSetCodecLoopback(BOOL bEnable)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(bEnable);

    return TRUE;
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
VOID HardwareContext::BSPAudioSetOutputGain(DWORD dwGain)
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

    DEBUGMSG(ZONE_FUNCTION, (_T("-HardwareContext::BSPAudioSetOutputGain dwGain %d \n"),dwGain));
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
VOID HardwareContext::BSPAudioSetInputGain(DWORD dwGain)
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
//  This function return input device number.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
UINT8 HardwareContext::BSPAudioGetInputDeviceNum()
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
UINT16 HardwareContext::BSPAllocOutputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr)
{
#ifdef BSP_AUDIO_DMA_BUF_ADDR
    // Use a statically defined memory region for the audio DMA buffers. This
    // may be either internal or external memory depending upon the address
    // region that is selected.
    pPhysAddr->HighPart = 0;
    pPhysAddr->LowPart  = BSP_AUDIO_DMA_BUF_ADDR;
    
    *pVirtAddr = (PBYTE)MmMapIoSpace(*pPhysAddr, BSP_AUDIO_DMA_BUF_SIZE, FALSE);

    // If virtual mapping succeeded and buffer is being allocated from IRAM
    if ((pVirtAddr) && (pPhysAddr->LowPart >= IMAGE_WINCE_IRAM_PA_START) &&
        ((pPhysAddr->LowPart+BSP_AUDIO_DMA_BUF_SIZE) <= (IMAGE_WINCE_IRAM_PA_START+IMAGE_WINCE_IRAM_SIZE)))
    {
        // Default mapping of audio buffer will be noncacheable-nonbufferable (strongly ordered).
        // Strongly ordered memory will cause a data memory barrier to be issued before and
        // after each memory access and prevents burst accesses.  Audio rendering performance 
        // can be significantly improved by remapping the audio buffer as noncacheable-bufferable 
        // (shared/nonshared device).  
        //
        // Small Page Descriptor:
        //      BITS[3:2] = CB (01b = shared device)
        //
        if (*pVirtAddr)
        {
            VirtualSetAttributes(*pVirtAddr, BSP_AUDIO_DMA_BUF_SIZE, 0x4, 0xC, NULL);
        }
            
        return BSP_AUDIO_DMA_BUF_SIZE;
    }
    else
    {
        return 0;
    }
#else
    // Use dynamically allocated memory for the audio DMA buffer (2 pages).
    static const DMA_BUFFER_REGION_SIZE_NBYTES = AUDIO_DMA_PAGE_SIZE * 2 ;

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
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HardwareContext::BSPDeallocOutputDMABuffer(PBYTE VirtAddr)
{
#ifdef BSP_AUDIO_DMA_BUF_ADDR
    UNREFERENCED_PARAMETER(VirtAddr);
#else
    // Only deallocate the audio DMA buffer memory if it was previously
    // dynamically allocated.
    PHYSICAL_ADDRESS phyAddr;

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Logical address parameter is ignored
    phyAddr.QuadPart = 0;
    HalFreeCommonBuffer(&Adapter, AUDIO_DMA_PAGE_SIZE*2, phyAddr, VirtAddr, FALSE);
#endif
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
UINT16 HardwareContext::BSPAllocInputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr)
{
    // Use dynamically allocated memory for the audio DMA buffer (2 pages).
    static const DMA_BUFFER_REGION_SIZE_NBYTES = AUDIO_DMA_PAGE_SIZE * 2 ;

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
//  Function: BSPDeallocInputDMABuffer
//
//  Deallocate the audio output DMA buffers.
//
//  Parameters:
//      VirtAddr
//          [in] The virtual address of the buffer.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HardwareContext:: BSPDeallocInputDMABuffer(PBYTE VirtAddr)
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

    HalFreeCommonBuffer(&Adapter, AUDIO_DMA_PAGE_SIZE*2, phyAddr, VirtAddr, FALSE);
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAllocOutputDMADescriptor
//
//  Allocate the audio output DMA Descriptor.
//
//  Parameters:
//      pVirtAddr
//          [out] The virtual address of the Descriptor.
//
//      pPhysAddr
//          [out] The physical address of the Descriptor.
//
//  Returns:
//      The buffer size in bytes.
//
//-----------------------------------------------------------------------------
UINT16 HardwareContext::BSPAllocOutputDMADescriptor(PAUDIO_DMA_DESCRIPTOR *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr)
{
#ifdef BSP_AUDIO_DMA_DESCRIPTOR_ADDR
    // Use a statically defined memory region for the audio DMA Descriptor. This
    // may be either internal or external memory depending upon the address
    // region that is selected.
    pPhysAddr->HighPart = 0;
    pPhysAddr->LowPart  = BSP_AUDIO_DMA_DESCRIPTOR_ADDR;
    
    *pVirtAddr = (PAUDIO_DMA_DESCRIPTOR)MmMapIoSpace(*pPhysAddr, BSP_AUDIO_DMA_DESCRIPTOR_SIZE, FALSE);

    // If virtual mapping succeeded and buffer is being allocated from IRAM
    if ((pVirtAddr) && (pPhysAddr->LowPart >= IMAGE_WINCE_IRAM_PA_START) &&
        ((pPhysAddr->LowPart+BSP_AUDIO_DMA_DESCRIPTOR_SIZE) <= (IMAGE_WINCE_IRAM_PA_START+IMAGE_WINCE_IRAM_SIZE)))
    {
        // Default mapping of audio buffer will be noncacheable-nonbufferable (strongly ordered).
        // Strongly ordered memory will cause a data memory barrier to be issued before and
        // after each memory access and prevents burst accesses.  Audio rendering performance 
        // can be significantly improved by remapping the audio buffer as noncacheable-bufferable 
        // (shared/nonshared device).  
        //
        // Small Page Descriptor:
        //      BITS[3:2] = CB (01b = shared device)
        //
        if (*pVirtAddr)
        {
            VirtualSetAttributes(*pVirtAddr, BSP_AUDIO_DMA_DESCRIPTOR_SIZE, 0x4, 0xC, NULL);
        }
    
        return BSP_AUDIO_DMA_DESCRIPTOR_SIZE;
    }else{
        return 0;
    }
#else
    // Use dynamically allocated memory for the audio DMA Descriptor (1 pages).
    static const DMA_BUFFER_REGION_SIZE_NBYTES = AUDIO_DMA_PAGE_SIZE;

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate DMA buffers from external memory
    *pVirtAddr = (PAUDIO_DMA_DESCRIPTOR)HalAllocateCommonBuffer(
        &Adapter,
        DMA_BUFFER_REGION_SIZE_NBYTES,
        pPhysAddr,
        FALSE);

    return DMA_BUFFER_REGION_SIZE_NBYTES;
#endif
}

//-----------------------------------------------------------------------------
//
//  Function: BSPDeallocOutputDMADescriptor
//
//  Deallocate the audio output DMA descriptor buffers.
//
//  Parameters:
//      VirtAddr
//          [in] The virtual address of the buffer.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HardwareContext::BSPDeallocOutputDMADescriptor(PBYTE VirtAddr)
{
#ifdef BSP_AUDIO_DMA_DESCRIPTOR_ADDR
    UNREFERENCED_PARAMETER(VirtAddr);
#else
    // Only deallocate the audio DMA Descriptor buffer memory if it was previously
    // dynamically allocated.
    PHYSICAL_ADDRESS phyAddr;

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Logical address parameter is ignored
    phyAddr.QuadPart = 0;
    HalFreeCommonBuffer(&Adapter, AUDIO_DMA_PAGE_SIZE, phyAddr, VirtAddr, FALSE);
#endif
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAllocInputDMADescriptor
//
//  Allocate the audio Input DMA Descriptor.
//
//  Parameters:
//      pVirtAddr
//          [out] The virtual address of the Descriptor.
//
//      pPhysAddr
//          [out] The physical address of the Descriptor.
//
//  Returns:
//      The buffer size in bytes.
//
//-----------------------------------------------------------------------------
UINT16 HardwareContext::BSPAllocInputDMADescriptor(PAUDIO_DMA_DESCRIPTOR *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr)
{
    // Use dynamically allocated memory for the audio DMA Descriptor (1 pages).
    static const DMA_BUFFER_REGION_SIZE_NBYTES = AUDIO_DMA_PAGE_SIZE;

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate DMA buffers from external memory
    *pVirtAddr = (PAUDIO_DMA_DESCRIPTOR)HalAllocateCommonBuffer(
        &Adapter,
        DMA_BUFFER_REGION_SIZE_NBYTES,
        pPhysAddr,
        FALSE);

    return DMA_BUFFER_REGION_SIZE_NBYTES;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPDeallocInputDMADescriptor
//
//  Deallocate the audio output DMA descriptor buffers.
//
//  Parameters:
//      VirtAddr
//          [in] The virtual address of the buffer.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HardwareContext::BSPDeallocInputDMADescriptor(PBYTE VirtAddr)
{ 
    // Only deallocate the audio DMA Descriptor buffer memory if it was previously
    // dynamically allocated.
    PHYSICAL_ADDRESS phyAddr;

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Logical address parameter is ignored
    phyAddr.QuadPart = 0;
    HalFreeCommonBuffer(&Adapter, AUDIO_DMA_PAGE_SIZE, phyAddr, VirtAddr, FALSE);
 
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSetupOutputDMADescriptor
//
//  Setup the audio output DMA descriptor structure.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::BSPSetupOutputDMADescriptor()
{
    if (!m_DMAOutBufPhysAddr.LowPart)
    {
        ERRORMSG(ZONE_ERROR, (_T("BSPSetupOutputDMADescriptor::Invalid DMA buffer physical address.\r\n")));
        return FALSE;
    }
    if(m_DMAOutDescriptorVirtAddr == NULL)
    {
        ERRMSG("InitOutputDMA: DMA Descriptor allocation failed");
        return FALSE;
    } 

    m_DMAOutDescriptorVirtAddr[BUFFER_A].NextCmdDesc =(DWORD) &m_DMAOutDescriptorPhysAddr[BUFFER_B];
    m_DMAOutDescriptorVirtAddr[BUFFER_A].BufAddr = (DWORD) m_DMAOutBufPhysAddr.LowPart;

    m_DMAOutDescriptorVirtAddr[BUFFER_A].Command = (BF_APBX_CHn_CMD_XFER_COUNT(AUDIO_DMA_PAGE_SIZE) | \
                                                    BF_APBX_CHn_CMD_CMDWORDS(0)                     | \
                                                    BF_APBX_CHn_CMD_WAIT4ENDCMD(0)                  | \
                                                    BF_APBX_CHn_CMD_SEMAPHORE(0)                    | \
                                                    BF_APBX_CHn_CMD_IRQONCMPLT(1)                   | \
                                                    BF_APBX_CHn_CMD_CHAIN(1)                        | \
                                                    BF_APBX_CHn_CMD_COMMAND(BV_APBX_CHn_CMD_COMMAND__DMA_READ));

    m_DMAOutDescriptorVirtAddr[BUFFER_A].PioWord[0] = 0;

    m_DMAOutDescriptorVirtAddr[BUFFER_B].NextCmdDesc = (DWORD) &m_DMAOutDescriptorPhysAddr[BUFFER_A];
    m_DMAOutDescriptorVirtAddr[BUFFER_B].BufAddr = (DWORD) m_DMAOutBufPhysAddr.LowPart + AUDIO_DMA_PAGE_SIZE;

    m_DMAOutDescriptorVirtAddr[BUFFER_B].Command = (BF_APBX_CHn_CMD_XFER_COUNT(AUDIO_DMA_PAGE_SIZE) | \
                                                    BF_APBX_CHn_CMD_CMDWORDS(0)                     | \
                                                    BF_APBX_CHn_CMD_WAIT4ENDCMD(0)                  | \
                                                    BF_APBX_CHn_CMD_SEMAPHORE(0)                    | \
                                                    BF_APBX_CHn_CMD_IRQONCMPLT(1)                   | \
                                                    BF_APBX_CHn_CMD_CHAIN(1)                        | \
                                                    BF_APBX_CHn_CMD_COMMAND(BV_APBX_CHn_CMD_COMMAND__DMA_READ));

    m_DMAOutDescriptorVirtAddr[BUFFER_B].PioWord[0] = 0;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSetupInputDMADescriptor
//
//  Setup the audio input DMA descriptor structure.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::BSPSetupInputDMADescriptor()
{
    if (!m_DMAInBufPhysAddr.LowPart)
    {
        ERRORMSG(ZONE_ERROR, (_T("BSPSetupOutputDMADescriptor::Invalid DMA buffer physical address.\r\n")));
        return FALSE;
    }
    if (!m_DMAINDescriptorVirtAddr)
    {
        ERRMSG("InitOutputDMA: DMA Descriptor allocation failed");
        return FALSE;
    }
    m_DMAINDescriptorVirtAddr[BUFFER_A].NextCmdDesc =(DWORD) &m_DMAInDescriptorPhysAddr[BUFFER_B];
    m_DMAINDescriptorVirtAddr[BUFFER_A].BufAddr = (DWORD) m_DMAInBufPhysAddr.LowPart;

    m_DMAINDescriptorVirtAddr[BUFFER_A].Command = (BF_APBX_CHn_CMD_XFER_COUNT(AUDIO_DMA_PAGE_SIZE) | \
                                                  BF_APBX_CHn_CMD_CMDWORDS(1)                      | \
                                                  BF_APBX_CHn_CMD_WAIT4ENDCMD(0)                   | \
                                                  BF_APBX_CHn_CMD_SEMAPHORE(0)                     | \
                                                  BF_APBX_CHn_CMD_IRQONCMPLT(1)                    | \
                                                  BF_APBX_CHn_CMD_CHAIN(1)                         | \
                                                  BF_APBX_CHn_CMD_COMMAND(BV_APBX_CHn_CMD_COMMAND__DMA_WRITE));

    m_DMAINDescriptorVirtAddr[BUFFER_A].PioWord[0] = (BF_SAIF_CTRL_SFTRST(0)                       | \
                                                      BF_SAIF_CTRL_CLKGATE(0)                      | \
                                                      BF_SAIF_CTRL_BITCLK_MULT_RATE(0)             | \
                                                      BF_SAIF_CTRL_BITCLK_BASE_RATE(0)             | \
                                                      BF_SAIF_CTRL_FIFO_ERROR_IRQ_EN(0)            | \
                                                      BF_SAIF_CTRL_FIFO_SERVICE_IRQ_EN(0)          | \
                                                      BF_SAIF_CTRL_DMAWAIT_COUNT(0)                | \
                                                      BF_SAIF_CTRL_CHANNEL_NUM_SELECT(0)           | \
                                                      BF_SAIF_CTRL_BIT_ORDER(0)                    | \
                                                      BF_SAIF_CTRL_DELAY(1)                        | \
                                                      BF_SAIF_CTRL_JUSTIFY(0)                      | \
                                                      BF_SAIF_CTRL_LRCLK_POLARITY(0)               | \
                                                      BF_SAIF_CTRL_BITCLK_EDGE(0)                  | \
                                                      BF_SAIF_CTRL_WORD_LENGTH(0)                  | \
                                                      BF_SAIF_CTRL_BITCLK_48XFS_ENABLE(0)          | \
                                                      BF_SAIF_CTRL_SLAVE_MODE(1)                   | \
                                                      BF_SAIF_CTRL_READ_MODE(1)                    | \
                                                      BF_SAIF_CTRL_RUN(1));


    m_DMAINDescriptorVirtAddr[BUFFER_B].NextCmdDesc =(DWORD) &m_DMAInDescriptorPhysAddr[BUFFER_A];
    m_DMAINDescriptorVirtAddr[BUFFER_B].BufAddr = (DWORD) m_DMAInBufPhysAddr.LowPart + AUDIO_DMA_PAGE_SIZE;

    m_DMAINDescriptorVirtAddr[BUFFER_B].Command = (BF_APBX_CHn_CMD_XFER_COUNT(AUDIO_DMA_PAGE_SIZE) | \
                                                   BF_APBX_CHn_CMD_CMDWORDS(1)                     | \
                                                   BF_APBX_CHn_CMD_WAIT4ENDCMD(0)                  | \
                                                   BF_APBX_CHn_CMD_SEMAPHORE(0)                    | \
                                                   BF_APBX_CHn_CMD_IRQONCMPLT(1)                   | \
                                                   BF_APBX_CHn_CMD_CHAIN(1)                        | \
                                                   BF_APBX_CHn_CMD_COMMAND(BV_APBX_CHn_CMD_COMMAND__DMA_WRITE));

    m_DMAINDescriptorVirtAddr[BUFFER_B].PioWord[0] = (BF_SAIF_CTRL_SFTRST(0)                       | \
                                                      BF_SAIF_CTRL_CLKGATE(0)                      | \
                                                      BF_SAIF_CTRL_BITCLK_MULT_RATE(0)             | \
                                                      BF_SAIF_CTRL_BITCLK_BASE_RATE(0)             | \
                                                      BF_SAIF_CTRL_FIFO_ERROR_IRQ_EN(0)            | \
                                                      BF_SAIF_CTRL_FIFO_SERVICE_IRQ_EN(0)          | \
                                                      BF_SAIF_CTRL_DMAWAIT_COUNT(0)                | \
                                                      BF_SAIF_CTRL_CHANNEL_NUM_SELECT(0)           | \
                                                      BF_SAIF_CTRL_BIT_ORDER(0)                    | \
                                                      BF_SAIF_CTRL_DELAY(1)                        | \
                                                      BF_SAIF_CTRL_JUSTIFY(0)                      | \
                                                      BF_SAIF_CTRL_LRCLK_POLARITY(0)               | \
                                                      BF_SAIF_CTRL_BITCLK_EDGE(0)                  | \
                                                      BF_SAIF_CTRL_WORD_LENGTH(0)                  | \
                                                      BF_SAIF_CTRL_BITCLK_48XFS_ENABLE(0)          | \
                                                      BF_SAIF_CTRL_SLAVE_MODE(1)                   | \
                                                      BF_SAIF_CTRL_READ_MODE(1)                    | \
                                                      BF_SAIF_CTRL_RUN(1));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAudioPowerUp
//
//  This function powers up the Audio module.
//
//  Parameters:
//      fullPowerUp
//          [in] Flag of full power-up.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HardwareContext::BSPAudioPowerUp(const BOOL fullPowerUp)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(fullPowerUp);

    // Not enter suspend, return with doing nothing
    if(!EnterSuspend)    return;

    EnterSuspend = 0;


    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_CODEC_ON);

}

//-----------------------------------------------------------------------------
//
//  Function: BSPAudioPowerDown
//
//  This function powers down the Audio module.
//
//  Parameters:
//      fullPowerOff
//          [in] Flag of full power-down.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HardwareContext:: BSPAudioPowerDown(const BOOL fullPowerOff)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(fullPowerOff);

    // If 5v is present, not enter suspend mode.
    if((HW_POWER_STS_RD() & BM_POWER_STS_VBUSVALID0))  return;

    g_pSGTL5000Codec->SGTL5000ConfigurePowerMode(POWER_MODE_CODEC_OFF);


    EnterSuspend = 1;

}

//-----------------------------------------------------------------------------
//
//  Function: BSPSetSpeakerEnable
//
//  the WaveDev2 architecture can send a waveoutmessage to enable the
//  audio output stream to play out of the external speaker.  This function sets
//  a flag on codec to indicate whether MM_WOM_FORCESPEAKER flag is set or not 
//
//  Parameters:
//      bEnable
//          [in] enable/disable Speaker
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext:: BSPSetSpeakerEnable(BOOL bEnable)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(bEnable);

}

//-----------------------------------------------------------------------------
//
//  Function: BSPIsHeadphoneDetectSupported
//
//  This function returns whether the platform supports HP detection
//
//  Parameters:
//      None
//
//  Returns:
//      Bool - True or False
//
//-----------------------------------------------------------------------------
BOOL HardwareContext:: BSPIsHeadphoneDetectSupported()
{
    return TRUE;
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
DWORD HardwareContext:: BSPGetHPDetectIRQ()
{
    return IRQ_GPIO2_PIN14;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioHandleHPDetectIRQ
//
//  This function handles the HP detect IRQ on the GPIO pin.  It implements the
//  following:
//  1. Checks interrupt status (ISR) to see if the GPIO IRQ event has happened
//  2. If yes, read the state of the GPIO pin.  If low, HP is plugged (value = 0)
//     If hign, HP is not plugged (value = 1)
//  3. Set the HP detect value on the codec object which will handle it appropriately
//  4. Clear the ISR

//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HardwareContext:: BSPAudioHandleHPDetectIRQ(VOID)
{
    UINT32 HPDetected, status;

    // Read the interrupt status register associated with GPIO2_14
    DDKGpioReadIntrPin(DDK_IOMUX_SSP1_D0_1, &status);

    // If GPIO2_14 Intr comes
    if(status)
    {
        // Read the value on the GPIO pin
        DDKGpioReadDataPin(DDK_IOMUX_SSP1_D0_1, &HPDetected);       

        HPDetected = !HPDetected;
        
        // Set the value on the codec object which will handle it appropriately
        g_pSGTL5000Codec->SGTL5000HPDetected(HPDetected);

        // clears the interrupt status register
        DDKGpioClearIntrPin(DDK_IOMUX_SSP1_D0_1);
    }
    // Audio playback will be done via speakers or headphones based on 
    // MM_WOM_FORCESPEAKER flag and whether the Headphones are plugged in
    g_pSGTL5000Codec->SGTL5000SpeakerHeadphoneSelection();

}

//-----------------------------------------------------------------------------
//
//  Function: BSPAudioIomuxConfig
//
//  This function is used to configure the GPIO.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HardwareContext:: BSPAudioIomuxConfig()
{
    DDK_GPIO_CFG intrCfg;

    // SAIF0_SDATA0 -- OUTPUT, codec I2S_DIN
    DDKIomuxSetPinMux(DDK_IOMUX_SAIF0_SDATA0, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SAIF0_SDATA0,
                         DDK_IOMUX_PAD_DRIVE_12MA,
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SAIF1_SDATA0, -- INPUT, codec I2S_DOUT
    DDKIomuxSetPinMux(DDK_IOMUX_SAIF1_SDATA0_1, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SAIF1_SDATA0_1,
                         DDK_IOMUX_PAD_DRIVE_12MA,
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SAIF0_ALT_BITCLK 
    DDKIomuxSetPinMux(DDK_IOMUX_SAIF0_BITCLK, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SAIF0_BITCLK,
                         DDK_IOMUX_PAD_DRIVE_12MA,
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SAIF0_LRCLK
    DDKIomuxSetPinMux(DDK_IOMUX_SAIF0_LRCLK, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SAIF0_LRCLK,
                         DDK_IOMUX_PAD_DRIVE_12MA,
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SAIF0_MCLK_BITCLK
    DDKIomuxSetPinMux(DDK_IOMUX_SAIF0_MCLK, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SAIF0_MCLK,
                         DDK_IOMUX_PAD_DRIVE_12MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    
    // Headphone detect pin -- GPIO2_14, Low active
    DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D0_1, DDK_IOMUX_MODE_GPIO);    
    
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D0_1,
                         DDK_IOMUX_PAD_DRIVE_12MA,
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    
    intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
    intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
    intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;
    intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;
    intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_LO;

    if(!DDKGpioConfig(DDK_IOMUX_SSP1_D0_1, intrCfg))
    {
        ERRORMSG(1,(TEXT("SAIF: Failed to config HP(LINE1) detect pin!!!\r\n")));
        return;
    }

}

