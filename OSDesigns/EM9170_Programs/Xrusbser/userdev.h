/*++
Module Name:  SerDev.h

Abstract: USB Host Serial Client driver.

Notes: 
--*/

#ifndef __USERDEV_H_
#define __USERDEV_H_
#include <UsbSer.h>
#include <Cserpdd.h>
class UsbSerClientDriver;
// Control Line State - sent to device on default control pipe
#define USB_COMM_DTR    0x0001
#define USB_COMM_RTS    0x0002    

// Abstract Control Model defines
#define USB_COMM_SEND_ENCAPSULATED_COMMAND		0x0000
#define USB_COMM_GET_ENCAPSULATED_RESPONSE		0x0001
#define USB_COMM_SET_COMM_FEATURE				0x0002
#define USB_COMM_GET_COMM_FEATURE				0x0003
#define USB_COMM_CLEAR_COMM_FEATURE				0x0004
#define USB_COMM_SET_LINE_CODING				0x0020
#define USB_COMM_GET_LINE_CODING				0x0021
#define USB_COMM_SET_CONTROL_LINE_STATE			0x0022
#define USB_COMM_SEND_BREAK						0x0023

// Line Coding Stop Bits
#define USB_COMM_STOPBITS_10					0x0000
#define USB_COMM_STOPBITS_15					0x0001
#define USB_COMM_STOPBITS_20					0x0002

// Line Coding Parity Type
#define USB_COMM_PARITY_NONE					0x0000
#define USB_COMM_PARITY_ODD						0x0001
#define USB_COMM_PARITY_EVEN					0x0002
#define USB_COMM_PARITY_MARK					0x0003
#define USB_COMM_PARITY_SPACE					0x0004

typedef struct _USB_COMM_LINE_CODING
{
    ULONG       DTERate;
    UCHAR       CharFormat;
    UCHAR       ParityType;
    UCHAR       DataBits;
} USB_COMM_LINE_CODING, *PUSB_COMM_LINE_CODING;

struct xr141x_txrx_mask_table
{
  ULONG indx;
  ULONG tx_mask;
  ULONG rx_mask_even;
  ULONG rx_mask_odd;
};

struct xr141x_txrx_mask_table xr141x_txrx_mask_lookuptable[] =
{
	{	0,	0x000,	0x000,	0x000	},
	{	1,	0x000,	0x000,	0x000	},
	{	2,	0x100,	0x000,	0x100	},
	{	3,	0x020,	0x400,	0x020	},
	{	4,	0x010,	0x100,	0x010	},
	{	5,	0x208,	0x040,	0x208	},
	{	6,	0x104,	0x820,	0x108	},
	{	7,	0x844,	0x210,	0x884	},
	{	8,	0x444,	0x110,	0x444	},
	{	9,	0x122,	0x888,	0x224	},
	{	10,	0x912,	0x448,	0x924	},
	{	11,	0x492,	0x248,	0x492	},
	{	12,	0x252,	0x928,	0x292	},
	{	13,	0x94A,	0x4A4,	0xA52	},
	{	14,	0x52A,	0xAA4,	0x54A	},
	{	15,	0xAAA,	0x954,	0x4AA	},
	{	16,	0xAAA,	0x554,	0xAAA	},
	{	17,	0x555,	0xAD4,	0x5AA	},
	{	18,	0xB55,	0xAB4,	0x55A	},
	{	19,	0x6B5,	0x5AC,	0xB56	},
	{	20,	0x5B5,	0xD6C,	0x6D6	},
	{	21,	0xB6D,	0xB6A,	0xDB6	},
	{	22,	0x76D,	0x6DA,	0xBB6	},
	{	23,	0xEDD,	0xDDA,	0x76E	},
	{	24,	0xDDD,	0xBBA,	0xEEE	},
	{	25,	0x7BB,	0xF7A,	0xDDE	},
	{	26,	0xF7B,	0xEF6,	0x7DE	},
	{	27,	0xDF7,	0xBF6,	0xF7E	},
	{	28,	0x7F7,	0xFEE,	0xEFE	},
	{	29,	0xFDF,	0xFBE,	0x7FE	},
	{	30,	0xF7F,	0xEFE,	0xFFE	},
	{	31,	0xFFF,	0xFFE,	0xFFD	},
};

#define XR_SET_REG              0
#define XR_GETN_REG             1

#define URM_REG TRUE
#define NON_URM_REG FALSE

// XR21V141x register set

#define UART_ENABLE			0x03

#define UART_CLOCK_DIVISOR0	0x04
#define UART_CLOCK_DIVISOR1	0x05
#define UART_CLOCK_DIVISOR2	0x06
#define UART_TX_CLOCK_MASK0	0x07
#define UART_TX_CLOCK_MASK1	0x08
#define UART_RX_CLOCK_MASK0	0x09
#define UART_RX_CLOCK_MASK1	0x0A

#define UART_CHAR_FORMAT	0x0B
#define UART_FLOW_CONTROL	0x0C

#define UART_XON_CHAR		0x10
#define UART_XOFF_CHAR		0x11

#define UART_ERROR_STATUS	0x13
#define UART_TX_BREAK		0x14
#define UART_XCVR_EN_DELAY	0x15

#define UART_GPIO_MODE		0x1A
#define UART_GPIO_DIRECTION	0x1B
#define UART_GPIO_SET		0x1D
#define UART_GPIO_CLR		0x1E
#define UART_GPIO_STATUS	0x1F

#define URM_FIFO_ENABLE_CHA		0x10
#define URM_FIFO_ENABLE_CHB		0x11
#define URM_FIFO_ENABLE_CHC		0x12
#define URM_FIFO_ENABLE_CHD		0x13
#define URM_RX_FIFO_RESET_CHA	0x18
#define URM_RX_FIFO_RESET_CHB	0x19
#define URM_RX_FIFO_RESET_CHC	0x1A
#define URM_RX_FIFO_RESET_CHD	0x1B
#define URM_TX_FIFO_RESET_CHA	0x1C
#define URM_TX_FIFO_RESET_CHB	0x1D
#define URM_TX_FIFO_RESET_CHC	0x1E
#define URM_TX_FIFO_RESET_CHD	0x1F

//
// Default xon/xoff characters.
//
#define SERIAL_DEF_XON 0x11
#define SERIAL_DEF_XOFF 0x13

// XR USB UART specific IOCTL codes and the related function ptototypes are 
// defined here 
// the IOCTL index starts from 50 (in pegdser.h the last ioctl index ends at 25)
#define IOCTL_XRUSBUART_ENABLE_HWFLOWCONTROL 		CTL_CODE(FILE_DEVICE_SERIAL_PORT,50,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_XRUSBUART_DISABLE_HWFLOWCONTROL 		CTL_CODE(FILE_DEVICE_SERIAL_PORT,51,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_XRUSBUART_ENABLE_SWFLOWCONTROL 		CTL_CODE(FILE_DEVICE_SERIAL_PORT,52,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_XRUSBUART_DISABLE_SWFLOWCONTROL 		CTL_CODE(FILE_DEVICE_SERIAL_PORT,53,METHOD_BUFFERED,FILE_ANY_ACCESS)

class UsbSerClientDriver: public CSerialPDD {
public:
    UsbSerClientDriver(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj);
    ~UsbSerClientDriver();
// For serial driver we have to do this.
    virtual BOOL Init();
    virtual void  PreDeinit();
protected:
    virtual BOOL CreateBulkIn(LPCUSB_ENDPOINT lpEndpoint);
    virtual BOOL CreateBulkOut(LPCUSB_ENDPOINT lpEndpoint);
    virtual BOOL CreateStatusIn(LPCUSB_ENDPOINT lpEndpoint);
public:
    virtual void    SerialRegisterBackup() {;};
    virtual void    SerialRegisterRestore() {;};
//  Tx Function.
    virtual BOOL    InitXmit(BOOL bInit) {
        if (m_lpSerialDataOut)
            return m_lpSerialDataOut->InitXmit(bInit);
        else
            return FALSE;
    }
    virtual void    XmitInterruptHandler(PUCHAR pTxBuffer, ULONG *pBuffLen) ;
    virtual void    XmitComChar(UCHAR ComChar)  {
        if (m_lpSerialDataOut)
            m_lpSerialDataOut->XmitComChar(ComChar);
    }
    virtual BOOL    EnableXmitInterrupt(BOOL bEnable) { return TRUE; };
    virtual BOOL    CancelXmit()  {
        if (m_lpSerialDataOut)
            m_lpSerialDataOut->CancelXmit();
        return TRUE;
    }
//
//  Rx Function.
    virtual BOOL    InitReceive(BOOL bInit) {
        if (m_lpSerialDataIn)
            return m_lpSerialDataIn->InitReceive(bInit);
        else
            return FALSE;
    }
    virtual ULONG   ReceiveInterruptHandler(PUCHAR pRxBuffer,ULONG *pBufflen) {
        DWORD dwReturn = 0;
        if (!(m_lpSerialDataIn && m_lpSerialDataIn->ReceiveInterruptHandler(pRxBuffer,pBufflen))) {
            if (pBufflen)
                dwReturn = *pBufflen;
        }
        return dwReturn;
    }
    virtual ULONG   CancelReceive() {
        if (m_lpSerialDataIn)
            m_lpSerialDataIn->CancelReceive();
        return 0;
    }
//
//  Modem
    virtual BOOL    InitModem(BOOL bInit)  {
        if (m_lpUsbSerDataStatus)
            return m_lpUsbSerDataStatus->InitModem(bInit);
        return FALSE;
    }
    virtual void    ModemInterruptHandler() { GetModemStatus();}
    virtual ULONG   GetModemStatus() ;
    void    SetDTR(BOOL bSet);
    void    SetRTS(BOOL bSet);
//  Line Function
    virtual BOOL    InitLine(BOOL bInit); // { return TRUE; }; // this function is called during COM_Open and we have stuff to do.-rkr
    virtual void    LineInterruptHandler() { };
    virtual void    SetBreak(BOOL bSet);
    virtual BOOL    SetBaudRate(ULONG BaudRate,BOOL bIrModule);
    virtual BOOL    SetByteSize(ULONG ByteSize) ;
    virtual BOOL    SetParity(ULONG Parity) ;
    virtual BOOL    SetStopBits(ULONG StopBits) ;
	virtual BOOL	Ioctl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);
	virtual BOOL    SetDCB(LPDCB lpDCB);
protected:
    virtual DWORD DataInNotify() {
        return NotifyPDDInterrupt(INTR_RX);
    }
    virtual DWORD DataOutNotify() {
        return NotifyPDDInterrupt(INTR_TX);
    }
    virtual DWORD StatusNotify() {
        return NotifyPDDInterrupt(INTR_MODEM);
    }
    SerialUsbClientDevice *   m_lpUsbClientDevice;
    SerialDataIn  *     m_lpSerialDataIn;
    SerialDataOut *     m_lpSerialDataOut;
    WORD m_wSetModemStatus;
    USB_COMM_LINE_CODING m_UsbCommLineCoding;
private:
    UsbSerDataStatus *  m_lpUsbSerDataStatus;
    BOOL GetLineCodeing;
    BOOL SetModemStatus;
    static DWORD WINAPI DataInStub(LPVOID lpvNotifyParameter) {
        return ((UsbSerClientDriver *)lpvNotifyParameter)->DataInNotify();
    }
    static DWORD WINAPI DataOutStub(LPVOID lpvNotifyParameter) {
        return ((UsbSerClientDriver *)lpvNotifyParameter)->DataOutNotify();
    }
    static DWORD WINAPI StatusStub(LPVOID lpvNotifyParameter) {
        return ((UsbSerClientDriver *)lpvNotifyParameter)->StatusNotify();
    }

	int currentportinterface;
	BOOL xrusbserWriteReg(UCHAR Reg, USHORT Value, BOOLEAN blockURM);
	UCHAR xrusbserReadReg(UCHAR Reg, BOOLEAN blockURM);
	BOOL SetCharFormat();
};


#define SET_CONTROL_LINE_STATE  0x22

#endif

