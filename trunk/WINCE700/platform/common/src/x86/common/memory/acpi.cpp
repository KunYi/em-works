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
#include <oal.h>
#include <acpi.h>

typedef struct  {
    char      sSignature[8];
    BYTE      bChecksum;
    BYTE      sOemId[6];
    BYTE      bRevision;
    DWORD     dwRsdtAddress;

    DWORD     dwLength;         // v2
    ULARGE_INTEGER XsdtAddress; // v2
    BYTE      bExtendedCheckSum;// v2
    BYTE      rgbReserved[3];   // v2
} AcpiTableRsdp;

// if your platform has invalid checksums on the tables set this to FALSE
static BOOL s_fAcpiCalcCheckSum = TRUE;

static AcpiInfo s_AcpiInfo = { sizeof (AcpiInfo), FALSE, 0, (DWORD)-1, 0 };

//------------------------------------------------------------------------------
//
//  Function:  AcpiScanRsdp
//
//  Scans Physical Memory from 'start' for 'length' to find the Root System Description Pointer
//
static const AcpiTableRsdp* AcpiScanRsdp (DWORD physStart, DWORD cbLength)
{
    const char szSig[] = "RSD PTR ";
    const char* psVirtStart = reinterpret_cast<const char *>(OALPAtoCA(physStart));

    // interate though the memory range looking for the ACPI sig on 16byte boundries
    for (DWORD dwOffset = 0; dwOffset < cbLength; dwOffset += 16) 
    {
        if (0 == strncmp(psVirtStart + dwOffset, szSig, _countof(szSig) - 1)) {
            return reinterpret_cast<const AcpiTableRsdp*>(psVirtStart + dwOffset);
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------
//
//  Function:  AcpiTableIdToString
//
//  Converts a four 'char' identifier to a Unicode null terminated string
//  NOTE: not multithreading safe
//
static TCHAR* AcpiTableIdToString(ACPI_TABLE Id)
{
    static TCHAR wszBuffer[5] = {0};
    wszBuffer[0] = reinterpret_cast<BYTE*>(&Id)[0];
    wszBuffer[1] = reinterpret_cast<BYTE*>(&Id)[1];
    wszBuffer[2] = reinterpret_cast<BYTE*>(&Id)[2];
    wszBuffer[3] = reinterpret_cast<BYTE*>(&Id)[3];
    return wszBuffer;
}

#define MAX_ACPI_TABLE_LENGTH   0x20000
//------------------------------------------------------------------------------
//
//  Function:  AcpiCheckSum
//
//  Validates the checksum of a block of data
//
static BOOL AcpiCheckSum (const BYTE* pbData, DWORD cbLength)
{
    BYTE bSum = 0;

    // Tables shouldn't be longer than <large number=5MB>
    if (cbLength > 0x20000)
    {
        OALMSG(OAL_ERROR, (TEXT(" ACPI table looks too large Start:0x%x Length:%d"), pbData, cbLength));
        return FALSE;
    }

    if (s_fAcpiCalcCheckSum)
    {
        for (DWORD i = 0; i < cbLength; ++i)
            bSum += *pbData++;
    }

    OALMSG(bSum != 0 && OAL_ERROR, (TEXT(" Invalid checksum found on ACPI Table: %s"), 
        AcpiTableIdToString(static_cast<ACPI_TABLE>(*reinterpret_cast<const DWORD*>(pbData)))));

    return bSum == 0;
}

//------------------------------------------------------------------------------
//
//  Function:  AcpiTableRsdp
//
//  Finds the Root System Description Pointer
//
static const AcpiTableRsdp* AcpiFindRsdp()
{
    // check first 1k
    const AcpiTableRsdp* pRsdp = AcpiScanRsdp(0, 0x400);

    if (!pRsdp)
        // check the next memory range, as defined in the ACPI spec
        pRsdp = AcpiScanRsdp(0xE0000, 0xfffff-0xe0000);

    if (pRsdp)
    {
        // we found the RSDP, make sure it's valid
        DWORD dwLength = pRsdp->dwLength;
        if (pRsdp->bRevision == 0)
            // Rev 0 had a 20 byte struct, with no Length member
            dwLength = 20;

        OALMSG(OAL_MEMORY, (TEXT(" RSDP Start:0x%x ACPI Rev:%d Length:%d"), pRsdp, pRsdp->bRevision, dwLength));

        if (AcpiCheckSum(reinterpret_cast<const BYTE*>(pRsdp), dwLength))
            return pRsdp;
    }

    return NULL;
}

// defined in x86 OEMAddressTable
extern "C" DWORD dwOEMTotalRAM;

//------------------------------------------------------------------------------
//
//  Function:  AcpiFindRsdt
//
//  Finds and verifies the Root System Description Table
//
static volatile AcpiTable* AcpiFindRsdt(__in const volatile AcpiTableRsdp* const pAcpiTableRsdp)
{
    if (!pAcpiTableRsdp)
        return NULL;

    if (pAcpiTableRsdp->dwRsdtAddress == NULL)
    {
        OALMSG(OAL_ERROR , (TEXT(" RSDT Not found")));
        return NULL;
    }

    // make sure the address is mapped
    if (pAcpiTableRsdp->dwRsdtAddress > dwOEMTotalRAM)
    {
        OALMSG(OAL_ERROR , (TEXT(" Found ACPI but it was located above %dMB at %dMB"), 
            dwOEMTotalRAM /1024/1024, pAcpiTableRsdp->dwRsdtAddress /1024/1024));
        return NULL;
    }
    
    volatile AcpiTable* pRsdt = reinterpret_cast<volatile AcpiTable*>(OALPAtoCA(pAcpiTableRsdp->dwRsdtAddress));

    OALMSG(OAL_MEMORY, (TEXT(" RSDT Start:0x%x Length:%d"), pRsdt, pRsdt->dwLength));

    if (pRsdt->Signature != ACPI_TABLE_RSDT)
    {
        OALMSG(OAL_ERROR , (TEXT(
            " Unexpected Root System Description Table: '%s' (Note: 'XSDT' is not supported)"
            ), AcpiTableIdToString(pRsdt->Signature)));
        return NULL;
    }
    
    if (AcpiCheckSum(
        const_cast<BYTE*>(reinterpret_cast<volatile BYTE*>(pRsdt)), 
        pRsdt->dwLength))
    {
        s_AcpiInfo.fAcpiFound = TRUE;
        s_AcpiInfo.dwAcpiVersion = pAcpiTableRsdp->bRevision;
        return pRsdt;
    }

    return NULL; 
}

//------------------------------------------------------------------------------
//
//  Function:  AcpiPhysicalAddressToValidTable
//
//  Takes a pointer to physical memory, validates its a ACPI table, and returns to pointer
//  in Virtual Memory
//
static volatile AcpiTable* AcpiPhysicalAddressToValidTable(__in void* pvTable)
{
    volatile AcpiTable* pRsdt = reinterpret_cast<volatile AcpiTable*>(
        OALPAtoCA(reinterpret_cast<DWORD>(pvTable)) );

    OALMSG(OAL_MEMORY, (TEXT(" %s Start:0x%x Length:%d"), AcpiTableIdToString(pRsdt->Signature), pRsdt, pRsdt->dwLength));

    if (pRsdt && AcpiCheckSum(
        const_cast<BYTE*>(reinterpret_cast<volatile BYTE*>(pRsdt)), 
        pRsdt->dwLength))
        return pRsdt;

    return NULL;
}

static volatile AcpiTable* AcpiFindTable(__in volatile AcpiTable* pAcpiTable, const ACPI_TABLE TableToFind);

//------------------------------------------------------------------------------
//
//  Function:  AcpiParseFacp
//
//  Parse the Fixed ACPI Description Table looking for 'TableToFind' in the DSDT and FACS
//
static volatile AcpiTable* AcpiParseFacp(
    __in_ecount(dwEntryCount) volatile DWORD * pdwEntries, 
    DWORD /*dwEntryCount*/, 
    const ACPI_TABLE TableToFind
    )
{
    const volatile AcpiTable* pRetVal = NULL;

    {   
        // 'FACS': Firmware ACPI Control Structure 
        // NOTE: This table doesn't have the standard header or checksum
        volatile AcpiTable* pRsdt = reinterpret_cast<volatile AcpiTable*>(
            OALPAtoCA(static_cast<DWORD>(*pdwEntries)) );

        OALMSG(OAL_MEMORY, (TEXT(" %s Start:0x%x Length:%d"), AcpiTableIdToString(pRsdt->Signature), pRsdt, pRsdt->dwLength));

        if (pRsdt)
            pRetVal = AcpiFindTable(pRsdt, TableToFind);

        ++pdwEntries;
    }

    if (!pRetVal) 
    {   
        // 'DSDT': Differentiated System Description Table
        volatile AcpiTable* pRsdt = AcpiPhysicalAddressToValidTable(reinterpret_cast<void*>(*pdwEntries));
        if (pRsdt)
            pRetVal = AcpiFindTable(pRsdt, TableToFind);
    }

    return NULL;
}

//------------------------------------------------------------------------------
//
//  Function:  AcpiParseRsdt
//
//  Parse the Root System Description Table looking for 'TableToFind' in all the subtables
//
static volatile AcpiTable* AcpiParseRsdt(
    __in_ecount(dwEntryCount) volatile DWORD * pdwEntries, 
    DWORD dwEntryCount, 
    const ACPI_TABLE TableToFind
    )
{
    volatile AcpiTable* pRetVal = NULL;

    for (DWORD i = 0; i < dwEntryCount && pRetVal == NULL; ++i)
    {
        volatile AcpiTable* pRsdt = AcpiPhysicalAddressToValidTable(reinterpret_cast<void*>(*pdwEntries));
        if (pRsdt)
            pRetVal = AcpiFindTable(pRsdt, TableToFind);
        ++pdwEntries;
    }

    return pRetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  AcpiFindTable
//
//  Parse the given pAcpiTable looking for 'TableToFind' in all the subtables
//
static volatile AcpiTable* AcpiFindTable(__in volatile AcpiTable* pAcpiTable, const ACPI_TABLE TableToFind)
{   
    if (pAcpiTable == NULL)
        return NULL;

    volatile DWORD * const pdwTableData = (volatile DWORD*)(pAcpiTable+1); 
    DWORD dwEntryCount = (pAcpiTable->dwLength - sizeof(AcpiTable)) / sizeof DWORD;
    volatile AcpiTable* RetVal = NULL;

    // if this tables ends after the current TablesEnd, then update with this tables end
    if (reinterpret_cast<DWORD>(OALUAtoCA(pAcpiTable)) + pAcpiTable->dwLength > s_AcpiInfo.dwAcpiTablesEnd)
        s_AcpiInfo.dwAcpiTablesEnd = reinterpret_cast<DWORD>(OALUAtoCA(pAcpiTable)) + pAcpiTable->dwLength;

    // if this tables start before the current TablesStart, then update with this tables start
    if (reinterpret_cast<DWORD>(OALUAtoCA(pAcpiTable)) < s_AcpiInfo.dwAcpiTablesStart)
        s_AcpiInfo.dwAcpiTablesStart = reinterpret_cast<DWORD>(OALUAtoCA(pAcpiTable));

    if (pAcpiTable->Signature == TableToFind)
        return pAcpiTable;

    // some special cases, which we want to recurse into
    switch (pAcpiTable->Signature)
    {
    case ACPI_TABLE_RSDT:
        // Root System Description Table 
        RetVal = AcpiParseRsdt(pdwTableData, dwEntryCount, TableToFind);
        break;

    case ACPI_TABLE_FACP:
        // Fixed ACPI Description Table (FADT) 
        RetVal = AcpiParseFacp(pdwTableData, dwEntryCount, TableToFind);
        break;

    default:
        break;
    }

    return RetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  AcpiEnumTables
//
//  Enumerates all the ACPI tables, which fills in the s_AcpiInfo struct used by AcpiInfo()
//
static void AcpiEnumTables()
{
    static BOOL fACPIEnumerated = FALSE;
    if (!fACPIEnumerated)
    {
        AcpiFindTable(AcpiFindRsdt(AcpiFindRsdp()), ACPI_TABLE_NULL);
        fACPIEnumerated = TRUE;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  AcpiFindATable
//
//  Finds a specific ACPI Table
//
BOOL AcpiFindATable(
                    const ACPI_TABLE TableToFind, 
                    __out volatile void** pvTableData, 
                    __out volatile AcpiTable** pHeader
                    )
{
    volatile AcpiTable* const pAcpiHeader = AcpiFindTable(AcpiFindRsdt(AcpiFindRsdp()), TableToFind);

    if (pHeader)
        *pHeader = pAcpiHeader;

    if (pvTableData)
        *pvTableData = reinterpret_cast<void*>(const_cast<AcpiTable*>(pAcpiHeader+1));

    return pAcpiHeader != NULL;
}

//------------------------------------------------------------------------------
//
//  Function:  AcpiInfo
//
//  Returns information on ACPI. returns TRUE if ACPI was found, and was in a 
//  physical address that could be accessed and Info was valid (with cbSize correct)
//
BOOL AcpiGetInfo(__out AcpiInfo* pInfo)
{
    if (!pInfo || pInfo->cbSize != sizeof AcpiInfo)
        return FALSE;

    // for s_AcpiInfo to have valid values we must enumerate all the tables
    AcpiEnumTables();

    memcpy(pInfo, &s_AcpiInfo, sizeof(s_AcpiInfo) );

    return s_AcpiInfo.fAcpiFound;
}


//------------------------------------------------------------------------------
//
//  Function:  AcpiFindTablesStartAddress
//
//  Returns the start 32bit physical address of the ACPI tables. 
//  It doesn't try to access this address.
//
DWORD AcpiFindTablesStartAddress()
{
    const AcpiTableRsdp* pRsdp = AcpiFindRsdp();
    return pRsdp? pRsdp->dwRsdtAddress : 0;
}

