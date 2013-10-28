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

#pragma warning(disable:4214)   // bit field types other than int

#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4127)   // conditional expression is constant
#pragma warning(disable:4054)   // cast of function pointer to PVOID
#pragma warning(disable:4206)   // translation unit is empty

#include <ndis.h>
#include <ntddndis.h>  // defines OID's
#include <Linklist.h>
#include <windows.h>
#include <nkintr.h>
#include <Winbase.h>  
#include <dbgapi.h>
#include "csp.h"
#ifdef UNDER_CE
#include <cebuscfg.h>
#endif  //  UNDER_CE

#include "fec.h"

#include "mp_dbg.h"
#include "mp_cmn.h"
#include "mp_def.h"
#include "mp.h"
#include "mp_nic.h"



