//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  flex_can.h
//
//  Header file for CAN bus driver.
//
//------------------------------------------------------------------------------
#ifndef __FLEX_CAN_H__
#define __FLEX_CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define CAN1_FID                         _T("CAN1:")
#define CAN2_FID                         _T("CAN2:")

#define CAN_NO_ERROR									(0)         // Last operation successful
#define CAN_ERR_MOPS_CREATE						(-1)        // Mutex Creation failed
#define CAN_ERR_PA_VA_MISSING					(-2)        // Physical -> Virtual Mapping failed
#define CAN_ERR_EOPS_CREATE						(-3)        // Event Creation failed
#define CAN_ERR_IRQ_SYSINTR_MISSING			(-4)        // IRQ -> System Interrupt ID Mapping failed
#define CAN_ERR_INT_INIT								(-5)        // Interrupt Initialization failed
#define CAN_ERR_WORKER_THREAD					(-6)        // Worker thread failed
#define CAN_ERR_NO_ACK_ISSUED					(-7)        // No Acknowledge Issued
#define CAN_ERR_NULL_BUF							(-8)        // NULL Buffer
#define CAN_ERR_INVALID_BUFSIZE					(-9)        // Invalid Buffer Size
#define CAN_ERR_NULL_LPIRESULT					(-10)       // NULL lpiResult field
#define CAN_ERR_CLOCK_FAILURE					(-11)       // CRM Operation Failure
#define CAN_ERR_TRANSFER_TIMEOUT				(-12)       // CAN transmit timeout error
#define CAN_ERR_ARBITRATION_LOST				(-13)       // CAN arbitration lost error
#define CAN_ERR_TRANSFER_ERR						(-14)       // CAN goto an undefined error
#define CAN_ERR_BIT0										(-15)       // At least one bit sent as dominant is received as dominant
#define CAN_ERR_BIT1										(-16)       // CANAt At least one bit sent as dominant is received as recessive
#define CAN_ERR_ACK										(-17)       // CAN Acknowledge Error has been detected by the transmitter node
#define CAN_ERR_CRC										(-18)       // CRC Error has been detected by the receiver node
#define CAN_ERR_FRM										(-19)       // CAN Form Error has been detected by the receiver node
#define CAN_ERR_STF										(-20)       // CAN Stuffing Error has been detected

#define CAN_CTRL_BIT_TIMING_1MHZ				0x01790000
#define CAN_CTRL_BIT_TIMING_800KHZ			0x01790003
#define CAN_CTRL_BIT_TIMING_500KHZ			0x02790004
#define CAN_CTRL_BIT_TIMING_250KHZ			0x05790004
#define CAN_CTRL_BIT_TIMING_125KHZ			0x0B790004
#define CAN_CTRL_BIT_TIMING_100KHZ			0x0E790004
#define CAN_CTRL_BIT_TIMING_60KHZ			0x18790084
#define CAN_CTRL_BIT_TIMING_50KHZ			0x1D790084
#define CAN_CTRL_BIT_TIMING_20KHZ			0x4A790084
#define CAN_CTRL_BIT_TIMING_10KHZ			0x95790084

#define CAN_MB_CS_CODE_MASK					(0xF << 24)
#define CAN_MB_CS_CODE_RX_INACTIVE		(0x0 << 24)
#define CAN_MB_CS_CODE_RX_FULL				(0x2 << 24)
#define CAN_MB_CS_CODE_RX_EMPTY				(0x4 << 24)
#define CAN_MB_CS_CODE_RX_OVERRUN		(0x6 << 24)
#define CAN_MB_CS_CODE_TX_INACTIVE			(0x8 << 24)
#define CAN_MB_CS_CODE_TX_ABORT			(0x9 << 24)
#define CAN_MB_CS_CODE_TX_SEND				(0xC << 24)


// IO Control Codes
//
// Developers are encourage to use the macros listed below to access the driver
// capabilities.
// All the IOCTL codes provided here has a macro equivalent. The details for each
// IOCTL will be explained in the corresponding macros.
//

#define CAN_IOCTL_START					        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3804, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CAN_IOCTL_STOP								CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3805, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CAN_IOCTL_RESET							CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3806, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define CAN_IOCTL_SET_BAUD_RATE	        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3807, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CAN_IOCTL_SET_FILTER					CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3808, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define CAN_IOCTL_ENABLE_SYNC				CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3809, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define CAN_IOCTL_SELF_TEST					CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3810, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CAN_IOCTL_GET_STATISTICS			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3811, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define CAN_IOCTL_WAIT_EVENT					CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3812, METHOD_BUFFERED, FILE_ANY_ACCESS) 

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//
// CS&ZHL July 26-2011: define can frame witch according FlexCAN Message Buffer
//
typedef struct
{
	DWORD		dwType;			// = 0：标准帧；= 1：扩展帧
	DWORD		dwID;				// 标准帧：11-bit；扩展帧：29-bit
	DWORD		dwRTR;				// = 0：数据帧；= 1：远程帧
	DWORD		dwPrio;				// 发送优先级 = 0 - 7，0为最高优先级，对接收数据包无意义
	DWORD 	dwDatLen;		// 数据长度 = 0 - 8
	UCHAR	    ucDat[8];			// 数据字节
}CAN_PACKET, *PCAN_PACKET;

#define	CAN_PACKET_TYPE_STANDARD		0
#define	CAN_PACKET_TYPE_EXTENDED		1

typedef struct
{
	DWORD		dwGroup;			// 组号，= 0，1，2
	DWORD		dwType;			// = 0：标准帧；= 1：扩展帧
	DWORD		dwID;				// 标准帧：11-bit；扩展帧：29-bit
	DWORD		dwRTR;				// = 0：数据帧；= 1：远程帧
	DWORD		dwMask;			// 各位与dwID对应，= 0：该位不检查；= 1：该位须检查
}CAN_FILTER, *PCAN_FILTER;


typedef struct
{
	DWORD		dwType;			// = 0：标准帧；= 1：扩展帧
	DWORD		dwID;				// 标准帧：11-bit；扩展帧：29-bit
	DWORD		dwRTR;				// = 0：数据帧；= 1：远程帧
}CAN_SYNC, *PCAN_SYNC;


//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif  // __CANBUS_H__

