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

Notes: 
--*/
#ifndef __DOT_LCD_H__
#define __DOT_LCD_H__

#include <ceddk.h>

// DrvEscape return codes
#define ESC_FAILED          (-1)
#define ESC_NOT_SUPPORTED   (0)
#define ESC_SUCCESS         (1)

class Dot_lcd  : public GPE
{
private:

	GPEMode			m_ModeInfo;
	void			*m_pVirtualFrameBuffer;

	SCODE			WrappedEmulatedLine
					(
						GPELineParms *pParms
					);
	SCODE			WrappedEmulatedBlt
					(
						GPEBltParms *pParms
					);
	
public:
					Dot_lcd();
	virtual int		NumModes();
	virtual SCODE	SetMode
					(
						int modeId,
						HPALETTE *pPalette
					);
	virtual int		InVBlank();
	virtual SCODE	SetPalette
					(
						const PALETTEENTRY *src,
						unsigned short firstEntry,
						unsigned short numEntries
					);
	virtual SCODE	GetModeInfo
					(
						GPEMode *pMode,
						int modeNo
					);
	virtual SCODE	SetPointerShape
					(
						GPESurf *pMask,
						GPESurf *pColorSurf,
						int xHot,
						int yHot,
						int cx,
						int cy
					);
	virtual SCODE	MovePointer
					(
						int x,
						int y
					);
	virtual void	WaitForNotBusy();
	virtual int		IsBusy();
	virtual void	GetPhysicalVideoMemory
					(
						unsigned long *pPhysicalMemoryBase,
						unsigned long *pVideoMemorySize
					);
	virtual SCODE	AllocSurface
					(
						GPESurf **ppSurf,
						int width,
						int height,
						EGPEFormat format,
						int surfaceFlags
					);
	virtual SCODE	Line
					(
						GPELineParms *pLineParms,
						EGPEPhase phase
					);
	virtual SCODE	BltPrepare
					(
						GPEBltParms *pBltParms
					);
	virtual SCODE	BltComplete
					(
						GPEBltParms *pBltParms
					);

	virtual ULONG DrvEscape
					(
						SURFOBJ * pso,
						ULONG iEsc,
						ULONG cjIn,
						PVOID pvIn, 
						ULONG cjOut, 
						PVOID pvOut
					);
	virtual VOID    PowerHandler(BOOL bOff);

private:
	HANDLE			m_hSyncEvent;
	HANDLE			m_bStopIntrProc;
	DWORD			m_dwlcdcSysintr;
	HANDLE			m_hSyncThread;
	HANDLE			m_hSysShutDownEvent;

	PHYSICAL_ADDRESS m_VideoMemPhysicalAddress;
	PHYSICAL_ADDRESS m_VideoMemVirtualAddress;
	ULONG			m_nLAWPhysical;
	DWORD			m_dwFrameBufferSize;

	unsigned char * m_VirtualMemAddr;

	int				m_nModeNumber;
	GPESurf			*m_pBackUpSurface;
	PVOID			pCurrentlySurface;
	// for power management
	CEDEVICE_POWER_STATE m_Dx;
	TCHAR m_szDevName[MAX_PATH];   // Device name
	TCHAR m_szGuidClass[MAX_PATH]; // Class GUID
	BOOL			ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid);
	
	BOOL			InitIrq( );
	BOOL			AllocPhysicalMemory( );
	BOOL			SetContrast( ContrastCmdInputParm *pContrastCmd, DWORD *pvOut);
	void			SetDisplayPower(CEDEVICE_POWER_STATE dx);
	void			PoweroffLCDIF();
	void			PowerOnLCDIF();
	BOOL			AdvertisePowerInterface(HMODULE hInst);

public:
	DWORD IntrProc( );

};


#endif __DOT_LCD_H__

