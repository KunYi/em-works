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
//------------------------------------------------------------------------------
//
//  File:  ioctl_tab.h
//
//  Configuration file for the OAL IOCTL component.
//
//  This file is included by the platform's ioctl.c file and defines the 
//  global IOCTL table, g_oalIoCtlTable[]. Therefore, this file may ONLY
//  define OAL_IOCTL_HANDLER entries. 
//
// IOCTL CODE,                          Flags   Handler Function
//------------------------------------------------------------------------------

//IOCTLs supporting XSCALE XDB BROWSER
{IOCTL_XSDBG_GETVERSION,               0,  XSCBwrIoControl},
{IOCTL_XSDBG_READCOPROCESSOR,          0,  XSCBwrIoControl},
{IOCTL_XSDBG_WRITECOPROCESSOR,         0,  XSCBwrIoControl},
{IOCTL_XSDBG_SETTASK,                  0,  XSCBwrIoControl},
{IOCTL_XSDBG_QUERYCTXREGS,             0,  OALQueryContextRegistersIoctl},
{IOCTL_XSDBG_READCTXREGS,              0,  OALReadContextRegisterIoctl},
{IOCTL_XSDBG_WRITECTXREGS,             0,  OALWriteContextRegisterIoctl},
{IOCTL_XSDBG_TRACECONTROL,             0,  XSCBwrIoControl},
{IOCTL_XSDBG_READTRACE,                0,  XSCBwrIoControl},
{IOCTL_XSDBG_READKMEM,                 0,  XSCBwrIoControl},
{IOCTL_XSDBG_GETTRACECONFIG,           0,  XSCBwrIoControl},

//------------------------------------------------------------------------------
