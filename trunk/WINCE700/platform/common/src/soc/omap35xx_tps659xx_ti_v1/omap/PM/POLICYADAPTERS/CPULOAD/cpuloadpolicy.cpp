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
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File:  cpuloadpolicy.cpp
//


//******************************************************************************
//  #INCLUDES
//******************************************************************************
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <dvfs.h>
#include <ceddkex.h>
#include <omap35xx.h>

//-----------------------------------------------------------------------------
#define POLICY_CONTEXT_COOKIE       'cpul'

#define CONSTRAINT_ID_DOMAIN        L"PWRDOM"
#define CONSTRAINT_ID_DVFS          L"DVFS"

//-----------------------------------------------------------------------------
//  local structures

typedef enum {
    Period = 0,
    RoofOpm,
    FloorOpm,
    HighT,
    LowT,
    ThreadPriority,
    BootOpm,
    BootEventName,
    BootTimeout,
    MpuActiveState,
    Total,
} Registry_e;

typedef struct
{
    HANDLE                          hDvfsConstraint;
    HANDLE                          hDomainConstraint;
    HANDLE                          hCpuLoadThread;
    HANDLE                          hCpuLoadEvent;
    DWORD                           dwMpuActiveState;
    DWORD                           dwPeriod;
    DWORD                           dwRoofOpm;  
    DWORD                           dwFloorOpm;
    DWORD                           dwHighPercent;
    DWORD                           dwLowPercent;
    DWORD                           dwThreadPriority;
    DWORD                           dwBootOpm;
    DWORD                           dwBootTimeout;
    WCHAR                           szBootEventName[MAX_PATH];
    HANDLE                          hBootEvent;
} CpuLoadPolicyInfo_t;

typedef struct
{
    LPTSTR                          lpRegname;
    DWORD                           dwSize;
} REGISTRY_PARAMETERS;

//-----------------------------------------------------------------------------
//  local variables
static CpuLoadPolicyInfo_t          s_CpuLoadPolicyInfo;

REGISTRY_PARAMETERS                 g_reg[]=
{
    {
        L"MonitorPeriod",   sizeof(DWORD)
    },{
        L"RoofOpm",         sizeof(DWORD)
    },{
        L"FloorOpm",        sizeof(DWORD)
    },{
        L"HighThreshold",   sizeof(DWORD)
    },{
        L"LowThreshold",    sizeof(DWORD)
    },{
        L"priority256",     sizeof(DWORD)
    },{
        L"BootOpm",         sizeof(DWORD)
    },{
        L"BootEventName",   sizeof(WCHAR[MAX_PATH])
    },{
        L"BootTimeout",     sizeof(DWORD)
    },{
        L"MpuActiveState",  sizeof(DWORD)
    }
};
    

//-----------------------------------------------------------------------------
// 
//  Function:  GetRegVal
//
//  Reads policy values from the registry
//
BOOL GetRegVal(
        WCHAR *szRegPath
        )
{
    BOOL rc=FALSE;
    HKEY hOpenedKey=NULL;

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                        szRegPath,
                                        0,
                                        0,
                                        &hOpenedKey))
        {
        goto cleanUp;
        }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[Period].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwPeriod,
                                        &g_reg[Period].dwSize))
        {
        goto closeKey;
        }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[RoofOpm].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwRoofOpm,
                                        &g_reg[RoofOpm].dwSize))
        {
        goto closeKey;
        }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[FloorOpm].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwFloorOpm,
                                        &g_reg[FloorOpm].dwSize))
        {
        goto closeKey;
        }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[HighT].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwHighPercent,
                                        &g_reg[HighT].dwSize))
        {
        goto closeKey;
        }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[LowT].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwLowPercent,
                                        &g_reg[LowT].dwSize))
        {
        goto closeKey;
        }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[ThreadPriority].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwThreadPriority,
                                        &g_reg[ThreadPriority].dwSize))
        {
        goto closeKey;
        }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[BootOpm].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwBootOpm,
                                        &g_reg[BootOpm].dwSize))
        {
        goto closeKey;
        }
    
    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[BootEventName].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.szBootEventName,
                                        &g_reg[BootEventName].dwSize))
        {
        goto closeKey;
        }

    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[BootTimeout].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwBootTimeout,
                                        &g_reg[BootTimeout].dwSize))
        {
        s_CpuLoadPolicyInfo.dwBootTimeout = INFINITE;
        }
    

    if (ERROR_SUCCESS != RegQueryValueEx(hOpenedKey,
                                        g_reg[MpuActiveState].lpRegname,
                                        NULL,
                                        NULL,
                                        (UCHAR *)&s_CpuLoadPolicyInfo.dwMpuActiveState,
                                        &g_reg[MpuActiveState].dwSize))
        {
        s_CpuLoadPolicyInfo.dwMpuActiveState = D2;
        }
    
    rc = TRUE;

closeKey:
        
    if(ERROR_SUCCESS != RegCloseKey(hOpenedKey))
        {
        rc = FALSE;
        goto cleanUp;
        }
    
cleanUp:

    return rc;
}


//-----------------------------------------------------------------------------
// 
//  Function:  CpuLoadThreadFn
//
//  Contains the Cpu load calculation and Opm change desition functionality
//
DWORD WINAPI CpuLoadThreadFn(LPVOID pvParam)
{
    DWORD dwIdleLast = 0;
    DWORD dwTickLast = 0;
    DWORD dwIdle;
    DWORD dwTick;
    DWORD dwCpuload = 0;
    DWORD dwOpm = 0;    
    POWERDOMAIN_CONSTRAINT_INFO domainConstraint;
    DWORD currentOpm = s_CpuLoadPolicyInfo.dwBootOpm;
    DWORD timeOut = s_CpuLoadPolicyInfo.dwBootTimeout;
    HANDLE hCpuLoadEvent = s_CpuLoadPolicyInfo.hCpuLoadEvent;

    // Wait until image boot process finishes
    //
    if(s_CpuLoadPolicyInfo.hBootEvent != NULL)
        {
        //Close handle to the boot event
        WaitForSingleObject(s_CpuLoadPolicyInfo.hBootEvent, timeOut);
        CloseHandle(s_CpuLoadPolicyInfo.hBootEvent); 
        }

    // UNDONE:
    // make this configurable via registry
    // Delay to allow graphical enviroment to be initialized
    Sleep(5000);

    while (TRUE)
        {
        dwIdle = GetIdleTime();
        dwTick = GetTickCount();
        dwCpuload =(100-((100*(dwIdle - dwIdleLast))/(dwTick - dwTickLast)));
        dwIdleLast = dwIdle;
        dwTickLast = dwTick;
        
        if (dwCpuload >= s_CpuLoadPolicyInfo.dwHighPercent &&
                       dwOpm < s_CpuLoadPolicyInfo.dwRoofOpm)
            {
            dwOpm = s_CpuLoadPolicyInfo.dwRoofOpm;
            }
        else if(dwCpuload < s_CpuLoadPolicyInfo.dwLowPercent &&
                          dwOpm > s_CpuLoadPolicyInfo.dwFloorOpm)
            {
            dwOpm--;
            }

        // UNDONE:
        // this should be more dynamic
        timeOut = s_CpuLoadPolicyInfo.dwPeriod;

        // update the operating mode on transition of operating point
        if (currentOpm != dwOpm)
            {        
            // update operating mode
            PmxUpdateConstraint(s_CpuLoadPolicyInfo.hDvfsConstraint, 
                        CONSTRAINT_MSG_DVFS_REQUEST, 
                        (void*)&dwOpm, 
                        sizeof(DWORD)
                        );

            // check if transitioning to floor levels
            if (dwOpm == s_CpuLoadPolicyInfo.dwFloorOpm)
                {
                domainConstraint.powerDomain = POWERDOMAIN_MPU;
                domainConstraint.state = CONSTRAINT_STATE_NULL;
                domainConstraint.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);

                // allow mpu to go to floor level
                PmxUpdateConstraint(s_CpuLoadPolicyInfo.hDomainConstraint, 
                        CONSTRAINT_MSG_POWERDOMAIN_REQUEST, 
                        (void*)&domainConstraint, 
                        sizeof(POWERDOMAIN_CONSTRAINT_INFO)
                        );
                }
            else if (currentOpm == s_CpuLoadPolicyInfo.dwFloorOpm)
                {
                domainConstraint.powerDomain = POWERDOMAIN_MPU;
                domainConstraint.state = s_CpuLoadPolicyInfo.dwMpuActiveState;
                domainConstraint.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);

                // put mpu floor level to an active level
                PmxUpdateConstraint(s_CpuLoadPolicyInfo.hDomainConstraint, 
                        CONSTRAINT_MSG_POWERDOMAIN_REQUEST, 
                        (void*)&domainConstraint, 
                        sizeof(POWERDOMAIN_CONSTRAINT_INFO)
                        );
                }

            // update local variable
            currentOpm = dwOpm;
            }

        // wait for designated timeout
        WaitForSingleObject(hCpuLoadEvent, timeOut);
        }
}

//-----------------------------------------------------------------------------
// 
//  Function:  CPULD_InitPolicy
//
//  Policy  initialization
//

HANDLE
CPULD_InitPolicy(
    _TCHAR const *szContext
    )
{
    DWORD dwLevel;
    POWERDOMAIN_CONSTRAINT_INFO domainConstraint;

    // initializt global structure
    memset(&s_CpuLoadPolicyInfo, 0, sizeof(CpuLoadPolicyInfo_t));       

    // Read registries
    if (GetRegVal((WCHAR*)szContext) == FALSE) return NULL;
    
    // Open boot named event
    s_CpuLoadPolicyInfo.hBootEvent = CreateEvent(NULL, 
                                        TRUE, 
                                        FALSE, 
                                        s_CpuLoadPolicyInfo.szBootEventName
                                        ); 

    // Create an event to signal the cpu load monitor thread
    s_CpuLoadPolicyInfo.hCpuLoadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Set boot opm value
    dwLevel = s_CpuLoadPolicyInfo.dwBootOpm;

    // Obtain DVFS constraint handler 
    s_CpuLoadPolicyInfo.hDvfsConstraint = PmxSetConstraintById(
                                    CONSTRAINT_ID_DVFS, 
                                    CONSTRAINT_MSG_DVFS_REQUEST, 
                                    (void*)&dwLevel, 
                                    sizeof(DWORD)
                                    );

    // Get handle to domain constraint
    // initialize domain constraint
    domainConstraint.powerDomain = POWERDOMAIN_MPU;
    domainConstraint.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);
    domainConstraint.state = s_CpuLoadPolicyInfo.dwMpuActiveState;
    
    s_CpuLoadPolicyInfo.hDomainConstraint = PmxSetConstraintById(
                                    CONSTRAINT_ID_DOMAIN, 
                                    CONSTRAINT_MSG_POWERDOMAIN_REQUEST, 
                                    (void*)&domainConstraint, 
                                    sizeof(POWERDOMAIN_CONSTRAINT_INFO)
                                    );
    

    // Start running the thread that will check for the cpu load
    s_CpuLoadPolicyInfo.hCpuLoadThread = CreateThread(NULL, 
                                            0,
                                            CpuLoadThreadFn,
                                            NULL,
                                            0,
                                            NULL
                                            );
    
    SetThreadPriority(s_CpuLoadPolicyInfo.hCpuLoadThread,
                           s_CpuLoadPolicyInfo.dwThreadPriority
                           );

    return (HANDLE)&s_CpuLoadPolicyInfo;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  CPULD_DeinitPolicy
//
//  Policy uninitialization
//
BOOL
CPULD_DeinitPolicy(
    HANDLE hPolicyAdapter
    )
{
    BOOL rc = FALSE;

    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_CpuLoadPolicyInfo) goto cleanUp;

    // release all resoureces   
    if (s_CpuLoadPolicyInfo.hDvfsConstraint != NULL)
        {
        PmxReleaseConstraint(s_CpuLoadPolicyInfo.hDvfsConstraint);
        }

    if (s_CpuLoadPolicyInfo.hDomainConstraint != NULL)
        {
        PmxReleaseConstraint(s_CpuLoadPolicyInfo.hDomainConstraint);
        }

    if (s_CpuLoadPolicyInfo.hDomainConstraint != NULL)
        {
        PmxReleaseConstraint(s_CpuLoadPolicyInfo.hDomainConstraint);
        }
    
    if (s_CpuLoadPolicyInfo.hCpuLoadThread != NULL)
        {
        TerminateThread(s_CpuLoadPolicyInfo.hCpuLoadThread,0);
        }

    if (s_CpuLoadPolicyInfo.hBootEvent != NULL)
        {
        CloseHandle(s_CpuLoadPolicyInfo.hBootEvent);
        }

    if (s_CpuLoadPolicyInfo.hCpuLoadEvent != NULL)
        {
        CloseHandle(s_CpuLoadPolicyInfo.hCpuLoadEvent);
        }

    // clear structures
    memset(&s_CpuLoadPolicyInfo, 0, sizeof(CpuLoadPolicyInfo_t));
    
    rc = TRUE;

cleanUp:
    return rc;
} 

//-----------------------------------------------------------------------------

