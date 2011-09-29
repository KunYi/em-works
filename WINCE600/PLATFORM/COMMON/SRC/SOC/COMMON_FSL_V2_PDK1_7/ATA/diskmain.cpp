//-----------------------------------------------------------------------------
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
// Copyright (C) 2006-2009 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Module Name: diskmain.cpp
//
//    Base ATA/ATAPI device abstraction.
//
//------------------------------------------------------------------------------

#include <atamain.h>
#include <ata.h>


//------------------------------------------------------------------------------
//
//  Module Name: diskmain.cpp
//
//    Base ATA/ATAPI device abstraction.
//
//------------------------------------------------------------------------------

static HANDLE g_hTestUnitReadyThread = NULL;

// ----------------------------------------------------------------------------
// Function: CDisk
//     Constructor
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CDisk::CDisk(
    )
{
    // empty
}

// ----------------------------------------------------------------------------
// Function: CDisk
//     Constructor
//
// Parameters:
//     hKey -
// ----------------------------------------------------------------------------

CDisk::CDisk(
    HKEY hKey
    )
{
    m_dwDeviceFlags = 0;
    m_pNextDisk = NULL;
    m_pATAReg = NULL;
    m_pATARegAlt = NULL;
    m_dwDevice = 0;
    m_hDevKey = hKey;
    m_dwDeviceId = 0;
    m_dwPort = 0;
    m_f16Bit = FALSE;
    m_fUseLBA48 = FALSE;
    m_fAtapiDevice = FALSE;
    m_fInterruptSupported = FALSE;
    m_szDiskName = NULL;
    m_fDMAActive = FALSE;
    m_dwOpenCount = 0;
    m_dwUnitReadyTime = 0;
    m_dwStateFlag = 0;
    m_dwLastCheckTime = 0;
    m_dwStride = 1;
    m_pDiskPower = NULL;
    m_rgbDoubleBuffer = NULL;
    m_wNextByte = 0xFFFF;

    m_pPort = NULL;

    // init generic structures
    InitializeCriticalSection(&m_csDisk);
    memset(&m_Id, 0, sizeof(IDENTIFY_DATA));
    memset(&m_DiskInfo, 0, sizeof(DISK_INFO));
    memset(&m_InqData, 0, sizeof(INQUIRY_DATA));
}

// ----------------------------------------------------------------------------
// Function: ~CDisk
//     Destructor
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CDisk::~CDisk(
    )
{
    if (m_hDevKey) {
        RegCloseKey(m_hDevKey);
    }

    if(m_pDiskPower != NULL) {
        delete m_pDiskPower;
    }

    DeleteCriticalSection(&m_csDisk);

    // deallocate double buffer, if present
    if (NULL != m_rgbDoubleBuffer) {
        LocalFree((HLOCAL)m_rgbDoubleBuffer);
    }

    // deallocate sterile I/O request, if present
    if (NULL != m_pSterileIoRequest) {
        LocalFree((HLOCAL)m_pSterileIoRequest);
    }
}

// ----------------------------------------------------------------------------
// Function: StallExecution
//     Stall execution for the specified period of time
//
// Parameters:
//     dwTime -
// ----------------------------------------------------------------------------

void
CDisk::StallExecution(
    DWORD dwTime
    )
{
    if ((dwTime >= 100) && (m_dwDeviceFlags & DFLAGS_DEVICE_CDROM)) {
        Sleep (dwTime / 100);
    }
    else {
        ::StallExecution(dwTime * 10);
    }
}

#define HELPER_

// These functions should be inlined or converted to macros
void CDisk::TakeCS()                    { EnterCriticalSection(&m_csDisk); }
void CDisk::ReleaseCS()                 { LeaveCriticalSection(&m_csDisk); }
void CDisk::Open()                      { InterlockedIncrement((LONG *)&m_dwOpenCount); }
void CDisk::Close()                     { InterlockedDecrement((LONG *)&m_dwOpenCount); }
BOOL CDisk::IsAtapiDevice()             { return m_fAtapiDevice; }
BOOL CDisk::IsCDRomDevice()             { return (((m_Id.GeneralConfiguration >> 8) & 0x1f) == ATA_IDDEVICE_CDROM); }
BOOL CDisk::IsDVDROMDevice()            { return TRUE; }
BOOL CDisk::IsDiskDevice()              { return (((m_Id.GeneralConfiguration >> 8) & 0x1f) == ATA_IDDEVICE_DISK); }
BOOL CDisk::IsRemoveableDevice()        { return (m_Id.GeneralConfiguration & IDE_IDDATA_REMOVABLE); }
BOOL CDisk::IsDMASupported()            { return ((m_Id.Capabilities & IDENTIFY_CAPABILITIES_DMA_SUPPORTED) && m_fDMAActive); }
BOOL CDisk::IsDRQTypeIRQ()              { return ((m_Id.GeneralConfiguration >> 5) & 0x0003) == ATA_DRQTYPE_INTRQ; }
WORD CDisk::GetPacketSize()             { return m_Id.GeneralConfiguration & 0x0003 ? 16 : 12; }
BOOL CDisk::IsValidCommandSupportInfo() { return ((m_Id.CommandSetSupported2 & (1 << 14)) && !(m_Id.CommandSetSupported2 & (1 << 15))); }
BOOL CDisk::IsWriteCacheSupported()     { return ((m_Id.CommandSetSupported1 & COMMAND_SET_WRITE_CACHE_SUPPORTED) && IsValidCommandSupportInfo()); }
BOOL CDisk::IsPMSupported()             { return (m_Id.CommandSetSupported1 & COMMAND_SET_POWER_MANAGEMENT_SUPPORTED && IsValidCommandSupportInfo()); }
BOOL CDisk::IsPMEnabled()               { return (IsPMSupported() && (m_Id.CommandSetFeatureEnabled1 & COMMAND_SET_POWER_MANAGEMENT_ENABLED)); }

// These functions are called (1x) in atamain and should be inlined
void CDisk::SetActiveKey(TCHAR *szActiveKey)
{
    StringCchCopyN(m_szActiveKey,MAX_PATH,szActiveKey,MAX_PATH - 1);
    m_szActiveKey[MAX_PATH - 1] = 0;
}

void CDisk::SetDeviceKey(TCHAR *szDeviceKey)
{
    StringCchCopyN(m_szDeviceKey,MAX_PATH,szDeviceKey,MAX_PATH - 1);
    m_szDeviceKey[MAX_PATH - 1] = 0;
}

#define _HELPER

// ----------------------------------------------------------------------------
// Function: InitController
//     Reset the controller and determine whether a device is present on the
//     channel; if a device is present, then query and store its capabilities
//
// Parameters:
//     fForce -
// ----------------------------------------------------------------------------

BOOL
CDisk::InitController(
    BOOL fForce
    )
{
    BOOL bRet = TRUE;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(fForce);

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: InitController+\r\n")));
    // if the controller has not already been reset, then perform a soft-reset
    // to enable the channel

    if (!(m_dwDeviceFlags & DFLAGS_DEVICE_INITIALIZED)) {

        // perform a soft-reset on the controller; if we don't do this, then
        // we won't be able to detect whether or not devices are present on the
        // channel

        bRet = ResetController(FALSE);
        if (!bRet) {
            goto exit;
        }

        // if interrupt is supported, enable interrupt

        if (m_fInterruptSupported) {
            SelectDevice();
            WriteAltDriveController(ATA_CTRL_ENABLE_INTR);
            EnableInterrupt();
        }
    }

    // issue IDENTIFY DEVICE and/or IDENTIFY PACKET DEVICE

    bRet = Identify();
    if (!bRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CDisk::InitController> Device did not respond to identify\r\n"
            )));
        goto exit;
    }
    else {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CDisk::InitController> Device responded to identify\r\n"
            )));
        m_dwDeviceFlags |= DFLAGS_DEVICE_INITIALIZED;
    }

exit:;
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: InitController-\r\n")));
    return bRet;
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
CDisk::ConfigureRegisterBlock(
    DWORD dwStride
    )
{
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: ConfigureRegisterBlock+\r\n")));
    m_dwStride = dwStride;
    m_dwDataDrvCtrlOffset = ATA_REG_DATA * dwStride;
    m_dwFeatureErrorOffset = ATA_REG_FEATURE * dwStride;
    m_dwSectCntReasonOffset = ATA_REG_SECT_CNT * dwStride;
    m_dwSectNumOffset = ATA_REG_SECT_NUM * dwStride;
    m_dwDrvHeadOffset = ATA_REG_DRV_HEAD * dwStride;
    m_dwCommandStatusOffset = ATA_REG_COMMAND * dwStride;
    m_dwByteCountLowOffset = ATA_REG_BYTECOUNTLOW * dwStride;
    m_dwByteCountHighOffset = ATA_REG_BYTECOUNTHIGH * dwStride;
    m_dwAltStatusOffset = ATA_REG_ALT_STATUS_CS1 * dwStride;
    m_dwAltDrvCtrl = ATA_REG_DRV_CTRL_CS1 * dwStride;
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: ConfigureRegisterBlock-\r\n")));
}

// ----------------------------------------------------------------------------
// Function: Init
//     This function is called by the IDE/ATA controller enumerator to trigger
//     the initialization of a device
//
// Parameters:
//     hActiveKey -
// ----------------------------------------------------------------------------

BOOL
CDisk::Init(
    HKEY hActiveKey
    )
{
    BOOL fRet = FALSE;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hActiveKey);

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: Init+\r\n")));
    m_dwWaitCheckIter = 0x200;
    m_dwWaitSampleTimes = 0x20;
    m_dwWaitStallTime = 0x10;

    m_dwDiskIoTimeOut = DEFAULT_DISK_IO_TIME_OUT;

    // replicate CDisk::ReadSettings

    m_dwUnitReadyTime = DEFAULT_MEDIA_CHECK_TIME;

    //m_fDMAActive = FALSE;
    m_dwDMAAlign = 0;

    // m_dwDeviceFlags |= DFLAGS_DEVICE_ISDVD; this is ignored

    if (m_pPort->m_pDskReg[m_dwDeviceId]->dwInterruptDriven) {
        m_fInterruptSupported = TRUE;
    }

    // initialize controller

    if (!InitController(TRUE)) {
        DEBUGMSG(ZONE_INIT, (_T(
            "Ata!CDisk::Init> Failed to initialize device\r\n"
            )));
        goto exit;
    }

    // set write cache mode, if write cache mode supported

    if (m_Id.CommandSetSupported1 & 0x20) {
        if (SetWriteCacheMode(m_pPort->m_pDskReg[m_dwDeviceId]->dwWriteCache)) {
            if (m_pPort->m_pDskReg[m_dwDeviceId]->dwWriteCache) {
                m_dwDeviceFlags |= DFLAGS_USE_WRITE_CACHE;
                DEBUGMSG(ZONE_INIT, (_T(
                    "Ata!CDisk::Init> Enabled write cache\r\n"
                    )));
            }
            else {
                m_dwDeviceFlags &= ~DFLAGS_USE_WRITE_CACHE;
                DEBUGMSG(ZONE_INIT, (_T(
                    "Ata!CDisk::Init> Disabled on device write cache\r\n"
                    )));
            }
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Failed to set write cache mode\r\n"
                )));
        }
    }

    // set read look-ahead, if read look-ahead supported

    if ((m_Id.CommandSetSupported1 & 0x40) && m_pPort->m_pDskReg[m_dwDeviceId]->dwLookAhead) {
        if (SetLookAhead()) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Enabled read look-ahead\r\n"
                )));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Failed to enable read look-ahead\r\n"
                )));
        }
    }

    // set transfer mode, if a specific transfer mode was specified in the
    // device's instance key

    BYTE bTransferMode = (BYTE)m_pPort->m_pDskReg[m_dwDeviceId]->dwTransferMode;
    if (0xFF != bTransferMode) {
        if (0x00 == bTransferMode) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Selecting PIO default mode(0x%x)\r\n"
                ), bTransferMode));
        }
        else if (0x01 == bTransferMode) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Selecting PIO default mode(0x%x); disabled IORDY\r\n"
                ), bTransferMode));
        }
        else if ((bTransferMode & 0xF8) == 0x08) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Selecting PIO flow control mode %d (0x%x)\r\n"
                ), (bTransferMode & 0x08), bTransferMode));
        }
        else if ((bTransferMode & 0xF0) == 0x20) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Selecting Multi-word DMA mode %d (0x%x)\r\n"
                ), bTransferMode, bTransferMode));
        }
        else if ((bTransferMode & 0xF0) == 0x40) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Selecting Ultra DMA mode %d (0x%x)\r\n"
                ), bTransferMode, bTransferMode));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Unknown transfer mode(0x%x)\r\n"
                ), bTransferMode));
        }
        // bTransferMode is a valid transfer mode
        if (!SetTransferMode(bTransferMode)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Failed to set transfer mode(0x%x)\r\n"
                ), bTransferMode));
        }
    }

    fRet = TRUE;

exit:;
    m_dwDevice=0;
    m_f16Bit=TRUE;
//  SetTransferMode(ATA_PIO_DEFAULT_MODE);
/* Jerry Changes for storage manager issue, 2007/01/26 */
/* Why set fRet to TRUE here? In case failed to initialize device */
//    fRet = TRUE;
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: Init-\r\n")));
    return fRet;
}

// ----------------------------------------------------------------------------
// Function: ResetController
//     Implement ATA/ATAPI-6 R3B 9.2 (Software reset protocol)
//
// Parameters:
//     bSoftReset -
// ----------------------------------------------------------------------------

BOOL
CDisk::ResetController(
    BOOL bSoftReset // ignore
    )
{
    DWORD dwAttempts = 0;
    BYTE bStatus = 0;
    BOOL fRet = FALSE;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(bSoftReset);

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: ResetController+\r\n")));
    // we have to negate the RESET signal for 5 microseconds before we assert it

    WriteAltDriveController(0x00);
    ::StallExecution(25);

    // Set_SRST
    // --------
    // to enter Set_SRST state, set SRST in the Device Control register to one;
    // this will assert the RESET signal and reset both devices on the current
    // channel

    WriteAltDriveController(0x04); // 0x04 == SRST

    // remain in this state for at least 5 microseconds; i.e., assert RESET signal
    // for at least 5 microseconds
    // if this is a hardware reset, then assert RESET signal for at least 25
    // microseconds

    ::StallExecution(25); // this should be CEDDK implementation

    // Clear_wait
    // ----------
    // clear SRST in the Device Control register, i.e., negate RESET signal

    WriteAltDriveController(0x00);

    // remain in this state for at least 2 milliseconds

    Sleep(5);

HSR2_Check_status:;

    // Check_status
    // ------------
    // read the Status or Alternate Status register
    // if BSY is set to one, then re-enter this state
    // if BSY is cleared to zero, check the ending status in the Error register
    // and the signature (9.12) and transition to Host_Idle
    // Robin: Change wait time to 10s to ensure hardware ready
    bStatus = GetAltStatus(); // read Status register
    if (bStatus & 0x80) {
        // BSY is set to one, re-enter this state
        DEBUGMSG(ZONE_INIT, (TEXT(
            "Ata!ResetController> Device is busy; %u seconds remaining\r\n"
            ), (10-dwAttempts)));
        Sleep(1000);
        dwAttempts += 1;
        // a device has at most 31 seconds to complete a software reset; we'll use 3 seconds
        if (dwAttempts == 10) {
            DEBUGMSG(ZONE_INIT, (TEXT("Ata!CDisk::ResetController> Timeout\r\n")));
            goto exit;
        }
        goto HSR2_Check_status;
    }
    DEBUGMSG(ZONE_INIT, (TEXT(
        "Ata!CDisk::ResetController> Device is ready\r\n"
        )));

    // BSY is cleared to zero, check the ending status in the Error register
    // and the signature
    // TODO: Check the signature (9.12)

    // if ERR bit set to one, then the reset failed
    bStatus = GetAltStatus(); // read Status register
    if (bStatus & 0x01) {
        // ERR is set to one
        // the bits in the Error register are valid, but the Error register
        // doesn't provide any useful information in the case of SRST failing
        DEBUGMSG(ZONE_INIT, (TEXT(
            "Ata!CDisk::ResetController> SRST failed\r\n"
            )));
        // TODO: Recover from error
        goto exit;
    }

    fRet = TRUE;

exit:;
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: ResetController-\r\n")));
    return fRet;
}

// ----------------------------------------------------------------------------
// Function: AtapiSoftReset
//     Issue ATAPI SOFT RESET command
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void
CDisk::AtapiSoftReset(
    )
{
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: AtapiSoftReset+\r\n")));
    WriteCommand(ATAPI_CMD_SOFT_RESET);
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 400);
    WaitForDisc(WAIT_TYPE_READY, 500);
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: AtapiSoftReset-\r\n")));
}

// ----------------------------------------------------------------------------
// Function: IsDevicePresent
//     Determine whether a device is present on the channel
//
// Parameters:
//     None
//
// Notes:
//     If a device is present on a channel, then the device's associated
//     Error register is populated with 0x1.  If a device is not present on
//     a channel, then the device's associated Error register is populated
//     with 0xa or 0xb, for master or slave, respectively.
// ----------------------------------------------------------------------------

BOOL
CDisk::IsDevicePresent(
    )
{
    BYTE bError;
    BYTE bStatus;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: IsDevicePresent+\r\n")));
    
    for(int i=0; i<5; i++)
    {
        // determine which device to select (i.e., which device this device is)
        if ((m_dwDevice == 0) || (m_dwDevice == 2)) {
            // select device 0
            ATA_WRITE_BYTE(m_pATAReg + m_dwDrvHeadOffset, ATA_HEAD_DRIVE_1);
        }
        else {
            // select device 1
            ATA_WRITE_BYTE(m_pATAReg + m_dwDrvHeadOffset, ATA_HEAD_DRIVE_2);
        }
    
        Sleep(1);
        // read Status register
        bStatus = GetAltStatus();
    
        // read Error register
        bError = GetError();
        // test Error register // Robin: not find "Error register is populated with 0x1" in ATA spec
        // if (bStatus & ATA_STATUS_READY) {
        if (bError == 0x1) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::IsDevicePresent> Device %d is present\r\n"
                ), m_dwDevice));
            return TRUE;
        }
        DEBUGMSG(ZONE_INIT, (_T(
            "Ata!CDisk::IsDevicePresent> Device %d is not present; Error register(0x%x)\r\n"
            ), m_dwDevice, bError));
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: IsDevicePresent-\r\n")));

    return FALSE;
}

// ----------------------------------------------------------------------------
// Function: SendExecuteDeviceDiagnostic
//     Implement ATA/ATAPI-6 R3B 9.10 (Device diagnostic protocol)
//
// Parameters:
//     pbDiagnosticCode - diagnostic code returned by controller in Error
//                        register as a result of issuing EXECUTE DEVICE
//                        DIAGNOSTIC (8.11)
//
//     pfIsAtapi - whether device is an ATAPI device
// ----------------------------------------------------------------------------

BOOL
CDisk::SendExecuteDeviceDiagnostic(
    PBYTE pbDiagnosticCode,
    PBOOL pfIsAtapi
    )
{
    BYTE bStatus = 0;
    DWORD dwWaitAttempts = 1200;
    BOOL fReadSignature = FALSE;

    PREFAST_DEBUGCHK(NULL != pbDiagnosticCode);
    PREFAST_DEBUGCHK(NULL != pfIsAtapi);

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SendExecuteDeviceDiagnostic+\r\n")));
    // HI4:HED0, write command

    WaitOnBusy(FALSE);
    WriteCommand(0x90); // EXECUTE DEVICE DIAGNOSTIC command code

    // HED0:Wait, wait for at least 2 milliseconds; see following Sleep(5)
    // HED2:Check_Status, wait on BSY=0

    for (;;) {
        Sleep(5);                 // wait 5 milliseconds
        bStatus = GetAltStatus(); // get status
        // test error
        if (bStatus & ATA_STATUS_ERROR) {
            // error
            DEBUGMSG(ZONE_ERROR, (TEXT(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Device failed to process command\r\n"
                ), m_dwDeviceId));
            break;
        }
        // test BSY=0
        if (!(bStatus & ATA_STATUS_BUSY)) break;
        // retry
        if (dwWaitAttempts-- == 0) {
            DEBUGMSG(ZONE_ERROR, (TEXT(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> No response; assuming channel is empty\r\n"
                ), m_dwDeviceId));
            break; // return FALSE;
        }
    }

    // inspect result of diagnosis (table 26, 8.11); select self

    SelectDevice();
    *pbDiagnosticCode = GetError();
    // PREFAST
    if ((m_dwDevice == 0) || (m_dwDevice == 2)) {
        // device 0 (master)
        if (*pbDiagnosticCode == 0x01) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Device 0 passed, Device 1 passed or not present\r\n"
                )));
            fReadSignature = TRUE; // read signature to determine if we're ATA or ATAPI
        }
        else if (*pbDiagnosticCode == 0x00 || (0x02 <= *pbDiagnosticCode && *pbDiagnosticCode <= 0x7F)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Device 0 failed, Device 1 passed or not present\r\n"
                )));
        }
        else if (*pbDiagnosticCode == 0x81) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Device 0 passed, Device 1 failed\r\n"
                )));
            fReadSignature = TRUE; // read signature to determine if we're ATA or ATAPI
        }
        else if (*pbDiagnosticCode == 0x80 || (0x82 <= *pbDiagnosticCode && *pbDiagnosticCode <= 0xFF)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Device 0 failed, Device 1 failed\r\n"
                )));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Unknown diagnostic code(0x%x)\r\n"
                ), *pbDiagnosticCode));
        }
    }
    else {
        // device 1 (slave)
        if (*pbDiagnosticCode == 0x01) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Device 1 passed\r\n"
                )));
            fReadSignature = TRUE; // read signature to determine if we're ATA or ATAPI
        }
        else if (*pbDiagnosticCode == 0x00 || (0x02 <= *pbDiagnosticCode && *pbDiagnosticCode <= 0x7F)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Device 1 failed\r\n"
                )));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Unknown diagnostic code(0x%x)\r\n"
                ), *pbDiagnosticCode));
        }
    }

    if (fReadSignature) {

        // we passed; read signature to determine if it's ATA or ATAPI

        // test for ATA
        if (
            ATA_READ_BYTE(m_pATAReg + m_dwSectNumOffset) == 0x01 &&
            ATA_READ_BYTE(m_pATAReg + m_dwByteCountLowOffset) == 0x00 &&
            ATA_READ_BYTE(m_pATAReg + m_dwByteCountHighOffset) == 0x00 // &&
            // ATA_READ_BYTE(m_pATAReg + m_dwDrvHeadOffset) == 0x00 &&
            // ATA_READ_BYTE(m_pATAReg + m_dwSectCntReasonOffset) == 0x01 &&
        ) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> ATA device\r\n"
                )));
            *pfIsAtapi = FALSE;
        }
        // test for ATAPI
        else if (
            ATA_READ_BYTE(m_pATAReg + m_dwSectNumOffset) == 0x01 &&
            ATA_READ_BYTE(m_pATAReg + m_dwByteCountLowOffset) == 0x14 &&
            ATA_READ_BYTE(m_pATAReg + m_dwByteCountHighOffset) == 0xEB // &&
            // ATA_READ_BYTE(m_pATAReg + m_dwDrvHeadOffset) == 0x00 &&
            // ATA_READ_BYTE(m_pATAReg + m_dwSectCntReasonOffset) == 0x01 &&
        ) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> ATAPI device\r\n"
                )));
            *pfIsAtapi = TRUE;
        }
        // unknown
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::SendExecuteDeviceDiagnostic> Device 0 = Unknown device type (i.e., not ATA, not ATAPI)\r\n"
                )));
        }
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SendExecuteDeviceDiagnostic-\r\n")));
    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: SendIdentifyDevice
//     Issue IDENTIFY_DEVICE or IDENTIFY_PACKET_DEVICE depending on whether
//     fIsAtapi is TRUE.  Implement PIO data-in command protocol as per
//     ATA/ATAPI-6 R3B 9.2.
//     Implement ATA/ATAPI-6 R3B 9.10 (Device diagnostic protocol)
//
// Parameters:
//     fIsAtapi - if device is ATAPI, send IDENTIFY PACKET DEVICE
//
// Notes:
//     After issuing a PIO data-in command, if BSY=0 and DRQ=0, then the device
//     failed to process the command.  However, there exist devices that
//     require additional time to return status via the Status register.  As
//     such, a delayed retry has been introduced to faciliate such devices,
//     even though their actions do not comply with the specification.
// ----------------------------------------------------------------------------

#define HPIOI1_CHECK_STATUS_RETRIES 10
BOOL
CDisk::SendIdentifyDevice(
    BOOL fIsAtapi
    )
{
    BOOL fResult = TRUE;
    DWORD dwRetries = 0;
    BYTE bStatus;               // Status register
    DWORD cbIdentifyDeviceData; // IDENTIFY DEVICE data size

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SendIdentifyDevice+\r\n")));
    // Host Idle protocol

    // select correct device
    SelectDevice();

    // HI1:Check_Status
    // ----------------
HI1_Check_Status:;
    bStatus = GetAltStatus();
    if ((bStatus & ATA_STATUS_BUSY) || (bStatus & ATA_STATUS_DATA_REQ)) { // BSY=1 or DRQ=1
        Sleep(5);
        goto HI1_Check_Status;
    }

    // HI3:Write_Parameters
    // --------------------
    // no paramters

    // HI4:Write_Command
    __try {
        WriteCommand(fIsAtapi ? ATAPI_CMD_IDENTIFY : ATA_CMD_IDENTIFY);
    }
    __except(filter()) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "Ata!CDisk::SendIdentifyDevice> Exception writing to Command register\r\n"
            )));
        fResult = FALSE;
        goto exit;
    }

    // PIO data-in command protocol

    // HPIOI1:Check_Status
    // -------------------
HPIOI1_Check_Status:;
    __try {
        bStatus = GetAltStatus();
    }
    __except (filter()) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "Ata!CDisk::SendIdentifyDevice> Exception\r\n"
            )));
        fResult = FALSE;
        goto exit;
    }
    if (!(bStatus & (ATA_STATUS_BUSY|ATA_STATUS_DATA_REQ))) { // BSY=0 and DRQ=0
        // an error occurred
        if (dwRetries < HPIOI1_CHECK_STATUS_RETRIES) {
            dwRetries++;
            Sleep(5);
            goto HPIOI1_Check_Status;
         }
         fResult = FALSE;
         goto exit;
    }
    if (bStatus & ATA_STATUS_BUSY) { // BSY=1
        goto HPIOI1_Check_Status;
    }
    if (!(bStatus & ATA_STATUS_BUSY) && (bStatus & ATA_STATUS_DATA_REQ)) { // BSY=0 and DRQ=1
        goto HPIOI2_Transfer_Data;
    }

    // HPIOI2:Transfer_Data
    // --------------------
    // (IDENTIFY [ATAPI] DEVICE only returns a single DRQ data block)
HPIOI2_Transfer_Data:;
    cbIdentifyDeviceData = sizeof(IDENTIFY_DATA);
    DEBUGCHK(cbIdentifyDeviceData <= BYTES_PER_SECTOR);
    // read result of IDENTIFY DEVICE/IDENTIFY PACKET DEVICE
    if (m_f16Bit) {
        cbIdentifyDeviceData /= 2;
        ReadWordBuffer((PWORD)&m_Id, cbIdentifyDeviceData);
    }
    else {
        ReadByteBuffer((PBYTE)&m_Id, cbIdentifyDeviceData);
    }
    // ignore extraneous data
    while (GetAltStatus() & ATA_STATUS_DATA_REQ ) {
        if (m_f16Bit) {
            ReadWord();
        }
        else {
            ReadByte();
        }
    }

    // Return to Host Idle protocol

exit:;
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SendIdentifyDevice-\r\n")));
    return fResult;
}

// ----------------------------------------------------------------------------
// Function: Identify
//     This function initiates communication with the device.  If the
//     appropriate device is detected on the channel, then instruct the device
//     to execute a diagnostic.  Issue IDENTIFY DEVICE/IDENTIFY PACKET DEVICE.
//     Validate the IDENTIFY data.  Inspect IDENTIFY data (if ATA, determine
//     which read/write commands to use, store disk geometry, etc.)
//
// Parameters:
//     None
// ----------------------------------------------------------------------------
BOOL CDisk::Identify()
{
    WORD wDevType = 0;     // command packet set implemented by device (e.g., direct-access, CD-ROM, etc.)
    BYTE bDiagnosticCode;  // SendExecuteDeviceDiagnostic argument
    BOOL fIsAtapi;         // SendExecuteDeviceDiagnostic argument

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: Identify+\r\n")));
    TakeCS();

    // test for device present

    if (!IsDevicePresent()) {
        ReleaseCS();
        return FALSE;
    }

    // issue EXECUTE DEVICE DIAGNOSTIC; determine whether device is ATA or ATAPI
    // (ignore the result of this call, as old devices fail to respond correctly)

    SendExecuteDeviceDiagnostic(&bDiagnosticCode, &fIsAtapi);

    // try ATA device
    if (SendIdentifyDevice(FALSE)) { // fIsAtapi=FALSE
        m_fAtapiDevice = FALSE;
        // IDE/ATA controller tweak for supporting a DMA-enabled ATAPI device
    if ((m_pPort->m_pDskReg[m_dwDeviceId]->dwDMA == 0x01) ||
          (m_pPort->m_pDskReg[m_dwDeviceId]->dwDMA == 0x02)){ // 0=PIO, 1/2=MDMA/UDMA
            m_fDMAActive = TRUE;
        }
    }
    else {
        // try ATAPI device
        if (SendIdentifyDevice(TRUE)) { // fIsAtapi=TRUE
            m_fAtapiDevice = TRUE;
        }
        else {
            DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
                "Ata!CDisk::Identify> Device failed to respond to IDENTIFY DEVICE and IDENTIFY PACKET DEVICE\r\n"
                )));
            ReleaseCS();
            return FALSE;
        }
    }

    ReleaseCS();

    // validate IDENTIFY DEVICE/IDENTIFY PACKET DEVICE signature; any empty
    // channel may return invalid data

    if ((m_Id.GeneralConfiguration == 0) || (m_Id.GeneralConfiguration == 0xffff) ||
        (m_Id.GeneralConfiguration == 0xff7f) ||
        (m_Id.GeneralConfiguration == 0x7fff) ||
       ((m_Id.GeneralConfiguration == m_Id.IntegrityWord) && (m_Id.NumberOfCurrentCylinders == m_Id.IntegrityWord))
    ) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Ata!CDisk::Identify> General configuration(%04X) not valid; device not present\r\n"
            ), m_Id.GeneralConfiguration));
        return FALSE;
    }

    // dump IDENTIFY DEVICE/IDENTIFY PACKET DEVICE data and supported transfer modes
#ifdef DEBUG 
    PIDENTIFY_DATA pId = &m_Id;
#endif
    DUMPIDENTIFY(pId);
    DUMPSUPPORTEDTRANSFERMODES(pId);

    // ATA/ATAPI-3 compatible devices store command packet set implemented by
    // device in bits 12-8 of word 0 of IDENTIFY DEVICE/IDENTIFY PACKET DEVICE
    // data (this information is retired in ATA/ATAPI-6)

    wDevType = (m_Id.GeneralConfiguration >> 8) & 0x1F;
    switch (wDevType) {
    case ATA_IDDEVICE_UNKNOWN:
        return FALSE;
    case ATA_IDDEVICE_CDROM:
        m_dwDeviceFlags |= DFLAGS_DEVICE_CDROM;
        break;
    case ATA_IDDEVICE_DISK:
        break;
    case ATA_IDDEVICE_OPTICAL_MEM:
        break;
    default:
        DEBUGMSG(ZONE_INIT, (_T("Ata!CDisk::Identify> Assuming direct-access device (hard disk drive)\r\n")));
        break;
    }
    // this is redundant; but various routines use this information
    m_dwDeviceFlags |= DFLAGS_DEVICE_PRESENT;
    m_dwDeviceFlags |= (IsAtapiDevice()) ? DFLAGS_ATAPI_DEVICE : 0;
    m_dwDeviceFlags |= (IsRemoveableDevice()) ? DFLAGS_REMOVABLE_DRIVE : 0;

    // ATAPI devices use ATAPI read/write commands; ATA devices support
    // single- and multi-sector transfers; if this is an ATA device, then
    // select multi-sector transfers, if supported

    if (!IsAtapiDevice()) {

        // default to single-sector transfers
        m_bReadCommand = ATA_CMD_READ; m_bWriteCommand = ATA_CMD_WRITE;

        if (m_Id.MaximumBlockTransfer != 0) {

            // device supports multi-sector transfers; enable multi-sector
            // transfers; issue SET MULTIPLE MODE command
            SelectDevice();
            WriteSectorCount((BYTE)m_Id.MaximumBlockTransfer);
            WriteCommand(ATA_CMD_SET_MULTIPLE);

            if (!WaitOnBusy(FALSE) && (GetAltStatus() & ATA_STATUS_READY)) {
                m_bReadCommand = ATA_CMD_MULTIPLE_READ; m_bWriteCommand = ATA_CMD_MULTIPLE_WRITE;
                m_bSectorsPerBlock = m_Id.MaximumBlockTransfer;
            }
            else {
                DEBUGMSG(ZONE_INIT, (_T(
                    "Ata!CDisk::Identify> (Warning) Failed to enable multi-sector transfers; using single-sector transfers\r\n"
                    )));
            }
        }
    }

    m_bDMAReadCommand = ATA_CMD_READ_DMA;
    m_bDMAWriteCommand = ATA_CMD_WRITE_DMA;

    m_fLBAMode = (m_Id.Capabilities & 0x0200) ? TRUE : FALSE;
    m_DiskInfo.di_flags = DISK_INFO_FLAG_MBR;                 // all ATA storage devices have an MBR
    m_DiskInfo.di_bytes_per_sect = BYTES_PER_SECTOR;          // start with 512, then go with SetInfo changes
    m_DiskInfo.di_cylinders = m_Id.NumberOfCylinders;
    m_DiskInfo.di_heads = m_Id.NumberOfHeads;
    m_DiskInfo.di_sectors = m_Id.SectorsPerTrack;
    if (m_fLBAMode) {
        m_DiskInfo.di_total_sectors = m_Id.TotalUserAddressableSectors;
        ConfigLBA48(); // set m_fUseLBA48 if applicable
    }
    else {
        m_DiskInfo.di_total_sectors= m_DiskInfo.di_cylinders*m_DiskInfo.di_heads*m_DiskInfo.di_sectors;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: Identify-\r\n")));
    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: ConfigLBA48
//     This is a helper function which is called after the IDENTIFY_DEVICE
//     command has been successfully executed.  It parses the results
//     of the IDENTIFY_DEVICE command to determine if 48-bit LBA is supported
//     by the device.
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void CDisk::ConfigLBA48(void)
{
    PIDENTIFY_DATA pId = (PIDENTIFY_DATA)&m_Id;

    // Word 87 (CommandSetFeatureDefault): 
    //         bit 14 is set and bit 15 is cleared if config data
    //         in word 86 (CommandSetFeatureEnabled2) is valid.
    // Note that this is only valid for non-ATAPI devices
    if ( !IsAtapiDevice() &&
         (pId->CommandSetFeatureDefault & (1 << 14)) &&
         !(pId->CommandSetFeatureDefault & (1 << 15)) &&
         (pId->CommandSetFeatureEnabled2 & (1 << 10)) )
        {
        DEBUGMSG(ZONE_INIT, (TEXT("Atapi!CDisk::ConfigLBA48> Device supports 48-bit LBA\r\n")));
        DEBUGMSG(ZONE_INIT, (TEXT("Atapi!CDisk::ConfigLBA48> Max LBA Address = 0x%08x%08x"),
                             pId->lMaxLBAAddress[1],
                             pId->lMaxLBAAddress[0]));

        m_fUseLBA48 = TRUE;

        // The CE file system currently supports a maximum of 32-bit sector addresses,
        if (pId->lMaxLBAAddress[1] == 0)
            m_DiskInfo.di_total_sectors = pId->lMaxLBAAddress[0];
        else
            m_DiskInfo.di_total_sectors = 0xFFFFFFFF; // Couldn't find MAX_DWORD

        ASSERT(pId->lMaxLBAAddress[1] <= 0x10000);

        // CDisk::Identify has determined whether or not the device supports multi-sector transfers
        // Update read/write command to use [READ|WRITE] [SECTORS|MULTIPLE] EXT
        if (m_bReadCommand == ATA_CMD_READ) 
        {
            m_bReadCommand = ATA_CMD_READ_SECTOR_EXT;
            m_bWriteCommand = ATA_CMD_WRITE_SECTOR_EXT;
        }
        else // CDisk::Identify has determined that the devce supports multi-sector transfers
        {
            m_bReadCommand = ATA_CMD_READ_MULTIPLE_EXT;
            m_bWriteCommand = ATA_CMD_WRITE_MULTIPLE_EXT;
        }
        m_bDMAReadCommand = ATA_CMD_READ_DMA_EXT;
        m_bDMAWriteCommand = ATA_CMD_WRITE_DMA_EXT;
        }
    else
    {
        m_fUseLBA48 = FALSE;
    }
}

// ----------------------------------------------------------------------------
// Function: ValidateSg
//     Map embedded pointers
//
// Parameters:
//     pSgReq -
//     InBufLen - The size of the actual buffer as specified by the user.
//                This needs to be cross checked with pSgReq->sr_num_sg to
//                prevent buffer overflows
//     saveoldptrs - Your old pointers are saved here. Must be at least
//                   pSgReq->sr_num_sg in size
// ----------------------------------------------------------------------------

BOOL
CDisk::ValidateSg(
    PSG_REQ pSgReq,
    DWORD InBufLen,
    DWORD dwArgType,
    OUT PUCHAR * saveoldptrs
    )
{
            DWORD dwIndex;
    PUCHAR ptemp;

    if (NULL == pSgReq) {
        ASSERT(pSgReq);
        return FALSE;
    }

    if (InBufLen < sizeof(SG_REQ)) {
        ASSERT(InBufLen >= sizeof(SG_REQ));
        return FALSE;
    }

    // pSgReq is a sterile copy of the caller's SG_REQ; we're supposed to map
    // the embedded sb_bufs back into the SG_REQ

    if(
        !(pSgReq->sr_num_sg >= 1) ||
        !(pSgReq->sr_num_sg <= MAX_SG_BUF) ||
        !((sizeof(SG_REQ) + sizeof(SG_BUF)*(pSgReq->sr_num_sg-1)) == InBufLen) ||
        !(pSgReq->sr_num_sec > 0))
    {
        ASSERT(pSgReq->sr_num_sg >= 1);
        ASSERT(pSgReq->sr_num_sg <= MAX_SG_BUF);
        ASSERT((sizeof(SG_REQ) + sizeof(SG_BUF)*(pSgReq->sr_num_sg-1)) == InBufLen);
        ASSERT(pSgReq->sr_num_sec > 0);
        return FALSE;
    }

    for (dwIndex = 0; dwIndex < pSgReq->sr_num_sg; dwIndex++) {
        if (
            (NULL == pSgReq->sr_sglist[dwIndex].sb_buf) ||
            (0 == pSgReq->sr_sglist[dwIndex].sb_len)
        ) {
            goto CleanUpLeak;
        }

        // Verify embedded pointer access and map user mode pointers

        if (FAILED(CeOpenCallerBuffer(
                    (PVOID *)&ptemp,
                    (LPVOID)pSgReq->sr_sglist[dwIndex].sb_buf,
                    pSgReq->sr_sglist[dwIndex].sb_len,
                    dwArgType,
                    FALSE)))
            {
            goto CleanUpLeak;
        }

        saveoldptrs[dwIndex] = pSgReq->sr_sglist[dwIndex].sb_buf;
        pSgReq->sr_sglist[dwIndex].sb_buf = ptemp;

        if (NULL == pSgReq->sr_sglist[dwIndex].sb_buf) {
            goto CleanUpLeak;
            }
        }

    return TRUE;

CleanUpLeak:

    if (FAILED(UnmapSg(
                pSgReq->sr_sglist,
                saveoldptrs,
                dwIndex,
                dwArgType)))
        {
        ASSERT(!"Cleanup call to CeCloseCallerBuffer failed unexpectedly");
            return FALSE;
        }

    return FALSE;

}


// ----------------------------------------------------------------------------
// Function: UnmapSg
//     UnMap embedded pointers, previously mapped by ValidateSg.
//     Basically, an SG Array version of CeCloseCallerBuffer
//
// Parameters:
//     sr_sglist - List of mapped SG buffers to unmap
//     saveoldptrs - List of old unmapped pointers
//     sr_sglistlen - The size of sr_sglist
//     dwArgType - ARG_O_PTR/ ARG_I_PTR
//     Return value -  HRESULT from a failed call to CeCloseCallerBuffer
//                     otherwise ERROR_SUCCESS
// ----------------------------------------------------------------------------

HRESULT
CDisk::UnmapSg(
    IN const SG_BUF * sr_sglist,
    IN const PUCHAR * saveoldptrs,
    IN DWORD sr_sglistlen,
    IN DWORD dwArgType
    )
{
    HRESULT dwError = ERROR_SUCCESS;    
    ASSERT(sr_sglistlen <= MAX_SG_BUF);
    
    for (DWORD dwIndex = 0; dwIndex < sr_sglistlen; dwIndex++) {

        HRESULT dwtemp;
        ASSERT(NULL != sr_sglist[dwIndex].sb_buf);
        ASSERT(0 != sr_sglist[dwIndex].sb_len);

        // Close pointers previously mapped in ValidateSg

        dwtemp = CeCloseCallerBuffer(
                    (LPVOID)sr_sglist[dwIndex].sb_buf,
                    (LPVOID)saveoldptrs[dwIndex],
                    sr_sglist[dwIndex].sb_len,
                    dwArgType);
        
        if (FAILED(dwtemp)) {
            ASSERT(!"Cleanup call to CeCloseCallerBuffer failed unexpectedly");
            dwError = dwtemp;
        }
    }

    return dwError;
    
}

// ----------------------------------------------------------------------------
// Function: SendDiskPowerCommand
//     Put the device into a specified power state.  The optional parameter is
//     programmed into the Sector Count register, which is used for the
//     ATA NEW CMD IDLE and ATA CMD STANDBY commands.
//
// Parameters:
//     bCmd -
//     bParam -
// ----------------------------------------------------------------------------

BOOL
CDisk::SendDiskPowerCommand(
    BYTE bCmd,
    BYTE bParam
    )
{
    BYTE bError, bStatus;
    BOOL fOk = TRUE;

    if(ZONE_CELOG) CeLogData(TRUE, CELID_ATA_POWERCOMMAND, &bCmd, sizeof(bCmd), 0, CELZONE_ALWAYSON, 0, FALSE);

    // HI:Check_Status (Host Idle); wait until BSY=0 and DRQ=0
    // read Status register
    for(;;) {
        bStatus = GetAltStatus();
        if (!(bStatus & (0x80|0x08))) break; // BSY := Bit 7, DRQ := Bit 3
        Sleep(5);
    }

    // HI:Device_Select; select device
    SelectDevice();

    // HI:Check_Status (Host Idle); wait until BSY=0 and DRQ=0
    // read Status register
    for (;;) {
        bStatus = GetAltStatus();
        if (!(bStatus & (0x80|0x08))) break; // BSY := Bit 7, DRQ := Bit 3
        Sleep(5);
    }

    // HI:Write_Parameters
    WriteSectorCount(bParam);
    // WriteAltDriveController(0x00); // disable interrupt (nIEN := Bit 1 of Device Control register)

    // HI:Write_Command
    WriteCommand(bCmd);

    // transition to non-data command protocol

    // HND:INTRQ_Wait
    // transition to HND:Check_Status
    // read Status register
    for (;;) { // BSY := Bit 7
        bStatus = GetAltStatus();
        bError = GetError();
        if (bError & 0x04) { // ABRT := Bit 2
            // command was aborted
            DEBUGMSG(ZONE_ERROR, (_T(
                "Ata!CDisk::SendDiskPowerCommand> Failed to send command 0x%x, parameter 0x%x\r\n"
                ), bCmd, bParam));
            fOk = FALSE;
            break;
        }
        if (!(bStatus & 0x80)) break; // BSY := Bit 7
        Sleep(5);
    }

    // transition to host idle protocol

    return fOk;
}

// ----------------------------------------------------------------------------
// Function: GetDiskPowerInterface
//     Return the power management object associated with this device
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CDiskPower *
CDisk::GetDiskPowerInterface(
    void
    )
{
    CDiskPower *pDiskPower = new CDiskPower;
    return pDiskPower;
}

// ----------------------------------------------------------------------------
// Function: SetDiskPowerState
//     Map a power state to an ATA power management command and issue the
//     command
//
// Parameters:
//     newDx -
// ----------------------------------------------------------------------------

BOOL
CDisk::SetDiskPowerState(
    CEDEVICE_POWER_STATE newDx
    )
{
    BYTE bCmd;

    DEBUGMSG(ZONE_FUNC,(TEXT("CDisk: SetDiskPowerState+\r\n")));

    if (ZONE_CELOG) {
        DWORD dwDx = (DWORD) newDx;
        CeLogData(TRUE, CELID_ATA_SETDEVICEPOWER, &dwDx, sizeof(dwDx), 0, CELZONE_ALWAYSON, 0, FALSE);
    }

    // on D0 go to IDLE to minimize latency during disk accesses
    if(newDx == D0 || newDx == D1) {
        bCmd = ATA_CMD_IDLE_IMMEDIATE;
    }
    else if(newDx == D2) {
        bCmd = ATA_CMD_STANDBY_IMMEDIATE;
    }
    else if(newDx == D3 || newDx == D4) {
        bCmd = ATA_CMD_SLEEP;
    }
    else {
        DEBUGMSG(ZONE_WARNING, (_T(
            "CDisk::SetDiskPowerState> Invalid power state value(%u)\r\n"
            ), newDx));
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNC,(TEXT("CDisk: SetDiskPowerState-\r\n")));
    // update the disk power state
    return SendDiskPowerCommand(bCmd);
}

// ----------------------------------------------------------------------------
// Function: WakeUp
//     Wake the device up from sleep
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CDisk::WakeUp(
    )
{
#if 0
    if (!ResetController(FALSE)) {
        return FALSE;
    }
    return SendIdentifyDevice(IsAtapiDevice());
#else
    BOOL fRet = FALSE;

    // reset controller

    if (!ResetController(FALSE)) {
        DEBUGMSG(ZONE_INIT, (_T(
            "Ata!CDisk::WakeUp> Reset Controller failed\r\n"
            )));
        goto exit;
    }

    if (!Identify()) {
        DEBUGMSG(ZONE_INIT, (_T(
            "Ata!CDisk::WakeUp> Identify failed\r\n"
            )));
        goto exit;
    }
    // set write cache mode, if write cache mode supported

    if (m_Id.CommandSetSupported1 & 0x20) {
        if (SetWriteCacheMode(m_pPort->m_pDskReg[m_dwDeviceId]->dwWriteCache)) {
            if (m_pPort->m_pDskReg[m_dwDeviceId]->dwWriteCache) {
                m_dwDeviceFlags |= DFLAGS_USE_WRITE_CACHE;
                DEBUGMSG(ZONE_INIT, (_T(
                    "Ata!CDisk::WakeUp> Enabled write cache\r\n"
                    )));
            }
            else {
                m_dwDeviceFlags &= ~DFLAGS_USE_WRITE_CACHE;
                DEBUGMSG(ZONE_INIT, (_T(
                    "Ata!CDisk::WakeUp> Disabled on device write cache\r\n"
                    )));
            }
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Failed to set write cache mode\r\n"
                )));
        }
    }

    // set read look-ahead, if read look-ahead supported

    if ((m_Id.CommandSetSupported1 & 0x40) && m_pPort->m_pDskReg[m_dwDeviceId]->dwLookAhead) {
        if (SetLookAhead()) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Enabled read look-ahead\r\n"
                )));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Failed to enable read look-ahead\r\n"
                )));
        }
    }

    // set transfer mode, if a specific transfer mode was specified in the
    // device's instance key

    BYTE bTransferMode = (BYTE)m_pPort->m_pDskReg[m_dwDeviceId]->dwTransferMode;
    if (0xFF != bTransferMode) {
        if (0x00 == bTransferMode) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Selecting PIO default mode(0x%x)\r\n"
                ), bTransferMode));
        }
        else if (0x01 == bTransferMode) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::Init> Selecting PIO default mode(0x%x); disabled IORDY\r\n"
                ), bTransferMode));
        }
        else if ((bTransferMode & 0xF8) == 0x08) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Selecting PIO flow control mode %d (0x%x)\r\n"
                ), (bTransferMode & 0x08), bTransferMode));
        }
        else if ((bTransferMode & 0xF0) == 0x20) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Selecting Multi-word DMA mode %d (0x%x)\r\n"
                ), bTransferMode, bTransferMode));
        }
        else if ((bTransferMode & 0xF0) == 0x40) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Selecting Ultra DMA mode %d (0x%x)\r\n"
                ), bTransferMode, bTransferMode));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Unknown transfer mode(0x%x)\r\n"
                ), bTransferMode));
        }
        // bTransferMode is a valid transfer mode
        if (!SetTransferMode(bTransferMode)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Ata!CDisk::WakeUp> Failed to set transfer mode(0x%x)\r\n"
                ), bTransferMode));
        }
    }

    // if interrupt is supported, enable interrupt
    
    if (m_fInterruptSupported) {
        SelectDevice();
        WriteAltDriveController(ATA_CTRL_ENABLE_INTR);
        EnableInterrupt();
    }

    fRet = TRUE;

exit:;
    return fRet;
#endif
}

// ----------------------------------------------------------------------------
// Function: MainIoctl
//     Process IOCTL_DISK_ and DISK_IOCTL_ I/O controls
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------
DWORD CDisk::MainIoctl(PIOREQ pIOReq)
{
    DWORD dwError = ERROR_SUCCESS;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl+\r\n")));
#if 0
    DEBUGMSG(ZONE_IOCTL, (TEXT(
        "Ata!CDisk::MainIoctl> IOCTL(%x), device(%x)\r\n"
        ), pIOReq->dwCode, m_dwDeviceId));
#endif
    // device is powering down; fail

    if (m_dwDeviceFlags & DFLAGS_DEVICE_PWRDN) {
        SetLastError(ERROR_DEVICE_NOT_AVAILABLE);
        return FALSE;
    }

    switch(pIOReq->dwCode) {
        case IOCTL_DISK_GETINFO:
        case DISK_IOCTL_GETINFO:
            DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl:DISK_IOCTL_GETINFO\r\n")));
            if (IsCDRomDevice()) {
                dwError = ERROR_BAD_COMMAND;
            }
            else {
                dwError = GetDiskInfo(pIOReq);
            }
            break;
        case IOCTL_DISK_DEVICE_INFO:
            DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl:IOCTL_DISK_DEVICE_INFO\r\n")));
            dwError = GetDeviceInfo(pIOReq);
            break;
        case DISK_IOCTL_GETNAME:
        case IOCTL_DISK_GETNAME:
            DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl:IOCTL_DISK_GETNAME\r\n")));
            dwError = GetDiskName(pIOReq);
            break;
        case DISK_IOCTL_SETINFO:
        case IOCTL_DISK_SETINFO:
            DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl:IOCTL_DISK_SETINFO\r\n")));
            dwError = SetDiskInfo(pIOReq);
            break;
        case DISK_IOCTL_READ:
        case IOCTL_DISK_READ:
        case DISK_IOCTL_WRITE:
        case IOCTL_DISK_WRITE:
        {
            PSG_REQ pUnsafeInBuf = (PSG_REQ)pIOReq->pInBuf;
            BOOL    fRead = (pIOReq->dwCode == DISK_IOCTL_READ || pIOReq->dwCode == IOCTL_DISK_READ);

            if (!pIOReq->dwInBufSize || !pIOReq->pInBuf) {
                dwError = ERROR_INVALID_PARAMETER;
                break;
            }
            
            if (pIOReq->dwInBufSize >(sizeof(SG_REQ) + ((MAX_SG_BUF) - 1) * sizeof(SG_BUF))) {
                dwError = ERROR_INVALID_PARAMETER;
                // size of m_pSterileIoRequest is sizeof(SG_REQ) + ((MAX_SG_BUF) - 1) * sizeof(SG_BUF))
                break;
                }

            // Copy the caller's SG_REQ.
            if (0 == CeSafeCopyMemory((LPVOID)m_pSterileIoRequest, (LPVOID)pUnsafeInBuf, pIOReq->dwInBufSize)) {
                dwError = ERROR_INVALID_PARAMETER;
                break;
                }

            if (NULL == m_pSterileIoRequest) {
                ASSERT(m_pSterileIoRequest);
                dwError = ERROR_GEN_FAILURE;
                break;
            }

            PUCHAR  savedoldptrs[MAX_SG_BUF] ;  // This will hold a copy of the user mode pointers that get overwritten
                                                // ValidateSg

            // Validate the SG_REQ request and map the caller's sb_bufs into
            // the sterile copy,
            if (FALSE == ValidateSg(m_pSterileIoRequest, pIOReq->dwInBufSize, fRead ? ARG_O_PTR : ARG_I_PTR,savedoldptrs)) {
                dwError = ERROR_INVALID_PARAMETER;
            break;
            }

            // Replace the caller's SG_REQ in the I/O request with the sterile
            // copy.
            pIOReq->pInBuf = (PBYTE)m_pSterileIoRequest;
            // Execute the I/O request.
            if (IsDMASupported()) {
                __try {
                    dwError = ReadWriteDiskDMA(pIOReq, fRead);
                }
                __except(filter()) {
                    AbortDMA();
                dwError=ERROR_INVALID_PARAMETER;
                    m_pSterileIoRequest->sr_status = ERROR_INVALID_PARAMETER;
                    goto Cleanup;
                }
            }
            else {
                // Allocate the double buffer, as required.
                if ((NULL == m_rgbDoubleBuffer) && (0 < m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize)) {
                    DEBUGMSG(ZONE_INIT, (TEXT("Atapi!CDisk::MainIoctl> Allocating double buffer (first use)\r\n")));
                    m_rgbDoubleBuffer = (PBYTE)LocalAlloc(LPTR, m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize);
                    if (NULL == m_rgbDoubleBuffer) {
                        DEBUGMSG(ZONE_ERROR, (TEXT("Atapi!CDisk::MainIoctl> Failed to allocate double buffer\r\n")));
                        dwError = ERROR_OUTOFMEMORY;
                        m_pSterileIoRequest->sr_status = ERROR_OUTOFMEMORY;
                        goto Cleanup;
                    }
                }
                // Perform the I/O.
                if (fRead) {
                    dwError = this->ReadDisk(pIOReq);
                }
                else {
                    dwError = this->WriteDisk(pIOReq);
                }
            }

Cleanup:
            if (FAILED(UnmapSg(
                        m_pSterileIoRequest->sr_sglist,
                        savedoldptrs,
                        m_pSterileIoRequest->sr_num_sg,
                        fRead ? ARG_O_PTR : ARG_I_PTR)))
            {
                ASSERT(!"Cleanup call to CeCloseCallerBuffer failed unexpectedly");
                dwError = ERROR_GEN_FAILURE;
            }
                    
            // Copy sr_status from the sterile SG_REQ to the caller's SG_REQ.
            __try {
                pUnsafeInBuf->sr_status = m_pSterileIoRequest->sr_status;
                }
            __except(filter()) {
                dwError = ERROR_INVALID_PARAMETER;
            }
            break;
        }
        case IOCTL_DISK_GET_STORAGEID:
            DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl:IOCTL_DISK_GET_STORAGEID\r\n")));
            dwError = GetStorageId(pIOReq);
            break;
        case DISK_IOCTL_FORMAT_MEDIA:
        case IOCTL_DISK_FORMAT_MEDIA:
            DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl:IOCTL_DISK_FORMAT_MEDIA\r\n")));
            dwError = ERROR_SUCCESS;
            break;
        case IOCTL_DISK_FLUSH_CACHE:
            DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl:IOCTL_DISK_FLUSH_CACHE\r\n")));
            dwError = FlushCache();
            break;

        default:
            dwError = ERROR_NOT_SUPPORTED;
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: MainIoctl-\r\n")));
    return dwError;
}

// ----------------------------------------------------------------------------
// Function: PerformIoctl
//     This is the top-most IOCTL processor and is used to trap IOCTL_POWER_
//     I/O controls to pass to the associated power management object
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

BOOL
CDisk::PerformIoctl(
    PIOREQ pIOReq
    )
{
    DWORD dwError = ERROR_SUCCESS;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: PerformIoctl+\r\n")));
#if 0
    DEBUGMSG(ZONE_IOCTL, (TEXT(
        "Ata!CDisk::PerformIoctl> IOCTL(%x), device(%x)\r\n"
        ), pIOReq->dwCode, m_dwDeviceId));
#endif
    if (pIOReq->pBytesReturned) {
        *(pIOReq->pBytesReturned) = 0;
    }

    TakeCS();
    m_pPort->TakeCS();

    if(!CSPATAEnableClock(TRUE)) // Enable the clock before IOCTL handle
    {
        DEBUGMSG(ZONE_WARNING, (TEXT(" ATA Clock cannot be enabled!\r\n")));
    }

    if (ZONE_CELOG) CeLogData(TRUE, CELID_ATA_STARTIOCTL, pIOReq, sizeof(*pIOReq), 0, CELZONE_ALWAYSON, 0, FALSE);

    __try {

        if (pIOReq->dwCode == IOCTL_POWER_CAPABILITIES) {

            DEBUGMSG(ZONE_FUNC,(TEXT("CDisk: PerformIoctl:IOCTL_POWER_CAPABILITIES\r\n")));
            // instantiate DiskPower object on first use, if necessary

            if (m_pDiskPower == NULL) {
                CDiskPower *pDiskPower = GetDiskPowerInterface();
                if (pDiskPower == NULL) {
                    DEBUGMSG(ZONE_WARNING, (_T(
                        "Ata!CDisk::PerformIoctl> Failed to create power management object\r\n"
                        )));
                }
                else if (!pDiskPower->Init(this)) {
                    DEBUGMSG(ZONE_WARNING, (_T(
                        "Ata!CDisk::PerformIoctl> Failed to initialize power management\r\n"
                        )));
                    delete pDiskPower;
                }
                else {
                    m_pDiskPower = pDiskPower;
                }
            }
        }

        if (m_pDiskPower != NULL) {

            // is this a power IOCTL? if an exception occurs, then we'll catch
            // it below
            dwError = m_pDiskPower->DiskPowerIoctl(pIOReq);
            if (dwError != ERROR_NOT_SUPPORTED) {
                goto done;
            }

            // request that the disk spin up (if it's not up already)
            if (!m_pDiskPower->RequestDevice()) {
                // the disk is powered down
                dwError = ERROR_RESOURCE_DISABLED;
                goto done;
            }
        }

        // call the driver
        dwError = MainIoctl(pIOReq);


        // indicate we're done with the disk
        if (m_pDiskPower != NULL) {
            m_pDiskPower->ReleaseDevice();
        }

done:;

    } __except(filter()) {
        dwError = ERROR_GEN_FAILURE;
    }

    if (ZONE_CELOG) CeLogData(TRUE, CELID_ATA_COMPLETEIOCTL, &dwError, sizeof(dwError), 0, CELZONE_ALWAYSON, 0, FALSE);

    if(!CSPATAEnableClock(FALSE)) // Disable the clock after IOCTL handle
    {
        DEBUGMSG(ZONE_WARNING, (TEXT(" ATA Clock cannot be disabled!\r\n")));
    }
    m_pPort->ReleaseCS();
    ReleaseCS();

    if (dwError != ERROR_SUCCESS) {
        SetLastError(dwError);
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: PerformIoctl-\r\n")));
    return (ERROR_SUCCESS == dwError);
}

// ----------------------------------------------------------------------------
// Function: PostInit
//     This function facilitates backward compatibility
//
// Parameters:
//     pPostInitBuf -
// ----------------------------------------------------------------------------

BOOL
CDisk::PostInit(
    PPOST_INIT_BUF pPostInitBuf
    )
{
    DWORD dwError = ERROR_SUCCESS;

//    DEBUGMSG(ZONEID_INIT, (TEXT("Ata!CDisk::PostInit> device(%d)\r\n"), m_dwDeviceId));

    m_hDevice = pPostInitBuf->p_hDevice;

    return (dwError == ERROR_SUCCESS);
}

// ----------------------------------------------------------------------------
// Function: GetDiskInfo
//     Implement IOCTL_DISK_GETINFO
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetDiskInfo(
    PIOREQ pIOReq
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DISK_INFO *pInfo = NULL;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: GetDiskInfo+\r\n")));
    // for B/C, this call has three forms; only pInBuf, only pOutBuf, or both
    // if both, then use pOutBuf

    if (pIOReq->pInBuf) {
        if (pIOReq->dwInBufSize != sizeof(DISK_INFO)) {
            return ERROR_INVALID_PARAMETER;
        }
        pInfo = (DISK_INFO *)pIOReq->pInBuf;
    }

    if (pIOReq->pOutBuf) {
        if (pIOReq->dwOutBufSize!= sizeof(DISK_INFO)) {
            return ERROR_INVALID_PARAMETER;
        }
        pInfo = (DISK_INFO *)pIOReq->pOutBuf;
    }

    if (!pInfo) {
        DEBUGMSG(ZONE_ERROR|ZONE_IOCTL, (_T(
            "Ata!CDisk::GetDiskInfo> bad argument; pInBuf/pOutBuf null\r\n")));
        return ERROR_INVALID_PARAMETER;
    }

    // TODO: if device is ATAPI, call AtapiGetDiskInfo

    if (ERROR_SUCCESS == dwError) {
        __try {
            memcpy(pInfo, &m_DiskInfo, sizeof(DISK_INFO));
            pInfo->di_flags |= DISK_INFO_FLAG_PAGEABLE;
            pInfo->di_flags &= ~DISK_INFO_FLAG_UNFORMATTED;
            if (pIOReq->pBytesReturned){
                *(pIOReq->pBytesReturned) = sizeof(DISK_INFO);
            }
        } __except(filter()) {
            dwError = ERROR_INVALID_PARAMETER;
        }
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: GetDiskInfo-\r\n")));
    return dwError;
}

// ----------------------------------------------------------------------------
// Function: SetDiskInfo
//     Implement IOCTL_DISK_SETINFO
//
// Parameters:
//     pSgReq -
//     InBufLen -
// ----------------------------------------------------------------------------

DWORD
CDisk::SetDiskInfo(
    PIOREQ pIOReq
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DISK_INFO *pInfo = (DISK_INFO *)pIOReq->pInBuf;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SetDiskInfo+\r\n")));
    if ((pIOReq->pInBuf == NULL) || (pIOReq->dwInBufSize != sizeof(DISK_INFO))) {
        return ERROR_INVALID_PARAMETER;
    }

    memcpy(&m_DiskInfo, pInfo, sizeof(DISK_INFO));

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SetDiskInfo-\r\n")));
    return dwError;
}

// ----------------------------------------------------------------------------
// Function: GetDeviceInfo
//     IOCTL_DISK_DEVICE_INFO
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetDeviceInfo(
    PIOREQ pIOReq
    )
{

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: GetDeviceInfo+\r\n")));
    if ((pIOReq->dwInBufSize == 0) || (pIOReq->pInBuf == NULL)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pIOReq->pBytesReturned) {
        *(pIOReq->pBytesReturned) = sizeof(STORAGEDEVICEINFO);
    }
    if (pIOReq->dwInBufSize < sizeof(STORAGEDEVICEINFO)) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
        
    ASSERT(m_storagedeviceinfo.cbSize == sizeof(STORAGEDEVICEINFO));

    PSTORAGEDEVICEINFO psdi = (PSTORAGEDEVICEINFO)pIOReq->pInBuf;

    //*psdi = m_storagedeviceinfo;
    GetDeviceInfo(psdi); //to get the Storage profile of the harddisk
        
    return ERROR_SUCCESS;

}

// ----------------------------------------------------------------------------
// Function: GetDeviceInfo
//     IOCTL_DISK_DEVICE_INFO
//     This function queries the registry and must be called only in the INIT
//     routines to prevent deadlocks, as this method will end up acquiring
//     the registry lock. Otherwise the registry lock, mapped file sections
//     global lock and block driver lock can possibly deadlock. Ideally mapped file
//     sections shouldn't have a global lock. m_szDeviceKey must be set before this
//     method is called
//
// Parameters:
//     PSTORAGEDEVICEINFO -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetDeviceInfo(
    PSTORAGEDEVICEINFO psdi
    )
{
    HKEY hKey;

    psdi->dwDeviceClass = 0;
    psdi->dwDeviceType = 0;
    psdi->dwDeviceFlags = 0;

    PTSTR szProfile = psdi->szProfile;

    StringCchCopy(szProfile,PROFILENAMESIZE,L"Default");

    if (ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, m_szDeviceKey, 0, 0, &hKey)) {
        hKey = NULL;
    }

    if (IsAtapiDevice() && IsCDRomDevice()) {

        psdi->dwDeviceClass = STORAGE_DEVICE_CLASS_MULTIMEDIA;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_REMOVABLE_MEDIA;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_ATAPI;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_PCIIDE;
        psdi->dwDeviceFlags |= STORAGE_DEVICE_FLAG_MEDIASENSE;
        psdi->dwDeviceFlags |= STORAGE_DEVICE_FLAG_READONLY;

        if (!hKey || !AtaGetRegistryString(hKey, REG_VALUE_CDPROFILE, &szProfile, sizeof(psdi->szProfile))) {
            StringCchCopy(psdi->szProfile,PROFILENAMESIZE,REG_VALUE_CDPROFILE);
        }

    }
    else {

        psdi->dwDeviceClass = STORAGE_DEVICE_CLASS_BLOCK;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_PCIIDE;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_ATA;
        psdi->dwDeviceFlags |= STORAGE_DEVICE_FLAG_READWRITE;

        if (!hKey || !AtaGetRegistryString(hKey, REG_VALUE_HDPROFILE, &szProfile, sizeof(psdi->szProfile))) {
            StringCchCopy(psdi->szProfile,PROFILENAMESIZE,REG_VALUE_HDPROFILE);
        }

    }

    psdi->cbSize = sizeof(STORAGEDEVICEINFO);
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: GetDeviceInfo-\r\n")));
    return ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------
// Function: GetDiskName
//     Implement IOCTL_DISK_GETNAME
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetDiskName(
    PIOREQ pIOReq
    )
{
    static PTCHAR szDefaultDiscDrive = (_T("External Volume"));
    PTCHAR szDiskName = NULL;
    DWORD dwSize;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: GetDiskName+\r\n")));

    if ((pIOReq->pBytesReturned == NULL) || (pIOReq->dwOutBufSize == 0) || (pIOReq->pOutBuf == NULL)) {
        return ERROR_INVALID_PARAMETER;
    }

    *(pIOReq->pBytesReturned) = 0;

    if (m_szDiskName) {
        if (wcslen(m_szDiskName)) {
            szDiskName = m_szDiskName;
        }
        else {
            return ERROR_NOT_SUPPORTED;
        }
    }
    else {
        szDiskName = szDefaultDiscDrive;
    }

    dwSize = (wcslen(szDiskName) + 1) * sizeof(TCHAR);

    if (pIOReq->dwOutBufSize < dwSize) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    StringCchCopy((PTCHAR) pIOReq->pOutBuf,pIOReq->dwOutBufSize,szDiskName);

    *(pIOReq->pBytesReturned) = dwSize;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: GetDiskName-\r\n")));
    return ERROR_SUCCESS;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL CDisk::DoRead(CSgReq* pSgReqWrapper, DWORD dwStartingSector, DWORD dwNumberOfSectors, PDWORD pdwBytesRead)
{
    DWORD dwBytesRead = 0;
    BYTE  bStatus;
    DWORD dwByteInDrqDataBlock = 0;
    BYTE  bData = 0;
    WORD  wData = 0;

    if (NULL == pSgReqWrapper) {
        ASSERT(NULL != pSgReqWrapper);
        return FALSE;
    }

    // Host_Idle
    // ---------
    // Issue the appropriate command.
    if (FALSE == SendIOCommand(dwStartingSector, dwNumberOfSectors, m_bReadCommand)) {
        DEBUGMSG(ZONE_ERROR, (_T("Atapi!CDisk::DoRead> Failed to issue read/write command\r\n")));
        return FALSE;
    }

    // INTRQ_Wait
    // ----------
    // Wait for interrupt if nIEN=0 (i.e., if interrupt enabled).
HPIOI_INTRQ_Wait:;
    if (m_fInterruptSupported) {
        if (FALSE == WaitForInterrupt(m_dwDiskIoTimeOut) || (ATA_INTR_ERROR == CheckIntrState())) {
            DEBUGMSG(ZONE_ERROR, (_T("Atapi!CDisk::DoRead> Failed to wait for interrupt (m_dwDeviceId=%d)\r\n"), m_dwDeviceId));
            return FALSE;
        }
    }

    // Check_Status
    // ------------
    // If BSY=0 and DRQ=0, transition to Host_Idle.
    // If BSY=1, re-enter this state.
    // If BSY=0 and DRQ=1, transition to Transfer_Data.
HPIOI_Check_Status:;
    // Read the Status register.
    bStatus = GetAltStatus();
    // Test for BSY=0 and DRQ=0.
    if ((!(bStatus & 0x80)) && (!(bStatus & 0x08))) {
        ASSERT(0 == dwNumberOfSectors);
        *pdwBytesRead = dwBytesRead;
        return TRUE;
    }
    // Test for BSY=1.
    if (bStatus & 0x80) {
        goto HPIOI_Check_Status;
    }
    // Test for BSY=0, DRQ=1.
    if ((!(bStatus & 0x80)) && (bStatus & 0x08)) {
        goto HPIOI_Transfer_Data_Setup;
    }
    // Illegal status.
    ASSERT(FALSE);
    return FALSE;

    // Transfer_Data
    // -------------
    // If read Data register, DRQ data block transfer not complete, re-enter
    // this state.
    // If read Data register, all data for command transferred, transition to
    // Host_Idle.
    // If read Data register, DRQ data block transferred, all data for command
    // not transferred, and nIEN=1, transition to Check_Status.
    // If read Data register, DRQ data block transferred, all data for command
    // not transferred, and nIEN=0, transition to INTRQ_Wait.
HPIOI_Transfer_Data_Setup:;
    dwByteInDrqDataBlock = 0;
HPIOI_Transfer_Data:;
    // Read Data register.
    if (m_f16Bit) {
        wData = ReadWord();
        VERIFY(TRUE == pSgReqWrapper->DoWriteWord(&wData));
        dwByteInDrqDataBlock += 2;
        dwBytesRead += 2;
    }
    else {
        bData = (BYTE)ReadByte();
        VERIFY(TRUE == pSgReqWrapper->DoWriteByte(&bData));
        dwByteInDrqDataBlock += 1;
        dwBytesRead += 1;
    }
    if (0 == (dwByteInDrqDataBlock % SECTOR_SIZE)) {
        dwNumberOfSectors -= 1;
    }
    // Is the transfer complete?
    if (0 == dwNumberOfSectors) {
        *pdwBytesRead = dwBytesRead;
        return TRUE;
        }
    // The transfer is not complete.  Has a DRQ data block been transferred?
    if (dwByteInDrqDataBlock == ((DWORD)SECTOR_SIZE * m_bSectorsPerBlock)) {
        goto HPIOI_INTRQ_Wait;
    }
    // A DRQ data block has not been transferred.  Continue transferring.
    goto HPIOI_Transfer_Data;
    }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL CDisk::DoRead(PBYTE pbBuf, DWORD dwStartingSector, DWORD dwNumberOfSectors, PDWORD pdwBytesRead)
{
    DWORD dwBytesRead = 0;
    BYTE  bStatus;
    DWORD dwByteInDrqDataBlock = 0;

    if (NULL == pbBuf) {
        ASSERT(NULL != pbBuf);
        return FALSE;
        }

        // Host_Idle
        // ---------
    // Issue the appropriate command.
    if (FALSE == SendIOCommand(dwStartingSector, dwNumberOfSectors, m_bReadCommand)) {
        DEBUGMSG(ZONE_ERROR, (_T("Atapi!CDisk::DoRead> Failed to issue read/write command\r\n")));
        return FALSE;
        }

        // INTRQ_Wait
        // ----------
    // Wait for interrupt if nIEN=0 (i.e., if interrupt enabled).
HPIOI_INTRQ_Wait:;
    if (m_fInterruptSupported) {
        if (FALSE == WaitForInterrupt(m_dwDiskIoTimeOut) || (ATA_INTR_ERROR == CheckIntrState())) {
            DEBUGMSG(ZONE_ERROR, (_T("Atapi!CDisk::DoRead> Failed to wait for interrupt (m_dwDeviceId=%d)\r\n"), m_dwDeviceId));
            return FALSE;
            }
        }

        // Check_Status
        // ------------
    // If BSY=0 and DRQ=0, transition to Host_Idle.
    // If BSY=1, re-enter this state.
    // If BSY=0 and DRQ=1, transition to Transfer_Data.
HPIOI_Check_Status:;
    // Read the Status register.
    bStatus = GetAltStatus();
    // Test for BSY=0 and DRQ=0.
    if ((!(bStatus & 0x80)) && (!(bStatus & 0x08))) {
        *pdwBytesRead = dwBytesRead;
        return TRUE;
    }
    // Test for BSY=1.
    if (bStatus & 0x80) {
        goto HPIOI_Check_Status;
    }
    // Test for BSY=0, DRQ=1.
    if ((!(bStatus & 0x80)) && (bStatus & 0x08)) {
        goto HPIOI_Transfer_Data_Setup;
    }
    // Illegal status.
    ASSERT(FALSE);
    return FALSE;

        // Transfer_Data
        // -------------
    // If read Data register, DRQ data block transfer not complete, re-enter
    // this state.
    // If read Data register, all data for command transferred, transition to
    // Host_Idle.
    // If read Data register, DRQ data block transferred, all data for command
    // not transferred, and nIEN=1, transition to Check_Status.
    // If read Data register, DRQ data block transferred, all data for command
    // not transferred, and nIEN=0, transition to INTRQ_Wait.
HPIOI_Transfer_Data_Setup:;
    dwByteInDrqDataBlock = 0;
HPIOI_Transfer_Data:;
    // Read Data register.
    if (m_f16Bit) {
        *((PWORD)pbBuf) = ReadWord();
        pbBuf += 2;
        dwByteInDrqDataBlock += 2;
        dwBytesRead += 2;
    }
    else {
        *((PBYTE)pbBuf) = (BYTE)ReadByte();
        pbBuf += 1;
        dwByteInDrqDataBlock += 1;
        dwBytesRead += 1;
    }
    if (0 == (dwByteInDrqDataBlock % SECTOR_SIZE)) {
        dwNumberOfSectors -= 1;
        }
    // Is the transfer complete?
    if (0 == dwNumberOfSectors) {
        *pdwBytesRead = dwBytesRead;
        return TRUE;
        }
    // The transfer is not complete.  Has a DRQ data block been transferred?
    if (dwByteInDrqDataBlock == ((DWORD)SECTOR_SIZE * m_bSectorsPerBlock)) {
            goto HPIOI_INTRQ_Wait;
        }
    // A DRQ data block has not been transferred.  Continue transferring.
        goto HPIOI_Transfer_Data;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
DWORD CDisk::ReadDisk(PIOREQ pIoReq)
{
    DWORD   m_dwDblBufSectorCount = 0;
    PSG_REQ pSgReq;
    CSgReq  SgReqWrapper;
    DWORD   dwStartingSector;
    DWORD   dwSectorsRemaining;
    DWORD   dwNumberOfSectors;
    DWORD   dwAbsoluteBufferLength;
    DWORD   dwBytesRead = 0;
    DWORD   dwBytesWritten = 0;

    DEBUGCHK(NULL != pIoReq);

    // Unpack the Scatter/Gather request.
    pSgReq = (PSG_REQ)pIoReq->pInBuf;

    // Ensure the Scatter/Gather request does not extend past the end of the
    // disk.
    if ((pSgReq->sr_start + pSgReq->sr_num_sec) > m_DiskInfo.di_total_sectors) {
        return ERROR_SECTOR_NOT_FOUND;
    }

    // Attach the Scatter/Gather request to the wrapper.
    if (FALSE == SgReqWrapper.DoAttach(pSgReq, SECTOR_SIZE)) {
        return ERROR_GEN_FAILURE;
    }

    // Prepare the transfer.
    dwStartingSector = SgReqWrapper.GetStartingSector();
    dwSectorsRemaining = SgReqWrapper.GetNumberOfSectors();
    dwNumberOfSectors = 0;
    dwAbsoluteBufferLength = SgReqWrapper.GetAbsoluteBufferLength();

    // Is double buffering required?
    if ((SgReqWrapper.GetNumberOfBuffers() > 1) && (NULL != m_rgbDoubleBuffer)) {
        m_dwDblBufSectorCount = (m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize / SECTOR_SIZE); // dwDrqDataBlockSize
        // Fill the double buffer from the disk and copy to the Scatter/Gather
        // buffer list.
        while (SgReqWrapper.GetAbsoluteBufferPosition() < dwAbsoluteBufferLength) {
            // Fill the double buffer from the disk.
            dwNumberOfSectors = MIN(m_dwDblBufSectorCount, dwSectorsRemaining);
            if (FALSE == this->DoRead(m_rgbDoubleBuffer, dwStartingSector, dwNumberOfSectors, &dwBytesRead)) {
                return ERROR_GEN_FAILURE;
            }
            ASSERT(dwBytesRead == (SECTOR_SIZE * dwNumberOfSectors));
            // Write to the Scatter/Gather buffer list.
            if (FALSE == SgReqWrapper.DoWriteMultiple(m_rgbDoubleBuffer, dwBytesRead, &dwBytesWritten)) {
                return ERROR_GEN_FAILURE;
            }
            ASSERT(dwBytesWritten == dwBytesRead);
            dwStartingSector += dwNumberOfSectors;
            dwSectorsRemaining -= dwNumberOfSectors;
        }
    }
    else {
        // Fill the Scatter/Gather buffer list from the disk.
        while (SgReqWrapper.GetAbsoluteBufferPosition() < dwAbsoluteBufferLength) {
            dwNumberOfSectors = MIN(256, dwSectorsRemaining);
            if (FALSE == this->DoRead(&SgReqWrapper, dwStartingSector, dwNumberOfSectors, &dwBytesRead)) {
                return ERROR_GEN_FAILURE;
            }
            ASSERT(dwBytesRead == (SECTOR_SIZE * dwNumberOfSectors));
            dwStartingSector += dwNumberOfSectors;
            dwSectorsRemaining -= dwNumberOfSectors;
        }
        }

    *pIoReq->pBytesReturned = dwBytesRead;
    return ERROR_SUCCESS;
            }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL CDisk::DoWrite(CSgReq* pSgReqWrapper, DWORD dwStartingSector, DWORD dwNumberOfSectors, PDWORD pdwBytesWritten)
{
    DWORD dwBytesWritten = 0;
    BYTE  bStatus;
    DWORD dwByteInDrqDataBlock = 0;
    BOOL  fComplete = FALSE;
    BYTE  bData = 0;
    WORD  wData = 0;

    if (NULL == pSgReqWrapper) {
        ASSERT(NULL != pSgReqWrapper);
        return FALSE;
        }

        // Host_Idle
        // ---------
    // Issue the appropriate command.
    if (FALSE == SendIOCommand(dwStartingSector, dwNumberOfSectors, m_bWriteCommand)) {
        DEBUGMSG(ZONE_ERROR, (_T("Atapi!CDisk::DoWrite> Failed to issue read/write command\r\n")));
        return FALSE;
    }

    // Check_Status
    // ------------
    // If BSY=0 and DRQ=0, transition to Host_Idle.
    // If BSY=1, re-enter this state.
    // If BSY=0 and DRQ=1, transition to Transfer_Data.
HPIOO_Check_Status:;
    // Read the Status register.
    bStatus = GetAltStatus();
    // Test for BSY=0 and DRQ=0.
    if ((!(bStatus & 0x80)) && (!(bStatus & 0x08))) {
        if (fComplete) {
            return TRUE;
        }
    }
    // Test for BSY=1.
    if (bStatus & 0x80) {
        goto HPIOO_Check_Status;
    }
    // Test for BSY=0 and DRQ=1.
    if ((!(bStatus & 0x80)) && (bStatus & 0x08)) {
        goto HPIOO_Transfer_Data_Reset_DRQ_Data_Block;
    }
    // Illegal status.
    ASSERT(FALSE);
    return FALSE;

    // Transfer_Data
    // -------------
    // If write Data register, DRQ data block transfer not complete, re-enter
    // this state.
    // If write Data register, all data for command transferred, transition to
    // Host_Idle.
    // If write Data register, DRQ data block transferred, all data for command
    // not transferred, and nIEN=1, transition to Check_Status.
    // If write Data register, DRQ data block transferred, all data for command
    // not transferred, and nIEN=0, transition to INTRQ_Wait.
HPIOO_Transfer_Data_Reset_DRQ_Data_Block:;
    dwByteInDrqDataBlock = 0;
HPIOO_Transfer_Data:;
    // Write to the Data register.
    if (m_f16Bit) {
        VERIFY(TRUE == pSgReqWrapper->DoReadWord(&wData));
        WriteWord(wData);
        dwByteInDrqDataBlock += 2;
        dwBytesWritten += 2;
    }
    else {
        VERIFY(TRUE == pSgReqWrapper->DoReadByte(&bData));
        WriteByte(bData);
        dwByteInDrqDataBlock += 1;
        dwBytesWritten += 1;
    }
    if (0 == (dwByteInDrqDataBlock % SECTOR_SIZE)) {
        dwNumberOfSectors -= 1;
    }
    // Is the transfer complete?
    if (0 == dwNumberOfSectors) {
        *pdwBytesWritten = dwBytesWritten;
        fComplete = TRUE;
        goto HPIOO_INTRQ_Wait;
        }
    // The transfer is not complete.  Has a DRQ data block been transferred?
    if (dwByteInDrqDataBlock == ((DWORD)SECTOR_SIZE * m_bSectorsPerBlock)) {
        goto HPIOO_INTRQ_Wait;
        }
    // A DRQ data block has not been transferred.  Continue transferring.
    goto HPIOO_Transfer_Data;

    // INTRQ_Wait
    // ----------
    // Wait for interrupt if nIEN=0 (i.e., if interrupt enabled).
HPIOO_INTRQ_Wait:;
    if (m_fInterruptSupported) {
        if (FALSE == WaitForInterrupt(m_dwDiskIoTimeOut) || (CheckIntrState() == ATA_INTR_ERROR)) {
            DEBUGMSG(ZONE_ERROR, (_T("Atapi!CDisk::DoWrite> Failed to wait for interrupt (m_dwDeviceId=%d)\r\n"), m_dwDeviceId));
            return FALSE;
        }
        }
    goto HPIOO_Check_Status;
            }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL CDisk::DoWrite(PBYTE pbBuf, DWORD dwStartingSector, DWORD dwNumberOfSectors, PDWORD pdwBytesWritten)
            {
    DWORD dwBytesWritten = 0;
    BYTE  bStatus;
    DWORD dwByteInDrqDataBlock = 0;
    BOOL  fComplete = FALSE;

    // Host_Idle
    // ---------
    // Issue the appropriate command.
    if (FALSE == SendIOCommand(dwStartingSector, dwNumberOfSectors, m_bWriteCommand)) {
        DEBUGMSG(ZONE_ERROR, (_T("Atapi!CDisk::DoWrite> Failed to issue read/write command\r\n")));
        return FALSE;
                }

        // Check_Status
        // ------------
    // If BSY=0 and DRQ=0, transition to Host_Idle.
    // If BSY=1, re-enter this state.
    // If BSY=0 and DRQ=1, transition to Transfer_Data.
HPIOO_Check_Status:;
    // Read the Status register.
    bStatus = GetAltStatus();
    // Test for BSY=0 and DRQ=0.
    if ((!(bStatus & 0x80)) && (!(bStatus & 0x08))) {
        if (fComplete) {
            return TRUE;
        }
    }
    // Test for BSY=1.
    if (bStatus & 0x80) {
        goto HPIOO_Check_Status;
    }
    // Test for BSY=0 and DRQ=1.
    if ((!(bStatus & 0x80)) && (bStatus & 0x08)) {
        goto HPIOO_Transfer_Data_Reset_DRQ_Data_Block;
        }
    // Illegal status.
    ASSERT(FALSE);
    return FALSE;

        // Transfer_Data
        // -------------
    // If write Data register, DRQ data block transfer not complete, re-enter
    // this state.
    // If write Data register, all data for command transferred, transition to
    // Host_Idle.
    // If write Data register, DRQ data block transferred, all data for command
    // not transferred, and nIEN=1, transition to Check_Status.
    // If write Data register, DRQ data block transferred, all data for command
    // not transferred, and nIEN=0, transition to INTRQ_Wait.
HPIOO_Transfer_Data_Reset_DRQ_Data_Block:;
    dwByteInDrqDataBlock = 0;
HPIOO_Transfer_Data:;
    // Write to the Data register.
    if (m_f16Bit) {
        WriteWord(*((PWORD)pbBuf));
        pbBuf += 2;
        dwByteInDrqDataBlock += 2;
        dwBytesWritten += 2;
    }
    else {
        WriteByte(*((PBYTE)pbBuf));
        pbBuf += 1;
        dwByteInDrqDataBlock += 1;
        dwBytesWritten += 1;
    }
    if (0 == (dwByteInDrqDataBlock % SECTOR_SIZE)) {
        dwNumberOfSectors -= 1;
    }
    // Is the transfer complete?
    if (0 == dwNumberOfSectors) {
        *pdwBytesWritten = dwBytesWritten;
        fComplete = TRUE;
        goto HPIOO_INTRQ_Wait;
    }
    // The transfer is not complete.  Has a DRQ data block been transferred?
    if (dwByteInDrqDataBlock == ((DWORD)SECTOR_SIZE * m_bSectorsPerBlock)) {
        goto HPIOO_INTRQ_Wait;
    }
    // A DRQ data block has not been transferred.  Continue transferring.
    goto HPIOO_Transfer_Data;

    // INTRQ_Wait
    // ----------
    // Wait for interrupt if nIEN=0 (i.e., if interrupt enabled).
HPIOO_INTRQ_Wait:;
    if (m_fInterruptSupported) {
        if (FALSE == WaitForInterrupt(m_dwDiskIoTimeOut) || (CheckIntrState() == ATA_INTR_ERROR)) {
            DEBUGMSG(ZONE_ERROR, (_T("Atapi!CDisk::DoWrite> Failed to wait for interrupt (m_dwDeviceId=%d)\r\n"), m_dwDeviceId));
            return FALSE;
        }
    }
    goto HPIOO_Check_Status;
        }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
DWORD CDisk::WriteDisk(PIOREQ pIoReq)
        {
    DWORD   m_dwDblBufSectorCount = 0;
    PSG_REQ pSgReq;
    CSgReq  SgReqWrapper;
    DWORD   dwStartingSector;
    DWORD   dwSectorsRemaining;
    DWORD   dwNumberOfSectors;
    DWORD   dwAbsoluteBufferLength;
    DWORD   dwBytesRead = 0;
    DWORD   dwBytesWritten = 0;

    DEBUGCHK(NULL != pIoReq);

    // Unpack the Scatter/Gather request.
    pSgReq = (PSG_REQ)pIoReq->pInBuf;

    // Ensure the Scatter/Gather request does not extend past the end of the
    // disk.
    if ((pSgReq->sr_start + pSgReq->sr_num_sec) > m_DiskInfo.di_total_sectors) {
        return  ERROR_SECTOR_NOT_FOUND;
        }

    // Attach the Scatter/Gather request to the wrapper.
    if (FALSE == SgReqWrapper.DoAttach(pSgReq, SECTOR_SIZE)) {
        return ERROR_GEN_FAILURE;
    }

    // Prepare the transfer.
    dwStartingSector = SgReqWrapper.GetStartingSector();
    dwSectorsRemaining = SgReqWrapper.GetNumberOfSectors();
    dwNumberOfSectors = 0;
    dwAbsoluteBufferLength = SgReqWrapper.GetAbsoluteBufferLength();

    // Is double buffering required?
    if ((SgReqWrapper.GetNumberOfBuffers() > 1) && (NULL != m_rgbDoubleBuffer)) {
        m_dwDblBufSectorCount = (m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize / SECTOR_SIZE); // dwDrqDataBlockSize
        // Fill the double buffer from the Scatter/Gather buffer list and write
        // to disk.
        while (SgReqWrapper.GetAbsoluteBufferPosition() < dwAbsoluteBufferLength) {
            // Fill the double buffer from the Scatter/Gather buffer list.
            dwNumberOfSectors = MIN(m_dwDblBufSectorCount, dwSectorsRemaining);
            if (FALSE == SgReqWrapper.DoReadMultiple(m_rgbDoubleBuffer, SECTOR_SIZE * dwNumberOfSectors, &dwBytesRead)) {
                return ERROR_GEN_FAILURE;
                }
            ASSERT(dwBytesRead == (SECTOR_SIZE * dwNumberOfSectors));
            // Write to disk.
            if (FALSE == this->DoWrite(m_rgbDoubleBuffer, dwStartingSector, dwNumberOfSectors, &dwBytesWritten)) {
                return ERROR_GEN_FAILURE;
            }
            ASSERT(dwBytesRead == dwBytesWritten);
            dwStartingSector += dwNumberOfSectors;
            dwSectorsRemaining -= dwNumberOfSectors;
                    }
                }
    else {
        // Empty the Scatter/Gather buffer list to the disk.
        while (SgReqWrapper.GetAbsoluteBufferPosition() < dwAbsoluteBufferLength) {
            dwNumberOfSectors = MIN(256, dwSectorsRemaining);
            if (FALSE == this->DoWrite(&SgReqWrapper, dwStartingSector, dwNumberOfSectors, &dwBytesWritten)) {
                return ERROR_GEN_FAILURE;
            }
            ASSERT(dwBytesWritten == (SECTOR_SIZE * dwNumberOfSectors));
            dwStartingSector += dwNumberOfSectors;
            dwSectorsRemaining -= dwNumberOfSectors;
        }
        }

    *pIoReq->pBytesReturned = dwBytesWritten;
    return ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------
// Function: ReadWriteDiskDMA
//     This function reads from/writes to an ATA device
//
// Parameters:
//     pIOReq -
//     fRead -
// ----------------------------------------------------------------------------

DWORD
CDisk::ReadWriteDiskDMA(
    PIOREQ pIOReq,
    BOOL fRead
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSG_REQ pSgReq = (PSG_REQ)pIOReq->pInBuf;
    //DWORD dwSectorsToTransfer;
    SG_BUF CurBuffer[MAX_SG_BUF];
    BYTE bCmd;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: ReadWriteDiskDMA+\r\n")));
    if ((pSgReq == NULL) || (pIOReq->dwInBufSize < sizeof(SG_REQ))) {
        return ERROR_INVALID_PARAMETER;
    }
    if ((pSgReq->sr_num_sec == 0) || (pSgReq->sr_num_sg == 0)) {
        return  ERROR_INVALID_PARAMETER;
    }
    if ((pSgReq->sr_start + pSgReq->sr_num_sec) > m_DiskInfo.di_total_sectors) {
        return ERROR_SECTOR_NOT_FOUND;
    }

    DEBUGMSG(ZONE_FUNC, (_T(
              "Ata!ReadWriteDiskDMA> sr_start(%ld), sr_num_sec(%ld), sr_num_sg(%ld)\r\n"
              ), pSgReq->sr_start, pSgReq->sr_num_sec, pSgReq->sr_num_sg));

    GetBaseStatus(); // clear pending interrupt

    DWORD dwStartBufferNum = 0, dwEndBufferNum = 0, dwEndBufferOffset = 0;
    DWORD dwNumSectors = pSgReq->sr_num_sec;
    DWORD dwStartSector = pSgReq->sr_start;
    DWORD dwMaxSectDMA;

    // process scatter/gather buffers in groups of MAX_SEC_PER_COMMAND sectors
    // each DMA request handles a new SG_BUF array which is a subset of the
    // original request, and may start/stop in the middle of the original buffer

    while (dwNumSectors) {

        // determine number of sectors to transfer
        dwMaxSectDMA = CSPATAMaxSectDMA();

        dwSectorsToTransfer = (dwNumSectors > dwMaxSectDMA) ? dwMaxSectDMA : dwNumSectors;

        // determine size (in bytes) of transfer
        DWORD dwBufferLeft = dwSectorsToTransfer * BYTES_PER_SECTOR;

        DWORD dwNumSg = 0;

        // while the transfer is not complete
        while (dwBufferLeft) {

            // determine the size of the current scatter/gather buffer
            DWORD dwCurBufferLen = pSgReq->sr_sglist[dwEndBufferNum].sb_len - dwEndBufferOffset;

            if (dwBufferLeft < dwCurBufferLen) {
                CurBuffer[dwEndBufferNum - dwStartBufferNum].sb_buf = pSgReq->sr_sglist[dwEndBufferNum].sb_buf + dwEndBufferOffset;
                CurBuffer[dwEndBufferNum - dwStartBufferNum].sb_len = dwBufferLeft;
                dwEndBufferOffset += dwBufferLeft;
                dwBufferLeft = 0;
            }
            else {
                CurBuffer[dwEndBufferNum - dwStartBufferNum].sb_buf = pSgReq->sr_sglist[dwEndBufferNum].sb_buf + dwEndBufferOffset;
                CurBuffer[dwEndBufferNum - dwStartBufferNum].sb_len = dwCurBufferLen;
                dwEndBufferOffset = 0;
                dwEndBufferNum++;
                dwBufferLeft -= dwCurBufferLen;
            }
            dwNumSg++;
        }

        bCmd = fRead ? m_bDMAReadCommand : m_bDMAWriteCommand ;

        WaitForInterrupt(0);

        // setup the DMA transfer
        if (!SetupDMA(CurBuffer, dwNumSg, fRead)) {
            dwError = fRead ? ERROR_READ_FAULT : ERROR_WRITE_FAULT;
            goto ExitFailure;
        }

        // write the read/write command
        if (!SendIOCommand(dwStartSector, dwSectorsToTransfer, bCmd)) {
            dwError = fRead ? ERROR_READ_FAULT : ERROR_WRITE_FAULT;
            AbortDMA();
            goto ExitFailure;
        }

        // start the DMA transfer
        if (BeginDMA(fRead)) {
            if (m_fInterruptSupported) {
                if (!WaitForInterrupt(m_dwDiskIoTimeOut) || (CheckIntrState() == ATA_INTR_ERROR)) {
                    DEBUGMSG(ZONE_IO || ZONE_WARNING, (_T(
                        "Ata!CDisk::ReadWriteDiskDMA> Failed to wait for interrupt; device(%d)\r\n"
                        ), m_dwDeviceId));
                    dwError = ERROR_READ_FAULT;
                    AbortDMA();
                    goto ExitFailure;
                }
            }
            // stop the DMA transfer
            if (EndDMA()) {
                WaitOnBusy(FALSE);
                CompleteDMA(CurBuffer, pSgReq->sr_num_sg, fRead);
            }
        }

        // update transfer
        dwStartSector += dwSectorsToTransfer;
        dwStartBufferNum = dwEndBufferNum;
        dwNumSectors -= dwSectorsToTransfer;
    }

ExitFailure:
    if (ERROR_SUCCESS != dwError) {
        ResetController();
    }
    pSgReq->sr_status = dwError;
    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: ReadWriteDiskDMA-\r\n")));
    return dwError;
}

// ----------------------------------------------------------------------------
// Function: GetStorageId
//     Implement IOCTL_DISK_GET_STORAGEID
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetStorageId(
    PIOREQ pIOReq
    )
{
    DWORD dwBytesLeft;
    PBYTE pDstOffset;
    PSTORAGE_IDENTIFICATION pStorageId = (PSTORAGE_IDENTIFICATION)pIOReq->pOutBuf;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: GetStorageId+\r\n")));
    // validate arguments
    if (!pStorageId || (pIOReq->dwOutBufSize < sizeof(STORAGE_IDENTIFICATION)) || !pIOReq->pBytesReturned) {
        return ERROR_INVALID_PARAMETER;
    }

    // prepare return structure
    pStorageId->dwSize = sizeof(STORAGE_IDENTIFICATION);
    pStorageId->dwFlags = 0; // {MANUFACTUREID,SERIALNUM}_INVALID

    // prepare return structure indicies, for write
    dwBytesLeft = pIOReq->dwOutBufSize - sizeof(STORAGE_IDENTIFICATION);
    pDstOffset = (PBYTE)(pStorageId + 1);
    pStorageId->dwManufactureIDOffset = pDstOffset - (PBYTE)pStorageId;

    SetLastError(ERROR_SUCCESS);

    // fetch manufacturer ID
    if (!ATAParseIdString((PBYTE)m_Id.ModelNumber, sizeof(m_Id.ModelNumber), &(pStorageId->dwManufactureIDOffset), &pDstOffset, &dwBytesLeft)) {
        pStorageId->dwFlags |= MANUFACTUREID_INVALID;
    }
    pStorageId->dwSerialNumOffset = pDstOffset - (PBYTE)pStorageId;
    // fetch serial number
    if (!ATAParseIdString((PBYTE)m_Id.SerialNumber, sizeof(m_Id.SerialNumber), &(pStorageId->dwSerialNumOffset), &pDstOffset, &dwBytesLeft)) {
        pStorageId->dwFlags |= SERIALNUM_INVALID;
    }
    pStorageId->dwSize = pDstOffset - (PBYTE)pStorageId;

    // store bytes written
    *(pIOReq->pBytesReturned)= min(pStorageId->dwSize, pIOReq->dwOutBufSize);

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: GetStorageId-\r\n")));
    return GetLastError();
}

// ----------------------------------------------------------------------------
// Function: SetWriteCacheMode
//     Issue SET FEATURES enable write cache command
//
// Parameters:
//     fEnable -
// ----------------------------------------------------------------------------

BOOL
CDisk::SetWriteCacheMode(
    BOOL fEnable
    )
{
    BYTE bError, bStatus;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SetWriteCacheMode+(fEnable:%d)\r\n"), fEnable));
    // select device
    SelectDevice();

    // wait for device to acknowledge selection
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 100);
    WaitForDisc(WAIT_TYPE_READY, 1000);
    WaitOnBusy(TRUE);

    // write command
    SelectDevice();
    WriteFeature(fEnable ? ATA_ENABLE_WRITECACHE : ATA_DISABLE_WRITECACHE);
    WriteCommand(ATAPI_CMD_SET_FEATURES);

    // wait for device to respond to command
    SelectDevice();
    WaitOnBusy(TRUE);
    SelectDevice();
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 200);

    // check response
    bStatus = GetBaseStatus();
    bError = GetError();
    if ((bStatus & ATA_STATUS_ERROR) && (bError & ATA_ERROR_ABORTED)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "Ata!CDisk::SetWriteCacheMode> Failed to enable write cache; status(%02X), error(%02X)\r\n"
            ), bStatus, bError));
        ResetController(FALSE);
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SetWriteCacheMode-\r\n")));
    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: SetLookAhead
//     Issue SET FEATURES enable read look-ahead command
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CDisk::SetLookAhead(
    )
{
    BYTE bError = 0;
    BYTE bStatus = 0;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SetLookAhead+\r\n")));
    // select device
    SelectDevice();

    // wait for device to acknowledge selection
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 100);
    WaitForDisc(WAIT_TYPE_READY, 1000);
    WaitOnBusy(TRUE);

    // write command
    SelectDevice();
    WriteFeature(ATA_ENABLE_LOOKAHEAD);
    WriteCommand(ATAPI_CMD_SET_FEATURES);

    // wait for device to respond to command
    SelectDevice();
    WaitOnBusy(TRUE);
    SelectDevice();
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 200);

    // check response
    bStatus = GetBaseStatus();
    bError = GetError();
    if ((bStatus & ATA_STATUS_ERROR) && (bError & ATA_ERROR_ABORTED)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "Ata!CDisk::SetLookAhead> Failed to enable read look-ahead; status(%02X), error(%02X)\r\n"
            ), bStatus, bError));
        ResetController(FALSE);
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: SetLookAhead-\r\n")));
    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: FlushCache
//     Issue FLUSH CACHE command; this command may take up to 30s to complete
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

DWORD
CDisk::FlushCache(
    )
{
    BYTE bError = 0;
    BYTE bStatus = 0;
    BOOL fOk = TRUE;

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: FlushCache+\r\n")));
    // if write cache is not enabled, then fail
    if (!(m_dwDeviceFlags & DFLAGS_USE_WRITE_CACHE)) {
        return ERROR_NOT_SUPPORTED;
    }

    // HI:Check_Status (Host Idle); wait until BSY=0 and DRQ=0
    // read Status register
    for (;;) {
        bStatus = GetAltStatus();
        if (!(bStatus & (0x80|0x08))) break; // BSY := Bit 7, DRQ := Bit 3
        Sleep(5);
    }

    // HI:Device_Select; select device
    SelectDevice();

    // HI:Check_Status (Host Idle); wait until BSY=0 and DRQ=0
    // read Status register
    for (;;) {
        bStatus = GetAltStatus();
        if (!(bStatus & (0x80|0x08))) break; // BSY := Bit 7, DRQ := Bit 3
        Sleep(5);
    }

    // HI:Write_Command
    WriteCommand(ATA_CMD_FLUSH_CACHE);

    // transition to non-data command protocol

    // HND:INTRQ_Wait
    // transition to HND:Check_Status
    // read Status register
    for (;;) { // BSY := Bit 7
        bStatus = GetAltStatus();
        bError = GetError();
        if (bError & 0x04) { // ABRT := Bit 2
            // command was aborted
            DEBUGMSG(ZONE_ERROR, (_T(
                "Ata!CDisk::FlushCache> Failed to send FLUSH CACHE\r\n"
                )));
            fOk = FALSE;
            break;
        }
        if (!(bStatus & 0x80)) break; // BSY := Bit 7
        Sleep(5);
    }

    // transition to host idle protocol

    DEBUGMSG(ZONE_FUNC, (TEXT("CDisk: FlushCache-\r\n")));
    return (fOk ? ERROR_SUCCESS : ERROR_GEN_FAILURE);
}

