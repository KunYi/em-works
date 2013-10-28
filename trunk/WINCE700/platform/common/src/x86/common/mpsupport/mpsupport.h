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
// Define the strucutres/functions used for MP support

#pragma once
#ifndef _MPSUPPORT_H_
#define _MPSUPPORT_H_

#pragma pack(push, 1)           // We want this structure packed exactly as declared!

typedef struct {
    USHORT Size;
    PVOID Base;
} FWORDPtr;

#pragma pack(pop)

//
// parameter passed to MP startup code, running in real-mode
//
typedef struct {
    DWORD       JumpInstr;
    DWORD       PageDirPhysAddr;
    ULONGLONG   tmpGDT[3];
    DWORD       Cr4Value;
    FWORDPtr    tmpGDTBase;
} MPStartParam, *PMPStartParam;


//
// x86 spec - BSP starts other CPUs by sending a "Startup IPI (SIPI)" to other cpus. All
//            CPUs will start execution in real mode, with cs:ip == [SIPI_VECOTR<<8]:0 
//            (The flat physical address of the SIPI startup address is SIPI_VECTOR << 12).
//            The flat startup address of SIPI must be < 1M (0x100000).
//            i.e. SIPI_VECTOR <= 0xff. 
//
//            For CEPC, bootloader and imager are setup to use address >= 1M, so the memory
//            are "free" to use, except for the memory used by bios. 
//            
//            For other x86 devices, make sure you update SIPI_VECTOR accordingly if the
//            memory below 1M will be used by image/bootloader (describe in config.bib for 
//            image, and boot.bib for bootloader).
//
#define SIPI_VECTOR             0x99                // use vector 0x99 for SIPI
#define SIPI_PAGE_PHYS_ADDR     (SIPI_VECTOR << 12) // SIPI page physical address

//
// IPI (Inter-Processor-Interrupt) Vector is used to handle inter-processor synchronization. 
// It must not conflict with any IRQ used by hardware interrupt. 
//
#define IPI_VECTOR              0xff                // use vector 0xff for IPI

//
// APIC registers
//
#define APIC_REGS_PHYS_BASE     0xFEE00000
typedef struct _APIC_REGS {
    DWORD reserved000[8];       // FEE00000-FEE0001F: reserved
    
    DWORD ApicId;               // FEE00020: APIC ID register
    DWORD reserved020[3];

    DWORD ApicVersion;          // FEE00030: APIC Version register
    DWORD reserved030[3];

    DWORD reserved040[16];      // FEE00040-FEE0007F: reserved

    DWORD TaskPrioReg;          // FEE00080: Task priority register
    DWORD reserved080[3];

    DWORD ArbPrioReg;           // FEE00090: Abritration priority register
    DWORD reserved090[3];
    
    DWORD ProcPrioReg;          // FEE000A0: Processor priority register
    DWORD reserved0a0[3];
    
    DWORD EOIReg;               // FEE000B0: EOI register
    DWORD reserved0b0[3];

    DWORD reserved0c0[4];       // FEE000C0: reserved
    
    DWORD LocDestReg;           // FEE000D0: Local Destination register
    DWORD reserved0d0[3];
    
    DWORD DestFmtReg;           // FEE000E0: destination format register
    DWORD reserved0e0[3];
    
    DWORD SpurIntVecReg;        // FEE000F0: Spurious Interrupt Vector register
    DWORD reserved0f0[3];

    BYTE  ISR[0x80];            // FEE00100-FEE0017F: In-Service Register
    BYTE  TMR[0x80];            // FEE00180-FEE001FF: Trigger Mode Register
    BYTE  IRR[0x80];            // FEE00200-FEE0027F: Interrupt Request Register
    
    DWORD ErrStatReg;           // FEE00280: Error Status register
    DWORD reserved280[3];
    
    DWORD reserved290[0x1C];    // FEE00290-FEE002FF: reserved

    DWORD IcrLow;               // FEE00300: Interrupt Command Register (0-31)
    DWORD reserved300[3];

    DWORD IcrHigh;              // FEE00310: Interrupt Command Register (32-64)
    DWORD reserved310[3];

    DWORD LvtTimer;             // FEE00320: LVT Timer Register
    DWORD reserved320[3];

    DWORD LvtThermal;           // FEE00330: LVT Thermal Sensor Register
    DWORD reserved330[3];

    DWORD LvtPerfCounter;       // FEE00340: LVT Perf Monitor Counter Register
    DWORD reserved340[3];

    DWORD LvtLint0;             // FEE00350: LVT LINT0 Register
    DWORD reserved350[3];

    DWORD LvtLint1;             // FEE00360: LVT LINT1 Register
    DWORD reserved360[3];

    DWORD LvtError;             // FEE00370: LVT Error Register
    DWORD reserved370[3];

    DWORD TimerInitCount;       // FEE00380: Initial Counter Register (For Timer)
    DWORD reserved380[3];

    DWORD TimerCurrCount;       // FEE00390: Current Count Register (For Timer)
    DWORD reserved390[3];

    DWORD reserved3a0[0x10];    // FEE003A0-FEE003DF: reserved

    DWORD DivConfig;            // FEE003E0: Divide Configuration Register (For Timer)
    DWORD reserved3e0[3];

    DWORD reserved3f0[4];       // FEE003F0-FEE002FF: reserved
}APIC_REGS;


#endif
