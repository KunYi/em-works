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
#include <oal.h>
#include <pci.h>
#include <x86kitl.h>
#include <bootarg.h>

//------------------------------------------------------------------------------
//
//  Function:  FindNICByType
//
//  Interates the g_NicSupported struct looking for an entry matching ucType
//  returns a pointer to the entry or NULL
//
static PCSUPPORTED_NIC FindNICByType(
                                     const UCHAR ucType
                                     )
{
    int i;
    for (i = 0; i < g_nNumNicSupported; i ++) 
    {
        if (g_NicSupported[i].Type == ucType) 
        {            
            return &g_NicSupported[i];
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------
//
//  Function:  FindNICByVendor
//
//  Interates the g_NicSupported struct looking for an entry with matching VendorID/DeviceID
//  returns a pointer to the entry or NULL
//
static PCSUPPORTED_NIC FindNICByVendor(
                                       __in const PCI_COMMON_CONFIG * const pPciCfg
                                       )
{
    int i;
    for (i = 0; i < g_nNumNicSupported; i ++) 
    {
        if ((g_NicSupported[i].wVenId == pPciCfg->VendorID) &&
            (g_NicSupported[i].wDevId == pPciCfg->DeviceID)) 
        {
            return &g_NicSupported[i];
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------
//
//  Function:  printPCIConfig
//
//  Dumps information about a PCI card
//
static void printPCIConfig(
                           __in const PCI_COMMON_CONFIG* const config
                           )
{
    KITLOutputDebugString("+printPCIConfig\r\n");
    KITLOutputDebugString("config.VendorID           = 0x%x\r\n", config->VendorID);
    KITLOutputDebugString("config.DeviceID           = 0x%x\r\n", config->DeviceID);
    KITLOutputDebugString("config.Command            = 0x%x\r\n", config->Command);
    KITLOutputDebugString("config.Status             = 0x%x\r\n", config->Status);
    KITLOutputDebugString("config.RevisionID         = 0x%x\r\n", config->RevisionID);
    KITLOutputDebugString("config.ProgIf             = 0x%x\r\n", config->ProgIf);
    KITLOutputDebugString("config.SubClass           = 0x%x\r\n", config->SubClass);
    KITLOutputDebugString("config.BaseClass          = 0x%x\r\n", config->BaseClass);
    KITLOutputDebugString("config.CacheLineSize      = 0x%x\r\n", config->CacheLineSize);
    KITLOutputDebugString("config.LatencyTimer       = 0x%x\r\n", config->LatencyTimer);
    KITLOutputDebugString("config.HeaderType         = 0x%x\r\n", config->HeaderType);
    KITLOutputDebugString("config.BIST               = 0x%x\r\n", config->BIST);
    KITLOutputDebugString("config.BaseAddresses[0]   = 0x%x\r\n", config->u.type1.BaseAddresses[0]);
    KITLOutputDebugString("config.BaseAddresses[1]   = 0x%x\r\n", config->u.type1.BaseAddresses[1]);
    KITLOutputDebugString("config.PrimaryBusNumber   = 0x%x\r\n", config->u.type1.PrimaryBusNumber);
    KITLOutputDebugString("config.SecondaryBusNumber = 0x%x\r\n", config->u.type1.SecondaryBusNumber);
    KITLOutputDebugString("config.SubordinateBusNumber  = 0x%x\r\n", config->u.type1.SubordinateBusNumber);
    KITLOutputDebugString("config.SecondaryLatencyTimer = 0x%x\r\n", config->u.type1.SecondaryLatencyTimer);
    KITLOutputDebugString("config.IOBase             = 0x%x\r\n", config->u.type1.IOBase);
    KITLOutputDebugString("config.IOLimit            = 0x%x\r\n", config->u.type1.IOLimit);
    KITLOutputDebugString("config.SecondaryStatus    = 0x%x\r\n", config->u.type1.SecondaryStatus);
    KITLOutputDebugString("config.MemoryBase         = 0x%x\r\n", config->u.type1.MemoryBase);
    KITLOutputDebugString("config.MemoryLimit        = 0x%x\r\n", config->u.type1.MemoryLimit);
    KITLOutputDebugString("config.PrefetchableMemoryBase         = 0x%x\r\n", config->u.type1.PrefetchableMemoryBase);
    KITLOutputDebugString("config.PrefetchableMemoryLimit        = 0x%x\r\n", config->u.type1.PrefetchableMemoryLimit);
    KITLOutputDebugString("config.PrefetchableMemoryBaseUpper32  = 0x%x\r\n", config->u.type1.PrefetchableMemoryBaseUpper32);
    KITLOutputDebugString("config.PrefetchableMemoryLimitUpper32 = 0x%x\r\n", config->u.type1.PrefetchableMemoryLimitUpper32);
    KITLOutputDebugString("config.IOBaseUpper        = 0x%x\r\n", config->u.type1.IOBaseUpper);
    KITLOutputDebugString("config.IOLimitUpper       = 0x%x\r\n", config->u.type1.IOLimitUpper);
    KITLOutputDebugString("config.Reserved2          = 0x%x\r\n", config->u.type1.Reserved2);
    KITLOutputDebugString("config.ExpansionROMBase   = 0x%x\r\n", config->u.type1.ExpansionROMBase);
    KITLOutputDebugString("config.InterruptLine      = 0x%x\r\n", config->u.type1.InterruptLine);
    KITLOutputDebugString("config.InterruptPin       = 0x%x\r\n", config->u.type1.InterruptPin);
    KITLOutputDebugString("config.BridgeControl      = 0x%x\r\n", config->u.type1.BridgeControl);
    KITLOutputDebugString("-printPCIConfig\r\n");
}

//------------------------------------------------------------------------------
//
//  Function:  InitKitlNIC
//
//  Finds a supported PCI NIC, matching against g_NicSupported
//  If nothing is found it will return attempt to use dwDfltType with the input params, dwIrq and dwIoBase.
//  If that failes too, it will return NULL
//
PCKITL_NIC_INFO InitKitlNIC(
                            DWORD dwIrq, 
                            DWORD dwIoBase, 
                            DWORD dwDfltType
                            )
{
    PCI_COMMON_CONFIG   pciConfig;
    int                 funcType, bus, device, function;
    PCSUPPORTED_NIC     pNicFound = FALSE;
    int                 length = 0;
    enum {
        FIND_BY_VENDOR, // 0
        FIND_BY_TYPE    // 1
    };

    // InitKitlNIC returns a pointer to this
    static KTIL_NIC_INFO KitlNic;

    KITLOutputDebugString("InitKitlNIC: Searching for PCI Ethernet NIC (dwIrq = %x, dwIoBase = %x, dwDfltType = %x) ...\r\n",
        dwIrq, dwIoBase, dwDfltType);

    // Pass 1: iterate searching for vendor (this is the best match)
    // Pass 2: iterate searching for matching type
    for (funcType = FIND_BY_VENDOR; funcType <= FIND_BY_TYPE; funcType++)
    {
        // iterate through buses
        for (bus = 0; bus < PCI_MAX_BUS; bus++) 
        {
            // iterate through devices
            for (device = 0; device < PCI_MAX_DEVICES; device++) 
            {
                // iterate through functions
                for (function = 0; function < PCI_MAX_FUNCTION; function++) 
                {
                    // read PCI config data
                    OAL_PCI_LOCATION pciLoc;
                    pciLoc.logicalLoc = bus << 16 | device << 8 | function;

                    length = OALPCICfgRead(0,
                        pciLoc,
                        0,
                        (sizeof(pciConfig) - sizeof(pciConfig.DeviceSpecific)),
                        &pciConfig);

                    if (length == 0 || (pciConfig.VendorID == 0xFFFF))
                        break;

                    // network controller or USB?
                    if (    (  (pciConfig.BaseClass == PCI_CLASS_NETWORK_CTLR)
                            && (pciConfig.SubClass  == PCI_SUBCLASS_NET_ETHERNET_CTLR)) // Network device.
                        ||  (  (pciConfig.BaseClass == PCI_CLASS_BRIDGE_DEV)
                            && (pciConfig.SubClass  == PCI_SUBCLASS_BR_OTHER))) 
                    {    
                        // Other Unknown Special Devices
                        DWORD dwFoundBase = pciConfig.u.type0.BaseAddresses[0] & 0xFFFFFFFC;
                        DWORD dwFoundIrq  = pciConfig.u.type0.InterruptLine;
                    
                        if (dwFoundIrq && dwFoundBase) 
                        {
                            if (   (!dwIrq && !dwIoBase)                                    // IRQ && IoBase not specified -- use 1st found
                                || (dwIrq == OAL_KITL_IRQ_INVALID)                          // Undefined IRQ = Poll mode -- use first found    
                                || (!dwIoBase && (dwIrq == dwFoundIrq))                     // IRQ match, no IO base specified
                                || (dwIoBase == dwFoundBase))                               // IOBase match
                            {
                                if(funcType == FIND_BY_VENDOR) 
                                {
                                    pNicFound = FindNICByVendor (&pciConfig);
                                }
                                else if(funcType == FIND_BY_TYPE) 
                                {
                                    pNicFound = FindNICByType ((UCHAR) dwDfltType);
                                }

                                if (pNicFound) 
                                {
                                    // found NIC card
                                    KitlNic.dwIoBase   = dwFoundBase;
                                    KitlNic.dwIrq      = dwFoundIrq;
                                    KitlNic.dwBus      = bus;
                                    KitlNic.dwDevice   = device;
                                    KitlNic.dwFunction = function;
                                    KitlNic.pDriver    = pNicFound->pDriver;
                                    KitlNic.dwType     = pNicFound->Type;
                                    memcpy (&KitlNic.pciConfig, &pciConfig, sizeof(pciConfig));

                                    KITLOutputDebugString ("InitKitlNIC: Found PCI Ethernet NIC (type = %x, IRQ=%d, IOBase=0x%x).\r\n",
                                        pNicFound->Type, dwFoundIrq, dwFoundBase);

                                    return &KitlNic;
                                }
                                else
                                {
                                    KITLOutputDebugString ("InitKitlNIC: skipping unknown PCI Ethernet NIC: (subclass=%x, Vendor=%x, Device=%x)\r\n", 
                                                            pciConfig.SubClass, pciConfig.VendorID, pciConfig.DeviceID);
                                }
                            }
                            else
                            {
                                // found a NIC, but it didn't match what the bootloader was using
                                KITLOutputDebugString ("InitKitlNIC: skipping PCI Ethernet NIC: (subclass = %x, IRQ=%d, IOBase=0x%x).\r\n",
                                                        pciConfig.SubClass, dwFoundIrq, dwFoundBase);
                            }
                        }
                    }
                
                    if (function == 0 && !(pciConfig.HeaderType & 0x80)) 
                        break;
                }
                if (length == 0)
                    break;
            }

            if (length == 0 && device == 0)
                break;
        }

    }

    // can't find it on PCI bus, if IRQ and IoBase are specified, use it
    pNicFound = FindNICByType ((UCHAR) dwDfltType);
    if (dwIrq && dwIoBase && pNicFound) 
    {
        KitlNic.dwIoBase   = dwIoBase;
        KitlNic.dwIrq      = dwIrq;
        KitlNic.pDriver    = pNicFound->pDriver;
        KitlNic.dwType     = dwDfltType;

        // Signal that we're using a device but it's not on the PCI bus
        memset(&KitlNic.pciConfig, LEGACY_KITL_DEVICE_BYTEPATTERN, sizeof(pciConfig));

        KITLOutputDebugString ("InitKitlNIC: Can't find PCI Ethernet NIC, use specified data (type = %x, IRQ=%d, IOBase=0x%x).\r\n",
            pNicFound->Type, dwIrq, dwIoBase);
        return &KitlNic;
    }
    
    return NULL;
}

//------------------------------------------------------------------------------
//
//  Function:  itoa10
//
//  Converts an interger 'n', to a string in 's', using base 10
//
static void itoa10(
                   int n,
                   __out_ecount(bufflen) CHAR s[],
                   int bufflen
                  )
{
    int i = 0; 

    // Get absolute value of number
    unsigned int val = (unsigned int)((n < 0) ? -n : n);

    // Extract digits in reverse order
    while (val)
    {
        // Make sure we don't step off the end of the character array (leave
        // room for the possible '-' sign and the null terminator).
        if (i < (bufflen - 2))
        {
            s[i++] = (val % 10) + '0';
        }

        val /= 10;
    }

    // Add sign if number negative
    if (n < 0) s[i++] = '-';

    s[i--] = '\0';

    // Reverse string
    for (n = 0; n < i; n++, i--) 
    {
        char swap = s[n];
        s[n] = s[i];
        s[i] = swap;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  FindNICAbbrev
//
//  Returns szAbbrev for the NIC whos upper 3 bytes of MAC match an entry in the g_NicSupported array
//  Commonly this is a two letter abbrevation, plus NULL terminator
//
static LPCSTR FindNICAbbrev(
                            DWORD dwUpperMAC
                            )
{
    int i;
    for (i = 0; i < g_nNumNicSupported; i ++) 
    {
        if (g_NicSupported[i].dwUpperMAC == dwUpperMAC) 
        {            
            return (const CHAR*)g_NicSupported[i].szAbbrev;
        }
    }
    KITLOutputDebugString ("FindNICAbbrev: Can't find NIC Abbreviation for MAC: %x\r\n", dwUpperMAC);
    return "";
}

//------------------------------------------------------------------------------
//
//  Function:  UpperDWFromMAC
//
//  Returns the top 3 bytes of the MAC address
//
static DWORD UpperDWFromMAC(
                            const UINT16 wMAC []
                           )
{
    DWORD ret;

    //
    // The WORDs in wMAC field are in net order, so we need to do some
    // serious shifting around.
    // A hex ethernet address of 12 34 56 78 9a bc is stored in wMAC array as
    // wMAC[0] = 3412, wMAC[1] = 7856, wMAC[2] = bc9a.
    // The 4 byte return value should look like 0x00123456
    //
    ret = (wMAC[0] & 0x00ff) << 16;
    ret |= wMAC[0] & 0xff00;
    ret |= wMAC[1] & 0x00ff;
    return ret;
}

//------------------------------------------------------------------------------
//
//  Function:  Splice
//
//  splices out a byte from a mac
//
static BYTE Splice(const UINT16 wMAC [3], BYTE byte)
{
    byte = byte%6; // macs are 48bit
    return byte%2?LOBYTE(wMAC[2-(byte>>1)]):HIBYTE(wMAC[2-(byte>>1)]);
}

//------------------------------------------------------------------------------
//
//  Function:  ToASCII
//
//  converts 4 bits to ASCII
//
static char ToASCII(BYTE b)
{
    b = b%0x10;// only accept 4bits
    return b<10?b+'0':b+'7';
}

//------------------------------------------------------------------------------
//
//  Function:  x86KitlCreateName
//
//  This function creates a device name from prefix and mac address (usually last
//  two bytes of MAC address used for download).
//
BOOL x86KitlCreateName(
                       __in const CHAR * const pPrefix, 
                       const UINT16 mac[], 
                       __out_bcount(OAL_KITL_ID_SIZE) CHAR * const pBuffer
                       )
{
    DWORD  dwUpperMAC = UpperDWFromMAC (mac);
    size_t nLen = 0, CchBuffer = 0;

    #ifndef BUILDING_BOOTLOADER // Not building boot loader
    const BOOT_ARGS ** ppArgs = (BOOT_ARGS **)(BOOT_ARG_PTR_LOCATION);
    const BOOT_ARGS * pArgs = (const BOOT_ARGS *)(*(const DWORD*)ppArgs | 0x80000000);
    #else
    #define BOOT_ARG_PTR_LOCATION_NP    0x001FFFFC
    const BOOT_ARGS ** ppArgs = (BOOT_ARGS **)(BOOT_ARG_PTR_LOCATION_NP);
    const BOOT_ARGS * pArgs = (const BOOT_ARGS *)(*(const DWORD*)ppArgs);
    #endif


    // calculate total length needed
    StringCchLengthA(pPrefix, MAX_PATH, &nLen);
    nLen += 2 + 5;      // 2 for vendor id, 5 for itoa of 16 bit value

    if (OAL_KITL_ID_SIZE < nLen) 
    {
        KITLOutputDebugString ("x86KitlCreateName: Device Name '%s' too long, can't create KITL name\r\n", pPrefix);
        return FALSE;
    }

    if (pArgs->MinorVersion >= 2 || pArgs->MajorVersion >= 2) {
        // v1.2 or greater uses a different bootme name
        StringCchCopyA (pBuffer, OAL_KITL_ID_SIZE, "PC-");
        StringCchLengthA(pBuffer, OAL_KITL_ID_SIZE, &CchBuffer);
        while (CchBuffer < OAL_KITL_ID_SIZE - 1) {
            BYTE b = Splice(mac, (BYTE)((OAL_KITL_ID_SIZE - CchBuffer)>>1) - 1);

            // if (not right aligned with the buffer)
            if ((OAL_KITL_ID_SIZE - CchBuffer - 1) % 2 != 1)
                pBuffer[CchBuffer++] = ToASCII(b>>4);
            pBuffer[CchBuffer++] = ToASCII(b&0xf);
        }
        pBuffer[OAL_KITL_ID_SIZE-1] = 0;
    } else {
        StringCbCopyA (pBuffer, OAL_KITL_ID_SIZE, pPrefix);
        StringCbCatA (pBuffer, OAL_KITL_ID_SIZE, FindNICAbbrev (dwUpperMAC));
        
        StringCchLengthA(pBuffer, OAL_KITL_ID_SIZE, &CchBuffer);
        itoa10 (((mac[2]>>8) | ((mac[2] & 0x00ff) << 8)), (pBuffer + CchBuffer), (OAL_KITL_ID_SIZE - CchBuffer));
    }
    KITLOutputDebugString ("x86KitlCreateName: Using Device Name '%s'\r\n", pBuffer);

    return TRUE;
}

