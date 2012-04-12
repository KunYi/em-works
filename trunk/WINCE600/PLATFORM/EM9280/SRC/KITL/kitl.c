//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File: kitl.c
//
// Support routines for KITL.
//
// Note: These routines are stubbed out in the kern image.
//
//-----------------------------------------------------------------------------
#include <bsp.h>
#include <kitl_cfg.h>
#include <usbkitl.h>

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables
//extern DWORD g_dwKITLThreadPriority;
PVOID pv_HWregRTC = NULL;
//-----------------------------------------------------------------------------
// Defines
//#define KITL_THREAD_HIGH_PRIORITY   131 // Default KITL thread priority is 131.

BSP_ARGS * g_pBSPArgs;

//------------------------------------------------------------------------------
// Types
extern BOOL ApbhDmaInit(void);
extern BOOL ApbhDmaDeInit(void);
extern BOOL ApbhDmaAlloc(void);
extern BOOL ApbhDmaDealloc(void);

extern BOOL IomuxAlloc(void);
extern BOOL IomuxDealloc(void);

extern BOOL ClockAlloc(void);

//------------------------------------------------------------------------------
// Global Variables

static OAL_KITL_ETH_DRIVER g_kitlENET = 
{
    ENETInit,
    ENETInitDMABuffer,
    NULL,
    ENETSendFrame,
    ENETGetFrame,
    ENETEnableInts,
    ENETDisableInts,
    ENETPowerOff,
    ENETPowerOn,
    ENETCurrentPacketFilter,
    ENETMulticastList
};


//------------------------------------------------------------------------------
// Local Variables
static OAL_KITL_ETH_DRIVER g_kitlUsbRndis = OAL_ETHDRV_RNDIS;
static OAL_KITL_SERIAL_DRIVER g_kitlUsbSerial = USB_SERIAL_KITL ;

static OAL_KITL_DEVICE g_kitlDevices[] =
{
    {
        L"USB_RNDIS ", Internal, CSP_BASE_REG_PA_USBCTRL0, 0,
        OAL_KITL_TYPE_ETH, &g_kitlUsbRndis
    },
    {
        L"USB_SERIAL",Internal,  CSP_BASE_REG_PA_USBCTRL0 + 1, 0,
        OAL_KITL_TYPE_SERIAL, &g_kitlUsbSerial
    },
    {
        L"ENET", Internal, CSP_BASE_REG_PA_ENET, 0,
        OAL_KITL_TYPE_ETH, &g_kitlENET
    },
    {
        NULL, 0, 0, 0, 0, NULL
    }
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: OEMKitlStartup
//
// First entry point to OAL when Kitldll is loaded. OEM calls
// KITLIoctl IOCTL_KITL_STARTUP when it's ready to start kitl.
//
// Parameters:
//      None.
//
// Returns:
//      If this function succeeds, it returns TRUE, otherwise
//      it returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL OEMKitlStartup(void)
{
    BOOL rc = FALSE;
    OAL_KITL_ARGS *pKITLArgs, KITLArgs;
    OAL_KITL_DEVICE *pKitlDevice = NULL;
    CHAR *pszDeviceId, buffer[OAL_KITL_ID_SIZE];
    UINT32 numKitlDevice = 0;

    g_pBSPArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;

    // KITL gets its own debug zone and does not share
    // the OAL one any longer.
    //
    // The default dpCurSettings of KITL is defined in
    // PLATFORM\COMMON\SRC\COMMON\LOG\KITL\debug.c.
    // We can change it here if needed.
    //
    // Also note that ZONE_KITL_ETHER should not be enabled unless you
    // suspect that there may be a problem with ETHDRV driver.
    // Otherwise, you will get an almost continuous stream of Ethernet frame
    // handling messages from the ETHDRV driver which will result in very
    // slow performance of the target device.
    //
    //dpCurSettings.ulZoneMask = (ZONE_WARNING | ZONE_ERROR |
    //    ZONE_INIT | ZONE_KITL_OAL | ZONE_KITL_ETHER);
    dpCurSettings.ulZoneMask = (ZONE_WARNING | ZONE_ERROR |
        ZONE_INIT);
    //dpCurSettings.ulZoneMask = 0x8003 | ZONE_KITL_OAL;

    //  KITLSetDebug(
    //      ZONE_WARNING    |  // 0x0001   // Warnings
    //      ZONE_INIT       |  // 0x0002   // Init messages, including register
    //      ZONE_FRAMEDUMP  |  // 0x0004   // Dump of each Tx/Rx frame, including time stamp
    //      ZONE_TIMER      |  // 0x0008   // Timer insertions/removals
    //      ZONE_SEND       |  // 0x0010   // Tx state info
    //      ZONE_RECV       |  // 0x0020   // Rx state info
    //      ZONE_RETRANSMIT |  // 0x0040   // Retransmissions
    //      ZONE_CMD        |  // 0x0080   // cesh commands
    //      ZONE_INTR       |  // 0x0100   // Interrupts
    //      ZONE_ADAPTER    |  // 0x0200   // Must be combined with INTR,SEND,or RECV
    //      ZONE_LEDS       |  // 0x0400   // For writing to alpha LEDs (useful for measuring timings)
    //      ZONE_DHCP       |  // 0x0800   //
    //      ZONE_KITL_OAL   |  // 0x1000   //
    //      ZONE_KITL_ETHER |  // 0x2000   //
    //      ZONE_ERROR      |  // 0x8000   // Error
    //      KITL_ZONE_NOSERIAL |       // if not set, allow blocked messages to go over serial port
    //      0);

    if(!ClockAlloc())
    {
        OALMSG(OAL_ERROR, (L"ERROR:(OEMKitlStartup)ClockAlloc failed!\r\n"));
        goto cleanUp;
    }

    if (!ApbhDmaAlloc())
    {
        OALMSG(OAL_ERROR, (L"ERROR:(OEMKitlStartup)ApbhDmaAlloc failed!\r\n"));
        goto cleanUp;
    }
    if (!IomuxAlloc())
    {
        OALMSG(OAL_ERROR, (L"ERROR:(OEMKitlStartup)IomuxAlloc failed!\r\n"));
        goto cleanUp;
    }
    if (!ApbhDmaInit())
    {
        OALMSG(OAL_ERROR, (L"ERROR:(OEMKitlStartup)ApbhDmaInit failed!\r\n"));
        goto cleanUp;
    }

    KITL_RETAILMSG(ZONE_KITL_OAL, ("+OEMKitlStartup\r\n"));

    // Map RTC
    // MAP the Hardware registers
    if(pv_HWregRTC == NULL)
    {
        pv_HWregRTC = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);
    }
    
    if (!pv_HWregRTC)
    {
        OALMSG(OAL_ERROR, (L"ERROR: InitRTC: pv_HWregRTC null pointer!\r\n"));
        goto cleanUp;
    }

    // Look for bootargs left by the bootloader or left over from an earlier
    // boot.
    //
    pKITLArgs   = (OAL_KITL_ARGS*) OALArgsQuery(OAL_ARGS_QUERY_KITL);
    pszDeviceId = (CHAR*) OALArgsQuery(OAL_ARGS_QUERY_DEVID);

    // If no KITL arguments were found (typically provided by the bootloader), then select
    // some default settings.
    //
    if (pKITLArgs == NULL)
    {
        KITL_RETAILMSG(ZONE_WARNING, ("WARN: Boot arguments not found, "
                                      "use defaults\r\n"));
        memset(&KITLArgs, 0, sizeof(KITLArgs));

        // By default, disnable: KITL, DHCP, and VMINI...
        KITLArgs.flags  = ((~OAL_KITL_FLAGS_ENABLED) & OAL_KITL_FLAGS_VMINI | OAL_KITL_FLAGS_DHCP);

        // Use built-in USB controller for KITL.
        pKitlDevice = &g_kitlDevices[OAL_KITL_ETH_INDEX];
        KITLArgs.devLoc.IfcType     = Internal;
        KITLArgs.devLoc.BusNumber   = 0;
        KITLArgs.devLoc.PhysicalLoc = (PVOID)(CSP_BASE_REG_PA_USBCTRL0);
        KITLArgs.devLoc.LogicalLoc  = (DWORD)KITLArgs.devLoc.PhysicalLoc;
        pKITLArgs = &KITLArgs;
    }

    // If there isn't a device id from bootloader then create one.
    //
    if (pszDeviceId == NULL)
    {
        OALKitlCreateName(BSP_DEVICE_PREFIX, pKITLArgs->mac, buffer);
        pszDeviceId = buffer;
    }
    for (numKitlDevice = 0;
         (numKitlDevice < (sizeof(g_kitlDevices)/sizeof(g_kitlDevices[0]))) &&
         (g_kitlDevices[numKitlDevice].name != NULL);
         numKitlDevice++)
    {
        if (((UINT32)pKITLArgs->devLoc.PhysicalLoc) == g_kitlDevices[numKitlDevice].id)
        {
            pKitlDevice = &g_kitlDevices[numKitlDevice];
            break;
        }
    }
    if (pKitlDevice == NULL)
        pKitlDevice = &g_kitlDevices[OAL_KITL_ETH_INDEX];

    // Can now enable KITL.
    //
    rc = OALKitlInit(pszDeviceId, pKITLArgs, pKitlDevice);

cleanUp:
    KITL_RETAILMSG(ZONE_KITL_OAL, ("-OEMKitlStartup(rc = %d)\r\n", rc));
    return(rc);
}
