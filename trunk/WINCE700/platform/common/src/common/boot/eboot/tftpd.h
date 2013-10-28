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
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

  tftpd.h

Abstract:

  This contains the tftpd.c specific data structure declarations.

Functions:


Notes:


Revision History:

--*/
#ifndef TFTPD_H
#define TFTPD_H

#define MAX_TFTP_FILENAME 32+1
#define MAX_TFTP_SERVER_PROCS 5





typedef struct TFtpdServerRegistryEntryTag {

    UINT16 fInUse;                          // True if this entry is being used
    char szFileName[MAX_TFTP_FILENAME];     // The file name associated with this server process
    TFtpdCallBackFunc pfCallBack;

} TFtpdServerRegistryEntry;


extern DWORD dwLaunchAddr;
extern BOOL  bDoLaunch;

void InitTFtpd( void );
UINT16 TFtpdServerRegister( char *pszFileName, TFtpdCallBackFunc pfCallBack );
void TFtpdServerUnRegister( char *pszFileName );
UINT16 GenerateSrcPort( void );
UINT16 TFtpdFormNewLink( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer, UINT16 iLinkSlot, UINT16 *pwMsg, UINT16 wMsgLength );
void TFtpdCallBack( UINT16 iLinkSlot, TFtpdCallBackOps Operation );


#endif
