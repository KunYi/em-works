/******************************************************************************

           Copyright (c) Emtronix 2008 .

            All rights reserved.  No part of this software may
            be published, distributed, translated or otherwise
            reproduced by any means or for any purpose without
            the prior written consent of Emtronix.

    Description:

    IRQ IOControl Interface.

******************************************************************************/

#include <winioctl.h>

// create unique system I/O control codes (IOCTL)
#define IOCTL_GET_IRQ_EVENT			CTL_CODE(FILE_DEVICE_UNKNOWN, 0xB01, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_EOI						CTL_CODE(FILE_DEVICE_UNKNOWN, 0xB02, METHOD_BUFFERED, FILE_ANY_ACCESS)
