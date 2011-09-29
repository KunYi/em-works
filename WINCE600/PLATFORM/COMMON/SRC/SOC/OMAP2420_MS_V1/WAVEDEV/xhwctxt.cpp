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
//
//  EAC controller configuration.
//
//  the implementation of these functions depends on the external configuration
//  of the omap chipset, e.g. which codec and modem is connected
//

#include <wavemain.h>
#include "xhwctxt.h"

// sidetone 0-100
#define DEFAULT_SIDETONE    100
// default 0db 
#define DEFAULT_DMAVOLUME   0xe7
#define DEFAULT_MIXVOLUME   0x67

//#undef DEBUGMSG
//#define DEBUGMSG(x, y) RETAILMSG(1,y)


static const 
DEVICE_REGISTRY_PARAM g_RegParams[] = {
    { 
        L"SidetoneHeadset", PARAM_DWORD, FALSE, 
        offset(ACAudioHWContext, m_nSidetoneHeadset),
        fieldsize(ACAudioHWContext, m_nSidetoneHeadset), 
        (VOID*)DEFAULT_SIDETONE
    }, 
    { 
        L"SidetoneSpeaker", PARAM_DWORD, FALSE, 
        offset(ACAudioHWContext, m_nSidetoneSpeaker),
        fieldsize(ACAudioHWContext, m_nSidetoneSpeaker), 
        (VOID*)DEFAULT_SIDETONE
    },
    { 
        L"DMAReadVol", PARAM_DWORD, FALSE, 
        offset(ACAudioHWContext, m_nDMAReadVol),
        fieldsize(ACAudioHWContext, m_nDMAReadVol), 
        (VOID*)DEFAULT_DMAVOLUME
    }, 
    { 
        L"DMAWriteVol", PARAM_DWORD, FALSE, 
        offset(ACAudioHWContext, m_nDMAWriteVol),
        fieldsize(ACAudioHWContext, m_nDMAWriteVol), 
        (VOID*)DEFAULT_DMAVOLUME
    }, 
    { 
        L"GSMSpeakerVol", PARAM_DWORD, FALSE, 
        offset(ACAudioHWContext, m_nGSMSpeakerVol),
        fieldsize(ACAudioHWContext, m_nGSMSpeakerVol), 
        (VOID*)DEFAULT_MIXVOLUME
    }, 
    { 
        L"WavSpeakerVol", PARAM_DWORD, FALSE, 
        offset(ACAudioHWContext, m_nWavSpeakerVol),
        fieldsize(ACAudioHWContext, m_nWavSpeakerVol), 
        (VOID*)DEFAULT_MIXVOLUME
    }, 
    { 
        L"WavGSMVol", PARAM_DWORD, FALSE, 
        offset(ACAudioHWContext, m_nWavGSMVol),
        fieldsize(ACAudioHWContext, m_nWavGSMVol), 
        (VOID*)DEFAULT_MIXVOLUME
    }, 
    { 
        L"GSMWavVol", PARAM_DWORD, FALSE, 
        offset(ACAudioHWContext, m_nGSMWavVol),
        fieldsize(ACAudioHWContext, m_nGSMWavVol), 
        (VOID*)DEFAULT_MIXVOLUME
    }, 
    { 
        L"StreamAttenMax", PARAM_DWORD, FALSE, 
        offset(HardwareContext, m_dwStreamAttenMax),
        fieldsize(HardwareContext, m_dwStreamAttenMax), 
        (VOID*)STREAM_ATTEN_MAX
    }, 
    { 
        L"SecondAttenMax", PARAM_DWORD, FALSE, 
        offset(HardwareContext, m_dwSecondAttenMax),
        fieldsize(HardwareContext, m_dwSecondAttenMax), 
        (VOID*)SECOND_ATTEN_MAX
    }, 
    { 
        L"DeviceAttenMax", PARAM_DWORD, FALSE, 
        offset(HardwareContext, m_dwDeviceAttenMax),
        fieldsize(HardwareContext, m_dwDeviceAttenMax), 
        (VOID*)DEVICE_ATTEN_MAX
    } 

};

//------------------------------------------------------------------------------
//
//  Function: ACAudioHWContext::ACAudioHWContext()
//  
//


ACAudioHWContext::ACAudioHWContext(LPTSTR lpszContext)
{
    // Bluetooth 
    m_CurBTEacConnection = CONNECTION_NOT_DEFINED;

    // modem 
    InitializeCriticalSection( &m_csModemDevice );
    m_fHeadsetOn = FALSE;
    m_fModemCodecOn = FALSE;
    m_hGsmDev = NULL;

    m_pMCBSPRegisters=NULL;
    m_pPRCMRegs=NULL;
    m_hSPI=NULL; 

    // power managment cs
    InitializeCriticalSection( &m_csAudioPRC );

    // EAC power
    m_bEACPowerOn=FALSE;         
    m_ExternPowerStateRequired = D4;
    
    // audio state variables
    m_bModemPortActive=FALSE;     
    m_bPowerTimeout=FALSE;        
    m_bBtHeadsetActive=FALSE;      
    m_bHeadsetActive=FALSE;       
    m_bLoudSpeakerActive=FALSE;   
    m_bChooseBtHeadset=FALSE;     

    // default audio sidetone volume settings
    m_nSidetoneHeadset=DEFAULT_SIDETONE;
    m_nSidetoneSpeaker=DEFAULT_SIDETONE;
    // mixer/dma default gain 0db
    m_nDMAReadVol    = DEFAULT_DMAVOLUME;
    m_nDMAWriteVol   = DEFAULT_DMAVOLUME;
    m_nGSMSpeakerVol = DEFAULT_MIXVOLUME;
    m_nWavSpeakerVol = DEFAULT_MIXVOLUME;
    m_nWavGSMVol     = DEFAULT_MIXVOLUME;
    m_nGSMWavVol     = DEFAULT_MIXVOLUME;

    // Read device parameters
    if (GetDeviceRegistryParams( 
         lpszContext,
         this, 
         dimof(g_RegParams), 
         g_RegParams
        )!=ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::ACAudioHWContext: "
            L"ReadRegistryParams() failed\r\n"
        ));
    }

   
}


//------------------------------------------------------------------------------
//
//  Function: ACAudioHWContext::~ACAudioHWContext()
//  
//

ACAudioHWContext::~ACAudioHWContext()
{
    // modem 
    DeleteCriticalSection( &m_csAudioPRC );
    DeleteCriticalSection( &m_csModemDevice );
    CloseDevice(m_hGsmDev);
}

//------------------------------------------------------------------------------
//
//  Function: EnableExtSpeaker
//  
//  Enable the hardware speaker.
//

void 
ACAudioHWContext::EnableExtSpeaker(BOOL bEnable)
{
    DEBUGMSG(ZONE_AC, (L"+ACAudioHWContext::EnableExtSpeaker %d\r\n", bEnable));
}

//------------------------------------------------------------------------------
//
//  Function:  ACAudioHWContext::HWInitNetwork()
//
//  Prepare network GSM audio master for data transfer.
//

BOOL 
ACAudioHWContext::HWInitNetwork( void )
{
    // Init parameters. 
    m_bModemPortActive=FALSE;     
    m_bPowerTimeout=FALSE;        
    m_bBtHeadsetActive=FALSE;      
    m_bHeadsetActive=FALSE;       
    m_bLoudSpeakerActive=FALSE;   
    m_bChooseBtHeadset=FALSE;     
    
    m_dwBtAudioRouting = BT_AUDIO_MODEM;
    
#ifdef CHOOSE_BT_HEADSET
    m_bChooseBtHeadset = TRUE;
#else
    m_bChooseBtHeadset = FALSE;
#endif

    InitModem ();
    HWUpdateAudioPRC ();
    if (!m_bEACPowerOn) {
        HWPowerDown();
    }

    return ( TRUE ); 
}   

//------------------------------------------------------------------------------
//
//  Function: ACAudioHWContext::HWDeinit()
//
//  Exit.
//

BOOL 
ACAudioHWContext::HWDeinit( void )
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: ACAudioHWContext::EAC_EnableNetwork()
//
//  Enable/disable network GSM audio data transfer.
//

BOOL 
ACAudioHWContext::HWEnableNetwork( BOOL bEnable )
{
    BOOL bRetVal = TRUE;    // Indicate no error.
    // this lock will nest the m_csAudioPRC critical section
    CAutoLock cs( &m_csAudioPRC );

    // Make sure the modem device is ready. 
    //    ModemNetworkEnabled(bEnable);

    // Check for enable network audio transfer.
    if ( bEnable )
    {
        // Check if network already running.
        if ( !m_bModemPortActive )
        {
            // Indicate network is running.
            m_bModemPortActive = TRUE;
        
            // Force not to use loud speaker. 
            m_bLoudSpeakerActive = FALSE;

            // Enable the audio path.
            HWUpdateAudioPRC();
       }
    }
    else    // Otherwise, disable network audio transfer.
    {
        // Check if network already stopped.
        if ( m_bModemPortActive )
        {
            // Indicate network is stopped.
            m_bModemPortActive = FALSE;
    
            // Force to use loud speaker. 
            m_bLoudSpeakerActive = TRUE;

            // Disable the audio path.
            HWUpdateAudioPRC();
        }
    }

    return ( bRetVal ); 
}   

//------------------------------------------------------------------------------
//
//  Function: ACAudioHWContext::AC_AudioPowerTimeout()
//
//  Indicates no activity so all amplifiers can be powered off.
//

BOOL 
ACAudioHWContext::HWAudioPowerTimeout( BOOL bTimeOut )
{
    CAutoLock cs( &m_csAudioPRC );

    // Power can be off since it is timeout. 
    m_bPowerTimeout=bTimeOut;

    HWUpdateAudioPRC ();

    return ( TRUE );
}

//------------------------------------------------------------------------------
//
//  Function: HWUpdateAudioPRC()
//
//  Power and routing control. Central enable/disable of audio ports, amplifiers,
//  paths according to state of routing and operation enable flags.
//

void 
ACAudioHWContext::HWUpdateAudioPRC( void )
{
    CAutoLock cs( &m_csAudioPRC );
    BOOL bBothHeadsetArePluggedIn = FALSE;

    DEBUGMSG(ZONE_AC, (L"+ACAudioHWContext::HWUpdateAudioPRC\r\n"));
    // The power can be off only: 
    //  1. when DMA is not running. 
    //  2. It is not during call.  

    if (!m_bDMARunning && !m_bModemPortActive)
    {
        // Force loud speaker off since backlight is off.
        if (m_bPowerTimeout)
        {
            if (m_bEACPowerOn)
            {
                m_bEACPowerOn = FALSE;

                DEBUGMSG(ZONE_AC,(L"ACAudioHWContext::HWUpdateAudioPRC: "
                    L"Force loud speaker off\r\n"
                ));
                // Turn off modem device.
                SetModemDevice (FALSE);

                // Power off codec
                SetCodecPower (FALSE);

                // Request to disable ontroller clocks. 
                SetControllerClocks (FALSE);  
            }
        }
        else
        {
            // Force loud speaker on when backlight is on.
            if (!m_bEACPowerOn)
            {
                DEBUGMSG(ZONE_AC, (L"ACAudioHWContext::HWUpdateAudioPRC: "
                    L"Force loud speaker on\r\n"
                ));
                // Turn on modem device.
                if (!SetModemDevice (TRUE))
                {
                    m_bEACPowerOn = FALSE;
                }
                else
                {
                    m_bEACPowerOn = TRUE;
                
                    // Make sure controller clocks are on first. 
                    SetControllerClocks (TRUE);  

                    // Power on codec
                    SetCodecPower (TRUE);
                }
            }
        }
    }

    // Otherwise we need to turn on the power. 
    else 
    {
        if (!m_bEACPowerOn)
        {   
            DEBUGMSG(ZONE_AC, (L"ACAudioHWContext::HWUpdateAudioPRC: "
                L"Turn on power\r\n"
            ));
            m_bPowerTimeout = FALSE;
            // Turn on modem device.
            if (!SetModemDevice (TRUE))
            {
                m_bEACPowerOn = FALSE;
            }
            else
            {
                m_bEACPowerOn = TRUE;

                // Make sure controller clocks are on first. 
                SetControllerClocks (TRUE);  

                // Power on codec
                SetCodecPower (TRUE);
            }
        }
    }

    // Headset routing.
    if (m_bHeadsetPluggedIn && m_bBtHeadsetSelected)
    {
        bBothHeadsetArePluggedIn = TRUE;

        if (m_bChooseBtHeadset)
            m_bHeadsetPluggedIn = FALSE;
        else
            m_bBtHeadsetSelected = FALSE;
    }


    if (m_bBtHeadsetSelected)
    {
        DEBUGMSG(ZONE_AC, (L"ACAudioHWContext::HWUpdateAudioPRC: "
            L"BtHeadsetSelected\r\n"));
        SelectBtAudio(m_dwBtAudioRouting);  
    }
    else
    {
        SelectVoiceCodec();
    }
    // default: headset off
    BOOL bNewHeadsetActive=FALSE;

    // Headset routing.
    if (m_bHeadsetPluggedIn || m_bBtHeadsetSelected)
    {
        // deactivate headset if somebody forces speaker mode,
        // e.g. incoming call
        bNewHeadsetActive = (m_NumForcedSpeaker == 0)?TRUE:FALSE;
    }

    if (!bNewHeadsetActive &&
         bNewHeadsetActive!=m_bHeadsetActive)
    {
        m_bHeadsetActive = bNewHeadsetActive;

        if (!m_bLoudSpeakerActive)
            SetSideTone (TRUE);

        SetModemHeadset (FALSE);
    } 
    else 
    if (bNewHeadsetActive &&
        bNewHeadsetActive!=m_bHeadsetActive)
    {
         m_bHeadsetActive = bNewHeadsetActive;

         if (!m_bLoudSpeakerActive)
             SetSideTone (TRUE);

         SetModemHeadset (TRUE);
    }

    // Toggle the loud speaker during voice call only.
    if (m_bModemPortActive && m_bToggleLoadSpeaker)
    {
        if (!m_bLoudSpeakerActive)
        {
            // Turn off mic Side Tone
            SetSideTone (FALSE);

            m_bLoudSpeakerActive = TRUE;
        }
        else
        {
            // Turn on mic Side Tone
            SetSideTone (TRUE);

            m_bLoudSpeakerActive = FALSE;
        }
    }

    // Reset the flag.
    m_bToggleLoadSpeaker = FALSE;

    // If both the physically connected headset and the
    // BT headset where connected, reset the variables
    if (bBothHeadsetArePluggedIn)
    {
        m_bBtHeadsetSelected    =
        m_bHeadsetPluggedIn     = TRUE;
    }
#ifdef DEBUG    
//  DumpMCBSPRegisters();
#endif
}
