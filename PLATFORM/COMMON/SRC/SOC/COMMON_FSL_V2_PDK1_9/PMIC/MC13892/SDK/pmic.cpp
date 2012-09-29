//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include "ceddk.h"
#pragma warning(pop)

#include "pmic.h"


//-----------------------------------------------------------------------------
// Global Variables
HANDLE hPMI;
LONG   RegRefCount[VMAX];
LONG   GateRefCount[2];
#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

DBGPARAM dpCurSettings = {
    _T("PMIC"),
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
        TEXT("Info"), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN // ulZoneMask
};

#endif  // DEBUG



//------------------------------------------------------------------------------
//
// Function: InitPMICSys
//
// This function initializes PMIC subsystems.
//
// Parameters:
//          none.
// Returns:
//              none.
//------------------------------------------------------------------------------
void InitPMICSys(void)
{
    memset(&RegRefCount, 0, sizeof(RegRefCount));
    memset(&GateRefCount, 0, sizeof(GateRefCount));
 
    if (PmicADCInit()!=PMIC_SUCCESS)
    {
        ERRORMSG(1, (_T("InitPMICSys: PmicADCInit failed.\r\n")));
    }
}


//------------------------------------------------------------------------------
//
// Function: DeinitPMICSys
//
// This function deinitializes PMIC subsystems.
//
// Parameters:
//          none.
// Returns:
//              none.
//------------------------------------------------------------------------------
void DeinitPMICSys(void)
{
    PmicADCDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the PMIC DDK module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the PMIC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
extern "C"
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HMODULE)hInstDll);
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS ATTACH TO PMIC SDK *****\r\n")));

            DisableThreadLibraryCalls((HMODULE) hInstDll);

            hPMI = CreateFile(TEXT("PMI1:"),
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_RANDOM_ACCESS,
                              NULL);
            if ((hPMI == NULL) || (hPMI == INVALID_HANDLE_VALUE))
            {
                ERRORMSG(TRUE, (_T("Failed in createFile() for PMI1:\r\n")));
                return FALSE;
            }

            InitPMICSys();
            break;

        case DLL_PROCESS_DETACH:
            if ((hPMI != NULL) && (hPMI != INVALID_HANDLE_VALUE))
            {
                CloseHandle(hPMI);
            }
            DeinitPMICSys();

            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM PMIC SDK *****\r\n")));
            break;
    }

    // return TRUE for success
    return TRUE;
}
