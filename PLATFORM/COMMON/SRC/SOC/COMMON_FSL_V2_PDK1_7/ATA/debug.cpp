//------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
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
//  File:  debug.cpp
//
//  Define the Debug Functions
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <debug.h>

#ifdef DEBUG

void DumpRegKey(DWORD dwZone, PTSTR szKey, HKEY hKey)
{
    DWORD dwIndex = 0;
    WCHAR szValueName[MAX_PATH];
    DWORD dwValueNameSize = MAX_PATH;
    BYTE pValueData[256];
    DWORD dwType;
    DWORD dwValueDataSize = sizeof(pValueData);

    DEBUGMSG(dwZone, (TEXT("Ata!DumpRegKey> %s \r\n"), szKey));
    while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex,
        szValueName,
        &dwValueNameSize,
        NULL, &dwType,
        pValueData,
                                         &dwValueDataSize))
        {
        if (REG_SZ == dwType) {
            DEBUGMSG(dwZone, (TEXT("\t\t%s = %s\r\n"), szValueName, (LPWSTR)pValueData));
        }
        else if (REG_DWORD == dwType) {
            DEBUGMSG(dwZone, (TEXT("\t\t%s = %08X\r\n"), szValueName, *(PDWORD)pValueData));
        }
        else if (REG_MULTI_SZ == dwType) {
            PWSTR pValueTemp = (PWSTR)pValueData;
            DEBUGMSG(dwZone, (TEXT("\t\t%s :\r\n"), szValueName));
            while (*pValueTemp) {
                DEBUGMSG(dwZone, (TEXT("\t\t\t%s\r\n"), (LPWSTR)pValueTemp));
                pValueTemp += (wcslen(pValueTemp) + 1);
            }
        }
        dwIndex++;
        dwValueDataSize = sizeof(pValueData);
        dwValueNameSize = MAX_PATH;
    }
}

VOID DumpIdentify(PIDENTIFY_DATA pId)
{
    DWORD i;
    PUCHAR pCh;
    TCHAR OutBuf[128];
    LPTSTR pOut;
    PUSHORT tempS;
    UCHAR tempByte;
    ULONG j,k;

    DEBUGMSG(ZONE_INIT, (TEXT("Ata!DumpIdentify> ...\r\n")));
    i = sizeof(IDENTIFY_DATA);
    pCh = (PUCHAR)pId;
    while (i) {
        pOut = OutBuf;
        k = (i < 16) ? i : 16;
        for (j = 0; j < k; j++) {
            pOut += wsprintf(pOut, TEXT("%2x "), pCh[j]);
        }
        if (k < 16) {
            for (j = 0; j < 16 - k; j++) {
                pOut += wsprintf(pOut, TEXT("   "));
            }
        }
        for (j = 0; j < k; j++) {
            if ((pCh[j] < ' ') || (pCh[j] > '~')) {
                pOut += wsprintf(pOut, TEXT("."));
            } else {
                pOut += wsprintf(pOut, TEXT("%c"), pCh[j]);
            }
        }
        DEBUGMSG(ZONE_INIT,(TEXT("%s\r\n"), OutBuf));
        i -= k;
        pCh += k;
    }
    if (pId->GeneralConfiguration & 0x8000) {
        DEBUGMSG(ZONE_INIT, (TEXT("    non-magnetic media\r\n")));
    }
    if (pId->GeneralConfiguration & 0x4000) {
        DEBUGMSG(ZONE_INIT, (TEXT("    format speed tolerance gap required\r\n")));
    }
    if (pId->GeneralConfiguration & 0x2000) {
        DEBUGMSG(ZONE_INIT, (TEXT("    track offset option available\r\n")));
    }
    if (pId->GeneralConfiguration & 0x1000) {
        DEBUGMSG(ZONE_INIT, (TEXT("    data strobe offset option available\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0800) {
        DEBUGMSG(ZONE_INIT, (TEXT("    rotational speed tolerance is > 0,5%\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0400) {
        DEBUGMSG(ZONE_INIT, (TEXT("    disk transfer rate > 10Mbs\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0200) {
        DEBUGMSG(ZONE_INIT, (TEXT("    disk transfer rate > 5Mbs but <= 10Mbs\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0100) {
        DEBUGMSG(ZONE_INIT, (TEXT("    disk transfer rate <= 5Mbs\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0080) {
        DEBUGMSG(ZONE_INIT, (TEXT("    removeable cartridge drive\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0040) {
        DEBUGMSG(ZONE_INIT, (TEXT("    fixed drive\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0020) {
        DEBUGMSG(ZONE_INIT, (TEXT("    spindle motor control option implemented\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0010) {
        DEBUGMSG(ZONE_INIT, (TEXT("    head switch time > 15us\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0008) {
        DEBUGMSG(ZONE_INIT, (TEXT("    not MFM encoded\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0004) {
        DEBUGMSG(ZONE_INIT, (TEXT("    soft sectored\r\n")));
    }
    if (pId->GeneralConfiguration & 0x0002) {
        DEBUGMSG(ZONE_INIT, (TEXT("    hard sectored\r\n")));
    }
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    number of cylinders                        : %d\r\n"), pId->NumberOfCylinders));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    number of heads                            : %d\r\n"),pId->NumberOfHeads));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    unformatted bytes per track                : %d\r\n"),pId->UnformattedBytesPerTrack));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    unformatted bytes per sector               : %d\r\n"),pId->UnformattedBytesPerSector));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    sectors per track                          : %d\r\n"),pId->SectorsPerTrack));
    tempS = pId->ModelNumber;
    for (k=0; k<20; k++) {
        tempByte = (UCHAR)(tempS[k] & 0x00FF);
        tempS[k] = tempS[k] >> 8;
        tempS[k] |= tempByte << 8;
    }
    tempS = pId->FirmwareRevision;
    for (k=0; k<4; k++) {
        tempByte = (UCHAR)(tempS[k] & 0x00FF);
        tempS[k] = tempS[k] >> 8;
        tempS[k] |= tempByte << 8;
    }
    tempS = pId->SerialNumber;
    for (k=0; k<10; k++) {
        tempByte = (UCHAR)(tempS[k] & 0x00FF);
        tempS[k] = tempS[k] >> 8;
        tempS[k] |= tempByte << 8;
    }
    if (pId->BufferType == 0) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    buffer type                                : unspecified\r\n")));
    }
    if (pId->BufferType == 1) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    buffer type                                : single port; no simultaneous transfers\r\n")));
    }
    if (pId->BufferType == 2) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    buffer type                                : dual port; simultaneous transfers supported\r\n")));
    }
    if (pId->BufferType == 3) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    buffer type                                : dual port; simultaneous transfer supported, read cache supported\r\n")));
    }
    if (pId->BufferType >= 4) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    buffer type                                : reserved\r\n")));
    }
    if (pId->BufferSectorSize == 0) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    buffer size                                : unspecified\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    buffer size                                : %d sectors\r\n"),pId->BufferSectorSize));
    }
    if (pId->MaximumBlockTransfer == 0) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    read/write multiple                        : not supported\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    read/write multiple                        : supported, %d sectors/interrupt\r\n"),
            pId->MaximumBlockTransfer));
    }
    if (pId->DoubleWordIo == 0) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    double word IO                             : not supported\r\n")));
    }
    else if (pId->DoubleWordIo == 1) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    double word IO                             : supported\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    double word IO                             : unrecognized mode\r\n")));
    }
    if (pId->Capabilities & 0x0200) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    LBA mode                                   : supported\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    LBA mode                                   : not supported\r\n")));
    }
    if (pId->Capabilities & 0x0100) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    DMA                                        : supported\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    DMA                                        : not supported\r\n")));
    }
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    PIO cycle timing mode                      : %x\r\n"), pId->PioCycleTimingMode));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    DMA cycle timing mode                      : %x\r\n"), pId->DmaCycleTimingMode));
    if ((pId->TranslationFieldsValid & 1) == 0) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    current size fields valid                  : unknown\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    current size fields valid                  : yes\r\n")));
    }
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    current number of cylinders                : %d\r\n"), pId->NumberOfCurrentCylinders));

    DEBUGMSG(ZONE_INIT, (TEXT(
            "    current number of heads                    : %d\r\n"), pId->NumberOfCurrentHeads));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    current number of sectors/track            : %d\r\n"), pId->CurrentSectorsPerTrack));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    current sector capacity                    : %d\r\n"), pId->CurrentSectorCapacity));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    sectors/interrupt with read/write multiple : %d\r\n"), pId->MultiSectorCount));
    if (pId->MultiSectorSettingValid & 1) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    multi sector setting                       : valid\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "    multi sector setting                       : invalid\r\n")));
    }
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    total user addressable sectors             : %d\r\n"), pId->TotalUserAddressableSectors));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    single word DMA modes supported            : %x\r\n"), pId->SingleDmaModesSupported));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    single word transfer mode active           : %x\r\n"), pId->SingleDmaTransferActive));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    multi word DMA modes supported             : %x\r\n"), pId->MultiDmaModesSupported));
    DEBUGMSG(ZONE_INIT, (TEXT(
            "    multi word transfer mode active            : %x\r\n"), pId->MultiDmaTransferActive));

}

#define TM_PIO           0x08
#define TM_MULTIWORD_DMA 0x20
#define TM_ULTRA_DMA     0x40

void
DumpSupportedTransferModes(
    PIDENTIFY_DATA pId
    )
{
    BOOL fDMASupported = TRUE; // whether UDMA or Multiword DMA is supported

    DEBUGMSG(ZONE_INIT, (_T(
        "Ata!DumpSupportedTransferModes> Supported transfer mode summary:\r\n"
        )));

    // determine whether IORDY is supported
    if (pId->Capabilities & IDENTIFY_CAPABILITIES_IOREADY_SUPPORTED) {
        DEBUGMSG(ZONE_INIT, (_T("    IORDY supported\r\n")));
    }

    // TranslationFieldsValid <=> Word 53
    if (pId->TranslationFieldsValid & 0x0002) {

        // Word 65
        DEBUGMSG(ZONE_INIT, (_T(
            "    minimum multiword DMA transfer cycle time per word (ns)            = %d\r\n"
            ), pId->MinimumMultiwordDMATime));
        // Word 66
        DEBUGMSG (ZONE_INIT, (_T(
            "    manufacturer's recommended multi word DMA transfer cycle time (ns) = %d\r\n"
            ), pId->ManuRecomendedDMATime));
        // Word 67
        DEBUGMSG(ZONE_INIT, (_T(
            "    minimum PIO transfer cycle time w/o flow control (ns)              = %d\r\n"
            ), pId->MinimumPIOxferTimeWOFlow));
        // Word 68
        DEBUGMSG(ZONE_INIT, (_T(
            "    minimum PIO transfer cycle time w/ IORDY flow control (ns)         = %d\r\n"
            ), pId->MinimumPIOxferTimeIORDYFlow));

        // Word 64; determine "best" supported PIO mode
        if (pId->AdvancedPIOxferreserved & 0x01) {
            DEBUGMSG(ZONE_INIT, (_T("    PIO mode 0 supported\r\n")));
        }
        if (pId->AdvancedPIOxferreserved & 0x02) {
            DEBUGMSG(ZONE_INIT, (_T("    PIO mode 1 supported\r\n")));
        }
        if (pId->AdvancedPIOxferreserved & 0x04) {
            DEBUGMSG(ZONE_INIT, (_T("    PIO mode 2 supported\r\n")));
        }
        if (pId->AdvancedPIOxferreserved & 0x08) {
            DEBUGMSG(ZONE_INIT, (_T("    PIO mode 3 supported\r\n")));
        }
        if (pId->AdvancedPIOxferreserved & 0x10) {
            DEBUGMSG(ZONE_INIT, (_T("    PIO mode 4 supported\r\n")));
        }
        if (!(pId->AdvancedPIOxferreserved & 0x1F)) {
            DEBUGMSG(ZONE_INIT, (_T("    PIO mode not supported\r\n")));
        }
    }
    else {
        DEBUGMSG(ZONE_INIT, (_T("    PIO mode not supported\r\n")));
    }

    // MultiDMAModesSupported <=> Word 63, low-byte
    // determine "best" supported Multiword DMA mode
    if (pId->MultiDmaModesSupported & 0x04) {
        DEBUGMSG(ZONE_INIT, (_T("    multi word DMA mode 2 and below is supported\r\n")));
    }
    else if (pId->MultiDmaModesSupported & 0x02) {
        DEBUGMSG(ZONE_INIT, (_T("    multi word DMA mode 1 and below is supported\r\n")));
    }
    else if (pId->MultiDmaModesSupported & 0x01) {
        DEBUGMSG(ZONE_INIT, (_T("    multi word DMA mode 0 is supported\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (_T("    multi word DMA mode not supported\r\n")));
        fDMASupported = FALSE;
    }
    if (fDMASupported) {
        // MultiDmaTransferActive <=> Word 64, high-byte; dump selected Multiword DMA modes
        if (pId->MultiDmaTransferActive & 0x04) {
            DEBUGMSG(ZONE_INIT, (_T("    >>> multi word DMA mode 2 selected by default <<<\r\n")));
        }
        if (pId->MultiDmaTransferActive & 0x02) {
            DEBUGMSG(ZONE_INIT, (_T("    >>> multi word DMA mode 1 selected by default <<<\r\n")));
        }
        if (pId->MultiDmaTransferActive & 0x01) {
            DEBUGMSG(ZONE_INIT, (_T("    >>> multi word DMA mode 0 selected by default <<<\r\n")));
        }
    }

    // TranslationFieldsValid <=> Word 53; UltraDMASupport <=> Word 88, low-byte
    // dump supported Ultra DMA modes
    if (pId->TranslationFieldsValid & 0x0004) {

        if (pId->UltraDMASupport & 0x20) {
            DEBUGMSG(ZONE_INIT, (_T("    ultra DMA mode 5 and below is supported\r\n")));
        }
        else if (pId->UltraDMASupport & 0x10) {
            DEBUGMSG(ZONE_INIT, (_T("    ultra DMA mode 4 and below is supported\r\n")));
        }
        else if (pId->UltraDMASupport & 0x08) {
            DEBUGMSG(ZONE_INIT, (_T("    ultra DMA mode 3 and below is supported\r\n")));
        }
        else if (pId->UltraDMASupport & 0x04) {
            DEBUGMSG(ZONE_INIT, (_T("    ultra DMA mode 2 and below is supported\r\n")));
        }
        else if (pId->UltraDMASupport & 0x02) {
            DEBUGMSG(ZONE_INIT, (_T("    ultra DMA mode 1 and below is supported\r\n")));
        }
        else if (pId->UltraDMASupport & 0x01) {
            DEBUGMSG(ZONE_INIT, (_T("    ultra DMA mode 0 is supported\r\n")));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T("    ultra DMA mode not supported\r\n")));
            fDMASupported = FALSE;
        }
        // UltraDMAActive <=> Word 88, high-byte; dump selected Ultra DMA modes
        if (fDMASupported) {
            if (pId->UltraDMAActive & 0x20) {
                DEBUGMSG(ZONE_INIT, (_T("    >>> ultra DMA mode 5 selected by default <<<\r\n")));
            }
            if (pId->UltraDMAActive & 0x10) {
                DEBUGMSG(ZONE_INIT, (_T("    >>> ultra DMA mode 4 selected by default <<<\r\n")));
            }
            if (pId->UltraDMAActive & 0x08) {
                DEBUGMSG(ZONE_INIT, (_T("    >>> ultra DMA mode 3 selected by default <<<\r\n")));
            }
            if (pId->UltraDMAActive & 0x04) {
                DEBUGMSG(ZONE_INIT, (_T("    >>> ultra DMA mode 2 selected by default <<<\r\n")));
            }
            if (pId->UltraDMAActive & 0x02) {
                DEBUGMSG(ZONE_INIT, (_T("    >>> ultra DMA mode 1 selected by default <<<\r\n")));
            }
            if (pId->UltraDMAActive & 0x01) {
                DEBUGMSG(ZONE_INIT, (_T("    >>> ultra DMA mode 0 selected by default <<<\r\n")));
            }
        }
    }
    else {
        DEBUGMSG(ZONE_INIT, (_T("    ultra DMA mode not supported\r\n")));
    }

    // CommandSetSupported1 <=> Word 82 <=> whether look-ahead + write cache are supported
    if (pId->CommandSetSupported1 & (1 << 6)) {
        DEBUGMSG(ZONE_INIT, (_T("    look-ahead supported\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (_T("    look-ahead not supported\r\n")));
    }
    if (pId->CommandSetSupported1 & (1 << 5)) {
        DEBUGMSG(ZONE_INIT, (_T("    write cache supported\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (_T("    write cache not supported\r\n")));
    }
    if (pId->CommandSetSupported1 & 0x01) {
        DEBUGMSG(ZONE_INIT, (_T("    S.M.A.R.T. feature set supported\r\n")));
    }
    else {
        DEBUGMSG(ZONE_INIT, (_T("    S.M.A.R.T. feature set not supported\r\n")));
    }
}

#endif
