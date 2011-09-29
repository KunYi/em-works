//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  nand_dma.h
//
//
//
//-----------------------------------------------------------------------------
#ifndef _NAND_DMA_H
#define _NAND_DMA_H

#include "csp.h"
#include "regsgpmi.h"
#include "regsdigctl.h"
#include "nand_hal.h"


////////////////////////////////////////////////////////////////////////////////
//! APBH Definitions
////////////////////////////////////////////////////////////////////////////////

typedef struct _dma_cmd
{
    struct _dma_cmd    *pNxt;
    hw_apbh_chn_cmd_t cmd;
    VOID                *pBuf;
    hw_gpmi_ctrl0_t ctrl;
    hw_gpmi_compare_t cmp;
} dma_cmd_t;


///////////////////////////////////////////////////////////////////
//! \name APBH DMA Structure Definitions
//@{
//////////////////////////////////////////////////////////////////

//! Define the APBH DMA structure without GPMI transfers.
typedef struct _apbh_dma_t
{
    struct _apbh_dma_t*   nxt;
    hw_apbh_chn_cmd_t cmd;
    VOID*                 bar;
} apbh_dma_t;

//! Define the APBH DMA structure with 1 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi1_t
{
    struct _apbh_dma_gpmi1_t*   nxt;
    hw_apbh_chn_cmd_t cmd;
    VOID*                       bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
        };
        UINT32 pio[1];
    };
} apbh_dma_gpmi1_t;

//! Define the APBH DMA structure with 2 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi2_t
{
    struct _apbh_dma_gpmi2_t*     nxt;
    hw_apbh_chn_cmd_t cmd;
    VOID*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
        };
        UINT32 pio[2];
    };
} apbh_dma_gpmi2_t;


//! Define the APBH DMA structure with 3 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi3_t
{
    struct _apbh_dma_gpmi1_t*     nxt;
    hw_apbh_chn_cmd_t cmd;
    VOID*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
            hw_gpmi_eccctrl_t gpmi_eccctrl;
        };
        UINT32 pio[3];
    };
} apbh_dma_gpmi3_t;

//! Define the APBH DMA structure with 4 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi4_t
{
    struct _apbh_dma_gpmi1_t*     nxt;
    hw_apbh_chn_cmd_t cmd;
    VOID*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
            hw_gpmi_eccctrl_t gpmi_eccctrl;
            hw_gpmi_ecccount_t gpmi_ecccount;

        };
        UINT32 pio[4];
    };
} apbh_dma_gpmi4_t;

//! Define the APBH DMA structure with 5 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi5_t
{
    struct _apbh_dma_gpmi1_t*     nxt;
    hw_apbh_chn_cmd_t cmd;
    VOID*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
            hw_gpmi_eccctrl_t gpmi_eccctrl;
            hw_gpmi_ecccount_t gpmi_ecccount;
            hw_gpmi_payload_t gpmi_payload;
        };
        UINT32 pio[5];
    };
} apbh_dma_gpmi5_t;

//! Define the APBH DMA structure with 6 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi6_t
{
    struct _apbh_dma_gpmi1_t*     nxt;
    hw_apbh_chn_cmd_t cmd;
    VOID*                         bar;
    union
    {
        struct
        {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
            hw_gpmi_eccctrl_t gpmi_eccctrl;
            hw_gpmi_ecccount_t gpmi_ecccount;
            hw_gpmi_payload_t gpmi_payload;
            hw_gpmi_auxiliary_t gpmi_auxiliary;

        };
        UINT pio[6];
    };
} apbh_dma_gpmi6_t;

//@}

////////////////////////////////////////////////////////////////////////////////
//! \name NAND Addressing Structures
////////////////////////////////////////////////////////////////////////////////
#define MAX_COLUMNS     2       //!< Current NANDs only use 2 bytes for column.
#define MAX_ROWS        3       //!< Current NANDs use a max of 3 bytes for row.

//! CLE command plus up to Max Columns and Max Rows.
#define CLE1_MAX_SIZE   MAX_COLUMNS+MAX_ROWS+1

// Miscellaneous Data
typedef struct _NAND_Misc_Struct
{
    DWORD uStartDMATime;
    DWORD uDMATimeout;
} NAND_Misc_Struct;

///////////////////////////////////////////////////////////////////
//!  \name   DMA Chain Structure Definitions
//////////////////////////////////////////////////////////////////

//! Number of commands sent for a NAND Device Reset.
#define NAND_RESET_DEVICE_SIZE  1
//! Number of commands sent for a NAND Device Read ID.
#define NAND_READ_ID_SIZE   2
//! Number of commands read for a NAND Device Read ID.
#define NAND_READ_ID_RESULT_SIZE  6    // Reading 6 bytes back.
//! Number of commands sent to read NAND Device Status.
#define NAND_READ_STATUS_SIZE  1
//! Number of commands read for a NAND Device Status result.
#define NAND_READ_STATUS_RESULT_SIZE  1

///////////////////////////////////////////////////////////////////////
//!     DMA Seed Structures.
//!     These structures hold the data elements needed by
//!     the DMA descriptor chains.
///////////////////////////////////////////////////////////////////////

typedef struct _NAND_dma_read_id_seed_t
{
    union
    {
        // Buffer for Reset Command.
        UCHAR tx_readid_command_buf[NAND_READ_ID_SIZE];
        struct
        {
            UCHAR txCLEByte;
            UCHAR txALEByte;
        };
    };
} NAND_dma_read_id_seed_t;

//! \brief Seed structure with values required for proper read.
//!
//! Dma seed structure for Read Page
//! Use data from this structure to fill in the DMA chain above.
typedef struct _NAND_read_seed_t
{
    // Number of Column & Row bytes to be sent.
    UINT32 uiAddressSize;
    // How many bytes of data do we want to read back?
    UINT32 uiReadSize;
    // How many bytes of redundant data do we want to read back?
    //UINT32    uiRdntReadSize;
    // What is the word size 16 or 8 bits?
    UINT32 uiWordSize;
    //! Should ECC be enabled?
    BOOL enableECC;
    
    union
    {
        // 1 byte CLE, up to 4 bytes of Column & Row.
        UCHAR tx_cle1_addr_buf[MAX_COLUMNS+MAX_ROWS+1];
        struct
        {
            UCHAR tx_cle1;
            union
            {
                UCHAR tx_addr[MAX_COLUMNS+MAX_ROWS];
                // Type2 array has 2 Columns & 3 Rows.
                struct
                {
                    UCHAR bType2Columns[MAX_COLUMNS];
                    UCHAR bType2Rows[MAX_ROWS];
                };
                // Type1 array has 1 Column & up to 3 Rows
                struct
                {
                    UCHAR bType1Columns[1];
                    UCHAR bType1Rows[MAX_ROWS];
                };
            };
        };
    };

    // buffer for 'tx_cle2_dma'
    union
    {
        UCHAR tx_cle2_buf[1];
        struct
        {
            UCHAR tx_cle2;
        };
    };

    // Buffer pointer for data
    VOID * pDataBuffer;
    // Buffer pointer for Auxiliary data (Redundant and ECC)..
    VOID * pAuxBuffer;

} NAND_read_seed_t;

typedef struct _NandDmaProgSeed_t
{
    // Command 1 along with address.
    union
    {
        // 1 byte CLE, up to 4 bytes of Column & Row.
        UCHAR tx_cle1_addr_buf[MAX_COLUMNS+MAX_ROWS+1];
        struct
        {
            UCHAR tx_cle1;
            union
            {
                UCHAR tx_addr[MAX_COLUMNS+MAX_ROWS];
                // Type2 array has 2 Columns & 2 Rows.
                struct
                {
                    UCHAR bType2Columns[MAX_COLUMNS];
                    UCHAR bType2Rows[MAX_ROWS];
                };
                // Type1 array has 1 Column & up to 3 Rows
                struct
                {
                    UCHAR bType1Columns[1];
                    UCHAR bType1Rows[MAX_ROWS];
                };
            };
        };
    };

    // buffer for 'tx_cle2_dma'
    union
    {
        UCHAR tx_cle2_buf[1];
        struct
        {
            UCHAR tx_cle2;
        };
    };

    UINT32 uiAddressSize;
    
    UINT32 uiWriteSize;
    // Status variables for testing success of program.
    UINT8 u8StatusCmd;
    UINT16 u16Status;
    UINT32 u32EnableHwEcc;
    // Buffer pointer for data
    VOID * pDataBuffer;
    // Buffer pointer for Auxiliary data (Redundant and ECC)..
    VOID * pAuxBuffer;
} NandDmaProgSeed_t;

// dma chain structure for NAND Erase Block.
typedef struct _NAND_dma_block_erase_seed_t
{
    // Number of Block bytes to be sent.
    UINT32 uiBlockAddressBytes;

    // buffer for 'tx_cle1_row_dma'
    union
    {
        // CLE + Maximum Block bytes to send.
        UCHAR tx_cle1_block_buf[MAX_ROWS+1];
        struct
        {
            UCHAR tx_cle1;
            UCHAR tx_block[MAX_ROWS];
        };
    };

    // buffer for 'tx_cle2_dma'
    union
    {
        UCHAR tx_cle2_buf[1];
        struct
        {
            UCHAR tx_cle2;
        };
    };

    // Status variables for testing success of erase.
    UINT8 u8StatusCmd;
    UINT16 u16Status;
    UINT32 u32StatusMaskRef;
} NAND_dma_block_erase_seed_t;

///////////////////////////////////////////////////////////////////////
//!     DMA Descriptor Structures.
//!     These structures hold the descriptor chains processed by the
//!     DMA.
///////////////////////////////////////////////////////////////////////

// dma chain structure for Reset Device
typedef struct _NAND_dma_reset_device_t
{
    // descriptor sequence
    apbh_dma_gpmi1_t wait4rdy_dma;
    apbh_dma_gpmi1_t sense_rdy_dma;
    apbh_dma_gpmi3_t tx_dma;
    apbh_dma_gpmi1_t wait_dma;
    apbh_dma_gpmi1_t sense_dma;

    // terminator functions
    apbh_dma_t success_dma;
    apbh_dma_t timeout_dma;

    // Buffer for Reset Command.
    UCHAR tx_reset_command_buf[NAND_RESET_DEVICE_SIZE];
} NAND_dma_reset_device_t;

// dma chain structure for Read ID
typedef struct _NAND_dma_read_id_t
{
    // descriptor sequence
    apbh_dma_gpmi1_t wait4rdy_dma;
    apbh_dma_gpmi1_t sense_rdy_dma;

    // descriptor sequence
    apbh_dma_gpmi3_t tx_dma;
    apbh_dma_gpmi1_t rx_dma;

    // terminator functions
    apbh_dma_t success_dma;
    apbh_dma_t timeout_dma;

    // Insert ReadID seed into here.
    NAND_dma_read_id_seed_t NandReadIDSeed;

} NAND_dma_read_id_t ;


// dma chain structure for Read Page
typedef struct _NAND_dma_read_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t tx_cle1_addr_dma;
    apbh_dma_gpmi1_t tx_cle2_dma;
    apbh_dma_gpmi1_t wait_dma;
    apbh_dma_gpmi1_t sense_dma;
    apbh_dma_gpmi6_t rx_data_dma;
    apbh_dma_gpmi3_t rx_wait4done_dma;

    // terminator functions
    apbh_dma_t success_dma;
    apbh_dma_t timeout_dma;

    NAND_read_seed_t NAND_DMA_Read_Seed;

} NAND_dma_read_t;


// dma chain structure for NAND Program.
typedef struct _NAND_dma_program_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t tx_cle1_addr_dma;
    apbh_dma_gpmi6_t tx_data_dma;
    apbh_dma_gpmi3_t tx_cle2_dma;
    apbh_dma_gpmi1_t wait_dma;
    apbh_dma_t sense_dma;
    // CheckStatus.
    apbh_dma_gpmi3_t statustx_dma;
    apbh_dma_gpmi2_t statusrx_dma;
    //apbh_dma_gpmi2_t  statcmp_dma;
    apbh_dma_t statbranch_dma;

    // terminator functions
    apbh_dma_t success_dma;
    apbh_dma_t program_failed_dma;

    // The buffers needed by the DMA.
    NandDmaProgSeed_t NandProgSeed;

} NAND_dma_program_t;

// dma chain structure for NAND Program.
typedef struct _NAND_dma_wait_rdy_t
{
    // descriptor sequence
    apbh_dma_gpmi1_t wait_dma;
    apbh_dma_gpmi1_t sense_dma;

    // terminator functions
    apbh_dma_t success_dma;
    apbh_dma_t timeout_dma;

} NAND_dma_wait_rdy_t;

// dma chain structure for NAND Erase Block.
typedef struct _NAND_dma_block_erase_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t tx_cle1_row_dma;
    apbh_dma_gpmi1_t tx_cle2_dma;
    apbh_dma_gpmi1_t wait_dma;
    apbh_dma_gpmi1_t sense_dma;
    // CheckStatus.
    apbh_dma_gpmi1_t statustx_dma;
    apbh_dma_gpmi1_t statusrx_dma;
    //apbh_dma_gpmi2_t  statcmp_dma;
    apbh_dma_t statbranch_dma;

    // terminator functions
    apbh_dma_t success_dma;
    apbh_dma_t program_failed_dma;

    // The buffers needed by the DMA
    NAND_dma_block_erase_seed_t NandEraseSeed;

} NAND_dma_block_erase_t;

// dma chain structure for NAND Read Status.
typedef struct _NAND_dma_read_status_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t tx_dma;
    apbh_dma_gpmi1_t rx_dma;

    // terminator functions
    apbh_dma_t success_dma;
    apbh_dma_t timeout_dma;

    // Hold the command here.
    UCHAR CommandBuffer;

    // result
    UCHAR ReadStatusResult;

} NAND_dma_read_status_t;

// dma chain structure for NAND Check Status.
typedef struct _NAND_dma_check_status_t
{
    // descriptor sequence
    apbh_dma_gpmi1_t tx_dma;
    apbh_dma_gpmi2_t cmp_dma;
    apbh_dma_t branch_dma;

    // terminator functions
    apbh_dma_t success_dma;
    apbh_dma_t timeout_dma;
} NAND_dma_check_status_t;

typedef union _NAND_dma_generic_struct_t
{
    NAND_dma_reset_device_t NandReset;
    NAND_dma_read_id_t NandReadID;
    NAND_dma_wait_rdy_t NandWaitForReady;
    NAND_dma_read_status_t NandReadStatus;
    NAND_dma_check_status_t NandCheckStatus;
} NAND_dma_generic_struct_t;


////////////////////////////////////////////////////////////////////////////////
//! NAND Definitions
////////////////////////////////////////////////////////////////////////////////
#define NAND0_APBH_CH            4

////////////////////////////////////////////////////////////////////////////////
//! Externs
////////////////////////////////////////////////////////////////////////////////
extern NAND_dma_read_t  *DmaReadDescriptor;
extern NAND_dma_block_erase_t  *DmaEraseBlockDescriptor;
extern NAND_dma_program_t  *DmaProgramDescriptor;
extern NAND_dma_generic_struct_t  *DmaGenericDescriptor;

////////////////////////////////////////////////////////////////////////////////
//! Prototypes
////////////////////////////////////////////////////////////////////////////////
// DMA build functions.
VOID  DMA_SetupResetDesc(NAND_dma_reset_device_t* pChain, UINT32 u32ChipSelect);
VOID  DMA_SetupReadIdDesc(NAND_dma_read_id_t* pChain, UINT32 u32ChipSelect, VOID*  pReadIDAddress, VOID* pReadIDResultBuffer);
VOID  DMA_SetupReadStatusDesc(NAND_dma_read_status_t* pChain, UINT32 u32ChipSelect, VOID* pBuffer);
VOID  DMA_ModifyReadDesc(NAND_dma_read_t* pChain, UINT32 u32ChipSelect, NAND_read_seed_t * pReadSeed);
VOID  DMA_ModifyWriteDesc(NAND_dma_program_t *pChain, UINT32 u32ChipSelect, NandDmaProgSeed_t   *pWriteSeed);
VOID  DMA_ModifyEraseDesc(NAND_dma_block_erase_t  *pChain, UINT32 u32BlockAddressBytes, UINT32 u32ChipSelect);
// DMA Utilities.
VOID DMA_Start(dma_cmd_t * pDmaCmd, UINT32 u32NANDDeviceNum);
BOOL DMA_Wait(UINT32 usec, UINT32 wNANDDeviceNum);
VOID DMA_Init(UINT32 u32NumAddressBytes);

ULONG DMA_MemAlloc(ULONG size);
VOID  DMA_MemFree(ULONG phys_addr);
VOID* DMA_MemTransfer(VOID* phys_addr);

#endif // _NAND_DMA_H
