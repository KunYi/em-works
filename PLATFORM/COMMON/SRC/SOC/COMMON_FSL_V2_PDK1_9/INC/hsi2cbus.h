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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  hsi2cbus.h
//
//  Header file for HSI2C bus driver.
//
//------------------------------------------------------------------------------
#ifndef __HSI2CBUS_H__
#define __HSI2CBUS_H__

#ifdef __cplusplus
extern "C" {
#endif
//#include "i2cbus.h"
//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define HSI2C1_FID                         _T("HIC1:")
#define HSI2C_MAX_FREQUENCY               400000      // Maximum HSI2C frequency setting, current setting
                                                      // true high speed should support to 34,000,000


#define HSI2C_NO_ERROR                    (0)         // Last operation successful
#define HSI2C_ERR_MOPS_CREATE             (-1)        // Mutex Creation failed
#define HSI2C_ERR_PA_VA_MISSING           (-2)        // Physical -> Virtual Mapping failed
#define HSI2C_ERR_EOPS_CREATE             (-3)        // Event Creation failed
#define HSI2C_ERR_IRQ_SYSINTR_MISSING     (-4)        // IRQ -> System Interrupt ID Mapping failed
#define HSI2C_ERR_INT_INIT                (-5)        // Interrupt Initialization failed
#define HSI2C_ERR_WORKER_THREAD           (-6)        // Worker thread failed
#define HSI2C_ERR_NO_ACK_ISSUED           (-7)        // No Acknowledge Issued
#define HSI2C_ERR_NULL_BUF                (-8)        // NULL Buffer
#define HSI2C_ERR_INVALID_BUFSIZE         (-9)        // Invalid Buffer Size
#define HSI2C_ERR_NULL_LPIRESULT          (-10)       // NULL lpiResult field
#define HSI2C_ERR_CLOCK_FAILURE           (-11)       // CRM Operation Failure
#define HSI2C_ERR_TRANSFER_TIMEOUT        (-12)       // HSI2C transmit timeout error
#define HSI2C_ERR_ARBITRATION_LOST        (-13)       // HSI2C arbitration lost error
#define HSI2C_ERR_STATEMENT_CORRUPT       (-14)       // HSI2C goto an undefined error

#define HSI2C_RW_WRITE                    1           // Perform Write operation
#define HSI2C_RW_READ                     2           // Perform Read operation
#define HSI2C_METHOD_MASK                 0x7f

#define HSI2C_MODE_MASK                   0x80
#define HSI2C_POLLING_MODE                0x80
#define HSI2C_INTERRUPT_MODE              0x00 

//
// IO Control Codes
//
// Developers are encourage to use the macros listed below to access the driver
// capabilities.
// All the IOCTL codes provided here has a macro equivalent. The details for each
// IOCTL will be explained in the corresponding macros.
//
#define HSI2C_IOCTL_SET_SLAVE_MODE        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_SET_MASTER_MODE       CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_IS_MASTER             CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_IS_SLAVE              CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3003, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_GET_CLOCK_RATE        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3004, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_SET_CLOCK_RATE        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3005, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_SET_FREQUENCY         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3006, METHOD_BUFFERED, FILE_ANY_ACCESS)
                                                                                                                    
                                                                                                                    
                                                                                                                    
                                                                                                                    
#define HSI2C_IOCTL_SET_SELF_ADDR         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3007, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_GET_SELF_ADDR         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_TRANSFER              CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3009, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_RESET                 CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3010, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_ENABLE_SLAVE          CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3011, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_DISABLE_SLAVE         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3012, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_SET_SLAVE_TXT         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3013, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_GET_SLAVE_TXT         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3014, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_SET_SLAVESIZE         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3015, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define HSI2C_IOCTL_GET_SLAVESIZE         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3016, METHOD_BUFFERED, FILE_ANY_ACCESS)


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
// HSI2C Packet
typedef struct
{
    BYTE byAddr;       // HSI2C slave device address for this I2C operation
    BYTE byRW;         // Read = HSI2C_READ or Write = HSI2C_WRITE
    PBYTE pbyBuf;      // Message Buffer
    WORD wLen;         // Message Buffer Length
    LPINT lpiResult;   // Contains the result of last operation
} HSI2C_PACKET, *PHSI2C_PACKET;

// HSI2C Transfer Block
typedef struct
{
    HSI2C_PACKET *pHSI2CPackets;
    INT32 iNumPackets;
} HSI2C_TRANSFER_BLOCK, *PHSI2C_TRANSFER_BLOCK;


//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------
HANDLE HSI2COpenHandle(LPCWSTR lpDevName);
BOOL HSI2CCloseHandle(HANDLE hI2C);
BOOL HSI2CSetSlaveMode(HANDLE hI2C);
BOOL HSI2CSetMasterMode(HANDLE hI2C);
BOOL HSI2CIsMaster(HANDLE hI2C, PBOOL pbIsMaster);
BOOL HSI2CIsSlave(HANDLE hI2C, PBOOL pbIsSlave);
BOOL HSI2CGetClockRate(HANDLE hI2C, PWORD pwClkRate);
BOOL HSI2CSetClockRate(HANDLE hI2C, WORD wClkRate);
BOOL HSI2CSetFrequency(HANDLE hI2C, DWORD dwFreq);
BOOL HSI2CSetSelfAddr(HANDLE hI2C, BYTE bySelfAddr);
BOOL HSI2CGetSelfAddr(HANDLE hI2C, PBYTE pbySelfAddr);
BOOL HSI2CTransfer(HANDLE hI2C, PHSI2C_TRANSFER_BLOCK pHSI2CTransferBlock);
BOOL HSI2CReset(HANDLE hI2C);
BOOL HSI2CEnableSlave(HANDLE hI2C);
BOOL HSI2CDisableSlave(HANDLE hI2C);
BOOL HSI2CGetSlaveSize(HANDLE hI2C, PDWORD pdwSize);
BOOL HSI2CSetSlaveSize(HANDLE hI2C, DWORD dwSize);
BOOL HSI2CGetSlaveText(HANDLE hI2C, PBYTE pbyTextBuf, DWORD dwBufSize, PDWORD pdwTextLen);
BOOL HSI2CSetSlaveText(HANDLE hI2C, PBYTE pbyTextBuf, DWORD dwTextLen);

#ifdef __cplusplus
}
#endif

#endif  // __HSI2CBUS_H__

