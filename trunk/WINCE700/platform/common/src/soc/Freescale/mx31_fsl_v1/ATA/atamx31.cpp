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
// Copyright (C) 2006,2007 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  atamx31.cpp
//
//  Base ATA MX31 device abstraction.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <devload.h>
#include <pm.h>
#include <nkintr.h>
#include <ceddk.h>
#include <diskio.h>
#include <atapi2.h>
#include <atamx31.h>
#include <ata_time.h>

//------------------------------------------------------------------------------
// Global Variables

LONG CMX31Disk::m_lDeviceCount = 0;

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
CreateMX31HD(
    HKEY hDevKey
    )
{
    return new CMX31Disk(hDevKey);
}

// ----------------------------------------------------------------------------
// Function: CMX31Disk
//     Constructor
//
// Parameters:
//     hKey -
// ----------------------------------------------------------------------------

CMX31Disk::CMX31Disk(
    HKEY hKey
    ) : CDisk(hKey)
{
    InterlockedIncrement(&m_lDeviceCount);

    g_pVAtaReg = NULL;
    fSDMAIntrEnable = FALSE;

    DEBUGMSG(ZONE_INIT|ZONE_MX31, (_T(
        "Ata!CMX31Disk::CMX31Disk> device count(%d)\r\n"
        ), m_lDeviceCount));
}

// ----------------------------------------------------------------------------
// Function: ~CMX31Disk
//     Destructor
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CMX31Disk::~CMX31Disk(
    )
{
    DeInitDMA();

    InterlockedDecrement(&m_lDeviceCount);

    DEBUGMSG(ZONE_INIT|ZONE_MX31, (_T(
        "Ata!CMX31Disk::~CMX31Disk> device count(%d)\r\n"
        ), m_lDeviceCount));
}

// ----------------------------------------------------------------------------
// Function: CopyDiskInfoFromPort
//     This function is not used
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void
CMX31Disk::CopyDiskInfoFromPort(
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
CMX31Disk::WaitForInterrupt(
    DWORD dwTimeOut
    )
{
    BYTE bStatus;
    BOOL fRet = TRUE;
    DWORD dwRet;
    DWORD dwWaitcount = 0;

    DEBUGMSG(ZONE_FUNC, (TEXT("CMX31Disk: WaitForInterrupt+\r\n")));

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

    while(fRet && dwTimeOut && fSDMAIntrEnable && !(bIntrPending & 0x80) && (dwWaitcount<3000))
    {
        Sleep(1);
        bIntrPending = INREG8(&g_pVAtaReg->InterruptPending);
        dwWaitcount++;
        if(dwWaitcount>1) DEBUGMSG(ZONE_FUNC, (_T("WARNING: DISK is not ready! dwWaitcount %d\r\n"), dwWaitcount));
    }

    if(bIntrPending & 0x08)
    {
        // read status; acknowledge interrupt
        bStatus = GetBaseStatus();
        if (bStatus & ATA_STATUS_ERROR) {
            //All destructive commands will return 51H in the Status register and 04H in the Error register
            if(bStatus==0x51)
            {
                bStatus = GetError();
                if(bStatus==0x04)
                    RETAILMSG(1, (TEXT("WARNING: NANDrive write protected!!!\r\n")));
            }
            else
                bStatus = GetError();

            fRet = FALSE;
            DEBUGMSG(ZONE_FUNC, (TEXT("CMX31Disk: WaitForInterrupt(Error: %x)\r\n"), bStatus));
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
    InterruptDone(m_pPort->m_dwSysIntr);

    if(fRet && dwTimeOut && fSDMAIntrEnable && !(bIntrPending & 0x80))
    {
        //fRet = FALSE;
        goto next_wait;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CMX31Disk: WaitForInterrupt-:%d,%x\r\n"), fRet, bIntrPending));
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
CMX31Disk::EnableInterrupt(
    )
{
    GetBaseStatus(); // acknowledge interrupt, if pending

    OUTREG8(&g_pVAtaReg->InterruptClear, 0xff);
    OUTREG8(&g_pVAtaReg->InterruptEnable, 0x68); //0x08);
    // signal interrupt done
    InterruptDone(m_pPort->m_dwSysIntr);
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
CMX31Disk::ConfigureRegisterBlock(
    DWORD dwStride
    )
{
    DEBUGMSG(ZONE_FUNC, (TEXT("CMX31Disk: ConfigureRegisterBlock+\r\n")));
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
    DEBUGMSG(ZONE_FUNC, (TEXT("CMX31Disk: ConfigureRegisterBlock-\r\n")));
}

// ----------------------------------------------------------------------------
// Function: Init
//     Initialize channel
//
// Parameters:
//     hActiveKey -
// ----------------------------------------------------------------------------

BOOL
CMX31Disk::Init(
    HKEY hActiveKey
    )
{
    BOOL bRet = FALSE;

    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk: Init+")));
    m_f16Bit = TRUE; // ATA is 16-bit

    // configure port
    if (!ConfigPort()) {
        DEBUGMSG(ZONE_INIT, (_T(
            "Ata!CMX31Disk::Init> Failed to configure port; device(%u)\r\n"
            ), m_dwDeviceId));
        goto exit;
    }

    // assign the appropriate folder name
    m_szDiskName = g_szATAHardDisk;

    if(!InitDMA())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("MMCSD_DMAInit: cannot init DMA!\r\n")));
        goto exit;
    }

    // finish intialization; i.e., initialize device
    bRet = CDisk::Init(hActiveKey);
    if (!bRet) {
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
                  OUTREG8(&g_pVAtaReg->ATAControl, 0x41);
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

    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk: Init-")));

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
CMX31Disk::MainIoctl(
    PIOREQ pIOReq
    )
{
    DEBUGMSG(ZONE_FUNC, (_T(
        "Ata!CMX31Disk::MainIoctl> IOCTL(%d), device(%d) \r\n"
        ), pIOReq->dwCode, m_dwDeviceId));

    return CDisk::MainIoctl(pIOReq);
}

// ----------------------------------------------------------------------------
// Function: ConfigPort
//     Initialize IST/ISR
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CMX31Disk::ConfigPort(
    )
{
    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk: ConfigPort+")));

    m_pATAReg = (PBYTE)m_pPort->m_dwRegBase;
    m_pATARegAlt = (PBYTE)m_pPort->m_dwRegAlt;

    if(!InitializePort())
    {
        return FALSE;
    }

    // this function is called for the master and slave on this channel; if
    // this has already been called, then exit
    if (m_pPort->m_hIRQEvent) {
        m_dwDeviceFlags |= DFLAGS_DEVICE_INITIALIZED;
        return TRUE;
    }

    // create interrupt event
    if (NULL == (m_pPort->m_hIRQEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CMX31Disk::ConfigPort> Failed to create interrupt event for device(%d)\r\n"
            ), m_dwDeviceId));
        return FALSE;
    }

    // Translate IRQ to SYSINTR
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &(m_pPort->m_dwIrq),
            sizeof(m_pPort->m_dwIrq), &m_pPort->m_dwSysIntr,
            sizeof(m_pPort->m_dwSysIntr), NULL))
    {
        ERRORMSG(1,(TEXT("%s: Request SYSINTR failed\r\n"), __WFUNCTION__));
        return FALSE;
    }
    // associate interrupt event with IRQ
    if (!InterruptInitialize(
        m_pPort->m_dwSysIntr,
        m_pPort->m_hIRQEvent,
        NULL,
        0)
    ) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CMX31Disk::ConfigPort> Failed to initialize interrupt(SysIntr(%d)) for device(%d)\r\n"
            ), m_pPort->m_dwSysIntr, m_dwDeviceId));
        return FALSE;
    }

    // Create interrupt wait event
    if (NULL == (m_pPort->m_hWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CMX31Disk::ConfigPort> Failed to create interrupt wait event for device(%d)\r\n"
            ), m_dwDeviceId));
        return FALSE;
    }

    // Create IST for ATA interrupts
    m_pPort->m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) IntrServThread, m_pPort, 0, NULL);
    if (!m_pPort->m_hThread)
    {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CMX31Disk::ConfigPort> Failed to create IST for device(%d)\r\n"
            ), m_dwDeviceId));
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk: ConfigPort-")));
    return TRUE;
}

// SetTimingRegisters() programs host timing. "speed" is the ATA mode programmed,
// mode can be PIO_MODE, UDMA_MODE or MDMA_MODE, ClkSpd is the clock period in
// ns
void CMX31Disk::SetTimingRegisters(int speed, int mode, int ClkSpd)
{
    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk: SetTimingRegisters+(%d,%d,%d)+"), speed,mode,ClkSpd));
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
    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk: SetTimingRegisters-")));
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
BOOL CMX31Disk::InitializePort(void)
{
    PHYSICAL_ADDRESS phyAddr;
    BOOL bRet = FALSE;

    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk: InitializePort+")));
    if(g_pVAtaReg == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_ATA_CTRL;
        g_pVAtaReg = (PCSP_ATA_REG) MmMapIoSpace(phyAddr, sizeof(CSP_ATA_REG), FALSE);

        // Check if virtual mapping failed
        if (g_pVAtaReg == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("CMX31Disk: MmMapIoSpace failed!\r\n")));
            goto exit;
        }

        m_pATAReg = (PBYTE) (g_pVAtaReg);
        m_pATARegAlt = (PBYTE) (g_pVAtaReg);

        ConfigureRegisterBlock(4);

        BSPATAPowerEnable(TRUE);

        // BSPATAIOMUXConfig configure the available IOMUX settings
        BSPATAIOMUXConfig();

        SetTimingRegisters(0, PIO_MODE, CLOCK_PERIOD);
        SetTimingRegisters(0, MDMA_MODE, CLOCK_PERIOD);
        SetTimingRegisters(0, UDMA_MODE, CLOCK_PERIOD);

        OUTREG8(&g_pVAtaReg->ATAControl, 0x00);
        Sleep(20);
        OUTREG8(&g_pVAtaReg->ATAControl, 0x40);
        Sleep(20);
    }
    bRet = TRUE;

exit:;
    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk: InitializePort-")));
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
void CMX31Disk::ShowRegisters(UCHAR usel)
{
    if(usel & 0x01)
    {
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG0 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG0, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG0)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG1 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG1, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG1)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG2 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG2, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG2)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG3 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG3, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG3)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG4 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG4, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG4)));
        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATA_TIME_CONFIG5 %p,0x%08x\n"), &g_pVAtaReg->ATA_TIME_CONFIG5, INREG32(&g_pVAtaReg->ATA_TIME_CONFIG5)));

        DEBUGMSG(ZONE_INIT, (_T("ATAReg: ATAControl %p,0x%02x\n"), &g_pVAtaReg->ATAControl, INREG8(&g_pVAtaReg->ATAControl)));
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
BOOL CMX31Disk::SetupDMA(PSG_BUF pSgBuf, DWORD dwSgCount, BOOL fRead)
{
    UINT8 chan = fRead ? DmaChanATARx : DmaChanATATx;
    BOOL rc = TRUE;
#ifdef USE_SDMA_SG_LIST
    BOOL fAlign = TRUE;
#else
    BOOL fAlign = FALSE;
#endif
    DWORD dwLockedPages = 0;
    DWORD dwSize;

    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk::SetupDMA(%d, %d)+\n"), fRead,
                         dwSectorsToTransfer * BYTES_PER_SECTOR));

    m_pDMASgBuf = pSgBuf;
    m_dwAlignedSgCount = 0;
    m_dwAlignedDescCount = 0;
    m_dwAlignedSgBytes = 0;
    m_DMAChan = chan;

    OUTREG8(&g_pVAtaReg->ATAControl, 0x40);

    DWORD currentSg = 0;
    DWORD currentDesc = 0;
    DWORD pdwPages[ATA_MAX_DESC_COUNT];

    // For each ATA watermark aligned scatter/gather buffer.
    while((currentSg < dwSgCount) && fAlign)
    {
        LPBYTE pBuffer = (LPBYTE)pSgBuf[currentSg].sb_buf;

        if (!LockPages(pBuffer, pSgBuf[currentSg].sb_len, pdwPages,
                       fRead ? LOCKFLAG_WRITE : LOCKFLAG_READ))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("CMX31Disk::SetupDMA> LockPages ")
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

            DWORD dwPhys = pdwPages[i] <<
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

            // If we have any unaligned SG pages, perform remainder of transfer
            // using a copy into an aligned buffer.  We require the buffers to
            // be cache line aligned to prevent coherency problems that may
            // result from concurrent accesses to cache line data that is
            // "shared" between the DMA and the CPU
            if (fRead && ((dwSize & CACHE_LINE_SIZE_MASK) || (dwPhys & CACHE_LINE_SIZE_MASK)))
            {
                //DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk::SetupDMA unaligned page (sg = %d, page = %d, size = %d\r\n"), currentSg, i, dwSize));
                UnlockPages(pBuffer,pSgBuf[currentSg].sb_len);
                --dwLockedPages;

                fAlign = FALSE;
            }
            else if(!fRead && ((dwSize & 0x3) || (dwPhys & 0x3)))
            {

                UnlockPages(pBuffer,pSgBuf[currentSg].sb_len);
                --dwLockedPages;

                fAlign = FALSE;
            }
            else
            {
                // if we're at the end to enable the intr flag
                DWORD dwFlags;
                if(dwTempLen == 0 && (currentSg == (dwSgCount - 1)))
                    dwFlags = DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP;
                else
                    dwFlags = DDK_DMA_FLAGS_CONT;

                //DEBUGMSG(ZONE_FUNC, (_T("DDKSdmaSetBufDesc-SG (BD[%d], chan = %d, flags = 0x%x, size = %d\r\n"), currentDesc, chan, dwFlags, dwSize));

                // add new descriptor to chain
                DDKSdmaSetBufDesc(chan, currentDesc, dwFlags, dwPhys,
                    0, DDK_DMA_ACCESS_32BIT, (WORD)dwSize);

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

    if (!fAlign)
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

        DEBUGCHK(!(dwSize & (ATA_DMA_WATERMARK - 1)));

        //DEBUGMSG(ZONE_FUNC, (_T("DDKSdmaSetBufDesc (BD[%d], chan = %d, flags = 0x%x, size = %d\r\n"), dwAlignedDescCount, chan, DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP, dwSize));

        // Set number of bytes to transfer
        DDKSdmaSetBufDesc(chan, m_dwAlignedDescCount,
                          DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP,
                          PhysDMABufferAddr.LowPart, 0, DDK_DMA_ACCESS_32BIT,
                          (WORD)(dwSize));
    }

    DDKSdmaStartChan(chan);

    // set fifo alarm to 16 halfwords, midway
    OUTREG8(&g_pVAtaReg->FIFOAlarm, ATA_DMA_WATERMARK/2);
    OUTREG8(&g_pVAtaReg->InterruptClear, 0xff);
    OUTREG8(&g_pVAtaReg->InterruptEnable, 0x80);

    return rc;
}

// ----------------------------------------------------------------------------
// Function: BeginDMA
//     Begin DMA transfer
//
// Parameters:
//     fRead -
// ----------------------------------------------------------------------------
BOOL CMX31Disk::BeginDMA(BOOL fRead)
{
    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk::BeginDMA+\n")));

    if (m_dwAlignedSgCount > 0)
    {
        CacheSync(CACHE_SYNC_DISCARD);
    }

    fSDMAIntrEnable = TRUE;

    BYTE bTransferMode = (BYTE)m_pPort->m_pDskReg[m_dwDeviceId]->dwTransferMode;

    switch(bTransferMode)
    {
        case 0x20:  // MDMA_MODE:
        case 0x21:
        case 0x22:
             OUTREG8(&g_pVAtaReg->ATAControl, fRead ? 0xd8 : 0xea);
             break;
        case 0x40:  // UDMA_MODE:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
             OUTREG8(&g_pVAtaReg->ATAControl, fRead ? 0xdc : 0xee);
             break;
     }
     DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk::BeginDMA-\n")));
   return TRUE;
}

// ----------------------------------------------------------------------------
// Function: EndDMA
//     End DMA transfer
//
// Parameters:
//     None
// ----------------------------------------------------------------------------
BOOL CMX31Disk::EndDMA()
{

    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk::EndDMA+\n")));

    DDKSdmaStopChan(m_DMAChan, TRUE);

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
CMX31Disk::AbortDMA(
    )
{
    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk::AbortDMA+\n")));
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
DWORD CMX31Disk::MoveDMABuffer(PSG_BUF pSgBuf, DWORD dwSgIndex, DWORD dwSgCount, BOOL fRead)
{
    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk::MoveDMABuffer(%d)+\n"), dwSgCount));

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
BOOL CMX31Disk::CompleteDMA(PSG_BUF pSgBuf, DWORD dwSgCount, BOOL fRead)
{
    DEBUGMSG(ZONE_FUNC, (_T("CMX31Disk::CompleteDMA+\n")));
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
BOOL CMX31Disk::MapDMABuffers(void)
{
   DMA_ADAPTER_OBJECT Adapter;
     DEBUGMSG(ZONE_FUNC,(_T("CMX31Disk::MapDMABuffers+\r\n")));

   pVirtDMABufferAddr = NULL;

   memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
   Adapter.InterfaceType = Internal;
   Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

   // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
   pVirtDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (ATA_SDMA_BUFFER_SIZE)
                                , &(PhysDMABufferAddr), FALSE);

   if (pVirtDMABufferAddr == NULL)
   {
            DEBUGMSG(ZONE_ERROR, (TEXT("ATA_MX31:MapDMABuffers() - Failed to allocate DMA buffer.\r\n")));
      return(FALSE);
   }

   DEBUGMSG(ZONE_FUNC,(_T("CMX31Disk::MapDMABuffers-\r\n")));
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
BOOL CMX31Disk::UnmapDMABuffers(void)
{
    PHYSICAL_ADDRESS phyAddr;
    DEBUGMSG(ZONE_FUNC,(_T("CMX31Disk::UnmapDMABuffers+\r\n")));

    if(pVirtDMABufferAddr)
    {
        // Logical address parameter is ignored
        phyAddr.QuadPart = 0;
        HalFreeCommonBuffer(NULL, 0, phyAddr, (PVOID)pVirtDMABufferAddr, FALSE);
    }

    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: DeinitChannelDMA
//
//  This function deinitializes the DMA channel for output.
//
// Parameters:
//        pController[in] - hardware context
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CMX31Disk::DeinitChannelDMA(void)
{
   if (DmaChanATARx != 0)
   {
       DDKSdmaCloseChan(DmaChanATARx);
       DmaChanATARx = 0;
   }
   if (DmaChanATATx != 0)
   {
       DDKSdmaCloseChan(DmaChanATATx);
       DmaChanATATx = 0;
   }
   return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: InitChannelDMA
//
//  This function initializes the DMA channel for output.
//
// Parameters:
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CMX31Disk::InitChannelDMA(void)
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNC,(_T("CMX31Disk::InitChannelDMA+\r\n")));

    // Check if DMA buffer has been allocated
    if (!PhysDMABufferAddr.LowPart || !pVirtDMABufferAddr)
    {
      DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA: Invalid DMA buffer physical address.\r\n")));
      goto cleanUp;
    }

    // Open virtual DMA channels
    DmaChanATARx = DDKSdmaOpenChan(DmaReqRx, BSPATASDMAchannelprio(), NULL, m_pPort->m_dwIrq);
    DEBUGMSG(ZONE_DMA,(_T("Channel Allocated(Rx) : %d\r\n"),DmaChanATARx));
    if (!DmaChanATARx)
    {
          DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): SdmaOpenChan for input failed.\r\n")));
          goto cleanUp;
    }

    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(DmaChanATARx, ATA_MAX_DESC_COUNT))
    {
      DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaAllocChain for input failed.\r\n")));
      goto cleanUp;
    }

    // Initialize the chain and set the watermark level
    if (!DDKSdmaInitChain(DmaChanATARx, ATA_DMA_WATERMARK))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    // Open virtual DMA channels
    DmaChanATATx = DDKSdmaOpenChan(DmaReqTx, BSPATASDMAchannelprio(), NULL, m_pPort->m_dwIrq);
    DEBUGMSG(ZONE_DMA,(_T("Channel Allocated(Tx) : %d\r\n"),DmaChanATATx));
    if (!DmaChanATATx)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): SdmaOpenChan(Rx) for input failed.\r\n")));
        goto cleanUp;
    }

    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(DmaChanATATx, ATA_MAX_DESC_COUNT))
    {
         DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaAllocChain for input failed.\r\n")));
         goto cleanUp;
    }

    // Initialize the chain and set the watermark level
    if (!DDKSdmaInitChain(DmaChanATATx, ATA_DMA_WATERMARK))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
   if (!rc)
   {
      DeinitChannelDMA();
   }
   DEBUGMSG(ZONE_DMA,(_T("CMX31Disk::InitChannelDMA-\r\n")));
   return rc;
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
BOOL CMX31Disk::InitDMA(void)
{
    volatile PBYTE m_pSPBAReg;        // Base address of device control register set (MmMapped)
    #define SPBA_PRR8 (m_pSPBAReg+0x20)

    PHYSICAL_ADDRESS phyAddr;

    DEBUGMSG(ZONE_FUNC, (TEXT("CMX31Disk::InitDMA+\r\n")));

    phyAddr.QuadPart = CSP_BASE_REG_PA_SPBA;
    m_pSPBAReg = (PBYTE) MmMapIoSpace(phyAddr, 0x3c, FALSE);
    if (m_pSPBAReg == NULL)
    {
        DEBUGMSG(1, (TEXT("ATA:  MmMapIoSpace failed!\r\n")));
    }

    // Allow access of SDMA to ATA via SPBA
    SETREG32(SPBA_PRR8, 0x07);  // 0x07 for write

    DmaReqTx = DDK_DMA_REQ_ATA_TX ;
    DmaReqRx = DDK_DMA_REQ_ATA_RX ;

    // Map the DMA buffers into driver's virtual address space
    if(!MapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ATA_MX31:InitDMA() - Failed to map DMA buffers.\r\n")));
        return FALSE;
    }

    // Initialize the output DMA
    if (!InitChannelDMA())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ATA_MX31:InitDMA() - Failed to initialize output DMA.\r\n")));
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CMX31Disk::InitDMA-\r\n")));
    return TRUE ;
}

BOOL CMX31Disk::DeInitDMA(void)
{
    if(!DeinitChannelDMA())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ATA_MX31:DeinitChannelDMA() - Failed to deinitialize DMA.\r\n")));
        return FALSE;
    }
    if(!UnmapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ATA_MX31:UnmapDMABuffers() - Failed to Unmap DMA buffers\r\n")));
        return FALSE;
    }

    return TRUE ;
}

#endif

