//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  nand_hal.h
//
//
//
//-----------------------------------------------------------------------------

#ifndef _NAND_HAL_H
#define _NAND_HAL_H

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////

#include "windef.h"
#include "windows.h"
#include "common_nandfmd.h"
///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

#define MAX_NAND_DEVICES 4

// Timeouts -- TBD -- These either get replaced altogether, or go somewhere else.
// WaitForREADY() timeout counts
// These control how long we will wait for the NAND to finish some internal operation
// (e.g. Read Page (to cache), Program Page (from cache), Erase Block, Reset).
// NOTE: For timeouts, each "count" is a microsecond..
// The settings below are probably very conservative, but still short in human time
#define NAND_READ_PAGE_TIMEOUT      (2000000)
#define NAND_RESET_TIMEOUT          (10000000)
#define NAND_WRITE_PAGE_TIMEOUT     (10000000)
#define NAND_ERASE_BLOCK_TIMEOUT    (40000000)

typedef enum {
    eNandProgCmdReadID                    = 0x000090,
    eNandProgCmdReadID2                   = 0x000091,
    eNandProgCmdReadStatus                = 0x000070,
    eNandProgCmdReset                     = 0x0000ff,
    eNandProgCmdSerialDataInput           = 0x000080,   // Page Program/Cache Program
    eNandProgCmdRead1                     = 0x000000,   // Read or Read for CopyBack
    eNandProgCmdRead1_2ndCycle            = 0x000030,   // Second Cycle for Read (Type 2 NANDs)
    eNandProgCmdReadForCopyBack_2ndCycle  = 0x000035,   // Second Cycle for Read for Copy Back
    eNandProgCmdReadForCacheCopyback_2nd  = 0x00003A,
    eNandProgCmdRead2                     = 0x000001,
    eNandProgCmdRead3                     = 0x000050,
    eNandProgCmdPageProgram               = 0x000010,   // Second cycle for wSerialDataInput for Page 
    eNandProgCmdPartialPageProgram        = 0x000011,
    eNandProgCmdCacheProgram              = 0x000015,   // Second cycle for wSerialDataInput for Cache Program
    eNandProgCmdCopyBackProgram           = 0x000085,
    eNandProgCmdCopyBack2Program          = 0x00008C,
    eNandProgCmdCopyBackProgram_2ndCycle  = 0x000010,   // Second cycle for Copy Back Program
    eNandProgCmdBlockErase                = 0x000060,
    eNandProgCmdBlockErase_2ndCycle       = 0x0000d0,
    eNandProgCmdRandomDataIn              = 0x000085,
    eNandProgCmdRandomDataOut             = 0x000005,
    eNandProgCmdRandomDataOut_2ndCycle    = 0x0000E0,
    eNandProgCmdReadMultiPlaneStatus      = 0x000071,   // MLC MultiPlane
    eNandProgCmdReadErrorStatus           = 0x000072,   // MLC Single Plane Error Status
    eNandProgCmdReadMultiPlaneErrorStatus = 0x000073,   // MLC MultiPlane Error Status.
    eNandProgCmdMultiPlaneWrite           = 0x000011,
    eNandProgCmdStatusModeReset           = 0x00007F,
    eNandProgCmdMultiPlaneRead_2ndCycle   = 0x000031,
    eNandProgCmdPageDataOutput            = 0x000006,
    eNandProgCmdMultiPlaneBlockErase      = 0x00ffff,   // TBD !!! Need code for this.
    eNandProgCmdNone                      = 0x7FFFFF,   // invalid entry
    eNandProgCmdDummyProgram              = 0x6FFFFF    // invalid entry
} NAND_PROGRAM_CODES;

typedef union {                     // All fields in nanoseconds

    // By placing this word before the bitfield it allows structure copies to be done
    //  safely by assignment rather than by memcpy.

    INT initializer;

    // These field lengths are arbitrary... they only need to be large enough to hold
    //  the values.  Possible optimization by using enumerations rather than the actual
    //  values here and teaching NANDHalSetFlashTiming how to interpret them.

    //! \brief NAND Timing structure for setting up the GPMI timing.
    //!
    //! This structure holds the timing for the NAND.  This data is used by
    //! GPMI_SetTiming to setup the GPMI hardware registers.
    NANDTiming NAND_Timing;
} NAND_Timing_t;


typedef struct _id_decode
{
    // Read ID Byte 1
    UINT32 MakerCode              : 8;
    // Read ID Byte 2
    UINT32 DeviceCode             : 8;
    // Read ID Byte 3
    UINT32 InternalChipNumber     : 2;        // Number of die = (1 << n)
    UINT32 CellType               : 2;        // Number of bits per memory cell = ( 1 << (n+1) )
    UINT32 VendorSpecific0        : 3;
    UINT32 CacheProgram           : 1;        // 0 == Not supported
    // Read ID Byte 4
    UINT32 PageSize               : 2;        // Page size in bytes = (1 << n) * 1024
    UINT32 RedundantAreaSize      : 1;        // Redundant area bytes per 512 data bytes = 8 * (1 << n)
    UINT32 Reserved0              : 1;
    UINT32 BlockSize              : 2;        // Block size in bytes = 64 * 1024 * (1 << n)
    UINT32 Organization           : 1;        // 0 == x8, 1 == x16
    UINT32 SamsungHSSerialAccess  : 1;        // 0 == 50/30ns, 1 == 25ns
    // Read ID Byte 5
    UINT32 VendorSpecific1        : 2;
    UINT32 PlaneNumber            : 2;        // # of planes total (see note below) = (1 << n)
    UINT32 PlaneSize              : 3;        // # of bytes per plane = 64 * 1024 * 1024 * (1 << n)
    UINT32 Reserved4              : 1;
    // Read ID Byte 6
    UINT32 Reserved5              : 3;
    UINT32 ToshibaHighSpeedMode   : 1;        // 0 == Not supported
    UINT32 Reserved6              : 4;
} t_id_decode;

extern PVOID pv_HWregGPMI;
extern PVOID pv_HWregBCH;
extern PVOID pv_HWregDIGCTL;

#endif // #ifndef _NAND_HAL_H


