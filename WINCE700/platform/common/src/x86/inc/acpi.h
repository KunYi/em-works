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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef enum {
    ACPI_TABLE_APIC = 'CIPA',
    ACPI_TABLE_DSDT = 'TDSD',
    ACPI_TABLE_ECDT = 'TDCE',
    ACPI_TABLE_FACP = 'PCAF',
    ACPI_TABLE_FACS = 'FCAF',
    ACPI_TABLE_OEMB = 'BMEO',
    ACPI_TABLE_PSDT = 'TDSP',
    ACPI_TABLE_RSDT = 'TDSR',
    ACPI_TABLE_SBST = 'TSBS',
    ACPI_TABLE_SLIT = 'TILS',
    ACPI_TABLE_SRAT = 'TARS',
    ACPI_TABLE_SSDT = 'TDSS',
    ACPI_TABLE_XSDT = 'TDSX',
    
    ACPI_TABLE_BERT = 'TREB',
    ACPI_TABLE_BOOT = 'TOOB',
    ACPI_TABLE_CPEP = 'PEPC',
    ACPI_TABLE_DBGP = 'PGBD',
    ACPI_TABLE_DMAR = 'RAMD',
    ACPI_TABLE_ERST = 'TSRE',
    ACPI_TABLE_HEST = 'TSEH',
    ACPI_TABLE_HPET = 'TEPH',
    ACPI_TABLE_IBFT = 'TFBI',
    ACPI_TABLE_MCFG = 'GFMC',
    ACPI_TABLE_SPCR = 'RCPS',
    ACPI_TABLE_SPMI = 'IMPS',
    ACPI_TABLE_TCPA = 'APCT',
    ACPI_TABLE_UEFI = 'IFEU',
    ACPI_TABLE_WAET = 'TEAW',
    ACPI_TABLE_WDAT = 'TADW',
    ACPI_TABLE_WDRT = 'TRDW',
    ACPI_TABLE_WSPT = 'TPSW',

    ACPI_TABLE_NULL = 0
} ACPI_TABLE;
#pragma pack(push, 1)
typedef struct  {
    ACPI_TABLE  Signature;
    DWORD dwLength;
    BYTE  bRevision;
    BYTE  bChecksum;
    char  sOemId[6];
    char  sManId[8];
    DWORD dwOemRev;
    char  sCreateId[4];
    DWORD dwCreateRev;
} AcpiTable;


typedef struct  {
    DWORD FIRMWARE_CTRL;
    DWORD DSDT;
    BYTE  Reserved1; // was INT_MODEL
    BYTE  Prefered_PM_Profile;
    WORD  SCI_INT;
    DWORD SMI_CMD;
    BYTE  ACPI_ENABLE;
    BYTE  ACPI_DISABLE;
    BYTE  S4BIOS_REQ;
    BYTE  PSTATE_CNT;
    DWORD PM1a_EVT_BLK;
    DWORD PM1b_EVT_BLK;
    DWORD PM1a_CNT_BLK;
    DWORD PM1b_CNT_BLK;
    DWORD PM2_CNT_BLK;
    DWORD PM_TMR_BLK;
    DWORD GPE0_BLK;
    DWORD GPE1_BLK;
    BYTE  PM1_EVT_LEN;
    BYTE  PM1_CNT_LEN;
    BYTE  PM2_CNT_LEN;
    BYTE  PM_TMR_LEN;
    BYTE  GPE0_BLK_LEN;
    BYTE  GPE1_BLK_LEN;
    BYTE  GPE1_BASE;
    BYTE  CST_CNT;
    WORD  P_LVL2_LAT;
    WORD  P_LVL3_LAT;
    WORD  FLUSH_SIZE;
    WORD  FLUSH_STIDE;
    BYTE  DUTY_OFFSET;
    BYTE  DUTY_WIDTH;
    BYTE  DAY_ALRM;
    BYTE  MON_ALRM;
    BYTE  CENTURY;
    WORD  IAPC_BOOT_ARCH;
    BYTE  Reserved2;
    DWORD Flags;
    BYTE  RESET_REG_AddressSpace; 
    BYTE  RESET_REG_BitWidth;
    BYTE  RESET_REG_BitOffset;
    BYTE  RESET_REG_AccessSize;
    ULARGE_INTEGER RESET_REG_Address;
    BYTE  RESET_VALUE;
    BYTE  Reserved3[3];
    ULARGE_INTEGER X_FIRMWARE_CTRL;
    ULARGE_INTEGER X_DSDT;
    BYTE  X_PM1a_EVT_BLK_AddressSpace; 
    BYTE  X_PM1a_EVT_BLK_BitWidth;
    BYTE  X_PM1a_EVT_BLK_BitOffset;
    BYTE  X_PM1a_EVT_BLK_AccessSize;
    ULARGE_INTEGER X_PM1a_EVT_BLK_Address;
    BYTE  X_PM1b_EVT_BLK_AddressSpace; 
    BYTE  X_PM1b_EVT_BLK_BitWidth;
    BYTE  X_PM1b_EVT_BLK_BitOffset;
    BYTE  X_PM1b_EVT_BLK_AccessSize;
    ULARGE_INTEGER X_PM1b_EVT_BLK_Address;
    BYTE  X_PM1a_CNT_BLK_AddressSpace; 
    BYTE  X_PM1a_CNT_BLK_BitWidth;
    BYTE  X_PM1a_CNT_BLK_BitOffset;
    BYTE  X_PM1a_CNT_BLK_AccessSize;
    ULARGE_INTEGER X_PM1a_CNT_BLK_Address;
    BYTE  X_PM1b_CNT_BLK_AddressSpace; 
    BYTE  X_PM1b_CNT_BLK_BitWidth;
    BYTE  X_PM1b_CNT_BLK_BitOffset;
    BYTE  X_PM1b_CNT_BLK_AccessSize;
    ULARGE_INTEGER X_PM1b_CNT_BLK_Address;
    BYTE  X_PM2_CNT_BLK_AddressSpace; 
    BYTE  X_PM2_CNT_BLK_BitWidth;
    BYTE  X_PM2_CNT_BLK_BitOffset;
    BYTE  X_PM2_CNT_BLK_AccessSize;
    ULARGE_INTEGER X_PM2_CNT_BLK_Address;
    BYTE  X_PM_TMR_BLK_AddressSpace; 
    BYTE  X_PM_TMR_BLK_BitWidth;
    BYTE  X_PM_TMR_BLK_BitOffset;
    BYTE  X_PM_TMR_BLK_AccessSize;
    ULARGE_INTEGER X_PM_TMR_BLK_Address;
    BYTE  X_GPE0_BLK_AddressSpace; 
    BYTE  X_GPE0_BLK_BitWidth;
    BYTE  X_GPE0_BLK_BitOffset;
    BYTE  X_GPE0_BLK_AccessSize;
    ULARGE_INTEGER X_GPE0_BLK_Address;
    BYTE  X_GPE1_BLK_AddressSpace; 
    BYTE  X_GPE1_BLK_BitWidth;
    BYTE  X_GPE1_BLK_BitOffset;
    BYTE  X_GPE1_BLK_AccessSize;
    ULARGE_INTEGER X_GPE1_BLK_Address;
} FADT;

typedef struct {
    DWORD cbSize;
    
    BOOL  fAcpiFound;
    DWORD dwAcpiVersion;
    DWORD dwAcpiTablesStart;
    DWORD dwAcpiTablesEnd;
} AcpiInfo;

#pragma pack(pop)

//------------------------------------------------------------------------------
//
//  Function:  AcpiInfo
//
//  Returns information on ACPI. returns TRUE if ACPI was found, and was in a 
//  physical address that could be accessed and "Info" was valid (with cbSize correct)
//
BOOL AcpiGetInfo(__out AcpiInfo* Info);

//------------------------------------------------------------------------------
//
//  Function:  AcpiFindTablesStartAddress
//
//  Returns the start 32bit physical address of the ACPI tables. 
//  It doesn't try to access this address.
//
DWORD AcpiFindTablesStartAddress(void);

BOOL AcpiFindATable(
                    const ACPI_TABLE TableToFind, 
                    __out volatile void** pvTableData, 
                    __out volatile AcpiTable** pHeader
                    );


#ifdef __cplusplus
}
#endif
