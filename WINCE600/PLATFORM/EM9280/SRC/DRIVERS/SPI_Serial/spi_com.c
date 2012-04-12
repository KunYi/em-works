//**********************************************************************
//                                                                      
// Filename: spi_com.c
//                                                                      
// Description: 
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Use of this source code is subject to the terms of the Cirrus end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to 
// use this source code. For a copy of the EULA, please see the 
// EULA.RTF on your install media.
//
// Copyright(c) Cirrus Logic Corporation 2005, All Rights Reserved                       
//                                                                      
//**********************************************************************

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the EULA.RTF on your
// install media.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

Abstract:  

Notes:
(1) CS			FEB-02-07: created from sl010com.c
(2) CS&ZHL	SEP-03-11: created from EM9161 ETA503 isa_com.c
--*/

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
#include <ser16550.h>
//#include <hw16550.h>

// Controller includes
#include "bsp.h"
#include "ioctl_cfg.h"

#undef ZONE_INIT
#undef ZONE_ERROR

// it is important to include head files "*.h" as above oder! 
#include "spi_com.h"
#include "ser_ht45b0f.h"

//
// CS&ZHL JUN-20-2008: Enable or Disable UART which shares interrupt with each other 
//
//#define IOCTL_HAL_ISA_UART_ENABLE		  CTL_CODE(FILE_DEVICE_HAL, 78, METHOD_BUFFERED, FILE_ANY_ACCESS)
//#define IOCTL_HAL_ISA_UART_DISABLE		  CTL_CODE(FILE_DEVICE_HAL, 79, METHOD_BUFFERED, FILE_ANY_ACCESS)

// #define DBGMSG
//#define DBGMSG NKDbgPrintfW

// Macros to read/write serial registers.
//#define INB(pInfo, reg) (READ_PORT_UCHAR((UCHAR *)((pInfo)->reg)))
//#define INB(pInfo, reg)             (*((volatile UCHAR *)((pInfo)->reg)))

// 1# And now, all the function prototypes
BOOL SL4_Init(
        PVOID       pHead,			//  points to device head
        PUCHAR      pRegBase,		// Pointer to 16550 register base
        UINT8       RegStride,		// Stride amongst the 16550 registers
        EVENT_FUNC  EventCallback,	// This callback exists in MDD
        PVOID       pMddHead,		// This is the first parm to callback
        PLOOKUP_TBL pBaudTable		// Pointer to baud rate table
        );

static BOOL	g_bRSTOUTn = 0;

/*///////////////////////////////////////////////////////////////////////////
 @doc OEM 
 @func PVOID | ExSerInit | Initializes device identified by argument.
 *  This routine sets information controlled by the user
 *  such as Line control and baud rate. It can also initialize events and
 *  interrupts, thereby indirectly managing initializing hardware buffers.
 *  Exported only to driver, called only once per process.
 *
 @rdesc The return value is a PVOID to be passed back into the HW
 dependent layer when HW functions are called.
//////////////////////////////////////////////////////////////////////////*/
static 
PVOID 
ExSerInit(
       ULONG   Identifier, // @parm Device identifier.
       PVOID   pMddHead,   // @parm First argument to mdd callbacks.
       PHWOBJ  pHWObj      // @parm Pointer to our own HW OBJ for this device
       )
{
    PSER_INFO	pHead;
    HKEY		hKey;
    DWORD		datasize = sizeof(DWORD);

    // Allocate for our main data structure and one of it's fields.
    pHead = (PSER_INFO)LocalAlloc( LMEM_ZEROINIT|LMEM_FIXED, sizeof(SER_INFO) );
    if ( !pHead )
        return( NULL );

	//
	// CS&ZHL SEP-3-2011: save dwDeviceArrayIndex from MDD
	//
	pHead->dwDeviceArrayIndex = pHWObj->dwIntID;
	pHWObj->dwIntID = 0;

	//
	// CS&ZHL JUN-02-2008: read deviceindex from registry
	//
    hKey = OpenDeviceKey((LPCTSTR)Identifier);
    if ( !hKey ) {
        DEBUGMSG (ZONE_INIT | ZONE_ERROR,
                  (TEXT("Failed to open devkeypath, COM_Init failed\r\n")));
        LocalFree(pHead);
        return(NULL);
    }

	// get local index number of HT45B0F
    datasize = sizeof(DWORD);
    if ( RegQueryValueEx(hKey, L"LocalIndex", NULL, NULL, (LPBYTE)&pHead->dwLocalIndex, &datasize) ) 
	{
        RETAILMSG(1, (TEXT("ExSerInit::Failed to get LocalIndex value\r\n")));
        RegCloseKey (hKey);
        LocalFree(pHead);
        return(NULL);
    }

	// get interrupt request number for each HT45B0F
    datasize = sizeof(DWORD);
    if ( RegQueryValueEx(hKey, L"IrqNum", NULL, NULL, (LPBYTE)&pHead->dwIrqNum, &datasize) ) 
	{
        RETAILMSG(1, (TEXT("ExSerInit::Failed to get IrqNum value\r\n")));
        RegCloseKey (hKey);
        LocalFree(pHead);
        return(NULL);
    }

	// get interrupt GPIO pin number for each HT45B0F
    datasize = sizeof(DWORD);
    if ( RegQueryValueEx(hKey, L"IrqGpioPin", NULL, NULL, (LPBYTE)&pHead->dwIrqGpioPin, &datasize) ) 
	{
        RETAILMSG(1, (TEXT("ExSerInit::Failed to get IrqGpioPin value\r\n")));
        RegCloseKey (hKey);
        LocalFree(pHead);
        return(NULL);
    }
	RegCloseKey (hKey);

	RETAILMSG(1, (TEXT("ExSerInit::COM%d - Init HT45B0F\r\n"), pHead->dwDeviceArrayIndex));

	//
	// CS&ZHL JLY-17-2008: Request SysIntr for the device
	//
	pHead->dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
						(PVOID)&pHead->dwIrqNum, 
						sizeof(pHead->dwIrqNum), 
						(PVOID)&pHead->dwSysIntr, 
						sizeof(pHead->dwSysIntr), NULL))
	{
		RETAILMSG(1, (TEXT("ExSerInit::Failed to request SysIntr for COM%d.\r\n"), pHead->dwDeviceArrayIndex));
		pHead->dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
		LocalFree(pHead);
		return(NULL);
	}


    pHead->pMddHead = pMddHead;
    pHead->pHWObj = pHWObj;
    pHead->OpenCount = 0;

    // Legacy - We have 2 identical fields because registry used to contain IRQ
    pHead->pHWObj->dwIntID = pHead->dwSysIntr;
    DEBUGMSG (1|ZONE_INIT, (TEXT("ExSerInit - SYSINTR %d\r\n"),  pHead->pHWObj->dwIntID));

    // Set up our Comm Properties data    
    pHead->CommProp.wPacketLength   = 0xffff;
    pHead->CommProp.wPacketVersion  = 0xffff;
    pHead->CommProp.dwServiceMask   = SP_SERIALCOMM;
    pHead->CommProp.dwReserved1     = 0;
    pHead->CommProp.dwMaxTxQueue    = 16;
    pHead->CommProp.dwMaxRxQueue    = 16;
    pHead->CommProp.dwMaxBaud       = BAUD_115200;
    pHead->CommProp.dwProvSubType   = PST_RS232;

    pHead->CommProp.dwProvCapabilities =
            PCF_SETXCHAR |
            PCF_INTTIMEOUTS |
            PCF_PARITY_CHECK |
            PCF_SPECIALCHARS |
            PCF_TOTALTIMEOUTS |
            PCF_XONXOFF;

    if(pHead->dwDeviceArrayIndex)
    {
        pHead->CommProp.dwProvCapabilities |= PCF_DTRDSR | PCF_RLSD | PCF_RTSCTS;
    }
    
    pHead->CommProp.dwSettableBaud      =
    BAUD_075 | BAUD_110 | BAUD_150 | BAUD_300 | BAUD_600 |
    BAUD_1200 | BAUD_1800 | BAUD_2400 | BAUD_4800 |
    BAUD_7200 | BAUD_9600 | BAUD_14400 |
    BAUD_19200 | BAUD_38400 | BAUD_56K | BAUD_128K |
    BAUD_115200 | BAUD_57600 | BAUD_USER;

    pHead->CommProp.dwSettableParams    =
    SP_BAUD | SP_DATABITS | SP_HANDSHAKING | SP_PARITY |
    SP_PARITY_CHECK | SP_RLSD | SP_STOPBITS;

    pHead->CommProp.wSettableData       =
    DATABITS_5 | DATABITS_6 | DATABITS_7 | DATABITS_8;

    pHead->CommProp.wSettableStopParity =
    STOPBITS_10 | STOPBITS_20 |
    PARITY_NONE | PARITY_ODD | PARITY_EVEN | PARITY_SPACE |
    PARITY_MARK;

    pHead->fIRMode  = FALSE;   // Select wired by default

    // Init HT45 info
	pHead->UartHt45.USR  = HT45B0F_USR_RIDLE | HT45B0F_USR_TIDLE | HT45B0F_USR_TXIF;
	pHead->UartHt45.UCR1 = 0;
	pHead->UartHt45.UCR2 = HT45B0F_UCR2_BAUD_DIV16;				// BaudRate = 1.8432MHz / (16 * (BRG + 1)) => same with 16C550
	pHead->UartHt45.BRG  = 12 - 1;								// default settings => 9600bps
	pHead->UartHt45.UCR3 = 0;

    DEBUGMSG (ZONE_INIT, (TEXT("ExSerInit - Init ht45b0f data\r\n")));
    if(!SL4_Init(pHead,				// @parm points to device head
		NULL,						// null Pointer, not used
		0,							// 0 Stride, not used
		EvaluateEventFlag,			// This callback exists in MDD
		pMddHead,					// This is the first parm to callback
		NULL))						// BaudRate Table = NULL: use default baud table
	{
		RETAILMSG(1, (TEXT("ExSerInit::COM%d hardware init failed\r\n"), pHead->dwDeviceArrayIndex));
		// release system interrupt resource
		KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, 
						&pHead->dwSysIntr, sizeof(pHead->dwSysIntr), 
						NULL, 0, 0);
		
		LocalFree(pHead);
		return(NULL);
	}

	RETAILMSG(1, (TEXT("ExSerInit::COM%d hardware init passed\r\n"), pHead->dwDeviceArrayIndex));
    return (pHead);
}

/*
 @doc OEM
 @func ULONG | ExSerClose | This routine closes the device identified by the PVOID returned by ExSerInit.
 *  Not exported to users, only to driver.
 *
 @rdesc The return value is 0.
 */
static ULONG ExSerClose( PVOID pHead )	   // @parm PVOID returned by ExSerInit.
{
    PSER_INFO		pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_CLOSE,(TEXT("+SerClose\r\n")));
    if ( pSerHead->OpenCount ) 
	{
        DEBUGMSG (ZONE_CLOSE, (TEXT("ExSerClose, closing device\r\n")));
        pSerHead->OpenCount--;

        DEBUGMSG (ZONE_CLOSE, (TEXT("ExSerClose - Calling SL_Close\r\n")));
        SL_Close( pHead );
    }

    DEBUGMSG (ZONE_CLOSE,(TEXT("-SerClose\r\n")));
    return (0);
}

/*
 @doc OEM 
 @func PVOID | ExSerDeinit | Deinitializes device identified by argument.
 *  This routine frees any memory allocated by ExSerInit.
 *
 */
//static BOOL ExSerDeinit( PVOID pHead )		   // @parm PVOID returned by ExSerInit.
static ULONG ExSerDeinit( PVOID pHead )		   // @parm PVOID returned by ExSerInit.
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;

    RETAILMSG(1, (TEXT("-->ExSerDeInit\r\n")));

    if ( !pSerHead )
        return (FALSE);

    // Make sure device is closed before doing DeInit
    if ( pSerHead->OpenCount )
        ExSerClose( pHead );

	//
	// CS&ZHL APR-07-2009: deinit Ser16550 object first!
	//
	SL_Deinit(pHead);

	//
	// CS&ZHL OCT-22-2008: release SysIntr
	//
	InterruptDisable(pSerHead->dwSysIntr);
	KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, 
					&pSerHead->dwSysIntr, sizeof(pSerHead->dwSysIntr), 
					NULL, 0, 0);

    // Free the HWObj
    LocalFree(pSerHead->pHWObj);
    
    // And now free the SER_INFO structure.
    LocalFree(pSerHead);

    //return (TRUE);
    return (0);
}

/*
 @doc OEM
 @func  VOID | ExSerGetCommProperties | Retrieves Comm Properties.
 *
 @rdesc None.
*/
// @parm PVOID returned by SerInit. 
// @parm Pointer to receive COMMPROP structure. 
static VOID ExSerGetCommProperties( PVOID pHead, LPCOMMPROP pCommProp )   
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;

    //*pCommProp = pHWHead->CommProp;
    memcpy( pCommProp, &pSerHead->CommProp, sizeof(COMMPROP));

    return;
}


/*
 @doc OEM
 @func VOID | SerSetBaudRate |
 * This routine sets the baud rate of the device.
 *  Not exported to users, only to driver.
 *
 @rdesc None.
 */
// @parm     PVOID returned by SerInit
// @parm     ULONG representing decimal baud rate.
static BOOL SerSetBaudRate( PVOID pHead, ULONG BaudRate	)   
{
    //PSER_INFO   pSerHead = (PSER_INFO)pHead;

    // Now set buadrate on the UART
    return ( SL_SetBaudRate( pHead, BaudRate ) );    
}

/*
 @doc OEM
 @func BOOL | SerPowerOff |
 *  Called by driver to turn off power to serial port.
 *  Not exported to users, only to driver.
 *
 @rdesc This routine returns a status.
 */
static BOOL ExSerPowerOff( PVOID pHead )       // @parm PVOID returned by SerInit.
{
    // First, power down the UART
    SL_PowerOff( pHead );

    return (TRUE);
}

/*
 @doc OEM
 @func BOOL | SerPowerOn |
 *  Called by driver to turn on power to serial port.
 *  Not exported to users, only to driver.
 *
 @rdesc This routine returns a status.
 */
static BOOL ExSerPowerOn( PVOID pHead )       // @parm  PVOID returned by SerInit.
{
    // First, power up the UART
    SL_PowerOn( pHead );

    return (TRUE);
}

/*
 @doc OEM
 @func BOOL | ExSerEnableIR | This routine enables ir.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSE otherwise.
 */
// @parm PVOID returned by Serinit.
// @parm PVOID returned by HWinit.
static BOOL ExSerEnableIR( PVOID pHead, ULONG BaudRate ) 
{
	// not support IR
    UNREFERENCED_PARAMETER(pHead);
    UNREFERENCED_PARAMETER(BaudRate);

	return (FALSE);
}

/*
 @doc OEM
 @func BOOL | ExSerDisableIR | This routine disable the ir.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSEotherwise.
 */
static BOOL ExSerDisableIR( PVOID pHead )		//@parm PVOID returned by Serinit.
{
    UNREFERENCED_PARAMETER(pHead);

	return (TRUE);
}

/*
 @doc OEM
 @func BOOL | ExSerOpen | This routine is called when the port is opened.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSEotherwise.
 */
static BOOL ExSerOpen( PVOID pHead )		/*@parm PVOID returned by Serinit. */
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;

    // Disallow multiple simultaneous opens
    if ( pSerHead->OpenCount )
        return (FALSE);

    pSerHead->OpenCount++;

    // NOTE: - If we wanted to support 16450s, we'll could dynamically
    // identify them here.
    pSerHead->ChipID = CHIP_ID_16550;

    // Init 16550 info
    DEBUGMSG (ZONE_OPEN, (TEXT("ExSerOpen - Calling SL_Open\r\n")));
    SL_Open( pHead );

    return (TRUE);
}

const HW_VTBL SerIoVTbl =
{
    ExSerInit,
    SL_PostInit,
    ExSerDeinit,
    ExSerOpen,
    ExSerClose,
    SL_GetInterruptType,
    SL_RxIntr,
    SL_TxIntrEx,
    SL_ModemIntr,
    SL_LineIntr,
    SL_GetRxBufferSize,
    ExSerPowerOff,
    ExSerPowerOn,
    SL_ClearDTR,
    SL_SetDTR,
    SL_ClearRTS,
    SL_SetRTS,
    ExSerEnableIR,
    ExSerDisableIR,
    SL_ClearBreak,
    SL_SetBreak,
    SL_XmitComChar,
    SL_GetStatus,
    SL_Reset,
    SL_GetModemStatus,
    ExSerGetCommProperties,
    SL_PurgeComm,
    SL_SetDCB,
    SL4_SetCommTimeouts,
	SL_Ioctl 
};

// GetSerialObj : The purpose of this function is to allow multiple PDDs to be
// linked with a single MDD creating a multiport driver.  In such a driver, the
// MDD must be able to determine the correct vtbl and associated parameters for
// each PDD.  Immediately prior to calling HWInit, the MDD calls GetSerialObject
// to get the correct function pointers and parameters.
//
PHWOBJ
GetSerialObject(
               DWORD DeviceArrayIndex
               )
{
    PHWOBJ pSerObj;

    // Unlike many other serial samples, we do not have a statically allocated
    // array of HWObjs.  Instead, we allocate a new HWObj for each instance
    // of the driver.  The MDD will always call GetSerialObj/HWInit/HWDeinit in
    // that order, so we can do the alloc here and do any subsequent free in
    // HWDeInit.

    // Allocate space for the HWOBJ.
    pSerObj = (PHWOBJ)LocalAlloc( LMEM_ZEROINIT|LMEM_FIXED ,
                                  sizeof(HWOBJ) );
    if ( !pSerObj )
        return (NULL);

    // Fill in the HWObj structure that we just allocated.

    pSerObj->BindFlags = THREAD_AT_OPEN;			// Have MDD create thread when device is first opened.
    //pSerObj->dwIntID = 0;							// SysIntr is filled in at init time
    pSerObj->dwIntID = DeviceArrayIndex;			// only for temperary save, will be filled into HWHead->dwDeviceArrayIndex
	pSerObj->pFuncTbl = (HW_VTBL *)&SerIoVTbl;		// UART4, UART8, UART9 expaned with HT45B0F

    // Now return this structure to the MDD.
    return (pSerObj);
}


// ----- end of isa_com.c ------