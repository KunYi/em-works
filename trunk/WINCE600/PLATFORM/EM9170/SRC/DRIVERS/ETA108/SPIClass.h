/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Emroonix, inc. Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
//  File:  eta108class.h
//
//   Header file for cspi bus driver.
//
//------------------------------------------------------------------------------
#ifndef __SPICALSS_H__
#define __SPICALSS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"
#include "cspibus.h"
#include "..\..\..\..\COMMON\SRC\SOC\COMMON_FSL_V2_PDK1_7\CSPIBUSV2\PDK\cspiClass.h"

//------------------------------------------------------------------------------
//Defines

//------------------------------------------------------------------------------
//Types
// 
class spiClass:public cspiClass
{

public:
 	spiClass();
 	~spiClass();
	BOOL CspiInitialize(DWORD Index);
	void CspiRelease(void);
public:
 
private:
 
};
 




#ifdef __cplusplus
}
#endif

#endif __SPICALSS_H__