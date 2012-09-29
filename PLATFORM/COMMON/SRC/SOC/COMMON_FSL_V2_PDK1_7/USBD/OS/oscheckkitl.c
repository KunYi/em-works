//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 

BOOL  FslUfnIsUSBKitlEnable()
{
    DWORD address =0;
    if(!KernelIoControl(IOCTL_KITL_GET_INFO,NULL,0,&address, sizeof(address),NULL ))
        return FALSE;
    
    //USB Serial KITL use CSP_BASE_REG_PA_USB+1, 
    //USB RNDIS KITL use CSP_BASE_REG_PA_USB
    if((address&0xFFFFFFF0) == CSP_BASE_REG_PA_USB )
    {
        return TRUE;
    }
    
    return FALSE;
}

