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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*

  Copyright(c) 1998,1999 Renesas Technology Corp.

	Module Name:

		CCSer_pdd.c

	Revision History:

		26th April 1999		Released

*/
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <serhw.h>
#include <ser16550.h>
#include <hw16550.h>
#include <serdbg.h>
#include <nkintr.h>

#include "ser_pdd.h"

//#include "mobytel.h"
#include "cc.h"
//#include "shx.h"
#include "macros.h"
// #define DBGMSG
#define DBGMSG NKDbgPrintfW

#define BAUD_TABLE_SIZE 23
static const
PAIRS	BaudPairs[BAUD_TABLE_SIZE] =	{
    {50,	    2307},
    {75,	    1538},
    {110,	    1049},
    {135,	    858},
    {150,	    769},
    {300,	    384},
    {600,	    192},
    {1200,	    96},
    {1800,	    64},
    {2000,	    58},
    {2400,	    48},
    {3600,	    32},
    {4800,	    24},
    {7200,	    16},
    {9600,	    12},
    {12800,	    9},
    {14400,	    8},
    {19200,     6},
    {23040,     5},
    {28800,     4},
    {38400,     3},
    {57600,     2},
    {115200,    1}
};


static const LOOKUP_TBL  BaudTable = {BAUD_TABLE_SIZE, (PAIRS *) BaudPairs};
// Miscellaneous internal routines.
PUCHAR
static
CCSer_InternalMapRegisterAddresses(
    ULONG   HWAddress,
    ULONG   Size
    )
{
	PUCHAR	ioPortBase; 

    DEBUGMSG(ZONE_FUNCTION, 
             (TEXT("+CCSer_InternalMapRegisterAddresses: adr=0x%x len=0x%x\r\n"),
			 HWAddress, Size));

	ioPortBase = VirtualAlloc(0, Size, MEM_RESERVE, PAGE_NOACCESS);
	if ( ioPortBase == NULL )
	{
		ERRORMSG(1, (TEXT("CCSer_InternalMapRegisterAddresses: VirtualAlloc failed!\r\n")));
	}
	else if ( !VirtualCopy((PVOID)ioPortBase, (PVOID)HWAddress, Size, PAGE_READWRITE|PAGE_NOCACHE) )
	{
		ERRORMSG(1, (TEXT("CCSer_InternalMapRegisterAddresses: VirtualCopy failed!\r\n")));
		VirtualFree( (PVOID)ioPortBase, 0, MEM_RELEASE );
		ioPortBase = 0;
	}

    DEBUGMSG(ZONE_FUNCTION, 
             (TEXT("-CCSer_InternalMapRegisterAddresses: mapped at 0x%x\r\n"),
              ioPortBase ));

    return ioPortBase;
}

static
BOOL
CCSerSetIRBaudRate(
    PSER_INFO   pHWHead,
    ULONG baud     // @parm     UINT16 what is the baud rate
    )
{
    DEBUGMSG (ZONE_INIT, (TEXT("Serial set IR Baud %d\r\n"),
                          baud));
        
    return TRUE;
}

/*
 *  NOTE : The caller should have set pHWHead->fIRMode.  It is not
 * set here, since power on/off may need to temporarily disable
 * the intefaces without actually overwriting the current recorded
 * mode.
 */
static
void
CCSerSetOutputMode(
    PSER_INFO   pHWHead,
    BOOL UseIR,     // @parm     BOOL Should we use IR interface
    BOOL Use9Pin    // @parm     BOOL Should we use Wire interface
    )
{
     // TODO - here you need to set the interface to either IR mode
     // or normal serial. Note that it is possible for both BOOls to
     // be false (i.e. power down), but never for both to be TRUE.
}
/*++
*******************************************************************************
Routine:

    Ser_GetRegistryData

Description:

    Take the registry path provided to COM_Init and use it to find this 
    requested comm port's DeviceArrayIndex, teh IOPort Base Address, and the
   Interrupt number.
   
Arguments:

    LPCTSTR regKeyPath  the registry path passed in to COM_Init.

Return Value:

    -1 if there is an error.

*******************************************************************************
--*/
BOOL
Ser_GetRegistryData(PSER_INFO pHWHead, LPCTSTR regKeyPath)
{
#define GCI_BUFFER_SIZE 256   

    LONG    regError;
    HKEY    hKey;
    DWORD   dwDataSize = GCI_BUFFER_SIZE;
    DDKISRINFO dii;
    DDKWINDOWINFO dwi;

    DEBUGMSG(ZONE_INIT, (TEXT("Try to open %s\r\n"), regKeyPath));

    // We've been handed the name of a key in the registry that was generated
    // on the fly by device.exe.  We're going to open that key and pull from it
    // a value that is the name of this serial port's real key.  That key
    // will have the DeviceArrayIndex that we're trying to find.  
    hKey = OpenDeviceKey(regKeyPath);
    if ( hKey == NULL ) {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR,
                 (TEXT("Failed to open device key\r\n")));
        return ( FALSE );        
    }

    // read interrupt configuration parameters
    dii.cbSize = sizeof(dii);
    regError = DDKReg_GetIsrInfo(hKey, &dii);
    if(regError == ERROR_SUCCESS) {
    	if(dii.dwSysintr != SYSINTR_NOP) {
    		pHWHead->dwSysIntr = dii.dwSysintr;
    	} else {
    		regError = ERROR_FILE_NOT_FOUND;
    	}
        pHWHead->ser16550.dwSysIntr = pHWHead->dwSysIntr;     
   		pHWHead->ser16550.dwIrq = dii.dwIrq;
        if(dii.szIsrDll[0] == 0) {
            pHWHead->ser16550.RegIsrDll[0]=0;
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (_T("COM16550:GetRegistryConfig: no ISR DLL specified\r\n")));
        } else {
            _tcscpy(pHWHead->ser16550.RegIsrDll, dii.szIsrDll);
        }
        if(dii.szIsrHandler[0] == 0) {
            pHWHead->ser16550.RegIsrHandler[0]=0;
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (_T("COM16550:GetRegistryConfig: no ISR handler specified\r\n")));
        } else {
            _tcscpy(pHWHead->ser16550.RegIsrHandler, dii.szIsrHandler);
        }         
    }
    
    if ( regError == ERROR_SUCCESS ) {
    	dwi.cbSize = sizeof(dwi);
    	regError = DDKReg_GetWindowInfo(hKey, &dwi);
    	if(regError == ERROR_SUCCESS) {
    		if(dwi.dwNumMemWindows == 1) {
    			pHWHead->dwMemBase = dwi.memWindows[0].dwBase;
    			pHWHead->dwMemLen = dwi.memWindows[0].dwLen;
    		} else {
    			regError = ERROR_FILE_NOT_FOUND;
    		}
    	}

    }

    RegCloseKey (hKey);

    if ( regError != ERROR_SUCCESS ) {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR,
                 (TEXT("Failed to get serial registry values, Error 0x%X\r\n"),
                  regError));
        return ( FALSE );
    }

    DEBUGMSG ( ZONE_INIT,
              (TEXT("SerInit - SysIntr %d, HD64465 UART base addr %X \r\n"),
               pHWHead->dwSysIntr, pHWHead->dwMemBase));
    return ( TRUE ); 
}


/*
 @doc OEM 
 @func PVOID | CCSerInit | Initializes device identified by argument.
 *  This routine sets information controlled by the user
 *  such as Line control and baud rate. It can also initialize events and
 *  interrupts, thereby indirectly managing initializing hardware buffers.
 *  Exported only to driver, called only once per process.
 *
 @rdesc The return value is a PVOID to be passed back into the HW
 dependent layer when HW functions are called.
 */
static
PVOID
CCSerInit(
    ULONG   Identifier,	// @parm Device identifier.
    PVOID	pMddHead,	// @parm First argument to mdd callbacks.
    PHWOBJ  pHWObj      // @parm Pointer to our own HW OBJ for this device
    )
{
    PSER_INFO   pHWHead;

     // Allocate for our main data structure and one of it's fields.
    pHWHead = (PSER_INFO)LocalAlloc( LMEM_ZEROINIT|LMEM_FIXED ,
                                         sizeof(SER_INFO) );
    if (!pHWHead)
        goto ALLOCFAILED;
    
    pHWHead->dwSysIntr = pHWObj->dwIntID; // Initial to default vaule;
    pHWHead->pHWObj = pHWObj;
    if ( ! Ser_GetRegistryData(pHWHead, (LPCTSTR)Identifier) ) {
        DEBUGMSG (ZONE_INIT|ZONE_ERROR,
                  (TEXT("SerInit - Unable to read registry data.  Failing Init !!! \r\n")));
        goto ALLOCFAILED;
    }
    pHWObj->dwIntID = pHWHead->dwSysIntr;
     // This call will map the address space for the 16550 UART.  
    pHWHead->pBaseAddress   = CCSer_InternalMapRegisterAddresses(
        pHWHead->dwMemBase, 0x400);
    
    pHWHead->pMddHead	  = pMddHead;

    pHWHead->cOpenCount   = 0;

     // Set up our Comm Properties data    
    pHWHead->CommProp.wPacketLength       = 0xffff;
    pHWHead->CommProp.wPacketVersion     = 0xffff;
    pHWHead->CommProp.dwServiceMask      = SP_SERIALCOMM;
    pHWHead->CommProp.dwReserved1	      = 0;
    pHWHead->CommProp.dwMaxTxQueue	      = 16;
    pHWHead->CommProp.dwMaxRxQueue	      = 16;
    pHWHead->CommProp.dwMaxBaud	      = BAUD_115200;
    pHWHead->CommProp.dwProvSubType      = PST_RS232;
    pHWHead->CommProp.dwProvCapabilities =
        PCF_DTRDSR | PCF_RLSD | PCF_RTSCTS |
        PCF_SETXCHAR |
        PCF_INTTIMEOUTS |
        PCF_PARITY_CHECK |
        PCF_SPECIALCHARS |
        PCF_TOTALTIMEOUTS |
        PCF_XONXOFF;
    pHWHead->CommProp.dwSettableBaud      =
		BAUD_075 | BAUD_110 | BAUD_150 | BAUD_300 | BAUD_600 |
		BAUD_1200 | BAUD_1800 |	BAUD_2400 | BAUD_4800 |
		BAUD_7200 | BAUD_9600 | BAUD_14400 |
		BAUD_19200 | BAUD_38400 | BAUD_56K | BAUD_128K |
		BAUD_115200 | BAUD_57600 | BAUD_USER;
    pHWHead->CommProp.dwSettableParams    =
        SP_BAUD | SP_DATABITS | SP_HANDSHAKING | SP_PARITY |
        SP_PARITY_CHECK | SP_RLSD | SP_STOPBITS;
    pHWHead->CommProp.wSettableData       =
        DATABITS_5 | DATABITS_6 | DATABITS_7 | DATABITS_8;
    pHWHead->CommProp.wSettableStopParity =
        STOPBITS_10 | STOPBITS_20 |
        PARITY_NONE | PARITY_ODD | PARITY_EVEN | PARITY_SPACE |
        PARITY_MARK;

     // Init 16550 info, register stride is 1 
    DEBUGMSG (ZONE_INIT, (TEXT("CCSerInit - Init 16550 data\r\n")));
    SL_Init( pHWHead, pHWHead->pBaseAddress, CC_16550_REG_STRIDE,
             EvaluateEventFlag, pMddHead, (PLOOKUP_TBL)&BaudTable);

    DEBUGMSG (ZONE_INIT,
              (TEXT("CCSerInit - Disabling UART Power\r\n")));
    pHWHead->fIRMode  = FALSE;   // Select wired by default
    CCSerSetOutputMode(pHWHead, FALSE, FALSE );    
    
    return pHWHead;
 
ALLOCFAILED:
    if (pHWHead != NULL) {
        if (pHWHead->pBaseAddress) {
            VirtualFree(pHWHead->pBaseAddress, 0, MEM_RELEASE);
        }
        LocalFree(pHWHead);
    }
    return NULL;
}

/*
 @doc OEM
 @func ULONG | CCSerClose | This routine closes the device identified by the PVOID returned by CCSerInit.
 *  Not exported to users, only to driver.
 *
 @rdesc The return value is 0.
 */
static
ULONG
CCSerClose(
    PVOID   pHead	// @parm PVOID returned by CCSerInit.
    )
{
    PSER_INFO	pHWHead = (PSER_INFO)pHead;
    ULONG  uTries;

    DEBUGMSG (ZONE_CLOSE,(TEXT("+CCSerClose\r\n")));
    if( pHWHead->cOpenCount )
    {
        DEBUGMSG (ZONE_CLOSE, 
                  (TEXT("CCSerClose, closing device\r\n")));
        pHWHead->cOpenCount--;
        
         // while we are still transmitting, sleep.
        uTries = 0;
        while ( (uTries++ < 100) &&
                !(pHWHead->ser16550.LSR & SERIAL_LSR_TEMT) )
        {
            DEBUGMSG (ZONE_CLOSE, 
			(TEXT("CCSerClose, TX in progress, LSR 0x%X\r\n"),
                       pHWHead->ser16550.LSR));
            Sleep(10);
        }

         // When the device is closed, we power it down.
        DEBUGMSG (ZONE_CLOSE, 
                  (TEXT("CCSerClose - Powering down UART\r\n")));
        pHWHead->fIRMode  = FALSE;  
        CCSerSetOutputMode(pHWHead, FALSE, FALSE );
		
    }
    SL_Close(pHead);
    DEBUGMSG (ZONE_CLOSE,(TEXT("-CCSerClose\r\n")));
    return 0;
}

/*
 @doc OEM 
 @func PVOID | CCSerDeinit | Deinitializes device identified by argument.
 *  This routine frees any memory allocated by CCSerInit.
 *
 */
static
BOOL
CCSerDeinit(
    PVOID   pHead	// @parm PVOID returned by CCSerInit.
    )
{
    PSER_INFO	pHWHead = (PSER_INFO)pHead;
    
    if ( !pHWHead )
        return FALSE;

     // Make sure device is closed before doing DeInit
    if( pHWHead->cOpenCount )
        CCSerClose( pHead );

    if ( pHWHead->pBaseAddress )
        VirtualFree(pHWHead->pBaseAddress, 0, MEM_RELEASE);
    
    LocalFree(pHWHead);

    return TRUE;
}

/*
 @doc OEM
 @func	VOID | CCSerGetCommProperties | Retrieves Comm Properties.
 *
 @rdesc	None.
 */
static
VOID
CCSerGetCommProperties(
    PVOID	pHead,	    // @parm PVOID returned by CCSerInit. 
    LPCOMMPROP	pCommProp   // @parm Pointer to receive COMMPROP structure. 
    )
{
    PSER_INFO	pHWHead = (PSER_INFO)pHead;

    *pCommProp = pHWHead->CommProp;
    return;
}


/*
 @doc OEM
 @func VOID | CCSerSetBaudRate |
 * This routine sets the baud rate of the device.
 *  Not exported to users, only to driver.
 *
 @rdesc None.
 */
static
BOOL
CCSerSetBaudRate(
    PVOID   pHead,	// @parm     PVOID returned by CCSerInit
    ULONG   BaudRate	// @parm     ULONG representing decimal baud rate.
    )
{
    PSER_INFO	pHWHead = (PSER_INFO)pHead;

     // If we are running in IR mode, try to set the IR baud
     // first, since it supports a subset of the rates supported
     // by the UART.  If we fail setting the IR rate, then
     // return an error and leave the UART alone.
    if( pHWHead->fIRMode )
    {
        if( ! CCSerSetIRBaudRate( pHWHead, BaudRate ) )
        {
            DEBUGMSG (ZONE_ERROR, 
                      (TEXT("Unsupported IR BaudRate\r\n")));
             // We should return an error, but vtbl doesn't expect one
            return FALSE; 
        }
    }

     // Now set buadrate on the UART
    return( SL_SetBaudRate( pHead, BaudRate ) );    
}

/*
 @doc OEM
 @func BOOL | CCSerPowerOff |
 *  Called by driver to turn off power to serial port.
 *  Not exported to users, only to driver.
 *
 @rdesc This routine returns a status.
 */
static
BOOL
CCSerPowerOff(
    PVOID   pHead	    // @parm	PVOID returned by CCSerInit.
    )
{
    PSER_INFO	pHWHead = (PSER_INFO)pHead;

     // First, power down the UART
    SL_PowerOff( pHWHead );
    
     // And then disable our IR and 9 Pin interface
    CCSerSetOutputMode( pHWHead, FALSE, FALSE );
    
    return TRUE;
}
 
/*
 @doc OEM
 @func BOOL | CCSerPowerOn |
 *  Called by driver to turn on power to serial port.
 *  Not exported to users, only to driver.
 *
 @rdesc This routine returns a status.
 */
static
BOOL
CCSerPowerOn(
    PVOID   pHead	    // @parm	PVOID returned by CCSerInit.
    )
{
    PSER_INFO	pHWHead = (PSER_INFO)pHead;

     // First, power up the UART
    SL_PowerOn( pHWHead );
    
     // And then enable our IR interface (if needed)
    CCSerSetOutputMode( pHWHead, pHWHead->fIRMode, !pHWHead->fIRMode );
    return TRUE;
}

/*
 @doc OEM
 @func BOOL | CCSerEnableIR | This routine enables ir.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSEotherwise.
 */
static
BOOL
CCSerEnableIR(
    PVOID   pHead, // @parm PVOID returned by CCSerinit.
    ULONG   BaudRate  // @parm PVOID returned by HWinit.
    )
{
    PSER_INFO	pHWHead = (PSER_INFO)pHead;

     // Not sure why he passes baudRate when its already in our
     // pHWHead.  So I'll ignore it and use the one in our struct.
    pHWHead->fIRMode  = TRUE;
    CCSerSetOutputMode( pHWHead, pHWHead->fIRMode, !pHWHead->fIRMode );
    return TRUE;
}

/*
 @doc OEM
 @func BOOL | CCSerDisableIR | This routine disable the ir.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSEotherwise.
 */
static
BOOL
CCSerDisableIR(
    PVOID   pHead /*@parm PVOID returned by CCSerinit. */
    )
{
    PSER_INFO	pHWHead = (PSER_INFO)pHead;

    pHWHead->fIRMode  = FALSE;
    CCSerSetOutputMode( pHWHead, pHWHead->fIRMode, !pHWHead->fIRMode );
    return TRUE;
}

/*
 @doc OEM
 @func BOOL | CCSerOpen | This routine is called when the port is opened.
 *  Not exported to users, only to driver.
 *
 @rdesc Returns TRUE if successful, FALSEotherwise.
 */
static
BOOL
CCSerOpen(
    PVOID   pHead /*@parm PVOID returned by CCSerinit. */
    )
{

    PSER_INFO	pHWHead = (PSER_INFO)pHead;

     // Disallow multiple simultaneous opens
    if( pHWHead->cOpenCount )
        return FALSE;

    pHWHead->cOpenCount++;

    DEBUGMSG (ZONE_OPEN,
              (TEXT("CCSerOpen - Selecting Non IR Mode\r\n")));

    pHWHead->fIRMode  = FALSE;   // Select wired by default
    CCSerSetOutputMode(pHWHead, pHWHead->fIRMode, !pHWHead->fIRMode );  

    // BUGBUG - If we want to support 16450s, we'll need to dynamically
    // identify them here.
    pHWHead->ser16550.ChipID = CHIP_ID_16550;

     // Init 16550 info
    DEBUGMSG (ZONE_OPEN, (TEXT("CCSerOpen - Calling SL_Open\r\n")));
    SL_Open( pHWHead );
    
    return TRUE;
}
const
HW_VTBL CCUARTIoVTbl= {
    CCSerInit,
    SL_PostInit,        
    CCSerDeinit,
    CCSerOpen,
    CCSerClose,
    SL_GetInterruptType,
    SL_RxIntr,
    SL_TxIntrEx,
    SL_ModemIntr,
    SL_LineIntr,
    SL_GetRxBufferSize,
    CCSerPowerOff,
    CCSerPowerOn,
    SL_ClearDTR,
    SL_SetDTR,
    SL_ClearRTS,
    SL_SetRTS,
    CCSerEnableIR,
    CCSerDisableIR,
    SL_ClearBreak,
    SL_SetBreak,
    SL_XmitComChar,
    SL_GetStatus,
    SL_Reset,
    SL_GetModemStatus,
    CCSerGetCommProperties,
    SL_PurgeComm,
    SL_SetDCB,
    SL_SetCommTimeouts,
    SL_Ioctl
    };

// GetSerialObj : The purpose of this function is to allow multiple PDDs to be
// linked with a single MDD creating a multiport driver.  In such a driver, the
// MDD must be able to determine the correct vtbl and associated parameters for
// each PDD.  Immediately prior to calling HWInit, the MDD calls GetSerialObject
// to get the correct function pointers and parameters.
//
HWOBJ   SerObj      = {
    THREAD_AT_INIT,
    0,
    (PHW_VTBL) &CCUARTIoVTbl
};

PHWOBJ
GetSerialObject(
               DWORD DeviceArrayIndex
               )
{
    return (&SerObj);
}


