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
//  Copyright (C) 2008 Freescale Semiconductor, Inc. All Rights Reserved.
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
extern UINT32 GetESAIBaseRegAddr(UINT32 index);
extern DWORD GetSdmaChannelIRQ(UINT32 chan);

extern UINT8 BSPAudioGetInputDeviceNum();
extern UINT16 BSPAllocOutputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr);
extern UINT16 BSPAllocInputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr);
extern VOID BSPDeallocOutputDMABuffer(PBYTE VirtAddr,PHYSICAL_ADDRESS phyAddr);
extern VOID BSPDeallocInputDMABuffer(PBYTE VirtAddr,PHYSICAL_ADDRESS phyAddr);
extern BOOL BSPAudioInitCodec();
extern VOID BSPAudioDeinitCodec();
extern VOID BSPAudioStartDAC();
extern VOID BSPAudioStopDAC();
extern VOID BSPAudioStartADC();
extern VOID BSPAudioStopADC();
extern VOID BSPAudioSetOutputGain(DWORD dwGain);
extern VOID BSPAudioSetInputGain(DWORD dwGain);
extern VOID BSPAudioIomuxConfig();
extern BOOL BSPESAIEnableClock(UINT32 index, BOOL bEnable);
extern UINT8 BSPAudioGetSdmaPriority();
extern VOID BSPAudioPowerUp(const BOOL fullPowerUp);
extern VOID BSPAudioPowerDown(const BOOL fullPowerOff);
extern AUDIO_PROTOCOL BSPAudioGetOutputProtocol();
extern BOOL BSPIsSupportedOutputFormat(PWAVEFORMATEXTENSIBLE pFmtEx);
extern BOOL BSPAudioConfigDAC(DWORD dwSampleRate, DWORD dwBitDepth, 
    DWORD dwChnMask,DWORD dwChnNum, AUDIO_PROTOCOL audioProtocol);
extern UINT32 BSPAudioGetTxBitClock(void);
extern AUDIO_PROTOCOL BSPAudioGetInputProtocol();
extern BOOL BSPIsSupportedInputFormat(PWAVEFORMATEXTENSIBLE pFmtEx);
extern BOOL BSPAudioConfigADC(DWORD dwSampleRate, DWORD dwBitDepth, 
    DWORD dwChnMask,DWORD dwChnNum, AUDIO_PROTOCOL audioProtocol);



//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

//----- Used to track DMA controllers status -----
#define DMA_CLEAR           0x00000000
#define DMA_DONEA           0x00000002
#define DMA_DONEB           0x00000004

//----- Used for scheduling DMA transfers -----
#define BUFFER_A            0
#define BUFFER_B            1

#define ESAI_TX_WATERMAKR   60
#define ESAI_RX_WATERMAKR   60

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
HardwareContext *g_pHWContext=NULL;

//-----------------------------------------------------------------------------
// Local Variables
static UINT32 PlaybackDisableDelayMsec = 1;//10;
static UINT32 RecordDisableDelayMsec = 10;

static BOOL g_saveOutputDMARunning = FALSE;
static BOOL g_saveInputDMARunning  = FALSE;


static const WCHAR c_szESAIKey[]   = L"Drivers\\BuiltIn\\ESAI";
static const WCHAR c_szAsrcSupport[] = L"AsrcSupport";
static const WCHAR c_szAsrcPairIndex[] = L"AsrcPairIndex";
static const WCHAR c_szAsrcMaxChnNum[] = L"AsrcMaxChnNum";


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
    m_DriverIndex       = Index;
    m_InPowerHandler    = FALSE;
    m_InputDMARunning   = FALSE;
    m_OutputDMARunning  = FALSE;
    //m_NumForcedSpeaker  = 0;

    m_dwOutputGain = 0xFFFFFFFF;
    m_dwInputGain  = 0xFFFFFFFF;
    m_fInputMute  = FALSE;
    m_fOutputMute = FALSE;

    m_OutputDMAStatus = DMA_CLEAR;
    m_InputDMAStatus  = DMA_CLEAR;
    
    m_bCanStopOutputDMA   = TRUE;
    m_bESAITXUnderrunError = FALSE;
    m_bESAIRXOverrunError = FALSE;
    m_delayBypass         = FALSE;

    m_bCodecLoopbackState = FALSE;

    m_bAsrcEnable = FALSE;
    m_bWaveOutputFormatInitialed = FALSE;
    m_bWaveInputFormatInitialed = FALSE;

    m_bSupportWAVEFORMATEX = (BOOL)GetDriverRegValue(L"SupportWAVEFORMATEX", 0);
    //m_bSupportWAVEFORMATEX = TRUE;
    m_InputDeviceContext.SetBaseSampleRate(INPUT_SAMPLERATE);
    m_OutputDeviceContext.SetBaseSampleRate(OUTPUT_SAMPLERATE);

    //
    // Initialize hardware
    //
    PHYSICAL_ADDRESS phyAddr;

    // Map audio peripherals
    phyAddr.QuadPart = GetESAIBaseRegAddr(0);

     
    m_pESAI = (PCSP_ESAI_REG) MmMapIoSpace(phyAddr, sizeof(CSP_ESAI_REG), FALSE);

    if (m_pESAI == NULL)
    {
        ERRMSG("MmMapIoSpace failed for ESAI");
        goto exit;
    }

    // Initialize audio Codec
    if (!BSPAudioInitCodec())
    {
        ERRMSG("Initialize Audio Codec failed");
        goto exit;
    }

    // Initialize ESAI
    InitESAI();

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

    InitAsrcP2P();
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

    DeinitAsrcP2P();
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

    // Free DMA buffers
    BSPDeallocOutputDMABuffer(m_DMAOutBufVirtAddr,m_DMAOutBufPhysAddr);
    BSPDeallocInputDMABuffer(m_DMAInBufVirtAddr,m_DMAInBufPhysAddr);

    // Deinit audio codec
    BSPAudioDeinitCodec();

    // Unmap audio peripherals
    if (m_pESAI)
    {
        MmUnmapIoSpace(m_pESAI, sizeof(CSP_ESAI_REG));
        m_pESAI = NULL;
    }

    return TRUE;
}



void HardwareContext::DumpESAIRegs(void)
{

#ifdef ESAI_DEBUG
    //OUTREG32(&m_pESAI->ECR,0x00000001);
    RETAILMSG(TRUE,(TEXT("ESAI dump regs +++++++++++++++++++\r\n")));
    RETAILMSG(TRUE,(TEXT("ECR:%x"),INREG32(&m_pESAI->ECR)));
    RETAILMSG(TRUE,(TEXT("ESR:%x"),INREG32(&m_pESAI->ESR)));
    RETAILMSG(TRUE,(TEXT("PCRC:%x"),INREG32(&m_pESAI->PCRC)));
    RETAILMSG(TRUE,(TEXT("PDRC:%x"),INREG32(&m_pESAI->PDRC)));
    RETAILMSG(TRUE,(TEXT("PRRC:%x"),INREG32(&m_pESAI->PRRC)));
    RETAILMSG(TRUE,(TEXT("RCCR:%x"),INREG32(&m_pESAI->RCCR)));
    RETAILMSG(TRUE,(TEXT("RCR:%x"),INREG32(&m_pESAI->RCR)));
    RETAILMSG(TRUE,(TEXT("RFCR:%x"),INREG32(&m_pESAI->RFCR)));
    RETAILMSG(TRUE,(TEXT("RFSR:%x"),INREG32(&m_pESAI->RFSR)));

    RETAILMSG(TRUE,(TEXT("RSMA:%x"),INREG32(&m_pESAI->RSMA)));
    RETAILMSG(TRUE,(TEXT("RSMB:%x"),INREG32(&m_pESAI->RSMB)));
    RETAILMSG(TRUE,(TEXT("SAICR:%x"),INREG32(&m_pESAI->SAICR)));
    RETAILMSG(TRUE,(TEXT("SAISR:%x"),INREG32(&m_pESAI->SAISR)));
    RETAILMSG(TRUE,(TEXT("TCCR:%x"),INREG32(&m_pESAI->TCCR)));
    RETAILMSG(TRUE,(TEXT("TCR:%x"),INREG32(&m_pESAI->TCR)));
    RETAILMSG(TRUE,(TEXT("TFCR:%x"),INREG32(&m_pESAI->TFCR)));
    RETAILMSG(TRUE,(TEXT("TFSR:%x"),INREG32(&m_pESAI->TFSR)));
    RETAILMSG(TRUE,(TEXT("TSMA:%x"),INREG32(&m_pESAI->TSMA)));
    RETAILMSG(TRUE,(TEXT("TSMB:%x"),INREG32(&m_pESAI->TSMB)));
    RETAILMSG(TRUE,(TEXT("ESAI dump regs ----------------------\r\n")));
#endif

    return;
}



//-----------------------------------------------------------------------------
//
//  Function: InitESAI
//
//  This function initializes the specified audio port for input/output.
//
//  Parameters:
//      The ESAI configuration to be used.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::InitESAI(void)
{
    BSPESAIEnableClock(0, TRUE);
    BSPAudioIomuxConfig();

    //enable esai
    OUTREG32(&m_pESAI->ECR,CSP_BITFVAL(ECR_ESAIEN,1)); //Enable esai

    //First put ESAI in Reset state then enable it
    OUTREG32(&m_pESAI->ECR,CSP_BITFVAL(ECR_ERST,1));
    OUTREG32(&m_pESAI->ECR,CSP_BITFVAL(ECR_ESAIEN,1) | CSP_BITFVAL(ECR_ETI,1));

    return; 
}

//-----------------------------------------------------------------------------
//
//  Function: ConfigESAITx
//
//  This function configures esai for transfer operation.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::ConfigESAITx(void)
{
    UINT32 esai_tx_clock_ctrl = 0;
    UINT32 esai_tx_ctrl = 0;
    UINT32 esai_tx_fifo_config = 0;
    
    //as esai does not duplex work, reset the whole esai to avoid errors
    ///OUTREG32(&m_pESAI->ECR,CSP_BITFVAL(ECR_ERST,1));
    ///OUTREG32(&m_pESAI->ECR,CSP_BITFVAL(ECR_ESAIEN,1) | CSP_BITFVAL(ECR_ETI,1));

    //reset the rx part, is it necessary?
    INSREG32(&m_pESAI->TCR,CSP_BITFMASK(TCR_TPR),CSP_BITFVAL(TCR_TPR,1));
    
    //reset fifo
    INSREG32(&m_pESAI->TFCR,CSP_BITFMASK(TFCR_TFR),CSP_BITFVAL(TFCR_TFR,1));

    //for playback, we always use 24 bit mode now, and use fifo to deal with 16bit data
    //OUTREG32(&m_pESAI->SAICR,CSP_BITFVAL(SAICR_ALC,0)|CSP_BITFVAL(SAICR_SYN,0));

    if(m_wavOutputFmtEx.Format.wBitsPerSample == 16){
        OUTREG32(&m_pESAI->SAICR,CSP_BITFVAL(SAICR_ALC,1)|CSP_BITFVAL(SAICR_SYN,0));
    }else{
         OUTREG32(&m_pESAI->SAICR,CSP_BITFVAL(SAICR_ALC,0)|CSP_BITFVAL(SAICR_SYN,0));
    }
 
    // left aligned mode  
    if (BSPAudioGetOutputProtocol() == AUDIO_PROTOCOL_LEFT_ALIGNED){
        esai_tx_ctrl = CSP_BITFVAL(TCR_PADC,1) | //TX ZERO PADDING ,BIT17  1: pad 0, 0:repeat
                   CSP_BITFVAL(TCR_TFSR,0) | //FS timing , BIT16=1, One bit earlier
                   CSP_BITFVAL(TCR_TFSL,0) | // FS length, BIT15 = 0, word length
                   CSP_BITFVAL(TCR_TSWS,0x1f) | //32bit slot len,24bit word len
                   CSP_BITFVAL(TCR_TMOD,0x1) | //0:normal mode  1:network mode
                   CSP_BITFVAL(TCR_TWA,0x0) |  //0:left aligned 1:right aligned
                   CSP_BITFVAL(TCR_TEIE,0x1) | //enable teie intr
                   CSP_BITFVAL(TCR_TSHFD,0x0); //msb first   
    }   

    if (BSPAudioGetOutputProtocol() == AUDIO_PROTOCOL_NETWORK){
        esai_tx_ctrl = CSP_BITFVAL(TCR_PADC,1) | //TX ZERO PADDING ,BIT17  1: pad 0, 0:repeat
                   CSP_BITFVAL(TCR_TFSR,0) | //FS timing , BIT16=0, Not One bit earlier
                   CSP_BITFVAL(TCR_TFSL,1) | // FS length, BIT15 = 1, bit length
                   CSP_BITFVAL(TCR_TSWS,0x1f) | //32bit slot len,24bit word len
                   CSP_BITFVAL(TCR_TMOD,0x1) | //0:normal mode  1:network mode
                   CSP_BITFVAL(TCR_TWA,0x0) |  //0:left aligned 1:right aligned
                   CSP_BITFVAL(TCR_TEIE,0x1) | //enable teie intr
                   CSP_BITFVAL(TCR_TSHFD,0x0); //msb first   
    }  
     
    OUTREG32(&m_pESAI->TCR, esai_tx_ctrl);
    
    //left aligned mode
    if(BSPAudioGetOutputProtocol()  == AUDIO_PROTOCOL_LEFT_ALIGNED){
        esai_tx_clock_ctrl = CSP_BITFVAL(TCCR_THCKD,0) | //HCKT is input (bit23=0)
                         CSP_BITFVAL(TCCR_TFSD,0) | //FST is input (bit22=0)
                         CSP_BITFVAL(TCCR_TCKD,0) | //SCKT is input (bit21=0)
                         CSP_BITFVAL(TCCR_THCKP,0) |  //dont care
                         CSP_BITFVAL(TCCR_TFSP,0) |  //FS polarity bit19, active low for i2s
                         CSP_BITFVAL(TCCR_TCKP,1) |   // tX clock polarity bit 18, clock out on falling edge
                         CSP_BITFVAL(TCCR_TDC,0x1);  //frame rate devider,  2word per frame
    }
    if(BSPAudioGetOutputProtocol() == AUDIO_PROTOCOL_NETWORK){
        esai_tx_clock_ctrl = CSP_BITFVAL(TCCR_THCKD,0) | //HCKT is input (bit23=0)
                         CSP_BITFVAL(TCCR_TFSD,0) | //FST is input (bit22=0)
                         CSP_BITFVAL(TCCR_TCKD,0) | //SCKT is input (bit21=0)
                         CSP_BITFVAL(TCCR_THCKP,0) |  //dont care
                         CSP_BITFVAL(TCCR_TFSP,0) |  //FS polarity bit19, bit19=0 positive active
                         CSP_BITFVAL(TCCR_TCKP,1) |   // tX clock polarity bit 18, clock out on falling edge
                         CSP_BITFVAL(TCCR_TDC,0x5);  //frame rate devider,  6word per frame
    }
    
    OUTREG32(&m_pESAI->TCCR, esai_tx_clock_ctrl);

  
    // DO we have to config TFCR here? or do it before enable tx?
    esai_tx_fifo_config = //CSP_BITFVAL(TFCR_TIEN,1) | //tx init en
                          //CSP_BITFVAL(TFCR_TWA,0x4) | //tx word alignment, bit15 msb
                          CSP_BITFVAL(TFCR_TFWM,ESAI_TX_WATERMAKR);  //Watermark
    OUTREG32(&m_pESAI->TFCR, esai_tx_fifo_config); 

 
    if(BSPAudioGetOutputProtocol() == AUDIO_PROTOCOL_NETWORK){
        OUTREG32(&m_pESAI->TSMA,m_wavOutputFmtEx.dwChannelMask);
        OUTREG32(&m_pESAI->TSMB,0);
    }else{
        OUTREG32(&m_pESAI->TSMA,0x03);
        OUTREG32(&m_pESAI->TSMB,0);
    }
    
    return;
}


//-----------------------------------------------------------------------------
//
//  Function: ConfigESAIRx
//
//  This function configures esai for receive operation.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::ConfigESAIRx(void)
{
    UINT32 esai_rx_clock_ctrl = 0;
    UINT32 esai_rx_ctrl = 0;
    UINT32 esai_rx_fifo_config = 0;

    //reset the whole esai to avoid errors, this must be removed if duplex is to be supported
    ///OUTREG32(&m_pESAI->ECR,CSP_BITFVAL(ECR_ERST,1));
    ///OUTREG32(&m_pESAI->ECR,CSP_BITFVAL(ECR_ESAIEN,1));

    //reset rx part
    INSREG32(&m_pESAI->RCR,CSP_BITFMASK(RCR_RPR),CSP_BITFVAL(RCR_RPR,1));
    
    //reset fifo
    INSREG32(&m_pESAI->RFCR,CSP_BITFMASK(RFCR_RFR),CSP_BITFVAL(RFCR_RFR,1));

    // use alc to deal with 16/24bit difference, the fifo seems not work well here
    if(m_wavInputFmtEx.Format.wBitsPerSample == 16){
        OUTREG32(&m_pESAI->SAICR,CSP_BITFVAL(SAICR_ALC,1)|CSP_BITFVAL(SAICR_SYN,0));
    }else{
         OUTREG32(&m_pESAI->SAICR,CSP_BITFVAL(SAICR_ALC,0)|CSP_BITFVAL(SAICR_SYN,0));
    }
 
    // left aligned mode  
    if (BSPAudioGetInputProtocol() == AUDIO_PROTOCOL_LEFT_ALIGNED){
        esai_rx_ctrl = CSP_BITFVAL(RCR_RFSR,0) | //FS timing , BIT16=1, Not One bit earlier
                   CSP_BITFVAL(RCR_RFSL,0) | // FS length, BIT15 = 0, word length
                   CSP_BITFVAL(RCR_RSWS,0x12) | //32bit slot len,16bit word len
                   CSP_BITFVAL(RCR_RMOD,0x1) | //0:normal mode  1:network mode
                   CSP_BITFVAL(RCR_RWA,0x0) |  //0:left aligned 1:right aligned
                   CSP_BITFVAL(RCR_REIE,0x1) | //enable teie intr
                   CSP_BITFVAL(RCR_RSHFD,0x0); //msb first   
    }   

    if (BSPAudioGetInputProtocol() == AUDIO_PROTOCOL_I2S){
        esai_rx_ctrl = CSP_BITFVAL(RCR_RFSR,1) | //FS timing , BIT16=1, One bit earlier
                   CSP_BITFVAL(RCR_RFSL,0) | // FS length, BIT15 = 0, Frame length
                   CSP_BITFVAL(RCR_RSWS,0x12) | //32bit slot len,16bit word len, this should be decided by bsp code
                   CSP_BITFVAL(RCR_RMOD,0x1) | //0:normal mode  1:network mode
                   CSP_BITFVAL(RCR_RWA,0x0) |  //0:left aligned 1:right aligned
                   CSP_BITFVAL(RCR_REIE,0x1) | //enable teie intr
                   CSP_BITFVAL(RCR_RSHFD,0x0); //msb first   
    }
     
    OUTREG32(&m_pESAI->RCR, esai_rx_ctrl);
    
    //left aligned mode
    if(BSPAudioGetInputProtocol()  == AUDIO_PROTOCOL_LEFT_ALIGNED){
        esai_rx_clock_ctrl = CSP_BITFVAL(RCCR_RHCKD,0) | //HCKT is input (bit23=0)
                         CSP_BITFVAL(RCCR_RFSD,0) | //FST is input (bit22=0)
                         CSP_BITFVAL(RCCR_RCKD,0) | //SCKT is input (bit21=0)
                         CSP_BITFVAL(RCCR_RHCKP,0) |  //dont care
                         CSP_BITFVAL(RCCR_RFSP,0) |  //FS polarity bit19, 0, active high
                         CSP_BITFVAL(RCCR_RCKP,0) |   // tX clock polarity bit 18, clock out on rising edge
                         CSP_BITFVAL(RCCR_RDC,0x1);  //frame rate devider,  2word per frame
    }
    if(BSPAudioGetInputProtocol() == AUDIO_PROTOCOL_I2S){
        esai_rx_clock_ctrl = CSP_BITFVAL(RCCR_RHCKD,0) | //HCKT is input (bit23=0)
                         CSP_BITFVAL(RCCR_RFSD,0) | //FST is input (bit22=0)
                         CSP_BITFVAL(RCCR_RCKD,0) | //SCKT is input (bit21=0)
                         CSP_BITFVAL(RCCR_RHCKP,0) |  //dont care
                         CSP_BITFVAL(RCCR_RFSP,0) |  //FS polarity bit19, bit19=0  active low
                         CSP_BITFVAL(RCCR_RCKP,1) |   // tX clock polarity bit 18, clock out on falling edge
                         CSP_BITFVAL(TCCR_TDC,0x1);  //frame rate devider,  2 word per frame
    }
    
    OUTREG32(&m_pESAI->RCCR, esai_rx_clock_ctrl);
  
    // DO we have to config TFCR here? or do it before enable tx?
    esai_rx_fifo_config = //CSP_BITFVAL(TFCR_TIEN,1) | //tx init en
                          //CSP_BITFVAL(TFCR_TWA,0x4) | //tx word alignment, bit15 msb
                          CSP_BITFVAL(RFCR_RFWM,ESAI_RX_WATERMAKR);  //Watermark
    OUTREG32(&m_pESAI->RFCR, esai_rx_fifo_config); 

    OUTREG32(&m_pESAI->RSMA,0x03);
    OUTREG32(&m_pESAI->RSMB,0);
    
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
    UINT16 dmaBufferSize;
 
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
    m_OutputDMAChan = DDKSdmaOpenChan(DDK_DMA_REQ_ESAI_TX, 
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
//  Name: ConfigOutputDMA
//
//  Description: config ouput dma according to playback format
//
//  Parameters: 
//      none.
//
//  Returns:
//      TRUE if successful, otherwise FALSE
//  Note:
//
//--------------------------------------------------------------------------
BOOL HardwareContext::ConfigOutputDMA()
{
    DDK_DMA_ACCESS dataWidth;
    switch (m_wavOutputFmtEx.Format.wBitsPerSample)
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
    
    return TRUE;
}

//--------------------------------------------------------------------------
//
//  Name: InitInputDMA
//
//  Description: Intialize input DMA channel
//
//  Parameters: 
//      none
//
//  Returns: 
//      TRUE if successful, FALSE if fail.
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
    
    UINT16 dmaBufferSize;
    
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
    m_InputDMAChan = DDKSdmaOpenChan(DDK_DMA_REQ_ESAI_RX, 
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

//--------------------------------------------------------------------------
//
//  Name: ConfigInputDMA
//
//  Description: config input dma according to record format
//
//  Parameters: 
//      none.
//
//  Returns:
//      TRUE if successful, otherwise FALSE
//  Note:
//
//--------------------------------------------------------------------------
BOOL HardwareContext::ConfigInputDMA()
{
    DDK_DMA_ACCESS dataWidth;
    switch (m_wavInputFmtEx.Format.wBitsPerSample)
    {
    case 8:
        dataWidth = DDK_DMA_ACCESS_8BIT;
        break;

    case 16:
        dataWidth = DDK_DMA_ACCESS_16BIT;
        break;

    case 24:
        dataWidth = DDK_DMA_ACCESS_32BIT;
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
//  Function: PrepareESAI
//
//  This function prepare esai before transfer begins, it's mainly used to prepare fifo.
//
//  Parameters:
//      audioBus 
//          [in] dac or adc.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::PrepareESAI(AUDIO_BUS audioBus)
{
    
    UNREFERENCED_PARAMETER(audioBus);

    DWORD esai_tx_fifo_config;
    DWORD esai_rx_fifo_config;

    if (audioBus == AUDIO_BUS_DAC){

        // reset fifo     
        INSREG32(&m_pESAI->TFCR,CSP_BITFMASK(TFCR_TFR),CSP_BITFVAL(TFCR_TFR,1));

        //enable fifo
        esai_tx_fifo_config = CSP_BITFVAL(TFCR_TIEN,1) | //tx init en   jeremy
                              // CSP_BITFVAL(TFCR_TWA,0x4) | //tx word alignment, bit15 msb
                              CSP_BITFVAL(TFCR_TFWM,ESAI_TX_WATERMAKR)|  //Watermark
                              CSP_BITFVAL(TFCR_TE0,1)|
                              CSP_BITFVAL(TFCR_TFE,1);
     
        if(m_wavOutputFmtEx.Format.wBitsPerSample== 16){
            ///esai_tx_fifo_config |= CSP_BITFVAL(TFCR_TWA,0x4); //tx word alignment, bit15 msb
            esai_tx_fifo_config |= CSP_BITFVAL(TFCR_TWA,0x2); //use alc to deal with 16 bit
        }else{
            esai_tx_fifo_config |= CSP_BITFVAL(TFCR_TWA,0x2); //tx word alignment, bit23 msb   
        }
                              
        if (BSPAudioGetOutputProtocol() != AUDIO_PROTOCOL_NETWORK){                      
            if (m_wavOutputFmtEx.Format.nChannels >2 ){
                esai_tx_fifo_config |= CSP_BITFVAL(TFCR_TE1,1);
            }    
            if (m_wavOutputFmtEx.Format.nChannels >4  ){
                esai_tx_fifo_config |= CSP_BITFVAL(TFCR_TE2,1);
            }
        }else{
            // for network mode only te0 is enabled
        } 
            
        OUTREG32(&m_pESAI->TFCR, esai_tx_fifo_config); 

        //stuff fifo
        for(int i=0; i< 120; i++){
            OUTREG32(&m_pESAI->ETDR,0);
        }
    }else if (audioBus == AUDIO_BUS_ADC){
        //reset fifo    
        INSREG32(&m_pESAI->RFCR,CSP_BITFMASK(RFCR_RFR),CSP_BITFVAL(RFCR_RFR,1));

        //enable fifo
        esai_rx_fifo_config = CSP_BITFVAL(RFCR_REXT,0) | //rx sign extend, zero extended:0
                              //CSP_BITFVAL(RFCR_RWA,0x4) | //rx word alignment, bit15 msb
                              CSP_BITFVAL(RFCR_RFWM,ESAI_RX_WATERMAKR)|  //Watermark
                              CSP_BITFVAL(RFCR_RE1,1)|
                              CSP_BITFVAL(RFCR_RFE,1);
     
        //we already used alc, for 16bit,  although data is aligned to bit23, only bit0~bit15 effect
        esai_rx_fifo_config |= CSP_BITFVAL(RFCR_RWA,0x02); //rx word alignment, bit23 msb

                              
        if (BSPAudioGetInputProtocol() != AUDIO_PROTOCOL_NETWORK){                      
            if (m_wavInputFmtEx.Format.nChannels >2 ){
                esai_rx_fifo_config |= CSP_BITFVAL(RFCR_RE2,1);
            }    
            if (m_wavInputFmtEx.Format.nChannels >4  ){
                //re0 is not used now,  it should be decided by bsp code that which re is used
                esai_rx_fifo_config |= CSP_BITFVAL(RFCR_RE0,1);
            }
        }else{
                //network mode is not used by rx now    
        } 
            
        OUTREG32(&m_pESAI->RFCR, esai_rx_fifo_config); 

    }        
    
    return;
}    

//-----------------------------------------------------------------------------
//
//  Function: StartESAI
//
//  This function enable esai 
//
//  Parameters:
//      audioBus 
//          [in] adc or dac.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::StartESAI(AUDIO_BUS audioBus)
{

    if (audioBus == AUDIO_BUS_DAC){
        DWORD esai_tx_en_mask = CSP_BITFMASK(TCR_TE0);
        DWORD esai_tx_en_val = CSP_BITFVAL(TCR_TE0,1);
                           
        if (BSPAudioGetOutputProtocol() != AUDIO_PROTOCOL_NETWORK){                      
            if (m_wavOutputFmtEx.Format.nChannels >2 ){
                esai_tx_en_mask |= CSP_BITFMASK(TCR_TE1);
                esai_tx_en_val |= CSP_BITFVAL(TCR_TE1,1);
            }    
            if (m_wavOutputFmtEx.Format.nChannels >4  ){
                esai_tx_en_mask |= CSP_BITFMASK(TCR_TE2);
                esai_tx_en_val |= CSP_BITFVAL(TCR_TE2,1);

            }
        }else{
            //use te0 only for network mode
        }

        INSREG32(&m_pESAI->TCR,esai_tx_en_mask,esai_tx_en_val); 

        OUTREG32(&m_pESAI->PCRC,0x00000FFF);
        OUTREG32(&m_pESAI->PRRC,0x00000FFF);
    }else if( audioBus == AUDIO_BUS_ADC){
        DWORD esai_rx_en_mask = CSP_BITFMASK(RCR_RE1);
        DWORD esai_rx_en_val = CSP_BITFVAL(RCR_RE1,1);

        //it should be bsp code to decide which re is used                                    
        if (BSPAudioGetInputProtocol() != AUDIO_PROTOCOL_NETWORK){                      
            if (m_wavInputFmtEx.Format.nChannels >2 ){
                esai_rx_en_mask |= CSP_BITFMASK(RCR_RE2);
                esai_rx_en_val |= CSP_BITFVAL(RCR_RE2,1);
            }    
            if (m_wavInputFmtEx.Format.nChannels >4  ){
                esai_rx_en_mask |= CSP_BITFMASK(RCR_RE0);
                esai_rx_en_val |= CSP_BITFVAL(RCR_RE0,1);
            }
        }else{
            
        }

        INSREG32(&m_pESAI->RCR,esai_rx_en_mask,esai_rx_en_val); 

        OUTREG32(&m_pESAI->PCRC,0x00000FFF);
        OUTREG32(&m_pESAI->PRRC,0x00000FFF);

    }

    return; 
}

//-----------------------------------------------------------------------------
//
//  Function: StopEsai
//
//  This function configures ESAI to stop .
//
//  Parameters:
//      audioBus 
//          [in] dac or adc.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopESAI(AUDIO_BUS audioBus)
{
    if(audioBus == AUDIO_BUS_DAC){
        DWORD esai_tx_fifo_en_mask = CSP_BITFMASK(TFCR_TFE)
                                    | CSP_BITFMASK(TFCR_TE0);
        
        DWORD esai_tx_en_mask = CSP_BITFMASK(TCR_TE0);
 
        if (BSPAudioGetOutputProtocol() != AUDIO_PROTOCOL_NETWORK){                      
            if (m_wavOutputFmtEx.Format.nChannels >2 ){
                esai_tx_en_mask |= CSP_BITFMASK(TCR_TE1);
                esai_tx_fifo_en_mask |= CSP_BITFMASK(TFCR_TE1);
               
            }    
            if (m_wavOutputFmtEx.Format.nChannels >4  ){
                esai_tx_en_mask |= CSP_BITFMASK(TCR_TE2);
                esai_tx_fifo_en_mask |= CSP_BITFMASK(TFCR_TE2);
            }
        }

        INSREG32(&m_pESAI->TCR,esai_tx_en_mask,0);  //stop tx
        INSREG32(&m_pESAI->TFCR,esai_tx_fifo_en_mask,0);

    }else{
        DWORD esai_rx_fifo_en_mask = CSP_BITFMASK(RFCR_RFE)
                                    | CSP_BITFMASK(RFCR_RE0);
        
        DWORD esai_rx_en_mask = CSP_BITFMASK(RCR_RE0);
 
        if (BSPAudioGetInputProtocol() != AUDIO_PROTOCOL_NETWORK){                      
            if (m_wavInputFmtEx.Format.nChannels >2 ){
                esai_rx_en_mask |= CSP_BITFMASK(RCR_RE1);
                esai_rx_fifo_en_mask |= CSP_BITFMASK(RFCR_RE1);
               
            }    
            if (m_wavOutputFmtEx.Format.nChannels >4  ){
                esai_rx_en_mask |= CSP_BITFMASK(RCR_RE2);
                esai_rx_fifo_en_mask |= CSP_BITFMASK(RFCR_RE2);
            }
        }

        INSREG32(&m_pESAI->RCR,esai_rx_en_mask,0);  //stop tx
        INSREG32(&m_pESAI->RFCR,esai_rx_fifo_en_mask,0);    

    }

    return;
 
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
    //if (m_bCodecLoopbackState)
    //{
    //    return FALSE;
    //}
    
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
    if (!m_OutputDMARunning || m_bESAITXUnderrunError)
    {
        // Initialize output DMA state variables. Note that if we get here
        // because m_bESAITXUnderrunError is still TRUE, then we must have
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
 
            if (m_bAsrcEnable){
                PrepareESAI(AUDIO_BUS_DAC);
                if(!m_bESAITXUnderrunError)
                    StartCodec(AUDIO_BUS_DAC);  //asrc requires clock to work
                AsrcStartConv(m_asrcPairIndex,TRUE,TRUE,TRUE);

                Sleep(1); //delay 1 ms, then start esai, or esai may under-run

            }else{
                DWORD dwHWSampleSize;
                if(m_wavOutputFmtEx.Format.wBitsPerSample ==16)
                    dwHWSampleSize = 2;
                else if(m_wavOutputFmtEx.Format.wBitsPerSample ==32)
                    dwHWSampleSize = 4;    
                else
                    dwHWSampleSize = 3;
                if(!m_bESAITXUnderrunError &&
                    !DDKSdmaInitChain(m_OutputDMAChan, 
                            ESAI_TX_WATERMAKR * dwHWSampleSize))
                {
                    ERRMSG("StartOutputDMA: Unable to initialize output DMA channel\r\n");

                    m_OutputDMARunning  = FALSE;
                    m_bCanStopOutputDMA = TRUE;
                
                    return FALSE;
                }

                // Start the output DMA channel. This must be done before we 
                // start esai and Codec because we have to make sure that audio
                // data is already available before that.
                
                PrepareESAI(AUDIO_BUS_DAC);            
                DDKSdmaStartChan(m_OutputDMAChan);
            }
            
        }

        // Check to see if we need to complete the handling of the last
        // ESAI TX underrun error which was flagged in interrupt thread.
        //
        // This takes care of the unusual situation whereby the last
        // playback operation ended with an ESAI TX underrun error but
        // we are now about to resume playback without having called
        // StopOutputDMA() in between.
        if (m_bESAITXUnderrunError)
        {
            // In this case, the ESAI has just been disabled and it does
            // not need to be completely reinitialized.
            //
            // Reenable the ESAI here if we disabled it during the last
            // interrupt thread call where we also had to handle a TX
            // underrun error and there was no more audio data available
            // to continue.
            StartESAI(AUDIO_BUS_DAC);

            // We have now completed the recovery from the most recent
            // ESAI TX underrun error.
            m_bESAITXUnderrunError = FALSE;
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
            // starting ESAI output because there are required delays involved
            // in enabling some of the Codec components. These required delays
            // will cause an ESAI FIFO underrun error if the ESAI transmitter 
            // is already running before the Codec components have been enabled.
            if (!m_bAsrcEnable)
            StartCodec(AUDIO_BUS_DAC);

            // Then start the SSI transmitter
            StartESAI(AUDIO_BUS_DAC);
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

        // Also reset the ESAI TX underrun error flag here since we are about
        // to shutdown the current audio setup. The audio hardware will be
        // fully reinitialized when StartOutputDMA() is next called so we no
        // longer care if an ESAI TX underrun error was previously flagged for
        // the current audio playback operation.
        m_bESAITXUnderrunError = FALSE;

        // Disable audio output hardware
        StopCodec(AUDIO_BUS_DAC);
        StopESAI(AUDIO_BUS_DAC);

        // Kill the output DMA channel
        if(m_bAsrcEnable){
            AsrcStopConv(m_asrcPairIndex,TRUE,TRUE,TRUE);
            //jeremy, modify the index later
        }else{
            DDKSdmaStopChan(m_OutputDMAChan, FALSE);
        }
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
    if(!m_InputDMARunning)
    {
        // Initialize the input DMA state variables
        m_InputDMARunning = TRUE;
        m_InputDMAStatus  = (DWORD)~(DMA_DONEA | DMA_DONEB);

        // Reset the state of the input DMA chain.
        //
        // Note that the second "watermark" argument to DDKSdmaInitChain()
        // is actually in bytes whereas "m_InputDMALevel" is given in
        // ESAI FIFO slots or words. Therefore, we have to multiply by the
        // audio data word size to ensure that we are properly emptying
        // the ESAI RX FIFO.
        //
        if(!DDKSdmaInitChain(m_InputDMAChan,
                        ESAI_TX_WATERMAKR * sizeof(HWSAMPLE)))
        {
            ERRMSG("StartInputDMA: Unable to initialize input DMA channel!\r\n");

            // Must reset the m_InputDMARunning state variable to FALSE
            // if we encounter an error here.
            m_InputDMARunning = FALSE;

            return FALSE;
        }

        PrepareESAI(AUDIO_BUS_ADC);

        // Start the input DMA
        DDKSdmaStartChan(m_InputDMAChan);

        // Enable audio input hardware. This must be done after we activate the
        // input DMA channel because the audio hardware will immediately start
        // recording when this call is made.
        //
        StartCodec(AUDIO_BUS_ADC);
        StartESAI(AUDIO_BUS_ADC); 
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

        // Disable audio input hardware
        StopCodec(AUDIO_BUS_ADC);
        StopESAI(AUDIO_BUS_ADC);

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


void HardwareContext::OutputIntrRoutine()
{
    UINT32 bufDescStatus[AUDIO_DMA_NUMBER_PAGES];
    UINT32 nTransferred;

    // Check for ESAI  transmitter underrun errors. These are
    // bad and can result in unpredictable swapping of the
    // left/right audio channels if we do not recover from
    // it properly.
    //if (EXTREG32BF(&m_pESAI->SAISR,SAISR_TDE)  && EXTREG32BF(&m_pESAI->SAISR, SAISR_TUE) )
    if (EXTREG32BF(&m_pESAI->SAISR,SAISR_TUE)) 
    {
        //RETAILMSG(1, (_T("ESAI TX underrun!!!!\r\n")));

        m_bESAITXUnderrunError = TRUE;
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

    // Check to see if an ESAI TX underrun error has just occurred
    // and, if necessary, recover from it.
    if (m_bESAITXUnderrunError)
    {
        // Start the ESAI TX underrun error recovery by disabling
        // the ESAI. This will also flush out any stray data that
        // may still be in the TX FIFO. Note that this is possible
        // because an earlier DMA operation may have refilled the
        // TX FIFO (we only get interrupts here when a DMA buffer
        // has been emptied).
        //
        StopESAI(AUDIO_BUS_DAC);

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

        if (m_bESAITXUnderrunError)
            PrepareESAI(AUDIO_BUS_DAC);

        
        DDKSdmaStartChan(m_OutputDMAChan);

        // Also reenable the ESAI here if we disabled it above when
        // we began our handling of the TX underrun error.
        if (m_bESAITXUnderrunError)
        {
            StartESAI(AUDIO_BUS_DAC);

            // We have now completed the recovery from the most
            // recent ESAI TX underrun error.
            m_bESAITXUnderrunError = FALSE;
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


    if (EXTREG32BF(&m_pESAI->SAISR,SAISR_ROE))
    {
        //RETAILMSG(1, (_T("ESAI RX overrun\r\n")));
        m_bESAIRXOverrunError = TRUE;
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

    if (m_bESAIRXOverrunError)
    {
        StopESAI(AUDIO_BUS_ADC);

        // Next stop all output DMA channel activity and flush out
        // both DMA buffers.
        DDKSdmaStopChan(m_InputDMAChan, TRUE);
        m_InputDMAStatus &= ~(DMA_DONEA | DMA_DONEB);
        checkFirst = BUFFER_A;
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
    }
    else
    {
        if(m_bESAIRXOverrunError){

            ConfigESAIRx();//We wanna reset the rx here
            PrepareESAI(AUDIO_BUS_ADC);

            // Start the input DMA
            DDKSdmaStartChan(m_InputDMAChan);

            // Enable audio input hardware. This must be done after we activate the
            // input DMA channel because the audio hardware will immediately start
            // recording when this call is made.
            StartESAI(AUDIO_BUS_ADC); // jeremy, temp put it here
            m_bESAIRXOverrunError = FALSE;
            
        }else{
            StopInputDMA();
        }
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

            if (!m_OutputDMARunning)
            {
                // The timer has expired without another output audio stream
                // being activated so we can really disable the DAC now.
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
    /*UNREFERENCED_PARAMETER(dwCode);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(pdwActualOut);

    return TRUE;*/
    
    
#if 1 
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

                //DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_CAPABILITIES = 0x%x\r\n"), ppc->DeviceDx));

                // Update returned data size.
                *pdwActualOut = sizeof(*ppc);
                bRetVal = TRUE;
            }
            else
            {
                //DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: Invalid parameter.\r\n" ) ) );
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
                //DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_QUERY = %d\r\n"), *pDxState));

                // Check for any valid power state.
                if (VALID_DX (*pDxState))
                {
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    bRetVal = TRUE;
                }
                else
                {
                    //DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_QUERY invalid power state.\r\n" ) ) );
                }
            }
            else
            {
                //DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_QUERY invalid parameter.\r\n" ) ) );
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
                //DEBUGMSG(ZONE_FUNCTION, ( TEXT( "WAVE: IOCTL_POWER_SET = %d.\r\n"), *pDxState));

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
                    //DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_SET invalid power state.\r\n" ) ) );
                }
            }
            else
            {
                //DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_SET invalid parameter.\r\n" ) ) );
            }

            break;
        }

        // Return the current device power state.
    case IOCTL_POWER_GET:
        {
            //DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_GET -- not supported!\r\n")));
            break;
        }

    default:
        //DEBUGMSG(ZONE_WARN, (TEXT("WAVE: Unknown IOCTL_xxx(0x%0.8X) \r\n"), dwCode));
        break;
    }

    return bRetVal;
#endif    
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

    // Manually set the timed delay disable events. This will explicitly
    // trigger any in-progress delays to immediately terminate.
    if (m_hDACDelayDisableEvent != NULL)
    {
        PulseEvent(m_hDACDelayDisableEvent);
    }

    if (m_hADCDelayDisableEvent != NULL)
    {
        PulseEvent(m_hADCDelayDisableEvent);
    }

    // This tells the low-level PMIC driver to immediately terminate any
    // pending timer delays.
    //BSPAudioPowerDown(FALSE);

    // Set this flag to allow special handling for the powerdown procedure.
    // The most important thing we want to avoid when powering down is using
    // any sort of timer or other type of delay mechanism. We need to bypass
    // anything like that and simply shutdown the hardware components when
    // performing a powerdown operation.
    m_delayBypass = TRUE;

    // Save state for correct powerup handling.
    g_saveOutputDMARunning = m_OutputDMARunning;
    g_saveInputDMARunning  = m_InputDMARunning;

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
    PWAVEFORMATEXTENSIBLE pWavFmtEx;

    pWavFmtEx = (PWAVEFORMATEXTENSIBLE)lpFormat;

    if(m_bWaveInputFormatInitialed){
        //we don't support full-duplex work, so if input format is initialed , any output format
        //is forbidden
        //return FALSE;
    }


    if(lpFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE){
        return FALSE;
    }    

    pWavFmtEx = (PWAVEFORMATEXTENSIBLE)lpFormat;

    if(BSPIsSupportedOutputFormat(pWavFmtEx)){
        // this format is directly supported by codec, we don't need asrc
        return TRUE;
    }
    

    //  sample rate can not be supported by codec directly, see if we can support it by asrc
    if((pWavFmtEx->Format.wBitsPerSample == 32) 
        && (pWavFmtEx->Samples.wValidBitsPerSample == 24)){
        //we can use asrc for such format
        if (m_bAsrcSupport == TRUE){
            return TRUE;
        }
    }

    return FALSE;
}


BOOL HardwareContext::IsSupportedInputFormat(LPWAVEFORMATEX lpFormat)
{
    PWAVEFORMATEXTENSIBLE pWavFmtEx;

    pWavFmtEx = (PWAVEFORMATEXTENSIBLE)lpFormat;

    if(m_bWaveOutputFormatInitialed){
        //we don't support full-duplex work, so if output format is initialed , any input format
        //is forbidden
        //return FALSE;
    }


    if(lpFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE){
        return FALSE;
    }    

    pWavFmtEx = (PWAVEFORMATEXTENSIBLE)lpFormat;

    if(BSPIsSupportedInputFormat(pWavFmtEx)){
        // this format is directly supported by codec, we don't need asrc
        return TRUE;
    }
    
    return FALSE;
}



BOOL HardwareContext::InitOutputWaveFormat(LPWAVEFORMATEX lpFormat)
{
    DWORD dwSize = sizeof(WAVEFORMATEXTENSIBLE);

    if(m_bWaveInputFormatInitialed){
        //we don't support full-duplex work, so if input format is initialed , any output format
        //is forbidden
        //return FALSE;
    }

    memcpy(&m_wavOutputFmtEx, lpFormat, dwSize);

    m_bAsrcEnable= FALSE;//NOT SUPPORT NOW
    m_bWaveOutputFormatInitialed = TRUE;


    if(BSPIsSupportedOutputFormat(&m_wavOutputFmtEx)){
        // the wave format is directly supported by code, config it
        BSPAudioConfigDAC(m_wavOutputFmtEx.Format.nSamplesPerSec,
            m_wavOutputFmtEx.Samples.wValidBitsPerSample, 
            m_wavOutputFmtEx.dwChannelMask,
            m_wavOutputFmtEx.Format.nChannels,
            BSPAudioGetOutputProtocol());    
        m_bAsrcEnable = FALSE;

    }else{
        if((m_wavOutputFmtEx.Format.wBitsPerSample == 32) 
        && (m_wavOutputFmtEx.Samples.wValidBitsPerSample == 24)
        && (m_bAsrcSupport == TRUE)){
        //we use asrc to support this case
       
            m_bAsrcEnable = TRUE;
            BSPAudioConfigDAC(OUTPUT_SAMPLERATE,
            24, 
            m_wavOutputFmtEx.dwChannelMask,
            m_wavOutputFmtEx.Format.nChannels,
            BSPAudioGetOutputProtocol());  

            //We may need to adjust the chn number & mask here
            //as asrc take only even number

        }else{
            // using software mixer, take only normal stereo data
            BSPAudioConfigDAC(OUTPUT_SAMPLERATE, 16, 
                0x3,2,
                BSPAudioGetOutputProtocol());
        }
    }

    ConfigESAITx();

    if(m_bAsrcEnable){
        ConfigAsrc(); // call this after codec is configured, BspAudioConfigDAC
        
    }else{

        ConfigOutputDMA();// config dma for  mem->esai
    }

    return TRUE;
}

BOOL HardwareContext:: InitInputWaveFormat(LPWAVEFORMATEX lpFormat)
{
    DWORD dwSize = sizeof(WAVEFORMATEXTENSIBLE);

    if(m_bWaveOutputFormatInitialed){
        //we don't support full-duplex work, so if output format is initialed , any input format
        //is forbidden
        //return FALSE;
    }

    memcpy(&m_wavInputFmtEx, lpFormat, dwSize);

    m_bWaveInputFormatInitialed = TRUE;


    if(BSPIsSupportedInputFormat(&m_wavInputFmtEx)){
        // the wave format is directly supported by code, config it
        BSPAudioConfigADC(m_wavInputFmtEx.Format.nSamplesPerSec,
            //.Samples.wValidBitsPerSample, 
            24,
            m_wavInputFmtEx.dwChannelMask,
            m_wavInputFmtEx.Format.nChannels,
            BSPAudioGetInputProtocol());    
        
    }else{
        return FALSE;
    }

    ConfigESAIRx();

    ConfigInputDMA();// config dma for  mem->esai
 
    return TRUE;
}

BOOL HardwareContext:: DeinitInputWaveFormat(LPWAVEFORMATEX lpFormat)
{
    UNREFERENCED_PARAMETER(lpFormat);
    m_bWaveInputFormatInitialed = FALSE;
    return TRUE;
}

BOOL HardwareContext:: DeinitOutputWaveFormat(LPWAVEFORMATEX lpFormat)
{
    UNREFERENCED_PARAMETER(lpFormat);
    m_bWaveOutputFormatInitialed = FALSE;
    return TRUE;
}



DWORD HardwareContext::ForceSpeaker( BOOL bForceSpeaker )
{
    UNREFERENCED_PARAMETER(bForceSpeaker);
    // If m_NumForcedSpeaker is non-zero, audio should be routed to an
    // external speaker (if hw permits).
    /*if (bForceSpeaker)
    {
        m_NumForcedSpeaker++;
        if (m_NumForcedSpeaker==1)
        {
            RecalcSpeakerEnable();
        }
    }
    else
    {
        m_NumForcedSpeaker--;
        if (m_NumForcedSpeaker==0)
        {
            RecalcSpeakerEnable();
        }
    }*/

    return MMSYSERR_NOERROR;
}

BOOL HardwareContext::SelectInputSource(DWORD nIndex)
{
    UNREFERENCED_PARAMETER(nIndex);

   /* Lock();

    // We do not want to change input source when record path is working
    if (m_InputDMARunning)
    {
        RETAILMSG(TRUE, (L"Can not change input source when "
                         L"recording is running\r\n"));
        return FALSE;
    }

    BSPAudioSelectADCSource(nIndex);
    
    Unlock();*/

    return TRUE;
}



void CallAsrcIntrThread(HardwareContext *pHWContext)
{
    pHWContext->AsrcIntrThread();
}


void HardwareContext::AsrcIntrThread()
{
    // Loop repeatedly to handle each asrc interrupt event until we
    // deinitialize the audio driver.
    //RETAILMSG(1,(TEXT("AsrcIntrThread,m_Initialized(%d) \r\n"),m_Initialized));
    while (m_Initialized)
    {
        WaitForSingleObject(m_hAsrcTrigEvent, INFINITE); //wait for start work
        if ((!m_Initialized))
        {
            break;
        }

        while(m_bAsrcOpened){
            WaitForSingleObject(m_hAsrcIntrEvent, INFINITE);
        
            // Immediately terminate this thread if Deinit() has been called to
            // unload the audio driver. Otherwise, handle the audio interrupt
            // event.
            if ((!m_Initialized) ||(!m_bAsrcOpened))
            {
                break;
            }

            // Grab the audio driver lock to avoid race conditions with the
            // StartOutputDMA() and StopOutputDMA() functions.
            Lock();

             if (m_OutputDMARunning) //this val used for both mem->esai & mem->asrc
            {
                AsrcIntrRoutine();
            }

            Unlock();
  
        }//end of while (m_bAsrcOpened)
    }

    // Close the event handle
    if (m_hAsrcIntrEvent != NULL)
    {
        CloseHandle(m_hAsrcIntrEvent);
        m_hAsrcIntrEvent = NULL;
    }

    if (m_hAsrcTrigEvent != NULL)
    {
        CloseHandle(m_hAsrcTrigEvent);
        m_hAsrcTrigEvent = NULL;
    }

}

void HardwareContext::AsrcIntrRoutine()
{
    UINT32 nTransferred;
    UINT32 bufStatus;

    // Check for ESAI  transmitter underrun errors. 
    if (EXTREG32BF(&m_pESAI->SAISR, SAISR_TUE) )
    {
        //RETAILMSG(1, (_T("ESAI TX underrun!!!!\r\n")));
        m_bESAITXUnderrunError = TRUE;

    }

    bufStatus = AsrcGetInputBufStatus(m_asrcPairIndex);

    
    //just for debug , print msg here
    if ((!(bufStatus & ASRC_BUF_INPUT_DONE_A)) && (!(bufStatus & ASRC_BUF_INPUT_DONE_B))){
        //RETAILMSG(1, (_T("Asrc p2p inter for nothing!!!!\r\n")));
        return; //don't deal with such intr
    }

    
    // Set DMA status bits according to retrieved transfer status
    if (bufStatus & ASRC_BUF_INPUT_DONE_A)
    {
        //mem->asrc , buffer a status
        m_OutputDMAStatus |= DMA_DONEA;
    }

    if (bufStatus & ASRC_BUF_INPUT_DONE_B)
    {
        //mem->asrc , buffer b status
        m_OutputDMAStatus |= DMA_DONEB;
    }


    // Check to see if an ESAI TX underrun error has just occurred
    // and, if necessary, recover from it.
    if (m_bESAITXUnderrunError)
    {
        // Start the ESAI TX underrun error recovery by disabling
        // the ESAI. This will also flush out any stray data that
        // may still be in the TX FIFO. Note that this is possible
        // because an earlier DMA operation may have refilled the
        // TX FIFO (we only get interrupts here when a DMA buffer
        // has been emptied).
        //
        StopESAI(AUDIO_BUS_DAC);

        // Next stop all output DMA channel activity and flush out
        // both DMA buffers.
        AsrcStopConv(m_asrcPairIndex,TRUE,TRUE,TRUE);
        m_OutputDMAStatus = DMA_DONEA | DMA_DONEB;
    }

    // Try to refill any empty output DMA buffers with new data.
    nTransferred = TransferOutputBuffers();


    // Don't restart the DMA unless we actually have some data
    // in at least one of the DMA buffers.
    // if (nTransferred > 0)
    if (!(m_OutputDMAStatus & DMA_DONEA) || !(m_OutputDMAStatus & DMA_DONEB))
    {

        if (m_bESAITXUnderrunError ){
            PrepareESAI(AUDIO_BUS_DAC);
            AsrcStartConv(m_asrcPairIndex,TRUE,TRUE,TRUE);
            Sleep(1);
        }else{
            // SDMA will disable itself on buffer underrun.
            // Force channel to be enabled since we now have
            // at least one buffer to be processed.
            AsrcStartConv(m_asrcPairIndex,FALSE,TRUE,FALSE); 

        }

       
        // Also reenable the SSI here if we disabled it above when
        // we began our handling of the TX underrun error.
        if (m_bESAITXUnderrunError)
        {
            StartESAI(AUDIO_BUS_DAC);

            // We have now completed the recovery from the most
            // recent ESAI TX underrun error.
            m_bESAITXUnderrunError = FALSE;
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
        //RETAILMSG(1, (_T("!!!!!!!!!!!!Trans buffer now data available\r\n")));

        m_bCanStopOutputDMA = TRUE;
        SetEvent(m_hStopOutputDMAEvent);//both mem->esai & mem->asrc are supposed to be output dma here
    }
}



//-----------------------------------------------------------------------------
//
//  Function:  InitAsrcP2P
//
//  This function init Asrc for p2p working mode,it's call when module is initialed
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::InitAsrcP2P(void)
{

    HKEY hKey = NULL;    
    DWORD dwVal;
    DWORD dwSize = sizeof(DWORD);

    //query registry, check if asrc is supported
    if (RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        c_szESAIKey,
        0,
        0,
        &hKey
        ) == ERROR_SUCCESS)
    {
        RegQueryValueEx (
            hKey,
            c_szAsrcSupport,
            NULL,
            NULL,
            (LPBYTE)&dwVal,
            &dwSize
        );

        if (dwVal == 1){
            m_bAsrcSupport = TRUE;
        }else{
            m_bAsrcSupport = FALSE;;
        }  
        
        //check the pair index for esai usage and max channel number
        if (m_bAsrcSupport){
            RegQueryValueEx (
                hKey,
                c_szAsrcPairIndex,
                NULL,
                NULL,
                (LPBYTE)&dwVal,
                &dwSize
            );

            switch(dwVal){
                case 0:
                    m_asrcPairIndex = ASRC_PAIR_A;
                    break;
                case 1:
                    m_asrcPairIndex = ASRC_PAIR_B;
                    break;
                case 2:
                    m_asrcPairIndex = ASRC_PAIR_C;
                    break;
                default:
                    m_bAsrcSupport = FALSE;
            }

            RegQueryValueEx (
                hKey,
                c_szAsrcMaxChnNum,
                NULL,
                NULL,
                (LPBYTE)&dwVal,
                &dwSize
            );

            if((dwVal >= 1) && ( dwVal <= 8 ) && (dwVal%2 != 1)){
                m_dwAsrcMaxChnNum = dwVal;
            }else{
                m_dwAsrcMaxChnNum = 0;
                m_bAsrcSupport = FALSE;    
                
            }

        }//end of if (m_bAsrcSupport)  
   
      
        RegCloseKey(hKey);
    }

    m_bAsrcEnable = FALSE;
    m_bAsrcOpened = FALSE;

    if(m_bAsrcSupport){

        AsrcInit(NULL);

        m_hAsrcIntrEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
        m_hAsrcTrigEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

        m_hAsrcIntrThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                       0,
                                       (LPTHREAD_START_ROUTINE)CallAsrcIntrThread,
                                       this,
                                       0,
                                       NULL);
        if (!m_hAsrcIntrThread)
        {
            m_bAsrcSupport = FALSE;
            return FALSE;
        }

        // Bump up the priority since the interrupt must be serviced immediately.
        CeSetThreadPriority(m_hAsrcIntrThread, GetInterruptThreadPriority());
   }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  DeinitAsrcP2P
//
//  This function deinit Asrc for p2p working mode
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------

void HardwareContext::DeinitAsrcP2P(void)
{
    if(!m_bAsrcSupport)
        return;

    if(m_bAsrcOpened){
        // we must release the pair if it's already requested
        AsrcClosePair(m_asrcPairIndex);
        m_bAsrcOpened = FALSE;
        SetEvent(m_hAsrcIntrEvent);
    }
    m_bAsrcEnable = FALSE;

    //m_Initialized should be false now, when this function is called
    SetEvent(m_hAsrcTrigEvent); //to end the asrc intr thread

    AsrcDeinit();
 
    return;
}



//-----------------------------------------------------------------------------
//
//  Function:  ConfigAsrc
//
//  This function configures asrc according to the input wave format
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::ConfigAsrc(void)
{
    
    ASRC_PAIR_INDEX pairIndex;
    ASRC_CONFIG_PARAM configParam;
    PHYSICAL_ADDRESS addrTemp;

 
    pairIndex = m_asrcPairIndex;

    if(m_bAsrcOpened){
        // we must release the pair if it's already requested
        AsrcClosePair(pairIndex);
        m_bAsrcOpened = FALSE;
        SetEvent(m_hAsrcIntrEvent);
    }
    
    if( AsrcOpenPair(pairIndex,
        (m_wavOutputFmtEx.Format.nChannels+1) & 0xfffe , // the number must be even
        m_wavOutputFmtEx.Format.nChannels,
        m_hAsrcIntrEvent, 
        m_hAsrcIntrEvent,
        ASRC_TRANS_MEM2ESAI) != pairIndex ){
            RETAILMSG(1,(TEXT("open asrc pair failed\r\n")));
            return FALSE; 
    }
    m_bAsrcOpened = TRUE;
    SetEvent(m_hAsrcTrigEvent);

    //the buffers allocated in InitDma function are used for both mem->esai and mem->asrc
    //functions, as they will not work at the same time,but they use different dma channel
    AsrcSetInputBuf(pairIndex,
        m_DMAOutBufPhysAddr,
        m_OutputDMAPageSize,
        0,
        m_DMAOutPageVirtAddr[BUFFER_A]);

    //PHYSICAL_ADDRESS addrTemp2;
    addrTemp.LowPart = m_DMAOutBufPhysAddr.LowPart + m_OutputDMAPageSize;
    
    AsrcSetInputBuf(pairIndex,
        addrTemp,
        m_OutputDMAPageSize,
        1,
        m_DMAOutPageVirtAddr[BUFFER_B]);

    
    addrTemp.LowPart = 0;
    AsrcSetOutputBuf(pairIndex,
        addrTemp,
        4096, // num of bytes, could be used, we just use some non-zero val here
        0,
        0);

    configParam.clkMode = ASRC_CLK_ONE_SRC_OUTPUT; //output clk from esai
    configParam.inClkSrc = ASRC_ICLK_SRC_ASRCK1; //Let asrc deal
    configParam.inputBitClkRate = 
        m_wavOutputFmtEx.Format.nSamplesPerSec * 32; 
    //this val doesn't mean anything as there's no input src clk
    configParam.inputSampleRate = m_wavOutputFmtEx.Format.nSamplesPerSec;
    configParam.outClkSrc = ASRC_OCLK_SRC_ESAI_TX;
    configParam.outputSampleRate = OUTPUT_SAMPLERATE;

    configParam.outputBitClkRate = BSPAudioGetTxBitClock();

    AsrcConfigPair(pairIndex, &configParam);

    AsrcSetP2PDeviceWML(pairIndex,ESAI_TX_WATERMAKR);
    return TRUE;
}

