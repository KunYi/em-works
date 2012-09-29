//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
// File: 
//     XVC.c
// 
// Description:
//     USB Transceiver Driver is a stream interface driver which exposes the 
//     stream interface functions. This driver detects the USB plug in type and 
//     accordingly activates the Host or Function controller driver. Upon 
//     unplug, the respective driver gives back the control to the transceiver 
//     driver.
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <oal.h>
#pragma warning(pop)

#include "xvc.h"
#include <common_usb.h>
#include <common_usbname.h>
#include <common_usbcommon.h>

#include <..\USBD\OS\oscheckkitl.c>

#ifdef USBXVR_INTERRUPT
#undef USBXVR_INTERRUPT
#endif
#define USBXVR_INTERRUPT 1

#ifdef DISABLE_DETACH_WAKEUP
#undef DISABLE_DETACH_WAKEUP
#endif

#define DEBUG_LOG_USBCV 0
// Remove-W4: Warning C4053 workaround
// The following definition public\common\oak\inc\usbfn.h caused 
// warning C4053: one void operand for '?:', so we just use #pragma
// to get rid of those MSFT stuff.

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

PUSBXVC g_pXVC = NULL;
BOOL    gfsvaim = FALSE;
#ifdef RESET_PHY
extern void BSPUsbResetPHY(CSP_USB_REGS * regs);
#endif
extern void BSPUsbSetPHYEnable(CSP_USB_REGS *regs, BOOL blEnable);
extern BOOL PowerDownSchemeExist(void);
//extern void BSPUsbXvrEnableVBUSIntr(PUCHAR pUSBRegs, BOOL blEnable);
//extern void BSPUsbXvrEnableIDIntr(PUCHAR pUSBRegs, BOOL blEnable);
extern void BSPUSBInterruptControl(DWORD dwIOCTL, PVOID pSysIntr, DWORD dwLength);
extern void BSPUsbPhyEnterLowPowerMode(PUCHAR baseMem, BOOL blEnable);
static void PowerDown();
static void PowerUp();
extern void RegisterCallback(BSP_USB_CALLBACK_FNS *pfn);
extern DWORD GetUSBOTGIRQ(VOID);

//------------------------------------------------------------------------------
// Function:  InitClock
//
// This function is to initialize the USB clock
//
// Parameter: 
//     NULL
//
// Return:
//   TRUE for success, FALSE if failure
//
//------------------------------------------------------------------------------
static BOOL InitClock()
{
    BOOL rc = FALSE;

    rc  = BSPUSBClockSwitch(TRUE);

    return rc;

}

//------------------------------------------------------------------------------
// Function:  DeInitClock
//
// This function is to de-initialize the USB clock
//
// Parameter: 
//    Null
//
// Return:
//   TRUE for success, FALSE if failure
//
//------------------------------------------------------------------------------
static BOOL DeInitClock(void)
{
    BOOL rc = FALSE;

    rc = BSPUSBClockSwitch(FALSE);

    return rc;
}

//------------------------------------------------------------------------------
// Function: DllEntry
//
// This is the entry and exit point for the XVC module.  This
// function is called when processed and threads attach and detach from
// this module.
//
// Parameters:
//     hInstDll
//         [IN] The handle to this module.
//
//     dwReason
//         [IN] Specifies a flag indicating why the DLL entry-point function
//              is being called.
//
//     lpvReserved
//         [IN] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//     TRUE if the XVC is initialized; FALSE if an error occurred during
//     initialization.
//------------------------------------------------------------------------------
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


//------------------------------------------------------------------------------
// Function: USBControllerRun
//
// Description: This function sets USB controller to either run or halted, and
//              function waits until controller is stopped or started.
//
// Parameters:
//     pRegs
//         [IN] Points to USB related registers mapped to virtual address
//     bRunMode
//         [IN] If TRUE, then set running, else set stopped
//       
// Return:
//     ERROR_SUCCESS means mode is set successfully
//     ERROR_GEN_FAILURE means couldn't set the mode for some reason
//
//------------------------------------------------------------------------------
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
        // In host mode, make sure HCH (halted) already, and that mode does 
        // change to halted (or not)
        USB_USBSTS_T stat;
        DWORD *pTmpStat = (DWORD*)&stat;
        int iAttempts;

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

//------------------------------------------------------------------------------
// Function: USBControllerReset
//
// Description: This function resets the USB Controller and waits until the 
//              reset process is completed.
//
// Parameters:
//     pRegs
//         [IN] Points to USB related registers mapped to virtual address
//      
// Return:
//     ERROR_SUCCESS means that the controller is rest successfully
//     ERROR_GEN_FAILURE means couldn't reset the mode for some reason
//
//------------------------------------------------------------------------------
static DWORD USBControllerReset( CSP_USB_REGS *pRegs )
{
    USB_USBCMD_T cmd;
    USB_USBMODE_T mode;
    DWORD *pTmpCmd = (DWORD*)&cmd;
    DWORD *pTmpMode = (DWORD*)&mode;
    int iAttempts;

    *pTmpMode = INREG32(&pRegs->OTG.USBMODE);

    // If in HOST mode, HCHalted(HCH) bit in register USBSTS should be checked 
    // firstly before reset. It is illegal to reset the controller while the 
    // controller is under running(refer to EHCI spec, definition of USBCMD 
    // register).
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


//------------------------------------------------------------------------------
// Function : XVC_ISTMain
//
// Description: This thread activates the Function or Host depending on the
//              plug inserted.
//
// Input :
//     LPVOID lpParameter
//
// Output:
//     void
//------------------------------------------------------------------------------

#define IDLE_TIMEOUT    3000
DWORD WINAPI XVC_ISTMain(LPVOID lpParameter)
{    
    HANDLE hFunction, hXcvr, hHost;
    ULONG WaitReturn;    
    CSP_USB_REGS *pUSBRegs = g_pXVC->pUSBRegs;
    TCHAR szUSBFunctionObjectName[30];
    TCHAR szUSBXcvrObjectName[30];
    TCHAR szUSBHostObjectName[30];
    DWORD timeout = IDLE_TIMEOUT;
    DWORD StringSize = 0;
    
    UNREFERENCED_PARAMETER(lpParameter);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_ISTMain+\r\n")));

    // Event created for Function
    StringSize = sizeof(szUSBFunctionObjectName) / sizeof(TCHAR);
    StringCchCopy(szUSBFunctionObjectName,StringSize,USBFunctionObjectName);
    StringCchCat(szUSBFunctionObjectName, StringSize, g_pXVC->szOTGGroup);

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
    StringCchCat(szUSBXcvrObjectName, StringSize, g_pXVC->szOTGGroup);

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
    StringCchCat(szUSBHostObjectName, StringSize, g_pXVC->szOTGGroup);

    hHost = CreateEvent(NULL, FALSE, FALSE, szUSBHostObjectName);
    if(GetLastError()==ERROR_ALREADY_EXISTS)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Opened an existing Host Event\r\n")));
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Created a new Host Event\r\n")));
    if (hHost == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XCVR: Create Event Failed for host!\r\n")));
    }

    // Initializes of the Transceiver driver, configure as client all the time.
    USBControllerRun(pUSBRegs, FALSE);
    USBControllerReset(pUSBRegs);

    {
        DWORD* tmpOtgsc;
        USB_OTGSC_T otgsc;

        tmpOtgsc = (DWORD*)&otgsc;
        *tmpOtgsc = INREG32(&(pUSBRegs)->OTG.OTGSC);
        //RETAILMSG(1, (L"x ini otgsc %x\r\n", *tmpOtgsc));
        if (otgsc.ID)
        {
            //B device
            //RETAILMSG(1, (L"X I D\r\n"));
            InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, FALSE);
        }
        else
        {
            //A device
            //RETAILMSG(1, (L"X I A\r\n"));
            InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, TRUE);
            {
                // here we need to tell CSC & CCS, if they are 1, means a device is attached, 
                // it seems that in this case, no interrupt will be issued by HC or USB PHY.
                // we must set interrupt manual
                USB_PORTSC_T state;
                DWORD *temp;
                temp = (DWORD *)&state;
                *temp = INREG32(&(pUSBRegs)->OTG.PORTSC);
                //if( (state.CSC == 1) || (state.CCS == 1))
                {
                    RETAILMSG(FALSE,(TEXT("manual set interrupt after init host\r\n")));
                    SetInterruptEvent(g_pXVC->dwSysIntr);  
                }
            }
        }
    }

    // after init otg transceiver, usb core clock should be opened.
    // g_pXVC->bUSBCoreClk = TRUE;

    {
        DWORD* tmpOtgsc;
        USB_OTGSC_T otgsc;

        tmpOtgsc = (DWORD*)&otgsc;
        *tmpOtgsc = INREG32(&(pUSBRegs)->OTG.OTGSC);
        if(otgsc.ID)
        {
            // we must add a delay here, to make sure potential mass storage device
            // can be mounted.
            RETAILMSG(FALSE,(TEXT("Sleep 500\r\n")));
            Sleep(500);
        }
        else
        {
            Sleep(100);
        }
    }

    USBControllerRun(pUSBRegs, TRUE);

    //for(;;) {
    while(!g_pXVC->bExitThread){
        USB_USBSTS_T source;
        USB_PORTSC_T state;
        USB_CTRL_T ctrl;
        USB_OTGSC_T otgsc;
        DWORD *temp;
        DWORD *temp2;
        DWORD *temp3 = (DWORD*)&ctrl;
        DWORD *temp4;
        BOOL newIntrInit = FALSE;
        BOOL deviceDetected = FALSE;
        
        g_pXVC->bInXVC = TRUE;

        RETAILMSG(FALSE,(TEXT(" XVC WAITING ON INTERRUPT EVENT\n")));
        WaitReturn = WaitForSingleObject(g_pXVC->hIntrEvent, timeout);
        
        if (WaitReturn == WAIT_TIMEOUT)
        {
            // Now it is ready to put the transceiver into suspend mode.
            // But be aware there is a device attached when boot up
            USB_PORTSC_T portsc;
            int i = 0;
            EnterCriticalSection(&g_pXVC->csPhyLowMode);
            temp = (DWORD *)&portsc;
            RETAILMSG(FALSE, (L"XVC TIMED OUT!!\r\n"));
            if( !g_pXVC->bUSBCoreClk )
            {
                // here if bUSBCoreClk is false, it means a powerdown happen, then
                // we need not to do anything
                LeaveCriticalSection(&g_pXVC->csPhyLowMode);
                timeout = INFINITE;//IDLE_TIMEOUT;
                continue;
            }

            temp2 = (DWORD*)&otgsc;

            // Enable wakeup logic before suspending
            // Check and make sure all wakeup interrupt are enabled
            *temp3 = INREG32(&pUSBRegs->USB_CTRL);
            ctrl.OWIE = 1;
            OUTREG32(&pUSBRegs->USB_CTRL, *temp3);

            do {
                *temp = INREG32(&(pUSBRegs)->OTG.PORTSC);
            } while ((portsc.SUSP == 0) && (i++ < 100));

            // Lock Start
            
            *temp = INREG32(&(pUSBRegs)->OTG.PORTSC);

            RETAILMSG(FALSE,(TEXT(" XVC TIMED OUT- Obtained CS g_pXVC->csPhyLowMode!!\n")));
            
            // Add the following will cause a disconnect interrupt
            // which is not our expect
            //USBControllerRun(pUSBRegs, FALSE);

#ifdef EXTERN_ULPI_PHY
            *temp2=INREG32(&(pUSBRegs)->OTG.OTGSC);
            if ((otgsc.ID == 1) && (portsc.PHCD == 0))
            {
                SetULPIToClientMode(pUSBRegs);
            }
#endif
            if (PowerDownSchemeExist())
            {
                // PHCD
                if (g_pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode)
                     g_pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode(pUSBRegs, FALSE); // BSPUsbXvrSetPhyPowerMode

                if (g_pXVC->bUSBCoreClk == TRUE)
                {
                    // Enable Low Power interrupt
                    BSPUsbPhyEnterLowPowerMode((PUCHAR)pUSBRegs, TRUE);

                    // Close Clock
                    g_pXVC->bUSBCoreClk = FALSE;
                    DeInitClock();
                }
            }

            LeaveCriticalSection(&g_pXVC->csPhyLowMode);
            RETAILMSG(FALSE,(TEXT(" XVC TIMED OUT- Released CS g_pXVC->csPhyLowMode!!\n")));
            // Now we can stop the USB clock
            RETAILMSG(FALSE, (TEXT("XVC - SUSPEND\r\n")));
            DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC - SUSPEND\r\n")));
            timeout = INFINITE;//IDLE_TIMEOUT;
            
            continue;
        }

        RETAILMSG(FALSE,(TEXT(" XVC INTERRUPT - REQ CS!!!\n")));
        EnterCriticalSection(&g_pXVC->csPhyLowMode);

        if (PowerDownSchemeExist())
        {
            if (g_pXVC->bUSBCoreClk == FALSE)
            {
                // Open Clock
                InitClock();
                g_pXVC->bUSBCoreClk = TRUE;

                // clear and disable low power interrupt
                BSPUsbPhyEnterLowPowerMode((PUCHAR)pUSBRegs, FALSE);
            }

            // PHCD
            if (g_pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode)
               g_pXVC->fnUsbXvr.pfnUSBSetPhyPowerMode(pUSBRegs, TRUE);  // BSPUsbXvrSetPhyPowerMode
        }   

        USBControllerRun(pUSBRegs, TRUE);

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
        if (ctrl.OWIR == 1)
        {
            RETAILMSG(FALSE, (L"XVC WAS A WAKE UP INTERRUPT !!!\n"));
            *temp3 = 0;
            ctrl.OWIE = 1;
            CLRREG32(&pUSBRegs->USB_CTRL, *temp3);
        }

        if (g_pXVC->bResume) {
            RETAILMSG(FALSE, (TEXT("XVC:Resume again\r\n")));
            g_pXVC->bResume = FALSE;
            // We remove the controller stop, reset and run part, since when 
            // wakeup due to device attach the first interrupt would be just 
            // wakeup but not the device attached status. The transceiver needs
            // some times to resume from suspend before processing the device 
            // status change interrupt. If we reset immediately, the transceiver
            // would go into weird state.
             
            // USBControllerRun(pUSBRegs, FALSE);
            // InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, FALSE);
            // USBControllerRun(pUSBRegs, TRUE);
            g_pXVC->devState = 0;
            deviceDetected = FALSE;
            timeout = IDLE_TIMEOUT;
            // remove continue and let it to do the work, if something plug in
            // continue;
        }

        if ((source.PCI == 1) && (state.CCS == 1))
        {
            RETAILMSG(FALSE,(TEXT(" XVC DEVICE IS CONNECTED\n")));
            // We have a device attached
            if (g_pXVC->devState == 0)
            {
#ifdef USBXVR_INTERRUPT
                InterruptDisable (g_pXVC->dwSysIntr); // to disable the interrupt
#endif
                if (INREG32(&pUSBRegs->OTG.OTGSC) & 0x100) // B-device
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("B-Device client\r\n")));
                    USBControllerRun(pUSBRegs, TRUE);

                    // By default is client, no need to do any configuration
                    SetEvent(hFunction);
                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("A-Device host\r\n")));

                    USBControllerRun(pUSBRegs, FALSE);
                    USBControllerReset(pUSBRegs);

                    // Configure the host                   
                    InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, TRUE);

                    SetEvent(hHost);
                }
                deviceDetected = TRUE;
            } // g_pXVC->devState == 0
        }   // source.PCI==1 && state.CCS==1        
        else if ((g_pXVC->devState == 0) && (deviceDetected == FALSE)) 
        {  
            
            DWORD temp_otgsc = INREG32(&pUSBRegs->OTG.OTGSC);
            RETAILMSG(FALSE,(TEXT(" XVC -AFTER INT -  DEVICE NOT DETECTED!!!\n")));

            if ((temp_otgsc & 0x100) == 0x0)
            {
#ifdef USBXVR_INTERRUPT
                InterruptDisable (g_pXVC->dwSysIntr); // to disable the interrupt
#endif
                DEBUGMSG(ZONE_FUNCTION, (TEXT("A-device host from polling\r\n")));
                //Configure the host

                USBControllerRun(pUSBRegs, FALSE);
                //Sleep(10);
                USBControllerReset(pUSBRegs );
                //Sleep(100);

                InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, TRUE);
                RETAILMSG(FALSE,(TEXT(" XVC SIGNAL THE HOST - TO CHW!\n")));

                SetEvent(hHost);
                deviceDetected = TRUE;
            }
        }


        if ((g_pXVC->devState == 0) && (deviceDetected == TRUE))
        {
            RETAILMSG(FALSE,(TEXT(" XVC - RETURNED FROM CHW!\n")));
                g_pXVC->devState = 1;
                g_pXVC->bInXVC = FALSE;
                g_pXVC->bResume = FALSE;

#ifdef DISABLE_DETACH_WAKEUP
            // Clear any wakeup interrupt as there would be a problem on host mode
            RETAILMSG(1, (L"disable in X?\r\n"));
            *temp3 = 0;
            ctrl.OWIE = 1;
            CLRREG32(&pUSBRegs->USB_CTRL, *temp3);

            // Disable wakeup
            BSPUSBInterruptControl(IOCTL_HAL_DISABLE_WAKE, &g_pXVC->dwSysIntr, sizeof(g_pXVC->dwSysIntr));           
#endif
            LeaveCriticalSection(&g_pXVC->csPhyLowMode);
            // Wait for detach occur            
            WaitReturn = WaitForSingleObject(hXcvr, INFINITE);      
            EnterCriticalSection(&g_pXVC->csPhyLowMode);    
            g_pXVC->bInXVC = TRUE;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("XVR In control again\r\n")));

            // I think we need to do some charge, discharge ....
#ifdef USBXVR_INTERRUPT
            newIntrInit = TRUE;
            if (!(InterruptInitialize(g_pXVC->dwSysIntr, g_pXVC->hIntrEvent, NULL, 0))) {
                newIntrInit = FALSE;
                DEBUGMSG(ZONE_ERROR, (TEXT("XVC_Init: Interrupt initialization failed!, ErrCode: 0x%x\r\n"), GetLastError()));
            }

#endif
            BSPUSBInterruptControl(IOCTL_HAL_ENABLE_WAKE, &g_pXVC->dwSysIntr, sizeof(g_pXVC->dwSysIntr));           

            // Initializes of the Transceiver driver
            USBControllerRun(pUSBRegs, FALSE);
            USBControllerReset(pUSBRegs);

            {
                DWORD* tmpOtgsc;
                USB_OTGSC_T otgsc1;

                tmpOtgsc = (DWORD*)&otgsc1;
                *tmpOtgsc = INREG32(&(pUSBRegs)->OTG.OTGSC);
                //RETAILMSG(1, (L"x ini otgsc %x\r\n", *tmpOtgsc));
                if (otgsc1.ID)
                {
                    //B device
                    //RETAILMSG(1, (L"X I D\r\n"));
                    InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, FALSE);
                }
                else
                {
                    //A device
                    //RETAILMSG(1, (L"X I A\r\n"));
                    InitializeOTGTransceiver((PCSP_USB_REGS *)&pUSBRegs, TRUE);
                }
            }

            USBControllerRun(pUSBRegs, TRUE);

            g_pXVC->devState = 0;
            deviceDetected = FALSE;
            timeout = IDLE_TIMEOUT;

        }

        if (newIntrInit == FALSE) {
            // Clear source bit
            OUTREG32(&pUSBRegs->OTG.USBSTS, *temp);
#ifdef USBXVR_INTERRUPT
            InterruptDone(g_pXVC->dwSysIntr);     
#endif
        }
        LeaveCriticalSection(&g_pXVC->csPhyLowMode);

        timeout = IDLE_TIMEOUT;

    } // While loop
    //before exit thread, we should raise signal to Host and device to make it possible to exit thread too
    SetEvent(hFunction);
    SetEvent(hHost);
    ExitThread(0);
    // Never to enter this region

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_ISTMain-\r\n")));
    return 0;
}

//------------------------------------------------------------------------------
// Function: XVC_Init
//
// The Device Manager calls this function as a result of a call to the
// ActivateDevice() function.  This function will retrieve the handle to the
// device context from the registry and then initialize the XVC module with
// interrupt initialisation.
//
// Parameters:
//     dwContext
//         [in] Pointer to a string containing the registry path to the
//              active key for the stream interface driver.
//
// Returns:
//     Returns a handle to the device context created if successful. Returns
//     zero if not successful.
//------------------------------------------------------------------------------
DWORD XVC_Init(DWORD dwContext)
{
    PHYSICAL_ADDRESS pa;    
    DWORD len;
    LPCTSTR pszActiveKey;

    //Checkout whether usb kitl enable. If usb kitl enable, init will return failure. 
    if(FslUfnIsUSBKitlEnable())
    {
        RETAILMSG(1,(_T("USB  XVC load failure because Usb Kitl Enabled\r\n")));
        return (DWORD)NULL;
    }

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    g_pXVC = LocalAlloc(LPTR, sizeof(USBXVC));
    if (g_pXVC == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("XVC_Init: Cannot allocate memory\r\n")));
        goto clean;
    }

    memset(g_pXVC, 0x00, sizeof(USBXVC));

    // Read device parameters
    pszActiveKey = (LPCTSTR) dwContext;
    if (GetDeviceRegistryParams(pszActiveKey, g_pXVC, dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: XVC_Init: "L"Failed read registry parameters\r\n"));
        goto clean;
    }

    if (g_pXVC->IsOTGSupport == 0)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("OTG Not support and unload the driver\r\n")));
        goto clean;
    }

    // Initialize CriticalSection object for USB PHY Low-power mode
    InitializeCriticalSection(&g_pXVC->csPhyLowMode);

    // Initialize XVC variables
    g_pXVC->bUSBCoreClk = FALSE;
    g_pXVC->bResume = FALSE;
    g_pXVC->bExitThread = FALSE;

    RegisterCallback(&g_pXVC->fnUsbXvr);
    // Map the USB registers
    pa.QuadPart = ((g_pXVC->memBase == 0)? CSP_BASE_REG_PA_USB: g_pXVC->memBase);
    len = ((g_pXVC->memLen == 0)? 0x1000: g_pXVC->memLen);

    g_pXVC->pUSBRegs = MmMapIoSpace(pa, len, FALSE);
    if (g_pXVC->pUSBRegs == NULL) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: XVC_Init: Controller registers mapping failed\r\n"));
        goto clean;
    }
    
    {
        INT32 aIrqs[3];
        aIrqs[0] = -1;
        aIrqs[1] = OAL_INTR_TRANSLATE;
        aIrqs[2] = GetUSBOTGIRQ();
        KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(aIrqs), &g_pXVC->dwSysIntr, sizeof(g_pXVC->dwSysIntr), NULL);
    }

    BSPUSBClockCreateFileMapping();    
    g_pXVC->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_pXVC->hIntrEvent == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: CreateEvent failed\r\n")));
        goto clean;
    }

#ifdef USBXVR_INTERRUPT
    // to initialize the interrupt 
    if (!(InterruptInitialize(g_pXVC->dwSysIntr, g_pXVC->hIntrEvent, NULL, 0))) {
        DEBUGMSG(ZONE_ERROR, (TEXT("XVC_Init: Interrupt initialization failed!, ErrCode: 0x%x\r\n"), 
                                    GetLastError()));
        goto clean;
    }
#else
    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Transceiver driver working in polling mode\r\n")));
#endif
    BSPUSBInterruptControl(IOCTL_HAL_ENABLE_WAKE, &g_pXVC->dwSysIntr, sizeof(g_pXVC->dwSysIntr));

    // We init Clock here
    InitClock();
    g_pXVC->bUSBCoreClk = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Interrupt initialized\r\n")));
    g_pXVC->hInterruptServiceThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XVC_ISTMain, NULL, 0, NULL);

    if (g_pXVC->hInterruptServiceThread == NULL) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Init: Failed to create Interrupt Service Thread\r\n")));
        goto clean;
    }
    
    CeSetThreadPriority(g_pXVC->hInterruptServiceThread, 102);

    return (DWORD)g_pXVC;

clean:
    if (g_pXVC && g_pXVC->pUSBRegs)
        MmUnmapIoSpace((VOID*)g_pXVC->pUSBRegs, 0x1000);

    if (g_pXVC)
        LocalFree(g_pXVC);

    return (DWORD)NULL;
}

//------------------------------------------------------------------------------
// Function: XVC_Deinit
//
// Description: The Device Manager calls this function as a result of a call to 
// the DeactivateDevice() function.  This function will return any resources
// allocated while using this driver.
//
// Parameters:
//     dwContext
//         [IN] Specifies the value returned by XVC_Init for the given service 
//              instance.
//
// Returns:
//     Always return TRUE.
//------------------------------------------------------------------------------
BOOL XVC_Deinit(DWORD dwContext)
{
    UNREFERENCED_PARAMETER(dwContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Deinit: dwContext = 0x%x\r\n"), dwContext));
    
    if (g_pXVC && g_pXVC->pUSBRegs)
    {
        g_pXVC->bExitThread = TRUE;
        SetEvent(g_pXVC->hIntrEvent);
        WaitForSingleObject(g_pXVC->hInterruptServiceThread, INFINITE);
        CloseHandle(g_pXVC->hInterruptServiceThread);
        CloseHandle(g_pXVC->hIntrEvent);
        MmUnmapIoSpace((VOID*)g_pXVC->pUSBRegs, 0x1000);
    }
    

    //BSPUSBClockDeleteFileMapping();
    if (g_pXVC)
        LocalFree(g_pXVC);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Deinit\r\n"), dwContext));
    return TRUE;
}

//------------------------------------------------------------------------------
// Function: XVC_Open
//
// Description: This function is called when an application attempts to 
// establish a connection to this driver. This function will verify that a 
// trusted application has made the request and deny access to all non-trusted 
// applications.
//
// Parameters:
//     dwData
//         [IN] Handle to the device context. The XVC_Init function creates and 
//              returns this handle.
//
//     dwAccess
//         [IN] Access code for the device. The access is a combination of read
//              and write access from CreateFile.
//
//     dwShareMode
//         [IN] File share mode of the device. The share mode is a combination 
//              of read and write access sharing from CreateFile.
//
// Returns:
//     This function returns a handle that identifies the open context of the 
//     device to the calling application. 
//------------------------------------------------------------------------------
DWORD XVC_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwAccess);
    UNREFERENCED_PARAMETER(dwShareMode);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Open: dwData = 0x%x, dwAccess = 0x%x, dwShareMode = 0x%x\r\n"), dwData, dwAccess, dwShareMode));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Open\r\n")));
    return dwData;
}

//------------------------------------------------------------------------------
// Function: XVC_Close
//
// Description: This function is called when an application attempts to close 
// a connection to this driver.
//
// Parameters:
//     Handle
//         [IN] Handle returned by the XVC_Open function, which is used to 
//              identify the open context of the device.
//
// Returns:
//     TRUE
//------------------------------------------------------------------------------
BOOL XVC_Close(DWORD Handle)
{
    UNREFERENCED_PARAMETER(Handle);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_Close: Handle = 0x%x\r\n"), Handle));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-XVC_Close: Handle = 0x%x\r\n"), Handle));
    return TRUE;
}

//------------------------------------------------------------------------------
// Function: XVC_PowerDown
//
// Description: This function suspends power to the device. It is useful only 
// with devices that can power down under software control.
//
// Parameters:
//     void
//
// Returns:
//     void
//------------------------------------------------------------------------------
void XVC_PowerDown(void)
{
    return;
}

//------------------------------------------------------------------------------
// Function: XVC_PowerUp
//
// Description: This function restores power to a device.
//
// Parameters:
//     void
//   
// Returns:
//     void
//------------------------------------------------------------------------------
void XVC_PowerUp(void)
{
    return ;
}

//------------------------------------------------------------------------------
// Function: PowerDown
//
// Description: This function suspends power to the device. It is useful only 
// with devices that can power down under software control.
// Parameters:
//     void
//
// Returns:
//     void
//------------------------------------------------------------------------------
void PowerDown(void)
{
    CSP_USB_REGS *pUSBRegs = g_pXVC->pUSBRegs;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+PowerDown \r\n"))); 
    EnterCriticalSection(&g_pXVC->csPhyLowMode);
    if (g_pXVC->bInXVC) {
        
        RETAILMSG(FALSE, (TEXT("+XVC PowerDown current thread priority 0x%x\r\n"),CeGetThreadPriority(GetCurrentThread())));  
        
        // Lock Start
        if (g_pXVC->bUSBCoreClk == FALSE)
        {
            InitClock();
            g_pXVC->bUSBCoreClk = TRUE;
        }

#ifdef EXTERN_ULPI_PHY
        //clear USBINTR
        g_pXVC->dwUSBIntrValue = INREG32(&pUSBRegs->OTG.USBINTR);
        OUTREG32(&pUSBRegs->OTG.USBINTR, 0);
        ISP1504Poweroff(pUSBRegs);

        // Wrong Code
        // if (g_pXVC->bUSBCoreClk == TRUE)
        // {
        //     InitClock();
        //     g_pXVC->bUSBCoreClk = FALSE;
        // }
#endif
        
        USBControllerRun(pUSBRegs, FALSE);

        // in MX35/25, there exist an issue that after system suspended, PHY state machine
        // is not compliance with Link, so that when you plug cable into OTG port at this time,
        // the PHY will work as if system not suspend, to avoid this issue, we must disable PHY
        // in PowerDown.
        // But if we want to register USB interrupt as system wakeup source, we should not disable
        // it.
        BSPUsbSetPHYEnable(pUSBRegs, FALSE);
        
        if (g_pXVC->fnUsbXvr.pfnUSBPowerDown)
            g_pXVC->fnUsbXvr.pfnUSBPowerDown(pUSBRegs, &g_pXVC->bUSBCoreClk);
        
        DeInitClock();
        g_pXVC->bUSBCoreClk = FALSE;
        ResetEvent(g_pXVC->hIntrEvent);
        
    }
    LeaveCriticalSection(&g_pXVC->csPhyLowMode);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-PowerDown \r\n")));
    RETAILMSG(FALSE, (TEXT("-PowerDown \r\n")));  
    return;
}

//------------------------------------------------------------------------------
// Function: PowerUp
//
// Description: This function restores power to a device.
//
// Parameters:
//     void
//   
// Returns:
//     void
//------------------------------------------------------------------------------
void PowerUp(void)
{
    CSP_USB_REGS *pUSBRegs = g_pXVC->pUSBRegs;
    
    EnterCriticalSection(&g_pXVC->csPhyLowMode);
    if (g_pXVC->bInXVC) {
        RETAILMSG(FALSE, (TEXT("+XVC PowerUP \r\n")));
              
        if (g_pXVC->bUSBCoreClk == FALSE)
        {
            InitClock();
            g_pXVC->bUSBCoreClk = TRUE;
        }
#ifdef EXTERN_ULPI_PHY
        //restall USBINTR
        OUTREG32(&pUSBRegs->OTG.USBINTR, g_pXVC->dwUSBIntrValue);
        ISP1504PowerUp(pUSBRegs);
        // Wrong Code
        // if (g_pXVC->bUSBCoreClk == TRUE)
        // {
        //     InitClock();
        //     g_pXVC->bUSBCoreClk = FALSE;
        // }
#endif

#ifdef RESET_PHY
        // here we need to reset PHY
        BSPUsbResetPHY(pUSBRegs);
#endif
        if (g_pXVC->fnUsbXvr.pfnUSBPowerUp)
            g_pXVC->fnUsbXvr.pfnUSBPowerUp(pUSBRegs, &g_pXVC->bUSBCoreClk);
        
        g_pXVC->bResume = TRUE;
        SetInterruptEvent(g_pXVC->dwSysIntr);  
        
    }
    LeaveCriticalSection(&g_pXVC->csPhyLowMode);
    RETAILMSG(FALSE, (TEXT("-PowerUp \r\n")));   
}

//------------------------------------------------------------------------------
// Function: XVC_Read
//
// Description: This function reads data from the device identified by the open 
//              context.
//
// Parameters:
//     Handle
//         [IN] Handle to the open context of the device
//
//     pBuffer 
//         [OUT] Pointer to the buffer that contains the data read out
// 
//     dwNumBytes
//         [IN] Number of bytes read to the pBuffer

// Returns:
//     Always return 0.
//------------------------------------------------------------------------------
DWORD XVC_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Read: Handle = 0x%x, pBuffer = 0x%x, dwNumBytes = 0x%x\r\n"),Handle, pBuffer, dwNumBytes));
    return 0;
}

//------------------------------------------------------------------------------
// Function: XVC_Write
//
// Description: This function writes data to the device.
//
// Parameters:
//     Handle
//         [IN] Handle to the open context of the device
//
//     pBuffer
//         [OUT] Pointer to the buffer that contains the data to write
//
//     dwNumBytes 
//         [IN] Number of bytes to write from the pBuffer buffer into the device
//
// Returns:
//     Always return 0.
//------------------------------------------------------------------------------
DWORD XVC_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Write: Handle = 0x%x, pBuffer = 0x%x, dwNumBytes = 0x%x\r\n"), Handle, pBuffer, dwNumBytes));
    return 0;
}

//------------------------------------------------------------------------------
// Function: XVC_Seek
//
// Description: This function moves the data pointer in the device.
//
// Parameters:
//     Handle
//         [IN] Handle to the open context of the device
//
//     lDistance
//         [IN] Number of bytes to move the data pointer in the device
//         
//     dwMoveMethod
//         [IN] Specifies the move method
//
// Returns:
//     Always return -1.
//------------------------------------------------------------------------------
DWORD XVC_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(lDistance);
    UNREFERENCED_PARAMETER(dwMoveMethod);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_Seek: Handle = 0x%x, lDistance = 0x%x, dwMoveMethod = 0x%x\r\n"), Handle, lDistance, dwMoveMethod));
    return (DWORD)-1;
}

//------------------------------------------------------------------------------
// Function: XVC_IOControl
//
// Description: This function is called when an application calls the 
// DeviceIoControl() function. This function operates differently based upon 
// the IOCTL that is passed to it.
//
// Parameters:
//     pXVCContext
//         [IN] Specifies the value returned by XVC_Init (Services.exe) for the 
//              given service instance 
//
//     source
//         [IN] Specifies the source of the IOCTL request
//
//     code
//         [IN] Specifies the IOCTL code for the operation
//
//     pInBuffer 
//         [IN] Pointer to a buffer that contains the data required to perform 
//              the operation. This parameter can be NULL if the code parameter 
//              specifies an operation that does not require data
//
//     inSize
//         [IN] Specifies the size, in bytes, of the buffer pointed to by
//              pInBuffer
//
//     pOutBuffer 
//         [OUT] Pointer to a buffer that receives the output data from the 
//               operation. This parameter can be NULL if the code parameter 
//               specifies an operation that does not produce output data.
//
//     outSize 
//         [IN] Specifies the size, in bytes, of the buffer pointed to by 
//              pOutBuffer
//
//     pOutSize 
//         [OUT] Pointer to a variable that receives the size, in bytes, 
//               of the data stored into the buffer pointed to by pOutBuffer 
//
// Returns:
//     TRUE if the IOCTL is handled. FALSE if the IOCTL was not recognized or
//     an error occurred while processing the IOCTL requested.
//------------------------------------------------------------------------------
BOOL XVC_IOControl(
    DWORD  pXVCContext,
    //IOCTL_SOURCE source,
    DWORD code,
    PUCHAR pInBuffer,
    DWORD inSize,
    PUCHAR pOutBuffer,
    DWORD outSize,
    PDWORD pOutSize
)
{
    CE_BUS_POWER_STATE *pBusPowerState;    
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    UNREFERENCED_PARAMETER(pOutSize);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(inSize);
    UNREFERENCED_PARAMETER(pXVCContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+XVC_IOControl: dwIoControlCode = 0x%x\r\n"), code));

    switch (code)
    {
        case IOCTL_BUS_GET_POWER_STATE:         
#if 0
            if (source != MDD_IOCTL)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("IOCTL_BUS_GET_POWER_STATE not MDD_IOCTL\r\n")));
                break;
            }
#endif

            DEBUGMSG(ZONE_FUNCTION, (TEXT("IOCTL_BUS_GET_POWER_STATE\r\n")));
            pBusPowerState = (CE_BUS_POWER_STATE*)pInBuffer;
            *pBusPowerState->lpceDevicePowerState = g_pXVC->CurPMPowerState;
            break;

        case IOCTL_BUS_SET_POWER_STATE:
#if 0
            if (source == MDD_IOCTL)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("IOCTL_BUS_SET_POWER_STATE MDD_IOCTL\r\n")));
                break;
            }
#endif
            DEBUGMSG(ZONE_FUNCTION, (TEXT("IOCTL_BUS_SET_POWER_STATE\r\n")));
            pBusPowerState = (CE_BUS_POWER_STATE*)pInBuffer;
            g_pXVC->CurPMPowerState = *pBusPowerState->lpceDevicePowerState;
        break;

        case IOCTL_POWER_CAPABILITIES:
            // tell the power manager about ourselves.
            DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_IOCTL_POWER_CAPABILITIES\r\n")));
            if ( pOutBuffer != NULL && 
                 outSize >= sizeof(POWER_CAPABILITIES) && 
                 pOutSize != NULL)
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pOutBuffer;
                memset(ppc, 0, sizeof(*ppc));
                ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D4);
             
                *pOutSize = sizeof(POWER_CAPABILITIES);
                dwErr = ERROR_SUCCESS;
            }
            break;

        case IOCTL_POWER_QUERY: 
            DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_IOCTL_POWER_QUERY\r\n")));
            if ( pOutBuffer != NULL && 
                 outSize == sizeof(CEDEVICE_POWER_STATE) && 
                 pOutSize != NULL)
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuffer;
                DEBUGMSG(ZONE_FUNCTION, (TEXT("NewDx = %d\r\n"), NewDx));

                if (VALID_DX(NewDx))
                {
                    // this is a valid Dx state so return a good status
                    *pOutSize = sizeof(CEDEVICE_POWER_STATE);
                    dwErr = ERROR_SUCCESS;
                }
            }
            break;

        case IOCTL_POWER_SET: 
            DEBUGMSG(ZONE_FUNCTION, (TEXT("XVC_IOCTL_POWER_SET\r\n")));
            if ( pOutBuffer != NULL && 
                 outSize == sizeof(CEDEVICE_POWER_STATE) && 
                 pOutSize != NULL)
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuffer;
                RETAILMSG(FALSE, (TEXT("current = 0x%x NewDx = %d\r\n"),g_pXVC->CurPMPowerState, NewDx));
                if(g_pXVC->CurPMPowerState != NewDx)
                {
                    if (NewDx == D0)
                    {
                        PowerUp();
                    }
                    else
                    {
                        PowerDown();
                    }
                }
                g_pXVC->CurPMPowerState = NewDx;
                *(PCEDEVICE_POWER_STATE)pOutBuffer = g_pXVC->CurPMPowerState;
                
                *pOutSize = sizeof(CEDEVICE_POWER_STATE);
                dwErr = ERROR_SUCCESS;
            }
            break;

        case IOCTL_POWER_GET: 
            if ( pOutBuffer != NULL && 
                 outSize == sizeof(CEDEVICE_POWER_STATE) && 
                 pOutSize != NULL)
            {
                // just return our CurrentDx value
                *(PCEDEVICE_POWER_STATE)pOutBuffer = g_pXVC->CurPMPowerState;
                *pOutSize = sizeof(CEDEVICE_POWER_STATE);
                dwErr = ERROR_SUCCESS;
            }
            break;
        default:
            break;
    }
    return TRUE;
}
