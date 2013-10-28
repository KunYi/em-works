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
#ifndef __EBOOT_H
#define __EBOOT_H

#include <windows.h>

/* @func BOOL | OEMEthGetFrame | Receive data from the debug Ethernet adapter
 *
 * @comm  This function is called by the EDBG subsystem to check to see if a frame has
 *        been received, and if so, copy it to the specified buffer.   NOTE: To optimize
 *        performance, it is recommended that broadcast frames be discarded as early as
 *        possible in this routine.  The only broadcast frames required by the EDBG
 *        subsystem are ARP messages, so processing can be reduced by filtering out all
 *        other broadcast frames.
 * @rdesc Return TRUE if receive data is present, FALSE if not.
 */

BOOL
OEMEthGetFrame(
    BYTE *pData,        // @parm [OUT] - Buffer to receive frame data
    UINT16 *pwLength    // @parm [IN]  - Set to length of buffer
                        //       [OUT] - Set to number of bytes received
    );

/* @func  BOOL | OEMEthSendFrame | Send data over debug Ethernet adapter
 * @rdesc Return TRUE if successful, FALSE if an error occurred and the data could not be sent.
 * @comm  This function is called by the EDBG subsystem to send an Ethernet frame.  The frame
 *        is completely formatted (including Ethernet header) before this function is called.
*/
BOOL
OEMEthSendFrame(
    BYTE *pData,        // @parm [IN] - Buffer containing frame data
    DWORD dwLength      // @parm [IN] - Number of bytes in frame
    );

#endif
