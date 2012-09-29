//-----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  atamain.h
//
//  ATA/ATAPI device driver definitions.
//
//------------------------------------------------------------------------------

#ifndef _ATAMAIN_H_
#define _ATAMAIN_H_

#include <windows.h>
#include <ceddk.h>
#include <devload.h>
#include <ddkreg.h>
#include <nkintr.h>
#include <celog.h>
#include <diskio.h>
#include <atapi2.h>
#include <cdioctl.h>
#include <dvdioctl.h>
#include <atapiio.h>
#include <helper.h>
#include <debug.h>
#include <diskmain.h>
#include <storemgr.h>
#include <winnt.h>
#include <scsi.h>
#include <ntddscsi.h>
#include <ntddmmc.h>

#define MIN(a,b)                          ((a)<(b)?(a):(b))
#define SECTOR_SIZE                       512

#define MAX_RESET_ATTEMPTS                256
#define MAX_SECT_PER_COMMAND              256
#define MAX_CD_SECT_PER_COMMAND           32
#define MAX_SECT_PER_EXT_COMMAND          65536   // As per the 48 bit LBA ATA spec

// IDE/ATA controller subkey names for device enumeration
#define REG_KEY_PRIMARY_MASTER            (_T("Device0"))
#define REG_KEY_PRIMARY_SLAVE             (_T("Device1"))
#define REG_KEY_SECONDARY_MASTER          (_T("Device2"))
#define REG_KEY_SECONDARY_SLAVE           (_T("Device3"))

// ATA/ATAPI device registry value definitions
#define REG_VAL_DSK_PORT             (_T("Port"))             // heap address of associated CPort instance
#define REG_VAL_DSK_INTERRUPTDRIVEN  (_T("InterruptDriven"))  // {0, 1}; 0 => polled I/O, 1 => interrupt I/O
#define REG_VAL_DSK_DMA              (_T("DMA"))              // {0, 1}; 0 => PIO, 1 => DMA, 2 => ATA DMA only
#define REG_VAL_DSK_DOUBLEBUFFERSIZE (_T("DoubleBufferSize")) // {512, ..., 131072}; scatter/gather processing
#define REG_VAL_DSK_DRQDATABLOCKSIZE (_T("DrqDataBlockSize")) // {512}; when we support R/W multiple, increase
#define REG_VAL_DSK_WRITECACHE       (_T("WriteCache"))       // {0, 1}; 0 => disable write cache; 1 => enable write cache
#define REG_VAL_DSK_LOOKAHEAD        (_T("LookAhead"))        // {0, 1}; 0 => disable look-ahead; 1 => enable look-ahead
#define REG_VAL_DSK_DEVICEID         (_T("DeviceId"))         // initially (0, 1, 2, 3), then resolve re-written as (0, 1); 0 => master, 1 => slave
#define REG_VAL_DSK_TRANSFERMODE     (_T("TransferMode"))     // 1 byte transfer mode encoding; see ATA/ATAPI 8.46.11; 0xFF is default mode
#define REG_VAL_DSK_IORDYENABLE      (_T("IORDYEnable"))      // 0 to disable Host IORDY 

#define REG_VAL_DSK_DOUBLEBUFFERSIZE_MAX 131072
#define REG_VAL_DSK_DOUBLEBUFFERSIZE_MIN 512
#define REG_VAL_DSK_DRQDATABLOCKSIZE_MAX 130560
#define REG_VAL_DSK_DRQDATABLOCKSIZE_MIN 512

// DSK_ registry value set
typedef struct _DSKREG {
    DWORD dwInterruptDriven;
    DWORD dwDMA;
    DWORD dwDoubleBufferSize;
    DWORD dwDrqDataBlockSize;
    DWORD dwWriteCache;
    DWORD dwLookAhead;
    DWORD dwDeviceId;
    DWORD dwTransferMode;
    DWORD dwIORDYEnable;
} DSKREG, *PDSKREG;

// Populate DSK_ registry value set from registry
BOOL
GetDSKRegistryValueSet(
    HKEY hDSKInstanceKey,
    PDSKREG pDskReg
    );

// Registry configuration value names
#define REG_VALUE_IOBASEADDRESS     TEXT("IOBaseAddress")    // not used
#define REG_VALUE_BMR               TEXT("BMR")              // not used
#define REG_VALUE_INTERRUPT         TEXT("Interrupt")        // not used
#define REG_VALUE_IRQ               TEXT("IRQ")              // not used
#define REG_VALUE_DVD               TEXT("DVD")              // read in diskmain.cpp!ReadSettings; not used
#define REG_VALUE_CHS               TEXT("CHSMode")          // read in diskmain.cpp!Identify
#define REG_VALUE_SYSINTR           TEXT("SysIntr")          // not used, but should be used in atamain.cpp
#define REG_VALUE_INTENABLE         TEXT("IntEnable")        // read in diskmain.cpp!ReadSettings; should be read in atamain.cpp
#define REG_VALUE_HDPROFILE         TEXT("HDProfile")        // read in diskmain.cpp!GetDeviceInfo; why can't this just be "StorageManagerProfile"?
#define REG_VALUE_CDPROFILE         TEXT("CDProfile")        // read in diskmain.cpp!GetDeviceInfo; why can't this just be "StorageManagerProfile"?
#define REG_VALUE_PCMCIAPROFILE     TEXT("PCMCIA")           // not used
#define REG_VALUE_ENABLE_WRITECACHE TEXT("EnableWriteCache") // read in diskmain.cpp!ReadSettings

// Registry configurable DMA alignment
#define REG_VALUE_DMA_ALIGN         TEXT("DMAAlignment")     // read in diskmain.cpp!ReadSettings; should be read in atamain.cpp
#define DEFAULT_DMA_ALIGN_VALUE     4

// Registry configurable timeout values
#define REG_VALUE_MEDIACHECKTIME    TEXT("MediaCheckTime")      // read in diskmain.cpp!ReadSettings
#define REG_VALUE_WAIT_CHECK_ITER   TEXT("WaitCheckIterations") // read in diskmain.cpp!ReadRegistry; why?
#define REG_VALUE_WAIT_SAMPLE_TIMES TEXT("WaitSampleTimes")     // read in diskmain.cpp!ReadRegistry; why?
#define REG_VALUE_WAIT_STALL_TIME   TEXT("WaitStallTime")       // read in diskmain.cpp!ReadRegistry; why?
#define REG_VALUE_DISK_IO_TIME_OUT  TEXT("DiskIOTimeOut")       // read in diskmain.cpp!ReadRegistry; why?

#define DEFAULT_MEDIA_CHECK_TIME    5000
#define DEFAULT_WAIT_CHECK_ITER     2000
#define DEFAULT_WAIT_SAMPLE_TIMES   100
#define DEFAULT_WAIT_STALL_TIME     400
#define DEFAULT_DISK_IO_TIME_OUT    20000

// "Settings" registry value
#define REG_VALUE_SETTINGS          TEXT("Settings")
#define REG_VALUE_DMA               TEXT("DMA")
#define ATA_SETTINGS_HDDMA          0x1 // Hard disk DMA enabled
#define ATA_SETTINGS_CDDMA          0x4 // CD-ROM/DVD DMA enabled
#define ATA_SETTINGS_HDINT          0x2 // Hard disk interrupt enabled
#define ATA_SETTINGS_CDINT          0x8 // CD-ROM/DVD interrupt enabled

#define REG_VALUE_PORT              (_T("Port"))

// Helper function prototype
BOOL
AtaIsValidDisk(
    CDisk *pDisk
    );


// Bus master definitions
#define BM_STATUS_SIMPLEX 0x80
#define BM_STATUS_D1_DMA  0x40
#define BM_STATUS_D0_DMA  0x20
#define BM_STATUS_INTR    0x04
#define BM_STATUS_ERROR   0x02
#define BM_STATUS_ACTIVE  0x01

// DMA support structures

typedef struct {
    LPBYTE pDstAddress;
    LPBYTE pSrcAddress;
    DWORD dwSize;
} SGCopyTable, *PSGCopyTable;

typedef struct {
    DWORD dwVirtualAddress;
    DWORD dwPhysicalAddress;
    DWORD dwFlags;
    DWORD dwSize;
} MEMTable, *PMEMTable;

typedef struct {
    DWORD physAddr;
    USHORT size;
    USHORT EOTpad;
} DMATable, *PDMATable;

#define MIN_PHYS_PAGES 4

typedef struct _PhysTable {
    LPBYTE pVirtualAddress;
    LPBYTE pPhysicalAddress;
} PhysTable, *PPhysTable;

// IDE/ATA channel abstraction
class CPort {
  public:
    // member variables
    CRITICAL_SECTION  m_csPort;        // protect access to I/O ports
    DWORD             m_fInitialized;  // whether port has been initialized by IDE driver
    DWORD             m_dwFlag;        // m_dwFlag
    DWORD             m_dwRegBase;     // base virtual address of command I/O port
    DWORD             m_dwRegAlt;      // base virtual address of status I/O port
    DWORD             m_dwBMR;         // base virtual address of bus master I/O port
    DWORD             m_dwBMRStatic;   // base physical address of bus master I/O port
    HANDLE            m_hIRQEvent;     // IRQ event handle
    PDSKREG           m_pDskReg[2];    // DSK_ registry value set for master, slave
    // not used
    HANDLE            m_hThread;       // not used; IST handle
    HANDLE            m_pDisk[2];      // only used by Promise; store handle?

    HANDLE            m_hWaitEvent;
    HANDLE            m_hIntrServThread;
    

    // constructors/destructors
    CPort();
    ~CPort();
    // member functions
    void TakeCS();
    void ReleaseCS();
    void PrintInfo();
};

#endif _ATAMAIN_H_
