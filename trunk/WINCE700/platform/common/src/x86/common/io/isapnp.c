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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <pc.h>
#include <oal.h>
#include <specstrings.h>

//
// Plug and Play Card Control Registers
//
enum 
{
    PNP_PORT_SET_READ_DATA         = 0x00,
    PNP_PORT_SERIAL_ISOLATION      = 0x01,
    PNP_PORT_CONFIG_CONTROL        = 0x02,
    PNP_PORT_WAKE_CSN              = 0x03,
    PNP_PORT_SET_CSN               = 0x06
};

//
// Config Control command
//
enum
{
    CONTROL_WAIT_FOR_KEY       = 0x02,
    CONTROL_RESET_CSN          = 0x04
};

enum { NUMBER_CARD_ID_BYTES  = 9 };
enum { NUMBER_CARD_ID_BITS   = (NUMBER_CARD_ID_BYTES * 8) };
static const BYTE CHECKSUMED_BITS       = 64;
static const BYTE LFSR_SEED             = 0x6A;
static const BYTE ISOLATION_TEST_BYTE_1 = 0x55;
static const BYTE ISOLATION_TEST_BYTE_2 = 0xAA;
static const DWORD PNP_ADDRESS_PORT     = 0x279;
static const DWORD PNP_WRITE_DATA_PORT  = 0xA79;
static const DWORD PNP_READ_DATA_PORT   = 0x203;
static BYTE* const PNP_ISA_IO_BASE      = NULL;

__inline void pnpWriteAddress(const BYTE data) { WRITE_PORT_UCHAR(PNP_ISA_IO_BASE + PNP_ADDRESS_PORT,    data); }
__inline void pnpWriteData(const BYTE data)    { WRITE_PORT_UCHAR(PNP_ISA_IO_BASE + PNP_WRITE_DATA_PORT, data); }
__inline BYTE pnpReadData()                    { return READ_PORT_UCHAR (PNP_ISA_IO_BASE + PNP_READ_DATA_PORT); }

static CRITICAL_SECTION    g_csISAConfig;
static BOOL                g_bISAInitialized;
static DWORD               g_usISAReadPort;
static BYTE                g_ucISANumberCSNs;


//------------------------------------------------------------------------------
//
//  Function:  PnPReadRegister
//
//  Reads a BYTE from a given register
//
static BYTE PnPReadRegister(
                            BYTE* const pIOSpace, 
                            BYTE ucRegNo
                            )
{
    WRITE_PORT_UCHAR((BYTE*)pIOSpace + PNP_ADDRESS_PORT, ucRegNo);
    return READ_PORT_UCHAR((BYTE*)pIOSpace + g_usISAReadPort);
}    



//------------------------------------------------------------------------------
//
//  Function:  PnPWriteRegister
//
//  writes a BYTE to a given register
//
static void PnPWriteRegister(
                             BYTE* const pIOSpace, 
                             BYTE ucRegNo, 
                             BYTE ucValue
                            )
{
    WRITE_PORT_UCHAR((BYTE*)pIOSpace + PNP_ADDRESS_PORT, ucRegNo);
    WRITE_PORT_UCHAR((BYTE*)pIOSpace + PNP_WRITE_DATA_PORT, ucValue);
}



//------------------------------------------------------------------------------
//
//  Function:  PnPSendInitiationKey
//
//  Sends the Initiation Key
//
static void PnPSendInitiationKey(
                                 BYTE* const pIOSpace
                                 )
{
    int i;
    static  BYTE   ucInitKey[] =
    {
        0x00, 0x00,
        0x6A, 0xB5, 0xDA, 0xED, 0xF6, 0xFB, 0x7D, 0xBE,
        0xDF, 0x6F, 0x37, 0x1B, 0x0D, 0x86, 0xC3, 0x61,
        0xB0, 0x58, 0x2C, 0x16, 0x8B, 0x45, 0xA2, 0xD1,
        0xE8, 0x74, 0x3A, 0x9D, 0xCE, 0xE7, 0x73, 0x39
    };

    for ( i = 0; i < _countof(ucInitKey); i++ ) 
    {
        WRITE_PORT_UCHAR((BYTE*) (pIOSpace + PNP_ADDRESS_PORT), ucInitKey[i]);
    }
}



//------------------------------------------------------------------------------
//
//  Function:  PnPSetWaitForKey
//
//  Tells the bus to wait for the key
//
static void PnPSetWaitForKey(
                             BYTE* const pIOSpace
                             )
{
    PnPWriteRegister(pIOSpace, PNP_PORT_CONFIG_CONTROL, CONTROL_WAIT_FOR_KEY);
}


//------------------------------------------------------------------------------
//
//  Function:  PnPWake
//
//  Wakes up 
//
static void PnPWake(
                    BYTE* const pIOSpace, 
                    BYTE ucCSN
                    )
{
    PnPWriteRegister(pIOSpace, PNP_PORT_WAKE_CSN, ucCSN);
}



//------------------------------------------------------------------------------
//
//  Function:  PnPReadSerialId
//
//  Reads the Serial Id
//
static void PnPReadSerialId(
                            BYTE* const pIOSpace, 
                            __out_bcount(NUMBER_CARD_ID_BYTES) BYTE* ucSerialId
                            )
{
    int i;
    for ( i = 0; i < NUMBER_CARD_ID_BYTES; i++ ) 
    {
        WRITE_PORT_UCHAR((BYTE*) (pIOSpace + PNP_ADDRESS_PORT), 0x05);

        while ( !(READ_PORT_UCHAR((BYTE*) (pIOSpace + g_usISAReadPort)) & 0x01) )
            ;

        WRITE_PORT_UCHAR((BYTE*)pIOSpace + PNP_ADDRESS_PORT, 0x04);
        ucSerialId[i] = READ_PORT_UCHAR((BYTE*)pIOSpace + g_usISAReadPort);
    }
}



//------------------------------------------------------------------------------
//
//  Function:  PnPReadResourceData
//
//  Reads the Resource Data about the bus
//
static int PnPReadResourceData(
                               BYTE* const pIOSpace, 
                               __out_bcount(iDataSize) PBYTE ucResourceData, 
                               int iDataSize
                               )
{
    int     i;
    int     bSawEndTag = FALSE;
    BYTE   ucChecksum = 0;

    for ( i = 0; i < iDataSize; i++ ) 
    {
        WRITE_PORT_UCHAR((BYTE*) (pIOSpace + PNP_ADDRESS_PORT), 0x05);

        while ( !(READ_PORT_UCHAR((BYTE*) (pIOSpace + g_usISAReadPort)) & 0x01) )
            ;

        WRITE_PORT_UCHAR((BYTE*) (pIOSpace + PNP_ADDRESS_PORT), 0x04);
        ucResourceData[i] = READ_PORT_UCHAR((BYTE*) (pIOSpace + g_usISAReadPort));

        ucChecksum += ucResourceData[i];

        if ( bSawEndTag ) 
        {
            if ( ucResourceData[i] != 0 && ucChecksum != 0 ) 
            {
                OALMSG(OAL_ERROR, (TEXT("PnPReadResourceData : Bad resource checksum\n")));
            }

            i++;
            break;
        }

        if ( ucResourceData[i] == 0x79 ) 
        {
            bSawEndTag = 1;
        }
    }

    return i;
}



//------------------------------------------------------------------------------
//
//  Function:  PnPGetLogicalDeviceInfo
//
//  Reads the Information about the devices on the bus
//
static int PnPGetLogicalDeviceInfo(
                                   __out_bcount(iDataSize) const BYTE* const ucResourceData, 
                                   int iDataSize, 
                                   PISA_PNP_LOGICAL_DEVICE_INFO pDeviceInfo
                                   )
{
    const BYTE* pCurrent = ucResourceData;
    int     iLength, iName;
    int     i;
    int     nDevices = 0, nCompatibleIDs = 0;

    if (iDataSize < sizeof(ISA_PNP_LOGICAL_DEVICE_INFO) * 8)
        return 0;

    memset(pDeviceInfo, 0, sizeof(ISA_PNP_LOGICAL_DEVICE_INFO) * 8);

    for ( i = 0; i < iDataSize; i++ ) 
    {
        if ( (*pCurrent & 0x80) == 0 ) 
        {
            // Small TAG
            iName = (*pCurrent >> 3) & 0x0F;
            iLength = (*pCurrent & 0x07) + 1;

            switch ( iName ) 
            {
            case 2:     // Logical Device ID
                if ( nDevices <= 8 ) 
                {
                    nDevices++;
                    nCompatibleIDs = 0;
                    pDeviceInfo[nDevices - 1].LogicalDeviceID =
                        pCurrent[1] << 24 | pCurrent[2] << 16 | pCurrent[3] << 8 | pCurrent[4];
                }
                break;

            case 3:     // Compatible Device ID
                if ( nDevices <= 8 ) 
                {
                    nCompatibleIDs++;
                    pDeviceInfo[nDevices - 1].CompatibleIDs[nCompatibleIDs - 1] =
                        pCurrent[1] << 24 | pCurrent[2] << 16 | pCurrent[3] << 8 | pCurrent[4];
                }
                break;
            }
        } 
        else 
        {
            // Large TAG
            iName = *pCurrent & 0x7F;
            iLength = ((pCurrent[2] << 8) | pCurrent[1]) + 3;
        }

        pCurrent += iLength;
        i += iLength - 1;
    }

    return nDevices;
}




//------------------------------------------------------------------------------
//
//  Function:  PnPIsolateCards
//
//  Isolates Cards
//
static void PnPIsolateCards()
{
    USHORT j;
    BYTE  cardId[NUMBER_CARD_ID_BYTES];
    BYTE  bit, bit7, checksum, byte1, byte2;
    BYTE  csn = 0;
    BOOL CardFound = TRUE;

    // First send Initiation Key to all the PNP ISA cards to enable PnP auto-config
    // ports and put all cards in configuration mode.
    PnPSendInitiationKey(NULL);

    // Reset all Pnp ISA cards' CSN to 0 and return to wait-for-key state
    pnpWriteAddress (PNP_PORT_CONFIG_CONTROL);
    pnpWriteData (CONTROL_WAIT_FOR_KEY | CONTROL_RESET_CSN);

    // Delay 2 msec for cards to load initial configuration state.
    NKSleep(2);

    // Put cards into configuration mode to ready isolation process.
    // The hardware on each PnP Isa card expects 72 pairs of I/O read
    // access to the read data port.
    PnPSendInitiationKey(NULL);

    // Starting Pnp Isa card isolation process.

    // Send WAKE[CSN=0] to force all cards without CSN into isolation
    // state to set READ DATA PORT.
    pnpWriteAddress(PNP_PORT_WAKE_CSN);
    pnpWriteData(0);

    // Set read data port to current testing value.
    pnpWriteAddress(PNP_PORT_SET_READ_DATA);
    pnpWriteData((BYTE)(PNP_READ_DATA_PORT >> 2));

    // Isolate one PnP ISA card until fail

    while ( CardFound ) 
    {
        // Read serial isolation port to cause PnP cards in the isolation
        // state to compare one bit of the boards ID.
        pnpWriteAddress(PNP_PORT_SERIAL_ISOLATION);

        // We need to delay 1 msec prior to starting the first pair of isolation
        // reads and must wait 250usec between each subsequent pair of isolation
        // reads.  This delay gives the ISA cards time to access information from
        // possible very slow storage device.
        NKSleep(1);

        memset(cardId, 0, NUMBER_CARD_ID_BYTES);
        checksum = LFSR_SEED;
        for ( j = 0; j < NUMBER_CARD_ID_BITS; j++ ) 
        {
            // Read card id bit by bit
            byte1 = pnpReadData();
            byte2 = pnpReadData();
            bit = (byte1 == ISOLATION_TEST_BYTE_1) && (byte2 == ISOLATION_TEST_BYTE_2);
            cardId[j / 8] |= bit << (j % 8);
            if ( j < CHECKSUMED_BITS ) 
            {
                // Calculate checksum and only do it for the first 64 bits
                bit7 = (((checksum & 2) >> 1) ^ (checksum & 1) ^ (bit)) << 7;
                checksum = (checksum >> 1) | bit7;
            }
            NKSleep(1);
        }

        // Verify the card id we read is legitimate
        // First make sure checksum is valid.  Note zero checksum is considered valid.
        if ( cardId[8] == 0 || checksum == cardId[8] ) 
        {
            // Next make sure cardId is not zero
            byte1 = 0;
            for ( j = 0; j < NUMBER_CARD_ID_BYTES; j++ ) 
            {
                byte1 |= cardId[j];
            }
            if ( byte1 != 0 ) {
                // Make sure the vender EISA ID bytes are nonzero
                if ( (cardId[0] & 0x7f) != 0 && cardId[1] != 0 ) 
                {

                    pnpWriteAddress(PNP_PORT_SET_CSN);

                    pnpWriteData(++csn);

                    // Do Wake[CSN] command to put the newly isolated card to
                    // sleep state and other un-isolated cards to isolation
                    // state.
                    pnpWriteAddress(PNP_PORT_WAKE_CSN);
                    pnpWriteData(0);

                    // ... to isolate more cards ...
                    CardFound = TRUE;
                    continue;
                }
            }
        }
        // could not isolate more cards ...
        CardFound = FALSE;
    }

    // Finaly put all cards into wait for key state.
    pnpWriteAddress(PNP_PORT_CONFIG_CONTROL);
    pnpWriteData(CONTROL_WAIT_FOR_KEY);
    g_ucISANumberCSNs = (BYTE) csn;
}



//------------------------------------------------------------------------------
//
//  Function:  ISAInitBusInfo
//
//  One time bus init
//
static void ISAInitBusInfo()
{
    if ( g_bISAInitialized ) 
    {
        return;
    }

    g_bISAInitialized = TRUE;

    InitializeCriticalSection(&g_csISAConfig);

    PnPIsolateCards();
    OALMSG(OAL_INFO && g_ucISANumberCSNs, (TEXT("PnP ISA InitBusInfo : %d card(s) found\r\n"), g_ucISANumberCSNs));

    g_usISAReadPort = PNP_READ_DATA_PORT;
}



//------------------------------------------------------------------------------
//
//  Function:  OALISACfgRead
//
//  Reads data from the bus
//
ULONG OALISACfgRead(
                     ULONG BusNumber,
                     ULONG SlotNumber,
                     ULONG Offset,
                     ULONG Length,
                     __out_bcount(Length) void* Buffer
                     )
{
    BYTE               ucCSNumber, ucLogicalDevice;
    BYTE               ucSerialID[NUMBER_CARD_ID_BYTES];
    BYTE               ucResourceData[2048];
    int                iResourceLength;
    ULONG              ulReturn;
    UNREFERENCED_PARAMETER(BusNumber);

    if ( g_bISAInitialized == FALSE )
        ISAInitBusInfo();

    if ( g_ucISANumberCSNs == 0 || g_ucISANumberCSNs == ~0U ) 
    {
        return 0;
    }

    ucCSNumber = (BYTE)(SlotNumber >> 8);
    ucLogicalDevice = (BYTE)SlotNumber;

    if (  ucCSNumber > g_ucISANumberCSNs 
       || ucCSNumber == 0 
       || (Offset == 0 && Length != sizeof(ISA_PNP_CONFIG)) 
       || (Offset == 1 && Length != sizeof(ISA_PNP_RESOURCES)) 
       || Offset > 1 
       ) 
    {
        return 0;
    }

    EnterCriticalSection(&g_csISAConfig);

    PnPSendInitiationKey(PNP_ISA_IO_BASE);

    PnPWake(PNP_ISA_IO_BASE, ucCSNumber);

    if ( Offset == 0 ) 
    {
        PISA_PNP_CONFIG     pPnPConfig = (PISA_PNP_CONFIG)Buffer;

        PnPReadSerialId(PNP_ISA_IO_BASE, ucSerialID);

        pPnPConfig->VendorID =
            ucSerialID[0] << 24 | ucSerialID[1] << 16 | ucSerialID[2] << 8 |
            ucSerialID[3];

        pPnPConfig->SerialNumber = 
            ucSerialID[7] << 24 | ucSerialID[6] << 16 | ucSerialID[5] << 8 |
            ucSerialID[4];

        iResourceLength = PnPReadResourceData(PNP_ISA_IO_BASE, ucResourceData, sizeof(ucResourceData));

        pPnPConfig->NumberLogicalDevices = PnPGetLogicalDeviceInfo(ucResourceData, sizeof(ucResourceData),
                                                                   pPnPConfig->LogicalDeviceInfo);

        ulReturn = sizeof(ISA_PNP_CONFIG);
    } 
    else 
    {
        PISA_PNP_RESOURCES  pPnPResources = (PISA_PNP_RESOURCES)Buffer;
        int                 i;
        BYTE               ucActive;

        PnPWriteRegister(PNP_ISA_IO_BASE, 0x07, ucLogicalDevice);

        ucActive = PnPReadRegister(PNP_ISA_IO_BASE, 0x30);

        if ( ucActive ) {
            pPnPResources->Flags = ISA_PNP_RESOURCE_FLAG_ACTIVE;

            for ( i = 0; i < 4; i++ ) 
            {
                pPnPResources->Memory24Descriptors[i].MemoryBase =
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x40 + i * 8)) << 8;
                pPnPResources->Memory24Descriptors[i].MemoryBase |=
                (USHORT)PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x41 + i * 8));
                pPnPResources->Memory24Descriptors[i].MemoryControl =
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x42 + i * 8));
                pPnPResources->Memory24Descriptors[i].MemoryUpperLimit =
                (USHORT)(PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x43 + i * 8)) << 8);
                pPnPResources->Memory24Descriptors[i].MemoryUpperLimit |=
                (USHORT)PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x44 + i * 8));
            }

            for ( i = 0; i < 8; i++ ) 
            {
                pPnPResources->IoPortDescriptors[i] =
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x60 + i * 2)) << 8;
                pPnPResources->IoPortDescriptors[i] |=
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x61 + i * 2));
            }

            pPnPResources->IRQDescriptors[0].IRQLevel = PnPReadRegister(PNP_ISA_IO_BASE, 0x70);
            pPnPResources->IRQDescriptors[0].IRQType = PnPReadRegister(PNP_ISA_IO_BASE, 0x71);
            pPnPResources->IRQDescriptors[1].IRQLevel = PnPReadRegister(PNP_ISA_IO_BASE, 0x72);
            pPnPResources->IRQDescriptors[1].IRQType = PnPReadRegister(PNP_ISA_IO_BASE, 0x73);

            pPnPResources->DMADescriptors[0] = PnPReadRegister(PNP_ISA_IO_BASE, 0x74);

            pPnPResources->DMADescriptors[1] = PnPReadRegister(PNP_ISA_IO_BASE, 0x75);

            for ( i = 0; i < 4; i++ ) 
            {
                pPnPResources->Memory32Descriptors[i].MemoryBase =
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x76 + i * 16)) << 24;
                pPnPResources->Memory32Descriptors[i].MemoryBase |=
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x77 + i * 16)) << 16;
                pPnPResources->Memory32Descriptors[i].MemoryBase |=
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x78 + i * 16)) << 8;
                pPnPResources->Memory32Descriptors[i].MemoryBase |=
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x79 + i * 16));

                pPnPResources->Memory32Descriptors[i].MemoryControl =
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x7A + i * 16));

                pPnPResources->Memory32Descriptors[i].MemoryUpperLimit =
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x7B + i * 16)) << 24;
                pPnPResources->Memory32Descriptors[i].MemoryUpperLimit |=
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x7C + i * 16)) << 16;
                pPnPResources->Memory32Descriptors[i].MemoryUpperLimit |=
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x7D + i * 16)) << 8;
                pPnPResources->Memory32Descriptors[i].MemoryUpperLimit |=
                PnPReadRegister(PNP_ISA_IO_BASE, (BYTE)(0x7E + i * 16));
            }

        } 
        else 
        {
            pPnPResources->Flags = 0;
        }

        ulReturn = sizeof(ISA_PNP_RESOURCES);
    }

    PnPSetWaitForKey(PNP_ISA_IO_BASE);

    LeaveCriticalSection(&g_csISAConfig);

    return ulReturn;
}



//------------------------------------------------------------------------------
//
//  Function:  OALISACfgWrite
//
//  Writes data to the bus
//
ULONG OALISACfgWrite(
                     ULONG BusNumber,
                     ULONG SlotNumber,
                     ULONG Offset,
                     ULONG Length,
                     __in_bcount(Length) void* Buffer
                     )
{
    BYTE               ucCSNumber, ucLogicalDevice;
    const ISA_PNP_RESOURCES* pPnPResources = (const ISA_PNP_RESOURCES*)Buffer;
    int                 i;
    UNREFERENCED_PARAMETER(BusNumber);


    if ( g_bISAInitialized == FALSE )
        ISAInitBusInfo();

    if ( g_ucISANumberCSNs == 0 || g_ucISANumberCSNs == ~0U ) 
    {
        return 0;
    }

    ucCSNumber = (BYTE)(SlotNumber >> 8);
    ucLogicalDevice = (BYTE)SlotNumber;

    if ( ucCSNumber > g_ucISANumberCSNs || ucCSNumber == 0 ||
         Offset != 1 || Length != sizeof(ISA_PNP_RESOURCES) ) 
    {
        return 0;
    }

    EnterCriticalSection(&g_csISAConfig);

    PnPSendInitiationKey(PNP_ISA_IO_BASE);

    PnPWake(PNP_ISA_IO_BASE, ucCSNumber);

    PnPWriteRegister(PNP_ISA_IO_BASE, 0x07, ucLogicalDevice);

    if ( pPnPResources->Flags & ISA_PNP_RESOURCE_FLAG_ACTIVE ) 
    {
        for ( i = 0; i < 4; i++ ) 
        {
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x40 + i * 8),
                            (BYTE)(pPnPResources->Memory24Descriptors[i].MemoryBase >> 8));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x41 + i * 8),
                            (BYTE)(pPnPResources->Memory24Descriptors[i].MemoryBase));

            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x42 + i * 8),
                            pPnPResources->Memory24Descriptors[i].MemoryControl);

            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x43 + i * 8),
                            (BYTE)(pPnPResources->Memory24Descriptors[i].MemoryUpperLimit >> 8));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x44 + i * 8),
                            (BYTE)(pPnPResources->Memory24Descriptors[i].MemoryUpperLimit));
        }

        for ( i = 0; i < 8; i++ ) 
        {
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x60 + i * 2),
                            (BYTE)(pPnPResources->IoPortDescriptors[i] >> 8));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x61 + i * 2),
                            (BYTE)(pPnPResources->IoPortDescriptors[i]));
        }

        PnPWriteRegister(PNP_ISA_IO_BASE, 0x70, pPnPResources->IRQDescriptors[0].IRQLevel);
        PnPWriteRegister(PNP_ISA_IO_BASE, 0x71, pPnPResources->IRQDescriptors[0].IRQType);
        PnPWriteRegister(PNP_ISA_IO_BASE, 0x72, pPnPResources->IRQDescriptors[1].IRQLevel);
        PnPWriteRegister(PNP_ISA_IO_BASE, 0x73, pPnPResources->IRQDescriptors[1].IRQType);

        PnPWriteRegister(PNP_ISA_IO_BASE, 0x74, pPnPResources->DMADescriptors[0]);
        PnPWriteRegister(PNP_ISA_IO_BASE, 0x75, pPnPResources->DMADescriptors[1]);

        for ( i = 0; i < 4; i++ ) 
        {
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x76 + i * 16),
                            (BYTE)(pPnPResources->Memory32Descriptors[i].MemoryBase >> 24));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x77 + i * 16),
                            (BYTE)(pPnPResources->Memory32Descriptors[i].MemoryBase >> 16));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x78 + i * 16),
                            (BYTE)(pPnPResources->Memory32Descriptors[i].MemoryBase >> 8));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x79 + i * 16),
                            (BYTE)(pPnPResources->Memory32Descriptors[i].MemoryBase));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x7A + i * 16),
                            pPnPResources->Memory32Descriptors[i].MemoryControl);
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x7B + i * 16),
                            (BYTE)(pPnPResources->Memory32Descriptors[i].MemoryUpperLimit >> 24));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x7C + i * 16),
                            (BYTE)(pPnPResources->Memory32Descriptors[i].MemoryUpperLimit >> 16));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x7D + i * 16),
                            (BYTE)(pPnPResources->Memory32Descriptors[i].MemoryUpperLimit >> 8));
            PnPWriteRegister(PNP_ISA_IO_BASE, (BYTE)(0x7E + i * 16),
                            (BYTE)(pPnPResources->Memory32Descriptors[i].MemoryUpperLimit));
        }

        PnPWriteRegister(PNP_ISA_IO_BASE, 0x30, 1);
    } 
    else 
    {
        PnPWriteRegister(PNP_ISA_IO_BASE, 0x30, 0);
    }


    PnPSetWaitForKey(PNP_ISA_IO_BASE);

    LeaveCriticalSection(&g_csISAConfig);

    return sizeof(ISA_PNP_RESOURCES);
}


