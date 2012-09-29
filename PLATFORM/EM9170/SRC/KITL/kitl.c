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
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
#include <debugserial.h>
#include <usbkitl.h>


//------------------------------------------------------------------------------
// External Functions
extern UINT32 OEMGetTickCount();
extern void EthMapIO(void);

//------------------------------------------------------------------------------
// External Variables 
extern DWORD g_dwKITLThreadPriority;


//-----------------------------------------------------------------------------
// Defines
#define USE_DHCP_RENEW              1

#define KITL_THREAD_HIGH_PRIORITY   131 // Default KITL thread priority is 131.


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables 


//------------------------------------------------------------------------------
// Local Variables 
static _OALKITLSharedDataStruct *g_pOALKITLSharedData;
static PCSP_EPIT_REG g_pEPIT;
static UINT32 g_nCountsPerMicrosec;

static OAL_KITL_ETH_DRIVER g_kitlEthLan911x = 
{
    LAN911xInit,
    NULL,
    NULL,
    LAN911xSendFrame,
    LAN911xGetFrame,
    LAN911xEnableInts,
    LAN911xDisableInts,
    NULL,
    NULL,
    LAN911xCurrentPacketFilter,
    LAN911xMulticastList
};

static OAL_KITL_ETH_DRIVER g_kitlUsbRndis = OAL_ETHDRV_RNDIS;
static OAL_KITL_SERIAL_DRIVER g_kitlUsbSerial = USB_SERIAL_KITL ;

static OAL_KITL_ETH_DRIVER g_kitlEthFEC = 
{
    FECInit,
    FECInitDMABuffer,
    NULL,
    FECSendFrame,
    FECGetFrame,
    FECEnableInts,
    FECDisableInts,
    NULL,
    NULL,
    FECCurrentPacketFilter,
    FECMulticastList
};

static OAL_KITL_SERIAL_DRIVER g_kitlCspSerial = 
{
    SerialInit,
    SerialDeinit,
    SerialSend,
    SerialSendComplete,
    SerialRecv,
    SerialEnableInts,
    SerialDisableInts,
    NULL,
    NULL,
    NULL
};

static OAL_KITL_DEVICE g_kitlDevices[] = 
{
    {
        L"LAN911x", Internal, BSP_BASE_REG_PA_LAN911x_IOBASE, 0,
        OAL_KITL_TYPE_ETH, &g_kitlEthLan911x
    },
    {
        L"USB_RNDIS ", Internal, CSP_BASE_REG_PA_USB, 0,
        OAL_KITL_TYPE_ETH, &g_kitlUsbRndis
    },
    {
        L"FEC", Internal, CSP_BASE_REG_PA_FEC, 0,
        OAL_KITL_TYPE_ETH, &g_kitlEthFEC
    },
    {
        L"CSPSERIAL", Internal, BSP_BASE_REG_PA_SERIALKITL, 0,
        OAL_KITL_TYPE_SERIAL, &g_kitlCspSerial
    },
    {
        L"USB_SERIAL",Internal, CSP_BASE_REG_PA_USB+1, 0,
        OAL_KITL_TYPE_SERIAL, &g_kitlUsbSerial
    },
    {
        NULL, 0, 0, 0, 0, NULL
    }
};


//------------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
//
//  Function: OALTimerGetClkSrc
//
//  This function returns the clock source setting used to program the EPIT_CR
//  CLKSRC bits.
//
//  Parameters:
//      None.
//
//  Returns:
//      EPIT clock source selection.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetClkSrc(void)
{
#if (BSP_EPIT_CLKSRC == EPIT_CR_CLKSRC_CKIL)

    return EPIT_CR_CLKSRC_CKIL;

#elif (BSP_EPIT_CLKSRC == EPIT_CR_CLKSRC_HIGHFREQ)
    
    return EPIT_CR_CLKSRC_HIGHFREQ;

#elif (BSP_EPIT_CLKSRC == EPIT_CR_CLKSRC_IPGCLK)

    return EPIT_CR_CLKSRC_IPGCLK;

#endif
    
}

//-----------------------------------------------------------------------------
//
//  Function: OALTimerGetClkFreq
//
//  This function returns the frequency of the EPIT input clock.
//
//  Parameters:
//      None.
//
//  Returns:
//      EPIT input clock.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetClkFreq(void)
{
    BSP_ARGS *pBspArgs = (BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START;
    switch(OALTimerGetClkSrc())
    {
    case EPIT_CR_CLKSRC_CKIL:
        return BSP_CLK_CKIL_FREQ;
    case EPIT_CR_CLKSRC_IPGCLK:
        return pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_EPIT];
    case EPIT_CR_CLKSRC_HIGHFREQ:
        return pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
    default:
        ERRORMSG(OAL_ERROR,(TEXT("Invalid Timer source\r\n")));
        DEBUGCHK(0);
        return 0;
    }
}


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
    BOOL rc;
    OAL_KITL_ARGS *pArgs, args;
    CHAR *szDeviceId, buffer[OAL_KITL_ID_SIZE];
    OAL_KITL_DEVICE *pKitlDevice = NULL;
    UINT32 *kitlFlags;
    UINT32 numKitlDevice = 0;

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
    // dpCurSettings.ulZoneMask = (ZONE_WARNING | ZONE_ERROR | 
    //     ZONE_INIT | ZONE_KITL_OAL | ZONE_KITL_ETHER);
    // dpCurSettings.ulZoneMask = 0x8003 | ZONE_KITL_OAL;

    //KITL_RETAILMSG(ZONE_KITL_OAL, ("+OEMKitlStartup\r\n"));
	// CS&ZHL JUN-1-2011:debug
    KITL_RETAILMSG(1, ("+OEMKitlStartup\r\n"));

    // Get _OALKITLSharedDataStruct initialized in OEMInit
    g_pOALKITLSharedData = (_OALKITLSharedDataStruct *)(g_pOemGlobal->pKitlInfo);

    // Map EPIT and calculate g_nCountsPerMsec for the references 
    // in function OALGetTickCount()
    g_pEPIT = (PCSP_EPIT_REG) OALPAtoUA(CSP_BASE_REG_PA_EPIT1);
    if (g_pEPIT == NULL)
    {
        KITL_RETAILMSG(ZONE_ERROR, ("+OEMKitlStartup: EPIT null pointer!\r\n"));
        rc = FALSE;
        goto cleanUp;
    }
    g_nCountsPerMicrosec = (OALTimerGetClkFreq() / 
        ((BSP_EPIT_PRESCALAR + 1) * 1000 * 1000));

    // Look for bootargs left by the bootloader or left over from an earlier
    // boot.
    //
    pArgs = (OAL_KITL_ARGS*)OALArgsQuery(OAL_ARGS_QUERY_KITL);
    szDeviceId = (CHAR *)OALArgsQuery(OAL_ARGS_QUERY_DEVID);
    kitlFlags = (UINT *) OALArgsQuery(BSP_ARGS_QUERY_KITL_FLAGS);   // Always valid

    // If we don't get kitl arguments use default
    if (pArgs == NULL)
    {
        KITL_RETAILMSG(ZONE_WARNING, ("WARN: Boot arguments not found, "
                                      "use defaults\r\n"));
        memset(&args, 0, sizeof(args));
        args.flags = *kitlFlags;

        pKitlDevice = &g_kitlDevices[OAL_KITL_ETH_INDEX];
        args.flags  |= OAL_KITL_FLAGS_ENABLED | OAL_KITL_FLAGS_VMINI | OAL_KITL_FLAGS_DHCP;
        args.mac[0] = (BSP_ARGS_DEFAULT_MAC_BYTE0 | (BSP_ARGS_DEFAULT_MAC_BYTE1 << 8));
        args.mac[1] = (BSP_ARGS_DEFAULT_MAC_BYTE2 | (BSP_ARGS_DEFAULT_MAC_BYTE3 << 8)) ;
        args.mac[2] = (BSP_ARGS_DEFAULT_MAC_BYTE4 | (BSP_ARGS_DEFAULT_MAC_BYTE5 << 8));
        args.devLoc.LogicalLoc = BSP_BASE_REG_PA_LAN911x_IOBASE;
        args.devLoc.IfcType     = Internal;
        pArgs = &args;
    }

#ifndef USE_DHCP_RENEW
    else if (pArgs->flags & OAL_KITL_FLAGS_DHCP)
    {
        // Reset IP address to force DHCP request instead of renew which
        // is acknowleged much sooner from the DHCP server
        pArgs->ipAddress = 0;
    }
#endif

    // If there isn't a device id from bootloader then create one.
    //
    if (szDeviceId == NULL)
    {
        OALKitlCreateName(BSP_DEVICE_PREFIX, args.mac, buffer);
        szDeviceId = buffer;
    }

    for (numKitlDevice = 0;
         (numKitlDevice < (sizeof(g_kitlDevices)/sizeof(g_kitlDevices[0]))) &&
         (g_kitlDevices[numKitlDevice].name != NULL);
         numKitlDevice++)
    {
        if (((UINT32)pArgs->devLoc.PhysicalLoc) == g_kitlDevices[numKitlDevice].id)
        {
            pKitlDevice = &g_kitlDevices[numKitlDevice];
            break;
        }
    }

    if (pKitlDevice == NULL)
        pKitlDevice = &g_kitlDevices[OAL_KITL_ETH_INDEX];

    // Check to see if we need to increase the KITL thread priority in order
    // to ensure reliable operation.
    if (g_dwKITLThreadPriority != KITL_THREAD_HIGH_PRIORITY)
    {
        KITL_RETAILMSG(ZONE_KITL_OAL, ("KITL: Increasing KITL thread priority "
                                       "from %d to %d\r\n",
                                       g_dwKITLThreadPriority,
                                       KITL_THREAD_HIGH_PRIORITY));
        g_dwKITLThreadPriority = KITL_THREAD_HIGH_PRIORITY;
    }

#ifdef		IMX257PDK_CPLD
    // CPLD has already been initialized during OEMInit, but we need
    // to map the CPLD I/O space since KITL runs in a separate DLL
    rc = CPLDMap();
    if (rc == FALSE)
    {
        KITL_RETAILMSG(ZONE_ERROR, ("ERROR:  CPLDMap failed in OEMKitlStartup.\r\n"));
        goto cleanUp;
    }

	// Override Ethernet controller I/O functions since MX25 3DS is 
    // connected to CPLD via SPI
    EthMapIO();
        
#endif	//IMX257PDK_CPLD

    // Can now enable KITL.
    //
    rc = OALKitlInit(szDeviceId, pArgs, pKitlDevice);			// CS&ZHL JUN-2-2011: this is provided by MS

cleanUp:
    KITL_RETAILMSG(ZONE_KITL_OAL, ("-OEMKitlStartup(rc = %d)\r\n", rc));
    return(rc);
}


//-----------------------------------------------------------------------------
//
// Function: OALSerialInit
//
// Initializes the internal UART with the specified communication settings.
//
// Parameters:
//      pSerInfo
//          [in] The pointer of SERIAL_INFO.
//
// Returns:
//      If this function succeeds, it returns TRUE, otherwise
//      it returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL OALSerialInit(PSERIAL_INFO pSerInfo)
{
    BOOL rc = FALSE;

    if (g_pOALKITLSharedData->g_pIOMUX)
    {
        rc = (OALConfigSerialUART(pSerInfo) &&
              OALConfigSerialIOMUX(pSerInfo->uartBaseAddr,
                                   g_pOALKITLSharedData->g_pIOMUX));
    }

    return rc;
}


//------------------------------------------------------------------------------
//
// Function:  OEMKitlSerialInit
//
// This function is called by SerialInit to initialize Serial KITL interface.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void OEMKitlSerialInit (void)
{
    SERIAL_INFO serInfo;

    serInfo.baudRate      = BSP_UART_KITL_SERIAL_BAUD;
    serInfo.uartBaseAddr  = BSP_BASE_REG_PA_SERIALKITL;
    serInfo.dataBits      = UART_UCR2_WS_8BIT;
    serInfo.parity        = UART_UCR2_PROE_EVEN;
    serInfo.stopBits      = UART_UCR2_STPB_1STOP;
    serInfo.bParityEnable = FALSE;
    serInfo.flowControl   = FALSE;

    OALSerialInit(&serInfo);
}


//------------------------------------------------------------------------------
//
// Function: OALGetTickCount
//
// This function is called by some KITL libraries to obtain relative time
// since timer runs. It is mostly used to implement timeout in network
// protocol.
//
// Parameters:
//      None.
//
// Returns:
//      The milliseconds since timer runs.
//
//------------------------------------------------------------------------------

UINT32 OALGetTickCount(void)
{
    UINT32 counts;

    // Calculate EPIT ticks since we started the system
    counts = EPIT_CNT_COUNT_MAX - INREG32(&g_pEPIT->CNT);

    return counts / (g_nCountsPerMicrosec * 1000);
}


//-----------------------------------------------------------------------------
//
//  Function: OALClockSetGatingMode
//
//  This function provides the OAL a safe mechanism for setting the clock 
//  gating mode of peripherals.
//
//  Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//      mode
//           [in] Requested clock gating mode for the peripheral.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
{
    BOOL fEnable;
    PCSP_CRM_REGS pCRM = g_pOALKITLSharedData->g_pCRM;

    if (pCRM)
    {
        // CGR is a shared register so we must disable interrupts temporarily
        // for safe access
        fEnable = INTERRUPTS_ENABLE(FALSE);

        // Update the clock gating mode
        INSREG32(&pCRM->CGR_REGS.CGR[CRM_CGR_INDEX(index)], CRM_CGR_MASK(index), 
            CRM_CGR_VAL(index, mode));

        INTERRUPTS_ENABLE(fEnable);
    }
}

