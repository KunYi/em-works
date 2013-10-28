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

#ifndef _CPUID_H
#define _CPUID_H

#include <pkfuncs.h>

// CPUID bit-field for 32-bit processor signature
typedef union _CPUID {
    DWORD Register;                     // Whole register access
    struct {
        unsigned int Stepping : 4;      // Stepping ID
        unsigned int Model : 4;         // Model number
        unsigned int Family : 4;        // Family code
        unsigned int Type : 2;          // Processor Type
        unsigned int Reserved : 2;      // Reserved field
        unsigned int ExtModel : 4;      // Extended model
        unsigned int ExtFamily : 8;     // Extended family
    } Field;                            // Bit-field access                        
} CPUID;

// Processor signature structure
typedef struct _CPUSIG {
    CPUID CpuId;
    WCHAR ProcessorName[32];
    DWORD InstructionSet;
} CPUSIG;

// AMD processor signature table
CPUSIG AMDTable[] = {
    {0x0400, L"Am486 or Am5x86", 0},
    {0x0500, L"AMD-K5 Model 0", PROCESSOR_FLOATINGPOINT}, 
    {0x0510, L"AMD-K5 Model 1", PROCESSOR_FLOATINGPOINT},
    {0x0520, L"AMD-K5 Model 2", PROCESSOR_FLOATINGPOINT},
    {0x0530, L"AMD-K5 Model 3", PROCESSOR_FLOATINGPOINT},
    {0x0560, L"AMD-K6 Model 6", PROCESSOR_FLOATINGPOINT},
    {0x0570, L"AMD-K6 Model 7", PROCESSOR_FLOATINGPOINT},
    {0x0580, L"AMD-K6-2 Model 8", PROCESSOR_FLOATINGPOINT},
    {0x0590, L"AMD-K6-III Model 8", PROCESSOR_FLOATINGPOINT},
    {0x0610, L"AMD Athlon Model 1", PROCESSOR_FLOATINGPOINT},
    {0x0620, L"AMD Athlon Model 2", PROCESSOR_FLOATINGPOINT},
    {0x0630, L"AMD Duron Model 3", PROCESSOR_FLOATINGPOINT},
    {0x0640, L"AMD Athlon Model 4", PROCESSOR_FLOATINGPOINT},
    {0x0660, L"AMD Athlon Model 6", PROCESSOR_FLOATINGPOINT},
    {0x0670, L"AMD Duron Model 7", PROCESSOR_FLOATINGPOINT}   
};

#define AMD_ENTRIES (sizeof(AMDTable) / sizeof(CPUSIG))
#define AMD_MASK 0x0FF0
#define AMD_VENDOR (L"AuthenticAMD")
#define AMD_NAME (L"AMD Inc.")

// Intel processor signature table
CPUSIG IntelTable[] = {
    {0x0440, L"Intel486 SL", 0},
    {0x0470, L"IntelDX2", PROCESSOR_FLOATINGPOINT},
    {0x0480, L"IntelDX4", PROCESSOR_FLOATINGPOINT},
    {0x1480, L"IntelDX4", PROCESSOR_FLOATINGPOINT},
    {0x0510, L"Pentium", PROCESSOR_FLOATINGPOINT},
    {0x0520, L"Pentium", PROCESSOR_FLOATINGPOINT},
    {0x1510, L"Pentium OverDrive", PROCESSOR_FLOATINGPOINT},
    {0x1520, L"Pentium OverDrive", PROCESSOR_FLOATINGPOINT},
    {0x1530, L"Pentium OverDrive", PROCESSOR_FLOATINGPOINT},
    {0x0540, L"Pentium with MMX", PROCESSOR_FLOATINGPOINT},
    {0x1540, L"Pentium OverDrive with MMX", PROCESSOR_FLOATINGPOINT},
    {0x0610, L"Pentium Pro", PROCESSOR_FLOATINGPOINT},
    {0x0630, L"Pentium II Model 3", PROCESSOR_FLOATINGPOINT},
    {0x1630, L"Pentium II OverDrive", PROCESSOR_FLOATINGPOINT},
    {0x0650, L"Pentium II Model 5", PROCESSOR_FLOATINGPOINT},
    {0x0660, L"Celeron II Model 6", PROCESSOR_FLOATINGPOINT},
    {0x0670, L"Pentium III Model 7", PROCESSOR_FLOATINGPOINT},
    {0x0680, L"Pentium III Model 8", PROCESSOR_FLOATINGPOINT},
    {0x06A0, L"Pentium III Xeon Model A", PROCESSOR_FLOATINGPOINT},
    {0x06B0, L"Pentium III Xeon Model B", PROCESSOR_FLOATINGPOINT},
    {0x0F00, L"Pentium 4", PROCESSOR_FLOATINGPOINT},
    {0x0F10, L"Pentium 4", PROCESSOR_FLOATINGPOINT}
};

#define INTEL_ENTRIES (sizeof(IntelTable) / sizeof(CPUSIG))
#define INTEL_MASK 0x3FF0
#define INTEL_VENDOR (L"GenuineIntel")
#define INTEL_NAME (L"Intel Corp.")

// NSC Geode processor signature table
CPUSIG GeodeTable[] = {
    {0x0540, L"Geode GXm", PROCESSOR_FLOATINGPOINT}   
};

#define GEODE_ENTRIES (sizeof(GeodeTable) / sizeof(CPUSIG))
#define GEODE_MASK 0x0FF0
#define GEODE_VENDOR (L"Geode by AMD")
#define GEODE_NAME AMD_NAME

// VIA/Centaur C3 processor signature table
CPUSIG VIATable[] = {
    {0x0670, L"VIA C3", PROCESSOR_FLOATINGPOINT}   
};

#define VIA_ENTRIES (sizeof(VIATable) / sizeof(CPUSIG))
#define VIA_MASK 0x0FF0
#define VIA_VENDOR (L"CentaurHauls")
#define VIA_NAME (L"VIA Technologies, Inc.")

// Vortex86 processor signature table
CPUSIG Vortex86Table[] = {
    {0x0522, L"Vortex86DX", PROCESSOR_FLOATINGPOINT},
    {0x0586, L"VortexMX/DX-II", PROCESSOR_FLOATINGPOINT}
};

#define VORTEX86_ENTRIES (sizeof(Vortex86Table) / sizeof(CPUSIG))
#define VORTEX86_MASK 0x0FFF
#define VORTEX86_VENDOR (L"Vortex86 SoC")
#define VORTEX86_NAME (L"DMP Electronics Inc")

#endif // _CPUID_H
