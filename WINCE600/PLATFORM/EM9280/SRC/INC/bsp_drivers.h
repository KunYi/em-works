//------------------------------------------------------------------------------
//
//  File:  bsp_dirvers.h
//
//------------------------------------------------------------------------------
#ifndef __BSP_DRIVERS_H
#define __BSP_DRIVERS_H


#define	GPIO0		(1 <<  0)
#define	GPIO1		(1 <<  1)
#define	GPIO2		(1 <<  2)
#define	GPIO3		(1 <<  3)
#define	GPIO4		(1 <<  4)
#define	GPIO5		(1 <<  5)
#define	GPIO6		(1 <<  6)
#define	GPIO7		(1 <<  7)
#define	GPIO8		(1 <<  8)
#define	GPIO9		(1 <<  9)
#define	GPIO10		(1 << 10)
#define	GPIO11		(1 << 11)
#define	GPIO12		(1 << 12)
#define	GPIO13		(1 << 13)
#define	GPIO14		(1 << 14)
#define	GPIO15		(1 << 15)
#define	GPIO16		(1 << 16)
#define	GPIO17		(1 << 17)
#define	GPIO18		(1 << 18)
#define	GPIO19		(1 << 19)
#define	GPIO20		(1 << 20)
#define	GPIO21		(1 << 21)
#define	GPIO22		(1 << 22)
#define	GPIO23		(1 << 23)
#define	GPIO24		(1 << 24)
#define	GPIO25		(1 << 25)
#define	GPIO26		(1 << 26)
#define	GPIO27		(1 << 27)
#define	GPIO28		(1 << 28)
#define	GPIO29		(1 << 29)
#define	GPIO30		(1 << 30)
#define	GPIO31		(1 << 31)


//
// GPIO IO Control Codes
//
#define GPIO_IOCTL_OUT_ENABLE			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3900, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define GPIO_IOCTL_OUT_DISABLE			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3901, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define GPIO_IOCTL_OUT_SET				CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3902, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define GPIO_IOCTL_OUT_CLEAR			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3903, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define GPIO_IOCTL_PIN_STATE			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3904, METHOD_BUFFERED, FILE_ANY_ACCESS)  

//
// CS&ZHL JLY-1-2011: create unique system I/O control codes (IOCTL) for ISA_IRQ
//
#define IOCTL_WAIT_FOR_IRQ				CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3920, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_EOI					CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3921, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// CS&ZHL MAY-31-2012: assign GPIO pin as RTS
//
#define IOCTL_SET_UART_RTS_PIN			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3924, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// CS&ZHL JUN-24-2011: cmd code for KernelIoControl
//
#define IOCTL_HAL_BOARDINFO_READ		CTL_CODE(FILE_DEVICE_HAL, 4020, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_TIMESTAMP_READ		CTL_CODE(FILE_DEVICE_HAL, 4021, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_VENDOR_ID_READ		CTL_CODE(FILE_DEVICE_HAL, 4022, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_CUSTOMER_ID_READ		CTL_CODE(FILE_DEVICE_HAL, 4023, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_BOARD_STATE_READ		CTL_CODE(FILE_DEVICE_HAL, 4024, METHOD_BUFFERED, FILE_ANY_ACCESS)

//--------------------------------------------------------------------------------------------------------
// CS&ZHL MAY-23-2012: definitions for AD in EM9280
//
#define	EM9280_DAQ_VOLTAGE_CH0			0
#define	EM9280_DAQ_VOLTAGE_CH1			1
#define	EM9280_DAQ_VDD_5V				2
#define	EM9280_DAQ_VDDIO_3V3			3
#define	EM9280_DAQ_VDDA_1V8				4
#define	EM9280_DAQ_VDDD_1V45			5
#define	EM9280_DAQ_CPU_TEMPERATURE		6
#define	EM9280_DAQ_BOARD_TEMPERATURE	7

typedef struct
{
	DWORD		dwCmd;			// = 0, 1, 2, ....
	DWORD		dwData;			// data value
	char		UnitName[16];	// return unit string: "Voltage", "Kalvin"
} DAQ_INFO, *PDAQ_INFO;
//
// end of CS&ZHL MAY-23-2012: definitions for AD in EM9280
//--------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------
// CS&ZHL JUN-12-2012: definitions for PWM in EM9280
//
typedef struct
{
	DWORD	dwFreq;			// in Hz, = 0: stop pwm output
	DWORD	dwDuty;			// in %, 1 - 99
	DWORD   dwResolution;	// = 1: unit; = 10: 0.1 unit; = 100: 0.01 unit
} PWM_INFO, *PPWM_INFO;

//--------------------------------------------------------------------------------------------------------
// CS&ZHL JUN-14-2012: definitions for I2C in EM9280
//
typedef struct
{
	BYTE	uHwAddr;		// 7-bit slave hardware address + 1-bit R/W flag in D0(LSB)
	DWORD	dwCmd;			// = 0xFFFFFFFF: invalid cmd, will be ignored
							// dwCmd.D31 = 0: single-byte cmd
							// dwCmd.D31 = 1: double-byte cmd, NOTE: hi-byte send first!
	PBYTE	pDatBuf;		// data buffer
	DWORD   dwDatLen;		// data length in byte
} I2C_INFO, *PI2C_INFO;

//--------------------------------------------------------------------------------------------------------
// zxw 2012-06-07 definitions for SPI in EM9280
//
typedef struct
{
	PBYTE  pDatBuf;			// data buffer
	DWORD  dwDatLen;		// data length in byte 
	BYTE   BitCount;		// SPI Transfer Data Bit Format = 8, 16
	BOOL   bLastTime;		// set when the last SPI transfer
}SPI_INFO , *PSPI_INFO;

#endif		//__BSP_DRIVERS_H