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
    virtual void BacklightEnable(BOOL Enable);
    virtual void InitDisplay();
	virtual void InitLCD(unsigned char* pV, ULONG pP );

private:
    DisplayControllerUC1698();

    void InitBacklight(UINT32 u32Frequency, UINT8 u8DutyCycle);
    void BSPInitLCDIF(BOOL bReset);
    void BSPResetController();
    BOOL DDKIomuxSetupLCDIFPins(BOOL bPoweroff);

private:
    static DisplayControllerUC1698 * SingletonController;    
    DWORD m_Bpp;

}; //class

#endif /* __LMS430_CONTROLLER_H__ */
