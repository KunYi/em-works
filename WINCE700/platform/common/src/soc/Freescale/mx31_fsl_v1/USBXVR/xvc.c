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
// Copyright (C) 2005-2007, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
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

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#ifdef USB_ELEGANT_INTR
#include <oal.h>
#endif
#pragma warning(pop)

#include <csp.h>
#include "xvc.h"
#include <mx31_usbname.h>
#include <mx31_usbcommon.h>

#ifdef USBXVR_INTERRUPT
#undef USBXVR_INTERRUPT
#endif
#define USBXVR_INTERRUPT 1

#ifdef DISABLE_DETACH_WAKEUP
#undef DISABLE_DETACH_WAKEUP
#endif

#define USBXVR_TO2  0x20
// Remove-W4: Warning C4053 workaround
// The following definition public\common\oak\inc\usbfn.h caused
// warning C4053: one void operand for '?:', so we just use #pragma
// to get rid of those MSFT stuff.

#define DEBUG_LOG_USBCV 0

#pragma warning(disable: 4053)
//-----------------------------------------------------------------------------
//  Device registry parameters

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

extern void SetULPIToClientMode(CSP_USB_REGS *regs);
extern void RegisterCallback(BSP_USB_CALLBACK_FNS *pfn);
#ifdef CABLE_SOLUTION
extern BOOL ISP1504Poweroff(CSP_USB_REGS * regs);
extern BOOL ISP1504PowerUp(CSP_USB_REGS * regs);
#endif
void PowerUp2Dev(CSP_USB_REGS* pUSBRegs);
void PowerUp2Host(CSP_USB_REGS* pUSBRegs);
//----------------------------------------------------
//
// Function:  InitClock
//
// This function is to initialize the USB clock
//
// Parameter:
//
//    Nil
//
// Return:
//
//   TRUE - success, FALSE - failure
//
//------------------------------------------------------
static BOOL InitClock()
{
    BOOL rc = FALSE;

    rc  = BSPUSBClockDisable(FALSE);

    return rc;

}

//----------------------------------------------------
//
// Function:  DeInitClock
//
// This function is to de-initialize the USB clock
//
// Parameter:
//
//    Nil
//
// Return:
//
//   TRUE - success, FALSE - failure
//
//------------------------------------------------------
static BOOL DeInitClock(void)
{
    BOOL rc = FALSE;

    rc = BSPUSBClockDisable(TRUE);

    return rc;
}


//------------------------------------------------------
//     Function: DllEntry
//
//     This is the entry and exit point for the XVC module.  This
//     function is called when processed and threads attach and detach from
//     this module.
//
//     Parameters:
//     hInstDll
//           [in] The handle to this module.
//
//     dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//     lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
//    Returns:
//           TRUE if the XVC is initialized; FALSE if an error occurred during
//           initialization.
//------------------------------------------------------

BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            DEBUGMSG(ZONE_INIT, (TEXT("XVC_DllEntry: DLL_PROCESS_ATTACH\r\n")));
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT, (TEXT("XVC_DllEntry: DLL_PROCESS_DETACH\r\n")));
            break;

        default:
            break;
    }

    return TRUE;
}


#ifndef DEBUG
//----------------------------------------------------------------
//
//  Function:  DumpUSBRegs
//
//  Dump all the registers in MX31 usb memory map
//
//  Parameter:
//      pUSBDRegs - pointer to usb memory map
//
//  Return
//     NULL
//
//----------------------------------------------------------------
void DumpUSBRegs(CSP_USB_REGS* pUSBDRegs)
{
    // offset 0x000 ~ 0x604
    DWORD offset;
    DWORD rangeS = 0x0, rangeE = 0x1f;

#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(pUSBDRegs);
#endif

    RETAILMSG(1, (L"USBCMD %x\r\n", INREG32(&pUSBDRegs->OTG.USBCMD)));
    RETAILMSG(1, (L"USBSTS %x\r\n", INREG32(&pUSBDRegs->OTG.USBSTS)));
    RETAILMSG(1, (L"PORTSC %x\r\n", INREG32(&pUSBDRegs->OTG.PORTSC)));
    RETAILMSG(1, (L"USBINTR %x\r\n", INREG32(&pUSBDRegs->OTG.USBINTR)));
    RETAILMSG(1, (L"USBCTRL %x\r\n", INREG32(&pUSBDRegs->USB_CTRL)));

    RETAILMSG(1, (L"%4x ~ %4x\t", rangeS, rangeE));
    for (offset = 0; offset <= 0x604; offset += 4)
    {
        RETAILMSG(1, (L"%12x", INREG32((PUCHAR)(pUSBDRegs) + offset)));
        if ((offset + 4) % 32 == 0)
        {
            RETAILMSG(1, (L"\r\n"));
            rangeS += 32;
            rangeE += 32;
            RETAILMSG(1, (L"%4x ~ %4x\t", rangeS, rangeE));
        }
    }
    RETAILMSG(1, (L"\r\n"));
}
#endif

//----------------------------------------------------
//      Function : USBControllerRun
//
//      Description: This sets USB controller to either run or halted
//
//      Input :
//            BOOL   bRunMode  -- if TRUE, then set running, else set stopped
//
//       Output:
//            void
//
//       Return:
//            ERROR_SUCCESS - okay, mode set
//            ERROR_GEN_FAILURE -- couldn't set the mode for some reason
//
//
//       Function waits until controller is stopped or started.
//----------------------------------------------------

static DWORD USBControllerRun( CSP_USB_REGS *pRegs, BOOL bRunMode )
{
        USB_USBCMD_T cmd;
        USB_USBMODE_T mode;
        DWORD *pTmpCmd = (DWORD*)&cmd;
        DWORD *pTmpMode = (DWORD*)&mode;

        *pTmpCmd = INREG32(&pRegs->OTG.USBCMD);

        if ( bRunMode )
        {
            DEBUGMSG(ZONE_FUNCTION,(TEXT("Start USB controller (RS=%d -> RS=1)\r\n"), cmd.RS));
            cmd.RS = 1;
        }
        else
        {
            DEBUGMSG(ZONE_FUNCTION,(TEXT("Stop USB controller (RS=%d -> RS=0)\r\n"), cmd.RS));
            cmd.RS = 0;
        }

        OUTREG32(&pRegs->OTG.USBCMD,*pTmpCmd);

        *pTmpMode = INREG32(&pRegs->OTG.USBMODE);
        if ( mode.CM == 0x3 )
        {
            // in host mode, make sure HCH (halted) already, and that mode does change to halted (or not)
            USB_USBSTS_T stat;
            DWORD *pTmpStat = (DWORD*)&stat;
            int iAttempts;

            RETAILMSG(FALSE, (L"-RCM\r\n"));
            if ( bRunMode )
            {
                *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
                if ( stat.HCH == 0)
                {
                    DEBUGMSG(ZONE_ERROR,(TEXT("USBControllerRun(1): ERROR ############ HCH=0 (stat=0x%x)\r\n"),*pTmpStat));
                    return ERROR_GEN_FAILURE;
                }
            }

            // wait for mode to change
            iAttempts = 0;
            do {
                *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
                if ( (!bRunMode && stat.HCH) || (bRunMode && (stat.HCH == 0)) )
                    return ERROR_SUCCESS;

                Sleep(1);
            } while ( iAttempts++ < 50 );

            DEBUGMSG(ZONE_ERROR,(TEXT("USBControllerRun(%d): failed to set/clear RS\r\n"),bRunMode));
            return ERROR_GEN_FAILURE;
        }

        return ERROR_SUCCESS;
}

//-------------------------------------------------------------------------
//
//      Function : USBControllerReset
//
//      Description: This reset the USB Controller
//
//      Parameters :
//          pRegs - Pointer to 3 USB Core registers
//
//
//      Return:
//           ERROR_SUCCESS - okay, mode set
//           ERROR_GEN_FAILURE -- couldn't set the mode for some reason
//
//
//--------------------------------------------------------------------------

static DWORD USBControllerReset( CSP_USB_REGS *pRegs )
{
        USB_USBCMD_T cmd;
        USB_USBMODE_T mode;
        DWORD *pTmpCmd = (DWORD*)&cmd;
        DWORD *pTmpMode = (DWORD*)&mode;
        int iAttempts;

        *pTmpMode = INREG32(&pRegs->OTG.USBMODE);

        if ( mode.CM == 0x03 )
        {
            USB_USBSTS_T stat;
            DWORD *pTmpStat = (DWORD*)&stat;

            *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
            if ( stat.HCH == 0)
            {
            DEBUGMSG(ZONE_ERROR,(TEXT("USBControllerReset: ########### HCH=0 (stat=0x%x)\r\n"),*pTmpStat));
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

        } while ( iAttempts++ < 1000 );

    DEBUGMSG(ZONE_ERROR,(TEXT("USBControllerReset: ######### Failed to reset controller\r\n")));
        return ERROR_GEN_FAILURE;
}

//----------------------------------------------------
//      Function : InitializeTransceiver
//
//      Description: This  Function initializes the transceiver
//
//      Input :
//            CSP_USB_REGS *pUSBRegs
//
//       Output:
//            void
//----------------------------------------------------

__inline void InitializeTransceiver(CSP_USB_REGS *pUSBRegs)
{
    // Initializes the Transceiver driver, configure as client all the time.
    USBControllerRun(pUSBRegs, FALSE);
    USBControllerReset(pUSBRegs);
#ifdef CABLE_SOLUTION
    {
        DWORD* tmpOtgsc;
        USB_OTGSC_T otgsc;

        tmpOtgsc = (DWORD*)&otgsc;
        *tmpOtgsc = INREG32(&(pUSBRegs)->OTG.OTGSC);
        if (otgsc.ID)
        {
            //B device
            InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, FALSE);
        }
        else
        {
            //A device
            InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, TRUE);
        }
    }
#else
    InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, FALSE);
#endif

    USBControllerRun(pUSBRegs, TRUE);
}

//----------------------------------------------------
//      Function : XVC_ISTMain
//
//      Description: This thread activates the Function or Host depending on the
//                   plug inserted.
//
//      Input :
//            LPVOID lpParameter
//
//       Output:
//            void
//----------------------------------------------------

#define IDLE_TIMEOUT    1500
DWORD WINAPI XVC_ISTMain(LPVOID lpParameter)
{
    HANDLE hFunction, hXcvr, hHost, hTransfer;
    ULONG WaitReturn;
    CSP_USB_REGS *pUSBRegs = pXVC->pUSBRegs;
    TCHAR szUSBFunctionObjectName[30];
    TCHAR szUSBTransferObjectName[30];
    TCHAR szUSBXcvrObjectName[30];
    TCHAR szUSBHostObjectName[30];
    DWORD timeout = IDLE_TIMEOUT;
    DWORD StringSize = 0;
    HANDLE hWait[2];

    UNREFERENCED_PARAMETER(lpParameter);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_ISTMain+\r\n")));

    // Event created for Function
    StringSize = sizeof(szUSBFunctionObjectName) / sizeof(TCHAR);
    StringCchCopy(szUSBFunctionObjectName,StringSize,USBFunctionObjectName);
    //lstrcpy(szUSBFunctionObjectName, USBFunctionObjectName);
    StringCchCat(szUSBFunctionObjectName, StringSize, pXVC->szOTGGroup);
    //lstrcat(szUSBFunctionObjectName, pXVC->szOTGGroup);
    hFunction = CreateEvent(NULL, FALSE, FALSE, szUSBFunctionObjectName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Opened an existing Func Event\r\n")));
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Created a new Func Event\r\n")));
    if (hFunction == NULL)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Create Event Failed for func!\r\n")));

    // Event created for Transceiver
    StringSize = sizeof(szUSBXcvrObjectName) / sizeof(TCHAR);
    StringCchCopy(szUSBXcvrObjectName,StringSize,USBXcvrObjectName);
    //lstrcpy(szUSBXcvrObjectName, USBXcvrObjectName);
    StringCchCat(szUSBXcvrObjectName, StringSize, pXVC->szOTGGroup);
    //lstrcat(szUSBXcvrObjectName, pXVC->szOTGGroup);
    hXcvr = CreateEvent(NULL, FALSE, FALSE, szUSBXcvrObjectName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Opened an existing XCVR Event\r\n")));
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Created a new XCVR Event\r\n")));
    if (hXcvr == NULL)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Create Event Failed for XCVR!\r\n")));

    // Event created for Host
    StringSize = sizeof(szUSBHostObjectName) / sizeof(TCHAR);
    StringCchCopy(szUSBHostObjectName,StringSize,USBHostObjectName);
    //lstrcpy(szUSBHostObjectName, USBHostObjectName);
    StringCchCat(szUSBHostObjectName, StringSize, pXVC->szOTGGroup);
    //lstrcat(szUSBHostObjectName, pXVC->szOTGGroup);
    hHost = CreateEvent(NULL, FALSE, FALSE, szUSBHostObjectName);
    if(GetLastError()==ERROR_ALREADY_EXISTS)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Opened an existing Host Event\r\n")));
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Created a new Host Event\r\n")));
    if (hHost == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Create Event Failed for host!\r\n")));
    }

    //Event created for Transfer
    StringSize = sizeof(szUSBTransferObjectName) / sizeof(TCHAR);
    StringCchCopy(szUSBTransferObjectName,StringSize,USBTransferObjectName);
    StringCchCat(szUSBTransferObjectName, StringSize, pXVC->szOTGGroup);

    hTransfer = CreateEvent(NULL, FALSE, FALSE, szUSBTransferObjectName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("UFN: Opened an existing Transfer Event\r\n")));
    }
    else
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("UFN: Created a new Transfer Event\r\n")));
    }
    if (hTransfer == NULL)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("UFN: Create Event Failed for Transfer!\r\n")));
    }

    hWait[0] = pXVC->hIntrEvent;
    hWait[1] = hTransfer;

    // the code for mouse cold boot is no longer needed here

    // Initializes the Transceiver driver, configure as client all the time.
    InitializeTransceiver(pUSBRegs);
    Sleep(100);

    for(;;) {
        USB_USBSTS_T source;
        USB_PORTSC_T state;
        USB_CTRL_T ctrl;
        USB_OTGSC_T otgsc;
        DWORD *temp;
        DWORD *temp2;
        DWORD *temp3;
        DWORD *temp4;
        BOOL newIntrInit = FALSE;
        BOOL deviceDetected = FALSE;

        pXVC->bInXVC = TRUE;

        RETAILMSG(FALSE,(TEXT(" XVC WAITING ON INTERRUPT EVENT\n")));

        RETAILMSG(DEBUG_LOG_USBCV, (L"xvc w %x\r\n", timeout));

        WaitReturn = WaitForMultipleObjects(2, hWait, FALSE, timeout);
        switch(WaitReturn)
        {
            case WAIT_OBJECT_0 + 0:
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("Interrupt Event\r\n")));
                break;
            }
            case WAIT_OBJECT_0 + 1:
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("Transfer Event\r\n")));
#ifdef USBXVR_INTERRUPT
                if (!(InterruptInitialize(pXVC->dwSysIntr, pXVC->hIntrEvent, NULL, 0)))
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("XVC_Init: Interrupt initialization failed!, ErrCode: 0x%x\r\n"), GetLastError()));
                }
#endif
                // Initializes the Transceiver driver
                InitializeTransceiver(pUSBRegs);
                continue;
            }
            case WAIT_TIMEOUT:
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("Time Out\r\n")));
                break;
            }
        }

        if (WaitReturn == WAIT_TIMEOUT)
        {
            // Now I am ready to put the transceiver into suspend mode.
            // But be aware there is a device attach when boot up
            USB_PORTSC_T portsc;
            int i = 0;
            temp = (DWORD *)&portsc;
            RETAILMSG(FALSE, (L" XVC TIMED OUT!!\n"));
#ifdef USBCV_FIX
            temp2 = (DWORD*)&otgsc;
#endif

            // Enable wakeup logic before suspending
            // Check and make sure all wakeup interrupt are enabled
            temp3 = (DWORD *)&ctrl;
            *temp3 = INREG32(&pUSBRegs->USB_CTRL);
            ctrl.OWIE = 1;
            ctrl.OUIE = 1;
            SETREG32(&pUSBRegs->USB_CTRL, *temp3);
            Sleep(100);

            do {
                //RETAILMSG(1, (TEXT("SUSP waiting\r\n")));
                *temp = INREG32(&(pUSBRegs)->OTG.PORTSC);
            } while ((portsc.SUSP == 0) && (i++ < 100));

            if (portsc.SUSP == 0)
            {
                RETAILMSG(DEBUG_LOG_USBCV, (L"XVC susp?\r\n"));
                DEBUGMSG(ZONE_ERROR, (TEXT("Failure to suspend the port, there must be something happening\r\n")));
                DEBUGMSG(ZONE_ERROR,
                    (TEXT("Receive interrupt with OTGSC (0x%x) USBSTS (0x%x), PORTSC (0x%x), USBCTRL (0x%x)\r\n"),
                    INREG32(&pUSBRegs->OTG.OTGSC), INREG32(&pUSBRegs->OTG.USBSTS), INREG32(&pUSBRegs->OTG.PORTSC[0]),
                    INREG32(&pUSBRegs->USB_CTRL)));
                DEBUGMSG(ZONE_ERROR, (TEXT("Forcing port suspend anyway\r\n")));
                USBControllerRun(pUSBRegs, FALSE);
#ifdef CABLE_SOLUTION
                *temp2=INREG32(&(pUSBRegs)->OTG.OTGSC);
                if (otgsc.ID)
                {
                    //B device
                    InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, FALSE);
                }
                else
                {
                    //A device
                    InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, TRUE);
                }
#else
                InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, FALSE);
#endif
                USBControllerRun(pUSBRegs, TRUE);
                // otherwise, just give up waiting for the port to suspend and try
                // forcing everything off.
            }
            RETAILMSG(FALSE,(TEXT(" XVC TIMED OUT- Requesting CS pXVC->csPhyLowMode!!\n")));
            // Lock Start
            EnterCriticalSection(&pXVC->csPhyLowMode);

            RETAILMSG(FALSE,(TEXT(" XVC TIMED OUT- Obtained CS pXVC->csPhyLowMode!!\n")));
            // Config the ULPI again to make sure
            *temp = INREG32(&(pUSBRegs)->OTG.PORTSC);

            // stop the controller before accessing the ULPI
            USBControllerRun(pUSBRegs, FALSE);
            Sleep(10);
#ifdef CABLE_SOLUTION
            if ((otgsc.ID == 1) && (portsc.PHCD == 0))
            {
                SetULPIToClientMode(pUSBRegs);
            }
#else
            if (portsc.PHCD == 0)
                SetULPIToClientMode(pUSBRegs);
#endif

            if (pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode)
                 pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode(pUSBRegs, FALSE);

            {
              if(!pXVC->bIsMX31TO2)
              {
                    BOOL bTemp;
                    if (pXVC->bPanicMode == TRUE)
                    {
                        bTemp = DDKClockDisablePanicMode();
                        pXVC->bPanicMode = FALSE;
                    }
              }
            }
            if (pXVC->bUSBCoreClk == TRUE)
            {
                RETAILMSG(FALSE, (L"DeInit Clock\r\n"));
                pXVC->bUSBCoreClk = FALSE;
                DeInitClock();
            }

            LeaveCriticalSection(&pXVC->csPhyLowMode);
            RETAILMSG(FALSE,(TEXT(" XVC TIMED OUT- Released CS pXVC->csPhyLowMode!!\n")));
            // Now we can stop the USB clock
            RETAILMSG(FALSE, (TEXT("XVC - SUSPEND\r\n")));
            DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC - SUSPEND\r\n")));
            timeout = INFINITE;
            continue;
        }

        RETAILMSG(FALSE, (L"Get event\r\n"));

        RETAILMSG(FALSE, (L"XVC INTERRUPT!!!\n"));

        EnterCriticalSection(&pXVC->csPhyLowMode);

        if (pXVC->bUSBCoreClk == FALSE)
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"Clock\r\n"));
            InitClock();
            pXVC->bUSBCoreClk = TRUE;
        }

        USBControllerRun(pUSBRegs, TRUE);

        if (pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode)
           pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode(pUSBRegs, TRUE);    //BSPUsbXvrSetPhyPowerMode

        temp=(DWORD *)&source;
        *temp = INREG32(&pUSBRegs->OTG.USBSTS);

        temp2 = (DWORD *)&state;
        *temp2 = INREG32(&pUSBRegs->OTG.PORTSC[0]);

        temp3 = (DWORD *)&ctrl;
        *temp3 = INREG32(&pUSBRegs->USB_CTRL);

        temp4 = (DWORD *)&otgsc;
        *temp4 = INREG32(&pUSBRegs->OTG.OTGSC);
#ifdef USBCV_FIX
        OUTREG32(&pUSBRegs->OTG.OTGSC, *temp4);
        RETAILMSG(DEBUG_LOG_USBCV, (L"after clear otgsc %x\r\n", INREG32(&pUSBRegs->OTG.OTGSC)));
#endif

        RETAILMSG(DEBUG_LOG_USBCV, (L"USBSTS %x, PORTSC %x, OTGSC %x, CTRL %x, USBINTR %x\r\n", *temp, *temp2, *temp4, *temp3, INREG32(&pUSBRegs->OTG.USBINTR)));

        if (ctrl.OWIR == 1)
        {
            RETAILMSG(FALSE, (L"XVC WAS A WAKE UP INTERRUPT !!!\n"));
            *temp3 = 0;
            ctrl.OWIE = 1;
            CLRREG32(&pUSBRegs->USB_CTRL, *temp3);
            Sleep(50);
            //RETAILMSG(FALSE, (TEXT("Clear the wake up bit and continue\r\n")));
            //DumpULPIRegs(pUSBRegs);
        }


        if (pXVC->bResume) {
            RETAILMSG(FALSE, (L"XVC:Resume again\r\n"));
            pXVC->bResume = FALSE;
            // We remove the controller stop, reset and run part, since when wakeup due to device attach
            // the first interrupt would be just wakeup but not the device attached status. The transceiver needs
            // some times to resume from suspend before processing the device status change interrupt.
            // If we reset immediately, the transceiver would go into weird state.
            //USBControllerRun(pUSBRegs, FALSE);
            //InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, FALSE);
            //USBControllerRun(pUSBRegs, TRUE);
            pXVC->devState = 0;
            deviceDetected = FALSE;
            timeout = IDLE_TIMEOUT;
            // remove continue and let it to do the work, if something plug in
            //continue;
        }

        if ((source.PCI == 1) && (state.CCS == 1))
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"XVC DEVICE IS CONNECTED\n"));
            // We have a device attached
            if (pXVC->devState == 0)
            {
                if(!pXVC->bIsMX31TO2)
                 {
                        if (pXVC->bPanicMode == FALSE)
                        {
                            DDKClockEnablePanicMode();
                            pXVC->bPanicMode = TRUE;
                        }
                }
#ifdef USBXVR_INTERRUPT
                InterruptDisable (pXVC->dwSysIntr); // to disable the interrupt
#endif
                // Don't clean up any interrupt, let the corresponding driver handle that
                //Sleep(1500); // there is a delay for OTGSC to detect the value
                //Sleep(1000);
                //RETAILMSG(1, (TEXT("OTGSC = 0x%x\r\n"), INREG32(&pUSBRegs->OTG.OTGSC)));
                if (INREG32(&pUSBRegs->OTG.OTGSC) & 0x100) // B-device
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("B-Device client\r\n")));
                    //RETAILMSG(1, (TEXT("Receive interrupt with USBSTS (0x%x), PORTSC (0x%x)\r\n"),
                    //  *temp, *temp2));

                    USBControllerRun(pUSBRegs, TRUE);

                    // By default is client, no need to do any configuration
                    RETAILMSG(DEBUG_LOG_USBCV, (L"X->F1\r\n"));
                    SetEvent(hFunction);

                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("A-Device host\r\n")));

                    USBControllerRun(pUSBRegs, FALSE);
                    USBControllerReset(pUSBRegs);

                    // Configure the host
                    InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, TRUE);

                    RETAILMSG(DEBUG_LOG_USBCV, (L"X->H1\r\n"));
                    SetEvent(hHost);
                }
                deviceDetected = TRUE;
            } // pXVC->devState == 0
        }   // source.PCI==1 && state.CCS==1
        else if ((pXVC->devState == 0) && (deviceDetected == FALSE))
        {
            DWORD temp_otgsc = INREG32(&pUSBRegs->OTG.OTGSC);
            RETAILMSG(FALSE,(TEXT(" XVC -AFTER INT -  DEVICE NOT DETECTED!!!\n")));

            if ((temp_otgsc & 0x100) == 0x0)
            {
                if(!pXVC->bIsMX31TO2)
                {
                    if (pXVC->bPanicMode == FALSE)
                    {
                        DDKClockEnablePanicMode();
                        pXVC->bPanicMode = TRUE;
                    }
                }

#ifdef USBXVR_INTERRUPT
                InterruptDisable (pXVC->dwSysIntr); // to disable the interrupt
#endif
                DEBUGMSG(ZONE_FUNCTION, (TEXT("A-device host from polling\r\n")));
                //Configure the host

                USBControllerRun(pUSBRegs, FALSE);
                Sleep(10);
                USBControllerReset(pUSBRegs);
                Sleep(100);

                InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, TRUE);
                RETAILMSG(FALSE,(TEXT(" XVC SIGNAL THE HOST - TO CHW!\n")));

                RETAILMSG(DEBUG_LOG_USBCV, (L"X->H2\r\n"));
                SetEvent(hHost);
                deviceDetected = TRUE;
            }
        }


        if ((pXVC->devState == 0) && (deviceDetected == TRUE))
        {
            RETAILMSG(FALSE, (L"XVC - RETURNED FROM CHW!\n"));
            pXVC->devState = 1;
            pXVC->bInXVC = FALSE;
            pXVC->bResume = FALSE;

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

            pXVC->bInXVC = TRUE;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("XVR In control again\r\n")));

                // I think we need to do some charge, discharge ....
#ifdef USBXVR_INTERRUPT
            newIntrInit = TRUE;
            if (!(InterruptInitialize(pXVC->dwSysIntr, pXVC->hIntrEvent, NULL, 0))) {
                newIntrInit = FALSE;
                DEBUGMSG(ZONE_ERROR, (TEXT("XVC_Init: Interrupt initialization failed!, ErrCode: 0x%x\r\n"), GetLastError()));
            }
#endif

#ifdef DISABLE_DETACH_WAKEUP
            // Enable wakeup
            KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &pXVC->dwSysIntr, sizeof(pXVC->dwSysIntr), NULL, 0, NULL);
#endif
            // Initializes the Transceiver driver
            InitializeTransceiver(pUSBRegs);

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
#if 0 // Remove-W4: Warning C4702 workaround
    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_ISTMain-\r\n")));
    return 0;
#endif
}

//----------------------------------------------------
//      Function: XVC_Init
//
//      The Device Manager calls this function as a result of a call to the
//      ActivateDevice() function.  This function will retrieve the handle to the
//      device context from the registry and then initialize the XVC module with
//      interrupt initialisation.
//
//      Parameters:
//           dwContext
//               [in] Pointer to a string containing the registry path to the
//               active key for the stream interface driver.
//
//      Returns:
//           Returns a handle to the device context created if successful. Returns
//           zero if not successful.
//----------------------------------------------------

DWORD XVC_Init(DWORD dwContext)
{
    PHYSICAL_ADDRESS pa;
    HANDLE hInterruptServiceThread;
    DWORD len;
    DWORD irq;
    LPCTSTR pszActiveKey;
    DWORD *pSREV = NULL;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    RETAILMSG(DEBUG_LOG_USBCV, (L"ERIC xvc_init .....\r\n"));

    pXVC = LocalAlloc(LPTR, sizeof(USBXVC));
    if (pXVC == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("XVC_Init: Cannot allocate memory\r\n")));
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
        DEBUGMSG(ZONE_ERROR, (TEXT("OTG Not support and unload the driver\r\n")));
        goto clean;
    }

    // Initialize CriticalSection object for USB PHY Low-power mode
    InitializeCriticalSection(&pXVC->csPhyLowMode);

    // Initialize XVC variables
//    pXVC->bUSBCoreClk = TRUE;
    pXVC->bUSBCoreClk = FALSE;

    pXVC->bResume = FALSE;

    RegisterCallback(&pXVC->fnUsbXvr);
    // Map the USB registers
    pa.QuadPart = ((pXVC->memBase == 0)? CSP_BASE_REG_PA_USBOTG: pXVC->memBase);
    len = ((pXVC->memLen == 0)? 0x1000: pXVC->memLen);

    pXVC->pUSBRegs = MmMapIoSpace(pa, len, FALSE);
    if (pXVC->pUSBRegs == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: XVC_Init: Controller registers mapping failed\r\n"));
        goto clean;
    }

    //Map SREV register - Used for Tape Out identification, for removal of PANIC mode changes in TO2
    pa.QuadPart = (CSP_BASE_REG_PA_IIM + 0x24);
    len = 0x04; // Four BYTES
    pSREV = MmMapIoSpace(pa, len, FALSE);
    if(pSREV ==NULL){
    DEBUGMSG(ZONE_ERROR, (L"ERROR: XVC_Init: SREV register mapping failed\r\n"));
        goto clean;
    }
    RETAILMSG(FALSE, (TEXT("SREV register = %x\n"),*(pSREV)));

    if(((*pSREV)&0xf0) == USBXVR_TO2)
    {
        pXVC->bIsMX31TO2 = TRUE;
    }
    else
    {
        pXVC->bIsMX31TO2 = FALSE;
    }

    RETAILMSG(FALSE,(TEXT("pXVC->bIsMX31TO2  = %d"),pXVC->bIsMX31TO2));

    if (pSREV)
    {
       MmUnmapIoSpace((VOID*)pSREV, 0x04);
       pSREV = NULL;
    }

    if(!pXVC->bIsMX31TO2)
    {
        pXVC->bPanicMode = TRUE;
    }
    irq = ((pXVC->dwIrq == 0)? 0x25: pXVC->dwIrq);
#ifdef USB_ELEGANT_INTR
    {
        INT32 aIrqs[3];
        aIrqs[0] = -1;
        aIrqs[1] = OAL_INTR_TRANSLATE;
        aIrqs[2] = IRQ_USB_OTG;
        KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(aIrqs), &pXVC->dwSysIntr, sizeof(pXVC->dwSysIntr), NULL);
    }
#else
    pXVC->dwSysIntr = GetSysIntr();
#endif

    BSPUSBClockCreateFileMapping();
    pXVC->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pXVC->hIntrEvent == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: CreateEvent failed\r\n")));
        goto clean;
    }

#ifdef USBXVR_INTERRUPT
    // to initialize the interrupt
    if (!(InterruptInitialize(pXVC->dwSysIntr, pXVC->hIntrEvent, NULL, 0))) {
        DEBUGMSG(ZONE_ERROR, (TEXT("XVC_Init: Interrupt initialization failed!, ErrCode: 0x%x\r\n"),
                                    GetLastError()));
        goto clean;
    }

    //RETAILMSG(1, (_T("Transceiver driver working in interrupt mode.\n")));
#else
    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Transceiver driver working in polling mode\r\n")));
#endif

    //KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &pXVC->dwSysIntr, sizeof(pXVC->dwSysIntr), NULL, 0, NULL);

    //RETAILMSG(1, (TEXT("Wakeup enable for transceiver\r\n")));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Interrupt initialized\r\n")));
    hInterruptServiceThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XVC_ISTMain, NULL, 0, NULL);

    if (hInterruptServiceThread == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Failed to create Interrupt Service Thread\r\n")));
        goto clean;
    }

    CeSetThreadPriority(hInterruptServiceThread, 102);

    //RETAILMSG(1,(TEXT("-XVC_Init: \r\n")));
    return (DWORD)pXVC;

clean:
    if (pXVC && pXVC->pUSBRegs)
        MmUnmapIoSpace((VOID*)pXVC->pUSBRegs, 0x1000);

    if (pXVC)
        LocalFree(pXVC);

    return (DWORD)NULL;
}

//----------------------------------------------------
//      Function: XVC_Deinit
//
//      The Device Manager calls this function as a result of a call to the
//      DeactivateDevice() function.  This function will return any resources
//      allocated while using this driver.
//
//      Parameters:
//           dwContext [in]
//
//      Returns:
//           TRUE
//----------------------------------------------------

BOOL XVC_Deinit(DWORD dwContext)
{
    UNREFERENCED_PARAMETER(dwContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Deinit: dwContext = 0x%x\r\n"), dwContext));
    if (pXVC && pXVC->pUSBRegs)
        MmUnmapIoSpace((VOID*)pXVC->pUSBRegs, 0x1000);

    BSPUSBClockDeleteFileMapping();
    if (pXVC)
        LocalFree(pXVC);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Deinit\r\n"), dwContext));
    return TRUE;
}

//----------------------------------------------------
//      Function: XVC_Open
//
//      Called when an application attempts to establish a connection to this driver.
//      This function will verify that a trusted application has made the request and
//      deny access to all non-trusted applications.
//
//      Parameters:
//            dwData [in]
//            dwAccess [in]
//            dwShareMode [in]
//
//      Returns:
//            dwData
//----------------------------------------------------

DWORD XVC_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwAccess);
    UNREFERENCED_PARAMETER(dwShareMode);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Open: dwData = 0x%x, dwAccess = 0x%x, dwShareMode = 0x%x\r\n"),
                                    dwData, dwAccess, dwShareMode));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Open\r\n")));
    return dwData;
}

//----------------------------------------------------
//       Function: XVC_Close
//
//       Called when an application attempts to close a connection to this driver.
//
//       Parameters:
//             Handle [in]
//
//       Returns:
//             TRUE
//----------------------------------------------------

BOOL XVC_Close(DWORD Handle)
{
    UNREFERENCED_PARAMETER(Handle);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Close: Handle = 0x%x\r\n"), Handle));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Close: Handle = 0x%x\r\n"), Handle));
    return TRUE;
}

//----------------------------------------------------
//     Function: XVC_PowerDown
//
//     This function suspends power to the device. It is useful only with devices
//     that can power down under software control.
//
//     Parameters:
//          void
//     Returns:
//          void
//----------------------------------------------------

void XVC_PowerDown(void)
{
    CSP_USB_REGS *pUSBRegs = pXVC->pUSBRegs;
    USB_USBCMD_T cmd;
    DWORD *pTmpCmd = (DWORD*)&cmd;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_PowerDown \r\n")));
    if (pXVC->bInXVC) {
        // Lock Start
#ifdef CABLE_SOLUTION
        if (pXVC->bUSBCoreClk == FALSE)
        {
            InitClock();
            pXVC->bUSBCoreClk = TRUE;
        }
        //clear USBINTR
        pXVC->dwUSBIntrValue = INREG32(&pUSBRegs->OTG.USBINTR);
        OUTREG32(&pUSBRegs->OTG.USBINTR, 0);

        // stop the controller so no attach will happen at once
        // when system resume
        *pTmpCmd = INREG32(&pUSBRegs->OTG.USBCMD);
        cmd.RS = 0;
        OUTREG32(&pUSBRegs->OTG.USBCMD, *pTmpCmd);

        if (pXVC->bUSBCoreClk == TRUE)
        {
            DeInitClock();
            pXVC->bUSBCoreClk = FALSE;
        }
#endif
        EnterCriticalSection(&pXVC->csPhyLowMode);
        if (pXVC->fnUsbXvr.pfnUSBPowerDown)    // this is acturally a NULL function
            pXVC->fnUsbXvr.pfnUSBPowerDown(pUSBRegs, &pXVC->bUSBCoreClk, &pXVC->bPanicMode);
        LeaveCriticalSection(&pXVC->csPhyLowMode);
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_PowerDown \r\n")));
    return;
}

//----------------------------------------------------
//
//   Function: XVC_PowerUp
//
//   This function restores power to a device.
//
//   Parameters:
//        void
//   Returns:
//       void
//----------------------------------------------------

void XVC_PowerUp(void)
{
    CSP_USB_REGS *pUSBRegs = pXVC->pUSBRegs;
    USB_OTGSC_T otgsc;
    DWORD* temp = (DWORD*)&otgsc;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_PowerUP \r\n")));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_PowerUp \r\n")));
    if (pXVC->bInXVC) {
        EnterCriticalSection(&pXVC->csPhyLowMode);
        if (pXVC->fnUsbXvr.pfnUSBPowerUp)   // this is acturally a NULL function
            pXVC->fnUsbXvr.pfnUSBPowerUp(pUSBRegs, &pXVC->bUSBCoreClk, &pXVC->bPanicMode);
        LeaveCriticalSection(&pXVC->csPhyLowMode);
#ifdef CABLE_SOLUTION
        if (pXVC->bUSBCoreClk == FALSE)
        {
            InitClock();
            pXVC->bUSBCoreClk = TRUE;
        }
        //recover USBINTR
        OUTREG32(&pUSBRegs->OTG.USBINTR, pXVC->dwUSBIntrValue);

        // We need to do a re-configuration at resume
        *temp = INREG32(&pUSBRegs->OTG.OTGSC);
        if (otgsc.ID)
        {
            // B device
            // Since we are in PowerUp function, no sleep can be called
            // we have to re-write InitializeOTGTransceiver
            PowerUp2Dev(pUSBRegs);
        }
        else
        {
            // A device
            PowerUp2Host(pUSBRegs);
        }

        if (pXVC->bUSBCoreClk == TRUE)
        {
            DeInitClock();
            pXVC->bUSBCoreClk = FALSE;
        }
#endif
        pXVC->bResume = TRUE;
        SetInterruptEvent(pXVC->dwSysIntr);
    }
    //RETAILMSG(1, (TEXT("-XVC_PowerUp \r\n")));

}

//----------------------------------------------------
//   Function: XVC_Read
//
//   This function reads data from the device identified by the open context.
//
//   Parameters:
//        Handle [in]
//        pBuffer [out]
//        dwNumBytes [in]
//   Returns:
//        Zero
//
//----------------------------------------------------

DWORD XVC_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Read: Handle = 0x%x, pBuffer = 0x%x, dwNumBytes = 0x%x\r\n"),
                                    Handle, pBuffer, dwNumBytes));
    return 0;
}

//----------------------------------------------------
//    Function: XVC_Write
//
//    This function writes data to the device.
//
//    Parameters:
//         Handle [in]
//         pBuffer [out]
//         dwNumBytes [in]
//
//    Returns:
//         zero
//----------------------------------------------------

DWORD XVC_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Write: Handle = 0x%x, pBuffer = 0x%x, dwNumBytes = 0x%x\r\n"),
                                    Handle, pBuffer, dwNumBytes));
    return 0;
}

//----------------------------------------------------
//    Function: XVC_Seek
//
//    This function moves the data pointer in the device.
//
//    Parameters:
//         Handle [in]
//         lDistance
//         dwMoveMethod
//
//    Returns:
//        -1
//----------------------------------------------------

DWORD XVC_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(lDistance);
    UNREFERENCED_PARAMETER(dwMoveMethod);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Seek: Handle = 0x%x, lDistance = 0x%x, dwMoveMethod = 0x%x\r\n"),
                                    Handle, lDistance, dwMoveMethod));
    return (DWORD) -1;
}

//----------------------------------------------------
//      Function: XVC_IOControl
//
//      Called when an application calls the DeviceIoControl() function.  This
//      function operates differently based upon the IOCTL that is passed to it.
//
//      Parameters:
//           Handle [in]
//
//           dwIoControlCode [in] The IOCTL requested.
//
//          pInBuf [in] Input buffer.
//
//          nInBufSize [in] Length of the input buffer.
//
//           pOutBuf [out] Output buffer.
//
//           nOutBufSize [in] The length of the output buffer.
//
//           pBytesReturned [out] Size of output buffer returned to application.
//
//      Returns:
//           TRUE if the IOCTL is handled. FALSE if the IOCTL was not recognized or
//           an error occurred while processing the IOCTL
//----------------------------------------------------

BOOL XVC_IOControl(
                   VOID *pXVCContext, IOCTL_SOURCE source, DWORD code, UCHAR *pInBuffer,
                   DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize
                   )

{
    CE_BUS_POWER_STATE *pBusPowerState;

    UNREFERENCED_PARAMETER(pOutSize);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(inSize);
    UNREFERENCED_PARAMETER(pXVCContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_IOControl: dwIoControlCode = 0x%x\r\n"), code));
    //RETAILMSG(1, (TEXT("+XVC_IOControl: dwIoControlCode = 0x%x\r\n"), code));
    switch (code)
    {
        case IOCTL_BUS_GET_POWER_STATE:
            if (source != MDD_IOCTL)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("IOCTL_BUS_GET_POWER_STATE not MDD_IOCTL\r\n")));
                break;
            }

            DEBUGMSG(ZONE_FUNCTION, (TEXT("IOCTL_BUS_GET_POWER_STATE\r\n")));
            pBusPowerState = (CE_BUS_POWER_STATE*)pInBuffer;
            *pBusPowerState->lpceDevicePowerState = pXVC->CurPMPowerState;
            break;

        case IOCTL_BUS_SET_POWER_STATE:
            RETAILMSG(FALSE, (L"XVC Set PowerState\r\n"));
            if (source == MDD_IOCTL)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("IOCTL_BUS_SET_POWER_STATE MDD_IOCTL\r\n")));
                break;
            }
            DEBUGMSG(ZONE_FUNCTION, (TEXT("IOCTL_BUS_SET_POWER_STATE\r\n")));
            pBusPowerState = (CE_BUS_POWER_STATE*)pInBuffer;
            pXVC->CurPMPowerState = *pBusPowerState->lpceDevicePowerState;
        break;

        default:
            break;
    }
    return TRUE;
}
