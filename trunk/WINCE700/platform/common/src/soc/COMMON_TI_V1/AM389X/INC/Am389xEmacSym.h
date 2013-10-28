//========================================================================
//   Copyright 2006 Mistral Software Pvt Ltd.
//
//   Use of this software is controlled by the terms and conditions found
//   in the license agreement under which this software has been supplied.
//========================================================================

//! \file Am389xEmacSym.h
//!
//! \brief Exported Symbols Header file for EMAC ethernet controller 
//! 
//! This header file contains the function declarations
//! which are exported by the EMAC Module for use
//! by the bootloader and the OAL Libraries.
//! 
//! 
//! \version  1.00 May 22nd 2006 File Created 

#ifndef __AM389X_EMACSYM_H_INCLUDED__
#define __AM389X_EMACSYM_H_INCLUDED__

#include <windows.h>

BOOL AM389X_EmacInit(BYTE*  pbBaseAddress, DWORD  dwMultiplier, USHORT MacAddr[3]);       
UINT16 AM389X_EmacGetFrame(BYTE* pbData, UINT16* pwLength );
UINT16 AM389X_EmacSendFrame( BYTE* pbData, DWORD dwLength );    
BOOL AM389X_EmacSetPllClock (DWORD Pll1Clock );

#endif /* #ifndef __AM389X_EMACSYM_H_INCLUDED__ */
    