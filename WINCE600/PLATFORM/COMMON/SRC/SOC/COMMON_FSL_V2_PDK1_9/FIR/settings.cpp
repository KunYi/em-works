//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  settings.cpp
//
//   This file implements the device specific functions for iMX51 fir device.
//
//------------------------------------------------------------------------------
#include "IrFir.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

baudRateInfo supportedBaudRateTable[NUM_BAUDRATES] = {
    {
        BAUDRATE_2400,
        2400,
        NDIS_IRDA_SPEED_2400,
    },
    {
        BAUDRATE_9600,
        9600,
        NDIS_IRDA_SPEED_9600,
    },
    {
        BAUDRATE_19200,
        19200,
        NDIS_IRDA_SPEED_19200,
    },
    {
        BAUDRATE_38400,
        38400,
        NDIS_IRDA_SPEED_38400,
    },
    {
        BAUDRATE_57600,
        57600,
        NDIS_IRDA_SPEED_57600,
    },
    {
        BAUDRATE_115200,
        115200,
        NDIS_IRDA_SPEED_115200,
    },
    {
        BAUDRATE_576000,
        576000,
        NDIS_IRDA_SPEED_576K,
    },
    {
        BAUDRATE_1152000,
        1152000,
        NDIS_IRDA_SPEED_1152K,
    },
    {
        BAUDRATE_4000000,
        4000000,
        NDIS_IRDA_SPEED_4M,
    }
};

_HW_IR_VTBL SIR_VTbl =
{
    SirInitialize,
    SirDeInitialize,
    SirOpen,
    SirClose,
    SirEnableInterrupt,
    SirDisableInterrupt,
    SirSendPacketQ,
    SirSend,
    SirReceive,
    SirInterruptHandler,
    SirSetSpeed
};


_HW_IR_VTBL FIR_VTbl =
{
    FirInitialize,
    FirDeinitialize,
    FirOpen,
    FirClose,
    FirEnableInterrupt,
    FirDisableInterrupt,
    FirSendPacketQ,
    FirSend,
    FirReceive,
    FirInterruptHandler,
    FirSetSpeed
};

//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: SetSpeed
//
//  This function sets the device to required speed level.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    This function returns the status of setting speed.
//
//-----------------------------------------------------------------------------
BOOLEAN SetSpeed( pFirDevice_t thisDev)
{
    baudRates baudrate;

    BOOLEAN firstInitialization = (!thisDev->linkSpeedInfo);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: **** SetSpeed(%d) ****\r\n"), thisDev->newSpeed));

    if (!IsListEmpty(&thisDev->SendQueue))
    {
        //  We can't set speed in the hardware while
        //  send packets are queued.
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: delaying set speed until all packets sent!\r\n")));

        if (thisDev->SendQueue.Blink==thisDev->SendQueue.Flink)
        {
            // Only one.  We need to change after this one.
            thisDev->setSpeedAfterCurrentSendPacket = TRUE;
        }
        else
        {
            thisDev->lastPacketAtOldSpeed = CONTAINING_RECORD(thisDev->SendQueue.Blink,
                NDIS_PACKET, MiniportReserved);
        }
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: delaying set-speed because send pkts queued\r\n")));
        return TRUE;
    }
    else if (thisDev->writePending)
    {
        thisDev->setSpeedAfterCurrentSendPacket = TRUE;
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: will set speed after current write pkt\r\n")));
        return TRUE;
    }
    else if(!thisDev->linkSpeedInfo)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: linkSpeedInfo ==  NULL.\r\n")));
        if(thisDev->newSpeed <= MAX_SIR_SPEED)
        {
            thisDev->IR_VTbl = &SIR_VTbl;
        }
        else
        {
            thisDev->IR_VTbl = &FIR_VTbl;
        }
    }
    else if(thisDev->linkSpeedInfo->bitsPerSec <= MAX_SIR_SPEED)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: set SIR speed\r\n")));
        // Disable interrupt first
        thisDev->IR_VTbl->m_pDisableInterrupt(thisDev);

        if(thisDev->newSpeed > MAX_SIR_SPEED)
        {
            // SIR to MIR/FIR
            thisDev->IR_VTbl->m_pReleaseAdapterResources(thisDev);
            thisDev->IR_VTbl = &FIR_VTbl;
            if(!thisDev->IR_VTbl->m_pAcquireAdapterResources(thisDev))
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("Settings: FiriOpen failed!\r\n")));
                return FALSE;
            }

            if (thisDev->newSpeed <= MAX_MIR_SPEED)
            {
                //  SIR to MIR
                BSPIrdaSetMode(MIR_MODE);
            }
            else
            {
                //  SIR to FIR
                BSPIrdaSetMode(FIR_MODE);
            }
        }
        else
        {
            // SIR to SIR
        }
    }
    else if(thisDev->linkSpeedInfo->bitsPerSec <= MAX_MIR_SPEED)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: set MIR speed\r\n")));
        // Disable interrupt first
        thisDev->IR_VTbl->m_pDisableInterrupt(thisDev);

        if(thisDev->newSpeed <= MAX_SIR_SPEED)
        {
            // MIR to SIR
            thisDev->IR_VTbl->m_pReleaseAdapterResources(thisDev);
            thisDev->IR_VTbl = &SIR_VTbl;
            if(!thisDev->IR_VTbl->m_pAcquireAdapterResources(thisDev))
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("Settings: Open failed!\r\n")));
                return FALSE;
            }

            BSPIrdaSetMode(SIR_MODE);
        }
        else if (thisDev->newSpeed > MAX_MIR_SPEED)
        {
            //  MIR to FIR
            BSPIrdaSetMode(FIR_MODE);
        }
        else
        {
            //  MIR to MIR
        }
    }
    else
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: set FIR speed\r\n")));
        // Disable interrupt first
        thisDev->IR_VTbl->m_pDisableInterrupt(thisDev);

        if(thisDev->newSpeed <= MAX_SIR_SPEED)
        {
            // FIR to SIR
            thisDev->IR_VTbl->m_pReleaseAdapterResources(thisDev);
            thisDev->IR_VTbl = &SIR_VTbl;
            if(!thisDev->IR_VTbl->m_pAcquireAdapterResources(thisDev))
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("Settings: Open failed!\r\n")));
                return FALSE;
            }

            BSPIrdaSetMode(SIR_MODE);
        }
        else if (thisDev->newSpeed <= MAX_MIR_SPEED)
        {
            // FIR to MIR
            BSPIrdaSetMode(MIR_MODE);
        }
        else
        {
            // FIR to FIR
        }
    }

    baudrate = thisDev->IR_VTbl->m_pSetupSpeed(thisDev);

    if (baudrate == BAUDRATE_INVALID)
    {
        DEBUGMSG(ZONE_WARN, (TEXT("Settings: set baudrate failed!\r\n")));
        return FALSE;
    }
    thisDev->linkSpeedInfo = &supportedBaudRateTable[baudrate];

    // Start HW to receive data
    if(!firstInitialization)
        thisDev->IR_VTbl->m_pSetupRecv(thisDev);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Settings: -SetSpeed\r\n")));

    return TRUE;
}
