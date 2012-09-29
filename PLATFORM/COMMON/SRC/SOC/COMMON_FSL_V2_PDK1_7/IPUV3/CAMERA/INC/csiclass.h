//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CsiClass.h
//
//  Common definitions for IPUv3's csi class module
//
//-----------------------------------------------------------------------------

#ifndef __CSICLASS_H__
#define __CSICLASS_H__

//------------------------------------------------------------------------------
// Defines


#define CSI_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CSI_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Functions
class CsiClass
{
    public:
        CsiClass(CSI_SELECT csi_sel);
        ~CsiClass(void);

        void CsiEnable();
        void CsiDisable();
        void CsiConfigureSensor(DWORD dwFramerate);
        void CsiChangeFrameRate(DWORD dwFramerate);
        BOOL CsiConfigure(csiSensorOutputFormat , csiSensorOutputResolution, CSI_PROTOCOL_INF *pPrtclInf, BOOL );
        void CsiZoom(DWORD);
        void CsiSourceSel(csiConnectedDataSource csi_sourcesel);
        CSI_SELECT CsiGetModuleInterface();
        void DumpCSIRegs(void);
        
    private:
        void CsiInit(CSI_SELECT csi_sel);
        void CsiDeinit();

        CSI_SELECT m_CsiSel;
        csiSensorOutputFormat m_OutFormat;
        csiSensorOutputResolution m_OutResolution;
        DWORD m_dwFramerate;

        BOOL m_bCsi0Disable;
        BOOL m_bCsi1Disable;
};

//------------------------------------------------------------------------------
// Functions



#endif  // __CSICLASS_H__
