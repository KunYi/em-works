//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
//------------------------------------------------------------------------------
//
//  File:  serial.c
//
//  This file implements the device specific functions for serial device.
//
//------------------------------------------------------------------------------

#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <windev.h>

#include <types.h>
#include <memory.h>
#include <nkintr.h>
#include <serhw.h>
#include <notify.h>
#include <devload.h>

#include "common_types.h"
#include "common_uartdbg.h"
#include "dserial.h"

extern PVOID pv_HWregUARTDbg;

//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPUartConfigureGPIO(ULONG HWAddr);
extern ULONG  BSPGetIRQ(ULONG HWAddr);

//-----------------------------------------------------------------------------
//
// Function: Ser_InternalMapRegisterAddresses
//
//  This function retrieves the current properties of the communications device.
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
PUCHAR static Ser_InternalMapRegisterAddresses(ULONG HWAddress, ULONG Size)
{
    PUCHAR ioPortBase = NULL;
    ULONG inIoSpace = 1;
    PHYSICAL_ADDRESS ioPhysicalBase = {HWAddress, 0};

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_InternalMapRegisterAddresses : HalTranslateBusAddress\r\n")));
    if (HalTranslateBusAddress(Isa, 0, ioPhysicalBase, &inIoSpace, &ioPhysicalBase))
    {
        if (!inIoSpace)
        {
            if ((ioPortBase = (PUCHAR)MmMapIoSpace(ioPhysicalBase, Size, FALSE)) == NULL)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("Error mapping I/O Ports\r\n")));
                return (ioPortBase);
            }
        }
        else
        {
            ioPortBase = (PUCHAR)ioPhysicalBase.LowPart;
        }
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error translating I/O Ports.\r\n")));
        return (ioPortBase);
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_InternalMapRegisterAddresses : %d\r\n"), ioPortBase));
    return (ioPortBase);
}
//-----------------------------------------------------------------------------
//
// Function: Ser_GetRegistryData
//
//  This function takes the registry path provided to COM_Init and uses it to find this requested comm
//  port's DeviceArrayIndex, the IOPort Base Address, and the interrupt number.
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
static BOOL Ser_GetRegistryData(PSER_INFO pSerHead, LPCTSTR regKeyPath)
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
    if (hKey == NULL)
    {
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

    if (regError == ERROR_SUCCESS)
    {
        dwDataSize = PC_REG_IOBASE_VAL_LEN;
        regError = RegQueryValueEx(
            hKey,
            PC_REG_IOBASE_VAL_NAME,
            NULL,
            NULL,
            (LPBYTE)(&pSerHead->dwIOBase),
            &dwDataSize);
    }

    if (regError == ERROR_SUCCESS)
    {
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

    if (regError != ERROR_SUCCESS)
    {
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
//  This routine sets information controlled by the user such as Line control and baud rate.
//  It can also initialize events and interrupts, thereby indirectly managing initializing hardware
//  buffers. Exported only to driver, called only once per process.
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
//      The return value is a PVOID to be passed back  into the HW dependent layer when HW
//      functions are called.
//
//-----------------------------------------------------------------------------
static PVOID SerInit(BOOL bIR, ULONG Identifier, PVOID pMDDContext, PHWOBJ pHWObj)
{
    PSER_INFO pSerHead = NULL;
    BOOL InitError = FALSE;
    ULONG irq;

    DEBUGMSG(ZONE_INIT, (TEXT("SerInit+\r\n")));

    // Allocate for our main data structure and one of it's fields.
    pSerHead = (PSER_INFO)LocalAlloc(LMEM_ZEROINIT|LMEM_FIXED, sizeof(SER_INFO));

    if (!pSerHead)
    {
        return(NULL);
    }

    if (!Ser_GetRegistryData(pSerHead, (LPCTSTR)Identifier))
    {
        DEBUGMSG(1, (TEXT("SerInit - Unable to read registry data!!\r\n")));
        InitError = TRUE;
    }

    BSPUartConfigureGPIO(pSerHead->dwIOBase);

    if (!InitError)
    {
        pSerHead->pBaseAddress = Ser_InternalMapRegisterAddresses(
            pSerHead->dwIOBase, pSerHead->dwIOLen);

        pSerHead->pHWObj = pHWObj;
        pSerHead->cOpenCount = 0;

        DEBUGMSG(ZONE_FUNCTION, (TEXT("SerInit \r\n")));

        irq = BSPGetIRQ(pSerHead->dwIOBase);

        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
                             &pSerHead->pHWObj->dwIntID, sizeof(DWORD), NULL))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: Failed to obtain sysintr value for UART interrupt.\r\n")));
            LocalFree(pSerHead);
            DEBUGMSG(ZONE_ERROR, (TEXT("SerInit - Initialization failed!!\r\n")));
            return (NULL);
        }
        pSerHead->irq = irq;

        // Set up our Comm Properties data
        pSerHead->CommProp.wPacketLength      = 0xFFFF;
        pSerHead->CommProp.wPacketVersion     = 0xFFFF;
        pSerHead->CommProp.dwServiceMask      = SP_SERIALCOMM;
        pSerHead->CommProp.dwReserved1        = 0;
        pSerHead->CommProp.dwMaxTxQueue       = 32;
        pSerHead->CommProp.dwMaxRxQueue       = 32;
        pSerHead->CommProp.dwMaxBaud          = BAUD_115200;
        pSerHead->CommProp.dwProvSubType      = PST_RS232;

        pSerHead->CommProp.dwProvCapabilities =
            PCF_DTRDSR | PCF_RLSD | PCF_RTSCTS |
            PCF_SETXCHAR | PCF_INTTIMEOUTS |
            PCF_PARITY_CHECK | PCF_SPECIALCHARS |
            PCF_TOTALTIMEOUTS | PCF_XONXOFF;

        pSerHead->CommProp.dwSettableBaud =   BAUD_300 |
            BAUD_600 | BAUD_1200 | BAUD_1800 | BAUD_2400 |
            BAUD_4800 | BAUD_7200 | BAUD_9600 | BAUD_14400 |
            BAUD_19200 | BAUD_38400 | BAUD_56K | BAUD_115200 |
            BAUD_57600 | BAUD_USER;

        pSerHead->CommProp.dwSettableParams =
            SP_BAUD | SP_DATABITS | SP_HANDSHAKING | SP_PARITY |
            SP_PARITY_CHECK | SP_RLSD | SP_STOPBITS;

        pSerHead->CommProp.wSettableData =
            DATABITS_5 |
            DATABITS_6 |
            DATABITS_7 |
            DATABITS_8;

        pSerHead->CommProp.wSettableStopParity =
            STOPBITS_10 | STOPBITS_20 |
            PARITY_NONE | PARITY_ODD | PARITY_EVEN;

        pSerHead->fIRMode = bIR;

        SL_Init(bIR,pSerHead->dwIOBase, pSerHead->pBaseAddress,
                pSerHead, EvaluateEventFlag, pMDDContext);

        return (pSerHead);
    }
    else
    {
        if (pSerHead->pBaseAddress)
            MmUnmapIoSpace(pSerHead->pBaseAddress, pSerHead->dwIOLen);

        LocalFree(pSerHead);

        DEBUGMSG(1, (TEXT("SerInit - Initialization failed!!\r\n")));
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
//      The return value is a PVOID to be passed back into the HW dependent layer when HW
//      functions are called.
//
//-----------------------------------------------------------------------------
static PVOID SerSerialInit(ULONG Identifier, PVOID pMDDContext, PHWOBJ pHWObj)
{
    DEBUGMSG(ZONE_INIT,(TEXT("SerSerialInit+\r\n")));
    return (SerInit(FALSE,Identifier, pMDDContext, pHWObj));
}

//-----------------------------------------------------------------------------
//
// Function: SerPostInit
//
//  This function is called by the upper layer to perform any necessary operations after initializing
//  all data structures and prepare the serial IST to begin handling interrupts. This call occurs as
//  the last step of the COM_Init routine.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned by SerInit function that contains
//                  implementation-specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerPostInit(PVOID pContext)
{
    DEBUGMSG(ZONE_INIT,(TEXT("SerPostInit+\r\n")));

    // Since we are just a library which might get used for
    // builtin ports which init at boot, or by PCMCIA ports
    // which init at Open, we can't do anything too fancy.
    // Lets just make sure we cancel any pending interrupts so
    // that if we are being used with an edge triggered PIC, he
    // will see an edge after the MDD hooks the interrupt.
    SL_ClearPendingInts(pContext);

    DEBUGMSG(ZONE_INIT,(TEXT("SerPostInit-\r\n")));
    return(TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SerOpen
//
//  This function is called by the upper layer to open the serial device. This function applies
//  power to the serial hardware.
//
// Parameters:
//      pContext
//          [in] Pointer to a context structure returned by SerInit function that contains
//                  implementation-specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerOpen(PVOID pContext)
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_OPEN,(TEXT("SerOpen+ \r\n")));

    // Disallow multiple simultaneous opens
    if (pSerHead->cOpenCount)
    {
        return (FALSE);
    }

    pSerHead->cOpenCount++;

    SL_Open(pSerHead);

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
//          [in] Pointer to a context structure returned by SerInit function that contains
//                  implementation-specific data describing the hardware device.
//
// Returns:
//      Zero indicates success.
//
//-----------------------------------------------------------------------------
static ULONG SerClose(PVOID pContext)
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;
    ULONG uTries;

    DEBUGMSG(ZONE_CLOSE,(TEXT("SerClose+ \r\n")));

    if (pSerHead->cOpenCount)
    {
        DEBUGMSG(ZONE_CLOSE, (TEXT("SerClose - closing device\r\n")));

        pSerHead->cOpenCount--;
        // while we are still transmitting, sleep.
        uTries = 0;
        while(!(HW_UARTDBGFR_RD()&BM_UARTDBGFR_TXFE)&& (uTries++ < 100))
        {    // TxFifo not empty..
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
//          [in] Pointer to a context structure returned by SerInit function that contains
//                  implementation-specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static ULONG SerDeinit(PVOID pContext)
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_INIT, (TEXT("SerDeinit+\r\n")));

    if (!pSerHead)
    {
        return (FALSE);
    }

    // Make sure device is closed before doing DeInit
    if (pSerHead->cOpenCount)
    {
        SerClose(pContext);
    }

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pSerHead->pHWObj->dwIntID,
                    sizeof(DWORD), NULL, 0, NULL);
    pSerHead->pHWObj->dwIntID = (DWORD)(SYSINTR_UNDEFINED);

    if (pSerHead->pBaseAddress)
    {
        MmUnmapIoSpace(pSerHead->pBaseAddress, pSerHead->dwIOLen);
    }

    SL_Deinit(pContext);

    // Free the HWObj
    LocalFree(pSerHead->pHWObj);

    // And now free the SER_INFO structure.
    LocalFree(pSerHead);

    DEBUGMSG(ZONE_INIT, (TEXT("SerDeinit-\r\n")));
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
//          [in] Pointer to a context structure returned by SerInit function that contains
//                  implementation-specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerPowerOff(PVOID pContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerPowerOff+\r\n")));

    // First, power down the UART
    SL_PowerOff(pContext);

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
//          [in] Pointer to a context structure returned by SerInit function that contains
//                  implementation-specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerPowerOn(PVOID pContext)
{

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerPowerOn+\r\n")));
    // First, power up the UART
    SL_PowerOn(pContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SerPowerOn-\r\n")));
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
//          [in] Pointer to a context structure returned by SerInit function that contains
//              implementation-specific data describing the hardware device.
//      pCommProp
//          [in] Pointer to a COMMPROP structure to
//                hold the communications property information.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID SerGetCommProperties(PVOID pContext, LPCOMMPROP pCommProp)
{
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SerGetCommProperties+\r\n")));

    *pCommProp = pSerHead->CommProp;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SerGetCommProperties-\r\n")));
    return;
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
//          [in] Pointer to a context structure returned by SerInit function that contains
//                  implementation-specific data describing the hardware device.
//      BaudRate
//          [in] Specifies the baud to set.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerEnableIR(PVOID pContext, ULONG BaudRate)
{

    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(BaudRate);

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
//          [in] Pointer to a context structure returned by SerInit function that contains
//                  implementation-specific data describing the hardware device.
//
// Returns:
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL SerDisableIR(PVOID pContext)
{
    UNREFERENCED_PARAMETER(pContext);

    DEBUGMSG(ZONE_IR,(TEXT("SerDisableIR+\r\n")));
    DEBUGMSG(ZONE_IR,(TEXT("SerDisableIR-\r\n")));

    return (TRUE);
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
    SL_Ioctl                        //WINCE600
};
//-----------------------------------------------------------------------------
//
// Function: GetSerialObject
//
//  The purpose of this function is to allow multiple PDDs to be linked with a single MDD creating
//  a multiport driver.  In such a driver, the MDD must be able to determine the correct vtbl and
//  associated parameters for each PDD. Immediately prior to calling HWInit, the MDD calls
//  GetSerialObject to get the correct function pointers and parameters.
//
// Parameters:
//      pContext
//          [in] Index into an array of serial devices, corresponding to the different lower layer
//                  implementations that may be shared by a single upper layer implementation.
//
// Returns:
//      Zero indicates success.
//
//-----------------------------------------------------------------------------
PHWOBJ GetSerialObject(DWORD DeviceArrayIndex)
{
    PHWOBJ pSerObj;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(DeviceArrayIndex);

    DEBUGMSG(ZONE_FUNCTION,(TEXT("GetSerialObject+ 0x%x\r\n"),DeviceArrayIndex));

    // Unlike many other serial samples, we do not have a statically allocated array of HWObjs.
    // Instead, we allocate a new HWObj for each instance of the driver.  The MDD will always
    // call GetSerialObj/HWInit/HWDeinit in that order, so we can do the alloc here and do any
    // subsequent free in HWDeInit.

    // Allocate space for the HWOBJ.
    pSerObj = (PHWOBJ)LocalAlloc(LMEM_ZEROINIT|LMEM_FIXED, sizeof(HWOBJ));
    if (!pSerObj)
    {
        return (NULL);
    }

    // Now return this structure to the MDD.
    // Fill in the HWObj structure that we just allocated.
    pSerObj->BindFlags = THREAD_AT_INIT;        // Have MDD create thread when device is initialized.
    pSerObj->dwIntID = 0;                       // SysIntr is filled in at init time

    pSerObj->pFuncTbl = (HW_VTBL *) &IoVTbl; // Return pointer to appropriate functions

    DEBUGMSG(ZONE_FUNCTION,(TEXT("GetSerialObject-\r\n")));
    return (pSerObj);
}
