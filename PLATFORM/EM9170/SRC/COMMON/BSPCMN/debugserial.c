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
//  File:  debug.c
//
//  This module is provides the interface to the serial port.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#include <nkintr.h>
#pragma warning(pop)

#include "bsp.h"
#include "debugserial.h"
#include "serialutils.h"

//------------------------------------------------------------------------------
// External Functions
extern BOOL OALBspArgsInit(BSP_ARGS *pBSPArgs);
extern VOID OALBspArgsPrint(BSP_ARGS * pBSPArgs);


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Externs

extern DWORD initialOALLogZones;
extern BSP_ARGS *g_pBSPArgs;

//------------------------------------------------------------------------------
// Global Variables

// Create global variables for the pointers to the CCM, IOMUX, and PBC control
// registers so that all of the existing OAL, PQOAL, and COMMON code will
// still work as before.
//
// These global variables are just copies of what is in the g_OALKITLSharedData
// structure which will be used to exchange data between KITL and the OAL.

PCSP_CRM_REGS   g_pCRM;
PCSP_IOMUX_REGS g_pIOMUX;

UINT16 g_PBCversion;
UINT32 g_oalPerClkFreq;
PCSP_UART_REG g_pUART;  // Also used by the OAL timer and IOCTL functions.


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// FUNCTION:    OALSerialInit
//
// DESCRIPTION:
//      Initializes the interal UART with the specified communication settings.
//
//      Note that KITL has it's own version of this function since it cannot
//      directly access the g_pIOMUX and g_pPBC global variables that are
//      provided by the OAL.
//
// PARAMETERS:
//      pSerInfo
//          [in]   Serial port configuration settings.
//
// RETURNS:
//      If this function succeeds, it returns TRUE, otherwise
//      it returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL OALSerialInit(PSERIAL_INFO pSerInfo)
{
#if (DEBUG_PORT == DBG_UART1)
    return (OALConfigSerialUART(pSerInfo) &&
            OALConfigSerialIOMUX (pSerInfo->uartBaseAddr, g_pIOMUX));
#else
    UNREFERENCED_PARAMETER(pSerInfo);
    return FALSE;
#endif
}


//------------------------------------------------------------------------------
//
//  Function: OEMInitDebugSerial
//
//  Initializes the debug serial port.
//
//------------------------------------------------------------------------------
VOID OEMInitDebugSerial()
{
#if (DEBUG_PORT == DBG_UART1)
    SERIAL_INFO serInfo;
#endif
    
    g_pBSPArgs = (BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START;    
    g_pUART = NULL;

    //Initialize all global variables upfront here.
    g_pCRM = (PCSP_CRM_REGS) OALPAtoUA(CSP_BASE_REG_PA_CRM);
    if (g_pCRM == NULL)
    {
        return;
    }

    // Initialize BSP_ARGS to get early clocking info
    OALBspArgsInit(g_pBSPArgs);

    g_pIOMUX = (PCSP_IOMUX_REGS) OALPAtoUA(CSP_BASE_REG_PA_IOMUXC);
    if (g_pIOMUX == NULL)
    {
        return;
    }

#if (DEBUG_PORT == DBG_UART1)
    serInfo.baudRate      = 115200;
    serInfo.dataBits      = UART_UCR2_WS_8BIT;
    serInfo.parity        = UART_UCR2_PROE_EVEN;
    serInfo.stopBits      = UART_UCR2_STPB_1STOP;
    serInfo.bParityEnable = FALSE;
    serInfo.flowControl   = FALSE;


    g_pUART = (PCSP_UART_REG) OALPAtoUA(CSP_BASE_REG_PA_UART1);
    if (g_pUART == NULL)
    {
        return;
    }
    serInfo.uartBaseAddr = CSP_BASE_REG_PA_UART1;
    
    OALSerialInit(&serInfo);

    // Configure the default log settings.
    OALLogSetZones(initialOALLogZones);

    // Serial debug support is now active.  Print BSP_ARGS info.
    OALBspArgsPrint(g_pBSPArgs);
#endif
}


//------------------------------------------------------------------------------
//
//  Function: OEMWriteDebugByte
//
//  Transmits a character out the debug serial port.
//
VOID OEMWriteDebugByte(UINT8 ch)
{
#if (DEBUG_PORT == DBG_UART1)
    if (g_pUART == NULL)
        return;

    // Wait until there is room in the FIFO
    while(INREG32(&g_pUART->UTS) & CSP_BITFMASK(UART_UTS_TXFULL))
        ; // Intentional polling loop.

    // Send the character
    OUTREG32(&g_pUART->UTXD, ch);
#else
    UNREFERENCED_PARAMETER(ch);
#endif
}


//------------------------------------------------------------------------------
//
//  Function: OEMReadDebugByte
//
//  Reads a byte from the debug serial port. Does not wait for a character.
//  If a character is not available function returns "OEM_DEBUG_READ_NODATA"
//
int OEMReadDebugByte()
{
    int retVal = OEM_DEBUG_READ_NODATA;

#if (DEBUG_PORT == DBG_UART1)
    if (INREG32(&g_pUART->USR2) & CSP_BITFMASK(UART_USR2_RDR))
    {
        retVal = INREG32(&g_pUART->URXD) & 0xFF;
    }
#endif

    return retVal;
}


/*
    @func   void | OEMWriteDebugLED | Writes specified pattern to debug LEDs 1-4.
    @rdesc  None.
    @comm
    @xref
*/
void OEMWriteDebugLED(UINT16 Index, DWORD Pattern)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Index);
    UNREFERENCED_PARAMETER(Pattern);

    // There is currently no support for any debug-specific LEDs so this
    // function is just a no-op.
}
