/*==================================================================================================

    Module Name:  hab_super_root.c

    General Description:  This module contains the HAB Super Root public key.
                          It is generated automatically by a script.
                          
    This file is SRK.CA4.FSL.wtls.c got from http://compass.freescale.net/go/189506512

 ===================================================================================================
    
    Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
    THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
    AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT

 ===================================================================================================
                                        INCLUDE FILES
 =================================================================================================*/
#include "bsp.h"

/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
 =================================================================================================*/

/*==================================================================================================
                                     LOCAL CONSTANTS
 =================================================================================================*/
 #define MAX_EXP_SIZE   4

/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
 =================================================================================================*/
typedef struct
{
    UINT8   rsa_exponent[MAX_EXP_SIZE]; /* RSA public exponent */
    UINT8   *rsa_modulus;               /* RSA modulus pointer */
    UINT16  exponent_size;              /* Exponent size in bytes */
    UINT16  modulus_size;               /* Modulus size in bytes */
    BOOL    init_flag;                  /* Indicates if key initialized */
} hab_rsa_public_key;

/*==================================================================================================
                                        LOCAL MACROS
 =================================================================================================*/

/*==================================================================================================
                                      LOCAL VARIABLES
 =================================================================================================*/

/*==================================================================================================
                                     GLOBAL VARIABLES
 =================================================================================================*/
/* Super Root key moduli */
const UINT8 hab_super_root_moduli[] = 
{
    /* modulus data */
    0x8e, 0x14, 0xdc, 0x02, 0xd4, 0xc7, 0xbe, 0xe4, 0x37, 0xd2, 0x54, 0x52, 
    0x80, 0xf5, 0xf6, 0x6c, 0xb1, 0x76, 0x27, 0xfe, 0x97, 0x7c, 0x13, 0xd9, 
    0x0d, 0x93, 0xb1, 0xc6, 0x45, 0x6f, 0x13, 0x4e, 0x5f, 0x2c, 0xea, 0xd4, 
    0x22, 0xae, 0x27, 0x58, 0xd9, 0xdb, 0x8c, 0x43, 0x64, 0x9d, 0xd5, 0xee, 
    0x0d, 0x9d, 0x23, 0x42, 0xff, 0x44, 0x2a, 0x90, 0xaf, 0xcc, 0x04, 0x65, 
    0x0a, 0xce, 0x66, 0x85, 0x53, 0xe5, 0x7a, 0xf2, 0xf7, 0xbf, 0xe0, 0xc2, 
    0x0a, 0x3a, 0x27, 0x3a, 0x47, 0xd3, 0xbb, 0x21, 0x12, 0x73, 0xc1, 0x05, 
    0xef, 0x26, 0x7d, 0x6f, 0x65, 0x99, 0xf5, 0xc3, 0xad, 0xa5, 0x8b, 0xc1, 
    0xa8, 0x94, 0xc9, 0x60, 0xc6, 0xa0, 0xec, 0xe2, 0xd0, 0xdd, 0xdf, 0x11, 
    0x73, 0xc2, 0xdc, 0x7b, 0x9c, 0x8a, 0x6e, 0x21, 0xd9, 0x15, 0x4d, 0x97, 
    0x0b, 0xb8, 0x1a, 0xa7, 0xb4, 0x80, 0xcc, 0x1d, 0xeb, 0x55, 0xaa, 0xc7, 
    0x90, 0xd1, 0xdd, 0xb9, 0x37, 0xb9, 0xe7, 0x2e, 0x46, 0x49, 0x41, 0xec, 
    0x02, 0xaf, 0xd4, 0xd7, 0x87, 0xca, 0x40, 0xd9, 0x05, 0x3f, 0xdf, 0xb7, 
    0x7b, 0xab, 0x3c, 0x53, 0x8a, 0x5f, 0x18, 0xe7, 0x28, 0x95, 0xe0, 0x65, 
    0x93, 0xd5, 0x3d, 0xe3, 0xa5, 0x5c, 0x68, 0x1b, 0x2f, 0x1a, 0xab, 0x46, 
    0xe3, 0xc9, 0x12, 0x08, 0x58, 0x4b, 0xa3, 0xa5, 0x14, 0x7f, 0xbd, 0xfe, 
    0xdf, 0x8a, 0x1a, 0x81, 0xe2, 0xf6, 0x0f, 0x1f, 0xf7, 0xd9, 0x15, 0x7f, 
    0x94, 0x45, 0xb7, 0xad, 0xc7, 0x1f, 0x43, 0x1c, 0x8e, 0x26, 0x32, 0xde, 
    0xcf, 0x5e, 0xb7, 0x9c, 0xa9, 0xb6, 0x74, 0xbc, 0x11, 0x7b, 0x7f, 0x69, 
    0x14, 0x02, 0x1d, 0xa3, 0x5c, 0xd7, 0xaf, 0x90, 0x25, 0xe6, 0xc8, 0xaf, 
    0x78, 0x96, 0xc4, 0xd3, 0x5d, 0x9c, 0xa5, 0x9d, 0x96, 0x49, 0xf6, 0x22, 
    0x39, 0x70, 0xf4, 0xbb
};

/* Super Root key */
const hab_rsa_public_key hab_super_root_key[] = 
{
    {
        /* RSA public exponent, right-padded */
        {0x03, 0x00, 0x00, 0x00},
        /* pointer to modulus data */
        (UINT8 *)&hab_super_root_moduli[0],
        /* Exponent size in bytes */
        0x01, 
        /* Modulus size in bytes */
        0x100, 
        /* Key data valid */
        TRUE
    }
};

/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*================================================================================================*/
