//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ata.cpp
//
//  Base ATA MX device abstraction.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <devload.h>
#include <pm.h>
#include <nkintr.h>
#include <ceddk.h>
#include <diskio.h>
#include <atapi2.h>
#include <ata.h>
#include <ata_time.h>

//------------------------------------------------------------------------------
// External Functions
extern int CSPATACLOCKPERIOD();
extern BOOL BSPATAIOMUXConfig(void);
extern UINT32 CSPATAGetBaseRegAddr();
extern DWORD CSPSPBAGetBaseRegAddr();
extern void CSPATASYSINTRDone();
extern BOOL CSPATAConfigInt(HANDLE hIRQEvent);
extern BOOL CSPATAPageAlign(DWORD dwPhys,DWORD dwSize,BOOL fRead);
extern void CSPATADMAAddItem(DWORD dwPhys,DWORD dwSize, DWORD dwDescCount, BOOL bBufferEnd,BOOL fRead);
extern void CSPATADMAStart();
extern DWORD CSPATARegControl(BYTE bTransferMode, BOOL fRead);
extern void CSPATADMAStop();
extern BOOL CSPATADMAInit();
extern BOOL CSPATADMADeInit();
extern DWORD CSPATARegTablePhy();
extern DWORD CSPATARegFIFOAlarm();
extern DWORD CSPATARegIntEnable();

//------------------------------------------------------------------------------
// Global Variables 
LONG CMXDisk::m_lDeviceCount = 0;

//------------------------------------------------------------------------------
// Local Variables 

static TCHAR *g_szATAHardDisk = TEXT("Hard Disk");

void DumpBuffer(PBYTE pInBuf, USHORT uBufSize);
void DumpFIFO(PBYTE g_pATA_BASE);

//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
//  Function:  AtaIntrServThread
//
//  This is the interrupt service thread for ATA interrupts.  
//
//  Parameters:
//      lpParam
//          [in] Thread data passed to the function using the 
//          lpParameter parameter of the CreateThread function.
//
//  Returns:
//      Returns thread exit code.
//
//-----------------------------------------------------------------------------
DWORD IntrServThread (LPVOID lpParam)
{
    DWORD rc = TRUE;
    CPort *pPort = (CPort *)lpParam;

    for (;;)
    {
        if(WaitForSingleObject(pPort->m_hIRQEvent, INFINITE) == WAIT_OBJECT_0)
        {
            // Signal the thread that called WaitForInterrupt to complete 
            // the interrupt servicing.
            SetEvent(pPort->m_hWaitEvent);
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

// ----------------------------------------------------------------------------
// Function: CreatePCIHD
//     Spawn function called by IDE/ATA controller enumerator
//
// Parameters:
//     hDevKey -
// ----------------------------------------------------------------------------

EXTERN_C
CDisk *
CreateMXHD(
    HKEY hDevKey
    )
{
    return new CMXDisk(hDevKey);
}

// ----------------------------------------------------------------------------
// Function: CMXDisk
//     Constructor
//
// Parameters:
//     hKey -
// ----------------------------------------------------------------------------

CMXDisk::CMXDisk(
    HKEY hKey
    ) : CDisk(hKey)
{
    InterlockedIncrement(&m_lDeviceCount);

    g_pVAtaReg = NULL; 
    fSDMAIntrEnable = FALSE;
    // PREFAST
    
    // ATAPI
    m_pSterileCdRomReadRequest = NULL;
    m_cbSterileCdRomReadRequest = 0;
}

// ----------------------------------------------------------------------------
// Function: ~CMXDisk
//     Destructor
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CMXDisk::~CMXDisk(
    )
{
    DeInitDMA(); 

    // atapi
    m_pSterileCdRomReadRequest = NULL;
    m_cbSterileCdRomReadRequest = 0;
    
    InterlockedDecrement(&m_lDeviceCount);
    // PREFAST
}

// ----------------------------------------------------------------------------
// Function: CopyDiskInfoFromPort
//     This function is not used
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void
CMXDisk::CopyDiskInfoFromPort(
    )
{
    ASSERT(m_pPort->m_dwRegBase != 0);
    m_pATAReg = (PBYTE)m_pPort->m_dwRegBase;
    m_pATARegAlt = (PBYTE)m_pPort->m_dwRegAlt;

    ASSERT(m_pPort->m_dwBMR != 0);
}

// ----------------------------------------------------------------------------
// Function: WaitForInterrupt
//     Wait for interrupt
//
// Parameters:
//     dwTimeOut -
// ----------------------------------------------------------------------------

BOOL
CMXDisk::WaitForInterrupt(
    DWORD dwTimeOut
    )
{
    BYTE bStatus;
    BOOL fRet = TRUE;
    DWORD dwRet;

    DEBUGMSG(ZONE_FUNC, (TEXT("CMXDisk: WaitForInterrupt+\r\n")));
    
    bIntrPending = 0;
next_wait:    
    // wait for interrupt
    dwRet = WaitForSingleObject(m_pPort->m_hWaitEvent, dwTimeOut);
    if (dwRet == WAIT_TIMEOUT) {
        fRet = FALSE;
    }
    else {
        if (dwRet != WAIT_OBJECT_0) {
            if (!WaitForDisc(WAIT_TYPE_DRQ, dwTimeOut, 10)) {
                fRet = FALSE;
            }
        }
    }

    bIntrPending = INREG8(&g_pVAtaReg->InterruptPending);
    if(bIntrPending & 0x08)
    {
        // read status; acknowledge interrupt
        bStatus = GetBaseStatus();
        if (bStatus & ATA_STATUS_ERROR) {
            bStatus = GetError();
            fRet = FALSE;
      DEBUGMSG(ZONE_FUNC, (TEXT("CMXDisk: WaitForInterrupt(Error: %x)\r\n"), bStatus));
        }
    } else
    if(bIntrPending & 0x20)
    {
        OUTREG8(&g_pVAtaReg->InterruptClear, 0x20);
        while(INREG32(&g_pVAtaReg->FIFO_FILL) >= 2)
        {
            *((PDWORD)(m_rgbDoubleBuffer+dwDoubleBufferPos)) = INREG32(&g_pVAtaReg->FIFO_DATA_32[0]) ;
            dwDoubleBufferPos += 4;
        }
    }

    // signal interrupt done
    CSPATASYSINTRDone();
    
    if(fRet && dwTimeOut && fSDMAIntrEnable && !(bIntrPending & 0x80)) 
    {
        //fRet = FALSE;
        goto next_wait;
    }
    else
    if(fRet && dwTimeOut && !fSDMAIntrEnable && !(bIntrPending & 0x08)) 
    {
        //fRet = FALSE;
        goto next_wait;
    }
    
    DEBUGMSG(ZONE_FUNC, (TEXT("CMXDisk: WaitForInterrupt-:%d,%x\r\n"), fRet, bIntrPending));
    return fRet;
}

// ----------------------------------------------------------------------------
// Function: EnableInterrupt
//     Enable channel interrupt
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void
CMXDisk::EnableInterrupt(
    )
{
    GetBaseStatus(); // acknowledge interrupt, if pending

    OUTREG8(&g_pVAtaReg->InterruptClear, 0xff);
    OUTREG8(&g_pVAtaReg->InterruptEnable, 0x68); //0x08);
    // signal interrupt done
    CSPATASYSINTRDone();
}

// ----------------------------------------------------------------------------
// Function: ConfigureRegisterBlock
//     This function is called by DSK_Init before any other CDisk function to
//     set up the register block.
//
// Parameters:
//     dwStride -
// ----------------------------------------------------------------------------

VOID
CMXDisk::ConfigureRegisterBlock(
    DWORD dwStride
    )
{
    DEBUGMSG(ZONE_FUNC, (TEXT("CMXDisk: ConfigureRegisterBlock+\r\n")));
    m_dwStride = dwStride;
    m_dwDataDrvCtrlOffset = ATA_REG_DATA * dwStride+ATA_DRIVE_REG_OFFSET;
    m_dwFeatureErrorOffset = ATA_REG_FEATURE * dwStride+ATA_DRIVE_REG_OFFSET;
    m_dwSectCntReasonOffset = ATA_REG_SECT_CNT * dwStride+ATA_DRIVE_REG_OFFSET;
    m_dwSectNumOffset = ATA_REG_SECT_NUM * dwStride+ATA_DRIVE_REG_OFFSET;
    m_dwDrvHeadOffset = ATA_REG_DRV_HEAD * dwStride+ATA_DRIVE_REG_OFFSET;
    m_dwCommandStatusOffset = ATA_REG_COMMAND * dwStride+ATA_DRIVE_REG_OFFSET;
    m_dwByteCountLowOffset = ATA_REG_BYTECOUNTLOW * dwStride+ATA_DRIVE_REG_OFFSET;
    m_dwByteCountHighOffset = ATA_REG_BYTECOUNTHIGH * dwStride+ATA_DRIVE_REG_OFFSET;
    m_dwAltStatusOffset = ATA_DRIVE_ALT_OFFSET; //ATA_REG_ALT_STATUS_CS1 * dwStride+atabase;
    m_dwAltDrvCtrl = ATA_DRIVE_ALT_OFFSET; //ATA_REG_DRV_CTRL_CS1 * dwStride+atabase;
    DEBUGMSG(ZONE_FUNC, (TEXT("CMXDisk: ConfigureRegisterBlock-\r\n")));
}

// ----------------------------------------------------------------------------
// Function: Init
//     Initialize channel
//
// Parameters:
//     hActiveKey -
// ----------------------------------------------------------------------------

BOOL
CMXDisk::Init(
    HKEY hActiveKey
    )
{
    BOOL bRet = FALSE;

    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk: Init+")));
    m_f16Bit = TRUE; // ATA is 16-bit
    CLOCK_PERIOD = CSPATACLOCKPERIOD();

    // configure port
    if (!ConfigPort()) {
        DEBUGMSG(ZONE_INIT, (_T(
            "Ata!CMXDisk::Init> Failed to configure port; device(%u)\r\n"
            ), m_dwDeviceId));
        goto exit;
    }

    // assign the appropriate folder name
    m_szDiskName = g_szATAHardDisk;

    // finish intialization; i.e., initialize device
    bRet = CDisk::Init(hActiveKey);
    if (!bRet) {
        goto exit;
    }

    if(!InitDMA())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("MMCSD_DMAInit: cannot init DMA!\r\n")));
        goto exit;
    }

    // this function is called for the master and slave on this channel; if
    // this has already been called, then exit
    if (m_pPort->m_hIRQEvent) {
        m_dwDeviceFlags |= DFLAGS_DEVICE_INITIALIZED;
        goto exit;
    }

    // create interrupt event
    if (NULL == (m_pPort->m_hIRQEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CMXDisk::ConfigPort> Failed to create interrupt event for device(%d)\r\n"
            ), m_dwDeviceId));
        goto exit;
    }

    if(!CSPATAConfigInt(m_pPort->m_hIRQEvent))
        goto exit;

    // Create interrupt wait event
    if (NULL == (m_pPort->m_hWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CMXDisk::ConfigPort> Failed to create interrupt wait event for device(%d)\r\n"
            ), m_dwDeviceId));
        goto exit;
    }

    // Create IST for ATA interrupts
    m_pPort->m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) IntrServThread, m_pPort, 0, NULL);      
    if (!m_pPort->m_hThread) 
    {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CMXDisk::ConfigPort> Failed to create IST for device(%d)\r\n"
            ), m_dwDeviceId));
        goto exit;
    }

    BYTE bPIOMode = 0;
    BYTE bDMAMode = 0;
    BYTE bTransferMode = (BYTE)m_pPort->m_pDskReg[m_dwDeviceId]->dwTransferMode;
    BYTE bIORDYEnable = (BYTE)m_pPort->m_pDskReg[m_dwDeviceId]->dwIORDYEnable;
    switch(bTransferMode)
    {
      case  0x08:
      case  0x09:
      case  0x0a:
      case  0x0b:
      case  0x0c:
              bPIOMode = bTransferMode - 0x08;
              if(bIORDYEnable && bPIOMode >= 3)
              {
                  Sleep(10);
                  OUTREG16(&g_pVAtaReg->ATAControl, 0x41);
                  Sleep(10);
              }
              break;
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
                bDMAMode = (BYTE)(bTransferMode - 0x40);
                SetTimingRegisters(bDMAMode, UDMA_MODE, CLOCK_PERIOD);
                break;
            case 0x20:
            case 0x21:
            case 0x22:
                bDMAMode = (BYTE)(bTransferMode - 0x20);
                SetTimingRegisters(bDMAMode, MDMA_MODE, CLOCK_PERIOD);
                break;
            default:
                bPIOMode = 0;
                break;
    }
    SetTimingRegisters(bPIOMode, PIO_MODE, CLOCK_PERIOD);
//    ShowRegisters(0x01);

    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk: Init-")));

exit:;
    return bRet;
}

// ----------------------------------------------------------------------------
// Function: MainIoctl
//     This is redundant
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CMXDisk::MainIoctl(
    PIOREQ pIOReq
    )
{
    DWORD dwError;
    
    DEBUGMSG(ZONE_FUNC, (_T(
        "Ata!CMXDisk::MainIoctl> IOCTL(%d), device(%d) \r\n"
        ), pIOReq->dwCode, m_dwDeviceId));

    dwError = CDisk::MainIoctl(pIOReq);

    if (dwError == ERROR_NOT_SUPPORTED) {

        switch(pIOReq->dwCode) {

            // SCSI passthru commands
            case IOCTL_SCSI_PASS_THROUGH:
            case IOCTL_SCSI_PASS_THROUGH_DIRECT:

            // supported ATAPI commands
            case IOCTL_CDROM_READ_SG:
            case IOCTL_CDROM_TEST_UNIT_READY:
            case IOCTL_CDROM_DISC_INFO:
            case IOCTL_CDROM_EJECT_MEDIA:
            case IOCTL_CDROM_LOAD_MEDIA:

            // supported DVD commands
            case IOCTL_DVD_START_SESSION:
            case IOCTL_DVD_READ_KEY:
            case IOCTL_DVD_END_SESSION:
            case IOCTL_DVD_SEND_KEY:
            case IOCTL_DVD_GET_REGION:

            // supported audio commands
            case IOCTL_CDROM_READ_TOC:
            case IOCTL_CDROM_GET_CONTROL:
            case IOCTL_CDROM_PLAY_AUDIO_MSF:
            case IOCTL_CDROM_SEEK_AUDIO_MSF:
            case IOCTL_CDROM_STOP_AUDIO:
            case IOCTL_CDROM_PAUSE_AUDIO:
            case IOCTL_CDROM_RESUME_AUDIO:
            case IOCTL_CDROM_GET_VOLUME:
            case IOCTL_CDROM_SET_VOLUME:
            case IOCTL_CDROM_READ_Q_CHANNEL:
            case IOCTL_CDROM_GET_LAST_SESSION:
            case IOCTL_CDROM_RAW_READ:
            case IOCTL_CDROM_DISK_TYPE:
            case IOCTL_CDROM_SCAN_AUDIO:
            case IOCTL_CDROM_ISSUE_INQUIRY:

                if (IsAtapiDevice()) {
                    dwError = AtapiIoctl(pIOReq);
                }
                else {
                    dwError = ERROR_INVALID_OPERATION;
                }
                break;

            default:
                dwError = ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return dwError;
}

// ----------------------------------------------------------------------------
// Function: ConfigPort
//     Initialize IST/ISR
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CMXDisk::ConfigPort(
    )
{
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk: ConfigPort+")));

    m_pATAReg = (PBYTE)m_pPort->m_dwRegBase;
    m_pATARegAlt = (PBYTE)m_pPort->m_dwRegAlt;

    if(!InitializePort()) 
    {
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk: ConfigPort-")));
    return TRUE;
}

// SetTimingRegisters() programs host timing. "speed" is the ATA mode programmed,
// mode can be PIO_MODE, UDMA_MODE or MDMA_MODE, ClkSpd is the clock period in
// ns 
void CMXDisk::SetTimingRegisters(int speed, int mode, int ClkSpd)
{
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk: SetTimingRegisters+(%d,%d,%d)+"), speed,mode,ClkSpd));
    INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG0, 0x0000ffff, 0x00000303);
    switch(mode)
    {
        case PIO_MODE:
            INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG0, 0xffff0000,
                (((t1_spec[speed] + ClkSpd) / ClkSpd) << 16) |
                (((t2_8spec[speed] + ClkSpd) / ClkSpd) << 24)) ;
            INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG1, 0xffffffff,
                (((t2_8spec[speed] + ClkSpd) / ClkSpd) ) |
                ((((tA_spec[speed] + ClkSpd) / ClkSpd) + 2) << 8) |
                (1 << 16) |
                (((t4_spec[speed] + ClkSpd) / ClkSpd) << 24)) ;
            INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG2, 0xff, (((t9_spec[speed] + ClkSpd) / ClkSpd)));
            break ;
        case MDMA_MODE :
            INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG2, 0xffffff00,
                (((tM_spec[speed] + ClkSpd) / ClkSpd) << 8) |
                (((tJNH_spec[speed] + ClkSpd) / ClkSpd) << 16) |
                    (((tD_spec[speed] + ClkSpd) / ClkSpd) << 24)) ;
            INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG3, 0xff, (((tKW_spec[speed] + ClkSpd) / ClkSpd))) ;
            break ;
        case UDMA_MODE :
            INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG3, 0xffffff00,
                (((tACK_spec[speed] + ClkSpd) / ClkSpd) << 8) |
                (((tENV_minspec[speed] + ClkSpd) / ClkSpd) << 16) |
                ((((tRP_spec[speed] + ClkSpd) / ClkSpd) + 2) << 24)) ;
            INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG4, 0xffffffff,
                (((tZAH_spec[speed] + ClkSpd) / ClkSpd)) |
                (((tMLI_spec[speed] + ClkSpd) / ClkSpd) << 8) |
                (((tDVH_spec[speed] + ClkSpd) / ClkSpd) << 16) |
                (((tDZFS_spec[speed] + ClkSpd) / ClkSpd) << 24)) ;
            INSREG32(&g_pVAtaReg->ATA_TIME_CONFIG5, 0xffffffff,
                (((tCYC_spec[speed] + ClkSpd) / ClkSpd)) << 24 |
                (((tDVS_spec[speed] + ClkSpd) / ClkSpd)) |
                (((tCVH_spec[speed] + ClkSpd) / ClkSpd) << 8) |
                (((tSS_spec[speed] + ClkSpd) / ClkSpd) << 16)) ;
            break ;
        default :
            break;
    }
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk: SetTimingRegisters-")));
}


// ----------------------------------------------------------------------------
// Function: InitializePort
//     Initialize ATA Port
//
// Parameters:
//     None
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
// ----------------------------------------------------------------------------
BOOL CMXDisk::InitializePort(void)
{
    PHYSICAL_ADDRESS phyAddr;
    BOOL bRet = FALSE;

    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk: InitializePort+")));
    if(g_pVAtaReg == NULL)
    {
        phyAddr.QuadPart = CSPATAGetBaseRegAddr();
        g_pVAtaReg = (PCSP_ATA_REG) MmMapIoSpace(phyAddr, sizeof(CSP_ATA_REG), FALSE);

        // Check if virtual mapping failed
        if (g_pVAtaReg == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("CMXDisk: MmMapIoSpace failed!\r\n")));
            goto exit;
        }

        m_pATAReg = (PBYTE) (g_pVAtaReg);
        m_pATARegAlt = (PBYTE) (g_pVAtaReg);

        ConfigureRegisterBlock(4);
        
        // BSPATAIOMUXConfig configure the available IOMUX settings 
        BSPATAIOMUXConfig();
        
        SetTimingRegisters(0, PIO_MODE, CLOCK_PERIOD);
        SetTimingRegisters(0, MDMA_MODE, CLOCK_PERIOD);
        SetTimingRegisters(0, UDMA_MODE, CLOCK_PERIOD);

        OUTREG16(&g_pVAtaReg->ATAControl, 0x00);
        Sleep(20);
        OUTREG16(&g_pVAtaReg->ATAControl, 0x40);
        Sleep(20);
    }
    bRet = TRUE;

exit:;
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk: InitializePort-")));
    return bRet;
}

// ----------------------------------------------------------------------------
// Function: ShowRegisters
//     Print current ATA registers
//
// Parameters:
//     None
// Returns:
//     None
// ----------------------------------------------------------------------------
void CMXDisk::ShowRegisters(UCHAR usel)
{
    if(usel & 0x01)
    {
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG0 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG0, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG0)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG1 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG1, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG1)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG2 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG2, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG2)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG3 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG3, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG3)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG4 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG4, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG4)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG5 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG5, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG5)));

        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATAControl %p,0x%02x\n"), &g_pVAtaReg->ATAControl, INREG16(&g_pVAtaReg->ATAControl)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: InterruptPending %p,0x%02x\n"), &g_pVAtaReg->InterruptPending, INREG8(&g_pVAtaReg->InterruptPending)));
    } 

    if(usel & 0x02)
    {
    }
}

#ifdef USE_SDMA

// ----------------------------------------------------------------------------
// Function: SetupDMA
//     Setup DMA transfer
//
// Parameters:
//     pSgBuf -
//     dwSgCount -
//     fRead -
// ----------------------------------------------------------------------------
BOOL CMXDisk::SetupDMA(PSG_BUF pSgBuf, DWORD dwSgCount, BOOL fRead)
{
    BOOL rc = TRUE;
#ifdef USE_SDMA_SG_LIST
    BOOL fAlign = TRUE;
#else
    BOOL fAlign = FALSE;
#endif
    DWORD dwLockedPages = 0;
    DWORD dwPhys;
    DWORD dwSize;
    DWORD dwDescCount;
    BOOL bBufferEnd;


    m_pDMASgBuf = pSgBuf;
    m_dwAlignedSgCount = 0;
    m_dwAlignedDescCount = 0;
    m_dwAlignedSgBytes = 0;

    OUTREG16(&g_pVAtaReg->ATAControl, 0x40);

    DWORD currentSg = 0;
    DWORD currentDesc = 0;
    DWORD pdwPages[ATA_MAX_PAGE_COUNT];

    // For each ATA watermark aligned scatter/gather buffer.
    while((currentSg < dwSgCount) && fAlign)
    {
        LPBYTE pBuffer = (LPBYTE)pSgBuf[currentSg].sb_buf; 

        if (!LockPages(pBuffer, pSgBuf[currentSg].sb_len, pdwPages,
                       fRead ? LOCKFLAG_WRITE : LOCKFLAG_READ))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("CMXDisk::SetupDMA> LockPages ")
                                  TEXT("failed\r\n")));

            // Try to fall back to copy using aligned buffer
            fAlign = FALSE;
            break;
        }
        else
        {
            // Keep track of how many pages we have locked so we can
            // unlock them if we encounter a failure
            dwLockedPages++;
        }

        DWORD dwMaxPages =
            ADDRESS_AND_SIZE_TO_SPAN_PAGES(pSgBuf[currentSg].sb_buf,
                                           pSgBuf[currentSg].sb_len);
        DWORD dwTempLen = pSgBuf[currentSg].sb_len;
        DWORD dwTempPtr = (DWORD)pBuffer;

        for(DWORD i = 0; (i < dwMaxPages) && (dwTempLen > 0) && fAlign; i++)
        {
            // try and rule out broken values
            DEBUGCHK(pdwPages[i] != 0);

            dwPhys = pdwPages[i] <<
                UserKInfo[KINX_PFN_SHIFT]; // get the start address of pages
            DWORD dwOffset = dwTempPtr &
                (UserKInfo[KINX_PAGESIZE] - 1); // offset into page, if any
            dwSize = UserKInfo[KINX_PAGESIZE] - dwOffset;

            if(dwSize > dwTempLen)
            {
                dwSize = dwTempLen;
            }

            dwPhys += dwOffset;

            dwTempLen -= dwSize;
            dwTempPtr += dwSize;

            if(!CSPATAPageAlign(dwPhys,dwSize,fRead))
            {
                //DEBUGMSG(ZONE_FUNC, (_T("CMXDisk::SetupDMA unaligned page (sg = %d, page = %d, size = %d\r\n"), currentSg, i, dwSize));
                UnlockPages(pBuffer,pSgBuf[currentSg].sb_len);
                --dwLockedPages;

                fAlign = FALSE;
            }
            else
            {
                dwDescCount=currentDesc;
                if(dwTempLen == 0 && (currentSg == (dwSgCount - 1)))
                    bBufferEnd = TRUE;
                else
                    bBufferEnd = FALSE;

            
                CSPATADMAAddItem(dwPhys,dwSize,dwDescCount, bBufferEnd, fRead);
                ++currentDesc;
            }
        }

        if (fAlign)
        {
            // Keep track of the aligned SG buffers.  If we encounter a
            // a misaligned length while walking through the current 
            // SG buffer pages, we will punt the remainder of the list back to
            // MapDMABuffers implementation
            m_dwAlignedSgBytes += pSgBuf[currentSg].sb_len;
            m_dwAlignedDescCount = currentDesc;
            m_dwAlignedSgCount = ++currentSg;
        }
    }

    if (!fAlign) // remain unaligned buffers
    {
        if (fRead)
        {
            dwSize = dwSectorsToTransfer * BYTES_PER_SECTOR - 
                     m_dwAlignedSgBytes;
        }
        else
        {
            dwSize = MoveDMABuffer(pSgBuf, m_dwAlignedSgCount, dwSgCount, fRead);
            DEBUGCHK(dwSize == (dwSectorsToTransfer * BYTES_PER_SECTOR -
                                m_dwAlignedSgBytes));
        }

        // DEBUGCHK(!(dwSize & (ATA_DMA_WATERMARK - 1)));

        //DEBUGMSG(ZONE_FUNC, (_T("DDKSdmaSetBufDesc (BD[%d], chan = %d, flags = 0x%x, size = %d\r\n"), dwAlignedDescCount, chan, DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP, dwSize));
        dwPhys=(DWORD)PhysDMABufferAddr.LowPart;
        dwDescCount=m_dwAlignedDescCount;
        bBufferEnd = TRUE;
        CSPATADMAAddItem(dwPhys, dwSize, dwDescCount, bBufferEnd,fRead);
    }

    CSPATADMAStart();

    OUTREG32(&g_pVAtaReg->ADMASysAddr, CSPATARegTablePhy());
    OUTREG16(&g_pVAtaReg->BlockCNT, dwSectorsToTransfer);
    OUTREG8(&g_pVAtaReg->FIFOAlarm, CSPATARegFIFOAlarm());
    OUTREG8(&g_pVAtaReg->InterruptClear, 0xff);
    OUTREG8(&g_pVAtaReg->InterruptEnable, CSPATARegIntEnable());

    return rc;
}

// ----------------------------------------------------------------------------
// Function: BeginDMA
//     Begin DMA transfer
//
// Parameters:
//     fRead -
// ----------------------------------------------------------------------------
BOOL CMXDisk::BeginDMA(BOOL fRead)
{
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk::BeginDMA+\n")));
    
    if (m_dwAlignedSgCount > 0)
    {
        CacheSync(CACHE_SYNC_DISCARD);
    }

    fSDMAIntrEnable = TRUE;
    
    BYTE bTransferMode = (BYTE)m_pPort->m_pDskReg[m_dwDeviceId]->dwTransferMode;
    OUTREG16(&g_pVAtaReg->ATAControl, CSPATARegControl(bTransferMode, fRead));
    
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk::BeginDMA-\n")));
    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: EndDMA
//     End DMA transfer
//
// Parameters:
//     None
// ----------------------------------------------------------------------------
BOOL CMXDisk::EndDMA()
{

    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk::EndDMA+\n")));

    CSPATADMAStop();

    // unlock any previous locked page ranges
    for(DWORD i = 0; i < m_dwAlignedSgCount; i++)
    {
        LPBYTE pBuffer = (LPBYTE) m_pDMASgBuf[i].sb_buf;
        UnlockPages(pBuffer,m_pDMASgBuf[i].sb_len);
    }

    fSDMAIntrEnable = FALSE;

    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: AbortDMA
//     Abort DMA transfer
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CMXDisk::AbortDMA(
    )
{
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk::AbortDMA+\n")));
    fSDMAIntrEnable = FALSE;

   return EndDMA();
}

// ----------------------------------------------------------------------------
// Function: MoveDMABuffer
//     Move data from s/g buffer to DMA buffer or vice versa
//
// Parameters:
//     fRead -
//
// ----------------------------------------------------------------------------
DWORD CMXDisk::MoveDMABuffer(PSG_BUF pSgBuf, DWORD dwSgIndex, DWORD dwSgCount, BOOL fRead)
{
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk::MoveDMABuffer(%d)+\n"), dwSgCount));

    DWORD dwOffset = 0;
    DWORD dwEnd = dwSectorsToTransfer * BYTES_PER_SECTOR - m_dwAlignedSgBytes;

    for ( ; dwSgIndex < dwSgCount; dwSgIndex++)
    {
        LPBYTE pBuffer = (LPBYTE) pSgBuf[dwSgIndex].sb_buf;
        DWORD dwSize = pSgBuf[dwSgIndex].sb_len;

        if((dwSize + dwOffset) > dwEnd)
            dwSize = dwEnd - dwOffset;

        if (fRead)
        {
            memcpy(pBuffer, pVirtDMABufferAddr + dwOffset, dwSize);
            //DEBUGMSG(ZONE_FUNC, (_T("MEMREAD %d\r\n"), dwSize));
        }

        else
        {
            memcpy(pVirtDMABufferAddr + dwOffset, pBuffer, dwSize);
            //DEBUGMSG(ZONE_FUNC, (_T("MEMWRITE %d\r\n"), dwSize));
        }

        dwOffset += dwSize;
    }

    return dwOffset;
}

// ----------------------------------------------------------------------------
// Function: CompleteDMA
//     Complete DMA transfer
//
// Parameters:
//     pSgBuf -
//     dwSgCount -
//     fRead -
// ----------------------------------------------------------------------------
BOOL CMXDisk::CompleteDMA(PSG_BUF pSgBuf, DWORD dwSgCount, BOOL fRead)
{
    DEBUGMSG(ZONE_FUNC, (_T("CMXDisk::CompleteDMA+\n")));
    fSDMAIntrEnable = FALSE;
      
    if(fRead)
    {
        if (m_dwAlignedSgCount != dwSgCount)
        {
#ifdef DEBUG
            DWORD dwSize = MoveDMABuffer(pSgBuf, m_dwAlignedSgCount, dwSgCount, fRead);
#else
            MoveDMABuffer(pSgBuf, m_dwAlignedSgCount, dwSgCount, fRead);
#endif
            DEBUGCHK(dwSize == (dwSectorsToTransfer * BYTES_PER_SECTOR - m_dwAlignedSgBytes));
        }
        
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: MapDMABuffers
//
// Allocate and map DMA buffer 
//
// Parameters:
//        pController[in] - hardware context
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CMXDisk::MapDMABuffers(void)
{
    DMA_ADAPTER_OBJECT Adapter;
    DWORD ATA_SDMA_BUFFER_SIZE= CSPATAMaxSectDMA()*512;

    DEBUGMSG(ZONE_FUNC,(_T("CMXDisk::MapDMABuffers+\r\n")));
      
    pVirtDMABufferAddr = NULL;
   
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
    pVirtDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (ATA_SDMA_BUFFER_SIZE)
                                , &(PhysDMABufferAddr), FALSE);

    // Check if DMA buffer has been allocated
    if (!PhysDMABufferAddr.LowPart || !pVirtDMABufferAddr)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ATA_MX:MapDMABuffers() - Failed to allocate DMA buffer.\r\n")));
        return(FALSE);
    }

    DEBUGMSG(ZONE_FUNC,(_T("CMXDisk::MapDMABuffers-\r\n")));
    return(TRUE);
}
//------------------------------------------------------------------------------
//
// Function: UnmapDMABuffers
//
//  This function unmaps the DMA buffers previously mapped with the
//  MapDMABuffers function.
//
// Parameters:
//        pController[in] - hardware context
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CMXDisk::UnmapDMABuffers(void)
{
    PHYSICAL_ADDRESS phyAddr;
    DMA_ADAPTER_OBJECT Adapter;
    DEBUGMSG(ZONE_FUNC,(_T("CMXDisk::UnmapDMABuffers+\r\n")));

    // PREFAST
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));

    if(pVirtDMABufferAddr)
    {
        // Logical address parameter is ignored
        phyAddr.QuadPart = 0;
        HalFreeCommonBuffer(&Adapter, 0, phyAddr, (PVOID)pVirtDMABufferAddr, FALSE);
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: InitDMA
//
//  Performs DMA channel intialization
//
// Parameters:
//        pController[in] - hardware context
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CMXDisk::InitDMA(void) 
{ 
    DEBUGMSG(ZONE_FUNC, (TEXT("CMXDisk::InitDMA+\r\n")));

    CSPATADMAInit();

    // Map the DMA buffers into driver's virtual address space
    if(!MapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ATA_MX:InitDMA() - Failed to map DMA buffers.\r\n")));
        return FALSE;
    }

    
    DEBUGMSG(ZONE_FUNC, (TEXT("CMXDisk::InitDMA-\r\n")));
    return TRUE ; 
}

BOOL CMXDisk::DeInitDMA(void) 
{
    CSPATADMADeInit();
    
    if(!UnmapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ATA_MX:UnmapDMABuffers() - Failed to Unmap DMA buffers\r\n")));
        return FALSE;
    }
    
    return TRUE ; 
}

#endif

// ----------------------------------------------------------------------------
// Function: SterilizeCdRomReadRequest
//     Sterilize user-supplied CDROM_READ request.  If *ppSafe is NULL or not
//     sufficiently large enough, then re-allocate.  If ppSafe is re-allocated,
//     then return the updated number of SGX_BUF-s through lpcSafeSgX.  This
//     function must be called within a __try/__except block.
//
// Parameters:
//     ppSafe -
//     lpcSafeSgX -
//     pUnsafe -
//     cbUnsafe -
// ----------------------------------------------------------------------------

BOOL CMXDisk::SterilizeCdRomReadRequest(
    PCDROM_READ* ppSafe,
    LPDWORD      lpcbSafe,
    PCDROM_READ  pUnsafe,
    DWORD        cbUnsafe,
    DWORD        dwArgType,
    OUT PUCHAR * saveoldptrs    
    )
{
    DWORD       i = 0, mappedbuffers;
    PCDROM_READ pSafe = NULL;
    DWORD       cbSafe = 0;
    PUCHAR      ptemp;

    // ppSafe, lpcSafeSgX, and pUnsafe are required.  However, *ppSafe may
    // be NULL.
    if ((NULL == ppSafe)|| (NULL == lpcbSafe) || (NULL == pUnsafe)) {
        return FALSE;
    }
    // Extract args so we don't have to continually dereference.
    pSafe = *ppSafe;
    cbSafe = *lpcbSafe;
    // Is unsafe request smaller than minimum?
    if (cbUnsafe < sizeof(CDROM_READ)) {
        return FALSE;
    }
    // Is unsafe request correctly sized?
    if (cbUnsafe < (sizeof(CDROM_READ) + (sizeof(SGX_BUF) * (pUnsafe->sgcount - 1)))) {
        return FALSE;
    }
    // Is unsafe request larger than safe request container?
    if (cbUnsafe > cbSafe) {
        // Deallocate safe request container, if applicable.
        if (NULL != pSafe) {
            LocalFree((HLOCAL)pSafe);
            pSafe = NULL;
        }
        // Reallocate safe request container.
        pSafe = (PCDROM_READ)LocalAlloc(LPTR, cbUnsafe);
        if (NULL == pSafe) {
            // Failed to reallocate safe request container.  Fail.
            *ppSafe = NULL;
            *lpcbSafe = 0;
            return FALSE;
        }
        // Update safe request container.
        *ppSafe = pSafe;
        *lpcbSafe = cbUnsafe;
        // Update extracted size arg.
        cbSafe = cbUnsafe;
    }
    // Safely copy unsafe request to safe request.
    if (0 == CeSafeCopyMemory((LPVOID)pSafe, (LPVOID)pUnsafe, cbUnsafe)) {
        return FALSE;
    }
    // Map unsafe embedded pointers to safe request.
    for (i = 0; i < pSafe->sgcount; i += 1) {

        if (
            (NULL == pSafe->sglist[i].sb_buf) ||
            (0 == pSafe->sglist[i].sb_len)
        ) {
            goto CleanUpLeak;
        }

        // Verify embedded pointer access and map user mode pointers

        if (FAILED(CeOpenCallerBuffer(
                    (PVOID *)&ptemp,
                    (LPVOID)pSafe->sglist[i].sb_buf,
                    pSafe->sglist[i].sb_len,
                    dwArgType,
                    FALSE)))
        {
            goto CleanUpLeak;
        }

        ASSERT(ptemp);
        saveoldptrs[i] = pSafe->sglist[i].sb_buf;
        pSafe->sglist[i].sb_buf = ptemp;
    }
    return TRUE;

CleanUpLeak:

    mappedbuffers = i;
    for (i = 0; i < mappedbuffers; i++) {

        ASSERT((NULL != pSafe->sglist[i].sb_buf) &&
               (0 == pSafe->sglist[i].sb_len));

        // Close previously mapped pointers

        if (FAILED(CeCloseCallerBuffer(
                    (LPVOID)pSafe->sglist[i].sb_buf,
                    (LPVOID)saveoldptrs[i],
                    pSafe->sglist[i].sb_len,
                    dwArgType)))
        {
            ASSERT(!"Cleanup call to CeCloseCallerBuffer failed unexpectedly");
            return FALSE;
        }
    }

    return FALSE;

}
