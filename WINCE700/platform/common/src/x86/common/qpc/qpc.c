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
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  
//------------------------------------------------------------------------------
#include <windows.h>
#include <timer.h>
#include <pc.h>
#include <oal.h>

extern volatile ULARGE_INTEGER CurTicks;
extern DWORD  g_dwOALTimerCount;

//------------------------------------------------------------------------------
//
//  GTCQueryPerformanceCounter
//
//  This is basically a variable incremented every clock interrupt, 
//  which is triggered by the PIT or RTC and typically comes every 10 or 15 ms. 
//
// Note: we implement this, rather than setting the pointer to NULL and using
// the kernel implementation, as the kernel implementation isn't 64bit
//
BOOL 
GTCQueryPerformanceCounter(
    LARGE_INTEGER *lpliPerformanceCount
    )
{
    static LARGE_INTEGER liBase;
    static DWORD LastCount;

    DWORD TickCount = CurMSec;

    // assume that this has been called in the last 49.71 days
    liBase.QuadPart += TickCount - LastCount;
    LastCount = TickCount;

    *lpliPerformanceCount = liBase;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  GTCQueryPerformanceFrequency
//  
BOOL 
GTCQueryPerformanceFreq(
    LARGE_INTEGER *lpliPerformanceFreq
    ) 
{
    lpliPerformanceFreq->HighPart = 0;
    lpliPerformanceFreq->LowPart  = 1000;
    return TRUE;
}


//==============================================================================

//------------------------------------------------------------------------------
//
//  PerfCountFreq
//  
DWORD
PerfCountFreq()
{
    return TIMER_FREQ;
}
//------------------------------------------------------------------------------
//
//  PerfCountSinceTick
//  
DWORD
PerfCountSinceTick(void)
{
    WORD wCurCount;
    UCHAR ucInterrupt;

    __asm {
TRY_LATCHING_AGAIN: 
             
        pushfd                  ; Save interrupt state
        cli

        in      al,20h          ; get current interrupt state bits
        mov     ucInterrupt, al

        in      al,40h          ; clear latches, in case they are loaded
        in      al,40h          ; 
        mov     al,11000010b    ;latch status and count of counter 0
        out     43h,al
        in      al,40h          ; read status
        shl     eax, 16         ; move into high side of eax
        in      al,40h          ; read counter 0 lsb
        mov     ah,al
        in      al,40h          ; read msb
        xchg    ah, al          ; Get them in the right order
        mov     wCurCount, ax
        shr     eax, 16         ; get the status back into al
        popfd                   ; Restore interrupt state
        
        test    al, 40h         ; did the latch operation fail?
        jne     TRY_LATCHING_AGAIN     ; if so, just do it again
    }

    //
    // Note : this is a countdown timer, not count up.
    //

    if (ucInterrupt & (1 << INTR_TIMER0)) {
        return g_dwOALTimerCount + (g_dwOALTimerCount - wCurCount);
    } else {
        return g_dwOALTimerCount - wCurCount;
    }
}



//------------------------------------------------------------------------------
//
//  PITQueryPerformanceCounter
//
//  These are hardware counters running at 3.579545 / 3 = 1.193182 MHz. 
//  They are quite slow to read (requires port I/O), on the order of 1 µs.
//  Should be accurate to < 100 ppm, hopefully closer to 50ppm
//  
BOOL 
PITQueryPerformanceCounter(
    LARGE_INTEGER *lpliPerformanceCount
    )
{
    ULARGE_INTEGER liBase;
    DWORD dwCurCount;

    // Make sure CurTicks is the same before and after read of counter to 
    // account for possible rollover
    do {
        liBase = CurTicks;
        dwCurCount = PerfCountSinceTick( );
    } while  (liBase.LowPart != CurTicks.LowPart);
    
    lpliPerformanceCount->QuadPart = liBase.QuadPart + dwCurCount;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  PITQueryPerformanceFrequency
//  
BOOL 
PITQueryPerformanceFrequency(
    LARGE_INTEGER *lpliPerformanceFreq
    ) 
{
    lpliPerformanceFreq->HighPart = 0;
    lpliPerformanceFreq->LowPart  = PerfCountFreq();
    return TRUE;
}

//==============================================================================

DWORD IntrFreePerfCountSinceTick();
//------------------------------------------------------------------------------
// IoDelay - busy waits for dwUsDelay uS
//
// Assumptions:
// Running single threaded with interrupts off
// IntrFreePerfCountSinceTick does increment (at TIMER_FREQ), 
// and g_dwOALTimerCount doesn't
// There are no DEBUGMSGs after the delay time has elapsed
DWORD IoDelay (DWORD dwUsDelay)
{
    const DWORD dwStartCount = IntrFreePerfCountSinceTick();
    const DWORD TimerKFreq = TIMER_FREQ / 1000;
    DWORD dwCurrCount = dwStartCount, dwElapsedCount = 0;
    const DWORD dwTotalDelayCount = ((ULONGLONG)dwUsDelay * TimerKFreq / 1000) 
        + dwStartCount;
    
    ASSERT(dwTotalDelayCount > dwUsDelay); 
    // if this fails, dwTotalDelayCount rolled over

    do {
        DWORD dwLastCount;
        do {
            dwLastCount = dwCurrCount;
            dwCurrCount = IntrFreePerfCountSinceTick();

        // while the count didn't roll over, and we have not finished
        } while (dwCurrCount >= dwLastCount && 
            dwElapsedCount + dwCurrCount < dwTotalDelayCount);

        // add the number of counts we looped for
        if (dwCurrCount < dwLastCount)
            dwElapsedCount += TimerKFreq;
        else
            // the last loop may not be a full ms
            dwElapsedCount += dwCurrCount;

    } while (dwElapsedCount < dwTotalDelayCount);

    // postcondition:dwElapsedCount should be dwTotalDelayCount,or just above it
    return dwElapsedCount / TimerKFreq;
}


//------------------------------------------------------------------------------
// GetTimeStampCounter - Get the value of the Timer Stamp Counter
//
__inline __declspec(naked) unsigned __int64 __cdecl GetTimeStampCounter(void)
{
    __asm
    {
        rdtsc
        ret
    }
}

//------------------------------------------------------------------------------
// CalibrateTSC - calibrates the Frequency of the Timer Stamp Counter
//
// dwUsDelay - Time is uS to calibrate for (recommend > 1ms, *must* be < 1s)
//
// Assumptions - CPU supports rdtsc instruction, IoDelay is accurate
//
static unsigned __int64 Freq = 0;
BOOL CalibrateTSC(DWORD dwUsDelay)
{
    unsigned __int64 TSCCount1 = 0, TSCCount2 = 0;
    DWORD Loops = 0, i, ActualUsDelay = 0;

    // run a couple times
    for (i = 0; i < 2; ++i) {
        TSCCount1 = GetTimeStampCounter();
        ActualUsDelay = IoDelay(dwUsDelay);
        TSCCount2 = GetTimeStampCounter();
    }

    Freq = (TSCCount2 - TSCCount1) * (1000 / ActualUsDelay);


    // since this calibration is inaccurate, and will always overestimate 
    // the actual TSC Frequency, round down to ~3 signifigant digits
    // This works well for frequencies % 100, 
    // but will not work so well for frequencies % 3
    while (Freq > 1000)
    {
        Freq /= 10;
        ++Loops;
    }
    
    do
    {
        Freq *= 10;
    }
    while (--Loops);

    DEBUGMSG(OAL_TIMER, (L"Estimated CPU Speed %dMhz", (DWORD)(Freq/1000000)));

    return Freq > 0;
}

//------------------------------------------------------------------------------
// RDTSCQueryPerformanceFreq
//
// returns the estimated Performance Counter Frequency
//
BOOL RDTSCQueryPerformanceFreq(LARGE_INTEGER *pFreq)
{
    pFreq->QuadPart = Freq;

    return TRUE;
}

//------------------------------------------------------------------------------
// GetCpuIdSignature
//
// returns the signature of the CPU designer
//
#define AMD_SIGNATURE       0x68747541      
// (ebx) value for cpuid instruction on AMD CPUs
__declspec(naked) DWORD GetCpuIdSignature (void)
{
    _asm {
        push    ebx     // cpuid instruction will change ebx

        xor     eax, eax
        cpuid
        mov     eax, ebx
        
        pop     ebx
        ret
    }
}

//------------------------------------------------------------------------------
// IsTSCInvariant - Determine if the Time Stamp Counter is in lock step across 
//                  cores
//
// notes: If a QueryPerformanceCounter goes backwards, 
//        then likely this function needs to include
//        the CPU that causes this, and return FALSE
//
// returns TRUE if the TSC is invariant across all cores
//
BOOL IsTSCInvariant()
{
    static enum { TSC_INVARIANT, TSC_VARIANT, TSC_UNKNOWN }UseTsc = TSC_UNKNOWN;

    if (UseTsc == TSC_UNKNOWN)
    {
        if (GetCpuIdSignature() == AMD_SIGNATURE)
        {
            DWORD EPMFlags;
            const DWORD EPM_ITSC = 1 << 8;
            __asm {
                mov eax, 80000007h
                cpuid
                mov EPMFlags, edx    
            }
            if (EPMFlags & EPM_ITSC)
                // AMD Processor that states TSC is invariant across cores
                UseTsc = TSC_INVARIANT;
            else
                // AMD Processor that may return different TSC values depending 
                // on which Core is running the read instruction
                UseTsc = TSC_VARIANT;
        }
        else
        {
            // Intel and other processors, that will return the same TSC value 
            // on any Core
            UseTsc = TSC_INVARIANT;
        }
    }

    return UseTsc == TSC_INVARIANT;
}

//------------------------------------------------------------------------------
// MPQueryPerformanceCounter
//
// Get the value of the Time Stamp Counter
// This is incremented every clock. The overhead is small - about 11 clocks
//
// Assumptions: CPU does not have dynamic frequency scaling enabled
//
BOOL RDTSCQueryPerformanceCounter(LARGE_INTEGER *pCounter)
{
    if (IsTSCInvariant() || g_pOemGlobal->fMPEnable == FALSE)
        pCounter->QuadPart = GetTimeStampCounter();
    else
        // TSC varies between CPUs, only use the one on CPU0
        NKSendInterProcessorInterrupt (IPI_TYPE_SPECIFIC_CPU, 0, 
            IPI_QUERY_PERF_COUNTER, (DWORD) pCounter);

    return TRUE;
}

//------------------------------------------------------------------------------
// IsRDTSCSupported
//
// returns TRUE if the RDTSC instruction is supported
//
BOOL IsRDTSCSupported (void)
{
    BOOL RetVal = 0;
    __try 
    {
        _asm {
            push    ebx

            mov     eax, 1
            cpuid
            mov     RetVal, edx
            
            pop     ebx
        }

        // bit 4 is 1 if rdtsc is supported
        return RetVal & (1 << 4);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        // likely cpuid was not supported
        return FALSE;
    }
}



//==============================================================================
#include <acpi.h>
typedef struct  {
    volatile BYTE  RevId;
    volatile BYTE  NumTimers; 
    volatile WORD  VendorID;
    volatile DWORD ClockPeriod;
    ULARGE_INTEGER Reserved1;
    volatile BYTE  GeneralConfig;
    BYTE           Reserved2;
    WORD           Reserved3;
    DWORD          Reserved4;
    ULARGE_INTEGER Reserved5;
    DWORD          Reserved6;
    DWORD          InterruptStatus;
} GeneralConfig;

static volatile ULARGE_INTEGER * pClock;
static volatile GeneralConfig * pGenConfig;

//------------------------------------------------------------------------------
//
//  HPETQueryPerformanceCounter
//
//  This is basically a variable incremented every clock interrupt, which is 
//  triggered by the PIT or RTC and typically comes every 10 or 15 ms. 
//
//  Power Management Considerations
//  It is the Operating System's responsibility to save and restore Event Timer 
//  hardware context if this needs to be preserved through ACPI System Sleep 
//  State transitions.
//  General behavioral rules for Event Timer hardware regarding sleep state 
//  transitions:
//  1. The Event Timer registers (including the main counter) are not expected 
//     to be preserved through an S3, S4, or S5 state.
//  2. The features and functions associated with these registers are not 
//     expected to be used in an S1 state.
//     Prior to going to an S1 state, all interrupts associated with this 
//     function should be disabled.
//  3. The main counter is permitted, but not required, to run during S1 or S2 
//     states. This allows mobile systems to stop clock generators feeding the 
//     main counter during S1 or S2 states.
BOOL 
HPETQueryPerformanceCounter(
    LARGE_INTEGER *lpliPerformanceCount
    )
{
    do {
        lpliPerformanceCount->HighPart = pClock->HighPart;
        lpliPerformanceCount->LowPart = pClock->LowPart;
    } while  ((ULONGLONG)lpliPerformanceCount->HighPart != pClock->HighPart);

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  HPETQueryPerformanceFreq
//
//  Fmin = 10 MHz
//  +- .05 % (500 ppm ), Over any interval >= 1 Millisecond
//  +- .2 % (2000 ppm), Over any interval <= 100 Microseconds
//  
static LARGE_INTEGER HPETFreq;
BOOL 
HPETQueryPerformanceFreq(
    LARGE_INTEGER *lpliPerformanceFreq
    ) 
{
    lpliPerformanceFreq->QuadPart = HPETFreq.QuadPart;

    return TRUE;
}

BOOL HPETInit()
{
    void * TableData;
    AcpiTable * TableHeader;

    typedef struct  {
        BYTE  RevId;
        BYTE  Stuff;
        WORD  PCIVendorId;
        BYTE  AddressSpace; 
        BYTE  BitWidth;
        BYTE  BitOffset;
        BYTE  Reserved;
        ULARGE_INTEGER Address;
        BYTE  HPETNum;
        WORD  MinClockTicks;
        BYTE  PageProtection;

    } HPET;

    typedef struct  {
        volatile WORD  BitFields;
        WORD           Reserved;
        volatile DWORD RouteCaps;
        volatile ULARGE_INTEGER RegistryAddressing;
        volatile DWORD FSBIRQMessageLocation;
        volatile DWORD FSBIRQMessage;
    } TimerConfig;

    if (AcpiFindATable(ACPI_TABLE_HPET, &TableData, &TableHeader)) {

        HPET* pHPET = (HPET*)TableData;

        DEBUGMSG(OAL_TIMER, (L"AddressSpace %x", pHPET->AddressSpace));
        DEBUGMSG(OAL_TIMER, (L"BitWidth %x", pHPET->BitWidth));
        DEBUGMSG(OAL_TIMER, (L"BitOffset %x", pHPET->BitOffset));
        DEBUGMSG(OAL_TIMER, (L"Address %x %x", pHPET->Address.HighPart, 
            pHPET->Address.LowPart));

        // just supporting 32bit not 48bit
        if (pHPET->Address.HighPart == 0 && 
            pHPET->AddressSpace == 0 &&
            pHPET->RevId <= 1) {
            volatile TimerConfig* pTimer0Config = 0;

            pGenConfig = NKCreateStaticMapping(pHPET->Address.LowPart>>8, 1024);

            // convert from femptoseconds to Hz
            HPETFreq.QuadPart = (unsigned __int64) 0x38D7EA4C68000 / 
                (unsigned __int64) pGenConfig->ClockPeriod;

            DEBUGMSG(OAL_TIMER, (L"ClockPeriod %d fs",pGenConfig->ClockPeriod));
            DEBUGMSG(OAL_TIMER, (L"ClockFreq %d Hz", HPETFreq.QuadPart));
            
            pClock = (ULARGE_INTEGER *)(((BYTE*)pGenConfig) + 0xf0);
            pTimer0Config = (TimerConfig *)(((BYTE*)pGenConfig) + 0x100);

            // just supporting 64bit timers, 32 could be supported if the 
            // rollover was caught.
            if (pTimer0Config->BitFields & 1 << 5) {

                // stop the timer, set it to a known value, then start it
                pGenConfig->GeneralConfig &= ~1;
                pClock->HighPart = 0;
                pClock->LowPart = (DWORD)-1;
                pGenConfig->GeneralConfig |= 1;

                // make sure it has rolled over
                if (pClock->HighPart != 0) {
                    return TRUE;
                }

                DEBUGMSG(OAL_TIMER, (L"HPET didn't start"));
                pGenConfig->GeneralConfig &= ~1;
            } else {
                DEBUGMSG(OAL_TIMER, (L"HPET doesn't support 64bit"));
            }
        } else {
            DEBUGMSG(OAL_TIMER,(L"HPET is outside 32bit memory address space"));
        }
    } else {
        DEBUGMSG(OAL_TIMER, (L"HPET not found"));
    }

    return FALSE;
}

//==============================================================================

//------------------------------------------------------------------------------
// x86InitPerfCounter
//
// Detects which High Perf counters are available and chooses one
//
BOOL x86InitPerfCounter (void)
{
    // note this is called from oeminit() which doesn't have exception handling
    DEBUGMSG(OAL_TIMER, (L"PIT used for QPC"));
    pQueryPerformanceCounter = PITQueryPerformanceCounter;
    pQueryPerformanceFrequency = PITQueryPerformanceFrequency;

    return TRUE;
}

//------------------------------------------------------------------------------
//  IsRunningOnVirtualMachine
//
//  Try a synthetic instruction (VMCPUID)
//  to figure out whether we're running in a VM. VMCPUID is guaranteed to
//  run in ring 3, so it is one of the few we can use to do this.
//
static BOOL IsRunningOnVirtualMachine()
{
    BOOL fRetVal = TRUE;
    DWORD OldPtr = UTlsPtr()[TLSSLOT_KERNEL];
    UTlsPtr()[TLSSLOT_KERNEL] |= TLSKERN_NOFAULT | TLSKERN_NOFAULTMSG;

    __try
    {
        __asm
        {
            // Execute a synthetic VMCPUID instruction.
            __emit  0x0F
            __emit  0xC7
            __emit  0xC8
            __emit  0x01
            __emit  0x00
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        // this should be EXCEPTION_ILLEGAL_INSTRUCTION
        fRetVal = FALSE;
    }

    UTlsPtr()[TLSSLOT_KERNEL] = OldPtr;
    DEBUGMSG(OAL_TIMER, (L"IsRunningOnVirtualMachine : %d", fRetVal));
    return fRetVal;
}

//------------------------------------------------------------------------------
//  QPCPostInit
//
//  Performs a second detect phase. This is run in a user thread,
//  where exceptions are supported (unlike x86InitPerfCounter)
//
BOOL QPCPostInit (void)
{
    if (IsRunningOnVirtualMachine()) {
        DEBUGMSG(OAL_TIMER, (L"GTC used for QPC"));
        pQueryPerformanceCounter = GTCQueryPerformanceCounter;
        pQueryPerformanceFrequency = GTCQueryPerformanceFreq;
    } else if (HPETInit()) {
        DEBUGMSG(OAL_TIMER, (L"HPET used for QPC"));
        pQueryPerformanceCounter   = HPETQueryPerformanceCounter;
        pQueryPerformanceFrequency = HPETQueryPerformanceFreq;
    } else if (IsRDTSCSupported() && CalibrateTSC(20000) && g_pOemGlobal->fMPEnable == TRUE) {
        DEBUGMSG(OAL_TIMER, (L"RDTSC used for QPC"));
        pQueryPerformanceCounter   = RDTSCQueryPerformanceCounter;
        pQueryPerformanceFrequency = RDTSCQueryPerformanceFreq;
    }

    return TRUE;
}
