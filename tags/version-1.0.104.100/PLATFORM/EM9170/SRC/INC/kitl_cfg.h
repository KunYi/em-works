//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  kitl_cfg.h
//
//  Configuration file for KITL
//
//-----------------------------------------------------------------------------
#ifndef __KITL_CFG_H
#define __KITL_CFG_H


//------------------------------------------------------------------------------
// Defines
#define OAL_KITL_ETH_INDEX          0
#define OAL_KITL_SERIAL_INDEX       1


//------------------------------------------------------------------------------
// Types

// Define a data structure that will be used to pass data between the OAL
// and KITL components.
typedef struct
{
    PCSP_CRM_REGS   g_pCRM;   // These pointers are required for serial port
//    PCSP_PBC_REGS   g_pPBC;   // Needed to access 3DS debug board resources //JJH not need anymore because LAN is accessed through CSPI
    PCSP_IOMUX_REGS g_pIOMUX; // OAL may need to access the serial ports
} _OALKITLSharedDataStruct;


//------------------------------------------------------------------------------
// Functions
extern BOOL LAN911xInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3]);
extern UINT16 LAN911xSendFrame(UINT8 *pBuffer, UINT32 length);
extern UINT16 LAN911xGetFrame(UINT8 *pBuffer, UINT16 *pLength);
extern VOID LAN911xEnableInts();
extern VOID LAN911xDisableInts();
extern VOID LAN911xCurrentPacketFilter(UINT32 filter);
extern BOOL LAN911xMulticastList(UINT8 *pAddresses, UINT32 count);

extern BOOL FECInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3]);
extern BOOL FECInitDMABuffer(UINT32 address, UINT32 size);
extern UINT16 FECSendFrame(UINT8 *pData, UINT32 length);
extern UINT16 FECGetFrame(UINT8 *pData, UINT16 *pLength);
extern VOID FECEnableInts();
extern VOID FECDisableInts();
extern VOID FECCurrentPacketFilter(UINT32 filter);
extern BOOL FECMulticastList(UINT8 *pAddresses, UINT32 count);

extern BOOL SerialInit(KITL_SERIAL_INFO *pInfo);
extern VOID SerialDeinit();
extern UINT16 SerialRecv(UINT8 *pData, UINT16 size);
extern UINT16 SerialSend(UINT8 *pData, UINT16 size);
extern VOID SerialSendComplete(UINT16 size);
extern VOID SerialEnableInts();
extern VOID SerialDisableInts();
extern VOID SerialFlowControl (BOOL fOn);

#endif // __KITL_CFG_H
