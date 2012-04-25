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
RGBQUAD _rgb1bpp[PALETTE_SIZE] =
{
	{ 0x00, 0x00, 0x00, 0 },    /*   0 */	/* Black        */   	  
	{ 0xff, 0xff, 0xff, 0 }     /* 255 */	/* White        */
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


Dot_lcd::Dot_lcd()
{
	
	RETAILMSG(1, (L"-->LCDIF Dot_lcd\r\n"));
	BSPSetDisplayController();

	BSPGetModeInfo( &m_ModeInfo,  m_nModeNumber );

	m_pMode = &m_ModeInfo;

	m_pPrimarySurface = new GPESurf( m_ModeInfo.width, m_ModeInfo.height, m_ModeInfo.format );
	if (!m_pPrimarySurface)
	{
		RETAILMSG(1, (TEXT("Wrap2bpp::Wrap2bpp: Error allocating GPESurf.\r\n")));
		return;

	}

	AllocPhysicalMemory();

	BSPSetDisplayBuffer( m_VideoMemPhysicalAddress.LowPart, (PVOID)m_VideoMemVirtualAddress.LowPart);

	InitIrq( );
	BSPInitLCD(  );

	//memset( m_VirtualMemAddr, 0, m_dwFrameBufferSize );
	
	
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
 								PALETTE_SIZE, 	// i.e. 2
 								(ULONG *)_rgb1bpp,
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
	/*if( x == -1 )
	{
		DispDrvrMoveCursor( m_ModeInfo.width, m_ModeInfo.height );		// disable cursor
	}
	{
		DispDrvrMoveCursor( x, y );		// disable cursor
	}*/
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

DWORD Dot_lcd::IntrProc( )
{
	while( !m_bStopIntrProc )
	{
		WaitForSingleObject( m_hSyncEvent, INFINITE );
		if( m_bStopIntrProc )
			break;

		BSPFrameBufferUpdate( m_pPrimarySurface->Buffer() );
		InterruptDone( m_dwlcdcSysintr );
	}
	return 0;
}

void RegisterDDHALAPI()
{
	;	// No DDHAL support in 2bpp wrapper
}

ulong BitMasks[] = { 0x0000,0x0001,0x0000 };

ULONG *APIENTRY DrvGetMasks(
    DHPDEV     dhpdev)
{
	return BitMasks;
}


