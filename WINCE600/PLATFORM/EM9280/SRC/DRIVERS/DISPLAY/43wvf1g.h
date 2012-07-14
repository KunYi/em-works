//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  43wvf1g.h
//  Declarations for DisplayController43WVF1G class
//
//
//-----------------------------------------------------------------------------

#ifndef __43WVF1G_CONTROLLER_H__
#define __43WVF1G_CONTROLLER_H__

#include "common_lcdif.h"
#include "display_controller.h"

class DisplayController43WVF1G : public DisplayController
{

public:
    ~DisplayController43WVF1G();

    virtual void DispDrvrPowerHandler(BOOL bOn, BOOL bInit, BOOL bReset,BOOL bPowerOff);
    static DisplayController43WVF1G* GetInstance();
    virtual DWORD GetWidth();
    virtual DWORD GetHeight();
    virtual void BacklightEnable(BOOL Enable);
    virtual void InitDisplay();

private:
    DisplayController43WVF1G();

    void InitBacklight(UINT32 u32Frequency, UINT8 u8DutyCycle);
    void BSPInitLCDIF(BOOL bReset);
    void BSPResetController();
    BOOL DDKIomuxSetupLCDIFPins(BOOL bPoweroff);

private:
    static DisplayController43WVF1G * SingletonController;    
    DWORD m_Bpp;

	// CS&ZHL MAY-8-2012: display format
	DWORD	m_dwWidth;
	DWORD	m_dwHeight;

}; //class

#endif /* __43WVF1G_CONTROLLER_H__ */
