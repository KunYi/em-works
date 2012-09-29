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
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bspbootshell.c
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  bootshell.h
//

#ifndef _BOOT_SHELL_H
#define _BOOT_SHELL_H
#define BOOT_MONITOR_PROMPT     "command -- "


VOID BootShell();
VOID OutputHelpInformation();
VOID DebugGetLine(UCHAR *Linebuffer);
BOOL SetBit(UCHAR * AddrSet,UCHAR * BitSet,UCHAR * ValueSet);
BOOL ShowReg(UCHAR * AddrSet);
BOOL SetReg(UCHAR * AddrSet,UCHAR * ValueSet);
UCHAR OEMWaitAndReadDebugByte();
UINT32 HexString2Hex(UCHAR * str);
UINT32 OffsetString2Decimal(UCHAR str1,UCHAR str2);

#endif
