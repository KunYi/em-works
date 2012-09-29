//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dma_descriptor.c
//
//  This file contains a Routines for building the NAND DMA descriptors.
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include "nand_dma.h"
#include "nand_hal.h"
#include "nand_ecc.h"
#pragma warning(pop)
#include "dma_descriptor.h"

VOID  DMA_ModifyWriteDesc(
    NAND_dma_program_t  *pChain,
    UINT32              u32ChipSelect,
    NandDmaProgSeed_t   *pWriteSeed)
{
    UINT32 iCLE1_Size;
    
    // CLE1 chain size is # columns + # Rows + CLE command.
    iCLE1_Size = pWriteSeed->uiAddressSize + 1;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor1: Issue NAND write setup command
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->tx_cle1_addr_dma.cmd.U = NAND_DMA_COMMAND_CMD(iCLE1_Size,0,NAND_LOCK,3);
    pChain->tx_cle1_addr_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                                 iCLE1_Size, BV_GPMI_CTRL0_ADDRESS_INCREMENT__ENABLED, ASSERT_CS);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor2: write the data payload
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if(pWriteSeed->u32EnableHwEcc)
    {
        pChain->tx_data_dma.cmd.U = NAND_DMA_TXDATA_CMD(0, 0, 6, 1, NO_DMA_XFER);
        pChain->tx_data_dma.gpmi_ctrl0.U = NAND_DMA_TXDATA_PIO(u32ChipSelect,
                                                                   BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, 0);
        pChain->tx_data_dma.gpmi_eccctrl.U = NAND_DMA_ECC_CTRL_PIO(0x1ff, BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_8_BIT);                                                           
        pChain->tx_data_dma.gpmi_ecccount.U = pWriteSeed->uiWriteSize;
        // Setup the data buffer.
        pChain->tx_data_dma.gpmi_payload.U = ((DWORD)DMA_MemTransfer(pWriteSeed->pDataBuffer) & 0xFFFFFFFC);
        // And the Auxiliary buffer here.
        pChain->tx_data_dma.gpmi_auxiliary.U = ((DWORD)DMA_MemTransfer(pWriteSeed->pAuxBuffer) & 0xFFFFFFFC);
    
        // Set Buffer Address Register to WriteBuffer.
        pChain->tx_data_dma.bar = DMA_MemTransfer(pWriteSeed->pDataBuffer);
    }
    else
    {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Descriptor2/3: write the data & aux payload
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // if not using BCH, the chain will be minor difference.
        pChain->tx_data_dma.cmd.U = NAND_DMA_TXDATA_CMD(pWriteSeed->uiWriteSize, 0, 4, 1, DMA_READ);
        pChain->tx_data_dma.gpmi_ctrl0.U = NAND_DMA_TXDATA_PIO(u32ChipSelect,
                                                               BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, pWriteSeed->uiWriteSize);
        pChain->tx_data_dma.bar = DMA_MemTransfer(pWriteSeed->pDataBuffer);
        // Compare isn't used.
        pChain->tx_data_dma.gpmi_compare.U = 0;
        pChain->tx_data_dma.gpmi_eccctrl.U = 0;
        pChain->tx_data_dma.gpmi_ecccount.U = 0;
    }
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor3: issue NAND write execute command
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->tx_cle2_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                            1, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor4: wait for ready
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->wait_dma.gpmi_ctrl0.U = NAND_DMA_WAIT4RDY_PIO(u32ChipSelect);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor5: psense compare
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor6: issue NAND status command
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->statustx_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                             NAND_READ_STATUS_SIZE, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor7: read status and compare
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->statusrx_dma.gpmi_ctrl0.U = NAND_DMA_RX_PIO(u32ChipSelect,
                                                        BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, NAND_READ_STATUS_RESULT_SIZE);
}

VOID  DMA_ModifyEraseDesc(
    NAND_dma_block_erase_t  *pChain,
    UINT32 u32BlockAddressBytes,
    UINT32 u32ChipSelect)
{
    // CLE1 chain size is # Blocks + CLE command.
    UINT32 iCLE1_Size = u32BlockAddressBytes + 1;
    
    pChain->tx_cle1_row_dma.cmd.U = NAND_DMA_COMMAND_CMD(iCLE1_Size, 0, NAND_LOCK,3);
    
    pChain->tx_cle1_row_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                                iCLE1_Size, BV_GPMI_CTRL0_ADDRESS_INCREMENT__ENABLED, ASSERT_CS);
    
    pChain->tx_cle2_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                            1, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);

    pChain->wait_dma.gpmi_ctrl0.U = NAND_DMA_WAIT4RDY_PIO(u32ChipSelect);

    pChain->statustx_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                             NAND_READ_STATUS_SIZE, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);

    pChain->statusrx_dma.gpmi_ctrl0.U = NAND_DMA_RX_PIO(u32ChipSelect,
                                                        BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, NAND_READ_STATUS_RESULT_SIZE);
}



////////////////////////////////////////////////////////////////////////////////
//! \brief      Build the Read Status DMA descriptor for the NAND.
//!
//! \fntype     Non-Reentrant
//!
//! Build descriptor to read the status.
//!
//! \param[in]  pChain - pointer to the descriptor chain that gets filled.
//! \param[in]  u32ChipSelect - Chip Select - NANDs 0-3.
//! \param[out] pBuffer - Word to put resulting status into.
//!
//! \note       branches to TRUE DMA upon completion.
//!             Command is held in separate structure because 0x70 (Status)
//!             or 0x71 (Cache Status) may be used.
//!
////////////////////////////////////////////////////////////////////////////////
VOID  DMA_SetupReadStatusDesc(
    NAND_dma_read_status_t* pChain,
    UINT32 u32ChipSelect,
    VOID* pBuffer)
{
    pChain->tx_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->rx_dma);
    //pChain->tx_dma.nxt = &pPhyDmaDescriptor[1];
    // Configure APBH DMA to push CheckStatus command (toggling CLE)
    // into GPMI_CTRL.
    // Transfer NAND_READ_STATUS_SIZE (1) bytes to GPMI when GPMI ready.
    // Wait for end command from GPMI before next part of chain.
    // Lock GPMI to this NAND during transfer.
    // DMA_READ - Perform PIO word transfers then transfer
    //            from memory to peripheral for specified # of bytes.
    pChain->tx_dma.cmd.U = NAND_DMA_COMMAND_CMD(NAND_READ_STATUS_SIZE,0,NAND_LOCK,3);

    // Point to byte where NAND Read Status Command is kept.
    //pChain->tx_dma.bar = pPhysStatusCmd;
    pChain->tx_dma.bar = DMA_MemTransfer(&pChain->CommandBuffer);
    // Setup GPMI bus for first part of Read Status Command.  Need to
    // set CLE high, then send Read Status command (0x70/71), then
    // clear CLE.
    pChain->tx_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                       NAND_READ_STATUS_SIZE,BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);

    // Set compare to NULL.
    pChain->tx_dma.gpmi_compare.U = (UINT)NULL;
    // Disable the ECC.
    pChain->tx_dma.gpmi_eccctrl.U = NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next dma chain is TRUE.
    //pChain->rx_dma.nxt = (apbh_dma_gpmi1_t*)&APBH_success_dma;
    pChain->rx_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->success_dma);
    // Read back 1 word.
    //pChain->rx_dma.cmd.U = NAND_DMA_RX_CMD(NAND_READ_STATUS_RESULT_SIZE,
    //                                           DECR_SEMAPHORE);
    pChain->rx_dma.cmd.U = NAND_DMA_RX_NO_ECC_CMD(NAND_READ_STATUS_RESULT_SIZE, 0);
    // Put result into pBuffer.
    //pChain->rx_dma.bar = pPhyData;
    pChain->rx_dma.bar = DMA_MemTransfer(pBuffer);
    // Read NAND_STATUS_SIZE bytes from GPMI.
    pChain->rx_dma.gpmi_ctrl0.U = NAND_DMA_RX_PIO(u32ChipSelect,
                                                  BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, NAND_READ_STATUS_RESULT_SIZE);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Initialize the Terminator functions
    // Next function is null.
    pChain->success_dma.nxt = (apbh_dma_t*) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    pChain->success_dma.cmd.U = ((UINT)
                                           (BF_APBH_CHn_CMD_IRQONCMPLT(1) | \
                                           BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
                                           BF_APBH_CHn_CMD_SEMAPHORE(1) | \
                                           BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to TRUE termination code.
    pChain->success_dma.bar = (VOID *) TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//! \brief      Build the Reset Device descriptor for the NAND.
//!
//! \fntype     Non-Reentrant
//!
//! Resets NAND by sending appropriate Reset sequence.  Currently, the
//! Reset command is the same for all NANDs.
//!
//! \param[in]  pChain - pointer to the descriptor chain that gets filled.
//! \param[in]  u32ChipSelect - Chip Select - NANDs 0-3.
//!
//! \note       Branches to Timeout or Success.
////////////////////////////////////////////////////////////////////////////////
VOID  DMA_SetupResetDesc(
    NAND_dma_reset_device_t         *pChain,
    UINT32 u32ChipSelect)
{
    // First we want to wait for Ready.  The chip may be busy on power-up.
    // Wait for Ready.
    pChain->wait4rdy_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->sense_rdy_dma);

    pChain->wait4rdy_dma.cmd.U = NAND_DMA_WAIT4RDY_CMD;
    // BAR points to alternate branch if timeout occurs.
    pChain->wait4rdy_dma.bar = (apbh_dma_gpmi1_t*)0x00;
    // Set GPMI wait for ready.
    pChain->wait4rdy_dma.gpmi_ctrl0.U = NAND_DMA_WAIT4RDY_PIO(u32ChipSelect);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Now check for successful Ready.

    pChain->sense_rdy_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->tx_dma);
    // Do not decrement semaphore - not the last command.
    pChain->sense_rdy_dma.cmd.U = NAND_DMA_SENSE_CMD(0);
    // BAR points to alternate branch if timeout occurs.
    pChain->sense_rdy_dma.bar = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->timeout_dma);
    // Even though PIO is unused, set it to zero for comparison purposes.
    pChain->sense_rdy_dma.gpmi_ctrl0.U = 0;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next command will be a wait.
    pChain->tx_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->wait_dma);
    // Configure APBH DMA for NAND Reset command.

    pChain->tx_dma.cmd.U = NAND_DMA_COMMAND_CMD(NAND_RESET_DEVICE_SIZE,0,NAND_LOCK,3);

    // Buffer Address Register being used to hold command.
    pChain->tx_dma.bar = DMA_MemTransfer(pChain->tx_reset_command_buf);
    // Setup GPMI bus for Reset Command.  Need to set CLE high, then
    // low, then ALE toggles high and low.  (Actually, in this case
    // ALE toggling probably isn't necessary)
    pChain->tx_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(
        u32ChipSelect, NAND_RESET_DEVICE_SIZE,0,ASSERT_CS);

    // Set compare to NULL.
    pChain->tx_dma.gpmi_compare.U = (reg32_t)NULL;
    // Disable the ECC.
    pChain->tx_dma.gpmi_eccctrl.U = NAND_DMA_ECC_PIO(
        BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Setup 2nd complete DMA sequence.
    // Wait for Ready.
    pChain->wait_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->sense_dma);
    pChain->wait_dma.cmd.U = NAND_DMA_WAIT4RDY_CMD;
    // BAR points to alternate branch if timeout occurs.
    pChain->wait_dma.bar = (apbh_dma_gpmi1_t*)0x00;
    // Set GPMI wait for ready.
    pChain->wait_dma.gpmi_ctrl0.U = NAND_DMA_WAIT4RDY_PIO(u32ChipSelect);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Now check for success.
    pChain->sense_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->success_dma);
    // Decrement semaphore.
    //pChain->sense_dma.cmd.U = NAND_DMA_SENSE_CMD(DECR_SEMAPHORE);
    pChain->sense_dma.cmd.U = NAND_DMA_SENSE_CMD(0);
    // BAR points to alternate branch if timeout occurs.
    pChain->sense_dma.bar = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->timeout_dma);
    // Even though PIO is unused, set it to zero for comparison purposes.
    pChain->sense_dma.gpmi_ctrl0.U = 0;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Initialize the Terminator functions
    // Next function is null.
    pChain->success_dma.nxt = (apbh_dma_t*) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    pChain->success_dma.cmd.U = ((reg32_t)
                                           (BF_APBH_CHn_CMD_IRQONCMPLT(1) | \
                                           BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
                                           BF_APBH_CHn_CMD_SEMAPHORE(1) | \
                                           BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to success termination code.
    pChain->success_dma.bar = (VOID *) TRUE;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next function is null.
    pChain->timeout_dma.nxt = (apbh_dma_t*) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    pChain->timeout_dma.cmd.U = ((reg32_t)
                                           (BF_APBH_CHn_CMD_IRQONCMPLT(1) | \
                                           BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
                                           BF_APBH_CHn_CMD_SEMAPHORE(1) | \
                                           BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to timeout termination code.
    pChain->timeout_dma.bar = (VOID *) FALSE;
}


////////////////////////////////////////////////////////////////////////////////
//! \brief Build the abbreviated DMA to send a NAND Read Command to the device.
//!
//! This function builds the DMA descriptor for a NAND Read command to the
//! device.  This function assumes the DMA has already been setup once so
//! only the parameters that change need to be updated.
//!
//! \param[in]  pChain Pointer to the DMA Chain.
//! \param[in]  u32ChipSelect Physical NAND number to read.
//! \param[in]  pReadSeed Pointer to structure containing the details of the
//!             NAND Read Command (size, address, etc).
//!
//! \return VOID
//!
//! \internal
////////////////////////////////////////////////////////////////////////////////
VOID  DMA_ModifyReadDesc(
    NAND_dma_read_t         *pChain,
    UINT32 u32ChipSelect,
    NAND_read_seed_t        *pReadSeed)
{
    pChain->tx_cle1_addr_dma.gpmi_ctrl0.B.CS = u32ChipSelect;
    pChain->tx_cle2_dma.gpmi_ctrl0.B.CS = u32ChipSelect;
    pChain->wait_dma.gpmi_ctrl0.B.CS = u32ChipSelect;

    if (pReadSeed->enableECC)
    {
        pChain->rx_data_dma.cmd.U = NAND_DMA_RX_CMD_ECC(0, 0);
        pChain->rx_data_dma.bar = 0x00;           // This field isn't used.
        if(pReadSeed->uiReadSize > 512)
        {   
            pChain->rx_data_dma.gpmi_eccctrl.U = NAND_DMA_ECC_CTRL_PIO(
                                            0x1ff,
                                            BV_GPMI_ECCCTRL_ECC_CMD__DECODE_8_BIT);
        }
        else
        {
            pChain->rx_data_dma.gpmi_eccctrl.U = NAND_DMA_ECC_CTRL_PIO(
                                            0x100,
                                            BV_GPMI_ECCCTRL_ECC_CMD__DECODE_8_BIT);
        }
        
        pChain->rx_data_dma.gpmi_ecccount.U = (UINT16)(pReadSeed->uiReadSize & 0xffff);
    }
    else
    {
        // ECC is disabled.
        pChain->rx_data_dma.cmd.U = NAND_DMA_RX_NO_ECC_CMD(pReadSeed->uiReadSize, 0);
        pChain->rx_data_dma.cmd.B.XFER_COUNT = pReadSeed->uiReadSize;
        pChain->rx_data_dma.bar = DMA_MemTransfer(pReadSeed->pDataBuffer);
        pChain->rx_data_dma.gpmi_eccctrl.U = 0;
        pChain->rx_data_dma.gpmi_ecccount.U = 0;
    }
    
    // Setup the data buffer.
    pChain->rx_data_dma.gpmi_payload.U = (((UINT32)DMA_MemTransfer(pReadSeed->pDataBuffer)) & 0xFFFFFFFC);
    // And the Auxiliary buffer here.
    pChain->rx_data_dma.gpmi_auxiliary.U = (((UINT32)DMA_MemTransfer(pReadSeed->pAuxBuffer)) & 0xFFFFFFFC);
    // Change only those values that need to be changed.
    pChain->rx_data_dma.gpmi_ctrl0.B.CS = u32ChipSelect;
    pChain->rx_data_dma.gpmi_ctrl0.B.XFER_COUNT = pReadSeed->uiReadSize;

    // Disable the Chip Select and other outstanding GPMI things.
    pChain->rx_wait4done_dma.gpmi_ctrl0.B.CS = u32ChipSelect;

    pChain->rx_data_dma.gpmi_compare.U = 0;
    
}

////////////////////////////////////////////////////////////////////////////////
//! \brief      Build the Read ID descriptor for the NAND.
//!
//! \fntype     Non-Reentrant
//!
//! Reads the ID of the NAND by sending the Read ID sequence.  Currently, six
//! bytes are read from the NAND.
//!
//! \param[in]  pChain - pointer to the descriptor chain that gets filled.
//! \param[in]  u32ChipSelect - Chip Select - NANDs 0-3.
//! \param[out] pReadIDAddress TBD
//! \param[out] pReadIDResultBuffer - Placeholder for Read ID Result.
//!
//! \note       Branches to Success DMA upon completion.
//!
//! \todo [PUBS] Define TBD parameter(s)
////////////////////////////////////////////////////////////////////////////////
VOID  DMA_SetupReadIdDesc(NAND_dma_read_id_t* pChain,
                             UINT32 u32ChipSelect,
                             VOID*  pReadIDAddress,
                             VOID*  pReadIDResultBuffer)
{
    // First we want to wait for Ready.  The chip may be busy on power-up.
    // Wait for Ready.
    pChain->wait4rdy_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->sense_rdy_dma);
    pChain->wait4rdy_dma.cmd.U = NAND_DMA_WAIT4RDY_CMD;

    // BAR points to alternate branch if timeout occurs.
    pChain->wait4rdy_dma.bar = (apbh_dma_gpmi1_t*)0x00;
    // Set GPMI wait for ready.
    pChain->wait4rdy_dma.gpmi_ctrl0.U = NAND_DMA_WAIT4RDY_PIO(u32ChipSelect);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Now check for successful Ready.
    pChain->sense_rdy_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->tx_dma);
    pChain->sense_rdy_dma.cmd.U = NAND_DMA_SENSE_CMD(0);
    // BAR points to alternate branch if timeout occurs.
    pChain->sense_rdy_dma.bar = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->timeout_dma);
    // Even though PIO is unused, set it to zero for comparison purposes.
    pChain->sense_rdy_dma.gpmi_ctrl0.U = 0;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next command in chain will be a read.
    pChain->tx_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->rx_dma);
    // Configure APBH DMA to push Read ID command (toggling CLE & ALE)
    // into GPMI_CTRL.
    // Transfer NAND_READ_ID_SIZE to GPMI when GPMI ready.
    // Transfer 1 word to GPMI_CTRL0 (see command below)
    // Wait for end command from GPMI before rx part of chain.
    // Lock GPMI to this NAND during transfer.
    // DMA_READ - Perform PIO word transfers then transfer
    //            from memory to peripheral for specified # of bytes.

    pChain->tx_dma.cmd.U = NAND_DMA_COMMAND_CMD(NAND_READ_ID_SIZE, 0, NAND_LOCK,3);

    // Buffer Address Register being used to hold Read ID command.
    //pChain->tx_dma.bar = pPhysAddr;  //FIXME - ReadID is packed inside chain.
    pChain->tx_dma.bar = DMA_MemTransfer(pReadIDAddress);
    // Setup GPMI bus for Read ID Command.  Need to set CLE high, then
    // low, then ALE toggles high and low.  Read ID Code sent during
    // CLE, 2nd byte (0x00) sent during ALE.
    pChain->tx_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                       NAND_READ_ID_SIZE, BV_GPMI_CTRL0_ADDRESS_INCREMENT__ENABLED, ASSERT_CS);


    // Nothing needs to happen to the compare.
    pChain->tx_dma.gpmi_compare.U = (reg32_t) NULL;
    // Disable the ECC.
    pChain->tx_dma.gpmi_eccctrl.U = NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Setup 2nd complete DMA sequence.
    // Setup to use SUCESS DMA sequence if successful.
    pChain->rx_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->success_dma);
    // Configure APBH DMA to push Read ID command
    // into GPMI_CTRL.
    // Transfer NAND_READ_ID_SIZE to GPMI when GPMI ready.
    // Transfer 1 word to GPMI_CTRL0 (see command below)
    // Wait for end command from GPMI before rx part of chain.
    // Lock GPMI to this NAND during transfer.
    // DMA_WRITE - Perform PIO word transfers then transfer to
    //             memory from peripheral for specified # of bytes.
    //pChain->rx_dma.cmd.U = NAND_DMA_RX_CMD(NAND_READ_ID_RESULT_SIZE,
    //                                       DECR_SEMAPHORE);  FIXME
    pChain->rx_dma.cmd.U = NAND_DMA_RX_NO_ECC_CMD(NAND_READ_ID_RESULT_SIZE, 0);
    // Buffer Address Register being used to hold Read ID result.
    pChain->rx_dma.bar = DMA_MemTransfer(pReadIDResultBuffer);
    // Setup GPMI bus for Read ID Result.  Read data back in.
    // Read RESULT_SIZE bytes (8 bit) data
    pChain->rx_dma.gpmi_ctrl0.U = NAND_DMA_RX_PIO(u32ChipSelect,
                                                  BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, NAND_READ_ID_RESULT_SIZE);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Initialize the Terminator functions
    // Next function is null.
    pChain->success_dma.nxt = (apbh_dma_t*) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    pChain->success_dma.cmd.U = ((reg32_t)
                                           (BF_APBH_CHn_CMD_IRQONCMPLT(1) | \
                                           BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
                                           BF_APBH_CHn_CMD_SEMAPHORE(1) | \
                                           BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to success termination code.
    pChain->success_dma.bar = (VOID *) TRUE;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next function is null.
    pChain->timeout_dma.nxt = (apbh_dma_t*) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    pChain->timeout_dma.cmd.U = ((reg32_t)
                                           (BF_APBH_CHn_CMD_IRQONCMPLT(1) | \
                                           BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
                                           BF_APBH_CHn_CMD_SEMAPHORE(1) | \
                                           BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to timeout termination code.
    pChain->timeout_dma.bar = (VOID *) FALSE;
}
