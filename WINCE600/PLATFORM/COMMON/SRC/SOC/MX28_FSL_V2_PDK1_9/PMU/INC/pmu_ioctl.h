//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:        pmu_ioctl.h
//
// Description: This header file defines IOCTL codes and parameter sturctures
// for PMU stream interface driver.
//
//------------------------------------------------------------------------------

#ifndef __PMU_IOCTL_H__
#define __PMU_IOCTL_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// Battery access
#define PMU_IOCTL_BATT_MONITOR_INIT    CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2000,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_BATT_CHARGE_SET      CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2001,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )
                                        
#define PMU_IOCTL_BATT_CHARGE_STOP     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2002,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_BATT_CHARGE_GET_STATUS   CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2003,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_BATT_GET_VOLTAGE     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2004,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )
//Power Rail Access
#define PMU_IOCTL_VDDD_GET_VOLTAGE     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2005,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_VDDD_GET_BRNOUT       CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2006,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_VDDD_SET_VOLTAGE      CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2007,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )
//Others
#define PMU_IOCTL_FET_SET_MODE         CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2008,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_POWER_SUPPLY_MODE_GET   CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x2009,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_POWER_THERMAL_SET     CTL_CODE(           \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x200a,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_POWER_THERMAL_GET     CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x200b,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_POWER_THERMAL_ENABLE  CTL_CODE(            \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x200c,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )
                                        
#define PMU_IOCTL_POWER_THERMAL_DISABLE  CTL_CODE(           \
                                        FILE_DEVICE_UNKNOWN, \
                                        0x200d,              \
                                        METHOD_BUFFERED,     \
                                        FILE_ANY_ACCESS      \
                                        )

#define PMU_IOCTL_GET_POWER_SOURCE		 CTL_CODE(           \
										FILE_DEVICE_UNKNOWN, \
										0x200e,              \
										METHOD_BUFFERED,     \
										FILE_ANY_ACCESS      \
										)

                                        
#ifdef __cplusplus
}
#endif

#endif // __PMU_IOCTL_H__