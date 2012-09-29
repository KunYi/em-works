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
//  File:  i2cbus.h
//
//  Header file for CAN bus driver.
//
//------------------------------------------------------------------------------
#ifndef __CANBUS_H__
#define __CANBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define CAN1_FID                         _T("CAN1:")
#define CAN2_FID                         _T("CAN2:")

#define CAN_NO_ERROR                    (0)         // Last operation successful
#define CAN_ERR_MOPS_CREATE             (-1)        // Mutex Creation failed
#define CAN_ERR_PA_VA_MISSING           (-2)        // Physical -> Virtual Mapping failed
#define CAN_ERR_EOPS_CREATE             (-3)        // Event Creation failed
#define CAN_ERR_IRQ_SYSINTR_MISSING     (-4)        // IRQ -> System Interrupt ID Mapping failed
#define CAN_ERR_INT_INIT                (-5)        // Interrupt Initialization failed
#define CAN_ERR_WORKER_THREAD           (-6)        // Worker thread failed
#define CAN_ERR_NO_ACK_ISSUED           (-7)        // No Acknowledge Issued
#define CAN_ERR_NULL_BUF                (-8)        // NULL Buffer
#define CAN_ERR_INVALID_BUFSIZE         (-9)        // Invalid Buffer Size
#define CAN_ERR_NULL_LPIRESULT          (-10)       // NULL lpiResult field
#define CAN_ERR_CLOCK_FAILURE           (-11)       // CRM Operation Failure
#define CAN_ERR_TRANSFER_TIMEOUT        (-12)       // CAN transmit timeout error
#define CAN_ERR_ARBITRATION_LOST        (-13)       // CAN arbitration lost error
#define CAN_ERR_TRANSFER_ERR            (-14)       // CAN goto an undefined error
#define CAN_ERR_BIT0                    (-15)       // At least one bit sent as dominant is received as dominant
#define CAN_ERR_BIT1                    (-16)       // CANAt At least one bit sent as dominant is received as recessive
#define CAN_ERR_ACK                     (-17)       // CAN Acknowledge Error has been detected by the transmitter node
#define CAN_ERR_CRC                     (-18)       // CRC Error has been detected by the receiver node
#define CAN_ERR_FRM                     (-19)       // CAN Form Error has been detected by the receiver node
#define CAN_ERR_STF                     (-20)       // CAN Stuffing Error has been detected

#define CAN_RW_WRITE                    0           // Perform Write operation
#define CAN_RW_READ                     1           // Perform Read operation


// IO Control Codes
//
// Developers are encourage to use the macros listed below to access the driver
// capabilities.
// All the IOCTL codes provided here has a macro equivalent. The details for each
// IOCTL will be explained in the corresponding macros.
//


#define CAN_IOCTL_GET_CLOCK_RATE        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3804, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CAN_IOCTL_SET_CLOCK_RATE			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3805, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define CAN_IOCTL_TRANSFER						CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3809, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Transfer Data
#define CAN_IOCTL_RESET							CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3810, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Software Reset
#define CAN_IOCTL_ENABLE_TLPRIO				CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3811, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CAN_IOCTL_DISABLE_TLPRIO			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3812, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define CAN_IOCTL_SET_CAN_MODE			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3813, METHOD_BUFFERED, FILE_ANY_ACCESS) 

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
// CAN Packet
typedef enum _CAN_FRAME_FORMAT {
    CAN_STANDARD=0,
    CAN_EXTENDED,
} CAN_FRAME_FORMAT;

typedef enum _CAN_RTR_FORMAT {
    CAN_DATA=0,
    CAN_REMOTE,
} CAN_RTR_FORMAT;

typedef enum _CAN_MODE {
    CAN_TX=0,
    CAN_RX,
} CAN_MODE;

typedef struct
{
    BYTE byIndex;       // CAN Bus Message Buffer index for RX or TX
    BYTE byRW;          // Read = CAN_READ or Write = CAN_WRITE
    CAN_FRAME_FORMAT fromat;//Frame format is standard or Frame format is standard
    CAN_RTR_FORMAT   frame;//Indicate the trimission frame is Remote frame or Data frame
    WORD  timestamp;         // Message Buffer Length ,the max length is 8 bytes
    BYTE  PRIO;          //the transmission priority.
    DWORD ID;            //ID for CAN Bus Message Buffer If you use  standard  format ID must 11 bit ,use extnded frame ID can be 29 bit
    PBYTE pbyBuf;      // Message Buffer data the max length is 8 bytes
    WORD  wLen;         // Message Buffer Length ,the max length is 8 bytes
    LPINT lpiResult;   // Contains the result of last operation
} CAN_PACKET, *PCAN_PACKET;

//
// CS&ZHL July 26-2011: define can frame witch according FlexCAN Message Buffer
//
typedef struct
{
	DWORD		dwType;			// = 0：标准帧；= 1：扩展帧
	DWORD		dwID;				// 标准帧：11-bit；扩展帧：29-bit
	DWORD		dwRTR;				// = 0：数据帧；= 1：远程帧
	DWORD		dwPrio;				// 发送优先级，= 0 - 7，0为最高优先级
	DWORD 	dwDatLen;		// 数据长度 = 0 - 8
	UCHAR	    ucDat[8];			// 数据字节
}CAN_FRAME, *PCAN_FRAME;

// CAN Transfer Block
typedef struct
{
    CAN_PACKET *pCANPackets;
    INT32 iNumPackets;
} CAN_TRANSFER_BLOCK, *PCAN_TRANSFER_BLOCK;


//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------
HANDLE CANOpenHandle(LPCWSTR lpDevName);
BOOL  CANCloseHandle(HANDLE hCAN);
BOOL  CANEnableTLPRIO(HANDLE hCAN);  // transmitted Local Priority enabled
BOOL  CANDisableTLPRIO(HANDLE hCAN);  //transmitted Local Priority disenabled
BOOL  CANTransfer(HANDLE hCAN, PCAN_TRANSFER_BLOCK pCANTransferBlock); //read and write data
BOOL  CANSetMode(HANDLE hCAN,DWORD index,CAN_MODE mode);  //set can index as transmitte or receiver mode 
BOOL  CANReset(HANDLE hCAN);


#ifdef __cplusplus
}
#endif

#endif  // __CANBUS_H__

