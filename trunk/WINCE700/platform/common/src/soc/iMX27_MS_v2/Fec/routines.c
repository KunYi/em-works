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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//

#include "precomp.h"
#pragma hdrstop
#pragma warning (disable: 4514 4706)

extern BOOL BSPFECIomuxConfig( IN BOOL Enable );
extern BOOL BSPFECClockConfig( IN BOOL Enable );
extern PCSP_FEC_REGS   gpFECReg; 

//-----------------------------------------------------------------------------
// Procedure: NICIssueFullReset
//
// Description:
//
// Arguments: IN Adapter structure pointer
//
// Returns:  void
//-----------------------------------------------------------------------------

VOID NICIssueFullReset(
    PMP_ADAPTER Adapter)
{
    NICDisableInterrupt(Adapter);
    FECEnetReset(Adapter,FALSE);    //HW reset
}

//-----------------------------------------------------------------------------
// Procedure: NICGetMediaState
//
// Description: This function returns the the new media state. If media state
//                   is changed, then it indicates to NDIS also.
//
// Arguments: IN Adapter structure pointer
//
// Returns:   MediaConnectStateConnected
//            MediaConnectStateDisconnected
//-----------------------------------------------------------------------------

NDIS_MEDIA_CONNECT_STATE 
NICGetMediaState(
    IN PMP_ADAPTER Adapter
    )
{
    return Adapter->MediaState;
}
