//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  sgtl5000codec.h
//
//  Declares all the control functions used to configure and control
//  SGTL5000 stereo audio codec.
//------------------------------------------------------------------------------

#ifndef __SGTL5000CODEC_H__
#define __SGTL5000CODEC_H__

#include "sgtl5000_regs.h"


#define BITSPERSAMPLE                   16

// Error defines
#define SGTL5000_SUCCESS                0x0
#define SGTL5000_INVALID                0x1  // invalid parameter
#define SGTL5000_I2C_HANDLE             0x2  // invalid I2C handle
#define SGTL5000_I2C_OPERROR            0x3  // I2C operation error

// I2C defines
#define SGTL5000_I2C_PORT               _T("I2C1:")
#define SGTL5000_I2C_ADDR               (0x0A<<1)
#define SGTL5000_I2C_SPEED              100000 // Hz


// Other defines
#define UNMUTE                          0x0
#define MUTE                            0x1
#define HP_LOWEST_VOL                   0x7F7F
#define HP_0DB_DEFAULT_VOL              0x1818
#define HP_DEFAULT_VOL                  0x2828
#define DISABLE                         0x0
#define ENABLE                          0x1
#define POWER_MODE_CODEC_OFF            0x0
#define POWER_MODE_CODEC_ON             0x1
#define POWER_MODE_PLAYBACK_STOP        0x2
#define POWER_MODE_RECORD_STOP          0x3
#define POWER_MODE_PLAYBACK_ON          0x4
#define POWER_MODE_RECORD_ON            0x5
#define SGTL5000_DAC_VOL_0dB_VAL        0x3C
#define SGTL5000_ADC_VOL_DEFAULT_VAL    0xA        // 15db
#define POWER_LEVEL_VDDIO               1800       //1800mv
#define POWER_LEVEL_VDDA                3300       //3300mv
#define VDDD_INTERNAL_DRIVEN            TRUE
#define PLL_OUTPUT_FREQ_180633600HZ     180633600
#define PLL_OUTPUT_FREQ_196608000HZ     196608000
#define SYS_MCLK                        22579200   //Hz, sample rate 44.1KHz

typedef enum
{
    ADC_INPUT_MIC       = 0,
    ADC_INPUT_LINEIN    = 1
} SGTL5000_ADC_INPUT;

typedef enum
{
    KHZ_32              = 0,
    KHZ_44_1            = 1,
    KHZ_48              = 2,
    KHZ_96              = 3
} SGTL5000_SAMPLE_RATE;

typedef enum
{
    WORK_MODE_DUPLEX    = 0,
    WORK_MODE_LOOPBACK
}SGTL5000_WORK_MODE;

class SGTL5000Codec
{
private:
    HANDLE m_hI2C;
    BOOL m_bI2CInited;
    UINT16 m_usHPVol;
    CRITICAL_SECTION m_csI2C;
    UINT32 m_inputPort;    
    UINT32 m_HPDetected;
    UINT32 m_ForceSpeaker;
    SGTL5000_WORK_MODE m_workMode;

public:
    // Constructor and destructor
    SGTL5000Codec (VOID);
    ~SGTL5000Codec (VOID);

    void SGTL5000DumpRegisters();
    void SGTL5000HPDetected(UINT32 HPDetected);
    void SGTL5000ForceSpeaker(UINT32 ForceSpeaker);
    void SGTL5000SpeakerHeadphoneSelection();

    // I2C Functions
    UINT32 SGTL5000I2CInit(VOID);
    VOID SGTL5000I2CClose(VOID);
    UINT32 SGTL5000WriteRegister(UINT16 usRegNum, UINT16 usValue );
    UINT32 SGTL5000ReadRegister(UINT16 usRegNum, UINT16* pusValue );
    UINT32 SGTL5000ModifyRegister(UINT16 usRegNum, 
                                  UINT16 usClearMask,
                                  UINT16 usBitFieldValue );

    //-------------------------------------------------------------
    // Chip Functions
    //-------------------------------------------------------------
    // Init
    UINT32 SGTL5000CodecInit(VOID);
    
    // Playback
    VOID SGTL5000ConfigureDuplexMode(VOID);
    VOID SGTL5000ConfigureLoopbackMode(VOID);

    // Power
    VOID SGTL5000ConfigurePowerMode(UINT16 usPowerMode);
    VOID SGTL5000SetDACPower(UINT16 usPower);    
    VOID SGTL5000SetADCPower(UINT16 usPower);
    VOID SGTL5000SetLINEOUTPower(UINT16 usPower);
    VOID SGTL5000SetHPOUTPower(UINT16 usPower, UINT16 usCaplessMode);

    // Clock
    VOID SGTL5000SetSampleRate(SGTL5000_SAMPLE_RATE eSampleRate);

    // DAC
    VOID SGTL5000DACMute(UINT16 usMuteValue);
    VOID SGTL5000SetDACVolume(BYTE usLeftVol, BYTE usRightVol);

    // ADC
    VOID SGTL5000ADCMute(UINT16 usMuteValue);
    VOID SGTL5000SetADCInput(SGTL5000_ADC_INPUT eADCInput);
    VOID SGTL5000SetADCVolume(BYTE usLeftVol, BYTE usRightVol);

    // Headphone (HP)
    VOID SGTL5000HPMute(UINT16 usMuteValue);
    VOID SGTL5000RampHPVolume(UINT16 usNewVol);

    // Line OUT (LINEOUT)
    VOID SGTL5000LineOutMute(UINT16 usMuteValue);

};

#endif //__SGTL5000CODEC_H__
