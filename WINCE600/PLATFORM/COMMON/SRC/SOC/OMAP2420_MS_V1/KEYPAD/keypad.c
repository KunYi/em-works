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
//  File: keypad.c
//
//  This file implements device driver for keypad. The driver isn't implemented
//  as classical keyboard driver. Instead implementation uses stream driver
//  model and it calls keybd_event to pass information to GWE subsystem.
//


#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap2420.h>
#include <keypad.h>
#include <pmpolicy.h>

#if 0
#undef DEBUGMSG
#define DEBUGMSG RETAILMSG
#define ZONE_ERROR          1
#define ZONE_WARN           1
#define ZONE_FUNCTION       1
#define ZONE_INIT           1
#define ZONE_INFO           1
#define ZONE_IST            1
#endif

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifdef DEBUG

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)
#define ZONE_INFO           DEBUGZONE(4)
#define ZONE_IST            DEBUGZONE(5)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"keypad", {
        L"Errors",      L"Warnings",    L"Function",    L"Init",
        L"Info",        L"IST",         L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0003
};

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define KPD_DEVICE_COOKIE           'kpdD'

#define KEYPAD_ROWS_MASK            ((1 << KEYPAD_ROWS) - 1)

#define VK_KEYS                     256
#define DWORD_BITS                  32

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    DWORD irqs[7];
    DWORD priority256;
    DWORD enableWake;
    DWORD samplePeriod;
    DWORD debounceTime;
    DWORD firstRepeat;
    DWORD nextRepeat;
    CRITICAL_SECTION cs;
    OMAP2420_GPIO_REGS *pGPIO1Regs;
    OMAP2420_GPIO_REGS *pGPIO2Regs;
    OMAP2420_GPIO_REGS *pGPIO3Regs;
    OMAP2420_GPIO_REGS *pGPIO4Regs;
    DWORD sysIntr;
    HANDLE hIntrEvent;
    HANDLE hIntrThread;
    BOOL intrThreadExit;
} KPD_DEVICE;

typedef struct {
    BOOL pending;
    BOOL remapped;
    DWORD time;
    DWORD state;
} KEYPAD_REMAP_STATE;

typedef struct {
    BOOL pending;
    DWORD time;
    BOOL blocked;
} KEYPAD_REPEAT_STATE;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"IrqRow0", PARAM_DWORD, TRUE, offset(KPD_DEVICE, irqs[2]),
        fieldsize(KPD_DEVICE, irqs[2]), NULL
    }, {
        L"IrqRow1", PARAM_DWORD, TRUE, offset(KPD_DEVICE, irqs[3]),
        fieldsize(KPD_DEVICE, irqs[3]), NULL
    }, {
        L"IrqRow2", PARAM_DWORD, TRUE, offset(KPD_DEVICE, irqs[4]),
        fieldsize(KPD_DEVICE, irqs[4]), NULL
    }, {
        L"IrqRow3", PARAM_DWORD, TRUE, offset(KPD_DEVICE, irqs[5]),
        fieldsize(KPD_DEVICE, irqs[5]), NULL
    }, {
        L"IrqRow4", PARAM_DWORD, TRUE, offset(KPD_DEVICE, irqs[6]),
        fieldsize(KPD_DEVICE, irqs[6]), NULL
    }, {
        L"Priority256", PARAM_DWORD, FALSE, offset(KPD_DEVICE, priority256),
        fieldsize(KPD_DEVICE, priority256), (VOID*)100
    }, {
        L"EnableWake", PARAM_DWORD, FALSE, offset(KPD_DEVICE, enableWake),
        fieldsize(KPD_DEVICE, enableWake), (VOID*)0
    }, {
        L"SamplePeriod", PARAM_DWORD, FALSE, offset(KPD_DEVICE, samplePeriod),
        fieldsize(KPD_DEVICE, samplePeriod), (VOID*)40
    }, {
        L"DebounceTime", PARAM_DWORD, FALSE, offset(KPD_DEVICE, debounceTime),
        fieldsize(KPD_DEVICE, debounceTime), (VOID*)0x50
    }, {
        L"FirstRepeat", PARAM_DWORD, FALSE, offset(KPD_DEVICE, firstRepeat),
        fieldsize(KPD_DEVICE, firstRepeat), (VOID*)500
    }, {
        L"NextRepeat", PARAM_DWORD, FALSE, offset(KPD_DEVICE, nextRepeat),
        fieldsize(KPD_DEVICE, nextRepeat), (VOID*)125
    }
};

//------------------------------------------------------------------------------
//  Local functions

BOOL KPD_Deinit(DWORD context);
static DWORD KPD_IntrThread(VOID *pParam);

//------------------------------------------------------------------------------
//
//  Function:  KPD_Init
//
//  Called by device manager to initialize device.
//
DWORD KPD_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    DWORD rc = (DWORD)NULL;
    KPD_DEVICE *pDevice = NULL;
    PHYSICAL_ADDRESS pa;


    DEBUGMSG(ZONE_FUNCTION, (
        L"+KPD_Init(%s, 0x%08x)\r\n", szContext, pBusContext
    ));

    // Create device structure
    pDevice = (KPD_DEVICE *)LocalAlloc(LPTR, sizeof(KPD_DEVICE));
    if (pDevice == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed allocate KDP driver structure\r\n"
        ));
        goto cleanUp;
    }

    // Set cookie & initialize critical section
    pDevice->cookie = KPD_DEVICE_COOKIE;
    InitializeCriticalSection(&pDevice->cs);

    // Read device parameters
    if (GetDeviceRegistryParams(
        szContext, pDevice, dimof(g_deviceRegParams), g_deviceRegParams
    ) != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed read KPD driver registry parameters\r\n"
        ));
        pDevice->irqs[0]= -1;
        pDevice->irqs[1]= 2;
        pDevice->irqs[2]=(DWORD)NULL;
        pDevice->irqs[3]=(DWORD)NULL;
        pDevice->irqs[4]=(DWORD)NULL;
        pDevice->irqs[5]=(DWORD)NULL;
        pDevice->irqs[6]=(DWORD)NULL;
        pDevice->priority256=100;
        pDevice->samplePeriod=40;
        pDevice->debounceTime=0x50;
        pDevice->firstRepeat=500;
        pDevice->nextRepeat=125;
//        goto cleanUp;
    }

    // map gpio memory space
    pa.QuadPart = OMAP2420_GPIO1_REGS_PA;
    pDevice->pGPIO1Regs = (OMAP2420_GPIO_REGS *) MmMapIoSpace(pa, sizeof(OMAP2420_GPIO_REGS), FALSE);
    if (pDevice->pGPIO1Regs == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: Failed to map GPIO1 registers\r\n"));
        goto cleanUp;
    }

    pa.QuadPart = OMAP2420_GPIO2_REGS_PA;
    pDevice->pGPIO2Regs = (OMAP2420_GPIO_REGS *) MmMapIoSpace(pa, sizeof(OMAP2420_GPIO_REGS), FALSE);
    if (pDevice->pGPIO2Regs == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: Failed to map GPIO2 registers\r\n"));
        goto cleanUp;
    }

    pa.QuadPart = OMAP2420_GPIO3_REGS_PA;
    pDevice->pGPIO3Regs = (OMAP2420_GPIO_REGS *) MmMapIoSpace(pa, sizeof(OMAP2420_GPIO_REGS), FALSE);
    if (pDevice->pGPIO3Regs == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: Failed to map GPIO3 registers\r\n"));
        goto cleanUp;
    }

    pa.QuadPart = OMAP2420_GPIO4_REGS_PA;
    pDevice->pGPIO4Regs = (OMAP2420_GPIO_REGS *) MmMapIoSpace(pa, sizeof(OMAP2420_GPIO_REGS), FALSE);
    if (pDevice->pGPIO4Regs == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: Failed to map GPIO4 registers\r\n"));
        goto cleanUp;
    }

    // Need to configure the row GPIO's as interrupt sources
    pDevice->irqs[0] = -1;
    pDevice->irqs[1] = 2;
    //pDevice->irqs[2] = IRQ_GPIO_0+88;	// row 0 = GPIO88
    //pDevice->irqs[3] = IRQ_GPIO_0+89;	// row 1 = GPIO89
    //pDevice->irqs[4] = IRQ_GPIO_0+124;	// row 2 = GPIO124
    //pDevice->irqs[5] = IRQ_GPIO_0+11;	// row 3 = GPIO11
    //pDevice->irqs[6] = IRQ_GPIO_0+6;	// row 4 = GPIO6


    // Map interrupts
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
        pDevice->irqs, sizeof(pDevice->irqs),
        &pDevice->sysIntr, sizeof(pDevice->sysIntr), NULL
    )) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed map Keyboard Interrupt\r\n"
        ));
        goto cleanUp;
    } else DEBUGMSG(ZONE_INIT, (L"KPD_Init: sysintr = %d",pDevice->sysIntr));

    // Enable wakeup from keyboard if required
    if (pDevice->enableWake != 0) {
        DEBUGMSG(ZONE_ERROR, (L"Enable keyboard as wakeup source\r\n"));
        if (!KernelIoControl(
            IOCTL_HAL_ENABLE_WAKE, &pDevice->sysIntr, sizeof(pDevice->sysIntr),
            NULL, 0, NULL
        )) {
            DEBUGMSG(ZONE_WARN, (L"WARN: KPD_Init: "
                L"Failed enable keyboard as wakeup source\r\n"
            ));
        }
    }

    // Create interrupt event
    pDevice->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevice->hIntrEvent == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed create interrupt event\r\n"
        ));
        goto cleanUp;
    }

    // Initialize interrupt
    if (!InterruptInitialize(pDevice->sysIntr, pDevice->hIntrEvent, NULL, 0)) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"InterruptInitialize failed\r\n"
        ));
        goto cleanUp;
    }
    // Start interrupt service thread
    if ((pDevice->hIntrThread = CreateThread(
        NULL, 0, KPD_IntrThread, pDevice, 0,NULL
    )) == NULL) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Init: "
            L"Failed create interrupt thread\r\n"
        ));
        goto cleanUp;
    }
    // Set thread priority
    CeSetThreadPriority(pDevice->hIntrThread, pDevice->priority256);

    // Return non-null value
    rc = (DWORD)pDevice;

cleanUp:
    if (rc == 0) KPD_Deinit((DWORD)pDevice);
    DEBUGMSG(ZONE_FUNCTION, (L"-KPD_Init(rc = %d\r\n", rc));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL KPD_Deinit(DWORD context)
{
    BOOL rc = FALSE;
    KPD_DEVICE *pDevice = (KPD_DEVICE*)context;


    DEBUGMSG(ZONE_FUNCTION, (L"+KPD_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if (pDevice == NULL || pDevice->cookie != KPD_DEVICE_COOKIE) {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPD_Deinit: "
            L"Incorrect context parameter\r\n"
        ));
        goto cleanUp;
    }

    // Close interrupt thread
    if (pDevice->hIntrThread != NULL) {
        // Signal stop to thread
        pDevice->intrThreadExit = TRUE;
        // Set event to wake it
        SetEvent(pDevice->hIntrEvent);
        // Wait until thread exits
        WaitForSingleObject(pDevice->hIntrThread, INFINITE);
    }

    // Unmap GPIO controller registers
        // Set all output columns to ones
        // Ie, drive all the column GPIO's

        // Mask keyboard interrupts
        // Unmap registers
    if (pDevice->pGPIO1Regs != NULL)
    {
        OUTREG32(&pDevice->pGPIO1Regs->ulGPIO_SETDATAOUT, 1<<15);    // column 2 = Drive GPIO15
        OUTREG32(&pDevice->pGPIO1Regs->ulGPIO_SETDATAOUT, 1<<12);    // column 4 = Drive GPIO12
        MmUnmapIoSpace((VOID*)pDevice->pGPIO1Regs, sizeof(OMAP2420_GPIO_REGS));
    }

    if (pDevice->pGPIO2Regs != NULL)
    {
        OUTREG32(&pDevice->pGPIO2Regs->ulGPIO_SETDATAOUT, 1<<4);     // column 3 = Drive GPIO36
        MmUnmapIoSpace((VOID*)pDevice->pGPIO2Regs, sizeof(OMAP2420_GPIO_REGS));
    }

    if (pDevice->pGPIO3Regs != NULL)
    {
        OUTREG32(&pDevice->pGPIO3Regs->ulGPIO_SETDATAOUT, 1<<26);    // column 0 = Drive GPIO90
        OUTREG32(&pDevice->pGPIO3Regs->ulGPIO_SETDATAOUT, 1<<27);    // column 1 = Drive GPIO91
        MmUnmapIoSpace((VOID*)pDevice->pGPIO3Regs, sizeof(OMAP2420_GPIO_REGS));
    }

    if (pDevice->pGPIO4Regs != NULL)
    {
        OUTREG32(&pDevice->pGPIO4Regs->ulGPIO_SETDATAOUT, 1<<1);     // column 5 = Drive GPIO97
        MmUnmapIoSpace((VOID*)pDevice->pGPIO4Regs, sizeof(OMAP2420_GPIO_REGS));
    }

    // Disable interrupt
    if (pDevice->sysIntr != 0) {
        InterruptDisable(pDevice->sysIntr);
        KernelIoControl(
            IOCTL_HAL_RELEASE_SYSINTR, &pDevice->sysIntr,
            sizeof(pDevice->sysIntr), NULL, 0, NULL
        );
    }

    // Close interrupt handler
    if (pDevice->hIntrEvent != NULL) CloseHandle(pDevice->hIntrEvent);

    // Delete critical section
    DeleteCriticalSection(&pDevice->cs);

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-KPD_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD KPD_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_Open
//
//  This function closes the device context.
//
BOOL KPD_Close(DWORD context)
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_IOControl
//
//  This function sends a command to a device.
//
BOOL KPD_IOControl(
    DWORD context, DWORD code, BYTE *pInBuffer, DWORD inSize, BYTE *pOutBuffer,
    DWORD outSize, DWORD *pOutSize
) {
    DEBUGMSG(ZONE_INIT, (L"KPD_IOControl"));
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  KPD_InterruptThread
//
//  This function acts as the IST for the keypad.
//
DWORD KPD_IntrThread(VOID *pContext)
{
    KPD_DEVICE *pDevice = (KPD_DEVICE*)pContext;
    DWORD timeout, time, ix;
    DWORD change, mask;
    UINT16 ir, ik;
    USHORT data[KEYPAD_COLUMNS];
    UINT16 ic=0;
    DWORD vkState[VK_KEYS/DWORD_BITS], vkNewState[VK_KEYS/DWORD_BITS];
    KEYPAD_REMAP_STATE *pRemapState = NULL;
    KEYPAD_REPEAT_STATE *pRepeatState = NULL;
    BOOL keyDown;
    UCHAR vk;

    DEBUGMSG(ZONE_IST, (L"KPD - Start IntrThread\r\n"));

    // Init data
    memset(vkState, 0, sizeof(vkState));

    // Initialize remap informations
    if (g_keypadRemap.count > 0) {
        // Allocate state structure for remap
        if ((pRemapState = LocalAlloc(
            LPTR, g_keypadRemap.count * sizeof(KEYPAD_REMAP_STATE)
        ))  == NULL) {
            DEBUGMSG(ZONE_ERROR, (L" KPD_IntrThread: "
                L"Failed allocate memory for virtual key remap\r\n"
            ));
            goto cleanUp;
        }
    }

    // Initialize repeat informations
    if (g_keypadRepeat.count > 0) {
        // Allocate state structure for remap
        if ((pRepeatState = LocalAlloc(
            LPTR, g_keypadRepeat.count * sizeof(KEYPAD_REPEAT_STATE)
        ))  == NULL) {
            DEBUGMSG(ZONE_ERROR, (L" KPD_IntrThread: "
                L"Failed allocate memory for virtual key auto repeat\r\n"
            ));
            goto cleanUp;
        }
    }

    // Set delay to sample period
    timeout = INFINITE;

    // Loop until we are not stopped...
    while (!pDevice->intrThreadExit) {

        if (pDevice->intrThreadExit) break;

        keyDown = FALSE;
        // Wait for event
        WaitForSingleObject(pDevice->hIntrEvent, timeout);
        Sleep(5); //settle time

        memset(data, 0, sizeof(data));
        for (ic=0; ic < KEYPAD_COLUMNS; ic++)
        {
            // Don't drive any column GPIO's
            OUTREG32(&pDevice->pGPIO3Regs->ulGPIO_SETDATAOUT, 1<<26);    // column 0 = Drive GPIO90
            OUTREG32(&pDevice->pGPIO3Regs->ulGPIO_SETDATAOUT, 1<<27);    // column 1 = Drive GPIO91
            OUTREG32(&pDevice->pGPIO1Regs->ulGPIO_SETDATAOUT, 1<<15);    // column 2 = Drive GPIO15
            OUTREG32(&pDevice->pGPIO2Regs->ulGPIO_SETDATAOUT, 1<<4);     // column 3 = Drive GPIO36
            OUTREG32(&pDevice->pGPIO1Regs->ulGPIO_SETDATAOUT, 1<<12);    // column 4 = Drive GPIO12
            OUTREG32(&pDevice->pGPIO4Regs->ulGPIO_SETDATAOUT, 1<<1);     // column 5 = Drive GPIO97

            // Ie, drive all the column GPIO's in turn
            if (ic == 0)
                OUTREG32(&pDevice->pGPIO3Regs->ulGPIO_CLEARDATAOUT, 1<<26);    // column 0 = Drive GPIO90
            else if (ic == 1)
                OUTREG32(&pDevice->pGPIO3Regs->ulGPIO_CLEARDATAOUT, 1<<27);    // column 1 = Drive GPIO91
            else if (ic == 2)
                OUTREG32(&pDevice->pGPIO1Regs->ulGPIO_CLEARDATAOUT, 1<<15);    // column 2 = Drive GPIO15
            else if (ic == 3)
                OUTREG32(&pDevice->pGPIO2Regs->ulGPIO_CLEARDATAOUT, 1<<4);     // column 3 = Drive GPIO36
            else if (ic == 4)
                OUTREG32(&pDevice->pGPIO1Regs->ulGPIO_CLEARDATAOUT, 1<<12);    // column 4 = Drive GPIO12
            else if (ic == 5)
                OUTREG32(&pDevice->pGPIO4Regs->ulGPIO_CLEARDATAOUT, 1<<1);     // column 5 = Drive GPIO97

            // Get row status
            if ((INREG32(&pDevice->pGPIO3Regs->ulGPIO_DATAIN) & (1<<24)) == 0) // row 0 GPIO 88
            {
                DEBUGMSG(ZONE_INFO, (L"Row 0 detected when column #%d driven. \r\n", ic));
                data[ic] |= 1;
            }
            if ((INREG32(&pDevice->pGPIO3Regs->ulGPIO_DATAIN) & (1<<25)) == 0) // row 1 GPIO 89
            {
                DEBUGMSG(ZONE_INFO, (L"Row 1 detected when column #%d driven.\r\n", ic));
                data[ic] |= 1 << 1;
            }
            if ((INREG32(&pDevice->pGPIO4Regs->ulGPIO_DATAIN) & (1<<28)) == 0) // row 2 GPIO 124
            {
                DEBUGMSG(ZONE_INFO, (L"Row 2 detected when column #%d driven.\r\n", ic));
                data[ic] |= 1 << 2;
            }
            if ((INREG32(&pDevice->pGPIO1Regs->ulGPIO_DATAIN) & (1<<11)) == 0) // row 3 GPIO 11
            {
                DEBUGMSG(ZONE_INFO, (L"Row 3 detected when column #%d driven.\r\n", ic));
                data[ic] |= 1 << 3;
            }
            if ((INREG32(&pDevice->pGPIO1Regs->ulGPIO_DATAIN) & (1<<6)) == 0) // row 4 GPIO 6
            {
                DEBUGMSG(ZONE_INFO, (L"Row 4 detected when column #%d driven.\r\n", ic));
                data[ic] |= 1 << 4;
            }
        } // for loop

//      data[0] = ~data[0];
//      data[1] = ~data[1];
//      data[2] = ~data[2];
//      data[3] = ~data[3];
//      data[4] = ~data[4];
//      data[5] = ~data[5];
        //--------------------------------------------------------------
        // Convert physical state to virtual keys state
        //--------------------------------------------------------------
        // Get new state for virtual key table
        memset(vkNewState, 0, sizeof(vkNewState));
        keyDown = FALSE;
        for (ic = 0, ik = 0; ic < KEYPAD_COLUMNS; ic++) {
            data[ic] = ~data[ic];
            // Find pressed virtual keys
            if ((data[ic] & KEYPAD_ROWS_MASK) == KEYPAD_ROWS_MASK) {
                ik += KEYPAD_ROWS;
            } else for (ir = 0; ir < KEYPAD_ROWS; ir++, ik++) {
                if ((data[ic] & (1 << ir)) == 0) {
                    vk = g_keypadVK[ik];
                    vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                    keyDown = TRUE;
                    DEBUGMSG(ZONE_INFO, (L"keyDown = TRUE \r\n"));
                }
            }
        }

        //--------------------------------------------------------------
        // Remap multi virtual keys to final virtual key
        //--------------------------------------------------------------
        time = GetTickCount();
        for (ix = 0; ix < g_keypadRemap.count; ix++) {
            const KEYPAD_REMAP_ITEM *pItem = &g_keypadRemap.pItem[ix];
            KEYPAD_REMAP_STATE *pState = &pRemapState[ix];
            DWORD state = 0;
            USHORT down = 0;

            // Count number of keys down & save down/up state
            for (ik = 0; ik < pItem->keys; ik++) {
                vk = pItem->pVKeys[ik];
                if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0) {
                    state |= 1 << ik;
                    down++;
                }
            }
            // Depending on number of keys down
            if (down >= pItem->keys && pItem->keys > 1) {
                // Clear all mapping keys
                for (ik = 0; ik < pItem->keys; ik++) {
                    vk = pItem->pVKeys[ik];
                    vkNewState[vk >> 5] &= ~(1 << (vk & 0x1F));
                }
                // All keys are down set final key
                vk = pItem->vkey;
                vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: "
                    L"Mapped vkey: 0x%x\r\n", vk
                ));

                // Clear remap pending flag
                pState->pending = FALSE;
                // Set remap processing flag
                pState->remapped = TRUE;
            } else if (down > 0) {
                // If already remapping or remapping is not pending
                // or pending time expired
                if (pState->remapped || !pState->pending ||
                    (INT32)(time - pState->time) < 0 ) {
                    // If we are not pending and not already remapping, start
                    if (!pState->pending && !pState->remapped) {
                        pState->pending = TRUE;
                        pState->time = time + pItem->delay;
                    }
                    // Clear all mapping keys
                    for (ik = 0; ik < pItem->keys; ik++) {
                        vk = pItem->pVKeys[ik];
                        vkNewState[vk >> 5] &= ~(1 << (vk & 0x1F));
                    }
                } else if (
                    pItem->keys == 1 && (INT32)(time - pState->time) >= 0
                ) {
                    // This is press and hold key
                   DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: "
                        L"Mapped press and hold vkey: 0x%x\r\n", vk
                    ));
                    vk = pItem->vkey;
                    vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                    keyDown = TRUE;
                    pState->pending = FALSE;
                }
            } else {
                // All keys are up, if remapping was pending set keys
                if (pState->pending) {
                    for (ik = 0; ik < pItem->keys; ik++) {
                        if ((pState->state & (1 << ik)) != 0) {
                            vk = pItem->pVKeys[ik];
                            vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                            DEBUGMSG(ZONE_INFO, (L"keyDown = TRUE 2\r\n"));
                            keyDown = TRUE;
                        }
                    }
                    pState->pending = FALSE;
                }
                pState->remapped = FALSE;
            }
            // Save key state
            pState->state = state;
        }

        //--------------------------------------------------------------
        // Find pressed/released keys
        //--------------------------------------------------------------
        for (ic = 0, vk = 0; ic < VK_KEYS/DWORD_BITS; ic++) {
            change = vkState[ic] ^ vkNewState[ic];
            if (change == 0) {
                vk += DWORD_BITS;
            } else for (mask = 1; mask != 0; mask <<= 1, vk++) {
                // Check for change
                if ((change & mask) != 0) {
                    if ((vkNewState[ic] & mask) != 0)
                    {
                        DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: Key Down: 0x%x\r\n", vk ));
                        keybd_event(vk, 0, 0, 0);

                        //Notify for power manager events
                        if(vk == VK_TPOWER)
                        {
                            PowerPolicyNotify(PPN_POWERBUTTONPRESSED, 0);
                        }
                        else
                        {
                            //Application button pressed
                            //PM uses this to indicate user activity and reset timers
                            PowerPolicyNotify(PPN_APPBUTTONPRESSED, 0);
                        }

                        if(vk == VK_TSTAR)
                        {
                            DEBUGMSG(ZONE_IST, (L"VK_TSTAR\r\n"));
                        }
                    }
                    else
                    {
                        DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: Key Up: 0x%x\r\n", vk ));
                        keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
                    }
                }
            }
        }

        //--------------------------------------------------------------
        //  Check for auto-repeat keys
        //--------------------------------------------------------------
        for (ix = 0; ix < g_keypadRepeat.count; ix++) {
            const KEYPAD_REPEAT_ITEM *pItem = &g_keypadRepeat.pItem[ix];
            const KEYPAD_REPEAT_BLOCK *pBlock = pItem->pBlock;
            KEYPAD_REPEAT_STATE *pState = &pRepeatState[ix];
            DWORD delay;
            BOOL blockRepeat = FALSE;
            UCHAR vkBlock;

            vk = pItem->vkey;
            if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0) {
                if (!pState->pending) {
                    // Key was just pressed
                    delay = pItem->firstDelay;
                    if (delay == 0) delay = pDevice->firstRepeat;
                    pState->time = time + delay;
                    pState->pending = TRUE;
                    pState->blocked = FALSE;
                } else if ((INT32)(time - pState->time) >= 0) {
                    // Check if any blocking keys are pressed
                    if (pBlock != 0) {
                        for (ik = 0; ik < pBlock->count; ik++) {
                            vkBlock = pBlock->pVKey[ik];
                            if ((
                                vkNewState[vkBlock >> 5] &
                                (1 << (vkBlock & 0x1F))
                            ) != 0) {
                                pState->blocked = TRUE;
                                DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: "
                                    L"Block repeat: 0x%x because of 0x%x\r\n",
                                    vk, vkBlock
                                ));
                                break;
                            }
                        }
                    }
                    // Repeat if not blocked
                    if (!pState->blocked) {
                        DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: "
                            L"Key Repeat: 0x%x\r\n", vk
                        ));
                        keybd_event(vk, 0, pItem->silent?KEYEVENTF_SILENT:0, 0);
                    }
                    // Set time for next repeat
                    delay = pItem->nextDelay;
                    if (delay == 0) delay = pDevice->nextRepeat;
                    pState->time = time + delay;
                }
            } else {
                pState->pending = FALSE;
                pState->blocked = FALSE;
            }
        }

        DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: Prepare for next run\r\n"));
        //--------------------------------------------------------------
        // Prepare for next run
        //--------------------------------------------------------------
        // New state become old
        memcpy(vkState, vkNewState, sizeof(vkState));

        // Don't drive any column GPIO's
        OUTREG32(&pDevice->pGPIO3Regs->ulGPIO_CLEARDATAOUT, 1<<26);    // column 0 = Drive GPIO90
        OUTREG32(&pDevice->pGPIO3Regs->ulGPIO_CLEARDATAOUT, 1<<27);    // column 1 = Drive GPIO91
        OUTREG32(&pDevice->pGPIO1Regs->ulGPIO_CLEARDATAOUT, 1<<15);    // column 2 = Drive GPIO15
        OUTREG32(&pDevice->pGPIO2Regs->ulGPIO_CLEARDATAOUT, 1<<4);     // column 3 = Drive GPIO36
        OUTREG32(&pDevice->pGPIO1Regs->ulGPIO_CLEARDATAOUT, 1<<12);    // column 4 = Drive GPIO12
        OUTREG32(&pDevice->pGPIO4Regs->ulGPIO_CLEARDATAOUT, 1<<1);     // column 5 = Drive GPIO97

        // Set timeout period depending on data state
        timeout = keyDown ? pDevice->samplePeriod : INFINITE;
        DEBUGMSG(ZONE_IST, (L" KPD_IntrThread: InterruptDone, timeout set to %d\r\n", timeout));
        // Interrupt is done
        InterruptDone(pDevice->sysIntr);
    }

cleanUp:

    if (pRemapState != NULL) LocalFree(pRemapState);
    if (pRepeatState != NULL) LocalFree(pRepeatState);
    return ERROR_SUCCESS;
}

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
