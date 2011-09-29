//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//  File:  pmicpdk.cpp
//
//  This file contains the PMIC chip specific functions that provide control
//  over the Power Management IC.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <cmnintrin.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "cspiutil.h"
#include "pmic_ioctl.h"
#include "pmic_lla.h"
#include "pmic_adc.h"
#include "regs_adc.h"
#include "regs.h"
#include "regs_battery.h"
#include "regs_connectivity.h"
#include "pmic_connectivity.h"
#include "pmic_convity_priv.h"


//-----------------------------------------------------------------------------
// External Functions
extern "C" int BSPPmicGetSpiPort(void);
extern "C" UINT32 BSPPmicGetSpiFreqOut();
extern "C" DWORD BSPPmicGetIrq(void);
extern "C" BOOL BSPPmicInit(HANDLE *phIntrHdlr);
extern "C" BOOL BSPPmicDeinit(void);
extern "C" BOOL BSPPmicClearIrq(void);
extern "C" BOOL BSPPmicGetIrqStatus(UINT32 *status);
extern "C" VOID BSPPmicPowerNotifySuspend(void);
extern "C" VOID BSPPmicPowerNotifyResume(void);
extern "C" VOID BSPPmicReadIntrStatus(void);
extern "C" VOID BSPPmicSignalOALRTC(void);

//-----------------------------------------------------------------------------
// External Variables
extern "C" BOOL g_bPmicUseCspiPolling;

//-----------------------------------------------------------------------------
// Defines

#define PMIC_INTS0_TO_AP                0xFFFFFF   // all interrupts to AP.
#define PMIC_INTS1_TO_AP                0xFFFFFF
#define PMIC_ALL_BITS                   0xFFFFFF

#define MC13783_ADC_DEFAULT_ATO_SETTING   0x01
#define MC13783_ADC_REGISTER_MASK_ALL     0xFFFFFF
#define ADC_OPERATION_TIMEOUT           5000

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

DBGPARAM dpCurSettings = {
    _T("PMICPDK"),
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
        TEXT("Info"), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN // | ZONEMASK_INIT | ZONEMASK_FUNC | ZONEMASK_INFO // ulZoneMask
};

#endif  // DEBUG

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static CRITICAL_SECTION pmicCs;
static CRITICAL_SECTION adcCs;
static CRITICAL_SECTION intRegCs;
static HANDLE hIntrEventPmic;
static HANDLE hPmicISTThread;
static DWORD dwSysIntrPmic;
static BOOL bISTTerminate;
static HANDLE phIntrHdlr[PMIC_INT_MAX_ID];
static volatile BOOL g_bAdcPolled = FALSE;
static CEDEVICE_POWER_STATE g_dxCurrent = D0;

//-----------------------------------------------------------------------------
// Local Functions
static BOOL InitializePMIC();
static VOID CleanupPMIC();
static PMIC_STATUS PmicADCGetSettings(UINT16 channel, UINT32 *group,
                                      UINT32 *chann, UINT32 *ena);
static PMIC_STATUS PmicADCConvert();
static VOID PmicADCDisable();
static VOID PmicReadEightValues(UINT16* pReturnVal);
static BOOL CheckIntIdValid(UINT32 IntID);
static BOOL PMICAdcSetMode(PMIC_TOUCH_MODE mode);
static DWORD CALLBACK PmicIsrThreadProc(LPVOID lpParameter);
static DWORD WINAPI PmicPowerNotificationThread(LPVOID lpParam);

extern "C" BOOL PMICIoctlIntEnable(UINT32 IntID, BOOL EnableFlag);
extern "C" VOID SetRegister(UINT32 addr, UINT32 data, UINT32 mask);
extern "C" VOID GetRegister(UINT32 addr, UINT32* content);

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the PMIC control module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the PMIC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
extern "C"
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HMODULE)hInstDll);
            DisableThreadLibraryCalls((HMODULE) hInstDll);

            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS ATTACH TO PMIC PDK *****\r\n")));

            break;

        case DLL_PROCESS_DETACH:

            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM PMIC PDK *****\r\n")));
            break;
    }

    // Return TRUE for success
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: InitializePMIC
//
// Initializes the PMIC module.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if the PMIC is initialized or FALSE if an error occurred during
//      initialization.  The reason for the failure can be retrieved by the
//      GetLastError().
//
//-----------------------------------------------------------------------------
static BOOL
InitializePMIC(void)
{
    // These definitions must match what is documented in the MC13783 L3 DTS
    // document and are used to check if the MC13783 is actually present or
    // not at the beginning of the driver initialization process.
    static const UINT32 MC13783_ICID_MASK  = 0x1C0; // ICID[2:0] corresponds to
                                                    // bits [8:6] of the ID
                                                    // register.
    static const UINT32 MC13783_ICID_VALUE = 0x80;  // This corresponds to
                                                    // ICID[2:0] = 010.

    BOOL initializedPMIC = FALSE;
    DWORD irq;
    int i;
    UINT32 isr;
    UINT32 temp_data;

    DEBUGMSG(ZONE_INIT || ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // Create critical sections.
    InitializeCriticalSection(&pmicCs);
    InitializeCriticalSection(&adcCs);
    InitializeCriticalSection(&intRegCs);

    // Initialize CSPI interface to communicate with the PMIC.
    initializedPMIC = cspiInitialize(BSPPmicGetSpiPort(),
                                     BSPPmicGetSpiFreqOut());

    if (!initializedPMIC)
    {
        ERRORMSG(TRUE, (TEXT("%s(): cspiInitialize() failed.\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    // Check to see if the MC13783 PMIC is actually present. We do that by
    // trying to read the Revision ID register. According to the MC13783
    // Level 3 Detailed Technical Specifications document, the ICID[2:0]
    // bits are hardwired to be 010 for the MC13783.
    //
    // If we fail to detect the presence of an MC13783 PMIC here, we should
    // immediately abort any further initialization steps.
    GetRegister(MC13783_REV_ADDR, &temp_data);
    if ((temp_data & MC13783_ICID_MASK) != MC13783_ICID_VALUE)
    {
        DEBUGMSG(TRUE, (TEXT("%s(): failed to detect MC17383 PMIC.\r\n"),
                        __WFUNCTION__));
        goto Error;
    }
    DEBUGMSG(TRUE, (TEXT("%s(): successfully detected MC17383 PMIC.\r\n"),
                    __WFUNCTION__));

    // Reset Interrupt registration table
    for (i = PMIC_INT_MAX_ID - 1; i>=0; i--)
    {
        phIntrHdlr[i] = NULL;
    }

    // Initialize the PMIC interrupt. Start by masking all interrupts.
    SetRegister(MC13783_INT_MSK0_ADDR, PMIC_ALL_BITS, PMIC_ALL_BITS);
    SetRegister(MC13783_INT_MSK1_ADDR, PMIC_ALL_BITS, PMIC_ALL_BITS);

    // Then clear all existing interrupts.
    GetRegister(MC13783_INT_STAT0_ADDR, &isr);
    SetRegister(MC13783_INT_STAT0_ADDR, (isr & PMIC_INTS0_TO_AP),
                PMIC_ALL_BITS);
    GetRegister(MC13783_INT_STAT1_ADDR, &isr);
    SetRegister(MC13783_INT_STAT1_ADDR, (isr & PMIC_INTS1_TO_AP),
                PMIC_ALL_BITS);

    // Initialize platform-specific configuration
    BSPPmicInit(phIntrHdlr);

    // Create an event for signalling PMIC interrupts.
    //
    //      pEventAttributes = NULL (must be NULL)
    //      bManualReset = FALSE => resets automatically to nonsignaled
    //                              state after waiting thread released
    //      bInitialState = FALSE => initial state is non-signaled
    //      lpName = NULL => object created without a name
    //
    hIntrEventPmic = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (hIntrEventPmic == NULL)
    {
        ERRORMSG(TRUE, (TEXT("%s(): CreateEvent() failed.\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    irq = BSPPmicGetIrq();

    // Call the OAL to translate the IRQ into a SysIntr value.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
                         &dwSysIntrPmic, sizeof(DWORD), NULL))
    {
        ERRORMSG(TRUE, (TEXT("%s(): IOCTL_HAL_REQUEST_SYSINTR failed.\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    // Register PMIC interrupt
    if (!InterruptInitialize(dwSysIntrPmic, hIntrEventPmic, NULL, 0))
    {
        ERRORMSG(TRUE, (TEXT("%s(): InterruptInitialize() failed!\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    // Ask the OAL to enable our interrupt to wake the system from suspend
    KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &dwSysIntrPmic,
                    sizeof(dwSysIntrPmic), NULL, 0, NULL);

    bISTTerminate = FALSE;

    hPmicISTThread = CreateThread(NULL, 0,
                                  (LPTHREAD_START_ROUTINE)PmicIsrThreadProc,
                                  0, 0, NULL);
    if (!hPmicISTThread)
    {
        ERRORMSG(TRUE, (TEXT("%s(): CreateThread() failed\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    // Create event for blocking on ADC conversions to complete
    phIntrHdlr[PMIC_MC13783_INT_ADCDONEI] = 
                CreateEvent(NULL, FALSE, FALSE, _T("EVENT_ADCDONE"));

    // Check if CreateEvent failed
    if (phIntrHdlr[PMIC_MC13783_INT_ADCDONEI] == NULL)
    {
        ERRORMSG(TRUE, (TEXT("%s(): CreateEvent() failed!\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    // Unmask ADC complete interrupt
    SetRegister(MC13783_INT_MSK0_ADDR, PMIC_ALL_BITS, PMIC_ALL_BITS);
    SetRegister(MC13783_INT_MSK0_ADDR, 0, (1 << PMIC_MC13783_INT_ADCDONEI));

    // Unmask TOD interrupts
    SetRegister(MC13783_INT_MSK1_ADDR, 0, MC13783_TODAM_MASK);

    DEBUGMSG(ZONE_INIT || ZONE_FUNC, (TEXT("-%s returning %d\r\n"),
        __WFUNCTION__, initializedPMIC));

    return initializedPMIC;

Error:
    CleanupPMIC();

    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: CleanupPMIC
//
// This function unmaps the registry space for the PMIC registers.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID
CleanupPMIC(void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s\r\n"), __WFUNCTION__));

    cspiRelease(BSPPmicGetSpiPort());

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntrPmic, sizeof(DWORD),
                    NULL, 0, NULL);
    dwSysIntrPmic = (DWORD)SYSINTR_UNDEFINED;

    // If PMIC no longer needs service
    // Clear the gpio interrupt flag(w1c)
    // Deinitialize platform-specific configuration
    BSPPmicDeinit();

    // kill the IST thread
    if (hPmicISTThread)
    {
        bISTTerminate = TRUE;
        CloseHandle(hPmicISTThread);
        hPmicISTThread = NULL;
    }

    // Close interrupt event handle
    if (hIntrEventPmic)
    {
        CloseHandle(hIntrEventPmic);
        hIntrEventPmic = NULL;
    }

    // Delete critical sections
    DeleteCriticalSection(&pmicCs);
    DeleteCriticalSection(&adcCs);
    DeleteCriticalSection(&intRegCs);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s\r\n"), __WFUNCTION__));
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityOpHandler
//
// This function does the general operations (Reset and Set Mode) for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//               inParam->PARAMS.ifMode contains the desired mode in case of
//               OP_SET_MODE.
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityOpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                 PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    UINT32 addr1 = MC13783_CHG_USB1_ADDR, data1 = 0, mask1 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    if (inParam->op == OP_RESET)
    {
        mask0 = 0xFFFFFFFF;
        mask1 = 0xFFFFFFFF;
        data0 = MC13783_USB0_ADDR_RESET;
        data1 = MC13783_CHG_USB1_ADDR_RESET;

        SetRegister(addr0, data0, mask0);
        SetRegister(addr1, data1, mask1);
        *result = PMIC_SUCCESS;
        rc = TRUE;
    }
    else if (inParam->op == OP_SET_MODE)
    {
        // in case of USB mode, first enable SPI control of USB.
        if (inParam->PARAMS.ifMode == USB)
        {
            mask0 = CSP_BITFMASK(MC13783_USB0_USBCNTRL);
            data0 = CSP_BITFVAL(MC13783_USB0_USBCNTRL,
                                MC13783_USB0_USBCNTRL_SPI);
            SetRegister(addr0, data0, mask0);
        }

        mask0 = CSP_BITFMASK(MC13783_USB0_CONMODE);
        data0 = CSP_BITFVAL(MC13783_USB0_CONMODE, inParam->PARAMS.ifMode);

        if (inParam->PARAMS.ifMode == USB)
        {
            // start USB in powered off, pull-up/downs disabled mode.
            mask1 = CSP_BITFMASK(MC13783_CHG_USB1_VUSBEN) |
                                    CSP_BITFMASK(MC13783_CHG_USB1_VBUSEN);
            data1 = 0;
            SetRegister(addr1, data1, mask1);

            mask0 |= CSP_BITFMASK(MC13783_USB0_USBXCVREN)            |
                                    CSP_BITFMASK(MC13783_USB0_USBPU) |
                                    CSP_BITFMASK(MC13783_USB0_UDPPD) |
                                    CSP_BITFMASK(MC13783_USB0_UDMPD);
            // no change to data0: all these are disabled by setting to 0.
        }
        else if (inParam->PARAMS.ifMode == RS232)
        {
            // This mode will set the connections as
            // RS232_TX_RX_EXTERNAL_DEFAULT and
            // RS232_TX_RX_INTERNAL_DEFAULT, and
            // output not tristated.
            mask0 |= CSP_BITFMASK(MC13783_CHG_USB1_RSPOL);
            data0 |= CSP_BITFVAL(MC13783_CHG_USB1_RSPOL,
                                 MC13783_CHG_USB1_RSTRI_NONE);
        }

        // no specific settings done for CEA-936 modes besides CONMODE itself.
        SetRegister(addr0, data0, mask0);
        *result = PMIC_SUCCESS;

        rc = TRUE;
    }

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityRs232OpHandler
//
// This function does the config setting in RS232 mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityRs232OpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                      PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    UINT32 addr1 = MC13783_CHG_USB1_ADDR, data1 = 0, mask1 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    if (inParam->op == OP_RS232_SET_CFG)
    {
        mask0 = CSP_BITFMASK(MC13783_USB0_CONMODE);
        mask1 = CSP_BITFMASK(MC13783_CHG_USB1_RSTRI) |
                CSP_BITFMASK(MC13783_CHG_USB1_RSPOL);

        data1 = CSP_BITFVAL(MC13783_CHG_USB1_RSTRI,
                            (inParam->PARAMS.RS232_CFG.txTristated == TRUE) ?
                            1: 0);

        if (inParam->PARAMS.RS232_CFG.cfgInternal == RS232_TX_UDATVP_RX_URXVM)
        {
            data0 = CSP_BITFVAL(MC13783_USB0_CONMODE,
                                MC13783_USB0_CONMODE_RS232_2);
        }
        else // DEFAULT or RS232_TX_USE0VM_RX_UDATVP 
        {
            data0 = CSP_BITFVAL(MC13783_USB0_CONMODE,
                                MC13783_USB0_CONMODE_RS232_1);
        }

        if (inParam->PARAMS.RS232_CFG.cfgExternal == RS232_TX_UDP_RX_UDM)
        {
            data1 |= CSP_BITFVAL(MC13783_CHG_USB1_RSPOL,
                                 MC13783_CHG_USB1_RSPOL_TXUDP_RXUDM);
        }
        // else DEFAULT or RS232_TX_UDM_RX_UDP, set to
        // MC13783_CHG_USB1_RSPOL_TXUDM_RXUDP (not set explicitly,
        // as MC13783_CHG_USB1_RSPOL_TXUDM_RXUDP is 0)

        SetRegister(addr0, data0, mask0);
        SetRegister(addr1, data1, mask1);
        *result = PMIC_SUCCESS;

        rc = TRUE;
    }

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityCea936OpHandler
//
// This function does the config setting in CEA-936 mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityCea936OpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                            PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    UINT32 addr1 = MC13783_CHG_USB1_ADDR, data1 = 0, mask1 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    if (inParam->op == OP_CEA936_SET_DETECT)
    {
        mask0 = CSP_BITFMASK(MC13783_USB0_DP150KPU) |
                             CSP_BITFMASK(MC13783_USB0_IDPUCNTRL);
        mask1 = CSP_BITFMASK(MC13783_CHG_USB1_ID100KPU);

        if (inParam->PARAMS.cea936DetectCfg & ACCESSORY_ID_ID100KPU)
        {
            data1 |= CSP_BITFVAL(MC13783_CHG_USB1_ID100KPU, 1);
        }

        if (inParam->PARAMS.cea936DetectCfg & ACCESSORY_ID_IDPUCNTRL)
        {
            data0 |= CSP_BITFVAL(MC13783_USB0_IDPUCNTRL, 1);
        }

        if (inParam->PARAMS.cea936DetectCfg & ACCESSORY_ID_DP150KPU)
        {
            data0 |= CSP_BITFVAL(MC13783_USB0_DP150KPU, 1);
        }

        SetRegister(addr0, data0, mask0);
        SetRegister(addr1, data1, mask1);
        *result = PMIC_SUCCESS;
        rc = TRUE;
    }
    else if (inParam->op == OP_CEA936_EXIT)
    {
        *result = PMIC_SUCCESS;
        rc = TRUE;

        switch (inParam->PARAMS.cea936ExitSignal)
        {
        case CEA936_UID_NO_PULLDOWN:
            mask0 = CSP_BITFMASK(MC13783_USB0_IDPD) |
                    CSP_BITFMASK(MC13783_USB0_IDPULSE);
            break;
        case CEA936_UID_PULLDOWN_6MS:
            mask0 = CSP_BITFMASK(MC13783_USB0_IDPD) |
                    CSP_BITFMASK(MC13783_USB0_IDPULSE);
            data0 = CSP_BITFVAL(MC13783_USB0_IDPULSE, 1);
            break;
        case CEA936_UID_PULLDOWN:
            mask0 = CSP_BITFMASK(MC13783_USB0_IDPD);
            data0 = CSP_BITFVAL(MC13783_USB0_IDPD, 1);
            break;
        case CEA936_UDMPULSE:
            mask0 = CSP_BITFMASK(MC13783_USB0_DMPULSE);
            data0 = CSP_BITFVAL(MC13783_USB0_DMPULSE, 1);
            break;
        default:
            *result = PMIC_ERROR;
            rc = FALSE;
            break;
        }

        SetRegister(addr0, data0, mask0);
    }

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbSetSpeedHandler
//
// This function does the speed setting in USB mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbSetSpeedHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                               PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    UINT32 addr1 = MC13783_CHG_USB1_ADDR, data1 = 0, mask1 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    if (inParam->PARAMS.USB_SPEED.usbSpeed == USB_HIGH_SPEED)
    {
        *result = PMIC_NOT_SUPPORTED;
        return rc;
    }

    mask0 = CSP_BITFMASK(MC13783_USB0_FSENB) |
            CSP_BITFMASK(MC13783_USB0_USBPU) |
            CSP_BITFMASK(MC13783_USB0_UDPPD) |
            CSP_BITFMASK(MC13783_USB0_UDMPD) |
            CSP_BITFMASK(MC13783_USB0_USBXCVREN);

    // turn on the transceiver
    data0 = CSP_BITFVAL(MC13783_USB0_USBXCVREN, MC13783_USB0_USBXCVREN_ENABLE);

    if (inParam->PARAMS.USB_SPEED.usbSpeed == USB_LOW_SPEED)
    {
        data0 |= CSP_BITFVAL(MC13783_USB0_FSENB, MC13783_USB0_FSENB_LOW_SPEED);
    }

    if (inParam->PARAMS.USB_SPEED.usbMode == USB_HOST)
    {
        // turn on the VBUS, and 15K pull-down resistors. 
        mask0 |= CSP_BITFMASK(MC13783_CHG_USB1_VBUSEN);
        data0 |= CSP_BITFVAL(MC13783_USB0_UDPPD, MC13783_USB_PDPU_SWITCHED_IN) |
                 CSP_BITFVAL(MC13783_USB0_UDMPD, MC13783_USB_PDPU_SWITCHED_IN) |
                 CSP_BITFVAL(MC13783_CHG_USB1_VBUSEN,
                             MC13783_CHG_USB1_OP_ENABLE);
    }
    else
    {
        // Turn on the 1.5k pull-up. Turn on 15k pull-down on the other
        // data line.
        data0 |= CSP_BITFVAL(MC13783_USB0_USBPU, MC13783_USB_PDPU_SWITCHED_IN);
        if (inParam->PARAMS.USB_SPEED.usbSpeed == USB_FULL_SPEED)
        {
            data0 |= CSP_BITFVAL(MC13783_USB0_UDMPD,
                                 MC13783_USB_PDPU_SWITCHED_IN);
        }
        else
        {
            data0 |= CSP_BITFVAL(MC13783_USB0_UDPPD,
                                 MC13783_USB_PDPU_SWITCHED_IN);
        }
    }

    SetRegister(addr0, data0, mask0);
    SetRegister(addr1, data1, mask1);

    *result = PMIC_SUCCESS;
    rc = TRUE;

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbSetPowerSourceHandler
//
// This function does the power specific setting in USB mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbSetPowerSourceHandler(
    PMIC_PARAM_CONVITY_OP *inParam, PMIC_STATUS *result)
{
    UINT32 addr1 = MC13783_CHG_USB1_ADDR, data1 = 0, mask1 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    // Turn on the VUSB regulator. set the input source and output voltage
    // as requested.
    mask1 = CSP_BITFMASK(MC13783_CHG_USB1_VUSBIN) |
            CSP_BITFMASK(MC13783_CHG_USB1_VUSB)   |
            CSP_BITFMASK(MC13783_CHG_USB1_VUSBEN);
    data1 = CSP_BITFVAL(MC13783_CHG_USB1_VUSBEN, MC13783_CHG_USB1_OP_ENABLE)   |
            CSP_BITFVAL(MC13783_CHG_USB1_VUSB, inParam->PARAMS.USB_PWR.pwrout) |
            CSP_BITFVAL(MC13783_CHG_USB1_VUSBIN, inParam->PARAMS.USB_PWR.pwrin);

    SetRegister(addr1, data1, mask1);

    *result = PMIC_SUCCESS;
    rc = TRUE;

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbSetXcvrHandler
//
// This function does the Transceiver specific setting in USB mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbSetXcvrHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                         PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    UINT32 addr1 = MC13783_CHG_USB1_ADDR, data1 = 0, mask1 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    switch (inParam->PARAMS.usbXcvrMode)
    {
    case USB_TRANSCEIVER_OFF:
        mask0 |= CSP_BITFMASK(MC13783_USB0_USBXCVREN);
        data0 |= CSP_BITFVAL(MC13783_USB0_USBXCVREN,
                             MC13783_USB0_USBXCVREN_DISABLE);
        // switching off VBUS charging, in case it was happening
        mask1 = CSP_BITFMASK(MC13783_CHG_USB1_VBUSEN);
        data1 = 0;
        SetRegister(addr1, data1, mask1);
        break;
    case USB_SUSPEND_ON:
        mask0 |= CSP_BITFMASK(MC13783_USB0_USBSUSPEND);
        data0 |= CSP_BITFVAL(MC13783_USB0_USBSUSPEND,
                             MC13783_USB0_USBSUSPEND_ON);
        break;
    case USB_SUSPEND_OFF:
        mask0 |= CSP_BITFMASK(MC13783_USB0_USBSUSPEND);
        data0 |= CSP_BITFVAL(MC13783_USB0_USBSUSPEND,
                             MC13783_USB0_USBSUSPEND_OFF);
        break;
    case USB_OTG_SRP_DLP_START:
        // note: this is "manual" DLP
        mask0 |= CSP_BITFMASK(MC13783_USB0_USBPU);
        data0 |= CSP_BITFVAL(MC13783_USB0_USBPU, MC13783_USB_PDPU_SWITCHED_IN);
        break;
    case USB_OTG_SRP_DLP_STOP:
        mask0 |= CSP_BITFMASK(MC13783_USB0_USBPU);
        data0 |= CSP_BITFVAL(MC13783_USB0_USBPU, MC13783_USB_PDPU_SWITCHED_OUT);
        break;
    case USB_SINGLE_ENDED_UNIDIR_TX:
    case USB_SINGLE_ENDED_UNIDIR_RX:
        mask0 |= CSP_BITFMASK(MC13783_USB0_DATSE0) |
                 CSP_BITFMASK(MC13783_USB0_BIDIR);
        data0 |= CSP_BITFVAL(MC13783_USB0_DATSE0, MC13783_USB0_DATSE0_SE) |
                 CSP_BITFVAL(MC13783_USB0_BIDIR, MC13783_USB0_BIDIR_UNIDIR);
        break;
    case USB_SINGLE_ENDED_BIDIR_TX:
    case USB_SINGLE_ENDED_BIDIR_RX:
        mask0 |= CSP_BITFMASK(MC13783_USB0_DATSE0) |
                 CSP_BITFMASK(MC13783_USB0_BIDIR);
        data0 |= CSP_BITFVAL(MC13783_USB0_DATSE0, MC13783_USB0_DATSE0_SE) |
                 CSP_BITFVAL(MC13783_USB0_BIDIR, MC13783_USB0_BIDIR_BIDIR);
        break;
    case USB_DIFFERENTIAL_UNIDIR_TX:
    case USB_DIFFERENTIAL_UNIDIR_RX:
        mask0 |= CSP_BITFMASK(MC13783_USB0_DATSE0) |
                 CSP_BITFMASK(MC13783_USB0_BIDIR);
        data0 |= CSP_BITFVAL(MC13783_USB0_DATSE0, MC13783_USB0_DATSE0_DIFF) |
                 CSP_BITFVAL(MC13783_USB0_BIDIR, MC13783_USB0_BIDIR_UNIDIR);
            break;
    case USB_DIFFERENTIAL_BIDIR_TX:
    case USB_DIFFERENTIAL_BIDIR_RX:
        mask0 |= CSP_BITFMASK(MC13783_USB0_DATSE0) |
                 CSP_BITFMASK(MC13783_USB0_BIDIR);
        data0 |= CSP_BITFVAL(MC13783_USB0_DATSE0, MC13783_USB0_DATSE0_DIFF) |
                 CSP_BITFVAL(MC13783_USB0_BIDIR, MC13783_USB0_BIDIR_BIDIR);
        break;
    case USB_SINGLE_ENDED_LOW:
        // With MC13783, the se0 can be controlled by the processor, not
        // by the PMIC. Hence NO-OP.
        break;
    default:
        *result = PMIC_ERROR;
        rc = FALSE;
        return rc;
    }

    SetRegister(addr0, data0, mask0);

    *result = PMIC_SUCCESS;
    rc = TRUE;

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbOtgBeginHnpHandler
//
// This function does the settings for beginning the HNP protocol in USB mode
// for connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbOtgBeginHnpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                             PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    mask0 = CSP_BITFMASK(MC13783_USB0_USBPU) |
            CSP_BITFMASK(MC13783_USB0_UDPPD) |
            CSP_BITFMASK(MC13783_USB0_UDMPD);

    if (inParam->PARAMS.usbOtgType == USB_A_DEVICE)
    {
        // enable pull up on D+ (ensure it is D+, not D-)
        mask0 |= CSP_BITFMASK(MC13783_USB0_FSENB); 
        data0 = CSP_BITFVAL(MC13783_USB0_USBPU, MC13783_USB_PDPU_SWITCHED_IN);
    }
    else if (inParam->PARAMS.usbOtgType == USB_B_DEVICE)
    {
        data0 = 0;
    }

    SetRegister(addr0, data0, mask0);

    *result = PMIC_SUCCESS;
    rc = TRUE;

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbOtgEndHnpHandler
//
// This function does the settings for ending the HNP protocol in USB mode
// for connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbOtgEndHnpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                           PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    mask0 = CSP_BITFMASK(MC13783_USB0_USBPU);

    if (inParam->PARAMS.usbOtgType == USB_A_DEVICE)
            data0 = 0;
    else if (inParam->PARAMS.usbOtgType == USB_B_DEVICE)
    {
        // enable pull up on D+ (ensure it is D+, not D-)
        mask0 |= CSP_BITFMASK(MC13783_USB0_FSENB);
        data0 = CSP_BITFVAL(MC13783_USB0_USBPU, MC13783_USB_PDPU_SWITCHED_IN);
    }

    SetRegister(addr0, data0, mask0);

    *result = PMIC_SUCCESS;
    rc = TRUE;

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbOtgSetConfigHandler
//
// This function does the settings for USB OTG in USB mode
// for connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbOtgSetConfigHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                              PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    if (inParam->PARAMS.usbOtgCfg & USB_OTG_SE0CONN)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_SE0CONN);
        mask0 |= CSP_BITFVAL(MC13783_USB0_SE0CONN,
                             MC13783_USB0_SE0CONN_AUTOCON_ENABLE);
    }

    if (inParam->PARAMS.usbOtgCfg & USB_OTG_DLP_SRP)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_DLPSRP);
        mask0 |= CSP_BITFVAL(MC13783_USB0_DLPSRP, MC13783_USB0_DLPSRP_TRIGGER);
    }

    if (inParam->PARAMS.usbOtgCfg & USB_PULL_OVERRIDE)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_PULLOVR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_PULLOVR, MC13783_USB0_PULLOVR_ENABLE);
    }

    if (inParam->PARAMS.usbOtgCfg & USB_VBUS_CURRENT_LIMIT_HIGH)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUSPULSETMR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUSPULSETMR,
                             MC13783_USB0_VBUSPULSETMR_200MA);
    }
    else if (inParam->PARAMS.usbOtgCfg & USB_VBUS_CURRENT_LIMIT_LOW)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUSPULSETMR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUSPULSETMR,
                             MC13783_USB0_VBUSPULSETMR_910UA);
    }
    else if (inParam->PARAMS.usbOtgCfg & USB_VBUS_CURRENT_LIMIT_LOW_10MS)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUSPULSETMR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUSPULSETMR,
                             MC13783_USB0_VBUSPULSETMR_910UA_10MS);
    }
    else if (inParam->PARAMS.usbOtgCfg & USB_VBUS_CURRENT_LIMIT_LOW_20MS)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUSPULSETMR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUSPULSETMR,
                             MC13783_USB0_VBUSPULSETMR_910UA_20MS);
    }
    else if (inParam->PARAMS.usbOtgCfg & USB_VBUS_CURRENT_LIMIT_LOW_30MS)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUSPULSETMR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUSPULSETMR,
                             MC13783_USB0_VBUSPULSETMR_910UA_30MS);
    }
    else if (inParam->PARAMS.usbOtgCfg & USB_VBUS_CURRENT_LIMIT_LOW_40MS)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUSPULSETMR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUSPULSETMR,
                             MC13783_USB0_VBUSPULSETMR_910UA_40MS);
    }
    else if (inParam->PARAMS.usbOtgCfg & USB_VBUS_CURRENT_LIMIT_LOW_50MS)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUSPULSETMR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUSPULSETMR,
                             MC13783_USB0_VBUSPULSETMR_910UA_50MS);
    }
    else if (inParam->PARAMS.usbOtgCfg & USB_VBUS_CURRENT_LIMIT_LOW_60MS)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUSPULSETMR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUSPULSETMR,
                             MC13783_USB0_VBUSPULSETMR_910UA_60MS);
    }

    if (inParam->PARAMS.usbOtgCfg & USB_VBUS_PULLDOWN)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUS70KPDENB);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUS70KPDENB,
                             MC13783_USB0_VBUS70KPDENB_ENABLE);
    }

    SetRegister(addr0, data0, mask0);

    *result = PMIC_SUCCESS;
    rc = TRUE;

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbOtgClearConfigHandler
//
// This function clears the settings for USB OTG in USB mode
// for connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbOtgClearConfigHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                     PMIC_STATUS *result)
{
    UINT32 addr0 = MC13783_USB0_ADDR,     data0 = 0, mask0 = 0;
    BOOL rc = FALSE;

    *result = PMIC_ERROR;

    if (inParam->PARAMS.usbOtgCfg & USB_OTG_SE0CONN)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_SE0CONN);
        mask0 |= CSP_BITFVAL(MC13783_USB0_SE0CONN,
                             MC13783_USB0_SE0CONN_AUTOCON_DISABLE);
    }

    if (inParam->PARAMS.usbOtgCfg & USB_PULL_OVERRIDE)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_PULLOVR);
        mask0 |= CSP_BITFVAL(MC13783_USB0_PULLOVR,
                             MC13783_USB0_PULLOVR_DISABLE);
    }

    if (inParam->PARAMS.usbOtgCfg & USB_VBUS_PULLDOWN)
    {
        data0 |= CSP_BITFMASK(MC13783_USB0_VBUS70KPDENB);
        mask0 |= CSP_BITFVAL(MC13783_USB0_VBUS70KPDENB,
                             MC13783_USB0_VBUS70KPDENB_DISABLE);
    }

    SetRegister(addr0, data0, mask0);

    *result = PMIC_SUCCESS;
    rc = TRUE;

    return rc;
}


//------------------------------------------------------------------------------
//
// Function: PmicADCGetSettings
//
// This function gets the adc configuration information.
//
// Parameters:
//      channel
//           [in] channels to be configured.
//      group
//           [out] group setting.
//      chann
//           [out] channel setting in ADC1.
//      ena
//           [out] bits to be enabled in ADC0.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
static PMIC_STATUS PmicADCGetSettings(UINT16 channel, UINT32 *group,
                               UINT32 *chann, UINT32 *ena)
{
    *group = 0;
    *chann = 0;
    *ena = 0;

    switch (channel)
    {
        case CSP_BITFMASK(MC13783_ADC_BATT):
            // Select group 0
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP0);
            // Select channel battery voltage
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL0_BATT);

            *ena = CSP_BITFVAL(MC13783_ADC0_BATICON, MC13783_ADC0_ENABLE);

            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_BATT\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_BATTISNS):
            // Select group 0
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP0);
            // Select channel battery current
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL0_BATTISNS);

            *ena = CSP_BITFVAL(MC13783_ADC0_BATICON, MC13783_ADC0_ENABLE);

            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_BATTISNS\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_BPSNS):
            // Select group 0
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP0);
            // Select channel application supply
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL0_BPSNS);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_BPSNS\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_CHRGRAW):
            // Select group 0
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP0);
            // Select channel charger voltage
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL0_CHRGRAW);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_CHRGRAW\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_CHRGISNS):
            // Select group 0
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP0);
            // Select channel battery current
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL0_CHRGISNS);
            // Enable CHRGICON bit to retrieve charger current sample from ADC
            *ena = CSP_BITFVAL(MC13783_ADC0_CHRGICON, MC13783_ADC0_ENABLE);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_CHRGISNS\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_RTHEN):
            // Select group 0
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP0);
            // Select channel battery pack thermistor
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL0_RTHEN);
            // Enable RTHEN bit to retrieve battery pack thermistor sample from
            // ADC
            *ena = CSP_BITFVAL(MC13783_ADC0_RTHEN, MC13783_ADC0_ENABLE);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_RTHEN\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_LICELL):
            // Select group 0
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP0);
            // Select channel backup voltage (LICELL)
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL0_LICELL);
            // Enable BATICON bit to retrieve backup voltage sample from ADC
            *ena = CSP_BITFVAL(MC13783_ADC0_LICELLCON, MC13783_ADC0_ENABLE);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_LICELL\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_DTHEN):
            // Select group 0
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP0);
            // Select channel die temperature
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL0_DTHEN);
            // Enable DTHEN bit to retrieve die temperature sample from ADC
            *ena = CSP_BITFVAL(MC13783_ADC0_DTHEN, MC13783_ADC0_ENABLE);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_DTHEN\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_ADIN8):
            // Select group 1
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP1);
            // Select channel ADIN8
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL1_ADIN8);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_ADIN8\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_ADIN9):
            // Select group 1
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP1);
            // Select channel ADIN9
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL1_ADIN9);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_ADIN9\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_ADIN10):
            // Select group 1
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP1);
            // Select channel ADIN10
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL1_ADIN10);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_ADIN10\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_ADIN11):
            // Select group 1
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP1);
            // Sselect channel ADIN11
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1,
                                 MC13783_ADC1_ADA_SEL1_ADIN11);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_ADIN11\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_TSX1):
            // Select group 1
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP1);
            // Select channel TSX1
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL1_TSX1);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_TSX1\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_TSX2):
            // Select group 1
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP1);
            // Select channel TSX2
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL1_TSX2);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_TSX2\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_TSY1):
            // Select group 1
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP1);
            // Select channel TSY1
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL1_TSY1);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_TSY1\r\n")));
            break;

        case CSP_BITFMASK(MC13783_ADC_TSY2):
            // Select group 1
            *group = CSP_BITFVAL(MC13783_ADC1_AD_SEL,
                                 MC13783_ADC1_AD_SEL_GROUP1);
            // Select channel TSY2
            *chann = CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL1_TSY2);
            DEBUGMSG(ZONE_FUNC, (_T("MC13783_ADC_TSY2\r\n")));
            break;

        default:
            return PMIC_PARAMETER_ERROR;
    }
    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicADCConvert
//
// This function triggers a new set of ADC conversions and waits for the
// conversions to complete.
//
// Parameters:
//      mode
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
static PMIC_STATUS PmicADCConvert(void)
{
    PMIC_STATUS rc = PMIC_ERROR;
    UINT32 addr, data, mask;

    addr = MC13783_ADC1_ADDR;
    data = CSP_BITFVAL(MC13783_ADC1_ASC, MC13783_ADC1_ASC_START_ADC);
    mask = CSP_BITFMASK(MC13783_ADC1_ASC);

    // If we are in polled communication mode
    if (g_bAdcPolled)
    {
        // Start the ADC conversion
        SetRegister(addr, data, mask);

        // Poll the ADC1 until the ASC bit is clear
        DWORD maxRetries = 1000;
        do 
        {
            GetRegister(addr, &data);
        } while ((data & mask) && --maxRetries);

        if (!maxRetries)
        {
            ERRORMSG(TRUE, (_T("ADC operation timeout. \r\n")));
            goto cleanUp;
        }
    }
    else
    {
        // Make sure ADC done interrupt is unmasked and cleared
        PMICIoctlIntEnable( PMIC_MC13783_INT_ADCDONEI, TRUE );

        // Start the ADC conversion
        SetRegister(addr, data, mask);

        // Wait for ADC complete interrupt
        if(WaitForSingleObject(phIntrHdlr[PMIC_MC13783_INT_ADCDONEI],
            ADC_OPERATION_TIMEOUT) != WAIT_OBJECT_0)
        {
            ERRORMSG(TRUE, (_T("ADC operation timeout. \r\n")));
            GetRegister(MC13783_INT_STAT0_ADDR, &data);
            RETAILMSG(TRUE, (_T("MC13783_INT_STAT0 = 0x%x"), data));
            GetRegister(MC13783_INT_MSK0_ADDR, &data);
            RETAILMSG(TRUE, (_T("MC13783_INT_MSK0 = 0x%x"), data));
            GetRegister(MC13783_INT_STAT1_ADDR, &data);
            RETAILMSG(TRUE, (_T("MC13783_INT_STAT1 = 0x%x"), data));
            GetRegister(MC13783_INT_MSK1_ADDR, &data);
            RETAILMSG(TRUE, (_T("MC13783_INT_MSK1 = 0x%x"), data));
            BSPPmicReadIntrStatus();
            goto cleanUp;
        }
    }

    rc = PMIC_SUCCESS;

cleanUp:
    return rc;

}

//-----------------------------------------------------------------------------
//
// Function: PmicADCDisable
//
// This function is just a helper function that disables ADC conversions
//
// Parameters:
//
// Returns:
//
//-----------------------------------------------------------------------------
static VOID PmicADCDisable(void)
{
    UINT32 addr, data, mask;

    // Disable the ADC
    addr = MC13783_ADC1_ADDR;
    data = CSP_BITFVAL(MC13783_ADC1_ADEN, MC13783_ADC1_ADEN_DISABLE);
    mask = CSP_BITFMASK(MC13783_ADC1_ADEN);

    SetRegister(addr, data, mask);
}

//-----------------------------------------------------------------------------
//
// Function: PmicReadEightValues
//
//      This function is a helper function that reads eight values at a time
// from the ADC. This method uses the auto increment feature with two offset
// pointers. This is done to optimize the ADC read. Uses 4 CSPI transactions
// compared to 16.
//
// Parameters:
//      pReturnVal
//          [out]   This points to the eight returned UINT16 values.
//
// Returns:
//
//-----------------------------------------------------------------------------

static VOID PmicReadEightValues( UINT16* pReturnVal )
{
    UINT32 t_addr, t_data, t_mask;

    // configure for auto increment
    t_addr = MC13783_ADC0_ADDR;
    t_data = CSP_BITFVAL(MC13783_ADC0_ADINC1, MC13783_ADC0_ADINC_AUTO_INCR) |
             CSP_BITFVAL(MC13783_ADC0_ADINC2, MC13783_ADC0_ADINC_AUTO_INCR);
    t_mask = CSP_BITFMASK(MC13783_ADC0_ADINC1) |
             CSP_BITFMASK(MC13783_ADC0_ADINC2);

    SetRegister(t_addr, t_data, t_mask);

    // Set reading channel to ADA
    t_addr = MC13783_ADC1_ADDR;
    t_data = CSP_BITFVAL(MC13783_ADC1_ASC, MC13783_ADC1_ASC_ADC_IDLE)   |
             // we will use auto ADINC1
             CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL0_BATT) |
              // we will use auto ADINC2
             CSP_BITFVAL(MC13783_ADC1_ADA2, MC13783_ADC1_ADA_SEL0_CHRGISNS);
    t_mask = CSP_BITFMASK(MC13783_ADC1_ASC)  |
             CSP_BITFMASK(MC13783_ADC1_ADA1) |
             CSP_BITFMASK(MC13783_ADC1_ADA2);

    SetRegister(t_addr, t_data, t_mask);

    // Read conversion values
    for ( int i = 0; i < 4; i++ )
    {
        // Read channel data
        t_addr = MC13783_ADC2_ADDR;
        GetRegister(t_addr, &t_data);

        pReturnVal[i] = (UINT16)CSP_BITFEXT(t_data, MC13783_ADC2_ADD1);
        DEBUGMSG(ZONE_FUNC, (_T("t_data[%x] = %x\r\n"), i, pReturnVal[i]));

        pReturnVal[i+4] = (UINT16)CSP_BITFEXT(t_data, MC13783_ADC2_ADD2);
        DEBUGMSG(ZONE_FUNC, (_T("t_data[%x] = %x\r\n"), (i+4), pReturnVal[i+4]));
    }

}

//-----------------------------------------------------------------------------
//
// Function: PMICAdcSetMode
//
// This function sets the mode for the PMIC ADC.
//
// Parameters:
//      mode
//          [in] Requested ADC.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
static BOOL PMICAdcSetMode(PMIC_TOUCH_MODE mode)
{
    BOOL rc = FALSE;
    UINT32 addr, data, mask;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    addr = MC13783_ADC0_ADDR;
    if (mode != TM_INACTIVE)
    {
        data = CSP_BITFVAL(MC13783_ADC0_TSMOD, mode)                   |
               CSP_BITFVAL(MC13783_ADC0_ADREFMOD, MC13783_ADC0_ENABLE) |
               CSP_BITFVAL(MC13783_ADC0_ADREFEN, MC13783_ADC0_ENABLE);
    }
    else
    {
        data = CSP_BITFVAL(MC13783_ADC0_TSMOD, mode)                    |
               CSP_BITFVAL(MC13783_ADC0_ADREFMOD, MC13783_ADC0_DISABLE) |
               CSP_BITFVAL(MC13783_ADC0_ADREFEN, MC13783_ADC0_DISABLE);
    }

    mask = CSP_BITFMASK(MC13783_ADC0_TSMOD)    |
           CSP_BITFMASK(MC13783_ADC0_ADREFMOD) |
           CSP_BITFMASK(MC13783_ADC0_ADREFEN);

    SetRegister(addr, data, mask);

    if (mode == TM_INTERRUPT)
    {
        // MC13783 workaround for Pass 2.0 and 2.1
        addr = MC13783_ADC1_ADDR;
        data = CSP_BITFVAL(MC13783_ADC1_AD_SEL, MC13783_ADC1_AD_SEL_GROUP1)   |
               CSP_BITFVAL(MC13783_ADC1_ADEN, MC13783_ADC1_ADEN_ENABLE)       |
               CSP_BITFVAL(MC13783_ADC1_RAND, MC13783_ADC1_RAND_1CHAN_8X)     |
               CSP_BITFVAL(MC13783_ADC1_ATO, MC13783_ADC_DEFAULT_ATO_SETTING) |
               CSP_BITFVAL(MC13783_ADC1_ATOX, MC13783_ADC1_ATOX_DELAY_FIRST)  |
               CSP_BITFVAL(MC13783_ADC1_ADTRIGIGN, MC13783_ADC1_ADTRIGIGN_IGNORE);
        mask = MC13783_ADC_REGISTER_MASK_ALL;

        SetRegister(addr, data, mask);

        // Request ADC conversion
        if (PmicADCConvert() != PMIC_SUCCESS)
        {
            ERRORMSG(1, (_T("PmicADCConvert failed!\r\n")));
            goto cleanUp;
        }

    }
    rc = TRUE;

cleanUp:

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: PMICIoctlAdcSetMode
//
// This function sets the mode for the PMIC ADC.
//
// Parameters:
//      pBufIn
//          [in] Points to requested PMIC_TOUCH_MODE.
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//          [out] Unused.
//
//      dwLenOut
//          [out] Unused.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL PMICIoctlAdcSetMode(PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut,
                         DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenIn);

    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s() %d\r\n"), __WFUNCTION__));
    EnterCriticalSection(&adcCs);

    rc = PMICAdcSetMode(*((PMIC_TOUCH_MODE *) pBufIn));

    LeaveCriticalSection(&adcCs);
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: CheckIntIdValid
//
// This is a helper function that checks the parameter range.
//
// Parameters:
//      IntID
//          [in] Interrupt ID.
//
// Returns:
//      TRUE if valid range, FALSE otherwise.
//
//-----------------------------------------------------------------------------
static BOOL CheckIntIdValid(UINT32 IntID)
{
    // Check low and high
    if ((IntID < 0) || (IntID > PMIC_INT_MAX_ID))
    {
        DEBUGMSG(ZONE_ERROR, (_T("Invalid Input Parameter.\r\n")));
        return FALSE;
    }

    // Check lower half
    if (IntID < PMIC_MC13783_INT_ID_OFFSET)
    {
        // Check range
        if ((PMIC_INTS0_TO_AP & (1 << IntID)) == 0)
        {
            DEBUGMSG(ZONE_ERROR, (_T("Invalid Input Parameter.\r\n")));
            return FALSE;
        }
    }
    // Check upper half
    else
    {
        // Check range
        if ((PMIC_INTS1_TO_AP & (1 << (IntID - PMIC_MC13783_INT_ID_OFFSET))) == 0)
        {
            DEBUGMSG(ZONE_ERROR, (_T("Invalid Input Parameter.\r\n")));
            return FALSE;
        }
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PMICIoctlIntEnable
//
// This function enables the selected PMIC interrupt.
//
// Parameters:
//      IntID
//          [in] Interrupt ID.
//
//      EnableFlag
//          [in] Flag set to TRUE = enable, FALSE = disable
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
extern "C"  BOOL PMICIoctlIntEnable(UINT32 IntID, BOOL EnableFlag)
{

    if (CheckIntIdValid(IntID))
    {
        EnterCriticalSection(&intRegCs);

        // Check lower half
        if (IntID < PMIC_MC13783_INT_ID_OFFSET)
        {
            if (EnableFlag)
            {
                SetRegister(MC13783_INT_STAT0_ADDR, 1 << IntID, 1 << IntID);
                // Unmask the interrupt
                SetRegister(MC13783_INT_MSK0_ADDR, 0, 1 << IntID);
            }
            else
                // Mask the interrupt
                SetRegister(MC13783_INT_MSK0_ADDR, 1 << IntID, 1 << IntID);
        }
        else
        {   // Check upper half
            IntID -= PMIC_MC13783_INT_ID_OFFSET;

            if (EnableFlag)
            {
                SetRegister(MC13783_INT_STAT1_ADDR, 1 << IntID, 1 << IntID);
                // Unmask the interrupt
                SetRegister(MC13783_INT_MSK1_ADDR, 0, 1 << IntID);
            }
            else
                // Mask the interrupt
                SetRegister(MC13783_INT_MSK1_ADDR, 1 << IntID, 1 << IntID);
        }

        LeaveCriticalSection(&intRegCs);

        return TRUE;

    }   // CheckIntIdValid
    else
        return FALSE;

}

//-----------------------------------------------------------------------------
//
// Function: PMICIoctlAdcTouchRead
//
// This function places the PMIC in touch position mode and acquires
// touch samples.
//
// Parameters:
//      pBufIn
//          [in] Unused.
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//          [out] Points to data buffer to hold touch samples.
//
//      dwLenOut
//          [out] Unused.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
static BOOL PMICIoctlAdcTouchRead(PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut,
                      DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufIn);

    BOOL rc = FALSE;
    UINT32 addr, data, mask;
    UINT16* pData = (UINT16*) pBufOut;

    // Grab critical section, ADC can only handle one set of conversions
    // at a time
    EnterCriticalSection(&adcCs);

    // Place the ADC in touch position mode
    if (!PMICAdcSetMode(TM_POSITION))
    {
        ERRORMSG(1, (_T("PMICAdcSetMode failed\r\n")));
        goto cleanUp;
    }

    // Set AD_SEL to 1 for second set of converters;
    // Enable A/D and set RAND to 0 for converting multiple channels
    addr = MC13783_ADC1_ADDR;
    data = CSP_BITFVAL(MC13783_ADC1_AD_SEL, MC13783_ADC1_AD_SEL_GROUP1)   |
           CSP_BITFVAL(MC13783_ADC1_ADEN, MC13783_ADC1_ADEN_ENABLE)       |
           CSP_BITFVAL(MC13783_ADC1_RAND, MC13783_ADC1_RAND_1CHAN_8X)     |
           CSP_BITFVAL(MC13783_ADC1_ATO, MC13783_ADC_DEFAULT_ATO_SETTING) |
           CSP_BITFVAL(MC13783_ADC1_ATOX, MC13783_ADC1_ATOX_DELAY_FIRST)  |
           CSP_BITFVAL(MC13783_ADC1_ADTRIGIGN, MC13783_ADC1_ADTRIGIGN_IGNORE);
    mask = MC13783_ADC_REGISTER_MASK_ALL;

    SetRegister(addr, data, mask);

    // We select 1 channel and 8 samples. So we set non-automatic 
    // increment mode.
    addr = MC13783_ADC0_ADDR;
    data = CSP_BITFVAL(MC13783_ADC0_ADREFMOD, MC13783_ADC0_ENABLE)      |
           CSP_BITFVAL(MC13783_ADC0_ADREFEN, MC13783_ADC0_ENABLE)       |
           CSP_BITFVAL(MC13783_ADC0_ADINC1, MC13783_ADC0_ADINC_NO_INCR) |
           CSP_BITFVAL(MC13783_ADC0_ADINC2, MC13783_ADC0_ADINC_NO_INCR);
    mask = CSP_BITFMASK(MC13783_ADC0_ADINC1)   |
           CSP_BITFMASK(MC13783_ADC0_ADINC2)   |
           CSP_BITFMASK(MC13783_ADC0_ADREFMOD) |
           CSP_BITFMASK(MC13783_ADC0_ADREFEN);

    SetRegister(addr, data, mask);

    // Request ADC conversion
    if (PmicADCConvert() != PMIC_SUCCESS)
    {
        ERRORMSG(1, (_T("PmicADCConvert failed!\r\n")));
        goto cleanUp;
    }

    // read out the eight values and place in area pointed to by pData
    PmicReadEightValues(pData);

    // Disable the ADC
    PmicADCDisable();
    rc = TRUE;

cleanUp:

    // Place the ADC in inactive mode
    if (!PMICAdcSetMode(TM_INACTIVE))
    {
        ERRORMSG(1, (_T("PMICAdcSetMode failed\r\n")));
    }

    LeaveCriticalSection(&adcCs);

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: PmicIoctlADCGetHandsetCurrent
//
// This function gets handset battery current measurement values. 
//
// Parameters:
//      pBufIn
//          [in] Points to requested ADC converter mode:
//               ADC_8CHAN_1X  or  ADC_1CHAN_8X.
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//          [out] pointer to the handset battery current measurement
//               value(s).
//
//      dwLenOut
//          [out] Unused.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
static BOOL PmicIoctlADCGetHandsetCurrent(PBYTE pBufIn, DWORD dwLenIn,
                                          PBYTE pBufOut, DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(dwLenIn);

    BOOL rc = FALSE;
    UINT32 addr, data, mask;
    PMIC_ADC_CONVERTOR_MODE mode = *((PMIC_ADC_CONVERTOR_MODE*)pBufIn);
    UINT16* pData = (UINT16*)pBufOut;

    // Grab critical section, ADC can only handle one set of
    // conversions at a time
    EnterCriticalSection(&adcCs);
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    addr = MC13783_ADC1_ADDR;
           // enable A/D 
    data = CSP_BITFVAL(MC13783_ADC1_ADEN, MC13783_ADC1_ADEN_ENABLE)       |
           CSP_BITFVAL(MC13783_ADC1_AD_SEL, MC13783_ADC1_AD_SEL_GROUP0)   |
           // set ATO delay only before first conversion
           CSP_BITFVAL(MC13783_ADC1_ATOX, MC13783_ADC1_ATOX_DELAY_FIRST)  |
           CSP_BITFVAL(MC13783_ADC1_ATO, MC13783_ADC_DEFAULT_ATO_SETTING) |
           CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL0_BATTISNS) |
           // ignore ADTRIG input
           CSP_BITFVAL(MC13783_ADC1_ADTRIGIGN, MC13783_ADC1_ADTRIGIGN_IGNORE);

    switch (mode)
    {
        // Set RAND = 0, i.e. read 8 channels, 1 time sample
        case ADC_8CHAN_1X: // RAND = 0
            data |= CSP_BITFVAL(MC13783_ADC1_RAND, MC13783_ADC1_RAND_8CHAN_1X);
            break;

        // Set RAND = 1, i.e. read 1 channel, 8 times sample
        case ADC_1CHAN_8X: // RAND = 1
            data |= CSP_BITFVAL(MC13783_ADC1_RAND, MC13783_ADC1_RAND_1CHAN_8X);
            break;
    }
    mask = MC13783_ADC_REGISTER_MASK_ALL;

    SetRegister(addr, data, mask);

    addr = MC13783_ADC0_ADDR;
    data = CSP_BITFVAL(MC13783_ADC0_ADINC1, MC13783_ADC0_ADINC_NO_INCR) |
           CSP_BITFVAL(MC13783_ADC0_ADINC2, MC13783_ADC0_ADINC_NO_INCR) |
           CSP_BITFVAL(MC13783_ADC0_BATICON, MC13783_ADC0_ENABLE);
    mask = CSP_BITFMASK(MC13783_ADC0_ADINC1) |
           CSP_BITFMASK(MC13783_ADC0_ADINC2) |
           CSP_BITFMASK(MC13783_ADC0_BATICON);

    SetRegister(addr, data, mask);


    // Request ADC conversion
    if (PmicADCConvert() != PMIC_SUCCESS)
    {
        ERRORMSG(1, (_T("PmicADCConvert failed!\r\n")));
        goto cleanUp;
    }

    // When in this mode, we only return one value (BATTISNS)
    if (mode == ADC_8CHAN_1X)
    {
        // Set reading channel to ADA
        addr = MC13783_ADC1_ADDR;
        data = CSP_BITFVAL(MC13783_ADC1_ASC, MC13783_ADC1_ASC_ADC_IDLE) |
               CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL0_BATTISNS);
        mask = CSP_BITFMASK(MC13783_ADC1_ADA1) |
               CSP_BITFMASK(MC13783_ADC1_ASC);

        SetRegister(addr, data, mask);

        // Read channel data
        addr = MC13783_ADC2_ADDR;
        GetRegister(addr, &data);

        *pData = (UINT16)CSP_BITFEXT(data, MC13783_ADC2_ADD1);
        DEBUGMSG(ZONE_FUNC, (_T("data = %d\r\n"), *pData));
    }
    else
    {   // else... read eight values at a time
        // ADA0=BATTP; ADA1=BATTP-BATTI; ADA2=BATTP; ADA3=BATTP-BATTI;
        // ADA4=BATTP; ADA5=BATTP-BATTI; ADA6=BATTP; ADA7=BATTP-BATTI.
        PmicReadEightValues(pData);
    }

    // Disable the ADC
    PmicADCDisable();
    rc = TRUE;

cleanUp:

    LeaveCriticalSection(&adcCs);
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function: PmicIoctlADCGetMultipleChannelsSamples
//
// This function gets one sample for multiple channels.
//
// Parameters:
//      channels
//          [in] selected  channels (up to 16 channels).
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//          [out] pointer to  the sampled values (up to 16 sampled values).
//               The returned results will be ordered according to the channel
//               selected.
//      dwLenOut
//          [out] Unused.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL PmicIoctlADCGetMultipleChannelsSamples(PBYTE pBufIn, DWORD dwLenIn,
                                            PBYTE pBufOut, DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(dwLenIn);

    BOOL rc = FALSE;
    UINT32 addr[3], data[3], mask[3];
    UINT32 temp, tmp_addr;
    UINT8 i, j;
    UINT16 channels = *((UINT16*)pBufIn);
    UINT16* pData = (UINT16*) pBufOut;
    UINT8 low_index = 0, high_index = 0;
    UINT16 tmp[2][8];

    // Grab critical section, ADC can only handle one set of
    // conversions at a time
    EnterCriticalSection(&adcCs);

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // Configure ADC for the specified channels conversion. We used 8 channel
    // and 1 time sample mode, and placed the A/D data in result register 1
    // and 2.
    //

    // If some channels in the group 0, other channels are in the group1,
    // we have to do two ADC conversions.

    // Group 0 when AD_SEL1 = 0
    if ((channels & CSP_BITFMASK(MC13783_ADC_BATT))     |
        (channels & CSP_BITFMASK(MC13783_ADC_BATTISNS)) |
        (channels & CSP_BITFMASK(MC13783_ADC_BPSNS))    |
        (channels & CSP_BITFMASK(MC13783_ADC_CHRGRAW))  |
        (channels & CSP_BITFMASK(MC13783_ADC_CHRGISNS)) |
        (channels & CSP_BITFMASK(MC13783_ADC_RTHEN))    |
        (channels & CSP_BITFMASK(MC13783_ADC_LICELL))   |
        (channels & CSP_BITFMASK(MC13783_ADC_DTHEN)))
    {
        addr[0] = MC13783_ADC1_ADDR;

                  // enable A/D
        data[0] = CSP_BITFVAL(MC13783_ADC1_ADEN, MC13783_ADC1_ADEN_ENABLE)     |
            // set RAND = 0, i.e. read 8 channels, 1 time sample
            CSP_BITFVAL(MC13783_ADC1_RAND, MC13783_ADC1_RAND_8CHAN_1X)         |
            // set ATO delay only before first conversion
            CSP_BITFVAL(MC13783_ADC1_ATOX, MC13783_ADC1_ATOX_DELAY_FIRST)      |
            CSP_BITFVAL(MC13783_ADC1_AD_SEL, MC13783_ADC1_AD_SEL_GROUP0)       |
            CSP_BITFVAL(MC13783_ADC1_ATO, MC13783_ADC_DEFAULT_ATO_SETTING)     |
            // ignore ADTRIG input
            CSP_BITFVAL(MC13783_ADC1_ADTRIGIGN, MC13783_ADC1_ADTRIGIGN_IGNORE) |
            // we will use auto ADINC1
            CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL0_BATT)         |
            // we will use auto ADINC2
            CSP_BITFVAL(MC13783_ADC1_ADA2, MC13783_ADC1_ADA_SEL0_CHRGISNS);

        mask[0] =  MC13783_ADC_REGISTER_MASK_ALL;

        high_index = 1;
    }
    else
    {
        low_index = 1;
    }

    // Group 1 when AD_SEL1 = 1
    if ((channels & CSP_BITFMASK(MC13783_ADC_ADIN8))  |
        (channels & CSP_BITFMASK(MC13783_ADC_ADIN9))  |
        (channels & CSP_BITFMASK(MC13783_ADC_ADIN10)) |
        (channels & CSP_BITFMASK(MC13783_ADC_ADIN11)) |
        (channels & CSP_BITFMASK(MC13783_ADC_TSX1))   |
        (channels & CSP_BITFMASK(MC13783_ADC_TSX2))   |
        (channels & CSP_BITFMASK(MC13783_ADC_TSY1))   |
        (channels & CSP_BITFMASK(MC13783_ADC_TSY2)))
    {
        addr[1] = MC13783_ADC1_ADDR;
                  // enable A/D
        data[1] = CSP_BITFVAL(MC13783_ADC1_ADEN, MC13783_ADC1_ADEN_ENABLE)     |
            // set RAND = 1, i.e. read 8 channels, 1 time sample
            CSP_BITFVAL(MC13783_ADC1_RAND, MC13783_ADC1_RAND_8CHAN_1X)         |
            // set ATO delay only before first conversion
            CSP_BITFVAL(MC13783_ADC1_ATOX, MC13783_ADC1_ATOX_DELAY_FIRST)      |
            CSP_BITFVAL(MC13783_ADC1_AD_SEL, MC13783_ADC1_AD_SEL_GROUP1)       |
            CSP_BITFVAL(MC13783_ADC1_ATO, MC13783_ADC_DEFAULT_ATO_SETTING)     |
            // ignore ADTRIG input
            CSP_BITFVAL(MC13783_ADC1_ADTRIGIGN, MC13783_ADC1_ADTRIGIGN_IGNORE) |
            CSP_BITFVAL(MC13783_ADC1_ADA1, MC13783_ADC1_ADA_SEL1_ADIN8)        |
            CSP_BITFVAL(MC13783_ADC1_ADA2, MC13783_ADC1_ADA_SEL1_TSX1);

        mask[1] =  MC13783_ADC_REGISTER_MASK_ALL;

        high_index = 2;
    }

    if (high_index <= low_index)
    {
        ERRORMSG(1,
         (_T("PmicIoctlADCGetMultipleChannelsSamples Invalid Parameter!\r\n")));
        goto cleanUp;
    }

    // We select 8 channels and 1 times sample. Set automatic increment mode
    addr[2] = MC13783_ADC0_ADDR;

    data[2] = CSP_BITFVAL(MC13783_ADC0_ADINC1, MC13783_ADC0_ADINC_AUTO_INCR) |
              CSP_BITFVAL(MC13783_ADC0_ADINC2, MC13783_ADC0_ADINC_AUTO_INCR) |
              CSP_BITFVAL(MC13783_ADC0_BATICON, MC13783_ADC0_ENABLE)         |
              CSP_BITFVAL(MC13783_ADC0_CHRGICON, MC13783_ADC0_ENABLE)        |
              CSP_BITFVAL(MC13783_ADC0_RTHEN, MC13783_ADC0_ENABLE)           |
              CSP_BITFVAL(MC13783_ADC0_LICELLCON, MC13783_ADC0_ENABLE)       |
              CSP_BITFVAL(MC13783_ADC0_DTHEN, MC13783_ADC0_ENABLE);

    mask[2] = CSP_BITFMASK(MC13783_ADC0_ADINC1)   |
              CSP_BITFMASK(MC13783_ADC0_ADINC2)   |
              CSP_BITFMASK(MC13783_ADC0_BATICON)  |
              CSP_BITFMASK(MC13783_ADC0_CHRGICON) |
              CSP_BITFMASK(MC13783_ADC0_RTHEN)    |
              CSP_BITFMASK(MC13783_ADC0_DTHEN)    |
              CSP_BITFMASK(MC13783_ADC0_LICELLCON);

    SetRegister(addr[2], data[2], mask[2]);

    // Process the ADC for group(s)
    for (i = low_index; i < high_index; i++)
    {
        SetRegister(addr[i], data[i], mask[i]);

        // Request ADC conversion
        if (PmicADCConvert() != PMIC_SUCCESS)
        {
            ERRORMSG(1, (_T("PmicADCConvert failed!\r\n")));
            goto cleanUp;
        }

        // One time reading for getting 2 values, total 4 times reading
        tmp_addr = MC13783_ADC2_ADDR;
        for (j = 0; j < 4; j++)
        {
            // Read channel data
            GetRegister(tmp_addr, &temp);

            // Get the ADC values
            tmp[i][j] = (UINT16)CSP_BITFEXT(temp, MC13783_ADC2_ADD1);
            tmp[i][4+j] = (UINT16)CSP_BITFEXT(temp, MC13783_ADC2_ADD2);

        } // End of for loop reading register

    } // End of for loop processing group(s)

    // Return the pData's values following channel order like:
    // BATT, BATTISNS, BPSNS, CHRGRAW, CHRGISNS, RTHEN, LICELL, DTHEN,
    // ADIN8, ADIN9, ADIN10, ADIN11, TSX1, TSX2, TSY1, TSY2

    // In addition, we will only return those values that the caller selects
    // the channel. No more!
    for (i = low_index; i<high_index; i++)
    {
        for (j = 0; j<8; j++)
        {
            if ((channels >> (i*8+j)) & 1)
            {
                *pData++ = tmp[i][j];
                DEBUGMSG(ZONE_FUNC, (_T("data = %d\r\n"), tmp[i][j]));
            }
        }
    }

    // Disable the ADC
    PmicADCDisable();
    rc = TRUE;

cleanUp:

    LeaveCriticalSection(&adcCs);
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: PmicIoctlADCGetSingleChannelSample
//
// This function gets one channel sample. It can be a single sample or multiple
// depending on the bMultipleSampleFlag.
//
// Parameters:
//      pBufIn
//          [in] pointer to the selected channel
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//         [out] pointer to  the sampled values
//
//      dwLenOut
//          [out] Unused.
//
//      bMultipleSampleFlag
//          [in] Flag to chose between 1x and 8x samples ... if TRUE then 8x
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//-----------------------------------------------------------------------------
BOOL
PmicIoctlADCGetSingleChannelSample(PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut,
                                   DWORD dwLenOut, BOOL bMultipleSampleFlag)
{
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(dwLenIn);
    
    BOOL rc = FALSE;
    UINT32 addr, data, mask;
    UINT32 temp = 0;
    UINT32 group = 0;
    UINT32 chann = 0;
    UINT32 ena = 0;
    UINT16 channels = *((UINT16*)pBufIn);
    UINT16* pData = (UINT16*) pBufOut;

    // Grab critical section, ADC can only handle one set of conversions
    // at a time.
    EnterCriticalSection(&adcCs);

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (PmicADCGetSettings(channels, &group, &chann, &ena) != PMIC_SUCCESS)
    {
        ERRORMSG(1,
         (_T("PmicIoctlADCGetSingleChannelSample Invalid Parameter!\r\n")));
        goto cleanUp;
    }

    if( bMultipleSampleFlag == TRUE)
    {
        // We used 1 channels and 8 times sample, and placed the A/D data in 
        // result register 1 Set RAND = 1, i.e. read 1 channels, 8 times sample
        data = CSP_BITFVAL(MC13783_ADC1_RAND, MC13783_ADC1_RAND_1CHAN_8X) |
               group | chann;
    }
    else
    {
        // We used 8 channel and 1 time sample, and placed the A/D data in 
        // result register 1. Set RAND = 0, i.e. read 8 channels, 1 time sample
        data = CSP_BITFVAL(MC13783_ADC1_RAND, MC13783_ADC1_RAND_8CHAN_1X) |
               group;
    }

    // enable A/D set ATO delay only before first conversion, and ignore ADTRIG
    //input
    addr = MC13783_ADC1_ADDR;
    data |= CSP_BITFVAL(MC13783_ADC1_ADEN, MC13783_ADC1_ADEN_ENABLE)       |
            CSP_BITFVAL(MC13783_ADC1_ATOX, MC13783_ADC1_ATOX_DELAY_FIRST)  |
            CSP_BITFVAL(MC13783_ADC1_ATO, MC13783_ADC_DEFAULT_ATO_SETTING) |
            CSP_BITFVAL(MC13783_ADC1_ADTRIGIGN, MC13783_ADC1_ADTRIGIGN_IGNORE);
    mask = MC13783_ADC_REGISTER_MASK_ALL;

    SetRegister(addr, data, mask);


    // We read one sample only. So we set non-automatic increment mode.
    addr = MC13783_ADC0_ADDR;
    data = CSP_BITFVAL(MC13783_ADC0_ADINC1, MC13783_ADC0_ADINC_NO_INCR) |
           CSP_BITFVAL(MC13783_ADC0_ADINC2, MC13783_ADC0_ADINC_NO_INCR) |
           ena;
    mask = CSP_BITFMASK(MC13783_ADC0_ADINC1)   |
           CSP_BITFMASK(MC13783_ADC0_ADINC2)   |
           CSP_BITFMASK(MC13783_ADC0_BATICON)  |
           CSP_BITFMASK(MC13783_ADC0_CHRGICON) |
           CSP_BITFMASK(MC13783_ADC0_RTHEN)    |
           CSP_BITFMASK(MC13783_ADC0_DTHEN)    |
           CSP_BITFMASK(MC13783_ADC0_LICELLCON);

    SetRegister(addr, data, mask);

    // Request ADC conversion
    if (PmicADCConvert() != PMIC_SUCCESS)
    {
        ERRORMSG(1, (_T("PmicADCConvert failed!\r\n")));
        goto cleanUp;
    }

    if( bMultipleSampleFlag == TRUE)
    {
        // read out the eight values and place in area pointed to by pData
        PmicReadEightValues(pData);
    }
    else
    {
        // read out one channel, set reading channel to ADA
        addr = MC13783_ADC1_ADDR;
        data = CSP_BITFVAL(MC13783_ADC1_ASC, MC13783_ADC1_ASC_ADC_IDLE) |chann;
        mask = CSP_BITFMASK(MC13783_ADC1_ADA1) |
               CSP_BITFMASK(MC13783_ADC1_ASC);

        SetRegister(addr, data, mask);

        // Read channel data
        addr = MC13783_ADC2_ADDR;
        GetRegister(addr, &temp);

        *pData = (UINT16)CSP_BITFEXT(temp, MC13783_ADC2_ADD1);
        DEBUGMSG(ZONE_FUNC, (_T("data = %d\r\n"), *pData));
    }

    // Disable the ADC
    PmicADCDisable();
    rc = TRUE;

cleanUp:

    LeaveCriticalSection(&adcCs);
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function: PmicIoctlADCSetComparatorThresholds
//
// This function sets WHIGH and WLOW for automatic ADC result comparators.  
//
// Parameters:
//      pBufIn
//          [in] pointer to threshold array
//                   threshold[0] = whigh
//                   threshold[1] = wlow
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//         [out] Unused
//
//      dwLenOut
//          [out] Unused.
//
// Returns:
//              Status.
//-----------------------------------------------------------------------------
BOOL PmicIoctlADCSetComparatorThresholds(PBYTE pBufIn, DWORD dwLenIn,
                        PBYTE pBufOut, DWORD dwLenOut)
{

    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);

    BOOL rc = FALSE;
    UINT32 addr, data, mask;
    UINT16 whigh, wlow;

    // Grab critical section, ADC can only handle one set of
    // conversions at a time
    EnterCriticalSection(&adcCs);

    whigh = (UINT16)pBufIn[0];
    wlow = (UINT16)pBufIn[1];

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // Configure ADC3 to set whigh and wlow values
    addr = MC13783_ADC3_ADDR;
    data = CSP_BITFVAL(MC13783_ADC3_WHIGH, 
           (whigh & MC13783_ADC_MAX_COMPARATOR_LEVEL) >> 4) |
           CSP_BITFVAL(MC13783_ADC3_WLOW,
           (wlow & MC13783_ADC_MAX_COMPARATOR_LEVEL) >> 4);
    mask = CSP_BITFMASK(MC13783_ADC3_WHIGH) | CSP_BITFMASK(MC13783_ADC3_WLOW);

    SetRegister(addr, data, mask);


    // Configure ADC0 to enable the comparison.
    addr = MC13783_ADC0_ADDR;
    data = CSP_BITFVAL(MC13783_ADC0_WCOMP, MC13783_ADC0_ENABLE);
    mask = CSP_BITFMASK(MC13783_ADC0_WCOMP);

    SetRegister(addr, data, mask);

    rc = TRUE;

    LeaveCriticalSection(&adcCs);
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Init
//
// The Device Manager calls this function as a result of a call to the
// ActivateDevice() function.  This function will retrieve the handle to the
// device context from the registry and then initialize the PMI module by
// turning off all of the peripherals that are not being used.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD
PMI_Init(LPCTSTR pContext)
{
    UNREFERENCED_PARAMETER(pContext);

    DWORD rc = 0;

    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("+%s(%s)\r\n"), __WFUNCTION__, pContext));

    // Initialize the PMIC driver
    if (!InitializePMIC())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s:  Failed to initialize PMIC!!!\r\n"),
            __WFUNCTION__));
        goto cleanUp;
    }

    rc = 1;

cleanUp:
    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("-%s() returning 1\r\n"), __WFUNCTION__));

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Deinit
//
// The Device Manager calls this function as a result of a call to the
// DeactivateDevice() function.  This function will return any resources
// allocated while using this driver.
//
// Parameters:
//      hDeviceContext
//          [in] The handle to the context.
//
// Returns:
//      TRUE
//
//-----------------------------------------------------------------------------
BOOL
PMI_Deinit(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER(hDeviceContext);

    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("+%s(%d)\r\n"), __WFUNCTION__, hDeviceContext));

    CleanupPMIC();

    DEBUGMSG(ZONE_INIT||ZONE_FUNC, 
                (TEXT("-%s() returning %d\r\n"), __WFUNCTION__, FALSE));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PMI_Open
//
// Called when an application attempts to establish a connection to this driver
// This function will verify that a trusted application has made the request 
// and deny access to all non-trusted applications.
//
// Parameters:
//      hDeviceContext
//          [in] Ignored
//      AccessCode
//          [in] Ignored
//      ShareMode
//          [in] Ignored
//
// Returns:
//      Returns 0 if the calling application is not trusted and 1 if it is
//      trusted.
//
//-----------------------------------------------------------------------------
DWORD
PMI_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);
    UNREFERENCED_PARAMETER(hDeviceContext);

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s(%d, %d, %d)\r\n"), __WFUNCTION__,
        hDeviceContext, AccessCode, ShareMode));


    DEBUGMSG(ZONE_FUNC, (TEXT("-%s() returning 1\r\n"), __WFUNCTION__));

    return 1;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Close
//
// Called when an application attempts to close a connection to this driver.
// This function does nothing.
//
// Parameters:
//      hOpenContext
//          [in] Ignored
//
// Returns:
//      TRUE
//
//-----------------------------------------------------------------------------
BOOL
PMI_Close(DWORD hOpenContext)
{
    UNREFERENCED_PARAMETER(hOpenContext);

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s(%d)\r\n"), __WFUNCTION__, hOpenContext));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s() returning TRUE\r\n"), __WFUNCTION__));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_IOControl
//
// Called when an application calls the DeviceIoControl() function.  This
// function operates differently based upon the IOCTL that is passed to it.
// The following table describes the expected values associated with each
// IOCTL implemented by this function.
//
// dwCode                      pBufIn         pBufOut         Description
// --------------------------- -------------- --------------- -----------------
//
// Parameters:
//      hOpenContext
//          [in] Ignored
//
//      dwCode
//          [in] The IOCTL requested.
//
//      pBufIn
//          [in] Input buffer.
//
//      dwLenIn
//          [in] Length of the input buffer.
//
//      pBufOut
//          [out] Output buffer.
//
//      dwLenOut
//          [out] The length of the output buffer.
//
//      pdwActualOut
//          [out] Size of output buffer returned to application.
//
// Returns:
//      TRUE if the IOCTL is handled. FALSE if the IOCTL was not recognized or
//      an error occurred while processing the IOCTL
//
//-----------------------------------------------------------------------------
BOOL
PMI_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    UNREFERENCED_PARAMETER(hOpenContext);

    BOOL result = FALSE;
    UINT32 temp;
    PMIC_PARAM_LLA_WRITE_REG* pwr;
    PMIC_PARAM_INT_REGISTER* pir;
    LPTSTR event;
    UINT32 reg_id, int_id;
    PVOID pDestMarshalled;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s(%d)\r\n"), __WFUNCTION__, hOpenContext));

    switch (dwCode)
    {
    case PMIC_IOCTL_LLA_READ_REG:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_LLA_READ_REG\r\n")));

        // temp is the PMIC register ID.
        temp = *pBufIn;

        EnterCriticalSection(&adcCs);
        GetRegister(temp, (UINT32*)pBufOut);
        LeaveCriticalSection(&adcCs);

        result = TRUE;
        break;

    case PMIC_IOCTL_LLA_WRITE_REG:
        DEBUGMSG(ZONE_FUNC,
                    (_T("PMI_IOControl PMIC_IOCTL_LLA_WRITE_REG\r\n")));

        pwr = (PMIC_PARAM_LLA_WRITE_REG *)pBufIn;

        EnterCriticalSection(&adcCs);
        SetRegister(pwr->addr,
                    (pwr->data & PMIC_ALL_BITS),
                    (pwr->mask & PMIC_ALL_BITS));
        LeaveCriticalSection(&adcCs);

        result = TRUE;
        break;

    case PMIC_IOCTL_LLA_INT_REGISTER:
        DEBUGMSG(ZONE_FUNC,
                 (_T("PMI_IOControl PMIC_IOCTL_LLA_INT_REGISTER\r\n")));

        pir = (PMIC_PARAM_INT_REGISTER *)pBufIn;

        if(CheckIntIdValid(pir->int_id))
        {
            if (phIntrHdlr[pir->int_id] != NULL)
            {
                result = FALSE;
                break;
            }
            else
            {
                // Ensure safe access to the event name buffer.
                if (CeOpenCallerBuffer(&pDestMarshalled,
                                       pir->event_name,
                                       MAX_PATH,
                                       ARG_I_PTR,
                                       FALSE) != S_OK)
                {
                    result = FALSE;
                    ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() failed\r\n")));
                    break;
                }

                // We can now safely use a pointer to our local copy of the
                // event name.
                event = (LPTSTR)pDestMarshalled;

                phIntrHdlr[pir->int_id] =
                    CreateEvent(NULL, FALSE, FALSE, event);

                if (phIntrHdlr[pir->int_id] == NULL)
                {
                    DEBUGMSG(ZONE_ERROR,
                                (_T("CreateEvent failed for event_name.\r\n")));
                    result = FALSE;
                    break;
                }

                // Unmask the interrupt.
                if (pir->int_id < PMIC_MC13783_INT_ID_OFFSET)
                {
                    reg_id = MC13783_INT_MSK0_ADDR;
                    int_id = (1 << pir->int_id);
                }
                else
                {
                    reg_id = MC13783_INT_MSK1_ADDR;
                    int_id = (1 << (pir->int_id - PMIC_MC13783_INT_ID_OFFSET));
                }
                EnterCriticalSection(&intRegCs);
                SetRegister(reg_id, 0, int_id);
                LeaveCriticalSection(&intRegCs);

                // Release the temporary access to the event name that was
                // obtained by calling CeOpenCallerBuffer().
                if (FAILED(CeCloseCallerBuffer(pDestMarshalled,
                                               pir->event_name,
                                               MAX_PATH,
                                               ARG_I_PTR)))
                {
                    ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() failed\r\n")));
                    result = FALSE;
                    break;
                }

                result = TRUE;
            }
        }   // CheckIntIdValid()
        break;

    case PMIC_IOCTL_LLA_INT_DEREGISTER:
        DEBUGMSG(ZONE_FUNC,
                    (_T("PMI_IOControl PMIC_IOCTL_LLA_INT_DEREGISTER\r\n")));

        // "temp" is the IntId.
        temp = *pBufIn;

        if(CheckIntIdValid(temp))
        {
            if (phIntrHdlr[temp] != NULL)
            {
                CloseHandle(phIntrHdlr[temp]);
                phIntrHdlr[temp] = NULL;
            }
            result = TRUE;
        }
        break;

    case PMIC_IOCTL_LLA_INT_COMPLETE:
        DEBUGMSG(ZONE_FUNC,
                 (_T("PMI_IOControl PMIC_IOCTL_LLA_INT_COMPLETE\r\n")));
        // Intentional fall through no break needed

    case PMIC_IOCTL_LLA_INT_ENABLE:
        DEBUGMSG(ZONE_FUNC,
                 (_T("PMI_IOControl PMIC_IOCTL_LLA_INT_ENABLE\r\n")));

        result = PMICIoctlIntEnable(*pBufIn, TRUE);
        break;

    case PMIC_IOCTL_LLA_INT_DISABLE:
        DEBUGMSG(ZONE_FUNC,
                    (_T("PMI_IOControl PMIC_IOCTL_LLA_INT_DISABLE\r\n")));

        result = PMICIoctlIntEnable(*pBufIn, FALSE);
        break;

    case PMIC_IOCTL_ADC_TOUCH_READ:
        result = PMICIoctlAdcTouchRead(pBufIn, dwLenIn, pBufOut, dwLenOut);
        break;

    case PMIC_IOCTL_ADC_SET_MODE:
        result = PMICIoctlAdcSetMode(pBufIn, dwLenIn, pBufOut, dwLenOut);
        break;

    case PMIC_IOCTL_ADC_GET_HS_CURRENT:
        result = PmicIoctlADCGetHandsetCurrent(pBufIn, dwLenIn, pBufOut,
                                               dwLenOut);
        break;

    case PMIC_IOCTL_ADC_GET_MUL_CH_SPL:
        result = PmicIoctlADCGetMultipleChannelsSamples(pBufIn, dwLenIn,
                                                        pBufOut, dwLenOut);
        break;

    case PMIC_IOCTL_ADC_GET_SGL_CH_8SPL:
        result = PmicIoctlADCGetSingleChannelSample(pBufIn, dwLenIn, pBufOut,
                                                    dwLenOut, TRUE);
        break;

    case PMIC_IOCTL_ADC_GET_SGL_CH_1SPL:
        result = PmicIoctlADCGetSingleChannelSample(pBufIn, dwLenIn, pBufOut,
                                                    dwLenOut, FALSE);
        break;

    case PMIC_IOCTL_ADC_SET_CMPTR_TRHLD:
        result = PmicIoctlADCSetComparatorThresholds(pBufIn, dwLenIn, pBufOut,
                                                     dwLenOut);
        break;

    case PMIC_IOCTL_CONVT_OP:
        result = PmicIoctlConvityOpHandler((PMIC_PARAM_CONVITY_OP *)pBufIn,
                                           (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_RS232_OP:
        result = PmicIoctlConvityRs232OpHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_CEA936_OP:
        result = PmicIoctlConvityCea936OpHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_SETSPEED:
        result = PmicIoctlConvityUsbSetSpeedHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_SETPWR:
        result = PmicIoctlConvityUsbSetPowerSourceHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_SETXCVR:
        result = PmicIoctlConvityUsbSetXcvrHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_BGNHNP:
        result = PmicIoctlConvityUsbOtgBeginHnpHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_ENDHNP:
        result = PmicIoctlConvityUsbOtgEndHnpHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_SETCFG:
        result = PmicIoctlConvityUsbOtgSetConfigHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_CLRCFG:
        result = PmicIoctlConvityUsbOtgClearConfigHandler(
                     (PMIC_PARAM_CONVITY_OP *)pBufIn, (PMIC_STATUS *)pBufOut);
        break;

    case IOCTL_POWER_CAPABILITIES:
        // Tell the power manager about ourselves.
        if (pBufOut != NULL                        &&
            dwLenOut >= sizeof(POWER_CAPABILITIES) &&
            pdwActualOut != NULL)
        {
            __try
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                memset(ppc, 0, sizeof(*ppc));
                ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D4);
                *pdwActualOut = sizeof(*ppc);
                result = TRUE;
            }
            __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                ERRORMSG(TRUE, (_T("Exception in DVFC ")
                                _T("IOCTL_POWER_CAPABILITIES\r\n")));
            }
        }

        break;

    case IOCTL_POWER_SET:
        if(pBufOut != NULL                          &&
           dwLenOut == sizeof(CEDEVICE_POWER_STATE) &&
           pdwActualOut != NULL)
        {
            __try
            {
                CEDEVICE_POWER_STATE dx = *(PCEDEVICE_POWER_STATE) pBufOut;
                if(VALID_DX(dx)) 
                {
                    // Any request that is not D0 becomes a D4 request
                    if(dx != D0) 
                    {
                        dx = D4;
                    }

                    *(PCEDEVICE_POWER_STATE) pBufOut = dx;
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                    EnterCriticalSection(&adcCs);
                    EnterCriticalSection(&pmicCs);

                    // If we are turning off
                    if (dx == D4)
                    {
                        // Force polled CSPI communication to avoid
                        // synchronization issues with suspending or
                        // powering off.
                        cspiConfigPolling(TRUE);

                        // Set global flag to force subsequent ADC
                        // conversions into polled mode.
                        g_bAdcPolled = TRUE;

                        BSPPmicPowerNotifySuspend();
                    }

                    // Else we are powering on
                    else
                    {
                        // Restore previous CSPI transfer mode
                        cspiConfigPolling(g_bPmicUseCspiPolling);

                        // Set global flag to force subsequent ADC conversions
                        // into polled mode
                        g_bAdcPolled = FALSE;

                        BSPPmicPowerNotifyResume();
                    }

                    LeaveCriticalSection(&pmicCs);
                    LeaveCriticalSection(&adcCs);

                    result = TRUE;
                }
            } 
            __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                ERRORMSG(TRUE, (_T("Exception in DVFC IOCTL_POWER_SET\r\n")));
            }
        }
        break;

    case IOCTL_POWER_GET:
        if(pBufOut != NULL                          &&
           dwLenOut == sizeof(CEDEVICE_POWER_STATE) &&
           pdwActualOut != NULL)
        {
            // Just return our current Dx value
            __try
            {
                *(PCEDEVICE_POWER_STATE) pBufOut = g_dxCurrent;
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                result = TRUE;
            }
            __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                ERRORMSG(TRUE, (_T("Exception in DVFC IOCTL_POWER_SET\r\n")));
            }
        }
        break;

    default:
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Unrecognized IOCTL %d\r\n"),
                 __WFUNCTION__, dwCode));

        result = FALSE;
        break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s() returning %d\r\n"),
             __WFUNCTION__, result));

    return result;
}


//-----------------------------------------------------------------------------
//
// Function: PMI_PowerUp
//
// This function restores power to a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PMI_PowerUp(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER(hDeviceContext);
}


//-----------------------------------------------------------------------------
//
// Function: PMI_PowerDown
//
// This function suspends power to the device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PMI_PowerDown(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER(hDeviceContext);

    cspiPowerDown();
}


//-----------------------------------------------------------------------------
//
// Function: SetRegister
//
// This function sets a PMIC register.
//
// Parameters:
//      addr
//          [in] index of the register.
//      data
//          [in] the data to be set.
//      mask
//          [in] a bitmao indicates the valid bits in the data parameter.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID
SetRegister(UINT32 addr, UINT32 data, UINT32 mask)
{
    UINT32 content;

    // If it is not updating the whole register (24 bits), we need to read
    // the register first.  Then we set the bits and write it back.
    EnterCriticalSection(&pmicCs);
    if ((mask & PMIC_ALL_BITS) ^ PMIC_ALL_BITS)
    {
        GetRegister(addr, &content);
        content &= ~mask;
        content |= data & mask;
    }
    else
    {
        content = data;
    }

    cspiLock();

    cspiAddWritePacket(addr, content);
    cspiDataExchange();
    cspiDiscardData();

    cspiUnlock();
    LeaveCriticalSection(&pmicCs);
}


//-----------------------------------------------------------------------------
//
// Function: GetRegister
//
// This function retrives a PMIC register.
//
// Parameters:
//      addr
//          [in] index of the register.
//      content
//          [out] the content of the register.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID
GetRegister(UINT32 addr, UINT32* content)
{
    cspiLock();

    cspiAddReadPacket(addr);
    cspiDataExchange();
    cspiReceiveData(content);

    cspiUnlock();
}


//-----------------------------------------------------------------------------
//
// Function: PmicIsrThreadProc
//
// ISR Thread Process that launches IST loop.
//
// Parameters:
//      LPVOID lpParameter.
//
// Returns:
//      1 if success, 0 if failure.
//
//-----------------------------------------------------------------------------
static DWORD CALLBACK
PmicIsrThreadProc(LPVOID lpParameter)
{
    UNREFERENCED_PARAMETER(lpParameter);

    UINT32 isr0, imr0, active_int0, int_src;
    UINT32 isr1, imr1, active_int1;
    UINT32 pmicInt;

    // Set thread priority.  Make sure this priority is higher (lower value)
    // than any PMIC client driver (touch, battery, audio, etc.) so that the
    // interrupt handling is properly sequenced.  In addition, make sure this
    // priority lower (higher value) than the CspiIntrServThread since 
    // this thread will block on CSPI transfers.
    CeSetThreadPriority(GetCurrentThread(), 98);

    while(!bISTTerminate)
    {
        if (WaitForSingleObject(hIntrEventPmic, INFINITE) == WAIT_OBJECT_0)
        {
            // Keep processing interrupts PMIC interrupt request is deasserted
            do
            {
                EnterCriticalSection(&intRegCs);

                // Query the current interrupt status and mask
                GetRegister(MC13783_INT_STAT0_ADDR, &isr0);
                GetRegister(MC13783_INT_MSK0_ADDR, &imr0);

                GetRegister(MC13783_INT_STAT1_ADDR, &isr1);
                GetRegister(MC13783_INT_MSK1_ADDR, &imr1);

                DEBUGMSG(ZONE_FUNC,
                (_T("PMIC_INT:  stat0 = 0x%x, msk0 = 0x%x\r\n"), isr0, imr0));

                DEBUGMSG(ZONE_FUNC,
                (_T("PMIC_INT:  stat1 = 0x%x, msk1 = 0x%x\r\n"), isr1, imr1));

                // Active interrupts are logical AND of ISR, ~IMR, and INT_SEL
                active_int0 = isr0 & (~imr0) & PMIC_INTS0_TO_AP;
                active_int1 = isr1 & (~imr1) & PMIC_INTS1_TO_AP;

                // We will service all active interrupts, so clear the 
                // corresponding bits in the ISR and mask them until 
                // client drivers notify us it is okay to reenable them 
                // ISR is write-1-clear
                // IMR bit = 1 => masked
                SetRegister(MC13783_INT_MSK0_ADDR, 
                            imr0 | active_int0, PMIC_ALL_BITS);
                SetRegister(MC13783_INT_STAT0_ADDR, active_int0, PMIC_ALL_BITS);

                // Don't mask the TOD interrupt
                SetRegister(MC13783_INT_MSK1_ADDR, imr1 | active_int1, 
                            PMIC_ALL_BITS & (~(MC13783_TODAM_MASK)));

                //clear the interrupt 
                SetRegister(MC13783_INT_STAT1_ADDR, active_int1, PMIC_ALL_BITS);

                LeaveCriticalSection(&intRegCs);

                DEBUGMSG(ZONE_FUNC,
                (_T("PMIC_INT:  Active Interrupts 0 = 0x%x\r\n"), active_int0));

                DEBUGMSG(ZONE_FUNC,
                (_T("PMIC_INT:  Active Interrupts 1 = 0x%x\r\n"), active_int1));

                // Service all active interrupts
                while (active_int0 || active_int1)
                {
                    // Find the next pending interrupt
                    if (active_int1)
                    {
                        int_src = 31 - _CountLeadingZeros(active_int1) +
                                  PMIC_MC13783_INT_ID_OFFSET;
                    }
                    else
                    {
                        int_src = 31 - _CountLeadingZeros(active_int0);
                    }

                    DEBUGMSG(ZONE_FUNC, (_T("pending interrupts 0 =0x%x, ")
                                         _T("interrupt 1 =0x%x %d\r\n"),
                                         active_int0, active_int1, int_src));

                    // If interrupt source is valid
                    if (int_src < PMIC_INT_MAX_ID)
                    {
                        //  check for TOD alarm
                        if (int_src == PMIC_MC13783_INT_TODAI)
                        {
                            BSPPmicSignalOALRTC();
                        }

                        // If client event installed.
                        else if (phIntrHdlr[int_src] != NULL)
                        {
                            // Signal registered event.
                            // Note:  Rescheduling may occur and could result
                            // in more interrupts pending.
                            SetEvent(phIntrHdlr[int_src]);
                        }
                        else
                        {
                            ERRORMSG(TRUE, (_T("No PMIC event registered for ")
                                            _T("interrupt source = %d\r\n"),
                                     int_src));
                        }
                    }
                    else
                    {
                        ERRORMSG(TRUE,
                        (_T("Invalid PMIC interrupt source (%d)\r\n"),int_src));
                    }

                    // Service complete, move on to next pending source
                    if (active_int1)
                    {
                        active_int1 &=
                            (~(1U << (int_src - PMIC_MC13783_INT_ID_OFFSET)));
                    }
                    else
                    {
                        active_int0 &= (~(1U << int_src));
                    }
                }

                BSPPmicClearIrq();
                if (!BSPPmicGetIrqStatus(&pmicInt))
                {
                    ERRORMSG(TRUE, (_T("DDKGpioReadIntrPin failed!\r\n")));
                    pmicInt = 0;
                }
            } while (pmicInt);

            InterruptDone(dwSysIntrPmic);
        }
    }

    ExitThread(TRUE);

    return 1;
}
