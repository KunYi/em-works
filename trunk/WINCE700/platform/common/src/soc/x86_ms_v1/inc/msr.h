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

#ifndef _MSR_H
#define _MSR_H


//
// Definitions required for working with x86 Model-Specific Registers
//


//------------------------------------------------------------------------------
// CPUID DEFINITIONS
//------------------------------------------------------------------------------

// IdentifyCpu returns the processor feature flags returned by the CPUID instruction
extern DWORD IdentifyCpu();

// See CPUID_ flags in public\common\oak\inc\pkfuncs.h

// The different parts of the CPU Signature
#define CPUID_MODEL(dwProcID)   (((dwProcID) >> 4) & 0xF)
#define CPUID_FAMILY(dwProcID)  (((dwProcID) >> 8) & 0xF)


//------------------------------------------------------------------------------
// PAT DEFINITIONS
//------------------------------------------------------------------------------

//
// PAT Entry Masks
//
#define PAT_Entry0Mask                      (0x0000000F)
#define PAT_Entry1Mask                      (0x00000F00)
#define PAT_Entry2Mask                      (0x000F0000)
#define PAT_Entry3Mask                      (0x0F000000)


//
// PAT Memory Types
//
// Cannot be used together
#define PAT_TypeUncacheable                 0
#define PAT_TypeWriteCombining              1
#define PAT_TypeWriteThrough                4
#define PAT_TypeWriteProtected              5
#define PAT_TypeWriteBack                   6
#define PAT_TypeUncachedOverrideable        7  // Can be overridden by WC in the MTRRs


//
// PAGE TABLE DEFINITIONS
//

// Three page table flags index into the PAT
#define x86_WRITE_THRU_MASK     0x00000008
#define x86_CACHE_MASK          0x00000010
#define x86_PAT_MASK            0x00000080

#define x86_PAT_INDEX0          (x86_PAT_MASK | 0             | 0)
#define x86_PAT_INDEX1          (x86_PAT_MASK | 0             | x86_WRITE_THRU_MASK)
#define x86_PAT_INDEX2          (x86_PAT_MASK | x86_CACHE_MASK | 0)
#define x86_PAT_INDEX3          (x86_PAT_MASK | x86_CACHE_MASK | x86_WRITE_THRU_MASK)
#define x86_PAT_INDEX_MASK      (x86_PAT_MASK | x86_CACHE_MASK | x86_WRITE_THRU_MASK)



//------------------------------------------------------------------------------
// MTRR DEFINITIONS
//------------------------------------------------------------------------------

//
// MTRR Memory Types
//
// Cannot be used together
#define MTRR_TypeUncacheable                0
#define MTRR_TypeWriteCombining             1
#define MTRR_TypeWriteThrough               4
#define MTRR_TypeWriteProtected             5
#define MTRR_TypeWriteBack                  6


//
// Contents of the 64-bit MTRRcap register if EAX=1
//
//  6                                    1 1
//  3                                    1 0 9 8 7              0
// |--------------------------------------|-|-|-|----------------|
//                RESERVED                 W   F      VCNT
//                                         C   X
//
// WC   - Write-combining memory type supported
// FX   - Fixed range registers supported
// VCNT - Number of variable range registers
//
#define MTRRcap_WriteCombineMask            ((DWORD)1 << 10)
#define MTRRcap_FixedRangeMask              ((DWORD)1 << 8)
#define MTRRcap_VarRangeRegisterCountMask   (((DWORD)1 << 8) - 1)


//
// Contents of the 64-bit MTRRdefType register:
//
//  6                                  1 1 1
//  3                                  2 1 0 9 8 7              0
// |------------------------------------|-|-|---|----------------|
//                RESERVED               E F RES      TYPE
//                                         E    
//
// E    - MTRR enable/disable
// FE   - Fixed-range MTRRs enable/disable
// RES  - RESERVED
// TYPE - Default memory type
//
#define MTRRdefType_EnableMask              ((DWORD)1 << 11)
#define MTRRdefType_FixedRangeEnableMask    ((DWORD)1 << 10)
#define MTRRdefType_DefaultTypeMask         (((DWORD)1 << 8) - 1)


//
// Contents of a 64-bit MTRRphysBase register:
//
//  6                  3 3             1 1  
//  3                  6 5             2 1     8 7              0
// |--------------------|---------------|-------|----------------|
//        RESERVED          PhysBase     RESERVD      TYPE
//                                              
//
// PhysBase - Base address of range, 4kB-aligned sign-extended
// TYPE     - Memory type of range
//
// On CE no address can exceed 32 bits so we can cut the base off there
#define MTRRphysBase_PhysBaseMask           (~(((DWORD)1 << 12) - 1))
#define MTRRphysBase_TypeMask               (((DWORD)1 << 8) - 1)


//
// Contents of a 64-bit MTRRphysMask register:
//
//  6                  3 3             1 1 1
//  3                  6 5             2 1 0                    0
// |--------------------|---------------|-|----------------------|
//        RESERVED          PhysMask     V        RESERVED
//                                              
//
// PhysMask - 24-bit mask that determines the range of the region being mapped,
//            according to tthe relationship:
//                Address_Within_Range AND PhysMask = PhysBase AND PhysMask
// V        - Valid, enables the register pair when set
//
// On CE no address can exceed 32 bits so we can cut the mask off there
#define MTRRphysMask_PhysMaskMask           (~(((DWORD)1 << 12) - 1))
#define MTRRphysMask_ValidMask              ((DWORD)1 << 11)



//------------------------------------------------------------------------------
// PERFORMANCE-MONITORING DEFINITIONS for P5-family processors
//------------------------------------------------------------------------------

// CESR Counter Control flags (bitmask together):
#define CESR_CC_COUNT_CLOCKS   0x4  // Set to count clocks (duration); clear to
                                    // count events
#define CESR_CC_ENABLE_CPL3    0x2  // Set to enable counting while CPL=3
#define CESR_CC_ENABLE_CPL012  0x1  // Set to enable counting while CPL=0, 1 or 2

//
// Contents of the 32-bit Control and Event Select Register (CESR) (P5 only):
//
//  3            2 2 2   2 2     1 1            1
//  1            6 5 4   2 1     6 5            0 9 8   6 5     0
// |--------------|-|-----|-------|--------------|-|-----|-------|
//     RESERVED    P  CC1    ES1      RESERVED    P  CC0    ES0
//                 C                              C
//                 1                              0
//
// PC0 and PC1 - Pin control flags: Selects the function of the external
//               performance-monitoring counter pin (PM0/BP0 and PM1/BP1).
//               Setting one of these flags to 1 causes the processor to assert
//               its associated pin when the counter has overflowed; setting the
//               flag to 0 causes the pin to be asserted when the counter has
//               been incremented.  These flags permit the pins to be 
//               individually programmed to indicate the overflow or incremented
//               condition.  Note that the external signaling of the event on
//               the pins will lag the internal event by a few clocks as the
//               signals are latched and buffered.
// CC0 and CC1 - Counter control fields: Controls the operation of the counter.
//               (See CESR_CC_* flags above.)
// ES0 and ES1 - Event select fields: Selects (by entering an event code in the
//               field) up to two events to be monitored.
//               (See CESR_PERFORMANCE_EVENT values.)
//
#define CESR_PinCtl1Shift                   25
#define CESR_PinCtl1Mask                    ((DWORD)1 << CESR_PinCtl1Shift)
#define CESR_CtrCtl1Shift                   22
#define CESR_EvtSel1Shift                   16
#define CESR_PinCtl0Shift                   9
#define CESR_PinCtl0Mask                    ((DWORD)1 << CESR_PinCtl0Shift)
#define CESR_CtrCtl0Shift                   6
#define MAKE_CESR_VALUE(fCountClocks0, wES0, fCountClocks1, wES1)              \
    (  ((DWORD)(((fCountClocks1) ? CESR_CC_COUNT_CLOCKS : 0)                   \
                | CESR_CC_ENABLE_CPL3                                          \
                | CESR_CC_ENABLE_CPL012) << CESR_CtrCtl1Shift)                 \
     | ((DWORD)((wES1) & 0x3F) << CESR_EvtSel1Shift)                           \
     | ((DWORD)(((fCountClocks0) ? CESR_CC_COUNT_CLOCKS : 0)                   \
                | CESR_CC_ENABLE_CPL3                                          \
                | CESR_CC_ENABLE_CPL012) << CESR_CtrCtl0Shift)                 \
     | ((DWORD)((wES0) & 0x3F)))


//
// The 40-bit CTR0 and CTR1 registers are just numbers.
//


//------------------------------------------------------------------------------
// PERFORMANCE-MONITORING DEFINITIONS for P6-family processors
//------------------------------------------------------------------------------

//
// Contents of the 32-bit PerfEvtSel0 and PerfEvtSel1 registers (P6 only):
//
//  3             2 2 2 2 2 1 1 1 1 1          
//  1             4 3 2 1 0 9 8 7 6 5          8 7              0
// |---------------|-|-|-|-|-|-|-|-|------------|----------------|
//   COUNTER MASK   I E   I P E O U  UNIT MASK     EVENT SELECT
//                  N N   N C   S S              
//                  V     T       R
//
// COUNTER MASK - When nonzero, the processor compares this mask to the number
//                of events counted during a single cycle.  If the event count
//                is greater than or equal to this mask, the counter is
//                incremented by one.  Otherwise the counter is not incremented.
//                This mask can be used to count events only if multiple
//                occurrences happen per clock (for example, two or more
//                instructions retired per clock).  If the counter-mask field is
//                0, then the counter is incremented each cycle by the number of
//                events that occurred that cycle.
// INV          - Invert counter mask: Inverts the result of the counter-mask
//                comparison when set, so that both greater than and less than
//                comparisons can be made.
// EN           - Enable Counters (PerfEvtSel0 ONLY): When set, performance
//                counting is enabled in both performance-monitoring counters;
//                when clear, both counters are disabled.
// RES          - RESERVED
// INT          - APIC interrupt enable: When set the processor generates an 
//                exception through its local APIC on counter overflow.
// PC           - Pin control: When set, the processor toggles the PMi pins and
//                increments the counter when performance-monitoring events
//                occur; when clear, the processor toggles the PMi pins when the
//                counter overflows.
// E            - Edge detect: Enables (when set) edge detection of events.  The
//                processor counts the number of deasserted to asserted 
//                transitions of any condition that can be expressed by the
//                other fields.  The mechanism is limited in that it does not
//                permit back-to-back assertions to be distinguished.  This
//                mechanism allows software to measure not only the fraction of
//                time spent in a particular state, but also the average length
//                of time spent in such a state (for example, the time spent
//                waiting for an interrupt to be serviced).
// OS           - Operating system mode: Specifies that events are counted only
//                when the processor is operating at privilege level 0.  This
//                flag can be used in conjunction with the USR flag.
// USR          - User mode: Specifies that events are counted only when the
//                processor is operating at privilege levels 1, 2 or 3.  This
//                flag can be used in conjunction with the OS flag.
// UNIT MASK    - Further qualifies the event selected in the event select 
//                field.  For example, for some cache events, the mask is used
//                as a MESI-protocol qualifier of cache states.
// EVENT SELECT - Selects the event to be monitored.
//
#define PerfEvtSel_CounterMaskShift         24
#define PerfEvtSel_InvertMask               ((DWORD)1 << 23)
#define PerfEvtSel_EnableMask               ((DWORD)1 << 22)
#define PerfEvtSel_InterruptEnableMask      ((DWORD)1 << 20)
#define PerfEvtSel_PinControlMask           ((DWORD)1 << 19)
#define PerfEvtSel_EdgeDetectMask           ((DWORD)1 << 18)
#define PerfEvtSel_OSModeMask               ((DWORD)1 << 17)
#define PerfEvtSel_UserModeMask             ((DWORD)1 << 16)
#define PerfEvtSel_UnitMaskShift            8
#define MAKE_PERFEVTSEL_VALUE(fInterrupt, wUnitMask, wEventType)               \
    (((fInterrupt) ? PerfEvtSel_InterruptEnableMask : 0)                       \
     | PerfEvtSel_OSModeMask | PerfEvtSel_UserModeMask                         \
     | ((DWORD)((wUnitMask) & 0xFF) << PerfEvtSel_UnitMaskShift)               \
     | ((DWORD)((wEventType) & 0xFF)))

//
// The 40-bit PerfCtr0 and PerfCtr1 registers are just numbers.
//


// Enable entry point for performance counter library
#ifdef ENABLE_PERFCTR
extern VOID CPUPerfCounterInit();
extern HRESULT CPUPerfCounterIoctl(LPVOID lpInBuf, DWORD nInBufSize,
                                   LPVOID lpOutBuf, DWORD nOutBufSize,
                                   LPDWORD lpBytesReturned);
extern VOID CPUPerfCounterOverflowHandler(unsigned int ra);
#else
#define CPUPerfCounterInit()                ((VOID)0)
#define CPUPerfCounterIoctl(lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned) \
    ((HRESULT)ERROR_NOT_SUPPORTED)
#define CPUPerfCounterOverflowHandler()     ((VOID)0)
#endif // ENABLE_PERFCTR


//------------------------------------------------------------------------------
// MSR DEFINITIONS
//------------------------------------------------------------------------------

// Addresses of MTRR and PAT registers, accessed via RDMSR and WRMSR instructions
#define MSRAddr_MTRRcap                     (0x000000FE)
#define MSRAddr_MTRRphysBase0               (0x00000200)  // PhysBaseN is Base0 + 2*N
#define MSRAddr_MTRRphysMask0               (0x00000201)  // PhysMaskN is Mask0 + 2*N
#define MSRAddr_MTRRdefType                 (0x000002FF)
#define MSRAddr_PAT                         (0x00000277)

// Address of Time Stamp Counter, accessed via RDMSR, WRMSR and RDTSC
#define MSRAddr_TSC                         (0x00000010)

// Addresses of Performance Counter registers for P5-family processors,
// accessed via RDMSR, WRMSR and RDPMC
#define MSRAddr_CESR                        (0x00000011)
#define MSRAddr_CTR0                        (0x00000012)
#define MSRAddr_CTR1                        (0x00000013)

// Addresses of Performance Counter registers for P6-family processors,
// accessed via RDMSR, WRMSR and RDPMC
#define MSRAddr_PerfCtr0                    (0x000000C1)
#define MSRAddr_PerfCtr1                    (0x000000C2)
#define MSRAddr_PerfEvtSel0                 (0x00000186)
#define MSRAddr_PerfEvtSel1                 (0x00000187)


extern BOOL NKwrmsr(
    DWORD dwAddr,       // Address of MSR being written
    DWORD dwValHigh,    // Upper 32 bits of value being written
    DWORD dwValLow      // Lower 32 bits of value being written
    );
extern BOOL NKrdmsr(
    DWORD dwAddr,       // Address of MSR being read
    DWORD *lpdwValHigh, // Receives upper 32 bits of value, can be NULL
    DWORD *lpdwValLow   // Receives lower 32 bits of value, can be NULL
    );


#endif // _MSR_H
