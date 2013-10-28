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
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  serial.c
//
//   This file implements the device specific functions for zeus serial device.
//
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// INCLUDE FILES
//------------------------------------------------------------------------------

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <nkintr.h>
#include <serhw.h>
#include <memory.h>
#include <notify.h>
#include <devload.h>
#include <ceddk.h>
#include <windev.h>
#include <serdbg.h>
#include <uart.h>
#include <csp.h>
#include <serial.h>

//------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------
extern BOOL BSPUartGetType( ULONG HWAddr, uartType_c * pType );

//------------------------------------------------------------------------------
// MACRO DEFINITIONS
//------------------------------------------------------------------------------
#define BAUD_TABLE_SIZE 23

//------------------------------------------------------------------------------
// Local Variables
static const PAIRS BaudPairs[BAUD_TABLE_SIZE] = {
    {50,        2307},
    {75,        1538},
    {110,       1049},
    {135,       858},
    {150,       769},
    {300,       384},
    {600,       192},
    {1200,      96},
    {1800,      64},
    {2000,      58},
    {2400,      48},
    {3600,      32},
    {4800,      24},
    {7200,      16},
    {9600,      12},
    {12800,     9},
    {14400,     8},
    {19200,     6},
    {23040,     5},
    {28800,     4},
    {38400,     3},
    {57600,     2},
    {115200,    1}
};

static const LOOKUP_TBL BaudTable = {BAUD_TABLE_SIZE, (PAIRS *) BaudPairs};

//------------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function: Ser_InternalMapRegisterAddresses
//
//  This function retrieves the current
//  properties of the communications device.
//
// Parameters:
//      HWAddress
//          [in] physical address of the hardware.
//      Size
//          [in] how much larger the address window is.
//
// Returns:
//      This function returns the base virtual address
//      that maps the base physical address for the range.
//
//-----------------------------------------------------------------------------
PUCHAR static Ser_InternalMapRegisterAddresses( ULONG HWAddress, ULONG Size )
{
    PUCHAR ioPortBase;
    ULONG inIoSpace = 1;
    PHYSICAL_ADDRESS ioPhysicalBase = {HWAddress, 0};

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_InternalMapRegisterAddresses : HalTranslateBusAddress 0x%x\r\n"), HWAddress));
    if (HalTranslateBusAddress(Isa, 0, ioPhysicalBase, &inIoSpace, &ioPhysicalBase)) {
        if (!inIoSpace) {
            if ((ioPortBase = (PUCHAR)MmMapIoSpace(ioPhysicalBase, Size, FALSE)) == NULL) {
                DEBUGMSG(ZONE_ERROR, (TEXT("Error mapping I/O Ports\r\n")));
                return (NULL);
            }
        } else {
            ioPortBase = (PUCHAR)ioPhysicalBase.LowPart;
        }
    } else {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error translating I/O Ports.\r\n")));
        return (NULL);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_InternalMapRegisterAddresses : %d\r\n"), ioPortBase));

    return (ioPortBase);
}


//-----------------------------------------------------------------------------
//
// Function: Ser_GetRegistryData
//
//  This function takes the registry path provided
//  to COM_Init and uses it to find this requested comm
//  port's DeviceArrayIndex, the IOPort Base Address,
//  and the interrupt number.
//
// Parameters:
//      pSerHead
//          [in] pointer to PSER_INFO structure.
//      regKeyPath
//          [in] the registry path passed in to COM_Init.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL Ser_GetRegistryData( PSER_INFO pSerHead, LPCTSTR regKeyPath )
{
    LONG regError;
    HKEY hKey;
    DWORD dwDataSize;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_GetRegistryData+:Try to open %s\r\n"), regKeyPath));

    // We've been handed the name of a key in the registry that was generated
    // on the fly by device.exe.  We're going to open that key and pull from it
    // a value that is the name of this serial port's real key.  That key
    // will have the DeviceArrayIndex that we're trying to find.
    hKey = OpenDeviceKey(regKeyPath);
    if (hKey == NULL) {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failed to open device key\r\n")));
        return (FALSE);
    }

    // Okay, we're finally ready to try and load our registry data.
    dwDataSize = PC_REG_DEVINDEX_VAL_LEN;
    regError = RegQueryValueEx(
                              hKey,
                              PC_REG_DEVINDEX_VAL_NAME,
                              NULL,
                              NULL,
                              (LPBYTE)(&pSerHead->dwDevIndex),
                              &dwDataSize);

    if (regError == ERROR_SUCCESS) {
        dwDataSize = PC_REG_IOBASE_VAL_LEN;
        regError = RegQueryValueEx(
                                  hKey,
                                  PC_REG_IOBASE_VAL_NAME,
                                  NULL,
                                  NULL,
                                  (LPBYTE)(&pSerHead->dwIOBase),
                                  &dwDataSize);
    }

    if (regError == ERROR_SUCCESS) {
        dwDataSize = PC_REG_IOLEN_VAL_LEN;
        regError = RegQueryValueEx(
                                  hKey,
                                  PC_REG_IOLEN_VAL_NAME,
                                  NULL,
                                  NULL,
                                  (LPBYTE)(&pSerHead->dwIOLen),
                                  &dwDataSize);
    }

    RegCloseKey (hKey);

    if (regError != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failed to get serial registry values, Error 0x%X\r\n"), regError));
        return (FALSE);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_GetRegistryData-\r\n")));

    return (TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerInit
//
//  This routine sets information controlled by
//  the user such as Line control and baud rate.
//  It can also initialize events and interrupts,
//  thereby indirectly managing initializing hardware
//  buffers. Exported only to driver, called only
//  once per process.
//
// Parameters:
//      bIR
//          [in] Is IR mode used.
//      Identifier
//          [in] Device identifier.
//      pMDDContext
//          [in] First argument to mdd callbacks.
//      pHWObj
//          [in] Pointer to our own HW OBJ for this device.
//
// Returns:
//      The return value is a PVOID to be passed back
//      into the HW dependent layer when HW functions are
//      called.
//
//-----------------------------------------------------------------------------
static PVOID SerInit( BOOL bIR, ULONG Identifier, PVOID pMDDContext, PHWOBJ pHWObj )
{
    PSER_INFO pSerHead;
    BOOL InitError = FALSE;
    ULONG irq;
    DEVICE_LOCATION devLoc;
    uartType_c type;
    DWORD dwSources;
    DWORD dwTransferred;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerInit+\r\n")));

    // Allocate for our main data structure and one of it's fields.
    pSerHead = (PSER_INFO)LocalAlloc(LMEM_ZEROINIT|LMEM_FIXED, sizeof(SER_INFO));

    if (!pSerHead)
        return(NULL);

    if (!Ser_GetRegistryData(pSerHead, (LPCTSTR)Identifier)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("SerInit - Unable to read registry data!!\r\n")));
        InitError = TRUE;
    }

    if (!InitError){
        pSerHead->pBaseAddress = Ser_InternalMapRegisterAddresses(pSerHead->dwIOBase, pSerHead->dwIOLen);

        pSerHead->pHWObj = pHWObj;
        pSerHead->cOpenCount = 0;

        DEBUGMSG(ZONE_FUNCTION, (TEXT("SerInit \r\n")));

        if (!BSPUartGetType(pSerHead->dwIOBase, &type)) {
            DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: Failed to obtain uart info.\r\n")));
            return (NULL);
        }

        // Use kernel IOCTL to translate the UART base address into an IRQ since
        // the IRQ value differs based on the SoC. Note that DEVICE_LOCATION
        // fields except IfcType and LogicalLoc are ignored for internal SoC
        // components.
        devLoc.IfcType = Internal;
        devLoc.LogicalLoc = pSerHead->dwIOBase;

        if (!KernelIoControl(IOCTL_HAL_REQUEST_IRQ, &devLoc, sizeof(devLoc),
            &irq, sizeof(irq), NULL))
        {
            ERRORMSG(1, (_T("Cannot obtain UART IRQ!\r\n")));
            LocalFree(pSerHead);
            DEBUGMSG(ZONE_ERROR, (TEXT("SerInit - Initialization failed!!\r\n")));
            return (NULL);
        }

        // Use kernel IOCTL to translate the UART base address into an IRQ since
        // the IRQ value differs based on the SoC. Note that DEVICE_LOCATION
        // fields except IfcType and LogicalLoc are ignored for internal SoC
        // components.
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
        &pSerHead->pHWObj->dwIntID, sizeof(DWORD), NULL)) {
            DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: Failed to obtain sysintr value for UART interrupt.\r\n")));
            LocalFree(pSerHead);
            DEBUGMSG(ZONE_ERROR, (TEXT("SerInit - Initialization failed!!\r\n")));
            return (NULL);
        }

        if (!bIR) {
            dwSources = pSerHead->pHWObj->dwIntID;
            if(KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &dwSources, sizeof(DWORD),
                NULL, 0, &dwTransferred) == FALSE)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: IOCTL_HAL_ENABLE_WAKE failed\r\n")));
                LocalFree(pSerHead);
                DEBUGMSG(ZONE_ERROR, (TEXT("SerInit - Initialization failed!!\r\n")));
                return (NULL);
            }
        }
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SerInit - bIR=%d, dwIntID=%d, irq=%d\r\n"), bIR, pSerHead->pHWObj->dwIntID, irq));

        // Set up our Comm Properties data
        pSerHead->CommProp.wPacketLength = 0xffff;
        pSerHead->CommProp.wPacketVersion = 0xffff;
        pSerHead->CommProp.dwServiceMask = SP_SERIALCOMM;
        pSerHead->CommProp.dwReserved1 = 0;
        pSerHead->CommProp.dwMaxTxQueue = 32;
        pSerHead->CommProp.dwMaxRxQueue = 32;
        pSerHead->CommProp.dwMaxBaud = BAUD_115200;
        pSerHead->CommProp.dwProvSubType = PST_RS232;

        pSerHead->CommProp.dwProvCapabilities =
            PCF_RTSCTS |PCF_SETXCHAR | PCF_INTTIMEOUTS |
            PCF_PARITY_CHECK | PCF_SPECIALCHARS |
            PCF_TOTALTIMEOUTS | PCF_XONXOFF;

        pSerHead->CommProp.dwSettableBaud =
            BAUD_075 | BAUD_110 | BAUD_150 | BAUD_300 |
            BAUD_600 | BAUD_1200 | BAUD_1800 | BAUD_2400 |
            BAUD_4800 | BAUD_7200 | BAUD_9600 | BAUD_14400 |
            BAUD_19200 | BAUD_38400 | BAUD_56K |BAUD_115200 |
            BAUD_57600 | BAUD_USER;

        pSerHead->CommProp.dwSettableParams =
            SP_BAUD | SP_DATABITS | SP_HANDSHAKING | SP_PARITY |
            SP_PARITY_CHECK | SP_RLSD | SP_STOPBITS;

        pSerHead->CommProp.wSettableData = DATABITS_7 | DATABITS_8;

        pSerHead->CommProp.wSettableStopParity = STOPBITS_10 | STOPBITS_20 |
            PARITY_NONE | PARITY_ODD | PARITY_EVEN;

        pSerHead->fIRMode = bIR;
        SL_Init(bIR, type, pSerHead->dwIOBase, pSerHead->pBaseAddress,
            pSerHead, EvaluateEventFlag, pMDDContext, (PLOOKUP_TBL)&BaudTable);

        return (pSerHead);
    }
    else {
        if (pSerHead->pBaseAddress)
            MmUnmapIoSpace(pSerHead->pBaseAddress, pSerHead->dwIOLen);

        LocalFree(pSerHead);

        DEBUGMSG(ZONE_FUNCTION, (TEXT("SerInit - Initialization failed!!\r\n")));

        return (NULL);
    }
}


//-----------------------------------------------------------------------------
//
// Function: SerSerialInit
//
//  This function calls SerInit to initialize serial device.
//
// Parameters:
//      Identifier
//          [in] Device identifier.
//      pMDDContext
//          [in] First argument to mdd callbacks.
//      pHWObj
//          [in] Pointer to our own HW OBJ for this device.
//
// Returns:
//      The return value is a PVOID to be passed back
//      into the HW dependent layer when HW functions are
//      called.
//
//-----------------------------------------------------------------------------
static PVOID SerSerialInit( ULONG Identifier, PVOID pMDDContext, PHWOBJ pHWObj )
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SerSerialInit+\r\n")));
    return (SerInit(FALSE, Identifier, pMDDContext, pHWObj));
}


//-----------------------------------------------------------------------------
//
// Function: SerIRInit
//
//  This function calls SerInit to initialize serial IR device.
//
// Parameters:
//      Identifier
//          [in] Device identifier.
//      pMDDContext
//          [in] First argument to mdd callbacks.
//      pHWObj
//          [in] Pointer to our own HW OBJ for this device.
//
// Returns:
//      The return value is a PVOID to be passed back
//      into the HW dependent layer when HW functions are
//      called.
//
//-----------------------------------------------------------------------------
static PVOID SerIRInit( ULONG Identifier, PVOID pMDDContext, PHWOBJ pHWObj )
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SerIRInit+\r\n")));
    return (SerInit(TRUE, Identifier, pMDDContext, pHWObj));
}


//-----------------------------------------------------------------------------
//
// Function: SerPostInit
//
//  This function is called by the upper layer to
//  perform any necessary operations after initializing
//  all data structures and prepare the serial IST to
//  begin handling interrupts. This call occurs as the
//  last step of the COM_Init routine.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerPostInit( PVOID pContext )
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SerPostInit+\r\n")));

    // Since we are just a library which might get used for
    // builtin ports which init at boot, we can't do anything too fancy.
    // Lets just make sure we cancel any pending interrupts so
    // that if we are being used with an edge triggered PIC, he
    // will see an edge after the MDD hooks the interrupt.
    SL_ClearPendingInts( pContext );

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SerPostInit-\r\n")));

    return(TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerOpen
//
//  This function is called by the upper layer to
//  open the serial device. This function applies
//  power to the serial hardware.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerOpen( PVOID pContext )
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_OPEN,(TEXT("SerOpen+ \r\n")));

    // Disallow multiple simultaneous opens
    if (pSerHead->cOpenCount)
        return (FALSE);

    pSerHead->cOpenCount++;

    SL_Reset(pSerHead);
    SL_Open(pSerHead);
    SL_SetOutputMode(pSerHead, pSerHead->fIRMode, !pSerHead->fIRMode);

    DEBUGMSG(ZONE_OPEN,(TEXT("SerOpen-\r\n")));
    return (TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerClose
//
//  This function is called by the upper layer to
//   close the serial device.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      Zero indicates success.
//
//-----------------------------------------------------------------------------
static ULONG SerClose( PVOID pContext )
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;
    ULONG uTries;

    DEBUGMSG(ZONE_CLOSE,(TEXT("SerClose+ \r\n")));

    if (pSerHead->cOpenCount) {
        DEBUGMSG(ZONE_CLOSE, (TEXT("SerClose - closing device\r\n")));

        pSerHead->cOpenCount--;

        // while we are still transmitting, sleep.
        uTries = 0;
        while(!(pSerHead->uart_info.pUartReg->USR2 & 0x00004000)
              && (uTries++ < 100)){    // TxFifo not empty..
            DEBUGMSG(ZONE_WARN, (TEXT("SerClose, TX in progress.\r\n")));
            Sleep(10);
        }

        // When the device is closed, we power it down.
        SL_Close(pSerHead);
    }

    DEBUGMSG(ZONE_CLOSE,(TEXT("SerClose-\r\n")));
    return (0);
}


//-----------------------------------------------------------------------------
//
// Function: SerDeinit
//
//  This function is called by the upper layer to
//  de-initialize the serial device.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerDeinit( PVOID pContext )
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerDeinit+\r\n")));

    if (!pSerHead)
        return (FALSE);

    // Make sure device is closed before doing DeInit
    if (pSerHead->cOpenCount)
        SerClose( pContext );

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pSerHead->pHWObj->dwIntID, sizeof(DWORD), NULL, 0, NULL);
    pSerHead->pHWObj->dwIntID = SYSINTR_UNDEFINED;

    if (pSerHead->pBaseAddress)
        MmUnmapIoSpace(pSerHead->pBaseAddress, pSerHead->dwIOLen);

    SL_Deinit( pContext );

    // Free the HWObj
    LocalFree(pSerHead->pHWObj);

    // And now free the SER_INFO structure.
    LocalFree(pSerHead);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerDeinit-\r\n")));
    return (TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerPowerOff
//
//  This function is called by driver to turn off
//  power to serial port.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerPowerOff( PVOID pContext )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerPowerOff+\r\n")));

    // First, power down the UART
    SL_PowerOff(pContext);

    // And then disable our IR and 9 Pin interface
    SL_SetOutputMode(pContext, FALSE, FALSE);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerPowerOff-\r\n")));
    return (TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerPowerOn
//
//  This function is called by driver to turn on
//  power to serial port.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerPowerOn( PVOID pContext )
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerPowerOn+\r\n")));
    // First, power up the UART
    SL_PowerOn( pContext );

    // And then enable our IR interface (if needed)
    SL_SetOutputMode(pSerHead, pSerHead->fIRMode, !pSerHead->fIRMode);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerPowerOn-\r\n")));
    return (TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerEnableIR
//
//  This function enables the infrared (IR)
//  serial interface.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//      BaudRate
//          [in] Specifies the baud to set.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerEnableIR( PVOID pContext, ULONG BaudRate )
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_IR,(TEXT("SerEnableIR+\r\n")));

    pSerHead->fIRMode = TRUE;

    SL_SetOutputMode(pSerHead, pSerHead->fIRMode, !pSerHead->fIRMode);

    DEBUGMSG(ZONE_IR,(TEXT("SerEnableIR-\r\n")));
    return (TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerDisableIR
//
//  This function disables the infrared (IR)
//   serial interface.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerDisableIR( PVOID pContext )
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_IR,(TEXT("SerDisableIR+\r\n")));

    pSerHead->fIRMode = FALSE;

    SL_SetOutputMode(pSerHead, pSerHead->fIRMode, !pSerHead->fIRMode);

    DEBUGMSG(ZONE_IR,(TEXT("SerDisableIR-\r\n")));
    return (TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerGetCommProperties
//
//  This function retrieves the current properties
//  of the communications device.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//      pCommProp
//          [in] Pointer to a COMMPROP structure to
//                hold the communications property information.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID SerGetCommProperties( PVOID pContext, LPCOMMPROP pCommProp )
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SerGetCommProperties+\r\n")));

    *pCommProp = pSerHead->CommProp;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SerGetCommProperties-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SerIRClearDTR
//
//  This routine clears DTR.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SerIRClearDTR( PVOID   pContext )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerIRClearDTR+\r\n")));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SerIRSetDTR
//
//  This routine sets DTR.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SerIRSetDTR( PVOID pContext )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerIRSetDTR+\r\n")));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SerIRClearRTS
//
//  This routine clears RTS.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SerIRClearRTS( PVOID pContext )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerIRClearRTS+\r\n")));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SerIRSetRTS
//
//  This routine sets RTS.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned
//                by SerInit function that contains implementation-
//                specific data describing the hardware device.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SerIRSetRTS( PVOID pContext )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerIRSetRTS+\r\n")));
    return;
}


const HW_VTBL IoVTbl = {
    SerSerialInit,
    SerPostInit,
    SerDeinit,
    SerOpen,
    SerClose,
    SL_GetIntrType,
    SL_RxIntrHandler,
    SL_TxIntrHandler,
    SL_ModemIntrHandler,
    SL_LineIntrHandler,
    SL_GetRxBufferSize,
    SerPowerOff,
    SerPowerOn,
    SL_ClearDTR,
    SL_SetDTR,
    SL_ClearRTS,
    SL_SetRTS,
    SerEnableIR,
    SerDisableIR,
    SL_ClearBreak,
    SL_SetBreak,
    SL_XmitComChar,
    SL_GetStatus,
    SL_Reset,
    SL_GetModemStatus,
    SerGetCommProperties,
    SL_PurgeComm,
    SL_SetDCB,
    SL_SetCommTimeouts,
};

const HW_VTBL IrVTbl = {
    SerIRInit,
    SerPostInit,
    SerDeinit,
    SerOpen,
    SerClose,
    SL_GetIntrType,
    SL_RxIntrHandler,
    SL_TxIntrHandler,
    SL_ModemIntrHandler,
    SL_LineIntrHandler,
    SL_GetRxBufferSize,
    SerPowerOff,
    SerPowerOn,
    SL_ClearDTR,
    SL_SetDTR,
    SL_ClearRTS,
    SL_SetRTS,
    SerEnableIR,
    SerDisableIR,
    SL_ClearBreak,
    SL_SetBreak,
    SL_XmitComChar,
    SL_GetStatus,
    SL_Reset,
    SL_GetModemStatus,
    SerGetCommProperties,
    SL_PurgeComm,
    SL_SetDCB,
    SL_SetCommTimeouts,
};


//-----------------------------------------------------------------------------
//
// Function: GetSerialObject
//
//  The purpose of this function is to allow
//  multiple PDDs to be linked with a single MDD
//  creating a multiport driver.  In such a driver,
//  the MDD must be able to determine the correct
//  vtbl and associated parameters for each PDD.
//  Immediately prior to calling HWInit, the MDD
//  calls GetSerialObject to get the correct function
//  pointers and parameters.
//
// Parameters:
//      pContext
//          [in] Index into an array of serial devices,
//                  corresponding to the different lower layer
//                  implementations that may be shared by a single
//                  upper layer implementation.
//
// Returns:
//      Zero indicates success.
//
//-----------------------------------------------------------------------------
PHWOBJ GetSerialObject( DWORD DeviceArrayIndex )
{
    PHWOBJ pSerObj;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("GetSerialObject+ 0x%x\r\n"),DeviceArrayIndex));

    // Unlike many other serial samples, we do not have a statically allocated
    // array of HWObjs.  Instead, we allocate a new HWObj for each instance
    // of the driver.  The MDD will always call GetSerialObj/HWInit/HWDeinit in
    // that order, so we can do the alloc here and do any subsequent free in
    // HWDeInit.

    // Allocate space for the HWOBJ.
    pSerObj = (PHWOBJ)LocalAlloc(LMEM_ZEROINIT|LMEM_FIXED , sizeof(HWOBJ));
    if ( !pSerObj )
        return (NULL);

    // Now return this structure to the MDD.
    // Fill in the HWObj structure that we just allocated.
    pSerObj->BindFlags = THREAD_AT_INIT;     // Have MDD create thread when device is initialized.
    pSerObj->dwIntID = 0;                    // SysIntr is filled in at init time

    if ( DeviceArrayIndex == 1 )
        pSerObj->pFuncTbl = (HW_VTBL *) &IrVTbl; // Return pointer to appropriate functions
    else
        pSerObj->pFuncTbl = (HW_VTBL *) &IoVTbl; // Return pointer to appropriate functions

    DEBUGMSG(ZONE_FUNCTION,(TEXT("GetSerialObject-\r\n")));

    return (pSerObj);
}
