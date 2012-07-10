//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  lms430.h
//  Declarations for DisplayControllerLMS430 class
//
//
//-----------------------------------------------------------------------------

#ifndef __UC1698_CONTROLLER_H__
#define __UC1698_CONTROLLER_H__

#include "common_lcdif.h"
#include "display_controller.h"

class DisplayControllerUC1698 : public DisplayController
{

public:
    ~DisplayControllerUC1698();

    virtual void DispDrvrPowerHandler(BOOL bOn, BOOL bInit, BOOL bReset,BOOL bPowerOff);
    static DisplayControllerUC1698 * GetInstance();

    virtual DWORD GetWidth();
    virtual DWORD GetHeight();
	virtual DWORD GetVideoMemorySize();

    virtual void BacklightEnable(BOOL Enable);
    virtual void InitDisplay();
	virtual void InitLCD( );
	virtual void  Update( PVOID pSurface );
	virtual void SetDisplayBuffer( ULONG PhysBase, PVOID VirtBase );
	virtual BOOL SetContrast( DWORD dwContrastLevel );
	virtual BOOL GetContrast( DWORD* dwContrastLevel, DWORD dwFlag );
	

private:
    DisplayControllerUC1698();

    void InitBacklight(UINT32 u32Frequency, UINT8 u8DutyCycle);
    void BSPInitLCDIF(BOOL bReset);
    void BSPResetController();
    BOOL DDKIomuxSetupLCDIFPins(BOOL bPoweroff);
	//void LCDPowerDown( );
	void LCDPowerUp( );
	void LCDPower( UINT32 powerVal );

private:
    static DisplayControllerUC1698 * SingletonController;    
    DWORD m_Bpp;
	ULONG m_PhysBase;
	PVOID m_pVirtBase; 
	BOOL  m_bSetContrast;
	BYTE m_uContrastLevel;
	CRITICAL_SECTION cs;
	BOOL  m_bPowerOff, m_bPowerOn;

}; //class

#endif /* __UC1698_CONTROLLER_H__ */
