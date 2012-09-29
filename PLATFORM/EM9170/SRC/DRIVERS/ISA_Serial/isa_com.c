//**********************************************************************
//                                                                      
// Filename: isa_com.c
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
(1) CS FEB-02-07: created from sl010com.c
--*/

#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <memory.h>
#include <serhw.h>
#include <ser16550.h>
#include <hw16550.h>
#include <nkintr.h>
#include <devload.h>

#include "at91sam9261.h"
#include "em9161_isa.h"

// Controller includes
#include "AT91SAM926x_oal_ioctl.h"
#include "at91sam9261ek_ioctl.h"

#undef ZONE_INIT
#include <serdbg.h>

// it is important to include head files "*.h" as above oder! 
#include "isa_com.h"
#include "isa_16550.h"

//
// CS&ZHL MAY-18-2009: ISA_CS1# range = 0x60 - 0x7F
//
#define	ISARegStride			1

//
// CS&ZHL JUN-20-2008: Enable or Disable UART which shares interrupt with each other 
//
//#define IOCTL_HAL_ISA_UART_ENABLE		  CTL_CODE(FILE_DEVICE_HAL, 78, METHOD_BUFFERED, FILE_ANY_ACCESS)
//#define IOCTL_HAL_ISA_UART_DISABLE		  CTL_CODE(FILE_DEVICE_HAL, 79, METHOD_BUFFERED, FILE_ANY_ACCESS)

// #define DBGMSG
//#define DBGMSG NKDbgPrintfW

// Macros to read/write serial registers.
//#define INB(pInfo, reg) (READ_PORT_UCHAR((UCHAR *)((pInfo)->reg)))
#define INB(pInfo, reg)             (*((volatile UCHAR *)((pInfo)->reg)))

// 1# And now, all the function prototypes
BOOL SL4_Init(
        PVOID       pHead,			//  points to device head
        PUCHAR      pRegBase,		// Pointer to 16550 register base
        UINT8       RegStride,		// Stride amongst the 16550 registers
        EVENT_FUNC  EventCallback,	// This callback exists in MDD
        PVOID       pMddHead,		// This is the first parm to callback
        PLOOKUP_TBL pBaudTable		// Pointer to baud rate table
        );


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
    PCOM_INFO	pHead;
    HKEY		hKey;
    LONG        regError;
    ULONG		datasize = sizeof(ULONG);
	DWORD		dwTmp;

    // Allocate for our main data structure and one of it's fields.
    pHead = (PCOM_INFO)LocalAlloc( LMEM_ZEROINIT|LMEM_FIXED, sizeof(COM_INFO) );
    if ( !pHead )
        return( NULL );

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

	// read device index
    datasize = sizeof(DWORD);
    if ( RegQueryValueEx(hKey, L"DeviceArrayIndex", NULL, NULL,
                         (LPBYTE)&pHead->dwDeviceArrayIndex, &datasize) ) {
        DEBUGMSG (ZONE_INIT | ZONE_ERROR,
                  (TEXT("Failed to get DeviceArrayIndex value, COM_Init failed\r\n")));
        RegCloseKey (hKey);
        LocalFree(pHead);
        return(NULL);
    }

	//
	// CS&ZHL JLY-15-2009: check if the driver is required to install
	//
    datasize = sizeof(DWORD);
    regError = RegQueryValueEx(
                              hKey, 
                              _T("TrueInstall"), 
                              NULL, 
                              NULL,
                              (LPBYTE)(&dwTmp), 
                              &datasize);
    if ( regError != ERROR_SUCCESS ) 
	{
		RETAILMSG(1, (TEXT("ExSerInit::failed to read TrueInstall.\r\n")));
		RegCloseKey (hKey);
        LocalFree(pHead);
        return (NULL);
    }
	if(!dwTmp)
	{
		RETAILMSG(1, (TEXT("ExSerInit::quit COM%d driver installation.\r\n"), pHead->dwDeviceArrayIndex));
		RegCloseKey (hKey);
        LocalFree(pHead);
        return (NULL);
	}

	// get base address
    datasize = sizeof(DWORD);
    if ( RegQueryValueEx(hKey, L"IOBaseAddress", NULL, NULL,
                         (LPBYTE)&dwTmp, &datasize) ) {
        DEBUGMSG (ZONE_INIT | ZONE_ERROR,
                  (TEXT("Failed to get IOBaseAddress value, COM_Init failed\r\n")));
        RegCloseKey (hKey);
        LocalFree(pHead);
        return(NULL);
    }
	pHead->pBaseAddress = (PUCHAR)dwTmp;

	// get DeviceID -> IrqNum
    datasize = sizeof(DWORD);
    if ( RegQueryValueEx(hKey, L"DeviceID", NULL, NULL,
                         (LPBYTE)&pHead->dwDeviceID, &datasize) ) {
        DEBUGMSG (ZONE_INIT | ZONE_ERROR,
                  (TEXT("Failed to get DeviceID value, COM_Init failed\r\n")));
        RegCloseKey (hKey);
        LocalFree(pHead);
        return(NULL);
    }

	// get Start COM logical number
    datasize = sizeof(DWORD);
    if ( RegQueryValueEx(hKey, L"StartDeviceIndex", NULL, NULL,
                         (LPBYTE)&pHead->dwStartDeviceIndex, &datasize) ) {
        DEBUGMSG (ZONE_INIT | ZONE_ERROR,
                  (TEXT("Failed to get StartDeviceIndex value, COM_Init failed\r\n")));
        RegCloseKey (hKey);
        LocalFree(pHead);
        return(NULL);
    }
    RegCloseKey (hKey);
    RETAILMSG(1, (TEXT("ExSerInit::COM%d - Init 16550\r\n"), pHead->dwDeviceArrayIndex));

	//
	// CS&ZHL JLY-17-2008: Request SysIntr for the device
	//
	pHead->dwSysIntr = SYSINTR_UNDEFINED;
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
						(PVOID)&pHead->dwDeviceID, 
						sizeof(pHead->dwDeviceID), 
						(PVOID)&pHead->dwSysIntr, 
						sizeof(pHead->dwSysIntr), NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: Failed to request SysIntr for COM%d.\r\n"), 
			pHead->dwDeviceArrayIndex));
		pHead->dwSysIntr = SYSINTR_UNDEFINED;
		LocalFree(pHead);
		return(NULL);
	}


    pHead->pMddHead = pMddHead;
    pHead->pHWObj = pHWObj;
    pHead->OpenCount = 0;

    // Legacy - We have 2 identical fields becausw registry used to contain IRQ
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
        pHead->CommProp.dwProvCapabilities |=PCF_DTRDSR | PCF_RLSD | PCF_RTSCTS;
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

    // Init 16550 info
    //DEBUGMSG (ZONE_INIT, (TEXT("ExSerInit - Init 16550 data\r\n")));
    if(!SL4_Init(pHead,				// @parm points to device head
		pHead->pBaseAddress,		// Pointer to 16550 register base
		ISARegStride,				// Stride amongst the 16550 registers = 1 bytes
		EvaluateEventFlag,			// This callback exists in MDD
		pMddHead,					// This is the first parm to callback
		NULL))						// BaudRate Table = NULL: use default baud table
	{
		RETAILMSG(1, (TEXT("ExSerInit -> not found!\r\n")));

		//
		// CS&ZHL MAY-19-2009: release SysIntr
		//
		KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, 
						&pHead->dwSysIntr, sizeof(pHead->dwSysIntr), 
						NULL, 0, 0);
		
		LocalFree(pHead);
		return(NULL);
	}
	RETAILMSG(1, (TEXT("ExSerInit -> success!\r\n")));

	//
	// CS&ZHL APR-07-2009: register irq info to enable interrupt sharing
	//
#ifdef	EM9161_ISA_UART_X4
	{
		EM9XXX_ISA_IRQ_INFO	IrqInfo;

		IrqInfo.dwIrqIndex    = pHead->dwDeviceArrayIndex - pHead->dwStartDeviceIndex;
		IrqInfo.dwIrqInfoAddr = (DWORD)(pHead->pBaseAddress + (ISARegStride * SCRATCH_REGISTER)); 
		if( !KernelIoControl(IOCTL_EM9XXX_SHARE_IRQ_LOAD, 
							&IrqInfo, 
							sizeof(EM9XXX_ISA_IRQ_INFO), 
							NULL,
							0, 
							NULL) )
		{
			RETAILMSG(1, (TEXT("COM%d IOCTL_EM9XXX_SHARE_IRQ_LOAD failed\r\n"), pHead->dwDeviceArrayIndex));
			LocalFree(pHead);
			return(NULL);
		}
	}
#endif	//EM9161_ISA_UART_X4

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
    PCOM_INFO		pSerHead = (PCOM_INFO)pHead;
    //PSER16550_INFO	pSer16550  = &(((PCOM_INFO)pHead)->ser16550);
    //ULONG			uTries;

    DEBUGMSG (ZONE_CLOSE,(TEXT("+SerClose\r\n")));
    if ( pSerHead->OpenCount ) 
	{
        DEBUGMSG (ZONE_CLOSE, (TEXT("ExSerClose, closing device\r\n")));
        pSerHead->OpenCount--;

		/*
        // while we are still transmitting, sleep.
        uTries = 0;
		pSer16550->IER = READ_PORT_UCHAR(pSer16550->pIER);
		while ( (pSer16550->IER & SERIAL_IER_THR)				// indicates TX in progress
              && (uTries++ < 100)								// safety net
              && !(pSer16550->LSR & SERIAL_LSR_TEMT) )			// indicates FIFO not yet empty
		{
			DEBUGMSG ( ZONE_CLOSE, (TEXT("SerClose, TX in progress, IER 0x%X, LSR 0x%X\r\n"),
					   *pSer16550->pIER, pSer16550->LSR));
			Sleep(10);
			pSer16550->IER = READ_PORT_UCHAR(pSer16550->pIER);
		}
		*/

        DEBUGMSG (ZONE_CLOSE, (TEXT("ExSerClose - Calling SL_Close\r\n")));
        SL4_Close( pHead );
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
static BOOL ExSerDeinit( PVOID pHead )		   // @parm PVOID returned by ExSerInit.
{
    PCOM_INFO   pSerHead = (PCOM_INFO)pHead;

    RETAILMSG(1, (TEXT("-->ExSerDeInit\r\n")));

    if ( !pSerHead )
        return (FALSE);

    // Make sure device is closed before doing DeInit
    if ( pSerHead->OpenCount )
        ExSerClose( pHead );

	//
	// CS&ZHL APR-07-2009: deinit Ser16550 object first!
	//
	SL4_Deinit(pHead);

	/*
	//
	// CS&ZHL OCT-22-2008: release SysIntr
	//
	InterruptDisable(pSerHead->dwSysIntr);
	KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, 
					&pSerHead->dwSysIntr, sizeof(pSerHead->dwSysIntr), 
					NULL, 0, 0);
	*/

    // Free the HWObj
    LocalFree(pSerHead->pHWObj);
    
    // And now free the SER_INFO structure.
    LocalFree(pSerHead);

    return (TRUE);
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
    PCOM_INFO   pSerHead = (PCOM_INFO)pHead;

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
    PCOM_INFO   pSerHead = (PCOM_INFO)pHead;

    // Now set buadrate on the UART
    return ( SL4_SetBaudRate( pHead, BaudRate ) );    
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
    SL4_PowerOff( pHead );

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
    SL4_PowerOn( pHead );

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
    PCOM_INFO   pSerHead = (PCOM_INFO)pHead;

    // Disallow multiple simultaneous opens
    if ( pSerHead->OpenCount )
        return (FALSE);

    pSerHead->OpenCount++;

    // NOTE: - If we wanted to support 16450s, we'll could dynamically
    // identify them here.
    pSerHead->ser16550.ChipID = CHIP_ID_16550;

    // Init 16550 info
    DEBUGMSG (ZONE_OPEN, (TEXT("ExSerOpen - Calling SL_Open\r\n")));
    SL4_Open( pHead );

    return (TRUE);
}

const HW_VTBL ISA_SerIoVTbl =
{
    ExSerInit,
    SL4_PostInit,
    ExSerDeinit,
    ExSerOpen,
    ExSerClose,
    SL4_GetInterruptType,
    SL4_RxIntr,
    SL4_TxIntrEx,
    SL4_ModemIntr,
    SL4_LineIntr,
    SL4_GetRxBufferSize,
    ExSerPowerOff,
    ExSerPowerOn,
    SL4_ClearDTR,
    SL4_SetDTR,
    SL4_ClearRTS,
    SL4_SetRTS,
    ExSerEnableIR,
    ExSerDisableIR,
    SL4_ClearBreak,
    SL4_SetBreak,
    SL4_XmitComChar,
    SL4_GetStatus,
    SL4_Reset,
    SL4_GetModemStatus,
    ExSerGetCommProperties,
    SL4_PurgeComm,
    SL4_SetDCB,
    SL4_SetCommTimeouts,
	SL4_Ioctl 
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
	pSerObj->pFuncTbl = (HW_VTBL *)&ISA_SerIoVTbl;	// UART0 - UART3 in ISA bus

    // Now return this structure to the MDD.
    return (pSerObj);
}


// ----- end of isa_com.c ------