//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  sdhccontrol.cpp
//
//------------------------------------------------------------------------------

#include "sdhc.h"


#define _DUMP_SSP       0


// Private memeber of the class
BOOL bCardPresent           = FALSE;        // Card Present
BOOL bSDIOInterruptsEnabled = FALSE;        // SDIO Interrupts Enabled
BOOL bSDBus4BitMode         = FALSE;        // SD Bus 4Bit Mode
UINT8 uChannel;                             // SDMMC DMA Channel, SSP1 or SSP2
DWORD dwSysIntr = (DWORD) SYSINTR_UNDEFINED;                          // sys intr for SSP hw interrupt and DMA interrupt
HANDLE hISTEvent = NULL;             // handle to interrupt event
HANDLE htIST        = NULL;             // handle to Interrupt Service Thread
BOOL bDriverShutdown = FALSE;
BOOL bCurrentReqFastPath = FALSE;
BOOL bUseDMA = FALSE;

SD_API_STATUS cmdStatus;

PVOID pv_HWregCLKCTRL = NULL;
PVOID pv_HWregSSP     = NULL;
PVOID pv_HWregAPBH    = NULL;
PVOID pv_HWregDIGCTL  = NULL;


typedef struct _ddi_ssp_DmaCcs_t
{
    struct _ddi_ssp_DmaCcs_t *pNext;
    hw_apbh_chn_cmd_t        DmaCmd;
    void                     *pDMABuffer;
    hw_ssp_ctrl0_t           SspCtrl0;
    hw_ssp_cmd0_t            SspCmd;
    hw_ssp_cmd1_t            SspArg;
} ddi_ssp_DmaCcs_t;


#define SSP_DESC_SIZE             (512)
#define MAXIMUM_DMA_TRANSFER_SIZE (0x00008000)

DMA_ADAPTER_OBJECT dmaAdapter;

ddi_ssp_DmaCcs_t *m_pVirDMADesc;                // pointer to buffers used for DMA transfers
ddi_ssp_DmaCcs_t *m_pPhyDMADesc;                // pointer to buffers used for DMA transfers

PBYTE            m_pDMABuffer;                  // pointer to buffers used for DMA transfers
PHYSICAL_ADDRESS m_pDMABufferPhys;              // physical address of the SMA buffer

void DumpSSPRegister()
{
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("\r\n\r\nDumpSSPRegister\r\n")));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_CTRL0:           0x%X\r\n"), HW_SSP_CTRL0_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_CMD0:            0x%X\r\n"), HW_SSP_CMD0_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_CMD1:            0x%X\r\n"), HW_SSP_CMD1_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_COMPREF:         0x%X\r\n"), HW_SSP_COMPREF_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_COMPMASK:        0x%X\r\n"), HW_SSP_COMPMASK_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_TIMING:          0x%X\r\n"), HW_SSP_TIMING_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_CTRL1:           0x%X\r\n"), HW_SSP_CTRL1_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_DATA:            0x%X\r\n"), HW_SSP_DATA_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP0:         0x%X\r\n"), HW_SSP_SDRESP0_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP1:         0x%X\r\n"), HW_SSP_SDRESP1_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP2:         0x%X\r\n"), HW_SSP_SDRESP2_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP3:         0x%X\r\n"), HW_SSP_SDRESP3_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_STATUS:          0x%X\r\n"), HW_SSP_STATUS_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_DEBUG:           0x%X\r\n"), HW_SSP_DEBUG_RD()));

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CTRL0:         0x%X\r\n"), HW_APBH_CTRL0_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CTRL1:         0x%X\r\n"), HW_APBH_CTRL1_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_NXTCMDAR:  0x%X\r\n"), HW_APBH_CHn_NXTCMDAR(HW_APBH_DMA_SSP1_CHANNEL)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_CURCMDAR:  0x%X\r\n"), HW_APBH_CHn_CURCMDAR(HW_APBH_DMA_SSP1_CHANNEL)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_CMD:       0x%X\r\n"), HW_APBH_CHn_CMD(HW_APBH_DMA_SSP1_CHANNEL)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_BAR:       0x%X\r\n"), HW_APBH_CHn_BAR(HW_APBH_DMA_SSP1_CHANNEL)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_SEMA:      0x%X\r\n"), HW_APBH_CHn_SEMA(HW_APBH_DMA_SSP1_CHANNEL)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_DEBUG1:    0x%X\r\n"), HW_APBH_CHn_DEBUG1(HW_APBH_DMA_SSP1_CHANNEL)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_DEBUG2:    0x%X\r\n"), HW_APBH_CHn_DEBUG2(HW_APBH_DMA_SSP1_CHANNEL)));
}


BOOL ssp_mmcsd_StartDma(UINT8   Channel,
                        BOOL    bRead,
                        BOOL    bCmd,
                        UINT16  u16ByteCount,
                        UINT32  *p32Buffer)
{
    m_pVirDMADesc->pNext = NULL;
    m_pVirDMADesc->DmaCmd.U = (BF_APBH_CHn_CMD_XFER_COUNT(u16ByteCount) |
                              (BF_APBH_CHn_CMD_CMDWORDS(bCmd ? 3 : 1) |
                               BF_APBH_CHn_CMD_WAIT4ENDCMD(1) |
                               BF_APBH_CHn_CMD_SEMAPHORE(1) |
                               BF_APBH_CHn_CMD_IRQONCMPLT(1) |
                               BF_APBH_CHn_CMD_COMMAND(bRead ? BV_APBH_CHn_CMD_COMMAND__DMA_WRITE :
                                                               BV_APBH_CHn_CMD_COMMAND__DMA_READ)));

    m_pVirDMADesc->pDMABuffer = (void *)p32Buffer;

    m_pVirDMADesc->SspCtrl0.U = HW_SSP_CTRL0_RD();
  
    if(bCmd)
    {
        m_pVirDMADesc->SspCmd.U = HW_SSP_CMD0_RD();
        m_pVirDMADesc->SspArg.U = HW_SSP_CMD1_RD();
    }

    // enable the interrupt
    if (!bCurrentReqFastPath)
        DDKApbhDmaEnableCommandCmpltIrq(Channel, TRUE);


    return DDKApbhStartDma(Channel, m_pPhyDMADesc, 1);
}


BOOL ssp_mmcsd_StopDma(UINT8 Channel)
{
    UINT32 u32Sema = (HW_APBH_CHn_SEMA_RD(Channel) & BM_APBH_CHn_SEMA_PHORE);
    UINT32 u32StartMS;

    // poll for completion when fast path is selected
    if (bCurrentReqFastPath)
    {

        u32StartMS = HW_DIGCTL_MICROSECONDS_RD();
        do
        {
            u32Sema = (HW_APBH_CHn_SEMA_RD(Channel) & BM_APBH_CHn_SEMA_PHORE);

            // 12000ms (12 s -- based on DATA TIMEOUT value in TIMING register) Timeout
            if(HW_DIGCTL_MICROSECONDS_RD() - u32StartMS > 12000000)
            {

                ERRORMSG(1, (L"SDHC DMA Channel time-out\r\n"));
                break;
            }
        }while(u32Sema != 0);
    }

    DDKApbhDmaEnableCommandCmpltIrq(Channel, FALSE);
    DDKApbhDmaClearCommandCmpltIrq(Channel);

    // clear error bit, if set
    HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH1_ERROR_IRQ);

    u32Sema = (HW_APBH_CHn_SEMA_RD(Channel) & BM_APBH_CHn_SEMA_PHORE);
        
    // Reset the DMA if hung
    if(u32Sema != 0)
    {
        DDKApbhDmaResetChan(Channel, TRUE);
        DEBUGMSG(SDCARD_ZONE_ERROR, (_T("DDKApbhDmaResetChan SSP1\r\n")));
        return FALSE;
    }

    return DDKApbhStopDma(Channel);
}


BOOL ssp_mmcsd_ResetDma(UINT8 Channel)
{
    return DDKApbhDmaResetChan(Channel, TRUE);
}


void SetClockRate(PDWORD pRate)
{   
    UINT32 u32Timing, u32ClkDiv =0;
    UINT32 u32SSPFreq; 
    DWORD dwRate = *pRate;

    switch(dwRate)
    {
    case SD_DEFAULT_CARD_ID_CLOCK_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD_DEFAULT_CARD_ID_CLOCK_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_240; 
        u32SSPFreq = SSP_24MHZ_FREQUENCY;
        break;

    case SD_LOW_SPEED_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD_LOW_SPEED_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_60; 
        u32SSPFreq = SSP_24MHZ_FREQUENCY;
        break;

    case MMC_FULL_SPEED_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("MMC_FULL_SPEED_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_4; 
        u32SSPFreq = SSP_72MHZ_FREQUENCY;
        break;

    case SD_FULL_SPEED_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD_FULL_SPEED_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_2; 
        u32SSPFreq = SSP_48MHZ_FREQUENCY;
        break;
        
    default:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("Unknown Speed\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_2; 
        u32SSPFreq = SSP_24MHZ_FREQUENCY;
        break;
    }
#if 0
    if(!DDKClockSetSspClk(&u32SSPFreq, TRUE))
    {
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SetClockRate : DDKClockSetSspClk False\r\n")));
    }     
#endif
    u32Timing = HW_SSP_TIMING_RD();
    u32Timing &= 0xFFFF0000;
    u32Timing |= (u32ClkDiv << 8);
    HW_SSP_TIMING_WR(u32Timing);

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SetClockRate: u32Timing= 0x%X, u32ClkDiv = %d, u32SSPFreq = %d\r\n"), u32Timing, u32ClkDiv, u32SSPFreq));
}


void SetInterface(PSD_CARD_INTERFACE pInterface)
{
    if(SD_INTERFACE_SD_MMC_1BIT == (pInterface->InterfaceMode))
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SetInterface : setting for 1 bit mode \r\n")));

        bSDBus4BitMode = FALSE;
        HW_SSP_CTRL0_SET(BF_SSP_CTRL0_BUS_WIDTH(BV_SSP_CTRL0_BUS_WIDTH_ONE_BIT));
    } 
    else if(SD_INTERFACE_SD_4BIT == (pInterface->InterfaceMode))
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SetInterface : setting for 4 bit mode \r\n")));

        bSDBus4BitMode = TRUE;
        HW_SSP_CTRL0_SET(BF_SSP_CTRL0_BUS_WIDTH(BV_SSP_CTRL0_BUS_WIDTH_FOUR_BIT));
    }
    else
    {
        bSDBus4BitMode = FALSE;
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SetInterface : 8 bit mode not supported\r\n")));
    }

    // set rate
    SetClockRate(&pInterface->ClockRate);    
}


///////////////////////////////////////////////////////////////////////////////
//  ControllerIst - controller interrupt service thread for driver
//  Input: 
//  Output: 
//  Return: Thread exit status
//  Notes:
///////////////////////////////////////////////////////////////////////////////
static DWORD WINAPI ControllerIst(PSDCARD_HC_CONTEXT pHCContext)
{
    DWORD dwWaitTime    = INFINITE;

    if (!CeSetThreadPriority(GetCurrentThread(), 151)) {
        DEBUGMSG(SDCARD_ZONE_WARN, (L"ControllerIst: Warning, failed to set CEThreadPriority\r\n"));
    }

    for(;;) {
        // wait for the controller interrupt
        if (WAIT_OBJECT_0 != WaitForSingleObject(hISTEvent, dwWaitTime)) 
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"ControllerIst: Wait Failed!\r\n"));
            break;
        }

        if (bDriverShutdown) {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"ControllerIst: Thread exiting!\r\n"));
            break;
        }

        SDHCDAcquireHCLock(pHCContext);

        HandleInterrupts(pHCContext);
        InterruptDone( dwSysIntr );
        SDHCDReleaseHCLock(pHCContext);
    }

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-ControllerIst\r\n"));
    return 0;

}


///////////////////////////////////////////////////////////////////////////////
//  SDDeInitialize - Deinitialize the the MMC Controller
//  Input:  pHCContext - Host controller context
//
//  Output:
//  Return: SD_API_STATUS code
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS SDDeinitialize(PSDCARD_HC_CONTEXT pHCContext)
{
    PSDHC_HARDWARE_CONTEXT pHardwareContext; // hardware context

    pHardwareContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);

    if( dwSysIntr != SYSINTR_UNDEFINED )
    {
        // disable controller interrupt
        InterruptDisable(dwSysIntr);

        // release the SYSINTR value
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntr, sizeof(DWORD), NULL, 0, NULL);
        dwSysIntr = (DWORD) SYSINTR_UNDEFINED;
    }

    // clean up controller IST
    if (NULL != htIST) {
        // wake up the IST
        bDriverShutdown = TRUE;
        SetEvent(hISTEvent);
        // wait for the thread to exit
        WaitForSingleObject(htIST, INFINITE); 
        CloseHandle(htIST);
        htIST = NULL;
    }

    // free controller interrupt event
    if (NULL != hISTEvent) {
        CloseHandle(hISTEvent);
        hISTEvent = NULL;
    }


    // free memory mapped resources
    if(NULL != pv_HWregSSP)
    {
        MmUnmapIoSpace((PVOID)pv_HWregSSP, 0x1000);
        pv_HWregSSP = NULL;
    }

    if(NULL != pv_HWregCLKCTRL) 
    {
        MmUnmapIoSpace((PVOID) pv_HWregCLKCTRL, 0x1000);
        pv_HWregCLKCTRL = NULL;
    }
    
    if(NULL != pv_HWregAPBH) 
    {
        MmUnmapIoSpace((PVOID)pv_HWregAPBH, 0x1000);
        pv_HWregAPBH = NULL;
    }

    if(NULL != pv_HWregDIGCTL) 
    {
        MmUnmapIoSpace((PVOID)pv_HWregDIGCTL, 0x1000);
        pv_HWregDIGCTL = NULL;
    }

    if(NULL != m_pVirDMADesc)
    {
        MmUnmapIoSpace((PVOID)m_pPhyDMADesc, SSP_DESC_SIZE);
        m_pVirDMADesc = NULL;
        m_pPhyDMADesc = NULL;
    }

    if(NULL != m_pDMABuffer)
    {
        HalFreeCommonBuffer(&dmaAdapter, 0, m_pDMABufferPhys, m_pDMABuffer, FALSE);
        m_pDMABuffer = NULL;
    }

    BSPSDHCDeinit();

    return SD_API_STATUS_SUCCESS;
}


void SSPInitialize()
{
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("+SSPInitialize\r\n")));

    HW_SSP_CTRL0_CLR(BM_SSP_CTRL0_SFTRST);
    while(HW_SSP_CTRL0_RD() & BM_SSP_CTRL0_SFTRST)
    {
        ;
    }

    HW_SSP_CTRL0_CLR(BM_SSP_CTRL0_CLKGATE);
    while(HW_SSP_CTRL0_RD() & BM_SSP_CTRL0_CLKGATE)
    {
        ;
    }

    // Initialize SSP for SD/MMC card
    HW_SSP_TIMING_WR((BF_SSP_TIMING_TIMEOUT(0xFFFF) |
                     BF_SSP_TIMING_CLOCK_DIVIDE(0xF0) |
                     BF_SSP_TIMING_CLOCK_RATE(0)));

    // WORD_LENGTH=8 + SD_MMC mode
    // SDIO_IRQ will be enabled in the SDIO_INTERRUPT_ON
    HW_SSP_CTRL1_WR(
                    BM_SSP_CTRL1_RESP_ERR_IRQ_EN |
                    BM_SSP_CTRL1_RESP_TIMEOUT_IRQ_EN |
                    BM_SSP_CTRL1_DATA_TIMEOUT_IRQ_EN |
                    BM_SSP_CTRL1_DATA_CRC_IRQ_EN |
                    BM_SSP_CTRL1_FIFO_UNDERRUN_EN |
                    BM_SSP_CTRL1_FIFO_OVERRUN_IRQ_EN |
                    BM_SSP_CTRL1_DMA_ENABLE |
                    BM_SSP_CTRL1_POLARITY |
                    BF_SSP_CTRL1_WORD_LENGTH(BV_SSP_CTRL1_WORD_LENGTH__EIGHT_BITS) |
                    BF_SSP_CTRL1_SSP_MODE(BV_SSP_CTRL1_SSP_MODE__SD_MMC));

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("-SSPInitialize\r\n")));
}

///////////////////////////////////////////////////////////////////////////////
//  SDInitialize - Initialize the the MMC Controller
//  Input:  pHardwareContext - newly allocated hardware context
//
//  Output:
//  Return: SD_API_STATUS code
//  Notes:
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS SDInitialize(PSDCARD_HC_CONTEXT pHCContext)
{
    SD_API_STATUS status = SD_API_STATUS_INSUFFICIENT_RESOURCES;   // intermediate status
    PSDHC_HARDWARE_CONTEXT pHardwareContext;        // hardware context

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDInitialize Begin\r\n")));
    
    PHYSICAL_ADDRESS STMP378x_SSP1_Base = {CSP_BASE_REG_PA_SSP1};
    PHYSICAL_ADDRESS STMP378x_CLKMGR_Base = {CSP_BASE_REG_PA_CLKCTRL};
    PHYSICAL_ADDRESS STMP378x_APBH_Base = {CSP_BASE_REG_PA_APBH};
    PHYSICAL_ADDRESS STMP378x_DIGCTL_Base = {CSP_BASE_REG_PA_DIGCTL};
    PHYSICAL_ADDRESS STMP378x_OCRAM_DESC_Base = {BSPSDHCGetDMADescAddress()};

    pHardwareContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);

    // Initialize
    bSDIOInterruptsEnabled = FALSE;
    bCardPresent = FALSE;
    bSDBus4BitMode = FALSE;
    uChannel = BSPSDHCGetDMAChannel();

    // BSP SDHC Initialization
    if (!BSPSDHCInit(pHardwareContext))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error initializing SSP hw\r\n")));
        goto Exit;
    }

    // Sysintr initialization
    if (!BSPSDHCSysIntrInit(&dwSysIntr))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating SD/MMC interrupt\r\n")));
        goto Exit;
    }

    pv_HWregSSP = (PVOID)MmMapIoSpace(STMP378x_SSP1_Base, 0x1000, FALSE);
    if(pv_HWregSSP == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating SD/MMC registers\r\n")));
        goto Exit;
    }

    pv_HWregCLKCTRL = (PVOID)MmMapIoSpace(STMP378x_CLKMGR_Base, 0x1000, FALSE);
    if(pv_HWregCLKCTRL == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating Clock control registers\r\n")));
        goto Exit;
    }

    pv_HWregAPBH = (PVOID)MmMapIoSpace(STMP378x_APBH_Base, 0x1000, FALSE);
    if(pv_HWregAPBH == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating APBH registers")));
        goto Exit;
    }

    pv_HWregDIGCTL = (PVOID)MmMapIoSpace(STMP378x_DIGCTL_Base, 0x1000, FALSE);
    if(pv_HWregDIGCTL == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating pv_HwRegDigitCtrl registers")));
        goto Exit;
    }
    
    m_pPhyDMADesc = (ddi_ssp_DmaCcs_t *)(BSPSDHCGetDMADescAddress());
    m_pVirDMADesc = (ddi_ssp_DmaCcs_t *)MmMapIoSpace(STMP378x_OCRAM_DESC_Base, SSP_DESC_SIZE, FALSE);

    dmaAdapter.ObjectSize = sizeof(dmaAdapter);
    dmaAdapter.InterfaceType = Internal;
    dmaAdapter.BusNumber = 0;
    m_pDMABuffer = (PBYTE)HalAllocateCommonBuffer(&dmaAdapter, MAXIMUM_DMA_TRANSFER_SIZE, &m_pDMABufferPhys, FALSE);
    if(m_pDMABuffer == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SHCDriver: - Unable to allocate memory for DMA buffers!\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    //Clock initialize
    HW_CLKCTRL_SSP_CLR(0x80000000);

    // SSP
    SSPInitialize();

    // Reset the DMA
    ssp_mmcsd_ResetDma(uChannel);

    // allocate the interrupt event for the SSP interrupt
    hISTEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if (NULL == hISTEvent) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    if ( !InterruptInitialize( dwSysIntr, hISTEvent, NULL, 0 ) ) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    // create the Controller IST thread
    htIST = CreateThread(NULL,
        0,
        (LPTHREAD_START_ROUTINE) ControllerIst,
        pHCContext,
        0,
        NULL);

    if (NULL == htIST) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    status = SD_API_STATUS_SUCCESS;

Exit:
    if(!SD_API_SUCCESS(status)) 
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDInitialize : SDDeinitialize\r\n")));
        // just call the deinit handler directly to cleanup
        SDDeinitialize(pHCContext);
    }

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDInitialize : status = 0x%X\r\n"), status));
    return status;
}


void EnableSDIOInterrupts(void) 
{
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("EnableSDIOInterrupts\r\n")));
    HW_SSP_CTRL0_SET(BM_SSP_CTRL0_SDIO_IRQ_CHECK);
    HW_SSP_CTRL1_SET(BM_SSP_CTRL1_SDIO_IRQ_EN);

    bSDIOInterruptsEnabled = TRUE;
}


void DisableSDIOInterrupts(void) 
{
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("DisableSDIOInterrupts\r\n")));
    HW_SSP_CTRL0_CLR(BM_SSP_CTRL0_SDIO_IRQ_CHECK);
    HW_SSP_CTRL1_CLR(BM_SSP_CTRL1_SDIO_IRQ_EN);

    bSDIOInterruptsEnabled = FALSE;
}


///////////////////////////////////////////////////////////////////////////////
//  ReadReceiveFifo - Read from the received Fifo
//  Input:  pRequest - the request to get the data from
//          ByteCount - number of bytes to read
//          MaxBytes - limit of this transfer
//  Output:
//  Return:
//  Notes:
//
///////////////////////////////////////////////////////////////////////////////
void ReadReceiveFifo(PSD_BUS_REQUEST pRequest, DWORD dwSize)
{   
    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("ReadReceiveFifo : pRequest->HCParam = %d, dwSize = %d\r\n"), pRequest->HCParam, dwSize));

    // wait for DMA completion
    if(!ssp_mmcsd_StopDma(uChannel))
    {
        DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("+ReadReceiveFifo: ddi_ssp_mmcsd_StopDma - failed\r\n")));
    }

    // safely copy the data
    SDPerformSafeCopy(&pRequest->pBlockBuffer[pRequest->HCParam],
                      m_pDMABuffer,
                      dwSize);

    pRequest->HCParam += dwSize;
}


///////////////////////////////////////////////////////////////////////////////
//  WriteTransmitFifo - Write to the transmit fifo
//  Input:  pController - the controler
//          pRequest    - the request
//  Output:
//  Return:  returns TRUE if the request has been fullfilled
//  Notes:
///////////////////////////////////////////////////////////////////////////////
void WriteTransmitFifo(PSD_BUS_REQUEST pRequest, DWORD dwSize)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("WriteTransmitFifo : pRequest->HCParam = %d, dwSize = %d\r\n"), pRequest->HCParam, dwSize));

    // wait for DMA completion
    if(!ssp_mmcsd_StopDma(uChannel)) 
    {
        DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("+WriteTransmitFifo: ddi_ssp_mmcsd_StopDma - failed\r\n")));
    }

    pRequest->HCParam += dwSize;
}


///////////////////////////////////////////////////////////////////////////////
//  HandleEndCommandInterrupt - Handle End of Command Interrupt
//  Input:  pController - the controller that is interrupting
//  Output:
//  Notes:
///////////////////////////////////////////////////////////////////////////////
void HandleEndCommandInterrupt(PSDCARD_HC_CONTEXT pHCContext)
{
    PSD_BUS_REQUEST pRequest;       // the request to complete
    UINT32          u32Response;

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+HandleEndCommandInterrupt\r\n")));

    // get the current request; if none, then mus tbe interrupt from SDIO card that caused this thread to run
    pRequest = SDHCDGetAndLockCurrentRequest(pHCContext, 0);
    if(NULL == pRequest)
    {
        return;
    }
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+HandleEndCommandInterrupt - HW_SSP_CTRL1: 0x%x\r\n"), HW_SSP_CTRL1_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+HandleEndCommandInterrupt - pRequest->TransferClass: 0x%x\r\n"), pRequest->TransferClass));

    // command/response only
    if(SD_COMMAND == pRequest->TransferClass) 
    {
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt COMMAND completed\r\n")));
    } 

    // either we need to poll for DMA completion for fast path, or we got the interrupt because DMA is complete unless SSP is still running
    else if (bUseDMA &&  (bCurrentReqFastPath || !HW_SSP_CTRL0.B.RUN))
    {
    
        if(SD_READ == pRequest->TransferClass)
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt starting READ TRANSFER of %d blocks of %d bytes\r\n"), pRequest->NumBlocks, pRequest->BlockSize ));

            ReadReceiveFifo(pRequest, pRequest->NumBlocks * pRequest->BlockSize);
        } 
        else 
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt starting WRITE TRANSFER of %d blocks of %d bytes\r\n"), pRequest->NumBlocks, pRequest->BlockSize ));

            WriteTransmitFifo(pRequest, pRequest->NumBlocks * pRequest->BlockSize);
        }

        bUseDMA = FALSE;
    }

    if(NoResponse != pRequest->CommandResponse.ResponseType) 
    {
        // read in the response words from the response fifo.
        if(ResponseR2 == pRequest->CommandResponse.ResponseType)  
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt - ResponseType: %d\r\n"), pRequest->CommandResponse.ResponseType));

            u32Response = HW_SSP_SDRESP0_RD();
            pRequest->CommandResponse.ResponseBuffer[0] = (UCHAR)(((u32Response >> 0) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[1] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[2] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[3] = (UCHAR)(((u32Response >> 24) & 0xFF));

            u32Response = HW_SSP_SDRESP1_RD();
            pRequest->CommandResponse.ResponseBuffer[4] = (UCHAR)(((u32Response >> 0) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[5] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[6] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[7] = (UCHAR)(((u32Response >> 24) & 0xFF));

            u32Response = HW_SSP_SDRESP2_RD();
            pRequest->CommandResponse.ResponseBuffer[8] = (UCHAR)(((u32Response >> 0) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[9] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[10] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[11] = (UCHAR)(((u32Response >> 24) & 0xFF));


            u32Response = HW_SSP_SDRESP3_RD();
            pRequest->CommandResponse.ResponseBuffer[12] = (UCHAR)(((u32Response >> 0) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[13] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[14] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[15] = (UCHAR)(((u32Response >> 24) & 0xFF));
        } 
        else 
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt - ResponseType: %d\r\n"), pRequest->CommandResponse.ResponseType));

            u32Response = HW_SSP_SDRESP1_RD();
            pRequest->CommandResponse.ResponseBuffer[5] = (UCHAR)(u32Response & 0x3F );
            u32Response = HW_SSP_SDRESP0_RD();
            pRequest->CommandResponse.ResponseBuffer[4] = (UCHAR)(((u32Response >> 24) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[3] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[2] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[1] = (UCHAR)(((u32Response >> 0) & 0xFF));

            pRequest->CommandResponse.ResponseBuffer[0] = (UCHAR)((0xFF));
        }
    }

    cmdStatus = SD_API_STATUS_SUCCESS;
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("-HandleEndCommandInterrupt\r\n")));
}


VOID HandleInterrupts(PSDCARD_HC_CONTEXT pHCContext)
{
    UINT32 u32Ctrl1;
    UINT32 u32Status;
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleInterrupts\r\n")));

    // read response and complete DMA if needed
    HandleEndCommandInterrupt(pHCContext);

    // check for SDIO card interrupt
    if(bSDIOInterruptsEnabled && HW_SSP_CTRL1.B.SDIO_IRQ)
    {
       DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDIO Interrupt\r\n")));

        // disable the SDIO interrupt
        DisableSDIOInterrupts();

        // clear the interrupt bit
        HW_SSP_CTRL1_SET(BM_SSP_CTRL1_SDIO_IRQ);
                
        // indicate that the card is interrupting
        SDHCDIndicateSlotStateChange(pHCContext, 0, DeviceInterrupting);
    }


    u32Ctrl1 = HW_SSP_CTRL1_RD();
    u32Status = HW_SSP_STATUS_RD();
    

    // RESP_ERR_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_RESP_ERR_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("RESP_ERR_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(BF_SSP_CTRL1_RESP_ERR_IRQ(1));
        cmdStatus = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
    }
        
    // RESP_TIMEOUT_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_RESP_TIMEOUT_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("RESP_TIMEOUT_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(BF_SSP_CTRL1_RESP_TIMEOUT_IRQ(1));
        cmdStatus = SD_API_STATUS_RESPONSE_TIMEOUT;        
    }

    // DATA_TIMEOUT_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_DATA_TIMEOUT_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("DATA_TIMEOUT_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(BF_SSP_CTRL1_DATA_TIMEOUT_IRQ(1));

        cmdStatus = SD_API_STATUS_DATA_TIMEOUT;
    }

    // DATA_CRC_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_DATA_CRC_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("DATA_CRC_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(BF_SSP_CTRL1_DATA_CRC_IRQ(1));
        cmdStatus = SD_API_STATUS_CRC_ERROR;
    }

    // FIFO_UNDERRUN_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_FIFO_UNDERRUN_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("FIFO_UNDERRUN_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(BF_SSP_CTRL1_FIFO_UNDERRUN_IRQ(1));
        cmdStatus = SD_API_STATUS_DATA_ERROR;
    }

    // FIFO_OVERRUN_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_FIFO_OVERRUN_IRQ(1))
    {
       DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("FIFO_OVERRUN_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(BF_SSP_CTRL1_FIFO_OVERRUN_IRQ(1));
        cmdStatus = SD_API_STATUS_DATA_ERROR;
    }

    if (!bCurrentReqFastPath && !HW_SSP_CTRL0.B.RUN)
    {

        PSD_BUS_REQUEST pRequest = SDHCDGetAndLockCurrentRequest(pHCContext, 0);
        if (pRequest)
            SDHCDIndicateBusRequestComplete(pHCContext, pRequest, cmdStatus);


    }
    
}


SD_API_STATUS SendCommand(PSD_BUS_REQUEST pRequest)
{     
    UINT32 cmdatRegister = 0;

    while((HW_SSP_STATUS_RD() & (BM_SSP_STATUS_BUSY | BM_SSP_STATUS_DATA_BUSY | BM_SSP_STATUS_CMD_BUSY) ))
    {
        Sleep(1);        
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD Bus Busy : HW_SSP_STATUS = 0x%X, DEBUG = 0x%08x\r\n"), HW_SSP_STATUS_RD(), HW_SSP_DEBUG_RD()));
    }
    
    // clear CTRL0 and COM0 register first
    HW_SSP_CTRL0_CLR(0xEFFFFFFF);
    HW_SSP_CMD0_CLR(0xFFFFFFFF);
    
    // set the command
    HW_SSP_CMD0_SET(BF_SSP_CMD0_CMD(pRequest->CommandCode));
    
    if((pRequest->CommandCode & 0xFF) == SD_CMD_IO_RW_DIRECT ||
       (pRequest->CommandCode & 0xFF) == SD_CMD_IO_RW_EXTENDED)
    {
        HW_SSP_CMD0_SET(BF_SSP_CMD0_APPEND_8CYC(1));
    }
    
    // set the argument
    HW_SSP_CMD1_WR(pRequest->CommandArgument);
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+SendCommand - ResponseType: 0x%x\r\n"), pRequest->CommandResponse.ResponseType));
    
    switch (pRequest->CommandResponse.ResponseType) 
    {
    case NoResponse:
        //No Setting here
        break;

    case ResponseR1:
    case ResponseR1b:
    case ResponseR5:
    case ResponseR6:
    case ResponseR7:
        // on an MMC controller R5 and R6 are really just an R1 response (CRC protected)
        cmdatRegister = BM_SSP_CTRL0_GET_RESP;
        break;

    case ResponseR2:
        cmdatRegister = BM_SSP_CTRL0_GET_RESP | BM_SSP_CTRL0_LONG_RESP;
        break;

    case ResponseR3:
    case ResponseR4:
        // R4 is really same as an R3 response on an MMC controller (non-CRC)
        cmdatRegister = BM_SSP_CTRL0_GET_RESP | BM_SSP_CTRL0_IGNORE_CRC;
        break;
    
    default:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SendCommand : Invalid ResponseType\r\n")));
        return SD_API_STATUS_INVALID_PARAMETER;
    }
    
    // check for Command Only
    if((SD_COMMAND == pRequest->TransferClass)) 
    {
        //Set BLOCK_SIZE = 0 and BLOCK_COUNT = 0
        HW_SSP_CMD0_CLR(0xFFF00);
        // HW_SSP_CMD0_SET(BF_SSP_CMD0_APPEND_8CYC(1));
    } 
    else 
    {
        // command with a data phase
        cmdatRegister |= BM_SSP_CTRL0_DATA_XFER;
    
        // set the buffer index to the end of the buffer
        pRequest->HCParam = 0;
    
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("pRequest->BlockSize: %d, pRequest->NumBlocks: %d\r\n"), pRequest->BlockSize, pRequest->NumBlocks));
        HW_SSP_CMD0_CLR(0xFFF00);
    
        if(pRequest->NumBlocks > 1)
        { 
            if(pRequest->BlockSize == 512)
            {
                HW_SSP_CMD0_SET(BF_SSP_CMD0_BLOCK_SIZE(9) | BF_SSP_CMD0_BLOCK_COUNT(pRequest->NumBlocks - 1));
            }

            cmdatRegister |= BF_SSP_CTRL0_XFER_COUNT(pRequest->BlockSize * pRequest->NumBlocks);
        }
        else
        {   
            cmdatRegister |= BF_SSP_CTRL0_XFER_COUNT(pRequest->BlockSize);
        }
       
    
        // check for read
        if(SD_READ == pRequest->TransferClass) 
        {
            cmdatRegister |= BM_SSP_CTRL0_READ;
        }
    
    }
    
    // check to see if we need to append the 80 clocks (i.e. this is the first transaction)
    
    // check to see if we need to enable the SDIO interrupt checking
    if(bSDIOInterruptsEnabled)
    {
        cmdatRegister |= BM_SSP_CTRL0_SDIO_IRQ_CHECK;
    }
    
    // check to see if we need to enable wide bus (4 bit) data transfer mode
    if(bSDBus4BitMode) 
    {
        cmdatRegister |= (BV_SSP_CTRL0_BUS_WIDTH_FOUR_BIT << BP_SSP_CTRL0_BUS_WIDTH);
    }

    cmdatRegister |= (BM_SSP_CTRL0_ENABLE | BM_SSP_CTRL0_WAIT_FOR_IRQ);
    HW_SSP_CTRL1_CLR(0xAAA08000);
    HW_SSP_CTRL0_WR(cmdatRegister);
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SendCommand - CMDAT Reg : 0x%08X, CMD : %d\r\n"), cmdatRegister, pRequest->CommandCode));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SendCommand : %d\r\n"), pRequest->CommandCode));
     
    if((cmdatRegister & BM_SSP_CTRL0_DATA_XFER) == 0)
    {
        HW_SSP_CTRL0_SET(BM_SSP_CTRL0_RUN);
    }

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_STATUS = 0x%X\r\nHW_SSP_CMD0 = 0x%X\r\nHW_SSP_CMD1 = 0x%X\r\nHW_SSP_CTRL0 = 0x%X\r\nHW_SSP_CTRL1 = 0x%X\r\n"), 
                    HW_SSP_STATUS_RD(), HW_SSP_CMD0_RD(), HW_SSP_CMD1_RD(),
                    HW_SSP_CTRL0_RD(), HW_SSP_CTRL1_RD()));
    

    // poll for completion of command
    if (pRequest->TransferClass == SD_COMMAND)
    {
        bUseDMA = FALSE;
        while(HW_SSP_CTRL0.B.RUN);
    }

    // start the DMA transfer
    else
    {    

        if (pRequest->TransferClass == SD_WRITE)
        {
            // perform the safe copy to DMA buffer before starting DMA for writes
            SDPerformSafeCopy(m_pDMABuffer,
                              &pRequest->pBlockBuffer[pRequest->HCParam],
                              pRequest->BlockSize * pRequest->NumBlocks);
        }
    
        if (!ssp_mmcsd_StartDma(uChannel, (pRequest->TransferClass == SD_READ), TRUE,  
                                          (UINT16)(pRequest->BlockSize * pRequest->NumBlocks), 
                                          (UINT32 *)m_pDMABufferPhys.LowPart))
        {
            return SD_API_STATUS_DATA_ERROR;
        }

        bUseDMA = TRUE;
    }
        
    return SD_API_STATUS_PENDING;
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCCancelIoHandler - io cancel handler
//  Input:  pHostContext - host controller context
//          Slot - slot the request is going on
//          pRequest - the request to be cancelled
//
//  Output:
//  Return: TRUE if the request was cancelled
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN SDHCCancelIoHandler(PSDCARD_HC_CONTEXT pHCContext,
                            DWORD              Slot,
                            PSD_BUS_REQUEST    pRequest)
{
    UNREFERENCED_PARAMETER(Slot);

    PSDHC_HARDWARE_CONTEXT pController;

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+SDHCancelIoHandler\r\n")));

    // get our extension
    pController = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);

    // release the lock before we complete the request
    SDHCDReleaseHCLock(pHCContext);

    // complete the request with a cancelled status
    SDHCDIndicateBusRequestComplete(pHCContext,
                                    pRequest,
                                    SD_API_STATUS_CANCELED);

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("-SDHCancelIoHandler\r\n")));

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCBusRequestHandler - bus request handler
//  Input:  pHostContext - host controller context
//          Slot - slot the request is going on
//          pRequest - the request
//
//  Output:
//  Return: SD_API_STATUS Code
//  Notes:  The request passed in is marked as uncancelable, this function
//          has the option of making the outstanding request cancelable
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS SDHCBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext,
                                    DWORD              Slot,
                                    PSD_BUS_REQUEST    pRequest)
{
    UNREFERENCED_PARAMETER(Slot);
    SD_API_STATUS retVal;
        
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHCBusRequestHandler - CMD: %02d DATA: 0x%08X, TC: %d\r\n"),
                     pRequest->CommandCode, pRequest->CommandArgument, pRequest->TransferClass));
       

    SDHCDAcquireHCLock(pHCContext);

    if (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)
    {
        bCurrentReqFastPath = TRUE;

        // mask interrupts at the core
        InterruptMask(dwSysIntr,TRUE);

        retVal = SendCommand(pRequest);
        if ( retVal == SD_API_STATUS_PENDING ) { // Polling for completion.
                HandleInterrupts(pHCContext);
                
            retVal = cmdStatus;
        }

        // unmask interrupt at the core
        InterruptMask(dwSysIntr,FALSE);
        
        bCurrentReqFastPath = FALSE;

    }
        
    else
    {
        retVal = SendCommand(pRequest);


        // for command only non-fast path requests, there will be no interrupts if there is no error, so set the interrupt event to grab the response
        if (pRequest->TransferClass == SD_COMMAND && 
             (!(HW_SSP_CTRL1_RD() & (BM_SSP_CTRL1_RESP_ERR_IRQ | BM_SSP_CTRL1_RESP_TIMEOUT_IRQ))) )
        {
            SetEvent(hISTEvent);
        }
        
        if(!SD_API_SUCCESS(retVal))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"SDHCBusRequestHandler: Error sending command:0x%02x\r\n", pRequest->CommandCode));      
        }
    }

    if (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)
    {
        if (retVal == SD_API_STATUS_SUCCESS)
            retVal = SD_API_STATUS_FAST_PATH_SUCCESS;
    }
    
    SDHCDReleaseHCLock(pHCContext);

    return retVal;
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCSlotOptionHandler - handler for slot option changes
//  Input:  pHostContext - host controller context
//          SlotNumber   - the slot the change is being applied to
//          Option       - the option code
//          pData        - data associated with the option
//          OptionSize   - size of option data
//  Output:
//  Return: SD_API_STATUS code
//  Notes:
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS SDHCSlotOptionHandler(PSDCARD_HC_CONTEXT    pHCContext,
                                    DWORD                 SlotNumber,
                                    SD_SLOT_OPTION_CODE   Option,
                                    PVOID                 pData,
                                    ULONG                 OptionSize)
{
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // status
    PSDHC_HARDWARE_CONTEXT pController;         // the controller

#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(SlotNumber);
#endif

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+SDHCSlotOptionHandler\r\n")));
    // get our extension
    pController = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);

    switch (Option) 
    {
    case SDHCDSetSlotPower:
        
        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - SetSlotPower : 0x%08X\r\n"), *((PDWORD)pData)));
        break;

    case SDHCDSetSlotInterface:
        
        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - SetSlotInterface : Clock Setting: %d\r\n"), ((PSD_CARD_INTERFACE)pData)->ClockRate));
        {
            PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;
            SetInterface(pInterface);
        }
        break;

    case SDHCDEnableSDIOInterrupts:
    case SDHCDAckSDIOInterrupt:
            
        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - EnableSDIOInterrupts/SDHCDAckSDIOInterrupt\r\n"), SlotNumber));

        EnableSDIOInterrupts();
            
        break;

    case SDHCDDisableSDIOInterrupts:

        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - DisableSDIOInterrupts : on slot %d\r\n"), SlotNumber));

        DisableSDIOInterrupts();

        break;

    case SDHCDGetWriteProtectStatus:

        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - SDHCDGetWriteProtectStatus : on slot %d\r\n"), SlotNumber));
        {
            PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;
            pInterface->WriteProtected = BSPSDHCIsWriteProtected();
        }

        break;
            
    case SDHCDSetSlotPowerState:
    case SDHCDGetSlotPowerState:

        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - SetSlotPowerState : 0x%08X\r\n"), *((PDWORD)pData)));
        break;
            
    case SDHCDQueryBlockCapability:
        {
            PSD_HOST_BLOCK_CAPABILITY pBlockCaps = (PSD_HOST_BLOCK_CAPABILITY)pData;

            RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler: Read Block Length: %d, Read Blocks: %d\r\n"), pBlockCaps->ReadBlockSize, pBlockCaps->ReadBlocks));
            RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler: Write Block Length: %d, Write Blocks: %d\r\n"), pBlockCaps->WriteBlockSize, pBlockCaps->WriteBlocks));
 
            if(pBlockCaps->ReadBlockSize > SDHC_MAX_BLOCK_SIZE) 
            {
                pBlockCaps->ReadBlockSize = SDHC_MAX_BLOCK_SIZE;
            }

            if(pBlockCaps->ReadBlockSize < SDHC_MIN_BLOCK_SIZE ) 
            {
                pBlockCaps->ReadBlockSize = SDHC_MIN_BLOCK_SIZE;
            }

            if(pBlockCaps->WriteBlockSize > SDHC_MAX_BLOCK_SIZE) 
            {
                pBlockCaps->WriteBlockSize = SDHC_MAX_BLOCK_SIZE;
            }

            if(pBlockCaps->WriteBlockSize < SDHC_MIN_BLOCK_SIZE ) 
            {
                pBlockCaps->WriteBlockSize = SDHC_MIN_BLOCK_SIZE;
            }
        }
        break;

    case SDHCDGetSlotInfo:
        
        if(OptionSize != sizeof(SDCARD_HC_SLOT_INFO) || pData == NULL)
        {
            status = SD_API_STATUS_INVALID_PARAMETER;
        }
        else
        {
            PSDCARD_HC_SLOT_INFO pSlotInfo = (PSDCARD_HC_SLOT_INFO)pData;

            // set the slot capabilities
            SDHCDSetSlotCapabilities(pSlotInfo, (SD_SLOT_SD_1BIT_CAPABLE |
                                                 SD_SLOT_SD_4BIT_CAPABLE |
                                                 SD_SLOT_SDIO_CAPABLE |
                                                 SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK));

            SDHCDSetVoltageWindowMask(pSlotInfo, (SD_VDD_WINDOW_2_9_TO_3_0 | 
                                                  SD_VDD_WINDOW_3_0_TO_3_1 | 
                                                  SD_VDD_WINDOW_3_1_TO_3_2));

            // Set optimal voltage
            SDHCDSetDesiredSlotVoltage(pSlotInfo, SD_VDD_WINDOW_3_0_TO_3_1);

            SDHCDSetMaxClockRate(pSlotInfo, 10000000);

            // Set power up delay. We handle this in SetVoltage().
            SDHCDSetPowerUpDelay(pSlotInfo, 250);   // 300
        }
        break;

    default:
        status = SD_API_STATUS_INVALID_PARAMETER;
        break;
    }

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("-SDHCSlotOptionHandler\r\n")));
    return status;
}


void ProcessCardInsertion(void *pContext)
{
    DWORD dwDefaultClock = SD_DEFAULT_CARD_ID_CLOCK_RATE;    
    PSDHC_HARDWARE_CONTEXT pHCDevice = (PSDHC_HARDWARE_CONTEXT)pContext;
    
    if(!bCardPresent) 
    {
        DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("ProcessCardInsertion\r\n")));
        // mark that the card is in the slot
        bCardPresent = TRUE;

        // set rate
        SetClockRate(&dwDefaultClock);

        // need to send at least 74 clocks to SDIO cards to allow initialization, so turn on continuous clocking
        HW_SSP_CMD0_SET(BM_SSP_CMD0_CONT_CLKING_EN);

        // give the card some time for initialization
        Sleep(100);

        // turn off the continuous clocking mode, so clk will only run when cmd or data are being processed
        HW_SSP_CMD0_CLR(BM_SSP_CMD0_CONT_CLKING_EN);        

        // turn off SDIO interrupts
        if(bSDIOInterruptsEnabled)
        {
            DisableSDIOInterrupts();
        }
    
        // indicate the slot change
        SDHCDIndicateSlotStateChange(pHCDevice->pHCContext,
                                     0,
                                     DeviceInserted);
    }
}


void ProcessCardRemoval(void *pContext)
{
    PSDHC_HARDWARE_CONTEXT pHCDevice = (PSDHC_HARDWARE_CONTEXT)pContext;
    if(bCardPresent)
    {
        DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("ProcessCardRemoval\r\n")));

        // mark that the card has been removed
        bCardPresent = FALSE;

        // turn off SDIO interrupts
        if(bSDIOInterruptsEnabled)
        {
            DisableSDIOInterrupts();
        }

        // indicate the slot change
        SDHCDIndicateSlotStateChange(pHCDevice->pHCContext,
                                     0,
                                     DeviceEjected);
    }
}


