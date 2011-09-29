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
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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

#define I2C_NO_ERROR                    (0)         // Last operation successful
#define I2C_ERR_MOPS_CREATE             (-1)        // Mutex Creation failed
#define I2C_ERR_PA_VA_MISSING           (-2)        // Physical -> Virtual Mapping failed
#define I2C_ERR_EOPS_CREATE             (-3)        // Event Creation failed
#define I2C_ERR_IRQ_SYSINTR_MISSING     (-4)        // IRQ -> System Interrupt ID Mapping failed
#define I2C_ERR_INT_INIT                (-5)        // Interrupt Initialization failed
#define I2C_ERR_WORKER_THREAD           (-6)        // Worker thread failed
#define I2C_ERR_NO_ACK_ISSUED           (-7)        // No Acknowledge Issued
#define I2C_ERR_NULL_BUF                (-8)        // NULL Buffer
#define I2C_ERR_INVALID_BUFSIZE         (-9)        // Invalid Buffer Size
#define I2C_ERR_NULL_LPIRESULT          (-10)       // NULL lpiResult field
#define I2C_ERR_CLOCK_FAILURE           (-11)       // CRM Operation Failure
#define I2C_ERR_TRANSFER_TIMEOUT        (-12)       // I2C transmit timeout error
#define I2C_ERR_ARBITRATION_LOST        (-13)       // I2C arbitration lost error
#define I2C_ERR_STATEMENT_CORRUPT       (-14)       // I2C goto an undefined error
#define I2C_ERR_EARLY_TERM                  (-15)       // I2C got NAK before end of transfer
#define I2C_ERR_GENERAL                        (-16)        // general error

#define I2C_RW_WRITE                    1           // Perform Write operation
#define I2C_RW_READ                     2           // Perform Read operation

#define I2C_MODE_MASK                   0x80
#define I2C_POLLING_MODE                0x80
#define I2C_INTERRUPT_MODE              0x00 

#define I2C_MAX_FREQUENCY               400000      // Maximum I2C frequency setting

//
// IO Control Codes
//
// Developers are encourage to use the macros listed below to access the driver
// capabilities.
// All the IOCTL codes provided here has a macro equivalent. The details for each
// IOCTL will be explained in the corresponding macros.
//
#define I2C_IOCTL_SET_SLAVE_MODE        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3000, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set I2C Bus to Slave Mode
#define I2C_IOCTL_SET_MASTER_MODE       CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3001, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set I2C Bus to Master Mode
#define I2C_IOCTL_IS_MASTER             CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3002, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Is it in Master Mode?
#define I2C_IOCTL_IS_SLAVE              CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3003, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Is it in Slave Mode?
#define I2C_IOCTL_GET_CLOCK_RATE        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3004, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Get Last Known Clock Rate
#define I2C_IOCTL_SET_CLOCK_RATE        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3005, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set Clock Rate
#define I2C_IOCTL_SET_SELF_ADDR         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3007, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Set My Address (Slave Mode)
#define I2C_IOCTL_GET_SELF_ADDR         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3008, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Get My Address (Slave Mode)
#define I2C_IOCTL_TRANSFER              CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3009, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Transfer Data
#define I2C_IOCTL_RESET                 CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3010, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Software Reset
#define I2C_IOCTL_ENABLE_SLAVE          CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3011, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Enable slave mode
#define I2C_IOCTL_DISABLE_SLAVE         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3012, METHOD_BUFFERED, FILE_ANY_ACCESS)  // Disable slave mode


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
// I2C Packet
typedef struct
{
    BYTE byRW;         // Read = I2C_READ or Write = I2C_WRITE
    PBYTE pbyBuf;      // Message Buffer: for 1st packet, THIS MUST CONTAIN: (slaveAddress | byRW) as first byte.
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
HANDLE I2COpenHandle(LPCWSTR lpDevName);
BOOL I2CCloseHandle(HANDLE hI2C);
BOOL I2CSetSlaveMode(HANDLE hI2C);
BOOL I2CSetMasterMode(HANDLE hI2C);
BOOL I2CIsMaster(HANDLE hI2C, PBOOL pbIsMaster);
BOOL I2CIsSlave(HANDLE hI2C, PBOOL pbIsSlave);
BOOL I2CGetClockRate(HANDLE hI2C, PDWORD pdwClkRate);
BOOL I2CSetClockRate(HANDLE hI2C, DWORD dwClkRate);
BOOL I2CSetSelfAddr(HANDLE hI2C, BYTE bySelfAddr);
BOOL I2CGetSelfAddr(HANDLE hI2C, PBYTE pbySelfAddr);
BOOL I2CTransfer(HANDLE hI2C, PI2C_TRANSFER_BLOCK pI2CTransferBlock);
BOOL I2CReset(HANDLE hI2C);
BOOL I2CEnableSlave(HANDLE hI2C);
BOOL I2CDisableSlave(HANDLE hI2C);


#ifdef __cplusplus
}
#endif

#endif  // __I2CBUS_H__

