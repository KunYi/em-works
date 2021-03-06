//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#ifndef _DBGSETTINGS_H
#define _DBGSETTINGS_H

#ifndef DEBUG
#define DEBUG                // always turn on debug output
#endif // DEBUG

#ifdef DEBUG

#define DEBUGMASK(bit)       (1 << (bit))

#define MASK_ERROR           DEBUGMASK(0)
#define MASK_WARN            DEBUGMASK(1)
#define MASK_INIT            DEBUGMASK(2)
#define MASK_FUNCTION        DEBUGMASK(3)
#define MASK_IOCTL           DEBUGMASK(4)
#define MASK_DEVICE          DEBUGMASK(5)

#ifdef CAMINTERFACE
DBGPARAM dpCurSettings = {
    _T("CAMMDD"), 
    {
        _T("Errors"), _T("Warnings"), _T("Init"), _T("Function"), 
        _T("Ioctl"), _T("Device"), _T("Activity"), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T(""),_T(""),_T(""),_T("") 
    },
    MASK_ERROR | MASK_WARN | MASK_INIT 
}; 
#else
extern DBGPARAM dpCurSettings;
#endif

#define ZONE_ERROR           DEBUGZONE(0)
#define ZONE_WARN            DEBUGZONE(1)
#define ZONE_INIT            DEBUGZONE(2)
#define ZONE_FUNCTION        DEBUGZONE(3)
#define ZONE_IOCTL           DEBUGZONE(4)
#define ZONE_DEVICE          DEBUGZONE(5)

#define CAM_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CAM_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#endif
#endif
