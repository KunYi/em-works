//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//
// Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  File: ecc.c
//
//  This file implements ECC algorithm used on H4Sample NAND flash. 
//
#include <windows.h>

//------------------------------------------------------------------------------

#define DATA_BUFF_LEN           512
#define ECC_BUFF_LEN            3               // # of bytes in ECC

#define NO_DATA_ERROR           0
#define ECC_ERROR               1
#define CORRECTABLE_ERROR       12              // half of the ECC bits are 1

//------------------------------------------------------------------------------

static const BYTE ByteToNumberOfOnes[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

#define CountNumberOfOnes(num) ByteToNumberOfOnes[num]

//------------------------------------------------------------------------------
//
//  Function:  ECC_CorrectData()
//
//  Corrects any errors (if possible) in the specified data.
//
//  This implemention uses 3 bytes of ECC info for every 512 bytes of data.
//  Furthermore, a only single-bit error can be corrected for every 512 bytes
//  of data.
//
//  This code is based on the ECC algorithm publicly available on Samsung's
//  FLASH media website.
//

//BOOL ECC_CorrectData(LPBYTE pData, LPBYTE pExistingECC, LPBYTE pNewECC)
BOOL ECC_CorrectData(LPBYTE pData, DWORD dwDataBuffLen, LPBYTE pExistingECC, DWORD dwECCBuffLen)
{
    DWORD i, numOnes, byteLocation, bitLocation;
    BYTE xorECC[ECC_BUFF_LEN];

    //----- 1. Check the parameters -----
    if ((pData == NULL) || (pExistingECC == NULL)) {
        return FALSE;
    }

    //----- 2. First, determine if this is a single-bit, correctable, error ----
    //         NOTE: To answer this question, the two ECC values are XOR'd 
    //               together and the total # of 1 bits is counted, which 
    //               then tell us if we can correct the erroneous single-bit 
    //               transition in the data.
    numOnes = 0;
    for (i = 0; i < ECC_BUFF_LEN; i++) {
        xorECC[i] = pExistingECC[i] ^ pExistingECC[i+ECC_BUFF_LEN];
        numOnes += CountNumberOfOnes(xorECC[i]);
    }

    switch (numOnes) {
    case NO_DATA_ERROR:                 // Data doesn't contain an error
        return TRUE;

    case ECC_ERROR:                     // Existing ECC value has gone bad!
        return FALSE;

    case CORRECTABLE_ERROR:             // Single-bit error
        break;

    default:                            // More than a single-bit error
        return FALSE;
    }
        
    //----- 3. Compute the location of the single-bit error -----
    byteLocation = ((xorECC[2]&0xf0)<<1) | ((xorECC[1]&0xf8)>>3);
    bitLocation  = xorECC[1] & 0x7;

    //----- 4. Correct the single-bit error -----
    if (pData[byteLocation] & (0x01 << bitLocation)) {
        pData[byteLocation] &= ~(0x01 << bitLocation);      // 0->1 error
    } else {
        pData[byteLocation] |= (0x01 <<  bitLocation);      // 1->0 error
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ECC_Compute()
//
//  Compute ECC for passed data.
//
//VOID ECC_Compute(BYTE data[512], LPBYTE ecc_gen)
BOOL ECC_ComputeECC(LPBYTE pData, DWORD dwDataBuffLen, LPBYTE pECC, DWORD dwECCBuffLen)
{
    ULONG   i;
    ULONG   paritc = 0;
    BYTE    parity_bit;
    BYTE    bitmask[6] = {0x55, 0xaa, 0x33, 0xcc, 0xf, 0xf0};
    ULONG   ulEcc;
    
    ulEcc = 0;
    for ( i = 0; i < 512; i++) {
        paritc = paritc ^ pData[i];
        if ((CountNumberOfOnes(pData[i] ) & 1) != 0) {
            ulEcc = ulEcc ^ i;
        }
    }

    pECC[1] = (BYTE)(ulEcc << 3);
    pECC[2] = (BYTE)(ulEcc >> 1) & 0xf0;

    if ((CountNumberOfOnes( paritc ) & 1) != 0) {
        // Invert copied bits
        pECC[0] = pECC[1] ^ 0xf8;
        pECC[2] |= (((~pECC[2]) >> 4) & 0x0f) ;
    } else {
        pECC[0] = pECC[1];
        pECC[2] |= (pECC[2] >> 4);
    }

    for (i = 0; i < 6; i++) {
        parity_bit = CountNumberOfOnes(paritc & bitmask[i]) & 1;
        parity_bit <<= (i >> 1);
        pECC[(i & 1)] |= parity_bit;
    }
    return( TRUE );
}

/*
------------------------------------------------------------------------------
Function:		ECC_IsDataValid()

Description:	Determines if the specfied buffer of data is valid.  

Notes:			To determine if the data is valid, new ECC information is 
				generated for the specified data and then compared to the  
				specified (a.k.a. existing) ECC information.  

Returns:		Boolean indicating success.
-------------------------------------------------------------------------------
*/

BOOL ECC_IsDataValid(LPBYTE pData, DWORD dwDataBuffLen, LPBYTE pExistingECC, DWORD dwECCBuffLen)
{
	static UCHAR i = 0;
	static UCHAR newECC[ECC_BUFF_LEN];


    //----- 0. Compare ECC buf lengths
    if( dwECCBuffLen != ECC_BUFF_LEN )
    {
        return FALSE;
    }

	//----- 1. Compute the new ECC information for the data -----
	//         NOTE: We assume that the input data buffer is a sector (512 bytes)
	if(!ECC_ComputeECC(pData, dwDataBuffLen, newECC, ECC_BUFF_LEN))
	{
		return FALSE;
	}

	//----- 2. Compare the generated ECC info with the existing ECC info  -----
	//         NOTE: The data is valid if XORing all the ECC info together equals zero
	for(i=0; i<ECC_BUFF_LEN; i++)
	{
		if((newECC[i] != *(pExistingECC+i)))
		{
			return FALSE;
		}
	}

	return TRUE;	
}

