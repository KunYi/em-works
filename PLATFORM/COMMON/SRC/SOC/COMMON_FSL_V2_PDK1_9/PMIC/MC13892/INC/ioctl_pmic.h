//-----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

#ifndef _IOCTL_PMIC_H_
#define _IOCTL_PMIC_H_

#include <winioctl.h>
                                                                                                                     
#define IOCTL_HAL_RTC_INIT    \
    CTL_CODE(FILE_DEVICE_HAL, 1100, METHOD_BUFFERED, FILE_ANY_ACCESS)
    
#define IOCTL_HAL_RTC_QUERY     \
    CTL_CODE(FILE_DEVICE_HAL, 1101, METHOD_BUFFERED, FILE_ANY_ACCESS)    
    
#define IOCTL_HAL_RTC_SYNC    \
    CTL_CODE(FILE_DEVICE_HAL, 1102, METHOD_BUFFERED, FILE_ANY_ACCESS)  
    
#define IOCTL_HAL_RTC_ALARM    \
    CTL_CODE(FILE_DEVICE_HAL, 1103, METHOD_BUFFERED, FILE_ANY_ACCESS)   
           
BOOL OALIoCtlInitRTC( UINT32 code, VOID *pInpBuffer, UINT32 inpSize,
                         VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);
                         
BOOL OALIoCtlHalRtcSync( UINT32 code, VOID *pInBuffer, UINT32 inSize, 
                         VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);  
                         
BOOL OALIoCtlHalRtcAlarm( UINT32 code, VOID *pInBuffer, UINT32 inSize, 
                         VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);   
                         
BOOL OALIoCtlHalRtcQuery( UINT32 code, VOID *pInBuffer, UINT32 inSize, 
                         VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);  


#endif // _IOCTL_PMIC_H_
