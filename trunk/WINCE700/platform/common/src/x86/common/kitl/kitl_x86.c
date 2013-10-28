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
#include <x86boot.h>
#include <pc.h>
#include <x86kitl.h>
#include <halether.h>

// set by the OEM as a ASCII string defining the root of the device name
// commonly "CEPC"
extern LPCSTR  g_oalDeviceNameRoot;

// the default AdapterType for KITL, used if g_pX86Info->ucKitlAdapterType 
// is set to EDBG_ADAPTER_DEFAULT, this value will be passed to InitKitlNIC().
// InitKitlNIC will use it to help it identify the correct PCI NIC if it fails to find 
// any NICs that match the list of supported vendors(g_NicSupported). Helpful if the adapter conforms to
// an existing driver, but has a new vendor ID
extern const UCHAR g_ucDlftKitlAdaptorType;


static OAL_KITL_DEVICE g_kitlDevice;

//------------------------------------------------------------------------------
//
//  Function:  InitKitlEtherArgs
//
//  Sets up pKitlArgs with the device information needed for a ethernet KITL connection
//
static BOOL InitKitlEtherArgs(
                              __out OAL_KITL_ARGS *pKitlArgs
                              )
{
    unsigned int loopCounter;
    const unsigned char* pCompareChar;
    BOOL usingPCIBus;
    PCKITL_NIC_INFO pNic = InitKitlNIC (g_pX86Info->ucKitlIrq, g_pX86Info->dwKitlBaseAddr, g_pX86Info->ucKitlAdapterType);

    if (!pNic) 
    {
        // can't find a ethernet controller, bail.
        return FALSE;
    }
    
    // PCIConfig bytes all equal to LEGACY_KITL_DEVICE_BYTEPATTERN is InitKitlNIC's
    // way of telling us that our KITL Ethernet device isn't on a PCI bus.
    usingPCIBus = FALSE;
    pCompareChar = (const unsigned char*)(&(pNic->pciConfig));
    for(loopCounter = 0; loopCounter < sizeof(pNic->pciConfig); loopCounter++)
    {
         if( pCompareChar[loopCounter] != LEGACY_KITL_DEVICE_BYTEPATTERN)
         {
              // Not LEGACY_KITL_DEVICE_BYTEPATTERN, must be a PCI device.
              KITLOutputDebugString("Using a KITL device on the PCI bus, not a legacy device.\r\n",sizeof(pNic->pciConfig));
              usingPCIBus = TRUE;
              break;
         }
    }

    if(usingPCIBus)
    {
        pKitlArgs->devLoc.IfcType       = g_kitlDevice.ifcType
                                        = PCIBus;
        // bits 8-15 of the BusNumber are the host-to-PCI-bridge number which is 0
        // for x86 devices.  bits 0-7 are the subordinate bus number.
        pKitlArgs->devLoc.BusNumber     = pNic->dwBus & 0xff;

        // bits 16-23: subordinate bus number (yes, this is also represented
        //                                     in devLoc. busNumber)
        // bits 8-15 : device number
        // bits 0-7  : function number
        pKitlArgs->devLoc.LogicalLoc    = ((pNic->dwBus & 0xff) << 16)
                                          | ((pNic->dwDevice & 0xff) << 8)
                                          | (pNic->dwFunction & 0xff);                                         

        // id to match against for PCIBus ethernet cards, common code checks against this value
        g_kitlDevice.id                 = pNic->pciConfig.VendorID | (pNic->pciConfig.DeviceID << 16);
    }
    else
    {
        // Not using the PCI bus
        pKitlArgs->devLoc.IfcType       = g_kitlDevice.ifcType
                                        = (UINT32)InterfaceTypeUndefined;

        // id to match against for legacy ethernet cards, common code compares these values
        pKitlArgs->devLoc.LogicalLoc    = g_kitlDevice.id
                                        = pNic->dwIoBase;
    }

    // init flags
    pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED | OAL_KITL_FLAGS_EXTNAME;
    if (g_pX86Info->KitlTransport & KTS_PASSIVE_MODE)
        pKitlArgs->flags |= OAL_KITL_FLAGS_PASSIVE;
    if (!g_pX86Info->fStaticIP)
        pKitlArgs->flags |= OAL_KITL_FLAGS_DHCP;
    if (g_pX86Info->fKitlVMINI)
        pKitlArgs->flags |= OAL_KITL_FLAGS_VMINI;

    if (g_pX86Info->ucKitlIrq == OAL_KITL_IRQ_INVALID)
        pKitlArgs->flags |= OAL_KITL_FLAGS_POLL;
    else if (g_pX86Info->ucKitlIrq == 0)
        // the bootloader didn't set the IRQ, update with the IRQ that KITL is actually using
        g_pX86Info->ucKitlIrq       = pNic->dwIrq? (UCHAR) pNic->dwIrq : OAL_INTR_IRQ_UNDEFINED;

    pKitlArgs->devLoc.Pin           = pNic->dwIrq? pNic->dwIrq : OAL_INTR_IRQ_UNDEFINED;
    pKitlArgs->ipAddress            = g_pX86Info->dwKitlIP;

    g_kitlDevice.type               = OAL_KITL_TYPE_ETH;
    g_kitlDevice.pDriver            = (void*) pNic->pDriver;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  InitKitlSerialArgs
//
//  Sets up pKitlArgs with the device information needed for a serial KITL connection
//
static BOOL InitKitlSerialArgs(
                               __out OAL_KITL_ARGS *pKitlArgs
                               )
{
    DWORD dwIoBase = (1 == g_pX86Info->ucComPort)? COM2_BASE : COM1_BASE;  // base address

    // init flags
    pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED | OAL_KITL_FLAGS_POLL;
    if (g_pX86Info->KitlTransport & KTS_PASSIVE_MODE)
        pKitlArgs->flags |= OAL_KITL_FLAGS_PASSIVE;

    pKitlArgs->devLoc.LogicalLoc    = dwIoBase;
    pKitlArgs->devLoc.Pin           = (DWORD)OAL_INTR_IRQ_UNDEFINED;
    pKitlArgs->baudRate             = CBR_115200;
    pKitlArgs->dataBits             = DATABITS_8;
    pKitlArgs->parity               = PARITY_NONE;
    pKitlArgs->stopBits             = STOPBITS_10;

    g_kitlDevice.type               = OAL_KITL_TYPE_SERIAL;
    g_kitlDevice.pDriver            = (void*) GetKitlSerialDriver ();
    
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OALKitlStart
//
//  This function is called to start KITL module
//
BOOL OALKitlStart()
{
    OAL_KITL_ARGS   kitlArgs;
    BOOL            fRet = FALSE;

    memset (&kitlArgs, 0, sizeof (kitlArgs));

    if (EDBG_ADAPTER_DEFAULT == g_pX86Info->ucKitlAdapterType) 
    {
        g_pX86Info->ucKitlAdapterType = g_ucDlftKitlAdaptorType;
    }

    // common parts
    g_kitlDevice.name       = g_oalIoCtlPlatformType;
    
    // start the requested transport
    switch (g_pX86Info->KitlTransport & ~KTS_PASSIVE_MODE)
    {
        case KTS_SERIAL:
            kitlArgs.devLoc.IfcType = g_kitlDevice.ifcType
                                    = (UINT32)InterfaceTypeUndefined;
            fRet = InitKitlSerialArgs (&kitlArgs);
            if (fRet)
            {
                g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;
            }
            break;

        case KTS_ETHER:
        case KTS_DEFAULT:
            fRet = InitKitlEtherArgs (&kitlArgs);
            break;
        default:
            break;
    }

    if (fRet) 
    {
        fRet = OALKitlInit ((CHAR*)g_pX86Info->szDeviceName, &kitlArgs, &g_kitlDevice);
    }
    return fRet;
}

//------------------------------------------------------------------------------
//
//  Function:  OALKitlCreateName
//
//  This function creates a device name from a prefix and mac address (usually the last
//  two bytes of MAC address used for download).
//
void OALKitlCreateName(
                       __in CHAR *pPrefix, 
                       UINT16 mac[3], 
                       __out_bcount(OAL_KITL_ID_SIZE) CHAR *pBuffer
                       )
{
    if (!x86KitlCreateName (pPrefix, mac, pBuffer)) 
    {
        // not much we can do here since it doesn't have a return value.
        // Just copy the prefix to buffer. (message had displayed already,
        // don't need to do it again)
        StringCbCopyA (pBuffer, OAL_KITL_ID_SIZE, pPrefix);
    }

    // update the mac address and device name
    memcpy (g_pX86Info->wMac, mac, sizeof (g_pX86Info->wMac));
    StringCbCopyA ((CHAR*)g_pX86Info->szDeviceName, sizeof g_pX86Info->szDeviceName, pBuffer);
    
    RETAILMSG (1, (L"OALKitlCreateName: Using Device Name '%a'\r\n", pBuffer));
}



