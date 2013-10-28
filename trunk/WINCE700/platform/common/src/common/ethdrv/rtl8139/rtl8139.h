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
// Prototypes for RTL8139.LIB functions
BOOL   RTL8139InitDMABuffer(DWORD dwStartAddress, DWORD dwSize);
BOOL   RTL8139Init( UINT8 *pbBaseAddress, UINT32 dwMultiplier, UINT16 MacAddr[3]);
void   RTL8139EnableInts();
void   RTL8139DisableInts();
DWORD  RTL8139GetPendingInts(void);
UINT16 RTL8139GetFrame( BYTE *pbData, UINT16 *pwLength );
UINT16 RTL8139SendFrame( BYTE *pbData, DWORD dwLength );
BOOL   RTL8139ReadEEPROM( UINT16 EEPROMAddress, UINT16 *pwVal);
BOOL   RTL8139WriteEEPROM( UINT16 EEPROMAddress, UINT16 Data );
DWORD  RTL8139SetOptions(DWORD dwOptions);

