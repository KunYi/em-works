//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: wm8580.cpp
//
//  Implementation of Multichannel Audio Codec WM8580.
//
//------------------------------------------------------------------------------
#include "bsp.h"
#include "wm8580.h"
#include "cspibus.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define CSPI_DEVICE_NAME   _T("SPI1:")

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
//  Function: CWM8580
//
//  Construction function of class CWM8580.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
CWM8580::CWM8580()
{
    DWORD error;
    
    m_hCSPI = CSPIOpenHandle(CSPI_DEVICE_NAME);
    
    if (m_hCSPI == INVALID_HANDLE_VALUE)
    {
        error = GetLastError();

        RETAILMSG(TRUE,(TEXT("Create CSPI file handle failed error=%d\r\n"), error));
        return;
    }

    return;

}


//-----------------------------------------------------------------------------
//
//  Function: ~CWM8580
//
//  Deconstruction function of class CWM8580.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
CWM8580::~CWM8580()
{
    // Close CSPI handle
    CloseHandle(m_hCSPI);
    return;
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
UINT8 CWM8580::ReadRegister(DWORD reg)
{
    BYTE txData[2];
    BYTE rxData[2]={0,0};
    UINT8 ret;

 
    CSPI_BUSCONFIG_T busConfig = {
        1,  //chipselect
        8000000,   //freq
        16,     //bitcount  ??
        FALSE,  // sspol
        TRUE,   //ssctl
        FALSE,  //pol  ??
        TRUE,  //pha  ??  FALSE
        0,      //drctl  ??
        FALSE,  //usedma
        TRUE  //usepolling;
    };

    CSPI_XCH_PKT_T xchPkt =
    {
        &busConfig,
        txData,
        rxData,
        1,   // ??
        NULL,
        0
    };

    txData[0] = 0;
    txData[1] =  (BYTE)((reg << 1) & 0xff);

    if (!CSPIExchange(m_hCSPI, &xchPkt)){

        RETAILMSG(TRUE,(TEXT("Read WM8580 REG Failed\r\n")));
        return 0;   
    }
    
    ret = (UINT8)rxData[0];
    
    return ret;

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
BOOL CWM8580::WriteRegister(DWORD reg, DWORD val)
{
    BYTE txData[2];
    BYTE rxData[2]={0,0};

    CSPI_BUSCONFIG_T busConfig = {
        1,  //chipselect
        8000000,   //freq
        16,     //bitcount  ??
        FALSE,  // sspol
        TRUE,   //ssctl
        FALSE,  //pol  ??
        TRUE,  //pha  ??
        0,      //drctl  ??
        FALSE,  //usedma
        TRUE  //usepolling;
    };

    CSPI_XCH_PKT_T xchPkt =
    {
        &busConfig,
        txData,
        rxData,
        1,   // ??
        NULL,
        0
    };

   
    txData[0] = (BYTE)(val & 0xff);
    txData[1] =  (BYTE)(((reg << 1)|(val>>8)) & 0xff);

    if (!CSPIExchange(m_hCSPI, &xchPkt)){

        RETAILMSG(TRUE,(TEXT("Write WM8580 REG Failed")));
        return FALSE;    
    }
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: InitCodec
//
//  This function initial wm8580 codec

//  Parameters:
//      None
//
//  Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
BOOL  CWM8580::InitCodec()
{
    WriteRegister(0x35,0x01);  //R53, Reset Device

    GetDeviceID();

    //Init Power
    WriteRegister(0x32,0x7e);  // R50, PWRDN1, default val, 001111110,Disable ALL DAC/ADC
    WriteRegister(0x33,0x3e);   // R51, PWRDN2,  000111110,En  OSC

    //PLL reg,use default, input is 12M ,pll a is 12.288

    WriteRegister(0x0f,0x24); //r15, dac contrl, 010 0100,din1->dac1, din2->dac2, din3->dac3
    WriteRegister(0x10,0x09);  //r16 ,0000 1001, left to left, right to right
    //WriteRegister(0x11,0x17);  //r17 ,0000 0000, def val, disable de-emphasis
    WriteRegister(0x12,0x3f);  //r18 ,0011 1111, def val, dont' invert phase

    //attenuation
    WriteRegister(0x1c,0xff);//r28  0 1111 1111, def val, 0db to all

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: GetDeviceID
//
//  This function get device ID, for read operation test
//  Parameters:
//      None
//
//  Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
BOOL  CWM8580::GetDeviceID()
{
    WriteRegister(0x34,0x10); // R52, readback, ReadEn = 1 (bit4), ContRead =0
    m_deviceID1 = ReadRegister(0x0); //R1, DEVID1
    m_deviceID2 = ReadRegister(0x01); //R2, DEVID2

    //RETAILMSG(TRUE,(TEXT("WM8580 DeviceID: %x %x\r\n"),m_deviceID1,m_deviceID2));
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: ConfigOutput
//
//  This function config the dac, make it ready for playback
//  Parameters:
//      dwSampleRate: sample rate configured for dac
//      dwBitDepth: always use 24 bit, as esai is a 24 bit module
//      dwChnMask: channel mask, used for network mode
//      audioProtocol:  network mode or left aligned mode
//  Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------

BOOL CWM8580::ConfigOutput(DWORD dwSampleRate, DWORD dwBitDepth, 
    DWORD dwChnMask, DWORD dwChnNum,AUDIO_PROTOCOL audioProtocol)
{
    DWORD dwPLLB1;    
    DWORD dwPLLB2;
    DWORD dwPLLB3;
    DWORD dwPLLB4;
    DWORD dwPAIF1;
    DWORD dwPAIF3;

    //RETAILMSG(TRUE,(TEXT("ConfigOutput\r\n")));
    
    m_dwOutputSampleRate = dwSampleRate;
    m_dwOutputBitDepth = dwBitDepth;
    m_dwOutputChnMask = dwChnMask;
    m_audioProtocol = audioProtocol;

    m_dwDACMask = 0;
    
    if(dwChnNum > 0)
        m_dwDACMask |= WM8580_DAC_1;
    if(dwChnNum > 2 )
        m_dwDACMask |= WM8580_DAC_2;
    if(dwChnNum > 4 )
        m_dwDACMask |= WM8580_DAC_3;
    

    if (audioProtocol == AUDIO_PROTOCOL_NETWORK){
        m_dwDACMask = WM8580_DAC_1 |  WM8580_DAC_2 | WM8580_DAC_3;
        //enable them all for network mode
    }


    //config clk freq
    switch(dwSampleRate){
         case 16000:
            //pll b = 12.288M hz prescale =0, postscale = 0, frqmode=10, pll_n=8,pll_k=0xC49BA, 768FS=101
            dwPLLB4 = 0x154;  //10 10 1 0 1 00   pll b-> clko mclk
            dwPLLB1 = 0x1BA;  // pll_k[8:0],  
            dwPLLB2 = 0x24;    //pll_k [17:9]
            dwPLLB3 = 0x83;  //pll_k[21:18]= 0011   pll_n = 1000,  
            dwPAIF1 = 0x05;
            dwPAIF3 = 0; //bit6=0 128* oversample
            break;

        case 22050:
            //pll b = 11.2896Mhz, pres=0,post=1,freqmode = 01,pll_n = 7, pll_k=0X21B089,512FS=100
            dwPLLB4 = 0x14e;  // 1010 01 1 10 pll b_>clk mclk
            dwPLLB1 = 0x089;  // pll_k[8:0],  
            dwPLLB2 = 0x0d8;    //pll_k [17:9]
            dwPLLB3 = 0x78;  //pll_k[21:18]= 1000   pll_n = 0111,  
            dwPAIF1 = 0x04;
            dwPAIF3 = 0; //bit6=0 128* oversample            
            break;    
            
        case 32000:
            //pll b = 12.288M hz prescale =0, postscale = 0, frqmode=10, pll_n=8,pll_k=0xC49BA, 384FS=011
            dwPLLB4 = 0x154;  //10 10 1 0 1 00   pll b-> clko mclk
            dwPLLB1 = 0x1BA;  // pll_k[8:0],  
            dwPLLB2 = 0x24;    //pll_k [17:9]
            dwPLLB3 = 0x83;  //pll_k[21:18]= 0011   pll_n = 1000,  
            dwPAIF1 = 0x03;
            dwPAIF3 = 0; //bit6=0 128* oversample
            break;
            
        case 44100:
            //pll b = 11.2896Mhz, pres=0,post=1,freqmode = 01,pll_n = 7, pll_k=0X21B089,256FS=010
            dwPLLB4 = 0x14e;  // 1010 01 1 10 pll b_>clk mclk
            dwPLLB1 = 0x089;  // pll_k[8:0],  
            dwPLLB2 = 0x0d8;    //pll_k [17:9]
            dwPLLB3 = 0x78;  //pll_k[21:18]= 1000   pll_n = 0111,  
            dwPAIF1 = 0x02;
            dwPAIF3 = 0; //bit6=0 128* oversample            
            break;
            
        case 48000:
            //pll b = 12.288Mhz, pres =0, post = 0,freqmode =10, 256fs=010
            dwPLLB4 = 0x154;  //10 10 10 1 00  ,mck from pll b
            //dwPLLB4 = 0x1d4;  //11 10 10 1 00  ,mck from osc
            dwPLLB1 = 0x1BA;  // pll_k[8:0],  
            dwPLLB2 = 0x24;    //pll_k [17:9]
            dwPLLB3 = 0x83;  //pll_k[21:18]= 0011   pll_n = 1000,  
            dwPAIF1 = 0x02;
            dwPAIF3 = 0; //bit6=0 128* oversample            
            break;

        case 64000:
            //pll b = 12.288Mhz, pres =0, post = 0,freqmode =10, 192fs=001
            dwPLLB4 = 0x154;  //10 10 10 1 00  ,mck from pll b
            //dwPLLB4 = 0x1d4;  //11 10 10 1 00  ,mck from osc
            dwPLLB1 = 0x1BA;  // pll_k[8:0],  
            dwPLLB2 = 0x24;    //pll_k [17:9]
            dwPLLB3 = 0x83;  //pll_k[21:18]= 0011   pll_n = 1000,  
            dwPAIF1 = 0x01;
            dwPAIF3 = 0x40; //bit6=1 64* oversample  for 192fs  
            break;
            
        case 88200:
            //pll b = 11.2896Mhz, pres = 0, post =0, freqmode=01,128fs=000
            dwPLLB4 = 0x14e;  // 1010 01 100
            dwPLLB1 = 0x089;  // pll_k[8:0],  
            dwPLLB2 = 0x0d8;    //pll_k [17:9]
            dwPLLB3 = 0x78;  //pll_k[21:18]= 1000   pll_n = 0111,  
            dwPAIF1 = 0x00;
            dwPAIF3 = 0x40; //bit6=1 64* oversample
            break;

        case 96000:
            //pll b =12.288 Mhz, pres =0, post =0,freqmode =10,128fs=000
            dwPLLB4 = 0x154;  //10 10 10 1 00
            dwPLLB1 = 0x1BA;  // pll_k[8:0],  
            dwPLLB2 = 0x24;    //pll_k [17:9]
            dwPLLB3 = 0x83;  //pll_k[21:18]= 0011   pll_n = 1000,  
            dwPAIF1 = 0x00;
            dwPAIF3 = 0x40; //bit6=1 64* oversample
            break;

        case 128000:
            //pll b =24.576 Mhz, pres =0, post =0,freqmode =01,192fs=000
            dwPLLB4 = 0x14c;  //10 10 01 1 00
            dwPLLB1 = 0x1BA;  // pll_k[8:0],  
            dwPLLB2 = 0x24;    //pll_k [17:9]
            dwPLLB3 = 0x83;  //pll_k[21:18]= 0011   pll_n = 1000,  
            dwPAIF1 = 0x01;
            dwPAIF3 = 0x40; //bit6=1 64* oversample            
            break;    

        case 176400:
            //pll b =22.58 Mhz, pres =0, post =,freqmode =01,128fs=000
            dwPLLB4 = 0x14c;  //10 10 01 1 00
            dwPLLB1 = 0x089;  // pll_k[8:0],  
            dwPLLB2 = 0x0d8;    //pll_k [17:9]
            dwPLLB3 = 0x78;  //pll_k[21:18]= 0011   pll_n = 1000,  
            dwPAIF1 = 0x00;
            dwPAIF3 = 0x40; //bit6=1 64* oversample            
            break;
            
        case 192000:
            //pll b =24.576 Mhz, pres =0, post =0,freqmode =01,128fs=000
            dwPLLB4 = 0x14c;  //10 10 01 1 00
            dwPLLB1 = 0x1BA;  // pll_k[8:0],  
            dwPLLB2 = 0x24;    //pll_k [17:9]
            dwPLLB3 = 0x83;  //pll_k[21:18]= 0011   pll_n = 1000,  
            dwPAIF1 = 0x00;
            dwPAIF3 = 0x40; //bit6=1 64* oversample
            
            break;
        default:
            return FALSE;
    }

    if(m_bCLKOEnable)
        dwPLLB4 |= 0x180; //CLKOUTSRC: 11 OSCCLK

    WriteRegister(0x04,dwPLLB1); //r4 PLLB1
    WriteRegister(0x05,dwPLLB2); //r5 PLLB2
    WriteRegister(0x06,dwPLLB3); //r6 PLLB3
    WriteRegister(0x07,dwPLLB4); //r7 PLLB4
    
    WriteRegister(0x08,0x1a);   //r8, 001 10 10, dac from pll b, auto config

    //r9 paif1, actually we use only 24 bit
    switch(dwBitDepth){
        case 16:
            dwPAIF3 |= 0x00; //bit3:2, 00 for 16bits
            break;

        case 24:
            dwPAIF3 |= 0x08; // 1000 
            break;

        case 32:
            dwPAIF3 |= 0x0c; //1100
            break;

        default:
            return FALSE;

   }
    
   /*if(dwBitDepth == 16){
        dwPAIF1 |= 0x08; //32 bclk, 0 for64 bclk
      }*/

    if(audioProtocol == AUDIO_PROTOCOL_NETWORK){
        dwPAIF1 |= 0x18; //bit4:3 11, bclk=system clk
        WriteRegister(0x0f,0x100); //r15, dac contrl, 10000 0000,din1->dac1, dac2, dac3
    } 
    
    dwPAIF1 |= 0xa0; //master mode , bit5=1, paifrxms_clksel=10, pll b, bit[7:6], 1010 0000

    WriteRegister(0x09,dwPAIF1);  //r9     


    //r12,  bus protocol    
    switch(audioProtocol){
        case AUDIO_PROTOCOL_I2S:
            dwPAIF3 |= 0x018a;  //    11? 00 ?? 10 r12 paifrx, i2s format,, clk not inverted, 128Xoversample
            break;
            
        case AUDIO_PROTOCOL_NETWORK:
            //dwPAIF3 |= 0x019f;  // 11  ? 01 11 11  dsp, mode b, 32 bits
            dwPAIF3 |= 0x018f; 
            break;
            
        case AUDIO_PROTOCOL_LEFT_ALIGNED:
            dwPAIF3 |= 0x181; // 1 1? 0 00 01  left justified
            break;
            
        default:
            return FALSE;

    }
    
    WriteRegister(0x0c,dwPAIF3);
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: EnableDAC
//
//  This function enable dac power for playback
//  Parameters:
//      None
//
//  Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------

BOOL CWM8580::EnableDAC()
{
    DWORD dwPWRDN1, dwPWRDN2;
   
    m_bDACPowerOn = TRUE;

    dwPWRDN2 = 0x3a;   //11 1 010  , OSC ,PLL B Eanble

    if( m_bADCPowerOn){
        dwPWRDN1 = ((~m_dwDACMask)&0x7) << 2; //

    }else{
        dwPWRDN1 = (((~m_dwDACMask)&0x7) << 2) | 0x2; //adc pwr down  
    }

   
    WriteRegister(0x33,dwPWRDN2);  //R50 PWRDN1
    WriteRegister(0x32,dwPWRDN1);  //R51 PWRDN2

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DisableDAC
//
//  This function disable dac power 
//  Parameters:
//      None
//
//  Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
BOOL CWM8580::DisableDAC()
{
    DWORD dwPWRDN1, dwPWRDN2;
     
    m_bDACPowerOn = TRUE;
    
    dwPWRDN2 = 0x3e;   //11 1110  , OSC  Eanble
    
    if( m_bADCPowerOn){
         dwPWRDN1 = 0x7c;  // 111 1100
    
    }else{
         dwPWRDN1 = 0x7e; //111 1110 adc pwr down  
    }
    
    
    WriteRegister(0x32,dwPWRDN1);  //R51 PWRDN2
    WriteRegister(0x33,dwPWRDN2);  //R50 PWRDN1
    
    return TRUE;

}


//-----------------------------------------------------------------------------
//
//  Function: SetOutputGain
//
//  This function adjust gain for dac
//  Parameters:
//      dwGainDAC1:  gain for dac1, bit[15:0] for dac1_L, bit[31:16] for dac1_R    
//      dwGainDAC2:  gain for dac2
//      dwGainDAC3:  gain for dac3
//      dwMask:   mask for channels to implement gain control
//  Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
BOOL CWM8580::SetOutputGain(DWORD dwGainDAC1, DWORD dwGainDAC2,
    DWORD dwGainDAC3, DWORD dwMask)
{
    UNREFERENCED_PARAMETER(dwMask);
    
    DWORD dwGain1L,dwGain2L,dwGain3L;
    DWORD dwGain1R,dwGain2R,dwGain3R;

    //dac attenuation: 00~ff  -max~ 0 db,step = 0.5db

    dwGain1L = dwGainDAC1 >> 24;
    dwGain1R = (dwGainDAC1 & 0xffff) >> 8;

    dwGain2L = dwGainDAC2 >> 24;
    dwGain2R = (dwGainDAC2 & 0xffff) >> 8;

    dwGain3L = dwGainDAC3 >> 24;
    dwGain3R = (dwGainDAC3 & 0xffff) >> 8;

   // dwMask = 0xff;
    /*
    if(dwMask & WM8580_DAC_MASTER_ALL){
        
        WriteRegister(0x1c,dwGain1L | 0x100); //R28, master diti attenuation
       // return TRUE;
     }*/

    if(dwMask & WM8580_DAC_1){
        //WriteRegister(0x14,dwGain1L | 0x100);  
        WriteRegister(0x14,dwGain1L ); 
        WriteRegister(0x15,dwGain1R | 0x100);
    }

    if(dwMask & WM8580_DAC_2){
        //WriteRegister(0x16,dwGain2L | 0x100); 
        WriteRegister(0x16,dwGain2L );
        WriteRegister(0x17,dwGain2R | 0x100);
    }

    if(dwMask & WM8580_DAC_3){
        //WriteRegister(0x18,dwGain2L | 0x100);  
        WriteRegister(0x18,dwGain2L );
        WriteRegister(0x19,dwGain2R | 0x100);
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: SetOutputMute
//
//  This function does not function now
//  Parameters:
//      None
//
//  Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
BOOL CWM8580::SetOutputMute(BOOL bMute,DWORD dwMask)
{
    UNREFERENCED_PARAMETER(bMute);
    UNREFERENCED_PARAMETER(dwMask);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: GetDACBitClock
//
//  This function returns bit clock for the playback audio bus, mainly used for asrc
//  Parameters:
//      None
//
//  Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
UINT32 CWM8580::GetDACBitClock()
{
    if (m_audioProtocol == AUDIO_PROTOCOL_NETWORK){
        switch(m_dwOutputSampleRate){
            case 16000:
                return 12288000;
            case 22050:
                return 11289600;
            case 32000:
                return 12288000;
            case 44100:
                return 11289600;
            case 48000:
                return 12288000;
            /*case 64000:
                return 12288000;    
            case 88200:
                return 11289600;
            case 96000:
                return 12288000;*/
            default:
                return 0;
       }


    }else if (m_audioProtocol == AUDIO_PROTOCOL_LEFT_ALIGNED){
        //we always use 32 bits for one slot, and 2 slot for 1 frame on one bus
        return (m_dwOutputSampleRate*64);
    }

    return 0;
}

BOOL CWM8580::EnableCLKO(void)
{
    DWORD dwPLLB4;

    switch(m_dwOutputSampleRate){
        case 16000:
            dwPLLB4 = 0x154; 
            break;

        case 22050:
            dwPLLB4 = 0x14e;  
            break;
            
        case 32000:
            dwPLLB4 = 0x154; 
            break;

        case 44100:
            dwPLLB4 = 0x14e;  // 1010 01 1 10 pll b_>clk mclk    
            break;

        case 48000:
            dwPLLB4 = 0x154;  //10 10 10 1 00  ,mck from pll b    
            break;
            
        case 64000:
            dwPLLB4 = 0x154;  //10 10 10 1 00  ,mck from pll b    
            break;    

        case 88200:
            dwPLLB4 = 0x14e; 
            break;

        case 96000:
            dwPLLB4 = 0x154;
            break;

        case 128000:
            dwPLLB4 = 0x14c;
            break;
            
        case 176400:
            dwPLLB4 = 0x14c;
            break;    
            
        case 192000:
            dwPLLB4 = 0x14c;
            break;


        default:
            dwPLLB4 = 0x154;
            break;
    }
    
    dwPLLB4 |= 0x180; //CLKOUTSRC: 11 OSCCLK

    WriteRegister(0x07,dwPLLB4);

    m_bCLKOEnable =  TRUE;

    return TRUE;    
}

