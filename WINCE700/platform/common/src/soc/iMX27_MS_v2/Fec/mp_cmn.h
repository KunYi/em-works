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
/*++

Module Name:
    mp_cmn.h

Abstract:
    Common definitions for the miniport and kd extension dll

Revision History:

Notes:

--*/

#ifndef _MP_CMN_H
#define _MP_CMN_H

// MP_TCB flags
#define fMP_TCB_IN_USE                         0x00000001
#define fMP_TCB_USE_LOCAL_BUF                  0x00000002
#define fMP_TCB_MULTICAST                      0x00000004  // a hardware workaround using multicast

#define fMP_TCB_WAIT_FOR_MAPPING               0x10000000
#define fMP_TCB_PREV_WAIT_FOR_MAPPING          0x20000000
#define fMP_TCB_FLAGS_MAPPING                  0x30000000

// MP_RFD flags                           
#define fMP_RFD_RECV_PEND                      0x00000001
#define fMP_RFD_ALLOC_PEND                     0x00000002
#define fMP_RFD_RECV_READY                     0x00000004
#define fMP_RFD_RESOURCES                      0x00000008
#define fMP_RFD_ALLOC_FROM_MEM_CHUNK           0x00000010

// MP_ADAPTER flags               
#define fMP_ADAPTER_SCATTER_GATHER             0x00000001
#define fMP_ADAPTER_RECV_LOOKASIDE             0x00000004
#define fMP_ADAPTER_INTERRUPT_IN_USE           0x00000008
#define fMP_ADAPTER_SECONDARY                  0x00000010

#define fMP_ADAPTER_NON_RECOVER_ERROR          0x00800000

#define fMP_ADAPTER_RESET_IN_PROGRESS          0x01000000
#define fMP_ADAPTER_NO_CABLE                   0x02000000 
#define fMP_ADAPTER_HARDWARE_ERROR             0x04000000
#define fMP_ADAPTER_REMOVE_IN_PROGRESS         0x08000000
#define fMP_ADAPTER_HALT_IN_PROGRESS           0x10000000

#define fMP_ADAPTER_LINK_DETECTION             0x20000000

#define fMP_RECV_READY                         0x40000000
#define fMP_ADAPTER_INITIALIZE_IN_PROGRESS     0x80000000

                                 
#define fMP_ADAPTER_FAIL_SEND_MASK             0x1ff00000                
#define fMP_ADAPTER_NOT_READY_MASK             0x3ff00000    


#endif  // _MP_CMN_H


