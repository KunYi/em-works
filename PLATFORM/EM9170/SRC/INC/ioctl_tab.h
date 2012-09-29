//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
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
// CS&ZHL MAY-13-2011: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
{ IOCTL_HAL_NANDFMD_ACCESS,              0,  OALIoCtlHalNandfmdAccess    },
#endif	//NAND_PDD

//
// CS&ZHL JUN-23-2011: supporting get board info
//
#ifdef EM9170
{ IOCTL_HAL_BOARDINFO_READ,				0,  OALIoCtlHalBoardInfoRead },
{ IOCTL_HAL_TIMESTAMP_WRITE,			0,  OALIoCtlHalTimeStampWrite },
{ IOCTL_HAL_TIMESTAMP_READ,				0,  OALIoCtlHalTimeStampRead },
{ IOCTL_HAL_VENDOR_ID_READ,				0,  OALIoCtlHalVendorIDRead },
{ IOCTL_HAL_CUSTOMER_ID_READ,			0,  OALIoCtlHalCustomerIDRead },
{ IOCTL_HAL_BOARD_STATE_READ,			0,  OALIoCtlHalBoardStateRead },
{ IOCTL_HAL_WATCHDOG_GET,				0,  OALIoCtlHalWatchdogGet },
#endif	//EM9170

{ IOCTL_HAL_POSTINIT,                    0,  OALIoCtlHalPostInit         },
{ IOCTL_HAL_PRESUSPEND,                  0,  OALIoCtlHalPresuspend       },
{ IOCTL_HAL_QUERY_DISPLAYSETTINGS,       0,  OALIoCtlQueryDispSettings   },
{ IOCTL_HAL_GET_HWENTROPY,               0,  OALIoCtlHalGetHWEntropy     },
{ IOCTL_HAL_IRQ2SYSINTR,                 0,  OALIoCtlHalIrq2Sysintr      },
{ IOCTL_HAL_FORCE_IRQ,                   0,  OALIoCtlHalForceIrq         },
{ IOCTL_HAL_UNFORCE_IRQ,                 0,  OALIoCtlHalUnforceIrq       },
{ IOCTL_HAL_GET_RANDOM_SEED,             0,  OALIoCtlHalGetRandomSeed    },
{ IOCTL_HAL_QUERY_SI_VERSION,            0,  OALIoCtlQuerySiVersion      },
//
// CS&ZHL JUN-2-2011: for iMX257PDK only
//
#ifdef		IMX257PDK_CPLD
{ IOCTL_HAL_CPLD_READ16,                 0,  OALIoCtlCpldRead16          },
{ IOCTL_HAL_CPLD_WRITE16,                0,  OALIoCtlCpldWrite16         },
{ IOCTL_HAL_SHARED_CSPI_TRANSFER,        0,  OALIoCtlSharedCspiTransfer  },
#endif		//IMX257PDK_CPLD

{ IOCTL_HAL_GET_RESET_CAUSE,      0,  OALIoCtlGetResetCause       },
{ IOCTL_KITL_GET_INFO,                   0,  OALIoCtlKitlGetInfo         },
// Required Termination
{ 0,                                     0,  NULL                        }
