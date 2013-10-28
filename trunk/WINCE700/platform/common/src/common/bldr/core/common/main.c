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
#include <bootCore.h>
#include <pehdr.h>
#include <romldr.h>
#include <boot.h>

//------------------------------------------------------------------------------
//  Global variables

ROMHDR* 
volatile
const
pTOC = (ROMHDR *)-1; 

DWORD
BootStackSize = 0x10000;

uint32_t 
g_dwOEMUnsupportedNICs[MAX_OEM_UNSUPPORTED_NICS] = {0};

uint32_t 
g_dwOEMUnsupportedNICCount = 0;

//------------------------------------------------------------------------------
//  C++ initializers

typedef void (__cdecl *_PVFV)();

#pragma data_seg(".CRT$XCA")
static
_PVFV 
__xc_a[] = { NULL };

#pragma data_seg(".CRT$XCZ")
static
_PVFV 
__xc_z[] = { NULL };

#pragma data_seg()

#pragma comment(linker, "-merge:.CRT=.data")

//------------------------------------------------------------------------------
//  External Functions

void
BootJumpTo(
    uint32_t address
    );

VOID
BootHeapInit(
    );

//------------------------------------------------------------------------------
//  Local Functions

static
bool_t
SetupCopySection(
    );

VOID
SetupStaticCppObjects(
    );

//------------------------------------------------------------------------------
//
//  Function:  BootMain
//
void
BootMain(
    )
{
    void *pContext = NULL;
    uint32_t state, address, index;
    
    // Setup global variables
    if (!SetupCopySection()) goto powerOff;

    // Initialize memory heap
    BootHeapInit();

    // Call static C++ constructors
    SetupStaticCppObjects();

    // Call OEM init function
    pContext = OEMBootInit();
    if (pContext == NULL) goto powerOff;

    // Call OEM load function until run state is returned
    state = BOOT_STATE_POWERON;
    do 
        {
        state = OEMBootLoad(pContext, state);
        if (state == BOOT_STATE_FAILURE) goto powerOff;
        }
    while (state != BOOT_STATE_RUN);

    // Call OEM to prepare for image run
    address = OEMBootRun(pContext);

    // Jump to physical address
    BootJumpTo(address);
    
powerOff:
    // Alert the user to any unsupported NICs before we power off the system.
    for (index=0; index < g_dwOEMUnsupportedNICCount; index++)
    {
        BootLog(L"Network device %04x:%04x is not supported.\r\n",
                (g_dwOEMUnsupportedNICs[index] & 0xffff), 
                (g_dwOEMUnsupportedNICs[index] >> 16));
    }

    OEMBootPowerOff(pContext);
    for(;;);
}

//------------------------------------------------------------------------------
//
//  Function:  SetupCopySection
//
//  Copies image's copy section data (initialized globals) to the correct
//  fix-up location.  Once completed, initialized globals are valid.
//
bool_t
SetupCopySection(
    )
{
    bool_t rc = false;
    uint32_t loop, count;
    COPYentry *pCopyEntry;
    const uint8_t *pSrc;
    uint8_t *pDst;


    if (pTOC == (ROMHDR *const) -1) goto cleanUp;

    pCopyEntry = (COPYentry *)pTOC->ulCopyOffset;
    for (loop = 0; loop < pTOC->ulCopyEntries; loop++)
        {
        count = pCopyEntry->ulCopyLen;
        pDst = (uint8_t*)pCopyEntry->ulDest;
        pSrc = (uint8_t*)pCopyEntry->ulSource; 
        memcpy(pDst, pSrc, count);
        pDst += count;
        count = pCopyEntry->ulDestLen - pCopyEntry->ulCopyLen;
        memset(pDst, 0, count);
        }

    rc = true;

cleanUp:    
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  SetupStaticCppObjects
//
//  This function calls constructors for static C++ objects.
//
void
SetupStaticCppObjects(
    )
{
    _PVFV* pfn = __xc_a;

    pfn++;
    while (pfn < __xc_z)
        {
        if (*pfn != NULL) (**pfn)();
        pfn++;
        }
}

//------------------------------------------------------------------------------
//
//  Function:  atexit
//
//  We must have this fake function to avoid C library implementation be
//  pull in by some other code. In reality we don't need do anything there
//  in most cases.
//
int 
__cdecl 
atexit(
    void (__cdecl *pfn)(void)
    )
{
    UNREFERENCED_PARAMETER(pfn);
    return 0;
}

//------------------------------------------------------------------------------

