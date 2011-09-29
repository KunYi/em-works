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
//------------------------------------------------------------------------------
//
//  The file implements HDQ device driver.
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap2420.h>
#include <CEDDKEX.H>
#include <buses.h>
#include <hdq.h>

/*
#undef DEBUGMSG
#define DEBUGMSG(x, y) RETAILMSG(1,y)

#ifdef RETAIL_DEBUG
#pragma optimize("", off)           // debug
#endif
*/
//------------------------------------------------------------------------------
// Global Variables

const GUID DEVICE_IFC_HDQ_GUID;
const long int cliDelay1=100; // 250 first hdq op dealy in msec 
const long int cliDelay2=100; // msec 
const long int cliDelay2A=100; // msec 
const long int cliDelay3=100; // msec 


#ifdef DEBUG

DBGPARAM dpCurSettings = {
    L"System Buses", {
        L"Errors",      L"Warnings",    L"Function",    L"Init",
        L"Info",        L"IST",         L"Power",       L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"HDQ",         L"I2C",         L"GPIO",        L"BUS"
    },
    0x0003
};

#endif


//------------------------------------------------------------------------------
//  Local Definitions

#define HDQ_DEVICE_COOKIE       'hdqD'

typedef struct {
    DWORD cookie;
    DWORD memBase;
    DWORD memLen;
    DWORD irq;
    DWORD breakTimeout;
    DWORD txTimeout;
    DWORD rxTimeout;
    OMAP2420_PRCM_REGS* pPRCMRegs;
    OMAP2420_HDQ_REGS* pHDQRegs; // uses OMAP2420_HDQ_1WIRE_REGS_PA
    HANDLE hParentBus;
    LONG instances;
    CRITICAL_SECTION cs;
    DWORD sysIntr;
    HANDLE hIntrEvent;
    DWORD mode;
    CEDEVICE_POWER_STATE powerState;
} HDQ_DEVICE;

BOOL bDumpOmapHdqRegs(HDQ_DEVICE* const pDevice);

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"MemBase", PARAM_DWORD, TRUE, offset(HDQ_DEVICE, memBase),
        fieldsize(HDQ_DEVICE, memBase), NULL // (VOID*)0x480B2000
    }, {
        L"MemLen", PARAM_DWORD, TRUE, offset(HDQ_DEVICE, memLen),
        fieldsize(HDQ_DEVICE, memLen), NULL // (VOID*)0x00001000
    }, {
        L"Irq", PARAM_DWORD, TRUE, offset(HDQ_DEVICE, irq),
        fieldsize(HDQ_DEVICE, irq), NULL // (VOID*)0x3A
    }, {
        L"BreakTimeout", PARAM_DWORD, FALSE, offset(HDQ_DEVICE, breakTimeout),
        fieldsize(HDQ_DEVICE, breakTimeout), (VOID*)1	// msec
    }, {
        L"TxTimeout", PARAM_DWORD, FALSE, offset(HDQ_DEVICE, txTimeout),
        fieldsize(HDQ_DEVICE, txTimeout), (VOID*)5	// in msec, > 8*190 microsec in spec
    }, {
        L"RxTimeout", PARAM_DWORD, FALSE, offset(HDQ_DEVICE, rxTimeout),
        fieldsize(HDQ_DEVICE, rxTimeout), (VOID*)5	// in ms, > 20+8*190=1840 microsec
    }
};

//------------------------------------------------------------------------------
//  Local Functions

BOOL HDQ_Deinit(DWORD context);
BOOL HDQ_Write(DWORD context, UCHAR address, USHORT data);
BOOL HDQ_Read(DWORD context, UCHAR address, USHORT *pData);
BOOL HDQ_SetMode(DWORD context, DWORD mode);


/* VOID* HdqOpen(void)
 * {
 *     HANDLE hDevice;
 *     DEVICE_CONTEXT_HDQ* pContext = NULL;
 * 
 *     hDevice=CreateFile(HDQ_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
 *     if(hDevice==INVALID_HANDLE_VALUE) goto clean;
 * 
 * // Allocate memory for our handler...
 *     if((pContext = (DEVICE_CONTEXT_HDQ *)LocalAlloc(
 *         LPTR, sizeof(DEVICE_CONTEXT_HDQ) )) == NULL)
 * 	{
 *         CloseHandle(hDevice);
 *         goto clean;
 *     };
 * 
 * // Get function pointers, fail when IOCTL isn't supported...
 *     if (DeviceIoControl(
 *         hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_HDQ_GUID,
 *         sizeof(DEVICE_IFC_HDQ_GUID), &pContext->ifc, sizeof(DEVICE_IFC_HDQ), 
 *         NULL, NULL)==0)
 *      {
 *         CloseHandle(hDevice);
 *         LocalFree(pContext);
 *         pContext = NULL;
 *         goto clean;
 *     };
 * 
 * // Save device handle
 *     pContext->hDevice=hDevice;
 * 
 * clean:
 *     return pContext;
 * }
 * 
 * VOID HdqClose(HANDLE hContext)
 * {
 *     DEVICE_CONTEXT_HDQ* pContext=(DEVICE_CONTEXT_HDQ*)hContext;
 *     CloseHandle(pContext->hDevice);
 *     LocalFree(pContext);
 * }
 * 
 * BOOL HdqWrite8(HANDLE hContext, UCHAR address, UCHAR data)
 * {
 *     DEVICE_CONTEXT_HDQ *pContext=(DEVICE_CONTEXT_HDQ*)hContext;
 *     return pContext->ifc.pfnWrite(
 *         pContext->ifc.context, address, (USHORT)data);
 * }
 * 
 * BOOL HdqRead8(HANDLE hContext, UCHAR address, UCHAR* pData)
 * {
 *     DEVICE_CONTEXT_HDQ* pContext=(DEVICE_CONTEXT_HDQ*)hContext;
 *     BOOL rc;
 *     USHORT data;
 * 
 *     if((rc=pContext->ifc.pfnRead(
 *         pContext->ifc.context, address, &data)))
 *     *pData=(UCHAR)data;
 *     return rc;
 * }
 * 
 * BOOL HdqWrite16(HANDLE hContext, UCHAR address, USHORT data)
 * {
 *     DEVICE_CONTEXT_HDQ* pContext = (DEVICE_CONTEXT_HDQ *)hContext;
 *     return pContext->ifc.pfnWrite(
 *         pContext->ifc.context, address, data);
 * }
 * 
 * BOOL HdqRead16(HANDLE hContext, UCHAR address, USHORT *pData)
 * {
 *     DEVICE_CONTEXT_HDQ *pContext = (DEVICE_CONTEXT_HDQ*)hContext;
 *     
 *     return pContext->ifc.pfnRead(pContext->ifc.context, address, pData);
 * }
 * 
 * VOID HdqSetMode(HANDLE hContext, DWORD mode)
 * {
 *     DEVICE_CONTEXT_HDQ *pContext = (DEVICE_CONTEXT_HDQ*)hContext;
 *     pContext->ifc.pfnSetMode(pContext->ifc.context, mode);
 * }
 */


//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  Standard Windows DLL entry point.
//
BOOL __stdcall DllMain(HANDLE hDLL, DWORD reason, VOID *pReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER(hDLL);
        DisableThreadLibraryCalls((HMODULE)hDLL);
        break;
    }
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  HDQ_Init
//
//  Called by device manager to initialize device.
//
DWORD HDQ_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    DWORD rc=(DWORD)NULL;
    HDQ_DEVICE* pDevice=NULL;
    PHYSICAL_ADDRESS pa;
    int i=0;

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"+HDQ_Init(%s, 0x%08x)\r\n", szContext, pBusContext));

// Create device structure
    pDevice=(HDQ_DEVICE*)LocalAlloc(LPTR, sizeof(HDQ_DEVICE));
    if(pDevice==NULL)
    {
        DWORD dwEr=GetLastError();
        DEBUGMSG(ZONE_ERROR, (L"ERROR %lu: HDQ_Init: "
            L"Failed allocate HDQ controller structure\r\n",dwEr
        ));
        goto cleanUp;
    };

// Set cookie
    pDevice->cookie=HDQ_DEVICE_COOKIE;

// Initalize critical section
    InitializeCriticalSection(&pDevice->cs);

// Read device parameters
    if(GetDeviceRegistryParams(szContext, pDevice, 
            dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Init: "
            L"Failed read HDQ driver registry parameters\r\n"
        ));
        goto cleanUp;
    };

// Open parent bus
    pDevice->hParentBus=CreateBusAccessHandle(szContext);
    if(pDevice->hParentBus==NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Init: "
            L"Failed open parent bus driver\r\n"
        ));
        goto cleanUp;
    };

// Set hardware to full power
    SetDevicePowerState(pDevice->hParentBus, D0, NULL);

// Map the PRCM registers
    pa.LowPart=OMAP2420_PRCM_REGS_PA;
    pDevice->pPRCMRegs=(OMAP2420_PRCM_REGS*)MmMapIoSpace(pa,
        sizeof(OMAP2420_PRCM_REGS), FALSE);
    if(pDevice->pPRCMRegs==NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Init: "
            L"Failed to map PRCM registers\n"
        ));
        goto cleanUp;
    };

// Map the HDQ registers
    pa.QuadPart=pDevice->memBase;
    pDevice->pHDQRegs=(OMAP2420_HDQ_REGS*)MmMapIoSpace(pa, 
        sizeof(OMAP2420_HDQ_REGS), FALSE);
    if(pDevice->pHDQRegs==NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Init: "
            L"Failed map HDQ controller registers\r\n"
        ));
        goto cleanUp;
    };

// Map HDQ_1WIRE interrupt
    if(KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &pDevice->irq, 
        sizeof(pDevice->irq), &pDevice->sysIntr,
        sizeof(pDevice->sysIntr), NULL)==0)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Init: "
            L"Failed map HDQ/1WIRE controller interrupt\r\n"
        ));
        goto cleanUp;
    };

// Create interrupt event
    pDevice->hIntrEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
    if(pDevice->hIntrEvent==NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Init: "
            L"Failed create interrupt event\r\n"
        ));
        goto cleanUp;
    };

// Initialize interrupt
    if(InterruptInitialize(pDevice->sysIntr, 
        pDevice->hIntrEvent, NULL, 0)==0)
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: HDQ_Init: "
            L"InterruptInitialize failed\r\n"
        ));
        goto cleanUp;
    };

    DEBUGMSG(ZONE_ERROR, (L"\tHDQ_Init: "
        L"Resetting OMAP HDQ module...\r\n"
    ));
// reset the hdq device
/*  SETREG32(&pDevice->pHDQRegs->ulHDQ_SYSCONFIG,HDQ_SYSCONFIG_SOFTRESET);
// wait for the reset to be completed
    for(; i<50; i++)
    {
        Sleep(10);
        if( INREG32(&pDevice->pHDQRegs->ulHDQ_SYSSTATUS) & HDQ_SYSSTATUS_RESETDONE)
        break;
    };
    if(i>=50)
        DEBUGMSG(ZONE_HDQ_1WIRE, (L"\tHDQ_Init: "
            L"Reset of OMAP HDQ module failed !!\r\n"
        ));
    else
        DEBUGMSG(ZONE_HDQ_1WIRE, (L"\tHDQ_Init: "
            L"Reset of OMAP HDQ module done. Dur=%ld\r\n", i*10
        ));
*/
// set the HDQ mode of operation - it is the default for OMAP HDQ, but must be sure
    CLRREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS,HDQ_CTRL_STATUS_1_WIRE_SINGLE_BIT);
// Enable the clock and set the mode
    SETREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_CLOCKENABLE);
    CLRREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_MODE);
    pDevice->mode=HDQ_MODE_HDQ8;

// Clear and enable interrupts
    INREG32(&pDevice->pHDQRegs->ulHDQ_INT_STATUS);
    SETREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_INTERRUPTMASK);

// Set hardware to power-off
    SetDevicePowerState(pDevice->hParentBus, D4, NULL);

// Return non-null value - hdq init completed successfully
    rc=(DWORD)pDevice;
// sr dump hdq registers
    bDumpOmapHdqRegs(pDevice);
cleanUp:
    if(rc==0)
    {
        HDQ_Deinit((DWORD)pDevice);
    };    
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"---HDQ_Init(rc=0x%x) and HDQ power is OFF now.\r\n", rc));
    return rc;
}

BOOL bDumpOmapHdqRegs(HDQ_DEVICE* const pDevice)
{
    DWORD dwReg=0;
    DEBUGMSG(ZONE_INFO, (L"+++bDumpOmapHdqRegs()\r\n- - - - - - Hdq Registers - - - - - - - -\r\n"));

    dwReg=INREG32(&pDevice->pHDQRegs->ulHDQ_REVISION);
    DEBUGMSG(ZONE_INFO, (L"\tHDQ_REVISION=%08lXh\r\n",dwReg));
    dwReg=INREG32(&pDevice->pHDQRegs->ulHDQ_TX_DATA);
    DEBUGMSG(ZONE_INFO, (L"\tHDQ_TX_DATA=%08lXh\r\n",dwReg));
    dwReg=INREG32(&pDevice->pHDQRegs->ulHDQ_RX_DATA);
    DEBUGMSG(ZONE_INFO, (L"\tHDQ_RX_DATA=%08lXh\r\n",dwReg));
    dwReg=INREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS);
    DEBUGMSG(ZONE_INFO, (L"\tHDQ_CTRL_STATUS=%08lXh\r\n",dwReg));
    dwReg=INREG32(&pDevice->pHDQRegs->ulHDQ_INT_STATUS);
    DEBUGMSG(ZONE_INFO, (L"\tHDQ_INT_STATUS=%08lXh\r\n",dwReg));
    dwReg=INREG32(&pDevice->pHDQRegs->ulHDQ_SYSCONFIG);
    DEBUGMSG(ZONE_INFO, (L"\tHDQ_SYSCONFIG=%08lXh\r\n",dwReg));
    dwReg=INREG32(&pDevice->pHDQRegs->ulHDQ_SYSSTATUS);
    DEBUGMSG(ZONE_INFO, (L"\tHDQ_SYSSTATUS=%08lXh\r\n",dwReg));

    DEBUGMSG(ZONE_INFO, (L"---bDumpOmapHdqRegs()\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL HDQ_Deinit(DWORD context)
{
    BOOL rc=FALSE;
    HDQ_DEVICE* pDevice=(HDQ_DEVICE*)context;

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"+HDQ_Deinit(0x%08x)\r\n", context));

// Check if we get correct context
    if(pDevice == NULL || pDevice->cookie != HDQ_DEVICE_COOKIE)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Deinit: "
            L"Incorrect context paramer\r\n"));
        goto cleanUp;
    };

// Disable and clear the interrupt
    CLRREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_INTERRUPTMASK);
    INREG32(&pDevice->pHDQRegs->ulHDQ_INT_STATUS);

// Set hardware to D4 and close parent bus driver
    if(pDevice->hParentBus!=NULL)
    {
        SetDevicePowerState(pDevice->hParentBus, D4, NULL);
        CloseBusAccessHandle(pDevice->hParentBus);
    };

// Delete critical section
    DeleteCriticalSection(&pDevice->cs);

// Unmap PRCM controller registers
    if(pDevice->pPRCMRegs!=NULL)
    {
        MmUnmapIoSpace((VOID*)pDevice->pPRCMRegs, sizeof(OMAP2420_PRCM_REGS));
    };

// Unmap HDQ_1Wire controller registers
    if(pDevice->pHDQRegs != NULL)
    {
        MmUnmapIoSpace((VOID*)pDevice->pHDQRegs, pDevice->memLen);
    };

// Release HDQ_1Wire controller interrupt
    if(pDevice->sysIntr != 0)
    {
        InterruptDisable(pDevice->sysIntr);
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pDevice->sysIntr,sizeof(pDevice->sysIntr), NULL, 0, NULL);
    };

// Close interrupt handler
    if(pDevice->hIntrEvent!=NULL)
    {
        CloseHandle(pDevice->hIntrEvent);
    };

// Free device structure
    LocalFree(pDevice);

// Done
    rc=TRUE;

cleanUp:
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"---HDQ_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD HDQ_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    DWORD rc=(DWORD)-1;
    HDQ_DEVICE* pDevice=(HDQ_DEVICE*)context;

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"+++HDQ_Open(0x%08x, 0x%08x, 0x%08x\r\n", context, accessCode, shareMode));

// Check if we get correct context
    if(pDevice==NULL || pDevice->cookie!=HDQ_DEVICE_COOKIE)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Open: "
            L"Incorrect context paramer\r\n"));
        goto cleanUp;
    };

// Return device context
    rc=context;

cleanUp:
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"---HDQ_Open()\r\n"));
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_Close
//
//  This function closes the device context.
//
BOOL HDQ_Close(DWORD context)
{
    BOOL rc=FALSE;
    HDQ_DEVICE* pDevice=(HDQ_DEVICE*)context;

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"+++HDQ_Close(0x%08x)\r\n", context));

// Check if we get correct context
    if(pDevice==NULL || pDevice->cookie!=HDQ_DEVICE_COOKIE)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Close: "
            L"Incorrect context paramer\r\n"));
        goto cleanUp;
    };

// Done
    rc=TRUE;

cleanUp:
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"---HDQ_Close(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_IOControl
//
//  This function sends a command to a device.
//
BOOL HDQ_IOControl(DWORD context, DWORD code, BYTE *pInBuffer, DWORD inSize, BYTE *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    BOOL rc=FALSE;
    HDQ_DEVICE* pDevice=(HDQ_DEVICE*)context;
    DEVICE_IFC_HDQ ifc;

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"+++HDQ_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize));

// Check if we get correct context
    if(pDevice==NULL || pDevice->cookie!=HDQ_DEVICE_COOKIE)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_IOControl: "
            L"Incorrect context paramer\r\n"));
        goto cleanUp;
    };

    switch (code)
    {
    case IOCTL_DDK_GET_DRIVER_IFC:
// We can give interface only to our peer in device process
        if(GetCurrentProcessId()!=(DWORD)GetCallerProcess())
        {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_IOControl: IOCTL_DDK_GET_DRIVER_IFC can be called only from "
                L"device process (caller process id 0x%08x)\r\n", GetCallerProcess()));
            SetLastError(ERROR_ACCESS_DENIED);
            break;
        }
        if(pInBuffer==NULL || inSize < sizeof(GUID))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        };
        if(IsEqualGUID(pInBuffer, &DEVICE_IFC_HDQ_GUID))
        {
            if(pOutSize!=NULL)
            {
                *pOutSize=sizeof(DEVICE_IFC_HDQ);
            }
            if(pOutBuffer==NULL || outSize < sizeof(DEVICE_IFC_HDQ))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            ifc.context=context;
            ifc.pfnWrite=HDQ_Write;
            ifc.pfnRead=HDQ_Read;
            ifc.pfnSetMode=HDQ_SetMode;
            if(!CeSafeCopyMemory(pOutBuffer, &ifc, sizeof(DEVICE_IFC_HDQ)))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            rc=TRUE;
            break;
        };
        SetLastError(ERROR_INVALID_PARAMETER);
        break;
    default:
        DEBUGMSG(ZONE_WARN, (L"WARN: HDQ_IOControl: "
            L"Unsupported code 0x%08x\r\n", code));
        SetLastError(ERROR_INVALID_PARAMETER);
        break;
    };

cleanUp:
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"---HDQ_IOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_PowerUp
//
//  This function restores power to a device.
//
VOID HDQ_PowerUp(DWORD context)
{
    HDQ_DEVICE* pDevice=(HDQ_DEVICE*)context;
    DWORD dwRegValue=0;
    DWORD cbRet=0;
// enable the HDQ clocks
#if 0
    SETREG32(&pDevice->pPRCMRegs->ulCM_FCLKEN1_CORE, PRCM_FCLKEN1_CORE_EN_HDQ);
    SETREG32(&pDevice->pPRCMRegs->ulCM_ICLKEN1_CORE, PRCM_ICLKEN1_CORE_EN_HDQ);
#else
// Enable the 1WIRE functional and interface clocks.
    dwRegValue=PRCM_FCLKEN1_CORE_EN_HDQ;
    KernelIoControl(IOCTL_FCLK1_ENB, (VOID*)&dwRegValue, sizeof(DWORD), NULL, 0, &cbRet);

    dwRegValue=PRCM_ICLKEN1_CORE_EN_HDQ;
    KernelIoControl(IOCTL_ICLK1_ENB, (VOID*)&dwRegValue, sizeof(DWORD), NULL, 0, &cbRet);
#endif
// enable the HDQ
    SETREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_CLOCKENABLE);

// clear and enable interrupts
    INREG32(&pDevice->pHDQRegs->ulHDQ_INT_STATUS);
    SETREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_INTERRUPTMASK);
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_PowerDown
//
//  This function suspends power to the device.
//
void HDQ_PowerDown(DWORD context)
{
    HDQ_DEVICE* pDevice=(HDQ_DEVICE*)context;
    DWORD dwRegValue=0;
    DWORD cbRet=0;

// disable and clear interrupts
    CLRREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_INTERRUPTMASK);
    INREG32(&pDevice->pHDQRegs->ulHDQ_INT_STATUS);

// disable the HDQ
    CLRREG32(&pDevice->pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_CLOCKENABLE);

// disable the HDQ clocks
#if 0
    CLRREG32(&pDevice->pPRCMRegs->ulCM_FCLKEN1_CORE, PRCM_FCLKEN1_CORE_EN_HDQ);
    CLRREG32(&pDevice->pPRCMRegs->ulCM_ICLKEN1_CORE, PRCM_ICLKEN1_CORE_EN_HDQ); 
#else
// Disable the 1WIRE functional and interface clocks.
    dwRegValue=PRCM_FCLKEN1_CORE_EN_HDQ;
    KernelIoControl(IOCTL_FCLK1_DIS, (VOID*)&dwRegValue, sizeof(DWORD), NULL, 0, &cbRet);

    dwRegValue=PRCM_ICLKEN1_CORE_EN_HDQ;
    KernelIoControl(IOCTL_ICLK1_DIS, (VOID*)&dwRegValue, sizeof(DWORD), NULL, 0, &cbRet);
#endif
    return;
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_Write
//
//  This function writes a byte of data in 8 bit mode to the specified address.
//
BOOL HDQ_Write(DWORD context, UCHAR address, USHORT data)
{
    BOOL rc=FALSE;
    HDQ_DEVICE* pDevice=(HDQ_DEVICE*)context;
    OMAP2420_HDQ_REGS* pHDQRegs;
    DWORD status, i, count;
    UCHAR buffer[3];

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"+++HDQ_Write(0x%08x, 0x%02x, 0x%x)\r\n", context, address, data));

// Check if we get correct context
    if(pDevice==NULL || pDevice->cookie!=HDQ_DEVICE_COOKIE)
    {
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_ERROR, (L"ERROR: HDQ_Write: "
            L"Incorrect context parameter\r\n"));
        goto exit;
    };

// Get hardware
    pHDQRegs=pDevice->pHDQRegs;
    EnterCriticalSection(&pDevice->cs);

// Make sure that clock is present
    SetDevicePowerState(pDevice->hParentBus, D0, NULL);
    HDQ_PowerUp(context);

// Clear the interrupt
    status=INREG32(&pHDQRegs->ulHDQ_INT_STATUS);
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_Write: "
        L"Interrupt status (1) was 0x%02x\r\n", status));

// Need to notify InterruptDone right away
    InterruptDone(pDevice->sysIntr);

// Send initialization pulse in 1-wire mode
    SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_MODE);
    SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_INITIALIZATION);
    SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_GO);

// Wait on completion interrupt (not documented)
    if(WaitForSingleObject(pDevice->hIntrEvent, pDevice->breakTimeout) == WAIT_TIMEOUT)
    {
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_ERROR, (L"ERROR: HDQ_Write: "
            L"Timeout in init pulse\r\n"));
    }

// Clear the interrupt
    status=INREG32(&pHDQRegs->ulHDQ_INT_STATUS);
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_Write: "
        L"Interrupt status (2) was 0x%02x\r\n", status));

// Need to notify InterruptDone right away
    InterruptDone(pDevice->sysIntr);

// Back to HDQ mode.
    CLRREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_MODE);

// We are going to do a write so set write bit in the address
    buffer[0]=address|0x80;
    buffer[1]=(UCHAR)data;
    switch(pDevice->mode)
    {
    case HDQ_MODE_HDQ8:
        count=2;
        break;
    case HDQ_MODE_HDQ16:
        buffer[2]=(UCHAR)(data >> 8);
        count=3;
        break;
    }
// Two write cycles required
    for(i=0; i < count; i++)
    {
// Write the value, set the direction and go
        OUTREG32(&pHDQRegs->ulHDQ_TX_DATA, buffer[i]);
        SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_DIR); //sr
        SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_GO);

// Wait on TX complete interrupt.
        if (WaitForSingleObject(pDevice->hIntrEvent, pDevice->txTimeout)==WAIT_TIMEOUT)
        {
            DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_ERROR, (L"ERROR: HDQ_Write: "
                L"Timeout in Tx\r\n"));
            goto cleanUp;
        }

// Clear the interrupt
        status=INREG32(&pHDQRegs->ulHDQ_INT_STATUS);
        InterruptDone(pDevice->sysIntr);
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_Write: "
            L"Interrupt status (%d) was 0x%x\r\n", 3 + i, status));

// Verify interrupt source
        if((status & HDQ_INT_STATUS_TXCOMPLETE)==0)
        {
            DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_ERROR, (L"ERROR: HDQ_Write: "
                L"TX complete expected (0x%x)\r\n", status));
            goto cleanUp;
        }
    };

// Done
    rc=TRUE;

cleanUp:
    SetDevicePowerState(pDevice->hParentBus, D4, NULL);
    LeaveCriticalSection(&pDevice->cs);

exit:    
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"---HDQ_Write(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_Read
//
//  This function reads from the specified address data in HDQ mode.
//
BOOL HDQ_Read(DWORD context, UCHAR address, USHORT* pData)
{
    BOOL rc=FALSE;
    HDQ_DEVICE* pDevice=(HDQ_DEVICE*)context;
    OMAP2420_HDQ_REGS* pHDQRegs=NULL;
    DWORD status, count, i;
    UCHAR buffer[2];
//

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"+++HDQ_Read(0x%08x, 0x%02x, 0x%08x)\r\n", context, address, pData));

    Sleep(cliDelay1); // tentaive
// Check if we get correct context
    if(pDevice==NULL || pDevice->cookie!=HDQ_DEVICE_COOKIE)
    {
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_ERROR, (L"ERROR: HDQ_Read: "
            L"Incorrect context parameter\r\n"));
        goto exit;
    }

// Get hardware
    pHDQRegs=pDevice->pHDQRegs;
    EnterCriticalSection(&pDevice->cs);

// Make sure that clock is present
    SetDevicePowerState(pDevice->hParentBus, D0, NULL);
    HDQ_PowerUp(context);

    switch(pDevice->mode)
    {
    case HDQ_MODE_HDQ8:
        count=1;
        break;
    case HDQ_MODE_HDQ16:
        count=2;
        break;
    };

// Clear the interrupt.
    status=INREG32(&pHDQRegs->ulHDQ_INT_STATUS);
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_Read: "
        L"Interrupt status (1) was 0x%02x\r\n", status));

// Need to notify InterruptDone right away
    InterruptDone(pDevice->sysIntr);
// Step 0: send break pulse
// get into status mode; Send break pulse in HDQ mode
    SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_MODE);
    SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_INITIALIZATION);
//    DEBUGMSG(ZONE_HDQ_1WIRE, (L"\tHDQ_Read: 1) Will Send out the break pulse and wait for %ld msec.\r\n",pDevice->breakTimeout));
//    DEBUGMSG(ZONE_HDQ_1WIRE, (L"\tHDQ_Read: 2) Next switch to Write mode, send out register Nr and wait for %ld msec.\r\n",pDevice->txTimeout));
//    DEBUGMSG(ZONE_HDQ_1WIRE, (L"\tHDQ_Read: 3) Then switch to Read mode and wait for %ld msec.\r\n",pDevice->rxTimeout));
// switch to go mode
    SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_GO);

// Wait for completion of break pulse to be sent out
// there are no interrupts set on break pulse, so just wait for it to be sent out
    Sleep(cliDelay2); // was Sleep(pDevice->breakTimeout);  0.32 msec or so
// Back to HDQ mode.
    CLRREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_MODE);
// Step 1:
// Write the value, 
    OUTREG32(&pHDQRegs->ulHDQ_TX_DATA, address); // 
// Step 2: set the direction as Write and go
    CLRREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_DIR); // was SET
    SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_GO);
//
// Wait on completion on transmit data 
    if(WaitForSingleObject(pDevice->hIntrEvent, pDevice->txTimeout)==WAIT_TIMEOUT)
    {
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"ERROR !! HDQ_Read(): "
            L"Timeout in TX for %ld msec.\r\n",pDevice->txTimeout));
//      goto cleanUp;
    }

// Clear interrupt
    status=INREG32(&pHDQRegs->ulHDQ_INT_STATUS);
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_Read: "
        L"Interrupt status (3) was 0x%02x\r\n", status));

// Need to notify InterruptDone right away
    InterruptDone(pDevice->sysIntr);

// Verify that register address was sent out completely.
    if((status & HDQ_INT_STATUS_TXCOMPLETE)==0)
    { // sr further timing adjustment may be helpful. It works with current settings well so disregard this msg
        DEBUGMSG(ZONE_HDQ_1WIRE, (L"ERROR: HDQ_Read: "
            L"TX complete expected (0x%02x)\r\n", status));
//        goto cleanUp;
    }
    else
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"\tHDQ_Read: "
        L"TX completed (0x%02x) ok.\r\n", status));
// pre-Step 3:
    status=0; // INREG32(&pHDQRegs->ulHDQ_INT_STATUS);

    for(i=0; i<count; i++)
    {
// The RX is slave driven, if we are slow byte can be already done.
        if((status & HDQ_INT_STATUS_RXCOMPLETE)==0)
        {
// Indicate read & go: 0 to Read bit, 1 to GO bit
// Step 3:
            SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_DIR); // was CLR
            SETREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_GO);
            Sleep(cliDelay3);
// Wait on RX complete interrupt
            if(WaitForSingleObject(pDevice->hIntrEvent, pDevice->rxTimeout)==WAIT_TIMEOUT)
            {
                DEBUGMSG(ZONE_ERROR, (L"ERROR: HDQ_Read: "
                    L"Timeout on RX at %ld msec\r\n",pDevice->rxTimeout));
                goto cleanUp;
            }
// Clear interrupt
            status=INREG32(&pHDQRegs->ulHDQ_INT_STATUS);
            DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_Read: "
                L"Interrupt status (5) was 0x%02x\r\n", status));

// Reenable interrupt
            InterruptDone(pDevice->sysIntr);
        }

// Verify interrupt source
        if((status & HDQ_INT_STATUS_RXCOMPLETE)==0)
        {
            DEBUGMSG(ZONE_HDQ_1WIRE, (L"ERROR: HDQ_Read: "
                L"RX complete expected (0x%02x)\r\n", status));
//            goto cleanUp;
        }
// Get data
        buffer[i]=(UCHAR)INREG32(&pHDQRegs->ulHDQ_RX_DATA);
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"\tHDQ_Read: "
            L"RX completed data=0x%02x\r\n", buffer[i]));
// Make sure we set GO bit on next byte
        status=0;
    };

    switch (pDevice->mode)
    {
    case HDQ_MODE_HDQ8:
        *pData=buffer[0];
        break;
    case HDQ_MODE_HDQ16:
        *pData=buffer[0]|(buffer[1] << 8);
        break;
    }

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_Read: "
        L"Address:=0x%02x, Data:=0x%02x\r\n", address, *pData));

// Done
    rc=TRUE;
//
cleanUp:
    SetDevicePowerState(pDevice->hParentBus, D4, NULL); // power off the hdq device
    LeaveCriticalSection(&pDevice->cs);
//
exit:    
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"---HDQ_Read(rc=%01ld)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  HDQ_SetMode
//
BOOL HDQ_SetMode(DWORD context, DWORD mode)
{
    BOOL rc = FALSE;
    HDQ_DEVICE *pDevice = (HDQ_DEVICE*)context;
    OMAP2420_HDQ_REGS *pHDQRegs;

    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"+HDQ_SetMode(0x%08x, %d)\r\n", context, mode));

// Check if we get correct context
    if (pDevice == NULL || pDevice->cookie != HDQ_DEVICE_COOKIE)
    {
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_ERROR, (L"ERROR: HDQ_SetMode: "
            L"Incorrect context paramer\r\n"));
        goto cleanUp;
    };
    pHDQRegs = pDevice->pHDQRegs;

    switch (mode)
    {
    case HDQ_MODE_HDQ8:
        CLRREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_MODE);
        pDevice->mode = HDQ_MODE_HDQ8;
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_SetMode: "
            L"New mode HDQ8\r\n"));
        rc = TRUE;
        break;
    case HDQ_MODE_HDQ16:
        CLRREG32(&pHDQRegs->ulHDQ_CTRL_STATUS, HDQ_CTRL_STATUS_MODE);
        pDevice->mode = HDQ_MODE_HDQ16;
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_INFO, (L"HDQ_SetMode: "
            L"New mode HDQ16\r\n"));
        rc = TRUE;
        break;
    default:
        DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_ERROR, (L"ERROR: HDQ_SetMode: "
            L"Unsupported mode requested (mode = 0x%x)\r\n", mode));
    }

cleanUp:
    DEBUGMSG(ZONE_HDQ_1WIRE | ZONE_FUNCTION, (L"-HDQ_SetMode(rc = 0x%x)\r\n", rc));
    return rc;
}

/*#ifdef RETAIL_DEBUG
#pragma optimize("", on)            // debug
#endif*/
