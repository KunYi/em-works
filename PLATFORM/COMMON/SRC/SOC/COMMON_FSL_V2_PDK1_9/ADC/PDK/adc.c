//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// adc.c
//
// SOC layer for ADC driver. 
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_types.h"
#include "common_macros.h"
#include "common_adc.h"
#include "adc.h"
#include "adc_ioctl.h"

#define CONTROLLER_MAX_FREQ 175000
#define PEN_DEBOUNCE_PERIOD 63


//-----------------------------------------------------------------------------
// External functions
extern BOOL BSPADCCIomuxConfig();
extern BOOL BSPADCEnableClock(BOOL fEnable);
extern UINT32 BSPADCGetIPGClock();
extern DWORD BSPADCGetHSYNCPolarity();
extern DWORD BSPADCGetHSYNCEnable();
extern DWORD CspADCTouchGetBaseRegAddr();


//-----------------------------------------------------------------------------
//
// Function: ADC_Deinit
//
// This function de-initializes the device.
//
// Parameters:
//      hDeviceContext 
//          [in] Handle to the device context. The XXX_Init function
//          creates and returns this identifier. 
//
// Returns:
//      TRUE when all is good (ie. every Opened instance was closed)
//      FALSE when all is bad  (ie. at least one instance is still open)
//
//-----------------------------------------------------------------------------
BOOL ADC_Deinit(DWORD hDeviceContext)
{
    T_DEVICE_CONTEXT* pDevCtxt = (T_DEVICE_CONTEXT*) hDeviceContext;

    if (pDevCtxt)
    {   
        if (pDevCtxt->pADCRegs)
        {
            MmUnmapIoSpace(pDevCtxt->pADCRegs,sizeof(CSP_ADC_REGS));
        }
        DeleteCriticalSection(&pDevCtxt->cs);
        LocalFree(pDevCtxt);
    }
    return TRUE;
}
    
//-----------------------------------------------------------------------------
//
// Function: ADC_Init
//
// This function initializes the structure required to use the ADC 
// controler and initalizes the hardware.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
//      dwBusContext
//          [in] Potentially process-mapped pointer passed as the fourth 
//          parameter to ActivateDeviceEx. If this driver was loaded through legacy 
//          mechanisms, then lpvBusContext is zero. This pointer, if used, has only
//          been remapped as it passes through the protected server library (PSL). 
//          The XXX_Init function is responsible for performing all protection 
//          checking. In addition, any pointers referenced through lpvBusContext 
//          must be remapped with the MapCallerPtr function before they can be 
//          dereferenced.
//
// Returns:
//          Returns a handle to the device context created if successful. 
//          Returns zero if not successful. This handle is passed to 
//          the XXX_Open, XXX_PowerDown, XXX_PowerUp, and XXX_Deinit functions.
//
//-----------------------------------------------------------------------------
DWORD ADC_Init(LPCTSTR pContext, DWORD dwBusContext)
{
    T_DEVICE_CONTEXT* pDevCtxt; 
    PHYSICAL_ADDRESS pa;
    UINT32 u32IpgClk;
    DWORD dwDivider;

    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(dwBusContext);

    pDevCtxt = LocalAlloc(LMEM_ZEROINIT|LMEM_FIXED, sizeof(T_DEVICE_CONTEXT));
    if (!pDevCtxt)
    {
        goto error;             
    }

    pa.QuadPart = (ULONGLONG) CspADCTouchGetBaseRegAddr();
    pDevCtxt->pADCRegs = (PCSP_ADC_REGS) MmMapIoSpace(pa,sizeof(CSP_ADC_REGS),FALSE);
    if (!pDevCtxt->pADCRegs)
    {
        goto error;
    }

    InitializeCriticalSection(&pDevCtxt->cs);

    pDevCtxt->fEnableWakeUp = FALSE;
 
    BSPADCCIomuxConfig();
    BSPADCEnableClock(TRUE);
    

    // enable the module (clock it)
    OUTREG32(&(pDevCtxt->pADCRegs->TGCR),CSP_BITFVAL(ADC_TGCR_IPG_CLK_EN    , ADC_TGCR_IPG_ON));
    
    // Reset the Touchscreen module
    OUTREG32(&(pDevCtxt->pADCRegs->TGCR),
        CSP_BITFVAL(ADC_TGCR_IPG_CLK_EN , ADC_TGCR_IPG_ON) |
        CSP_BITFVAL(ADC_TGCR_TSC_RST    , ADC_TGCR_TSC_RESET));
    do
    {
        Sleep(1);
    } while (EXTREG32BF(&(pDevCtxt->pADCRegs->TGCR),ADC_TGCR_TSC_RST));

    // Reset the touchscreen function
    OUTREG32(&(pDevCtxt->pADCRegs->TGCR),
        CSP_BITFVAL(ADC_TGCR_IPG_CLK_EN , ADC_TGCR_IPG_ON) |
        CSP_BITFVAL(ADC_TGCR_FUNC_RST, ADC_TGCR_FUNC_RESET));
    do
    {
        Sleep(1);
    } while (EXTREG32BF(&(pDevCtxt->pADCRegs->TGCR),ADC_TGCR_FUNC_RST));


    // Configure the ADC/Touchscreen controller with default settings
    u32IpgClk = BSPADCGetIPGClock();
    if (u32IpgClk == 0)
    {
        ERRORMSG(1,(TEXT("ADC : Invalid IPG Clk\r\n")));
        goto error;
    }
    dwDivider = u32IpgClk / CONTROLLER_MAX_FREQ;
    if (dwDivider > 64)
    {
        dwDivider = 64;
    }
    else if (dwDivider < 10)
    {
        dwDivider = 10;
    }
    
    dwDivider = (dwDivider-2)/2;
    
    OUTREG32(&(pDevCtxt->pADCRegs->TGCR),
        CSP_BITFVAL(ADC_TGCR_IPG_CLK_EN , ADC_TGCR_IPG_ON) |
        CSP_BITFVAL(ADC_TGCR_TSC_RST    ,0) |
        CSP_BITFVAL(ADC_TGCR_FUNC_RST   ,0) |
        CSP_BITFVAL(ADC_TGCR_SLPC       ,0) | // Logic will work in normal mode (not in sleep mode)
        CSP_BITFVAL(ADC_TGCR_STLC       ,ADC_TGCR_SETTLING_NOT_LACTHED) |
        CSP_BITFVAL(ADC_TGCR_HSYNCEN    ,BSPADCGetHSYNCEnable()  ) |
        CSP_BITFVAL(ADC_TGCR_HSYNCPOL   ,BSPADCGetHSYNCPolarity()) |
        CSP_BITFVAL(ADC_TGCR_POWERMODE  ,ADC_TGCR_POWER_ALWAYS_ON) |
        CSP_BITFVAL(ADC_TGCR_INTREFEN   ,ADC_TGCR_INTREFEN_ON) |
        CSP_BITFVAL(ADC_TGCR_ADCCLKCFG  ,dwDivider) |
        CSP_BITFVAL(ADC_TGCR_PDEN       ,ADC_TGCR_PDEN_ENABLE) |
        CSP_BITFVAL(ADC_TGCR_PDBEN      ,ADC_TGCR_PDBEN_ENABLE) |
        CSP_BITFVAL(ADC_TGCR_PDBTIME    ,PEN_DEBOUNCE_PERIOD)       
        );

    return (DWORD) pDevCtxt;

error:
    ADC_Deinit((DWORD)pDevCtxt);
    return (DWORD) NULL;
}

//-----------------------------------------------------------------------------
// Function: ADC_Open
//
// An application indirectly invokes this function when it calls the 
// CreateFile function to open special device file names.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function
//          creates and returns this handle.
//
//      AccessCode
//          [in] Not Used
//
//      ShareMode
//          [in]Not Used
//
// Returns:
//      This function returns a handle that identifies the open context 
//      of the device to the calling application. If your device can be 
//      opened multiple times, use this handle to identify each open context.
//      This identifier is passed into the XXX_Read, XXX_Write, XXX_Seek, 
//      and XXX_IOControl functions. Returns zero if the device cannot be 
//      opened.
//
//-----------------------------------------------------------------------------
DWORD ADC_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    T_OPEN_CONTEXT *pOpenCtxt;
    T_DEVICE_CONTEXT* pDevCtxt = (T_DEVICE_CONTEXT*) hDeviceContext;

    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    pOpenCtxt = LocalAlloc(LMEM_ZEROINIT|LMEM_FIXED, sizeof(T_OPEN_CONTEXT));
    if (pOpenCtxt)
    {
        pOpenCtxt->pDevCtxt = pDevCtxt;
    }

    return (DWORD) pOpenCtxt;
}


//-----------------------------------------------------------------------------
//
// Function: ADC_Close
//
// This function closes a device context created by the hOpenContext parameter.
//
// Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to 
//          identify the open context of the device.
//
// Returns:
//      TRUE indicates success.
//      FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL ADC_Close(DWORD hOpenContext)
{
    T_OPEN_CONTEXT* pOpenCtxt = (T_OPEN_CONTEXT*) hOpenContext;

    if (pOpenCtxt)
    {
        LocalFree(pOpenCtxt);
    }
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: ADC_IOControl
//
// This function sends a command to a device.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier. 
//
//      dwCode
//          [in] I/O control operation to perform. These codes are 
//          device-specific and are usually exposed to developers through a 
//          header file. 
//
//      pBufIn
//          [in] Pointer to the buffer containing data to transfer to the device. 
//
//      dwLenIn
//          [in] Number of bytes of data in the buffer specified for pBufIn. 
//
//      pBufOut
//          [in/out] Pointer to the buffer used to transfer the output data from 
//          the device. 
//
//      dwLenOut
//          [in/out] Maximum number of bytes in the buffer specified by pBufOut. 
//
//      pdwActualOut
//          [out] Pointer to the DWORD buffer that this function uses to 
//          return the actual number of bytes received from the device. 
//
// Returns:
//      TRUE indicates success.
//      FALSE indicates failure.
//-----------------------------------------------------------------------------
BOOL ADC_IOControl( DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    T_OPEN_CONTEXT* pOpenCtxt = (T_OPEN_CONTEXT*) hOpenContext;
    BOOL bRet = TRUE;

    switch (dwCode)
    {
    case IOCTL_GENERAL_CONFIG:
        if ((pBufIn == NULL) || (dwLenIn != sizeof(T_IOCTL_GENERAL_CONFIG_PARAM)))
        {
            bRet = FALSE;
        }
        break;
    case IOCTL_CFG_ITEM:
        if ((pBufIn == NULL) || (dwLenIn != sizeof(T_IOCTL_CFG_ITEM_PARAM)))
        {
            bRet = FALSE;
        }
        break;
    case IOCTL_CFG_QUEUE:
        if ((pBufIn == NULL) || (dwLenIn != sizeof(T_IOCTL_CFG_QUEUE_PARAM)))
        {
            bRet = FALSE;
        }
        break;
    case IOCTL_START_ACQUIRE_SNGL:
        if ((pBufIn == NULL) || (dwLenIn != sizeof(T_IOCTL_START_ACQUIRE_SNGL_PARAM)))
        {
            bRet = FALSE;
        }
        break;
    case IOCTL_STOP_ACQUIRE_SNGL:
        if ((pBufIn == NULL) || (dwLenIn != sizeof(T_IOCTL_STOP_ACQUIRE_SNGL_PARAM)))
        {
            bRet = FALSE;
        }
        break;
    case IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS:
        if ((pBufIn == NULL) || (dwLenIn != sizeof(T_IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS_PARAM)))
        {
            bRet = FALSE;
        }
        break;
    case IOCTL_WAKEUP_SOURCE:
        if ((pBufIn == NULL) || (dwLenIn != sizeof(T_IOCTL_WAKEUP_SOURCE_PARAM)))
        {
            bRet = FALSE;
        }
        break;
    }



    if (bRet == FALSE)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    else    
    {

        switch(dwCode)
        {
        case IOCTL_GENERAL_CONFIG:
            ERRORMSG(1,(TEXT("not implemented yet\r\n")));
            bRet = FALSE;
            break;
        case IOCTL_CFG_ITEM:
            {
                T_IOCTL_CFG_ITEM_PARAM* pParam = NULL;
                
                EnterCriticalSection(&pOpenCtxt->pDevCtxt->cs);

                pParam = (T_IOCTL_CFG_ITEM_PARAM*) pBufIn;

                if (pParam->eItemID == TICR)
                {
                    OUTREG32(&pOpenCtxt->pDevCtxt->pADCRegs->TICR,pParam->dwItemConfig);
                }
                else if ((pParam->eItemID >= TCC0) && (pParam->eItemID <= TCC7))
                {
                    OUTREG32(&pOpenCtxt->pDevCtxt->pADCRegs->TCC_REGS.TCC[pParam->eItemID - TCC0],pParam->dwItemConfig);
                }
                else if ((pParam->eItemID >= GCC0) && (pParam->eItemID <= GCC7))
                {
                    OUTREG32(&pOpenCtxt->pDevCtxt->pADCRegs->GCC_REGS.GCC[pParam->eItemID - GCC0],pParam->dwItemConfig);
                }
                else
                {
                    bRet = FALSE;                   
                }   
                LeaveCriticalSection(&pOpenCtxt->pDevCtxt->cs);
            }
            break;
        case IOCTL_CFG_QUEUE:
            {
                T_ADC_QUEUE* pQueue = NULL;
                T_IOCTL_CFG_QUEUE_PARAM* pParam = NULL;

                EnterCriticalSection(&pOpenCtxt->pDevCtxt->cs);

                pParam = (T_IOCTL_CFG_QUEUE_PARAM*) pBufIn;

                if (pParam->eQueueID == GENERAL_QUEUE)
                {
                    pQueue = &(pOpenCtxt->pDevCtxt->pADCRegs->GC_QUEUE_REGS.GC_QUEUE);
                }
                else if (pParam->eQueueID == TOUCH_QUEUE)
                {
                    pQueue = &(pOpenCtxt->pDevCtxt->pADCRegs->TC_QUEUE_REGS.TC_QUEUE);
                }
                else
                {
                    bRet = FALSE;
                }

                if (pQueue)
                {
                    // Reset the FIFO
                    INSREG32BF(&pQueue->QCR,ADC_TCQCR_FRST,ADC_TCQCR_FRST_FRESET);
                    INSREG32BF(&pQueue->QCR,ADC_TCQCR_FRST,0);
                    // Reset the Queue
                    INSREG32BF(&pQueue->QCR,ADC_TCQCR_QRST,ADC_TCQCR_QRST_QRESET);
                    INSREG32BF(&pQueue->QCR,ADC_TCQCR_QRST,0);

                    pQueue->QSR = pQueue->QSR;
                    pQueue->Q_ITEM_7_0 = pParam->dwItem_7_0;
                    pQueue->Q_ITEM_15_8 = pParam->dwItem_15_8;
                    pQueue->QCR = pParam->dwQConfigRreg;
                    pQueue->QMR = pParam->dwQIntConfig;
                }
                LeaveCriticalSection(&pOpenCtxt->pDevCtxt->cs);
            }
            break;
        case IOCTL_START_ACQUIRE_SNGL:
            {
                T_IOCTL_START_ACQUIRE_SNGL_PARAM* pParam = NULL;

                EnterCriticalSection(&pOpenCtxt->pDevCtxt->cs);

                pParam = (T_IOCTL_START_ACQUIRE_SNGL_PARAM*) pBufIn;

                if (pParam->eQueueID == GENERAL_QUEUE)
                {
                    INSREG32(&pOpenCtxt->pDevCtxt->pADCRegs->GC_QUEUE_REGS.GC_QUEUE.QCR,
                             CSP_BITFMASK(ADC_GCQCR_QSM),
                             CSP_BITFVAL(ADC_GCQCR_QSM, ADC_GCQCR_QSM_FQS));                    

                    INSREG32(&pOpenCtxt->pDevCtxt->pADCRegs->GC_QUEUE_REGS.GC_QUEUE.QCR,
                             CSP_BITFMASK(ADC_GCQCR_RPT),
                             CSP_BITFVAL(ADC_GCQCR_RPT, ADC_GCQCR_RPT_REPEAT));    

                    INSREG32(&pOpenCtxt->pDevCtxt->pADCRegs->GC_QUEUE_REGS.GC_QUEUE.QCR,
                             CSP_BITFMASK(ADC_GCQCR_FQS),
                             CSP_BITFVAL(ADC_GCQCR_FQS, ADC_GCQCR_FQS_START));
                }
                else if (pParam->eQueueID == TOUCH_QUEUE)
                {
                    INSREG32(&pOpenCtxt->pDevCtxt->pADCRegs->TC_QUEUE_REGS.TC_QUEUE.QCR,
                             CSP_BITFMASK(ADC_TCQCR_QSM),
                             CSP_BITFVAL(ADC_TCQCR_QSM, ADC_TCQCR_QSM_FQS));                    

                    INSREG32(&pOpenCtxt->pDevCtxt->pADCRegs->TC_QUEUE_REGS.TC_QUEUE.QCR,
                             CSP_BITFMASK(ADC_TCQCR_RPT),
                             CSP_BITFVAL(ADC_TCQCR_RPT, ADC_TCQCR_RPT_REPEAT));    

                    INSREG32(&pOpenCtxt->pDevCtxt->pADCRegs->TC_QUEUE_REGS.TC_QUEUE.QCR,
                             CSP_BITFMASK(ADC_TCQCR_FQS),
                             CSP_BITFVAL(ADC_TCQCR_FQS, ADC_TCQCR_FQS_START));
                }
                else
                {
                    bRet = FALSE;
                }
                LeaveCriticalSection(&pOpenCtxt->pDevCtxt->cs);
            }
            break;
        case IOCTL_STOP_ACQUIRE_SNGL:
            {
                T_IOCTL_STOP_ACQUIRE_SNGL_PARAM* pParam = NULL;

                EnterCriticalSection(&pOpenCtxt->pDevCtxt->cs);

                pParam = (T_IOCTL_STOP_ACQUIRE_SNGL_PARAM*) pBufIn;
                if (pParam->eQueueID == GENERAL_QUEUE)
                {
                    CLRREG32(&pOpenCtxt->pDevCtxt->pADCRegs->GC_QUEUE_REGS.GC_QUEUE.QCR,
                             CSP_BITFVAL(ADC_GCQCR_FQS, ADC_GCQCR_FQS_START));

                    CLRREG32(&pOpenCtxt->pDevCtxt->pADCRegs->GC_QUEUE_REGS.GC_QUEUE.QCR,
                             CSP_BITFVAL(ADC_GCQCR_RPT, ADC_GCQCR_RPT_REPEAT));

                    INSREG32(&pOpenCtxt->pDevCtxt->pADCRegs->GC_QUEUE_REGS.GC_QUEUE.QCR,
                             CSP_BITFMASK(ADC_GCQCR_QSM),
                             CSP_BITFVAL(ADC_GCQCR_QSM, ADC_GCQCR_QSM_STOP));
                }
                else if (pParam->eQueueID == TOUCH_QUEUE)
                {
                    CLRREG32(&pOpenCtxt->pDevCtxt->pADCRegs->TC_QUEUE_REGS.TC_QUEUE.QCR,
                             CSP_BITFVAL(ADC_TCQCR_FQS, ADC_TCQCR_FQS_START));

                    CLRREG32(&pOpenCtxt->pDevCtxt->pADCRegs->TC_QUEUE_REGS.TC_QUEUE.QCR,
                             CSP_BITFVAL(ADC_TCQCR_RPT, ADC_TCQCR_RPT_REPEAT));

                    INSREG32(&pOpenCtxt->pDevCtxt->pADCRegs->TC_QUEUE_REGS.TC_QUEUE.QCR,
                             CSP_BITFMASK(ADC_TCQCR_QSM),
                             CSP_BITFVAL(ADC_TCQCR_QSM, ADC_TCQCR_QSM_STOP));
                }
                else
                {
                    bRet = FALSE;
                }
                LeaveCriticalSection(&pOpenCtxt->pDevCtxt->cs);
            }
            break;
        case IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS:
            {
                DWORD dwActualOut = 0;
                DWORD dwRemainingSample = 0;
                UINT16* pBufOut16 = NULL; 
                T_ADC_QUEUE* pQueue = NULL;
                T_IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS_PARAM* pParam = NULL;
                
                EnterCriticalSection(&pOpenCtxt->pDevCtxt->cs);

                dwRemainingSample = (dwLenOut/sizeof(UINT16));
                pBufOut16 = (UINT16*) pBufOut; 
                pParam = (T_IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS_PARAM*) pBufIn;

                if (pParam->eQueueID == GENERAL_QUEUE)
                {
                    pQueue = &(pOpenCtxt->pDevCtxt->pADCRegs->GC_QUEUE_REGS.GC_QUEUE);
                }
                else if (pParam->eQueueID == TOUCH_QUEUE)
                {
                    pQueue = &(pOpenCtxt->pDevCtxt->pADCRegs->TC_QUEUE_REGS.TC_QUEUE);
                }
                else
                {
                    bRet = FALSE;
                }
                           
                if (pBufOut16 && pQueue)
                {                    
                    DWORD dwStartDate = GetTickCount();
                 
                    while ((!EXTREG32BF(&pQueue->QSR,ADC_TCQSR_FDRY)) && ((int) (GetTickCount()-dwStartDate) < pParam->dwTimeout))
                    {
                        Sleep(1);
                    }
                    
                
                    if (!EXTREG32BF(&pQueue->QSR,ADC_TCQSR_FDRY))
                    {
                        // Reset the FIFO
                        INSREG32BF(&pQueue->QCR,ADC_TCQCR_FRST,ADC_TCQCR_FRST_FRESET);
                        Sleep(1);
                        INSREG32BF(&pQueue->QCR,ADC_TCQCR_FRST,0);
                        bRet = FALSE;
                    }
                    else
                    {
                        while ((!EXTREG32BF(&pQueue->QSR,ADC_TCQSR_EMPT)) && dwRemainingSample)
                        {
                            *pBufOut16 = (UINT16) INREG32(&pQueue->QFIFO);
                            dwRemainingSample--;
                            dwActualOut+=sizeof(UINT16);
                            pBufOut16++;
                        }
                    }
                    
                }
                if (pdwActualOut)
                {
                    *pdwActualOut = dwActualOut;
                }
                if (pQueue)
                {
                    OUTREG32(&pQueue->QSR,INREG32(&pQueue->QSR));
                }
                LeaveCriticalSection(&pOpenCtxt->pDevCtxt->cs);
            }
            break;
        case IOCTL_WAKEUP_SOURCE:
            {
                T_IOCTL_WAKEUP_SOURCE_PARAM* pParam = (T_IOCTL_WAKEUP_SOURCE_PARAM*) pBufIn;
                pOpenCtxt->pDevCtxt->fEnableWakeUp = pParam->fEnableWakeUp;
            }
            break;
        case IOCTL_POWER_CAPABILITIES:
            // Tell the power manager about ourselves.
            if (pBufOut != NULL                        &&
                dwLenOut >= sizeof(POWER_CAPABILITIES) &&
                pdwActualOut != NULL)
            {
               PREFAST_SUPPRESS(6320, "Generic exception handler");
                __try
                {
                    PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                    memset(ppc, 0, sizeof(*ppc));
                    ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D4);
                    *pdwActualOut = sizeof(*ppc);
                    bRet = TRUE;
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
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
               PREFAST_SUPPRESS(6320, "Generic exception handler");
                __try
                {
                    CEDEVICE_POWER_STATE dx = *(PCEDEVICE_POWER_STATE) pBufOut;
                    if(VALID_DX(dx)) 
                    {
                        // Any request that is not D0 becomes a D4 request
                        if(dx != D0) 
                        {
                            dx = D4;

                            if (!pOpenCtxt->pDevCtxt->fEnableWakeUp) //Turn off the internal reference and the clocks only if the ADC isn't used as a wakeup source
                            {
                                // Turn off internal reference
                                INSREG32BF(&(pOpenCtxt->pDevCtxt->pADCRegs->TGCR),ADC_TGCR_INTREFEN,ADC_TGCR_INTREFEN_OFF);
                                // Disable the controller
                                INSREG32BF(&(pOpenCtxt->pDevCtxt->pADCRegs->TGCR),ADC_TGCR_IPG_CLK_EN,ADC_TGCR_IPG_OFF);                                                
                                // Disable the ADC clock
                                BSPADCEnableClock(FALSE);
                            } 
                            else
                            {
                                //TSC interrurpt logic will work in sleep mode
                                INSREG32BF(&(pOpenCtxt->pDevCtxt->pADCRegs->TGCR),ADC_TGCR_SLPC,1);
                            }
                        }

                        *(PCEDEVICE_POWER_STATE) pBufOut = dx;
                        *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                       
                        if (dx == D0)
                        {
                            if (!pOpenCtxt->pDevCtxt->fEnableWakeUp)
                            {
                                // Enable the ADC clock
                                BSPADCEnableClock(TRUE);
                                //Enable the controller
                                INSREG32BF(&(pOpenCtxt->pDevCtxt->pADCRegs->TGCR),ADC_TGCR_IPG_CLK_EN,ADC_TGCR_IPG_ON);
                                // turn on the internal reference
                                INSREG32BF(&(pOpenCtxt->pDevCtxt->pADCRegs->TGCR),ADC_TGCR_INTREFEN,ADC_TGCR_INTREFEN_ON);           
                            } 
                            else
                            {
                                //TSC interrupt logic will work in normal mode 
                                INSREG32BF(&(pOpenCtxt->pDevCtxt->pADCRegs->TGCR),ADC_TGCR_SLPC,0); 
                            }
                        }

                        bRet = TRUE;
                    }
                } 
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    ERRORMSG(TRUE, (_T("Exception in ADC IOCTL_POWER_SET\r\n")));
                }
            }
            break;

        case IOCTL_POWER_GET:
            if(pBufOut != NULL                          &&
               dwLenOut == sizeof(CEDEVICE_POWER_STATE) &&
               pdwActualOut != NULL)
            {
                PREFAST_SUPPRESS(6320, "Generic exception handler");
                __try
                {
          
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    bRet = TRUE;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    ERRORMSG(TRUE, (_T("Exception in ADC IOCTL_POWER_SET\r\n")));
                }
            }
            break;
         
        default:
            break;
        }
    }

    
    return bRet;
}


//-----------------------------------------------------------------------------
//
// Function: ADC_Read
//
// This function reads data from the device identified by the open 
// context. Stubs for the interface these should not be used.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      pBuffer 
//         [out] Pointer to the buffer that stores the data read from the 
//                 device. This buffer should be at least Count bytes long. 
//      Count 
//          [in] Number of bytes to read from the device into pBuffer. 
//
// Returns:  
//      Returns zero.
//
//-----------------------------------------------------------------------------
DWORD ADC_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(Count);
    return 0;
}

//-----------------------------------------------------------------------------
//
// Function: ADC_Write
//
// This function writes data to the device.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      pBuffer 
//         [out] Pointer to the buffer that contains the data to write. 
//      Count 
//          [in] Number of bytes to read from the device into pBuffer. 
//
// Returns:  
//      Returns 0.
//
//-----------------------------------------------------------------------------
DWORD ADC_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD Count)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(Count);
    return 0;
}

//-----------------------------------------------------------------------------
//
// Function: ADC_Seek
//
// This function moves the data pointer in the device.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      Amount 
//         [in] Number of bytes to move the data pointer in the device. 
//               A positive value moves the data pointer toward the end of the 
//               file, and a negative value moves it toward the beginning.
//      Type 
//         [in] Starting point for the data pointer. 
//
// Returns:  
//      Returns 0.
//
//-----------------------------------------------------------------------------
DWORD ADC_Seek( DWORD hOpenContext, long Amount, WORD Type)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(Amount);
    UNREFERENCED_PARAMETER(Type);
    return 0;
}

//-----------------------------------------------------------------------------
//
// Function: ADC_PowerUp
//
// This function suspends power to the device. It is useful only with 
// devices that can power down under software control.
//
// Parameters:
//      hDeviceContext 
//          [in] Handle to the device context.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
void ADC_PowerUp(DWORD hDeviceContext)
{
     UNREFERENCED_PARAMETER(hDeviceContext);
   
}

//-----------------------------------------------------------------------------
//
// Function: ADC_PowerDown
//
// This function restores power to a device.
//
// Parameters:
//      hDeviceContext 
//          [in] Handle to the device context.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
void ADC_PowerDown(DWORD hDeviceContext)
{
   UNREFERENCED_PARAMETER(hDeviceContext);   
}
