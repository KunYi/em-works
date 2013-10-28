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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File: mcbspprofile.c
//

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <Pkfuncs.h>
#include <pm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <memtxapi.h>
#include <initguid.h>
#include <bus.h>

#include "debug.h"
#include "mcbsptypes.h"
#include "mcbspprofile.h"


//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::Initialize
//
//
void
McbspProfile_t::Initialize()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_ConfigCommonRegisters(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}

//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::ContextRestore
//
//
void
McbspProfile_t::ContextRestore()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_UpdateRegisters(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::EnableSampleRateGenerator
//
//
void
McbspProfile_t::EnableSampleRateGenerator()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_EnableSampleRateGenerator(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}

//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::EnableTransmitter
//
//
void
McbspProfile_t::EnableTransmitter()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_EnableTransmitter(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::EnableReceiver
//
//
void
McbspProfile_t::EnableReceiver()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_EnableReceiver(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::ResetSampleRateGenerator
//
//
void
McbspProfile_t::ResetSampleRateGenerator()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_ResetSampleRateGenerator(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::ResetTransmitter
//
//
void
McbspProfile_t::ResetTransmitter()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_ResetTransmitter(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::ResetReceiver
//
//
void
McbspProfile_t::ResetReceiver()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_ResetReceiver(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: McbspProfile_t::ClearIRQStatus
//
//
void
McbspProfile_t::ClearIRQStatus()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_ClearIRQStatus(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
// I2SSlaveProfile_t class functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function: I2SSlaveProfile_t::Initialize
//
//
void
I2SSlaveProfile_t::Initialize()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    ConfigureDevConfForClks(m_pDevice, FALSE);

    // initialize shadow registers
    //
    mcbsp_ResetShadowRegisters(m_pDevice);

    // overwrite default configuration with registry values
    //
    if (m_pDevice->useRegistryForMcbsp)
        {
        mcbsp_GetRegistryValues(m_pDevice);
        }

    mcbsp_ResetSampleRateGenerator(m_pDevice);
    mcbsp_ResetTransmitter(m_pDevice);
    mcbsp_ResetReceiver(m_pDevice);

    mcbsp_ConfigureSampleRateGenerator(m_pDevice);
    mcbsp_ConfigureTransmitter(m_pDevice);
    mcbsp_ConfigureReceiver(m_pDevice);
    mcbsp_ClearIRQStatus(m_pDevice);
    mcbsp_UpdateRegisters(m_pDevice);

    // Do I2S specific configuration
    mcbsp_ConfigI2SProfile(m_pDevice);

#ifdef DEBUG
    //mcbsp_DumpReg(m_pDevice, L"After configuration");
    RETAILMSG(1,(TEXT("McBSP is in Slave mode\r\n")));
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: I2SSlaveProfile_t::ContextRestore
//
//
void
I2SSlaveProfile_t::ContextRestore()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    // Call base class function
    McbspProfile_t::ContextRestore();

    // Do I2S specific configuration
    mcbsp_ConfigI2SProfile(m_pDevice);

#ifdef DEBUG
    mcbsp_DumpReg(m_pDevice, L"After context restore configuration");
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}

//------------------------------------------------------------------------------
// I2SMasterProfile_t class functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function: I2SMasterProfile_t::Initialize
//
//
void
I2SMasterProfile_t::Initialize()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    // McBSP to use external clock (T2 256fs clk) for CLKS
    ConfigureDevConfForClks(m_pDevice, TRUE);

    // initialize shadow registers
    //
    mcbsp_ResetShadowRegisters(m_pDevice);

    // Configure for MASTER mode
    //
    mcbsp_ConfigureForMaster(m_pDevice);

    mcbsp_ResetSampleRateGenerator(m_pDevice);
    mcbsp_ResetTransmitter(m_pDevice);
    mcbsp_ResetReceiver(m_pDevice);

    mcbsp_ConfigureSampleRateGenerator(m_pDevice);
    mcbsp_ConfigureTransmitter(m_pDevice);
    mcbsp_ConfigureReceiver(m_pDevice);

    mcbsp_ClearIRQStatus(m_pDevice);
    mcbsp_UpdateRegisters(m_pDevice);

    // Do I2S specific configuration
    mcbsp_ConfigI2SProfile(m_pDevice);

#ifdef DEBUG
    //mcbsp_DumpReg(m_pDevice, L"After configuration");
    RETAILMSG(1,(TEXT("McBSP is in Master mode\r\n")));
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: I2SMasterProfile_t::ContextRestore
//
//
void
I2SMasterProfile_t::ContextRestore()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    // Call base class function
    McbspProfile_t::ContextRestore();

    // Do I2S specific configuration
    mcbsp_ConfigI2SProfile(m_pDevice);

#ifdef DEBUG
    mcbsp_DumpReg(m_pDevice, L"After context restore configuration");
#endif

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}

//------------------------------------------------------------------------------
// TDMProfile_t class functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function: TDMProfile_t::Initialize
//
//
void
TDMProfile_t::Initialize()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    // Call base class function
    McbspProfile_t::Initialize();

    // Do TDM specific configuration
    mcbsp_ConfigTDMProfile(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: TDMProfile_t::ContextRestore
//
//
void
TDMProfile_t::ContextRestore()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    // Call base class function
    McbspProfile_t::ContextRestore();

    // Do TDM specific configuration
    mcbsp_ConfigTDMProfile(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}


//------------------------------------------------------------------------------
//
//  Function: TDMProfile_t::SetTxChannelsRequested
//
//
void
TDMProfile_t::SetTxChannelsRequested()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_ConfigTDMTxChannels(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}

//------------------------------------------------------------------------------
//
//  Function: TDMProfile_t::SetRxChannelsRequested
//
//
void
TDMProfile_t::SetRxChannelsRequested()
{
    DEBUGMSG(ZONE_FUNCTION, (L"MCP:+%S\r\n", __FUNCTION__));

    mcbsp_ConfigTDMRxChannels(m_pDevice);

    DEBUGMSG(ZONE_FUNCTION, (L"MCP:-%S\r\n", __FUNCTION__));
}

//------------------------------------------------------------------------------
