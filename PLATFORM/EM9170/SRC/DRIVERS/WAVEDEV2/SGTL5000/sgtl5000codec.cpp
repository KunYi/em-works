//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  sgtl5000codec.cpp
//
//  This file implements the SGTL5000 driver functions used to configure
//  SGTL5000 for initialize/playback/record/power management.
//
//------------------------------------------------------------------------------

#include "bsp.h"
#include "i2cbus.h"
#include "sgtl5000codec.h"

//=============================================================================
// Constructor and Destructor
//=============================================================================
    
//-----------------------------------------------------------------------------
//
//  Function: SGTL5000Codec
//
//  Constructor for SGTL5000 audio codec driver
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
SGTL5000Codec::SGTL5000Codec(VOID)
{
    m_bI2CInited = FALSE;
    
    m_inputPort = ADC_INPUT_MIC;    
    m_HPDetected = 1;
    m_ForceSpeaker = 0;
    
    m_workMode = WORK_MODE_DUPLEX;
    
    //set up CLKO
    //DDKClockSetCKO(1, DDK_CLOCK_CKO_SRC_UNGATED_IPG_CLK, 5);

    // Initialize the I2C connection and get a handle
    SGTL5000I2CInit();
}

//-----------------------------------------------------------------------------
//
//  Function: ~SGTL5000Codec
//
//  Destructor for SGTL5000 audio codec driver
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
SGTL5000Codec::~SGTL5000Codec(VOID)
{
    if (m_bI2CInited)
    {
        SGTL5000I2CClose();
    }
}

//=============================================================================
// I2C Register Read/Write Functions
//=============================================================================

//-----------------------------------------------------------------------------
//
// Function: SGTL5000I2CInit
//
// This function open and init I2C handle for SGTL5000 audio codec.
//
// Parameters:
//      None.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
UINT32 SGTL5000Codec::SGTL5000I2CInit(VOID)
{
    UINT32 uiStatus = SGTL5000_SUCCESS;
    DWORD dwFrequency = SGTL5000_I2C_SPEED;

    // Check if already initialized
    if (m_bI2CInited)
    {
        return uiStatus;
    }
    
    m_hI2C = I2COpenHandle(I2C1_FID);
    
    if (m_hI2C == INVALID_HANDLE_VALUE)
    {
        uiStatus = SGTL5000_I2C_HANDLE;
        return uiStatus;
    }

    if (!I2CSetMasterMode(m_hI2C))
    {
        uiStatus = SGTL5000_I2C_OPERROR;
        SGTL5000I2CClose();
        return uiStatus;
    }
    
    // Initialize the device internal fields
    if (!I2CSetFrequency(m_hI2C, dwFrequency))
    {
        uiStatus = SGTL5000_I2C_OPERROR;
        SGTL5000I2CClose();
        return uiStatus;
    }

    // create I2C critical section
    InitializeCriticalSection(&m_csI2C);

    m_bI2CInited = TRUE;
    
    return uiStatus;
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000I2CClose
//
// This function close the I2C handle for SGTL5000 audio codec.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000I2CClose(VOID)
{
    if (m_hI2C)
    {
        I2CCloseHandle(m_hI2C);
        m_hI2C = INVALID_HANDLE_VALUE;
        m_bI2CInited = FALSE;
        DeleteCriticalSection(&m_csI2C);
    }
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000WriteRegister
//
// This function writes a 16-bit value to a 16-bit register on the SGTL5000
// audio codec
//
// Parameters:
//      usRegNum
//          [in] Specifies 16-bit register address
//      usValue
//          [in] Specifies the 16-bit register value to write
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
UINT32 SGTL5000Codec::SGTL5000WriteRegister( UINT16 usRegNum, UINT16 usValue )
{    
    UINT32 uiStatus = SGTL5000_SUCCESS;
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket;
    INT iResult;
    UINT8 data[4];

    if (m_hI2C == INVALID_HANDLE_VALUE)
    {
        uiStatus = SGTL5000_I2C_HANDLE;
        return uiStatus;
    }

    // set data buffer with register value and data
    data[0] = (UINT8)(( usRegNum >> 8 ) & 0xFF);
    data[1] = (UINT8)(( usRegNum ) & 0xFF);
    data[2] = (UINT8)(( usValue >> 8 ) & 0xFF);
    data[3] = (UINT8)(( usValue ) & 0xFF);

    I2CPacket.wLen = 4;
    I2CPacket.byRW = I2C_RW_WRITE;
    I2CPacket.pbyBuf = (PBYTE)&data;
    I2CPacket.byAddr = SGTL5000_I2C_ADDR;
    // if I2C encounters an error, it will write to *lpiResult. 
    I2CPacket.lpiResult = &iResult;

    I2CXferBlock.pI2CPackets = &I2CPacket;
    I2CXferBlock.iNumPackets = 1;

    // Write register via I2C
    uiStatus = I2CTransfer(m_hI2C,&I2CXferBlock);    
    if (TRUE != uiStatus)
    {
        RETAILMSG(TRUE, (_T("SGTL5000WriteRegister(0x%x)=0x%x failed!\r\n"), 
         usRegNum, usValue));            
    }

    return uiStatus;
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000ReadRegister
//
// This function reads a 16-bit register on the SGTL5000
// audio codec
//
// Parameters:
//      usRegNum
//          [in] Specifies 16-bit register address
//      pusValue
//          [out] Returns the 16-bit register value
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
UINT32 SGTL5000Codec::SGTL5000ReadRegister( UINT16 usRegNum, UINT16* pusValue )
{
    UINT32 uiStatus = SGTL5000_SUCCESS;
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPktArray[2];
    INT32 iResult;
    UINT8 uiRegData[2];
    UINT8 uiRegValue[2];
    
    if (m_hI2C == INVALID_HANDLE_VALUE)
    {
        uiStatus = SGTL5000_I2C_HANDLE;
        return uiStatus;
    }
    
    // set data buffer with register value
    uiRegData[0] = (UINT8)(( usRegNum & 0xFF00) >> 8);
    uiRegData[1] = (UINT8)(usRegNum & 0x00FF);

    I2CPktArray[0].pbyBuf = (PBYTE) &uiRegData[0];
    I2CPktArray[0].wLen = 2; // let I2C know how much to transmit

    I2CPktArray[0].byRW = I2C_RW_WRITE;
    I2CPktArray[0].byAddr = SGTL5000_I2C_ADDR;
    // if I2C encounters an error, it will write to *lpiResult.    
    I2CPktArray[0].lpiResult = &iResult; 

    I2CPktArray[1].pbyBuf = (PBYTE) &uiRegValue;
    I2CPktArray[1].wLen = 2; // let I2C know how much to receive

    I2CPktArray[1].byRW = I2C_RW_READ;
    I2CPktArray[1].byAddr = SGTL5000_I2C_ADDR;
    // if I2C encounters an error, it will write to *lpiResult.
    I2CPktArray[1].lpiResult = &iResult; 

    I2CXferBlock.pI2CPackets = I2CPktArray;
    I2CXferBlock.iNumPackets = 2;

    // Read register via I2C
    uiStatus = I2CTransfer(m_hI2C,&I2CXferBlock);
    
    // Form 16-bit register value
    *pusValue = ((UINT16)uiRegValue[0] << 8) | (UINT16)uiRegValue[1];

    if (TRUE != uiStatus)
    {
        RETAILMSG(1, (_T("SGTL5000ReadRegister(0x%x)=0x%x failed!\r\n"), 
            usRegNum, *pusValue));
    }

    return uiStatus;
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000ModifyRegister
//
// This function modifies bit-field value of the specified register using the 
// clearmask provided.  This function first reads the register value, modifies 
// the bit-field value without affecting other bits and then writes the register 
// value back to the register
//
// Parameters:
//      usRegNum
//          [in] Specifies 16-bit register address
//      usMask
//          [in] Specifies the mask to clear the specific bit-field
//      usBitFieldValue
//          [in] Specifies the new bit-field value to be written
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
UINT32 SGTL5000Codec::SGTL5000ModifyRegister( UINT16 usRegNum, 
                               UINT16 usMask, 
                               UINT16 usBitFieldValue )
{
    UINT32 uiStatus = SGTL5000_SUCCESS;
    UINT16 usValue;

    // After the register value is read below, another thread could potentially
    // overwrite the value before this function writes the new value.  To prevent
    // this, acquire a critical section lock
    EnterCriticalSection(&m_csI2C);
    
    uiStatus = SGTL5000ReadRegister( usRegNum, &usValue );

    // mask out old setting
    usValue &= ~usMask;

    // set new bit field values, but only bit fields that were cleared first
    usValue |= (usMask) & usBitFieldValue;

    // write out new value
    uiStatus = SGTL5000WriteRegister( usRegNum, usValue );

    // Leave critical section lock
    LeaveCriticalSection(&m_csI2C);

    return uiStatus;
}

//=============================================================================
// SGTL5000 Playback/Record Functions
//=============================================================================

//-----------------------------------------------------------------------------
//
// Function: SGTL5000CodecInit
//
// This function initializes the SGTL5000 audio codec chip.  It sets up:
// 1.  Configure the analog ground reference voltage values based on supply 
// voltage
// 2.  Configure the PLL dividers
// 3.  Other analog block configuration such as zero cross detect, short detect
//
// Parameters:
//      None.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
UINT32 SGTL5000Codec::SGTL5000CodecInit(VOID)
{
    UINT32 uiStatus = SGTL5000_SUCCESS;
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;
    UINT16 Int_Divisor = 0;
    UINT16 Frac_Divisor = 0;
    DWORD  VagVal = 0;
    DWORD  LoVagCntrl = 0;
    DWORD  PllOutputFreq = 0;

    //-------------------Power Configuration-----------------------                                                                            
    // NOTE!!! Some settings below are dependent on the voltage level
    // used for VDDIO/VDDA supplies.  All settings that are VDDIO/VDDA value dependent 
    
    // Set VAG (Analog ground level) level to to VDDA/2, and 
    // reference bias current for all the analog blocks set to -50% 
    // from the nominal value
    VagVal = POWER_LEVEL_VDDA/2;
    if(VagVal > VAG_VAL_MAX_MV)
    {
        VagVal = VAG_VAL_MAX_MV;
    }
    else if(VagVal < VAG_VAL_MIN_MV)
    {
        VagVal = VAG_VAL_MIN_MV;
    }

    RegValue = CSP_BITFVAL(SMALL_POP, SMALL_POP_NORMAL_RAMP) |
                   (UINT16)CSP_BITFVAL(VAG_VAL, VAG_VAL(VagVal)) |
                   CSP_BITFVAL(BIAS_CTRL, BIAS_CTRL_MINUS_50);
    SGTL5000WriteRegister(SGTL5000_CHIP_REF_CTRL_ADDR, RegValue);

    // Set LINEOUT VAG voltage to VDDIO/2 and bias current
    // the recommended value of 0.36mA for 10kOhm load with 1nF 
    // capacitance
    LoVagCntrl = POWER_LEVEL_VDDIO/2;
    if(LoVagCntrl > LO_VAGCNTRL_MAX_MV)
    {
        LoVagCntrl = LO_VAGCNTRL_MAX_MV;
    }
    else if(LoVagCntrl < LO_VAGCNTRL_MIN_MV)
    {
        LoVagCntrl = LO_VAGCNTRL_MIN_MV;
    }
               
    RegValue = CSP_BITFVAL(OUT_CURRENT, OUT_CURRENT_36) |
                   (UINT16)CSP_BITFVAL(LO_VAGCNTRL, LO_VAGCNTRL(LoVagCntrl));
                       
    uiStatus = SGTL5000WriteRegister(SGTL5000_CHIP_LINE_OUT_CTRL_ADDR, RegValue);

    //------------------Startup volume configuration-----------------  
    // Set LINEOUT volume based on VAG and LINEOUT_VAG values
    // The LO_VOL_LEFT and LO_VOL_RIGHT be 40*log(VAG_VAL/LO_VAGCNTRL) + 15
    // VDDA  VDDIO  LO_VOL_*
    // 1.8V  1.8V   0x0F
    // 1.8V  3.3V   0x06
    // 3.3V  1.8V   0x19
    // 3.3V  3.3V   0x0F    
    uiStatus = SGTL5000WriteRegister(SGTL5000_CHIP_LINE_OUT_VOL_ADDR, 0x0F0F);

    //----------------Other Analog Block Configuration---------------  

    // Enable short detect mode for headphone left/right
    // and center channel and set short detect current trip level
    // to 75mA
    RegValue = CSP_BITFVAL(MODE_LR, MODE_LR_ENABLE_DETECTOR_RESET_TIMEOUT) |
                   CSP_BITFVAL(LVLADJR, LVLADJR_75MA) |
                   CSP_BITFVAL(LVLADJL, LVLADJL_75MA);
    SGTL5000WriteRegister(SGTL5000_CHIP_SHORT_CTRL_ADDR, RegValue);

    // Enable Zero-cross detect for HP_OUT (bit 5) and ADC (bit 1)
    RegValue = CSP_BITFVAL(EN_ZCD_ADC, EN_ZCD_ADC_ENABLED) |
                   CSP_BITFVAL(MUTE_ADC, MUTE_ADC_MUTE) |
                   CSP_BITFVAL(MUTE_HP, MUTE_HP_MUTE) |
                   CSP_BITFVAL(EN_ZCD_HP, EN_ZCD_HP_ENABLED) |
                   CSP_BITFVAL(MUTE_LO, MUTE_LO_MUTE);
    SGTL5000WriteRegister(SGTL5000_CHIP_ANA_CTRL_ADDR, RegValue);

    PllOutputFreq = PLL_OUTPUT_FREQ_180633600HZ;
    Int_Divisor = (UINT16)(PllOutputFreq/SYS_MCLK);
    Frac_Divisor = (UINT16)((((float)PllOutputFreq/(float)SYS_MCLK)-Int_Divisor)*2048);

    RegValue = CSP_BITFVAL(INT_DIVISOR, Int_Divisor) |
                    CSP_BITFVAL(FRAC_DIVISOR, Frac_Divisor);
    SGTL5000WriteRegister(SGTL5000_CHIP_PLL_CTRL_ADDR, RegValue);
    
    SGTL5000SetADCInput((SGTL5000_ADC_INPUT)m_inputPort);
    
    // Set default volume for ADC
    RegValue = CSP_BITFVAL(ADC_VOL_LEFT, SGTL5000_ADC_VOL_DEFAULT_VAL) |
                CSP_BITFVAL(ADC_VOL_RIGHT, SGTL5000_ADC_VOL_DEFAULT_VAL) ;
    RegMask = CSP_BITFMASK(ADC_VOL_LEFT) | 
              CSP_BITFMASK(ADC_VOL_RIGHT);  
              
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_ADC_CTRL_ADDR,
                   RegMask,
                   RegValue);

    // Workaround for 37821276
    // Set I2S_SCLK pad drive strength to the maximum
    RegValue = CSP_BITFVAL(I2S_SCLK, 0x3);
    RegMask = CSP_BITFMASK(I2S_SCLK);
    SGTL5000ModifyRegister(SGTL5000_CHIP_PAD_STRENGTH_ADDR,
                   RegMask,
                   RegValue);

    return uiStatus;
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000ConfigureDuplexMode
//
// This function performs following operations to configure the chip for
// playback/record (full duplex) mode.
// 1.  Sets up I2S0 -> DAC -> HP, LINEOUT route for playback
//     Sets up LINEIN -> ADC -> I2S0 route for record
// 2.  Configures the MIC bias and gain
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000ConfigureDuplexMode(VOID)
{
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    m_workMode = WORK_MODE_DUPLEX;

    // Configure I2S0 -> DAC -> HP_OUT, LINEOUT
    // Configure ADC -> I2S0
    // RegValue = 0x0010
    RegValue = CSP_BITFVAL(DAC_SELECT, DAC_SELECT_I2S_IN);
    SGTL5000WriteRegister(SGTL5000_CHIP_SSS_CTRL_ADDR, RegValue);

    
    RegValue = CSP_BITFVAL(SELECT_HP, SELECT_HP_DAC);
    RegMask = CSP_BITFMASK(SELECT_HP);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_CTRL_ADDR, RegMask, RegValue);

    // Configure MIC settings - Enable MIC with 2k bias resistor, 
    // set bias volt to 1.5V, MIC gain to +30dB
    // RegValue = 0x0102
    RegValue = CSP_BITFVAL(MIC_GAIN, MIC_GAIN_30DB) |
                  CSP_BITFVAL(MIC_BIAS_RESISTOR, MIC_BIAS_RESISTOR_2K) | 
                  CSP_BITFVAL(MIC_BIAS_VOLTAGE, 1);
    SGTL5000WriteRegister(SGTL5000_CHIP_MIC_CTRL_ADDR, RegValue);
 }

//-----------------------------------------------------------------------------
//
// Function: SGTL5000ConfigureLoopbackMode
//
// This function performs following operations to configure the chip for
// loop back mode.
// 1.  Mutes DACs and HP before configuration
// 2.  Sets up ADC-> DAC -> HP, LINEOUT route for playback
// 3.  Unmute HP and LINEOUT
// 4.  Ramp the HP volume to the desired volume
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000ConfigureLoopbackMode(VOID)
{
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    m_workMode = WORK_MODE_LOOPBACK;
    
    // Mute DAC and ADC before configuration
    SGTL5000DACMute(MUTE);
    SGTL5000ADCMute(MUTE);

    // Configure LINEIN->ADC -> DAC -> HP_OUT, LINEOUT
    // RegValue = 0x0010
    RegValue = CSP_BITFVAL(DAC_SELECT, DAC_SELECT_ADC);
    SGTL5000WriteRegister(SGTL5000_CHIP_SSS_CTRL_ADDR, RegValue);

    
    RegValue = CSP_BITFVAL(SELECT_HP, SELECT_HP_DAC);
    RegMask = CSP_BITFMASK(SELECT_HP);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_CTRL_ADDR, RegMask, RegValue);

    // Configure MIC settings - Enable MIC with 2k bias resistor, 
    // set bias volt to 1.5V, MIC gain to +30dB
    // RegValue = 0x0102
    RegValue = CSP_BITFVAL(MIC_GAIN, MIC_GAIN_30DB) |
                  CSP_BITFVAL(MIC_BIAS_RESISTOR, MIC_BIAS_RESISTOR_2K) | 
                  CSP_BITFVAL(MIC_BIAS_VOLTAGE, 1);
    SGTL5000WriteRegister(SGTL5000_CHIP_MIC_CTRL_ADDR, RegValue);

    //----------------Unmute HPOUT, LINEOUT ----------------------
    // Unmute HP
    SGTL5000HPMute(UNMUTE);

    // Unmute LINEOUT
    SGTL5000LineOutMute(UNMUTE);
   
    // Ramp up HP volume to default of 0dB
    SGTL5000RampHPVolume(HP_DEFAULT_VOL);

    
 }


//-----------------------------------------------------------------------------
//
// Function: SGTL5000ConfigurePowerMode
//
// This function configures the various power modes of the chip
//
// Parameters:
//      usPowerMode     [in]Power mode to be configured on the chip
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000ConfigurePowerMode(UINT16 usPowerMode)
{
    UINT16 RegValue ,RegValue1 = 0;
    UINT16 RegMask = 0;
    switch (usPowerMode)
    {
        case POWER_MODE_CODEC_OFF:
            // Mute DAC and ADC
            SGTL5000DACMute(MUTE);
            SGTL5000ADCMute(MUTE);
            // Ramp down the HP volume to the lowest level -51.5dB
            SGTL5000RampHPVolume(HP_LOWEST_VOL);
            // Mute HP
            SGTL5000HPMute(MUTE);
            // Mute LINEOUT
            SGTL5000LineOutMute(MUTE);


            // Power down the VAG first and wait for 300ms before
            // turning off HP and LINEOUT
            RegValue = CSP_BITFVAL(VAG_POWERUP, DISABLE);
            RegMask = CSP_BITFMASK(VAG_POWERUP);
            SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR, 
                                   RegMask,
                                   RegValue);
            Sleep(300);

            
            //  VDDD is internally driven by the chip
            #if (VDDD_INTERNAL_DRIVEN)
            {
                // Enable the simple linreg supply   
                RegValue1 |= CSP_BITFVAL(LINREG_SIMPLE_POWERUP, ENABLE);
            }
            // VDDD is externally driven
            #else
            {
                // Disable the simple linreg supply
                RegValue1 |= CSP_BITFVAL(LINREG_SIMPLE_POWERUP, DISABLE);
            }
            #endif
 
            // As the workaround of pop noise in suspend operation,
            // we do not shut down any analog power here.
/*
            // Before turning off the PLL supply, switch the clock from using 
            // PLL to MCLK.
            RegValue = CSP_BITFVAL(MCLK_FREQ, MCLK_FREQ_256_FS);
            RegMask = CSP_BITFMASK(MCLK_FREQ);                                                
            SGTL5000ModifyRegister(SGTL5000_CHIP_CLK_CTRL_ADDR, RegMask, RegValue);
 
            // Power off the PLL and VCOAMP (needed for PLL) powered up
            RegValue1 |=  CSP_BITFVAL(VCOAMP_POWERUP, DISABLE) | 
                        CSP_BITFVAL(PLL_POWERUP, DISABLE);
            SGTL5000WriteRegister(SGTL5000_CHIP_ANA_POWER_ADDR, RegValue1);
*/
            // Power down all the digital blocks
            SGTL5000WriteRegister(SGTL5000_CHIP_DIG_POWER_ADDR, 0x0000);

            break;

        case POWER_MODE_CODEC_ON:

           

            // Mute DAC/ADC
            SGTL5000DACMute(MUTE);
            SGTL5000ADCMute(MUTE);
            // Ramp down the HP volume to the lowest level -51.5dB
            SGTL5000RampHPVolume(HP_LOWEST_VOL);
            // Mute HP
            SGTL5000HPMute(MUTE);
            // Mute LINEOUT
            SGTL5000LineOutMute(MUTE);

            //  If VDDD is internally driven by the chip
            #if (VDDD_INTERNAL_DRIVEN)
            {
                // Configure internal VDDD level to 1.2V (bits 3:0)    
                SGTL5000WriteRegister(SGTL5000_CHIP_LINREG_CTRL_ADDR, 0x0008);
                // Enable the primary VDDD linear regulator
                RegValue1 |= CSP_BITFVAL(LINREG_D_POWERUP, ENABLE);
                // Disable simple (low power regulator).  This is only needed
                // when the primary regulator is disabled.
                RegValue1 |= CSP_BITFVAL(LINREG_SIMPLE_POWERUP, DISABLE);
            }
            // VDDD is externally driven to the chip
            #else
            {
                // Disable the Simple LinReg supply
                RegValue1 |= CSP_BITFVAL(LINREG_SIMPLE_POWERUP, DISABLE);
                // Disable the primary VDDD linear regulator
                RegValue1 |= CSP_BITFVAL(LINREG_D_POWERUP, DISABLE);
            }
            #endif

            // If both VDDA and VDDIO are less than 3.1V, enable the 
            // charge pump supply
            #if((POWER_LEVEL_VDDA < 3100) && (POWER_LEVEL_VDDIO < 3100))
            {              
                // Turn on the charge pump supply (set bit 11)
                RegValue1 |= CSP_BITFVAL(VDDC_CHRGPMP_POWERUP, ENABLE);                                
            }
            #endif            

            RegValue1 |= CSP_BITFVAL(DAC_POWERUP, ENABLE) |                
                CSP_BITFVAL(REFTOP_POWERUP, ENABLE) |  
                CSP_BITFVAL(VCOAMP_POWERUP, ENABLE) | 
                CSP_BITFVAL(PLL_POWERUP, ENABLE) | 
                CSP_BITFVAL(RIGHT_DAC_POWERUP, ENABLE)|
                CSP_BITFVAL(LINEOUT_POWERUP, ENABLE) |
                CSP_BITFVAL(HEADPHONE_POWERUP, ENABLE);
    
            SGTL5000WriteRegister(SGTL5000_CHIP_ANA_POWER_ADDR, RegValue1);

            Sleep(300);
    
            // Power up VAG supply
            RegValue = CSP_BITFVAL(VAG_POWERUP, ENABLE);
            RegMask = CSP_BITFMASK(VAG_POWERUP);
            SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR, 
                           RegMask,
                           RegValue);

            // Power on the digital blocks
            RegValue = CSP_BITFVAL(I2S_IN_POWERUP, ENABLE) |
                        CSP_BITFVAL(DAP_DIG_POWERUP, ENABLE) | 
                        CSP_BITFVAL(DAC_DIG_POWERUP, ENABLE); 
            SGTL5000WriteRegister(SGTL5000_CHIP_DIG_POWER_ADDR, RegValue);
            // Set sample rate to 44.1kHz, and enable to use PLL
            RegValue = CSP_BITFVAL(SYS_FS, SYS_FS_44_1_KHZ) |
                        CSP_BITFVAL(MCLK_FREQ, MCLK_FREQ_PLL);
            RegMask = CSP_BITFMASK(SYS_FS) |
                        CSP_BITFMASK(MCLK_FREQ);                                                
            SGTL5000ModifyRegister(SGTL5000_CHIP_CLK_CTRL_ADDR, RegMask, RegValue);

            
            //------------------I2S0 Configuration----------------------
            switch(BITSPERSAMPLE)
            {
                case 16:
                    // Set as I2S Master, 16-bit data length
                    RegValue = CSP_BITFVAL(MS, MS_MASTER) | 
                                    CSP_BITFVAL(DLEN, DLEN_16_BITS) |
                                    CSP_BITFVAL(SCLKFREQ, SCLKFREQ_64_FS); 
                    break;
                case 24:
                    // Set as I2S Master, 24-bit data length
                    RegValue = CSP_BITFVAL(MS, MS_MASTER) | 
                                    CSP_BITFVAL(DLEN, DLEN_24_BITS) |
                                    CSP_BITFVAL(SCLKFREQ, SCLKFREQ_64_FS);
                    break;
                default:
                    RETAILMSG(TRUE,(TEXT("Error,not supported sample rate\r\n")));
                    return;
                    break;
            }
            SGTL5000WriteRegister(SGTL5000_CHIP_I2S_CTRL_ADDR, RegValue);


            
            // Configure the playback route
            if(m_workMode == WORK_MODE_LOOPBACK){
                SGTL5000ConfigureLoopbackMode();
                // Power up the ADC
                RegValue = CSP_BITFVAL(ADC_POWERUP, ENABLE);
                RegMask = CSP_BITFMASK(ADC_POWERUP);            
                //If input is linein, we enable the right adc
                if(m_inputPort == ADC_INPUT_LINEIN){
                    RegValue |= CSP_BITFVAL(RIGHT_ADC_POWERUP, ENABLE);    
                    RegMask |= CSP_BITFMASK(RIGHT_ADC_POWERUP); 
                }
                SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR,
                           RegMask,
                           RegValue);                                                                          
      
                // Power up digital blocks for record
                RegValue = CSP_BITFVAL(I2S_OUT_POWERUP, ENABLE) |
                        CSP_BITFVAL(ADC_DIG_POWERUP, ENABLE) ;
                RegMask = CSP_BITFMASK(I2S_OUT_POWERUP) | 
                      CSP_BITFMASK(ADC_DIG_POWERUP);                        
                SGTL5000ModifyRegister(SGTL5000_CHIP_DIG_POWER_ADDR,
                           RegMask,
                           RegValue); 
            
                // Unmute ADC
                SGTL5000ADCMute(UNMUTE);
                // Unmute DAC
                SGTL5000DACMute(UNMUTE);
            }else{                
                SGTL5000ConfigureDuplexMode();
            }    

            // Unmute HP
            SGTL5000HPMute(UNMUTE);

            // Unmute LINEOUT
            SGTL5000LineOutMute(UNMUTE);
           
            // Ramp up HP volume to default of 0dB
            SGTL5000RampHPVolume(HP_DEFAULT_VOL);
            break;
        case POWER_MODE_PLAYBACK_ON:      

            // Unmute DAC
            SGTL5000DACMute(UNMUTE);

            break;

        case POWER_MODE_PLAYBACK_STOP:

            // We cannot power down HPOUT or LINEOUT without powering
            // down the VAG supply.  If we turn down VAG supply, we have to
            // wait for 300ms before we turn of HPOUT or LINEOUT amps.  Due
            // to this extra overhead, we will leave DAC, HPOUT and LINEOUT
            // powered on and just mute the DAC.

            // Mute DAC
            SGTL5000DACMute(MUTE);

            break;
        
        case POWER_MODE_RECORD_ON:
        
            // Power up the ADC
            RegValue = CSP_BITFVAL(ADC_POWERUP, ENABLE);
            RegMask = CSP_BITFMASK(ADC_POWERUP);            
            //If input is linein, we enable the right adc
            if(m_inputPort == ADC_INPUT_LINEIN){
                RegValue |= CSP_BITFVAL(RIGHT_ADC_POWERUP, ENABLE);    
                RegMask |= CSP_BITFMASK(RIGHT_ADC_POWERUP); 
            }
            SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR,
                           RegMask,
                           RegValue);                                                                          
      
            // Power up digital blocks for record
            RegValue = CSP_BITFVAL(I2S_OUT_POWERUP, ENABLE) |
                        CSP_BITFVAL(ADC_DIG_POWERUP, ENABLE) ;
            RegMask = CSP_BITFMASK(I2S_OUT_POWERUP) | 
                      CSP_BITFMASK(ADC_DIG_POWERUP);                        
            SGTL5000ModifyRegister(SGTL5000_CHIP_DIG_POWER_ADDR,
                           RegMask,
                           RegValue); 
            
            // Unmute ADC
            SGTL5000ADCMute(UNMUTE);
            
            break;

        case POWER_MODE_RECORD_STOP:

            // Mute ADC
            SGTL5000ADCMute(MUTE);

            // Power down ADC.
            RegValue = CSP_BITFVAL(ADC_POWERUP, DISABLE);
            RegMask = CSP_BITFMASK(ADC_POWERUP);
            //If input is linein, we enable the right adc
            if(m_inputPort == ADC_INPUT_LINEIN){
                RegValue |= CSP_BITFVAL(RIGHT_ADC_POWERUP, ENABLE);    
                RegMask |= CSP_BITFMASK(RIGHT_ADC_POWERUP); 
            }
            SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR,
                           RegMask,
                           RegValue); 
           
            // Power down digital blocks related to recording
            RegValue = CSP_BITFVAL(I2S_OUT_POWERUP, DISABLE) |
                        CSP_BITFVAL(ADC_DIG_POWERUP, DISABLE) ;
            RegMask = CSP_BITFMASK(I2S_OUT_POWERUP) | 
                      CSP_BITFMASK(ADC_DIG_POWERUP);                        

            SGTL5000ModifyRegister(SGTL5000_CHIP_DIG_POWER_ADDR,
                           RegMask,
                           RegValue); 

            break;

    }
}

//=============================================================================
// SGTL5000 Chip Helper Functions
//=============================================================================
//-----------------------------------------------------------------------------
//
// Function: SGTL5000RampHPVolume
//
// This function configures the analog volume of the headphone.  The
// headphone volume must be ramped to avoid any pops.
//
// Parameters:
//      usNewVol
//          [in] The new volume in hex
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000RampHPVolume(UINT16 usNewVol)
{
    UINT16 usCurrentVol;
    SGTL5000ReadRegister(SGTL5000_CHIP_ANA_HP_CTRL_ADDR, &usCurrentVol);
    
    // Assuming that both left and right volume set to the same level
    // SGTL5000 can support independent left and right volume.  The code needs
    // to be modified to support it.
    UINT16 usNewVolLeft = usNewVol & 0xFF;
    UINT16 usCurrentVolLeft = usCurrentVol & 0xFF;
    //Only checking the left volume
    UINT16 usNumSteps = usNewVolLeft - usCurrentVolLeft; 
    if (usNumSteps == 0) return;
    
    if (usCurrentVolLeft > usNewVolLeft)
    {
        usNumSteps = -(usNumSteps);
    }
    
    for (UINT16 i = 0; i < usNumSteps; i++)
    {
        if (usNewVolLeft > usCurrentVolLeft)
        {
            ++usCurrentVolLeft;
        }
        else
        {
            --usCurrentVolLeft;
        }
        
        // Set both left and right volume to same level
        usCurrentVol = (usCurrentVolLeft << 8) | (usCurrentVolLeft);
        SGTL5000WriteRegister(SGTL5000_CHIP_ANA_HP_CTRL_ADDR, usCurrentVol);
        m_usHPVol = usCurrentVol;
    }
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SetDACPower
//
// This function powers on/off the DAC left/right channels.  Note: There is
// only one DAC on SGTL5000.  Both Headphone and LINEOUT use the same DAC.
//
// Parameters:
//      usPower
//          [in] ENABLE:  Power on, DISABLE: Power Off
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000SetDACPower(UINT16 usPower)
{
    UINT16 RegValue = 0;
    UINT16 RegMask = 0; 
        
    // Power on/off the DAC left and right channels
    RegValue = CSP_BITFVAL(DAC_POWERUP, usPower);
    RegMask = CSP_BITFMASK(DAC_POWERUP);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR,
                           RegMask,
                           RegValue); 
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SetADCPower
//
// This function powers on/off the ADC left/right channels.  Note: There is
// only one ADC on SGTL5000.  Both MICIN and LINEIN use the same ADC.
//
// Parameters:
//      usPower
//          [in] ENABLE:  Power on, DISABLE: Power Off
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000SetADCPower(UINT16 usPower)
{
    UINT16 RegValue = 0;
    UINT16 RegMask = 0; 
        
    // Power on/off the ADC left and right channels
    RegValue = CSP_BITFVAL(ADC_POWERUP, usPower) |
                CSP_BITFVAL(RIGHT_ADC_POWERUP, usPower);
    RegMask = CSP_BITFMASK(ADC_POWERUP) |
                CSP_BITFMASK(RIGHT_ADC_POWERUP);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR,
                           RegMask,
                           RegValue); 
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SetLINEOUTPower
//
// This function powers on/off the LINEOUT amplifier.
//
// Parameters:
//      usPower
//          [in] ENABLE:  Power on, DISABLE: Power Off
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000SetLINEOUTPower(UINT16 usPower)
{       
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    // Power on/off the LINEOUT
    RegValue = CSP_BITFVAL(LINEOUT_POWERUP, usPower);
    RegMask = CSP_BITFMASK(LINEOUT_POWERUP);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR,
                           RegMask,
                           RegValue);
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SetHPOUTPower
//
// This function powers on/off the HP amplifier and enables/disables capless mode.
// NOTE:  The HP volume must be ramped down before turning off HP power.  
// Conversely, the HP volume must be ramped up after the HP power is turned on.
//
// Parameters:
//      usPower
//          [in] ENABLE:  Power on, DISABLE: Power Off
//      usCaplessMode
//          [in] ENABLE:  Capless mode on, DISABLE: Capless mode off
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000SetHPOUTPower(UINT16 usPower, UINT16 usCaplessMode)
{       
    UINT16 RegValue = 0;
    UINT16 RegMask = 0; 
        
    // Power on/off HP and enable/disable capless mode
    RegValue = CSP_BITFVAL(CAPLESS_HP_POWERUP, usCaplessMode) |
                    CSP_BITFVAL(HEADPHONE_POWERUP, usPower);
    RegMask = CSP_BITFMASK(CAPLESS_HP_POWERUP) |
                    CSP_BITFMASK(HEADPHONE_POWERUP);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_POWER_ADDR,
                           RegMask,
                           RegValue); 
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SetSampleRate
//
// Configures the sample rate
//
// Parameters:
//      eSampleRate
//          [in] Valid values are KHZ_32, KHZ_44_1, KHZ_48, KHZ_96
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000SetSampleRate(SGTL5000_SAMPLE_RATE eSampleRate)
{       
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    RegValue = (UINT16)CSP_BITFVAL(SYS_FS, eSampleRate);
    RegMask = CSP_BITFMASK(SYS_FS);
     SGTL5000ModifyRegister(SGTL5000_CHIP_CLK_CTRL_ADDR,
                            RegMask,
                            RegValue);
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SetDACVolume
//
// This function configures the DAC digital volume.  This volume
// affects both LINEOUT and HP.  The DAC automatically ramps up/down
// the volume
//
// Parameters:
//      usLeftVol
//          [in] The left channel volume in hex
//      usRightVol
//          [in] The right channel volume in hex
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000SetDACVolume(BYTE usLeftVol, BYTE usRightVol)
{
    UINT16 RegValue = 0;

    // The volume will be automatically ramped
    RegValue = CSP_BITFVAL(DAC_VOL_RIGHT, usRightVol) |
                    CSP_BITFVAL(DAC_VOL_LEFT, usLeftVol);
    SGTL5000WriteRegister(SGTL5000_CHIP_DAC_VOL_ADDR, RegValue);
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SetADCVolume
//
// This function configures the ADC analog volume.
//
// Parameters:
//      usLeftVol
//          [in] The left channel volume in hex
//      usRightVol
//          [in] The right channel volume in hex
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000SetADCVolume(BYTE usLeftVol, BYTE usRightVol)
{
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    RegValue = CSP_BITFVAL(ADC_VOL_RIGHT, usRightVol) |
                CSP_BITFVAL(ADC_VOL_LEFT, usLeftVol);
    RegMask = CSP_BITFMASK(ADC_VOL_RIGHT) |
                CSP_BITFMASK(ADC_VOL_LEFT);
        
    // Write the ADC left/right volume
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_ADC_CTRL_ADDR,
                           RegMask,
                           RegValue);
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000DACMute
//
// This function mute/unmutes the DAC.
//
// Parameters:
//      usMuteValue
//          [in] Mute value:  MUTE(1) or UNMUTE(0)
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000DACMute(UINT16 usMuteValue)
{
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    RegValue = CSP_BITFVAL(DAC_MUTE_RIGHT, usMuteValue) |
                CSP_BITFVAL(DAC_MUTE_LEFT, usMuteValue) |
                // Enable soft mute
                CSP_BITFVAL(VOL_RAMP_EN, VOL_RAMP_EN_ENABLE)| 
                // Enable fast(exponential) soft mute
                CSP_BITFVAL(VOL_RAMP_EN, VOL_EXPO_RAMP_EXPONENTIAL);  
    
    RegMask = CSP_BITFMASK(DAC_MUTE_RIGHT) |
                CSP_BITFMASK(DAC_MUTE_LEFT) |
                CSP_BITFMASK(VOL_RAMP_EN);

    SGTL5000WriteRegister(SGTL5000_CHIP_ADCDAC_CTRL_ADDR, RegValue);

}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000HPMute
//
// This function mute/unmutes the HP.
//
// Parameters:
//      usMuteValue
//          [in] Mute value:  MUTE(1) or UNMUTE(0)
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000HPMute(UINT16 usMuteValue)
{   
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    // Modify only the bits affecting mute/unmute
    RegValue = CSP_BITFVAL(MUTE_HP, usMuteValue);
    RegMask = CSP_BITFMASK(MUTE_HP);

    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_CTRL_ADDR,
                           RegMask,
                           RegValue ); 
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000LineOutMute
//
// This function mute/unmutes the LINEOUT.
//
// Parameters:
//      usMuteValue
//          [in] Mute value:  MUTE(1) or UNMUTE(0)
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000LineOutMute(UINT16 usMuteValue)
{   
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    // Modify only the bits affecting mute/unmute   
    RegValue = CSP_BITFVAL(MUTE_LO, usMuteValue);
    RegMask = CSP_BITFMASK(MUTE_LO);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_CTRL_ADDR,
                           RegMask,
                           RegValue ); 
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000ADCMute
//
// This function mute/unmutes the ADC.
//
// Parameters:
//      usMuteValue
//          [in] Mute value:  MUTE(1) or UNMUTE(0)
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000ADCMute(UINT16 usMuteValue)
{
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;
    
    // Modify only the bits affecting mute/unmute
    RegValue = CSP_BITFVAL(MUTE_ADC, usMuteValue);
    RegMask = CSP_BITFMASK(MUTE_ADC);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_CTRL_ADDR,
                           RegMask,
                           RegValue );    
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SetADCInput
//
// This function selects MICIN or LINEIN as ADC input
//
// Parameters:
//      eADCInput
//          [in] MICIN or LINEIN
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SGTL5000Codec::SGTL5000SetADCInput(SGTL5000_ADC_INPUT eADCInput)
{   
    UINT16 RegValue = 0;
    UINT16 RegMask = 0;

    RegValue = (UINT16)CSP_BITFVAL(SELECT_ADC, eADCInput);
    RegMask = CSP_BITFMASK(SELECT_ADC);
    SGTL5000ModifyRegister(SGTL5000_CHIP_ANA_CTRL_ADDR,
                           RegMask,
                           RegValue);    
                           
    m_inputPort = eADCInput;  
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000HPDetected
//
// This function is called by bsp specific hardware context to indicate if
// Headphones are plugged in or not
//
// Parameters:
//      HPDetected
//          [in]    1 = HP is detected (plugged in)
//                  0 = HP is not detected (not plugged)
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SGTL5000Codec::SGTL5000HPDetected(UINT32 HPDetected)
{
    m_HPDetected = HPDetected;
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000ForceSpeaker
//
// The wavedev2 driver sets the force speaker flag for some audio streams that
// must only be played from the speakers (loud tones like ring tones, alarms etc)
//
// Parameters:
//      ForceSpeaker
//          [in]    1 = Playback from speaker is forced
//                  0 = Playback from speaker is not forced
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SGTL5000Codec::SGTL5000ForceSpeaker(UINT32 ForceSpeaker)
{
    m_ForceSpeaker = ForceSpeaker;
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000SpeakerHeadphoneSelection
//
// Audio playback will be done via speakers or headphones based on 
// MM_WOM_FORCESPEAKER flag and whether the Headphones are plugged in
// HP or LINEOUT will be muted/unmuted accordingly
//
// Parameters:
//      None
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SGTL5000Codec::SGTL5000SpeakerHeadphoneSelection()
{
    // If force speaker flag is set or if no headphone is detected, unmute
    // the LINEOUT and mute the HP
    if (m_ForceSpeaker || !m_HPDetected)
    {       
        // Mute HP
        SGTL5000HPMute(MUTE);
        // Turn on the Maxim speaker amplifier

        // Unmute LINEOUT
        SGTL5000LineOutMute(UNMUTE);
    }
    // If force speaker flag is not set and if headphone is plugged, unmute
    // the HP and mute the LINEOUT
    else 
    {
        // Unmute HP
        SGTL5000HPMute(UNMUTE);
        // Mute LINEOUT
        SGTL5000LineOutMute(MUTE);
        // Turn off the Maxim speaker amplifier
        
    }
}

//-----------------------------------------------------------------------------
//
// Function: SGTL5000DumpRegisters
//
// Dumps all the register value for debugging purpose
//
// Parameters:
//      None
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SGTL5000Codec::SGTL5000DumpRegisters()
{   
    UINT16 Value = 0;

    SGTL5000ReadRegister(SGTL5000_CHIP_ID_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("CHIP_ID = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_DIG_POWER_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("CHIP_DIG_POWER = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_CLK_CTRL_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("CHIP_CLK_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_I2S_CTRL_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("CHIP_I2S_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_SSS_CTRL_ADDR, &Value);     
    RETAILMSG(TRUE, (TEXT("CHIP_SSS_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_ADCDAC_CTRL_ADDR, &Value);  
    RETAILMSG(TRUE, (TEXT("CHIP_ADCDAC_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_DAC_VOL_ADDR, &Value);      
    RETAILMSG(TRUE, (TEXT("CHIP_DAC_VOL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_PAD_STRENGTH_ADDR, &Value); 
    RETAILMSG(TRUE, (TEXT("CHIP_PAD_STRENGTH_ADDR= 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_ANA_ADC_CTRL_ADDR, &Value); 
    RETAILMSG(TRUE, (TEXT("CHIP_ANA_ADC_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_ANA_HP_CTRL_ADDR, &Value);  
    RETAILMSG(TRUE, (TEXT("CHIP_ANA_HP_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_ANA_CTRL_ADDR, &Value);     
    RETAILMSG(TRUE, (TEXT("CHIP_ANA_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_LINREG_CTRL_ADDR, &Value);  
    RETAILMSG(TRUE, (TEXT("CHIP_LINREG_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_REF_CTRL_ADDR, &Value);     
    RETAILMSG(TRUE, (TEXT("CHIP_REF_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_MIC_CTRL_ADDR, &Value);     
    RETAILMSG(TRUE, (TEXT("CHIP_MIC_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_LINE_OUT_CTRL_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("CHIP_LINE_OUT_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_LINE_OUT_VOL_ADDR, &Value); 
    RETAILMSG(TRUE, (TEXT("CHIP_LINE_OUT_VOL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_ANA_POWER_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("CHIP_ANA_POWER = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_PLL_CTRL_ADDR, &Value); 
    RETAILMSG(TRUE, (TEXT("CHIP_PLL_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_CLK_TOP_CTRL_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("CHIP_CLK_TOP_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_ANA_STATUS_ADDR, &Value);  
    RETAILMSG(TRUE, (TEXT("CHIP_ANA_STATUS = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_CHIP_SHORT_CTRL_ADDR, &Value);  
    RETAILMSG(TRUE, (TEXT("CHIP_SHORT_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_CONTROL_ADDR, &Value);      
    RETAILMSG(TRUE, (TEXT("DAP_CONTROL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_PEQ_ADDR, &Value);          
    RETAILMSG(TRUE, (TEXT("DAP_PEQ = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_BASS_ENHANCE_ADDR, &Value); 
    RETAILMSG(TRUE, (TEXT("DAP_BASS_ENHANCE = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_BASS_ENHANCE_CTRL_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_BASS_ENHANCE_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AUDIO_EQ_ADDR, &Value);      
    RETAILMSG(TRUE, (TEXT("DAP_AUDIO_EQ = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_SGTL_SURROUND_ADDR, &Value); 
    RETAILMSG(TRUE, (TEXT("DAP_SGTL_SURROUND = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_FILTER_COEF_ACCESS_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_FILTER_COEF_ACCESS = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_B0_MSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_B0_MSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_B0_LSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_B0_LSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AUDIO_EQ_BASS_BAND0_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_AUDIO_EQ_BASS_BAND0 = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AUDIO_EQ_BAND1_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_AUDIO_EQ_BAND1 = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AUDIO_EQ_BAND2_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_AUDIO_EQ_BAND2 = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AUDIO_EQ_BAND3_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_AUDIO_EQ_BAND3 = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AUDIO_EQ_TREBLE_BAND4_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_AUDIO_EQ_TREBLE_BAND4 = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_MAIN_CHAN_ADDR, &Value);     
    RETAILMSG(TRUE, (TEXT("DAP_MAIN_CHAN = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_MIX_CHAN_ADDR, &Value);      
    RETAILMSG(TRUE, (TEXT("DAP_MIX_CHAN = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AVC_CTRL_ADDR, &Value);      
    RETAILMSG(TRUE, (TEXT("DAP_AVC_CTRL = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AVC_THRESHOLD_ADDR, &Value); 
    RETAILMSG(TRUE, (TEXT("DAP_AVC_THRESHOLD = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AVC_ATTACK_ADDR, &Value);    
    RETAILMSG(TRUE, (TEXT("DAP_AVC_ATTACK = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_AVC_DECAY_ADDR, &Value);     
    RETAILMSG(TRUE, (TEXT("DAP_AVC_DECAY = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_B1_MSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_B1_MSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_B1_LSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_B1_LSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_B2_MSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_B2_MSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_B2_LSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_B2_LSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_A1_MSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_A1_MSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_A1_LSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_A1_LSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_A2_MSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_A2_MSB = 0x%x \r\n"),Value));
    SGTL5000ReadRegister( SGTL5000_DAP_COEF_WR_A2_LSB_ADDR, &Value);
    RETAILMSG(TRUE, (TEXT("DAP_COEF_WR_A2_LSB = 0x%x \r\n"),Value));
}

