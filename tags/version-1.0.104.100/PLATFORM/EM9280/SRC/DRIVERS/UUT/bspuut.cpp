//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bspuut.cpp
//
//  Implements bsp related uce commands.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:  6287 6262 4201 4512 4100 4115 4214)
#include <windows.h>
#include "stdio.h"
#include "stdlib.h"
#pragma warning(pop)


BOOL OTPProgram(char *pBuff);
BOOL BSPUceCmdDeal(char * pbCmd)
{
    if (strncmp(pbCmd,"OtpSendData",11) == 0) 
    {
        RETAILMSG(1, (L"Receiving OTP bits information for programming.\r\n"));       
        OTPProgram(pbCmd);
    }
    return TRUE;
}

BOOL BSPUceTransDeal(PBYTE pbData,  DWORD DataLength)
{
    UNREFERENCED_PARAMETER(pbData);
    UNREFERENCED_PARAMETER(DataLength);
    
    return TRUE;    
}