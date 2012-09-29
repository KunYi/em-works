//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: adc_ioctl.h
//
//
//-----------------------------------------------------------------------------
#ifndef __ADC_IOCTL_H__
#define __ADC_IOCTL_H__

#define IOCTL_GENERAL_CONFIG        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CFG_ITEM              CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CFG_QUEUE             CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_START_ACQUIRE_SNGL    CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3003, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STOP_ACQUIRE_SNGL     CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3004, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS \
                                    CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3005, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WAKEUP_SOURCE         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3006, METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef enum {
    TICR,

    TCC0,
    TCC1,
    TCC2,
    TCC3,
    TCC4,
    TCC5,
    TCC6,
    TCC7,   
    
    GCC0,
    GCC1,
    GCC2,
    GCC3,
    GCC4,
    GCC5,
    GCC6,
    GCC7,
    ITEM_ID_END
} T_ITEM_ID;

typedef enum {
    TOUCH_QUEUE,
    GENERAL_QUEUE,
} T_QUEUE_ID;


typedef struct {
    T_ITEM_ID eItemID;
    DWORD dwItemConfig;
} T_IOCTL_CFG_ITEM_PARAM;

typedef struct {    
    DWORD dwGeneralConfig;
} T_IOCTL_GENERAL_CONFIG_PARAM;

typedef struct {    
    T_QUEUE_ID eQueueID;
    DWORD dwItem_7_0;
    DWORD dwItem_15_8;
    DWORD dwQConfigRreg; 
    DWORD dwQIntConfig;
} T_IOCTL_CFG_QUEUE_PARAM;

typedef struct {    
    T_QUEUE_ID eQueueID;
} T_IOCTL_START_ACQUIRE_SNGL_PARAM;

typedef struct {    
    T_QUEUE_ID eQueueID;
} T_IOCTL_STOP_ACQUIRE_SNGL_PARAM;

typedef struct {    
    T_QUEUE_ID eQueueID;
    DWORD dwTimeout;
} T_IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS_PARAM;

typedef struct {    
    BOOL fEnableWakeUp;
} T_IOCTL_WAKEUP_SOURCE_PARAM;

#endif //__ADC_IOCTL_H__
