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

#include <wavemain.h>
#include "xhwctxt.h"

#include "ti_constants.h"
#include "ceddk.h"

//------------------------------------------------------------------------------
//
//  Function: SelectBtAudio
//
//

void 
ACAudioHWContext::SelectBtAudio(DWORD dwAudioRouting)
{
    DEBUGMSG(ZONE_AC,(TEXT("AC: SelectBtAudio(%d)\r\n"),dwAudioRouting));

/*    EnableEAC12MClockREQ(TRUE);

    if (m_CurBTEacConnection != V3U3V2T4_TO_EAC_BT_AUSPI)
    {
        ConfigEacBTAuSpiPins(V3U3V2T4_TO_EAC_BT_AUSPI);         
    }

    m_CurBTEacConnection = V3U3V2T4_TO_EAC_BT_AUSPI;   


    if (dwAudioRouting & BT_AUDIO_SYSTEM)
    {
        // Enable system audio to BT AUSPI port
        SETREG16(&m_pMCBSPRegisters->AMSCFR,BIT10);
    }
    else
    {
        // Disable system audio to BT AUSPI port
        CLRREG16(&m_pMCBSPRegisters->AMSCFR,BIT10);
    }
    
    if (dwAudioRouting & BT_AUDIO_MODEM)
    {
        // Enable modem audio to BT Auspi port
        SETREG16(&m_pMCBSPRegisters->AMSCFR, BIT9 | BIT11);
    }
    else
    {
        // Disable modem audio to BT AUSPI port
        CLRREG16(&m_pMCBSPRegisters->AMSCFR, BIT9 | BIT11);
    }
*/        
}

//------------------------------------------------------------------------------
//
//  Function: SelectVoiceCodec
//
//

void 
ACAudioHWContext::SelectVoiceCodec()
{
    DEBUGMSG(ZONE_AC,(TEXT("AC: SelectVoiceCodec\r\n")));
    
/*    if (m_CurBTEacConnection != W6R9Y6Y5_TO_EAC_BT_AUSPI)
    {
        EnableEAC12MClockREQ(TRUE);
        ConfigEacBTAuSpiPins(W6R9Y6Y5_TO_EAC_BT_AUSPI);    
        
        // Enable modem audio to BT Auspi port
        SETREG16(&m_pMCBSPRegisters->AMSCFR,BIT9 | BIT11);

        // Disable system audio to BT AUSPI port
        CLRREG16(&m_pMCBSPRegisters->AMSCFR,BIT10);
    }
            
    m_CurBTEacConnection = W6R9Y6Y5_TO_EAC_BT_AUSPI;   
*/        
}


