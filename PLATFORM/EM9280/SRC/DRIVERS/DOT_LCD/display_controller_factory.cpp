//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  display_controller_factory.cpp
//
//  Implementation of class DisplayControllerFactory.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
//#include "precomp.h"
#pragma warning(pop)

#include "display_controller_factory.h"
#include "UC1698.h"

const GUID DisplayControllerFactory::ControllerLMS430 = {
    0x1ed47d96, 0x6842, 0x4c20,
    { 0x87, 0x5, 0xbf, 0x5, 0xad, 0xd, 0xfc, 0x33 }
};

// CS&ZHL MAR-6-2012: "GUID"="{83A0FF68-78BB-4DF5-8DEB-077961EE75BC}"
const GUID DisplayControllerFactory::Controller43WVF1G = {
    0x83a0ff68, 0x78bb, 0x4df5,
    { 0x8d, 0xeb, 0x7, 0x79, 0x61, 0xee, 0x75, 0xbc }
};

//------------------------------------------------------------------------------
//
// Function: GetDisplayController
//
//      This function returns DisplayController pointer according to GUID.
//
// Parameters
//      guid.
//          [in] Pointer to GUID
//
// Returns
//      Pointer to DisplayController.
//
//------------------------------------------------------------------------------
DisplayController* DisplayControllerFactory::GetDisplayController(const GUID* const guid)
{
    //if (*guid == Controller43WVF1G)
    //{
        return DisplayControllerUC1698::GetInstance();
    //}
   // return NULL;
} //DisplayControllerFactory
