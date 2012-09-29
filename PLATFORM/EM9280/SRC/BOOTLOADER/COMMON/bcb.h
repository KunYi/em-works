//-----------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  bcb.h
//
//  Contains BCB NAND flash support functions.
//
//-----------------------------------------------------------------------------


#ifndef _NAND_BCB_H
#define _NAND_BCB_H


//#include <nand_hal.h>
#include <nand_ecc.h>
#include <nand_dma.h>
#include <nand_gpmi.h>

//-----------------------------------------------------------------------------
// Defines
#define NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES        (512) 
#define NAND_HC_ECC_OFFSET_DATA_COPY    12
#define NAND_HC_ECC_OFFSET_PARITY_COPY  ((NAND_HC_ECC_OFFSET_DATA_COPY) + 512)


//! \brief Structure defining where FCB and DBBT parameters are located.
//!
//! This structure defines the basic fingerprint template for both the Firmware
//! Control Block (FCB) and the Discovered Bad Block Table (DBBT).  This
//! template is used to determine if the sector read is a Boot Control Block.
//! This structure defines the Firmware Control Block (FCB).  This block
//! contains information describing the timing for the NAND, the number of
//! NANDs in the system, the block size of the NAND, the page size of the NAND,
//! and other criteria for the NAND.  This is information that is
//! required just to successfully communicate with the NAND.
//! This block contains information describing the version as well as the layout of
//! the code and data on the NAND Media.  For the ROM, we're only concerned
//! with the boot firmware start.  Additional information may be stored in
//! the Reserved area.  This area will be of interest to the SDK.
//! This structure also defines the Discovered Bad Block Table (DBBT) header.  
//! This block contains the information used for parsing the bad block tables
//! which are stored in subsequent 2K sectors.  The DBBT header is 8K, followed
//! by the first NANDs entries on a subsequent 2K page (determined by how many 
//! 2K pages the first nand requires)
typedef struct _BootBlockStruct_t
{
    UINT32    m_u32Checksum;         //!< First fingerprint in first byte.
    UINT32    m_u32FingerPrint;      //!< 2nd fingerprint at byte 4.
    UINT32    m_u32Version;          //!< 3rd fingerprint at byte 8.
    union
    {
        struct
        {            
            NAND_Timing_t   m_NANDTiming;                   //!< Optimum timing parameters for Tas, Tds, Tdh in nsec.
            UINT32        m_u32Reserved;                  //mx28 needs a dword reservation
            
            UINT32        m_u32DataPageSize;              //!< 2048 for 2K pages, 4096 for 4K pages.
            UINT32        m_u32TotalPageSize;             //!< 2112 for 2K pages, 4314 for 4K pages.
            UINT32        m_u32SectorsPerBlock;           //!< Number of 2K sections per block.
            UINT32        m_u32NumberOfNANDs;             //!< Total Number of NANDs - not used by ROM.
            UINT32        m_u32TotalInternalDie;          //!< Number of separate chips in this NAND.
            UINT32        m_u32CellType;                  //!< MLC or SLC.
            UINT32        m_u32EccBlockNEccType;         //!< Type of ECC, can be one of BCH-0-20
            UINT32        m_u32EccBlock0Size;             //!< Number of bytes for Block0 - BCH
            UINT32        m_u32EccBlockNSize;             //!< Block size in bytes for all blocks other than Block0 - BCH
            UINT32        m_u32EccBlock0EccType;         //!< Ecc level for Block 0 - BCH
            UINT32        m_u32MetadataBytes;             //!< Metadata size - BCH
            UINT32        m_u32NumEccBlocksPerPage;       //!< Number of blocks per page for ROM use - BCH
            UINT32        m_u32EccBlockNEccLevelSDK;      //!< Type of ECC, can be one of BCH-0-20
            UINT32        m_u32EccBlock0SizeSDK;          //!< Number of bytes for Block0 - BCH
            UINT32        m_u32EccBlockNSizeSDK;          //!< Block size in bytes for all blocks other than Block0 - BCH
            UINT32        m_u32EccBlock0EccLevelSDK;      //!< Ecc level for Block 0 - BCH
            UINT32        m_u32NumEccBlocksPerPageSDK;    //!< Number of blocks per page for SDK use - BCH
            UINT32        m_u32MetadataBytesSDK;          //!< Metadata size - BCH
            UINT32        m_u32EraseThreshold;            //!< To set into BCH_MODE register.
            UINT32        m_u32BootPatch;                 //!< 0 for normal boot and 1 to load patch starting next to FCB.
            UINT32        m_u32PatchSectors;              //!< Size of patch in sectors.
            UINT32        m_u32Firmware1_startingSector;  //!< Firmware image starts on this sector.
            UINT32        m_u32Firmware2_startingSector;  //!< Secondary FW Image starting Sector.
            UINT32        m_u32SectorsInFirmware1;        //!< Number of sectors in firmware image.
            UINT32        m_u32SectorsInFirmware2;        //!< Number of sector in secondary FW image.
            UINT32        m_u32DBBTSearchAreaStartAddress;//!< Page address where dbbt search area begins
            UINT32        m_u32BadBlockMarkerByte;        //!< Byte in page data that have manufacturer marked bad block marker, this will
                                                            //!< bw swapped with metadata[0] to complete page data.
            UINT32        m_u32BadBlockMarkerStartBit;    //!< For BCH ECC sizes other than 8 and 16 the bad block marker does not start 
                                                            //!< at 0th bit of m_u32BadBlockMarkerByte. This field is used to get to the 
                                                            //!< start bit of bad block marker byte with in m_u32BadBlockMarkerByte.
            UINT32        m_u32BBMarkerPhysicalOffset;    //!< FCB value that gives byte offset for bad block marker on physical NAND page.
        } FCB_Block;
        struct
        {                            
            UINT32        m_u32NumberBB;                    //!< # Bad Blocks stored in this table for NAND0.
            UINT32        m_u32Number2KPagesBB;           //!< Bad Blocks for NAND0 consume this # of 2K pages.   
        } DBBT_Block;
    };
} BootBlockStruct_t;


#endif
