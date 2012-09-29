/*++

Module Name:  SerDev.h

Abstract: USB Host Serial Client driver.


Notes: 
--*/
#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <serhw.h>
#include <Serdbg.h>
#include "CSerPdd.h"
#include <USerDev.h>
const USB_COMM_LINE_CODING cInitUsbCommLineCoding = {9600, USB_COMM_STOPBITS_10, USB_COMM_PARITY_NONE,8};
UsbSerClientDriver::UsbSerClientDriver(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj)
:   CSerialPDD(lpActivePath,pMdd,pHwObj)
,   m_UsbCommLineCoding(cInitUsbCommLineCoding)
{
    m_lpUsbClientDevice = NULL;
    m_lpSerialDataIn = NULL;
    m_lpSerialDataOut = NULL;
    m_lpUsbSerDataStatus = NULL;
    m_wSetModemStatus = USB_COMM_DTR|USB_COMM_RTS;
    CRegistryEdit cActiveRegistry(HKEY_LOCAL_MACHINE,lpActivePath);
    DWORD dwType;
    DWORD dwClientInfo;
    DWORD dwLen = sizeof(dwClientInfo);

	// defaults 8-N-1	
	m_UsbCommLineCoding.CharFormat = 8;
	m_UsbCommLineCoding.ParityType = USB_COMM_PARITY_NONE;
	m_UsbCommLineCoding.DataBits = USB_COMM_STOPBITS_10;
 
    if (cActiveRegistry.IsKeyOpened() && 
            cActiveRegistry.RegQueryValueEx(DEVLOAD_CLIENTINFO_VALNAME,&dwType,(PBYTE)&dwClientInfo,&dwLen) &&
            dwType == DEVLOAD_CLIENTINFO_VALTYPE && dwLen == sizeof(dwClientInfo)) {
        m_lpUsbClientDevice = (SerialUsbClientDevice *)dwClientInfo;
    }
    else
        ASSERT(FALSE);
}
BOOL UsbSerClientDriver::Init()
{
    DEBUGMSG(ZONE_INIT,(TEXT("+UsbSerClientDriver::Init")));
    if (m_lpUsbClientDevice && CSerialPDD::Init()) {
		LPCUSB_INTERFACE lpTargetInterface = m_lpUsbClientDevice->GetTargetInterface(m_lpUsbClientDevice->nexttargetinterface++);
        // BulkIn, BulkOut, Option Interrupt.
        if (lpTargetInterface) {
			currentportinterface = lpTargetInterface->Descriptor.bInterfaceNumber >> 1;
            DWORD dwEndpoints = lpTargetInterface->Descriptor.bNumEndpoints;
            LPCUSB_ENDPOINT lpEndpoint = lpTargetInterface->lpEndpoints ;
            while (dwEndpoints && lpEndpoint!=NULL) {
                if ((lpEndpoint->Descriptor.bmAttributes &  USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK) {
                    if (USB_ENDPOINT_DIRECTION_IN(lpEndpoint->Descriptor.bEndpointAddress)) {
                        CreateBulkIn(lpEndpoint);
                    }
                    else {
                        CreateBulkOut(lpEndpoint);
                    }
                }
                else
                if ((lpEndpoint->Descriptor.bmAttributes &  USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_INTERRUPT &&
                        USB_ENDPOINT_DIRECTION_IN(lpEndpoint->Descriptor.bEndpointAddress)) {
                    CreateStatusIn(lpEndpoint);
                }
                lpEndpoint ++;
                dwEndpoints --;
            }
        }
        DEBUGMSG(ZONE_INIT,(TEXT("-UsbSerClientDriver::Init(m_lpSerialDataIn =%x, m_lpSerialDataOut =%x ,m_lpUsbSerDataStatus=%x )"),
            m_lpSerialDataIn,m_lpSerialDataOut,m_lpUsbSerDataStatus));
        ASSERT(m_lpSerialDataIn!=NULL && m_lpSerialDataOut!=NULL);
        return(m_lpSerialDataIn!=NULL && m_lpSerialDataOut!=NULL); 
    }
    return FALSE;
}
BOOL UsbSerClientDriver::CreateBulkIn(LPCUSB_ENDPOINT lpEndpoint)
{
    if (m_lpSerialDataIn ==NULL) {
        m_lpSerialDataIn = new SerialDataIn(*lpEndpoint,m_lpUsbClientDevice,DataInStub,this);
        if (m_lpSerialDataIn && !m_lpSerialDataIn->Init()) {
            delete m_lpSerialDataIn;
            m_lpSerialDataIn = NULL;
        }
    }
    return (m_lpSerialDataIn !=NULL) ;
}
BOOL UsbSerClientDriver::CreateBulkOut(LPCUSB_ENDPOINT lpEndpoint)
{
    if (m_lpSerialDataOut == NULL) {
        m_lpSerialDataOut = new SerialDataOut(*lpEndpoint,m_lpUsbClientDevice,DataOutStub,this);
        if (m_lpSerialDataOut && !m_lpSerialDataOut->Init()) {
            delete m_lpSerialDataOut;
            m_lpSerialDataOut = NULL;
        }
    }
    return  (m_lpSerialDataOut != NULL);
}
BOOL UsbSerClientDriver::CreateStatusIn(LPCUSB_ENDPOINT lpEndpoint)
{
    m_wSetModemStatus = 0;
    if (m_lpUsbSerDataStatus == NULL) {
        m_lpUsbSerDataStatus = new UsbSerDataStatus(*lpEndpoint,m_lpUsbClientDevice,StatusStub, this) ;
        if (m_lpUsbSerDataStatus!=NULL && !m_lpUsbSerDataStatus->Init()) { // Fail Init
            delete m_lpUsbSerDataStatus;
            m_lpUsbSerDataStatus = NULL;
        }
    }
    return (m_lpUsbSerDataStatus != NULL);
}
UsbSerClientDriver::~UsbSerClientDriver()
{
    if ( m_lpSerialDataIn != NULL)
        delete m_lpSerialDataIn;
    if (m_lpSerialDataOut != NULL)
        delete m_lpSerialDataOut;
    if (m_lpUsbSerDataStatus != NULL)
        delete m_lpUsbSerDataStatus;
    m_lpUsbClientDevice = NULL;
}
void  UsbSerClientDriver::PreDeinit()
{
    if ( m_lpSerialDataIn != NULL) {
        delete m_lpSerialDataIn;
        m_lpSerialDataIn = NULL;
    }
    if (m_lpSerialDataOut != NULL) {
        delete m_lpSerialDataOut;
        m_lpSerialDataOut = NULL; 
    }
    if (m_lpUsbSerDataStatus != NULL) {
        delete m_lpUsbSerDataStatus;
        m_lpUsbSerDataStatus = NULL;
    }
    m_lpUsbClientDevice = NULL;
}
// Xmit.
void    UsbSerClientDriver::XmitInterruptHandler(PUCHAR pTxBuffer, ULONG *pBuffLen)
{
    if (pTxBuffer && pBuffLen && *pBuffLen) { // If there is something for transmit.
        if ((m_DCB.fOutxCtsFlow && IsCTSOff()) ||(m_DCB.fOutxDsrFlow && IsDSROff())) { // We are in flow off
            DEBUGMSG(ZONE_THREAD|ZONE_WRITE,(TEXT("CPdd16550::XmitInterruptHandler! Flow Off, Data Discard.\r\n")));
            *pBuffLen = 0;
        }
    }
    if (m_lpSerialDataOut)
        m_lpSerialDataOut->XmitInterruptHandler(pTxBuffer,pBuffLen);
}
//
//  Modem
ULONG   UsbSerClientDriver::GetModemStatus() 
{
    ULONG ulModemStatus= MS_CTS_ON|MS_DSR_ON|MS_RING_ON|MS_RLSD_ON;
    if (m_lpUsbSerDataStatus) {
        ULONG ulEvent = 0;
#if ORIG
        ulModemStatus = m_lpUsbSerDataStatus->GetModemStatus(ulEvent);
#else
		// changing based on GPIO status
		ulModemStatus = xrusbserReadReg(UART_GPIO_STATUS, NON_URM_REG);	
		// map exar usb gpio values to 16550 uart specific MSR
		UCHAR temp = 0;
		if ((ulModemStatus & 0x01) != 0x01)
			temp |= 0x40;
		if ((ulModemStatus & 0x02) != 0x02)
			temp |= 0x80;
		if ((ulModemStatus & 0x04) != 0x04)
			temp |= 0x20;
		if ((ulModemStatus & 0x10) != 0x10)
			temp |= 0x10;

		ulModemStatus = temp;
#endif
        if (ulEvent!=0)
            EventCallback(ulEvent,ulModemStatus);
    }

    return ulModemStatus;
};

BOOL    UsbSerClientDriver::InitLine(BOOL bInit)
{
	//set flow control to none during initialization
	USHORT flow_mode, gpio_mode;
	flow_mode=0; 
	gpio_mode = 0;
	xrusbserWriteReg(UART_ENABLE, 0, NON_URM_REG);			// Disable TX and RX of that channel
	xrusbserWriteReg(UART_FLOW_CONTROL, (flow_mode & 0x07), NON_URM_REG);	
	xrusbserWriteReg(UART_GPIO_MODE, gpio_mode, NON_URM_REG);

	xrusbserWriteReg(UART_GPIO_DIRECTION, 0x28, NON_URM_REG);

	// enable the uart and reset tx/rx fifos - which are UART manager registers
	switch(currentportinterface)
	{
	case 0:
		xrusbserWriteReg(URM_FIFO_ENABLE_CHA, 1, URM_REG);	// Enable TX FIFO
		xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);			// Enable TX and RX of that channel
		xrusbserWriteReg(URM_FIFO_ENABLE_CHA, 3, URM_REG);	// Enable RX FIFO
		xrusbserWriteReg(URM_RX_FIFO_RESET_CHA, 1, URM_REG); // Writing a non-zero value to these registers resets the FIFOs.
		xrusbserWriteReg(URM_TX_FIFO_RESET_CHA, 1, URM_REG);
		break;
	case 1:
		xrusbserWriteReg(URM_FIFO_ENABLE_CHB, 1, URM_REG);
		xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
		xrusbserWriteReg(URM_FIFO_ENABLE_CHB, 3, URM_REG);
		xrusbserWriteReg(URM_RX_FIFO_RESET_CHB, 1, URM_REG);
		xrusbserWriteReg(URM_TX_FIFO_RESET_CHB, 1, URM_REG);
		break;
	case 2:
		xrusbserWriteReg(URM_FIFO_ENABLE_CHC, 1, URM_REG);
		xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
		xrusbserWriteReg(URM_FIFO_ENABLE_CHC, 3, URM_REG);
		xrusbserWriteReg(URM_RX_FIFO_RESET_CHC, 1, URM_REG);
		xrusbserWriteReg(URM_TX_FIFO_RESET_CHC, 1, URM_REG);
		break;
	case 3:
		xrusbserWriteReg(URM_FIFO_ENABLE_CHD, 1, URM_REG);
		xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
		xrusbserWriteReg(URM_FIFO_ENABLE_CHD, 3, URM_REG);
		xrusbserWriteReg(URM_RX_FIFO_RESET_CHD, 1, URM_REG);
		xrusbserWriteReg(URM_TX_FIFO_RESET_CHD, 1, URM_REG);
		break;
	default:
		DEBUGMSG(ZONE_INIT,(TEXT("Incorrect ChannelNumber in InitLine\n")));
		break;
	}

	//clear cts and dtr
	SetRTS(FALSE);
	SetDTR(FALSE);

    return TRUE;
}

BOOL UsbSerClientDriver::SetDCB(LPDCB lpDCB)
{
    BOOL bReturn = TRUE;
    if (IsOpen() && lpDCB && lpDCB->DCBlength == sizeof(DCB)) {
        
        m_HardwareLock.Lock();
        DCB LocalDCB = *lpDCB;
        if (lpDCB->BaudRate != m_DCB.BaudRate) {
            if (lpDCB->BaudRate == 0 || !SetBaudRate(lpDCB->BaudRate, m_fIREnable)) {
                LocalDCB.BaudRate = m_DCB.BaudRate ;
                bReturn = FALSE;
            }
        }
        if (lpDCB->ByteSize != m_DCB.ByteSize ) {
            if (!SetByteSize(lpDCB->ByteSize))  {
                LocalDCB.ByteSize = m_DCB.ByteSize;
                bReturn = FALSE;
            }
        }
        if (lpDCB->Parity != m_DCB.Parity) {
            if (!SetParity(lpDCB->Parity)) {
                LocalDCB.Parity = m_DCB.Parity ;
                bReturn=FALSE;
            }
        }
        if (lpDCB->StopBits != m_DCB.StopBits) {
            if (!SetStopBits(lpDCB->StopBits)) {
                LocalDCB.StopBits = m_DCB.StopBits;
                bReturn = FALSE;
            }
        }
        m_DCB = LocalDCB;
		m_HardwareLock.Unlock();        
// for flow control
		USHORT flow_mode, gpio_mode;
		if(LocalDCB.fRtsControl == RTS_CONTROL_HANDSHAKE)
		{
			flow_mode = 1;
			gpio_mode = 1;

			xrusbserWriteReg(UART_ENABLE, 0, NON_URM_REG);			// Disable TX and RX of that channel
			xrusbserWriteReg(UART_FLOW_CONTROL, (flow_mode & 0x07), NON_URM_REG);
			xrusbserWriteReg(UART_GPIO_MODE, gpio_mode, NON_URM_REG);

			switch(currentportinterface)
			{
			case 0:				
				xrusbserWriteReg(URM_FIFO_ENABLE_CHA, 1, URM_REG);	// Enable TX FIFO
				xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);			// Enable TX and RX of that channel
				xrusbserWriteReg(URM_FIFO_ENABLE_CHA, 3, URM_REG);	// Enable RX FIFO
				xrusbserWriteReg(URM_RX_FIFO_RESET_CHA, 1, URM_REG); // Writing a non-zero value to these registers resets the FIFOs.
				xrusbserWriteReg(URM_TX_FIFO_RESET_CHA, 1, URM_REG);
				break;
			case 1:
				xrusbserWriteReg(URM_FIFO_ENABLE_CHB, 1, URM_REG);
				xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
				xrusbserWriteReg(URM_FIFO_ENABLE_CHB, 3, URM_REG);
				xrusbserWriteReg(URM_RX_FIFO_RESET_CHB, 1, URM_REG);
				xrusbserWriteReg(URM_TX_FIFO_RESET_CHB, 1, URM_REG);
				break;
			case 2:
				xrusbserWriteReg(URM_FIFO_ENABLE_CHC, 1, URM_REG);
				xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
				xrusbserWriteReg(URM_FIFO_ENABLE_CHC, 3, URM_REG);
				xrusbserWriteReg(URM_RX_FIFO_RESET_CHC, 1, URM_REG);
				xrusbserWriteReg(URM_TX_FIFO_RESET_CHC, 1, URM_REG);
				break;
			case 3:
				xrusbserWriteReg(URM_FIFO_ENABLE_CHD, 1, URM_REG);
				xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
				xrusbserWriteReg(URM_FIFO_ENABLE_CHD, 3, URM_REG);
				xrusbserWriteReg(URM_RX_FIFO_RESET_CHD, 1, URM_REG);
				xrusbserWriteReg(URM_TX_FIFO_RESET_CHD, 1, URM_REG);
				break;
			default:
				DEBUGMSG(ZONE_INIT,(TEXT("Incorrect ChannelNumber in InitLine\n")));
				break;
			}
		}
		else if( LocalDCB.fOutX || LocalDCB.fInX )
		{
			flow_mode = 2;
			gpio_mode = 0;
			xrusbserWriteReg(UART_ENABLE, 0, NON_URM_REG);			// Disable TX and RX of that channel
			xrusbserWriteReg(UART_XON_CHAR, SERIAL_DEF_XON, NON_URM_REG);
			xrusbserWriteReg(UART_XOFF_CHAR, SERIAL_DEF_XOFF, NON_URM_REG);
			xrusbserWriteReg(UART_FLOW_CONTROL, (flow_mode & 0x07), NON_URM_REG);
			xrusbserWriteReg(UART_GPIO_MODE, gpio_mode, NON_URM_REG);

			switch(currentportinterface)
			{
			case 0:				
				xrusbserWriteReg(URM_FIFO_ENABLE_CHA, 1, URM_REG);	// Enable TX FIFO
				xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);			// Enable TX and RX of that channel
				xrusbserWriteReg(URM_FIFO_ENABLE_CHA, 3, URM_REG);	// Enable RX FIFO
				xrusbserWriteReg(URM_RX_FIFO_RESET_CHA, 1, URM_REG); // Writing a non-zero value to these registers resets the FIFOs.
				xrusbserWriteReg(URM_TX_FIFO_RESET_CHA, 1, URM_REG);
				break;
			case 1:
				xrusbserWriteReg(URM_FIFO_ENABLE_CHB, 1, URM_REG);
				xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
				xrusbserWriteReg(URM_FIFO_ENABLE_CHB, 3, URM_REG);
				xrusbserWriteReg(URM_RX_FIFO_RESET_CHB, 1, URM_REG);
				xrusbserWriteReg(URM_TX_FIFO_RESET_CHB, 1, URM_REG);
				break;
			case 2:
				xrusbserWriteReg(URM_FIFO_ENABLE_CHC, 1, URM_REG);
				xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
				xrusbserWriteReg(URM_FIFO_ENABLE_CHC, 3, URM_REG);
				xrusbserWriteReg(URM_RX_FIFO_RESET_CHC, 1, URM_REG);
				xrusbserWriteReg(URM_TX_FIFO_RESET_CHC, 1, URM_REG);
				break;
			case 3:
				xrusbserWriteReg(URM_FIFO_ENABLE_CHD, 1, URM_REG);
				xrusbserWriteReg(UART_ENABLE, 3, NON_URM_REG);
				xrusbserWriteReg(URM_FIFO_ENABLE_CHD, 3, URM_REG);
				xrusbserWriteReg(URM_RX_FIFO_RESET_CHD, 1, URM_REG);
				xrusbserWriteReg(URM_TX_FIFO_RESET_CHD, 1, URM_REG);
				break;
			default:
				DEBUGMSG(ZONE_INIT,(TEXT("Incorrect ChannelNumber in InitLine\n")));
				break;
			}
		}
		else
		{
			// not needed as it is already set in InitLine
//			flow_mode=0; 
//			gpio_mode = 0;
//			xrusbserWriteReg(UART_FLOW_CONTROL, (flow_mode & 0x07), NON_URM_REG);	
//			xrusbserWriteReg(UART_GPIO_MODE, gpio_mode, NON_URM_REG);
		}
// 
    }
    else
        bReturn = FALSE;
    return bReturn;
};


void UsbSerClientDriver::SetBreak(BOOL bSet)
{
    BOOL bReturn;
	if(bSet)
		bReturn = xrusbserWriteReg(UART_TX_BREAK, 0x01, NON_URM_REG); // turn-on
	else
		bReturn = xrusbserWriteReg(UART_TX_BREAK, 0x00, NON_URM_REG); // turn-off
	ASSERT(bReturn==TRUE);
}

void    UsbSerClientDriver::SetDTR(BOOL bSet)
{
	BOOL bReturn;
    if(bSet)
		bReturn = xrusbserWriteReg(UART_GPIO_CLR, 0x08, NON_URM_REG);
	else
		bReturn = xrusbserWriteReg(UART_GPIO_SET, 0x08, NON_URM_REG);
	ASSERT(bReturn==TRUE);
}

void    UsbSerClientDriver::SetRTS(BOOL bSet)
{
	BOOL bReturn;
	if(bSet)
		bReturn = xrusbserWriteReg(UART_GPIO_CLR, 0x20, NON_URM_REG);		
	else
		bReturn = xrusbserWriteReg(UART_GPIO_SET, 0x20, NON_URM_REG);
	ASSERT(bReturn==TRUE);
}

BOOL     UsbSerClientDriver::SetBaudRate(ULONG BaudRate,BOOL bIrModule)
{   
	ULONG divisor, tx_mask, rx_mask, D32, indx;
	divisor = 48000000 / BaudRate;

	D32  = (48000000 * 32) / BaudRate;
	indx = D32 % 32;

	tx_mask = xr141x_txrx_mask_lookuptable[indx].tx_mask;
	if(divisor % 2)
		rx_mask = xr141x_txrx_mask_lookuptable[indx].rx_mask_even;
	else
		rx_mask = xr141x_txrx_mask_lookuptable[indx].rx_mask_odd;

	m_HardwareLock.Lock();
    m_UsbCommLineCoding.DTERate = BaudRate;
    m_DCB.BaudRate = BaudRate;
	m_HardwareLock.Unlock();

	BOOL bReturn;	
	bReturn = xrusbserWriteReg(UART_CLOCK_DIVISOR0, (USHORT)(divisor & 0xFF), NON_URM_REG);
	bReturn = xrusbserWriteReg(UART_CLOCK_DIVISOR1, (USHORT)((divisor>>8) & 0xFF), NON_URM_REG);
	bReturn = xrusbserWriteReg(UART_CLOCK_DIVISOR2, (USHORT)((divisor>>16) & 0xFF), NON_URM_REG);
	bReturn = xrusbserWriteReg(UART_TX_CLOCK_MASK0, (USHORT)(tx_mask & 0xFF), NON_URM_REG);
	bReturn = xrusbserWriteReg(UART_TX_CLOCK_MASK1, (USHORT)((tx_mask>>8) & 0xFF), NON_URM_REG);
	bReturn = xrusbserWriteReg(UART_RX_CLOCK_MASK0, (USHORT)(rx_mask & 0xFF), NON_URM_REG);
	bReturn = xrusbserWriteReg(UART_RX_CLOCK_MASK1, (USHORT)((rx_mask>>8) & 0xFF), NON_URM_REG);
	ASSERT(bReturn==TRUE);
    return bReturn;
}

BOOL    UsbSerClientDriver::SetCharFormat()
{
	USHORT Value=0,data_bit=0,parity=0,stop_bits=0;

	if(m_UsbCommLineCoding.CharFormat > 6 && m_UsbCommLineCoding.CharFormat < 10) // we support only 7-8-9
	{
		data_bit = m_UsbCommLineCoding.CharFormat;
	}
	else
	{
		return FALSE;
	}

	switch (m_UsbCommLineCoding.ParityType)
	{
        case USB_COMM_PARITY_NONE:
			parity = 0x00;
            break;
        case USB_COMM_PARITY_ODD:
			parity = 0x01;
            break;
		case USB_COMM_PARITY_EVEN:
			parity = 0x02;
            break;
		case USB_COMM_PARITY_MARK:
			parity = 0x03;
            break;
        case USB_COMM_PARITY_SPACE:
			parity = 0x04;
            break;        
        default:
            return FALSE;
	}

	switch (m_UsbCommLineCoding.DataBits)
	{
        case USB_COMM_STOPBITS_10:
			stop_bits = 0x00;
            break;
        case USB_COMM_STOPBITS_20:
			stop_bits = 0x01;
            break;
        default:
			return FALSE;
    }

	BOOL bReturn = xrusbserWriteReg(UART_CHAR_FORMAT, (stop_bits<<7)|(parity<<4)|(data_bit & 0x0F), NON_URM_REG);
	ASSERT(bReturn==TRUE);
	return bReturn;
}

BOOL    UsbSerClientDriver::SetByteSize(ULONG ByteSize)
{
    m_HardwareLock.Lock();  
    m_UsbCommLineCoding.CharFormat = (BYTE)ByteSize;
    m_DCB.ByteSize = (BYTE)ByteSize;
	m_HardwareLock.Unlock();

	BOOL bReturn = SetCharFormat();
	return bReturn;
}
BOOL    UsbSerClientDriver::SetParity(ULONG Parity)
{
    m_HardwareLock.Lock();  
    m_UsbCommLineCoding.ParityType = (BYTE)Parity;
    m_DCB.Parity = (BYTE)Parity;
    m_HardwareLock.Unlock();

	BOOL bReturn = SetCharFormat();
	return bReturn;
}
BOOL    UsbSerClientDriver::SetStopBits(ULONG StopBits) 
{
    m_HardwareLock.Lock();  
    m_UsbCommLineCoding.DataBits = (BYTE)StopBits;
    m_DCB.StopBits = (BYTE)StopBits;
    m_HardwareLock.Unlock();

    BOOL bReturn = SetCharFormat();
	return bReturn;
}

BOOL	UsbSerClientDriver::Ioctl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
	//
	// IOCTL_POWER_CAPABILITIES and IOCTL_POWER_SET are just copy/pasted/inherited from CSerialPDD::Ioctl function
	//
    BOOL RetVal=TRUE;
    switch ( dwCode ) {
    case IOCTL_POWER_CAPABILITIES: 
        if (!pBufOut || dwLenOut < sizeof(POWER_CAPABILITIES) || !pdwActualOut) {
            SetLastError(ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
        } else {
            m_IsThisPowerManaged= TRUE;
            PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pBufOut;        
            *ppc= GetPowerCapabilities();
            if (pdwActualOut)
                *pdwActualOut = sizeof(POWER_CAPABILITIES);
            RetVal = TRUE;
        }
        break;
    case IOCTL_POWER_SET:
        if (!pBufOut || dwLenOut < sizeof(CEDEVICE_POWER_STATE) || !pdwActualOut || m_PowerHelperHandle == INVALID_HANDLE_VALUE ) {
            SetLastError(ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
        } else {
            m_IsThisPowerManaged= TRUE;
            CEDEVICE_POWER_STATE newDx = *(PCEDEVICE_POWER_STATE) pBufOut;
            DEBUGMSG(1, (TEXT("COM: IOCTL_POWER_SET: D%d\r\n"), newDx));
            RetVal = DDKPwr_SetDeviceLevel( m_PowerHelperHandle, newDx, NULL );
            // did we set the device power?
            if(RetVal == TRUE) {
                *(PCEDEVICE_POWER_STATE)pBufOut = DDKPwr_GetDeviceLevel(m_PowerHelperHandle );
                if (pdwActualOut) {
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                }
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
        }
        break;
    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        RetVal = FALSE;
        break;
    }
    return RetVal;
}

BOOL    UsbSerClientDriver::xrusbserWriteReg(UCHAR Reg, USHORT Value, BOOLEAN blockURM)
{
    USB_DEVICE_REQUEST usbRequest;
	UCHAR block;
	
	usbRequest.bmRequestType = USB_REQUEST_VENDOR | USB_REQUEST_HOST_TO_DEVICE;
	usbRequest.bRequest = XR_SET_REG;
	usbRequest.wLength = 0;
	if(blockURM) // if this is for URM (UART Manager Register)
		block = 4;
	else
		block = currentportinterface;
	usbRequest.wIndex = (block<<8) | Reg;	
	usbRequest.wValue = Value;	

	BOOL bReturn = m_lpUsbClientDevice->IssueVendorTransfer(USB_OUT_TRANSFER, &usbRequest, NULL, NULL);
    ASSERT(bReturn==TRUE);
	m_lpUsbClientDevice->CloseTransfer();
    return bReturn;
}

UCHAR    UsbSerClientDriver::xrusbserReadReg(UCHAR Reg, BOOLEAN blockURM)
{
    USB_DEVICE_REQUEST usbRequest;
	UCHAR block;
	UCHAR ucValue;
	
	usbRequest.bmRequestType = USB_REQUEST_VENDOR | USB_REQUEST_DEVICE_TO_HOST;
	usbRequest.bRequest = XR_GETN_REG;
	usbRequest.wLength = sizeof(USHORT);
	if(blockURM) // if this is for URM (UART Manager Register)
		block = 4;
	else
		block = currentportinterface;
	usbRequest.wIndex = (block<<8) | Reg;
	ucValue = 0; // just in case

	BOOL bReturn = m_lpUsbClientDevice->IssueVendorTransfer(USB_IN_TRANSFER, &usbRequest, &ucValue, NULL);
    ASSERT(bReturn==TRUE);
	m_lpUsbClientDevice->CloseTransfer();
    return ucValue;
}

CSerialPDD * CreateSerialObject(LPTSTR lpActivePath, PVOID pMdd,PHWOBJ pHwObj, DWORD DeviceArrayIndex)
{
    DEBUGMSG(ZONE_INIT,(TEXT("+CreateSerialObject, DeviceArrayIndex=%d"),DeviceArrayIndex));
    CSerialPDD * pSerialPDD = NULL;

	// some of the code here is removedas it was for asyncmodem

	if (DeviceArrayIndex == 0) {
		pSerialPDD= new UsbSerClientDriver(lpActivePath,pMdd, pHwObj);
        if (pSerialPDD && !pSerialPDD->Init()) {
            delete pSerialPDD;
            pSerialPDD = NULL;
        }
    }
    DEBUGMSG(ZONE_INIT,(TEXT("-CreateSerialObject, pSerialPDD=%x"),pSerialPDD));
    return pSerialPDD;
}
void DeleteSerialObject(CSerialPDD * pSerialPDD)
{
    if (pSerialPDD)
        delete pSerialPDD;
}

