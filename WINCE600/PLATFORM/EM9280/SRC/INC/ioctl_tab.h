//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ioctl_tab.h
//
//  Configuration file for the OAL IOCTL component.
//
//  This file is included by the platform's ioctl.c file and defines the
//  global IOCTL table, g_oalIoCtlTable[]. Therefore, this file may ONLY
//  define OAL_IOCTL_HANDLER entries.
//
// IOCTL CODE,                          Flags   Handler Function
//------------------------------------------------------------------------------

#include <oal_ioctl_tab.h>

#ifdef OAL_ILTIMING
{ IOCTL_HAL_ILTIMING,                    0,  OALIoCtlHalILTiming         },
#endif

//
// CS&ZHL FEB-28-2012: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
{ IOCTL_HAL_NANDFMD_ACCESS,              0,  OALIoCtlHalNandfmdAccess    },
#endif	//NAND_PDD

#ifdef	EM9280
#ifndef	UUT
// CS&ZHL MAY-13-2012: supporting SSP0 based SPI port
{ IOCTL_HAL_SPI_ACCESS,                  0,  OALIoCtlHalSpiAccess        },
// end of CS&ZHL MAY-13-2012: supporting SSP0 based SPI port
#endif	//UUT

// CS&ZHL MAY-18-2012: supporting GPIO based I2C port
{ IOCTL_HAL_I2C_ACCESS,                  0,  OALIoCtlHalI2cAccess        },
// end of CS&ZHL MAY-18-2012: supporting GPIO based I2C port
#endif	//EM9280

//
// CS&ZHL APR-06-2012: supporting get board info
//
{ IOCTL_HAL_BOARDINFO_READ,				0,  OALIoCtlHalBoardInfoRead },
{ IOCTL_HAL_TIMESTAMP_READ,				0,  OALIoCtlHalTimeStampRead },
{ IOCTL_HAL_VENDOR_ID_READ,				0,  OALIoCtlHalVendorIDRead },
{ IOCTL_HAL_CUSTOMER_ID_READ,			0,  OALIoCtlHalCustomerIDRead },
{ IOCTL_HAL_BOARD_STATE_READ,			0,  OALIoCtlHalBoardStateRead },
{ IOCTL_HAL_WATCHDOG_GET,				0,  OALIoCtlHalWatchdogGet },

// CS&ZHL SEP-18-2012: set the flag to inform WDT that app is fine
{ IOCTL_HAL_WSTARTUP_END,				0,  OALIoCtlHalSetWstartupEndFlag },

// CS&ZHL APR-9-2012: read FSL copyright info and chip ID
{ IOCTL_HAL_CPU_INFO_READ,               0,  OALIoCtlHalGetCPUInfo       },
// end of CS&ZHL APR-9-2012: read FSL copyright info and chip ID

{ IOCTL_HAL_POSTINIT,                    0,  OALIoCtlHalPostInit         },
{ IOCTL_HAL_PRESUSPEND,                  0,  OALIoCtlHalPresuspend       },
{ IOCTL_HAL_GET_HWENTROPY,               0,  OALIoCtlHalGetHWEntropy     },
{ IOCTL_KITL_GET_INFO,                   0,  OALIoCtlKitlGetInfo         },
{ IOCTL_HAL_POWER_OFF_ENABLE,            0,  OALIoCtlPowerOffEnable      },
{ IOCTL_HAL_QUERY_BOARD_ID,              0,  OALIoCtlQueryBoardId        },
{ IOCTL_HAL_QUERY_BOOT_MODE,             0,  OALIoCtlQueryBootMode       },
{ IOCTL_HAL_SET_BOOT_SOURCE,             0,  OALIoCtlSetBootSource       },
{ IOCTL_HAL_QUERY_UPDATE_SIG,            0,  OALIoCtlQueryUpdateSig      },
{ IOCTL_HAL_SET_UPDATE_SIG,              0,  OALIoCtlSetUpdateSig        },
{ 0,                                     0,  NULL                        } // Terminator
