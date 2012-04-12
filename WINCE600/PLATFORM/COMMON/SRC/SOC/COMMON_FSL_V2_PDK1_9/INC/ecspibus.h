//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ecspibus.h
//
//   Header file for cspi bus driver.
//
//------------------------------------------------------------------------------

#ifndef __ECSPIBUS_H__
#define __ECSPIBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define CSPI_IOCTL_EXCHANGE           CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3030, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CSPI_IOCTL_ENABLE_LOOPBACK    CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3031, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CSPI_IOCTL_DISABLE_LOOPBACK   CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3032, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define CSPI_MAXENQ_EVENT_NAME        64

#define CSPI_CS0                0x0
#define CSPI_CS1                0x1
#define CSPI_CS2                0x2
#define CSPI_CS3                0x3

#define CSPI_DRCTL_DONTCARE     0x0
#define CSPI_DRCTL_FALLING_EDGE 0x1
#define CSPI_DRCTL_ACTIVE_LOW   0x2
#define CSPI_DRCTL_RSV          0x3
//------------------------------------------------------------------------------
// Types

// eCSPI bus configuration
typedef struct
{
    UINT8   ChannelSelect;      //CS0, CS1, CS2, CS3
    UINT32  Freq;
    UINT32  BurstLength;        //bitcount,  recommend 32bit as unit. 
    BOOL    SSPOL;              
    BOOL    SCLKPOL;
    BOOL    SCLKPHA;
    UINT8   DRCTL;              //SPI_RDY enable
    BOOL    usedma;
} CSPI_BUSCONFIG_T, *PCSPI_BUSCONFIG_T;


// eCSPI exchange packet
typedef struct
{
    PCSPI_BUSCONFIG_T pBusCnfg;
    LPVOID pTxBuf;
    LPVOID pRxBuf;
    UINT32 xchCnt;              //32bit unit; It must equal to BurstLength/32 or BurstLength/32 +1 (if BurstLength isn't integral multiple of 32bit)
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

#endif // __ECSPIBUS_H__
