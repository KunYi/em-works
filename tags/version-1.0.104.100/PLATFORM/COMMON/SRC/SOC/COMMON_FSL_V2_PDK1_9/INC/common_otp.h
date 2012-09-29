//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: common_otp.h
//
//  Provides definitions for the FlashInfoExt structure and soc/bsp functions
//  that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_OTP_H
#define __COMMON_OTP_H

#if    __cplusplus
extern "C" {
#endif

#pragma warning(push)
#pragma warning(disable:4201)

typedef struct _OtpProgram
{
    unsigned int OtpData;
    unsigned int OtpAddr;
}OtpProgram, * POtpProgram;

#pragma warning(pop)

#ifdef __cplusplus
}
#endif

#endif // __COMMON_OTP_H
