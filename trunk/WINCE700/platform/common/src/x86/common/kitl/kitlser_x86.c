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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  
    kitlser.c
    
Abstract:

    Platform specific code for serial KITL services.
        
Functions:


Notes: 

--*/

#include <windows.h>
#include <pc.h>
#include <wdm.h> // for READ_PORT_UCHAR, WRITE_PORT_UCHAR
#include <oal.h>
#include "x86kitl.h"

// COM Line Status
#define LS_TSR_EMPTY        0x40
#define LS_THR_EMPTY        0x20
#define LS_RX_BREAK         0x10
#define LS_RX_FRAMING_ERR   0x08
#define LS_RX_PARITY_ERR    0x04
#define LS_RX_OVERRUN       0x02
#define LS_RX_DATA_READY    0x01
#define LS_RX_ERRORS        (LS_RX_FRAMING_ERR | LS_RX_PARITY_ERR | LS_RX_OVERRUN)

// COM Modem Status
#define MS_CARRIER_DETECT   0x80
#define MS_RING             0x40
#define MS_DSR              0x20
#define MS_CTS              0x10
#define MS_DELTA_DCD        0x08
#define MS_TRAIL_EDGE_RI    0x04
#define MS_DELTA_DSR        0x02
#define MS_DELTA_CTS        0x01

// COM Modem Control
#define MC_RTS              0x02
#define MC_DTR              0x01

// default baud divisor
#define DEF_BAUD_DIVISOR    0x01 // 115,200 bps

// receive timeout
#define TIMEOUT_RECV        100 // ms

static UCHAR *KitlIoPortBase;

/* COM16550Init
 *
 *  Called by PQOAL KITL framework to initialize the serial port
 *
 *  Return Value:
 */
BOOL COM16550Init (KITL_SERIAL_INFO *pSerInfo)
{
    KITLOutputDebugString ("+COM16550Initx\n");
    KITLOutputDebugString ("   pAddress = 0x%x\n", pSerInfo->pAddress);
    KITLOutputDebugString ("   BaudRate = 0x%x\n", pSerInfo->baudRate);
    KITLOutputDebugString ("   DataBits = 0x%x\n", pSerInfo->dataBits);
    KITLOutputDebugString ("   StopBits = 0x%x\n", pSerInfo->stopBits);
    KITLOutputDebugString ("   Parity   = 0x%x\n", pSerInfo->parity);

    KitlIoPortBase = pSerInfo->pAddress;
    if(KitlIoPortBase)
    {
        // TBD: use the serial information
        KITLOutputDebugString("Configuring serial port, IOBASE = 0x%x\n", KitlIoPortBase);
        WRITE_PORT_UCHAR(KitlIoPortBase+comIntEnable,   0x00);  // Interrupts off
        WRITE_PORT_UCHAR(KitlIoPortBase+comLineControl, 0x80);  // Access the Baud Divisor
        WRITE_PORT_UCHAR(KitlIoPortBase+comDivisorLow,  DEF_BAUD_DIVISOR);
        WRITE_PORT_UCHAR(KitlIoPortBase+comDivisorHigh, 0x00);  //
        WRITE_PORT_UCHAR(KitlIoPortBase+comFIFOControl, 0x07);  // Enable and clear FIFOs, if present
        WRITE_PORT_UCHAR(KitlIoPortBase+comLineControl, 0x03);  // 8 bit, no parity
        WRITE_PORT_UCHAR(KitlIoPortBase+comModemControl,0x01);  // Assert DTR line, lower RTS line

        // clear comm errors
        READ_PORT_UCHAR(KitlIoPortBase+comLineStatus);

        pSerInfo->bestSize = 1;       // read it one by one

        KITLOutputDebugString("Serial port configured for transport\n");
    }
    KITLOutputDebugString ("-COM16550Init\n");

    return 0 != KitlIoPortBase;
}

/* COM16550WriteData
 *
 *  Block until the byte is sent
 *
 *  Return Value: TRUE on success, FALSE otherwise
 */
UINT16 COM16550WriteData (UINT8 *pch, UINT16 length)
{
    if (!KitlIoPortBase) {
        length = 0;
    } else {
        DEBUGCHK (length == 1);

        // check that send transmitter holding register is empty
        while(!(READ_PORT_UCHAR(KitlIoPortBase+comLineStatus) & LS_THR_EMPTY)) {
            ;
        }
        // write character to port
        WRITE_PORT_UCHAR(KitlIoPortBase+comTxBuffer, *pch);

    }
    return length;
}

VOID COM16550FlowControl (BOOL fOn)
{
    if (KitlIoPortBase) {
        UCHAR uCtrl = (READ_PORT_UCHAR(KitlIoPortBase+comModemControl) & ~MC_RTS);

        WRITE_PORT_UCHAR (KitlIoPortBase+comModemControl, (UCHAR)(uCtrl | (fOn? MC_RTS : 0)));

        if (fOn) {
            // clear interrupts, if applicable
            READ_PORT_UCHAR (KitlIoPortBase+comIntId);
        }
    }
}

/* COM16550ReadData
 *
 *  Called from PQOAL KITL to read a byte from serial port
 *
 *  Return Value: TRUE on success, FALSE otherwise
 */
UINT16 COM16550ReadData (UINT8 *pch, UINT16 length)
{
    UCHAR uStatus;
    UINT16 count = 0;
    UNREFERENCED_PARAMETER(length);

    if (KitlIoPortBase) {
        uStatus = READ_PORT_UCHAR (KitlIoPortBase+comLineStatus);

        if (LS_RX_DATA_READY & uStatus) {
            if (LS_RX_ERRORS & uStatus) {
                KITLOutputDebugString ("E ");
                WRITE_PORT_UCHAR (KitlIoPortBase+comFIFOControl, 0x07);
            } else {
                *pch = READ_PORT_UCHAR(KitlIoPortBase+comRxBuffer);
                count = 1;
            }
        }
    }


    return count;
}


/* COM16550EnableInt
 *
 *  Enable Recv data interrupt
 *
 *  Return Value:
 */
VOID COM16550EnableInt (void)
{
    // polling, no interrupt
}

/* COM16550DisableInt
 *
 *  Disable Recv data interrupt
 *
 *  Return Value:
 */
VOID COM16550DisableInt (void)
{
    // polling, no interrupt
}


// serial driver
OAL_KITL_SERIAL_DRIVER DrvSerial = {
    COM16550Init,
    NULL,
    COM16550WriteData,
    NULL,
    COM16550ReadData,
    COM16550EnableInt,
    COM16550DisableInt,
    NULL,
    NULL,
    COM16550FlowControl,
};

const OAL_KITL_SERIAL_DRIVER *GetKitlSerialDriver (void)
{
    return &DrvSerial;
}
