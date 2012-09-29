//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  hwctxt.cpp
//
//  Provides BSP-specific configuration routines for the SPDIF driver.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4127 4201)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include "wavemain.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables

#ifdef DEBUG

extern DBGPARAM dpCurSettings;

#endif // #ifdef DEBUG


//-----------------------------------------------------------------------------
// Defines                

// Size in bytes of each DMA buffer. We allocate 2 DMA buffers for SPDIF playback
#define SPDIF_DMA_PAGE_SIZE         4096

#define SPDIF_SFCSR_TX_WATERMARK    4    // 2*(rightFIFO+leftFIFO)


//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

UINT16 SPDIFDMAPageSize = SPDIF_DMA_PAGE_SIZE;

//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
//
//  Function: BSPInit
//
//  This function maps the peripheral registers of the SPDIF devices for
//  direct access by the driver.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::BSPInit()
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    DEBUGMSG(ZONE_FUNCTION, (_T("+HardwareContext::BSPInit\r\n")));

    // Start by getting access to the SPDIF hardware control registers.
    phyAddr.QuadPart = CSP_BASE_REG_PA_SPDIF;

    // Map peripheral physical address to virtual address.
    m_pSpdifReg = (PSPDIF_REG) MmMapIoSpace(phyAddr, sizeof(SPDIF_REG), FALSE);

    // Check if the virtual mapping failed.
    if (m_pSpdifReg == NULL)
    {
        ERRORMSG(ZONE_ERROR, (_T("MapRegisters:  MmMapIoSpace failed for SPDIF reg \r\n")));
        goto cleanUp;
    }


    // PowerUP hardware
    BSPSPDIFPowerUp();
    
    BSPSPDIFPowerDown();

    rc = TRUE;

cleanUp:
    if (!rc) BSPDeinit();

    DEBUGMSG(ZONE_FUNCTION, (_T("-HardwareContext::BSPInit\r\n")));

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPRequestIrqs
//
//  This function request the IRQ for SPDIF. For data driven by SDMA, so
//  SPDIF IRQ need not be registered.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::BSPRequestIrqs(VOID)
{
    // Call the OAL to translate the SPDIF IRQ into a SYSINTR value.
    UINT32 aIrqs[5];
    int i = 2;
    aIrqs[0] = (UINT32) -1;
    aIrqs[1] = 0;
    aIrqs[i++] = IRQ_SPDIF_DMA;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(UINT32) * i,
                         &m_dwSysintrSPDIF, sizeof(UINT32), NULL))
    {
        ERRORMSG(ZONE_ERROR, (TEXT("ERROR: HardwareContext::BSPRequestIrqs: Failed ")
                              TEXT("to obtain sysintr value for SPDIF ")
                              TEXT("interrupt.\r\n")));
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPDeinit
//
//  This function unmaps the peripheral registers previously mapped with
//  the MapRegisters function.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::BSPDeinit()
{
    DEBUGMSG(ZONE_FUNCTION, (_T("+HardwareContext::BSPDeinit\r\n")));

    // Unmap SPDIF peripheral registers
    if (m_pSpdifReg)
    {
        MmUnmapIoSpace(m_pSpdifReg, sizeof(SPDIF_REG));
        m_pSpdifReg = NULL;
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("-HardwareContext::BSPDeinit\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAllocDMABuffers
//
//  Allocate the SPDIF DMA buffers.
//
//  Parameters:
//      pVirtAddr
//          [out] Virtuall address of allocated DMA buffers.
//
//      pAddress
//          [out] Physical address of allocated DMA buffers.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::BSPAllocDMABuffers(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pAddress)
{
#ifdef BSP_SPDIF_DMA_BUF_ADDR

    // Use a statically defined memory region for the SPDIF DMA buffers. This
    // may be either internal or external memory depending upon the address
    // region that is selected.
    pAddress->HighPart = 0;
    pAddress->LowPart  = BSP_SPDIF_DMA_BUF_ADDR;

    *pVirtAddr = (PBYTE)MmMapIoSpace(*pAddress, BSP_SPDIF_DMA_BUF_SIZE, FALSE);

    // If virtual mapping succeeded and buffer is being allocated from IRAM
    if ((pVirtAddr) && (pAddress->LowPart >= IMAGE_WINCE_IRAM_PA_START) &&
        ((pAddress->LowPart+BSP_SPDIF_DMA_BUF_SIZE) <= (IMAGE_WINCE_IRAM_PA_START+IMAGE_WINCE_IRAM_SIZE)))
    {
        // Hardware constraints require IRAM to be accessed with AHB or
        // we can incur errors on subsequent accesses to DRAM.  Update
        // the MMU attributes to use backwards-compatible extended
        // small page format.  This format allows us to set the TEX bits
        // to mark the IRAM region for non-shared device (directs
        // access to AHB)
        //
        // Extended Small Page Format
        // --------------------------
        // BITS[3:2] = CB (01b = shared device)
        //
        if (*pVirtAddr)
        {
            VirtualSetAttributes(*pVirtAddr, BSP_SPDIF_DMA_BUF_SIZE, 0x4, 0xC, NULL);
        }

        SPDIFDMAPageSize = BSP_SPDIF_DMA_BUF_SIZE/2;
    }

    return;
#else
    // Use dynamically allocated memory for the SPDIF DMA buffer (2 pages).
    static const DMA_BUFFER_REGION_SIZE_NBYTES = SPDIF_DMA_PAGE_SIZE * 2 ;

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate DMA buffers from external memory
    *pVirtAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter,
                                          DMA_BUFFER_REGION_SIZE_NBYTES,
                                          pAddress,
                                          FALSE);

    SPDIFDMAPageSize = SPDIF_DMA_PAGE_SIZE;

    return;
#endif
}

//-----------------------------------------------------------------------------
//
//  Function: BSPDeallocDMABuffers
//
//  Deallocate the SPDIF DMA buffers that were previously allocated by calling
//  BSPAllocDMABuffers().
//
//  Parameters:
//      virtualAddress - virtual address of allocated DMA buffers
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::BSPDeallocDMABuffers(PVOID virtualAddress)
{
#ifdef BSP_SPDIF_DMA_BUF_ADDR
    UNREFERENCED_PARAMETER(virtualAddress);
#else

    int  DMA_BUFFER_REGION_SIZE_NBYTES = 0;

    // If SPDIF RX enabled
    if (m_pSpdifRxHwContext)
        DMA_BUFFER_REGION_SIZE_NBYTES = SPDIFDMAPageSize * 2;

    // If SPDIF TX enabled
    if (m_pSpdifTxHwContext)
        DMA_BUFFER_REGION_SIZE_NBYTES += SPDIFDMAPageSize * 2;

    // Only deallocate the SPDIF DMA buffer memory if it was previously
    // dynamically allocated.
    PHYSICAL_ADDRESS phyAddr;

    // Logical address parameter is ignored
    phyAddr.QuadPart = 0;
    
    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    HalFreeCommonBuffer(&Adapter, DMA_BUFFER_REGION_SIZE_NBYTES, phyAddr, virtualAddress, FALSE);
#endif
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSPDIFPowerUp
//
//  This function powers up the SPDIF chip.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::BSPSPDIFPowerUp()
{
    DEBUGMSG(ZONE_FUNCTION, (_T("+HardwareContext::BSPSPDIFPowerUp\r\n")));

    // Open SPDIF clock
    DDKClockSetGatingMode(DDK_CLOCK_GATE_SPDIF_CLK, FALSE);

    // Reset SPDIF module
    HW_SPDIF_CTRL_CLR(BM_SPDIF_CTRL_SFTRST);

    HW_SPDIF_CTRL_CLR(BM_SPDIF_CTRL_CLKGATE);
    
    m_bPowerdown = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (_T("-HardwareContext::BSPSPDIFPowerUp\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSPDIFPowerDown
//
//  This function powers down the SPDIF chip.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::BSPSPDIFPowerDown()
{
    DEBUGMSG(ZONE_FUNCTION, (_T("+HardwareContext::BSPSPDIFPowerDown\r\n")));

    if(m_bPowerdown)
        return;

    // Gate off SPDIF clock
    DDKClockSetGatingMode(DDK_CLOCK_GATE_SPDIF_CLK, TRUE);

    HW_SPDIF_CTRL_SET(BM_SPDIF_CTRL_CLKGATE);

    m_bPowerdown = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (_T("-HardwareContext::BSPSPDIFPowerDown\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSPDIFGetConfig
//
//  This function Indicate the platform SPDIF's capability
//
//  Parameters:
//      pConfig -- Indicate the platform SPDIF's capability and configuration.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::BSPSPDIFGetConfig(UINT32 *pConfig)
{
    *pConfig |= SPDIF_CONFIG_TX_ENABLED;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSPDIFStartOutput
//
//  This function start the SPDIF output
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::BSPSPDIFStartOutput(void)
{   
    CCHANNEL channel;

    DEBUGMSG(ZONE_FUNCTION, (_T("+SpdifTxHwContext::BSPSPDIFStartOutput\r\n")));

    // write channel data (consumer mode)
    memset(&channel, 0, sizeof(channel));
    channel.H.Ctrl.ChannelStatus = IEC958_CON_CTRL_CONS; //Consumer use of channel status block
    if (m_WaveFormat.wFormatTag == WAVE_FORMAT_PCM) 
        channel.H.Ctrl.AudioFormat   = IEC958_CON_AUDIO_PCM;
    else //SPDIF_AUDIO_TYPE_COMPRESSED
        channel.H.Ctrl.AudioFormat   = IEC958_CON_AUDIO_COMPRESSED;    
    channel.H.Ctrl.Copyright     = IEC958_CON_COPYRIGHT_NONE;
    channel.H.Ctrl.AddInfo       = IEC958_CON_EMPHASIS_5015;
    channel.H.Ctrl.ChannelMode   = IEC958_CON_CHANNEL_MODE0;
    //channel.H.Ctrl.CategoryCode = 
    channel.H.Ctrl.SourceNumber  = IEC958_CON_SOURCE_NUMBER_2;
    //channel.H.Ctrl.ChannelNumber =

    channel.L.Ctrl.ClockAccuracy = IEC958_CON_CLOCK_ACCURACY_LEVEL2;
    
    if (m_WaveFormat.nSamplesPerSec == 44100)
        channel.L.Ctrl.SampleFreq    = IEC958_CON_SAMPLE_FREQ_44100;
    else if (m_WaveFormat.nSamplesPerSec == 32000)
        channel.L.Ctrl.SampleFreq    = IEC958_CON_SAMPLE_FREQ_32000;
    else //48K
        channel.L.Ctrl.SampleFreq    = IEC958_CON_SAMPLE_FREQ_48000;

    if (m_WaveFormat.wBitsPerSample == 16)
    {
        channel.L.Ctrl.MaxLenth      = IEC958_CON_MAX_LENGTH_20;
        channel.L.Ctrl.Samplenth     = IEC958_CON_SAMPLE_LENGTH_20_16; //IEC958_CON_SAMPLE_LENGTH_20_16;
    }
    else //wBitsPerSample ==  24. 32
    {
        channel.L.Ctrl.MaxLenth      = IEC958_CON_MAX_LENGTH_24;
        channel.L.Ctrl.Samplenth     = IEC958_CON_SAMPLE_LENGTH_24_20; //IEC958_CON_SAMPLE_LENGTH_24_20;
    }

    HW_SPDIF_FRAMECTRL_SET(BF_SPDIF_FRAMECTRL_PRO(m_CChanel.H.Ctrl.ChannelStatus)|
                           BF_SPDIF_FRAMECTRL_AUDIO(m_CChanel.H.Ctrl.AudioFormat)|
                           BF_SPDIF_FRAMECTRL_COPY(m_CChanel.H.Ctrl.Copyright)   |
                           BF_SPDIF_FRAMECTRL_PRE(m_CChanel.H.Ctrl.AddInfo)      |
                           BF_SPDIF_FRAMECTRL_CC(m_CChanel.H.Ctrl.CategoryCode));



    m_CChanel = channel;

    //Config samplerate
    if (m_WaveFormat.nSamplesPerSec == 44100)
    {
        HW_SPDIF_SRR_WR(BF_SPDIF_SRR_BASEMULT(1)|0x0AC44);
    }
    else if (m_WaveFormat.nSamplesPerSec == 32000)
    {
        HW_SPDIF_SRR_WR(BF_SPDIF_SRR_BASEMULT(1)|0x07D00); 
    }
    else // 48K
    {
        HW_SPDIF_SRR_WR(BF_SPDIF_SRR_BASEMULT(1)|0x0BB80);
    }

    //SPDIF CONTROL Register
    HW_SPDIF_CTRL_SET(BF_SPDIF_CTRL_RUN(1));

#ifdef SPDIF_DEBUG
    m_pHardwareContext->DumpHwRegs();
#endif

    DEBUGMSG(ZONE_FUNCTION, (_T("-SpdifTxHwContext::BSPSPDIFStartOutput\n")));

    return TRUE;

}

//-----------------------------------------------------------------------------
//
//  Function: BSPInitOutput
//
//  This function init the SPDIF output
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::BSPInitOutput(void)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("+SpdifTxHwContext::BSPInitOutput\r\n")));

    m_OutputDMALevel = SPDIF_SFCSR_TX_WATERMARK; 

    // Config SPDIF IOMUX
    DDKIomuxSetPinMux(DDK_IOMUX_SPDIF_TX, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SPDIF_TX, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DEBUGMSG(ZONE_FUNCTION, (_T("-SpdifTxHwContext::BSPInitOutput\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSPDIFStopOutput
//
//  This function stop the SPDIF output
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifTxHwContext::BSPSPDIFStopOutput(void)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("+SpdifTxHwContext::BSPSPDIFStopOutput\r\n")));

    // Stop SPDIF
    HW_SPDIF_CTRL_CLR(BF_SPDIF_CTRL_RUN(1));

#ifdef SPDIF_DEBUG
    m_pHardwareContext->DumpHwRegs();
#endif
    DEBUGMSG(ZONE_FUNCTION, (_T("-SpdifTxHwContext::BSPSPDIFStopOutput\r\n")));

    return TRUE;

}

BOOL SpdifTxHwContext::BSPSetupOutputDMADescriptor(PHYSICAL_ADDRESS pAddress)
{
    if (!pAddress.LowPart)
    {
        ERRORMSG(ZONE_ERROR, (_T("BSPSetupOutputDMADescriptor::Invalid DMA buffer physical address.\r\n")));
        return FALSE;
    }
    if(m_DMAOutDescriptorVirtAddr == NULL)
    {
        ERRMSG("InitOutputDMA: DMA Descriptor allocation failed");
        return FALSE;
    }
    m_DMAOutDescriptorVirtAddr[OUT_BUFFER_A].NextCmdDesc =(DWORD) &m_DMAOutDescriptorPhysAddr[OUT_BUFFER_B];
    m_DMAOutDescriptorVirtAddr[OUT_BUFFER_A].BufAddr = (DWORD) pAddress.LowPart;

    m_DMAOutDescriptorVirtAddr[OUT_BUFFER_A].Command = (BF_APBX_CHn_CMD_XFER_COUNT(SPDIF_DMA_PAGE_SIZE) | \
                                                        BF_APBX_CHn_CMD_CMDWORDS(1)                     | \
                                                        BF_APBX_CHn_CMD_WAIT4ENDCMD(0)                  | \
                                                        BF_APBX_CHn_CMD_SEMAPHORE(0)                    | \
                                                        BF_APBX_CHn_CMD_IRQONCMPLT(1)                   | \
                                                        BF_APBX_CHn_CMD_CHAIN(1)                        | \
                                                        BF_APBX_CHn_CMD_COMMAND(BV_APBX_CHn_CMD_COMMAND__DMA_READ));

    m_DMAOutDescriptorVirtAddr[OUT_BUFFER_A].PioWord[0] = (BF_SPDIF_CTRL_SFTRST(0)                      | \
                                                           BF_SPDIF_CTRL_CLKGATE(0)                     | \
                                                           BF_SPDIF_CTRL_DMAWAIT_COUNT(0)               | \
                                                           BF_SPDIF_CTRL_WAIT_END_XFER(1)               | \
                                                           BF_SPDIF_CTRL_WORD_LENGTH(0)                 | \
                                                           BF_SPDIF_CTRL_FIFO_UNDERFLOW_IRQ(0)          | \
                                                           BF_SPDIF_CTRL_FIFO_OVERFLOW_IRQ(0)           | \
                                                           BF_SPDIF_CTRL_FIFO_ERROR_IRQ_EN(0)           | \
                                                           BF_SPDIF_CTRL_RUN(1));

    m_DMAOutDescriptorVirtAddr[OUT_BUFFER_B].NextCmdDesc = (DWORD) &m_DMAOutDescriptorPhysAddr[OUT_BUFFER_A];
    m_DMAOutDescriptorVirtAddr[OUT_BUFFER_B].BufAddr = (DWORD) pAddress.LowPart + SPDIF_DMA_PAGE_SIZE;

    m_DMAOutDescriptorVirtAddr[OUT_BUFFER_B].Command = (BF_APBX_CHn_CMD_XFER_COUNT(SPDIF_DMA_PAGE_SIZE) | \
                                                        BF_APBX_CHn_CMD_CMDWORDS(1)                     | \
                                                        BF_APBX_CHn_CMD_WAIT4ENDCMD(0)                  | \
                                                        BF_APBX_CHn_CMD_SEMAPHORE(0)                    | \
                                                        BF_APBX_CHn_CMD_IRQONCMPLT(1)                   | \
                                                        BF_APBX_CHn_CMD_CHAIN(1)                        | \
                                                        BF_APBX_CHn_CMD_COMMAND(BV_APBX_CHn_CMD_COMMAND__DMA_READ));

    m_DMAOutDescriptorVirtAddr[OUT_BUFFER_B].PioWord[0] = (BF_SPDIF_CTRL_SFTRST(0)                      | \
                                                           BF_SPDIF_CTRL_CLKGATE(0)                     | \
                                                           BF_SPDIF_CTRL_DMAWAIT_COUNT(0)               | \
                                                           BF_SPDIF_CTRL_WAIT_END_XFER(1)               | \
                                                           BF_SPDIF_CTRL_WORD_LENGTH(0)                 | \
                                                           BF_SPDIF_CTRL_FIFO_UNDERFLOW_IRQ(0)          | \
                                                           BF_SPDIF_CTRL_FIFO_OVERFLOW_IRQ(0)           | \
                                                           BF_SPDIF_CTRL_FIFO_ERROR_IRQ_EN(0)           | \
                                                           BF_SPDIF_CTRL_RUN(1));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPAllocOutputDMADescriptor
//
//  Allocate the SPDIF output DMA Descriptor.
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
UINT16 SpdifTxHwContext::BSPAllocOutputDMADescriptor(PAUDIO_DMA_DESCRIPTOR *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr)
{
#ifdef BSP_SPDIF_DMA_DESCRIPTOR_ADDR
    // Use a statically defined memory region for the SPDIF DMA Descriptor. This
    // may be either internal or external memory depending upon the address
    // region that is selected.
    pPhysAddr->HighPart = 0;
    pPhysAddr->LowPart  = BSP_SPDIF_DMA_DESCRIPTOR_ADDR;

    *pVirtAddr = (PAUDIO_DMA_DESCRIPTOR)MmMapIoSpace(*pPhysAddr, BSP_SPDIF_DMA_DESCRIPTOR_SIZE, FALSE);

    // If virtual mapping succeeded and buffer is being allocated from IRAM
    if ((pVirtAddr) && (pPhysAddr->LowPart >= IMAGE_WINCE_IRAM_PA_START) &&
        ((pPhysAddr->LowPart+BSP_SPDIF_DMA_DESCRIPTOR_SIZE) <= (IMAGE_WINCE_IRAM_PA_START+IMAGE_WINCE_IRAM_SIZE)))
    {
        // Hardware constraints require IRAM to be accessed with AHB or
        // we can incur errors on subsequent accesses to DRAM.  Update
        // the MMU attributes to use backwards-compatible extended
        // small page format.  This format allows us to set the TEX bits
        // to mark the IRAM region for non-shared device (directs
        // access to AHB)
        //
        // Extended Small Page Format
        // --------------------------
        // BITS[3:2] = CB (01b = shared device)
        //        
        if (*pVirtAddr)
        {
            VirtualSetAttributes(*pVirtAddr, BSP_SPDIF_DMA_DESCRIPTOR_SIZE, 0x4, 0xC, NULL);
        }

        return BSP_SPDIF_DMA_DESCRIPTOR_SIZE;
    }
    else{
        return 0;
    }
#else
    // Use dynamically allocated memory for the SPDIF DMA Descriptor (1 pages).
    static const DMA_BUFFER_REGION_SIZE_NBYTES = SPDIF_DMA_PAGE_SIZE;

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
//  Function: BSPDeallocOutputDMABuffer
//
//  Deallocate the SPDIF output DMA buffers.
//
//  Parameters:
//      VirtAddr
//          [in] The virtual address of the buffer.
//
//  Returns:
//      The buffer size in bytes.
//
//-----------------------------------------------------------------------------
VOID SpdifTxHwContext::BSPDeallocOutputDMADescriptor(PAUDIO_DMA_DESCRIPTOR VirtAddr)
{
#ifdef BSP_SPDIF_DMA_DESCRIPTOR_ADDR
        UNREFERENCED_PARAMETER(VirtAddr);
#else
    // deallocate dynamically allocated memory for the SPDIF DMA Descriptor (1 pages).
    PHYSICAL_ADDRESS phyAddr;
    int  DMA_BUFFER_REGION_SIZE_NBYTES = 0;

    DMA_BUFFER_REGION_SIZE_NBYTES = SPDIF_DMA_PAGE_SIZE;

    DMA_ADAPTER_OBJECT Adapter;
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    // Logical address parameter is ignored
    phyAddr.QuadPart = 0;
    HalFreeCommonBuffer(&Adapter, DMA_BUFFER_REGION_SIZE_NBYTES, phyAddr, VirtAddr, FALSE);
#endif
}

//-----------------------------------------------------------------------------
//
//  Function: BSPGetTXDMAChannel
//
//  Get SPDIF TX DMA Channel number.
//
//  Parameters:
//      None.
//
//  Returns:
//      DMA Channel number.
//
//-----------------------------------------------------------------------------
UINT8 SpdifTxHwContext::BSPGetTXDMAChannel(void)
{
    return APBX_CHANNEL_SPDIF_TX;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSPDIFStartInput
//
//  This function start the SPDIF input
//  input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
// Stub since MX51 does not have SPDIF RX function
BOOL SpdifRxHwContext::BSPSPDIFStartInput(void)
{
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPSPDIFStopInput
//
//  This function stop the SPDIF input
//  input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifRxHwContext::BSPSPDIFStopInput(void)
{
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPInitInput
//
//  This function init the SPDIF input
//  input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SpdifRxHwContext::BSPInitInput()
{
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPGetFreqMesa
//
//  This function get the frequency of SPDIF input
//  input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns the frequency of SPDIF input. 0 indicate either input not run 
//      or hardware is not ready.
//
//-----------------------------------------------------------------------------
UINT32 SpdifRxHwContext::BSPGetFreqMesa(void)
{
    return 0;
}
