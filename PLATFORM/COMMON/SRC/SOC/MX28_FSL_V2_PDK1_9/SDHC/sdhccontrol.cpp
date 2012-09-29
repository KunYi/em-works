//------------------------------------------------------------------------------
//
//  Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  sdhccontrol.cpp
//
//------------------------------------------------------------------------------

#include "sdhc.h"
#include "sdbus_ext.hpp"

#define _DUMP_SSP       0
#define MAX_SLOT        4

PVOID pv_HWregSSP0;
PVOID pv_HWregSSP1;
PVOID pv_HWregSSP2;
PVOID pv_HWregSSP3;

PVOID pv_HWregCLKCTRL = NULL;
PVOID pv_HWregSSP     = NULL;
PVOID pv_HWregAPBH    = NULL;
PVOID pv_HWregDIGCTL  = NULL;

static CRITICAL_SECTION    g_cs[MAX_SLOT];
static int dwCount[MAX_SLOT];
    
typedef struct _ddi_ssp_DmaCcs_t
{
    struct _ddi_ssp_DmaCcs_t *pNext;
    hw_apbh_chn_cmd_t        DmaCmd;
    void                     *pDMABuffer;
    hw_ssp_ctrl0_t           SspCtrl0;
    hw_ssp_cmd0_t            SspCmd;
    hw_ssp_cmd1_t            SspArg;

    hw_ssp_xfer_size_t  Xfer_size;
    hw_ssp_block_size_t  Block_size;
    //hw_ssp_compref_t   Compref;
    //hw_ssp_compmask_t  Compmask;
    //hw_ssp_timing_t    Timing;
    //hw_ssp_ctrl1_t     Ctrl1;
    
} ddi_ssp_DmaCcs_t;


#define SSP_DESC_SIZE             (512)
#define MAXIMUM_DMA_TRANSFER_SIZE (0x00008000)

static BOOL SDHCSetupCardDetectIST(void *pHardwareContext);
static void SDHCCleanupCardDetectIST(void *pHardwareContext);
static DWORD SDHCCardDetectThread(void *pHWContext);
static UINT8 SDHCGetDMAChannel(UINT32 index);
static BOOL SDHCClockGate(DWORD dwIndex, BOOL bGate);

DWORD ComputeLog2(DWORD dwNum)
{
    DWORD log2 = 0;
    DWORD temp = dwNum;
    //----- 1. SPECIAL CASE: 2 raised to any exponent never equals 0 ----
    if(dwNum == 0)
    {
        ERRORMSG(TRUE, (_T("PARA Invalid\r\n")));
        return FALSE;
    }
    
    //----- 2. Keep dividing by 2 until the LSB is 1 -----
    while(!(dwNum & 0x000000001))
    {
        dwNum >>= 1;
        log2++;
    }

    //----- 3. If (dwNum>>1) != 0, dwNum wasn't a power of 2 -----
    if(dwNum>>1)
    {
        ERRORMSG(TRUE, (_T("dwNum wasn't a power of 2: 0x%x\r\n"),temp ));
        return FALSE;
    }
   
    return log2;
}
void DumpSSPRegister(DWORD dwIndex)
{
#ifndef DEBUG
    UNREFERENCED_PARAMETER(dwIndex);
#else
    SDHCClockGate(dwIndex, FALSE);
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("\r\n\r\nDumpSSPRegister\r\n")));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_CTRL0:           0x%X\r\n"), HW_SSP_CTRL0_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_CMD0:            0x%X\r\n"), HW_SSP_CMD0_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_CMD1:            0x%X\r\n"), HW_SSP_CMD1_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_BLOCKSIZE:    0x%X\r\n"), HW_SSP_BLOCK_SIZE_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_XFERSIZE:       0x%X\r\n"), HW_SSP_XFER_SIZE_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_COMPREF:         0x%X\r\n"), HW_SSP_COMPREF_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_COMPMASK:        0x%X\r\n"), HW_SSP_COMPMASK_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_TIMING:          0x%X\r\n"), HW_SSP_TIMING_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_CTRL1:           0x%X\r\n"), HW_SSP_CTRL1_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_DATA:            0x%X\r\n"), HW_SSP_DATA_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP0:         0x%X\r\n"), HW_SSP_SDRESP0_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP1:         0x%X\r\n"), HW_SSP_SDRESP1_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP2:         0x%X\r\n"), HW_SSP_SDRESP2_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP3:         0x%X\r\n"), HW_SSP_SDRESP3_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP3:         0x%X\r\n"), HW_SSP_SDRESP3_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_SDRESP3:         0x%X\r\n"), HW_SSP_SDRESP3_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_DEBUG:         0x%X\r\n"), HW_SSP_DEBUG_RD(dwIndex)));
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_DDR   :          0x%X\r\n"), HW_SSP_DDR_CTRL_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_DLL   :           0x%X\r\n"), HW_SSP_DLL_CTRL_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CTRL0:         0x%X\r\n"), HW_APBH_CTRL0_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CTRL1:         0x%X\r\n"), HW_APBH_CTRL1_RD()));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_NXTCMDAR:  0x%X\r\n"), HW_APBH_CHn_NXTCMDAR(SDHCGetDMAChannel(dwIndex))));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_CURCMDAR:  0x%X\r\n"), HW_APBH_CHn_CURCMDAR(SDHCGetDMAChannel(dwIndex))));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_CMD:       0x%X\r\n"), HW_APBH_CHn_CMD(SDHCGetDMAChannel(dwIndex))));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_BAR:       0x%X\r\n"), HW_APBH_CHn_BAR(SDHCGetDMAChannel(dwIndex))));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_SEMA:      0x%X\r\n"), HW_APBH_CHn_SEMA(SDHCGetDMAChannel(dwIndex))));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_DEBUG1:    0x%X\r\n"), HW_APBH_CHn_DEBUG1(SDHCGetDMAChannel(dwIndex))));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_APBH_CHn_DEBUG2:    0x%X\r\n"), HW_APBH_CHn_DEBUG2(SDHCGetDMAChannel(dwIndex))));
    
    SDHCClockGate(dwIndex, TRUE);
#endif    
}

//-----------------------------------------------------------------------------
//
// Function:  ssp_mmcsd_StartDma
//
// This function start SDHC DMA channel .
//
// Parameters:
//      pContext
//          [in] Hardware Context.
//      Channel
//          [in] Channel to be started
//      bRead
//          [in] if read request
//      bCmd
//          [in] if a command request
//      u16ByteCount
//          [in] count byte to transfer
//      p32Buffer
//          [in] buffer to  transfer
//
// Returns:
//      True for success
//
//-----------------------------------------------------------------------------
BOOL ssp_mmcsd_StartDma(PSDHC_HARDWARE_CONTEXT pContext, 
                        UINT8   Channel,
                        BOOL    bRead,
                        BOOL    bCmd,
                        UINT16  uBlockSize,
                        UINT16  uBlockCount,
                        UINT32  *p32Buffer)
{
    ddi_ssp_DmaCcs_t *pVirDMADesc = (ddi_ssp_DmaCcs_t*)pContext->runContext.pVirDMADesc;
    DWORD dwIndex = pContext->runContext.dwSSPIndex;
    UINT16 u16ByteCount = uBlockSize*uBlockCount;
    
    pVirDMADesc->pNext = NULL;
    pVirDMADesc->DmaCmd.U = (BF_APBH_CHn_CMD_XFER_COUNT(u16ByteCount) |
                              (BF_APBH_CHn_CMD_CMDWORDS(bCmd ? 5 : 1) |
                               BF_APBH_CHn_CMD_WAIT4ENDCMD(1) |
                               BF_APBH_CHn_CMD_SEMAPHORE(1) |
                               BF_APBH_CHn_CMD_IRQONCMPLT(1) |
                               BF_APBH_CHn_CMD_CHAIN(1) | 
                               BF_APBH_CHn_CMD_COMMAND(bRead ? BV_APBH_CHn_CMD_COMMAND__DMA_WRITE :
                                                               BV_APBH_CHn_CMD_COMMAND__DMA_READ)));

    pVirDMADesc->pDMABuffer = (void *)p32Buffer;

    pVirDMADesc->SspCtrl0.U = HW_SSP_CTRL0_RD(dwIndex);
    pVirDMADesc->Xfer_size.U= u16ByteCount;
    if(uBlockCount == 1)
        pVirDMADesc->Block_size.U = BF_SSP_BLOCK_SIZE_BLOCK_SIZE(0) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(uBlockCount - 1);
    else
        pVirDMADesc->Block_size.U = BF_SSP_BLOCK_SIZE_BLOCK_SIZE(ComputeLog2(uBlockSize)) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(uBlockCount - 1);

    if(bCmd)
    {
        pVirDMADesc->SspCmd.U = HW_SSP_CMD0_RD(dwIndex);
        pVirDMADesc->SspArg.U = HW_SSP_CMD1_RD(dwIndex);
    }

    // enable the interrupt
    if (!pContext->runContext.bCurrentReqFastPath)
        DDKApbhDmaEnableCommandCmpltIrq(Channel, TRUE);


    return DDKApbhStartDma(Channel, pContext->runContext.pPhyDMADesc, 1);
}

//-----------------------------------------------------------------------------
//
// Function:  ssp_mmcsd_StopDma
//
// This function stop SDHC DMA channel .
//
// Parameters:
//      pContext
//          [in] Hardware Context.
//      Channel
//          [in] Channel to be started
//
// Returns:
//      True for success
//
//-----------------------------------------------------------------------------
BOOL ssp_mmcsd_StopDma(PSDHC_HARDWARE_CONTEXT pContext,  UINT8 Channel)
{
    UINT32 u32Sema = (HW_APBH_CHn_SEMA_RD(Channel) & BM_APBH_CHn_SEMA_PHORE);
    UINT32 u32StartMS;

    // poll for completion when fast path is selected
    if (pContext->runContext.bCurrentReqFastPath)
    {

        u32StartMS = HW_DIGCTL_MICROSECONDS_RD();
        do
        {
            u32Sema = (HW_APBH_CHn_SEMA_RD(Channel) & BM_APBH_CHn_SEMA_PHORE);

            // 12000ms (12 s -- based on DATA TIMEOUT value in TIMING register) Timeout
            if(HW_DIGCTL_MICROSECONDS_RD() - u32StartMS > 5000000)
            {

                ERRORMSG(1, (L"SDHC DMA Channel time-out\r\n"));

                DumpSSPRegister(0);
                break;
            }
        }while(u32Sema != 0);
    }

    DDKApbhDmaEnableCommandCmpltIrq(Channel, FALSE);
    DDKApbhDmaClearCommandCmpltIrq(Channel);

    // clear error bit, if set
    switch(Channel)
    {
        case DDK_APBH_CHANNEL_SSP0:  
            HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH0_ERROR_IRQ);
            break;
        case DDK_APBH_CHANNEL_SSP1:  
            HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH1_ERROR_IRQ);
            break;
        case DDK_APBH_CHANNEL_SSP2:  
            HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH2_ERROR_IRQ);
            break;
        case DDK_APBH_CHANNEL_SSP3:  
            HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH3_ERROR_IRQ);
            break;                            
    }
    
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

//-----------------------------------------------------------------------------
//
// Function:  ssp_mmcsd_ResetDma
//
// This function reset SDHC DMA channel .
//
// Parameters:
//      Channel
//          [in] Channel to be started
//
// Returns:
//      True for success
//
//-----------------------------------------------------------------------------
BOOL ssp_mmcsd_ResetDma(UINT8 Channel)
{
    return DDKApbhDmaResetChan(Channel, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
//  SetClockRate - Set the SSP clock 
//  Input: 
//     dwIndex : instance index
//  Output:
//      pRate : 
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
void SetClockRate(DWORD dwIndex, PDWORD pRate)
{   
    UINT32 u32Timing, u32ClkDiv =0, u32ClkRate = 0;
    UINT32 u32SSPFreq, rootfreq = 0; 
    DWORD dwRate = *pRate;
    UINT32 u32Div;
    BOOL   bSkip = FALSE;
    
    switch(dwIndex)
    {
        case 0:
            DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP0, &rootfreq);
            break;
        case 1:
            DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP1, &rootfreq);
            break;
        case 2:
            DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP2, &rootfreq);
            break;
        case 3:
            DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP3, &rootfreq);
            break;
        default:
            DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP0, &rootfreq);
            break;
    }
    
    DEBUGMSG(SDCARD_ZONE_INFO, (_T("rootfreq=0x%x\r\n"),rootfreq));
    
    if(dwRate < rootfreq / 2)
    {
        bSkip = TRUE;   
        DEBUGMSG(SDCARD_ZONE_INFO, (_T("dwRate=0x%x, skip\r\n"),dwRate));
    }
    
    switch(dwRate)
    {
    case SD_DEFAULT_CARD_ID_CLOCK_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD_DEFAULT_CARD_ID_CLOCK_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_60; 
        u32SSPFreq = SD_DEFAULT_CARD_ID_CLOCK_RATE;
        break;

    case SD_LOW_SPEED_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD_LOW_SPEED_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_40; 
        u32SSPFreq = SD_LOW_SPEED_RATE;
        break;

    case MMC_FULL_SPEED_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("MMC_FULL_SPEED_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_2; 
        u32SSPFreq = MMC_FULL_SPEED_RATE;
        break;
    case SD_FULL_SPEED_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD_FULL_SPEED_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_2; 
        u32SSPFreq = SSP_24MHZ_FREQUENCY;
        break;
    case SD_HIGH_SPEED_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD_HIGH_SPEED_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_2; 
        u32SSPFreq = SSP_48MHZ_FREQUENCY;
        break;   
    case MMC_HIGH_SPEED_RATE:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("MMC_HIGH_SPEED_RATE\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_2; 
        u32SSPFreq = SSP_48MHZ_FREQUENCY;
        break;           
    default:
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("Unknown Speed\r\n")));
        u32ClkDiv = SSP_CLOCK_DIVIDE_2; 
        u32SSPFreq = SSP_24MHZ_FREQUENCY;
        break;
    }
    
    if(bSkip)
        goto skip_config;
        
    if(dwIndex == 0 || dwIndex == 1)
    {
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_IO0, &rootfreq);
    }
    else if(dwIndex == 2 || dwIndex == 3)
    {
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_IO1, &rootfreq);
    }
    else
    {
        ERRORMSG(TRUE, (_T("Invalid index:%d\r\n"),dwIndex));
        return;
    }
    
    u32Div = rootfreq / SSP_CLK;
    
    if(dwIndex == 0)
    {
        if (!DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP0, DDK_CLOCK_BAUD_SOURCE_REF_IO0, u32Div))
        {
            return;
        }
        
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP0, &rootfreq);
    }
    else if(dwIndex == 1)
    {
        if (!DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP1, DDK_CLOCK_BAUD_SOURCE_REF_IO0, u32Div))
        {
            return;
        }
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP1, &rootfreq);
    }
    else if(dwIndex == 2)
    {
        if (!DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP2, DDK_CLOCK_BAUD_SOURCE_REF_IO1, u32Div))
        {
            return;
        }
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP2, &rootfreq);
    }
    else
    {
        if (!DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP3, DDK_CLOCK_BAUD_SOURCE_REF_IO1, u32Div))
        {
            return;
        }
        DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP3, &rootfreq);
    }
    
skip_config:
    if(rootfreq / u32SSPFreq / u32ClkDiv >= 1)
        u32ClkRate = (rootfreq / u32SSPFreq + u32ClkDiv - 1) / u32ClkDiv - 1;
    else
        u32ClkRate = 0;
    
    SDHCClockGate(dwIndex, FALSE);
    
    u32Timing = HW_SSP_TIMING_RD(dwIndex);
    u32Timing &= 0xFFFF0000;
    u32Timing |= (u32ClkDiv << 8) | u32ClkRate;
    
    HW_SSP_TIMING_WR(dwIndex, u32Timing);
    
    SDHCClockGate(dwIndex, TRUE);
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SetClockRate: u32Timing= 0x%X, u32ClkDiv = %d, u32SSPFreq = %d\r\n"), u32Timing, u32ClkDiv, u32SSPFreq));
}


//-----------------------------------------------------------------------------
//
// Function:  SDHCGetBaseRegAddr
//
// This function returns the physical base address for the
// SDHC registers based on the device index requested.
//
// Parameters:
//      index
//          [in] Index of the SDHC device requested.
//
// Returns:
//      Physical base address for SDHC registers, or 0 if an
//      invalid index was passed.
//
//-----------------------------------------------------------------------------
DWORD SDHCGetBaseRegAddr(UINT32 index)
{
    switch (index)
    {
        case 0:
            return CSP_BASE_REG_PA_SSP0;
        case 1:
            return CSP_BASE_REG_PA_SSP1;
        case 2:
            return CSP_BASE_REG_PA_SSP2;
        case 3:
            return CSP_BASE_REG_PA_SSP3;
        default:
            return 0;
    }
}

//-----------------------------------------------------------------------------
//
// Function:  SDHCGetDMAChannel
//
// This function returns DMA channel the device index requested.
//
// Parameters:
//      index
//          [in] Index of the SDHC device requested.
//
// Returns:
//      DMA Channel request or 0 if an invalid index was passed.
//
//-----------------------------------------------------------------------------
UINT8 SDHCGetDMAChannel(UINT32 index)
{
    switch (index)
    {
        case 0:
            return DDK_APBH_CHANNEL_SSP0;
        case 1:
            return DDK_APBH_CHANNEL_SSP1;
        case 2:
            return DDK_APBH_CHANNEL_SSP2;
        case 3:
            return DDK_APBH_CHANNEL_SSP3;                
        default:
            return 0;
    }
}


//-----------------------------------------------------------------------------
//
// Function:  SDHCClockGate
//
// This function gate the SSP clock.
//
// Parameters:
//      dwIndex
//          [in] Index of the SDHC device requested.
//      bGate
//          [in] True for Gate, FALSE for not Gate
//
// Returns:
//      True for success
//
//-----------------------------------------------------------------------------
BOOL SDHCClockGate(DWORD dwIndex, BOOL bGate)
{
    DDK_CLOCK_GATE_INDEX gateIndex;
        
    switch (dwIndex)
    {
        case 0:
            gateIndex = DDK_CLOCK_GATE_SSP0_CLK;
            break;
        case 1:
            gateIndex = DDK_CLOCK_GATE_SSP1_CLK;
            break;
        case 2:
            gateIndex = DDK_CLOCK_GATE_SSP2_CLK;
            break;
        case 3:
            gateIndex = DDK_CLOCK_GATE_SSP3_CLK;                
            break;
        default:
            return FALSE;
    }    
    
    EnterCriticalSection(&g_cs[dwIndex]);
    
    if(bGate == FALSE)
    {
        if(dwCount[dwIndex] == 0)
        {
            DDKClockSetGatingMode(gateIndex, FALSE);
        }
        dwCount[dwIndex]++;
    }       
    else
    {
        if(dwCount[dwIndex] == 1)
        {
            DDKClockSetGatingMode(gateIndex, TRUE);
        }
        
        if(dwCount[dwIndex] != 0)
            dwCount[dwIndex]--;
    }
    
    LeaveCriticalSection(&g_cs[dwIndex]);
    
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  SetInterface - Setup the bit mode/clock for the SDHC Controller
//  Input:  
//     pHCContext - Hardware controller context
//     pInterface - interface want to be set
//  Output:
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
void SetInterface(PSDHC_HARDWARE_CONTEXT pContext, PSD_CARD_INTERFACE pInterface)
{
    DWORD dwIndex = pContext->runContext.dwSSPIndex;

    SDHCClockGate(dwIndex, FALSE);
    
    if(SD_INTERFACE_SD_MMC_1BIT == (pInterface->InterfaceMode))
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SetInterface : setting for 1 bit mode \r\n")));

        pContext->runContext.bSDBus4BitMode = FALSE;
        pContext->runContext.bSDBus8BitMode = FALSE;
        HW_SSP_CTRL0_CLR(dwIndex, BF_SSP_CTRL0_BUS_WIDTH(0x3));
        HW_SSP_CTRL0_SET(dwIndex, BF_SSP_CTRL0_BUS_WIDTH(BV_SSP_CTRL0_BUS_WIDTH_ONE_BIT));
    } 
    else if(SD_INTERFACE_SD_4BIT == (pInterface->InterfaceMode))
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SetInterface : setting for 4 bit mode \r\n")));

        pContext->runContext.bSDBus4BitMode = TRUE;
        pContext->runContext.bSDBus8BitMode = FALSE;
        HW_SSP_CTRL0_CLR(dwIndex, BF_SSP_CTRL0_BUS_WIDTH(0x3));
        HW_SSP_CTRL0_SET(dwIndex, BF_SSP_CTRL0_BUS_WIDTH(BV_SSP_CTRL0_BUS_WIDTH_FOUR_BIT));
    }
    else if(SD_INTERFACE_MMC_8BIT == (pInterface->InterfaceMode))
    {
        RETAILMSG(TRUE, (TEXT("SetInterface : setting for 8 bit mode \r\n")));
        pContext->runContext.bSDBus4BitMode = FALSE;
        pContext->runContext.bSDBus8BitMode = TRUE;
        HW_SSP_CTRL0_CLR(dwIndex, BF_SSP_CTRL0_BUS_WIDTH(0x3));
        HW_SSP_CTRL0_SET(dwIndex, BF_SSP_CTRL0_BUS_WIDTH(BV_SSP_CTRL0_BUS_WIDTH_EIGHT_BIT));
    }
    else
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SetInterface : unknown bit mode \r\n")));
    }
    
    // set rate
    SetClockRate(dwIndex, &pInterface->ClockRate);    
    
    SDHCClockGate(dwIndex, TRUE);    
    
}


///////////////////////////////////////////////////////////////////////////////
//  ControllerIst - controller interrupt service thread for driver
//  Input: 
//     pHCContext - Host controller context
//  Output: 
//  Return: Thread exit status
//  Notes:
///////////////////////////////////////////////////////////////////////////////
static DWORD WINAPI ControllerIst(PSDCARD_HC_CONTEXT pHCContext)
{
    DWORD dwWaitTime    = INFINITE;
    PSDHC_HARDWARE_CONTEXT pContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext); 

    if (!CeSetThreadPriority(GetCurrentThread(), 151)) {
        DEBUGMSG(SDCARD_ZONE_WARN, (L"ControllerIst: Warning, failed to set CEThreadPriority\r\n"));
    }

    for(;;) {
        // wait for the controller interrupt
        if (WAIT_OBJECT_0 != WaitForSingleObject(pContext->runContext.hISTEvent, dwWaitTime)) 
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"ControllerIst: Wait Failed!\r\n"));
            break;
        }

        if (pContext->runContext.bDriverShutdown) {      
            DEBUGMSG(SDCARD_ZONE_WARN, (L"ControllerIst: Thread exiting!\r\n"));
            break;
        }

        SDHCDAcquireHCLock(pHCContext);

        HandleInterrupts(pHCContext);
        InterruptDone(pContext->runContext.dwSysIntr );
        SDHCDReleaseHCLock(pHCContext);
    }

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-ControllerIst\r\n"));
    return 0;

}


///////////////////////////////////////////////////////////////////////////////
//  SDDeInitialize - Deinitialize the the SD/MMC Controller
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
    PSDHC_HARDWARE_CONTEXT pContext; // hardware context
    DWORD dwSSPIndex;
    
    pContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("+SDDeinitialize\r\n")));

    if(RunContext.dwSysIntr != SYSINTR_UNDEFINED )
    {
        // disable controller interrupt
        InterruptDisable(RunContext.dwSysIntr);

        // release the SYSINTR value
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &RunContext.dwSysIntr, sizeof(DWORD), NULL, 0, NULL);
        RunContext.dwSysIntr = (DWORD) SYSINTR_UNDEFINED;
    }

    // clean up controller IST
    if (NULL != RunContext.htIST) {
        // wake up the IST
        RunContext.bDriverShutdown = TRUE;
        SetEvent(RunContext.hISTEvent);
        // wait for the thread to exit
        WaitForSingleObject(RunContext.htIST, INFINITE); 
        CloseHandle(RunContext.htIST);
        RunContext.htIST = NULL;
    }

    // free controller interrupt event
    if (NULL !=RunContext. hISTEvent) {
        CloseHandle(RunContext.hISTEvent);
        RunContext.hISTEvent = NULL;
    }

    SDHCCleanupCardDetectIST(pContext);

    // free memory mapped resources
    if(RunContext.dwSSPIndex == 0 && NULL != pv_HWregSSP0)
    {
        MmUnmapIoSpace((PVOID)pv_HWregSSP0, 0x1000);
        pv_HWregSSP0 = NULL;
    }
    else if(RunContext.dwSSPIndex == 1 && NULL != pv_HWregSSP1)
    {
        MmUnmapIoSpace((PVOID)pv_HWregSSP1, 0x1000);
        pv_HWregSSP1 = NULL;
    }
    else if(RunContext.dwSSPIndex == 2 && NULL != pv_HWregSSP2)
    {
        MmUnmapIoSpace((PVOID)pv_HWregSSP2, 0x1000);
        pv_HWregSSP2 = NULL;
    }
    else if(RunContext.dwSSPIndex == 3 && NULL != pv_HWregSSP3)
    {
        MmUnmapIoSpace((PVOID)pv_HWregSSP3, 0x1000);
        pv_HWregSSP3 = NULL;
    }   
    
    if(NULL != RunContext.pVirDMADesc)
    {
        MmUnmapIoSpace((PVOID)RunContext.pPhyDMADesc, SSP_DESC_SIZE);
        RunContext.pVirDMADesc = NULL;
        RunContext. pPhyDMADesc = NULL;
    }

    if(NULL != RunContext.pDMABuffer)
    {
        HalFreeCommonBuffer(&RunContext.dmaAdapter, 0, RunContext.pDMABufferPhys, RunContext.pDMABuffer, FALSE);
        RunContext.pDMABuffer = NULL;
    }

    BSPSDHCDeinit(pContext);

    for(dwSSPIndex = 0; dwSSPIndex < MAX_SLOT; dwSSPIndex++)
    {
        DeleteCriticalSection(&g_cs[dwSSPIndex]);
    }
    
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("-SDDeinitialize\r\n")));
    return SD_API_STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  SSPInitialize - Initialize the the SD/MMC Controller
//  Input:  
//     dwIndex - instance index
//
//  Output:
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
void SSPInitialize(DWORD dwIndex)
{
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("+SSPInitialize\r\n")));

    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_SFTRST);
    while(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_SFTRST)
    {
        ;
    }

    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_CLKGATE);
    while(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_CLKGATE)
    {
        ;
    }

    HW_SSP_CTRL0_SET(dwIndex, BM_SSP_CTRL0_SFTRST);
    while(!(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_CLKGATE))
    {
        ;
    }
    
    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_SFTRST);
    while(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_SFTRST)
    {
        ;
    }

    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_CLKGATE);
    while(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_CLKGATE)
    {
        ;
    }
    
    // Initialize SSP for SD/MMC card
    HW_SSP_TIMING_WR(dwIndex, (BF_SSP_TIMING_TIMEOUT(0xFFFF) |
                     BF_SSP_TIMING_CLOCK_DIVIDE(0xF0) |
                     BF_SSP_TIMING_CLOCK_RATE(0)));

    // WORD_LENGTH=8 + SD_MMC mode
    // SDIO_IRQ will be enabled in the SDIO_INTERRUPT_ON
    HW_SSP_CTRL1_WR(dwIndex, 
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
//  SDInitialize - Initialize the the SD/MMC Controller
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
    PSDHC_HARDWARE_CONTEXT pContext;        // hardware context
    DWORD dwSSPIndex = 0;
    PHYSICAL_ADDRESS SSPPhyAddr;
    PHYSICAL_ADDRESS DescPhyAddr;

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("+SDInitialize\r\n")));
    
    PHYSICAL_ADDRESS CLKMGR_Base = {CSP_BASE_REG_PA_CLKCTRL};
    PHYSICAL_ADDRESS APBH_Base = {CSP_BASE_REG_PA_APBH};
    PHYSICAL_ADDRESS DIGCTL_Base = {CSP_BASE_REG_PA_DIGCTL};
    
    for(dwSSPIndex = 0; dwSSPIndex < MAX_SLOT; dwSSPIndex++)
    {
        InitializeCriticalSection(&g_cs[dwSSPIndex]);
    }
    
    pContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);
    dwSSPIndex = RunContext.dwSSPIndex;
    SSPPhyAddr.QuadPart = SDHCGetBaseRegAddr(dwSSPIndex);

    if(!SSPPhyAddr.QuadPart)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDInitialize:  invalid SDHC instance!\r\n")));
        goto Exit;
    }

    // Map peripheral physical address to virtual address
    switch(dwSSPIndex)
    {
        case 0:
            pv_HWregSSP0 = (PVOID) MmMapIoSpace(SSPPhyAddr, 0x1000, FALSE);
            break;
        case 1:
            pv_HWregSSP1 = (PVOID) MmMapIoSpace(SSPPhyAddr, 0x1000, FALSE);
            break;
        case 2:
            pv_HWregSSP2 = (PVOID) MmMapIoSpace(SSPPhyAddr, 0x1000, FALSE);
            break;
        case 3:
            pv_HWregSSP3 = (PVOID) MmMapIoSpace(SSPPhyAddr, 0x1000, FALSE);
            break;
        default:
            break;
    }
    // Check if virtual mapping failed
    if((pv_HWregSSP0 == NULL) && (pv_HWregSSP1 == NULL) && (pv_HWregSSP2 == NULL) && (pv_HWregSSP3 == NULL))
    { 
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDInitialize:  MmMapIoSpace failed!\r\n")));
        goto Exit;
    }
    
    // Initialize
    RunContext.bSDIOInterruptsEnabled = FALSE;
    RunContext.bCardPresent = FALSE;
    RunContext.bSDBus4BitMode = FALSE;
    RunContext.bSDBus8BitMode = FALSE;
    RunContext.uChannel = SDHCGetDMAChannel(dwSSPIndex);

    // BSP SDHC Initialization
    if (!BSPSDHCInit(pContext))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error initializing SSP hw\r\n")));
        goto Exit;
    }

    // Sysintr initialization
    if (!BSPSDHCSysIntrInit(dwSSPIndex, &(RunContext.dwSysIntr)))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating SD/MMC interrupt\r\n")));
        goto Exit;
    }

    pv_HWregCLKCTRL = (PVOID)MmMapIoSpace(CLKMGR_Base, 0x1000, FALSE);
    if(pv_HWregCLKCTRL == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating Clock control registers\r\n")));
        goto Exit;
    }

    pv_HWregAPBH = (PVOID)MmMapIoSpace(APBH_Base, 0x1000, FALSE);
    if(pv_HWregAPBH == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating APBH registers")));
        goto Exit;
    }

    pv_HWregDIGCTL = (PVOID)MmMapIoSpace(DIGCTL_Base, 0x1000, FALSE);
    if(pv_HWregDIGCTL == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating pv_HwRegDigitCtrl registers")));
        goto Exit;
    }
    
    RunContext.pPhyDMADesc = (PVOID)(BSPSDHCGetDMADescAddress(dwSSPIndex, SSP_DESC_SIZE));
    DescPhyAddr.QuadPart = BSPSDHCGetDMADescAddress(dwSSPIndex, SSP_DESC_SIZE);
    RunContext.pVirDMADesc = MmMapIoSpace(DescPhyAddr, SSP_DESC_SIZE, FALSE);

    RunContext.dmaAdapter.ObjectSize = sizeof(RunContext.dmaAdapter);
    RunContext.dmaAdapter.InterfaceType = Internal;
    RunContext.dmaAdapter.BusNumber = 0;
    RunContext.pDMABuffer = (PBYTE)HalAllocateCommonBuffer(&RunContext.dmaAdapter, MAXIMUM_DMA_TRANSFER_SIZE, &RunContext.pDMABufferPhys, FALSE);
    if(RunContext.pDMABuffer == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SHCDriver: - Unable to allocate memory for DMA buffers!\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    SDHCClockGate(dwSSPIndex, FALSE);

    // SSP
    SSPInitialize(dwSSPIndex);

    // Reset the DMA
    DDKApbhDmaResetChan(RunContext.uChannel,TRUE);
    DDKApbhDmaInitChan(RunContext.uChannel, TRUE);
    
    // allocate the interrupt event for the SSP interrupt
    RunContext.hISTEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if (NULL == RunContext.hISTEvent) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    if ( !InterruptInitialize( RunContext.dwSysIntr, RunContext.hISTEvent, NULL, 0 ) ) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    // create the Controller IST thread
    RunContext.htIST = CreateThread(NULL,
        0,
        (LPTHREAD_START_ROUTINE) ControllerIst,
        pHCContext,
        0,
        NULL);

    if (NULL == RunContext.htIST) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    SDHCSetupCardDetectIST(pContext);

    status = SD_API_STATUS_SUCCESS;

Exit:
    if(!SD_API_SUCCESS(status)) 
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDInitialize : SDDeinitialize\r\n")));
        // just call the deinit handler directly to cleanup
        SDDeinitialize(pHCContext);
    }
    SDHCClockGate(dwSSPIndex, TRUE);
    
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("-SDInitialize : status = 0x%X\r\n"), status));
    return status;
}

///////////////////////////////////////////////////////////////////////////////
//  EnableSDIOInterrupts - Enable SDIO interrupt
//  Input:  
//     pContext - hardware context
//
//  Output:
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
void EnableSDIOInterrupts(PSDHC_HARDWARE_CONTEXT pContext) 
{
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("EnableSDIOInterrupts\r\n")));
    
    SDHCClockGate(RunContext.dwSSPIndex, FALSE);
    
    HW_SSP_CTRL0_SET(RunContext.dwSSPIndex, BM_SSP_CTRL0_SDIO_IRQ_CHECK);
    HW_SSP_CTRL1_SET(RunContext.dwSSPIndex, BM_SSP_CTRL1_SDIO_IRQ_EN);

    //SDHCClockGate(RunContext.dwSSPIndex, TRUE);
    
    RunContext.bSDIOInterruptsEnabled = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  DisableSDIOInterrupts - Disable SDIO interrupt
//  Input:  
//     pContext - hardware context
//
//  Output:
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
void DisableSDIOInterrupts(PSDHC_HARDWARE_CONTEXT pContext)
{
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("DisableSDIOInterrupts\r\n")));
    
    //SDHCClockGate(RunContext.dwSSPIndex, FALSE);
    
    HW_SSP_CTRL0_CLR(RunContext.dwSSPIndex, BM_SSP_CTRL0_SDIO_IRQ_CHECK);
    HW_SSP_CTRL1_CLR(RunContext.dwSSPIndex, BM_SSP_CTRL1_SDIO_IRQ_EN);

    SDHCClockGate(RunContext.dwSSPIndex, TRUE);
    
    RunContext.bSDIOInterruptsEnabled = FALSE;
}


///////////////////////////////////////////////////////////////////////////////
//  ReadReceiveFifo - Read from the received Fifo
//  Input: pContext - the hardware context for transfer  
//          pRequest - the request to get the data from
//          ByteCount - number of bytes to read
//          MaxBytes - limit of this transfer
//  Output:
//  Return:
//  Notes:
//
///////////////////////////////////////////////////////////////////////////////
void ReadReceiveFifo(PSDHC_HARDWARE_CONTEXT pContext, PSD_BUS_REQUEST pRequest, DWORD dwSize)
{   
    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("ReadReceiveFifo : pRequest->HCParam = %d, dwSize = %d\r\n"), pRequest->HCParam, dwSize));

    // wait for DMA completion
    if(!ssp_mmcsd_StopDma(pContext, RunContext.uChannel))
    {
        DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("+ReadReceiveFifo: ddi_ssp_mmcsd_StopDma - failed\r\n")));
    }

    // safely copy the data
    SDPerformSafeCopy(&pRequest->pBlockBuffer[pRequest->HCParam],
                      RunContext.pDMABuffer,
                      dwSize);

    pRequest->HCParam += dwSize;
}


///////////////////////////////////////////////////////////////////////////////
//  WriteTransmitFifo - Write to the transmit fifo
//  Input: pContext - the hardware context for transfer  
//          pController - the controler
//          pRequest    - the request
//  Output:
//  Return:  returns TRUE if the request has been fullfilled
//  Notes:
///////////////////////////////////////////////////////////////////////////////
void WriteTransmitFifo(PSDHC_HARDWARE_CONTEXT pContext, PSD_BUS_REQUEST pRequest, DWORD dwSize)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("WriteTransmitFifo : pRequest->HCParam = %d, dwSize = %d\r\n"), pRequest->HCParam, dwSize));

    // wait for DMA completion
    if(!ssp_mmcsd_StopDma(pContext, RunContext.uChannel)) 
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
    PSDHC_HARDWARE_CONTEXT pContext; // hardware context
    DWORD         dwIndex;

    pContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+HandleEndCommandInterrupt\r\n")));

    // get the current request; if none, then mus tbe interrupt from SDIO card that caused this thread to run
    pRequest = SDHCDGetAndLockCurrentRequest(pHCContext, 0);
    if(NULL == pRequest)
    {
        return;
    }

    dwIndex = RunContext.dwSSPIndex;
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+HandleEndCommandInterrupt - HW_SSP_CTRL1: 0x%x\r\n"), HW_SSP_CTRL1_RD(dwIndex)));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+HandleEndCommandInterrupt - pRequest->TransferClass: 0x%x\r\n"), pRequest->TransferClass));

    // command/response only
    if(SD_COMMAND == pRequest->TransferClass) 
    {
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt COMMAND completed\r\n")));
    } 

    // either we need to poll for DMA completion for fast path, or we got the interrupt because DMA is complete unless SSP is still running
    else if (RunContext.bUseDMA &&  (RunContext.bCurrentReqFastPath || !HW_SSP_CTRL0(dwIndex).B.RUN))
    {
    
        if(SD_READ == pRequest->TransferClass)
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt starting READ TRANSFER of %d blocks of %d bytes\r\n"), pRequest->NumBlocks, pRequest->BlockSize ));

            ReadReceiveFifo(pContext, pRequest, pRequest->NumBlocks * pRequest->BlockSize);
        } 
        else 
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt starting WRITE TRANSFER of %d blocks of %d bytes\r\n"), pRequest->NumBlocks, pRequest->BlockSize ));

            WriteTransmitFifo(pContext, pRequest, pRequest->NumBlocks * pRequest->BlockSize);
        }

        RunContext.bUseDMA = FALSE;
    }

    if(NoResponse != pRequest->CommandResponse.ResponseType) 
    {
        // read in the response words from the response fifo.
        if(ResponseR2 == pRequest->CommandResponse.ResponseType)  
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt - ResponseType: %d\r\n"), pRequest->CommandResponse.ResponseType));

            u32Response = HW_SSP_SDRESP0_RD(dwIndex);
            pRequest->CommandResponse.ResponseBuffer[0] = (UCHAR)(((u32Response >> 0) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[1] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[2] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[3] = (UCHAR)(((u32Response >> 24) & 0xFF));

            u32Response = HW_SSP_SDRESP1_RD(dwIndex);
            pRequest->CommandResponse.ResponseBuffer[4] = (UCHAR)(((u32Response >> 0) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[5] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[6] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[7] = (UCHAR)(((u32Response >> 24) & 0xFF));

            u32Response = HW_SSP_SDRESP2_RD(dwIndex);
            pRequest->CommandResponse.ResponseBuffer[8] = (UCHAR)(((u32Response >> 0) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[9] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[10] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[11] = (UCHAR)(((u32Response >> 24) & 0xFF));


            u32Response = HW_SSP_SDRESP3_RD(dwIndex);
            pRequest->CommandResponse.ResponseBuffer[12] = (UCHAR)(((u32Response >> 0) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[13] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[14] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[15] = (UCHAR)(((u32Response >> 24) & 0xFF));
        } 
        else 
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleEndCommandInterrupt - ResponseType: %d\r\n"), pRequest->CommandResponse.ResponseType));

            u32Response = HW_SSP_SDRESP1_RD(dwIndex);
            pRequest->CommandResponse.ResponseBuffer[5] = (UCHAR)(u32Response & 0x3F );
            u32Response = HW_SSP_SDRESP0_RD(dwIndex);
            pRequest->CommandResponse.ResponseBuffer[4] = (UCHAR)(((u32Response >> 24) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[3] = (UCHAR)(((u32Response >> 16) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[2] = (UCHAR)(((u32Response >> 8) & 0xFF));
            pRequest->CommandResponse.ResponseBuffer[1] = (UCHAR)(((u32Response >> 0) & 0xFF));

            pRequest->CommandResponse.ResponseBuffer[0] = (UCHAR)((0xFF));
        }
    }

    RunContext.cmdStatus = SD_API_STATUS_SUCCESS;
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("-HandleEndCommandInterrupt\r\n")));
}

///////////////////////////////////////////////////////////////////////////////
//  HandleInterrupts - Handle SD/MMC controller interrupt
//  Input:  
//     pHCContext - host controller context
//
//  Output:
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
VOID HandleInterrupts(PSDCARD_HC_CONTEXT pHCContext)
{
    UINT32 u32Ctrl1;
    UINT32 u32Status;
    PSDHC_HARDWARE_CONTEXT pContext; // hardware context
    DWORD         dwIndex;

    pContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);

    dwIndex = RunContext.dwSSPIndex;
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HandleInterrupts\r\n")));

    // read response and complete DMA if needed
    HandleEndCommandInterrupt(pHCContext);

    // check for SDIO card interrupt
    if(RunContext.bSDIOInterruptsEnabled && HW_SSP_CTRL1(dwIndex).B.SDIO_IRQ)
    {
       DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDIO Interrupt\r\n")));

        // disable the SDIO interrupt
        DisableSDIOInterrupts(pContext);

        SDHCClockGate(RunContext.dwSSPIndex, FALSE);
        // clear the interrupt bit
        HW_SSP_CTRL1_SET(dwIndex, BM_SSP_CTRL1_SDIO_IRQ);
                
        // indicate that the card is interrupting
        SDHCDIndicateSlotStateChange(pHCContext, 0, DeviceInterrupting);
    }


    u32Ctrl1 = HW_SSP_CTRL1_RD(dwIndex);
    u32Status = HW_SSP_STATUS_RD(dwIndex);
    
    // RESP_ERR_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_RESP_ERR_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("RESP_ERR_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(dwIndex, BF_SSP_CTRL1_RESP_ERR_IRQ(1));
        RunContext.cmdStatus = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
    }
        
    // RESP_TIMEOUT_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_RESP_TIMEOUT_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("RESP_TIMEOUT_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(dwIndex, BF_SSP_CTRL1_RESP_TIMEOUT_IRQ(1));
        RunContext.cmdStatus = SD_API_STATUS_RESPONSE_TIMEOUT;        
    }

    // DATA_TIMEOUT_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_DATA_TIMEOUT_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("DATA_TIMEOUT_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(dwIndex, BF_SSP_CTRL1_DATA_TIMEOUT_IRQ(1));

        RunContext.cmdStatus = SD_API_STATUS_DATA_TIMEOUT;
    }

    // DATA_CRC_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_DATA_CRC_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("DATA_CRC_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(dwIndex, BF_SSP_CTRL1_DATA_CRC_IRQ(1));
        RunContext.cmdStatus = SD_API_STATUS_CRC_ERROR;
    }

    // FIFO_UNDERRUN_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_FIFO_UNDERRUN_IRQ(1))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("FIFO_UNDERRUN_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(dwIndex, BF_SSP_CTRL1_FIFO_UNDERRUN_IRQ(1));
        RunContext.cmdStatus = SD_API_STATUS_DATA_ERROR;
    }

    // FIFO_OVERRUN_IRQ
    if(u32Ctrl1 & BF_SSP_CTRL1_FIFO_OVERRUN_IRQ(1))
    {
       DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("FIFO_OVERRUN_IRQ\r\n")));
        HW_SSP_CTRL1_CLR(dwIndex, BF_SSP_CTRL1_FIFO_OVERRUN_IRQ(1));
        RunContext.cmdStatus = SD_API_STATUS_DATA_ERROR;
    }
    
    if(!RunContext.bCardPresent)
    {
        RunContext.cmdStatus = SD_API_STATUS_DEVICE_REMOVED;
        PSD_BUS_REQUEST pRequest = SDHCDGetAndLockCurrentRequest(pHCContext, 0);
        if (pRequest)
        {
            SDHCDIndicateBusRequestComplete(pHCContext, pRequest, RunContext.cmdStatus);
        }
        
        DDKApbhDmaEnableCommandCmpltIrq(RunContext.uChannel, FALSE);
        DDKApbhDmaClearCommandCmpltIrq(RunContext.uChannel);
    
        // clear error bit, if set
        switch(RunContext.uChannel)
        {
            case DDK_APBH_CHANNEL_SSP0:  
                HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH0_ERROR_IRQ);
                break;
            case DDK_APBH_CHANNEL_SSP1:  
                HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH1_ERROR_IRQ);
                break;
            case DDK_APBH_CHANNEL_SSP2:  
                HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH2_ERROR_IRQ);
                break;
            case DDK_APBH_CHANNEL_SSP3:  
                HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH3_ERROR_IRQ);
                break;                            
        }
    }
        
    if (!RunContext.bCurrentReqFastPath)
    {

        PSD_BUS_REQUEST pRequest = SDHCDGetAndLockCurrentRequest(pHCContext, 0);
        if (pRequest)
            SDHCDIndicateBusRequestComplete(pHCContext, pRequest, RunContext.cmdStatus);


    }
    SDHCClockGate(dwIndex, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
//  SendCommand - Send commond to SD/MMC 
//  Input:  
//     pContext - hardware context
//     pRequest - SD Bus request command
//  Output:
//  Return: SD_API_STATUS code
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS SendCommand(PSDHC_HARDWARE_CONTEXT pContext, PSD_BUS_REQUEST pRequest)
{     
    UINT32 cmdatRegister = 0;
    DWORD dwIndex = RunContext.dwSSPIndex;

    while((HW_SSP_STATUS_RD(dwIndex) & (BM_SSP_STATUS_BUSY | BM_SSP_STATUS_DATA_BUSY | BM_SSP_STATUS_CMD_BUSY) ))
    {
        Sleep(1);        
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD Bus Busy : HW_SSP_STATUS = 0x%X, DEBUG = 0x%08x\r\n"), HW_SSP_STATUS_RD(dwIndex), HW_SSP_DEBUG_RD(dwIndex)));
        if(!RunContext.bCardPresent)
        {
            return SD_API_STATUS_DEVICE_REMOVED;
        }
    }
    
    // clear CTRL0 and COM0 register first
    HW_SSP_CTRL0_CLR(dwIndex, 0xEFFFFFFF);
    HW_SSP_CMD0_CLR(dwIndex, 0xFFFFFFFF);
    HW_SSP_XFER_SIZE_CLR(dwIndex, 0xFFFFFFFF);
    HW_SSP_BLOCK_SIZE_CLR(dwIndex, 0xFFFFFFF);

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SD Bus SendCommand : HW_SSP_DDR_CTRL_RD = 0x%X\r\n"), HW_SSP_DDR_CTRL_RD(dwIndex)));
    
    HW_SSP_DDR_CTRL_WR(dwIndex, 0x0);
    //HW_SSP_DDR_CTRL_SET(dwIndex, BF_SSP_DDR_CTRL_DMA_BURST_TYPE(2));
    
    // set the command
    HW_SSP_CMD0_SET(dwIndex, BF_SSP_CMD0_CMD(pRequest->CommandCode));
    
    if(RunContext.bMMCDDRMode)
    {
        HW_SSP_CMD0_SET(dwIndex, BF_SSP_CMD0_DBL_DATA_RATE_EN(1));
    }
    else
    {
        HW_SSP_CMD0_CLR(dwIndex, BF_SSP_CMD0_DBL_DATA_RATE_EN(1));
    }
    
    if((pRequest->CommandCode & 0xFF) == SD_CMD_IO_RW_DIRECT ||
       (pRequest->CommandCode & 0xFF) == SD_CMD_IO_RW_EXTENDED)
    {
        HW_SSP_CMD0_SET(dwIndex, BF_SSP_CMD0_APPEND_8CYC(1));
    }
    
    // set the argument
    HW_SSP_CMD1_WR(dwIndex, pRequest->CommandArgument);
    
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
        HW_SSP_CMD0_CLR(dwIndex, 0xFFF00);
        // HW_SSP_CMD0_SET(BF_SSP_CMD0_APPEND_8CYC(1));
    } 
    else 
    {
        // command with a data phase
        cmdatRegister |= BM_SSP_CTRL0_DATA_XFER;
    
        // set the buffer index to the end of the buffer
        pRequest->HCParam = 0;
    
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("pRequest->BlockSize: %d, pRequest->NumBlocks: %d\r\n"), pRequest->BlockSize, pRequest->NumBlocks));
        HW_SSP_CMD0_CLR(dwIndex, 0xFFF00);
    
        if(pRequest->NumBlocks > 1)
        {
            HW_SSP_BLOCK_SIZE_SET(dwIndex, 
                BF_SSP_BLOCK_SIZE_BLOCK_SIZE(ComputeLog2(pRequest->BlockSize)) | 
                BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pRequest->NumBlocks - 1));

            //cmdatRegister |= BF_SSP_CTRL0_XFER_COUNT(pRequest->BlockSize * pRequest->NumBlocks);
            HW_SSP_XFER_SIZE_SET(dwIndex, pRequest->BlockSize * pRequest->NumBlocks);
        }
        else
        {   
            //cmdatRegister |= BF_SSP_CTRL0_XFER_COUNT(pRequest->BlockSize);
            HW_SSP_XFER_SIZE_SET(dwIndex, pRequest->BlockSize);
        }
       
    
        // check for read
        if(SD_READ == pRequest->TransferClass) 
        {
            cmdatRegister |= BM_SSP_CTRL0_READ;
        }
    
    }
    
    // check to see if we need to append the 80 clocks (i.e. this is the first transaction)
    
    // check to see if we need to enable the SDIO interrupt checking
    if( RunContext.bSDIOInterruptsEnabled)
    {
        cmdatRegister |= BM_SSP_CTRL0_SDIO_IRQ_CHECK;
    }
    
    // check to see if we need to enable wide bus (4 bit) data transfer mode
    if( RunContext.bSDBus4BitMode) 
    {
        cmdatRegister |= (BV_SSP_CTRL0_BUS_WIDTH_FOUR_BIT << BP_SSP_CTRL0_BUS_WIDTH);
    }
    
    if( RunContext.bSDBus8BitMode) 
    {
        cmdatRegister |= (BV_SSP_CTRL0_BUS_WIDTH_EIGHT_BIT << BP_SSP_CTRL0_BUS_WIDTH);
    }
    
    cmdatRegister |= (BM_SSP_CTRL0_ENABLE | BM_SSP_CTRL0_WAIT_FOR_IRQ);
    HW_SSP_CTRL1_CLR(dwIndex, 0xAAA08000);
    HW_SSP_CTRL0_WR(dwIndex, cmdatRegister);
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SendCommand - CMDAT Reg : 0x%08X, CMD : %d\r\n"), cmdatRegister, pRequest->CommandCode));
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SendCommand : %d\r\n"), pRequest->CommandCode));
     
    if((cmdatRegister & BM_SSP_CTRL0_DATA_XFER) == 0)
    {
        HW_SSP_CTRL0_SET(dwIndex, BM_SSP_CTRL0_RUN);
    }

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("HW_SSP_STATUS = 0x%X\r\nHW_SSP_CMD0 = 0x%X\r\nHW_SSP_CMD1 = 0x%X\r\nHW_SSP_CTRL0 = 0x%X\r\nHW_SSP_CTRL1 = 0x%X\r\n"), 
                    HW_SSP_STATUS_RD(dwIndex), HW_SSP_CMD0_RD(dwIndex), HW_SSP_CMD1_RD(dwIndex),
                    HW_SSP_CTRL0_RD(dwIndex), HW_SSP_CTRL1_RD(dwIndex)));
    

    // poll for completion of command
    if (pRequest->TransferClass == SD_COMMAND)
    {
        RunContext.bUseDMA = FALSE;
        while(HW_SSP_CTRL0(dwIndex).B.RUN)
        {
            if(!RunContext.bCardPresent)
            {
                return SD_API_STATUS_DEVICE_REMOVED;
            }
        }
    }

    // start the DMA transfer
    else
    {    

        if (pRequest->TransferClass == SD_WRITE)
        {
            // perform the safe copy to DMA buffer before starting DMA for writes
            SDPerformSafeCopy(RunContext.pDMABuffer,
                              &pRequest->pBlockBuffer[pRequest->HCParam],
                              pRequest->BlockSize * pRequest->NumBlocks);
        }
    
        if (!ssp_mmcsd_StartDma(pContext, RunContext.uChannel, (pRequest->TransferClass == SD_READ), TRUE,  
                                          (UINT16)(pRequest->BlockSize), (UINT16) pRequest->NumBlocks, 
                                          (UINT32 *)RunContext.pDMABufferPhys.LowPart))
        {
            return SD_API_STATUS_DATA_ERROR;
        }

        RunContext.bUseDMA = TRUE;
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
    PSDHC_HARDWARE_CONTEXT pContext;         // the controller
    DWORD dwIndex;

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+SDHCBusRequestHandler\r\n")));
    // get our extension
    pContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);
    dwIndex = RunContext.dwSSPIndex;
        
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHCBusRequestHandler - CMD: %02d DATA: 0x%08X, TC: %d\r\n"),
                     pRequest->CommandCode, pRequest->CommandArgument, pRequest->TransferClass));
       

    SDHCDAcquireHCLock(pHCContext);

    SDHCClockGate(dwIndex, FALSE);
    
    if (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)
    {
        RunContext.bCurrentReqFastPath = TRUE;

        // mask interrupts at the core
        InterruptMask(RunContext.dwSysIntr,TRUE);

        retVal = SendCommand(pContext, pRequest);
        if ( retVal == SD_API_STATUS_PENDING ) { // Polling for completion.
                HandleInterrupts(pHCContext);
                
            retVal = RunContext.cmdStatus;
        }

        // unmask interrupt at the core
        InterruptMask(RunContext.dwSysIntr,FALSE);
        
        RunContext.bCurrentReqFastPath = FALSE;

    }
        
    else
    {
        retVal = SendCommand(pContext, pRequest);


        // for command only non-fast path requests, there will be no interrupts if there is no error, so set the interrupt event to grab the response
        if (pRequest->TransferClass == SD_COMMAND && 
             (!(HW_SSP_CTRL1_RD(dwIndex) & (BM_SSP_CTRL1_RESP_ERR_IRQ | BM_SSP_CTRL1_RESP_TIMEOUT_IRQ))) )
        {
            SetEvent(RunContext.hISTEvent);
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
    PSDHC_HARDWARE_CONTEXT pContext;         // the controller

    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("+SDHCSlotOptionHandler\r\n")));
    // get our extension
    pContext = GetExtensionFromHCDContext(PSDHC_HARDWARE_CONTEXT, pHCContext);

    switch (Option) 
    {
    case SDHCDSetSlotPower:
        
        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - SetSlotPower : 0x%08X\r\n"), *((PDWORD)pData)));
        break;

    case SDHCDSetSlotInterface:
        
        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - SetSlotInterface : Clock Setting: %d\r\n"), ((PSD_CARD_INTERFACE)pData)->ClockRate));
        {
            PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;
            SetInterface(pContext, pInterface);
        }
        break;
    case SDHCDSetSlotInterfaceEx:
            {
                PSD_CARD_INTERFACE_EX pInterfaceEx = (PSD_CARD_INTERFACE_EX)pData;
                SD_CARD_INTERFACE Interface = {
                    pInterfaceEx->InterfaceModeEx.bit.mmc8Bit!=0? SD_INTERFACE_MMC_8BIT :
                        (pInterfaceEx->InterfaceModeEx.bit.sd4Bit!=0?SD_INTERFACE_SD_4BIT: SD_INTERFACE_SD_MMC_1BIT),
                    pInterfaceEx->ClockRate,
                    pInterfaceEx->InterfaceModeEx.bit.sdWriteProtected!=0
                };
                SetInterface(pContext, &Interface);
            }
            break;
    case SDHCDEnableSDIOInterrupts:
    case SDHCDAckSDIOInterrupt:
            
        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - EnableSDIOInterrupts/SDHCDAckSDIOInterrupt\r\n"), SlotNumber));

        EnableSDIOInterrupts(pContext);
            
        break;

    case SDHCDDisableSDIOInterrupts:

        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - DisableSDIOInterrupts : on slot %d\r\n"), SlotNumber));

        DisableSDIOInterrupts(pContext);

        break;

    case SDHCDGetWriteProtectStatus:

        RETAILMSG(_DUMP_SSP, (TEXT("SDHCSlotOptionHandler - SDHCDGetWriteProtectStatus : on slot %d\r\n"), SlotNumber));
        {
            PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;
            pInterface->WriteProtected = BSPSDHCIsWriteProtected(RunContext.dwSSPIndex);
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
                                                 SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK |
                                                (BSPSDHCSupport8Bit(SlotNumber) ? SD_SLOT_MMC_8BIT_CAPABLE : 0) |
                                                SD_SLOT_HIGH_SPEED_CAPABLE));

            SDHCDSetVoltageWindowMask(pSlotInfo, (SD_VDD_WINDOW_2_9_TO_3_0 | 
                                                  SD_VDD_WINDOW_3_0_TO_3_1 | 
                                                  SD_VDD_WINDOW_3_1_TO_3_2));

            // Set optimal voltage
            SDHCDSetDesiredSlotVoltage(pSlotInfo, SD_VDD_WINDOW_3_0_TO_3_1);

            SDHCDSetMaxClockRate(pSlotInfo, 60000000);

            // Set power up delay. We handle this in SetVoltage().
            SDHCDSetPowerUpDelay(pSlotInfo, 250);   // 300
        }
        break;

    default:
        status = SD_API_STATUS_INVALID_PARAMETER;
        break;
    }
    
    //belowing is for eMMC4.4 DDR mode
    if((SD_SLOT_OPTION_CODE_EXT)Option == SDHCDEnableDDRMode)
    {
        if(BSPSDHCSupportDDRMode(pContext->runContext.dwSSPIndex))
        {
            BSPSDHCEnableDDRMode(pContext);
            HW_SSP_CTRL1_CLR(pContext->runContext.dwSSPIndex, BM_SSP_CTRL1_POLARITY);
            status = SD_API_STATUS_SUCCESS;
        }
    }
    
    if((SD_SLOT_OPTION_CODE_EXT)Option == SDHCDDisableDDRMode)
    {
        if(BSPSDHCSupportDDRMode(pContext->runContext.dwSSPIndex))
        {
            BSPSDHCDisableDDRMode(pContext);
            HW_SSP_CTRL1_SET(pContext->runContext.dwSSPIndex, BM_SSP_CTRL1_POLARITY);
            status = SD_API_STATUS_SUCCESS;
        }
    }
    
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("-SDHCSlotOptionHandler\r\n")));
    return status;
}

///////////////////////////////////////////////////////////////////////////////
//  ProcessCardInsertion - deal with card insertion  
//  Input:  
//     pHardwareContext - hardware context
//  Output:
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
void ProcessCardInsertion(void *pHardwareContext )
{
    DWORD dwDefaultClock = SD_DEFAULT_CARD_ID_CLOCK_RATE;    
    PSDHC_HARDWARE_CONTEXT pContext= (PSDHC_HARDWARE_CONTEXT)pHardwareContext;
    DWORD dwIndex = RunContext.dwSSPIndex;
    
    if(!RunContext.bCardPresent) 
    {
        DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("ProcessCardInsertion\r\n")));
        // mark that the card is in the slot
        RunContext.bCardPresent = TRUE;
        BSPSDHCDisableDDRMode(pContext);
        
        SDHCClockGate(dwIndex, FALSE);
        
        HW_SSP_CTRL1_SET(dwIndex, BM_SSP_CTRL1_POLARITY);
        // set rate
        SetClockRate(dwIndex, &dwDefaultClock);

        // need to send at least 74 clocks to SDIO cards to allow initialization, so turn on continuous clocking
        HW_SSP_CMD0_SET(dwIndex, BM_SSP_CMD0_CONT_CLKING_EN);

        // give the card some time for initialization
        Sleep(100);

        // turn off the continuous clocking mode, so clk will only run when cmd or data are being processed
        HW_SSP_CMD0_CLR(dwIndex, BM_SSP_CMD0_CONT_CLKING_EN);        

        // turn off SDIO interrupts
        if(RunContext.bSDIOInterruptsEnabled)
        {
            DisableSDIOInterrupts(pContext);
        }
        
        SDHCClockGate(dwIndex, TRUE);

        // indicate the slot change
        SDHCDIndicateSlotStateChange(pContext->pHCContext,
                                     0,
                                     DeviceInserted);
    }
}

///////////////////////////////////////////////////////////////////////////////
//  ProcessCardRemoval - deal with card removal
//  Input:  
//     pHardwareContext - hardware context
//  Output:
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
void ProcessCardRemoval(void *pHardwareContext)
{
    PSDHC_HARDWARE_CONTEXT pContext = (PSDHC_HARDWARE_CONTEXT)pHardwareContext;
    if(RunContext.bCardPresent)
    {
        DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("ProcessCardRemoval\r\n")));

        // mark that the card has been removed
        RunContext.bCardPresent = FALSE;
        BSPSDHCDisableDDRMode(pContext);
        
        SetEvent(RunContext.hISTEvent);
        
        // turn off SDIO interrupts
        if(RunContext.bSDIOInterruptsEnabled)
        {
            DisableSDIOInterrupts(pContext);
        }

        // indicate the slot change
        SDHCDIndicateSlotStateChange(pContext->pHCContext,
                                     0,
                                     DeviceEjected);
#ifdef MSCDSK
        // In MX2x, the card removal processing is moved here
        // So we need also duplicate the usb mass storage processing
        // code here
        {
            HANDLE hSDDisk = NULL;
            BOOL bHandleCloseSD = FALSE;
            DWORD dwErr = 0;
            hSDDisk = CreateEvent(NULL, FALSE, FALSE, L"MSC_SDDISK");
            if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseSD = TRUE;

            dwErr = WaitForSingleObject(hSDDisk, 0);
            if (dwErr != WAIT_TIMEOUT)
            {
                //means event is set
                HANDLE hDetachEvent = NULL;
                BOOL bHandleCloseDetach = FALSE;
                hDetachEvent = CreateEvent(NULL, FALSE, FALSE, L"MSC_DETACH_EVENT");
                if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseDetach = TRUE;

                if (hSDDisk) ResetEvent(hSDDisk);
                if (hDetachEvent) SetEvent(hDetachEvent);

                if (hDetachEvent && bHandleCloseDetach) CloseHandle(hDetachEvent);

            }
            if (hSDDisk && bHandleCloseSD) CloseHandle(hSDDisk);
        }
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////
//  SDHCCardDetectThread - routine for card detect 
//  Input:  
//     pHWContext - hardware context
//  Output:
//  Return: 
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
static DWORD SDHCCardDetectThread(void *pHWContext)
{
    DWORD dwStatus;
    PSDHC_HARDWARE_CONTEXT pContext = (PSDHC_HARDWARE_CONTEXT)pHWContext;
    DWORD dwIndex = RunContext.dwSSPIndex;

    RunContext.bCardDetectThreadRunning = TRUE;
    
    // Check in case that the card is already inserted
    if(BSPSDHCIsCardPresent(dwIndex))
    {
        SetEvent(RunContext.hCardDetectEvent);
    }

    while (RunContext.bCardDetectThreadRunning)
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("Wait for hCardDetectEvent\r\n")));
   
        dwStatus = WaitForSingleObject(RunContext.hCardDetectEvent, INFINITE);
        if(WAIT_OBJECT_0 != dwStatus) 
        {
            continue;
        }

        if (!RunContext.bCardDetectThreadRunning)
            break;

        SDHCDAcquireHCLock(((PSDHC_HARDWARE_CONTEXT)pContext)->pHCContext);

        if(RunContext.bReinsertTheCard) 
        {
            ProcessCardRemoval(pContext);
            RunContext.bReinsertTheCard = FALSE;
        }

        BSPSDHCCardDetectThread(pContext);
        
        // Enable the interrupt
        InterruptDone(RunContext.dwCardDetectSysintr);

        SDHCDReleaseHCLock(((PSDHC_HARDWARE_CONTEXT)pContext)->pHCContext);
        
    }

    return 0;
    
}

///////////////////////////////////////////////////////////////////////////////
//  SDHCSetupCardDetectIST - setup card detect IST 
//  Input:  
//     pHardwareContext - hardware context
//  Output:
//  Return: TRUE for success
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
BOOL SDHCSetupCardDetectIST(void *pHardwareContext)
{
    //DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("+SDHCSetupCardDetectIST\n")));
    
    DWORD dwSDCDIrq ;
    PSDHC_HARDWARE_CONTEXT pContext = (PSDHC_HARDWARE_CONTEXT)pHardwareContext;
    DWORD dwIndex = RunContext.dwSSPIndex;
    
    if (!BSPGetCardDetectIRQ(dwIndex, &dwSDCDIrq))
    {
        DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHCSetupCardDetectIST : not support card detect function \r\n")));
        return TRUE;
    }
    
    if(!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwSDCDIrq, sizeof(dwSDCDIrq), &RunContext.dwCardDetectSysintr, sizeof(DWORD), NULL))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("Error obtaining card detect SYSINTR value!\n")));
        RunContext.dwCardDetectSysintr = (DWORD)SYSINTR_UNDEFINED;
        return FALSE;
    }       
    
    // Create Interrupt Event
    RunContext.hCardDetectEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == RunContext.hCardDetectEvent) 
    {
        return FALSE;
    }

    // Register the Interrupt
    if(!InterruptInitialize(RunContext.dwCardDetectSysintr,
                            RunContext.hCardDetectEvent,
                            NULL,
                            0))
    {
        return FALSE;
    }

    // Thread to process the card detection
    RunContext.hCardDetectThread = CreateThread(NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)SDHCCardDetectThread,
                                    pHardwareContext,
                                    0,
                                    NULL);
    if(NULL == RunContext.hCardDetectThread) 
    {
        return FALSE;
    }
    else
    {
        CeSetThreadPriority(RunContext.hCardDetectThread, SDHC_CD_THREAD_PRIORITY);
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  SDHCCleanupCardDetectIST - cleanup card detect IST 
//  Input:  
//     pHardwareContext - hardware context
//  Output:
//  Return: None
//  Notes:
//
//
///////////////////////////////////////////////////////////////////////////////
static void SDHCCleanupCardDetectIST(void *pHardwareContext)
{
    PSDHC_HARDWARE_CONTEXT pContext = (PSDHC_HARDWARE_CONTEXT)pHardwareContext;
    //DWORD dwIndex = RunContext.dwSSPIndex;
    
    InterruptDisable(RunContext.dwCardDetectSysintr);
    
    RunContext.bCardDetectThreadRunning = FALSE;
    
    if (RunContext.hCardDetectEvent)
        SetEvent(RunContext.hCardDetectEvent);

    // Close the Thread
    if (NULL != RunContext.hCardDetectThread) 
    {
        WaitForSingleObject(RunContext.hCardDetectThread, INFINITE);
        CloseHandle(RunContext.hCardDetectThread);
        RunContext.hCardDetectThread = NULL;
    }
        
    // Close the Event
    if (NULL != RunContext.hCardDetectEvent) 
    {
        CloseHandle(RunContext.hCardDetectEvent);
        RunContext.hCardDetectEvent = NULL;
    }
}
