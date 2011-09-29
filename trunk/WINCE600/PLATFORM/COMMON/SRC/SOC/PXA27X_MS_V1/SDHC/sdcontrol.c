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
//
// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE
//
// Module Name:
//
//    SDControl.c
//
// Abstract:
//
//    PXA27X SDIO controller implementation
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////

//#define EXTENSIVE_DEBUGGING

#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <celog.h>
#include <giisr.h>
#include "SDCardDDK.h"
#include "SDHCD.h"
#include "SD.h"

#ifdef CELOGENABLE
#define _CeLogEnable TRUE
#else
#define _CeLogEnable FALSE
#endif 

    // prototypes
PVOID VirtualAllocCopy(unsigned size,char *str,PVOID pVirtualAddress);
DWORD SDControllerIstThread(PSDH_HARDWARE_CONTEXT pHCDevice);
DWORD SDDMAIstThread(PSDH_HARDWARE_CONTEXT pHCDevice);
#ifdef DEBUG
void DumpRegisters(PSDH_HARDWARE_CONTEXT pController);
void DumpGPIORegisters(PSDH_HARDWARE_CONTEXT pController);
#endif
BOOL PrepareDmaTransfer( PSDH_HARDWARE_CONTEXT pController, PSD_BUS_REQUEST pRequest );

#ifdef DEBUG
#define HEXBUFSIZE 1024
char szHexBuf[HEXBUFSIZE];

#define TRANSFER_SIZE(pRequest)            ((pRequest)->BlockSize * (pRequest)->NumBlocks)

char* HexDisplay( BYTE *pBuffer, DWORD dwLength )
{
    DWORD dwTemp = 0;
	while( dwTemp < dwLength && (dwTemp < (HEXBUFSIZE / 2 - 1) ) )
	{
		szHexBuf[dwTemp*2] = pBuffer[dwTemp] / 16;
		szHexBuf[dwTemp*2+1] = pBuffer[dwTemp] % 16;
		
		if( szHexBuf[dwTemp*2] < 10 )
			szHexBuf[dwTemp*2] += '0';
		else
			szHexBuf[dwTemp*2] += 'a' - 10;

		if( szHexBuf[dwTemp*2+1] < 10 )
			szHexBuf[dwTemp*2+1] += '0';
		else
			szHexBuf[dwTemp*2+1] += 'a' - 10;

		dwTemp++;
	}
	szHexBuf[dwTemp*2] = 0;

    return szHexBuf;
}

void DumpHexDisplay( BYTE *pBuffer, DWORD dwLength )
{
    const int lsize = 64;
    char szBuffer[65];
    char* pszData;
    int l = 0;
    int total_l = dwLength / lsize;
    int r = dwLength % lsize;

    pszData = HexDisplay( pBuffer, dwLength );

    for( l = 0; l < total_l; l++ )
    {
        strncpy( szBuffer, pszData + l * lsize, lsize );
        szBuffer[lsize] = 0;
        DEBUGMSG( TRUE, (TEXT("%S\r\n"), szBuffer ) );
    }
    if( r )
    {
        strncpy( szBuffer, pszData + l * lsize, r );
        szBuffer[r] = 0;
        DEBUGMSG( TRUE, (TEXT("%S\r\n"), szBuffer ) );
    }
}

#endif

__inline void WRITE_MMC_REGISTER_DWORD(PSDH_HARDWARE_CONTEXT pHc, DWORD RegOffset, DWORD Value) 
{
    BYTE *pRegBaseAddr, *regAddr;
    volatile DWORD *pdwRegAddr;
    pRegBaseAddr = (BYTE*)(pHc->pSDMMCRegisters);
    regAddr = pRegBaseAddr + RegOffset;
    pdwRegAddr = (DWORD*)regAddr;
    *pdwRegAddr = Value;
}

__inline DWORD READ_MMC_REGISTER_DWORD(PSDH_HARDWARE_CONTEXT pHc, DWORD RegOffset)
{
    BYTE *pRegBaseAddr, *regAddr;
    volatile DWORD *pdwRegAddr;
    pRegBaseAddr = (BYTE*)(pHc->pSDMMCRegisters);
    regAddr = pRegBaseAddr + RegOffset;
    pdwRegAddr = (DWORD*)regAddr;
    return (*pdwRegAddr);
}

#define IS_PROGRAM_DONE(pHc) (READ_MMC_REGISTER_DWORD((pHc), MMC_STAT) & MMC_STAT_PROGRAM_DONE)
#define IS_TRANSFER_DONE(pHc) (READ_MMC_REGISTER_DWORD((pHc), MMC_STAT) & MMC_STAT_DATA_TRANSFER_DONE)

//#define RX_FIFO_FULL(pHc) (READ_MMC_REGISTER_DWORD((pHc), MMC_STAT) & MMC_STAT_RCV_FIFO_FULL)
//#define TX_FIFO_EMPTY(pHc) (READ_MMC_REGISTER_DWORD((pHc), MMC_STAT) & MMC_STAT_XMIT_FIFO_EMPTY)

#define TRANSFER_IS_WRITE(pRequest) ((SD_WRITE == (pRequest)->TransferClass))
#define TRANSFER_IS_READ(pRequest)  ((SD_READ == (pRequest)->TransferClass))

BOOL CLOCK_IS_ON(PSDH_HARDWARE_CONTEXT pHc)
{
    if( READ_MMC_REGISTER_DWORD( pHc, MMC_STAT ) & MMC_STAT_CLOCK_ENABLED )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

__inline void READ_MOD_WRITE_MMC_REGISTER_AND_OR(PSDH_HARDWARE_CONTEXT pHc, DWORD RegOffset, DWORD AndValue, DWORD OrValue)
{
    DWORD regValue;
    regValue = READ_MMC_REGISTER_DWORD(pHc, RegOffset);
    regValue &= (AndValue);
    regValue |= (OrValue);
    DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SHCDriver: - Setting MMC Reg 0x%x to 0x%x\r\n"), RegOffset, regValue));
    WRITE_MMC_REGISTER_DWORD(pHc, RegOffset, regValue);
}

__inline void WRITE_MMC_IMASK_DWORD(PSDH_HARDWARE_CONTEXT pHc, DWORD Value) 
{
    EnterCriticalSection(&(pHc->intrRegCriticalSection));
    WRITE_MMC_REGISTER_DWORD( pHc, MMC_IMASK, Value);
    LeaveCriticalSection(&(pHc->intrRegCriticalSection));
}

__inline DWORD READ_MMC_IMASK_DWORD(PSDH_HARDWARE_CONTEXT pHc)
{
    DWORD dwRetVal;
    EnterCriticalSection(&(pHc->intrRegCriticalSection));
    dwRetVal = READ_MMC_REGISTER_DWORD( pHc, MMC_IMASK );
    LeaveCriticalSection(&(pHc->intrRegCriticalSection));
    return dwRetVal;
}

__inline void READ_MOD_WRITE_MMC_IMASK_AND_OR(PSDH_HARDWARE_CONTEXT pHc, DWORD AndValue, DWORD OrValue)
{
    EnterCriticalSection(&(pHc->intrRegCriticalSection));
    READ_MOD_WRITE_MMC_REGISTER_AND_OR( pHc, MMC_IMASK, AndValue, OrValue );
    LeaveCriticalSection(&(pHc->intrRegCriticalSection));
}

#define CLOCK_OFF_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_CLOCK_OFF_INT_MASKED)
 
    // macro to turn SDIO interrupts on
#define SDIO_INTERRUPT_ON(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, ~MMC_IMASK_SDIO_INT_MASKED, 0)

    // macro to turn SDIO interrupts off
#define SDIO_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_SDIO_INT_MASKED)

    // macro to turn RX FIFO interrupts on
#define RX_FIFO_INTERRUPT_ON(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, ~MMC_IMASK_RXFIFO_REQ_INT_MASKED, 0)

    // macro to turn RX FIFO interrupts off
#define RX_FIFO_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_RXFIFO_REQ_INT_MASKED)

    // macro to turn TX FIFO interrupts on
#define TX_FIFO_INTERRUPT_ON(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, ~MMC_IMASK_TXFIFO_REQ_INT_MASKED, 0)

    // macro to turn TX FIFO interrupts off
#define TX_FIFO_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_TXFIFO_REQ_INT_MASKED)

#define TRANSFER_DONE_INTERRUPT_ON(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, ~MMC_IMASK_DATA_TRAN_DONE_INT_MASKED, 0) 

#define TRANSFER_DONE_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_DATA_TRAN_DONE_INT_MASKED) 

#define END_CMD_INTERRUPT_ON(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, ~MMC_IMASK_END_CMD_INT_MASKED, 0) 

#define END_CMD_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_END_CMD_INT_MASKED) 

#define PROGRAM_DONE_INTERRUPT_ON(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, ~MMC_IMASK_PROG_DONE_INT_MASKED, 0) 

#define PROGRAM_DONE_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_PROG_DONE_INT_MASKED) 

#define PROGRAM_DATA_ERROR_INTERRUPT_ON(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, ~MMC_IMASK_DATA_ERROR_INT_MASKED, 0) 

#define PROGRAM_DATA_ERROR_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_DATA_ERROR_INT_MASKED) 

#define PROGRAM_RESPONSE_ERROR_INTERRUPT_ON(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, ~MMC_IMASK_RESPONSE_ERROR_INT_MASKED, 0) 

#define PROGRAM_RESPONSE_ERROR_INTERRUPT_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_RESPONSE_ERROR_INT_MASKED) 

#define TX_BUFFER_PARTIAL_FULL(pHc) \
    WRITE_MMC_REGISTER_DWORD(pHc, MMC_PRTBUF, MMC_PRTBUF_BUFFER_PARTIAL_FULL)

#define TX_BUFFER_PARTIAL_NOT_FULL(pHc) \
    WRITE_MMC_REGISTER_DWORD(pHc, MMC_PRTBUF, 0)

#define ALL_INTERRUPTS_OFF(pHc) \
    READ_MOD_WRITE_MMC_IMASK_AND_OR(pHc, 0xffffffff, MMC_IMASK_ALL_INTERRUPTS_MASKED) 

    // clock rate table
typedef struct _CLOCK_RATE_ENTRY {
    DWORD Frequency;
    UCHAR ControlValue;
} CLOCK_RATE_ENTRY, *PCLOCK_RATE_ENTRY;

CLOCK_RATE_ENTRY SDClockTable[] = 
{   {312500,  0x06},
    {625000,   0x05},
    {1250000,  0x04},
    {2500000,  0x03},
    {5000000,  0x02},
    {10000000, 0x01},
    {20000000, 0x00},   // 20 Mhz
};

#define NUM_CLOCK_ENTRIES sizeof(SDClockTable)/sizeof(CLOCK_RATE_ENTRY)

///////////////////////////////////////////////////////////////////////////////
//  SDClockOff - turn off the MMC clock
//  Input:  pHc - hardware context
//  Output: 
//  Return:
//  Notes:  
//
///////////////////////////////////////////////////////////////////////////////
VOID SDClockOff(PSDH_HARDWARE_CONTEXT pHc)
{
        // check to see if the clock is on
    if (!CLOCK_IS_ON(pHc)) {
        return;
    }

    DbgPrintZo(SDH_CLOCK_ZONE, (TEXT("SDClockOff - turning off clock \n")));
        // turn off the clock
    WRITE_MMC_REGISTER_DWORD(pHc, MMC_STRPCL, MMC_STRPCL_STOP_CLOCK);
       
    while (CLOCK_IS_ON(pHc)) {
        // sit here and wait for the clock to turn off
        DbgPrintZo(SDH_CLOCK_ZONE, (TEXT("Waiting for clock off\n")));
    }

    DbgPrintZo(SDH_CLOCK_ZONE, (TEXT("SDClockOff - Clock is now off \n")));
}


///////////////////////////////////////////////////////////////////////////////
//  SDClockOn - turn on the MMC Clock
//  Input:  pHc - hardware context
//  Output: 
//  Return:
//  Notes:  
//          
//
///////////////////////////////////////////////////////////////////////////////
VOID SDClockOn(PSDH_HARDWARE_CONTEXT pHc)
{
        // turn on the clock
    WRITE_MMC_REGISTER_DWORD(pHc, MMC_STRPCL, MMC_STRPCL_START_CLOCK);
}


///////////////////////////////////////////////////////////////////////////////
//  SDClockOn - turn on the MMC Clock
//  Input:  pHc - hardware context
//          pRate - pointer to desired clock rate in Hz
//  Output:
//  Return: 
//  Notes:  
//
///////////////////////////////////////////////////////////////////////////////
VOID SDSetRate(PSDH_HARDWARE_CONTEXT pHc, PDWORD pRate)
{
    ULONG ii;           // table index variable
    DWORD rate;
    BOOL fClockRunning;

    fClockRunning = CLOCK_IS_ON(pHc);
    SDClockOff(pHc);

    rate = *pRate;
    if( rate > pHc->dwMaximumSDClockFrequency )
        rate = pHc->dwMaximumSDClockFrequency;
   
        // check to see if the rate is below the first entry in the table
    if (rate <= SDClockTable[0].Frequency) {
        ii = 0;
    } else {

            // scan through the table looking for a frequency that
            // is close to the requested rate
        for (ii = 0; ii < (NUM_CLOCK_ENTRIES - 1); ii++) {
            if ((rate >= SDClockTable[ii].Frequency) &&
                (rate < SDClockTable[ii+1].Frequency)) {
                break;
            } 
        }
    }

    DbgPrintZo(SDH_CLOCK_ZONE, (TEXT("SDClockOn - Requested Rate: %d, Setting clock rate to %d Hz \n"),
           *pRate, SDClockTable[ii].Frequency ));

        // return the actual fruency
    *pRate = SDClockTable[ii].Frequency;

         // set the clock rate
    WRITE_MMC_REGISTER_DWORD(pHc, MMC_CLKRT, SDClockTable[ii].ControlValue);
    pHc->dwSDClockFrequency = SDClockTable[ii].Frequency;
        
    if( fClockRunning )
    {
        SDClockOn( pHc );
    }
}
static BOOL IndicateBusRequestComplete(PSDCARD_HC_CONTEXT pHCContext,
                                    PSD_BUS_REQUEST pRequest,
                                    SD_API_STATUS      Status)
{
    BOOL fRet = FALSE;
    PSDH_HARDWARE_CONTEXT pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);
    if (pController && pController->pCurrentRequest == pRequest) {
        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("IndicateBusRequestComplete - pRequest = %x, Status = %d\n"),pRequest,Status));
        pController->pCurrentRequest = NULL;
        if (pController->fCurrentRequestFastPath ) {
            if (Status == SD_API_STATUS_SUCCESS) {
                Status = SD_API_STATUS_FAST_PATH_SUCCESS;
            }
            pController->FastPathStatus = Status ;
        }
        else 
            SDHCDIndicateBusRequestComplete(pHCContext,pRequest,Status);
        fRet = TRUE;
    }
    ASSERT(fRet);
    return fRet;
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
    PSDH_HARDWARE_CONTEXT pHardwareContext; // hardware context
    PSD_BUS_REQUEST     pRequest = NULL;       // the request to complete

    pHardwareContext = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

        // mark for shutdown
    pHardwareContext->DriverShutdown = TRUE;

    if( pHardwareContext->hControllerInterruptEvent )
    {
        // wake up the controller IST
        SetEvent(pHardwareContext->hControllerInterruptEvent);
    }

    if (NULL != pHardwareContext->hDMAInterruptEvent) 
    {
        // wake up the DMA IST
        SetEvent(pHardwareContext->hDMAInterruptEvent);
    }

    // clean up controller IST
    if (NULL != pHardwareContext->hControllerInterruptThread) {
            // wait for the thread to exit
        WaitForSingleObject(pHardwareContext->hControllerInterruptThread, INFINITE); 
        CloseHandle(pHardwareContext->hControllerInterruptThread);
        pHardwareContext->hControllerInterruptThread = NULL;
    }
        
    // clean up DMA IST
    if (NULL != pHardwareContext->hDmaInterruptThread) {
        // wait for the thread to exit
        WaitForSingleObject(pHardwareContext->hDmaInterruptThread, INFINITE); 
        CloseHandle(pHardwareContext->hDmaInterruptThread);
        pHardwareContext->hDmaInterruptThread = NULL;
    }
        
        // free controller interrupt event
    if (NULL != pHardwareContext->hControllerInterruptEvent) {
        CloseHandle(pHardwareContext->hControllerInterruptEvent);
        pHardwareContext->hControllerInterruptEvent = NULL;
    }

        // free the DMA interrupt event
    if( pHardwareContext->hDMAInterruptEvent )
    {
        CloseHandle(pHardwareContext->hDMAInterruptEvent);
        pHardwareContext->hDMAInterruptEvent = NULL;
    }

        // make sure all interrupt sources are disabled
    if( pHardwareContext->dwSysintrSDMMC != SYSINTR_UNDEFINED )
    {
        InterruptDisable (pHardwareContext->dwSysintrSDMMC);
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pHardwareContext->dwSysintrSDMMC, sizeof(DWORD), NULL, 0, NULL);
        pHardwareContext->dwSysintrSDMMC = SYSINTR_UNDEFINED;
    }
    if( pHardwareContext->dwDmaSysIntr != SYSINTR_UNDEFINED )
    {
        InterruptDisable (pHardwareContext->dwDmaSysIntr);
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pHardwareContext->dwDmaSysIntr, sizeof(DWORD), NULL, 0, NULL);
        pHardwareContext->dwDmaSysIntr = SYSINTR_UNDEFINED;
    }

        // unload the DMA ISR DLL
    if( pHardwareContext->hDMAIsrHandler )
    {
        FreeIntChainHandler(pHardwareContext->hDMAIsrHandler);
        pHardwareContext->hDMAIsrHandler = NULL;
    }

        // free the DMA buffer
    if( pHardwareContext->pDMABuffer )
    {
        HalFreeCommonBuffer( NULL, 0, pHardwareContext->pDMABufferPhys, pHardwareContext->pDMABuffer, FALSE );
        pHardwareContext->pDMABuffer = NULL;
        pHardwareContext->dwDmaBufferSize = 0;
    }

        // free the DMA descriptors buffer
    if( pHardwareContext->pDMADescriptors )
    {
        DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SHCDriver: - Unable to allocate memory for DMA descriptors!\r\n")));
        HalFreeCommonBuffer( NULL, 0, pHardwareContext->pDMADescriptorsPhys, (PVOID)pHardwareContext->pDMADescriptors, FALSE );
        pHardwareContext->pDMADescriptors = NULL;
    }

        // clean up card insertion IST and free card insertion interrupt
    CleanupCardDetectIST();

        // turn the hardware off
    SDClockOff( pHardwareContext );
    MMCPowerControl( FALSE );

        // if there is a pending request, cancel it
    if( pHardwareContext && (pRequest = pHardwareContext->pCurrentRequest) != NULL)
    {
        DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCD:SHCDriver() - aborting current request!\r\n")));
        IndicateBusRequestComplete(pHCContext, pRequest, SD_API_STATUS_SHUT_DOWN);
    }


        // If a card is inserted, signal that it was ejected...
    if(pHardwareContext && pHardwareContext->DevicePresent)
    {
        // indicate the slot change 
        SDHCDIndicateSlotStateChange(pHCContext, 0, DeviceEjected); 
        pHardwareContext->DevicePresent = FALSE;
    }

        // free memory mapped resources

    if (NULL != pHardwareContext->pSDMMCRegisters) {
        MmUnmapIoSpace((PVOID)pHardwareContext->pSDMMCRegisters, sizeof(BULVERDE_MMC_REG));
        pHardwareContext->pSDMMCRegisters = NULL;
    }

    if (NULL != pHardwareContext->pGPIORegisters) {
        MmUnmapIoSpace((PVOID)pHardwareContext->pGPIORegisters, sizeof(BULVERDE_GPIO_REG));
        pHardwareContext->pGPIORegisters = NULL;
    }

    if (NULL != pHardwareContext->pClkMgrRegisters) {
        MmUnmapIoSpace((PVOID)pHardwareContext->pClkMgrRegisters, sizeof(BULVERDE_CLKMGR_REG));
        pHardwareContext->pClkMgrRegisters = NULL;
    }

    if (NULL != pHardwareContext->pDMARegisters) {
        MmUnmapIoSpace((PVOID)pHardwareContext->pDMARegisters, sizeof(BULVERDE_DMA_REG));
        pHardwareContext->pDMARegisters = NULL;
    }

    if(NULL != pHardwareContext->hBusAccessHandle) {
        CloseBusAccessHandle(pHardwareContext->hBusAccessHandle);
        pHardwareContext->hBusAccessHandle = NULL;
    }
    UnInitializeHardware();

    DeleteCriticalSection(&pHardwareContext->ControllerCriticalSection);
    DeleteCriticalSection(&pHardwareContext->intrRegCriticalSection);

    return SD_API_STATUS_SUCCESS;
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
    ULONG fInIOSpace;
    DWORD dwSDIOIrq;
    DWORD dwRegVal;                                 // intermediate value
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // intermediate status
    DWORD         threadID;                         // thread ID
    PSDH_HARDWARE_CONTEXT pHardwareContext;       // hardware context
    PHYSICAL_ADDRESS Bulverde_GPIO_Base = {BULVERDE_BASE_REG_PA_GPIO};
    PHYSICAL_ADDRESS Bulverde_SDMMC_Base = {BULVERDE_BASE_REG_PA_MMC};
    PHYSICAL_ADDRESS Bulverde_CLKMGR_Base = {BULVERDE_BASE_REG_PA_CLKMGR};
    PHYSICAL_ADDRESS Bulverde_DMA_Base = {BULVERDE_BASE_REG_PA_DMAC};

    pHardwareContext = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    InitializeCriticalSection(&pHardwareContext->ControllerCriticalSection);

    pHardwareContext->fSDIOEnabled = FALSE;
    pHardwareContext->pCurrentRequest = NULL ;
    pHardwareContext->fSDIOInterruptPending = FALSE;
    pHardwareContext->f4BitMode = FALSE;
    pHardwareContext->DevicePresent = FALSE;
    pHardwareContext->hDMAInterruptEvent = NULL;
    pHardwareContext->hDMAIsrHandler = NULL;
    pHardwareContext->DriverShutdown = FALSE;
    pHardwareContext->hDMAInterruptEvent = NULL;
    pHardwareContext->hDmaInterruptThread = NULL;
    pHardwareContext->pDMABuffer = NULL;
    pHardwareContext->pDMADescriptors = NULL;
    pHardwareContext->dwControllerIstTimeout = INFINITE;
    InitializeCriticalSection(&pHardwareContext->intrRegCriticalSection);

#ifdef DEBUG
    pHardwareContext->fDMATransferInProgress = FALSE;
#endif
    
    GetSystemInfo( &pHardwareContext->systemInfo );

    if( !InitializeHardware( pHardwareContext->hBusAccessHandle ) )
    {
		DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error initializing platform specific hardware\r\n")));
        goto exitInit;
    }

    if( !BusTransBusAddrToVirtual( pHardwareContext->hBusAccessHandle, Internal, 0, Bulverde_GPIO_Base, sizeof(BULVERDE_GPIO_REG), &fInIOSpace, (PPVOID)&(pHardwareContext->pGPIORegisters) ) )
	{
		DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating Bulverde GPIO registers\r\n")));
        goto exitInit;
	}

    if( !BusTransBusAddrToVirtual( pHardwareContext->hBusAccessHandle, Internal, 0, Bulverde_SDMMC_Base, sizeof(BULVERDE_MMC_REG), &fInIOSpace, (PPVOID)&(pHardwareContext->pSDMMCRegisters) ) )
	{
		DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating Bulverde SD/MMC registers\r\n")));
        goto exitInit;
	}

    if( !BusTransBusAddrToVirtual( pHardwareContext->hBusAccessHandle, Internal, 0, Bulverde_CLKMGR_Base, sizeof(BULVERDE_CLKMGR_REG), &fInIOSpace, (PPVOID)&(pHardwareContext->pClkMgrRegisters) ) )
	{
		DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating Bulverde Clock control registers\r\n")));
        goto exitInit;
	}

    if( !BusTransBusAddrToVirtual( pHardwareContext->hBusAccessHandle, Internal, 0, Bulverde_DMA_Base, sizeof(BULVERDE_DMA_REG), &fInIOSpace, (PPVOID)&(pHardwareContext->pDMARegisters) ) )
	{
		DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("InitializeHardware:: Error allocating Bulverde DMA control registers\r\n")));
        goto exitInit;
	}

    // enable the MMC Unit Clock
    dwRegVal = pHardwareContext->pClkMgrRegisters->cken;
    dwRegVal |= (1 << 12);
    pHardwareContext->pClkMgrRegisters->cken = dwRegVal;

    //////////////////////////////////////////////////////////
    // Configure GPIO_32 as Alternate Function 2 out (MMC_CLK)

    // assume that the MMC_CLK is active-low signal driven
    dwRegVal = pHardwareContext->pGPIORegisters->GPCR1;
    dwRegVal |= 0x00000001;
    pHardwareContext->pGPIORegisters->GPCR1 = dwRegVal;
    // change the direction to OUT
    dwRegVal = pHardwareContext->pGPIORegisters->GPDR1;
    dwRegVal |= 0x00000001;
    pHardwareContext->pGPIORegisters->GPDR1 = dwRegVal;
    // change to Alternate Function 2
    dwRegVal = pHardwareContext->pGPIORegisters->GAFR1_L;
    dwRegVal = ( dwRegVal & 0xfffffffc ) | 0x00000002;
    pHardwareContext->pGPIORegisters->GAFR1_L = dwRegVal;
    
    //////////////////////////////////////////////////////////
    // Configure GPIO_112 as Alternate Function 1 (MMC_CMD)

    // assume that the MMC_CLK is active-high signal driven
    dwRegVal = pHardwareContext->pGPIORegisters->GPSR3;
    dwRegVal |= 0x00010000;
    pHardwareContext->pGPIORegisters->GPSR3 = dwRegVal;
    // change the direction to OUT
    dwRegVal = pHardwareContext->pGPIORegisters->GPDR3;
    dwRegVal |= 0x00010000;
    pHardwareContext->pGPIORegisters->GPDR3 = dwRegVal;
    // change to Alternate Function 1
    dwRegVal = pHardwareContext->pGPIORegisters->GAFR3_U;
    dwRegVal = ( dwRegVal & 0xfffffffc ) | 0x00000001;
    pHardwareContext->pGPIORegisters->GAFR3_U = dwRegVal;
    
    //////////////////////////////////////////////////////////
    // Configure GPIO_92 as Alternate Function 1 (MMC_DAT0)
    
    // assume that the MMC_CLK is active-high signal driven
    dwRegVal = pHardwareContext->pGPIORegisters->GPSR2;
    dwRegVal |= 0x10000000;
    pHardwareContext->pGPIORegisters->GPSR2 = dwRegVal;
    // change the direction to OUT
    dwRegVal = pHardwareContext->pGPIORegisters->GPDR2;
    dwRegVal |= 0x10000000;
    pHardwareContext->pGPIORegisters->GPDR2 = dwRegVal;
    // change to Alternate Function 1
    dwRegVal = pHardwareContext->pGPIORegisters->GAFR2_U;
    dwRegVal = ( dwRegVal & 0xfcffffff ) | 0x01000000;
    pHardwareContext->pGPIORegisters->GAFR2_U = dwRegVal;
    
    //////////////////////////////////////////////////////////
    // Configure GPIO_109-GPIO_111 as Alternate Function 1 (MMC_DAT1-MMC_DAT3)

    // assume that the MMC_CLK is active-high signal driven
    dwRegVal = pHardwareContext->pGPIORegisters->GPSR3;
    dwRegVal |= 0x0000e000;
    pHardwareContext->pGPIORegisters->GPSR3 = dwRegVal;
    // change the direction to OUT
    dwRegVal = pHardwareContext->pGPIORegisters->GPDR3;
    dwRegVal |= 0x0000e000;
    pHardwareContext->pGPIORegisters->GPDR3 = dwRegVal;
    // change to Alternate Function 1
    dwRegVal = pHardwareContext->pGPIORegisters->GAFR3_L;
    dwRegVal = ( dwRegVal & 0x03ffffff ) | 0x54000000;
    pHardwareContext->pGPIORegisters->GAFR3_L = dwRegVal;

#ifdef DEBUG
    DumpRegisters( pHardwareContext );
    DumpGPIORegisters( pHardwareContext );
#endif

    // allocate the interrupt event
    pHardwareContext->hControllerInterruptEvent = CreateEvent(NULL, FALSE, FALSE,NULL);
    
    if (NULL == pHardwareContext->hControllerInterruptEvent) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

        // convert the hardware SD/MMC controller interrupt IRQ into a logical SYSINTR value
    dwSDIOIrq = pHardwareContext->dwSDMMCIrq;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwSDIOIrq, sizeof(DWORD), &(pHardwareContext->dwSysintrSDMMC), sizeof(DWORD), NULL))
    {
        // invalid SDIO SYSINTR value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("Error obtaining SDIO SYSINTR value!\n")));
        pHardwareContext->dwSysintrSDMMC = SYSINTR_UNDEFINED;
        goto exitInit;
    }

        // initialize the interrupt event
    if (!InterruptInitialize (pHardwareContext->dwSysintrSDMMC,
                              pHardwareContext->hControllerInterruptEvent,
                              NULL,
                              0)) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    if( pHardwareContext->dwDmaChannel != 0xffffffff )
    {
        // allocate the DMA interrupt event
        pHardwareContext->hDMAInterruptEvent = CreateEvent(NULL, FALSE, FALSE,NULL);

        if( pHardwareContext->dwDmaSysIntr == SYSINTR_UNDEFINED )
        {
                // convert the hardware DMA controller interrupt IRQ into a logical SYSINTR value
            DWORD dwDMAIrq = pHardwareContext->dwDmaIRQ;
            if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwDMAIrq, sizeof(DWORD), &(pHardwareContext->dwDmaSysIntr), sizeof(DWORD), NULL))
            {
                // invalid SDIO SYSINTR value!
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("Error obtaining DMA SYSINTR value!\n")));
                pHardwareContext->dwDmaSysIntr = SYSINTR_UNDEFINED;
                status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
                goto exitInit;
            }
        }

        // allocate the DMA data & descriptors buffers
        if( pHardwareContext->dwDmaBufferSize )
        {
            DMA_ADAPTER_OBJECT dmaAdapter;
            dmaAdapter.ObjectSize = sizeof(dmaAdapter);
            dmaAdapter.InterfaceType = Internal;
            dmaAdapter.BusNumber = 0;
            pHardwareContext->pDMABuffer = (PBYTE)HalAllocateCommonBuffer( &dmaAdapter, 
                                                                           pHardwareContext->dwDmaBufferSize,
                                                                           &pHardwareContext->pDMABufferPhys,
                                                                           FALSE );
            if( pHardwareContext->pDMABuffer == NULL )
            {
                DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SHCDriver: - Unable to allocate memory for DMA buffers!\r\n")));
                pHardwareContext->dwDmaBufferSize = 0;
            }
            else
            { // allocate DMA descriptors
                DWORD dwDescriptorsSize = pHardwareContext->dwDmaBufferSize / pHardwareContext->systemInfo.dwPageSize;
                if( pHardwareContext->dwDmaBufferSize % pHardwareContext->systemInfo.dwPageSize )
                {
                    dwDescriptorsSize++;
                }
                dwDescriptorsSize *= sizeof(DMADescriptorChannelType);
                
                pHardwareContext->pDMADescriptors = (volatile DMADescriptorChannelType*)HalAllocateCommonBuffer( &dmaAdapter, 
                                                                               dwDescriptorsSize,
                                                                               &pHardwareContext->pDMADescriptorsPhys,
                                                                               FALSE );
                if( pHardwareContext->pDMADescriptors == NULL )
                {
                    DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SHCDriver: - Unable to allocate memory for DMA descriptors!\r\n")));
                    HalFreeCommonBuffer( &dmaAdapter, 
                                           pHardwareContext->dwDmaBufferSize,
                                           pHardwareContext->pDMABufferPhys,
                                           pHardwareContext->pDMABuffer,
                                           FALSE );
                    pHardwareContext->pDMABuffer = NULL;
                    pHardwareContext->dwDmaBufferSize = 0;
                }
            }
        }

            // install the DMA ISR handler
        if( pHardwareContext->wszDmaIsrDll[0] )
        {
            GIISR_INFO Info;
            PVOID PhysAddr;
            DWORD inIoSpace = 0;    // io space
            PHYSICAL_ADDRESS DmaRegisterAddress = {DMA_INTERRUPT_REGISTER, 0}; 
            
            pHardwareContext->hDMAIsrHandler = LoadIntChainHandler(pHardwareContext->wszDmaIsrDll, 
                                                                     pHardwareContext->wszDmaIsrHandler,
                                                                     (BYTE)pHardwareContext->dwDmaIRQ);
            if (pHardwareContext->hDMAIsrHandler == NULL) {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("LoadIntChainHandler (%s, %s, %d) failed!\r\n"),
                                            pHardwareContext->wszDmaIsrDll, 
                                            pHardwareContext->wszDmaIsrHandler,
                                            (BYTE)pHardwareContext->dwDmaIRQ));
                status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
                goto exitInit;
            }

            if (!BusTransBusAddrToStatic(pHardwareContext->hBusAccessHandle, Internal, 0, DmaRegisterAddress, sizeof(DWORD), &inIoSpace, &PhysAddr)) {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"SDHC: Failed TransBusAddrToStatic\r\n"));
                status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
                goto exitInit;
            }
            
            DEBUGMSG(SDCARD_ZONE_INIT, (L"SDHC: Installed ISR handler, Dll = '%s', Handler = '%s', Irq = %d, PhysAddr = 0x%x\r\n", 
                pHardwareContext->wszDmaIsrDll, pHardwareContext->wszDmaIsrHandler, pHardwareContext->dwDmaIRQ, PhysAddr));

            // Set up ISR handler
            Info.SysIntr = pHardwareContext->dwDmaSysIntr;
            Info.CheckPort = TRUE;
            Info.PortIsIO = FALSE;
            Info.UseMaskReg = FALSE;
            Info.PortAddr = (DWORD)PhysAddr;
            Info.PortSize = sizeof(DWORD);
            Info.Mask = 1 << pHardwareContext->dwDmaChannel;
            
            if (!KernelLibIoControl(pHardwareContext->hDMAIsrHandler, IOCTL_GIISR_INFO, &Info, sizeof(Info), NULL, 0, NULL)) {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"SDHC: KernelLibIoControl call failed.\r\n"));
            }
        }

            // initialize the DMA interrupt event
        if (!InterruptInitialize (pHardwareContext->dwDmaSysIntr,
                                  pHardwareContext->hDMAInterruptEvent,
                                  NULL,
                                  0)) {
            status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
            goto exitInit;
        }

            // create the interrupt thread for controller interrupts
        pHardwareContext->hDmaInterruptThread = CreateThread(NULL,
                                                          0,
                                                          (LPTHREAD_START_ROUTINE)SDDMAIstThread,
                                                          pHardwareContext,
                                                          0,
                                                          &threadID);

        if (NULL == pHardwareContext->hDmaInterruptThread) {
            status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
            goto exitInit;
        }
    }

        // create the interrupt thread for controller interrupts
    pHardwareContext->hControllerInterruptThread = CreateThread(NULL,
                                                      0,
                                                      (LPTHREAD_START_ROUTINE)SDControllerIstThread,
                                                      pHardwareContext,
                                                      0,
                                                      &threadID);

    if (NULL == pHardwareContext->hControllerInterruptThread) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    if (!SetupCardDetectIST(pHardwareContext))
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
    }

exitInit:

    if (!SD_API_SUCCESS(status)) {
            // just call the deinit handler directly to cleanup
        SDDeinitialize(pHCContext);
    }

    return status;

}

///////////////////////////////////////////////////////////////////////////////
//  SDHCancelIoHandler - io cancel handler 
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
BOOLEAN SDHCancelIoHandler(PSDCARD_HC_CONTEXT pHCContext, 
                             DWORD              Slot, 
                             PSD_BUS_REQUEST    pRequest)
{
    PSDH_HARDWARE_CONTEXT    pController;

        // for now, we should never get here because all requests are non-cancelable
        // the hardware supports timeouts so it is impossible for the controller to get stuck
    DEBUG_ASSERT(FALSE);

        // get our extension 
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

        // --- Stop hardware, cancel the request!

        // release the lock before we complete the request
    SDHCDReleaseHCLock(pHCContext);
 
        // complete the request with a cancelled status
    IndicateBusRequestComplete(pHCContext,
                                    pRequest,
                                    SD_API_STATUS_CANCELED);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  SDBusIssueRequest - bus request handler 
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
SD_API_STATUS SDBusIssueRequest(PSDCARD_HC_CONTEXT pHCContext, 
                                     DWORD              Slot, 
                                     PSD_BUS_REQUEST    pRequest) 
{

    BOOL fExtraDelay = FALSE;
    PSDH_HARDWARE_CONTEXT    pController;     // our controller
    DWORD                      cmdatRegister;   // CMDAT register

        // get our extension 
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("SDBusIssueRequest - pRequest = %x, CMD: 0x%02X DATA: 0x%08X, TC: %d\n"),
            pRequest,pRequest->CommandCode, pRequest->CommandArgument, pRequest->TransferClass));


        // stop the clock
    SDClockOff(pController);

        // set the command
    WRITE_MMC_REGISTER_DWORD(pController, MMC_CMD, pRequest->CommandCode);
        // set the argument,  high part
    WRITE_MMC_REGISTER_DWORD(pController, MMC_ARGH, (pRequest->CommandArgument >> 16));
        // set the argument,  high part
    WRITE_MMC_REGISTER_DWORD(pController, MMC_ARGL, (pRequest->CommandArgument & 0x0000FFFF));


    switch (pRequest->CommandResponse.ResponseType) {

        case NoResponse:
            cmdatRegister = MMC_CMDAT_RESPONSE_NONE;
            break;
        case ResponseR1b:
                // response1 with busy signalling
            cmdatRegister = MMC_CMDAT_RESPONSE_R1 | MMC_CMDAT_EXPECT_BUSY;
            break;
        case ResponseR1:
        case ResponseR5:
        case ResponseR6:
                // on an MMC controller R5 and R6 are really just an R1 response (CRC protected)
            cmdatRegister = MMC_CMDAT_RESPONSE_R1;
            break;
        case ResponseR2:    
            cmdatRegister = MMC_CMDAT_RESPONSE_R2;
            break;
        case ResponseR3:
        case ResponseR4:    
                // R4 is really same as an R3 response on an MMC controller (non-CRC)
            cmdatRegister = MMC_CMDAT_RESPONSE_R3;
            break;

        default:
            DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("SDBusIssueRequest failed (Invalid parameter)\n")));
            return SD_API_STATUS_INVALID_PARAMETER;
    }

    pController->fDMATransfer = FALSE;
    pController->fDMATransferCancelled = FALSE;

        // check for Command Only
    if ((SD_COMMAND == pRequest->TransferClass)) {
       
            // set the length of the block
        WRITE_MMC_REGISTER_DWORD(pController, MMC_BLKLEN, 0);

            // set the number of blocks
        WRITE_MMC_REGISTER_DWORD(pController, MMC_NOB, 0);

    } else {
            // its a command with a data phase
        cmdatRegister |= MMC_CMDAT_DATA_EN;
        
            // set the buffer index to the end of the buffer
        pRequest->HCParam = 0;      

            // set the length of the block
        WRITE_MMC_REGISTER_DWORD(pController, MMC_BLKLEN, pRequest->BlockSize);

            // set the number of blocks
        WRITE_MMC_REGISTER_DWORD(pController, MMC_NOB, pRequest->NumBlocks);

            // check for write
        if (TRANSFER_IS_WRITE(pRequest)) {
            cmdatRegister |= MMC_CMDAT_DATA_WRITE;
        } 

        // check to see if we can use DMA for data transfer
        if( PrepareDmaTransfer( pController, pRequest ) )
        {
            cmdatRegister |= MMC_CMDAT_DMA_ENABLE;
            pController->fDMATransfer = TRUE;
        }
    }

        // check to see if we need to append the 80 clocks (i.e. this is the first transaction)
    if (pController->SendInitClocks) {
        pController->SendInitClocks = FALSE;
        cmdatRegister |= MMC_CMDAT_INIT;
        fExtraDelay = TRUE;
    }

        // check to see if we need to enable the SDIO interrupt checking
    if (pController->fSDIOEnabled) {
        cmdatRegister |= MMC_CMDAT_SDIO_INT_EN;
    }

        // check to see if we need to enable wide bus (4 bit) data transfer mode
    if (pController->f4BitMode) {
        cmdatRegister |= MMC_CMDAT_SD_4DAT;
    }

    

        // write the CMDAT register
    WRITE_MMC_REGISTER_DWORD(pController, MMC_CMDAT, cmdatRegister);
    DbgPrintZo(SDH_SEND_ZONE, (TEXT("SDBusIssueRequest - CMDAT Reg: 0x%08X, CMD:%d \n"),
                cmdatRegister, pRequest->CommandCode));

        // set the the response timeout
    WRITE_MMC_REGISTER_DWORD(pController, MMC_RESTO, SDH_DEFAULT_RESPONSE_TIMEOUT_CLOCKS);
        // set the data receive timeout
    WRITE_MMC_REGISTER_DWORD(pController, MMC_RDTO, SDH_DEFAULT_DATA_TIMEOUT_CLOCKS);

    SetCurrentState(pController, CommandSend);
        // turn on the command complete and the response error interrupts
    END_CMD_INTERRUPT_ON(pController);
    PROGRAM_RESPONSE_ERROR_INTERRUPT_ON(pController);

        // turn on the clock 
    SDClockOn(pController);

    if( fExtraDelay )
    {
        fExtraDelay = FALSE;
        Sleep(500);
    }
       
#if DEBUG
    {
        DWORD mmcStatus;

        mmcStatus = READ_MMC_REGISTER_DWORD(pController, MMC_STAT);

        if (mmcStatus & 0x0000003F) {
                // these errors should be cleared
            DbgPrintZo(SDCARD_ZONE_ERROR, 
                (TEXT("********* SDBusIssueRequest - MMC Status did not clear : 0x%08X \n"),
                (mmcStatus & 0x0000003F)));
        }
    
    }
#endif 
    DbgPrintZo(SDH_SEND_ZONE, (TEXT("SDBusIssueRequest - Request Sent\n")));

    return SD_API_STATUS_PENDING;
}
#define NUM_BYTE_FOR_FAST_PASS 0x1000
BOOL SDControllerISTHandler(PSDH_HARDWARE_CONTEXT pHCDevice, BOOL fTimeOut);
VOID HandleDMAInterrupt(PSDH_HARDWARE_CONTEXT pController);
///////////////////////////////////////////////////////////////////////////////
//  SDHBusRequestHandler - bus request handler 
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
SD_API_STATUS SDHBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext, 
                                     DWORD              Slot, 
                                     PSD_BUS_REQUEST    pRequest) 
{
    SD_API_STATUS status;
    PSDH_HARDWARE_CONTEXT      pController;     // our controller
    BOOL fHandled = FALSE;
    const TCHAR inData[] = TEXT("SDHBusRequestHandler IN");
    const TCHAR OutData[] = TEXT("SDHBusRequestHandler Out");
    DEBUGMSG(SDCARD_ZONE_FUNC,(TEXT("+SDHBusRequestHandler pRequest=%x"),pRequest));
    RETAILCELOG(_CeLogEnable,CELID_RAW_WCHAR, (PVOID)inData, sizeof(inData));
        // get our extension 
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);
    ACQUIRE_LOCK(pController);
    if ( pController->pCurrentRequest) { // We have outstand request.
        ASSERT(FALSE);
        IndicateBusRequestComplete(pHCContext, pRequest, SD_API_STATUS_CANCELED);
        pController->pCurrentRequest = NULL;
    }
    pController->fCurrentRequestFastPath = FALSE;
    pController->pCurrentRequest = pRequest ;
    // if no data transfer involved, use FAST PATH
    if (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE && 
            !( SD_COMMAND != pRequest->TransferClass && 
                pRequest->NumBlocks * pRequest->BlockSize >=  pController->dwPollingModeSize)){   // We do fast path here.
        pController->fCurrentRequestFastPath = TRUE;
        InterruptMask(pController->dwSysintrSDMMC,TRUE);
        InterruptMask(pController->dwDmaSysIntr,TRUE);
        status = SDBusIssueRequest( pHCContext, Slot, pRequest );
        if( status == SD_API_STATUS_PENDING ) { // Polling for completion.
            while (pController->pCurrentRequest) {
                SDControllerISTHandler(pController, !IsCardPresent());
                if ( pController->fDMATransfer  && !pController->fDMATransferCancelled)
                    HandleDMAInterrupt(pController);
            }               
            status = pController->FastPathStatus;
            if (status == SD_API_STATUS_SUCCESS) {
                status = SD_API_STATUS_FAST_PATH_SUCCESS;
            }
        }
        InterruptMask(pController->dwDmaSysIntr,FALSE);
        InterruptMask(pController->dwSysintrSDMMC,FALSE);
        ASSERT(pController->fCurrentRequestFastPath);

        fHandled = TRUE;
    }
    else {
        pRequest->SystemFlags &= ~SD_FAST_PATH_AVAILABLE ;
        status = SDBusIssueRequest( pHCContext, Slot, pRequest );
    }
    RELEASE_LOCK(pController);
    RETAILCELOG(_CeLogEnable,CELID_RAW_WCHAR, (PVOID)OutData, sizeof(OutData));
    DEBUGMSG(SDCARD_ZONE_FUNC,(TEXT("-SDHBusRequestHandler pRequest=%x"),pRequest));
    return status;
}
///////////////////////////////////////////////////////////////////////////////
//  SDHSlotOptionHandler - handler for slot option changes
//  Input:  pHostContext - host controller context
//          SlotNumber   - the slot the change is being applied to
//          Option       - the option code
//          pData        - data associated with the option
//          OptionSize   - size of option data
//  Output: 
//  Return: SD_API_STATUS code
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS SDHSlotOptionHandler(PSDCARD_HC_CONTEXT    pHCContext,
                                     DWORD                 SlotNumber, 
                                     SD_SLOT_OPTION_CODE   Option, 
                                     PVOID                 pData,
                                     ULONG                 OptionSize)
{
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // status
    PSDH_HARDWARE_CONTEXT    pController;         // the controller
    PSD_HOST_BLOCK_CAPABILITY  pBlockCaps;          // queried block capabilities

        // get our extension 
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    switch (Option) {

        case SDHCDSetSlotPower:
            DbgPrintZo(SDCARD_ZONE_INIT, 
                (TEXT("SDHSlotOptionHandler - called - SetSlotPower : 0x%08X  \n"), 
                *((PDWORD)pData)));
            break;

        case SDHCDSetSlotInterface:
            DbgPrintZo(SDCARD_ZONE_INIT, 
                (TEXT("SDHSlotOptionHandler - called - SetSlotInterface : Clock Setting: %d \n"), 
                ((PSD_CARD_INTERFACE)pData)->ClockRate));
            
            if (SD_INTERFACE_SD_MMC_1BIT == 
                ((PSD_CARD_INTERFACE)pData)->InterfaceMode) {
                DbgPrintZo(SDCARD_ZONE_INIT, 
                        (TEXT("SDHSlotOptionHandler - called - SetSlotInterface : setting for 1 bit mode \n")));
                pController->f4BitMode = FALSE;
            } else {
                DbgPrintZo(SDCARD_ZONE_INIT, 
                        (TEXT("SDHSlotOptionHandler - called - SetSlotInterface : setting for 4 bit mode \n")));
                pController->f4BitMode = TRUE;
            }
                // set rate
            SDSetRate(pController, &((PSD_CARD_INTERFACE)pData)->ClockRate);
            
            break;

        case SDHCDEnableSDIOInterrupts:
            
            DbgPrintZo(SDCARD_ZONE_INIT, 
                (TEXT("SDHSlotOptionHandler - called - EnableSDIOInterrupts : on slot %d  \n"),
                SlotNumber));

            SDIO_INTERRUPT_ON(pController);
            pController->fSDIOEnabled = TRUE;

            break;

        case SDHCDAckSDIOInterrupt:

                // acquire the lock to block the SDIO interrupt thread
            ACQUIRE_LOCK(pController);

            if ( ( READ_MMC_REGISTER_DWORD( pController, MMC_STAT ) & 0x8000 ) &&
                 ( READ_MMC_REGISTER_DWORD( pController, MMC_IREG ) & 0x0800 ) &&
                 pController->fSDIOEnabled )
            {
                DbgPrintZo(/*SDCARD_ZONE_INIT*/SDH_INTERRUPT_ZONE, (TEXT("SDIO INT (still)!\n")));
                SDHCDIndicateSlotStateChange(pController->pHCContext, 
                                                0,
                                                DeviceInterrupting);
            }
            else if( pController->fSDIOEnabled )
            {
                SDIO_INTERRUPT_ON(pController);
            }

            RELEASE_LOCK(pController);

            break;

        case SDHCDDisableSDIOInterrupts:
            DbgPrintZo(SDCARD_ZONE_INIT, 
                (TEXT("SDHSlotOptionHandler - called - DisableSDIOInterrupts : on slot %d  \n"),
                SlotNumber));

            SDIO_INTERRUPT_OFF(pController);
            pController->fSDIOEnabled = FALSE;

            break;

        case SDHCDGetWriteProtectStatus:
            
            DbgPrintZo(SDCARD_ZONE_INIT, 
                (TEXT("SDHSlotOptionHandler - called - SDHCDGetWriteProtectStatus : on slot %d  \n"),
                 SlotNumber)); 
            
            if( IsCardWriteProtected() ) {
                ((PSD_CARD_INTERFACE)pData)->WriteProtected = TRUE;
                DbgPrintZo(SDCARD_ZONE_INIT, (TEXT("SDHSlotOptionHandler - Card is write protected \n"))); 
            } else {
                ((PSD_CARD_INTERFACE)pData)->WriteProtected = FALSE;
                DbgPrintZo(SDCARD_ZONE_INIT, (TEXT("SDHSlotOptionHandler - Card is write enabled \n"))); 
            }

            break;

         case SDHCDQueryBlockCapability:
            pBlockCaps = (PSD_HOST_BLOCK_CAPABILITY)pData;

             DbgPrintZo(SDCARD_ZONE_INIT, 
             (TEXT("SDHSlotOptionHandler: Read Block Length: %d , Read Blocks: %d\n"), 
                pBlockCaps->ReadBlockSize, 
                pBlockCaps->ReadBlocks));
             DbgPrintZo(SDCARD_ZONE_INIT, 
             (TEXT("SDHSlotOptionHandler: Write Block Length: %d , Write Blocks: %d\n"), 
                pBlockCaps->WriteBlockSize, 
                pBlockCaps->WriteBlocks));

                // the PXA27x controller can only handle up to 1024 bytes
                // with a minimum of 32 bytes per transfer
            if (pBlockCaps->ReadBlockSize > SDH_MAX_BLOCK_SIZE) {
                pBlockCaps->ReadBlockSize = SDH_MAX_BLOCK_SIZE;
            }

            if (pBlockCaps->ReadBlockSize < SDH_MIN_BLOCK_SIZE ) {
                pBlockCaps->ReadBlockSize = SDH_MIN_BLOCK_SIZE;
            }

            if (pBlockCaps->WriteBlockSize > SDH_MAX_BLOCK_SIZE) {
                pBlockCaps->WriteBlockSize = SDH_MAX_BLOCK_SIZE;
            }
            
            if (pBlockCaps->WriteBlockSize < SDH_MIN_BLOCK_SIZE ) {
                pBlockCaps->WriteBlockSize = SDH_MIN_BLOCK_SIZE;
            }

                // the PXA27x controller can handle 64K blocks,
                // we leave the number of blocks alone
             
            break;

        case SDHCDGetSlotInfo:
            if( OptionSize != sizeof(SDCARD_HC_SLOT_INFO) || pData == NULL )
            {
                status = SD_API_STATUS_INVALID_PARAMETER;
            }
            else
            {
                PSDCARD_HC_SLOT_INFO pSlotInfo = (PSDCARD_HC_SLOT_INFO)pData;

                // set the slot capabilities
                SDHCDSetSlotCapabilities(pSlotInfo, SD_SLOT_SD_1BIT_CAPABLE | 
                                                    SD_SLOT_SD_4BIT_CAPABLE |
                                                    SD_SLOT_SDIO_CAPABLE);

                SDHCDSetVoltageWindowMask(pSlotInfo, (SD_VDD_WINDOW_3_2_TO_3_3 | SD_VDD_WINDOW_3_3_TO_3_4)); 

                // Set optimal voltage
                SDHCDSetDesiredSlotVoltage(pSlotInfo, SD_VDD_WINDOW_3_2_TO_3_3);

                SDHCDSetMaxClockRate(pSlotInfo, pController->dwMaximumSDClockFrequency);

                // Set power up delay. We handle this in SetVoltage().
                SDHCDSetPowerUpDelay(pSlotInfo, 300);
            }
            break;

        default:
           status = SD_API_STATUS_INVALID_PARAMETER;

    }

    return status;
}

///////////////////////////////////////////////////////////////////////////////
//  HandleProgramDone - Handle program done interrupt
//  Input:  pController - the controller that is interrupting
//  Output: 
//  Return:
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
VOID HandleProgramDone(PSDH_HARDWARE_CONTEXT pController)
{
    PSD_BUS_REQUEST pRequest;       // current request

    PROGRAM_DONE_INTERRUPT_OFF(pController);

        // get the current request  
    pRequest = pController->pCurrentRequest;

        // this should never happen because we mark the request as un-cancelable.
    DEBUG_ASSERT(NULL != pRequest);
    if(NULL == pRequest)
    {
        return;
    }

    SetCurrentState(pController, WriteDataDone);

        // notice there is no status to check for a programming error
        // this is up to the upper level drivers to send a card status command
    DbgPrintZo(SDH_TRANSMIT_ZONE, (TEXT("HandleProgramDone: Programming Complete \n")));

    if( !( pController->fClockAlwaysOn || ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
    {
            // turn off the clock
        SDClockOff(pController);
            // complete the request
    }

    DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleProgramDone reports Bus Request Succeeded\n")));

    IndicateBusRequestComplete(pController->pHCContext,
                                    pRequest ,
                                    SD_API_STATUS_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////////
//  EmptyReceiveFifo - Empty the receive Fifo
//  Input:  pController - the controller instance
//          pRequest - the request to get the data from
//          ByteCount - number of bytes to read
//          MaxBytes - limit of this transfer
//  Output: 
//  Return:
//  Notes:  
//      
///////////////////////////////////////////////////////////////////////////////
VOID EmptyReceiveFifo(PSDH_HARDWARE_CONTEXT pController, 
                      PSD_BUS_REQUEST         pRequest,
                      ULONG                   ByteCount,
                      ULONG                   MaxBytes)
{   
    ULONG MaxBytesToRead;
    PBYTE pCurPtr;
    DWORD dwTotalRead;

    volatile UCHAR *pMMC_RX_Fifo = (volatile UCHAR *)&(pController->pSDMMCRegisters->rxfifo);
    volatile DWORD *pMMC_RX_FifoDW = (volatile DWORD *)&(pController->pSDMMCRegisters->rxfifo);


    MaxBytesToRead = MaxBytes - pRequest->HCParam;
    pCurPtr = pRequest->pBlockBuffer+pRequest->HCParam;
    
    if( ByteCount > MaxBytesToRead )
    {
        ByteCount = MaxBytesToRead;
    }
    dwTotalRead = ByteCount;
    
    // we are touching the block buffer, we must set the process permissions

            // empty the FIFO
        while (ByteCount) {
            if( ByteCount >= 4 )
            {
                union {
                    BYTE    dataByte[4];
                    DWORD   dataLong;
                } data;
                register PBYTE pSrc = data.dataByte;
                // read in the dword from the FIFO
                data.dataLong = *pMMC_RX_FifoDW;
                *(pCurPtr++) = *(pSrc++);
                *(pCurPtr++) = *(pSrc++);
                *(pCurPtr++) = *(pSrc++);
                *(pCurPtr++) = *(pSrc++);
                ByteCount -= 4;
                
            }
            else while (ByteCount)  {
               // read in the byte from the FIFO
                *(pCurPtr++) = *pMMC_RX_Fifo;
                ByteCount--;
            }
        };
    pRequest->HCParam += dwTotalRead;
}

///////////////////////////////////////////////////////////////////////////////
//  HandleTransferDone- Handle transfer done interrupt 
//  Input:  pController - the controller that is interrupting
//          fForceTimeout - if true, we reached a timeout during transfer
//  Output: 
//  Return:
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
VOID HandleTransferDone(PSDH_HARDWARE_CONTEXT pController, BOOL fForceTimeout)
{
    PSD_BUS_REQUEST pRequest;       // current request
    DWORD           regValue;       // intermediate byte value
    ULONG           maxBytes;       // max bytes

        // turn off the transfer done interrupt
    TRANSFER_DONE_INTERRUPT_OFF(pController);
        // turn off data error interrupt
    PROGRAM_DATA_ERROR_INTERRUPT_OFF(pController);
      
    pController->dwControllerIstTimeout = INFINITE;

        // get the current request  
    pRequest = pController->pCurrentRequest; 

        // this should never happen because we mark the request as un-cancelable
    DEBUG_ASSERT(NULL != pRequest);
    if( !pRequest ) 
    {
        RX_FIFO_INTERRUPT_OFF(pController);
        TX_FIFO_INTERRUPT_OFF(pController);
        return;
    }

    if (TRANSFER_IS_READ(pRequest)) {
            // make sure RX fifo interrupt is off 
        RX_FIFO_INTERRUPT_OFF(pController);
    } else if (TRANSFER_IS_WRITE(pRequest)) {
            // make sure TX fifo interrupt is off 
        TX_FIFO_INTERRUPT_OFF(pController);
            // can't turn off the clock until the prog done interrupt!
    } else {
        DEBUG_ASSERT(FALSE);
    }

        // check the transfer status
    regValue = READ_MMC_REGISTER_DWORD(pController, MMC_STAT);

        // check for errors
    if (regValue & MMC_STAT_FLASH_ERROR) {
        ASSERT(0);
        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleTransferDone reports FLASH ERROR\n")));
        IndicateBusRequestComplete(pController->pHCContext,
                                        pRequest ,
                                        SD_API_STATUS_DATA_ERROR);
        return;
    }

    if (regValue & MMC_STAT_SPI_WR_ERROR) {
        ASSERT(0);
    }

    if (regValue & MMC_STAT_RD_STALLED) {
        ASSERT(0);
    }

    if ( ( regValue & MMC_STAT_READ_TIMEOUT ) || fForceTimeout ) {
        DbgPrintZo(SDCARD_ZONE_ERROR, (TEXT("HandleTransferDoneInterrupt: Read Data TimedOut \n")));     
        
        if( !( pController->fClockAlwaysOn || 
               ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
        {
                // turn off the clock
            SDClockOff(pController);
        }

        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleTransferDone reports DATA TIMEOUT\n")));
        IndicateBusRequestComplete(pController->pHCContext,
                                        pRequest ,
                                        SD_API_STATUS_DATA_TIMEOUT);
        return;

    } else if (regValue & MMC_STAT_READ_DATA_CRC_ERROR) {

        DbgPrintZo(SDCARD_ZONE_ERROR, (TEXT("HandleTransferDoneInterrupt: Read Data Contains CRC error \n"))); 
        if( !( pController->fClockAlwaysOn || 
               ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
        {
                // turn off the clock
            SDClockOff(pController);
        }
        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleTransferDone reports CRC ERROR\n")));
        IndicateBusRequestComplete(pController->pHCContext,
                                        pRequest ,
                                        SD_API_STATUS_CRC_ERROR);
        return;
    } else if (regValue & MMC_STAT_WRITE_DATA_CRC_ERROR) {

        DbgPrintZo(SDCARD_ZONE_ERROR, (TEXT("HandleTransferDoneInterrupt: Card received Write Data with CRC error \n"))); 
        if( !( pController->fClockAlwaysOn || 
               ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
        {
                // turn off the clock
            SDClockOff(pController);
        }
        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleTransferDone reports CRC ERROR\n")));
        IndicateBusRequestComplete(pController->pHCContext,
                                        pRequest ,
                                        SD_API_STATUS_CRC_ERROR);
        return;
    }

    
#ifdef DEBUG
    ASSERT( pController->fDMATransferInProgress == FALSE );
#endif

    if(pController->fDMATransfer)
    {
        pRequest->HCParam = pRequest->NumBlocks * pRequest->BlockSize;
    }


    if (TRANSFER_IS_READ(pRequest)) {

        if( pController->fDMATransfer ) {
            if( pController->fDMAUsingDriverBuffer )
            {
                // copy data from our DMA buffer into client buffer
                if( !SDPerformSafeCopy(pRequest->pBlockBuffer,
                                       pController->pDMABuffer,
                                       pRequest->BlockSize * pRequest->NumBlocks) )
                {
                    ASSERT(0);

                    DbgPrintZo(SDCARD_ZONE_ERROR, (TEXT("HandleTransferDoneInterrupt: Access Violation\n")));
    
                    if( !( pController->fClockAlwaysOn || 
                           ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
                    {
                            // turn off the clock
                        SDClockOff(pController);
                    }

                    DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleTransferDone reports Access Violation\n")));
                    IndicateBusRequestComplete(pController->pHCContext,
                                                    pRequest ,
                                                    SD_API_STATUS_ACCESS_VIOLATION);
                    return;
                }
            }

        } else {
                // why are we doing this here? If the remaining read data is less than a Fifo's worth (32)
                // we won't get the RX Fifo Read Request interrupt because the fifo won't be full.
                // also even if it is full or if this isn't the case the TRANSFER_DONE bit seems to mask it out
                // anyways this prevents the problem where there are bytes stuck in the Fifo
            maxBytes = pRequest->NumBlocks * pRequest->BlockSize;

            if (pRequest->HCParam < maxBytes) {
                DbgPrintZo(SDH_RECEIVE_ZONE, (TEXT("HandleTransferDoneInterrupt: Fifo contains remaining data, Max: %d, current count %d  \n"),
                    maxBytes, pRequest->HCParam));
                    // get the remaining bytes out of the FIFO
                    __try {
                        EmptyReceiveFifo(pController, 
                                 pRequest, 
                                 (maxBytes - pRequest->HCParam), 
                                 maxBytes);
                    }__except(EXCEPTION_EXECUTE_HANDLER) {
                    }
            }
        }

        SetCurrentState(pController, ReadDataDone);

        if( !( pController->fClockAlwaysOn || 
               ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
        {
                // now it is safe to turn off the clock
            SDClockOff(pController);
        }

        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("Bytes read: [%S]\n"), HexDisplay( pRequest->pBlockBuffer, TRANSFER_SIZE(pRequest) ) ) );
    }


    if (TRANSFER_IS_WRITE(pRequest)) {

        if (!IS_PROGRAM_DONE(pController)) {
            SetCurrentState(pController, ProgramWait);
            
                // turn on programming done interrupt
            PROGRAM_DONE_INTERRUPT_ON(pController); 
            //Sleep(500);

                // check to see if programming is finished
            if (!IS_PROGRAM_DONE(pController)) {
                DbgPrintZo(SDH_TRANSMIT_ZONE, (TEXT("HandleTransferDoneInterrupt: Programming Not Complete \n")));   
            }

            // if we wait on the programming done interrupt this could
            // go on forever because now it is up to the memory card, 
            // we may have to make this request cancelable at this point
            
            return;
        }
    } 

    DEBUG_ASSERT((pRequest->HCParam >= (pRequest->NumBlocks * pRequest->BlockSize)));
        // complete the request
    DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleTransferDone reports Data Transfer Completed\n")));
    if (TRANSFER_IS_READ(pRequest)) {
		DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("Bytes read: [%S]\n"), HexDisplay( pRequest->pBlockBuffer, TRANSFER_SIZE(pRequest) ) ) );
	}

    IndicateBusRequestComplete(pController->pHCContext,
                                    pRequest ,
                                    SD_API_STATUS_SUCCESS);
}


///////////////////////////////////////////////////////////////////////////////
//  SDLoadXmitFifo - load the transmit fifo
//  Input:  pController - the controler
//          pRequest    - the request 
//  Output: 
//  Return:  returns TRUE if the request has been fullfilled
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL SDLoadXmitFifo(PSDH_HARDWARE_CONTEXT pController, PSD_BUS_REQUEST pRequest, DWORD maxBytes)
{
    ULONG           remainingBytes; // remaining bytes in the transfer
    DWORD           ii;
    volatile UCHAR *pMMC_TX_Fifo = (volatile UCHAR *)&(pController->pSDMMCRegisters->txfifo);
    volatile DWORD *pMMC_TX_FifoDW = (volatile DWORD *)&(pController->pSDMMCRegisters->txfifo);
    PBYTE pSrc;

    DbgPrintZo(SDH_TRANSMIT_ZONE, (TEXT("SDLoadXmitFifo: Current %d \n"),pRequest->HCParam));

        // make sure the partial full flag is cleared
    TX_BUFFER_PARTIAL_NOT_FULL(pController);


        // figure out how much to prefetch from the user
        // buffer safely
    remainingBytes = maxBytes - pRequest->HCParam;

    if (remainingBytes > MMC_TXFIFO_SIZE) {
            // fix it
        remainingBytes = MMC_TXFIFO_SIZE;
    } 
    pSrc = pRequest->pBlockBuffer+pRequest->HCParam;

            // according to the spec (15.2.8.3) the TX Fifo interrupt asserts for every empty fifo
            // (32 bytes)
            // so we write a Fifo's worth, as per spec 
    ii = 0;
    while( remainingBytes > 0 )
    {
        if( remainingBytes >= 4 )
        {
            union {
                BYTE dataByte[4];
                DWORD dataLong;
            } data;
            register PBYTE pDst = data.dataByte;
            *(pDst++) = *(pSrc++);
            *(pDst++) = *(pSrc++);
            *(pDst++) = *(pSrc++);
            *(pDst++) = *(pSrc++);
            // transfer bytes to the fifo from the safe buffer
            *pMMC_TX_FifoDW = data.dataLong;
            
            remainingBytes-=4;
            ii+=4;
        }
        else
        {
                // transfer bytes to the fifo from the safe buffer
            *pMMC_TX_Fifo = *(pSrc++);
            remainingBytes--;
            ii++;
        }
    } // while
    pRequest->HCParam += ii;

        // check for a partial buffer
    if (ii < MMC_TXFIFO_SIZE) {
        TX_BUFFER_PARTIAL_FULL(pController);
    }

    DbgPrintZo(SDH_TRANSMIT_ZONE, (TEXT("SDLoadXmitFifo: New Current %d  \n"),pRequest->HCParam));
    
        // see if we are done
    if (pRequest->HCParam >= maxBytes) {
        return TRUE;
    } 

    return FALSE;
}
DWORD GetMMCInterrupts(PSDH_HARDWARE_CONTEXT pHCDevice);
///////////////////////////////////////////////////////////////////////////////
//  HandleXmitInterrupt - handle the Xmit Fifo Empty interrupt
//  Input:  pController - the controler
//  Output: 
//  Return:
//  Notes:   
///////////////////////////////////////////////////////////////////////////////
VOID HandleXmitInterrupt(PSDH_HARDWARE_CONTEXT pController)
{
    PSD_BUS_REQUEST pRequest;       // current request
    DWORD maxBytes;
    
    ASSERT( pController->fDMATransfer == FALSE );

        // get the current request  
    pRequest = pController->pCurrentRequest;

        // this should never happen because we mark the request as un-cancelable
    DEBUG_ASSERT(NULL != pRequest);
    if( pRequest == NULL )
    {
        TX_FIFO_INTERRUPT_OFF(pController);
        return;
    }

    maxBytes = pRequest->NumBlocks * pRequest->BlockSize;
    while (pRequest->HCParam < maxBytes && IsCardPresent()) {
        DWORD dwIntr = GetMMCInterrupts(pController);
        if (dwIntr & MMC_IREG_TXFIFO_REQ) {
            BOOL fReturn ;
            __try {
                fReturn = SDLoadXmitFifo(pController, pRequest, maxBytes);
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                fReturn = TRUE ;
            }
            if (fReturn)
                break;                
        }            
        if (dwIntr & (MMC_IREG_END_CMD | MMC_IREG_RES_ERR))
            break;
    }
    // the request is complete  
    // turn off the Fifo Interrupts
    TX_FIFO_INTERRUPT_OFF(pController);
    SetCurrentState(pController, WriteDataTransferDone);

    // now we need to wait for the controller to transmit (transfer done) and the card 
    // to complete programming (program done)
    // if the transfer is done or programming is done before we go back into the interrupt wait
    // the interrupt bit will be set and the IST loop will handle the transfer done in this same thread     

}

///////////////////////////////////////////////////////////////////////////////
//  HandleReceiveInterrupt - Handle recieve data interrupt
//  Input:  pController - the controller that is interrupting
//  Output: 
//  Return:
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
VOID HandleReceiveInterrupt(PSDH_HARDWARE_CONTEXT pController)
{
    PSD_BUS_REQUEST pRequest;       // current request
    DWORD           maxBytes;       // max bytes
    ULONG           ii = 0;         // loop variable

    ASSERT( pController->fDMATransfer == FALSE );

        // get the current request  
    pRequest = pController->pCurrentRequest;

        // this should never happen because we mark the request as un-cancelable
    DEBUG_ASSERT(NULL != pRequest);
    if(NULL == pRequest)
    {
        RX_FIFO_INTERRUPT_OFF(pController);
        return;
    }

    maxBytes = pRequest->NumBlocks * pRequest->BlockSize;
    
    DbgPrintZo(SDH_RECEIVE_ZONE, (TEXT("HandleReceiveInterrupt: Max: %d, Current %d \n"),
        maxBytes, pRequest->HCParam));
 
        // according to the spec (15.2.8.2) the RX Fifo interrupt asserts for every 32 bytes and
        // remains asserted until the RX fifo is empty, once it is empty 
        // the interrupt req resets and won't assert until 32 more bytes are received
        // or until the transfer is complete and their is a partial Fifo
    if ((maxBytes - pRequest->HCParam) >= (LONG)MMC_RXFIFO_SIZE) {
            // because the remaining bytes is greater than or equal to the fifo size,
            // the fifo better be full as per Intel spec!
        // DEBUG_ASSERT(RX_FIFO_FULL(pController)); @todo
    }

    // read a Fifo's worth, as per spec 
    while ( IsCardPresent() && pRequest->HCParam<maxBytes ) {
        DWORD dwIntr = GetMMCInterrupts(pController);
        if (dwIntr & MMC_IREG_RXFIFO_REQ) {
            __try { 
                EmptyReceiveFifo(pController, pRequest, MMC_RXFIFO_SIZE, maxBytes);
            }__except(EXCEPTION_EXECUTE_HANDLER) {
                break;
            }
        }
        if (dwIntr & (MMC_IREG_END_CMD | MMC_IREG_RES_ERR))
            break;
    }

    DbgPrintZo(SDH_RECEIVE_ZONE, (TEXT("HandleReceiveInterrupt: New Current %d  \n"),pRequest->HCParam));
    
        // see if we are done
    if (pRequest->HCParam >= maxBytes) {
        DbgPrintZo(SDH_RECEIVE_ZONE, (TEXT("HandleReceiveInterrupt: Data Transfer Completing waiting for TRANS_DONE..\n")));    
            // if we are finished, turn off the RX Fifo request interrupt
        RX_FIFO_INTERRUPT_OFF(pController);  
        SetCurrentState(pController, ReadDataTransferDone);
        // now we need to wait for the controller to perform the CRC check and issue trailing clocks (transfer done)
        // if the transfer is done, the interrupt bit will be set and the IST loop will 
        // handle the transfer done in this same thread      
    }  

        // we could mark the request as cancelable again...
}

///////////////////////////////////////////////////////////////////////////////
//  PrepareDmaTransferOnDriverBuffer - Prepares the DMA transfer on driver allocated buffer
//  Input:  pController - the controller that is interrupting
//  Output: 
//  Return:
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL PrepareDmaTransferOnDriverBuffer( PSDH_HARDWARE_CONTEXT pController, PSD_BUS_REQUEST pRequest )
{
    DWORD dwTransferLength;
    DWORD dwDataAddress;
    DWORD dwNextDescriptorAddress;
    DWORD dwDescriptorIndex = 0;

    ASSERT( MAXIMUM_DMA_TRANSFER_SIZE >= pController->systemInfo.dwPageSize );

    dwTransferLength = pRequest->BlockSize * pRequest->NumBlocks;
    dwDataAddress = pController->pDMABufferPhys.LowPart;
    dwNextDescriptorAddress = pController->pDMADescriptorsPhys.LowPart + sizeof(DMADescriptorChannelType);

    // copy the data to the driver buffer for writing
    if( !(TRANSFER_IS_READ(pRequest)) )
    {
        if( !SDPerformSafeCopy(pController->pDMABuffer,
                               pRequest->pBlockBuffer,
                               dwTransferLength) )
        {
            return FALSE;
        }
    }

    // prepare the descriptors
    while( dwTransferLength > 0 )
    {
        DWORD dwBlockLength = dwTransferLength;
        if( dwBlockLength > pController->systemInfo.dwPageSize )
        {
            dwBlockLength = pController->systemInfo.dwPageSize;
        }

        ASSERT( dwNextDescriptorAddress % 16 == 0 ); // make sure the address is 128 bit aligned
        pController->pDMADescriptors[dwDescriptorIndex].ddadr = dwNextDescriptorAddress;

        if( (TRANSFER_IS_READ(pRequest)) )
        {
            pController->pDMADescriptors[dwDescriptorIndex].dsadr = SDIO_RX_FIFO;
            pController->pDMADescriptors[dwDescriptorIndex].dtadr = dwDataAddress;
            pController->pDMADescriptors[dwDescriptorIndex].dcmd  = DCMD_INC_TRG_ADDR |
                                                       DCMD_FLOW_SRC | 
                                                       //DCMD_END_IRQ_EN |
                                                       ( 3 << 16 ) | // 32 bytes maximum burst size of each data transfer
                                                       ( 3 << 14 ) | // 4 bytes width
                                                       dwBlockLength;
        }
        else
        {
            pController->pDMADescriptors[dwDescriptorIndex].dsadr = dwDataAddress;
            pController->pDMADescriptors[dwDescriptorIndex].dtadr = SDIO_TX_FIFO;
            pController->pDMADescriptors[dwDescriptorIndex].dcmd  = DCMD_INC_SRC_ADDR |
                                                       DCMD_FLOW_TRG | 
                                                       //DCMD_END_IRQ_EN |
                                                       ( 3 << 16 ) | // 32 bytes maximum burst size of each data transfer
                                                       ( 3 << 14 ) | // 4 bytes width
                                                       dwBlockLength;
        }

        dwTransferLength -= dwBlockLength;

        if( dwTransferLength == 0 )
        {
            pController->pDMADescriptors[dwDescriptorIndex].ddadr = 1; // this is the last descriptor, set the STOP bit
        }

        dwDataAddress += dwBlockLength;
        dwNextDescriptorAddress += sizeof(DMADescriptorChannelType);
        dwDescriptorIndex++;
    } // while

    pController->fDMAUsingDriverBuffer = TRUE;
    pController->dwBytesRemaining = 0;
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  PrepareDmaTransfer - Prepares the DMA for transfer
//  Input:  pController - the controller that is interrupting
//  Output: 
//  Return:
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL PrepareDmaTransfer( PSDH_HARDWARE_CONTEXT pController, PSD_BUS_REQUEST pRequest )
{
    DWORD dwTransferLength;
    DWORD dwMaximumPages;
    BOOL fRetVal = TRUE;

    dwTransferLength = pRequest->BlockSize * pRequest->NumBlocks;
    pController->fDMAUsingDriverBuffer = FALSE;

    pController->dwClientBufferSize = 0;
    pController->pClientBuffer = NULL;

    if( pController->dwDmaChannel == 0xffffffff )
    { // DMA is disabled in registry
        return FALSE;
    }

    if( 0 != dwTransferLength % MMC_TXFIFO_SIZE )
    {
        return FALSE; // data size not a multiple of FIFO size
    }
    
    if( ( 0 != (((DWORD)pRequest->pBlockBuffer) % SDIO_DMA_ALIGNMENT )) &&
        ( dwTransferLength > pController->dwDmaBufferSize ) )
    {
        return FALSE; // buffer is not 32 bytes aligned, and is too large to fit in driver's allocated buffer
    }

    if( 0 != ((DWORD)pRequest->pBlockBuffer) % SDIO_DMA_ALIGNMENT )
    {
        // buffer is not 32 bytes aligned, we will use driver allocated DMA buffer
        return PrepareDmaTransferOnDriverBuffer( pController, pRequest );
    }

    // buffer is 32 bytes aligned.  We will do DMA on client provided buffer

    // calculate the number of physical pages that the buffer occupies
    dwMaximumPages = COMPUTE_PAGES_SPANNED(pRequest->pBlockBuffer, dwTransferLength);
    
    if( pController->pPFNs == NULL )
    {
        // allocate a buffer for PFNs
        pController->pPFNs = (PDWORD)LocalAlloc(0, dwMaximumPages * sizeof(DWORD) );
        if( pController->pPFNs == NULL )
        {
            return FALSE;
        }
        pController->nPFNCount = dwMaximumPages;
    }
    else if( dwMaximumPages > pController->nPFNCount )
    {
        // increase the buffer for PFNs
        HLOCAL hTemp = LocalReAlloc( pController->pPFNs, dwMaximumPages * sizeof(DWORD), LMEM_MOVEABLE );
        if( hTemp == NULL )
        {
            return FALSE;
        }

        pController->pPFNs = (PDWORD)hTemp;
        pController->nPFNCount = dwMaximumPages;
    }

    // lock the pages containing the client provided buffer
    if( !LockPages( pRequest->pBlockBuffer, dwTransferLength, pController->pPFNs, (TRANSFER_IS_READ(pRequest)) ? LOCKFLAG_READ : LOCKFLAG_WRITE ) )
    {
        fRetVal = FALSE;
    }
    CacheRangeFlush( pRequest->pBlockBuffer, dwTransferLength, CACHE_SYNC_ALL );
    
    // set the transfer parameters
    pController->dwPFNIndex = 0;
    pController->dwPageOffset = ((DWORD)pRequest->pBlockBuffer) % pController->systemInfo.dwPageSize;
    pController->dwBytesRemaining = dwTransferLength;

    pController->dwClientBufferSize = dwTransferLength;
    pController->pClientBuffer = pRequest->pBlockBuffer;

    return fRetVal;
}

void DoDMATransferOnDriverBuffer( PSDH_HARDWARE_CONTEXT pController, PSD_BUS_REQUEST pRequest )
{
    if (TRANSFER_IS_READ(pRequest)){
        pController->pDMARegisters->drcmr[DMA_CHMAP_SDIO_TX] = 0;
        pController->pDMARegisters->drcmr[DMA_CHMAP_SDIO_RX] = DMA_MAP_VALID_MASK | pController->dwDmaChannel;
    } else {
        pController->pDMARegisters->drcmr[DMA_CHMAP_SDIO_RX] = 0;
        pController->pDMARegisters->drcmr[DMA_CHMAP_SDIO_TX] = DMA_MAP_VALID_MASK | pController->dwDmaChannel;
    } 

    // Set the DESCRIPTOR FETCH mode
    pController->pDMARegisters->dcsr[pController->dwDmaChannel] =  0;

    // program the transfer descriptor
    pController->pDMARegisters->ddg[pController->dwDmaChannel].ddadr = pController->pDMADescriptorsPhys.LowPart;

#ifdef DEBUG
    pController->fDMATransferInProgress = TRUE;
#endif

    pController->pDMARegisters->dcsr[pController->dwDmaChannel] |=  DCSR_RUN | DCSR_STOPIRQEN;          // set the RUN bit
}

void DoDMATransferRead( PSDH_HARDWARE_CONTEXT pController, PSD_BUS_REQUEST pRequest )
{
    DWORD dwTransferLength;
    DWORD dwTargetAddress;

    ASSERT( MAXIMUM_DMA_TRANSFER_SIZE >= pController->systemInfo.dwPageSize );

    // calculate how much data should be read in this DMA transfer
    dwTransferLength = pController->systemInfo.dwPageSize - pController->dwPageOffset;
    if( dwTransferLength > pController->dwBytesRemaining )
    {
        dwTransferLength = pController->dwBytesRemaining;
    }

    // calculate the target physical address
    dwTargetAddress = ( pController->pPFNs[pController->dwPFNIndex] << UserKInfo[KINX_PFN_SHIFT] ) + pController->dwPageOffset;

    // Set the NO DESCRIPTOR FETCH mode
    pController->pDMARegisters->dcsr[pController->dwDmaChannel] =  DCSR_NOFETCH;// | DCSR_ENDINTR;

    pController->dwPFNIndex++;
    pController->dwPageOffset = 0;
    pController->dwBytesRemaining -= dwTransferLength;

    // program the source, target and transfer parameters
    pController->pDMARegisters->ddg[pController->dwDmaChannel].dsadr = SDIO_RX_FIFO;
    pController->pDMARegisters->ddg[pController->dwDmaChannel].dtadr = dwTargetAddress;
    pController->pDMARegisters->ddg[pController->dwDmaChannel].dcmd = DCMD_INC_TRG_ADDR |
                                                                       DCMD_FLOW_SRC | 
                                                                       //DCMD_END_IRQ_EN |
                                                                       ( 3 << 16 ) | // 32 bytes maximum burst size of each data transfer
                                                                       ( 3 << 14 ) | // 4 bytes width
                                                                       dwTransferLength;

#ifdef DEBUG
    pController->fDMATransferInProgress = TRUE;
#endif

    pController->pDMARegisters->drcmr[DMA_CHMAP_SDIO_RX] = DMA_MAP_VALID_MASK | pController->dwDmaChannel;
    pController->pDMARegisters->dcsr[pController->dwDmaChannel] |=  DCSR_RUN | DCSR_STOPIRQEN;          // set the RUN bit
}

void DoDMATransferWrite( PSDH_HARDWARE_CONTEXT pController, PSD_BUS_REQUEST pRequest )
{
    DWORD dwTransferLength;
    DWORD dwSourceAddress;

    ASSERT( MAXIMUM_DMA_TRANSFER_SIZE >= pController->systemInfo.dwPageSize );

    // calculate how much data should be written in this DMA transfer
    dwTransferLength = pController->systemInfo.dwPageSize - pController->dwPageOffset;
    if( dwTransferLength > pController->dwBytesRemaining )
    {
        dwTransferLength = pController->dwBytesRemaining;
    }

    // calculate the target physical address
    dwSourceAddress = ( pController->pPFNs[pController->dwPFNIndex] << UserKInfo[KINX_PFN_SHIFT] ) + pController->dwPageOffset;

    // Set the NO DESCRIPTOR FETCH mode
    pController->pDMARegisters->dcsr[pController->dwDmaChannel] =  DCSR_NOFETCH; // | DCSR_ENDINTR;

    pController->dwPFNIndex++;
    pController->dwPageOffset = 0;
    pController->dwBytesRemaining -= dwTransferLength;

    // program the source, target and transfer parameters
    pController->pDMARegisters->ddg[pController->dwDmaChannel].dsadr = dwSourceAddress;
    pController->pDMARegisters->ddg[pController->dwDmaChannel].dtadr = SDIO_TX_FIFO;
    pController->pDMARegisters->ddg[pController->dwDmaChannel].dcmd = DCMD_INC_SRC_ADDR |
                                                                       DCMD_FLOW_TRG | 
                                                                       //DCMD_END_IRQ_EN |
                                                                       ( 3 << 16 ) | // 32 bytes maximum burst size of each data transfer
                                                                       ( 3 << 14 ) | // 1 byte width
                                                                       dwTransferLength;

#ifdef DEBUG
    pController->fDMATransferInProgress = TRUE;
#endif

    pController->pDMARegisters->drcmr[DMA_CHMAP_SDIO_TX] = DMA_MAP_VALID_MASK | pController->dwDmaChannel;
    pController->pDMARegisters->dcsr[pController->dwDmaChannel] |=  DCSR_RUN | DCSR_STOPIRQEN;          // set the RUN bit
}

///////////////////////////////////////////////////////////////////////////////
//  HandleDMAInterrupt - Handle DMA interrupt
//  Input:  pController - the controller that is interrupting
//  Output: 
//  Return:
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
VOID HandleDMAInterrupt(PSDH_HARDWARE_CONTEXT pController)
{
    DWORD DCSR;
    PSD_BUS_REQUEST     pRequest;       // the request to complete

    // abort if we are trying to stop the DMA transfer from the ProcessCardRemoval routine
    if( pController->fDMATransferCancelled )
        return;

        // get the current request  
    pRequest = pController->pCurrentRequest;

    if(NULL == pRequest)
    {
        pController->pDMARegisters->dcsr[pController->dwDmaChannel] = DCSR_NOFETCH;// | DCSR_ENDINTR;
        if( !(pController->fDMAUsingDriverBuffer) )
        { // we use client provided DMA buffer.  Unlock the memory pages where it is located.
            if( pController->pClientBuffer )
            {
                UnlockPages( pController->pClientBuffer, pController->dwClientBufferSize );
                pController->pClientBuffer = NULL;
                pController->dwClientBufferSize = 0;
            }
        }
        return;
    }

    ASSERT(pController->fDMATransfer);
    
#ifdef DEBUG
    pController->fDMATransferInProgress = FALSE;
#endif

    DCSR = pController->pDMARegisters->dcsr[pController->dwDmaChannel];

    // check for DMA bus errors
    if( DCSR & DCSR_BUSERRINTR )
    {
        ASSERT(0); // DMA Bus error!

        DbgPrintZo(SDCARD_ZONE_ERROR, (TEXT("HandleDMAInterrupt: DMA BUS ERROR\n")));     
        
        if( !( pController->fClockAlwaysOn || 
               ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
        {
                // turn off the clock
            SDClockOff(pController);
        }

        if( !(pController->fDMAUsingDriverBuffer) )
        { // we use client provided DMA buffer.  Unlock the memory pages where it is located.
            if( pController->pClientBuffer )
            {
                UnlockPages( pController->pClientBuffer, pController->dwClientBufferSize );
                pController->pClientBuffer = NULL;
                pController->dwClientBufferSize = 0;
            }
        }

        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleDMAInterrupt reports DMA BUS ERROR\n")));
        IndicateBusRequestComplete(pController->pHCContext,
                                        pRequest ,
                                        SD_API_STATUS_UNSUCCESSFUL);
        return;
    }

    if( DCSR & DCSR_STARTINTR )
    {
        ASSERT( FALSE ); // unexpected
    }

    if( DCSR & DCSR_ENDINTR )
    {
        ASSERT( FALSE ); // unexpected
    }

    if( DCSR & DCSR_EOR_INTR )
    {
        ASSERT( FALSE ); // unexpected
    }

    if( DCSR & DCSR_STOPINTR )
    {
        // check if there is more data to transfer
        if( pController->dwBytesRemaining > 0 )
        {
            // if card was ejected
            if( !IsCardPresent() )
            {
                DbgPrintZo(SDCARD_ZONE_ERROR, (TEXT("HandleDMAInterrupt: Card ejected!\n")));     

                if( !( pController->fClockAlwaysOn || 
                       ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
                {
                        // turn off the clock
                    SDClockOff(pController);
                }

                if( !(pController->fDMAUsingDriverBuffer) )
                { // we use client provided DMA buffer.  Unlock the memory pages where it is located.
                    if( pController->pClientBuffer )
                    {
                        UnlockPages( pController->pClientBuffer, pController->dwClientBufferSize );
                        pController->pClientBuffer = NULL;
                        pController->dwClientBufferSize = 0;
                    }
                }

                DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleDMAInterrupt reports Card ejected\n")));
                IndicateBusRequestComplete(pController->pHCContext,
                                                pRequest ,
                                                SD_API_STATUS_DEVICE_REMOVED);
                return;
            }

            if (TRANSFER_IS_READ(pRequest)){
                DoDMATransferRead( pController, pRequest );
            } else {
                DoDMATransferWrite( pController, pRequest );
            }
        }
        else
        {
            pController->pDMARegisters->dcsr[pController->dwDmaChannel] = DCSR_NOFETCH;// | DCSR_ENDINTR;
            if( !(pController->fDMAUsingDriverBuffer) )
            { // we use client provided DMA buffer.  Unlock the memory pages where it is located.
                if( pController->pClientBuffer )
                {
                    UnlockPages( pController->pClientBuffer, pController->dwClientBufferSize );
                    pController->pClientBuffer = NULL;
                    pController->dwClientBufferSize = 0;
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  CalculateTransferTimeout - calculate the data transfer timeout
//  Input:  pController - the controller that is interrupting
//          pRequest - current request
//  Output: 
//  Returns:  transfer timeout in milliseconds
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD CalculateTransferTimeout(PSDH_HARDWARE_CONTEXT pController, PSD_BUS_REQUEST pRequest)
{
    unsigned __int64 dwTransferTime;

    // get the number of bytes to transfer
    dwTransferTime = pRequest->BlockSize * pRequest->NumBlocks;
    
    // now calculate the number of bits to transfer
    if( pController->f4BitMode )
    {
        dwTransferTime *= 2;
    }
    else
    {
        dwTransferTime *= 8;
    }

    // increase the transfer time estimate by a factor of 5
    dwTransferTime *= TRANSFER_TIMEOUT_FACTOR; 
    
    // now calculate how long (in seconds) it will take to transfer
    dwTransferTime /= pController->dwSDClockFrequency;

    // add a 10 seconds constant delay
    dwTransferTime += TRANSFER_TIMEOUT_CONSTANT; 

    // return the delay in milliseconds
    return (DWORD)(dwTransferTime * 1000);
}

///////////////////////////////////////////////////////////////////////////////
//  HandleEndCommandInterrupt - Handle End of Command Interrupt
//  Input:  pController - the controller that is interrupting
//  Output: 
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
VOID HandleEndCommandInterrupt(PSDH_HARDWARE_CONTEXT pController)
{
    DWORD               statRegister;   // status register
    PSD_BUS_REQUEST     pRequest;       // the request to complete
    DWORD               regValue;       // intermediate reg value
    LONG                fifoCount;     // starting offset in response buffer
    PBYTE               pSrcPtr;

        // get the current request  
    pRequest = pController->pCurrentRequest;

        // this should never happen because we mark the request as un-cancelable
    DEBUG_ASSERT(NULL != pRequest);
    if( pRequest == NULL )
    {
        END_CMD_INTERRUPT_OFF(pController);
        return;
    }

        // get the stat register
    statRegister = READ_MMC_REGISTER_DWORD(pController, MMC_STAT);

        // mask the END_CMD interrupt, the command is complete , however
        // reading the STAT register doesn't clear the interrupt
        // we need to just mask this interrupt out
    END_CMD_INTERRUPT_OFF(pController);
        // mask the RESPONSE_ERROR interrupt
    PROGRAM_RESPONSE_ERROR_INTERRUPT_OFF(pController);
       
    if (statRegister & MMC_STAT_FLASH_ERROR) {
        ASSERT(0);
        regValue = READ_MMC_REGISTER_DWORD(pController, MMC_CMD);
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("HandleEndCommandInterrupt: response for command %d , FLASH ERROR \n"),regValue));

        if( !( pController->fClockAlwaysOn || 
               ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
        {
                // turn off the clock
            SDClockOff(pController);
        }
            // complete the current request with a timeout
        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleEndCommandInterrupt reports FLASH ERROR\n")));
        IndicateBusRequestComplete(pController->pHCContext,
                                        pRequest ,
                                        SD_API_STATUS_UNSUCCESSFUL);
        return;
    }

    if (statRegister & MMC_STAT_SPI_WR_ERROR) {
        ASSERT(0);
    }

    if (statRegister & MMC_STAT_RD_STALLED) {
        ASSERT(0);
    }

    if (statRegister & MMC_STAT_RESPONSE_TIMEOUT) {
        regValue = READ_MMC_REGISTER_DWORD(pController, MMC_CMD);
        regValue &= 0x3F;
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("HandleEndCommandInterrupt: response for command %d , timed - out \n"),regValue));

        if( !( pController->fClockAlwaysOn || 
               ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
        {
                // turn off the clock
            SDClockOff(pController);
        }
            // complete the current request with a timeout
        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleEndCommandInterrupt reports RESPONSE TIMEOUT\n")));
        IndicateBusRequestComplete(pController->pHCContext,
                                        pRequest ,
                                        SD_API_STATUS_RESPONSE_TIMEOUT);
        return;
    }

    if (statRegister & MMC_STAT_RESPONSE_CRC_ERROR)   {
      // Intel PXA27x has bug calculating CRC for CMD2, CMD9, CMD10.
      // See "Intel(R) PXA27x Family Processor, Specification Update
      // - Revision 001" of April 2004 (order number 280071-001), 
      // Errata "E42" for details.
      // The recommended workaround is to ignore the error.
      // Note: Because the CRC just fails when bit 128 is set, one
      // could ignore the CRC only in instances where that bit was
      // set.  One could also calculate the CRC for the case where
      // bit 128 is set.
      // The developer chose not to spend time implementing these
      // workaround, and took the simple way out.
      // If there are problems with a device, especially during the
      // initial device handshaking, then adding these workarounds
      // would make more sense.
      //
      // Added CMD3 to list of CRC errors to ignore because it also has CRC errors!
      if ((pRequest->CommandCode == SD_CMD_ALL_SEND_CID) ||   // CMD2
         (pRequest->CommandCode == SD_CMD_MMC_SET_RCA) ||   // CMD3
         (pRequest->CommandCode == SD_CMD_SEND_CSD) ||      // CMD9
         (pRequest->CommandCode == SD_CMD_SEND_CID)) {      // CMD10
         // If this is one of the commands that is affected by the
         // errata mentioned above, then clear the status register
         // of this error and continue as if it didn't occur.
         statRegister &= ~MMC_STAT_RESPONSE_CRC_ERROR;
         DbgPrintZo(SDCARD_ZONE_ERROR, 
                  (TEXT("HandleEndCommandInterrupt: Ignoring CRC ERROR for CMD%d\n"), pRequest->CommandCode));
      }
      else
      {
            // Report the CRC error and fail.
            regValue = READ_MMC_REGISTER_DWORD(pController, MMC_CMD);
            regValue &= 0x3F;

            if( !( pController->fClockAlwaysOn || 
                   ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
            {
                    // turn off the clock
                SDClockOff(pController);
            }
            DbgPrintZo(SDCARD_ZONE_ERROR, (TEXT("HandleEndCommandInterrupt: response for command %d , contains a CRC error \n"), regValue));
                // complete the current request with a CRC error status
            DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleEndCommandInterrupt reports CRC ERROR\n")));
            IndicateBusRequestComplete(pController->pHCContext,
                                            pRequest ,
                                            SD_API_STATUS_CRC_ERROR);
            return;
        }
    }

   
    if (NoResponse == pRequest->CommandResponse.ResponseType) {
        pSrcPtr = NULL;
    } else if (ResponseR2 == pRequest->CommandResponse.ResponseType) {
            // 8 words - 128 bits
        fifoCount = SDH_RESPONSE_FIFO_DEPTH;
        pSrcPtr = pRequest->CommandResponse.ResponseBuffer + sizeof(pRequest->CommandResponse.ResponseBuffer);
    } else {
        // 3 WORDS - 48 bits
        fifoCount = 3;
        pSrcPtr = pRequest->CommandResponse.ResponseBuffer + 3*sizeof(WORD);
    }

    if (NoResponse != pRequest->CommandResponse.ResponseType && pSrcPtr!=NULL) {
        while (fifoCount--) {
            union {
                WORD wDataWord;
                BYTE bDataByte[2];
            } data;
            data.wDataWord = (USHORT)(READ_MMC_REGISTER_DWORD(pController, MMC_RES));
            *(--pSrcPtr)=data.bDataByte[1];
            *(--pSrcPtr)=data.bDataByte[0];
        }
    }

        // check for command/response only
    if (SD_COMMAND == pRequest->TransferClass) {

            // check to see if this request was a response with busy
        if (ResponseR1b == pRequest->CommandResponse.ResponseType) { 

            while( ( !( statRegister & MMC_STAT_PROGRAM_DONE ) ) &&
                   ( !( pController->DriverShutdown ) ) &&
                   IsCardPresent() )
            {
                statRegister = READ_MMC_REGISTER_DWORD(pController, MMC_STAT);
            }
        }

        if( !( pController->fClockAlwaysOn || 
               ( pController->fClockOnIfInterruptsEnabled && pController->fSDIOEnabled ) ) )
        {
                // complete the current request here, there's no data phase
                // turn off the clock
            SDClockOff(pController);
        }

        SetCurrentState(pController, CommandComplete);
        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleEndCommandInterrupt reports Bus Request Completed\n")));
        IndicateBusRequestComplete(pController->pHCContext,
                                        pRequest ,
                                        SD_API_STATUS_SUCCESS);    
    } else {

        // set the transfer timeout
        pController->dwControllerIstTimeout = CalculateTransferTimeout(pController, pRequest);
        if (TRANSFER_IS_READ(pRequest)){   
            DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleEndCommandInterrupt starting READ TRANSFER of %d blocks of %d bytes\n"), pRequest->NumBlocks, pRequest->BlockSize ));

            if( pController->fDMATransfer )
            {
                SetCurrentState(pController, ReadDataTransfer);
                //RX_FIFO_INTERRUPT_ON(pController);
                TRANSFER_DONE_INTERRUPT_ON(pController);
                PROGRAM_DATA_ERROR_INTERRUPT_ON(pController);
                if( pController->fDMAUsingDriverBuffer )
                {
                    DoDMATransferOnDriverBuffer( pController, pRequest );
                }
                else
                {
                    DoDMATransferRead( pController, pRequest );
                }
            }
            else
            {

                    // turn on RX Fifo interrupts
                RX_FIFO_INTERRUPT_ON(pController);
                SetCurrentState(pController, ReadDataTransfer);
                    // turn on the transfer done interrupt to check for timeout on reads
                    // the receive handler will turn this off after getting the first byte
                TRANSFER_DONE_INTERRUPT_ON(pController);
                PROGRAM_DATA_ERROR_INTERRUPT_ON(pController);
            }
        } else {
            DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("HandleEndCommandInterrupt starting WRITE TRANSFER of %d blocks of %d bytes\n"), pRequest->NumBlocks, pRequest->BlockSize ));
            DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("Bytes wrtn: [%S]\n"), HexDisplay( pRequest->pBlockBuffer, TRANSFER_SIZE(pRequest) )) );
            SetCurrentState(pController, WriteDataTransfer);

            if( pController->fDMATransfer )
            {
                //TX_FIFO_INTERRUPT_ON(pController);
                TRANSFER_DONE_INTERRUPT_ON(pController);
                PROGRAM_DATA_ERROR_INTERRUPT_ON(pController);
                if( pController->fDMAUsingDriverBuffer )
                {
                    DoDMATransferOnDriverBuffer( pController, pRequest );
                }
                else
                {
                    DoDMATransferWrite( pController, pRequest );
                }
            }
            else
            {
                    // turn on Fifo interrupts
                TX_FIFO_INTERRUPT_ON(pController);
                    // turn on transfer interrupts
                TRANSFER_DONE_INTERRUPT_ON(pController);
                PROGRAM_DATA_ERROR_INTERRUPT_ON(pController);

                __try {
                    SDLoadXmitFifo(pController, pRequest,pRequest->NumBlocks * pRequest->BlockSize);
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                };
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  HandleSDIOInterrupt - Handle SDIO interrupt
//  Input:  pController - the controller that is interrupting
//  Output: 
//  Return:
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
VOID HandleSDIOInterrupt(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    if( IsCardPresent( pHCDevice ) && pHCDevice->DevicePresent ) 
    {
            // disable the SDIO interrupt
        SDIO_INTERRUPT_OFF(pHCDevice);
            // indicate that the card is interrupting
        DbgPrintZo(SDH_SDBUS_INTERACTION_ZONE, (TEXT("Got SDIO Interrupt\n")));
        SDHCDIndicateSlotStateChange(pHCDevice->pHCContext, 
                                        0,
                                        DeviceInterrupting);
    }
}


///////////////////////////////////////////////////////////////////////////////
//  GetMMCInterrupts - Get MMC interrupts
//  Input:  pHCDevice - the controller 
//  Output: 
//  Return: bit mask of the interrupts
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD GetMMCInterrupts(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    DWORD intr;
    DWORD interrupts;    // current interrupts
    DWORD interruptMask; // interrupt mask
#ifdef DEBUG
    DWORD stat;
#endif

        // get interrupts
    intr = READ_MMC_REGISTER_DWORD(pHCDevice, MMC_IREG) 
                 & MMC_IREG_INTERRUPTS;

    // get the interrupt masks so we know which ones we don't care about
        // the handlers will turn off (mask) interrupts 
    interruptMask = (~(READ_MMC_REGISTER_DWORD(pHCDevice, MMC_IMASK))) 
                    & MMC_IREG_INTERRUPTS;

        // mask it
    interrupts = intr & interruptMask;

#ifdef DEBUG
    stat = READ_MMC_REGISTER_DWORD(pHCDevice,MMC_STAT);
                                     
    if( pHCDevice->fSDIOEnabled )
    {
        DbgPrintZo(SDH_INTERRUPT_ZONE, 
            (TEXT("S=%04X I=%04X M=%04X R=%04X\n"), 
                stat, intr, interruptMask, interrupts));
    }
#endif

    return interrupts;
}

///////////////////////////////////////////////////////////////////////////////
//  SDDMAIstThread - IST thread for DMA Controller
//  Input:  pHCDevice - the controller instance
//  Output: 
//  Return: Thread exit code
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD SDDMAIstThread(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    DWORD waitStatus;    // wait status

    if (!CeSetThreadPriority(GetCurrentThread(), 
        pHCDevice->DmaIstThreadPriority)) {
        DbgPrintZo(SDCARD_ZONE_WARN, 
            (TEXT("SDDMAIstThread: warning, failed to set CEThreadPriority \n")));
    }

    while(1) {

        waitStatus = WaitForSingleObject(pHCDevice->hDMAInterruptEvent, INFINITE);

        if (WAIT_OBJECT_0 != waitStatus) {
            DbgPrintZo(SDCARD_ZONE_WARN, 
                (TEXT("SDDMAIstThread: Wait Failed! 0x%08X \n"), waitStatus));
                // bail out
            return 0;
        }

        if (pHCDevice->DriverShutdown) {
            DbgPrintZo(1, (TEXT("SDDMAIstThread: Thread Exiting\n")));
            return 0;
        }

        DbgPrintZo(SDH_INTERRUPT_ZONE, (TEXT("SDDMAIst+++++++++++++ \n")));
        ACQUIRE_LOCK(pHCDevice);
        HandleDMAInterrupt( pHCDevice );
        RELEASE_LOCK(pHCDevice);
        DbgPrintZo(SDH_INTERRUPT_ZONE, (TEXT("SDDMAIst-------------- \n")));
        
        InterruptDone(pHCDevice->dwDmaSysIntr);
    }
}
BOOL SDControllerISTHandler(PSDH_HARDWARE_CONTEXT pHCDevice, BOOL fTimeOut)
{
    DWORD interrupts;    // current interrupts
    DWORD interruptMask;
    BOOL  fForceTimeout;
    DbgPrintZo(SDH_INTERRUPT_ZONE, (TEXT("SDControllerISTHandler+++++++++++++ \n")));

    interrupts = GetMMCInterrupts(pHCDevice);

    // We have observed that sometimes, after an abrupt card removal and insert,
    // the controller will "stall" during the first data transfer, there will be
    // no hardware timeout.  If this is the case, we will simulate a hardware timeout 
    // interrupt.
    fForceTimeout = FALSE;
    if (fTimeOut) {
        // check if the MMC_IREG_DAT_ERR interrupt is enabled 
        interruptMask = (~(READ_MMC_REGISTER_DWORD(pHCDevice, MMC_IMASK))) 
                        & MMC_IREG_INTERRUPTS;
        if( interruptMask & MMC_IREG_DAT_ERR )
        {
            DbgPrintZo(SDCARD_ZONE_WARN, 
                (TEXT("SDControllerIstThread: Unexpected hardware timeout!\n")));
            interrupts |= MMC_IREG_DAT_ERR;
            fForceTimeout = TRUE;
        }
        pHCDevice->dwControllerIstTimeout = INFINITE;
    }


        // loop until all interrupts are serviced
    while (interrupts) {
        DbgPrintZo(SDH_INTERRUPT_ZONE, 
                (TEXT("SDControllerIstThread: Controller Interrupt: 0x%08X \n"),interrupts));

        if (interrupts & MMC_IREG_TINT) {
                // no one should be turning this on
            DEBUG_ASSERT(FALSE);
        }

        if (interrupts & MMC_IREG_RD_STALLED) {
                // no one should be turning this on
            DEBUG_ASSERT(FALSE);
        }

        if (interrupts & MMC_IREG_SDIO_SUSPEND_ACK) {
                // no one should be turning this on
            DEBUG_ASSERT(FALSE);
        }

        if (interrupts & MMC_IREG_CLOCK_IS_OFF) {
                // no one should be turning this on
            DEBUG_ASSERT(FALSE);
                // mask the interrupt
            CLOCK_OFF_INTERRUPT_OFF(pHCDevice);
        }

        if (interrupts & (MMC_IREG_END_CMD | MMC_IREG_RES_ERR)) {
            DbgPrintZo(SDH_INTERRUPT_ZONE, 
                (TEXT("SDControllerIstThread: MMC_IREG_END_CMD \n")));
            HandleEndCommandInterrupt(pHCDevice);
        }

        if (interrupts & MMC_IREG_STOP_CMD) {
            DbgPrintZo(SDH_INTERRUPT_ZONE, 
                (TEXT("SDControllerIstThread: MMC_IREG_STOP_CMD \n")));
            DEBUG_ASSERT(FALSE);
        }
        
        if (interrupts & MMC_IREG_RXFIFO_REQ ) {
            DbgPrintZo(SDH_INTERRUPT_ZONE, 
                (TEXT("SDControllerIstThread: MMC_IREG_RXFIFO_REQ \n")));
            HandleReceiveInterrupt(pHCDevice);
        }
        
        if (interrupts & MMC_IREG_TXFIFO_REQ) {
            DbgPrintZo(SDH_INTERRUPT_ZONE, 
                (TEXT("SDControllerIstThread: MMC_IREG_TXFIFO_REQ \n")));
            HandleXmitInterrupt(pHCDevice);
        }

        if (interrupts & MMC_IREG_PROG_DONE) {
            DbgPrintZo(SDH_INTERRUPT_ZONE, 
                (TEXT("SDControllerIstThread: MMC_IREG_PROG_DONE \n")));
            HandleProgramDone(pHCDevice);
        } 
            // DATA transfer done should be checked last so that the 
            // HandleReceive and HandleTransmit
            // have a chance to finish copying from the fifos
        if (interrupts & (MMC_IREG_DATA_TRAN_DONE|MMC_IREG_DAT_ERR)) {
            DbgPrintZo(SDH_INTERRUPT_ZONE, 
                (TEXT("SDControllerIstThread: MMC_IREG_DATA_TRAN_DONE \n")));
            HandleTransferDone(pHCDevice, fForceTimeout);
        }

        if (interrupts & MMC_IREG_SDIO_INT) {
            DbgPrintZo(SDH_INTERRUPT_ZONE, 
                (TEXT("SDControllerIstThread: MMC_IREG_SDIO_INT \n")));
            HandleSDIOInterrupt(pHCDevice);
        } 

        interrupts = GetMMCInterrupts(pHCDevice);
    } // while
    DbgPrintZo(SDH_INTERRUPT_ZONE, (TEXT("SDControllerISTHandler-------------- \n")));
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
//  SDControllerIstThread - IST thread for MMC Controller driver
//  Input:  pHCDevice - the controller instance
//  Output: 
//  Return: Thread exit code
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD SDControllerIstThread(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    DWORD waitStatus;    // wait status

    if (!CeSetThreadPriority(GetCurrentThread(), 
        pHCDevice->ControllerIstThreadPriority)) {
        DbgPrintZo(SDCARD_ZONE_WARN, 
            (TEXT("SDControllerIstThread: warning, failed to set CEThreadPriority \n")));
    }

    while(1) {
        waitStatus = WaitForSingleObject(pHCDevice->hControllerInterruptEvent, 
                                         pHCDevice->dwControllerIstTimeout);

        if (WAIT_FAILED == waitStatus) {
            DbgPrintZo(SDCARD_ZONE_WARN, 
                (TEXT("SDControllerIstThread: Wait Failed! 0x%08X \n"), waitStatus));
                // bail out
            return 0;
        }

        if (pHCDevice->DriverShutdown) {
            DbgPrintZo(1, (TEXT("SDControllerIstThread: Thread Exiting\n")));
            return 0;
        }
        ACQUIRE_LOCK(pHCDevice);
        SDControllerISTHandler(pHCDevice, waitStatus != WAIT_OBJECT_0 );
        RELEASE_LOCK(pHCDevice);
        InterruptDone(pHCDevice->dwSysintrSDMMC);
    }

}


void ProcessCardInsertion( void *pContext )
{
    DWORD initializationClock = SD_DEFAULT_CARD_ID_CLOCK_RATE;

    PSDH_HARDWARE_CONTEXT pHCDevice = (PSDH_HARDWARE_CONTEXT)pContext;
    if (!pHCDevice->DevicePresent) {
            // if we have stable insertion and there wasn't a device mark it
        DbgPrintZo(SDH_INTERRUPT_ZONE, 
            (TEXT("CardDetectIstThread: Device Fully Inserted ! \n"))); 
            // mark that the card is in the slot
        pHCDevice->DevicePresent = TRUE;
    
            // flag that this is the first command sent
        pHCDevice->SendInitClocks = TRUE;

            // turn the Multimedia Card power on
        MMCPowerControl( TRUE );

            // reset the clock to the ID rate
            // shut off clock first
        SDClockOff(pHCDevice);
            // set rate
        SDSetRate(pHCDevice, &initializationClock);

            // give the card some time for initialization
        Sleep(100);

            // indicate the slot change
        SDHCDIndicateSlotStateChange(pHCDevice->pHCContext, 
                                        0,
                                        DeviceInserted);
    }
}

void ProcessCardRemoval( void *pContext )
{
    PSDH_HARDWARE_CONTEXT pHCDevice = (PSDH_HARDWARE_CONTEXT)pContext;
    PSD_BUS_REQUEST pRequest;       // current request

    if( pHCDevice->DevicePresent )
    {
        DbgPrintZo(SDH_INTERRUPT_ZONE, 
            (TEXT("CardDetectIstThread: Card Removal Detected! \n"))); 
            // mark that the card has been removed
        pHCDevice->DevicePresent = FALSE;

        
            // indicate the slot change 
        SDHCDIndicateSlotStateChange(pHCDevice->pHCContext, 
                                        0,
                                        DeviceEjected); 

            // shut off clock first
        SDClockOff(pHCDevice);

        // Complete any pending request
        if((pRequest = pHCDevice->pCurrentRequest) != NULL)
        {
            ALL_INTERRUPTS_OFF(pHCDevice);

            // if we were doing a DMA transfer, stop the DMA and reset the interrupt status
            if( pHCDevice->fDMATransfer )
            {
                pHCDevice->fDMATransferCancelled = TRUE;
                pHCDevice->pDMARegisters->dcsr[pHCDevice->dwDmaChannel] &=  !(DCSR_RUN | DCSR_STOPIRQEN);

                if( !(pHCDevice->fDMAUsingDriverBuffer) )
                { // we use client provided DMA buffer.  Unlock the memory pages where it is located.
                    if( pHCDevice->pClientBuffer )
                    {
                        UnlockPages( pHCDevice->pClientBuffer, pHCDevice->dwClientBufferSize );
                        pHCDevice->pClientBuffer = NULL;
                        pHCDevice->dwClientBufferSize = 0;
                    }
                }

                #ifdef DEBUG
                    pHCDevice->fDMATransferInProgress = FALSE;
                #endif

                InterruptDone(pHCDevice->dwDmaSysIntr);
            }

            IndicateBusRequestComplete(pHCDevice->pHCContext,
                                            pRequest ,
                                            SD_API_STATUS_DEVICE_REMOVED);
        }

            // turn the Multimedia Card power off
        MMCPowerControl( FALSE );
    }
}

BOOL DriverShutdown(void *pContext)
{
    PSDH_HARDWARE_CONTEXT pHCDevice = (PSDH_HARDWARE_CONTEXT)pContext;
    return pHCDevice->DriverShutdown;
}

void SDControllerPowerDown(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    ASSERT( pHCDevice );
    if( !pHCDevice )
        return;

    // Notify the SD Bus driver of the PowerDown event
    SDHCDPowerUpDown(pHCDevice->pHCContext, FALSE, FALSE, 0);

    // shut off clock first
    SDClockOff(pHCDevice);

    // turn the Multimedia Card power on
    MMCPowerControl( FALSE );
}

void SDControllerPowerUp(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    ASSERT( pHCDevice );
    if( !pHCDevice )
        return;

    // Notify the SD Bus driver of the PowerUp event
    SDHCDPowerUpDown(pHCDevice->pHCContext, TRUE, FALSE, 0);

    // simulate a card ejection/insertion
    SimulateCardInsertion();

}

#ifdef DEBUG

void DumpRegisters(PSDH_HARDWARE_CONTEXT pController)
{
    BOOL fQuit = TRUE;
#ifdef EXTENSIVE_DEBUGGING
    fQuit = FALSE;
#endif
    if( !fQuit )
    {
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("SD/MMC Registers Dump Begin\r\n")));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("STRPC =0x%08X\n"), pController->pSDMMCRegisters->strpc));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("STAT  =0x%08X\n"), pController->pSDMMCRegisters->stat));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("CLKRT =0x%08X\n"), pController->pSDMMCRegisters->clkrt));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("SPI   =0x%08X\n"), pController->pSDMMCRegisters->spi));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("CMDAT =0x%08X\n"), pController->pSDMMCRegisters->cmdat));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("RESTO =0x%08X\n"), pController->pSDMMCRegisters->resto));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("RDTO  =0x%08X\n"), pController->pSDMMCRegisters->rdto));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("BLKLE =0x%08X\n"), pController->pSDMMCRegisters->blkle));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("NOB   =0x%08X\n"), pController->pSDMMCRegisters->nob));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("PRTBU =0x%08X\n"), pController->pSDMMCRegisters->prtbu));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("IMASK =0x%08X\n"), pController->pSDMMCRegisters->imask));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("IREG  =0x%08X\n"), pController->pSDMMCRegisters->ireg));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("CMD   =0x%08X\n"), pController->pSDMMCRegisters->cmd));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("ARGH  =0x%08X\n"), pController->pSDMMCRegisters->argh));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("ARGL  =0x%08X\n"), pController->pSDMMCRegisters->argl));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("RES   =0x%08X\n"), pController->pSDMMCRegisters->res));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("RXFIFO=----------\n")));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("TXFIFO=----------\n")));
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("SD/MMC Registers Dump End\r\n")));
    }
}

void DumpGPIORegisters(PSDH_HARDWARE_CONTEXT pController)
{
    BOOL fQuit = TRUE;
#ifdef EXTENSIVE_DEBUGGING
    fQuit = FALSE;
#endif
    if( !fQuit )
    {
        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("GPIO Registers Dump Begin\r\n")));

        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("Alt. Function Select Registers:\n GAFR0_L=0x%08X, GAFR1_L=0x%08X, GAFR2_L=0x%08X, GAFR3_L=0x%08X\n GAFR0_U=0x%08X, GAFR1_U=0x%08X, GAFR2_U=0x%08X, GAFR3_U=0x%08X\n"),
            pController->pGPIORegisters->GAFR0_L, pController->pGPIORegisters->GAFR1_L, pController->pGPIORegisters->GAFR2_L, pController->pGPIORegisters->GAFR3_L,
            pController->pGPIORegisters->GAFR0_U, pController->pGPIORegisters->GAFR1_U, pController->pGPIORegisters->GAFR2_U, pController->pGPIORegisters->GAFR3_U));

        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("Pin direction Registers:\n GPDR0=0x%08X, GPDR1=0x%08X, GPDR2=0x%08X, GPDR3=0x%08X\n"),
            pController->pGPIORegisters->GPDR0, pController->pGPIORegisters->GPDR1, pController->pGPIORegisters->GPDR2, pController->pGPIORegisters->GPDR3));

        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("Pin Level Registers:\n GPLR0=0x%08X, GPLR1=0x%08X, GPLR2=0x%08X, GPLR3=0x%08X\n"),
            pController->pGPIORegisters->GPLR0, pController->pGPIORegisters->GPLR1, pController->pGPIORegisters->GPLR2, pController->pGPIORegisters->GPLR3));

        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("Pin Rising Edge Detect Enable Registers:\n GRER0=0x%08X, GRER1=0x%08X, GRER2=0x%08X, GRER3=0x%08X\n"),
            pController->pGPIORegisters->GRER0, pController->pGPIORegisters->GRER1, pController->pGPIORegisters->GRER2, pController->pGPIORegisters->GRER3));

        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("Pin Falling Edge Detect Enable Registers:\n GFER0=0x%08X, GFER1=0x%08X, GFER2=0x%08X, GFER3=0x%08X\n"),
            pController->pGPIORegisters->GFER0, pController->pGPIORegisters->GFER1, pController->pGPIORegisters->GFER2, pController->pGPIORegisters->GFER3));

        DbgPrintZo(SDCARD_ZONE_WARN, (TEXT("GPIO Registers Dump End\r\n")));
    }
}

#endif

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE
