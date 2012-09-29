//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  IGPSCtlPdd.h
//
//  Contains declarations of platform-specific functions.
//
//-----------------------------------------------------------------------------
/// \file Trigger inclusion in doxygen documentation

    bool initHw(LPCTSTR pContext);
    bool deinitHw(DWORD deviceContext);
    void enableAsicReset();
    void disableAsicReset();
    void enableAsicPowerOn();
    void disableAsicPowerOn();
