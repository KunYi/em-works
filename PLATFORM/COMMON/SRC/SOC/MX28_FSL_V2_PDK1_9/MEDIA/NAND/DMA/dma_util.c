//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dma_util.c
//
//  This file contains private dma functions used for NAND operations
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
#include "csp.h"
#include "dma_descriptor.h"
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

NAND_Misc_Struct zNAND_Misc[MAX_NAND_DEVICES];

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//! \brief      Build the Erase Block DMA descriptor for the NAND.
//!
//! \fntype     Non-Reentrant
//!
//! Build descriptor to erase the block.  This is a synchronous call.
//!
//! \param[in]  pChain - pointer to the descriptor chain that gets filled.
//! \param[in]  u32BlockAddressBytes TBD
//! \param[in]  u32ChipSelect - Chip Select - NANDs 0-3.
//!
//! \note       branches to TRUE DMA upon completion.
//!
//! \todo [PUBS] Define TBD parameter(s)
////////////////////////////////////////////////////////////////////////////////
VOID  SetupDMAEraseDesc(
    NAND_dma_block_erase_t  *pChain,
    UINT32 u32BlockAddressBytes,
    UINT32 u32ChipSelect)
{
    // CLE1 chain size is # Blocks + CLE command.
    UINT32 iCLE1_Size = u32BlockAddressBytes + 1;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Point to next command.
    pChain->tx_cle1_row_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->tx_cle2_dma);
    //pChain->tx_cle1_row_dma.nxt = &pPhyDmaDescriptor[1];
    // Configure APBH DMA to push Erase command (toggling CLE)
    // into GPMI_CTRL.
    // Transfer CLE1_SIZE (3) bytes to GPMI when GPMI ready.
    // Transfer CLE1 and the row address bytes to GPMI_CTRL0.
    // Wait for end command from GPMI before next part of chain.
    // Lock GPMI to this NAND during transfer.
    // DMA_READ - Perform PIO word transfers then transfer
    //            from memory to peripheral for specified # of bytes.
    pChain->tx_cle1_row_dma.cmd.U = NAND_DMA_COMMAND_CMD(iCLE1_Size, 0, NAND_LOCK,3);
    // Buffer Address Register holds Erase Block command and address.
    pChain->tx_cle1_row_dma.bar = DMA_MemTransfer(&pChain->NandEraseSeed.tx_cle1_block_buf);
    //pChain->tx_cle1_row_dma.bar = pPhysCmdAddr;
    // Setup GPMI bus for first part of Write Command.  Need to set CLE
    // high, then send Write command (0x80), then clear CLE, set ALE high
    // send 4 address bytes.
    pChain->tx_cle1_row_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                                iCLE1_Size, BV_GPMI_CTRL0_ADDRESS_INCREMENT__ENABLED, ASSERT_CS);

    // Set compare to NULL.
    pChain->tx_cle1_row_dma.gpmi_compare.U = (UINT)NULL;
    // Disable the ECC.
    pChain->tx_cle1_row_dma.gpmi_eccctrl.U = NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Setup 2nd transfer.
    // Setup next command - wait.
    pChain->tx_cle2_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->wait_dma);
    //pChain->tx_cle2_dma.nxt = &pPhyDmaDescriptor[2];
    // Configure APBH DMA to push 2nd Erase command (toggling CLE)
    // into GPMI_CTRL.
    // Transfer CLE2_SIZE (1) bytes to GPMI when GPMI ready.
    // Transfer CLE2 byte to GPMI_CTRL0 (see command below)
    // Wait for end command from GPMI before next part of chain.
    // Lock GPMI to this NAND during transfer.
    // DMA_READ - Perform PIO word transfers then transfer
    //            from memory to peripheral for specified # of bytes.
    pChain->tx_cle2_dma.cmd.U = NAND_DMA_COMMAND_CMD(1, 0, NAND_LOCK,1);
    // Buffer Address Register holds tx_cle2 command
    pChain->tx_cle2_dma.bar = DMA_MemTransfer(&pChain->NandEraseSeed.tx_cle2_buf);
    //pChain->tx_cle2_dma.bar = pPhyCmd2;
    // Setup GPMI bus for second part of Erase Command.  Need to set CLE
    // high, then send Erase2 command (0xD0), then clear CLE.
    pChain->tx_cle2_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                            1, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Setup Wait for Ready descriptor.
    // Setup TRUE DMA pointer.
    pChain->wait_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->sense_dma);
    //pChain->wait_dma.nxt = &pPhyDmaDescriptor[3];
    // Setup Wait for Ready (No transfer count)
    pChain->wait_dma.cmd.U = NAND_DMA_WAIT4RDY_CMD;
    // If there is an error, load Timeout DMA sequence.
    pChain->wait_dma.bar = 0x0;
    // Wait for Ready to go high.
    pChain->wait_dma.gpmi_ctrl0.U = NAND_DMA_WAIT4RDY_PIO(u32ChipSelect);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Now use Sense sequence to wait for ready.
    pChain->sense_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->statustx_dma);
    //pChain->sense_dma.nxt = (apbh_dma_gpmi1_t*)&pPhyDmaDescriptor[4];
    // Wait for Ready (No transfer count)- Decrement semaphore.
    //pChain->sense_dma.cmd.U = NAND_DMA_SENSE_CMD(DECR_SEMAPHORE); FIXME
    pChain->sense_dma.cmd.U = NAND_DMA_SENSE_CMD(0);
    // If failure occurs, branch to pTimeout DMA.
    //pChain->sense_dma.bar = (apbh_dma_gpmi1_t*)&APBH_PROGRAM_FAILED_DMA;
    pChain->sense_dma.bar = (apbh_dma_t*)DMA_MemTransfer(&pChain->program_failed_dma);
    // Even though PIO is unused, set it to zero for comparison purposes.
    pChain->sense_dma.gpmi_ctrl0.U = 0;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next link will Read Status.
    pChain->statustx_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->statusrx_dma);
    //pChain->statustx_dma.nxt = &pPhyDmaDescriptor[5];
    // Configure APBH DMA to push CheckStatus command (toggling CLE)
    // into GPMI_CTRL.
    // Transfer NAND_READ_STATUS_SIZE (1) bytes to GPMI when GPMI ready.
    // Wait for end command from GPMI before next part of chain.
    // Lock GPMI to this NAND during transfer.
    // DMA_READ - Perform PIO word transfers then transfer
    //            from memory to peripheral for specified # of bytes.
    pChain->statustx_dma.cmd.U = NAND_DMA_COMMAND_CMD(NAND_READ_STATUS_SIZE,0, NAND_LOCK,1);
    // Point to structure where NAND Read Status Command is kept.
    pChain->statustx_dma.bar = DMA_MemTransfer(&pChain->NandEraseSeed.u8StatusCmd);
    //pChain->statustx_dma.bar = pPhyStatusCmd;
    // Setup GPMI bus for first part of Read Status Command.  Need to
    // set CLE high, then send Read Status command (0x70/71), then
    // clear CLE.
    pChain->statustx_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                             NAND_READ_STATUS_SIZE, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next Link determines TRUE or FAILURE.
    pChain->statusrx_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->statbranch_dma);
    //pChain->statusrx_dma.nxt = (apbh_dma_gpmi1_t*) &pPhyDmaDescriptor[6];
    // Send a Read & Compare command to the NAND.
    pChain->statusrx_dma.cmd.U = NAND_DMA_RX_NO_ECC_CMD(NAND_READ_STATUS_RESULT_SIZE, 0);
    // No DMA Transfer.
    pChain->statusrx_dma.bar = DMA_MemTransfer(&pChain->NandEraseSeed.u16Status);
    // GPMI commands.
    pChain->statusrx_dma.gpmi_ctrl0.U = NAND_DMA_RX_PIO(u32ChipSelect,
                                                        BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, NAND_READ_STATUS_RESULT_SIZE);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Branch to appropriate result DMA.
    //pChain->statbranch_dma.nxt = (apbh_dma_t*)&APBH_success_dma;
    pChain->statbranch_dma.nxt = (apbh_dma_t*)DMA_MemTransfer(&pChain->success_dma);
    // Based upon above Compare.
    //pChain->branch_dma.cmd.U = NAND_DMA_SENSE_CMD(DECR_SEMAPHORE);
    pChain->statbranch_dma.cmd.U = NAND_DMA_SENSE_CMD(0);
    // Failure.
    //pChain->sense_dma.bar = (apbh_dma_gpmi1_t*)&APBH_PROGRAM_FAILED_DMA;
    pChain->sense_dma.bar = (apbh_dma_t*)DMA_MemTransfer(&pChain->program_failed_dma);
    // Even though PIO is unused, set it to zero for comparison purposes.
    pChain->sense_dma.gpmi_ctrl0.U = 0;

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

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next function is null.
    pChain->program_failed_dma.nxt = (apbh_dma_t*) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    pChain->program_failed_dma.cmd.U = ((UINT)
                                           (BF_APBH_CHn_CMD_IRQONCMPLT(1) | \
                                           BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
                                           BF_APBH_CHn_CMD_SEMAPHORE(1) | \
                                           BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to timeout termination code.
    pChain->program_failed_dma.bar = (VOID *) FALSE;
}
////////////////////////////////////////////////////////////////////////////////
//! \brief      Build the Complete Read descriptor for the NAND.
//!
//! \fntype     Non-Reentrant
//!
//! Descriptor to read data from the NAND.  This may be either a full Sector
//! read (2112 bytes) or a partial sector read depending upon the size passed
//! in with the pReadSeed structure.
//!
//! \param[in]  pChain - pointer to the descriptor chain that gets filled.
//! \param[in]  u32ChipSelect - Chip Select - NANDs 0-3.
//! \param[in]  u32AddressSize TBD
//! \param[in]  u32DataSize TBD
//! \param[in]  pDataBuffer TBD
//!
//! \note       branches to pTimeout or pTRUE DMA upon completion.
//!             2048 bytes + 7 bytes of RA + 21 bytes extra + 36 bytes ECC
//!
//! \todo [PUBS] Define TBD parameter(s)
////////////////////////////////////////////////////////////////////////////////
VOID  SetupDMAReadDesc(
    NAND_dma_read_t *pChain,
    UINT32 u32ChipSelect,
    UINT32 u32AddressSize,
    UINT32 u32DataSize,
    VOID   *pDataBuffer)
{
    // CLE1 chain size is # columns + # Rows + CLE command.
    UINT32 iCLE1_Size = u32AddressSize + 1;

    UNREFERENCED_PARAMETER(u32DataSize);
    UNREFERENCED_PARAMETER(pDataBuffer);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor1: Issue NAND read setup command
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->tx_cle1_addr_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->tx_cle2_dma);
    pChain->tx_cle1_addr_dma.cmd.U = NAND_DMA_COMMAND_CMD(iCLE1_Size, 0, NAND_LOCK, 3);
    pChain->tx_cle1_addr_dma.bar = DMA_MemTransfer(pChain->NAND_DMA_Read_Seed.tx_cle1_addr_buf);
    pChain->tx_cle1_addr_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                            iCLE1_Size, BV_GPMI_CTRL0_ADDRESS_INCREMENT__ENABLED, ASSERT_CS);
    pChain->tx_cle1_addr_dma.gpmi_compare.U = (reg32_t)NULL;
    pChain->tx_cle1_addr_dma.gpmi_eccctrl.U = NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor2: Issue NAND read execute command
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->tx_cle2_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->wait_dma);
    pChain->tx_cle2_dma.cmd.U = NAND_DMA_COMMAND_CMD(1, 0, NAND_LOCK, 1);
    pChain->tx_cle2_dma.bar = DMA_MemTransfer(pChain->NAND_DMA_Read_Seed.tx_cle2_buf);
    pChain->tx_cle2_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                       1, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor3: wait for ready
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->wait_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->sense_dma);
    pChain->wait_dma.cmd.U = NAND_DMA_WAIT4RDY_CMD;
    pChain->wait_dma.bar = NULL;
    pChain->wait_dma.gpmi_ctrl0.U = NAND_DMA_WAIT4RDY_PIO(u32ChipSelect);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor4: psense compare
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->sense_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->rx_data_dma);
    pChain->sense_dma.cmd.U = NAND_DMA_SENSE_CMD(0);
    pChain->sense_dma.bar = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->timeout_dma);
    pChain->sense_dma.gpmi_ctrl0.U = 0;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor5: read 4K page plus 65 byte meta-data NAND data
    //              and send it to ECC block
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->rx_data_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->rx_wait4done_dma);
    if (pChain->NAND_DMA_Read_Seed.enableECC)
    {
        pChain->rx_data_dma.cmd.U = NAND_DMA_RX_CMD_ECC(0, 0);
        pChain->rx_data_dma.bar = 0x00;           // This field isn't used.
        pChain->rx_data_dma.gpmi_compare.U = 0x00; // This field isn't used.
        pChain->rx_data_dma.gpmi_eccctrl.U = NAND_DMA_ECC_CTRL_PIO(0, BV_GPMI_ECCCTRL_ECC_CMD__DECODE_8_BIT);
        pChain->rx_data_dma.gpmi_ecccount.B.COUNT = (reg16_t)BF_GPMI_ECCCOUNT_COUNT(pChain->NAND_DMA_Read_Seed.uiReadSize);
    }
    else
    {
        pChain->rx_data_dma.cmd.U = NAND_DMA_RX_NO_ECC_CMD(pChain->NAND_DMA_Read_Seed.uiReadSize, 0);
        pChain->rx_data_dma.bar = (VOID *)(((UINT32)pChain->NAND_DMA_Read_Seed.pDataBuffer) & 0xFFFFFFFC); // not sure if this is right...
        pChain->rx_data_dma.gpmi_compare.U = 0x00;    // This field isn't used.
        pChain->rx_data_dma.gpmi_eccctrl.U = 0;    // This field isn't used.
        pChain->rx_data_dma.gpmi_ecccount.U = 0;    // This field isn't used.
    }
    // Setup the data buffer.
    pChain->rx_data_dma.gpmi_payload.U = (((UINT32)pChain->NAND_DMA_Read_Seed.pDataBuffer) & 0xFFFFFFFC);
    // And the Auxiliary buffer here.
    pChain->rx_data_dma.gpmi_auxiliary.U = (((UINT32)pChain->NAND_DMA_Read_Seed.pAuxBuffer) & 0xFFFFFFFC);

    pChain->rx_data_dma.gpmi_ctrl0.U = NAND_DMA_RX_PIO(u32ChipSelect,
                                                       BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, pChain->NAND_DMA_Read_Seed.uiReadSize);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor6: disable ECC block
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->rx_wait4done_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->success_dma);
    pChain->rx_wait4done_dma.cmd.U = NAND_DMA_DISABLE_ECC_TRANSFER;
    pChain->rx_wait4done_dma.bar = NULL;
    pChain->rx_wait4done_dma.gpmi_ctrl0.U = NAND_DMA_DISABLE_ECC_PIO(u32ChipSelect);
    pChain->rx_wait4done_dma.gpmi_compare.U = 0x00;
    pChain->rx_wait4done_dma.gpmi_eccctrl.U =
        BF_GPMI_ECCCTRL_ENABLE_ECC(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor7: deassert NAND lock
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->success_dma.nxt = (apbh_dma_t*) 0x0;
    pChain->success_dma.cmd.U = ((reg32_t)
                                           (BF_APBH_CHn_CMD_IRQONCMPLT(1) | \
                                           BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
                                           BF_APBH_CHn_CMD_SEMAPHORE(1) | \
                                           BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
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
//! \brief      Build the Complete Program descriptor for the NAND.
//!
//! \fntype     Non-Reentrant
//!
//! Descriptor to write data to the NAND as a complete transaction.  This
//! descriptor sends a Command, Address, Command2 , then the data to the NAND.
//! This may be either a full Sector write (2112 bytes) or a partial sector
//! write depending upon the size passed in with the pSeed structure.
//!
//! \param[in]  pChain - pointer to the descriptor chain that gets filled.
//! \param[in]  u32ChipSelect - Chip Select - NANDs 0-3.
//! \param[in]  u32AddressSize TBD
//! \param[in]  u32DataSize - Number of bytes of data to be written.
//! \param[in]  u32EccSize - Number of bits of correction (4 bit or 8 bit).
//! \param[in]  pWriteBuffer - Data buffer to write to NAND.
//! \param[in]  pAuxBuffer - Auxillary buffer for use in write to NAND.
//!
//! \note       branches to Timeout or TRUE DMA upon completion.
//!             Data is written differently if 3700 is used because the
//!             ECC is tacked on, so ECC is not included here.
//!
//! \todo [PUBS] Define TBD parameter(s)
////////////////////////////////////////////////////////////////////////////////
VOID  SetupDMAWriteDesc(
    NAND_dma_program_t      *pChain,
    UINT32 u32ChipSelect,
    UINT32 u32AddressSize,
    UINT32 u32DataSize,
    VOID                            *pWriteBuffer,
    VOID                            *pAuxBuffer)
{
    UINT32 iCLE1_Size;
    
    // CLE1 chain size is # columns + # Rows + CLE command.
    iCLE1_Size = u32AddressSize + 1;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor1: Issue NAND write setup command
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->tx_cle1_addr_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->tx_data_dma);
    pChain->tx_cle1_addr_dma.cmd.U = NAND_DMA_COMMAND_CMD(iCLE1_Size,0,NAND_LOCK,3);
    pChain->tx_cle1_addr_dma.bar = DMA_MemTransfer(&pChain->NandProgSeed.tx_cle1_addr_buf[0]);
    pChain->tx_cle1_addr_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                                 iCLE1_Size, BV_GPMI_CTRL0_ADDRESS_INCREMENT__ENABLED, ASSERT_CS);
    pChain->tx_cle1_addr_dma.gpmi_compare.U = (UINT)NULL;
    pChain->tx_cle1_addr_dma.gpmi_eccctrl.U = 0;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor2: write the data payload
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    {
        pChain->tx_data_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->tx_cle2_dma);
        
        // Set DMA command.
        pChain->tx_data_dma.cmd.U = NAND_DMA_TXDATA_CMD(0, 0, 6, 1, NO_DMA_XFER);
        pChain->tx_data_dma.gpmi_ctrl0.U = NAND_DMA_TXDATA_PIO(u32ChipSelect,
                                                               BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, 0);
        // Compare isn't used.
        pChain->tx_data_dma.gpmi_compare.U = 0;
        pChain->tx_data_dma.gpmi_eccctrl.U = NAND_DMA_ECC_CTRL_PIO(0x1ff, BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_8_BIT);
        pChain->tx_data_dma.gpmi_ecccount.U = u32DataSize;
        pChain->tx_data_dma.gpmi_payload.U = ((DWORD)DMA_MemTransfer(pWriteBuffer) & 0xFFFFFFFC);
        pChain->tx_data_dma.gpmi_auxiliary.U = ((DWORD)DMA_MemTransfer(pAuxBuffer) & 0xFFFFFFFC);
    }

    // Set Buffer Address Register to WriteBuffer.
    pChain->tx_data_dma.bar = DMA_MemTransfer(pWriteBuffer);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor3: issue NAND write execute command
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Current Action - Send CLE2 to the NAND.
    // Next Action - Wait for Write to complete.
    pChain->tx_cle2_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->wait_dma);
    pChain->tx_cle2_dma.cmd.U = NAND_DMA_COMMAND_CMD(1,0,NAND_LOCK,3);
    pChain->tx_cle2_dma.bar = DMA_MemTransfer(pChain->NandProgSeed.tx_cle2_buf);
    pChain->tx_cle2_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                            1, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);

    // Set compare to NULL.
    pChain->tx_cle2_dma.gpmi_compare.U = (UINT)NULL;
    // Disable the ECC.
    pChain->tx_cle2_dma.gpmi_eccctrl.U = NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor4: wait for ready
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->wait_dma.nxt = (apbh_dma_gpmi1_t*) DMA_MemTransfer(&pChain->sense_dma);
    pChain->wait_dma.cmd.U = NAND_DMA_WAIT4RDY_CMD;
    pChain->wait_dma.bar = 0x00;
    pChain->wait_dma.gpmi_ctrl0.U = NAND_DMA_WAIT4RDY_PIO(u32ChipSelect);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor5: psense compare
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->sense_dma.nxt = (apbh_dma_t*)DMA_MemTransfer(&pChain->statustx_dma);
    pChain->sense_dma.cmd.U = NAND_DMA_SENSE_CMD(0);
    pChain->sense_dma.bar = (apbh_dma_t*)DMA_MemTransfer(&pChain->program_failed_dma);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor6: issue NAND status command
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->statustx_dma.nxt = (apbh_dma_gpmi1_t*)DMA_MemTransfer(&pChain->statusrx_dma);
    pChain->statustx_dma.cmd.U = NAND_DMA_COMMAND_CMD(NAND_READ_STATUS_SIZE,0, NAND_LOCK, 3);
    pChain->statustx_dma.bar = DMA_MemTransfer(&pChain->NandProgSeed.u8StatusCmd);
    pChain->statustx_dma.gpmi_ctrl0.U = NAND_DMA_COMMAND_PIO(u32ChipSelect,
                                                             NAND_READ_STATUS_SIZE, BV_GPMI_CTRL0_ADDRESS_INCREMENT__DISABLED, ASSERT_CS);


    // Set compare to NULL.
    pChain->statustx_dma.gpmi_compare.U = (UINT)NULL;
    // Disable the ECC.
    pChain->statustx_dma.gpmi_eccctrl.U = NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor7: read status and compare
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->statusrx_dma.nxt = (apbh_dma_gpmi2_t*)DMA_MemTransfer(&pChain->statbranch_dma);
    pChain->statusrx_dma.cmd.U = NAND_DMA_RX_NO_ECC_CMD(NAND_READ_STATUS_RESULT_SIZE, 0);
    pChain->statusrx_dma.bar = DMA_MemTransfer(&pChain->NandProgSeed.u16Status);
    pChain->statusrx_dma.gpmi_ctrl0.U = NAND_DMA_RX_PIO(u32ChipSelect,
                                                        BV_GPMI_CTRL0_WORD_LENGTH__8_BIT, NAND_READ_STATUS_RESULT_SIZE);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor8: psense compare
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pChain->statbranch_dma.nxt = (apbh_dma_t*)DMA_MemTransfer(&pChain->success_dma);
    pChain->statbranch_dma.cmd.U = NAND_DMA_SENSE_CMD(0);
    pChain->sense_dma.bar = (apbh_dma_t*)DMA_MemTransfer(&pChain->program_failed_dma);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Descriptor9: emit GPMI interrupt
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

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Next function is null.
    pChain->program_failed_dma.nxt = (apbh_dma_t*) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    pChain->program_failed_dma.cmd.U = ((UINT)
                                           (BF_APBH_CHn_CMD_IRQONCMPLT(1) | \
                                           BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
                                           BF_APBH_CHn_CMD_SEMAPHORE(1) | \
                                           BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to timeout termination code.
    pChain->program_failed_dma.bar = (VOID *) FALSE;
}
////////////////////////////////////////////////////////////////////////////////
//! \brief      Wait for DMA to complete
//!
//! \fntype     Non-Reentrant
//!
//! Waits for NAND DMA command chain corresponding to wNANDDeviceNum to complete.
//!
//! \param[in]  u32usec Number of microseconds to wait before timing out
//! \param[in]  u32NANDDeviceNum Which NAND should be polled.
//!
//! \retval     TRUE if DMA completed without an error
//! \retval     FALSE ERROR_DDI_NAND_DMA_TIMEOUT if DMA never completed
//!
//! \note       Uses the NXTCMDAR field of the last DMA command to signal
//!             result of the DMA chain.
//!
//! \todo       INTegrate ThreadX INTo here.
////////////////////////////////////////////////////////////////////////////////
BOOL DMA_Wait(UINT32 u32usec, UINT32 u32NANDDeviceNum)
{
    INT32 i32Sema;
    UINT8 r32ChipDmaNumber = (NAND0_APBH_CH+(UINT8)u32NANDDeviceNum);

    zNAND_Misc[u32NANDDeviceNum].uDMATimeout = u32usec;

    // We'll always poll for DMA completion.
    {
        UINT32 u32Start;

        // Microsecond read - always read at start of transaction so that if
        // ThreadX times out, that time is included in the overall timeout time.
        u32Start = zNAND_Misc[u32NANDDeviceNum].uStartDMATime;

        // end of DMA chain will decrement semaphore.  Poll Semaphore for
        // DMA completion.
        do {
            i32Sema = DDKApbhDmaGetPhore(r32ChipDmaNumber);

        } while ((i32Sema!= 0) &&
                 (((HW_DIGCTL_MICROSECONDS_RD() - u32Start) & 0xffffffff) < u32usec));
    }

    // if timeout return error, else return NXTCMDAR field from last DMA command
    if (i32Sema != 0)
    {
        // abort dma by resetting channel
        DDKApbhDmaResetChan(r32ChipDmaNumber,FALSE);

        // Abort GPMI wait
        RETAILMSG(1, (TEXT("-DMA_Wait(ERROR_DDI_NAND_DMA_TIMEOUT)\r\n")));

        return FALSE;
    } else {
        return TRUE;
    }
}

////////////////////////////////////////////////////////////////////////////////
//! \brief      Start the appropriate DMA channel
//!
//! \fntype     Non-Reentrant
//!
//! Starts a NAND DMA command channel.
//!
//! \param[in]  pDmaCmd PoINTer to dma command structure
//! \param[in]  u32NANDDeviceNum Which NAND should be started.
//!
//! \retval     None
//!
//! \note       Assumes DMA is free. It is the caller's responsibility
//!             to make sure this is true!
//!
//! \todo       Use locking mechanism to grab DMA channel.
////////////////////////////////////////////////////////////////////////////////
VOID DMA_Start(dma_cmd_t * pDmaCmd, UINT32 u32NANDDeviceNum)
{
    // Initialize DMA by setting up NextCMD field
    UINT8 r32ChipDmaNumber = (NAND0_APBH_CH+(UINT8)u32NANDDeviceNum);

    DDKApbhStartDma(r32ChipDmaNumber,(PVOID)DMA_MemTransfer(pDmaCmd),1);

    zNAND_Misc[u32NANDDeviceNum].uStartDMATime = HW_DIGCTL_MICROSECONDS_RD();
}

////////////////////////////////////////////////////////////////////////////////
//! \brief      Initialize the NAND DMA chains.
//!
//! \fntype     Non-Reentrant
//!
//! Much of the DMA chain can be pre-initialized to speed up subsequent events.
//!
//! \note       Seed values are
//!
//! \todo
////////////////////////////////////////////////////////////////////////////////
VOID DMA_Init(
    UINT32 u32NumAddressBytes)
{
    int i = 0;
    
    DmaReadDescriptor = (NAND_dma_read_t *)DMA_MemAlloc((ULONG)sizeof(NAND_dma_read_t));

    DmaEraseBlockDescriptor = (NAND_dma_block_erase_t *)DMA_MemAlloc((ULONG)sizeof(NAND_dma_block_erase_t));

    DmaProgramDescriptor = (NAND_dma_program_t *)DMA_MemAlloc((ULONG)sizeof(NAND_dma_program_t));

    DmaGenericDescriptor = (NAND_dma_generic_struct_t *)DMA_MemAlloc((ULONG)sizeof(NAND_dma_generic_struct_t));

    for(i = 0; i < MAX_NAND_DEVICES; i++)
    {
        // APBH - disable reset, enable clock and reset NAND channel
        DDKApbhDmaChanCLKGATE((UINT8)(NAND0_APBH_CH + i),TRUE);
    
        // Reset dma channels, and clear IRQs.
        //DDKApbhDmaResetChan((UINT8)(NAND0_APBH_CH + i),TRUE);
    
        DDKApbhDmaEnableCommandCmpltIrq((UINT8)(NAND0_APBH_CH + i),FALSE);
    }
    
    // Pre-Build up the Read DMA.  Even if some values are left empty,
    // they'll be filled in later.
    {
        NAND_read_seed_t *pDmaReadSeed = (NAND_read_seed_t *) &(DmaReadDescriptor->NAND_DMA_Read_Seed);
    
        pDmaReadSeed->enableECC = TRUE;
        
        // Alway 2 column bytes.
        pDmaReadSeed->uiAddressSize = u32NumAddressBytes;
    
        // set the Word size
        // default to 8 bit data width
        pDmaReadSeed->uiWordSize =  BV_GPMI_CTRL0_WORD_LENGTH__8_BIT;
        
        // Setup the Read Data command words.
        pDmaReadSeed->tx_cle1 = (UINT8)eNandProgCmdRead1;
        pDmaReadSeed->tx_cle2 = (UINT8)eNandProgCmdRead1_2ndCycle;
    
        // Fill in the Column Address (Always 2 bytes)
        pDmaReadSeed->bType2Columns[0] = (UCHAR)(0);
        pDmaReadSeed->bType2Columns[1] = (UCHAR)(0);
    
        // Fill in the Row Address. (Can be 2 or 3 bytes)
        // Fill in the Column Address (Always 2 bytes)
        pDmaReadSeed->bType2Rows[0] = (UCHAR)(0);
        pDmaReadSeed->bType2Rows[1] = (UCHAR)(0);
        pDmaReadSeed->bType2Rows[2] = (UCHAR)(0);
    
        // Buffer pointers used for DMA chain.
        pDmaReadSeed->pDataBuffer = NULL;
        pDmaReadSeed->pAuxBuffer = NULL;
    
        SetupDMAReadDesc(DmaReadDescriptor, 0, pDmaReadSeed->uiAddressSize,
                                4096, NULL);
    }
    // Pre-Build up the Write DMA.  Even if some values are left empty,
    // they'll be filled in later.
    {
        // Set the number of address bytes.
        DmaProgramDescriptor[0].NandProgSeed.uiAddressSize = u32NumAddressBytes;
        // Load Command Code for Serial Data Input (0x80).
        DmaProgramDescriptor[0].NandProgSeed.tx_cle1 = eNandProgCmdSerialDataInput;
        // Load command and mask for GetStatus portion of DMA.
        DmaProgramDescriptor[0].NandProgSeed.u8StatusCmd = eNandProgCmdReadStatus;
        // Pre-Build up the Read DMA.  Even if some values are left empty,
        // they'll be filled in later.
        SetupDMAWriteDesc(&DmaProgramDescriptor[0],
                               0,                       // u32ChipSelect
                               u32NumAddressBytes,      // u32AddressSize
                               0,                       // u32DataSize
                               NULL,                    // pWriteBuffer
                               NULL);                   // pAuxBuffer
    }
    // Pre-Build up the Erase DMA.  Even if some values are left empty,
    // they'll be filled in later.
    {
        DmaEraseBlockDescriptor->NandEraseSeed.tx_block[0] = 0;
        DmaEraseBlockDescriptor->NandEraseSeed.tx_block[1] = 0;
        DmaEraseBlockDescriptor->NandEraseSeed.tx_block[2] = 0;
    
        // Load Command Code for Serial Data Input (0x80)
        DmaEraseBlockDescriptor->NandEraseSeed.tx_cle1 = eNandProgCmdBlockErase;
    
        // Load Command Code for Page Program (0x10)
        DmaEraseBlockDescriptor->NandEraseSeed.tx_cle2 = eNandProgCmdBlockErase_2ndCycle;
    
        // Load command and mask for GetStatus portion of DMA.
        DmaEraseBlockDescriptor->NandEraseSeed.u8StatusCmd = eNandProgCmdReadStatus;
    
        // Build the DMA that will program this sector.
        SetupDMAEraseDesc(DmaEraseBlockDescriptor, u32NumAddressBytes, 0);       
    }                    
}
