//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cspibus.h
//
//   Header file for cspi bus driver.
//
//------------------------------------------------------------------------------

#ifndef __CSPIBUS_H__
#define __CSPIBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define CSPI_IOCTL_EXCHANGE           CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3030, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CSPI_IOCTL_ENABLE_LOOPBACK    CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3031, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CSPI_IOCTL_DISABLE_LOOPBACK   CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3032, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define CSPI_MAXENQ_EVENT_NAME        64
//------------------------------------------------------------------------------
// Types

// CSPI bus configuration
typedef struct
{
    UINT8   chipselect;
    UINT32  freq;
    UINT8   bitcount;
    BOOL    sspol;
    BOOL    ssctl;
    BOOL    pol;
    BOOL    pha;
    UINT8   drctl;
    BOOL    usedma;
    BOOL    usepolling;
} CSPI_BUSCONFIG_T, *PCSPI_BUSCONFIG_T;


// CSPI exchange packet
typedef struct
{
    PCSPI_BUSCONFIG_T pBusCnfg;
    LPVOID pTxBuf;
    LPVOID pRxBuf;
    UINT32 xchCnt;
    LPWSTR xchEvent;
    UINT32 xchEventLength;
} CSPI_XCH_PKT_T, *PCSPI_XCH_PKT_T;

//------------------------------------------------------------------------------
// Functions
HANDLE CSPIOpenHandle(LPCWSTR lpDevName);
BOOL CSPIExchange(HANDLE hCSPI, PCSPI_XCH_PKT_T pCspiXchPkt);
BOOL CSPIDisableLoopback(HANDLE hCSPI);
BOOL CSPIEnableLoopback(HANDLE hCSPI);

#ifdef __cplusplus
}
#endif

#endif // __CSPIBUS_H__
