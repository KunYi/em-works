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
typedef enum _nand_ecc_type 
{
    BCH_Ecc_0bit = 0,
    BCH_Ecc_2bit,
    BCH_Ecc_4bit,
    BCH_Ecc_6bit,
    BCH_Ecc_8bit,
    BCH_Ecc_10bit,
    BCH_Ecc_12bit,
    BCH_Ecc_14bit,
    BCH_Ecc_16bit,
    BCH_Ecc_18bit,
    BCH_Ecc_20bit
} nand_ecc_type_t;


#define ECCN_2K64_PAGE   (BCH_Ecc_4bit)
#define ECCN_4K128_PAGE  (BCH_Ecc_4bit)
#define ECCN_4K218_PAGE  (BCH_Ecc_8bit)

#define ECC0_2K4K_PAGE  (BCH_Ecc_8bit)

#define METADATA_EXTRA  (2)
#define METADATA_SIZE   (METADATA_EXTRA + sizeof(SectorInfo))

//= int(2048 - ((METADATA_SIZE + ECC0_2K4K_PAGE * 13 / 8) + (512 + ECCN_2K64_PAGE * 13 / 8) * 3) + 512 * 3)
#define BBMarkByteOffsetInPageData_2K64   2005
#define BBMarkBitOffset_2K64              4     // ECCN_2K64_PAGE * 2 % 8

//= int(4096 - ((METADATA_SIZE + ECC0_2K4K_PAGE * 13 / 8) + (512 + ECCN_4K128_PAGE * 13 / 8) * 7) + 512 * 7)
#define BBMarkByteOffsetInPageData_4K128   4027
#define BBMarkBitOffset_4K128              4    // ECCN_4K128_PAGE * 2 % 8

//= int(4096 - ((METADATA_SIZE + ECC0_2K4K_PAGE * 13 / 8) + (512 + ECCN_4K218_PAGE * 13 / 8) * 7) + 512 * 7)
#define BBMarkByteOffsetInPageData_4K218   3982
#define BBMarkBitOffset_4K218              0    // ECCN_4K218_PAGE * 2 % 8
 
#define BV_GPMI_ECCCTRL_ECC_CMD__DECODE_4_BIT  0x0
#define BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_4_BIT  0x1
#define BV_GPMI_ECCCTRL_ECC_CMD__DECODE_8_BIT  0x2
#define BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_8_BIT  0x3

VOID ECC_Init();
BOOL ECC_IsCorrectable();
VOID ECC_ConfigLayout(WORD PageSize, WORD SpareSize);


#define DDI_NAND_HAL_CLEAR_ECC_COMPLETE_FLAG() (HW_BCH_CTRL_CLR(BM_BCH_CTRL_COMPLETE_IRQ))

#endif // _NAND_ECC_H
