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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------
#include <windows.h>
#include <cpuid.h>
#include <oal.h>

extern LPCWSTR g_pszDfltProcessorName;

BOOL x86IoCtlProcessorInfo (
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned
) {
    CPUID CpuInfo = {0};
    DWORD CpuMask = 0;
    BOOL CpuId;
    char VendorID[13];
    PPROCESSOR_INFO pProcInfo = (PPROCESSOR_INFO)lpOutBuf;

    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(code);

    if(lpBytesReturned)
    {
        *lpBytesReturned = sizeof(PROCESSOR_INFO);
    }

    if (!lpOutBuf && nOutBufSize > 0 ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!lpOutBuf || nOutBufSize < sizeof(PROCESSOR_INFO)) {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    memset(pProcInfo, 0, sizeof(PROCESSOR_INFO));

    // Use CPUID instruction to get information about platform
    __try {
        _asm {
            mov eax, 0
            cpuid
            mov dword ptr [VendorID], ebx
            mov dword ptr [VendorID+4], edx
            mov dword ptr [VendorID+8], ecx

            mov eax, 1
            cpuid
            and eax, 03fffh
            mov CpuInfo.Register, eax
        }

        VendorID[12] = '\0';

        CpuId = TRUE;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        CpuId = FALSE;
    }

    pProcInfo->wVersion = 1;
 
    if (CpuId) {
        int i;
        CPUSIG *CpuTable = NULL;
        int Entries = 0;

        // Check vendor ID for manufacturer
        if (!NKstrcmpiAandW(VendorID, AMD_VENDOR)) {          // We match "AuthenticAMD"
            CpuTable = AMDTable;
            Entries = AMD_ENTRIES;
            CpuMask = AMD_MASK;
            NKwcscpy (pProcInfo->szVendor, AMD_NAME);
        } else if (!NKstrcmpiAandW(VendorID, INTEL_VENDOR)) { // We match "GenuineIntel"
            CpuTable = IntelTable;
            Entries = INTEL_ENTRIES;
            CpuMask = INTEL_MASK;
            NKwcscpy (pProcInfo->szVendor, INTEL_NAME);
        } else if (!NKstrcmpiAandW(VendorID, GEODE_VENDOR)) { // We match "Geode by NSC"
            CpuTable = GeodeTable;
            Entries = GEODE_ENTRIES;
            CpuMask = GEODE_MASK;
            NKwcscpy (pProcInfo->szVendor, GEODE_NAME);
        } else if (!NKstrcmpiAandW(VendorID, VIA_VENDOR)) {   // We match "CentaurHauls"
            CpuTable = VIATable;
            Entries = VIA_ENTRIES;
            CpuMask = VIA_MASK;
            NKwcscpy (pProcInfo->szVendor, VIA_NAME);
        } else if (!NKstrcmpiAandW(VendorID, VORTEX86_VENDOR)) {   // We match "Vortex86 SoC"
            CpuTable = Vortex86Table;
            Entries = VORTEX86_ENTRIES;
            CpuMask = VORTEX86_MASK;
            NKwcscpy (pProcInfo->szVendor, VORTEX86_NAME);
        }

        // Look for CPU in appropriate table
        for (i  = 0; i < Entries; i++) {
            if ((CpuInfo.Register & CpuMask) == CpuTable[i].CpuId.Register) {
                NKwcscpy (pProcInfo->szProcessorName, CpuTable[i].ProcessorName);

                pProcInfo->wProcessorRevision = (WORD)CpuInfo.Field.Stepping;
                pProcInfo->dwInstructionSet = CpuTable[i].InstructionSet;

                return TRUE;
            }
        }
    }
        
    // No CPUID instruction or unknown processor, use default value
    NKwcscpy (pProcInfo->szProcessorName, g_pszDfltProcessorName);
    pProcInfo->dwInstructionSet = PROCESSOR_FLOATINGPOINT;
    return TRUE;
}
