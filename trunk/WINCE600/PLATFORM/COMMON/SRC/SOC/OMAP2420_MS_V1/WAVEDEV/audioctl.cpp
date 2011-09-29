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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  EAC controller platform specific configuration.
//
//  Most settings depend on the codec, modem and bluetooth chip connected to
//  the OMAP2420 chipset.
//
#include <wavemain.h>
#include <ti_constants.h>
#include <ceddk.h>
#include <spi.h>
#include <tsc2101.h>
#include "xhwctxt.h"

//------------------------------------------------------------------------------
//  Globals

const GUID DEVICE_IFC_CLK_GUID;
const GUID DEVICE_IFC_SPI_GUID;

//------------------------------------------------------------------------------
//
//  platform specific definition for codec port
//
//  on P2 Sample platform the codec port is configured as I2S
//

//------------------------------------------------------------------------------
//
// EAC BT auSPI pin muxing defines and function.
//

#define MODE1_INTERNAL_BTAUSPI_SOURCE_BITS  (BIT3|BIT2)  // Bits used to select BT EAC AuSPI source                                            
#define MODE1_V3U3V2T4_AS_BTAUSPI_SOURCE    (BIT2)       // use pins V3/U3/V2/T4 as source
#define MODE1_W6R9Y6Y5_AS_BTAUSPI_SOURCE    0            // use pins W6/R9/Y6/Y5 as source

// TSC2101 constants
#define TSC2101_PAGE2   0x1000


//------------------------------------------------------------------------------
//
//  TSC2101Write - uses SPI to write a value to one of the TSC2101 registers
//

static void TSC2101Write(HANDLE hSPI, WORD wReg, WORD wData)
{
    DWORD   dwCommand;

    dwCommand = ((DWORD)(TSC2101_PAGE2 | wReg) << 16) | wData;
    SPITransfer(hSPI, &dwCommand);
}

//------------------------------------------------------------------------------
//
//  TSC2101Read - uses SPI to read a value from one of the TSC2101 registers
//

static WORD TSC2101Read(HANDLE hSPI, WORD wReg)
{
    DWORD   dwCommand;

    dwCommand = (DWORD)(TSC2101_READ | TSC2101_PAGE2 | wReg) << 16;
    SPITransfer(hSPI, &dwCommand);
    return (WORD)dwCommand;
}

//------------------------------------------------------------------------------
//
//  test code to generate a tone without DMA
//

//#define TEST_TONE

#if defined(TEST_TONE)

#define TEST_SOUND  awSound     // awTone (below) or awSound (from sound.h)
#define TEST_READ_COUNT     44100
#if (TEST_SOUND == awSound)
#define TEST_WRITE_COUNT    1
#else
#define TEST_WRITE_COUNT    1000
#endif

#include "sound.h"  // awSound array

WORD awInput[TEST_READ_COUNT];
WORD awTone[] =
{
    0x0ce4, 0x0ce4, 0x1985, 0x1985, 0x25A1, 0x25A1, 0x30FD, 0x30FE,
    0x3B56, 0x3B55, 0x447A, 0x447A, 0x4C3B, 0x4C3C, 0x526D, 0x526C,
    0x56F1, 0x56F1, 0x59B1, 0x59B1, 0x5A9E, 0x5A9D, 0x59B1, 0x59B2,
    0x56F3, 0x56F2, 0x526D, 0x526D, 0x4C3B, 0x4C3B, 0x447C, 0x447C,
    0x3B5A, 0x3B59, 0x30FE, 0x30FE, 0x25A5, 0x25A6, 0x1989, 0x198A,
    0x0CE5, 0x0CE3, 0x0000, 0x0000, 0xF31C, 0xF31C, 0xE677, 0xE676,
    0xDA5B, 0xDA5B, 0xCF03, 0xCF03, 0xC4AA, 0xC4AA, 0xBB83, 0xBB83,
    0xB3C5, 0xB3C5, 0xAD94, 0xAD94, 0xA90D, 0xA90E, 0xA64F, 0xA64E,
    0xA562, 0xA563, 0xA64F, 0xA64F, 0xA910, 0xA90F, 0xAD93, 0xAD94,
    0xB3C4, 0xB3C4, 0xBB87, 0xBB86, 0xC4AB, 0xC4AB, 0xCF03, 0xCF03,
    0xDA5B, 0xDA5A, 0xE67B, 0xE67B, 0xF31B, 0xF3AC, 0x0000, 0x0000,
    0x0CE4, 0x0CE4, 0x1985, 0x1985, 0x25A1, 0x25A1, 0x30FD, 0x30FE,
    0x3B56, 0x3B55, 0x447A, 0x447A, 0x4C3B, 0x4C3C, 0x526D, 0x526C,
    0x56F1, 0x56F1, 0x59B1, 0x59B1, 0x5A9E, 0x5A9D, 0x59B1, 0x59B2,
    0x56F3, 0x56F2, 0x526D, 0x526D, 0x4C3B, 0x4C3B, 0x447C, 0x447C,
    0x3B5A, 0x3B59, 0x30FE, 0x30FE, 0x25A5, 0x25A6, 0x1989, 0x198A,
    0x0CE5, 0x0CE3, 0x0000, 0x0000, 0xF31C, 0xF31C, 0xE677, 0xE676,
    0xDA5B, 0xDA5B, 0xCF03, 0xCF03, 0xC4AA, 0xC4AA, 0xBB83, 0xBB83,
    0xB3C5, 0xB3C5, 0xAD94, 0xAD94, 0xA90D, 0xA90E, 0xA64F, 0xA64E,
    0xA562, 0xA563, 0xA64F, 0xA64F, 0xA910, 0xA90F, 0xAD93, 0xAD94,
    0xB3C4, 0xB3C4, 0xBB87, 0xBB86, 0xC4AB, 0xC4AB, 0xCF03, 0xCF03,
    0xDA5B, 0xDA5A, 0xE67B, 0xE67B, 0xF31B, 0xF3AC, 0x0000, 0x0000,
    0x0CE4, 0x0CE4, 0x1985, 0x1985, 0x25A1, 0x25A1, 0x30FD, 0x30FE,
    0x3B56, 0x3B55, 0x447A, 0x447A, 0x4C3B, 0x4C3C, 0x526D, 0x526C,
    0x56F1, 0x56F1, 0x59B1, 0x59B1, 0x5A9E, 0x5A9D, 0x59B1, 0x59B2,
    0x56F3, 0x56F2, 0x526D, 0x526D, 0x4C3B, 0x4C3B, 0x447C, 0x447C,
    0x3B5A, 0x3B59, 0x30FE, 0x30FE, 0x25A5, 0x25A6, 0x1989, 0x198A,
    0x0CE5, 0x0CE3, 0x0000, 0x0000, 0xF31C, 0xF31C, 0xE677, 0xE676,
    0xDA5B, 0xDA5B, 0xCF03, 0xCF03, 0xC4AA, 0xC4AA, 0xBB83, 0xBB83,
    0xB3C5, 0xB3C5, 0xAD94, 0xAD94, 0xA90D, 0xA90E, 0xA64F, 0xA64E,
    0xA562, 0xA563, 0xA64F, 0xA64F, 0xA910, 0xA90F, 0xAD93, 0xAD94,
    0xB3C4, 0xB3C4, 0xBB87, 0xBB86, 0xC4AB, 0xC4AB, 0xCF03, 0xCF03,
    0xDA5B, 0xDA5A, 0xE67B, 0xE67B, 0xF31B, 0xF3AC, 0x0000, 0x0000
};

static BOOL McBSPRead(OMAP2420_McBSP_REGS *pMcBSP, PWORD pwData)
{
    WORD    wReg, wAttempts;

    *pwData = INREG16(&pMcBSP->usMCBSP_DRR1);
    wReg = INREG16(&pMcBSP->usMCBSP_SPCR1);
    if (wReg & MCBSP_RSYNCERR)
    {
        OUTREG16(&pMcBSP->usMCBSP_SPCR1, wReg & (~MCBSP_RSYNCERR));
        DEBUGMSG(ZONE_AC, (L"AC: McBSPRead FAILED on RSYNC\r\n"));
    }
    else
    {
        for (wAttempts = 0; wAttempts < 1000; wAttempts++)
        {
            wReg = INREG16(&pMcBSP->usMCBSP_SPCR1);
            if (wReg & MCBSP_RRDY)
            {
                return TRUE;
            }
        }
        OUTREG16(&pMcBSP->usMCBSP_SPCR1, wReg & (~MCBSP_RRST));
        Sleep(1);
        OUTREG16(&pMcBSP->usMCBSP_SPCR1, wReg | MCBSP_RRST);
        Sleep(1);
        DEBUGMSG(ZONE_AC, (L"AC: McBSPRead FAILED on RRDY\r\n"));
    }

    return FALSE;
}

static BOOL McBSPWrite(OMAP2420_McBSP_REGS *pMcBSP, WORD wData)
{
    WORD    wReg, wAttempts;

    OUTREG16(&pMcBSP->usMCBSP_DXR1, wData);
    wReg = INREG16(&pMcBSP->usMCBSP_SPCR2);
    if (wReg & MCBSP_XSYNCERR)
    {
        OUTREG16(&pMcBSP->usMCBSP_SPCR2, wReg & (~MCBSP_XSYNCERR));
        DEBUGMSG(ZONE_AC, (L"AC: McBSPWrite FAILED on XSYNC\r\n"));
    }
    else
    {
        for (wAttempts = 0; wAttempts < 1000; wAttempts++)
        {
            wReg = INREG16(&pMcBSP->usMCBSP_SPCR2);
            if (wReg & MCBSP_XRDY)
            {
                return TRUE;
            }
        }
        OUTREG16(&pMcBSP->usMCBSP_SPCR2, wReg & (~MCBSP_XRST));
        Sleep(1);
        OUTREG16(&pMcBSP->usMCBSP_SPCR2, wReg | MCBSP_XRST);
        Sleep(1);
        DEBUGMSG(ZONE_AC, (L"AC: McBSPWrite FAILED on XRDY\r\n"));
    }

    return FALSE;
}

static void TestTone(OMAP2420_McBSP_REGS *pMcBSP)
{
    DWORD   i, j;

    DEBUGMSG(ZONE_AC, (L"AC: +TestTone\r\n"));
    Sleep(10);

    // write the test data
    for (i = 0; i < TEST_WRITE_COUNT; i++)
    {
        for (j = 0; j < sizeof(TEST_SOUND) / sizeof(WORD); j++)
        {
            if (!McBSPWrite(pMcBSP, TEST_SOUND[j]))
            {
                DEBUGMSG(ZONE_AC, (L"AC: -TestTone write %u %u\r\n", i, j));
                return;
            }
        }
    }

    // read some input data
    DEBUGMSG(ZONE_AC, (L"AC: start recording\r\n"));
    for (i = 0; i < TEST_READ_COUNT; i++)
    {
        if (!McBSPRead(pMcBSP, awInput + i))
        {
            DEBUGMSG(ZONE_AC, (L"AC: -TestTone read %u\r\n", i));
            return;
        }
    }
    DEBUGMSG(ZONE_AC, (L"AC: stop recording\r\n"));
    for (i = 0; i < TEST_READ_COUNT; i++)
    {
        if (!(i & 0xF))
        {
            DEBUGMSG(ZONE_AC, (L"\r\n\t"));
        }
        DEBUGMSG(ZONE_AC, (L"%04X, ", awInput[i]));
    }

    // write back what was read
    for (i = 0; i < TEST_READ_COUNT; i++)
    {
        if (!McBSPWrite(pMcBSP, awInput[i]))
        {
            DEBUGMSG(ZONE_AC, (L"AC: -TestTone writeback %u\r\n", i));
            return;
        }
    }

    Sleep(10);
    DEBUGMSG(ZONE_AC, (L"AC: -TestTone\r\n"));
}

#endif  // TEST_TONE

//------------------------------------------------------------------------------
//
//  Function:  HardwareContext::CreateHWContext
//  
//  This function creates a hardware context using a platform specific derived
//  class based on HardwareContext. In order to support different hardware 
//  configurations in one binary the approbriate class could be created here.
//
//

BOOL HardwareContext::CreateHWContext(DWORD Index)
{
    if (g_pHWContext)
    {
        return TRUE;
    }

    g_pHWContext = new ACAudioHWContext((LPTSTR)Index);

    if (!g_pHWContext)
    {
        return FALSE;
    }
    
    return g_pHWContext->Init(Index);
}

//------------------------------------------------------------------------------
//
//  Function: HWMapControllerRegs() 
//  
//  map hardware registers to device driver virtual addresses
//

BOOL 
ACAudioHWContext::HWMapControllerRegs()
{
    PHYSICAL_ADDRESS pa;

    DEBUGMSG(ZONE_AC, (L"+ACAudioHWContext::HWMapControllerRegs()\r\n"));

    // set the power state
    if (!m_hParent)
    {
        DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::HWMapControllerRegs: "
            L"ERROR setting the power state.\r\n"
        ));
        goto ErrExit;
    }
    m_CurPowerState = D2; 
    SetDevicePowerState(m_hParent, D2 , NULL);

    // get the McBSP pointer
    pa.HighPart= 0;
    pa.LowPart = AUDIO_MCBSP_REGS_PA;
    m_pMCBSPRegisters = (OMAP2420_McBSP_REGS *)MmMapIoSpace(pa, N1KB, FALSE);
    if (!m_pMCBSPRegisters)
    {
        DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::HWMapControllerRegs: "
            L"ERROR mapping MCBSP registers.\r\n"
        ));
        goto ErrExit;
    }

    // get the PRCM registers pointer
    pa.LowPart = OMAP2420_PRCM_REGS_PA;
    m_pPRCMRegs = (OMAP2420_PRCM_REGS *)MmMapIoSpace(pa, sizeof(OMAP2420_PRCM_REGS), FALSE);
    if (!m_pPRCMRegs)
    {
        DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::HWMapControllerRegs: "
            L"Allocating PRCM register failed.\r\n"
        ));
        goto ErrExit;
    }

    // open the SPI device
    m_hSPI = SPIOpen();
    if (!m_hSPI)
    {
        DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::HWMapControllerRegs: "
            L"Failed to open the SPI device driver.\r\n"
        ));
        goto ErrExit;
    }

    // configure the SPI device
    if (!SPISetSlaveAddress(m_hSPI, 0))
    {
        DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::HWMapControllerRegs: "
            L"Failed to set the SPI slave address.\r\n"
        ));
        goto ErrExit;
    }

    return TRUE;

ErrExit:
    // reset all mappings in case of error
    m_pMCBSPRegisters = NULL;
    m_pPRCMRegs = NULL;
    m_hSPI = NULL; 
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function: SetCodecPower 
// 
//  Set CODEC port power state.
//

VOID 
ACAudioHWContext::SetCodecPower(BOOL fPowerOn)
{
    DEBUGMSG(ZONE_AC, (L"+ACAudioHWContext::SetCodecPower(%x)\r\n", fPowerOn));

    // power on or off the BSP and CODEC
    if (fPowerOn)
    {
        CLRREG16(&m_pMCBSPRegisters->usMCBSP_PCR, MCBSP_IDLEEN);
        TSC2101Write(m_hSPI, TSC2101_AUDCTRL_POWER, CPC_SP1PWDN | CPC_SP2PWDN);
    }
    else
    {
        TSC2101Write(m_hSPI, TSC2101_AUDCTRL_POWER, CPC_MBIAS_HND | CPC_MBIAS_HED | CPC_ASTPWD |
                    CPC_SP1PWDN | CPC_SP2PWDN | CPC_DAPWDN | CPC_ADPWDN | CPC_VGPWDN |
                    CPC_COPWDN | CPC_LSPWDN);
        SETREG16(&m_pMCBSPRegisters->usMCBSP_PCR, MCBSP_IDLEEN);
    }
}

//------------------------------------------------------------------------------
//
//  Function: InitCodecPort() 
//  
//  Configures the codec port. 
//

void 
ACAudioHWContext::InitCodecPort()
{
    UINT16 uiTmp;

    DEBUGMSG(ZONE_AC, (L"+ACAudioHWContext::InitCodecPort()\r\n"));

    // power down everything
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_POWER, CPC_MBIAS_HND | CPC_MBIAS_HED | CPC_ASTPWD |
                    CPC_SP1PWDN | CPC_SP2PWDN | CPC_DAPWDN | CPC_ADPWDN | CPC_VGPWDN |
                    CPC_COPWDN | CPC_LSPWDN);

    // headset input not muted, AGC for Headset In off
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_HEADSET, HGC_ADPGA_HED(0x7F));

    // handset input not muted, AGC for Handset In off
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_HANDSET, HNGC_ADPGA_HND(0x7F));

    // mute analog sidetone, select MIC_INHED input for headset
    // Cell Phone In not connected
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_MIXER, MPC_ASTMU | MPC_ASTG(0x600) | MPC_MICADC |
                    MPC_MICSEL(1));

    // ADC, DAC, Analog Sidetone, cellphone, buzzer
    // softstepping enabled, 1dB AGC hysteresis, MICes bias 2V 
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_4, AC4_MB_HED(0));

    // Set codec output volume
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_DAC, DGC_DALVL(0) | DGC_DARVL(0));

    // DAC left and right routed to SPK2, SPK1/2 unmuted
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_5, AC5_DAC2SPK1(3) | AC5_AST2SPK1 | AC5_KCL2SPK1 |
                    AC5_DAC2SPK2(3) | AC5_AST2SPK2 | AC5_KCL2SPK2 | AC5_HDSCPTC);

    // OUT8P/N muted, CPOUT muted
//  TSC2101Write(m_hSPI, TSC2101_AUDCTRL_6, AC6_MUTSPK2 |
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_6, AC6_MUTSPK2 | AC6_SPL2LSK | AC6_AST2LSK |
                    AC6_LDSCPTC | AC6_VGNDSCPTC);

    // Headset/Hook switch detect disabled
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_7, 0);

    // set I2S, word length, and reference sampling rate (RFS) divisor
    if (BITSPERSAMPLE == 20) uiTmp = 1;
    else if (BITSPERSAMPLE == 24) uiTmp = 2;
    else if (BITSPERSAMPLE == 32) uiTmp = 3;
    else uiTmp = 0;
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_1, AC1_WLEN(uiTmp) | AC1_DACFS(0) | AC1_ADCFS(0));

    // make the TSC2101 the master vs. the McBSP, set the RFS to 44100 or 48000
    uiTmp = AC3_SLVMS | ((SAMPLERATE == 44100) ? AC3_REFFS : 0);
    TSC2101Write(m_hSPI, TSC2101_AUDCTRL_3, uiTmp);

    // program the PLL's
    if (SAMPLERATE == 44100)
    {
        // 44.1 KHz, 12 MHz MCLK, 5264 D_VAL
        TSC2101Write(m_hSPI, TSC2101_AUDCTRL_PLL0, PLL1_PLLSEL | PLL1_PVAL(1) | PLL1_I_VAL(7));
        TSC2101Write(m_hSPI, TSC2101_AUDCTRL_PLL1, PLL2_D_VAL(5264));
    }
    else
    {
        // 48 KHz, 12 MHz MCLK, 1920 D_VAL
        TSC2101Write(m_hSPI, TSC2101_AUDCTRL_PLL0, PLL1_PLLSEL | PLL1_PVAL(1) | PLL1_I_VAL(8));
        TSC2101Write(m_hSPI, TSC2101_AUDCTRL_PLL1, PLL2_D_VAL(1920));
    }

    // go into idle mode and configure the clocks
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_PCR, MCBSP_IDLEEN | MCBSP_CLKRM | MCBSP_SCLKME |
                MCBSP_FSXP | MCBSP_FSRP | MCBSP_CLKXP | MCBSP_CLKRP);

    if (BITSPERSAMPLE == 20) uiTmp = MCBSP_WORD_20;
    else if (BITSPERSAMPLE == 24) uiTmp = MCBSP_WORD_24;
    else if (BITSPERSAMPLE == 32) uiTmp = MCBSP_WORD_32;
    else uiTmp = MCBSP_WORD_16;

    // receive 1 word of BITSPERSAMPLE in a frame,
    // in 2 phases (1 word each) with a 1-bit delay
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_RCR1, MCBSP_RFRLEN1(0) | MCBSP_RWDLEN1(uiTmp));
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_RCR2, MCBSP_RPHASE | MCBSP_RFRLEN2(0) |
                MCBSP_RWDLEN2(uiTmp) | MCBSP_RDATDLY(1));

    // transmit 1 word of BITSPERSAMPLE in a frame,
    // in 2 phases (1 word each) with a 1-bit delay
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_XCR1, MCBSP_XFRLEN1(0) | MCBSP_XWDLEN1(uiTmp));
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_XCR2, MCBSP_XPHASE | MCBSP_XFRLEN2(0) |
                MCBSP_XWDLEN2(uiTmp) | MCBSP_XDATDLY(1) | MCBSP_XFIG);

    // set the clocks
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_SRGR1, MCBSP_FWID(BITSPERSAMPLE - 1) |
                MCBSP_CLKGDV(0));
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_SRGR2, MCBSP_GSYNC | MCBSP_CLKSP |
                MCBSP_FSGM | MCBSP_FPER(BITSPERSAMPLE * 2 - 1));

    //Left Justify, Clockstop with no delay, Receiver Disabled
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_SPCR1, MCBSP_RINTM(3));

    // Set transmit interrupt on XSYNCERR.
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_SPCR2, MCBSP_FREE | MCBSP_XINTM(3));

    // Delay while new divisors take effect.
    Sleep(100);
}

//------------------------------------------------------------------------------
//
//  Function: InitModemPort() 
//  
//  Configures the modem port. 
//

void 
ACAudioHWContext::InitModemPort()
{
    DEBUGMSG(ZONE_AC, (L"+ACAudioHWContext::InitModemPort()\r\n"));

    // Modem Port Control register
/*    USHORT usVal = 0;

    OUTREG16(&m_pMCBSPRegisters->usMCBSP_MPCTR,usVal);

    // Modem Main Port Configuration register
    usVal = INREG16(&m_pMCBSPRegisters->usMCBSP_MPMCCFR);
    usVal &= 8000;
                    //  Data justify left, filled with zero 
                    //  Expand and compand disabled.
#ifndef NO_EAC_MIXING
    usVal |= BIT8;  //  Master mode(Lead2 GSM voice is slave)
#endif
    usVal |= BIT7;  //  Frame Sync rising
    usVal |= BIT6;  //  Frame Sync active high
    usVal |= BIT5;  //  Bit clock polarity rising
    usVal |= 0x0F;  //  16 bit
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_MPMCCFR,usVal);

    // Set the registers first before enabling the 
    // channel, otherwise 
    usVal = INREG16(&m_pMCBSPRegisters->usMCBSP_MPCTR);
    usVal |= BIT3;  // Prescale clock divisor
    usVal |= BIT7;  // Enable main channel
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_MPCTR,usVal);

    // Enable the clock at the last step
    usVal |= BIT0;  // Clock running
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_MPCTR,usVal);
*/
}

//------------------------------------------------------------------------------
//
//  Function: InitBluetoothPort() 
//  
//  Configures the bluetooth port. 
//


void 
ACAudioHWContext::InitBluetoothPort()
{
    DEBUGMSG(ZONE_AC, (L"ACAudioHWContext::InitBluetoothPort()\r\n"));

    // Bluetooth Port Control register
/*    USHORT usVal = 0;

    OUTREG16(&m_pMCBSPRegisters->usMCBSP_BPCTR,usVal);

    // Bluetooth Main Port Configuration register
    usVal = INREG16(&m_pMCBSPRegisters->usMCBSP_BPMCCFR);
    usVal &= 8000;
                    //  Data justify left, filled with zero 
                    //  Expand and compand disabled.
                    //  Slave mode (Syren is master)
    usVal |= BIT7;  //  Frame Sync rising
    usVal |= BIT6;  //  Frame Sync active high
    usVal |= BIT5;  //  Clock Sync rising
    usVal |= 0x0F;  //  16 bit,
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_BPMCCFR,usVal);

    // Set the registers first before enabling the 
    // channel
    usVal = INREG16(&m_pMCBSPRegisters->usMCBSP_BPCTR);
    usVal |= BIT3;  // Prescale clock divisor 16
    usVal |= BIT7;  // Enable main channel
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_BPCTR,usVal);

    // Enable the clock at the last step
    usVal |= BIT0;  // Clock running
    OUTREG16(&m_pMCBSPRegisters->usMCBSP_BPCTR,usVal);
*/    
}

//------------------------------------------------------------------------------
//
//  Function:  HWInitController
//  
//  Main init of uWire controller for OZ Board based on SAMPLERATE and MCLK.
//  Defaults are 16bit/stereo samples.
//  
//

void 
ACAudioHWContext::HWInitController()
{
    DEBUGMSG(ZONE_AC, (L"+ACAudioHWContext::HWInitController()\r\n"));

    // power everything up and configure it
    HWPowerUp();
    InitCodecPort();
    InitModemPort();
    InitBluetoothPort();

#if defined(TEST_TONE)
    HWEnableInputChannel(TRUE);
    HWEnableOutputChannel(TRUE);
    TestTone(m_pMCBSPRegisters);
    HWEnableInputChannel(FALSE);
    HWEnableOutputChannel(FALSE);
#endif

#ifdef DEBUG
    DumpMCBSPRegisters();
#endif
}

//------------------------------------------------------------------------------
//
//  Function:  HWPowerUp()
//  
//  Power up the uWire controller using MCLK.
//

void 
ACAudioHWContext::HWPowerUp()
{
    DEBUGMSG(ZONE_AC, (L"+ACAudioHWContext::HWPowerUp()\r\n"));

    SetControllerClocks(TRUE);
}

//------------------------------------------------------------------------------
//
//  Function:  HWPowerDown
//  
//
//  Power down the EAC controller using MCLK.
//

void ACAudioHWContext::HWPowerDown()
{
    DEBUGMSG(ZONE_AC,(L"AC: HWPowerDown()\r\n"));

    SetControllerClocks(FALSE);
}

//------------------------------------------------------------------------------
//
//  Function:  SetControllerClocks
//  
//  Set the Oscillator and MCLK of the EAC controller. 
//

void 
ACAudioHWContext::SetControllerClocks(BOOL fOn)
{
    DEBUGMSG(ZONE_AC,(L"AC: SetControllerClocks (%x)\r\n", fOn));

    DWORD regBit, cbRet;

    if (fOn)
    {
        // enable the McBSP clocks
        regBit = AUDIO_PRCM_FCLKEN_MCBSP;
        KernelIoControl(IOCTL_FCLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
        regBit = AUDIO_PRCM_ICLKEN_MCBSP;
        KernelIoControl(IOCTL_ICLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
    }
    else 
    {
        // disable the McBSP clocks
        regBit = AUDIO_PRCM_FCLKEN_MCBSP;
        KernelIoControl(IOCTL_FCLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
        regBit = AUDIO_PRCM_ICLKEN_MCBSP;
        KernelIoControl(IOCTL_ICLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
    }
}

//------------------------------------------------------------------------------
//
//  Function: SetRecordMemoPath  
//  
//  
//  Control the record memo path using K6 to connect modem input with wave input
//

void 
ACAudioHWContext::SetRecordMemoPath(BOOL fSetOn)
{
    DEBUGMSG(ZONE_AC,(L"AC: SetRecordMemoPath(%x)\r\n", 1));

/*    USHORT usVal;
 
    usVal = INREG16(&m_pMCBSPRegisters->usMCBSP_AMSCFR);
    
    // Let a memo or talk get through
    if(fSetOn)
    {
        usVal |= BIT5;
        usVal |= BIT7;  
        usVal &=~BIT1; // we must turn off K2 here due to sidetone

        // dev note: enabling K8 may cause extensive sideton feedback on some devices
        // it may be necessary to open K2 to avoid this effect
    }
    else
    {
        usVal &= ~BIT5;
        usVal &= ~BIT7;
        usVal |=  BIT1;
    }

    OUTREG16(&m_pMCBSPRegisters->usMCBSP_AMSCFR,usVal);
*/
}

//------------------------------------------------------------------------------
//
//  Function: HWEnableInputChannel 
//  
//  Enable/Disable audio input channel
//
//
//

void 
ACAudioHWContext::HWEnableInputChannel(BOOL fEnable)
{
    DEBUGMSG(ZONE_AC,(L"AC: HWEnableInputChannel (%x)\r\n", 1));

    // enable or disable the receiver
    if (fEnable)
    {
        SETREG16(&m_pMCBSPRegisters->usMCBSP_SPCR1, MCBSP_RRST);
    }
    else
    {
        CLRREG16(&m_pMCBSPRegisters->usMCBSP_SPCR1, MCBSP_RRST);
    }
}

//------------------------------------------------------------------------------
//
//  Function: EnableOutputChannel 
//  
//  Enable/Disable audio output channel
//
//

void 
ACAudioHWContext::HWEnableOutputChannel(BOOL fEnable)
{
    DEBUGMSG(ZONE_AC,(L"AC: HWEnableOutputChannel (%x)\r\n", fEnable));

    // enable or disable the transmitter
    if (fEnable)
    {
        SETREG16(&m_pMCBSPRegisters->usMCBSP_SPCR2, MCBSP_XRST | MCBSP_GRST | MCBSP_FRST);
    }
    else
    {
        CLRREG16(&m_pMCBSPRegisters->usMCBSP_SPCR2, MCBSP_XRST);
    }
}

//------------------------------------------------------------------------------
//
//  Function:  ConfigEacBTAuSpiPins
//  
//  Configures pins to either EAC BT auSPI or GPIO functionality.
//

void 
ACAudioHWContext::ConfigEacBTAuSpiPins(PIN_FUNC PinFunction)
{
    DEBUGMSG(ZONE_AC, (L"ACAudioHWContext::ConfigEacBTAuSpiPins(%x)\r\n", PinFunction));
/*
    // Disable AuSpi clock
    CLRREG16(&m_pMCBSPRegisters->usMCBSP_BPCTR,BIT0);

    switch(PinFunction)
    {
    case V3U3V2T4_TO_EAC_BT_AUSPI:
        CLRREG32(&m_pCFGRegs->MODE1,MODE1_INTERNAL_BTAUSPI_SOURCE_BITS); 
        SETREG32(&m_pCFGRegs->MODE1,MODE1_V3U3V2T4_AS_BTAUSPI_SOURCE); 
        break;
        
    case W6R9Y6Y5_TO_EAC_BT_AUSPI:
        CLRREG32(&m_pCFGRegs->MODE1, MODE1_INTERNAL_BTAUSPI_SOURCE_BITS); 
        SETREG32(&m_pCFGRegs->MODE1, MODE1_W6R9Y6Y5_AS_BTAUSPI_SOURCE);         
        break;  
    } 

    // Enable AuSpi clocks
    SETREG16(&m_pMCBSPRegisters->usMCBSP_BPCTR,BIT0);
*/    
}

//------------------------------------------------------------------------------
//
//  Function:  SetAMRcapture
//  

BOOL 
ACAudioHWContext::SetAMRcapture(BOOL fStart)
{
    DEBUGMSG(ZONE_AC, (L"ACAudioHWContext::SetAMRcapture(%d)\r\n", fStart));

/*    // Start AMR capture. 
    if (fStart)
    {
        CLRREG32(&m_pCFGRegs->IO_CONFIG2,(BIT7|BIT6|BIT5));       // Bt port

        // GSM SYREN VOICE.
        SETREG32(&m_pCFGRegs->IO_CONFIG2, BIT5);                    

        // Select Bt pins to lead2 vspi port.
        CLRREG32(&m_pCFGRegs->MODE1,(BIT1|BIT0));               
    }

    // Stop AMR capture. 
    else
    {
        CLRREG32(&m_pCFGRegs->IO_CONFIG2,(BIT7|BIT6|BIT5));       // Bt port

        // Select Bt pins to lead2 vspi port.
        SETREG32(&m_pCFGRegs->MODE1,(BIT1|BIT0));                      

        // Select EAC modem to lead2 vspi port.
        SETREG32(&m_pCFGRegs->MODE1,BIT0);                       
    }
*/
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACAudioHWContext::DumpMCBSPRegisters()
//  
//  debug helper function dumps all of the configuration registers  
//

void 
ACAudioHWContext::DumpMCBSPRegisters()
{
    WORD    wReg, wData;

    DEBUGMSG(ZONE_AC, (L"AC: McBSP Registers\r\n"));
    DEBUGMSG(ZONE_AC, (L"AC:    DRR2:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_DRR2)));
    DEBUGMSG(ZONE_AC, (L"AC:    DRR1:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_DRR1)));
    DEBUGMSG(ZONE_AC, (L"AC:    DXR2:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_DXR2)));
    DEBUGMSG(ZONE_AC, (L"AC:    DXR1:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_DXR1)));
    DEBUGMSG(ZONE_AC, (L"AC:    SPCR1: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_SPCR1)));
    DEBUGMSG(ZONE_AC, (L"AC:    SPCR2: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_SPCR2)));
    DEBUGMSG(ZONE_AC, (L"AC:    PCR:   %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_PCR)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCR1:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCR1)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCR2:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCR2)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCR1:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCR1)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCR2:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCR2)));
    DEBUGMSG(ZONE_AC, (L"AC:    SRGR1: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_SRGR1)));
    DEBUGMSG(ZONE_AC, (L"AC:    SRGR2: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_SRGR2)));
    DEBUGMSG(ZONE_AC, (L"AC:    MCR1:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_MCR1)));
    DEBUGMSG(ZONE_AC, (L"AC:    MCR2:  %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_MCR2)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCERA: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCERA)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCERB: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCERB)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCERC: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCERC)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCERD: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCERD)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCERE: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCERE)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCERF: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCERF)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCERG: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCERG)));
    DEBUGMSG(ZONE_AC, (L"AC:    RCERH: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_RCERH)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCERA: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCERA)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCERB: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCERB)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCERC: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCERC)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCERD: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCERD)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCERE: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCERE)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCERF: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCERF)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCERG: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCERG)));
    DEBUGMSG(ZONE_AC, (L"AC:    XCERH: %04X\r\n", INREG16(&m_pMCBSPRegisters->usMCBSP_XCERH)));
    DEBUGMSG(ZONE_AC, (L"AC: TSC2101 Page 2 Registers:\r\n"));
    for (wReg = 0; wReg < 0x28; wReg++)
    {
        wData = TSC2101Read(m_hSPI, wReg << 5);
        DEBUGMSG(ZONE_AC, (L"AC:    Register[%02X] = %04X\r\n", wReg, wData));
    }
}
