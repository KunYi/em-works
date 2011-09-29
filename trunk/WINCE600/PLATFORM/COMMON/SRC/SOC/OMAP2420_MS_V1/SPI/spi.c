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
//  File: spi.c
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <pm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <spi.h>
#include <buses.h>
#include <omap2420.h>


//------------------------------------------------------------------------------
//  Local Definitions

#define SPI_DEVICE_COOKIE       'spiD'
#define SPI_INSTANCE_COOKIE     'spiI'

//#undef DEBUGMSG
//#define DEBUGMSG(x, y) RETAILMSG(1,y)

extern DBGPARAM dpCurSettings;
//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    DWORD memBase;
    DWORD irq;
    LONG instances;
    HANDLE hParentBus;
    OMAP2420_MCSPI_REGS *pSPIRegs;
    CRITICAL_SECTION cs;
    DWORD sysIntr;
    HANDLE hIntrEvent;
    DWORD timeout;
    CEDEVICE_POWER_STATE powerState;
} SPI_DEVICE;

typedef struct {
    DWORD cookie;
    SPI_DEVICE *pDevice;
    DWORD address;
} SPI_INSTANCE;

//------------------------------------------------------------------------------
//  Local Functions

BOOL  SPI_Deinit(DWORD context);
BOOL  SPI_SetSlaveAddress(DWORD context, DWORD address);
BOOL  SPI_SetupMCSPI_Channel(OMAP2420_MCSPI_REGS *pSPIRegs, DWORD address);
VOID  SPI_ClockOn(SPI_DEVICE *pDevice);
VOID  SPI_ClockOff(SPI_DEVICE *pDevice);

//------------------------------------------------------------------------------
//  Global variables

const GUID DEVICE_IFC_SPI_GUID;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"MemBase", PARAM_DWORD, TRUE, offset(SPI_DEVICE, memBase),
        fieldsize(SPI_DEVICE, memBase), NULL
    }, { 
        L"Irq", PARAM_DWORD, TRUE, offset(SPI_DEVICE, irq),
        fieldsize(SPI_DEVICE, irq), NULL
    }, { 
        L"Timeout", PARAM_DWORD, FALSE, offset(SPI_DEVICE, timeout),
        fieldsize(SPI_DEVICE, timeout), (VOID*)500
    }        
};

//------------------------------------------------------------------------------
//
//  Function:  SPI_Init
//
//  Called by device manager to initialize device.
//
DWORD SPI_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    DWORD rc = (DWORD)NULL;
    SPI_DEVICE *pDevice = NULL;
    PHYSICAL_ADDRESS pa;
    DWORD dwCount=0;

    DEBUGMSG(ZONE_INIT, (
        L"+SPI_Init(%s, 0x%08x)\r\n", szContext, pBusContext
    ));

    // Create device structure
    pDevice = (SPI_DEVICE *)LocalAlloc(LPTR, sizeof(SPI_DEVICE));
    if (pDevice == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_Init: "
            L"Failed allocate SPI controller structure\r\n"
        ));
        goto cleanUp;
    }

    // Set cookie
    pDevice->cookie = SPI_DEVICE_COOKIE;

    // Initalize critical section
    InitializeCriticalSection(&pDevice->cs);

    // Read device parameters
    if (GetDeviceRegistryParams(
        szContext, pDevice, dimof(g_deviceRegParams), g_deviceRegParams
    ) != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_Init: "
            L"Failed read SPI driver registry parameters\r\n"
        ));
        goto cleanUp;
    }

    // Open parent bus
    pDevice->hParentBus = CreateBusAccessHandle(szContext);
    if (pDevice->hParentBus == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_Init: "
            L"Failed open parent bus driver\r\n"
        ));
        goto cleanUp;
    }

    // Set hardware to full power
    pDevice->powerState = D0;
    SPI_ClockOn(pDevice);
    SetDevicePowerState(pDevice->hParentBus, D0, NULL);

    // Map SPI controller
    pa.QuadPart = pDevice->memBase;
    pDevice->pSPIRegs = MmMapIoSpace(pa, sizeof(OMAP2420_MCSPI_REGS), FALSE);
    if (pDevice->pSPIRegs == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_Init: "
            L"Failed map SPI controller registers\r\n"
        ));
        goto cleanUp;
    }

    // Map SPI interrupt
    if (!KernelIoControl(
        IOCTL_HAL_REQUEST_SYSINTR, &pDevice->irq, sizeof(pDevice->irq), 
        &pDevice->sysIntr, sizeof(pDevice->sysIntr), NULL
    )) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_Init: "
            L"Failed map SPI controller interrupt\r\n"
        ));
        goto cleanUp;
    }

    // Create interrupt event
    pDevice->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevice->hIntrEvent == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_Init: "
            L"Failed create interrupt event\r\n"
        ));
        goto cleanUp;
    }

    // Initialize interrupt
    if (!InterruptInitialize(pDevice->sysIntr, pDevice->hIntrEvent, NULL, 0)) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Init: "
            L"InterruptInitialize failed\r\n"
        ));
        goto cleanUp;
    }

    // Reset the SPI controller
    SETREG32(&pDevice->pSPIRegs->ulMCSPI_SYSCONFIG, SPI_SYSC_SRST);

    // TODO: Get base clock from kernel & read L/H periods from registry

    // Wait until resetting is done
    while ( !(INREG32(&pDevice->pSPIRegs->ulMCSPI_SYSSTATUS) & BIT0)) {
        Sleep (1);

        if (dwCount++>0x100)
        {
            // Break out dead lock, something is wrong.
            ERRORMSG (TRUE, (TEXT("SPI: ERROR holding in reset.\n")));
            goto cleanUp;
        }
    }

    // Disable all interrupts.
    OUTREG32(&pDevice->pSPIRegs->ulMCSPI_IRQENABLE, 0);

    // Clear interrupts.
    OUTREG32(&pDevice->pSPIRegs->ulMCSPI_IRQSTATUS, 0xFFFF);


    // Setup Module Control as master
    OUTREG32(&pDevice->pSPIRegs->ulMCSPI_MODULCTRL, 0);

    // Turn on the clock on default 1M rate.
    // Reset the SPI controller
    SETREG32(&pDevice->pSPIRegs->ulMCSPI_SYSCONFIG, 0x308);
//    g_pSpiRegs->ulSET1 &= ~0x001E;
//    g_pSpiRegs->ulSET1 |= (GetClockParams(INIT_CLOCK_RATE) << 1);
//    g_pSpiRegs->ulSET1 |= BIT0;


    // Enable the SPI
//    SETREG16(&pDevice->pSPIRegs->CON, SPI_CON_EN);

    // Return non-null value
    rc = (DWORD)pDevice;

    // Set this driver to internal suspend mode
    SetDevicePowerState(pDevice->hParentBus, pDevice->powerState = D4, NULL);
    SPI_ClockOff(pDevice);

cleanUp:
    if (rc == 0) SPI_Deinit((DWORD)pDevice);
    DEBUGMSG(ZONE_INIT, (L"-SPI_Init(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL SPI_Deinit(DWORD context)
{
    BOOL rc = FALSE;
    SPI_DEVICE *pDevice = (SPI_DEVICE*)context;

    DEBUGMSG(ZONE_INIT, (L"+SPI_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if (pDevice == NULL || pDevice->cookie != SPI_DEVICE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Deinit: "
            L"Incorrect context paramer\r\n"
        ));
        goto cleanUp;
    }

    // Check for open instances
    if (pDevice->instances > 0) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Deinit: "
            L"Deinit with active instance (%d instances active)\r\n",
            pDevice->instances
        ));
        goto cleanUp;
    }

    // Set hardware to D4 and close parent bus driver
    if (pDevice->hParentBus!= NULL) 
    {
        SetDevicePowerState(pDevice->hParentBus, D4,NULL);
        CloseBusAccessHandle(pDevice->hParentBus);
    }

    // Delete critical section
    DeleteCriticalSection(&pDevice->cs);

    // Unmap SPI controller registers
    if (pDevice->pSPIRegs != NULL)
    {
        MmUnmapIoSpace((VOID*)pDevice->pSPIRegs, sizeof(OMAP2420_MCSPI_REGS));
    }

    //Turn clocks off
    SPI_ClockOff(pDevice);

    // Release SPI controller interrupt
    if (pDevice->sysIntr != 0) 
    {
        InterruptDisable(pDevice->sysIntr);
        KernelIoControl(
            IOCTL_HAL_RELEASE_SYSINTR, &pDevice->sysIntr,
            sizeof(pDevice->sysIntr), NULL, 0, NULL
        );
    }

    // Close interrupt handler
    if (pDevice->hIntrEvent != NULL) CloseHandle(pDevice->hIntrEvent);

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_INIT, (L"-SPI_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD SPI_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    DWORD rc = (DWORD)NULL;
    SPI_DEVICE *pDevice = (SPI_DEVICE*)context;
    SPI_INSTANCE *pInstance = NULL;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+SPI_Open(0x%08x, 0x%08x, 0x%08x\r\n", context, accessCode, shareMode
    ));

    // Check if we get correct context
    if (pDevice == NULL || pDevice->cookie != SPI_DEVICE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Open: "
            L"Incorrect context parameter\r\n"
        ));
        goto cleanUp;
    }

    // Create device structure
    pInstance = (SPI_INSTANCE*)LocalAlloc(LPTR, sizeof(SPI_INSTANCE));
    if (pInstance == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_Open: "
            L"Failed allocate SPI instance structure\r\n"
        ));
        goto cleanUp;
    }

    // Set cookie
    pInstance->cookie = SPI_INSTANCE_COOKIE;

    // Save device reference
    pInstance->pDevice = pDevice;

    // Increment number of open instances
    InterlockedIncrement(&pDevice->instances);

    // sanity check number of instances
    ASSERT(pDevice->instances > 0);

    // Done...
    rc = (DWORD)pInstance;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-SPI_Open(rc = 0x%08x)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_Close
//
//  This function closes the device context.
//
BOOL SPI_Close(DWORD context)
{
    BOOL rc = FALSE;
    SPI_DEVICE *pDevice;
    SPI_INSTANCE *pInstance = (SPI_INSTANCE*)context;

    DEBUGMSG(ZONE_FUNCTION, (L"+SPI_Close(0x%08x)\r\n", context));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != SPI_INSTANCE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Transfer: "
            L"Incorrect context paramer\r\n"
        ));
        goto cleanUp;
    }

    // Get device context
    pDevice = pInstance->pDevice;

    // sanity check number of instances
    ASSERT(pDevice->instances > 0);

    // Decrement number of open instances
    InterlockedDecrement(&pDevice->instances);

    // Free instance structure
    LocalFree(pInstance);

    // Done...
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-SPI_Close(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_Transfer
//
//  This function reads data from the device identified by the open context.
//
DWORD SPI_Transfer(DWORD context, PVOID pBuffer)
{

    DWORD rc = 0;
    SPI_DEVICE *pDevice;
    SPI_INSTANCE *pInstance = (SPI_INSTANCE*)context;
    OMAP2420_MCSPI_REGS *pSPIRegs;
    UINT32* pData = (UINT32*)pBuffer;
    DWORD dwCount;

//    DEBUGMSG(ZONE_FUNCTION, ( L"+SPI_Transfer(0x%08x, 0x%08x\r\n", context, *pData));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != SPI_INSTANCE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Transfer: "
            L"Incorrect context paramer\r\n"
        ));
        goto clean;
    }
    pDevice = pInstance->pDevice;
    pSPIRegs = pDevice->pSPIRegs;
    // Get hardware
    EnterCriticalSection(&pDevice->cs);
    
    // Set this driver to Active Mode
    SPI_ClockOn(pDevice);
    SetDevicePowerState(pDevice->hParentBus, pDevice->powerState = D0, NULL);
    switch (pInstance->address) {
    case 0:
        // Enable the channel
        SETREG32(&pSPIRegs->ulMCSPI_CHCTRL0, BIT0);

        OUTREG32(&pSPIRegs->ulMCSPI_TX0, *pData);
        // Wait for transfer to finish.
        dwCount = 0;
        while(!(INREG32(&pSPIRegs->ulMCSPI_CHSTATUS0) & BIT2))
        {
            if (dwCount++>0x1ffff)
            {
                // Break out dead lock, something is wrong.
                ERRORMSG (1, (L"SPI_Transfer: SPI is not responding CHSTATUS = 0x%x\r\n",INREG32(&pSPIRegs->ulMCSPI_CHSTATUS0)));
                goto clean;
            }
        }
        *pData = 0xFFFF & INREG32(&pSPIRegs->ulMCSPI_RX0);
        break;

    default:
        DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_Transfer: Incorrect channel address\r\n"));
        goto clean;
    }
    rc = 1;

clean:
    // Disable the channel.
    CLRREG32(&pSPIRegs->ulMCSPI_CHCTRL0, BIT0);

    // Set this driver to Suspend Mode
    SPI_ClockOff(pDevice);
    SetDevicePowerState(pDevice->hParentBus, pDevice->powerState = D4, NULL);
    
    // Release hardware
    LeaveCriticalSection(&pDevice->cs);
//    DEBUGMSG(ZONE_FUNCTION, (L"-SPI_Transfer(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_IOControl
//
//  This function sends a command to a device.
//
BOOL SPI_IOControl(
    DWORD context, DWORD dwCode, BYTE *pInBuffer, DWORD inSize, BYTE *pOutBuffer,
    DWORD outSize, DWORD *pOutSize
) {
    BOOL bRetVal = FALSE;
    SPI_DEVICE *pDevice = NULL;
    SPI_INSTANCE *pInstance = (SPI_INSTANCE*)context;
    DEVICE_IFC_SPI ifc;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+SPI_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, dwCode, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
    ));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != SPI_INSTANCE_COOKIE) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_IOControl: "
            L"Incorrect context paramer\r\n"
        ));
        goto clean;
    }

    //Get Device
    pDevice = pInstance->pDevice;

    switch (dwCode) {
    case IOCTL_DDK_GET_DRIVER_IFC:
        // We can give interface only to our peer in device process
        if (GetCurrentProcessId() != (DWORD)GetCallerProcess()) {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: SPI_IOControl: "
                L"IOCTL_DDK_GET_DRIVER_IFC can be called only from "
                L"device process (caller process id 0x%08x)\r\n",
                GetCallerProcess()
            ));
            SetLastError(ERROR_ACCESS_DENIED);
            goto clean;
        }
        // Check input parameters
        if (pInBuffer == NULL || inSize < sizeof(GUID)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        if (IsEqualGUID(pInBuffer, &DEVICE_IFC_SPI_GUID)) {
            if (pOutSize != NULL) *pOutSize = sizeof(DEVICE_IFC_SPI);
            if (pOutBuffer == NULL || outSize < sizeof(DEVICE_IFC_SPI)) {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            ifc.context = context;
            ifc.pfnSetSlaveAddress = SPI_SetSlaveAddress;
            ifc.pfnTransfer = SPI_Transfer;
            if (!CeSafeCopyMemory(pOutBuffer, &ifc, sizeof(DEVICE_IFC_SPI))) {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            bRetVal = TRUE;
            break;
        }
        SetLastError(ERROR_INVALID_PARAMETER);
        break;
    case IOCTL_SPI_SET_SLAVE_ADDRESS:
        if (pOutSize != NULL) *pOutSize = sizeof(DWORD);
        if (pInBuffer == NULL || inSize < sizeof(DWORD)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        bRetVal = SPI_SetSlaveAddress(context, (DWORD)*pInBuffer);
        break;


    // Power management functions.
    // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
    {
        POWER_CAPABILITIES pc;

        // Check arguments.
        if ( pOutBuffer == NULL || outSize < sizeof(POWER_CAPABILITIES))
        {
            ERRORMSG( TRUE, ( TEXT( "SPI: IOCTL_POWER_CAPABILITIES "
                L"Invalid parameter.\r\n" ) ) );
            break;
        }

        // Clear capabilities structure.
        memset(&pc, 0, sizeof(POWER_CAPABILITIES));

        // Set power capabilities. Supports D0 and D4.
        pc.DeviceDx = DX_MASK(D0)|DX_MASK(D4);

        DEBUGMSG(ZONE_POWER, (TEXT("SPI: IOCTL_POWER_CAPABILITIES = 0x%x\r\n"), pc.DeviceDx));

        if (CeSafeCopyMemory(pOutBuffer, &pc, sizeof(pc)) == 0)
        {
            ERRORMSG( TRUE, ( TEXT( "SPI: CeSafeCopyMemory Failed\r\n" ) ) );
            break;
        }

        // Update returned data size.
        if (pOutSize)
        {
            *pOutSize = sizeof(pc);
        }
        bRetVal = TRUE;
        break;
    }

    // Indicate if the device is ready to enter a new device power state.
    case IOCTL_POWER_QUERY:
    {
        DEBUGMSG(ZONE_POWER, (TEXT("SPI: IOCTL_POWER_QUERY"
            L"Deprecated Function Called\r\n")));
        bRetVal = FALSE;
        break;
    }

    // Request a change from one device power state to another
    // This driver self-manages it's internal power state by controlling
    // functional and interface clocks as needed in the Read and Write
    // functions rather than waiting for PM to tell it to save power
    // So the set calls below just update the power state variable
    case IOCTL_POWER_SET:
    {
        CEDEVICE_POWER_STATE dxState;

        // Check arguments.
        if (pOutBuffer == NULL || outSize < sizeof(CEDEVICE_POWER_STATE))
        {
            ERRORMSG( TRUE, ( TEXT( "SPI: IOCTL_POWER_SET"
                L"Invalid parameter.\r\n" ) ) );
            break;
        }

        if (CeSafeCopyMemory(&dxState, pOutBuffer, sizeof(dxState)) == 0) break;

        DEBUGMSG(ZONE_POWER, ( TEXT( "SPI: IOCTL_POWER_SET = %d.\r\n"), dxState));

        // Check for any valid power state.
        if (VALID_DX(dxState))
        {
            // Power off
            if ( dxState == D4 )
            {
                // Set this driver to low power mode external state
                pDevice->powerState = dxState;
            }
            // Power on.
            else
            {
                //Set to high power external state
                pDevice->powerState = dxState;
            }
            bRetVal = TRUE;
        }
        else
        {
            ERRORMSG( TRUE, ( TEXT( "SPI: IOCTL_POWER_SET "
                L"Invalid power state.\r\n" ) ) );
        }
        break;
    }

    // Return the current device power state.
    case IOCTL_POWER_GET:
    {
        // Check arguments.
        if (pOutBuffer == NULL || outSize < sizeof(CEDEVICE_POWER_STATE))
        {
            ERRORMSG( TRUE, ( TEXT( "SPI: IOCTL_POWER_GET "
                L"Invalid parameter.\r\n" ) ) );
            break;
        }

        //Copy current state
        if (CeSafeCopyMemory(pOutBuffer, &pDevice->powerState, sizeof(pDevice->powerState)) == 0)
        {
            ERRORMSG( TRUE, ( TEXT( "SPI: CeSafeCopyMemory Failed\r\n" ) ) );
            break;
        }

        // Update returned data size.
        if (pOutSize)
        {
            *pOutSize = sizeof(pDevice->powerState);
        }

        DEBUGMSG(ZONE_POWER, (TEXT("SPI: IOCTL_POWER_GET: %d\r\n"), pDevice->powerState));
        bRetVal = TRUE;
        break;
    }


    default:
        ERRORMSG(1, (TEXT("SPI: Unknown IOCTL_xxx(0x%0.8X) \r\n"), dwCode));
        break;
    }

clean:
    DEBUGMSG(ZONE_FUNCTION, (L"-SPI_IOControl(rc = %d)\r\n", bRetVal));
    return bRetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_PowerUp
//
//  This function restores power to a device.
//
VOID SPI_PowerUp(DWORD context)
{
}

//------------------------------------------------------------------------------
//
//  Function:  SPI_PowerDown
//
//  This function suspends power to the device.
//
void SPI_PowerDown(DWORD context)
{
}

//------------------------------------------------------------------------------

BOOL SPI_SetSlaveAddress(DWORD context, DWORD address)
{
    BOOL rc = FALSE;
    SPI_INSTANCE *pInstance = (SPI_INSTANCE*)context;
    SPI_DEVICE *pDevice;

    DEBUGMSG(ZONE_FUNCTION, (L"SPI_SetSlaveAddress 0x%x\r\n", address));

    // Check if we get correct context
    if (pInstance == NULL || pInstance->cookie != SPI_INSTANCE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: SPI_SetSlaveAddress: "
            L"Incorrect context paramer\r\n"
        ));
        goto cleanUp;
    }

    //Get Device
    pDevice = pInstance->pDevice;

    //Set Address
    pInstance->address = address;

    // Get hardware
    EnterCriticalSection(&pDevice->cs);
    
    // Set this driver to Active Mode
    SPI_ClockOn(pDevice);
    SetDevicePowerState(pDevice->hParentBus, pDevice->powerState = D0, NULL);

    //Set channel address
    SPI_SetupMCSPI_Channel(pInstance->pDevice->pSPIRegs, pInstance->address);

    // Set this driver to Suspend Mode
    SPI_ClockOff(pDevice);
    SetDevicePowerState(pDevice->hParentBus, pDevice->powerState = D4, NULL);
    
    // Release hardware
    LeaveCriticalSection(&pDevice->cs);    
    
    rc = TRUE;

cleanUp:
    return rc;
}



//SPI_SetupMCSPI_Channel ***Assumes we already hold the hardware critical section and clocks are enabled***
BOOL SPI_SetupMCSPI_Channel(OMAP2420_MCSPI_REGS *pSPIRegs, DWORD address)
{
    BOOL rc = FALSE;
    //DEBUGMSG(ZONE_FUNCTION, (L"SPI_SetupMCSPI_Channel 0x%x\r\n", address));
    // Config the channel:
    if (address == 0)
    {
        //BIT0 = 0 Data are latched on odd numbered edges 
        //BIT1 = 1 Active Low Clock 
        //BIT5-2 = 6 Clock Divider =64 divide from 48MHz
        //BIT6 = 1 EPOL Active Low CS
        //BIT11-7 = F, SPI Word = 16 bit
        //BIT11-7 = 1F, SPI Word = 16 bit
        //BIT13-12 = 0, Transmit AND Receive mode
        //BIT14 = 0, DMA Write req disabled
        //BIT15 = 0, DMA Read req disabled
        //BIT16 = 1, transmission on SOMI disabled
        //BIT17 = 0, transmission on SIMO enabled
        //BIT18 = 0, reception on SOMI enabled
        //BIT19 = 0, TURBO deactivated
        //BIT20 = 0, FORCE HIGH between words
        //BIT21-31 = 0 reserved
        OUTREG32(&pSPIRegs->ulMCSPI_CHCONF0, 0x00010FDA);
    }
    else
    {
        goto cleanUp;
    }
    rc = TRUE;

cleanUp:
    return rc;
}



//  Turn on the clocks
VOID SPI_ClockOn(SPI_DEVICE *pDevice)
{
    DWORD regBit,cbRet;

    //Check if it's SPI1 or SPI2
    if(OMAP2420_MCSPI1_REGS_PA == pDevice->memBase)
    {
        DEBUGMSG(ZONE_POWER, (TEXT("SPI1: SPI_ClockOn\r\n") ));
        regBit = PRCM_FCLKEN1_CORE_EN_MCSPI1;
        KernelIoControl(IOCTL_FCLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
        regBit = PRCM_ICLKEN1_CORE_EN_MCSPI1;
        KernelIoControl(IOCTL_ICLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
    }
    else
    {
        //Assume SPI2
        DEBUGMSG(ZONE_POWER, (TEXT("SPI2: SPI_ClockOn\r\n") ));
        regBit = PRCM_FCLKEN1_CORE_EN_MCSPI2;
        KernelIoControl(IOCTL_FCLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
        regBit = PRCM_ICLKEN1_CORE_EN_MCSPI2;
        KernelIoControl(IOCTL_ICLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
    }
}

// Turn the clocks off
VOID SPI_ClockOff(SPI_DEVICE *pDevice)
{
    DWORD regBit,cbRet;

    //Check if it's SPI1 or SPI2
    if(OMAP2420_MCSPI1_REGS_PA== pDevice->memBase)
    {
        DEBUGMSG(ZONE_POWER, (TEXT("SPI1: SPI_ClockOff\r\n") ));
        regBit = PRCM_FCLKEN1_CORE_EN_MCSPI1;
        KernelIoControl(IOCTL_FCLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
        regBit = PRCM_ICLKEN1_CORE_EN_MCSPI1;
        KernelIoControl(IOCTL_ICLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
    }
    else
    {
        //Assume SPI2
        DEBUGMSG(ZONE_POWER, (TEXT("SPI2: SPI_ClockOff\r\n") ));
        regBit = PRCM_FCLKEN1_CORE_EN_MCSPI2;
        KernelIoControl(IOCTL_FCLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
        regBit = PRCM_ICLKEN1_CORE_EN_MCSPI2;
        KernelIoControl(IOCTL_ICLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
    }
}

//------------------------------------------------------------------------------

