//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  display_controller.h
//  Declarations for DisplayController class
//
//
//-----------------------------------------------------------------------------

#ifndef __DISPLAY_CONTROLLER_H__
#define __DISPLAY_CONTROLLER_H__
class DisplayController
{
public:
    virtual ~DisplayController() {}
    virtual void  DispDrvrPowerHandler(BOOL bOn, BOOL bInit, BOOL bReset,BOOL bPowerOff) = 0;
    virtual DWORD GetWidth() = 0;
    virtual DWORD GetHeight() = 0;
    virtual void  BacklightEnable(BOOL Enable) = 0;
    virtual void  InitDisplay() = 0;
	virtual void  InitLCD(unsigned char* pV, ULONG pP ) =0;
}; //class

#endif /* __DISPLAY_CONTROLLER_H__ */