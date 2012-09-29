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
// File: ether.c
//
// EBOOT ethernet routines
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "loader.h"
#include "kitl_cfg.h"


//-----------------------------------------------------------------------------
// External Variables
extern BSP_ARGS *g_pBSPArgs;
extern BOOL RndisInit( BYTE *pbBaseAddress, DWORD dwMultiplier, USHORT MacAddr[3]);
extern BOOT_CFG    g_BootCFG;

extern BOOL USBSerialInit();
//-----------------------------------------------------------------------------
// External Functions

extern void EthMapIO(void);

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables

// Function pointers to the support library functions of the currently 
// installed debug ethernet controller.
PFN_EDBG_INIT               pfnEDbgInit;
PFN_EDBG_GET_FRAME          pfnEDbgGetFrame;
PFN_EDBG_SEND_FRAME         pfnEDbgSendFrame;
PFN_EDBG_ENABLE_INTS        pfnEDbgEnableInts;
PFN_EDBG_DISABLE_INTS       pfnEDbgDisableInts;
PFN_EDBG_GET_PENDING_INTS   pfnEDbgGetPendingInts;
PFN_EDBG_READ_EEPROM        pfnEDbgReadEEPROM;
PFN_EDBG_WRITE_EEPROM       pfnEDbgWriteEEPROM;
PFN_EDBG_SET_OPTIONS        pfnEDbgSetOptions;
PFN_EDBG_INIT_DMABUFFER     pfnEDbgInitDMABuffer;


//-----------------------------------------------------------------------------
// Local function

static int InitUSBRNDISDevice(OAL_KITL_ARGS *pKITLArgs)
{
    KITLOutputDebugString("INFO: Trying to initialize USB RNDIS...\r\n"); 

    // same as ethernet
    memcpy( pKITLArgs->mac, g_BootCFG.mac, 6);
    pKITLArgs->mac[0] |= 2;

    KITLOutputDebugString("INFO: MAC address: %x-%x-%x-%x-%x-%x\r\n",
        g_pBSPArgs->kitl.mac[0] & 0x00FF, g_pBSPArgs->kitl.mac[0] >> 8,
        g_pBSPArgs->kitl.mac[1] & 0x00FF, g_pBSPArgs->kitl.mac[1] >> 8,
        g_pBSPArgs->kitl.mac[2] & 0x00FF, g_pBSPArgs->kitl.mac[2] >> 8);

    // init USB registers
    if (HostMiniInit((BYTE *) NULL, 1, pKITLArgs->mac))
    {
        pfnEDbgInit           = RndisInit;
        pfnEDbgEnableInts     = RndisEnableInts;
        pfnEDbgDisableInts    = RndisDisableInts;    
        pfnEDbgGetFrame       = RndisEDbgGetFrame;       
        pfnEDbgSendFrame      = RndisEDbgSendFrame;      
        pfnEDbgReadEEPROM     = NULL;     
        pfnEDbgWriteEEPROM    = NULL;    

        // TODO: These two functons are provided in \oak\drivers\ethdbg\rne_mdd but not in \oak\drivers\netcard\rndismini

        pfnEDbgGetPendingInts = RndisGetPendingInts;  // TODO: This is called in OEMEthISR - what to do
        pfnEDbgSetOptions     = RndisSetOptions;   // TODO: This function does nothing

#ifdef IMGSHAREETH
        pfnCurrentPacketFilter = RndisCurrentPacketFilter;
        pfnMulticastList       = RndisMulticastList;
#endif

        pKITLArgs->flags |= OAL_KITL_FLAGS_POLL;

        // Save the device location information for later use.
        pKITLArgs->devLoc.IfcType     = Internal;
        pKITLArgs->devLoc.BusNumber   = 0;
        pKITLArgs->devLoc.PhysicalLoc = (PVOID)CSP_BASE_REG_PA_USB;    // not a real Ethernet card
        pKITLArgs->devLoc.LogicalLoc  = (DWORD)pKITLArgs->devLoc.PhysicalLoc;

        return 0;
        //return (EDBG_USB_RNDIS);
    }
    else
    {
        KITLOutputDebugString("ERROR: Failed to initialize ARC Rndis USB Ethernet controller.\n");
        return -1;
    }
}


//-----------------------------------------------------------------------------
//
// Function: InitSpecifiedEthDevice
//
// Initializes the specified Ethernet device to be used for download and 
// KITL services.
//
// Parameters:
//      pKITLArgs
//          [in/out] Points to the KITL argument structure.
//
//      EthDevice
//          [in] Ethernet device to be initialized.
//
// Returns:
//      Returns Ethernet adapter type intialized on success, otherwise 
//      returns -1.
//
//-----------------------------------------------------------------------------
UINT32 InitSpecifiedEthDevice(OAL_KITL_ARGS *pKITLArgs, UINT32 EthDevice)
{
    UINT32 rc = (UINT32)-1;
    g_SerialUSBDownload = FALSE;
    KITLOutputDebugString("+InitSpecifiedEthDevice \r\n");
    
    switch(EthDevice)
    {
    case ETH_DEVICE_LAN911x:
        // Override Ethernet controller I/O functions since MX25 3DS is 
        // connected to CPLD via SPI
        EthMapIO();

        if (LAN911xInit((PBYTE) OALPAtoUA(BSP_BASE_REG_PA_LAN911x_IOBASE), 0, pKITLArgs->mac))
        {
            pfnEDbgInit         = (PFN_EDBG_INIT) LAN911xInit;
            pfnEDbgGetFrame     = (PFN_EDBG_GET_FRAME) LAN911xGetFrame;
            pfnEDbgSendFrame    = (PFN_EDBG_SEND_FRAME) LAN911xSendFrame;
            pfnEDbgEnableInts   = (PFN_EDBG_ENABLE_INTS) LAN911xEnableInts;     
            pfnEDbgDisableInts  = (PFN_EDBG_DISABLE_INTS)LAN911xDisableInts;    
            
            // Save the device location information for later use.
            //
            pKITLArgs->devLoc.IfcType       = Internal;
            pKITLArgs->devLoc.BusNumber     = 0;
            pKITLArgs->devLoc.PhysicalLoc   = (PVOID)(BSP_BASE_REG_PA_LAN911x_IOBASE);
            pKITLArgs->devLoc.LogicalLoc    = (DWORD)pKITLArgs->devLoc.PhysicalLoc;

            KITLOutputDebugString("INFO: LAN911x Ethernet controller initialized.\r\n");
            rc = EDBG_ADAPTER_OEM;
        }
        else
        {
            KITLOutputDebugString("ERROR: Failed to initialize LAN911x Ethernet controller.\n");
        }
        break;

    case ETH_DEVICE_FEC:
        // Initialize DMA buffer for FEC
        if (!FECInitDMABuffer((UINT32) OALPAtoUA(IMAGE_SHARE_FEC_RAM_PA_START), IMAGE_SHARE_FEC_RAM_SIZE))
        {
            KITLOutputDebugString("ERROR: Failed to initialize Fast Ethernet DMA buffer.\n");
            break;
        }

        // Initialize FEC module
        if (FECInit((PBYTE) OALPAtoUA(CSP_BASE_REG_PA_FEC), 0, pKITLArgs->mac))
        {
            pfnEDbgInit             = (PFN_EDBG_INIT) FECInit;
            pfnEDbgGetFrame         = (PFN_EDBG_GET_FRAME) FECGetFrame;
            pfnEDbgSendFrame        = (PFN_EDBG_SEND_FRAME) FECSendFrame;
            pfnEDbgEnableInts       = (PFN_EDBG_ENABLE_INTS) FECEnableInts;
            pfnEDbgDisableInts      = (PFN_EDBG_DISABLE_INTS) FECDisableInts;
            pfnEDbgInitDMABuffer    = (PFN_EDBG_INIT_DMABUFFER) FECInitDMABuffer;
            
            // Save the device location information for later use.
            //
            pKITLArgs->devLoc.IfcType     = Internal;
            pKITLArgs->devLoc.BusNumber   = 0;
            pKITLArgs->devLoc.PhysicalLoc = (PVOID)(CSP_BASE_REG_PA_FEC);
            pKITLArgs->devLoc.LogicalLoc  = (DWORD)pKITLArgs->devLoc.PhysicalLoc;

            KITLOutputDebugString("INFO: Fast Ethernet controller initialized.\r\n");
            rc = EDBG_ADAPTER_OEM;
        }
        else
        {
            KITLOutputDebugString("ERROR: Failed to initialize Fast Ethernet controller.\n");
        }
        break;

    case ETH_DEVICE_USB:
        if( InitUSBRNDISDevice( pKITLArgs) != -1)
        {
            rc = EDBG_ADAPTER_OEM;
        }
        else
        {
            KITLOutputDebugString("ERROR: Failed to initialize USB RNDIS Ethernet controller.\n");
        }
        break;

    case SER_DEVICE_USB:
        // Indicate to OEMPreDownload that serial will be used
       
        if(!USBSerialInit())
        {
            KITLOutputDebugString("ERROR: Failed to detect and initialize USB Function controller.\r\n");
            break;
            
        }
        g_SerialUSBDownload = TRUE;
        rc = SDBG_USB_SERIAL;
         // same as ethernet
        memcpy( pKITLArgs->mac, g_BootCFG.mac, 6);
        pKITLArgs->mac[0] |= 2;

        // Save the device location information for later use.
        pKITLArgs->devLoc.IfcType     = Internal;
        pKITLArgs->devLoc.BusNumber   = 0;
        pKITLArgs->devLoc.PhysicalLoc = (PVOID)(CSP_BASE_REG_PA_USB+0x1);    // Just has to be different than RNDIS
        pKITLArgs->devLoc.LogicalLoc  = (DWORD)pKITLArgs->devLoc.PhysicalLoc;

        break;

    default:
        KITLOutputDebugString("ERROR: Failed to initialize unknown Ethernet controller. EthDevice = %d\n",EthDevice);
        break;
    }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALKitlInitEthIrq
//
//  This function is called by OAL_KITLETH_INIT function to configure
//  the IRQ line for the Ethernet controller.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL OALKitlInitEthIrq(void)
{
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  OALKitlClearEthIrq
//
//  This function is called by OAL_KITLETH_GET_FRAME function to clear
//  the IRQ from the Ethernet controller.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//------------------------------------------------------------------------------
void OALKitlClearEthIrq(void)
{
}


//------------------------------------------------------------------------------
//
//  Function:  OALKitlSendEthSelfReset
//
//  This function is called by OAL_KITLETH_INIT function to determine if
//  the Ethernet self reset should be issued to the controller.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE if self reset should be issued, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL OALKitlSendEthSelfReset(void)
{
    return FALSE;
}
