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
//------------------------------------------------------------------------------
//
//  File:  omap2420gpe.h
//
#ifndef __OMAP2420GPE_H
#define __OMAP2420GPE_H

//#define DEBUG

//------------------------------------------------------------------------------
// for DrvEscape

#define ESC_SUCCESS             (1)
#define ESC_FAILED              (-1)
#define ESC_NOTIMPLEMENTED      (0)

//------------------------------------------------------------------------------

#define BLTFUNC                 SCODE (GPE::*)(struct GPEBltParms *)
#define LINEFUNC                SCODE (GPE::*)(struct GPELineParms *)

//------------------------------------------------------------------------------
//
//  Define:  MEM_IDX_xxx
//
//  Symbolic name for m_memBase/m_memLen values.
//

//  Modify the Platform.reg file also

#define MEM_IDX_DSS             0
#define MEM_IDX_LCD             1
#define MEM_IDX_RFB             2
#define MEM_IDX_VENC            3
#define MEM_IDX_SRAM            4

//------------------------------------------------------------------------------

#if defined(TVOUT)
typedef struct __VENC_INIT__
{
    UINT32 ulVENC_F_CONTROL;
    UINT32 ulVENC_SYNC_CTRL;
    UINT32 ulVENC_LLEN;
    UINT32 ulVENC_FLENS;
    UINT32 ulVENC_HFLTR_CTRL;
    UINT32 ulVENC_GAIN_U;
    UINT32 ulVENC_GAIN_V;
    UINT32 ulVENC_GAIN_Y;
    UINT32 ulVENC_BLACK_LEVEL;
    UINT32 ulVENC_BLANK_LEVEL;
    UINT32 ulVENC_X_COLOR;
    UINT32 ulVENC_M_CONTROL;
    UINT32 ulVENC_BSTAMP_WSS_DATA;
    UINT32 ulVENC_S_CARR;
    UINT32 ulVENC_SAVID__EAVID;
    UINT32 ulVENC_FLEN__FAL;
    UINT32 ulVENC_LAL__PHASE_RESET;
    UINT32 ulVENC_HS_INT_START_STOP_X;
    UINT32 ulVENC_HS_EXT_START_STOP_X;
    UINT32 ulVENC_VS_INT_START_X;
    UINT32 ulVENC_VS_INT_STOP_X__VS_INT_START_Y;
    UINT32 ulVENC_VS_INT_STOP_Y__VS_EXT_START_X;
    UINT32 ulVENC_VS_EXT_STOP_X__VS_EXT_START_Y;
    UINT32 ulVENC_VS_EXT_STOP_Y;
    UINT32 ulVENC_AVID_START_STOP_X;
    UINT32 ulVENC_AVID_START_STOP_Y;
    UINT32 ulVENC_FID_INT_START_X__FID_INT_START_Y;
    UINT32 ulVENC_FID_INT_OFFSET_Y__FID_EXT_START_X;
    UINT32 ulVENC_FID_EXT_START_Y__FID_EXT_OFFSET_Y;
    UINT32 ulVENC_TVDETGP_INT_START_STOP_X;
    UINT32 ulVENC_TVDETGP_INT_START_STOP_Y;
    UINT32 ulVENC_GEN_CTRL;
}
VENC_INIT;
#endif	// defined(TVOUT)

//------------------------------------------------------------------------------

class OMAP2420Surf;

//------------------------------------------------------------------------------

class OMAP2420GPE : public DDGPE
{
public:
    DWORD                   m_memBase[5];
    DWORD                   m_memLen[5];
    DWORD                   m_bitsPerPixel;
    DWORD                   m_fbCached;
    LPWSTR                  m_szDisplayPowerClass;
    DWORD                   m_powerDelay;
    DWORD                   m_priority256;
    
private:
    OMAP2420_DSS_REGS       *m_pDSSRegs;
    OMAP2420_DISPC_REGS     *m_pDISPCRegs;
    OMAP2420_VENC_REGS      *m_pVENCRegs;

    HANDLE                  m_hDisplay;

    OMAP2420Surf            *m_pVisibleSurface;
    OMAP2420Surf            *m_pVisibleOverlay;
    BOOL                    m_flipBusy;
    GPEMode                 m_gpeMode;
    CEDEVICE_POWER_STATE    m_currentDX;
    CEDEVICE_POWER_STATE    m_externalDX;
    GUID                    m_powerClassGUID;

    HANDLE                  m_hPowerEvent;
    HANDLE                  m_hPowerThread;
    BOOL                    m_powerThreadExit;

    CRITICAL_SECTION        m_powerCS;

    LONG                    m_updateFlag;
    DWORD                   m_DPI;
//cc #if defined(TVOUT)
    OMAP2420_VID_REGS       *m_pTVVidRegs;
    HANDLE                  m_hI2C;
    DWORD                   m_dwTVOut;
//cc #endif

private:
    BOOL SetPower(CEDEVICE_POWER_STATE dx);

    void InitController(OMAP2420_DISPLAY_MODE_INFO *pInfo);
    void SetControllerAddresses(void);

    SCODE WrappedEmulatedLine(GPELineParms *lineParameters);

    static DWORD PowerThreadStub(VOID *pContext);
    DWORD PowerThread();

    public:
    OMAP2420GPE();
    virtual ~OMAP2420GPE();

    BOOL Init(LPCWSTR context);

    virtual int InVBlank();
    virtual int NumModes();

    virtual SCODE SetPalette(const PALETTEENTRY*, WORD, WORD) { return S_OK; }

    virtual SCODE AllocSurface(GPESurf **, int, int, EGPEFormat, int);
    
    virtual SCODE AllocSurface(DDGPESurf **,  int, int, EGPEFormat, EDDGPEPixelFormat, int);

    virtual SCODE Line(GPELineParms*, EGPEPhase);
    virtual SCODE BltPrepare(GPEBltParms*);
    virtual SCODE BltComplete(GPEBltParms*);

    virtual SCODE GetModeInfo(GPEMode*, int);
    virtual SCODE SetMode(int, HPALETTE*);
    virtual SCODE SetPointerShape(GPESurf*, GPESurf*, int, int, int, int);
    virtual SCODE MovePointer(int, int);

    virtual VOID PowerHandler(BOOL bOff);

    virtual VOID SetVisibleSurface(GPESurf*, DWORD, BOOL);
    virtual BOOL SurfaceBusyFlipping(GPESurf*);

    virtual ULONG DrvEscape(SURFOBJ*, ULONG, ULONG, VOID*, ULONG, VOID*);

    void BuildDDHALInfo(DDHALINFO*, DWORD);

    int GetGameXInfo(ULONG, ULONG, VOID*, ULONG, VOID*);

    DWORD UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd);
    DWORD SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd);

//cc #if defined(TVOUT)
    BOOL ReadI2C(DWORD dwAddress, DWORD dwAddressSize, UCHAR ucReg, UCHAR *pucData);
    BOOL WriteI2C(DWORD dwAddress, DWORD dwAddressSize, UCHAR ucReg, UCHAR ucData);
    BOOL ReadI2CIO(DWORD dwAddress, DWORD dwAddressSize, UCHAR *pucData);
    BOOL WriteI2CIO(DWORD dwAddress, DWORD dwAddressSize, UCHAR ucData);
    BOOL EnableVREF(BOOL fEnable);
    void SetTVOut(DWORD dwTVOut);
//cc #endif
#if defined(DEBUG)
    void DumpRegs(LPTSTR szMsg);
    void DumpVidRegs(DWORD dwIndex);
#endif
};

    //------------------------------------------------------------------------------

#endif // __OMAP2420GPE_H
