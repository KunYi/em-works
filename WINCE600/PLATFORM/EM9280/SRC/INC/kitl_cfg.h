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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  kitl_cfg.h
//
//
//
//------------------------------------------------------------------------------

#ifndef __KITL_CFG_H
#define __KITL_CFG_H


//------------------------------------------------------------------------------
// Defines
#define OAL_KITL_ETH_INDEX          0
#define OAL_KITL_SERIAL_INDEX       1

extern BOOL ETHInitDMABuffer(UINT32 address, UINT32 size);
extern BOOL ENETInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3]);
extern BOOL ENETInitDMABuffer(UINT32 address, UINT32 size);
extern UINT16 ENETSendFrame(UINT8 *pData, UINT32 length);
extern UINT16 ENETGetFrame(UINT8 *pData, UINT16 *pLength);
extern VOID ENETEnableInts();
extern VOID ENETDisableInts();
extern VOID ENETPowerOff();
extern VOID ENETPowerOn();
extern VOID ENETCurrentPacketFilter(UINT32 filter);
extern BOOL ENETMulticastList(UINT8 *pAddresses, UINT32 count);
//------------------------------------------------------------------------------
// Types

// Define a data structure that will be used to pass data between the OAL
// and KITL components.

#endif // __KITL_CFG_H
