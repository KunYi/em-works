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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        USBMSFNDBG.H

Abstract:

        USB Mass Storage Function Driver Debug Data.
        
--*/

//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2010 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT.
//
//------------------------------------------------------------------------------

#ifndef __USBMSFNDBG_H__
#define __USBMSFNDBG_H__


#include <usbfntypes.h>


#ifndef dim
#define dim(x) (sizeof(x)/sizeof((x)[0]))
#endif


#define UMS_REG_INTERFACE_SUBCLASS_VAL       (_T("InterfaceSubClass"))
#define UMS_REG_INTERFACE_PROTOCOL_VAL       (_T("InterfaceProtocol"))
#define UMS_REG_VENDOR_VAL                   (_T("Vendor"))
#define UMS_REG_PRODUCT_VAL                  (_T("Product"))
#define UMS_REG_BUFFER_VAL                   (_T("InitialDataBufferSize"))
#define UMS_REG_TRANSFER_THREAD_PRIORITY_VAL (_T("TransferThreadPriority"))

#define PHDC_INTERFACE_CLASS                    0x0f
#define PHDC_INTERFACE_SUBCLASS                 0x00
#define PHDC_INTERFACE_PROTOCOL                 0x00

#ifdef DEBUG
#define ZONE_ERROR              DEBUGZONE(0)
#define ZONE_WARNING            DEBUGZONE(1)
#define ZONE_INIT               DEBUGZONE(2)
#define ZONE_FUNCTION           DEBUGZONE(3)
#define ZONE_COMMENT            DEBUGZONE(4)
extern DBGPARAM dpCurSettings;
#define FUNCTION_ENTER_MSG() DEBUGMSG(ZONE_FUNCTION, (_T("%s ++\r\n"), pszFname))
#define FUNCTION_LEAVE_MSG() DEBUGMSG(ZONE_FUNCTION, (_T("%s --\r\n"), pszFname))
#else
#define FUNCTION_ENTER_MSG()
#define FUNCTION_LEAVE_MSG()
#endif // DEBUG

#ifndef SHIP_BUILD
#define STR_MODULE _T("UsbMsFn!")
#define SETFNAME(name) LPCTSTR pszFname;\
                       pszFname = STR_MODULE name _T(":")
#else
#define SETFNAME(name)
#endif // SHIP_BUILD


#endif // __USBMSFNDBG_H__
