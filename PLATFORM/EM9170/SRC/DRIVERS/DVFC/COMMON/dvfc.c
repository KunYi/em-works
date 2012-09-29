//-----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  dvfc.c
//
//  Provides BSP-specific configuration routines for the DVFS/DPTC driver.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include "bsp.h"
#include "dvfs.h"


//-----------------------------------------------------------------------------
// External Functions
extern BOOL DvfcInitVoltageSupplies(VOID);
extern UINT32 DvfcUpdateSupplyVoltage(UINT32 mV, UINT32 dvs, DDK_DVFC_DOMAIN domain);
extern PDDK_CLK_CONFIG DDKClockGetSharedConfig(VOID);
extern VOID DDKClockLock(VOID);
extern VOID DDKClockUnlock(VOID);

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define DVFC_VERBOSE            FALSE
#define DVFC_IST_PRIORITY       250

#define CSPDDK_LOCK()           DDKClockLock()
#define CSPDDK_UNLOCK()         DDKClockUnlock()


//-----------------------------------------------------------------------------
// Types
typedef enum
{
    // CPU setpoint parameters
    DVFC_PARM_ARM_SRC       = 0,
    DVFC_PARM_ARM_CLK_DIV   = 1,
    DVFC_PARM_AHB_CLK_DIV   = 2,

    DVFC_PARM_ENUM_END      = 3
} DVFC_SETPOINT_PARM;

typedef struct
{
    UINT32 parm[DVFC_PARM_ENUM_END];
    DDK_DVFC_SETPOINT_INFO setPointInfo;    
} DVFC_SETPOINT_CONFIG, *PDVFC_SETPOINT_CONFIG;   


//-----------------------------------------------------------------------------
// Global Variables
PCSP_CRM_REGS g_pCRM;
DWORD g_DvfcIrq = IRQ_CCM;


//-----------------------------------------------------------------------------
// Local Variables
static HANDLE g_hSetPointEvent;
static int g_IstPriority;
static HANDLE g_hDvfcWorkerThread;
static HANDLE g_hDvfcWorkerEvent;
static CEDEVICE_POWER_STATE g_dxCurrent[DDK_DVFC_DOMAIN_ENUM_END];
static DDK_DVFC_SETPOINT g_SetpointLoad[DDK_DVFC_DOMAIN_ENUM_END];

static DVFC_SETPOINT_CONFIG g_SetPointConfig[DDK_DVFC_DOMAIN_ENUM_END][DDK_DVFC_SETPOINT_ENUM_END] = 
{
    // CPU SETPOINTS
    {
        // HIGH SETPOINT
        {
            {
                BSP_DVFS_CPU_HIGH_ARM_SRC,
                BSP_DVFS_CPU_HIGH_ARM_CLK_DIV,
                BSP_DVFS_CPU_HIGH_AHB_CLK_DIV
            },
            {
                BSP_DVFS_CPU_HIGH_mV,
                {
                    BSP_DVFS_CPU_HIGH_ARM_FREQ,
                    BSP_DVFS_CPU_HIGH_AHB_FREQ,
                }
            }
        },

        // MEDIUM SETPOINT
        {
            {
                BSP_DVFS_CPU_MED_ARM_SRC,
                BSP_DVFS_CPU_MED_ARM_CLK_DIV,
                BSP_DVFS_CPU_MED_AHB_CLK_DIV
            },
            {
                BSP_DVFS_CPU_MED_mV,
                {
                    BSP_DVFS_CPU_MED_ARM_FREQ,
                    BSP_DVFS_CPU_MED_AHB_FREQ,
                }
            }
        },

        // LOW SETPOINT
        {
            {
                BSP_DVFS_CPU_LOW_ARM_SRC,
                BSP_DVFS_CPU_LOW_ARM_CLK_DIV,
                BSP_DVFS_CPU_LOW_AHB_CLK_DIV
            },
            {
                BSP_DVFS_CPU_LOW_mV,
                {
                    BSP_DVFS_CPU_LOW_ARM_FREQ,
                    BSP_DVFS_CPU_LOW_AHB_FREQ,
                }
            }
        }
        
    }
};


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcGetThreadPriority
//
//  This function provides the thread priority for the DVFC IST.
//
//  Parameters:
//      None.
//
//  Returns:
//      DVFC thread priority.
//
//-----------------------------------------------------------------------------
int BSPDvfcGetThreadPriority(void)
{
    return g_IstPriority;
}


//-----------------------------------------------------------------------------
//
//  Function:  DvfcMapDevPwrStateToSetpoint
//
//  This function maps the specified device power state into a DVFC setpoint.
//
//  Parameters:
//      dx
//          [in] Device power state to be mapped.
//
//      domain
//          [in] Specifies DVFC domain.
//
//  Returns:
//      DVFC setpoint associated with the device power state.
//
//-----------------------------------------------------------------------------
DDK_DVFC_SETPOINT DvfcMapDevPwrStateToSetpoint(CEDEVICE_POWER_STATE dx,
                                               DDK_DVFC_DOMAIN domain)
{
    DDK_DVFC_SETPOINT setpoint;
    
    switch(domain)
    {

    default:
        switch(dx)
        {
        case D0:
            setpoint = DDK_DVFC_SETPOINT_HIGH;
            break;

        case D1:
            setpoint = DDK_DVFC_SETPOINT_MEDIUM;
            break;

        case D2:
            setpoint = DDK_DVFC_SETPOINT_LOW;
            break;

        case D4:
            // Force HIGH setpoint for suspend/resume
            setpoint = DDK_DVFC_SETPOINT_HIGH;
            break;

        default:
            setpoint = DDK_DVFC_SETPOINT_LOW;
            break;
        }
        break;
    }

    return setpoint;
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcGetSupportedDx
//
//  This function returns the supported device states for the DVFC driver.
//
//  Parameters:
//      None.
//
//  Returns:
//      Bitmask indicating supported device power states.
//
//-----------------------------------------------------------------------------
UCHAR BSPDvfcGetSupportedDx(void)
{
    return (DX_MASK(D0) | DX_MASK(D1) | DX_MASK(D2) | DX_MASK(D4));
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcPowerGet
//
//  This function responds to a IOCTL_POWER_GET request from the WinCE 
//  Power Manager.
//
//  Parameters:
//      domainIndex
//          [in] DVFC domain index
//
//  Returns:
//      Device power state for the specified domain.
//
//-----------------------------------------------------------------------------
CEDEVICE_POWER_STATE BSPDvfcPowerGet(UINT32 domainIndex)
{
    DDK_DVFC_DOMAIN domain;
    
    if ((domainIndex > 0) && (domainIndex <= DDK_DVFC_DOMAIN_ENUM_END))
    {
        // Convert domain index into DDK_DVFC_DOMAIN enum
        domain = domainIndex - 1;
        return g_dxCurrent[domain];
    }
    else
    {
        return PwrDeviceUnspecified;
    }
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcPowerSet
//
//  This function responds to a IOCTL_POWER_SET request from the WinCE 
//  Power Manager.
//
//  Parameters:
//      domainIndex
//          [in] DVFC domain index
//
//      dx
//          [in] Requested device state.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPDvfcPowerSet(UINT32 domainIndex, CEDEVICE_POWER_STATE dx)
{
    DDK_DVFC_SETPOINT setpoint;
    DDK_DVFC_DOMAIN domain;

    if ((domainIndex == 0) || (domainIndex > DDK_DVFC_DOMAIN_ENUM_END))
    {
        return FALSE;
    }

    // Convert domain index into DDK_DVFC_DOMAIN enum
    domain = domainIndex -1;
    
    RETAILMSG(DVFC_VERBOSE, (_T("DOM%d = D%d\r\n"), domain, dx));

    if (g_dxCurrent[domain] != PwrDeviceUnspecified)
    {
        setpoint = DvfcMapDevPwrStateToSetpoint(g_dxCurrent[domain], domain);
        DDKClockSetpointRelease(setpoint);
    }
            
    if (dx != PwrDeviceUnspecified)
    {
        setpoint = DvfcMapDevPwrStateToSetpoint(dx, domain);
        // If we are suspending the system, block on the setpoint
        // change to ensure we will resume with the required
        // setpoint configuration.
        if (dx == D4)
        {
            DDKClockSetpointRequest(setpoint, TRUE);
        }
        else
        {
            DDKClockSetpointRequest(setpoint, FALSE);
        }
    }

    g_dxCurrent[domain] = dx;
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  DvfcUpdateSetpoint
//
//  This function updates the current frequency/voltage setpoint for 
//  the system.
//
//  Parameters:
//      pNewCfg
//          [in] Points to new frequency/voltage setpoint.
//
//      domain
//          [in] Specifies DVFC domain.
//
//      usDelay
//          [in] Specifies a delay in usec to wait for the voltage ramp
//               to occur for the new setpoint.
//
//  Returns:
//      Returns TRUE if the setpoint update was successful, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DvfcUpdateSetpoint(PDVFC_SETPOINT_CONFIG pNewCfg, 
                        DDK_DVFC_DOMAIN domain, UINT32 usDelay)
{
    LARGE_INTEGER liFreq, liStallCount, liStart, liEnd;
        
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(domain);

    // If voltage ramp delay is needed, poll performance counter until
    // ramp is complete
    if (usDelay)
    {
        QueryPerformanceCounter(&liStart);
        QueryPerformanceFrequency(&liFreq);
        liStallCount.QuadPart = ((liFreq.QuadPart * usDelay - 1) / 1000000) + 1;
        do {
            QueryPerformanceCounter(&liEnd);
        } while ( (liEnd.QuadPart - liStart.QuadPart) <= liStallCount.QuadPart);
    }

    // Scale the CPU frequency using CCTL
    INSREG32(&g_pCRM->CCTL, 
            CSP_BITFMASK(CRM_CCTL_ARM_SRC) 
            | CSP_BITFMASK(CRM_CCTL_ARMCLK_DIV) 
            | CSP_BITFMASK(CRM_CCTL_AHBCLK_DIV),
            CSP_BITFVAL(CRM_CCTL_ARM_SRC, pNewCfg->parm[DVFC_PARM_ARM_SRC])
            | CSP_BITFVAL(CRM_CCTL_ARMCLK_DIV, pNewCfg->parm[DVFC_PARM_ARM_CLK_DIV])
            | CSP_BITFVAL(CRM_CCTL_AHBCLK_DIV, pNewCfg->parm[DVFC_PARM_AHB_CLK_DIV]));


    return TRUE;
}


#if (DVFC_VERBOSE == TRUE)
//-----------------------------------------------------------------------------
//
//  Function:  DumpSetpoint
//
//  Dumps setpoint information.
//
//  Parameters:
//      domain
//          [in] Specifies DVFC domain.
//
//      setpoint
//          [in] Specifies current setpoint.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DumpSetpoint(DDK_DVFC_DOMAIN domain, DDK_DVFC_SETPOINT setpoint)
{
    switch (setpoint)
    {
    case DDK_DVFC_SETPOINT_HIGH:
        RETAILMSG(TRUE, (_T("DOM%d = HIGH\r\n"), domain));
        break;

    case DDK_DVFC_SETPOINT_MEDIUM:
        RETAILMSG(TRUE, (_T("DOM%d = MED\r\n"), domain));
        break;

    case DDK_DVFC_SETPOINT_LOW:
        RETAILMSG(TRUE, (_T("DOM%d = LOW\r\n"), domain));
        break;
    }
}
#endif


//-----------------------------------------------------------------------------
//
//  Function:  DvfcWorkerThread
//
//  This is the worker thread to respond to DVFC setpoint change requests.
//
//  Parameters:
//      lpParam
//          [in] Thread data passed to the function using the 
//          lpParameter parameter of the CreateThread function. Not used.
//
//  Returns:
//      Returns thread exit code.
//
//-----------------------------------------------------------------------------
static DWORD WINAPI DvfcWorkerThread (LPVOID lpParam)
{
    DWORD rc = TRUE;
    DDK_DVFC_SETPOINT setpointCur;
    DDK_DVFC_SETPOINT setpointReq;
    DDK_DVFC_SETPOINT setpointLoad;
    DDK_DVFC_SETPOINT setpointMin;
    DDK_DVFC_SETPOINT setpointMax;
    DDK_DVFC_DOMAIN domain;
    BOOL bSignalDdkClk, bLowerVoltage, bUnlock;
    UINT32 usDelay;
    UINT32 mVoltCur, mVoltReq;
    PDDK_CLK_CONFIG pDdkClkConfig = DDKClockGetSharedConfig();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParam);
    
    CeSetThreadPriority(GetCurrentThread(), g_IstPriority);

    for (;;)
    {
        if(WaitForSingleObject(g_hDvfcWorkerEvent, INFINITE) == WAIT_OBJECT_0)
        {
            for (domain = 0; domain < DDK_DVFC_DOMAIN_ENUM_END; domain++)
            {            
                // Reset state variables
                bSignalDdkClk = FALSE;
                bLowerVoltage = FALSE;
                bUnlock = FALSE;
                setpointCur = pDdkClkConfig->setpointCur[domain];
                setpointMin = pDdkClkConfig->setpointMin[domain];
                setpointMax = pDdkClkConfig->setpointMax[domain];
                setpointLoad = pDdkClkConfig->setpointLoad[domain];
                pDdkClkConfig->bSetpointPending = TRUE;
                
                // DDKClockSetCKO(TRUE, DDK_CLOCK_CKO_SRC_UNGATED_ARM_CLK, 9);
    
                // Evaluate the required setpoint by inspecting the globally shared 
                // reference count for each setpoint
                for (setpointReq = setpointMax; setpointReq < setpointMin; setpointReq++)
                {
                    if (pDdkClkConfig->setpointReqCount[domain][setpointReq] != 0)
                    {
                        break;
                    }
                }

                // If load tracking request results in higher setpoint,
                // override the request from CSPDDK
                if (setpointLoad < setpointReq)
                {
                    setpointReq = setpointLoad;
                }

                // Check if we are already at the required setpoint
                if (setpointCur == setpointReq)
                {
                    bSignalDdkClk = TRUE;
                    goto cleanUp;
                }

                // Assume no delay is needed during setpoint transition
                usDelay = 0;
                    
                // If new setpoint requires higher voltage, submit request to PMIC.  Note
                // that we do this before grabbing the CSPDDK lock since the PMIC may need
                // to enable clocks using the CSPDDK.
                mVoltCur = g_SetPointConfig[domain][setpointCur].setPointInfo.mV;
                mVoltReq = g_SetPointConfig[domain][setpointReq].setPointInfo.mV;
                if (mVoltReq > mVoltCur)
                {
                    usDelay = DvfcUpdateSupplyVoltage(mVoltReq, setpointReq, domain);

                    // Check if voltage update failed
                    if (usDelay == INFINITE) 
                    {
                        goto cleanUp;
                    }
                }
                // Else new setpoint needs lower voltage
                else if (mVoltReq < mVoltCur)
                {
                    // Set flag to lower voltage when we are done
                    bLowerVoltage = TRUE;
                }

                // Grab CSPDDK lock while updating the setpoint
                CSPDDK_LOCK();
                bUnlock = TRUE;
                if (!DvfcUpdateSetpoint(&g_SetPointConfig[domain][setpointReq],
                                        domain, usDelay))
                {
                    // We encountered an error, set flag to lower the voltage
                    bLowerVoltage = TRUE;
                    goto cleanUp;
                }
                setpointCur = setpointReq;
                bSignalDdkClk = TRUE;
#if (DVFC_VERBOSE == TRUE)
                DumpSetpoint(domain, setpointCur);
#endif

cleanUp:
                // Update current setpoint to reflect successful transitions
                pDdkClkConfig->setpointCur[domain] = setpointCur;
                pDdkClkConfig->bSetpointPending = FALSE;

                // If we acquired CSPDDK lock, unlock it now
                if (bUnlock)
                {
                    CSPDDK_UNLOCK();
                }

                // If flag was set to lower the voltage
                if (bLowerVoltage)
                {
                    DvfcUpdateSupplyVoltage(g_SetPointConfig[domain][setpointCur].setPointInfo.mV, 
                                            setpointCur, domain);
                }            
                
                // Signal drivers blocked on CSPDDK that are waiting for setpoint transition
                if (bSignalDdkClk)
                {
                    SetEvent(g_hSetPointEvent);
                }
        
            }
        }
        else 
        {
            // Abnormal signal
            rc = FALSE;
            break;
        }
    }

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcIntrServ
//
//  This function is invoked from the DVFC interrupt service thread to
//  perform the frequency/voltage switch.    
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void BSPDvfcIntrServ(void)
{
    // Signal the DVFC driver about our request
    SetEvent(g_hDvfcWorkerEvent);
    
    // Clear the signal from the OAL
    KernelIoControl(IOCTL_HAL_UNFORCE_IRQ, &g_DvfcIrq, sizeof(UINT32), NULL, 0, NULL);
}


//-----------------------------------------------------------------------------
//
//  Function: BSPDvfcInit
//
//  This function provides platform-specific initialization for supporting 
//  DVFS/DPTC.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPDvfcInit(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;
    PDDK_CLK_CONFIG pDdkClkConfig = DDKClockGetSharedConfig();
    DDK_DVFC_DOMAIN domain;

    // Map CRM
    phyAddr.QuadPart = CSP_BASE_REG_PA_CRM;
    g_pCRM = (PCSP_CRM_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_CRM_REGS),
                                          FALSE);
    // Check if virtual mapping failed
    if (g_pCRM == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    // Create event to sync with DVFC setpoint transitions
    g_hSetPointEvent = CreateEvent(NULL, TRUE, FALSE, L"EVENT_SETPOINT");

    if (g_hSetPointEvent == NULL)
    {
        ERRORMSG(TRUE, (_T("CreateEvent failed!\r\n")));
        goto cleanUp;
    }

    // Supply default DVFC configuration
    g_IstPriority = DVFC_IST_PRIORITY;

    // Create event to signal DVFC worker thread
    g_hDvfcWorkerEvent = CreateEvent(NULL, FALSE, FALSE, L"EVENT_DVFC_WORKER");

    if (g_hDvfcWorkerEvent == NULL)
    {
        ERRORMSG(TRUE, (_T("CreateEvent failed!\r\n")));
        goto cleanUp;
    }

    // Create worker thread for DVFC setpoint requests
    g_hDvfcWorkerThread = CreateThread(NULL, 0, DvfcWorkerThread, NULL, 0, NULL);      
    if (!g_hDvfcWorkerThread) 
    {
        ERRORMSG(TRUE, (_T("CreateThread failed for DVFC worker thread!\r\n")));
        goto cleanUp;
    }

    // Initialize PMIC voltage supplies
    if (!DvfcInitVoltageSupplies())
    {
        ERRORMSG(TRUE, (_T("DvfcInitVoltageSupplies failed!\r\n")));
        goto cleanUp;
    }

    for (domain = 0; domain < DDK_DVFC_DOMAIN_ENUM_END; domain++)
    {
        g_dxCurrent[domain] = PwrDeviceUnspecified;
    
        // Set DVFC device power state to D0
        if (!BSPDvfcPowerSet(domain+1, D0))
        {
            ERRORMSG(TRUE, (_T("BSPDvfcPowerSet failed!\r\n")));
            goto cleanUp;
        }

        g_dxCurrent[domain] = D0;
    }
    
    rc = TRUE;

cleanUp:

    // If initialization succeeded, report that the DVFC is active
    if (pDdkClkConfig)
    {
        pDdkClkConfig->bDvfcActive = rc;
    }
    
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcDeinit
//
//  This function deinitializes the platform-specific DVFS/DPTC support 
//  established by BSPDvfcInit.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPDvfcDeinit(void)
{
    return TRUE;    
}


