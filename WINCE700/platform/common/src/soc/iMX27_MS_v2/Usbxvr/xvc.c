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
//  Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

 /*!
 *
 * @File: xvc.c
 *
 * Description:
 *          USB Transceiver Driver is a stream interface driver
 *          which exposes the stream interface functions. This driver
 *          detects the USB plug in type and accordingly activates the Host or
 *          Function controller driver. Upon unplug, the respective driver
 *          gives back the control to the transceiver driver.
 *
 */

#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <csp.h>
#include "xvc.h"
#include <mx27_usbname.h>
#include <mx27_usbcommon.h>

#ifdef USBXVR_INTERRUPT
#undef USBXVR_INTERRUPT
#endif
#define USBXVR_INTERRUPT 1

#ifdef DISABLE_DETACH_WAKEUP
#undef DISABLE_DETACH_WAKEUP
#endif
static const int MAX_RETRY= 100;

//------------------------------------------------------------------------------
//  Device registry parameters

//#define FREESCALE_DEBUG
static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"MemBase", PARAM_DWORD, TRUE, offset(USBXVC, memBase),
        fieldsize(USBXVC, memBase), NULL
    }, {
        L"MemLen", PARAM_DWORD, TRUE, offset(USBXVC, memLen),
        fieldsize(USBXVC, memLen), NULL
    }, {
        L"Irq", PARAM_DWORD, TRUE, offset(USBXVC, dwIrq),
        fieldsize(USBXVC, dwIrq), NULL
    },{
        L"OTGSupport", PARAM_DWORD, TRUE, offset(USBXVC, IsOTGSupport),
        fieldsize(USBXVC, IsOTGSupport), (void *)0
    },{
        L"OTGGroup", PARAM_STRING, TRUE, offset(USBXVC, szOTGGroup),
        fieldsize(USBXVC, szOTGGroup), NULL
    }
};

PUSBXVC pXVC;
BOOL    gfsvaim = FALSE;
HANDLE hWaitEvents[2];

extern void SetULPIToClientMode(CSP_USB_REGS *regs);
extern void RegisterCallback(BSP_USB_CALLBACK_FNS *pfn);
extern BOOL Initialize1504Client(CSP_USB_REGS * regs);


static BOOL InitClock()
{
    BOOL rc = FALSE;

    rc  = USBClockDisable(FALSE);

    return rc;

}
static BOOL DeInitClock(void)
{
    BOOL rc = FALSE;

    rc = USBClockDisable(TRUE);

    return rc;
}

/*
 *     Function: DllEntry
 *
 *     This is the entry and exit point for the XVC module.  This
 *     function is called when processed and threads attach and detach from this
 *     module.
 *
 *     Parameters:
 *     hInstDll
 *           [in] The handle to this module.
 *
 *     dwReason
 *           [in] Specifies a flag indicating why the DLL entry-point function
 *           is being called.
 *
 *     lpvReserved
 *           [in] Specifies further aspects of DLL initialization and cleanup.
 *
 *    Returns:
 *           TRUE if the XVC is initialized; FALSE if an error occurred during
 *           initialization.
 *
 */
BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
        DEBUGMSG(ZONE_INIT, (TEXT("XVC_DllEntry: DLL_PROCESS_ATTACH\r\n")));
        break;

        case DLL_PROCESS_DETACH:
        DEBUGMSG(ZONE_INIT, (TEXT("XVC_DllEntry: DLL_PROCESS_DETACH\r\n")));
        break;
    }

    return TRUE;
}


/*
 *      Function : USBControllerRun
 *
 *      Description: This sets USB controller to either run or halted
 *
 *      Input :
 *            BOOL   bRunMode  -- if TRUE, then set running, else set stopped
 *
 *       Output:
 *            void
 *
 *       Return:
 *            ERROR_SUCCESS - okay, mode set
 *            ERROR_GEN_FAILURE -- couldn't set the mode for some reason
 *
 *
 *       Function waits until controller is stopped or started.
 */

static DWORD USBControllerRun( CSP_USB_REGS *pRegs, BOOL bRunMode )
{
        USB_USBCMD_T cmd;
        USB_USBMODE_T mode;
        DWORD *pTmpCmd = (DWORD*)&cmd;
        DWORD *pTmpMode = (DWORD*)&mode;

        *pTmpCmd = INREG32(&pRegs->OTG.USBCMD);
        if ( bRunMode )
        {
            DEBUGMSG(1,(TEXT("Start USB controller (RS=%d -> RS=1)\r\n"), cmd.RS));
            cmd.RS = 1;
        }
        else
        {
            DEBUGMSG(1,(TEXT("Stop USB controller (RS=%d -> RS=0)\r\n"), cmd.RS));
            cmd.RS = 0;
        }


        *pTmpMode = INREG32(&pRegs->OTG.USBMODE);
        //CM-Controller Mode- 2 means Device Controller, 3 means Host Controller
        if ( mode.CM == 0x3 )
        {
            // in host mode, make sure HCH (halted) already, and that mode does change to halted (or not)
            USB_USBSTS_T stat;
            DWORD *pTmpStat = (DWORD*)&stat;
            int iAttempts;

            if ( bRunMode )
            {
                // wait for mode to change
                iAttempts = 0;
                do {
                    *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
                    if ( stat.HCH != 0 )
                        break;
                    Sleep(1);
                } while ( ++iAttempts < MAX_RETRY );

                if ( stat.HCH == 0)
                {
                    RETAILMSG(1,(TEXT("USBControllerRun(1): ERROR ############ HCH=0 (stat=0x%x)\r\n"),*pTmpStat));
                    return ERROR_GEN_FAILURE;
                }
            }

            OUTREG32(&pRegs->OTG.USBCMD,*pTmpCmd);

            // wait for mode to change
            iAttempts = 0;
            do {
                *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
                if ( (!bRunMode && stat.HCH) || (bRunMode && (stat.HCH == 0)) )
                    return ERROR_SUCCESS;

                Sleep(1);
            } while ( iAttempts++ < MAX_RETRY );

            RETAILMSG(1,(TEXT("USBControllerRun(%d): failed to set/clear RS\r\n"),bRunMode));
            return ERROR_GEN_FAILURE;
        }else
        {
            OUTREG32(&pRegs->OTG.USBCMD,*pTmpCmd);
        }


        return ERROR_SUCCESS;
}

static DWORD USBControllerReset( CSP_USB_REGS *pRegs )
{
        USB_USBCMD_T cmd;
        USB_USBMODE_T mode;
        DWORD *pTmpCmd = (DWORD*)&cmd;
        DWORD *pTmpMode = (DWORD*)&mode;
        int iAttempts;

        *pTmpMode = INREG32(&pRegs->OTG.USBMODE);

        if ( mode.CM == 0x03 )//if Host Controller
        {
            USB_USBSTS_T stat;
            DWORD *pTmpStat = (DWORD*)&stat;

            // wait for mode to change
            iAttempts = 0;
            do {
                *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
                if ( stat.HCH != 0 )
                    break;
                Sleep(1);
            } while ( ++iAttempts < MAX_RETRY );

            if ( stat.HCH == 0)
            {
                RETAILMSG(1,(TEXT("USBControllerReset: ########### HCH=0 (stat=0x%x)\r\n"),*pTmpStat));
                return ERROR_GEN_FAILURE;
            }
        }

        *pTmpCmd = INREG32(&pRegs->OTG.USBCMD);
        cmd.RST = 1;
        OUTREG32(&pRegs->OTG.USBCMD,*pTmpCmd);
        iAttempts = 0;
        do {
            Sleep(0);
            *pTmpCmd = INREG32(&pRegs->OTG.USBCMD);
            if ( cmd.RST == 0 )
                return ERROR_SUCCESS;

        } while ( iAttempts++ < MAX_RETRY );

        RETAILMSG(1,(TEXT("USBControllerReset: ######### Failed to reset controller\r\n")));
        return ERROR_GEN_FAILURE;
}


/*
 *      Function : XVC_ISTMain
 *
 *      Description: This thread activates the Function or Host depending on the
 *                   plug inserted.
 *
 *      Input :
 *            LPVOID lpParameter
 *
 *       Output:
 *            void
 */
#define IDLE_TIMEOUT    1000
DWORD WINAPI XVC_ISTMain(LPVOID lpParameter)
{
    HANDLE hFunction, hXcvr, hHost, hTransferEvent;
    ULONG WaitReturn;
    CSP_USB_REGS *pUSBRegs = pXVC->pUSBRegs;
    TCHAR szUSBFunctionObjectName[30];
    TCHAR szUSBXcvrObjectName[30];
    TCHAR szUSBHostObjectName[30];
    TCHAR szUSBTransferEventName[30];
    DWORD timeout = IDLE_TIMEOUT;


    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_ISTMain+\r\n")));

    // Event created for Function
    StringCchCopy(szUSBFunctionObjectName, _countof(szUSBFunctionObjectName),USBFunctionObjectName);
    StringCchCat(szUSBFunctionObjectName, _countof(szUSBFunctionObjectName), pXVC->szOTGGroup);
    //RETAILMSG(1, (TEXT("XVC: CreateEvent:%s\r\n"), szUSBFunctionObjectName));
    hFunction = CreateEvent(NULL, FALSE, FALSE, szUSBFunctionObjectName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Opened an existing Func Event\r\n")));
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Created a new Func Event\r\n")));
    if (hFunction == NULL)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Create Event Failed for func!\r\n")));

    // Event created for Transceiver
    StringCchCopy(szUSBXcvrObjectName,_countof(szUSBXcvrObjectName),USBXcvrObjectName);
    StringCchCat(szUSBXcvrObjectName, _countof(szUSBXcvrObjectName), pXVC->szOTGGroup);
    //RETAILMSG(1, (TEXT("XVC: CreateEvent:%s\r\n"), szUSBXcvrObjectName));
    hXcvr = CreateEvent(NULL, FALSE, FALSE, szUSBXcvrObjectName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Opened an existing XCVR Event\r\n")));
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Created a new XCVR Event\r\n")));
    if (hXcvr == NULL)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Create Event Failed for XCVR!\r\n")));

    // Event created for Host
    StringCchCopy(szUSBHostObjectName, _countof(szUSBHostObjectName),USBHostObjectName);
    StringCchCat(szUSBHostObjectName, _countof(szUSBHostObjectName), pXVC->szOTGGroup);
    //RETAILMSG(1, (TEXT("XVC: CreateEvent:%s\r\n"), szUSBHostObjectName));
    hHost = CreateEvent(NULL, FALSE, FALSE, szUSBHostObjectName);
    if(GetLastError()==ERROR_ALREADY_EXISTS)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Opened an existing Host Event\r\n")));
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Created a new Host Event\r\n")));
    if (hHost == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Create Event Failed for host!\r\n")));
    }

    // Event created for transferring control from function controller
    StringCchCopy(szUSBTransferEventName, _countof(szUSBTransferEventName),USBTransferEventName);
    StringCchCat(szUSBTransferEventName, _countof(szUSBTransferEventName), pXVC->szOTGGroup);

    hTransferEvent = CreateEvent(NULL, FALSE, FALSE, szUSBTransferEventName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Opened an existing Transfer Event\r\n")));
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Created a new Transfer Event\r\n")));
    if (hTransferEvent == NULL)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Create Event Failed for Transfer!\r\n")));

    hWaitEvents[1] = hTransferEvent;

    // Initializes of the Transceiver driver, configure as client all the time.
    USBControllerRun(pUSBRegs, FALSE);
    RETAILMSG(1,(TEXT("OTG Trasceiver Stop \r\n")));

    //#if (USB_HOST_MODE ==1)
    //    Initialize1504Client(pRegs);
    //#endif
    //RETAILMSG(1,(TEXT("Init 1504 \r\n")));

    USBControllerReset(pUSBRegs);
    RETAILMSG(1,(TEXT("OTG Trasceiver Reset \r\n")));

    InitializeOTGTransceiver(&pUSBRegs, FALSE);
    RETAILMSG(1,(TEXT("Init OTG Trasceiver \r\n")));

    USBControllerRun(pUSBRegs, TRUE);
    RETAILMSG(1,(TEXT("OTG Trasceiver Run \r\n")));
    Sleep(100);

    while (!pXVC->ExitInterruptThread) {
        USB_USBSTS_T source={0};
        USB_PORTSC_T state={0};
        USB_CTRL_T ctrl={0};
        USB_OTGSC_T otgsc={0};
        DWORD *temp;
        DWORD *temp2;
        DWORD *temp3;
        DWORD *temp4;
        BOOL newIntrInit = FALSE;
        BOOL deviceDetected = FALSE;

        pXVC->bInXVC = TRUE;

        WaitReturn = WaitForMultipleObjects(2, hWaitEvents, FALSE, timeout);
        switch(WaitReturn)
        {
            case WAIT_OBJECT_0 + 0:
                DEBUGMSG(1, (TEXT("$$$$$Interrupt Event is Set\r\n")));
                break;
            case WAIT_OBJECT_0 + 1:
                DEBUGMSG(1, (TEXT("$$$$$Transfer Event Set\r\n")));
                goto XVR_CTRL;
                break;
            case WAIT_TIMEOUT:
                DEBUGMSG(1, (TEXT("$$$$$Timeout Occured \r\n")));
                break;
            default:
                RETAILMSG(1, (TEXT("Unknown Event Fired. WaitReturn= %d \r\n"), WaitReturn));
                break;
        }

        if (WaitReturn == WAIT_TIMEOUT)
        {
            // Now I am ready to put the transceiver into suspend mode.
            // But be aware there is a device attach when boot up
            USB_PORTSC_T portsc;
            DWORD *temp = (DWORD *)&portsc;
            int i = 0;

            do {
                *temp = INREG32(&(pUSBRegs)->OTG.PORTSC);
            } while ((portsc.SUSP == 0) && (i++ < 100));

            if (portsc.SUSP == 0)
            {
                RETAILMSG(1, (TEXT("Failure to suspend the port, there must be something happening\r\n")));
                RETAILMSG(1,
                    (TEXT("Receive interrupt with OTGSC (0x%x) USBSTS (0x%x), PORTSC (0x%x), USBCTRL (0x%x)\r\n"),
                    INREG32(&pUSBRegs->OTG.OTGSC), INREG32(&pUSBRegs->OTG.USBSTS), INREG32(&pUSBRegs->OTG.PORTSC[0]),
                    INREG32(&pUSBRegs->USB_CTRL)));


                DEBUGMSG(1, (TEXT("Forcing port suspend anyway\r\n")));
                USBControllerRun(pUSBRegs, FALSE);
           }

            // Check and make sure all wakeup interrupt are enabled
            temp3 = (DWORD *)&ctrl;
            *temp3 = INREG32(&pUSBRegs->USB_CTRL);
            ctrl.OWIE = 1;
            ctrl.OUIE = 1;
            SETREG32(&pUSBRegs->USB_CTRL, *temp3);
                //RETAILMSG(1, (TEXT("Set USB Ctrl register\r\n")));
                //DumpULPIRegs(pUSBRegs);

            // Lock Start
            EnterCriticalSection(&pXVC->csPhyLowMode);
            //RETAILMSG(1, (TEXT("WaitTimeOut\r\n")));

            // Config the ULPI again to make sure
            *temp = INREG32(&(pUSBRegs)->OTG.PORTSC);
            // stop the controller before accessing the ULPI
            USBControllerRun(pUSBRegs, FALSE);

            if (portsc.PHCD == 0)
            {
                RETAILMSG(1, (TEXT("Setting ULPI to Client\r\n")));
                SetULPIToClientMode(pUSBRegs);
            }

            //USBControllerReset(pUSBRegs);



            if (pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode)
            {
                 pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode(pUSBRegs, FALSE);
            }

            {

                if (pXVC->bPanicMode == TRUE)
                {
                    pXVC->bPanicMode = FALSE;
                }
            }

            if (pXVC->bUSBCoreClk == TRUE)
            {
                pXVC->bUSBCoreClk = FALSE;
                DeInitClock();
            }
            LeaveCriticalSection(&pXVC->csPhyLowMode);
            // Now we can stop the USB clock
            RETAILMSG(1, (TEXT("XVC - SUSPEND\r\n")));
            timeout = INFINITE;
            continue;
        }

//INTERRUPT_PROCESSING:
        EnterCriticalSection(&pXVC->csPhyLowMode);
        if (pXVC->bUSBCoreClk == FALSE)
        {
            InitClock();
            pXVC->bUSBCoreClk = TRUE;
            Sleep(200);
            USBControllerRun(pUSBRegs, TRUE);
        }

        if (pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode)
        {
           pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode(pUSBRegs, TRUE);
        }

        temp=(DWORD *)&source;
        *temp = INREG32(&pUSBRegs->OTG.USBSTS);

        temp2 = (DWORD *)&state;
        *temp2 = INREG32(&pUSBRegs->OTG.PORTSC[0]);

        temp3 = (DWORD *)&ctrl;
        *temp3 = INREG32(&pUSBRegs->USB_CTRL);

        temp4 = (DWORD *)&otgsc;
        *temp4 = INREG32(&pUSBRegs->OTG.OTGSC);

#if 0
        RETAILMSG(1, (TEXT("Receive interrupt with OTGSC (0x%x) USBSTS (0x%x), PORTSC (0x%x), USBCTRL (0x%x)\r\n"),
            INREG32(&pUSBRegs->OTG.OTGSC), *temp, *temp2, *temp3));

        DumpUSBRegs(pUSBRegs);
#endif

        if (ctrl.OWIR == 1)
        {
            *temp3 = 0;
            ctrl.OWIE = 1;
            CLRREG32(&pUSBRegs->USB_CTRL, *temp3);
        }

        if ((source.PCI == 1) && (state.CCS == 1))
        {
            // We have a device attached
            if (pXVC->devState == 0)
            {
                if (pXVC->bPanicMode == FALSE)
                {
                    pXVC->bPanicMode = TRUE;
                }
#ifdef USBXVR_INTERRUPT
                InterruptDisable (pXVC->dwSysIntr); // to disable the interrupt
#endif
                // Don't clean up any interrupt, let the corresponding driver handle that
                if (INREG32(&pUSBRegs->OTG.OTGSC) & 0x100) // B-device
                {
                    RETAILMSG(1, (TEXT("B-Device client\r\n")));

#ifdef FREESCALE_DEBUG
                    DumpULPIRegs(pUSBRegs);
#endif
                    USBControllerRun(pUSBRegs, TRUE);

                    // By default is client, no need to do any configuration
                    SetEvent(hFunction);

                }
                else
                {
                    RETAILMSG(1, (TEXT("A-Device host\r\n")));

                    USBControllerRun(pUSBRegs, FALSE);
                    USBControllerReset(pUSBRegs );

                    // Configure the host
                    SetOTGTranseiverMode((PCSP_USB_REGS *)&pUSBRegs, TRUE); //To Host

                    SetEvent(hHost);
                }
                deviceDetected = TRUE;
            } // pXVC->devState == 0
        }   // source.PCI==1 && state.CCS==1
        else if ((pXVC->devState == 0) && (deviceDetected == FALSE))
        {
            DWORD temp = INREG32(&pUSBRegs->OTG.OTGSC);

            if ((temp & 0x100) == 0x0)      //ID-0 A Device, 1 B Device
            {
                if (pXVC->bPanicMode == FALSE)
                {
                    pXVC->bPanicMode = TRUE;
                }


#ifdef USBXVR_INTERRUPT
                InterruptDisable (pXVC->dwSysIntr); // to disable the interrupt
#endif
                RETAILMSG(1, (TEXT("A-device host from polling\r\n")));

                // Exit thread when we are asked so...
                if (pXVC->ExitInterruptThread)
                {
                    //Close handle for function event
                    if (hFunction != NULL)
                    {
                        CloseHandle(hFunction);
                        hFunction = NULL;
                    }

                    //Close handle for transceiver event
                    if (hXcvr != NULL)
                    {
                        CloseHandle(hXcvr);
                        hXcvr = NULL;
                    }

                    //Close handle for Transfer event
                    if (hTransferEvent != NULL)
                    {
                        CloseHandle(hTransferEvent);
                        hTransferEvent = NULL;
                    }

                    //Close handle for host event
                    if (hHost != NULL)
                    {
                        CloseHandle(hHost);
                        hHost = NULL;
                    }
                    break;
                }

                //Configure the host

                USBControllerRun(pUSBRegs, FALSE);
                USBControllerReset(pUSBRegs );

                SetOTGTranseiverMode((PCSP_USB_REGS *)&pUSBRegs, TRUE);//To host

                SetEvent(hHost);
                deviceDetected = TRUE;
            }
        }


        if ((pXVC->devState == 0) && (deviceDetected == TRUE))
        {
            pXVC->devState = 1;
            pXVC->bInXVC = FALSE;
            //pXVC->bResume = FALSE;

#ifdef DISABLE_DETACH_WAKEUP
            // Clear any wakeup interrupt as there would be a problem on host mode
            *temp3 = 0;
            ctrl.OWIE = 1;
            ctrl.OUIE = 1;
            CLRREG32(&pUSBRegs->USB_CTRL, *temp3);

            // Disable wakeup
            KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &pXVC->dwSysIntr, sizeof(pXVC->dwSysIntr), NULL, 0, NULL);
#endif
            // Wait for detach occur
            WaitReturn = WaitForSingleObject(hXcvr, INFINITE);
            LeaveCriticalSection(&pXVC->csPhyLowMode);

XVR_CTRL:
            EnterCriticalSection(&pXVC->csPhyLowMode);
            pXVC->bInXVC = TRUE;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("XVR In control again\r\n")));

            temp=(DWORD *)&source;
            *temp = INREG32(&pUSBRegs->OTG.USBSTS);

                // I think we need to do some charge, discharge ....
#ifdef USBXVR_INTERRUPT
            newIntrInit = TRUE;
            if (!(InterruptInitialize(pXVC->dwSysIntr, pXVC->hIntrEvent, NULL, 0))) {
                newIntrInit = FALSE;
                DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Interrupt initialization failed!, ErrCode: 0x%x\r\n"), GetLastError()));
            }

#endif

#ifdef DISABLE_DETACH_WAKEUP
            // Enable wakeup
            KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &pXVC->dwSysIntr, sizeof(pXVC->dwSysIntr), NULL, 0, NULL);
#endif
            //Switch to Device mode only if it's required
            {
                USB_USBMODE_T Mode;
                DWORD *dwCMMode = (DWORD*) &Mode;
                *dwCMMode = INREG32(&pUSBRegs->OTG.USBMODE);

                if( (Mode.CM != CM_DEVICE_CONTROLLER))
                {
                    // Initializes of the Transceiver driver
                    USBControllerRun(pUSBRegs, FALSE);
                    USBControllerReset(pUSBRegs);

                    SetOTGTranseiverMode((PCSP_USB_REGS *)&pUSBRegs, FALSE); //To Device

                    USBControllerRun(pUSBRegs, TRUE);
                }
            }
            pXVC->devState = 0;
            deviceDetected = FALSE;
            timeout = IDLE_TIMEOUT;

        }

        if (newIntrInit == FALSE) {
            // Clear source bit
            OUTREG32(&pUSBRegs->OTG.USBSTS, *temp);
#ifdef USBXVR_INTERRUPT
            InterruptDone(pXVC->dwSysIntr);
#endif
        }
        LeaveCriticalSection(&pXVC->csPhyLowMode);

        timeout = IDLE_TIMEOUT;

    } // While loop
    // Never to enter this region
    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_ISTMain-\r\n")));
    return 0;
}

/*
 *      Function: XVC_Init
 *
 *      The Device Manager calls this function as a result of a call to the
 *      ActivateDevice() function.  This function will retrieve the handle to the
 *      device context from the registry and then initialize the XVC module with
 *      interrupt initialisation.
 *
 *      Parameters:
 *           dwContext
 *               [in] Pointer to a string containing the registry path to the
 *               active key for the stream interface driver.
 *
 *      Returns:
 *           Returns a handle to the device context created if successful. Returns
 *           zero if not successful.
 */
DWORD XVC_Init(DWORD dwContext)
{
    PHYSICAL_ADDRESS pa;
    DWORD len;
    DWORD irq;
    LPCTSTR pszActiveKey;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    pXVC = LocalAlloc(LPTR, sizeof(USBXVC));
    if (pXVC == NULL)
    {
        RETAILMSG(1, (TEXT("XVC_Init: Cannot allocate memory\r\n")));
        goto clean;
    }

    memset(pXVC, 0x00, sizeof(USBXVC));

    // Read device parameters
    pszActiveKey = (LPCTSTR) dwContext;
    if (GetDeviceRegistryParams(pszActiveKey, pXVC, dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: XVC_Init: "L"Failed read registry parameters\r\n"));
        goto clean;
    }

    if (pXVC->IsOTGSupport == 0)
    {
        RETAILMSG(1, (TEXT("OTG Not support and unload the driver\r\n")));
        goto clean;
    }

    // Initialize CriticalSection object for USB PHY Low-power mode
    InitializeCriticalSection(&pXVC->csPhyLowMode);

    // Initialize XVC variables
    pXVC->bUSBCoreClk = TRUE;
    pXVC->bPanicMode = TRUE;

    //pXVC->bResume = FALSE;

    RegisterCallback(&pXVC->fnUsbXvr);
    // Map the USB registers
    pa.QuadPart = ((pXVC->memBase == 0)? CSP_BASE_REG_PA_USB: pXVC->memBase);
    len = ((pXVC->memLen == 0)? 0x1000: pXVC->memLen);

    pXVC->pUSBRegs = MmMapIoSpace(pa, len, FALSE);
    if (pXVC->pUSBRegs == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: XVC_Init: Controller registers mapping failed\r\n"));
        goto clean;
    }

    irq = ((pXVC->dwIrq == 0)? 0x25: pXVC->dwIrq);
#if 1
    pXVC->dwSysIntr = GetSysIntr();
#else
    if (!KernelIoControl(
            IOCTL_HAL_REQUEST_SYSINTR, &(pXVC->dwIrq), sizeof(DWORD),
            &pXVC->dwSysIntr, sizeof(pXVC->dwSysIntr), NULL
        )) goto clean;
#endif


    USBClockCreateFileMapping();
    pXVC->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pXVC->hIntrEvent == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: CreateEvent failed\r\n")));
        goto clean;
    }
    hWaitEvents[0] = pXVC->hIntrEvent;
#ifdef USBXVR_INTERRUPT
    // to initialize the interrupt
    if (!(InterruptInitialize(pXVC->dwSysIntr, pXVC->hIntrEvent, NULL, 0))) {
        DEBUGMSG(1, (TEXT("XVC_Init: Interrupt initialization failed!, ErrCode: 0x%x\r\n"), GetLastError()));
        goto clean;
    }

    //RETAILMSG(1, (_T("Transceiver driver working in interrupt mode.\n")));
#else
    RETAILMSG(1, (TEXT("XVC_Init: Transceiver driver working in polling mode\r\n")));
#endif

    KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &pXVC->dwSysIntr, sizeof(pXVC->dwSysIntr), NULL, 0, NULL);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Interrupt initialized\r\n")));
    pXVC->ExitInterruptThread = FALSE;
    pXVC->hInterruptServiceThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XVC_ISTMain, NULL, 0, NULL);

    if (pXVC->hInterruptServiceThread == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Failed to create Interrupt Service Thread\r\n")));
        goto clean;
    }

    CeSetThreadPriority(pXVC->hInterruptServiceThread, 102);
    return (DWORD)pXVC;

clean:
    if (pXVC && pXVC->pUSBRegs)
        MmUnmapIoSpace((VOID*)pXVC->pUSBRegs, 0x1000);

    if (pXVC)
        LocalFree(pXVC);

    return (DWORD)NULL;
}

/*
 *      Function: XVC_Deinit
 *
 *      The Device Manager calls this function as a result of a call to the
 *      DeactivateDevice() function.  This function will return any resources
 *      allocated while using this driver.
 *
 *      Parameters:
 *           dwContext [in]
 *
 *      Returns:
 *           TRUE
 */

BOOL XVC_Deinit(DWORD dwContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Deinit: dwContext = 0x%x\r\n"), dwContext));

        // Stop interrupt thread
    if (pXVC->hInterruptServiceThread != NULL)
    {
        pXVC->ExitInterruptThread = TRUE;
        SetEvent(pXVC->hIntrEvent);
        WaitForSingleObject(pXVC->hInterruptServiceThread, INFINITE);
        CloseHandle(pXVC->hInterruptServiceThread);
    }

    // Close interrupt handler
    if (pXVC->hIntrEvent != NULL)
    {
        CloseHandle(pXVC->hIntrEvent);
        pXVC->hIntrEvent = NULL;
    }

    // Unmap USB controller registers
    if (pXVC && pXVC->pUSBRegs)
    {
        MmUnmapIoSpace((VOID*)pXVC->pUSBRegs, 0x1000);
        pXVC->pUSBRegs = NULL;
    }

    // Release interrupt
    if (pXVC->dwSysIntr != 0)
    {
        InterruptDisable(pXVC->dwSysIntr);
        pXVC->dwSysIntr = 0;
    }

    USBClockDeleteFileMapping();

    // Free XVC context
    if (pXVC)
        LocalFree(pXVC);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Deinit\r\n"), dwContext));
    return TRUE;
}

/*
 *      Function: XVC_Open
 *
 *      Called when an application attempts to establish a connection to this driver.
 *      This function will verify that a trusted application has made the request and
 *      deny access to all non-trusted applications.
 *
 *      Parameters:
 *            dwData [in]
 *            dwAccess [in]
 *            dwShareMode [in]
 *
 *      Returns:
 *            dwData
 */

DWORD XVC_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Open: dwData = 0x%x, dwAccess = 0x%x, dwShareMode = 0x%x\r\n"), dwData, dwAccess, dwShareMode));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Open\r\n")));
    return dwData;
}

/*
 *       Function: XVC_Close
 *
 *       Called when an application attempts to close a connection to this driver.
 *
 *       Parameters:
 *             Handle [in]
 *
 *       Returns:
 *             TRUE
 */

BOOL XVC_Close(DWORD Handle)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Close: Handle = 0x%x\r\n"), Handle));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Close: Handle = 0x%x\r\n"), Handle));
    return TRUE;
}

/*
 *     Function: XVC_PowerDown
 *
 *     This function suspends power to the device. It is useful only with devices
 *     that can power down under software control.
 *
 *     Parameters:
 *          void
 *     Returns:
 *          void
 */

void XVC_PowerDown(void)
{
    CSP_USB_REGS *pUSBRegs = pXVC->pUSBRegs;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_PowerDown \r\n")));
    if (pXVC->bInXVC) {
        // Lock Start
        EnterCriticalSection(&pXVC->csPhyLowMode);
        if (pXVC->fnUsbXvr.pfnUSBPowerDown)
        {
            pXVC->fnUsbXvr.pfnUSBPowerDown(pUSBRegs, &pXVC->bUSBCoreClk, &pXVC->bPanicMode);
        }
        LeaveCriticalSection(&pXVC->csPhyLowMode);
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_PowerDown \r\n")));

    return;

}

/*
 *
 *   Function: XVC_PowerUp
 *
 *   This function restores power to a device.
 *
 *   Parameters:
 *        void
 *   Returns:
 *       void
 */

void XVC_PowerUp(void)
{

    CSP_USB_REGS *pUSBRegs = pXVC->pUSBRegs;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_PowerUP \r\n")));
    if (pXVC->bInXVC) {
        EnterCriticalSection(&pXVC->csPhyLowMode);

        if (pXVC->fnUsbXvr.pfnUSBPowerUp)
        {
            pXVC->fnUsbXvr.pfnUSBPowerUp(pUSBRegs, &pXVC->bUSBCoreClk, &pXVC->bPanicMode);
        }
        LeaveCriticalSection(&pXVC->csPhyLowMode);
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_PowerUp \r\n")));


}

/*
 *   Function: XVC_Read
 *
 *   This function reads data from the device identified by the open context.
 *
 *   Parameters:
 *        Handle [in]
 *        pBuffer [out]
 *        dwNumBytes [in]
 *   Returns:
 *        Zero
 *
 */

DWORD XVC_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Read: Handle = 0x%x, pBuffer = 0x%x, dwNumBytes = 0x%x\r\n"), Handle, pBuffer, dwNumBytes));
    return 0;
}

/*
 *    Function: XVC_Write
 *
 *    This function writes data to the device.
 *
 *    Parameters:
 *         Handle [in]
 *         pBuffer [out]
 *         dwNumBytes [in]
 *
 *    Returns:
 *         zero
 */

DWORD XVC_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Write: Handle = 0x%x, pBuffer = 0x%x, dwNumBytes = 0x%x\r\n"), Handle, pBuffer, dwNumBytes));
    return 0;
}

/*
 *    Function: XVC_Seek
 *
 *    This function moves the data pointer in the device.
 *
 *    Parameters:
 *         Handle [in]
 *         lDistance
 *         dwMoveMethod
 *
 *    Returns:
 *        -1
 */

DWORD XVC_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Seek: Handle = 0x%x, lDistance = 0x%x, dwMoveMethod = 0x%x\r\n"), Handle, lDistance, dwMoveMethod));
    return (DWORD) -1;
}

/*
 *      Function: XVC_IOControl
 *
 *      Called when an application calls the DeviceIoControl() function.  This
 *      function operates differently based upon the IOCTL that is passed to it.
 *
 *      Parameters:
 *           Handle [in]
 *
 *           dwIoControlCode [in] The IOCTL requested.
 *
 *          pInBuf [in] Input buffer.
 *
 *          nInBufSize [in] Length of the input buffer.
 *
 *           pOutBuf [out] Output buffer.
 *
 *           nOutBufSize [in] The length of the output buffer.
 *
 *           pBytesReturned [out] Size of output buffer returned to application.
 *
 *      Returns:
 *           TRUE if the IOCTL is handled. FALSE if the IOCTL was not recognized or
 *           an error occurred while processing the IOCTL
 */

BOOL XVC_IOControl(VOID *pXVCContext,
                   DWORD code,
                   UCHAR *pInBuffer,
                   DWORD inSize,
                   UCHAR *pOutBuffer,
                   DWORD outSize,
                   DWORD *pOutSize)

{
    BOOL bReturn = FALSE;
    CSP_USB_REGS *pUSBRegs = ((PUSBXVC)pXVCContext)->pUSBRegs;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_IOControl: dwIoControlCode = 0x%x\r\n"), code));
    switch (code)
    {
        case IOCTL_POWER_CAPABILITIES:
            if (!pOutBuffer || outSize < sizeof(POWER_CAPABILITIES))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                bReturn = FALSE;
            }
            else
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pOutBuffer;

                // Clear capabilities structure.
                memset(ppc, 0, sizeof(POWER_CAPABILITIES));

               // Set power capabilities. Supports D0 and D4.
                ppc->DeviceDx = DX_MASK(D0)|DX_MASK(D4);

                // Update returned data size.
                if (pOutSize)
                    *pOutSize = sizeof(POWER_CAPABILITIES);
                bReturn = TRUE;
            }
            break;

        case IOCTL_POWER_GET: // gets the current device power state
            if (!pOutBuffer || outSize < sizeof(CEDEVICE_POWER_STATE) )
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                bReturn = FALSE;
            }
            else
            {
                *(PCEDEVICE_POWER_STATE) pOutBuffer = pXVC->CurPMPowerState;
                if (pOutSize) {
                    *pOutSize = sizeof(CEDEVICE_POWER_STATE);
                }
                bReturn = TRUE;
            }
            break;

        case IOCTL_POWER_SET:
            if (!pOutBuffer || outSize < sizeof(CEDEVICE_POWER_STATE) )
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                bReturn = FALSE;
            }
            else
            {
                pXVC->CurPMPowerState = *(PCEDEVICE_POWER_STATE) pOutBuffer;

                if (pXVC->CurPMPowerState == D0)
                {
                    if (pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode)
                    {
                        if(USBClockInit())
                        {
                            pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode(pUSBRegs, TRUE);
                        }
                    }
                }
                else if (pXVC->CurPMPowerState == D4)
                {
                    if (pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode)
                    {
                        if (USBClockInit())
                        {
                            pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode(pUSBRegs, FALSE);
                        }
                    }
                }

                DEBUGMSG(ZONE_FUNCTION, (TEXT("USBOTG::IOControl:IOCTL_POWER_SET: D%d\r\n"), pXVC->CurPMPowerState));

                // did we set the device power?
                if (pOutSize) {
                    *pOutSize = sizeof(CEDEVICE_POWER_STATE);
                }
                bReturn = TRUE;
            }
            break;

      default:
            break;
    }
    return bReturn;
}

/* EOF*/
