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
//    sd.h   
//
// Abstract:
//
//    definitions for PXA27x Controller
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _SDH_DEFINED
#define _SDH_DEFINED

#include <bulverde.h>

#define SDH_TAG 'SDH'

#define MAXIMUM_SDCLOCK_FREQUENCY 20000000
#define NUM_BYTE_FOR_POLLING_MODE 0x800
#define TRANSFER_TIMEOUT_CONSTANT 10
#define TRANSFER_TIMEOUT_FACTOR 5

// DMA definitions

/**
 * DCSR Register 
 **/
#define DCSR_BUSERRINTR     (0x1U <<  0)  // Bus error status bit
#define DCSR_STARTINTR      (0x1U <<  1)  // Descriptor fetch status 
#define DCSR_ENDINTR        (0x1U <<  2)  // finish status
#define DCSR_STOPINTR       (0x1U <<  3)  // stopped status
#define DCSR_REQPEND        (0x1U <<  8)  // Request Pending (read-only)
#define DCSR_EOR_INTR       (0x1U <<  9)
#define DCSR_CMP_ST         (0x1U << 10)
#define DCSR_STARTIRQEN     (0x1U << 21) // Enable the start interrupt (when the descriptor is loaded)
#define DCSR_CLR_CMP_ST     (0x1U << 24)
#define DCSR_SET_CMP_ST     (0x1U << 25)
#define DCSR_EOR_STOP_EN    (0x1U << 26)
#define DCSR_EOR_JMP_ENT    (0x1U << 27)
#define DCSR_EOR_IRQ_EN     (0x1U << 28)
#define DCSR_STOPIRQEN      (0x1U << 29) // Enable the stopped interrupt (when the descriptor is done)
#define DCSR_NOFETCH        (0x1U << 30) // Descriptor fetch mode, 0 = fetch
#define DCSR_RUN            (0x1U << 31) // run, 1=start


/**
 * DCMD Register
 **/
#define DCMD_LEN            (0x1U <<  0)
#define DCMD_WIDTH          (0x1U << 14)
#define DCMD_SIZE           (0x1U << 16)
#define DCMD_FLY_BY_T       (0x1U << 19)
#define DCMD_FLY_BY_S       (0x1U << 20)
#define DCMD_END_IRQ_EN     (0x1U << 21)
#define DCMD_START_IRQ_EN   (0x1U << 22)
#define DCMD_ADDR_MODE      (0x1U << 23)
#define DCMD_CMP_EN         (0x1U << 25)
#define DCMD_FLOW_TRG       (0x1U << 28)
#define DCMD_FLOW_SRC       (0x1U << 29)
#define DCMD_INC_TRG_ADDR   (0x1U << 30)
#define DCMD_INC_SRC_ADDR   (0x1U << 31)

#define DMA_MAP_VALID_MASK  (0x1U <<  7)  // Request is mapped to a valid channel indicated by DRCMRx(3:0)

#define DMA_INTERRUPT_REGISTER  0x400000f0

#define MAXIMUM_DMA_TRANSFER_SIZE 0x1fff
#define SDIO_DMA_ALIGNMENT        32

#define SDH_DMA_CONTROLLER_PRIORITY 99

#define SDIO_TX_FIFO            0x41100044  // address of the SDIO transmit FIFO register
#define SDIO_RX_FIFO            0x41100040  // address of the SDIO receive FIFO register

#define DMA_CHMAP_SDIO_RX       21          // DRCMR21  - map dma channel for SDIO receive request
#define DMA_CHMAP_SDIO_TX       22          // DRCMR22  - map dma channel for AC97 transmit request

#define MMC_RXFIFO_SIZE 32
#define MMC_TXFIFO_SIZE 32

    // GPIO definitions for the MMC controller
#define MMCCLK_PIN                  GPIO_32;
#define MMCCMD_PIN                  GPIO_112;
#define MMCDAT0_PIN                 GPIO_92;
#define MMCDAT1_PIN                 GPIO_109;
#define MMCDAT2_PIN                 GPIO_110;
#define MMCDAT3_PIN                 GPIO_111;

#define GPIO6_MMCCLK_PIN            GPIO_6
#define GPIO8_MMCCS0_PIN            GPIO_8
#define GPIO10_DAT0_PIN_POLL        GPIO_10
#define GPIO12_DAT1_PIN_INTERRUPT   GPIO_12

    // GPIO control register offsets
#define GPIO_GPLR0  0x0000  // pin level register
#define GPIO_GRER0  0x0030  // Rising Edge Detect register       
#define GPIO_GFER0  0x003C  // Falling Edge Detect register
#define GPIO_GPSR0  0x0018  // pin output select register
#define GPIO_GPDR0  0x000C  // pin direction register 
#define GPIO_GAFR0_L 0x00054 // alternate function register
    
#define GPIO_ALT_FN_CLEAR_BITS 0x03
#define GPIO_ALT_FN_1_BITS     0x01
#define GPIO_6_ALTFN_SHIFT     12
#define GPIO_8_ALTFN_SHIFT     16
#define GPIO_10_ALTFN_SHIFT    20
#define GPIO_12_ALTFN_SHIFT    24

    // MMC Controller register offsets
#define MMC_STRPCL 0x0000   // clock start/stop
#define MMC_STAT   0x0004   // MMC Status register
#define MMC_CLKRT  0x0008   // clock rate
#define MMC_SPI    0x000C   // SPI mode control
#define MMC_CMDAT  0x0010   // command/response/data sequence
#define MMC_RESTO  0x0014   // response timeout
#define MMC_RDTO   0x0018   // expected data read timeout
#define MMC_BLKLEN 0x001C   // data phase block length
#define MMC_NOB    0x0020   // number of blocks
#define MMC_PRTBUF 0x0024   // partial MMC TX Fifo
#define MMC_IMASK  0x0028   // interrupt mask
#define MMC_IREG   0x002C   // interrupt register
#define MMC_CMD    0x0030   // command
#define MMC_ARGH   0x0034   // MSW of command argument
#define MMC_ARGL   0x0038   // LSW of command argument
#define MMC_RES    0x003C   // response Fifo

    // OS Timer register offsets
#define OST_COUNT_REG       0x0010   // OS Timer count register

    // bit definitions for MMC_STRPCL
#define MMC_STRPCL_START_CLOCK 0x02
#define MMC_STRPCL_STOP_CLOCK  0x01

    // bit definitions for MMC_IMASK
#define MMC_IMASK_DATA_TRAN_DONE_INT_MASKED   (1 << 0)
#define MMC_IMASK_PROG_DONE_INT_MASKED        (1 << 1)
#define MMC_IMASK_END_CMD_INT_MASKED          (1 << 2)
#define MMC_IMASK_STOP_CMD_INT_MASKED         (1 << 3)
#define MMC_IMASK_CLOCK_OFF_INT_MASKED        (1 << 4)
#define MMC_IMASK_RXFIFO_REQ_INT_MASKED       (1 << 5)
#define MMC_IMASK_TXFIFO_REQ_INT_MASKED       (1 << 6)
#define MMC_IMASK_TRANSMIT_TIMEOUT_INT_MASKED (1 << 7)
#define MMC_IMASK_DATA_ERROR_INT_MASKED       (1 << 8)
#define MMC_IMASK_RESPONSE_ERROR_INT_MASKED   (1 << 9)
#define MMC_IMASK_STALLED_READ_INT_MASKED     (1 << 10)
#define MMC_IMASK_SDIO_INT_MASKED             (1 << 11)
#define MMC_IMASK_SDIO_SUSPEND_ACK_INT_MASKED (1 << 12)
#define MMC_IMASK_ALL_INTERRUPTS_MASKED      (MMC_IMASK_DATA_TRAN_DONE_INT_MASKED   | \
                                              MMC_IMASK_PROG_DONE_INT_MASKED        | \
                                              MMC_IMASK_END_CMD_INT_MASKED          | \
                                              MMC_IMASK_STOP_CMD_INT_MASKED         | \
                                              MMC_IMASK_CLOCK_OFF_INT_MASKED        | \
                                              MMC_IMASK_RXFIFO_REQ_INT_MASKED       | \
                                              MMC_IMASK_TXFIFO_REQ_INT_MASKED)      | \
                                              MMC_IMASK_TRANSMIT_TIMEOUT_INT_MASKED | \
                                              MMC_IMASK_DATA_ERROR_INT_MASKED       | \
                                              MMC_IMASK_RESPONSE_ERROR_INT_MASKED   | \
                                              MMC_IMASK_STALLED_READ_INT_MASKED     | \
                                              MMC_IMASK_SDIO_INT_MASKED             | \
                                              MMC_IMASK_SDIO_SUSPEND_ACK_INT_MASKED
    // bit definitions for MMC_IREG
#define MMC_IREG_DATA_TRAN_DONE             (1 << 0)
#define MMC_IREG_PROG_DONE                  (1 << 1)
#define MMC_IREG_END_CMD                    (1 << 2)
#define MMC_IREG_STOP_CMD                   (1 << 3)
#define MMC_IREG_CLOCK_IS_OFF               (1 << 4)
#define MMC_IREG_RXFIFO_REQ                 (1 << 5)
#define MMC_IREG_TXFIFO_REQ                 (1 << 6)
#define MMC_IREG_TINT                       (1 << 7)
#define MMC_IREG_DAT_ERR                    (1 << 8)
#define MMC_IREG_RES_ERR                    (1 << 9)
#define MMC_IREG_RD_STALLED                 (1 << 10)
#define MMC_IREG_SDIO_INT                   (1 << 11)
#define MMC_IREG_SDIO_SUSPEND_ACK           (1 << 12)
#define MMC_IREG_INTERRUPTS                 0x1FFF

    // bit definitions for MMC_CMDAT
#define MMC_CMDAT_RESPONSE_NONE 0x00        // no response
#define MMC_CMDAT_RESPONSE_R1   0x01        // expected R1 response
#define MMC_CMDAT_RESPONSE_R2   0x02        // expected R2 response
#define MMC_CMDAT_RESPONSE_R3   0x03        // expected R3 response
#define MMC_CMDAT_DATA_EN       (1 << 2)    // data transfer to follow
#define MMC_CMDAT_DATA_WRITE    (1 << 3)    // data transfer is a write
#define MMC_CMDAT_STREAM        (1 << 4)    // data transfer is stream mode
#define MMC_CMDAT_EXPECT_BUSY   (1 << 5)    // the command uses busy signalling
#define MMC_CMDAT_INIT          (1 << 6)    // add init clocks
#define MMC_CMDAT_DMA_ENABLE    (1 << 7)    // enable DMA
#define MMC_CMDAT_SD_4DAT       (1 << 8)    // enable 4 bit data transfers
#define MMC_CMDAT_STOP_TRAN     (1 << 10)   // stop data transmission
#define MMC_CMDAT_SDIO_INT_EN   (1 << 11)   // enable controller to check for an SDIO interrupt from the card
#define MMC_CMDAT_SDIO_SUSPEND  (1 << 12)   // SDIO CMD 52, suspend current data transfer
#define MMC_CMDAT_SDIO_RESUME   (1 << 13)   // SDIO CMD 52, resume a suspended data transfer

    // bit definitions for MMC_STAT
#define MMC_STAT_READ_TIMEOUT          (1 << 0)
#define MMC_STAT_RESPONSE_TIMEOUT      (1 << 1)
#define MMC_STAT_WRITE_DATA_CRC_ERROR  (1 << 2)

#define MMC_STAT_READ_DATA_CRC_ERROR   (1 << 3)
#define MMC_STAT_SPI_READ_TOKEN_ERROR  (1 << 4)
#define MMC_STAT_RESPONSE_CRC_ERROR    (1 << 5)

#define MMC_STAT_CLOCK_ENABLED         (1 << 8)

#define MMC_STAT_FLASH_ERROR           (1 << 9)
#define MMC_STAT_SPI_WR_ERROR          (1 << 10)

#define MMC_STAT_DATA_TRANSFER_DONE    (1 << 11)
#define MMC_STAT_PROGRAM_DONE          (1 << 12)
#define MMC_STAT_END_CMD_RES           (1 << 13)

#define MMC_STAT_RD_STALLED            (1 << 14)
#define MMC_STAT_SDIO_INT              (1 << 15)
#define MMC_STAT_SDIO_SUSPEND_ACK      (1 << 16)

    // bit definition for MMC_PRTBUF
#define MMC_PRTBUF_BUFFER_PARTIAL_FULL (1 << 0)

typedef enum {
    Idle = -1 ,
    CommandSend = 1,
    CommandComplete = 2,
    ResponseWait = 3,
    WriteDataTransfer = 4,
    WriteDataTransferDone = 5,
    ProgramWait = 6,
    WriteDataDone = 7,
    ReadDataTransfer = 8,
    ReadDataTransferDone = 9,
    ReadDataDone = 10,
} SDHSTATE;


  // PXA27x hardware specific context
typedef struct _SDH_HARDWARE_CONTEXT {
   volatile BULVERDE_GPIO_REG     *pGPIORegisters;     // GPIO registers
   volatile BULVERDE_MMC_REG      *pSDMMCRegisters;    // SD/MMC controller registers
   volatile BULVERDE_CLKMGR_REG   *pClkMgrRegisters;   // Clock Manager registers
   volatile BULVERDE_DMA_REG      *pDMARegisters;      // DMA control registers

   LPCTSTR              pszActiveKey;
   HANDLE               hBusAccessHandle;
   PSDCARD_HC_CONTEXT   pHCContext;      // the host controller context
   PSD_BUS_REQUEST      pCurrentRequest; // Current Processing Request.
   BOOL                 fCurrentRequestFastPath;
   SD_API_STATUS        FastPathStatus;

   DWORD                dwSDMMCIrq;       // SD/MMC Controller interrupt IRQ value
   DWORD                dwSysintrSDMMC;   // SD/MMC controller interrupt SYSINTR value
   HANDLE               hControllerInterruptEvent;     // controller interrupt event
   HANDLE               hControllerInterruptThread;    // controller interrupt thread
   int                  ControllerIstThreadPriority;   // controller IST thread priority
   DWORD                dwControllerIstTimeout;
   DWORD                dwPollingModeSize;          // Fast path polling mode size.

   SYSTEM_INFO          systemInfo;
   
   PBYTE                pDMABuffer;             // pointer to buffers used for DMA transfers
   PHYSICAL_ADDRESS     pDMABufferPhys;         // physical address of the SMA buffer
   volatile DMADescriptorChannelType *pDMADescriptors;   // array of DMA descriptors
   PHYSICAL_ADDRESS     pDMADescriptorsPhys;    // physical address of the DMA descriptors buffer
   DWORD                dwDmaBufferSize;        // DMA buffer size (optional, may be 0)
   TCHAR                wszDmaIsrDll[1024];    // DMA ISR handler DLL name
   TCHAR                wszDmaIsrHandler[256]; // DMA ISR handler function name
   DWORD                dwDmaIRQ;               // DMA interrupt IRQ
   DWORD                dwDmaSysIntr;           // DMA interrupt SYSINT value
   HANDLE               hDMAIsrHandler;         // handle to DMA ISR handler
   HANDLE               hDMAInterruptEvent;     // DMA interrupt event
   DWORD                DmaIstThreadPriority;   // DMA IST thread priority
   HANDLE               hDmaInterruptThread;    // handle to the DMA IST thread
   DWORD                dwDmaChannel;           // DMA channel to use or 0xffffffff if no DMA
   BOOL                 fDMATransfer;           // if TRUE, data will be transferred via DMA
   BOOL                 fDMAUsingDriverBuffer;  // if TRUE, we transfer data to/from driver allocated buffer, otherwise we use client provided buffer
   BOOL                 fDMATransferCancelled;  // if TRUE, the DMA transfer is being cancelled
   DWORD                dwClientBufferSize;
   PVOID                pClientBuffer;
   PDWORD               pPFNs;                  // arra of pointers to lock pages of the data buffer
   DWORD                nPFNCount;              // size (in DWORDs) of buffer pointed by pPFNs
   DWORD                dwPageOffset;           // offset of the next block of data to be transferred
   DWORD                dwPFNIndex;             // index of the next page to be transferred
   DWORD                dwBytesRemaining;       // number of bytes remaining for transfer
#ifdef DEBUG
   BOOL                 fDMATransferInProgress;
#endif

   BOOL                 DriverShutdown;                // controller shutdown
   CRITICAL_SECTION     ControllerCriticalSection;     // controller critical section 
   BOOL                 SendInitClocks;                // flag to indicate that we need to send the init clock
   WCHAR                RegPath[256];                  // reg path  
   SDHSTATE             CurrentState;                  // current transfer state
   UCHAR                RcvBuffer[MMC_RXFIFO_SIZE];    // receive buffer
   UCHAR                XmitBuffer[MMC_TXFIFO_SIZE];   // xmit buffer
   BOOL                 fSDIOEnabled;                  // SDIO interrupts enabled
   BOOL                 fSDIOInterruptPending;         // indicates that and SDIO interupt has occured
   BOOL                 f4BitMode;                     // indicates that 4 bit data transfer mode is enabled
   BOOL                 fClockAlwaysOn;                // indicates that MMC clock should always remain ON
   BOOL                 fClockOnIfInterruptsEnabled;   // indicates that clock should remain on if SDIO interrupts are enabled
   BOOL                 DevicePresent;                 // device is present in the slot

   DWORD                dwSDClockFrequency;            // current SD clock frequency (Hz)
   DWORD                dwMaximumSDClockFrequency;     // maximum SD clock frequency (Hz)

   CRITICAL_SECTION     intrRegCriticalSection;        // imask register critical section

}SDH_HARDWARE_CONTEXT, *PSDH_HARDWARE_CONTEXT;

#define SetCurrentState(pHC, d) \
{\
    ((pHC)->CurrentState = (d)); \
}

#define ACQUIRE_LOCK(pHC) EnterCriticalSection(&(pHC)->ControllerCriticalSection)
#define RELEASE_LOCK(pHC) LeaveCriticalSection(&(pHC)->ControllerCriticalSection)

    // prototypes for handlers
BOOLEAN SDHCancelIoHandler(PSDCARD_HC_CONTEXT pHCContext,DWORD Slot, PSD_BUS_REQUEST pRequest);
SD_API_STATUS SDHBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext,DWORD Slot, PSD_BUS_REQUEST pRequest);
SD_API_STATUS SDHSlotOptionHandler(PSDCARD_HC_CONTEXT    pHCContext,
                                     DWORD                 SlotNumber, 
                                     SD_SLOT_OPTION_CODE   Option, 
                                     PVOID                 pData,
                                     ULONG                 OptionSize);

    // other prototypes
SD_API_STATUS SDDeinitialize(PSDCARD_HC_CONTEXT pHCContext);
SD_API_STATUS SDInitialize(PSDCARD_HC_CONTEXT pHCContext);

#define SDH_INTERRUPT_ZONE    SDCARD_ZONE_0
#define SDH_SEND_ZONE         SDCARD_ZONE_1
#define SDH_RESPONSE_ZONE     SDCARD_ZONE_2
#define SDH_RECEIVE_ZONE      SDCARD_ZONE_3
#define SDH_CLOCK_ZONE        SDCARD_ZONE_4
#define SDH_TRANSMIT_ZONE     SDCARD_ZONE_5
#define SDH_SDBUS_INTERACTION_ZONE     SDCARD_ZONE_7

#define SDH_INTERRUPT_ZONE_ON ZONE_ENABLE_0
#define SDH_SEND_ZONE_ON      ZONE_ENABLE_1
#define SDH_RESPONSE_ZONE_ON  ZONE_ENABLE_2
#define SDH_RECEIVE_ZONE_ON   ZONE_ENABLE_3
#define SDH_CLOCK_ZONE_ON     ZONE_ENABLE_4
#define SDH_TRANSMIT_ZONE_ON  ZONE_ENABLE_5
#define SDH_SDBUS_INTERACTION_ZONE_ON  ZONE_ENABLE_7

#define SDH_CARD_CONTROLLER_PRIORITY 100
#define SDH_DEFAULT_RESPONSE_TIMEOUT_CLOCKS 64
#define SDH_DEFAULT_DATA_TIMEOUT_CLOCKS 0xFFFF

#define SDH_RESPONSE_FIFO_DEPTH              8  // 
      
#define SDH_MAX_BLOCK_SIZE           1023
#define SDH_MIN_BLOCK_SIZE           32

void  ProcessCardInsertion(void *pContext);
void  ProcessCardRemoval(void *pContext);
BOOL  DriverShutdown(void *pContext);
BOOL LoadRegistrySettings( HKEY hKeyDevice, PSDH_HARDWARE_CONTEXT pController );

    // platform specific functions
BOOL  InitializeHardware( HANDLE hBusAccessHandle );
void  UnInitializeHardware();

BOOL LoadPlatformRegistrySettings( HKEY hKeyDevice );

void  MMCPowerControl( BOOL fPowerOn );

BOOL  IsCardWriteProtected();
BOOL  IsCardPresent();
BOOL  SetupCardDetectIST(void *pContext);
void  CleanupCardDetectIST();
void  SimulateCardInsertion();

#endif

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

