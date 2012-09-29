/*++

Module Name:
    usbser.cpp

Abstract:
    USB Client Driver for Exar USB serial Class.

Functions:

Notes:

--*/

#include <windows.h>
#include <types.h>
#include <usbser.h>

#define ICLASSSTRING TEXT("{CC5195AC-BA49-48a0-BE17-DF6D1B0173DD}")
#define PREFIX_STRING TEXT("COM")
#define XRUSBSER_CLASS TEXT("XRUSBSER")

USB_DRIVER_SETTINGS SerDriverSettings_1414 = {
    sizeof(USB_DRIVER_SETTINGS),
        0x04e2,0x1414,USB_NO_INFO,//Vendor Info
        USB_NO_INFO,USB_NO_INFO,USB_NO_INFO,//Device Info
        USB_NO_INFO,USB_NO_INFO,USB_NO_INFO // Interface Info.
};
USB_DRIVER_SETTINGS SerDriverSettings_1412 = {
    sizeof(USB_DRIVER_SETTINGS),
        0x04e2,0x1412,USB_NO_INFO,//Vendor Info
        USB_NO_INFO,USB_NO_INFO,USB_NO_INFO,//Device Info
        USB_NO_INFO,USB_NO_INFO,USB_NO_INFO // Interface Info.
};
USB_DRIVER_SETTINGS SerDriverSettings_1410 = {
    sizeof(USB_DRIVER_SETTINGS),
        0x04e2,0x1410,USB_NO_INFO,//Vendor Info
        USB_NO_INFO,USB_NO_INFO,USB_NO_INFO,//Device Info
        USB_NO_INFO,USB_NO_INFO,USB_NO_INFO // Interface Info.
};

static DWORD dwUsbSerIndex = 0;
REGINI UsbSerReg[] = {
   {TEXT("Prefix"),(PBYTE)PREFIX_STRING,sizeof(PREFIX_STRING),REG_SZ},
   {TEXT("DeviceArrayIndex"),(PBYTE)&dwUsbSerIndex,sizeof(DWORD),REG_DWORD},
   {TEXT("IClass"),(PBYTE)ICLASSSTRING,sizeof(ICLASSSTRING),REG_SZ}
};


/*
 * @func   BOOL | USBInstallDriver | Install USB client driver.
 * @rdesc  Return TRUE if install succeeds, or FALSE if there is some error.
 * @comm   This function is called by USBD when an unrecognized device
 *         is attached to the USB and the user enters the client driver
 *         DLL name.  It should register a unique client id string with
 *         USBD, and set up any client driver settings.
 * @xref   <f USBUnInstallDriver>
 */
extern "C" BOOL 
USBInstallDriver(
    LPCWSTR szDriverLibFile)  // @parm [IN] - Contains client driver DLL name
{
    DEBUGMSG(ZONE_INIT, (TEXT("Start USBInstallDriver %s "),szDriverLibFile));
    BOOL fRet = FALSE;
    USBDriverClass usbInstall(ZONE_INIT);
    if(szDriverLibFile && usbInstall.Init()) {
        size_t szLength = 0;
        StringCchLength( szDriverLibFile,MAX_PATH,&szLength);
        REGINI DriverDllEntry = {TEXT("Dll"),(PBYTE)szDriverLibFile,(szLength +1)*sizeof(TCHAR),REG_SZ };
		if (usbInstall.RegisterClientDriverID(XRUSBSER_CLASS))
		{
			if(usbInstall.RegisterClientSettings(szDriverLibFile,XRUSBSER_CLASS,&SerDriverSettings_1414))
			{ 
				usbInstall.SetDefaultDriverRegistry(XRUSBSER_CLASS, sizeof(UsbSerReg)/sizeof(UsbSerReg[0]), UsbSerReg, TRUE );
				usbInstall.SetDefaultDriverRegistry(XRUSBSER_CLASS, 1 , &DriverDllEntry, TRUE );
			}
			if(usbInstall.RegisterClientSettings(szDriverLibFile,XRUSBSER_CLASS,&SerDriverSettings_1412))
			{ 
				usbInstall.SetDefaultDriverRegistry(XRUSBSER_CLASS, sizeof(UsbSerReg)/sizeof(UsbSerReg[0]), UsbSerReg, TRUE );
				usbInstall.SetDefaultDriverRegistry(XRUSBSER_CLASS, 1 , &DriverDllEntry, TRUE );
			}
			if(usbInstall.RegisterClientSettings(szDriverLibFile,XRUSBSER_CLASS,&SerDriverSettings_1410))
			{ 
				usbInstall.SetDefaultDriverRegistry(XRUSBSER_CLASS, sizeof(UsbSerReg)/sizeof(UsbSerReg[0]), UsbSerReg, TRUE );
				usbInstall.SetDefaultDriverRegistry(XRUSBSER_CLASS, 1 , &DriverDllEntry, TRUE );
			}
		}
        fRet = TRUE;
    }
    
    DEBUGMSG(ZONE_INIT, (TEXT("End USBInstallDriver return value = %d "),fRet));
    ASSERT(fRet);
    return fRet;
}

/*
 * @func   BOOL | USBUnInstallDriver | Uninstall USB client driver.
 * @rdesc  Return TRUE if install succeeds, or FALSE if there is some error.
 * @comm   This function can be called by a client driver to deregister itself
 *         with USBD.
 * @xref   <f USBInstallDriver>
 */
extern "C" BOOL 
USBUnInstallDriver()
{
    BOOL fRet = FALSE;
    USBDriverClass usbInstall(ZONE_INIT);
    if(usbInstall.Init()) {
        usbInstall.UnRegisterClientSettings(XRUSBSER_CLASS,NULL,&SerDriverSettings_1414);
        fRet = TRUE;
    }
    return fRet;
}
// Attach
BOOL SerialUsbClientDevice::IsThisInterfaceSupported(LPCUSB_INTERFACE usbInterface)
{
    if ( usbInterface && (usbInterface->Descriptor.bNumEndpoints==2 || usbInterface->Descriptor.bNumEndpoints==3) ) {
        // BulkIn, BulkOut, Option Interrupt.
        DWORD dwEndpointss = usbInterface->Descriptor.bNumEndpoints;
        LPCUSB_ENDPOINT lpEndpoint = usbInterface->lpEndpoints ;
        BOOL fBulkIn = FALSE;
        BOOL fBulkOut = FALSE;
        while (dwEndpointss && lpEndpoint!=NULL) {
            if ((lpEndpoint->Descriptor.bmAttributes &  USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK) {
                if (USB_ENDPOINT_DIRECTION_IN(lpEndpoint->Descriptor.bEndpointAddress))
                    fBulkIn = TRUE;
                else
                    fBulkOut = TRUE;
            }
            dwEndpointss--;
            lpEndpoint++;
        }
        ASSERT(fBulkIn && fBulkOut);
        if (fBulkIn && fBulkOut)
		{
            m_lpTargetInterface = usbInterface ;
			m_lpTargetInterface_1414[usbInterface->Descriptor.bInterfaceNumber >> 1] = usbInterface;
		}
        else
            ASSERT(FALSE);
        return (fBulkIn && fBulkOut);
    }
    return FALSE;
}

BOOL 
USBDeviceAttach(
    USB_HANDLE hDevice,           // @parm [IN] - USB device handle
    LPCUSB_FUNCS lpUsbFuncs,      // @parm [IN] - Pointer to USBDI function table
    LPCUSB_INTERFACE lpInterface, // @parm [IN] - If client is being loaded as an interface
    LPCWSTR szUniqueDriverId,     // @parm [IN] - Contains client driver id string
    LPBOOL fAcceptControl,        // @parm [OUT]- Filled in with TRUE if we accept control 
    LPCUSB_DRIVER_SETTINGS lpDriverSettings,// @parm [IN] - Contains pointer to USB_DRIVER_SETTINGS
    DWORD dwUnused)               // @parm [IN] - Reserved for use with future versions of USBD
{
    DEBUGMSG(ZONE_INIT, (TEXT("Device Attach")));
    PREFAST_ASSERT(fAcceptControl!=NULL)
    *fAcceptControl = FALSE;
    SerialUsbClientDevice * pUsbDevice = new SerialUsbClientDevice(hDevice,lpUsbFuncs,lpInterface,szUniqueDriverId,lpDriverSettings);
    if (pUsbDevice && pUsbDevice->Init()) {
        *fAcceptControl = TRUE;
    }
    else if (pUsbDevice) {
        delete pUsbDevice;
    }
    return TRUE;
};
// Function.
DMA_ADAPTER_OBJECT SerialDataIn::m_Adapter = {sizeof(DMA_ADAPTER_OBJECT),Internal,0} ;
SerialDataIn::SerialDataIn(USB_ENDPOINT usbEndpoint,UsbClientDevice *lpClientDevice,LPTHREAD_START_ROUTINE lpCallbackAddr, LPVOID lpvCallbackParam) 
:   UsbAsyncClassPipe (usbEndpoint,lpClientDevice,NUM_OF_IN_TRANSFER,ZONE_READ)
,   CMiniThread(0,TRUE)
,   m_lpCallbackAddr(lpCallbackAddr)
,   m_lpvCallbackParam(lpvCallbackParam)
{
    m_VirtualAddress = NULL;
    m_dwCurIndex  = 0 ;
    m_hThreadRun = CreateEvent(NULL,TRUE,FALSE,NULL);
}
SerialDataIn::~SerialDataIn() 
{
    m_bTerminated = TRUE;
    InitReceive(FALSE);
    if (m_hThreadRun)
        SetEvent(m_hThreadRun);
    ThreadStart();
    ThreadTerminated( 1000 );
    if (m_VirtualAddress!=NULL) {
        HalFreeCommonBuffer(&m_Adapter,m_dwTotalSize,m_PhysicalAddress,m_VirtualAddress,FALSE);
    }
    if (m_hThreadRun)
        CloseHandle(m_hThreadRun);
}
BOOL SerialDataIn::Init() 
{
    if (UsbAsyncClassPipe::Init() && Descriptor.wMaxPacketSize !=0) {
        m_dwNumOfSegment = NUM_OF_IN_TRANSFER;
        m_dwTotalSize= Descriptor.wMaxPacketSize * NUM_OF_IN_TRANSFER;
#if ORIG // this seems to read all the data in the uart continuosly in large chunks without letting the h/w flow control kick in - ravi
        m_dwTotalSize = (m_dwTotalSize /PAGE_SIZE + 1)*PAGE_SIZE;
        m_dwSegmentSize = (m_dwTotalSize /NUM_OF_IN_TRANSFER / Descriptor.wMaxPacketSize)* Descriptor.wMaxPacketSize ;
#else	// reading lesser data per shot lets the uart to slow down if flow control is set - 0x40 is the size now. earlier it used to be 0x1000
        m_dwSegmentSize = m_dwTotalSize;
#endif
        ASSERT((m_dwSegmentSize & (Descriptor.wMaxPacketSize-1)) == 0); // Aligment needed for transfer.
        m_VirtualAddress = (PBYTE)HalAllocateCommonBuffer(&m_Adapter,m_dwTotalSize,&m_PhysicalAddress,FALSE);
        if (m_VirtualAddress!=NULL) {
            ThreadStart();
            return TRUE;
        }
        else {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    return FALSE;
}
BOOL SerialDataIn::InitReceive(BOOL bInit) 
{
    Lock();
    m_dwCurIndex = 0;
    if (bInit) {
        m_dwCurIndex = 0 ;
        ResetTransferQueue();
        ResetPipe(TRUE);
        for (DWORD dwIndex = 0 ; dwIndex < m_dwNumOfSegment; dwIndex ++) {
            m_dwSegDataOffset[dwIndex] = m_dwSegDataDataLen [dwIndex] = 0; 
            BOOL bResult = BulkOrIntrTransfer(USB_NO_WAIT|USB_SHORT_TRANSFER_OK,
                m_dwSegmentSize,
                m_VirtualAddress+(dwIndex*m_dwSegmentSize),
                m_PhysicalAddress.u.LowPart+(dwIndex*m_dwSegmentSize),
                SERIAL_DATAIN_COOKIE+dwIndex);
            ASSERT(bResult==TRUE);
        }
        if (m_hThreadRun)
            SetEvent(m_hThreadRun);
    }
    else {
        CloseAllArmedTransfer();
        if (m_hThreadRun)
            ResetEvent(m_hThreadRun);
    }
    Unlock();
    return TRUE;
}
BOOL SerialDataIn::ReceiveInterruptHandler(PUCHAR pRxBuffer,ULONG *pBufflen)
{
    Lock();
    BOOL bReturn = FALSE;
    if (pRxBuffer && pBufflen && *pBufflen) {
        DWORD dwBufferSize = *pBufflen ;
        *pBufflen = 0 ;
		DWORD dwClientInfo = GetClientInfo();
        ASSERT((dwClientInfo & SERIAL_DATAIN_COOKIE_MASK)== SERIAL_DATAIN_COOKIE);
        ASSERT(m_dwCurIndex == (dwClientInfo & (~SERIAL_DATAIN_COOKIE_MASK)));
        m_dwCurIndex =(dwClientInfo & (~SERIAL_DATAIN_COOKIE_MASK));
        if ( IsFrontArmedTransferComplete () ) {
            if (m_dwSegDataDataLen[m_dwCurIndex] == 0) { // Have't update the Transfer Status yet.
                DWORD dwError = USB_NO_ERROR;
                DWORD dwLength;
                if (GetFrontArmedTransferStatus(&dwLength, &dwError) && dwError == USB_NO_ERROR) { // Complete with no error.
                    m_dwSegDataOffset[m_dwCurIndex] = 0;
                    m_dwSegDataDataLen[m_dwCurIndex] = min(dwLength,m_dwSegmentSize);
                    DWORD dwCopyLength = min(m_dwSegDataDataLen[m_dwCurIndex],dwBufferSize);
                    memcpy(pRxBuffer,m_VirtualAddress + m_dwSegmentSize*m_dwCurIndex ,dwCopyLength);
                    m_dwSegDataOffset[m_dwCurIndex] = dwCopyLength;
                    *pBufflen = dwCopyLength;
                    bReturn = TRUE;
                    DEBUGMSG(ZONE_READ,(TEXT("ReceiveInterruptHandler:%d Copied  ,Transfer Size=%d"),dwCopyLength,dwLength));
                }
                else {
                    DEBUGMSG(ZONE_ERROR,(TEXT("ReceiveInterruptHandler:Transfer Error %d ,dwLength=%d"),dwError,dwLength));                    
                }
            } 
			// ravi - updated logic: if *pBufflen sent from MDD is smaller than the data available,
			// then in the previous call (in the previous function) only that much data will be sent
			// and in the next call the above if case (if (m_dwSegDataDataLen[m_dwCurIndex] == 0))
			// will fail and the rest of the data will be sent in this case
			if(!bReturn)
			if (m_dwSegDataOffset[m_dwCurIndex] < m_dwSegDataDataLen[m_dwCurIndex] ){ // We have Extra.
                DWORD dwCopyLen = min (dwBufferSize, m_dwSegDataDataLen[m_dwCurIndex] - m_dwSegDataOffset[m_dwCurIndex]) ;
                memcpy(pRxBuffer,m_VirtualAddress + (m_dwSegmentSize*m_dwCurIndex + m_dwSegDataOffset[m_dwCurIndex]) ,dwCopyLen);
                *pBufflen = dwCopyLen;
                m_dwSegDataOffset[m_dwCurIndex] += dwCopyLen; // this is the actual code that fixed extra data bug.
															  // wrong value was being updated here
                bReturn = TRUE;
            }

            if (m_dwSegDataOffset[m_dwCurIndex]>= m_dwSegDataDataLen[m_dwCurIndex] ){
                CloseFrontArmedTransfer();
                m_dwSegDataOffset[m_dwCurIndex]= m_dwSegDataDataLen[m_dwCurIndex] =  0 ;
                BOOL bResult = BulkOrIntrTransfer(USB_NO_WAIT|USB_SHORT_TRANSFER_OK,
                    m_dwSegmentSize,
                    m_VirtualAddress+ (m_dwCurIndex * m_dwSegmentSize),
                    m_PhysicalAddress.LowPart+(m_dwCurIndex*m_dwSegmentSize),
                    SERIAL_DATAIN_COOKIE+m_dwCurIndex);
                ASSERT(bResult==TRUE);
                m_dwCurIndex ++;
                if (m_dwCurIndex>=NUM_OF_IN_TRANSFER)
                    m_dwCurIndex = 0;
            }
        }
    }
    Unlock();
    return bReturn;
}
ULONG   SerialDataIn::CancelReceive() 
{
    Lock();
    m_dwSegDataDataLen[m_dwCurIndex] = 0;
    DWORD dwIndex = NUM_OF_IN_TRANSFER;
    while (IsFrontArmedTransferComplete () && dwIndex) {
        CloseFrontArmedTransfer();
        m_dwSegDataDataLen[m_dwCurIndex]  = 0;
        BOOL bResult = BulkOrIntrTransfer(USB_NO_WAIT|USB_SHORT_TRANSFER_OK,
            m_dwSegmentSize,
            m_VirtualAddress+ (m_dwCurIndex * m_dwSegmentSize),
            m_PhysicalAddress.LowPart+(m_dwCurIndex*m_dwSegmentSize),
            SERIAL_DATAIN_COOKIE+m_dwCurIndex);
        m_dwCurIndex ++;
        dwIndex --;
    }
    ASSERT(dwIndex !=0);
    Unlock() ;
    return 0;
}
DWORD SerialDataIn::ThreadRun() 
{
    while ( !IsTerminated() ) {
        if (m_hThreadRun)
            WaitForSingleObject(m_hThreadRun,INFINITE);
        BOOL bReturn = WaitForTransferComplete();
        ASSERT(bReturn == TRUE);
        if  (!IsTerminated() && m_lpCallbackAddr!=NULL)
            (*m_lpCallbackAddr)(m_lpvCallbackParam);
    }
    return 0;
};
#define NUM_OF_OUT_TRANSFER 1
SerialDataOut::SerialDataOut(USB_ENDPOINT usbEndpoint,UsbClientDevice *lpClientDevice,LPTHREAD_START_ROUTINE lpCallbackAddr, LPVOID lpvCallbackParam)
:   UsbAsyncClassPipe (usbEndpoint,lpClientDevice,NUM_OF_OUT_TRANSFER,ZONE_WRITE)
,   CMiniThread(0,TRUE)
,   m_lpCallbackAddr(lpCallbackAddr)
,   m_lpvCallbackParam(lpvCallbackParam)
{
    m_fZeroLengthNeeded = FALSE;
    m_hThreadRun = CreateEvent(NULL,TRUE,FALSE,NULL);
}
SerialDataOut::~SerialDataOut() 
{
    m_bTerminated = TRUE;
    CloseAllArmedTransfer();
    if (m_hThreadRun)
        SetEvent(m_hThreadRun);
    ThreadStart() ;
    ThreadTerminated( 1000 );
    if (m_hThreadRun)
        CloseHandle(m_hThreadRun);
}
BOOL SerialDataOut::Init()
{
    BOOL bReturn = FALSE;
    if (UsbAsyncClassPipe::Init() && m_hThreadRun!=NULL && Descriptor.wMaxPacketSize !=0) {
        ThreadStart();
        bReturn = TRUE;
    };
    return TRUE;
}
BOOL    SerialDataOut::InitXmit(BOOL bInit) 
{
    Lock();
    m_fZeroLengthNeeded = FALSE;
    if (bInit) {
        ResetPipe(TRUE);
        if (m_hThreadRun)
            SetEvent(m_hThreadRun);
    }
    else {
        if (m_hThreadRun)
            ResetEvent(m_hThreadRun);
        CloseAllArmedTransfer();
    }
    Unlock();
    return TRUE;
}
void SerialDataOut::XmitInterruptHandler(PUCHAR pTxBuffer, ULONG *pBuffLen) 
{ 
    Lock();
    if (IsEmptyTransferAvailable() || IsFrontArmedTransferComplete()) {
        CloseFrontArmedTransfer(); // Close Outstanding Transfer if we have any because MDD say so.
        PREFAST_DEBUGCHK(pBuffLen!=NULL);
        if (*pBuffLen) {
            if (IsEmptyTransferAvailable()) {
                BOOL bResult = BulkOrIntrTransfer(USB_NO_WAIT|USB_SHORT_TRANSFER_OK, *pBuffLen,pTxBuffer,0);
                ASSERT(bResult == TRUE);
                m_fZeroLengthNeeded = (( *pBuffLen & (Descriptor.wMaxPacketSize-1)) == 0? TRUE: FALSE);
                SetEvent(m_hThreadRun);
            }
            else
                ASSERT(FALSE);
        }
        else if (m_fZeroLengthNeeded) {
            BOOL bResult = BulkOrIntrTransfer(USB_NO_WAIT|USB_SHORT_TRANSFER_OK, 0 ,NULL,0);
            ASSERT(bResult == TRUE);
            m_fZeroLengthNeeded = FALSE;
            SetEvent(m_hThreadRun);
        }
        else 
            ResetEvent(m_hThreadRun);
    }
    else { // We can not do anything here. We may need wait for next next.
        *pBuffLen = 0;
        SetEvent(m_hThreadRun);
    }
    
    Unlock();
}
void    SerialDataOut::XmitComChar(UCHAR ComChar) 
{
    // This function has to poll until the Data can be sent out.
    BOOL bDone = FALSE;
    DWORD dwCount = 3;
    do {
        Lock();
        if (IsEmptyTransferAvailable() ) {  // If Empty.
            BOOL bResult = BulkOrIntrTransfer(USB_NO_WAIT|USB_SHORT_TRANSFER_OK,1, &ComChar,0);
            if (bResult) {
                WaitForTransferComplete(1000);
                CloseFrontArmedTransfer();
            }
            bDone = TRUE;
            ASSERT(bResult == TRUE);
        }
        Unlock();
        if (!bDone)
            WaitForTransferComplete(1000);
    }
    while (!bDone && dwCount--);
    
}
BOOL    SerialDataOut::CancelXmit() 
{
    if (!IsEmptyTransferAvailable() )
        CloseAllArmedTransfer();
    return TRUE;
}
DWORD  SerialDataOut::ThreadRun() 
{
    while ( !IsTerminated() ) {
        if (m_hThreadRun)
            WaitForSingleObject(m_hThreadRun,INFINITE);
        if ( !IsTerminated()) {
            BOOL bReturn = WaitForTransferComplete();
            ASSERT(bReturn == TRUE);
            if  (!IsTerminated() && m_lpCallbackAddr!=NULL)
                (*m_lpCallbackAddr)(m_lpvCallbackParam);
        }
    }
    return 0;
}

UsbSerDataStatus::UsbSerDataStatus(USB_ENDPOINT usbEndpoint,UsbClientDevice *lpClientDevice,LPTHREAD_START_ROUTINE lpCallbackAddr, LPVOID lpvCallbackParam)
:   UsbSyncClassPipe(usbEndpoint,lpClientDevice,ZONE_FLOW)
,   CMiniThread(0,TRUE)
,   m_lpCallbackAddr(lpCallbackAddr)
,   m_lpvCallbackParam(lpvCallbackParam)
{
    m_dwPrevModemData = m_dwCurModemData = 0;
    m_hThreadRun = CreateEvent(NULL,TRUE,FALSE,NULL);
}
UsbSerDataStatus::~UsbSerDataStatus() 
{
    m_bTerminated = TRUE;
    if (m_hThreadRun)
        SetEvent(m_hThreadRun);
    CloseTransfer();
    ThreadStart() ;
    ThreadTerminated( 1000 );
}
BOOL UsbSerDataStatus::InitModem(BOOL bInit) 
{
    if (bInit) {
        ResetPipe(TRUE);
        SetEvent(m_hThreadRun);
        ThreadStart();
    }
    else {
        CloseTransfer() ;
    }
    return TRUE;
}
ULONG UsbSerDataStatus::GetModemStatus(ULONG& ulEvent) 
{
    Lock();
    ulEvent = 0;
    DWORD dwEventDiff = m_dwCurModemData ^ m_dwPrevModemData;
    m_dwPrevModemData = m_dwCurModemData;
    ulEvent |= ((dwEventDiff & MS_CTS_ON )? EV_CTS: 0);
    ulEvent |= ((dwEventDiff & MS_RLSD_ON )? EV_RLSD: 0);
    ulEvent |= ((dwEventDiff & MS_DSR_ON )? EV_DSR: 0);
    ulEvent |= ((dwEventDiff & MS_RING_ON )? EV_RING: 0);        
    Unlock();
    return m_dwCurModemData;
}
DWORD  UsbSerDataStatus::ThreadRun() 
{
    while ( !IsTerminated() && m_lpTransfer) {
        if (m_hThreadRun)
            WaitForSingleObject(m_hThreadRun,INFINITE);
        USB_COMM_SERIAL_STATUS CommSerialStatus ;
        BOOL bReturn = BulkOrIntrTransfer(0,sizeof(CommSerialStatus), &CommSerialStatus,0,INFINITE);
        if  (!IsTerminated()) {
            if (m_lpTransfer->IsCompleteNoError()) {
                DWORD dwByteTransfered= 0;
                DWORD dwUsbError = USB_NO_ERROR;
                if (m_lpTransfer->GetTransferStatus(&dwByteTransfered,&dwUsbError) && dwByteTransfered==sizeof(CommSerialStatus)) {
                    if (CommSerialStatus.Notification == USB_COMM_SERIAL_STATE) {
                        Lock();
                        m_dwCurModemData = MS_CTS_ON|MS_RLSD_ON|MS_DSR_ON|MS_RING_ON;
                        m_dwCurModemData &= ~((CommSerialStatus.SerialState&USB_COMM_DCD)==0?MS_RLSD_ON:0);
                        m_dwCurModemData &= ~((CommSerialStatus.SerialState&USB_COMM_DSR)==0?MS_DSR_ON:0);
                        m_dwCurModemData &= ~((CommSerialStatus.SerialState&USB_COMM_RING)==0?MS_RING_ON:0);
                        Unlock();
                        DEBUGMSG(ZONE_FLOW,(TEXT("UsbSerDataStatus::ThreadRun: ModemStatus SerialState=%x, m_dwCurModemData=%x updated"),CommSerialStatus.SerialState,m_dwCurModemData));
                        if  ( m_lpCallbackAddr!=NULL)
                            (*m_lpCallbackAddr)(m_lpvCallbackParam);
                    }
                }
            }
            if  (IsPipeHalted()) { // Reset it.
                ResetPipe();
            }
        }
        if (!m_lpTransfer->IsTransferEmpty()) {
            m_lpTransfer->CloseTransfer();
        }
    }
    return 0;
}
ULONG SerialDataStatus::GetModemStatus(ULONG& ulEvent) 
{
    Lock();
    ulEvent = 0;
    DWORD dwEventDiff = m_dwCurModemData ^ m_dwPrevModemData;
    m_dwPrevModemData = m_dwCurModemData;
    ulEvent |= ((dwEventDiff & MS_CTS_ON )? EV_CTS: 0);
    ulEvent |= ((dwEventDiff & MS_RLSD_ON )? EV_RLSD: 0);
    ulEvent |= ((dwEventDiff & MS_DSR_ON )? EV_DSR: 0);
    ulEvent |= ((dwEventDiff & MS_RING_ON )? EV_RING: 0);        
    Unlock();
    return m_dwCurModemData;
}
DWORD  SerialDataStatus::ThreadRun() 
{
    while ( !IsTerminated() &&m_lpTransfer ) {
        BYTE bModemSetState[2];
        BOOL bReturn = BulkOrIntrTransfer(USB_SHORT_TRANSFER_OK,sizeof(bModemSetState), bModemSetState, 0, INFINITE);
        ASSERT(bReturn == TRUE);
        if  (!IsTerminated()) {
            if (m_lpTransfer->IsCompleteNoError()) {
                DWORD dwByteTransfered= 0;
                DWORD dwUsbError = USB_NO_ERROR;
                if (m_lpTransfer->GetTransferStatus(&dwByteTransfered,&dwUsbError) && dwByteTransfered==sizeof(bModemSetState)) {
                    Lock();
                    m_dwCurModemData = MS_CTS_ON|MS_RLSD_ON|MS_DSR_ON;
                    m_dwCurModemData &= ~((bModemSetState[0]&USBFN_SERIAL_CTS_SET)!=0?MS_CTS_ON:0);
                    m_dwCurModemData &= ~((bModemSetState[0]&USBFN_SERIAL_DSR_SET)!=0?MS_RLSD_ON|MS_DSR_ON:0);
                    Unlock();
                    if  ( m_lpCallbackAddr!=NULL)
                        (*m_lpCallbackAddr)(m_lpvCallbackParam);
                }
            }
            if  (IsPipeHalted()) { // Stop it.
                ResetEvent(m_hThreadRun);
                WaitForSingleObject(m_hThreadRun,INFINITE);
            }
        }
        if (!m_lpTransfer->IsTransferEmpty()) {
            m_lpTransfer->CloseTransfer();
        }
    }
    return 0;
}