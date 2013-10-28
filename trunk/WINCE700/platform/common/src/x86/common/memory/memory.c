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

#include <windows.h>
#include <pc.h>
#include <msr.h>
#include <oal.h>
#include <acpi.h>
#include <vmlayout.h>
#include <bldver.h>
#include <x86Boot.h>

static DWORD g_dwCPUFeatures;
static int g_nMtrrCnt;
static int g_nMtrrInuse;

//-----------------------------------------------------------------------------
//
//  WriteMtrrRegPair
//
//  
//
static void WriteMtrrRegPair(
                             int ixReg, 
                             const LARGE_INTEGER *pliPhysBase, 
                             const LARGE_INTEGER *pliPhysMask
                             )
{
    OALMSG(OAL_MEMORY, (L"Setting MTRR reg-pair:%d with 0x%8.8lx%8.8lx, 0x%8.8lx%8.8lx\r\n", ixReg,
        pliPhysBase->HighPart, pliPhysBase->LowPart| MTRR_TypeWriteCombining, 
        pliPhysMask->HighPart, pliPhysMask->LowPart));

    // Per Intel MTRR document, we need to disable/enable interrupt, flush cache and TLB, and disable MTRR before/after
    // changing MTRR registers. However, since we use this only for non-memory area (video frame buffer) during 
    // initialization, both cache and TLB shouldn't have any references to them. We'll need to revisit this function
    // if the assumption is no longer true.
    //
    // pre_mtrr_change();
    NKwrmsr(MSRAddr_MTRRphysBase0 + (2 * ixReg), pliPhysBase->HighPart, pliPhysBase->LowPart | MTRR_TypeWriteCombining);
    NKwrmsr(MSRAddr_MTRRphysMask0 + (2 * ixReg), pliPhysMask->HighPart, pliPhysMask->LowPart);
    // post_mtrr_change();

}

#ifdef DEBUG
//-----------------------------------------------------------------------------
//
//  DumpMTRR
//
//  
//
static void DumpMTRR()
{
    int i;
    DWORD dwHi, dwLo;
    for (i = 0; i < g_nMtrrCnt; i ++) {
        NKrdmsr (MSRAddr_MTRRphysMask0 + 2*i, &dwHi, &dwLo);
        NKDbgPrintfW (L"MTRR PhysMask%d: %8.8lx%8.8lx\r\n", i, dwHi, dwLo);
        NKrdmsr (MSRAddr_MTRRphysBase0 + 2*i, &dwHi, &dwLo);
        NKDbgPrintfW (L"MTRR PhysBase%d: %8.8lx%8.8lx\r\n", i, dwHi, dwLo);
    }
}
#endif

//-----------------------------------------------------------------------------
//
//  LSB
//
//  return the least significant non-zero bit
//
static __inline DWORD LSB(
                          DWORD x
                          )
{
    return x & (0-x);
}

//-----------------------------------------------------------------------------
//
//  MSB
//
//  return the most significant non-zero bit
//
static __inline DWORD MSB(
                          DWORD x
                          )
{
    DWORD msb = x;
    while (x &= (x-1)) 
    {
        msb = x;
    }
    return msb;
}

//-----------------------------------------------------------------------------
//
// CheckExistingMTRR: check if the requested range overlap with existing ranges
//
// returns: MTRR_ALREADY_EXIST - if already exists and is of type PAGE_WRITECOMBINE
//          MTRR_NOT_EXIST     - if not existing and no overlapping with any existing ranges
//          MTRR_OVERLAPPED    - if overlapped with existing ranges (FAIL)
//
enum MTRR
{
    MTRR_ALREADY_EXIST,
    MTRR_NOT_EXIST,
    MTRR_OVERLAPPED
};
static int CheckExistingMTRR(
                             const LARGE_INTEGER *pliPhysBase, 
                             DWORD dwPhysSize
                             )
{
    int i;
    LARGE_INTEGER liReg = {0};

    DEBUGCHK (LSB (dwPhysSize) == dwPhysSize);       // dwPhysSize must be power of 2
    
    // loop through the MTTR register pairs
    for (i = 0; i < g_nMtrrCnt; i ++) 
    {
        NKrdmsr (MSRAddr_MTRRphysMask0 + 2*i, (DWORD*)&liReg.HighPart, &liReg.LowPart);
        
        if (liReg.LowPart & MTRRphysMask_ValidMask) 
        {
            // register is in use, check to see if it covers our request
            // NOTE: we're assuming all MTRR ranges are of size 2^^n
            //       and physbase is also 2^^n aligned
            DWORD dwRegSize = LSB (liReg.LowPart & -PAGE_SIZE);
            DWORD dwRegType;
            NKrdmsr (MSRAddr_MTRRphysBase0 + 2*i, (DWORD*)&liReg.HighPart, &liReg.LowPart);
            dwRegType = liReg.LowPart & MTRRphysBase_TypeMask;
            liReg.LowPart -= dwRegType;

            if ((pliPhysBase->QuadPart < liReg.QuadPart + dwRegSize) 
                && (pliPhysBase->QuadPart + dwPhysSize >= liReg.QuadPart)) 
            {
                // range overlapped - return MTRR_ALREADY_EXIST if fully contained and of the same type.
                //                  - otherwise return MTRR_OVERLAPPED
                return ((MTRR_TypeWriteCombining == dwRegType)
                        &&(pliPhysBase->QuadPart >= liReg.QuadPart)
                        && (pliPhysBase->QuadPart + dwPhysSize <= liReg.QuadPart + dwRegSize))
                    ?   MTRR_ALREADY_EXIST
                    :   MTRR_OVERLAPPED;
            }
        }
    }
    return MTRR_NOT_EXIST;
}

//-----------------------------------------------------------------------------
//
//  PATSetMemoryAttributes
//
//  PAT version of OEMSetMemoryAttributes
//
static BOOL PATSetMemoryAttributes(
                                   void* pVirtAddr,       // Virtual address of region
                                   void* pPhysAddrShifted,// PhysicalAddress >> 8 (to support up to 40 bit address)
                                   DWORD  cbSize,          // Size of the region
                                   DWORD  dwAttributes     // attributes to be set
                                   )
{
    UNREFERENCED_PARAMETER(pPhysAddrShifted);

    return (g_dwCPUFeatures & CPUID_PAT)
        ? NKVirtualSetAttributes (pVirtAddr, cbSize,
                                  x86_PAT_INDEX0,     // Index of first upper PAT entry
                                  x86_PAT_INDEX_MASK, // Mask of all PAT index bits
                                  &dwAttributes)
        : FALSE;

}

//-----------------------------------------------------------------------------
//
//  MTRRSetMemoryAttributes
//
//  MTRR version of OEMSetMemoryAttributes
//
static BOOL MTRRSetMemoryAttributes(
                                    const void* pVirtAddr,       // Virtual address of region
                                    const void* const pPhysAddrShifted,// PhysicalAddress >> 8 (to support up to 40 bit address)
                                    DWORD  cbSize,          // Size of the region
                                    DWORD  dwAttributes     // attributes to be set
                                    )
{
    UNREFERENCED_PARAMETER(pVirtAddr);
    UNREFERENCED_PARAMETER(dwAttributes);
    UNREFERENCED_PARAMETER(pPhysAddrShifted);

    // try MTRR if we physical address is know and MTRR is available
    if (g_nMtrrCnt && (PHYSICAL_ADDRESS_UNKNOWN != pPhysAddrShifted)) 
    {
        // MTRR supported
        LARGE_INTEGER liPhysMask;
        LARGE_INTEGER liPhysBase;
        DWORD cbOrigSize = cbSize;      // save the original size since we need to do it in 2-passes
        DWORD cbAlignedSize, dwAlignedBase;
        int   nRegNeeded = 0, n;

        liPhysBase.QuadPart = ((ULONGLONG) pPhysAddrShifted) << 8;   // real physical address

        // first: calculate number of register pairs needed
        // count MTRR registers needed
        for ( ; cbSize; cbSize -= cbAlignedSize, liPhysBase.QuadPart += cbAlignedSize) 
        {
            cbAlignedSize = MSB(cbSize);            // maximun alignment that can be satisfied
            if (liPhysBase.LowPart) 
            {
                dwAlignedBase = LSB (liPhysBase.LowPart); // least alignment required for base
                if (cbAlignedSize > dwAlignedBase) 
                {
                    cbAlignedSize = dwAlignedBase;
                }
            }
            n = CheckExistingMTRR (&liPhysBase, cbAlignedSize);

            if (MTRR_OVERLAPPED == n) 
            {
                // do not support overlapped with existing ranges
                OALMSG(OAL_MEMORY, (L"OEMSetMemoryAttributes: MTRR existing Range overlapped with the new request\r\n"));
                return FALSE;
            }
            if (MTRR_NOT_EXIST == n)
            {
                ++nRegNeeded;
            }
        }   

        OALMSG(OAL_MEMORY, (L"OEMSetMemoryAttributes: nRegNeeded = %d\r\n", nRegNeeded));
        
        // do we have enough registers left?
        if (g_nMtrrInuse + nRegNeeded > g_nMtrrCnt) 
        {
            // running out of MTRR registers
            OALMSG(OAL_MEMORY, (L"OEMSetMemoryAttributes: Run out of MTRR registers\r\n"));
            return FALSE;
        }

        if (!nRegNeeded) 
        {
            // no register needed, the range already exists with Write-Combine
            return TRUE;
        }

        // registers available, start allocating
        liPhysBase.QuadPart = ((ULONGLONG) pPhysAddrShifted) << 8;   // real physical address
        liPhysMask.HighPart = 0xF;          // size can never be > 4G, thus the high part of mask shold always be 0xf
        
        // update MTRR registers
        for (n = 0, cbSize = cbOrigSize; cbSize; cbSize -= cbAlignedSize, liPhysBase.QuadPart += cbAlignedSize) 
        {
            cbAlignedSize = MSB(cbSize);    // maximun alignment that can be satisfied
            if (liPhysBase.LowPart) 
            {
                dwAlignedBase = LSB (liPhysBase.LowPart); // least alignment required for base
                if (cbAlignedSize > dwAlignedBase) 
                {
                    cbAlignedSize = dwAlignedBase;
                }
            }

            if (MTRR_ALREADY_EXIST != CheckExistingMTRR (&liPhysBase, cbAlignedSize)) 
            {
                // find a free MTRR register
                do {
                    DWORD dummy, dwPhysMask;
                    NKrdmsr (MSRAddr_MTRRphysMask0 + 2*n, &dummy, &dwPhysMask);
                    if (!(MTRRphysMask_ValidMask & dwPhysMask)) 
                    {
                        // found
                        break;
                    }
                    n ++;
                } while (n < g_nMtrrCnt);

                DEBUGCHK (n < g_nMtrrCnt);
                liPhysMask.LowPart = (0-cbAlignedSize) | MTRRphysMask_ValidMask;
                WriteMtrrRegPair (n ++, &liPhysBase, &liPhysMask);
            }
            
        }

        g_nMtrrInuse += nRegNeeded;
        return TRUE;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
//
//  OEMSetMemoryAttributes
//
//  OEM function to change memory attributes that isn't supported by kernel.
//  Current implementaion only supports PAGE_WRITECOMBINE.
//
//  This function first try to use PAT, and then try MTRR if PAT isn't available.
//
static BOOL OEMSetMemoryAttributes(
                                   LPVOID pVirtAddr,       // Virtual address of region
                                   LPVOID pPhysAddrShifted,// PhysicalAddress >> 8 (to support up to 40 bit address)
                                   DWORD  cbSize,          // Size of the region
                                   DWORD  dwAttributes     // attributes to be set
                                   )
{
    if (PAGE_WRITECOMBINE ^ dwAttributes) 
    {
        OALMSG(OAL_MEMORY, (L"OEMSetMemoryAttributes: Only PAGE_WRITECOMBINE is supported\r\n"));
        return FALSE;
    }

    // I decided to try MTRR first since MTRR shows better perf improvement on my CEPC (64M vs. 48M).
    // Depending on platform, you might want to reverse the order if PAT has a better perf improvement.
    return MTRRSetMemoryAttributes (pVirtAddr, pPhysAddrShifted, cbSize, dwAttributes)
        || PATSetMemoryAttributes (pVirtAddr, pPhysAddrShifted, cbSize, dwAttributes);
}

//-----------------------------------------------------------------------------
//
//  InitializePATAndMTRR
//
//  Initialize memory type settings in the PAT (page attribute table).  The PAT
//  is a single 64-bit register which is a table of memory types.  Page table
//  entries index into the PAT via three bits: PCD, PWT, and PATi.  Each page 
//  uses the memory type stored in the PAT in that location.  By default the 
//  first four entries in the PAT (PATi=0) are initialized by the CPU on reboot 
//  to the legacy values that are implied by PCD and PWT for backward-
//  compatibility.  The remaining four PAT entries (PATi=1) may be programmed 
//  to any type desired.
//
//  In the i486 OAL we program the first upper PAT entry (PATi=1, PCD=0, PWT=0)
//  to be write-combined.  The remaining three upper PAT entries are currently
//  left as their default values and unused in the page table.
//
//  The PAT index is set on page table entries by OEMVirtualProtect, based on
//  the PAGE_NOCACHE, PAGE_x86_WRITETHRU, and PAGE_WRITECOMBINE flags passed
//  down by VirtualProtect.
//
static void InitializePATAndMTRR()
{
    DWORD dwHighPart, dwLowPart;

    OALMSG(OAL_MEMORY, (L"dwCPUFeatures = %8.8lx\r\n", g_dwCPUFeatures));

    // can't do anything if MSR is not supported
    if (!(g_dwCPUFeatures & CPUID_MSR)) 
    {
        return;
    }
    
    // Is PAT supported?
    if (g_dwCPUFeatures & CPUID_PAT) 
    {
        // PAT is supported
        
        // Read the current PAT values
        NKrdmsr(MSRAddr_PAT, &dwHighPart, &dwLowPart);

        // MUST leave dwLowPart as-is for backward compatibility!
        // Set the first entry in the high part to be write-combining
        dwHighPart = (dwHighPart & ~PAT_Entry0Mask) | PAT_TypeWriteCombining;

        // Write the new PAT values
        NKwrmsr(MSRAddr_PAT, dwHighPart, dwLowPart);
        
    }

    if (g_dwCPUFeatures & CPUID_MTRR) 
    {
        // MTRR is supported
        DWORD dummy, dwMTRR;

        // read the MTRRcap register and check if WC is supported
        NKrdmsr(MSRAddr_MTRRcap, &dummy, &dwMTRR);
        if (dwMTRR & MTRRcap_WriteCombineMask) 
        {

            int i;
            // WC is supported, continue to initialize the MTRR registers/globals
            g_nMtrrCnt = (int) (dwMTRR & MTRRcap_VarRangeRegisterCountMask);
            OALMSG(OAL_MEMORY, (L"g_nMtrrCnt = %d\r\n", g_nMtrrCnt));
            DEBUGCHK (g_nMtrrCnt);
#ifdef DEBUG
            DumpMTRR ();
#endif
            // figure out the variable entries that are already in use
            for (i = 0; i < g_nMtrrCnt; i ++) 
            {
                NKrdmsr (MSRAddr_MTRRphysMask0 + 2*g_nMtrrInuse, &dummy, &dwMTRR);
                OALMSG(OAL_MEMORY, (L"PhysMask%d: %8.8lx%8.8lx\r\n", i, dummy, dwMTRR));
                if (dwMTRR & MTRRphysMask_ValidMask)
                    g_nMtrrInuse ++;
            }
            OALMSG(OAL_MEMORY, (L"g_nMtrrInuse = %d\r\n", g_nMtrrInuse));

            // enable MTRR if it's not already enabled
            NKrdmsr (MSRAddr_MTRRdefType, &dummy, &dwMTRR);
            if (!(dwMTRR & MTRRdefType_EnableMask)) 
            {
                NKwrmsr (MSRAddr_MTRRdefType, 0, dwMTRR | MTRRdefType_EnableMask);
            }

        }
    }

    // initialize the pfnNKSetMemoryAttributes function pointer
    pfnOEMSetMemoryAttributes = OEMSetMemoryAttributes;
    
}

#define PHYS_DYNAMIC_RAM_START      0x20000000      // physical address at 512M

// defined in x86 OEMAddressTable
extern const OAL_ADDRESS_TABLE g_oalAddressTable[];

RAMTableEntry RamEntry[] = {
    { PHYS_DYNAMIC_RAM_START >> 8, 0xA0000000, 0 }, // up to 3G of RAM (kernel will auto-detect)
};

const RamTable g_oalRamTable = {
    MAKELONG (CE_MINOR_VER, CE_MAJOR_VER),
    1,
    RamEntry
};

static PCRamTable OEMGetRamTable (void)
{
    return &g_oalRamTable;
}

#define BYTE_TO_MBYTE(x)       ((x) >> 20)
#define MBYTE_TO_BYTE(x)       ((x) << 20)
//-----------------------------------------------------------------------------
//
//  x86InitMemory
//
//  
//
void x86InitMemory()
{
    // NOTE: 1st entry of g_oalAddressTable MUST be RAM
    const DWORD dwRamStartVIRT = g_oalAddressTable[0].CA;   // Virtual Address of RAM Start
    const DWORD dwRamStartPHYS = g_oalAddressTable[0].PA;   // Physical Address of RAM Start
    const DWORD dwOEMTotalRAM  = g_oalAddressTable[0].size; // total statically mapped RAM
    DWORD dwAcpiPHYS = AcpiFindTablesStartAddress ();       // physical address of ACPI Table
    DWORD dwRamEndPHYS;                                     // Physical Address of RAM End
    DWORD dwRamTop;

    // Use the PAT/MTRR to add write-combining support to VirtualProtect
    g_dwCPUFeatures = IdentifyCpu();
    InitializePATAndMTRR ();

    // calculate Physical Address of RAM End
    dwRamEndPHYS = dwRamStartPHYS + dwOEMTotalRAM;

    // oalAddressTable should NEVER specify RAM > 512M, RAM of physical address > 512M needs to be reported with RamTable (dynamic mapped).
    DEBUGCHK (dwRamEndPHYS <= PHYS_DYNAMIC_RAM_START);

    if (g_pX86Info->RamTop.QuadPart) {
        DWORD dwBootloaderRamPHYS;
        if (g_pX86Info->RamTop.HighPart) {
            ASSERT(g_pX86Info->RamTop.HighPart == 0); // only 4096 supported by CE
            g_pX86Info->RamTop.QuadPart = 0xffffffff;
        }
        OALMSG (OAL_INFO, (L" Bootloader reported %dMB\r\n", BYTE_TO_MBYTE(g_pX86Info->RamTop.LowPart) ));

        dwBootloaderRamPHYS = PAGEALIGN_DOWN (g_pX86Info->RamTop.LowPart);

        if (dwBootloaderRamPHYS > PHYS_DYNAMIC_RAM_START) {
            // Bootloader reported more than 512M, change the RamSize in RAM Table.
            RamEntry[0].RamSize = dwBootloaderRamPHYS - PHYS_DYNAMIC_RAM_START;
        } else if (dwRamEndPHYS > dwBootloaderRamPHYS) {
            dwRamEndPHYS = dwBootloaderRamPHYS;
        }
    } 
    
    dwRamTop = (dwRamEndPHYS < PHYS_DYNAMIC_RAM_START ? dwRamEndPHYS : RamEntry[0].RamSize + PHYS_DYNAMIC_RAM_START);

    if (dwRamTop > dwAcpiPHYS) {
        // this path will occur when booting from eboot
        if (dwAcpiPHYS > MBYTE_TO_BYTE(1)) { // check if there is ACPI table. 
            OALMSG (OAL_INFO, (L" Using ACPI location to determine RAM size\r\n"));
            OALMSG (OAL_INFO, (L" ACPI Tables found at 0x%x\r\n", dwAcpiPHYS));

            dwAcpiPHYS = PAGEALIGN_DOWN (dwAcpiPHYS);
            
            if (dwAcpiPHYS > PHYS_DYNAMIC_RAM_START) {
                // ACPI above 512M, change the RamSize in RAM Table.
                RamEntry[0].RamSize = dwAcpiPHYS - PHYS_DYNAMIC_RAM_START;
            } else if (dwRamEndPHYS > dwAcpiPHYS) {
                // if ACPI is below 512M, make sure we don't use RAM in that area
                dwRamEndPHYS = dwAcpiPHYS;
            }
        } else {
            if (0 < dwAcpiPHYS && dwAcpiPHYS <= MBYTE_TO_BYTE(1)) {
                OALMSG (OAL_INFO, (L" ACPI Tables are at the start of RAM (0x%x), and will not be used to determine RAM size.\r\n", dwAcpiPHYS));
            }
        }
    }

    dwRamTop = (dwRamEndPHYS < PHYS_DYNAMIC_RAM_START ? dwRamEndPHYS : RamEntry[0].RamSize + PHYS_DYNAMIC_RAM_START);

    OALMSG (OAL_INFO, (L" RAM reported to kernel %dMB\r\n", BYTE_TO_MBYTE (dwRamTop)));

    // Update MainMemoryEndAddress to max possible. Kernel will do auto-detection for us.
    g_pOemGlobal->dwMainMemoryEndAddress = dwRamStartVIRT + (dwRamEndPHYS - dwRamStartPHYS);

    if (PHYS_DYNAMIC_RAM_START == dwRamEndPHYS) {
        // if ram ends before PHYS_DYNAMIC_RAM_START, the image is built with IMGRAMXXX set, where
        // it wanted to use exactly that amount of RAM. Don't specify extra RAM in that case.
        // Otherwise update pfnGetOEMRamTable such that kernel can use the extra RAM above 512M.
        g_pOemGlobal->pfnGetOEMRamTable = OEMGetRamTable;
    }
    
}

//------------------------------------------------------------------------------
