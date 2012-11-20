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
/*++
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Module Name:    

   Abstract:

   Functions:


   Notes:


   --*/
//------------------------------------------------------------------------------
//
//  Copyright Emtronix, Inc. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
// File:        ddlcdif_shutdown.cpp
//
// Implementation of DDLcdif surface allocation/manipulation/free routines.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include "precomp.h"
#include "dispperf.h"
#pragma warning(pop)

//------------------------------------------------------------------------------
// Defines
#define		PALETTE_SIZE				256
// RGB colour setting
#define		COLOR_R						16
#define		COLOR_G						8
#define		COLOR_B						0
#define     SZREGKEY					(TEXT("System\\GDI\\Drivers"))
#define		SZCONTRASTLEVEL  			(TEXT("ContrastLevel"))
#define		SZSHUTDOWNIMAGE				(TEXT("ShutDownImage"))
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern DWORD BSPGetWidth(DWORD modeNumber);
extern DWORD BSPGetHeight(DWORD modeNumber);

PALETTEENTRY		StdPalette[PALETTE_SIZE];
BYTE				pBuffer[400*1024];
// Show Power down image
BOOL ShowSplash( int modeNumber, WORD *m_pSplashSurface)
{
	PBITMAPFILEHEADER	pBmpFileHead;
	PBITMAPINFOHEADER	pBmpInfoHead;
	PBYTE				pBitmap;
	DWORD				dwStatus,dwSize, dwType;
	TCHAR				szImageName[MAX_PATH], szDefaultImage[MAX_PATH];
	HKEY				hKey;
	HANDLE				hFile;
	RGBQUAD				*pPalette;
	DWORD				index;
	DWORD				x, y;
	COLORREF			dwPoint;
	//PBYTE				pBuffer = NULL;
	BOOL				rc = FALSE;
	WORD				*pwTmp, wColor;

	switch( BSPGetWidth(modeNumber) )
	{
	case 320:
		_tcscpy(szDefaultImage, _T("Windows\\ShutDownSplash320240.bmp"));
		break;

	case 480:
		_tcscpy(szDefaultImage, _T("Windows\\ShutDownSplash480272.bmp"));
		break;

	default:
		return FALSE;
	}

	_tcscpy(szImageName, szDefaultImage);
	dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, SZREGKEY,0,0,&hKey);
	if (dwStatus == ERROR_SUCCESS)
	{
		dwSize = sizeof(szImageName);
		dwStatus = RegQueryValueEx(hKey, SZSHUTDOWNIMAGE, NULL, 
			&dwType, (LPBYTE)szImageName, &dwSize);
		RegCloseKey(hKey);
		if (dwStatus != ERROR_SUCCESS )
		{
			_tcscpy(szImageName, szDefaultImage);
		}
	}

	hFile = CreateFile( szImageName ,GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, 0 );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		_tcscpy(szImageName, szDefaultImage);
		hFile = CreateFile( szImageName ,GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, 0 );
	}

	if( hFile != INVALID_HANDLE_VALUE )
	{
		//pBuffer	= (PBYTE)malloc(400*1024);	
		if( pBuffer == NULL )
		{
			RETAILMSG(1, (TEXT("malloc error\r\n")));
			goto error_showsplash;
		}
		BOOL bResult = ReadFile( hFile, pBuffer, 400*1024, &dwSize, NULL );
		if( bResult )
		{
			pBmpFileHead = (PBITMAPFILEHEADER)pBuffer;
			pBmpInfoHead = (PBITMAPINFOHEADER)&pBuffer[sizeof(BITMAPFILEHEADER)];
			pBitmap = pBuffer + pBmpFileHead->bfOffBits;
			// If BMP signature not found generate error and return
			if (pBmpFileHead->bfType != 0x4D42 || 
				pBmpInfoHead->biWidth != (LONG)BSPGetWidth(modeNumber) ||
				pBmpInfoHead->biHeight != (LONG)BSPGetHeight(modeNumber) ) 
			{
				RETAILMSG( 1, (TEXT("DDLcdif::ShowSplash: ERROR Not a bmp file. pBmpInfoHead->biWidth=%d, pBmpInfoHead->biHeight=%d\r\n"),
					pBmpInfoHead->biWidth, pBmpInfoHead->biHeight));
				goto error_showsplash;
			}
			
			// get bmp palette data if needed
			if ( pBmpInfoHead->biBitCount  == 8 || pBmpInfoHead->biBitCount  == 4 )
			{
				pPalette = (RGBQUAD *)((LPSTR)pBmpInfoHead + (WORD)(pBmpInfoHead->biSize));

				/* read the palette information */
				for(index=0; index < 256; index++)
				{
					StdPalette[index].peRed   = pPalette[index].rgbRed;
					StdPalette[index].peGreen = pPalette[index].rgbGreen;
					StdPalette[index].peBlue  = pPalette[index].rgbBlue;
					StdPalette[index].peFlags = pPalette[index].rgbReserved;
				}
			}
			else
			{
				RETAILMSG(1, (TEXT("DDLcdif::ShowSplash:ERROR pBmIH->biBitCount=0x%x Expected: 0x0008\r\n"),
					pBmpInfoHead->biBitCount));
				goto error_showsplash;
			}

			y = pBmpInfoHead->biHeight;
			while(y--)
			{
				for(x = 0; x < (DWORD)pBmpInfoHead->biWidth; x++) 
				{
					// Clear color value
					dwPoint = 0;
					// Read 8 BPP BMP image color value.
					// Find out RGB value for that color from
					// 256 color standard pallete

					dwPoint = (StdPalette[*pBitmap].peRed    << COLOR_R )  |
							  (StdPalette[*pBitmap].peGreen  << COLOR_G )  |
						      (StdPalette[*pBitmap].peBlue   << COLOR_B );

					// Point to the next pixel color
					pBitmap++;

					pwTmp = (WORD*)m_pSplashSurface + y * pBmpInfoHead->biWidth + x;
					wColor = (WORD)(((dwPoint & 0xF80000) >> 8) | ((dwPoint & 0xFC00) >> 5) | ((dwPoint & 0xF8) >> 3));
					*pwTmp = wColor;
	
				}
			}
		}	
	}
	else
	{
		RETAILMSG(1, (TEXT("DDLcdif::ShowSplash: Error opening shotdown splash.\r\n")));
		goto error_showsplash;
	}
	rc = TRUE;

error_showsplash:
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );	
	}
// 	if( pBuffer )
// 	{
// 		free( pBuffer );
// 		pBuffer = NULL;
// 	}
	return rc;
}

BOOL DDLcdif::AllocSplashSurfaces(DWORD modeNumber)
{
	BOOL rc = FALSE;
//	DWORD phys,virt,offset;

	// LCD surface
	//
	if(FAILED(AllocSurface((DDGPESurf **) &m_pSplashSurface,
		BSPGetWidth(modeNumber),
		BSPGetHeight(modeNumber),
		m_pMode->format,
		m_pModeEx->ePixelFormat,
		GPE_REQUIRE_VIDEO_MEMORY)))
	{
		ERRORMSG(1,(L"DDLcdif:: alloc primary surface failed\r\n"));
		ASSERT(0);
		goto _AllocSurfdone;
	}

//	virt = (DWORD) m_pPrimarySurface->Buffer();
//	offset = virt - (DWORD) VirtualMemAddr;
//	phys =  (DWORD) PhysicalMemAddr + offset;
//	BSPSetDisplayBuffer((PVOID)phys, (PVOID)virt);
//	m_pSplashSurface = (DDGPESurf *)m_pPrimarySurface;

	rc = TRUE;  // success

_AllocSurfdone:
	return rc;
}

DWORD WINAPI ShutdownSplash(LPVOID lpParameter)
{
	DDLcdif* ddgpe = (DDLcdif*) lpParameter;
	
	if( !ddgpe->AllocSplashSurfaces( DISPLAY_MODE_DEVICE ))
		return 1;

	if( !ShowSplash( DISPLAY_MODE_DEVICE, (WORD *)ddgpe->m_pSplashSurface->Buffer( ) ))
		return 1;

	HANDLE hSysShutDownEvent = CreateEvent(NULL, FALSE, FALSE, 
		L"PowerManager/SysShutDown_Active");

	WaitForSingleObject( hSysShutDownEvent, INFINITE );

	ddgpe->SetVisibleSurface( ddgpe->m_pSplashSurface );
	CloseHandle( hSysShutDownEvent );
	RETAILMSG(1, (TEXT("Shotdown splash...\r\n")));

	return 0;
}
