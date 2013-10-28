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
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  i2cbus.h
//
//  Header file for I2C bus driver.
//
//------------------------------------------------------------------------------
#ifndef __I2CBUS_H__
#define __I2CBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

#define I2C_NO_ERROR                        0           // Last operation successful
#define I2C_ERR_MOPS_CREATE                 -1          // Mutex Creation failed
#define I2C_ERR_PA_VA_MISSING               -2          // Physical -> Virtual Mapping failed
#define I2C_ERR_EOPS_CREATE                 -3          // Event Creation failed
#define I2C_ERR_IRQ_SYSINTR_MISSING         -4          // IRQ -> System Interrupt ID Mapping failed
#define I2C_ERR_INT_INIT                    -5          // Interrupt Initialization failed
#define I2C_ERR_WORKER_THREAD               -6          // Worker thread failed
#define I2C_ERR_NO_ACK_ISSUED               -7          // No Acknowledge Issued
#define I2C_ERR_NULL_BUF                    -8          // NULL Buffer
#define I2C_ERR_INVALID_BUFSIZE             -9          // Invalid Buffer Size
#define I2C_ERR_NULL_LPIRESULT              -10         // NULL lpiResult field
#define I2C_ERR_CLOCK_FAILURE               -11         // CRM Operation Failure
#define I2C_ERR_TRANSFER_TIMEOUT            -12         // I2C transmit timeout error
#define I2C_ERR_ARBITRATION_LOST            -13         // I2C arbitration lost error

#define I2C_RW_WRITE                    1               // Perform Write operation
#define I2C_RW_READ                     2               // Perform Read operation

#define I2C_MAX_FREQUENCY                    300000     // Maximum I2C frequency setting

// IO Control Codes
//
// Developers are encourage to use the macros listed below to access the driver
// capabilities.
// All the IOCTL codes provided here has a macro equivalent. The details for each
// IOCTL will be explained in the corresponding macros.
//
#define I2C_IOCTL_SET_SLAVE_MODE            CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3000, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set I2C Bus to Slave Mode
#define I2C_IOCTL_SET_MASTER_MODE           CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3001, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set I2C Bus to Master Mode
#define I2C_IOCTL_IS_MASTER                 CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3002, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Is it in Master Mode?
#define I2C_IOCTL_IS_SLAVE                  CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3003, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Is it in Slave Mode?
#define I2C_IOCTL_GET_CLOCK_RATE            CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3004, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Get Last Known Clock Rate
#define I2C_IOCTL_SET_CLOCK_RATE            CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3005, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set Clock Rate
#define I2C_IOCTL_SET_FREQUENCY             CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3006, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set the desired frequency. Note this is
                                                                                                                        // different from set clock rate. Set clock
                                                                                                                        // rate uses the values in the I2C Spec Table
                                                                                                                        // This function code will determine the best
                                                                                                                        // clock rate to used based on the hw clock
#define I2C_IOCTL_SET_SELF_ADDR             CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3007, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set My Address (Slave Mode)
#define I2C_IOCTL_GET_SELF_ADDR             CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3008, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Get My Address (Slave Mode)
#define I2C_IOCTL_TRANSFER                  CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3009, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Transfer Data
#define I2C_IOCTL_RESET                     CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3010, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Software Reset


// I2C Bus Class
#define I2C_BUS_CAM                 1           // CAM Bus Index
#define I2C_BUS_AUD                 2           // AUD Bus Index

// I2C File Name
#define I2C_FID_CAM                 _T("I2C1:")     // CAM File Index
#define I2C_FID_AUD                 _T("I2C2:")     // AUD File Index


// IOCTL_MACROS
// Please note that literal values cannot be used. Whatever values that is to be passed into the macro must
// reside in a variable.
//
// Example: to check if the bus is in master mode
//
//  BOOL bIsMaster = FALSE;
//
//  I2C_MACRO_IS_MASTER(hI2C, bIsMaster);
//
//  if (bIsMaster == TRUE)
//      printf("I2C Bus is in master mode");
//  else
//      printf("I2C Bus is in slave mode");
//

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_SET_SLAVE_MODE
//
// This macro set the I2C device in slave mode.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_SET_SLAVE_MODE(hDev) \
    (DeviceIoControl(hDev, I2C_IOCTL_SET_SLAVE_MODE, NULL, 0, NULL, 0, NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_SET_MASTER_MODE
//
// This macro set the I2C device in master mode.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_SET_MASTER_MODE(hDev) \
    (DeviceIoControl(hDev, I2C_IOCTL_SET_MASTER_MODE, NULL, 0, NULL, 0, NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_IS_MASTER
//
// This macro determines whether the I2C is currently in Master mode.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//      bIsMaster
//          [out]   TRUE if the I2C device is in Master mode.
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_IS_MASTER(hDev, bIsMaster) \
    (DeviceIoControl(hDev, I2C_IOCTL_IS_MASTER, NULL, 0, (PBYTE) &bIsMaster, sizeof(bIsMaster), NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_IS_SLAVE
//
// This macro determines whether the I2C is currently in Slave mode.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//      bIsSlave
//          [out]   TRUE if the I2C device is in Slave mode.
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_IS_SLAVE(hDev, bIsSlave) \
    (DeviceIoControl(hDev, I2C_IOCTL_IS_SLAVE, NULL, 0, (PBYTE) &bIsSlave, sizeof(bIsSlave), NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_GET_CLOCK_RATE
//
// This macro will retrieve the clock rate divisor. Note that the value is not
// the absolute peripheral clock frequency. The value retrieved should be
// compared against the I2C specifications to obtain the true frequency.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//      wClkRate
//          [out]   Contains the divisor index. Refer to I2C specification to
//          obtain the true clock frequency.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_GET_CLOCK_RATE(hDev, wClkRate) \
    (DeviceIoControl(hDev, I2C_IOCTL_GET_CLOCK_RATE, NULL, 0, (PBYTE) &wClkRate, sizeof(wClkRate), NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_SET_CLOCK_RATE
//
// This macro will initialize the I2C device with the given clock rate. Note
// that this macro does not expect to receive the absolute peripheral clock
// frequency. Rather, it will be expecting the clock rate divisor index stated
// in the I2C specification. If absolute clock frequency must be used, please
// use the macro I2C_MACRO_SET_FREQUENCY.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//      wClkRate
//          [in]    Contains the divisor index. Refer to I2C specification to
//          obtain the true clock frequency.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_SET_CLOCK_RATE(hDev, wClkRate) \
    (DeviceIoControl(hDev, I2C_IOCTL_SET_CLOCK_RATE, (PBYTE) &wClkRate, sizeof(wClkRate), NULL, 0, NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_SET_FREQUENCY
//
// This macro will estimate the nearest clock rate acceptable for I2C device
// and initialize the I2C device to use the estimated clock rate divisor. If
// the estimated clock rate divisor index is required, please refer to the macro
// I2C_MACRO_GET_CLOCK_RATE to determine the estimated index.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//      dwFreq
//          [in]    The desired frequency.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_SET_FREQUENCY(hDev, dwFreq) \
    (DeviceIoControl(hDev, I2C_IOCTL_SET_FREQUENCY, (PBYTE) &dwFreq, sizeof(dwFreq), NULL, 0, NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_SET_SELF_ADDR
//
// This macro will initialize the I2C device with the given address. The device
// will be expected to respond when any master within the I2C bus wish to
// proceed with any transfer. Note that this macro will have no effect if the
// I2C device is in Master mode.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//      bySelfAddr
//          [in]    The expected I2C device address. The valid range of address
//          is [0x00, 0x7F].
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_SET_SELF_ADDR(hDev, bySelfAddr) \
    (DeviceIoControl(hDev, I2C_IOCTL_SET_SELF_ADDR, (PBYTE) &bySelfAddr, sizeof(bySelfAddr), NULL, 0, NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_GET_SELF_ADDR
//
// This macro will retrieve the address of the I2C device. Note that this macro
// is only meaningful if it is currently in Slave mode.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//      bySelfAddr
//          [out]   The current I2C device address. The valid range of address
//          is [0x00, 0x7F].
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_GET_SELF_ADRR(hDev, bySelfAddr) \
    (DeviceIoControl(hDev, I2C_IOCTL_GET_SELF_ADRR, NULL, 0, (PBYTE) &bySelfAddr, sizeof(bySelfAddr), NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_TRANSFER
//
// This macro performs one or more I2C read or write operations.  pPackets contains
// a pointer to the first of an array of I2C packets to be processed by the I2C.
// All the required information for the I2C operations should be contained
// in the array elements of pPackets.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//      pI2CTransferBlock
//          [in]    I2C_TRANSFER_BLOCK structure type. 
//                  The fields of I2C_TRANSFER_BLOCK are described below:
//                  pI2CPackets[in]     Holds the pointer to the I2C Packets
//                                      to transfer.
//                  iNumPackets[in]     Number of packets in pI2CPackets array.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_TRANSFER(hDev, pI2CTransferBlock) \
    (DeviceIoControl(hDev, I2C_IOCTL_TRANSFER, (PBYTE) pI2CTransferBlock, sizeof(I2C_TRANSFER_BLOCK), NULL, 0, NULL, NULL))

//-----------------------------------------------------------------------------
//
// Macro:   I2C_MACRO_RESET
//
// This macro perform a hardware reset. Note that the I2C driver will still
// maintain all the current information of the device, which includes all the
// initialized addresses.
//
// Parameters:
//      hDev
//          [in]    The I2C device handle retrieved from CreateFile().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
#define I2C_MACRO_RESET(hDev) \
    (DeviceIoControl(hDev, I2C_IOCTL_RESET, NULL, 0, NULL, 0, NULL, NULL))

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

// I2C Packet
typedef struct
{
    BYTE byAddr;       // I2C slave device address for this I2C operation
    BYTE byRW;         // Read = I2C_READ or Write = I2C_WRITE
    PBYTE pbyBuf;      // Message Buffer
    WORD wLen;         // Message Buffer Length
    LPINT lpiResult;   // Contains the result of last operation
} I2C_PACKET, *PI2C_PACKET;

// I2C Transfer Block
typedef struct
{
    I2C_PACKET *pI2CPackets;
    INT32 iNumPackets;
} I2C_TRANSFER_BLOCK, *PI2C_TRANSFER_BLOCK;

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif  // __I2CBUS_H__

