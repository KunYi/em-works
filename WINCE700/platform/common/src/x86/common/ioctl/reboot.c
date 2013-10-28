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
#include <x86boot.h>
#include <kitl.h>
#include <oal.h>
#include <bootarg.h>
#include <acpi.h>

// Hardware reset fix-up var (see config.bib).
//
DWORD *pdwHardReset = (DWORD *)-1;


// Reboot address
//
extern DWORD dwRebootAddress;
extern void StartUp(void);

static BYTE *RebootAddr;
static BYTE RebootMagic;
static BYTE RebootAddressSpace;

//------------------------------------------------------------------------------
//
//  Function: x86RebootInit
//
//  called during oeminit, retrives the reset vector from the FADT
//
void x86RebootInit(void)
{
    void * TableData;
    AcpiTable * TableHeader;
    if (AcpiFindATable(ACPI_TABLE_FACP, &TableData, &TableHeader) 
        && TableHeader->dwLength >= offsetof(FADT, Reserved3)+sizeof(AcpiTable)) {
        FADT* pFADT = (FADT*)TableData;
        if (pFADT->RESET_REG_Address.HighPart == 0) {
            ASSERT(pFADT->RESET_REG_BitOffset == 0);
            ASSERT(pFADT->RESET_REG_BitWidth == 8);
            RebootAddr = (BYTE*)pFADT->RESET_REG_Address.LowPart;
            RebootMagic = pFADT->RESET_VALUE;
            RebootAddressSpace = pFADT->RESET_REG_AddressSpace;
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function: HardResetUsingACPI
//
//  Attempts a Reset according to how ACPI said it could be done
//  On some Intel boxes this is the same as HardResetUsingPIIX4
//
static void HardResetUsingACPI()
{
    if (RebootAddr && RebootMagic) {
        switch (RebootAddressSpace) {
            case 0:
                *RebootAddr = RebootMagic;
                break;
            case 1:
                WRITE_PORT_UCHAR(RebootAddr, RebootMagic);
                break;
            default:
                break;
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function: HardResetUsingPIIX4
//
//  Attempts a Reset by setting the PCI Reset bit introduced on the PIIX4E
//
static void HardResetUsingPIIX4()
{
    // PIIX4 PCI Reset Control Register
    //
    // Bits 1 and 2 in this register are used by PIIX4 to generate a hard reset or a soft reset. During a hard reset, 
    // PIIX4 asserts CPURST, PCIRST#, and RSTDRV, as well as reset its core and suspend well logic. During  
    // a soft reset, PIIX4 asserts INIT. 
    enum { PIIX4_PCI_RSTCTRL = 0xcf9 };

    // Bit 2: Reset CPU (RCPU). This bit is used to initiate (transitions from 0 to 1) a hard reset (bit 1 in this 
    //        register is set to 1) or a soft reset to the CPU. PIIX4 will also initiate a hard reset when PWROK is
    //        asserted. This bit cannot be read as a 1. 
    // Bit 1: System Reset (SRST). This bit is used to select the type of reset generated when bit 2 in this 
    //        register is set to 1. When SRST=1, PIIX4 initiates a hard reset to the CPU when bit 2 in this 
    //        register transitions from 0 to 1. When SRST=0, PIIX4 initiates a soft reset when bit 2 in this 
    //        register transitions from 0 to 1.  
    enum 
    {
        PIIX4_PCI_RSTCTRL_RCPU = 0x4,
        PIIX4_PCI_RSTCTRL_SRST = 0x2
    };

    RETAILMSG(ZONE_WARNING, (TEXT("Trying hard reset via PIIX4E PCI Reset Control Register...\r\n")));

    // This should be set first, but it fails on some chipsets, 
    // resulting in a lockup, so we set RCPU as we reset
    // WRITE_PORT_UCHAR(PIIX4_PCI_RSTCTRL, PIIX4_PCI_RSTCTRL_RCPU); 

    WRITE_PORT_UCHAR((UCHAR*)PIIX4_PCI_RSTCTRL, PIIX4_PCI_RSTCTRL_RCPU | PIIX4_PCI_RSTCTRL_SRST); 
    NKSleep(1);

    RETAILMSG(ZONE_WARNING, (TEXT("...PIIX4E Hard Reset failed\r\n")));
}

//------------------------------------------------------------------------------
//
//  Function:  ResetUsing8042
//
//  Attempts a Reset by pulsing the reset line attached to the 8042 controller
//
static void ResetUsing8042()
{
    // 64h  (write):  8042 command register:  Writing this port sets Bit 3
    //    of the status register to 1 and the byte is treated
    //    as a controller command.  Devices attached to the
    //    8042 should be disabled before issuing commands that
    //    return data since data in the output register will
    //    be overwritten.
    enum { I8042_CMD = 0x64 };

    // Fx: Pulse Output Port: Bits 0-3 of the 8042 output port can be
    //    pulsed low for 6 ms;  Bits 0-3 of command indicate which
    //    Bits should be pulsed; 0=pulse, 1=don't pulse; pulsing
    //    Bit 0 results in CPU reset since it is connected to system
    //    reset line.
    enum { I8042_CMD_PULSEPORT = 0xf0 };

    RETAILMSG(ZONE_WARNING, (TEXT("Trying reset via 8042 keyboard controller...\r\n")));

    WRITE_PORT_UCHAR((UCHAR*)I8042_CMD, I8042_CMD_PULSEPORT | ((BYTE)~1)) ;
    NKSleep(6);

    RETAILMSG(ZONE_ERROR, (TEXT("...8042 Reset failed\r\n")));
}

BOOL x86IoCtlHalReboot (
    UINT32 code, const VOID *lpInBuf, UINT32 nInBufSize, const VOID *lpOutBuf, 
    UINT32 nOutBufSize, const UINT32 *lpBytesReturned
) {
    WORD* pwRebootSig = (WORD *) BOOT_ARG_REBOOT_SIG_LOCATION;

    UNREFERENCED_PARAMETER(lpBytesReturned);
    UNREFERENCED_PARAMETER(nOutBufSize);
    UNREFERENCED_PARAMETER(lpOutBuf);
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(code);

    // There are two ways to reboot this system: a hardware reset (which will cause
    // the BIOS to run again) and a soft reset (which jumps back to the bootloader
    // which is already in RAM).  The former will reinitialize all hardware (of
    // most interest is the PCI bus and PCI devices) and will also clear RAM.  The
    // latter will not clear RAM and thus will allow the RAM filesystem to persist
    // across reboots.  While the soft reset is faster and keeps the RAM filesystem
    // intact, because it doesn't reset and reinitialize the PCI bus, PCI devices
    // don't know that the system has been reset and outstanding DMA operations or
    // interrupts can cause problems.
    //
    // The correct way to handle both the hardware and the soft resets is to make
    // use of the power manager to transition into a reboot power state.  The power
    // manager will request that all drivers quiesce their devices before it calls
    // this IOCTL.  When done this way, the soft reboot case won't have the 
    // DMA/interrupt problem noted above (it's only an issue when this IOCTL is 
    // called directly outside of the power manager).
    //
    // In order to minimize the cases in which we do a soft reboot (to thus minimize
    // the possibility problems), we'll use the following algorithm to determine 
    // when to hardware reboot:
    //
    // 1. If the user has built this image with the variable BSP_HARDRESET == 1 (note
    //    this is a romimage fixup variable so only makeimg needs to be run when
    //    the value is changed), then we'll force a hard reset.  Otherwise:
    // 2. If there isn't a KITL connection, then we'll force a hard reset.  Otherwise:
    // 3. Do a soft reboot.
    //

    // Are we doing a hardware reset (see config.bib)?
    //
    if (pdwHardReset || !(g_pX86Info->KitlTransport & ~KTS_PASSIVE_MODE) || (g_pX86Info->KitlTransport == KTS_NONE)) {

        *pwRebootSig = 0;

        HardResetUsingACPI();
        HardResetUsingPIIX4();

        ResetUsing8042();

    } else {
        // Perform a soft reset of the device...
        //
        // OEMIoControl runs at Ring 1, but to reset, we need to be running at Ring 0.
        // To accomplish this, we set dwRebootAddress which is checked in the PerpISR
        // routine (ISRs run at Ring 0).  If set, the PerpISR will reset the device.
        //
        RETAILMSG(1, (TEXT("Attempting soft reset, make sure you have unloaded all PCI drivers first.\r\n")));
        RETAILMSG(1, (TEXT("Soft reset is known to be unreliable, please consider using hard reset and\r\n")));
        RETAILMSG(1, (TEXT(" a non volatile store.\r\n")));
        RETAILMSG(1, (TEXT("Trying Soft Reset via jumping to address 0x%08X...\r\n"), g_pX86Info->dwRebootAddr));
        ASSERT(dwRebootAddress); // shouldn't be NULL

        // Set the reboot sig in the global bootargs struct to preserve the registry across the reboot.
        *pwRebootSig = BOOTARG_REBOOT_SIG;

        dwRebootAddress = g_pX86Info->dwRebootAddr;
        NKSleep(2);

        RETAILMSG(1, (TEXT("System should reboot on the next tick...\r\n")));
    }

    // if we got this far, something is wrong - return error.
    return FALSE;
}
