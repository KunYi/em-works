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

#include "precomp.h"
#include "csp.h"



#define PALETTE_SIZE              	2
#define PALETTE_4BPP_SIZE          16

RGBQUAD _rgb1bpp[PALETTE_SIZE] =
{
	{ 0x00, 0x00, 0x00, 0 },    /*   0 */	/* Black        */   	  
	{ 0xff, 0xff, 0xff, 0 }     /* 255 */	/* White        */
};

// Greyscalebpp palette. 
RGBQUAD _rgb4bpp[PALETTE_4BPP_SIZE] =
{
	{ 0x00, 0x00, 0x00, 0 },   \
	{ 0x10, 0x10, 0x10, 0 },   \
	{ 0x20, 0x20, 0x20, 0 },   \
	{ 0x30, 0x30, 0x30, 0 },   \
	{ 0x40, 0x40, 0x40, 0 },   \
	{ 0x50, 0x50, 0x50, 0 },   \
	{ 0x60, 0x60, 0x60, 0 },   \
	{ 0x70, 0x70, 0x70, 0 },   \
	{ 0x80, 0x80, 0x80, 0 },   \
	{ 0x90, 0x90, 0x90, 0 },   \
	{ 0xa0, 0xa0, 0xa0, 0 },   \
	{ 0xb0, 0xb0, 0xb0, 0 },   \
	{ 0xc0, 0xc0, 0xc0, 0 },   \
	{ 0xd0, 0xd0, 0xd0, 0 },   \
	{ 0xe0, 0xe0, 0xe0, 0 },   \
	{ 0xFF, 0xFF, 0xFF, 0 }    \
};

INSTANTIATE_GPE_ZONES(0x3,"MGDI Driver","unused1","unused2")	 /* Start with Errors, warnings, and temporary messages */

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern void BSPBacklightEnable(BOOL Enable);
extern void BSPSetDisplayController();
extern void BSPSetDisplayBuffer(ULONG PhysBase, PVOID pVirtBase);
extern BOOL BSPInitLCD( );
extern DWORD BSPGetIRQ( );
extern void BSPGetModeInfo(GPEMode* pMode, int modeNumber);
extern DWORD BSPGetVideoMemorySize();
extern void BSPFrameBufferUpdate( PVOID pSurface );
extern BOOL BSPSetContrast( DWORD dwContrastLevel );
extern BOOL BSPGetContrast( DWORD* dwContrastLevel, DWORD dwFlag );
extern void BSPDisplayPowerHandler(BOOL bOn);

BOOL bSuspended = FALSE;
BYTE g_Buffer[200*1024];

BOOL APIENTRY GPEEnableDriver(          // This gets around problems exporting from .lib
	ULONG          iEngineVersion,
	ULONG          cj,
	DRVENABLEDATA *pded,
	PENGCALLBACKS  pEngCallbacks);
BOOL APIENTRY DrvEnableDriver(
	ULONG          iEngineVersion,
	ULONG          cj,
	DRVENABLEDATA *pded,
	PENGCALLBACKS  pEngCallbacks)
{
	return GPEEnableDriver( iEngineVersion, cj, pded, pEngCallbacks );
}

DWORD DotLcdcIntr( Dot_lcd* pClass )
{
	pClass->IntrProc();

	// We shouldn't come to here
	return 0;
}

static GPE *pGPE = (GPE *)NULL;
// Main entry point for a GPE-compliant driver
GPE *GetGPE()
{
	if( !pGPE )
		pGPE = new Dot_lcd();
	return pGPE;
}

//------------------------------------------------------------------------------
//
// Function: PowerHandler
//
// Power handler function for a GPE-compliant driver.
// We do not thing here since DrvEscape will handle for it.
//
// Parameters:
//      bOff.
//          [in] Power status
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID
Dot_lcd::PowerHandler(
					  BOOL bOff
					  )
{
	// turn off the display if it is not already turned off
	// (turning on is controlled by DDLcdif::SetPmPowerState)
	if (bOff && !bSuspended) {
		DEBUGMSG(1, (TEXT("DDLcdif::PowerHandler: TurnOff Display\r\n")));
		bSuspended = TRUE;
	}
	else
	{
		DEBUGMSG(1, (TEXT("DDLcdif::PowerHandler: TurnOn Display\r\n")));
		bSuspended = FALSE;    
	}
}

Dot_lcd::Dot_lcd()
{
	
	RETAILMSG(1, (L"-->LCDIF Dot_lcd\r\n"));
	BSPSetDisplayController();

	BSPGetModeInfo( &m_ModeInfo,  m_nModeNumber );

	m_pMode = &m_ModeInfo;

	m_pPrimarySurface = new GPESurf( m_ModeInfo.width, m_ModeInfo.height, m_ModeInfo.format );
	if (!m_pPrimarySurface)
	{
		RETAILMSG(1, (TEXT("Dot_lcd::Dot_lcd: Error allocating GPESurf.\r\n")));
		return;
	}

	m_pBackUpSurface = new GPESurf( m_ModeInfo.width, m_ModeInfo.height, m_ModeInfo.format );
	if (!m_pBackUpSurface)
	{
		RETAILMSG(1, (TEXT("Dot_lcd::Dot_lcd: Error allocating GPESurf.\r\n")));
		return;
	}

	AllocPhysicalMemory();

	BSPSetDisplayBuffer( m_VideoMemPhysicalAddress.LowPart, (PVOID)m_VideoMemVirtualAddress.LowPart);
	pCurrentlySurface = (PVOID)m_pPrimarySurface->Buffer();

	// Remain Splash
	PHYSICAL_ADDRESS phyAddr;
	PBYTE pvBootLoaderFramBuffer;
	phyAddr.QuadPart = IMAGE_WINCE_DISPLAY_RAM_PA_START+0x10000;

	pvBootLoaderFramBuffer = (PBYTE)MmMapIoSpace(phyAddr, 160*80, FALSE);
	DWORD y;
	PBYTE pFrameBuffer;
	pFrameBuffer = (BYTE *)m_pPrimarySurface->Buffer();
	y = 160;
	while( y-- )
	{
		memcpy( pFrameBuffer+y*80, pvBootLoaderFramBuffer, 80 );
		pvBootLoaderFramBuffer+=80;
	}
		
	// Get handle to System PowerDown Event
	m_hSysShutDownEvent = CreateEvent(NULL, FALSE, FALSE, 
		L"PowerManager/SysShutDown_Active");

	// Show Power down image
	BITMAPFILEHEADER*	pBmpFileHead;
	BITMAPINFOHEADER*	pBmpInfoHead;
	PBYTE				pBitmap;
	DWORD				dwStatus,dwSize, dwType;
	TCHAR				szImageName[MAX_PATH] = _T("Windows\\ShutDownSplash160160.bmp");
	HKEY  hKey;
	HANDLE hFile;
	
	dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, SZREGKEY,0,0,&hKey);
	if (dwStatus == ERROR_SUCCESS)
	{
		dwSize = sizeof(szImageName);
		dwStatus = RegQueryValueEx(hKey, SZSHUTDOWNIMAGE, NULL, 
			&dwType, (LPBYTE)szImageName, &dwSize);
		RegCloseKey(hKey);
		if (dwStatus != ERROR_SUCCESS )
		{
			_tcscpy(szImageName, _T("Windows\\ShutDownSplash160160.bmp"));
		}

	}

	hFile = CreateFile( szImageName ,GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, 0 );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		_tcscpy(szImageName, _T("Windows\\ShutDownSplash160160.bmp"));
		hFile = CreateFile( szImageName ,GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, 0 );
	}
	
	if( hFile != INVALID_HANDLE_VALUE )
	{
		BOOL bResult = ReadFile( hFile, g_Buffer, 200*1024, &dwSize, NULL );
		if( bResult )
		{
			pBmpFileHead = (BITMAPFILEHEADER*)g_Buffer;
			pBmpInfoHead = (BITMAPINFOHEADER*)&g_Buffer[sizeof(BITMAPFILEHEADER)];
			if((pBmpInfoHead->biWidth == 160) && (pBmpInfoHead->biHeight == 160) && (pBmpInfoHead->biBitCount == 4))
			{
				pFrameBuffer = (BYTE *)m_pBackUpSurface->Buffer();
				pBitmap = g_Buffer + pBmpFileHead->bfOffBits;
				y = (DWORD)(pBmpInfoHead->biHeight);
				while( y-- )
				{
					memcpy( pFrameBuffer+y*80, pBitmap, 80 );
					pBitmap+=80;
				}
			}
		}
		CloseHandle( hFile );
	}
	else
	{
		RETAILMSG(1, (TEXT("Dot_lcd::Dot_lcd: Error opening image.\r\n")));
	}

	
	InitIrq( );

	BSPInitLCD(  );

	// Notify the system that we are a power-manageable device.
	m_Dx = D0;
 	if(!AdvertisePowerInterface(g_hmodDisplayDll))
 	{
 		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("Dot_lcd: failed to enable power management\r\n")));
// 		// non fatal
// 		// ASSERT(0);
 	}

	BSPBacklightEnable(TRUE);
	RETAILMSG(1, (L"<--LCDIF Dot_lcd\r\n"));
}


//------------------------------------------------------------------------------
//
// Function: AllocPhysicalMemory
//
// Allocate contiguous memory for the display driver.
// NOTE: Should be called only once at startup.
//
// Updates class members:
//      FrameBufferSize
//      PhysicalMemAddr
//      VirtualMemAddr
//      FrameBufferHeap
//
// Parameters:
//      none
//
// Returns:
//      TRUE for success, FALSE if fail.
//
//------------------------------------------------------------------------------
BOOL Dot_lcd::AllocPhysicalMemory( )
{
	BOOL rc = FALSE;

	m_dwFrameBufferSize = BSPGetVideoMemorySize();

	memset(&m_VideoMemPhysicalAddress, 0, sizeof(PHYSICAL_ADDRESS));
	memset(&m_VideoMemVirtualAddress, 0, sizeof(PHYSICAL_ADDRESS));

	m_VideoMemVirtualAddress.LowPart = (ULONG)AllocPhysMem(m_dwFrameBufferSize,
		PAGE_READWRITE | PAGE_NOCACHE,
		0,
		0,
		&m_VideoMemPhysicalAddress.LowPart);

	if (!m_VideoMemVirtualAddress.LowPart)
	{
		RETAILMSG(1, (TEXT("HalAllocateCommonBuffer: memory allocation failed (error = 0x%x).\r\n"), GetLastError()));
		ASSERT(0);
		goto _AllocPhysMemdone;  
	}

	rc = TRUE;

_AllocPhysMemdone:
	if(rc)
	{
		DEBUGMSG(1, (TEXT("DISPLAY: memory allocation OK\r\n")));
		DEBUGMSG(1, (TEXT("    Virtual address : 0x%08x\r\n"), (DWORD) VirtualMemAddr));
		DEBUGMSG(1, (TEXT("    Physical address: 0x%08x\r\n"), (DWORD) PhysicalMemAddr));
		DEBUGMSG(1, (TEXT("    Size            : 0x%08x\r\n"), (DWORD) FrameBufferSize));
	}
	return rc;



}

BOOL Dot_lcd::InitIrq( )
{
	BOOL result = FALSE;

	// Initialze LCDC interrupt
	m_hSyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(NULL == m_hSyncEvent)
	{
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc InitHardware: failed to create the intr event, error[%d]!\r\n"), GetLastError()));
		goto _done;
	}

	// Get LCDC IRQ
	DWORD irq = BSPGetIRQ();
	// Request an associated systintr
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
		&m_dwlcdcSysintr, sizeof(DWORD), NULL)){
			DEBUGMSG(GPE_ZONE_ERROR, (_T("Failed to obtain sysintr value!\r\n")));
			return 0;
	}

	// Initialize interrupt
	if(!InterruptInitialize(m_dwlcdcSysintr, m_hSyncEvent, NULL, NULL))
	{
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc InitHardware: fail to initialize the interrupt, error[%d]!\r\n"), GetLastError()));
		goto _done;
	}

	// Create the IST (thread)
	m_bStopIntrProc = FALSE;
	m_hSyncThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DotLcdcIntr, this, 0, NULL);
	if(NULL == m_hSyncThread)
	{
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc InitHardware: fail to initiate the interrupt thread, error[%d]!\r\n"), GetLastError()));
		goto _done;
	}

	result = TRUE;

_done:
	return result;

}

SCODE Dot_lcd::SetMode( int modeId, HPALETTE *pPalette )
{
	if( modeId != 0 )
		return E_INVALIDARG;

	// Here, we use EngCreatePalette to create a palette that that MGDI will use as a
	// stock palette

	DEBUGMSG(1,(TEXT("SetMode - creating stock palette\r\n")));

 	if( pPalette )
 	{
 		*pPalette = EngCreatePalette
 							(
 								PAL_INDEXED,
 								PALETTE_4BPP_SIZE ,        //      4, 	// i.e. 2
 								(ULONG *)_rgb4bpp,
 								0,
 								0,
 								0
 							);
 //		DEBUGMSG(1,(TEXT("Created 1 Bpp palette, handle = 0x%08x\r\n"),*pPalette));
 	}

	DEBUGMSG(1,(TEXT("SetMode done\r\n")));

	return S_OK;				// Mode is inherently set
}

SCODE Dot_lcd::GetModeInfo
(
	GPEMode *pMode,
	int modeNo
)
{
	if( modeNo != 0 )
		return E_INVALIDARG;

	*pMode = m_ModeInfo;

	return S_OK;
}

int Dot_lcd::NumModes()
{
	return 1;
}

SCODE Dot_lcd::SetPointerShape(
	GPESurf *pMask,
	GPESurf *pColorSurf,
	int xHot,
	int yHot,
	int cx,
	int cy )
{
	return S_OK;
}

SCODE Dot_lcd::MovePointer(
	int x,
	int y )
{
	DEBUGMSG(GPE_ZONE_HW,(TEXT("Moving cursor to %d,%d\r\n"), x, y ));
// 	if( x == -1 )
// 	{
// 		DispDrvrMoveCursor( m_ModeInfo.width, m_ModeInfo.height );		// disable cursor
// 	}
// 	{
// 		DispDrvrMoveCursor( x, y );		// disable cursor
// 	}
	return S_OK;
}

void Dot_lcd::WaitForNotBusy()
{
	return;
}

int Dot_lcd::IsBusy()
{
	return 0;	// Never busy as there is no acceleration
}

void Dot_lcd::GetPhysicalVideoMemory
(
	unsigned long *pPhysicalMemoryBase,
	unsigned long *pVideoMemorySize
)
{
	// No DDHAL support in 2bpp wrapper
	*pPhysicalMemoryBase = (unsigned long)(void *)0;//(unsigned long)m_nLAWPhysical;
	*pVideoMemorySize =0;// DispDrvr_cdwStride * DispDrvr_cyScreen / 4;
}


SCODE Dot_lcd::AllocSurface(
	GPESurf **ppSurf,
	int width,
	int height,
	EGPEFormat format,
	int surfaceFlags )
{
	if( surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY )
		return E_OUTOFMEMORY;	// Can't allocate video-memory surfaces in the Wrap2bpp environment
	// Allocate from system memory
	DEBUGMSG(GPE_ZONE_CREATE,(TEXT("Creating a GPESurf in system memory. EGPEFormat = %d\r\n"), (int)format ));
	*ppSurf = new GPESurf( width, height, format );
	if( *ppSurf )
	{
		// check we allocated bits succesfully
		if( !((*ppSurf)->Buffer()) )
			delete *ppSurf;	// and then return E_OUTOFMEMORY
		else
			return S_OK;
	}
	return E_OUTOFMEMORY;
}

SCODE Dot_lcd::WrappedEmulatedLine( GPELineParms *pParms )
{
	SCODE sc = EmulatedLine( pParms );	// Draw to the backup framebuffer

	if( FAILED(sc) )
		return sc;

	// Now, calculate the dirty-rect to refresh to the actual hardware
	RECT bounds;

	int N_plus_1;			// Minor length of bounding rect + 1

	if( pParms->dN )	// The line has a diagonal component (we'll refresh the bounding rect)
		N_plus_1 = 1 + ( ( pParms->cPels * pParms->dN ) / pParms->dM );
	else
		N_plus_1 = 1;

	switch( pParms->iDir )
	{
		case 0:
			bounds.left = pParms->xStart;
			bounds.top = pParms->yStart;
			bounds.right = pParms->xStart + pParms->cPels + 1;
			bounds.bottom = bounds.top + N_plus_1;
			break;
		case 1:
			bounds.left = pParms->xStart;
			bounds.top = pParms->yStart;
			bounds.bottom = pParms->yStart + pParms->cPels + 1;
			bounds.right = bounds.left + N_plus_1;
			break;
		case 2:
			bounds.right = pParms->xStart + 1;
			bounds.top = pParms->yStart;
			bounds.bottom = pParms->yStart + pParms->cPels + 1;
			bounds.left = bounds.right - N_plus_1;
			break;
		case 3:
			bounds.right = pParms->xStart + 1;
			bounds.top = pParms->yStart;
			bounds.left = pParms->xStart - pParms->cPels;
			bounds.bottom = bounds.top + N_plus_1;
			break;
		case 4:
			bounds.right = pParms->xStart + 1;
			bounds.bottom = pParms->yStart + 1;
			bounds.left = pParms->xStart - pParms->cPels;
			bounds.top = bounds.bottom - N_plus_1;
			break;
		case 5:
			bounds.right = pParms->xStart + 1;
			bounds.bottom = pParms->yStart + 1;
			bounds.top = pParms->yStart - pParms->cPels;
			bounds.left = bounds.right - N_plus_1;
			break;
		case 6:
			bounds.left = pParms->xStart;
			bounds.bottom = pParms->yStart + 1;
			bounds.top = pParms->yStart - pParms->cPels;
			bounds.right = bounds.left + N_plus_1;
			break;
		case 7:
			bounds.left = pParms->xStart;
			bounds.bottom = pParms->yStart + 1;
			bounds.right = pParms->xStart + pParms->cPels + 1;
			bounds.top = bounds.bottom - N_plus_1;
			break;
		default:
			DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Invalid direction: %d\r\n"),pParms->iDir));
			return E_INVALIDARG;
	}

	//DispDrvrDirtyRectDump( (LPRECT)&bounds );

	return sc;
}
   
			  
SCODE Dot_lcd::Line(
	GPELineParms *pLineParms,
	EGPEPhase phase )
{
	DEBUGMSG(GPE_ZONE_LINE,(TEXT("Dot_lcd::Line\r\n")));

	if( phase == gpeSingle || phase == gpePrepare )
	{
		if( pLineParms->pDst != m_pPrimarySurface  )
			pLineParms->pLine = &GPE::EmulatedLine;
		else
			pLineParms->pLine = (SCODE (GPE::*)(struct GPELineParms *))&Dot_lcd::WrappedEmulatedLine;
	}
	return S_OK;
}

#undef SWAP
#define SWAP(type,a,b) { type tmp; tmp=a; a=b; b=tmp; }

SCODE Dot_lcd::WrappedEmulatedBlt( GPEBltParms *pParms )
{
	SCODE sc = EmulatedBlt( pParms );	// Draw to the backup framebuffer

	if( FAILED(sc) )
		return sc;

	// Now, calculate the dirty-rect to refresh to the actual hardware
	RECT bounds;

	bounds.left = pParms->prclDst->left;
	bounds.top = pParms->prclDst->top;
	bounds.right = pParms->prclDst->right;
	bounds.bottom = pParms->prclDst->bottom;

	if( bounds.left > bounds.right )
	{
		SWAP( int, bounds.left, bounds.right )
	}
	if( bounds.top > bounds.bottom )
	{
		SWAP( int, bounds.top, bounds.bottom )
	}

	//DispDrvrDirtyRectDump( (LPRECT)&bounds );

	return sc;
}


SCODE Dot_lcd::BltPrepare(
	GPEBltParms *pBltParms )
{
	DEBUGMSG(GPE_ZONE_LINE,(TEXT("Dot_lcd::BltPrepare\r\n")));

	if(  pBltParms->pDst != m_pPrimarySurface )
		pBltParms->pBlt = &GPE::EmulatedBlt;
	else
		pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))&Dot_lcd::WrappedEmulatedBlt;

	return S_OK;
}

// This function would be used to undo the setting of clip registers etc
SCODE Dot_lcd::BltComplete( GPEBltParms *pBltParms )
{
	return S_OK;
}

int Dot_lcd::InVBlank()
{
	return 0;
}

SCODE Dot_lcd::SetPalette
(
	const PALETTEENTRY *src,
	unsigned short firstEntry,
	unsigned short numEntries
)
{
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
//
//  void GetFromRegistry(DWORD *dwState1, DWORD *dwState2, LPCTSTR lpszRegKey, 
//                      LPCTSTR lpszUseBat, LPCTSTR lpszUseExt)    
//
//  Get values from the registry. Set values to 1 in case of query errors.
//
////////////////////////////////////////////////////////////////////////////////
BOOL GetFromRegistry(DWORD *dwState, LPCTSTR lpszRegKey, LPCTSTR lpszContrast) 
{
	HKEY    hKey;
	DWORD   dwSize, dwType;

	
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszRegKey, 0, 0, &hKey))
	{
		dwSize = sizeof(DWORD); 
		// Query 'ContrastLevel' value to set the state of the "While on battery power" check box.
		RegQueryValueEx(hKey, lpszContrast, 0, &dwType, (LPBYTE)dwState, &dwSize);

		RegCloseKey(hKey);      
		return TRUE;
	}   
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
//
//  void SetToRegistry(DWORD *dwState1, DWORD *dwState2, LPCTSTR lpszRegKey, 
//                      LPCTSTR lpszRegValue1, LPCTSTR lpszRegValue2) 
//
//  Set values to the regsitry. 
//
//////////////////////////////////////////////////////////////////////////////// 
BOOL SetToRegistry(DWORD *dwState, LPCTSTR lpszRegKey, 
				   LPCTSTR lpszRegValue )    
{
	HKEY    hKey;
	ULONG	retval = (ULONG)ESC_FAILED;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszRegKey, 0, 0, &hKey))
	{       
		RegSetValueEx(hKey, lpszRegValue, 0, REG_DWORD, (LPBYTE)dwState, sizeof(DWORD));
		
		RegCloseKey(hKey);
		return TRUE;
	}   
	return FALSE;
}
//------------------------------------------------------------------------------
//
// Function: ConvertStringToGuid
//
// This converts a string into a GUID.
//
// Parameters:
//      pszGuid
//          [in] Pointer to GUID in string format.
//
//      pGuid
//          [out] Pointer to GUID struct for output
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL Dot_lcd::ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid)
{
	UINT Data4[8];
	int  Count;
	BOOL fOk = FALSE;
	TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

	DEBUGCHK(pGuid != NULL && pszGuid != NULL);
	PREFAST_SUPPRESS(6320, "Generic exception handler");
	__try
	{
		if (_stscanf(pszGuid, pszGuidFormat, &pGuid->Data1,
			&pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3],
			&Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
		{
			for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
			{
				PREFAST_SUPPRESS(6385, "_stscanf is a banned API, must be guaranteed by ourselves.");
				pGuid->Data4[Count] = (UCHAR) Data4[Count];
			}
		}
		fOk = TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		fOk = FALSE;
	}

	return fOk;
}
//------------------------------------------------------------------------------
//
// Function: AdvertisePowerInterface
//
// This routine notifies the OS that we support the
// Power Manager IOCTLs (through ExtEscape(), which
// calls DrvEscape()).
//
// Parameters:
//      hInst
//          [in] handle to module DLL
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL Dot_lcd::AdvertisePowerInterface(HMODULE hInst)
{
	GUID gTemp;
	BOOL fOk = FALSE;
	HKEY hk;
	DWORD dwStatus;

	// assume we are advertising the default class
	_tcscpy(m_szGuidClass, PMCLASS_DISPLAY);
	m_szGuidClass[MAX_PATH-1] = 0;
	fOk = ConvertStringToGuid(m_szGuidClass, &gTemp);
	DEBUGCHK(fOk);

	// check for an override in the registry
	dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\GDI\\Drivers"), 0, 0, &hk);
	if(dwStatus == ERROR_SUCCESS)
	{
		DWORD dwType, dwSize;
		dwSize = sizeof(m_szGuidClass);
		dwStatus = RegQueryValueEx(hk, _T("DisplayPowerClass"), NULL, &dwType, (LPBYTE)m_szGuidClass, &dwSize);
		if(dwStatus == ERROR_SUCCESS && dwType == REG_SZ)
		{
			// got a guid string, convert it to a guid
			fOk = ConvertStringToGuid(m_szGuidClass, &gTemp);
			DEBUGCHK(fOk);
		}

		// release the registry key
		RegCloseKey(hk);
	}

	// figure out what device name to advertise
	if(fOk)
	{
		fOk = GetModuleFileName(hInst, m_szDevName, sizeof(m_szDevName) / sizeof(m_szDevName[0]));
		DEBUGCHK(fOk);
	}

	// now advertise the interface
	if(fOk)
	{
		fOk = AdvertiseInterface(&gTemp, m_szDevName, TRUE);
		DEBUGCHK(fOk);
	}

	return fOk;
}


//------------------------------------------------------------------------------
//
// Function: PoweroffLCDIF
//
//      This function turns off the LCD.
//
// Parameters
//      None.
//
// Returns
//      None.
//
//------------------------------------------------------------------------------
VOID Dot_lcd::PoweroffLCDIF()
{
//	BSPBacklightEnable(FALSE);
	BSPDisplayPowerHandler(POWEROFF);
}

//------------------------------------------------------------------------------
//
// Function: PowerOnLCDIF
//
//      This function turns on the LCD and starts the LCDIF running.
//
// Parameters
//      None.
//
// Returns
//      None.
//
//------------------------------------------------------------------------------
VOID Dot_lcd::PowerOnLCDIF()
{
	BSPDisplayPowerHandler(POWERON);
//	BSPBacklightEnable(TRUE);
}

//------------------------------------------------------------------------------
//
// Function: SetDisplayPower
//
// This function sets the display hardware according
// to the desired video power state. Also updates
// current device power state variable.
//
// Parameters:
//      dx
//          [in] Desired video power state.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID Dot_lcd::SetDisplayPower(CEDEVICE_POWER_STATE dx)
{
	switch(dx)
	{
	case D0:
		if(m_Dx != dx)
		{
			PowerOnLCDIF();

			m_Dx = D0;
		}
		break;

	case D1:
	case D2:
	case D3:
	case D4:
		if(m_Dx != D4)
		{
			//Sleep(200); // IMPORTANT: Wait at least one frame time!

			PoweroffLCDIF();

			m_Dx = D4;
		}
		break;

	default:
		break;
	}
}

//------------------------------------------------------------------------------
//
// Function: DrvEscape
//
// This routine handles the needed DrvEscape codes.
//
// Parameters:
//      pso
//          [in] Pointer to a SURFOBJ structure that describes the surface
//          to which the call is directed.
//
//      iEsc
//          [in] Query. The meaning of the other parameters depends on
//          this value. QUERYESCSUPPORT is the only predefined value; it
//          queries whether the driver supports a particular escape function.
//          In this case, pvIn points to an escape function number; cjOut and
//          pvOut are ignored. If the specified function is supported, the
//          return value is nonzero.
//
//      cjIn
//          [in] Size, in bytes, of the buffer pointed to by pvIn.
//
//      pvIn
//          [in] Pointer to the input data for the call. The format of the
//          input data depends on the query specified by the iEsc parameter.
//
//      cjOut
//          [in] Size, in bytes, of the buffer pointed to by pvOut.
//
//      pvOut
//          [out] Pointer to the output buffer. The format of the output data
//          depends on the query specified by the iEsc parameter.
//
// Returns:
//      TRUE            successful
//      FALSE          failed
//
//------------------------------------------------------------------------------
ULONG Dot_lcd::DrvEscape(SURFOBJ * pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut)
{
	ULONG retval = (ULONG)ESC_FAILED;
	//PVIDEO_POWER_MANAGEMENT psPowerManagement;

	UNREFERENCED_PARAMETER(pso);

	switch (iEsc)
	{
	case QUERYESCSUPPORT:
		if(pvIn != NULL && cjIn == sizeof(DWORD))
		{
			// Query DrvEscape support functions
			DWORD EscapeFunction;
			EscapeFunction = *(DWORD *)pvIn;
			if ((EscapeFunction == QUERYESCSUPPORT)			 ||
				(EscapeFunction == IOCTL_POWER_CAPABILITIES) ||
				(EscapeFunction == IOCTL_POWER_QUERY)        ||
				(EscapeFunction == IOCTL_POWER_SET)          ||
				(EscapeFunction == IOCTL_POWER_GET)          ||
				(EscapeFunction == CONTRASTCOMMAND)  
				)
				retval = ESC_SUCCESS;
			else
				retval = ESC_FAILED;

			if(retval != ESC_SUCCESS)
				SetLastError(ERROR_INVALID_PARAMETER);
		}
		else
			SetLastError(ERROR_INVALID_PARAMETER);
		break;
	case IOCTL_POWER_CAPABILITIES:
		//DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: IOCTL_POWER_CAPABILITIES\r\n")));
		if(pvOut != NULL && cjOut == sizeof(POWER_CAPABILITIES))
		{
			PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pvOut;
			memset(ppc, 0, sizeof(POWER_CAPABILITIES));
			ppc->DeviceDx = 0x11;   // support D0 and D4
			ppc->WakeFromDx = 0x00; // No wake capability
			ppc->InrushDx = 0x00;       // No in rush requirement
			ppc->Power[D0] = 600;                   // 0.6W
			ppc->Power[D1] = (DWORD)PwrDeviceUnspecified;
			ppc->Power[D2] = (DWORD)PwrDeviceUnspecified;
			ppc->Power[D3] = (DWORD)PwrDeviceUnspecified;
			ppc->Power[D4] = 0;
			ppc->Latency[D0] = 0;
			ppc->Latency[D1] = (DWORD)PwrDeviceUnspecified;
			ppc->Latency[D2] = (DWORD)PwrDeviceUnspecified;
			ppc->Latency[D3] = (DWORD)PwrDeviceUnspecified;
			ppc->Latency[D4] = 0;
			ppc->Flags = 0;
			retval = ESC_SUCCESS;
		}
		else
			SetLastError(ERROR_INVALID_PARAMETER);
		break;

	case IOCTL_POWER_QUERY:
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
			// return a good status on any valid query, since we are always ready to
			// change power states.
			CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pvOut;
			if(VALID_DX(NewDx))
			{
				// this is a valid Dx state so return a good status
				retval = ESC_SUCCESS;
			}
			else
				SetLastError(ERROR_INVALID_PARAMETER);
			DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: IOCTL_POWER_QUERY %u %s\r\n"),
				NewDx, retval? L"succeeded" : L"failed"));
		}
		else
			SetLastError(ERROR_INVALID_PARAMETER);
		break;

	case IOCTL_POWER_GET:
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
			CEDEVICE_POWER_STATE CurrentDx = m_Dx;
			*(PCEDEVICE_POWER_STATE)pvOut = CurrentDx;
			retval = ESC_SUCCESS;
			DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: %s IOCTL_POWER_GET: passing back %u\r\n"), m_szDevName, CurrentDx));
		}
		else
			SetLastError(ERROR_INVALID_PARAMETER);
		break;

	case IOCTL_POWER_SET:
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
			CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pvOut;
			if(VALID_DX(NewDx))
			{
				SetDisplayPower(NewDx);
				retval = ESC_SUCCESS;
				DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: %s IOCTL_POWER_SET %u: passing back %u\r\n"), m_szDevName, NewDx, m_Dx));
			}
			else
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: IOCTL_POWER_SET: invalid state request %u\r\n"), NewDx));
			}
		}
		else
			SetLastError(ERROR_INVALID_PARAMETER);
		break;

	case CONTRASTCOMMAND:
		if( pvIn != NULL && cjIn == sizeof(ContrastCmdInputParm) &&
			pvOut != NULL && cjOut == sizeof(DWORD))
		{
			if(  SetContrast( (ContrastCmdInputParm *)pvIn, (DWORD*)pvOut) )
				retval = ESC_SUCCESS;
		}
		break;

	default:
		retval = ESC_FAILED;
		break;
	}
	return retval;
}

BOOL Dot_lcd::SetContrast( ContrastCmdInputParm *pContrastCmd, DWORD *pvOut)
{
	BOOL bResult = FALSE;

	switch( pContrastCmd->command )
	{
	case CONTRAST_CMD_GET:
		bResult = GetFromRegistry( pvOut, SZREGKEY, SZCONTRASTLEVEL);
		break;

	case CONTRAST_CMD_SET:
		*pvOut = pContrastCmd->parm;
		if( BSPSetContrast(*pvOut) )
		{
			bResult = SetToRegistry( pvOut, SZREGKEY, SZCONTRASTLEVEL);
		}
		break;

	case CONTRAST_CMD_INCREASE:
		if( GetFromRegistry( pvOut, SZREGKEY, SZCONTRASTLEVEL) )
		{
			*pvOut += pContrastCmd->parm;
			if(  BSPSetContrast(*pvOut))
				bResult = SetToRegistry( pvOut, SZREGKEY, SZCONTRASTLEVEL);
		}
		break;

	case CONTRAST_CMD_DECREASE:
		if( GetFromRegistry( pvOut, SZREGKEY, SZCONTRASTLEVEL) )
		{
			*pvOut -= pContrastCmd->parm;
			if(  BSPSetContrast( *pvOut) )
				bResult = SetToRegistry( pvOut, SZREGKEY, SZCONTRASTLEVEL);
		}
		break;

	case CONTRAST_CMD_DEFAULT:
		bResult = BSPGetContrast( pvOut, GET_DEFAULT_CONTRAST_LEVEL );
		break;

	case CONTRAST_CMD_MAX:
		bResult = BSPGetContrast( pvOut, GET_MAX_CONTRAST_LEVEL );
		break;

	default:
		break;
	}
	return bResult;
}

DWORD Dot_lcd::IntrProc( )
{
	while( !m_bStopIntrProc )
	{
		WaitForSingleObject( m_hSyncEvent, INFINITE );
		if( m_bStopIntrProc )
			break;

		// System power down event, Show PowerDown image
		if( WaitForSingleObject( m_hSysShutDownEvent, 0 ) == WAIT_OBJECT_0 )
		{
			pCurrentlySurface = (PVOID)m_pBackUpSurface->Buffer() ;
		}

		BSPFrameBufferUpdate( pCurrentlySurface );
		InterruptDone( m_dwlcdcSysintr );
	}
	return 0;
}

void RegisterDDHALAPI()
{
	;	// No DDHAL support in 2bpp wrapper
}


ulong BitMasks[] = { 0x0000,0x0000,0x0000 };

ULONG *APIENTRY DrvGetMasks(
    DHPDEV     dhpdev)
{
	return BitMasks;
}


