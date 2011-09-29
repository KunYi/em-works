//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  nand_ecc.h
//
//
//
//-----------------------------------------------------------------------------
#ifndef _NAND_ECC_H
#define _NAND_ECC_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

//ecc interface

#define ECCN_2K4K_PAGE  3
#define ECC0_2K4K_PAGE  4

#define NAND_METADATA_SIZE_4BIT             19
#define NAND_METADATA_SIZE_8BIT             65
#define NAND_ECC_BYTES_4BIT                 9
#define NAND_ECC_BYTES_8BIT                 18

#define BV_GPMI_ECCCTRL_ECC_CMD__DECODE_4_BIT  0x0
#define BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_4_BIT  0x1
#define BV_GPMI_ECCCTRL_ECC_CMD__DECODE_8_BIT  0x2
#define BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_8_BIT  0x3

VOID ECC_Init();
BOOL ECC_IsCorrectable();
VOID ECC_ConfigLayout(WORD PageSize, WORD SpareSize);


#define DDI_NAND_HAL_CLEAR_ECC_COMPLETE_FLAG() (HW_BCH_CTRL_CLR(BM_BCH_CTRL_COMPLETE_IRQ))

#endif // _NAND_ECC_H
