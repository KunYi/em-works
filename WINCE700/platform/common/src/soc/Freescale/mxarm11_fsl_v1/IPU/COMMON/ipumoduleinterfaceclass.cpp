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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
//  Copyright (C) 2003, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  IpuModuleInterfaceClass.cpp
//
//  Implementation of CMOS Sensor Interface Product Device Driver
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <NKIntr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "mxarm11.h"
#include "IpuModuleInterfaceClass.h"
#include "cameradbg.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define IPU_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define IPU_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: IpuModuleInterfaceClass
//
// IpuModuleInterfaceClass constructor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
IpuModuleInterfaceClass::IpuModuleInterfaceClass()
{
    static BOOL isInitialized = FALSE;

    IPU_FUNCTION_ENTRY();

    // We only want to initialize the static variables once
    if (!isInitialized)
    {
        // Allocate IPU driver data structures.
        if (!IpuAlloc())
        {
            DEBUGMSG(ZONE_ERROR, (_T("IPU common Init:  Allocation failed!\r\n")));
        }

        // Get mutex for access to IPU registers.
        m_hIpuMutex = CreateMutex(NULL, FALSE, IPU_COMMON_MUTEX);

        if (m_hIpuMutex == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (_T("IPU common Init:  CreateMutex failed!\r\n")));
        }
        
        isInitialized = TRUE;
    }

    IPU_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: ~IpuModuleInterfaceClass
//
// IpuModuleInterfaceClass destructor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
IpuModuleInterfaceClass::~IpuModuleInterfaceClass()
{
    IPU_FUNCTION_ENTRY();

    // Unmap IPU register pointer
    IpuDealloc();

    if (m_hIpuMutex != NULL)
    {
        CloseHandle(m_hIpuMutex);
        m_hIpuMutex = NULL;
    }

    IPU_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  IpuAlloc
//
// This function allocates the data structures required for interaction
// with the IPU hardware.
//
// Parameters:
//      None.
//
// Returns:  
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL IpuModuleInterfaceClass::IpuAlloc(void)
{

    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (m_pIPU == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_IPU;

        // Map peripheral physical address to virtual address
        m_pIPU = (PCSP_IPU_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_REGS), 
            FALSE); 
        
        // Check if virtual mapping failed
        if (m_pIPU == NULL)
        {
            DEBUGMSG(ZONE_ERROR, 
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }
        
    rc = TRUE;

cleanUp:    
    // If initialization failed, be sure to clean up
    if (!rc) IpuDealloc();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  IpuDealloc
//
// This function deallocates the data structures required for interaction
// with the IPU hardware.
//
// Parameters:
//      None.
//
// Returns:  
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void IpuModuleInterfaceClass::IpuDealloc(void)
{
    IPU_FUNCTION_ENTRY();
    
    // Unmap peripheral registers
    if (m_pIPU)
    {
        MmUnmapIoSpace(m_pIPU, sizeof(CSP_IPU_REGS));
        m_pIPU = NULL;
    }
    
    IPU_FUNCTION_EXIT();
}