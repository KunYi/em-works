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
//  File: pci.c
//
//  This file implements PCI configuration space read/write for Vrc5477.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <vrc5477.h>

//------------------------------------------------------------------------------
//
//  Function:  OALPCICfgRead
//
//  This functions read information from PCI configuration space on Vrc5477.
//  Current implementation doesn't use offset in configuration space because
//  it apparently doesn't work. So number of devices on bus 0 is limited by
//  size of first window (usually used as PCI memory window).
//
UINT32 OALPCICfgRead(
    UINT32 busId, OAL_PCI_LOCATION pciLoc, UINT32 offset, UINT32 size,
    VOID *pData
) {
    VRC5477_REGS *pVRC5477Regs = OALPAtoUA(VRC5477_REG_PA);
    UINT32 *pPciInit, pciInit, pciType, window;
    UINT32 address, value, count = 0;
    BOOL enabled;
    UINT8 *p = (UINT8*)pData;

    OALMSG(OAL_PCI&&OAL_FUNC, (
        L"+OALPCICfgRead(%d, %d/%d/%d, %d, %d, 0x%08x)\r\n", busId, 
        pciLoc.bus, pciLoc.dev, pciLoc.fnc, offset, size, pData
    ));


    // We support only buses 0 and 1
    if (busId > 1) {
        OALMSG(OAL_WARN, (
            L"WARN: OALPCICfgRead: Unsupported bus id %d\r\n", busId
        ));
        goto cleanUp;
    }

    // Check region to read
    if ((offset + size) > 256) goto cleanUp;

    // Depending on bus number
    if (busId == 0) {
        // Use this register
        pPciInit = &pVRC5477Regs->PCIINIT00;
        window = INREG32(&pVRC5477Regs->PCIW0);
        // Get configuration address
        if (pciLoc.bus == 0) {
            // Type 0 configuration
            if (pciLoc.dev == 0 || pciLoc.dev > 21) goto cleanUp;
            address = (1 << (pciLoc.dev + 10))|(pciLoc.fnc << 8);
            pciType = PCI_INIT_TYPE0;
        } else {
            // Type 1 configuration
            if (pciLoc.bus > 31) goto cleanUp;
            address = (pciLoc.fnc << 8) |(pciLoc.dev << 11) |(pciLoc.bus << 16);
            pciType = PCI_INIT_TYPE1;
        }
    } else {
        // There is only bus 0 on internal PCI
        if (pciLoc.bus != 0) goto cleanUp;
        // Type 0 configuration
        if (pciLoc.dev == 0 || pciLoc.dev > 21) goto cleanUp;
        // Use this register
        pPciInit = &pVRC5477Regs->PCIINIT01;
        window = INREG32(&pVRC5477Regs->IOPCIW0);
        // Type 0 configuration
        address = (1 << (pciLoc.dev + 10))|(pciLoc.fnc << 8);
        pciType = PCI_INIT_TYPE0;
    }                

    // Check if we fit to window, if not we failed
    window = 1 << (36 - (window & 0xF));
    if (address >= window) goto cleanUp;

    // Disable interrupt
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Switch from memory cycle to config
    pciInit = INREG32(pPciInit);
    OUTREG32(pPciInit, PCI_INIT_TYPE_CFG|pciType);

    // Get window address
    address += window & 0xFFE00000;

    // Transform address
    address = (UINT32)OALPAtoUA(address);

    // First read any unaligned data on start
    if ((offset & 0x03) != 0) {
        value = INREG32(address + (offset & ~0x03));
        value >>= (offset & 0x03) << 3;
        while ((offset & 0x03) != 0 && size > 0) {
            *p++ = (UINT8)value;
            value >>= 8;
            offset++;
            size--;
            count++;
        }
    }      
    
    // Then read full DWORDs
    while (size >= 4) {
        value = INREG32(address + offset);
        *p++ = (UINT8)value;
        *p++ = (UINT8)(value >> 8);
        *p++ = (UINT8)(value >> 16);
        *p++ = (UINT8)(value >> 24);
        offset += 4;
        size -= 4;
        count += 4;
    }   
    
    // And remaining data at end    
    if (size > 0) {
        value = INREG32(address + offset);
        while (size > 0) {
            *p++ = (UINT8)value;
            value >>= 8;
            size--;
            count++;
        }
    }

    // Switch back to original cycles
    OUTREG32(pPciInit, pciInit);
    INTERRUPTS_ENABLE(enabled);
    
cleanUp:
    OALMSG(OAL_PCI&&OAL_FUNC, (L"-OALPCICfgRead(count = %d)\r\n", count));
    return count;
}

//------------------------------------------------------------------------------
//
//  Function:  OALPCICfgWrite
//
//  This functions write information to PCI configuration space on Vrc5477.
//  Current implementation doesn't use offset in configuration space because
//  it apparently doesn't work. So number of devices on bus 0 is limited by
//  size of first window (usually used as PCI memory window).
//
UINT32 OALPCICfgWrite(
    UINT32 busId, OAL_PCI_LOCATION pciLoc, UINT32 offset, UINT32 size,
    VOID *pData
) {
    VRC5477_REGS *pVRC5477Regs = OALPAtoUA(VRC5477_REG_PA);
    UINT32 *pPciInit, pciInit, pciType, window;
    UINT32 address, value, count = 0;
    BOOL enabled;
    UINT8 *p = (UINT8*)pData;

    OALMSG(OAL_PCI&&OAL_FUNC, (
        L"+OALPCICfgWrite(%d, %d/%d/%d, %d, %d, 0x%08x\r\n",
        busId, pciLoc.bus, pciLoc.dev, pciLoc.fnc, offset, size, pData
    ));

    // We support only buses 0 and 1
    if (busId > 1) {
        OALMSG(OAL_WARN, (
            L"WARN: OALPCICfgWrite: Unsupported bus id %d\r\n", busId
        ));
        goto cleanUp;
    }

    // Check region to read
    if ((offset + size) > 256) goto cleanUp;

    // Depending on bus number
    if (busId == 0) {
        // Use this register
        pPciInit = &pVRC5477Regs->PCIINIT00;
        window = INREG32(&pVRC5477Regs->PCIW0);
        // Get configuration address
        if (pciLoc.bus == 0) {
            // Type 0 configuration
            if (pciLoc.dev > 21) goto cleanUp;
            address = (1 << (pciLoc.dev + 10))|(pciLoc.fnc << 8);
            pciType = PCI_INIT_TYPE0;
        } else {
            // Type 1 configuration
            if (pciLoc.bus > 31) goto cleanUp;
            address = (pciLoc.fnc << 8) |(pciLoc.dev << 11) |(pciLoc.bus << 16);
            pciType = PCI_INIT_TYPE1;
        }
    } else {
        // There is only bus 0 on internal PCI
        if (pciLoc.bus != 0 || pciLoc.dev > 21) goto cleanUp;
        // Use this register
        pPciInit = &pVRC5477Regs->PCIINIT01;
        window = INREG32(&pVRC5477Regs->IOPCIW0);
        // Type 0 configuration
        address = (1 << (pciLoc.dev + 10))|(pciLoc.fnc << 8);
        pciType = PCI_INIT_TYPE0;
    }                

    // Check if we fix to window, if not we failed
    window = 1 << (36 - (window & 0xF));
    if (address >= window) goto cleanUp;

    // Disable interrupt
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Switch to config read/write cycles
    pciInit = INREG32(pPciInit);
    OUTREG32(pPciInit, PCI_INIT_TYPE_CFG|pciType);

    // Get window address
    address += window & 0xFFE00000;

    // Transform address
    address = (UINT32)OALPAtoUA(address);

    // First write any unaligned data on start
    if ((offset & 0x03) != 0) {
        value = INREG32(address + (offset & ~0x03));
        while ((offset & 0x03) != 0 && size > 0) {
            value &= ~(0xFF << ((offset & 0x03) << 3));
            value |= (UINT32)(*p++ << ((offset & 0x03) << 3));
            offset++;
            size--;
            count++;
        }
        OUTREG32(address + ((offset - 1)&~0x03), value);
    }
    
    // Then write full DWORDs
    while (size >= 4) {
       value =  (UINT32)(*p++);
       value |= (UINT32)(*p++ << 8);
       value |= (UINT32)(*p++ << 16);
       value |= (UINT32)(*p++ << 24);
       OUTREG32(address + offset, value);
       offset += 4;
       size -= 4;
       count += 4;
    }
    
    // And remaining data at end    
    if (size > 0) {
        value = INREG32(address + offset);
        while (size > 0) {
            value &= ~(0xFF << ((offset & 0x03) << 3));
            value |= (UINT32)(*p++ << ((offset & 0x03) << 3));
            offset++;
            size--;
            count++;
        }
        OUTREG32(address + ((offset - 1)&~0x03), value);
    }

    // Switch back to original cycles
    OUTREG32(pPciInit, pciInit);
    INTERRUPTS_ENABLE(enabled);

cleanUp:
    OALMSG(OAL_PCI&&OAL_FUNC, (L"-OALPCICfgWrite(count = %d)\r\n", count));
    return count;
}

//------------------------------------------------------------------------------

VOID OALPCIPowerOff(UINT32 busId)
{
}

//------------------------------------------------------------------------------

VOID OALPCIPowerOn(UINT32 busId)
{
}

//------------------------------------------------------------------------------

