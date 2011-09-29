//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header:  simce.h
//
//------------------------------------------------------------------------------

#ifndef __SIMCE_H__ 
#define __SIMCE_H__


//------------------------------------------------------------------------------
// Defines

//Vendor information
#define SIM_VENDOR_NAME            "Freescale"
#define SIM_PRODUCT_NAME           "Sim"

#define SIM_REG_PATH            "Drivers\\BuiltIn\\SIM"

#define POLLING_PERIOD                      500 // 500 ms

#define SIM_REG_IOBASE_VAL_NAME     TEXT("IOBase")
#define SIM_REG_ALTIOBASE_VAL_NAME      TEXT("AlternateIOBase")
#define SIM_REG_INDEX_VAL_NAME      TEXT("Index")

#define EnterDevice(pSCE) InterlockedIncrement(&(pSCE)->ReaderExtension->d_RefCount)
#define LeaveDevice(pSCE) InterlockedDecrement(&(pSCE)->ReaderExtension->d_RefCount)

//------------------------------------------------------------------------------
// Types

//Values for READER_EXTENSION.d_uReaderState
typedef enum _READER_STATE {
    STATE_INITING = 1,
    STATE_CLOSED,
    STATE_OPENED,
    STATE_DEAD,
    STATE_REMOVED
} READER_STATE;


//------------------------------------------------------------------------------
// Functions


#endif  // __SIMCE_H__
