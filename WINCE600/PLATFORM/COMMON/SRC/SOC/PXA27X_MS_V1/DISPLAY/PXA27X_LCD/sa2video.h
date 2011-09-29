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
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.
** Title to the Material remains with Intel Corporation or its suppliers and licensors.
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
*/

#ifndef __SA2VIDEO_H__
#define __SA2VIDEO_H__

#include <dispdrvr.h>
#include <pm.h>

// CE uses a 32 x 32 pixel cursor.
#define CURSOR_SIZE 32*32

// OEM escape code base
#define ESCAPECODEBASE          100000
#define DISPLAYPAGE             (ESCAPECODEBASE + 1)
#define VERTICALBLANKINTERRUPT  (ESCAPECODEBASE + 3)
#define OS_SCREENACCESS         (ESCAPECODEBASE + 4)
#define SCROLL                  (ESCAPECODEBASE + 5)


// GAPI support
#define kidVersion100   100
#define kfLandscape     0x8        // Screen is rotated 270 degrees
#define kfPalette       0x10
#define kfDirect        0x20
#define kfDirect555     0x40
#define kfDirect565     0x80
#define kfDirect888     0x100
#define GETGXINFO       0x00020000

struct GXDeviceInfo {
    long idVersion;
    void * pvFrameBuffer;
    unsigned long cbStride;
    unsigned long cxWidth;
    unsigned long cyHeight;
    unsigned long cBPP;
    unsigned long ffFormat;
    short vkButtonUpPortrait;
    short vkButtonUpLandscape;
    POINT ptButtonUp;
    short vkButtonDownPortrait;
    short vkButtonDownLandscape;
    POINT ptButtonDown;
    short vkButtonLeftPortrait;
    short vkButtonLeftLandscape;
    POINT ptButtonLeft;
    short vkButtonRightPortrait;
    short vkButtonRightLandscape;
    POINT ptButtonRight;
    short vkButtonAPortrait;
    short vkButtonALandscape;
    POINT ptButtonA;
    short vkButtonBPortrait;
    short vkButtonBLandscape;
    POINT ptButtonB;
    short vkButtonCPortrait;
    short vkButtonCLandscape;
    POINT ptButtonC;
    short vkButtonStartPortrait;
    short vkButtonStartLandscape;
    POINT ptButtonStart;
    void * pvReserved1;
    void * pvReserved2;
};

class SA2Video : public DDGPE
{
private:
    TCHAR           m_DisplayDeviceName[MAX_PATH];
    CEDEVICE_POWER_STATE m_PmPowerState;
    GPEMode         m_ModeInfo;
    GPEModeEx       m_pModeEx;
    HANDLE          m_hevInterrupt;
    void           *m_pVirtualFrameBuffer;
    DWORD           m_VirtualFrameBuffer;
    Node2D         *m_p2DVideoMemory;
    DWORD           m_cbScanLineLength;
    DWORD           m_RedMaskSize;
    DWORD           m_RedMaskPosition;
    DWORD           m_GreenMaskSize;
    DWORD           m_GreenMaskPosition;
    DWORD           m_BlueMaskSize;
    DWORD           m_BlueMaskPosition;

    BOOL            ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid);
    BOOL            AdvertisePowerInterface();
    ULONG           PmToVideoPowerState(CEDEVICE_POWER_STATE Dx);
    void            SetPmPowerState(CEDEVICE_POWER_STATE PowerState);
    ULONG           GetVideoPowerState(void);
    CEDEVICE_POWER_STATE  VideoToPmPowerState(ULONG PowerState);
    CEDEVICE_POWER_STATE  GetPmPowerState(void);

    SCODE           WrappedEmulatedLine(GPELineParms *pParms);
    SCODE           WrappedEmulatedBlt(GPEBltParms *pParms);

    BOOL            m_CursorDisabled;
    BOOL            m_CursorVisible;
    BOOL            m_CursorForcedOff;
    POINTL          m_CursorSize;
    RECTL           m_CursorRect;

    USHORT          m_CursorBackingStore[CURSOR_SIZE];

public:
    BOOL            m_InDDraw;

                    SA2Video();
    virtual int     NumModes();
    virtual SCODE   SetMode(int modeId, HPALETTE *pPalette);
    virtual int     InVBlank();
    virtual SCODE   SetPalette(const PALETTEENTRY *src,
                        unsigned short firstEntry,unsigned short numEntries);
    virtual SCODE   GetModeInfo(GPEMode *pMode,int modeNo );
    virtual VOID    PowerHandler(BOOL bOff);
    virtual SCODE   SetPointerShape(GPESurf *pMask,GPESurf *pColorSurf,
                        int xHot,int yHot,int cx,int cy);
    virtual SCODE   MovePointer(int x,int y);
    virtual void    WaitForNotBusy();
    virtual int     IsBusy();
    virtual void    GetPhysicalVideoMemory(unsigned long *pPhysicalMemoryBase,
                        unsigned long *pVideoMemorySize);
    virtual SCODE   AllocSurface(GPESurf **ppSurf,int width,int height,
                        EGPEFormat format,int surfaceFlags);
    virtual SCODE   Line(GPELineParms *pLineParms,EGPEPhase phase);
    virtual SCODE   BltPrepare(GPEBltParms *pBltParms);
    virtual SCODE   BltComplete(GPEBltParms *pBltParms);
    virtual ULONG   DrvEscape(SURFOBJ *pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut);

    void            GetVirtualVideoMemory(unsigned long * virtualMemoryBase, unsigned long *videoMemorySize);
    virtual SCODE   AllocSurface(DDGPESurf **ppSurf, int width, int height, EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags);

    int GetRotateModeFromReg();
    void SetRotateParams();
    LONG DynRotate(int angle);

    void CursorOn();
    void CursorOff();

    friend void buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx );
};

#endif // __SA2VIDEO_H__



