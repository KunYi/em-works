//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  display_controller_factory.h
//  Declarations for DisplayControllerFactory class
//
//
//-----------------------------------------------------------------------------

#ifndef __DISPLAY_CONTROLLER_FACTORY_H__
#define __DISPLAY_CONTROLLER_FACTORY_H__

//#include <windows.h>
//#include "precomp.h"

class DisplayController;

class DisplayControllerFactory
{
public:
    static const GUID Controller43WVF1G;
    static const GUID ControllerLMS430;

private:
    DisplayControllerFactory() {
    }

public:
    ~DisplayControllerFactory() {}

    static DisplayController* GetDisplayController(const GUID* const guid);
}; //class
#endif /* __DISPLAY_CONTROLLER_FACTORY_H__ */
