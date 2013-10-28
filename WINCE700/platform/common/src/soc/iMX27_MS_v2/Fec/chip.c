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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  chip.c
//
//  Implementation of FEC Driver
//
//  This file implements hardware related functions for FEC.
//
//-----------------------------------------------------------------------------


#include "precomp.h"
#include "phys.h"

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------
extern PCSP_FEC_REGS    gpFECReg;
NDIS_HANDLE             gFECNdisHandle;

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
FEC_MII_LIST   gMIICmds[FEC_MII_COUNT];
PFEC_MII_LIST  gMIIFree = NULL;
PFEC_MII_LIST  gMIIHead = NULL;
PFEC_MII_LIST  gMIITail = NULL;

//------------------------------------------------------------------------------
// File-local(static) Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

extern BOOL BSPFECIomuxConfig( IN BOOL Enable );
extern BOOL BSPFECClockConfig( IN BOOL Enable );

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

void  FECSetMII(IN  PMP_ADAPTER Adapter);
void  FECResetMII(IN  PMP_ADAPTER Adapter);

BOOL FECQueueMII(
    IN  PMP_ADAPTER Adapter,
    IN UINT RegValue,
    IN void (*OpFunc)(UINT, NDIS_HANDLE)
    );

void FECDoMIICmd(
    IN  PMP_ADAPTER Adapter,
    IN PFEC_PHY_CMD pCmd
    );

void FECGetPHYId2(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    );

//------------------------------------------------------------------------------
// Functions implementation
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// MII management related functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: FECSetMII
//
// This function sets the clock for the MII interface
//
// Parameters:
//      Adapter
//          [in] Pointer to a structure which contains the status and control
//               information for the FEC controller and MII
// Return Value:
//      None.
//
//------------------------------------------------------------------------------
void  FECSetMII(IN  PMP_ADAPTER Adapter)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECSetMII\r\n")));

    // Set MII speed to 2.5MHz
    Adapter->MIIPhySpeed = 14;

    OUTREG32(&gpFECReg->MSCR,
                     CSP_BITFVAL(FEC_MSCR_MIISPEED, Adapter->MIIPhySpeed));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECSetMII\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECResetMII
//
// This function resets the clock for the MII interface
//
// Parameters:
//      Adapter
//          [in] Pointer to a structure which contains the status and control
//               information for the FEC controller and MII
// Return Value:
//      None.
//
//------------------------------------------------------------------------------
void  FECResetMII(IN  PMP_ADAPTER Adapter)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECResetMII\r\n")));

    //reset the MII Control Register
    OUTREG32(&gpFECReg->MSCR,0);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECResetMII\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECQueueMII
//
// This function adds a MII management command to the MII command list
//
// Parameters:
//      Adapter
//          [in] Pointer to a structure which contains the status and control
//               information for the FEC controller and MII
//      RegValue
//          [in] The MII command which will be added to the MII command list
//
//      OpFunc
//          [in] An optional function which will be performed when the MII interrupt
//               arrived.
// Return Value:
//      TRUE if the MII command has been added to the command list successfully,
//      FALSE if the command list is full.
//
//------------------------------------------------------------------------------
BOOL FECQueueMII(
    PMP_ADAPTER     Adapter,
    IN UINT RegValue,
    IN void (*OpFunc)(UINT, NDIS_HANDLE)
    )
{
    BOOL RetVal = TRUE;
    PFEC_MII_LIST MIIPoint;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECQueueMII\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECQueueMII Reg Value 0x%x\n"), RegValue));

    // add PHY address to the MII command
    RegValue |= CSP_BITFVAL(FEC_MMFR_PA, Adapter->MIIPhyAddr);

    if((MIIPoint = gMIIFree) != NULL)
    {
        gMIIFree = MIIPoint->MIINext;
        MIIPoint->MIIRegVal = RegValue;
        MIIPoint->MIIFunction = OpFunc;
        MIIPoint->MIINext = NULL;

        if(gMIIHead)
        {
            gMIITail->MIINext = MIIPoint;
            gMIITail = MIIPoint;
        }
        else
        {
            gMIIHead = gMIITail = MIIPoint;
            OUTREG32(&gpFECReg->MMFR, RegValue);
        }
    }
    else
    {
        RetVal = FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECQueueMII\r\n")));

    return RetVal;
}

//------------------------------------------------------------------------------
//
// Function: FECDoMIICmd
//
// This function will call FECQueueMII to queue all the requested MII management
// commands to the sending list.
//
// Parameters:
//      Adapter
//          [in] Points to a structure which contains the status and control
//               information for the FEC controller and MII
//
//      pCmd
//          [in] Points to a FEC_PHY_CMD array which specifies the MII management
//               commands and the parsing functions
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECDoMIICmd(
    IN  PMP_ADAPTER Adapter,
    IN PFEC_PHY_CMD pCmd
    )
{
    UINT i;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECDoMIICmd\r\n")));

    if(!pCmd)
        return;

    for(i = 0; (pCmd + i)->MIIData != FEC_MII_END; i++ )
    {
        FECQueueMII(Adapter, (pCmd + i)->MIIData, (pCmd + i)->MIIFunct);
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECDoMIICmd\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseMIISr
//
// This function parses the status register data of the external MII compatible
// PHY(s).
//
// Parameters:
//      RegVal
//          [in] the status register value get from external MII compatible
//               PHY(s)
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECParseMIISr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;

    PMP_ADAPTER Adapter = ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseMIISr\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECParseMIISr Reg Value 0x%x\n"), RegVal));

    s = &(Adapter->MIIPhyStatus);
    Status = *s & ~(PHY_STAT_LINK | PHY_STAT_FAULT | PHY_STAT_ANC);

    if(RegVal & LAN8700_BASICSTATUS_LINKSTATUS)
        Status |= PHY_STAT_LINK;
    if(RegVal & LAN8700_BASICSTATUS_REMOTEFAULT)
        Status |= PHY_STAT_FAULT;
    if(RegVal & LAN8700_BASICSTATUS_AN_COMPLETE)
        Status |= PHY_STAT_ANC;

    *s = Status;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseMIISr\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseMIICr
//
// This function parses the control register data of the external MII compatible
// PHY(s).
//
// Parameters:
//      RegVal
//          [in] the control register value get from external MII compatible
//               PHY(s)
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECParseMIICr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;

    PMP_ADAPTER Adapter = ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseMIICr\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECParseMIICr Reg Value 0x%x\n"), RegVal));

    s = &(Adapter->MIIPhyStatus);
    Status = *s & ~(PHY_CONF_ANE | PHY_CONF_LOOP);

    if(RegVal & LAN8700_BASICCONTROL_AN_ENABLE)
        Status |= PHY_CONF_ANE;
    if(RegVal & LAN8700_BASICCONTROL_LOOPBACK)
        Status |= PHY_CONF_LOOP;

    *s = Status;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseMIICr\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseMIIAnar
//
// This function parses the Auto-Negotiation Advertisement Register data of the
// external MII compatible PHY(s).
//
// Parameters:
//      RegVal
//          [in] the Auto-Negotiation Advertisement Register value get from
//               external MII compatible PHY(s)
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECParseMIIAnar(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;

    PMP_ADAPTER Adapter = ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseMIIAnar\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECParseMIIAnar Reg Value 0x%x\n"), RegVal));

    s = &(Adapter->MIIPhyStatus);
    Status = *s & ~(PHY_CONF_SPMASK);

    if(RegVal & LAN8700_AN_ADVERTISEMENT_10BASE_T)
        Status |= PHY_CONF_10HDX;
    if(RegVal & LAN8700_AN_ADVERTISEMENT_10BASE_T_FULLDUPLEX)
        Status |= PHY_CONF_10FDX;
    if(RegVal & LAN8700_AN_ADVERTISEMENT_100BASE_TX)
        Status |= PHY_CONF_100HDX;
    if(RegVal & LAN8700_AN_ADVERTISEMENT_100BASE_TX_FULLDUPLEX)
        Status |= PHY_CONF_100FDX;

    *s = Status;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseMIIAnar\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseLAN8700SR2
//
// This function parses the status register data of the external LAN8700 PHY.
//
// Parameters:
//      RegVal
//          [in] the status Register value get from external LAN8700 PHY
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECParseLAN8700SR2(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;

    PMP_ADAPTER Adapter = ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseAm79c874Sr\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECParseAm79c874Sr Reg Value 0x%x\n"), RegVal));

    s = &(Adapter->MIIPhyStatus);
    Status = *s & ~(PHY_STAT_SPMASK | PHY_STAT_ANC);
    Adapter->SpeedMode = FALSE;

    if(RegVal & LAN8700_BASICSTATUS_AN_COMPLETE)
    {
        Status |= PHY_STAT_ANC;
        if(RegVal & LAN8700_BASICSTATUS_100BASE_TX_FULLDUPLEX)
        {
            Status |= PHY_STAT_100FDX;
            Adapter->SpeedMode = TRUE;
        }
        else if(RegVal & LAN8700_BASICSTATUS_100BASE_TX_HALFDUPLEX)
        {
            Status |= PHY_STAT_100HDX;
            Adapter->SpeedMode = TRUE;
        }
        else if(RegVal & LAN8700_BASICSTATUS_10BASE_T_FULLDUPLEX)
        {
            Status |= PHY_STAT_10FDX;
        }
        else if(RegVal & LAN8700_BASICSTATUS_10BASE_TX_HALFDUPLEX)
        {
            Status |= PHY_STAT_10HDX;
        }
    }
    if (Adapter->SpeedMode)
    {
        Adapter->usLinkSpeed = SPEED_100MBPS;
    }
    else
    {
        Adapter->usLinkSpeed = SPEED_10MBPS;
    }

    *s = Status;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseLAN8700SR2\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseLAN8700Isr
//
// This function parses the interrupt source register data of the external
// LAN8700 PHY.
//
// Parameters:
//      RegVal
//          [in] the interrupt source Register value from external LAN8700 PHY
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECParseLAN8700Isr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    return;
}

//------------------------------------------------------------------------------
//
// Function: FECParsePHYLink
//
// This function will update the link status according to the Status register
// of the external PHY.
//
// Parameters:
//      RegVal
//          [in] the MII frame value which is read from external PHY registers
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize.
//
// Return Value:
//      None.
//
//------------------------------------------------------------------------------
void FECParsePHYLink(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    NDIS_MEDIA_CONNECT_STATE OldState;
    NDIS_MEDIA_CONNECT_STATE CurrentState;

    PMP_ADAPTER Adapter = ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParsePHYLink\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECParsePHYLink Reg Value 0x%x\n"), RegVal));

    if(RegVal & 0x0004)     //SR 3rd bit is link status
    {
        Adapter->MIIPhyStatus |= PHY_STAT_LINK;
        Adapter->LinkStatus = TRUE;
        CurrentState =  MediaConnectStateConnected;
    }else
    {
        Adapter->LinkStatus = FALSE;
        CurrentState = MediaConnectStateDisconnected;
    }
    OldState = Adapter->MediaState;
    if (OldState != CurrentState)
    {
        RETAILMSG(TRUE,(TEXT("FECParsePHYLink- Media State Toggle- %d \n"),CurrentState));
        NdisDprAcquireSpinLock(&Adapter->Lock);
        Adapter->MediaState = CurrentState;
        FECIndicateLinkState(Adapter);
        DBGPRINT(ZONE_WARN, ("FECParsePHYLink: Media state changed to %s\n",
                ((Adapter->MediaState == MediaConnectStateConnected)?
                "Connected": "Disconnected")));
        NdisDprReleaseSpinLock(&Adapter->Lock);
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParsePHYLink\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECGetPHYId
//
// Scan all of the MII PHY addresses looking for someone to respond with a valid
// ID. This usually happens quickly.
//
// Parameters:
//      MIIReg
//          [in] the MII frame value which is read from external PHY registers
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize.
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECGetPHYId(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT PhyType;
    PMP_ADAPTER Adapter = ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECGetPHYId\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECGetPHYId MII Reg Value 0x%x\n"), MIIReg));

    Sleep(100);

    if(Adapter->MIIPhyAddr < PHY_MAX_COUNT)
    {
        if((PhyType = (MIIReg & MMI_DATA_MASK)) != MMI_DATA_MASK && PhyType != 0)
        {
            // The first part of the ID have been got, then get the
            // the remainder
            Adapter->MIIPhyId = PhyType << 16;
            FECQueueMII(Adapter, MII_READ_COMMAND(MII_REG_PHYIR2), FECGetPHYId2);
        }
        else
        {
            // Try the next PHY address
            Adapter->MIIPhyAddr++;
            FECQueueMII(Adapter, MII_READ_COMMAND(MII_REG_PHYIR1), FECGetPHYId);
        }
    }else
    {
        // Close the clock for MII
        Adapter->MIIPhySpeed = 0;
        INSREG32BF( &gpFECReg->MSCR, FEC_MSCR_MIISPEED, Adapter->MIIPhySpeed );

        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: No external PHY found\r\n"), __WFUNCTION__));
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECGetPHYId\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECGetPHYId2
//
// Read the second part of the external PHY id.
//
// Parameters:
//      MIIReg
//          [in] the MII frame value which is read from external PHY registers
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize.
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------

void FECGetPHYId2(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT i;

    PMP_ADAPTER Adapter= ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECGetPHYId2\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECGetPHYId2 MII Reg Value 0x%x\n"), MIIReg));

    Adapter->MIIPhyId |= (MIIReg & MMI_DATA_MASK);

    for(i = 0; PhyInfo[i] != NULL; i++)
    {
        if(PhyInfo[i]->PhyId == Adapter->MIIPhyId)
            break;
    }
    if(PhyInfo[i])
        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: The name for the external PHY is %s\r\n"), __WFUNCTION__, PhyInfo[i]->PhyName));
    else
        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: No supported PHY found\r\n"), __WFUNCTION__));

    Adapter->MIIPhy = PhyInfo[i];
    Adapter->MIIPhyIdDone = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECGetPHYId2\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECUpdateLinkStatus
//
// This function will send the command to the external PHY to get the link
// status of the cable. The updated link status is stored in the context
// area designated by the parameter MiniportAdapterContext.
//
// Parameters:
//      MiniportAdapterHandle
//          [in] Specifies the handle to a FEC driver allocated context area
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize.
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECUpdateLinkStatus(
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    PMP_ADAPTER Adapter= ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECUpdateLinkStatus\r\n")));

    FECDoMIICmd(Adapter, PHYCmdLink);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECUpdateLinkStatus\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECDispPHYCfg
//
// This function displays the current status of the external PHY
//
// Parameters:
//      MIIReg
//          [in] the MII frame value which is read from external PHY registers
//
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize.
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECDispPHYCfg(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;

    PMP_ADAPTER Adapter= ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECDispPHYCfg\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECDispPHYCfg MII Reg Value 0x%x\n"), MIIReg));

    Status = Adapter->MIIPhyStatus;
    if(Status & PHY_CONF_ANE)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: Auto-negotiation is on\r\n"), __WFUNCTION__));
    else
        DEBUGMSG(ZONE_INFO, (TEXT("%s: Auto-negotiation is off\r\n"), __WFUNCTION__));

    if(Status & PHY_CONF_100FDX)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: 100M Full Duplex Mode\r\n"), __WFUNCTION__));
    if(Status & PHY_CONF_100HDX)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: 100M Half Duplex Mode\r\n"), __WFUNCTION__));
    if(Status & PHY_CONF_10FDX)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: 10M Full Duplex Mode\r\n"), __WFUNCTION__));
    if(Status & PHY_CONF_10HDX)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: 10M Half Duplex Mode\r\n"), __WFUNCTION__));
    if(!(Status & PHY_CONF_SPMASK))
        DEBUGMSG(ZONE_INFO, (TEXT("%s: No speed/duplex selected\r\n"), __WFUNCTION__));
    if(Status & PHY_CONF_LOOP)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: Loop back mode is enabled\r\n"), __WFUNCTION__));

    Adapter->MIISeqDone =  TRUE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECDispPHYCfg\r\n")));
    return;
}

//------------------------------------------------------------------------------
//
// Function: FECEnetDeinit
//
// This function Deinitialize the FEC hardware. It will free the DMA buffers and
// disable the GPIO and CLK for FEC.
//
// Parameters:
//      pEthernet
//          [in] the FEC driver context area allocated in function FECInitialize
//
// Return Value:
//      None.
//
//------------------------------------------------------------------------------
void FECEnetDeinit()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnetDeinit\r\n")));

    // disable GPIO and CLK for FEC
    BSPFECIomuxConfig( FALSE );
    BSPFECClockConfig( FALSE );

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnetDeinit\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECEnetAutoNego
//
// This function issues the auto-negotiation sequences to the external PHY device
//
// Parameters:
//      Adapter
//          [in] the FEC driver context area allocated in function FECInitialize
//
// Return value
//      None
//
//------------------------------------------------------------------------------
NDIS_STATUS FECEnetAutoNego(IN PMP_ADAPTER Adapter)
{
    int iRetries = 0;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnetAutoNego\r\n")));

    // Wait for MII interrupt to set to TURE
    while ((iRetries < MAX_RETRY_AUTONEGO) && (Adapter->MIIPhyIdDone != 1)) //Trying till 10 second
    {
        iRetries++;
        NdisMSleep (SLEEP_RETRY_AUTONEGO);
    }
    if (iRetries < MAX_RETRY_AUTONEGO)
    {
        if(Adapter->MIIPhy)
        {
            // Set to auto-negotiation mode and restart the
            // auto-negotiation process
            FECDoMIICmd(Adapter, Adapter->MIIPhy->PhyActint);
            FECDoMIICmd(Adapter, Adapter->MIIPhy->PhyConfig);
            FECDoMIICmd(Adapter, PHYCmdCfg);
        }
    }
    else
    {
        Status = NDIS_STATUS_FAILURE;
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnetAutoNego\r\n")));
    return Status;
}

//------------------------------------------------------------------------------
//
// Function: CalculateHashValue
//
// This function calculates the 6-bit Hash value for multicasting.
//
// Parameters:
//      pAddr
//        [in] pointer to a ethernet address
//
// Returns:
//      Returns the calculated 6-bit Hash value.
//
//------------------------------------------------------------------------------
UCHAR CalculateHashValue(UCHAR *pAddr)
{
    ULONG CRC;
    UCHAR HashValue = 0;
    UCHAR AddrByte;
    int byte, bit;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +CalculateHashValue\r\n")));

    // calculate CRC32 value of MAC address
    CRC = CRC_PRIME;

    for(byte=0; byte < ETHER_ADDR_SIZE; byte++)
    {
        AddrByte = *pAddr++;

        for(bit = 0; bit < 8; bit++, AddrByte >>= 1)
        {
            CRC = (CRC >> 1) ^
                    (((CRC ^ AddrByte) & 1) ? CRC_POLYNOMIAL : 0);
        }
    }

    // only upper 6 bits (HASH_BITS) are used which point to specific
    // bit in the hash registers
    HashValue = (UCHAR)((CRC >> (32 - HASH_BITS)) & 0x3f);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -CalculateHashValue [0x%.8X]\r\n"), HashValue));

    return HashValue;
}

void ClearPromiscous()
{
    INSREG32BF(&gpFECReg->RCR, FEC_RCR_PROM, 0);
}

void SetPromiscous()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +SetPromiscous\r\n")));

    INSREG32BF(&gpFECReg->RCR,FEC_RCR_PROM, 1);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -SetPromiscous\r\n")));
}

void SetUnicast(PMP_ADAPTER Adapter)
{
    UCHAR HashValue = 0;

    DBGPRINT(MP_TRACE, ("UCast = %02x-%02x-%02x-%02x-%02x-%02x\n",
            Adapter->FecMacAddress[0],
            Adapter->FecMacAddress[1],
            Adapter->FecMacAddress[2],
            Adapter->FecMacAddress[3],
            Adapter->FecMacAddress[4],
            Adapter->FecMacAddress[5]));

    CLRREG32(&gpFECReg->IAUR, IAUR_CLEAR_ALL);
    CLRREG32(&gpFECReg->IALR, IALR_CLEAR_ALL);

    HashValue = CalculateHashValue( Adapter->FecMacAddress );
    if( HashValue > 31 )
        SETREG32(&gpFECReg->IAUR, 1 << (HashValue-32));
    else
        SETREG32(&gpFECReg->IALR, 1 << HashValue);
}

//------------------------------------------------------------------------------
//
// Function: AddMultiCast
//
// This function adds the Hash value to the Hash table(GAUR and GALR).
//
// Parameters:
//      pAddr
//          [in] pointer to a ethernet address.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void AddMultiCast( UCHAR *pAddr )
{
    UCHAR HashValue = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +AddMultiCast\r\n")));

    HashValue = CalculateHashValue( pAddr );

    if( HashValue > 31 )
        SETREG32(&gpFECReg->GAUR, 1 << (HashValue-32));
    else
        SETREG32(&gpFECReg->GALR, 1 << HashValue);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -AddMultiCast\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ClearAllMultiCast
//
// This function clears the Hash table(GAUR and GALR).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void ClearAllMultiCast()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +ClearAllMultiCast\r\n")));

    // clear all GAUR and GALR bits
    DBGPRINT(MP_TRACE, ("MC List Cleared \n"));

    CLRREG32(&gpFECReg->GAUR, GAUR_CLEAR_ALL);
    CLRREG32(&gpFECReg->GALR, GALR_CLEAR_ALL);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -ClearAllMultiCast\r\n")));
}
//-----------------------------------------------------------------------
NDIS_STATUS FECEnetInit(
    IN  PMP_ADAPTER     Adapter
    )
/*++
Routine Description:

    Initialize the adapter and set up everything

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_HARD_ERRORS

--*/
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    unsigned int i;

    DBGPRINT(MP_TRACE, ("--> FECEnetInit \n"));

    do
    {
        //Initialize the FEC Hardware
        // enable gpio and clock for FEC hardware
        BSPFECIomuxConfig( TRUE );
        BSPFECClockConfig( TRUE );

        // set up our link indication variable
        // it doesn't matter what this is right now because it will be
        // set correctly if link fails
        Adapter->MediaState = MediaConnectStateUnknown;
        Adapter->MediaDuplexState = MediaDuplexStateUnknown;
        Adapter->LinkSpeed = NDIS_LINK_SPEED_UNKNOWN;

        Adapter->CurrentPowerState = NdisDeviceStateD0;
        Adapter->NextPowerState    = NdisDeviceStateD0;

        NIC_CLEAR_RECV_READY(Adapter);  //set as recv not ready

        // issue a reset to the FEC hardware
        INSREG32BF(&gpFECReg->ECR, FEC_ECR_RESET, FEC_ECR_RESET_RESET);  //@@

        // wait for the hareware reset to complete
        while(EXTREG32BF(&gpFECReg->ECR, FEC_ECR_RESET));

        // set the receive and transmit BDs ring base to
        // hardware registers(ERDSR & ETDSR)
        OUTREG32(&gpFECReg->ERDSR, (ULONG) Adapter->HwRecvMemAllocPa.QuadPart);
        OUTREG32(&gpFECReg->ETDSR, (ULONG) Adapter->HwSendChipMemAllocPa.QuadPart);   //As per buffer allocation

        // set other hardware registers
        OUTREG32(&gpFECReg->EIR, FEC_EIR_CLEARALL_MASK);
        OUTREG32(&gpFECReg->IAUR, 0);
        OUTREG32(&gpFECReg->IALR, 0);
        OUTREG32(&gpFECReg->GAUR, 0);
        OUTREG32(&gpFECReg->GALR, 0);

        OUTREG32(&gpFECReg->EMRBR, PKT_MAXBLR_SIZE);

        // disable internal loopback
        INSREG32BF(&gpFECReg->RCR, FEC_RCR_LOOP, FEC_RCR_LOOP_DISABLE);

        // disable broadcast indicates by default
        INSREG32BF(&gpFECReg->RCR, FEC_RCR_BCREJ, FEC_RCR_BCREJ_ENABLE);

        // enable independent receive/transmit by default
        INSREG32BF(&gpFECReg->RCR, FEC_RCR_DRT, FEC_RCR_DRT_DISABLE);

        // disable flow control by default
        INSREG32BF(&gpFECReg->RCR, FEC_RCR_FCE, FEC_RCR_FCE_DISABLE);



        OUTREG32(&gpFECReg->RCR,
                    CSP_BITFVAL(FEC_RCR_MAXFL, PKT_MAXBUF_SIZE)|
                    CSP_BITFVAL(FEC_RCR_MIIMODE, FEC_RCR_MIIMODE_ENABLE));
        OUTREG32(&gpFECReg->TCR, 0);

        NIC_SET_RECV_READY(Adapter); //set as recieve ready

        // Set the station address for the FEC Adapter
        OUTREG32(&gpFECReg->PALR, Adapter->FecMacAddress[3] |
                          Adapter->FecMacAddress[2]<<8 |
                          Adapter->FecMacAddress[1]<<16 |
                          Adapter->FecMacAddress[0]<<24);

        OUTREG32(&gpFECReg->PAUR, Adapter->FecMacAddress[5]<<16 |
                          Adapter->FecMacAddress[4]<<24);

        SetUnicast(Adapter);

        // Enable RxF, TxF and MII interrupts
        OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_TXF, FEC_EIMR_TXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_LC, FEC_EIMR_LC_UNMASK)   |
                    CSP_BITFVAL(FEC_EIMR_UN, FEC_EIMR_UN_UNMASK)   |
                    CSP_BITFVAL(FEC_EIMR_RL, FEC_EIMR_RL_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_HBERR, FEC_EIMR_HBERR_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_BABT, FEC_EIMR_BABT_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_EBERR, FEC_EIMR_EBERR_UNMASK));


        // enable the FEC
        INSREG32BF(&gpFECReg->ECR, FEC_ECR_ETHEREN, FEC_ECR_ETHEREN_ENABLE);

        INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);

        for(i = 0; i < FEC_MII_COUNT-1; i++)
        {
            gMIICmds[i].MIINext = &gMIICmds[i+1];
        }
        gMIIFree = gMIICmds;

        // setup MII interface
        FECSetMII(Adapter);

        // Queue up command to detect the PHY and initialize the remainder of
        // the interface
        Adapter->MIIPhyIdDone = FALSE;
        Adapter->MIIPhyAddr = 0;
        Adapter->MIISeqDone = FALSE;
        Adapter->LinkStatus = FALSE;
        Adapter->SpeedMode = FALSE;
        Adapter->MIIPhy = NULL;
        FECQueueMII( Adapter, MII_READ_COMMAND(MII_REG_PHYIR1), FECGetPHYId );
    } while (FALSE);
    DBGPRINT(MP_TRACE, ("<-- FECEnetInit \n"));
    return Status;
}
//------------------------------------------------------------------------------
//
// Function: FECEnetReset
//
// This function is called to restart the FEC hardware. Linking status change
// or switching between half and full duplex mode will cause this function to
// be called.
//
// Parameters:
//      MiniportAdapterHandle
//          [in] Specifies the handle to the driver allocated context area in
//               which the driver maintains FEC adapter state, set up by
//               FECInitialize
//
//      DuplexMode
//          [in] TRUE for full duplex mode, FALSE for half duplex mode
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
void FECEnetReset(
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN BOOL DuplexMode
    )
{
    UINT i;
    PMP_ADAPTER Adapter= ((PMP_ADAPTER)(MiniportAdapterHandle));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnetReset\r\n")));
    DEBUGMSG(ZONE_INIT, (TEXT("FEC: +FECEnetReset Mode 0x%x\n"), DuplexMode));

    // Issues a hardware reset, we should wait for this
    INSREG32BF(&gpFECReg->ECR, FEC_ECR_RESET, FEC_ECR_RESET_RESET);
    while(EXTREG32BF(&gpFECReg->ECR, FEC_ECR_RESET));

    // Enable RxF, TxF and MII interrupts
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_TXF, FEC_EIMR_TXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_LC, FEC_EIMR_LC_UNMASK)   |
                    CSP_BITFVAL(FEC_EIMR_UN, FEC_EIMR_UN_UNMASK)   |
                    CSP_BITFVAL(FEC_EIMR_RL, FEC_EIMR_RL_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_HBERR, FEC_EIMR_HBERR_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_BABT, FEC_EIMR_BABT_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_EBERR, FEC_EIMR_EBERR_UNMASK));

    // Clear all pending interrupts
    OUTREG32(&gpFECReg->EIR, FEC_EIR_CLEARALL_MASK);
    OUTREG32(&gpFECReg->ERDSR,(ULONG) Adapter->HwRecvMemAllocPa.QuadPart);
    OUTREG32(&gpFECReg->ETDSR, (ULONG) Adapter->HwSendChipMemAllocPa.QuadPart);   //As per buffer allocation

    // Set the station address for the FEC Adapter
    OUTREG32(&gpFECReg->PALR, Adapter->FecMacAddress[3] |
                      Adapter->FecMacAddress[2]<<8 |
                      Adapter->FecMacAddress[1]<<16 |
                      Adapter->FecMacAddress[0]<<24);

    OUTREG32(&gpFECReg->PAUR, Adapter->FecMacAddress[5]<<16 |
                      Adapter->FecMacAddress[4]<<24);

    // Reset all multicast hashtable and individual hashtable
    // NIDS library will restore this by calling
    // FECSetInformation() function
    OUTREG32(&gpFECReg->IAUR, 0);
    OUTREG32(&gpFECReg->IALR, 0);
    OUTREG32(&gpFECReg->GAUR, 0);
    OUTREG32(&gpFECReg->GALR, 0);

    SetUnicast(Adapter);

    // Set maximum receive buffer size, the size must be aligned with
    // 16 bytes
    OUTREG32(&gpFECReg->EMRBR, PKT_MAXBLR_SIZE);

    // Enable MII mode and set the Duplex mode
    if(DuplexMode)
    {
        // MII mode enable and FD(Full Duplex) enable
        OUTREG32(&gpFECReg->RCR,
                    CSP_BITFVAL(FEC_RCR_MAXFL, PKT_MAXBUF_SIZE)|
                    CSP_BITFVAL(FEC_RCR_MIIMODE, FEC_RCR_MIIMODE_ENABLE)|
                    CSP_BITFVAL(FEC_RCR_DRT, FEC_RCR_DRT_DISABLE));

        OUTREG32(&gpFECReg->TCR,
                    CSP_BITFVAL(FEC_TCR_FDEN, FEC_TCR_FDEN_ENABLE));
        Adapter->usDuplexMode = 2;  //Full Duplex
    }
    else
    {
        // MII mode enable and FD disable
        OUTREG32(&gpFECReg->RCR,
                    CSP_BITFVAL(FEC_RCR_MAXFL, PKT_MAXBUF_SIZE)|
                    CSP_BITFVAL(FEC_RCR_MIIMODE, FEC_RCR_MIIMODE_ENABLE)|
                    CSP_BITFVAL(FEC_RCR_DRT, FEC_RCR_DRT_ENABLE));

        OUTREG32(&gpFECReg->TCR, 0);
        Adapter->usDuplexMode = 1;  //Half duplex
    }
    // Set MII speed
    OUTREG32(&gpFECReg->MSCR,
                     CSP_BITFVAL(FEC_MSCR_MIISPEED, Adapter->MIIPhySpeed));


    // Reset the MII command list
    gMIIHead = NULL;
    gMIIFree = NULL;
    gMIITail = NULL;

    for(i = 0; i < FEC_MII_COUNT-1; i++)
    {
        gMIICmds[i].MIINext = &gMIICmds[i+1];
    }

    gMIIFree = gMIICmds;
    // Finally, enable the fec and enable receive/transmit processing
    INSREG32BF(&gpFECReg->ECR, FEC_ECR_ETHEREN, FEC_ECR_ETHEREN_ENABLE);

    INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnetReset\r\n")));
    return;
}
