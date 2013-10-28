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
//  File: ioctl.c
//
//  This file implements OALIoCtlHalGetCacheInfo function called from
//  OEMIoControl handler for IOCTL_HAL_GETCACHEINFO code.
//
#include <windows.h>
#include <pkfuncs.h>
#include <oal.h>

typedef struct
{
    UINT32 descriptor;
    OAL_CACHE_INFO cacheInfo;
} x86CPUIDPair;

static const DWORD NUMVALIDCPUIDDESCRIPTORS = 56;
static const DWORD MAXCPUIIDITERATIONS = 6;

static x86CPUIDPair x86CPUIDTable[NUMVALIDCPUIDDESCRIPTORS];

//------------------------------------------------------------------------------
//
//  Function:  MapCacheAssoc
//      Maps a descriptor byte to the corresponding associativity based on
//      the 8006h CPUID instruction. Returns a non-zero value upon success.
//
DWORD MapCacheAssoc(DWORD dwInput)
{
    DWORD map[] = {0, 1, 2, 0, 4, 0, 8, 0, 16, 0, 32, 64, 96, 128, 0};
    ASSERT(dwInput <= 0xF);
    if (dwInput <= 0xF)
    {
        return map[dwInput];
    }
    return 0;
}

//------------------------------------------------------------------------------
//
//  Function:  ProcessDescriptorByte
//  Maps a descriptor byte to the corresponding cache values based upon the
//  static table.  Returns TRUE for success, FALSE for failure.
//
static BOOL ProcessDescriptorByte(
                                  const BYTE & descriptor
                                  )
{
    for (int loopCounter = 0; loopCounter < NUMVALIDCPUIDDESCRIPTORS; loopCounter++)
    {
        if (descriptor == x86CPUIDTable[loopCounter].descriptor)
        {
            // matched a table entry, update the global cache info.
            // Cache Info shouldn’t overlap, so we can use logical OR to track them
            g_oalCacheInfo.L1Flags |= x86CPUIDTable[loopCounter].cacheInfo.L1Flags;
            g_oalCacheInfo.L1ISetsPerWay |= x86CPUIDTable[loopCounter].cacheInfo.L1ISetsPerWay;
            g_oalCacheInfo.L1INumWays |= x86CPUIDTable[loopCounter].cacheInfo.L1INumWays;
            g_oalCacheInfo.L1ILineSize |= x86CPUIDTable[loopCounter].cacheInfo.L1ILineSize;
            g_oalCacheInfo.L1ISize |= x86CPUIDTable[loopCounter].cacheInfo.L1ISize;
            g_oalCacheInfo.L1DSetsPerWay |= x86CPUIDTable[loopCounter].cacheInfo.L1DSetsPerWay;
            g_oalCacheInfo.L1DNumWays |= x86CPUIDTable[loopCounter].cacheInfo.L1DNumWays;
            g_oalCacheInfo.L1DLineSize |= x86CPUIDTable[loopCounter].cacheInfo.L1DLineSize;
            g_oalCacheInfo.L1DSize |= x86CPUIDTable[loopCounter].cacheInfo.L1DSize;

            g_oalCacheInfo.L2Flags |= x86CPUIDTable[loopCounter].cacheInfo.L2Flags;
            g_oalCacheInfo.L2ISetsPerWay |= x86CPUIDTable[loopCounter].cacheInfo.L2ISetsPerWay;
            g_oalCacheInfo.L2INumWays |= x86CPUIDTable[loopCounter].cacheInfo.L2INumWays;
            g_oalCacheInfo.L2ILineSize |= x86CPUIDTable[loopCounter].cacheInfo.L2ILineSize;
            g_oalCacheInfo.L2ISize |= x86CPUIDTable[loopCounter].cacheInfo.L2ISize;
            g_oalCacheInfo.L2DSetsPerWay |= x86CPUIDTable[loopCounter].cacheInfo.L2DSetsPerWay;
            g_oalCacheInfo.L2DNumWays |= x86CPUIDTable[loopCounter].cacheInfo.L2DNumWays;
            g_oalCacheInfo.L2DLineSize |= x86CPUIDTable[loopCounter].cacheInfo.L2DLineSize;
            g_oalCacheInfo.L2DSize |= x86CPUIDTable[loopCounter].cacheInfo.L2DSize;
            return TRUE;
        }
    }

    // got an unexpected byte, either something is very wrong or our table is out of date
    return FALSE;
}



//------------------------------------------------------------------------------
//
//  Function:  interpretCPUIDCacheInfo
//
//  Updates g_oalCacheInfo with cache information based upon the input value.
//  Expected: the input value is a register produced by the CPUID instruction.
//  Returns TRUE for success, FALSE for failure.
//
static BOOL x86InterpretCPUIDValue(
                                   const UINT32 & cpuIDValue
                                   )
{
    BOOL rc = TRUE;

    //check most significant bit of cpuIDValue - if it is 0 then the value contains valid 1-byte descriptors
    if (cpuIDValue & 0x80000000)
    {
        return FALSE;
    }

    // valid descriptor, for each byte, scan the table
    // return true if we've successfully updated the global cache info based on all four bytes
    // Try to process all four bytes even if one of them fails to process
    // (when may happen whenever our table is out of date).
    if (ProcessDescriptorByte(cpuIDValue & 0xFF))
    {
        rc = FALSE;
    }

    if (ProcessDescriptorByte(cpuIDValue >> 8 & 0xFF))
    {
        rc = FALSE;
    }

    if (ProcessDescriptorByte(cpuIDValue >> 16 & 0xFF))
    {
        rc = FALSE;
    }

    if (ProcessDescriptorByte(cpuIDValue >> 24 & 0xFF))
    {
        rc = FALSE;
    }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  x86InitializeCPUIDTable
//
//  Fills in a static table based on the specification of the CPUID instruction.
//
static void x86InitializeCPUIDTable()
{
    // initialize all table entries to 0 so that we can use |= to update the global cache info later
    memset(x86CPUIDTable, 0, (sizeof(x86CPUIDPair)*NUMVALIDCPUIDDESCRIPTORS));

    x86CPUIDTable[0].descriptor = 0x00; // Null descriptor
    x86CPUIDTable[1].descriptor = 0x01; // Instruction TLB: 4 KByte Pages, 4-way set associative, 32 entries
    x86CPUIDTable[2].descriptor = 0x02; // Instruction TLB: 4 MByte Pages, 4-way set associative, 2 entries
    x86CPUIDTable[3].descriptor = 0x03; // Data TLB: 4 KByte Pages, 4-way set associative, 64 entries
    x86CPUIDTable[4].descriptor = 0x04; // Data TLB: 4 MByte Pages, 4-way set associative, 8 entries

    x86CPUIDTable[5].descriptor = 0x06; // 1st-level instruction cache: 8 KBytes, 4-way set associative, 32 byte line size
    x86CPUIDTable[5].cacheInfo.L1ISize = 0x2000;
    x86CPUIDTable[5].cacheInfo.L1INumWays = 0x4;
    x86CPUIDTable[5].cacheInfo.L1ILineSize = 0x20;

    x86CPUIDTable[6].descriptor = 0x08; // 1st-level instruction cache: 16 KBytes, 4-way set associative, 32 byte line size
    x86CPUIDTable[6].cacheInfo.L1ISize = 0x4000;
    x86CPUIDTable[6].cacheInfo.L1INumWays = 0x4;
    x86CPUIDTable[6].cacheInfo.L1ILineSize = 0x20;

    x86CPUIDTable[7].descriptor = 0x0A; // 1st-level data cache: 8 KBytes, 2-way set associative, 32 byte line size
    x86CPUIDTable[7].cacheInfo.L1DSize = 0x2000;
    x86CPUIDTable[7].cacheInfo.L1DNumWays = 0x2;
    x86CPUIDTable[7].cacheInfo.L1DLineSize = 0x20;

    x86CPUIDTable[8].descriptor = 0x0C; // 1st-level data cache: 16 KBytes, 4-way set associative, 32 byte line size
    x86CPUIDTable[8].cacheInfo.L1DSize = 0x4000;
    x86CPUIDTable[8].cacheInfo.L1DNumWays = 0x4;
    x86CPUIDTable[8].cacheInfo.L1DLineSize = 0x20;

    x86CPUIDTable[9].descriptor = 0x22; // 3rd-level cache: 512 KBytes, 4-way set associative, 64 byte line size, 2 lines per sector
    x86CPUIDTable[10].descriptor = 0x23; // 3rd-level cache: 1 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector
    x86CPUIDTable[11].descriptor = 0x25; // 3rd-level cache: 2 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector
    x86CPUIDTable[12].descriptor = 0x29; // 3rd-level cache: 4 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector

    x86CPUIDTable[13].descriptor = 0x2C; // 1st-level data cache: 32 KBytes, 8-way set associative, 64 byte line size
    x86CPUIDTable[13].cacheInfo.L1DSize = 0x8000;
    x86CPUIDTable[13].cacheInfo.L1DNumWays = 0x8;
    x86CPUIDTable[13].cacheInfo.L1DLineSize = 0x40;

    x86CPUIDTable[14].descriptor = 0x30; // 1st-level instruction cache: 32 KBytes, 8-way set associative, 64 byte line size
    x86CPUIDTable[14].cacheInfo.L1ISize = 0x8000;
    x86CPUIDTable[14].cacheInfo.L1INumWays = 0x8;
    x86CPUIDTable[14].cacheInfo.L1ILineSize = 0x40;

    x86CPUIDTable[15].descriptor = 0x40; // No 2nd-level cache or, if processor contains a valid 2nd-level cache, no 3rd-level cache
    x86CPUIDTable[16].descriptor = 0x41; // 2nd-level cache: 128 KBytes, 4-way set associative, 32 byte line size
    x86CPUIDTable[16].cacheInfo.L2DSize = 0x20000;
    x86CPUIDTable[16].cacheInfo.L2DNumWays = 0x4;
    x86CPUIDTable[16].cacheInfo.L2DLineSize = 0x20;

    x86CPUIDTable[17].descriptor = 0x42; // 2nd-level cache: 256 KBytes, 4-way set associative, 32 byte line size
    x86CPUIDTable[17].cacheInfo.L2DSize = 0x40000;
    x86CPUIDTable[17].cacheInfo.L2DNumWays = 0x4;
    x86CPUIDTable[17].cacheInfo.L2DLineSize = 0x20;

    x86CPUIDTable[18].descriptor = 0x43; // 2nd-level cache: 512 KBytes, 4-way set associative, 32 byte line size
    x86CPUIDTable[18].cacheInfo.L2DSize = 0x80000;
    x86CPUIDTable[18].cacheInfo.L2DNumWays = 0x4;
    x86CPUIDTable[18].cacheInfo.L2DLineSize = 0x20;

    x86CPUIDTable[19].descriptor = 0x44; // 2nd-level cache: 1 MByte, 4-way set associative, 32 byte line size
    x86CPUIDTable[19].cacheInfo.L2DSize = 0x100000;
    x86CPUIDTable[19].cacheInfo.L2DNumWays = 0x4;
    x86CPUIDTable[19].cacheInfo.L2DLineSize = 0x20;

    x86CPUIDTable[20].descriptor = 0x45; // 2nd-level cache: 2 MByte, 4-way set associative, 32 byte line size
    x86CPUIDTable[20].cacheInfo.L2DSize = 0x200000;
    x86CPUIDTable[20].cacheInfo.L2DNumWays = 0x4;
    x86CPUIDTable[20].cacheInfo.L2DLineSize = 0x20;

    x86CPUIDTable[21].descriptor = 0x46; // 3rd-level cache: 4 MByte, 4-way set associative, 64 byte line size
    x86CPUIDTable[22].descriptor = 0x47; // 3rd-level cache: 8 MByte, 8-way set associative, 64 byte line size

    x86CPUIDTable[23].descriptor = 0x50; // Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 64 entries
    x86CPUIDTable[24].descriptor = 0x51; // Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 128 entries
    x86CPUIDTable[25].descriptor = 0x52; // Instruction TLB: 4 KByte and 2-MByte or 4-MByte pages, 256 entries
    x86CPUIDTable[26].descriptor = 0x5B; // Data TLB: 4 KByte and 4 MByte pages, 64 entries
    x86CPUIDTable[27].descriptor = 0x5C; // Data TLB: 4 KByte and 4 MByte pages,128 entries
    x86CPUIDTable[28].descriptor = 0x5D; // Data TLB: 4 KByte and 4 MByte pages,256 entries

    x86CPUIDTable[29].descriptor = 0x60; // 1st-level data cache: 16 KByte, 8-way set associative, 64 byte line size
    x86CPUIDTable[29].cacheInfo.L1DSize = 0x4000;
    x86CPUIDTable[29].cacheInfo.L1DNumWays = 0x8;
    x86CPUIDTable[29].cacheInfo.L1DLineSize = 0x40;

    x86CPUIDTable[30].descriptor = 0x66; // 1st-level data cache: 8 KByte, 4-way set associative, 64 byte line size
    x86CPUIDTable[30].cacheInfo.L1DSize = 0x2000;
    x86CPUIDTable[30].cacheInfo.L1DNumWays = 0x4;
    x86CPUIDTable[30].cacheInfo.L1DLineSize = 0x40;

    x86CPUIDTable[31].descriptor = 0x67; // 1st-level data cache: 16 KByte, 4-way set associative, 64 byte line size
    x86CPUIDTable[31].cacheInfo.L1DSize = 0x4000;
    x86CPUIDTable[31].cacheInfo.L1DNumWays = 0x4;
    x86CPUIDTable[31].cacheInfo.L1DLineSize = 0x40;

    x86CPUIDTable[32].descriptor = 0x68; // 1st-level data cache: 32 KByte, 4-way set associative, 64 byte line size
    x86CPUIDTable[32].cacheInfo.L1DSize = 0x8000;
    x86CPUIDTable[32].cacheInfo.L1DNumWays = 0x4;
    x86CPUIDTable[32].cacheInfo.L1DLineSize = 0x40;

    x86CPUIDTable[33].descriptor = 0x70; // Trace cache: 12 K-micro-op, 8-way set associative
    x86CPUIDTable[34].descriptor = 0x71; // Trace cache: 16 K-micro-op, 8-way set associative
    x86CPUIDTable[35].descriptor = 0x72; // Trace cache: 32 K-micro-op, 8-way set associative

    x86CPUIDTable[36].descriptor = 0x78; // 2nd-level cache: 1 MByte, 4-way set associative, 64byte line size
    x86CPUIDTable[36].cacheInfo.L2DSize = 0x100000;
    x86CPUIDTable[36].cacheInfo.L2DNumWays = 0x4;
    x86CPUIDTable[36].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[36].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[37].descriptor = 0x79; // 2nd-level cache: 128 KByte, 8-way set associative, 64 byte line size, 2 lines per sector
    x86CPUIDTable[37].cacheInfo.L2DSize = 0x20000;
    x86CPUIDTable[37].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[37].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[37].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[38].descriptor = 0x7A; // 2nd-level cache: 256 KByte, 8-way set associative, 64 byte line size, 2 lines per sector
    x86CPUIDTable[38].cacheInfo.L2DSize = 0x40000;
    x86CPUIDTable[38].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[38].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[38].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[39].descriptor = 0x7B; // 2nd-level cache: 512 KByte, 8-way set associative, 64 byte line size, 2 lines per sector
    x86CPUIDTable[39].cacheInfo.L2DSize = 0x80000;
    x86CPUIDTable[39].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[39].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[39].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[40].descriptor = 0x7C; // 2nd-level cache: 1 MByte, 8-way set associative, 64 byte line size, 2 lines per sector
    x86CPUIDTable[40].cacheInfo.L2DSize = 0x100000;
    x86CPUIDTable[40].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[40].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[40].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[41].descriptor = 0x7D; // 2nd-level cache: 2 MByte, 8-way set associative, 64byte line size
    x86CPUIDTable[41].cacheInfo.L2DSize = 0x200000;
    x86CPUIDTable[41].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[41].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[41].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[42].descriptor = 0x7F; // 2nd-level cache: 512 KByte, 2-way set associative, 64-byte line size
    x86CPUIDTable[42].cacheInfo.L2DSize = 0x80000;
    x86CPUIDTable[42].cacheInfo.L2DNumWays = 0x2;
    x86CPUIDTable[42].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[42].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[43].descriptor = 0x82; // 2nd-level cache: 256 KByte, 8-way set associative, 32 byte line size
    x86CPUIDTable[43].cacheInfo.L2DSize = 0x40000;
    x86CPUIDTable[43].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[43].cacheInfo.L2DLineSize = 0x20;
    x86CPUIDTable[43].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[44].descriptor = 0x83; // 2nd-level cache: 512 KByte, 8-way set associative, 32 byte line size
    x86CPUIDTable[44].cacheInfo.L2DSize = 0x80000;
    x86CPUIDTable[44].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[44].cacheInfo.L2DLineSize = 0x20;
    x86CPUIDTable[44].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[45].descriptor = 0x84; // 2nd-level cache: 1 MByte, 8-way set associative, 32 byte line size
    x86CPUIDTable[45].cacheInfo.L2DSize = 0x100000;
    x86CPUIDTable[45].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[45].cacheInfo.L2DLineSize = 0x20;
    x86CPUIDTable[45].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[46].descriptor = 0x85; // 2nd-level cache: 2 MByte, 8-way set associative, 32 byte line size
    x86CPUIDTable[46].cacheInfo.L2DSize = 0x200000;
    x86CPUIDTable[46].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[46].cacheInfo.L2DLineSize = 0x20;
    x86CPUIDTable[46].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[47].descriptor = 0x86; // 2nd-level cache: 512 KByte, 4-way set associative, 64 byte line size
    x86CPUIDTable[47].cacheInfo.L2DSize = 0x80000;
    x86CPUIDTable[47].cacheInfo.L2DNumWays = 0x4;
    x86CPUIDTable[47].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[47].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[48].descriptor = 0x87; // 2nd-level cache: 1 MByte, 8-way set associative, 64 byte line size
    x86CPUIDTable[48].cacheInfo.L2DSize = 0x100000;
    x86CPUIDTable[48].cacheInfo.L2DNumWays = 0x8;
    x86CPUIDTable[48].cacheInfo.L2DLineSize = 0x40;
    x86CPUIDTable[48].cacheInfo.L2Flags |= CF_UNIFIED;

    x86CPUIDTable[49].descriptor = 0xB0; // Instruction TLB: 4 KByte Pages, 4-way set associative, 128 entries
    x86CPUIDTable[50].descriptor = 0xB3; // Data TLB: 4 KByte Pages, 4-way set associative, 128 entries
    x86CPUIDTable[51].descriptor = 0xB4; // Data TLB, 4K pages, 4 ways, 256 entries
    x86CPUIDTable[52].descriptor = 0xBA; // Data TLB, 4K pages, 4 ways, 64 entries
    x86CPUIDTable[53].descriptor = 0xC0; // Data TLB, 4K/4M pages, 4 ways, 8 entries

    x86CPUIDTable[54].descriptor = 0xF0; // 64-Byte Prefetching
    x86CPUIDTable[55].descriptor = 0xF1; // 128-Byte Prefetching
}

//------------------------------------------------------------------------------
//
//  Function:  x86IoCtlGetCacheInfo
//
//  Implements the IOCTL_HAL_GET_CACHE_INFO. This function fills in a
//  CacheInfo structure.
//
extern "C" BOOL x86IoCtlHalGetCacheInfo(
                                        UINT32 /*code*/, 
                                        void * /*pInpBuffer*/, 
                                        UINT32 /*inpSize*/, 
                                        __out_bcount(outSize) void *pOutBuffer,
                                        UINT32 outSize, 
                                        __out UINT32 *pOutSize
                                        ) 
{
    BOOL rc = FALSE;
    CacheInfo *pInfo = (CacheInfo*)pOutBuffer;

    OALMSG(OAL_FUNC, (L"+x86IoctlGetCacheInfo(...)\r\n"));

    if (pOutSize) 
    {
        *pOutSize = sizeof(CacheInfo);
    }
    if (pOutBuffer == NULL && outSize > 0) 
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: x86IoctlGetDeviceCacheInfo: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    if (pOutBuffer == NULL || outSize < sizeof(CacheInfo)) 
    {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (
            L"WARN: x86IoctlGetDeviceCacheInfo: Buffer too small\r\n"
        ));
        goto cleanUp;
    }

    // clear our output struct
    memset(pInfo, 0, sizeof(CacheInfo));

    // set up our table so we can interpret the cpu's return values
    x86InitializeCPUIDTable();

    // this loop reads the cache info from the cpu
    UINT32 iterationNumber = 0;
    BOOL fCacheInfoObtained = FALSE;
    while(iterationNumber < MAXCPUIIDITERATIONS)
    {
        UINT32 CPUIDTempBuffer[4] = {0, 0, 0, 0};

        __try {
            __asm {
            mov eax, 02h
            cpuid                                  ; when eax is 02h we are querying for cache info
            mov CPUIDTempBuffer, eax     ; cpuid fills eax, ebx, ecx, and edx with descriptors of cache
            mov CPUIDTempBuffer+4, ebx
            mov CPUIDTempBuffer+8, ecx
            mov CPUIDTempBuffer+12, edx
            }
        }
        __except (GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            //There was an error in calling CPUID which means we have no cache info to return.
            NKSetLastError(ERROR_NOT_SUPPORTED);
            OALMSG(OAL_ERROR, (
                L"ERROR: x86IoctlGetDeviceCacheInfo: CPUID instruction failed, no cache info available\r\n"
            ));
            break;
        }

        // CPUID returned empty, so the current CPU may not support 0000_0002h
        // we will try a different CPUID (8000_0005h and 8000_0006h)
        if (CPUIDTempBuffer[0] == 0 && CPUIDTempBuffer[1] == 0 &&
            CPUIDTempBuffer[2] == 0 && CPUIDTempBuffer[3] == 0)
        {
            break;
        }

        x86InterpretCPUIDValue(CPUIDTempBuffer[0]);
        x86InterpretCPUIDValue(CPUIDTempBuffer[1]);
        x86InterpretCPUIDValue(CPUIDTempBuffer[2]);
        x86InterpretCPUIDValue(CPUIDTempBuffer[3]);

        iterationNumber++;

        // If the least significant byte of eax is 01, then no further calls are necessary to retrieve
        // cache information.  Otherwise there is more cache info and we need to make another call.
        if((CPUIDTempBuffer[0] & 0x000000FF) == 0x01)
        {
            fCacheInfoObtained = TRUE;
            break;
        }
    }
        
    // We could not get Cache Info Using 0000_0002h CPUID, let's try to get the
    // cache info using 8000_0005h and 8000_0006h
    if (!fCacheInfoObtained)
    {
        UINT32 CPUIDTempBuffer[3];
        __try {
            __asm {
            mov eax, 80000005h
            cpuid                          ; when eax is 8000_0005h we are querying for L1 cache info
            mov CPUIDTempBuffer, ecx
            mov CPUIDTempBuffer+4, edx
            mov eax, 80000006h
            cpuid                          ; when eax is 8000_0006h we are querying for L2 cache info
            mov CPUIDTempBuffer+8, ecx
            }
        } 
        __except (GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            //There was an error in calling CPUID which means we have no cache info to return.
            NKSetLastError(ERROR_NOT_SUPPORTED);
            OALMSG(OAL_ERROR, (
                L"ERROR: x86IoctlGetDeviceCacheInfo: CPUID instruction failed, no cache info available\r\n"
            ));
            goto cleanUp;
        }

        // Cache Info shouldn’t overlap, so we can use logical OR to track them
        g_oalCacheInfo.L1DSize |= ((CPUIDTempBuffer[0] >> 24) & 0xFF) * 1024; //convert from KB to Bytes
        g_oalCacheInfo.L1DLineSize |= (CPUIDTempBuffer[0] & 0xFF);
        g_oalCacheInfo.L1DNumWays |= ((CPUIDTempBuffer[0] >> 16) & 0xFF);
        g_oalCacheInfo.L1ISize |= ((CPUIDTempBuffer[1] >> 24) & 0xFF) * 1024; //convert from KB to Bytes
        g_oalCacheInfo.L1ILineSize |= (CPUIDTempBuffer[1] & 0xFF);
        g_oalCacheInfo.L1INumWays |= ((CPUIDTempBuffer[1] >> 16) & 0xFF);

        g_oalCacheInfo.L2DSize |= ((CPUIDTempBuffer[2] >> 16) & 0xFFFF) * 1024 ; //convert from KB to Bytes
        g_oalCacheInfo.L2DLineSize |= CPUIDTempBuffer[2] & 0xFF;
        g_oalCacheInfo.L2DNumWays |= MapCacheAssoc((CPUIDTempBuffer[2] >> 12) & 0xF);
        
        if (g_oalCacheInfo.L2DSize != 0 || g_oalCacheInfo.L2DLineSize != 0 ||
            g_oalCacheInfo.L2DNumWays != 0)
        {
            g_oalCacheInfo.L2Flags |= CF_UNIFIED;
            // leave instruction caches as 0 since unified
        }
    }

    // copy from our global cache struct into the slightly smaller struct we're returning
    pInfo->dwL1Flags = g_oalCacheInfo.L1Flags;
    pInfo->dwL1ICacheLineSize = g_oalCacheInfo.L1ILineSize;
    pInfo->dwL1ICacheNumWays = g_oalCacheInfo.L1INumWays;
    pInfo->dwL1ICacheSize = g_oalCacheInfo.L1ISize;
    pInfo->dwL1DCacheLineSize = g_oalCacheInfo.L1DLineSize;
    pInfo->dwL1DCacheNumWays = g_oalCacheInfo.L1DNumWays;
    pInfo->dwL1DCacheSize = g_oalCacheInfo.L1DSize;

    pInfo->dwL2Flags = g_oalCacheInfo.L2Flags;
    pInfo->dwL2ICacheLineSize = g_oalCacheInfo.L2ILineSize;
    pInfo->dwL2ICacheNumWays = g_oalCacheInfo.L2INumWays;
    pInfo->dwL2ICacheSize = g_oalCacheInfo.L2ISize;
    pInfo->dwL2DCacheLineSize = g_oalCacheInfo.L2DLineSize;
    pInfo->dwL2DCacheNumWays = g_oalCacheInfo.L2DNumWays;
    pInfo->dwL2DCacheSize = g_oalCacheInfo.L2DSize;

    rc = TRUE;

cleanUp:
    OALMSG(OAL_FUNC, (L"-x86IoctlGetCacheInfo(rc = %d)\r\n", rc));
    return rc;
}
