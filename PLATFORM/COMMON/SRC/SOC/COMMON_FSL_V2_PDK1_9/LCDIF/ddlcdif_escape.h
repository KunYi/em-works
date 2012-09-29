//-----------------------------------------------------------------------------
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
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  ddlcdif_escape.h
//
//  Add functions in DrvEscape
//
//------------------------------------------------------------------------------
#ifndef _DRIVERS_DISPLAY_DDLCDIF_DDLCDIFESCAPE_H
#define _DRIVERS_DISPLAY_DDLCDIF_DDLCDIFESCAPE_H


// structure for VirtualSetAttributesEx
typedef struct LcdifVMSetAttributeExDataStruct{
  LPVOID lpvAddress;    // starting address of virtual memory
  DWORD cbSize;         // size of virtual memory
}LcdifVMSetAttributeExData, *pLcdifVMSetAttributeExData;

BOOL LcdifVMSetAttributeEx(pLcdifVMSetAttributeExData pData);

#endif // _DRIVERS_DISPLAY_DDLCDIF_DDLCDIFESCAPE_H
