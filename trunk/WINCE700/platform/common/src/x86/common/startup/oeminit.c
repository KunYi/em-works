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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

Abstract:  
   This file implements the NK kernel interfaces for firmware interrupt
  support on the CEPC.
 
Functions:


Notes: 

--*/

#include <windows.h>
#include <oal.h>
#include <nkintr.h>
#include <x86boot.h>

#include "pehdr.h"
#include "romldr.h"
#include "romxip.h"


#define NOT_FIXEDUP ((DWORD*)-1)

//------------------------------------------------------------------------------
//
// dwOEMDrWatsonSize
//
// Global variable which specify DrWatson buffer size. It can be fixed
// in config.bib via FIXUPVAR.
//
#define DR_WATSON_SIZE_NOT_FIXEDUP  ((DWORD)-1)
DWORD dwOEMDrWatsonSize = DR_WATSON_SIZE_NOT_FIXEDUP;

void  PCIInitBusInfo (void);
DWORD OEMPowerManagerInit(void);
void  InitClock(void);
void  x86InitMemory (void);
void  x86InitPICs(void);
void  x86RebootInit(void);
void  RTCPostInit();
void  QPCPostInit();

extern LPCWSTR g_pPlatformManufacturer;
extern LPCWSTR g_pPlatformName;
static BOOL g_fPostInit;
void OALMpInit (void);

//------------------------------------------------------------------------------
//
// x86InitRomChain
//
// Initialize the rom chain
//
static void x86InitRomChain()
{
    // Added for MultiXIP stuff
    static      ROMChain_t  s_pNextRom[MAX_ROM] = {0};
    DWORD       dwRomCount = 0;
    DWORD       dwChainCount = 0;
    DWORD *     pdwCurXIP;
    DWORD       dwNumXIPs;
    PXIPCHAIN_ENTRY pChainEntry = NULL;
    static DWORD *pdwXIPLoc = NOT_FIXEDUP;

#ifdef DEBUG
    TCHAR       szXIPName[XIP_NAMELEN];
    int         i;
#endif

#ifdef DEBUG
    NKOutputDebugString (TEXT("Looking for rom chain\n"));
#endif

    if(pdwXIPLoc == NOT_FIXEDUP)
    {
#ifdef DEBUG
        NKOutputDebugString (TEXT("Rom chain NOT found\n"));
#endif
        return;  // no chain or not fixed up properly
    }

#ifdef DEBUG
    NKOutputDebugString (TEXT("Rom chain found\n"));
#endif


    // set the top bit to mark it as a virtual address
    pdwCurXIP = (DWORD*)(((DWORD)pdwXIPLoc) | 0x80000000);

    // first DWORD is number of XIPs
    dwNumXIPs = (*pdwCurXIP);

    if(dwNumXIPs > MAX_ROM)
    {
        NKOutputDebugString (TEXT("ERROR: Number of XIPs exceeds MAX\n"));
        return;
    }

    pChainEntry = (PXIPCHAIN_ENTRY)(pdwCurXIP + 1);

    while(dwChainCount < dwNumXIPs)
    {
        if ((pChainEntry->usFlags & ROMXIP_OK_TO_LOAD) &&  // flags indicates valid XIP
            *(const DWORD*)(((DWORD)(pChainEntry->pvAddr)) + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
        {
            s_pNextRom[dwRomCount].pTOC = *(ROMHDR **)(((DWORD)(pChainEntry->pvAddr)) + ROM_SIGNATURE_OFFSET + 4);
            s_pNextRom[dwRomCount].pNext = NULL;

#ifdef DEBUG
            NKOutputDebugString ( _T("XIP found: ") );

            for (i = 0; (i< (XIP_NAMELEN-1)) && (pChainEntry->szName[i] != '\0'); ++i)
            {
                szXIPName[i] = (TCHAR)pChainEntry->szName[i];
            }
            szXIPName[i] = TEXT('\0');
            NKOutputDebugString ( szXIPName );

            NKOutputDebugString ( _T("\n") );
#endif

            if (dwRomCount != 0)
            {
                s_pNextRom[dwRomCount-1].pNext = &s_pNextRom[dwRomCount];
            }
            else
            {
                OEMRomChain = s_pNextRom;
            }
            dwRomCount++;
        }
        else
        {
            NKOutputDebugString ( _T("Invalid XIP found\n") );
        }

        ++pChainEntry;
        dwChainCount++;
    }

#ifdef DEBUG
    {
        ROMChain_t         *pchain = OEMRomChain;

        NKOutputDebugString ( _T("chain contents...\n") );
        while(pchain){
            NKOutputDebugString ( _T("found item\n") );
            pchain = pchain->pNext;
        }
    }
#endif
    
}

//------------------------------------------------------------------------------
//
// OEMInit
//
// OEMInit is called by the kernel after it has performed minimal
// initialization. Interrupts are disabled and the kernel is not
// ready to handle exceptions. The only kernel service available
// to this function is HookInterrupt. This should be used to
// install ISR's for all the hardware interrupts to be handled by
// the firmware. Note that ISR's must be installed for any interrupt
// that is to be routed to a device driver - otherwise the
// InterruptInitialize call from the driver will fail.
//
void OEMInit()
{   
    // Set up the debug zones according to the fix-up variable initialOALLogZones
    OALLogSetZones(initialOALLogZones);

    OALMSG(OAL_FUNC, (L"+OEMInit\r\n"));

    // initialize interrupts
    OALIntrInit ();

    // initialize PIC
    x86InitPICs();

    // Initialize PCI bus information
    PCIInitBusInfo ();

    // starts KITL (will be a no-op if KITLDLL doesn't exist)
    KITLIoctl (IOCTL_KITL_STARTUP, NULL, 0, NULL, 0, NULL);
    
#ifdef DEBUG

    // Instead of calling OEMWriteDebugString directly, call through exported
    // function pointer.  This will allow these messages to be seen if debug
    // message output is redirected to Ethernet or the parallel port.  Otherwise,
    // lpWriteDebugStringFunc == OEMWriteDebugString.    
    NKOutputDebugString (TEXT("CEPC Firmware Init\r\n"));

#endif

    // sets the global platform manufacturer name and platform name
    g_oalIoCtlPlatformManufacturer = g_pPlatformManufacturer;
    g_oalIoCtlPlatformName = g_pPlatformName;

    OEMPowerManagerInit();
    
    // initialize clock
    InitClock();

    // initialize memory (detect extra ram, MTRR/PAT etc.)
    x86InitMemory ();

    // Reserve 128kB memory for Watson Dumps
    dwNKDrWatsonSize = 0;
    if (dwOEMDrWatsonSize != DR_WATSON_SIZE_NOT_FIXEDUP) 
    {
        dwNKDrWatsonSize = dwOEMDrWatsonSize;
    }

    x86RebootInit();
    x86InitRomChain();

    OALMpInit ();

#ifdef DEBUG
    NKOutputDebugString (TEXT("Firmware Init Done.\r\n"));
#endif

    OALMSG(OAL_FUNC, (L"-OEMInit\r\n"));
}

//------------------------------------------------------------------------------
//
// IsAfterPostInit
//
// TRUE once the kernel has performed a more full featured init. Critical Sections can now be used
//
BOOL IsAfterPostInit() 
{ 
    return g_fPostInit; 
}

//------------------------------------------------------------------------------
//
// x86IoCtlPostInit
//
// Called once the kernel has performed a more full featured init. Critical Sections can now be used
//
BOOL x86IoCtlPostInit(
                      UINT32 code, 
                      __in_bcount(nInBufSize) const void * lpInBuf, 
                      UINT32 nInBufSize, 
                      __out_bcount(nOutBufSize) const void * lpOutBuf, 
                      UINT32 nOutBufSize, 
                      __out UINT32 * lpBytesReturned
                      ) 
{
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(lpOutBuf);
    UNREFERENCED_PARAMETER(nOutBufSize);

    if (lpBytesReturned) 
    {
        *lpBytesReturned = 0;
    }

    RTCPostInit();
    QPCPostInit();

    g_fPostInit = TRUE;

    return TRUE;
}
