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
/*

  Copyright(c) 1998,1999 Renesas Technology Corp.

	Module Name:

		SER_PDD.h

	Revision History:

		26th April 1999		Released

*/
#ifndef __SER_PDD_H__
#define __SER_PDD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @doc HWINTERNAL
 * @struct SER_INFO | Private structure.
 */
    
    typedef struct __SER_INFO
    {
         // Put lib struct first so we can easily cast pointers
        SER16550_INFO ser16550;
        
         // now hardware specific goodies
        PUCHAR		pBaseAddress;		// @field Start of serial registers
        DWORD       dwMemBase;       // @field Base Address - unmapped
        DWORD       dwMemLen;        // @field Length
        DWORD       dwSysIntr;       // @field System Interrupt number for this peripheral

        UINT8       cOpenCount;     // @field Count of concurrent opens
        COMMPROP	CommProp;	    // @field Pointer to CommProp structure.
        PVOID		pMddHead;		// @field First arg to mdd callbacks.
        BOOL		fIRMode;		// @field Boolean, are we running in IR mode?
        PHWOBJ      pHWObj;         // @field Pointer to PDDs HWObj structure
    } SER_INFO, *PSER_INFO;


#ifdef __cplusplus
}
#endif


#endif __SER_PDD_H__
