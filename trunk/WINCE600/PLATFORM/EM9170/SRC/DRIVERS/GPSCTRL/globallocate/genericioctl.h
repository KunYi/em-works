/*******************************************************************************
* Global Locate A-GPS chipset Application Programming Interface
*
* Copyright (c) 2001-2007 by Global Locate, Inc. All Rights Reserved.
*
* The information contained herein is confidential property of Global Locate. 
* The use, copying, transfer or disclosure of such information is prohibited 
* except by express written agreement with Global Locate.
*******************************************************************************/
#ifndef GENERIC_IOCTL_H__    /* { */
#define GENERIC_IOCTL_H__    

#include "winioctl.h"

#ifndef CTL_CODE  // {

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#endif  // }

namespace GlobalLocate
{
namespace Gpsct
{
namespace Hal
{
namespace Generic
{
namespace Ioctl
{

const int GPS_FILE_DEVICE = 40045;

#define GPS_IOCTL(cmd) CTL_CODE(GPS_FILE_DEVICE,2100+(cmd),\
                            METHOD_BUFFERED, FILE_ANY_ACCESS)

enum Gpio
{
    GPS_POWER_DOWN_DATA =  GPS_IOCTL(1),    // obsolete
    GPS_CHIPSET_CONTROL =  GPS_IOCTL(2),    // REQUIRED
    GPS_RADIO_CLOCK     =  GPS_IOCTL(3),    // obsolete
    GPS_GET_CELL_INFO   =  GPS_IOCTL(4),    // obsolete
    GPS_GET_MSISDN      =  GPS_IOCTL(5),    // obsolete
    GPS_CUSTOM_PARAM    =  GPS_IOCTL(6),    // obsolete
    GPS_SECONDS         =  GPS_IOCTL(7),    // unused feature
    GPS_TRACE           =  GPS_IOCTL(8),    // future feature
    GPS_INTERRUPT_ACKNOWLEDGE
                        =  GPS_IOCTL(9),    // future feature
    GPS_IOCTL_GET_MILLISECOND_COUNT
                        =  GPS_IOCTL(10)

};

typedef struct GPS_GPIO_CONTROL
{
    DWORD id;
    DWORD value;
} GPS_GPIO_CONTROL;


}
} 
}
}
}

#endif                  /* } */
