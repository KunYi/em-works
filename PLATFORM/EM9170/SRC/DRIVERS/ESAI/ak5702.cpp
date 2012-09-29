//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale SemiconductorGI P, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: ak5702.cpp
//
//  Implementation of AKM stereo CODEC AK5702.
//
//------------------------------------------------------------------------------
#include "bsp.h"
#include "ak5702.h"
#include "i2cbus.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define I2C_FID_AK5702              _T("I2C1:") 
#define AK5702_I2C_ADDRESS          (0x13)


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function: CAK5702
//
//  Construction function of class AK5702.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
CAK5702::CAK5702()
{
    // Init state flags
    m_bInitialed = FALSE;
    
    // Open I2C handle
    m_hI2C = I2COpenHandle(I2C_FID_AK5702);
    if (m_hI2C == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(TRUE,(_T("%s: Open I2C handle failed!\r\n"), 
            __WFUNCTION__));
        return;
    }

    // Set I2C master mode
    if (!I2CSetMasterMode(m_hI2C))
    {
        ERRORMSG(TRUE, (_T("%s: Set I2C master mode failed!\r\n"), 
            __WFUNCTION__));
        goto cleanUp;
    }

    return;

cleanUp:
    I2CCloseHandle(m_hI2C);
    return;
}


//-----------------------------------------------------------------------------
//
//  Function: ~CAK5702
//
//  Deconstruction function of class AK5702.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
CAK5702::~CAK5702()
{
    // Close I2C handle
    I2CCloseHandle(m_hI2C);
}


//-----------------------------------------------------------------------------
//
//  Function: ReadRegister
//
//  This function returns value of the specified register.
//
//  Parameters:
//      Reg
//          [in] The register address.
//
//  Returns:
//      The value of the register. A value of -1 indicates failure.
//
//-----------------------------------------------------------------------------
BYTE CAK5702::ReadRegister(BYTE Reg)
{
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket[2];
    BYTE dataIn, dataOut;
    INT result[2];

    // Fill packets for register read
    dataIn = Reg & AK5702_REG_ADDR_MASK;
    I2CPacket[0].pbyBuf = &dataIn;
    I2CPacket[0].wLen = sizeof(dataIn);
    I2CPacket[0].byAddr = AK5702_I2C_ADDRESS;
    I2CPacket[0].byRW = I2C_RW_WRITE;
    I2CPacket[0].lpiResult = &result[0];

    I2CPacket[1].pbyBuf = &dataOut;
    I2CPacket[1].wLen = sizeof(dataOut);
    I2CPacket[1].byAddr = AK5702_I2C_ADDRESS;
    I2CPacket[1].byRW = I2C_RW_READ;
    I2CPacket[1].lpiResult = &result[1];

    // Fill transfer block
    I2CXferBlock.pI2CPackets = I2CPacket;
    I2CXferBlock.iNumPackets = 2;

    // Transfer
    I2CTransfer(m_hI2C, &I2CXferBlock);
    if (result[0] || result[1])
    {
        ERRORMSG(TRUE, (_T("%s: Read AK5702 register 0x%x failed, err code(%d)(%d)!\r\n"), 
             __WFUNCTION__,Reg,result[0],result[1]));
        return (BYTE)-1;
    }

    return dataOut;  
}


//-----------------------------------------------------------------------------
//
//  Function: WriteRegister
//
//  This function writes the specified register.
//
//  Parameters:
//      Reg
//          [in] The register address.
//
//      Val
//          [in] The value to write.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL CAK5702::WriteRegister(BYTE Reg, BYTE Val)
{
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket;
    BYTE data[2];
    INT result;

    // Fill packet for register write
    data[0] = Reg & AK5702_REG_ADDR_MASK;
    data[1] = Val;
    I2CPacket.wLen = sizeof(data);
    I2CPacket.pbyBuf = data;
    I2CPacket.byRW = I2C_RW_WRITE;
    I2CPacket.byAddr = AK5702_I2C_ADDRESS;
    I2CPacket.lpiResult = &result;

    // Fill transfer block
    I2CXferBlock.pI2CPackets = &I2CPacket;
    I2CXferBlock.iNumPackets = 1;

    // Transfer
    I2CTransfer(m_hI2C, &I2CXferBlock);
    if (result)
    {
        ERRORMSG(TRUE, (_T("%s: Write AK5702 register 0x%x failed, err code(%d)!\r\n"), 
             __WFUNCTION__,Reg,result));
        return FALSE;
    }

    return TRUE;
}



VOID CAK5702::DumpRegs(VOID)
{

#ifdef AK5702_DEBUG
    for(BYTE i=0; i<0x1F;i++)
        RETAILMSG(1,(TEXT("Reg%x:%x"),i,ReadRegister(i)));
#endif

    return;
}

//-----------------------------------------------------------------------------
//
//  Function: InitAK5702
//
//  This function initialize ak5702.
//
//  Parameters:
//      N/A
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL CAK5702::InitAK5702(VOID)
{
    
    BYTE val;

    // Reset the AK5702
    // Configure ESAI_RST pin 
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_UPLL_BYPCLK, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    // Configure ESAI_GPIO pin 
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_EXT_ARMCLK, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    
    DDKGpioWriteDataPin(ESAI_RST_GPIO_PORT, ESAI_RST_GPIO_PIN, 0);
    Sleep(10);
    DDKGpioWriteDataPin(ESAI_RST_GPIO_PORT, ESAI_RST_GPIO_PIN, 1);

    m_bInitialed = TRUE;

    m_bPLLPower = FALSE;
    m_bVComPower = FALSE;
    m_bADCPower = FALSE;

    
    //Set the default val
    m_audioProtocol = AUDIO_PROTOCOL_I2S;
    m_dwSampleRate = 44100;
    m_dwBitDepth = 16;
    m_dwChnNum = 4;
    m_dwChnMask= 0x0f;
    m_pwMode = AK5702_POWER_FULLOFF;    

    //Enable VCOM
    val= CSP_BITFVAL(AK5702_PM_PMVCM,1);
    if(!WriteRegister(AK5702_REG_PM, val)){
        goto errexit;
    }
    m_bVComPower = TRUE;

    m_pwMode = AK5702_POWER_STANDBY;

    WriteRegister(AK5702_REG_MICGAIN, 0);
    WriteRegister(AK5702_REG_MICGAINB, 0);

#ifdef AK5702_DEBUG
    DumpRegs();
#endif    
    return TRUE;

errexit:
    m_bInitialed = FALSE;
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: ConfigADC
//
//  This function configure adc for record function.
//
//  Parameters:
//    audioProtocol  
//          [in] protocol for the audio bus
//
//    dwSampleRate
//          [in] samplerate of the input wave data.
//
//    dwBitDepth  
//          [in] bitdepth of input wave data, 24 or 16
//
//    dwChnNum
//          [in] channel number of the input wave data
//
//    dwBitDepth  
//          [in] channel mask  of input wave data
//
//    pllMode
//          [in] input clock for ak5702 

//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL CAK5702::ConfigADC(AUDIO_PROTOCOL audioProtocol,
    DWORD dwSampleRate,
    DWORD dwBitDepth,
    DWORD dwChnNum, 
    DWORD dwChnMask,
    AK5702_PLL_MODE pllMode)
{
    BYTE val;

    if(!m_bInitialed)
        return FALSE;

    //RETAILMSG(1,(TEXT("Config ADC: sample(%d)bit(%d)chn(%d)mask(%x)pro(%d)pll(%d)\r\n")
    //    ,dwSampleRate,dwBitDepth,dwChnNum,dwChnMask,(DWORD)audioProtocol,
    //    (DWORD)pllMode));
    
    m_audioProtocol = audioProtocol;
    m_dwSampleRate = dwSampleRate;
    m_dwBitDepth = dwBitDepth;
    m_dwChnNum = dwChnNum;
    m_dwChnMask = dwChnMask;
    m_pllMode = pllMode;

    //config the pll setting and enable pll power
    val = (BYTE)CSP_BITFVAL(AK5702_PLL_PLL,pllMode)|
          CSP_BITFVAL(AK5702_PLL_PMPLL,1) |  //We always use PLL master mode
          CSP_BITFVAL(AK5702_PLL_MS,1);

    if(!WriteRegister(AK5702_REG_PLL,val)){
        goto errexit;
    }
    m_bPLLPower = TRUE;
    m_pwMode = AK5702_POWER_FULLON;

    //Config audio fmt
    if (audioProtocol == AUDIO_PROTOCOL_I2S){
        val = (BYTE) CSP_BITFVAL(AK5702_FMT_DIF,3) |  //i2s
              CSP_BITFVAL(AK5702_FMT_BCKP,0)|  //Bclk pol
              CSP_BITFVAL(AK5702_FMT_MSBS,0) |  //FS phase, half clk
              CSP_BITFVAL(AK5702_FMT_MIXA,0) | //no MIX
              CSP_BITFVAL(AK5702_FMT_TDM, 0)| //Not use TDM
              (1<<5); // bit5 always ==1 

        if(!WriteRegister(AK5702_REG_FMT,val)){
            goto errexit;
        }

        val = CSP_BITFVAL(AK5702_FMTB_MIXB,0) | //no MIX
              (1<<5);

        if(!WriteRegister(AK5702_REG_FMTB,val)){
            goto errexit;
        }
        

    }else if (audioProtocol == AUDIO_PROTOCOL_LEFT_ALIGNED){
        val = CSP_BITFVAL(AK5702_FMT_DIF,2) |  //i2s
              CSP_BITFVAL(AK5702_FMT_BCKP,0)|  //Bclk pol
              CSP_BITFVAL(AK5702_FMT_MSBS,0) |  //FS phase, half clk
              CSP_BITFVAL(AK5702_FMT_MIXA,0) | //no MIX
              CSP_BITFVAL(AK5702_FMT_TDM, 0)| //Not use TDM
              (1<<5);

        if(!WriteRegister(AK5702_REG_FMT,val)){
            goto errexit;
        }

        val = CSP_BITFVAL(AK5702_FMTB_MIXB,0) | //no MIX
              (1<<5);

        if(!WriteRegister(AK5702_REG_FMTB,val)){
            goto errexit;
        }


    }else{
        goto errexit;
    }
    

    //Set  FS
    switch(dwSampleRate){
        case 8000:
            val = AK5702_FS_8K;
            break;
            
        case 12000:
            val = AK5702_FS_12K;
            break;
        case 16000:
            val = AK5702_FS_16K;
            break;

        case 24000:
            val = AK5702_FS_24K;
            break;

        case 32000:
            val = AK5702_FS_32K;
            break;

        case 48000:
            val = AK5702_FS_48K;
            break;

        case 44100:
            val = AK5702_FS_44K1;
            break;

        default:
            goto errexit;            

    }

    if(dwBitDepth > 16){
        val |= CSP_BITFVAL(AK5702_FS_BCKO,2); //64FS
    }else{
        val |= CSP_BITFVAL(AK5702_FS_BCKO,1); //32FS
    }
    
    if(!WriteRegister(AK5702_REG_FS,val)){
        goto errexit;
    }

    //Dont output CLKO, use def val for CLKO REG

    //Vol control in-dependendt
    WriteRegister(AK5702_REG_VOL,01);
    WriteRegister(AK5702_REG_VOLB,01);

#ifdef AK5702_DEBUG
    DumpRegs();
#endif

    return TRUE;
    
errexit:
    SetPLLPower(FALSE);
    return FALSE; 
}


//-----------------------------------------------------------------------------
//
//  Function: EnalbeADC
//
//  This function enables adc.
//
//  Parameters:
//      N/A
//
//  Returns:
//      Reture TRUE if successful, FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL CAK5702::EnalbeADC(void)
{
    BYTE val;

    if(!m_bInitialed)
        return FALSE;

    //Sig sel and enalbe Mic PM
    //LIN1,RIN2->ADCA, LIN3,RIN4->ADCB
    val = CSP_BITFVAL(AK5702_SS_INAR,1) |
          CSP_BITFVAL(AK5702_SS_PMMPA,1);
    if(!WriteRegister(AK5702_REG_SIG, val)){
        goto errexit;
    }

    val = CSP_BITFVAL(AK5702_SSB_INBR,1) |
          CSP_BITFVAL(AK5702_SSB_PMMPB,1);
    if(!WriteRegister(AK5702_REG_SIGB, val)){
        goto errexit;
    }


    //MIC GAIN, USE DEFAULT
    
    // Enalbe ADC PM
    if(m_dwChnMask & AK5702_CHN_AL)
        val |= CSP_BITFVAL(AK5702_PM_PMADAL,1);
    
    if(m_dwChnMask & AK5702_CHN_AR)
        val |= CSP_BITFVAL(AK5702_PM_PMADAR,1);

    val |= CSP_BITFVAL(AK5702_PM_PMVCM,1);
    if(!WriteRegister(AK5702_REG_PM, val)){
        goto errexit;
    }

    val = 0;

    if(m_dwChnMask & AK5702_CHN_BL)
        val |= CSP_BITFVAL(AK5702_PMB_PMADBL,1);

    if(m_dwChnMask & AK5702_CHN_BR)
        val |= CSP_BITFVAL(AK5702_PMB_PMADBR,1);

    if(!WriteRegister(AK5702_REG_PMB, val)){
        goto errexit;
    }

    m_bADCPower = TRUE;

#ifdef AK5702_DEBUG
    DumpRegs();    
#endif

    return TRUE;

errexit:
    DisableADC();
    return FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function: DisableADC
//
//  This function enables adc.
//
//  Parameters:
//      N/A
//
//  Returns:
//      Reture TRUE if successful, FALSE if failed.
//
//-----------------------------------------------------------------------------
BOOL CAK5702::DisableADC(void)
{
    if(!m_bInitialed)
        return FALSE;
 
    //Disable ADC Power
    if(!WriteRegister(AK5702_REG_PM,CSP_BITFVAL(AK5702_PM_PMVCM,1))){
        goto errexit;
    }

    if(!WriteRegister(AK5702_REG_PMB,0)){
        goto errexit;
    }

    //Disable MIC Power
    if(!WriteRegister(AK5702_REG_SIG, 0)){
        goto errexit;
    }

    if(!WriteRegister(AK5702_REG_SIGB, 0)){
        goto errexit;
    }
    
    m_bADCPower = FALSE;
    
    return TRUE;

errexit:
    return FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function: SetPLLPower
//
//  This function sets the pll power.
//
//  Parameters:
//      bPowerOn
//          [in]  enalbe pll power (TRUE) or disable pll power(FALSE)
//
//  Returns:
//      none.
//
//-----------------------------------------------------------------------------
VOID CAK5702::SetPLLPower(BOOL bPowerOn)
{
    BYTE val;

    if(!m_bInitialed)
        return;
    
    if (m_bPLLPower == bPowerOn)
        return;

    if(bPowerOn){
       val = (BYTE)CSP_BITFVAL(AK5702_PLL_PLL,m_pllMode)|
          CSP_BITFVAL(AK5702_PLL_PMPLL,1) |  //We always use PLL master mode
          CSP_BITFVAL(AK5702_PLL_MS,1);

       WriteRegister(AK5702_REG_PLL,val);  
       m_bPLLPower = TRUE;
    }else{
       val = (BYTE)CSP_BITFVAL(AK5702_PLL_PLL,m_pllMode)|
          CSP_BITFVAL(AK5702_PLL_PMPLL,0) |  //We always use PLL master mode
          CSP_BITFVAL(AK5702_PLL_MS,1);

       WriteRegister(AK5702_REG_PLL,val);  
       m_bPLLPower = FALSE;        

    }
    
    return;
}

//-----------------------------------------------------------------------------
//
//  Function: SetCodecPower
//
//  This function sets the power state of codec.
//
//  Parameters:
//      pwMode
//          [in] Power state: full off (all power is off) standby(vcom is on) fullon (pll is enalbe and
//                bclk/fs is output on bus)  
//
//  Returns:
//      none.
//
//-----------------------------------------------------------------------------
VOID CAK5702::SetCodecPower(AK5702_POWER_MODE pwMode)
{
    if(!m_bInitialed)
        return;

    if(m_pwMode == pwMode){
        return;
    }    

    if(pwMode == AK5702_POWER_FULLOFF){ //all power is off
        if(m_bADCPower)
            DisableADC();

        if(m_bPLLPower)
            SetPLLPower(FALSE);

        //Disable VCOM
        WriteRegister(AK5702_REG_PM,0);    

        m_bVComPower = FALSE;
    }

    if(pwMode == AK5702_POWER_STANDBY){ //only vcom is on
        if(m_bADCPower)
            DisableADC();

        if(m_bPLLPower)
            SetPLLPower(FALSE);

        //Set VCOM ON    
        WriteRegister(AK5702_REG_PM,CSP_BITFVAL(AK5702_PM_PMVCM,1)); 
        m_bVComPower = TRUE;
    }

    if(pwMode == AK5702_POWER_FULLON){ //Pll is on while adc disabled
        if(!m_bVComPower){
            WriteRegister(AK5702_REG_PM,CSP_BITFVAL(AK5702_PM_PMVCM,1)); 
            m_bVComPower = TRUE;
        }

        if(!m_bPLLPower){
            SetPLLPower(TRUE);
        }

    }

    m_pwMode = pwMode;

    return;    

}



//-----------------------------------------------------------------------------
//
//  Function: SetInputGain
//
//  This function sets AK5702 digital input volume level.
//
//  Parameters:
//      ucVolAL
//          [in] The desired ADCA left channel volume level.
//
//      ucVolAR
//          [in] The desired ADCA right channel volume level.
//
//      ucVolBL
//          [in] The desired ADCB left channel volume level.
//
//      ucVolBR
//          [in] The desired ADCB right channel volume level.
//
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID CAK5702::SetInputGain(BYTE ucVolAL, BYTE ucVolAR,
    BYTE ucVolBL, BYTE ucVolBR, DWORD dwChnMask)
{
    if(!m_bInitialed)
        return;

    if(dwChnMask & AK5702_CHN_AL)
        WriteRegister(AK5702_REG_LINVOL, ucVolAL);
    
    if(dwChnMask & AK5702_CHN_AR)
        WriteRegister(AK5702_REG_RINVOL, ucVolAR);

    if(dwChnMask & AK5702_CHN_BL)
        WriteRegister(AK5702_REG_LINVOLB, ucVolBL);

    if(dwChnMask & AK5702_CHN_BR)
        WriteRegister(AK5702_REG_RINVOLB, ucVolBR);

    return;
}


