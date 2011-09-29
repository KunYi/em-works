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
#define GPIO_IOCTL_OUT_ENABLE					CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3900, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define GPIO_IOCTL_OUT_DISABLE				CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3901, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define GPIO_IOCTL_OUT_SET						CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3902, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define GPIO_IOCTL_OUT_CLEAR					CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3903, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define GPIO_IOCTL_PIN_STATE					CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3904, METHOD_BUFFERED, FILE_ANY_ACCESS)  

//
// ISA IO Control Codes
//
#define ISA_IOCTL_READ_WRITE					CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3910, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define ISA_IOCTL_BUS_RESET						CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3911, METHOD_BUFFERED, FILE_ANY_ACCESS)  
//#define ISA_IOCTL_REDA_CS1						CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3912, METHOD_BUFFERED, FILE_ANY_ACCESS)  
//#define ISA_IOCTL_WRITE_CS1						CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3913, METHOD_BUFFERED, FILE_ANY_ACCESS)  

typedef struct
{
	DWORD dwCmd;		// = 0: Read, = 1: Write
	DWORD	dwSeg;		// = 0: ISA_CS0, = 1: ISA_CS1
	DWORD dwOffset;
	DWORD	dwValue;		// only lower byte valid
} ISA_BUS_ACCESS, *PISA_BUS_ACCESS;

//
// CS&ZHL JUN-24-2011: cmd code for KernelIoControl
//
#define IOCTL_HAL_BOARDINFO_READ			CTL_CODE(FILE_DEVICE_HAL, 4020, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_TIMESTAMP_READ			CTL_CODE(FILE_DEVICE_HAL, 4021, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_VENDOR_ID_READ			CTL_CODE(FILE_DEVICE_HAL, 4022, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_CUSTOMER_ID_READ		CTL_CODE(FILE_DEVICE_HAL, 4023, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_BOARD_STATE_READ		CTL_CODE(FILE_DEVICE_HAL, 4024, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_WATCHDOG_GET				CTL_CODE(FILE_DEVICE_HAL, 4025, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// CS&ZHL JLY-1-2011: create unique system I/O control codes (IOCTL) for ISA_IRQ
//
#define IOCTL_WAIT_FOR_IRQ				CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3920, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_EOI						CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3921, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif		//__BSP_DRIVERS_H