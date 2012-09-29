;------------------------------------------------------------------------------
;
;   Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   FILE:   xldr.s
;   
;   Provides support for booting from CSPI bootstrap and launching
;   an image from a SPI Flash device connected to the CSPI1 SS1.
;
;------------------------------------------------------------------------------

    INCLUDE armmacros.s
    INCLUDE mx25_base_regs.inc
    INCLUDE mx25_base_mem.inc
    INCLUDE mx25_crm.inc
    INCLUDE mx25_esdramc.inc
    INCLUDE mx25_cspi.inc
    INCLUDE cspichip.inc
    INCLUDE image_cfg.inc

; CSPI constants
CSPI_BASE_REGISTER          EQU     (CSP_BASE_REG_PA_CSPI1)
CRM_CSPI_PER_CLK_EN         EQU     (1 << 5)
CSPI_MAX_NUMBER_BYTE_EXCHANGE        EQU     (512)

    OPT 2                                       ; disable listing
    INCLUDE kxarm.h
    OPT 1                                       ; reenable listing
   
    TEXTAREA

    ; romimage needs pTOC. give it one.
pTOC    DCD -1
    EXPORT pTOC


;******************************************************************************
;*
;* FUNCTION:    StartUp
;*
;* DESCRIPTION: System bootstrap function
;*
;* PARAMETERS:  None
;*
;* RETURNS:     None
;*
;******************************************************************************
 
    STARTUPTEXT
    LEAF_ENTRY StartUp
    
    ;; now we do the external RAM init. it's located in another file because it's shared among all the XLDRs
    INCLUDE xldr_sdram_init.inc
    
entry_point
    ; Gate clock into CSPI Module
    ;
    ldr r1, =(CSP_BASE_REG_PA_CRM + CRM_CGCR1_OFFSET)
    ldr r0, [r1]
    orr r0, r0, #(CRM_CSPI_PER_CLK_EN)
    str r0, [r1]
    
    ; Configure CSPI operation
    ;   
    ;   BURST_LENGTH - Burst Length (fff << 20)           = 0xfff00000
    ;   DATA_RATE - DataRate (1 << 16)                  = 0x00010000
    ;   CHIP_SELECT - Slave Select (1 << 12)            = 0x00001000
    ;   DRCTL - SPI Data Ready Control (0 << 8)         = 0x00000000
    ;   SSPOL - SPI Polarity Select (0 << 7)            = 0x00000000
    ;   SSCTL - SS Waveform Select (0 << 6)             = 0x00000000
    ;   PHA - Clock/Data Phase C0ntrol (0 << 5)         = 0x00000000
    ;   POL - Clock Polarity Control (0 << 4)           = 0x00000000
    ;   SMC - Start Mode Control (0 << 3)               = 0x00000000
    ;   XCH - Exchange Bit (0 << 2)                     = 0x00000000
    ;   MODE - Mode Select (1 << 1)                     = 0x00000002
    ;   EN - Enable Control (0 << 0)                    = 0x00000000
    ;                                                   --------
    ;                                                     0xfff11002
    ;
    ldr     r1, =(CSPI_BASE_REGISTER)
    ldr     r0, =0xfff11002
    str    r0, [r1, #8]

    ; Current external RAM load address is kept in R3
    ldr     r3,  =(IMAGE_BOOT_BOOTIMAGE_RAM_PA_START)

    ; Current block is kept in R13
    ldr     r13, =(IMAGE_BOOT_BOOTIMAGE_SPI_OFFSET >> CSPI_BLOCK_SIZE_LSH)
    
    ; sector address set in r12
    mov     r12, r13, lsl #(CSPI_SECTOR_NUM_LSH)

    ; Copy Eboot from CSPI Flash to Eboot (current flash chip supports reading of multiple sectors/blocks in 1 read operation)
    bl      CspiReadEboot

    ; Jump to EBOOT image copied to DRAM
    ldr     r1, =(IMAGE_BOOT_BOOTIMAGE_RAM_PA_START)
    mov     pc, r1
    nop
    nop
    nop
    nop

forever
    b       forever
    
    
;-------------------------------------------------------------------------------
;
;   Function:  CspiReadEboot
;
;   This function reads data (length of read defined by Size of Eboot) from the SPI Flash device
;   from the specified SPI Flash device Sector address.
;
;   Parameters:
;       address (r12)
;           [in/out] - Block address for SPI Flash device. Read starts from this address.
;       RAM address (r3)
;           [in/out] - RAM Address of next location to store data to. Function updates this value as
;           data is being read in.
;
;   Returns:
;       None.
;-------------------------------------------------------------------------------
    LEAF_ENTRY CspiReadEboot
    
    ldr r4, =(CSPI_BASE_REGISTER + CSPI_RXDATA_OFFSET)
    ldr r5, =(CSPI_BASE_REGISTER + CSPI_TXDATA_OFFSET)
    ldr r6, =(CSPI_BASE_REGISTER + CSPI_CONREG_OFFSET)
    ldr r7, =(CSPI_BASE_REGISTER + CSPI_STATREG_OFFSET)
    
    
    ;convert r12 into the byte address to read from
    mov r12, r12 lsl #(CSPI_SECTOR_SIZE_LSH)
    
    ; r10 contains number of bytes recieved.
    mov r10, #0
    
    ;r11 contains number of bytes sent
    mov r11, #0
    
    ;r2 contains number of bytes received plus the size of the next transfer
    mov r2, #0
    
    ldr r9, =(IMAGE_BOOT_BOOTIMAGE_SPI_SIZE) ;number of bytes to be transmitted in total
    
    ; Enable CSPI Controller
    ldr r1, =(CSPI_CONREG_ENABLE)
    ldr r0, [r6]
    orr r0, r0, r1
    str r0, [r6]

FillAddress
    ; Fill TX FIFO w/ command + flat address
    ldr r0, =(CMD_READ)
    mov r0, r0, lsl #24
    orr r0, r0, r12
    str r0, [r5]
    
    ; fill the TX FIFO to full before signalling start of transaction
fill_tx_fifo
    ldr r0, [r7]
    mov r8, #(CSPI_STATREG_TXFULL)
    and r0, r0, r8
    cmp r0, r8
    beq start_xchg      ; if TX FIFO is full then start transmission
    mov r0, #0          ;load in dummy data to transmit
    str r0, [r5]
    add r11, r11, #4
    b fill_tx_fifo
    
    ; Signal that system is ready to perform a CSPI transaction
start_xchg    
    ; Start Exchange
    ldr r1, =(CSPI_CONREG_XCHG)
    ldr r0, [r6]
    orr r0, r0, r1
    str r0, [r6]
    
    ; Read in first 4 dummy bytes
dummy_read
    ldr r0, [r7]
    mov r8, #(CSPI_STATREG_RXRDY)
    and r0, r8, r0
    cmp r0, r8
    bne dummy_read      ; if RX Ready bit is not set then keep polling it
    ldr r0, [r4]        ;read in RX data from RX FIFO, dummy data so discard
    mov r0, #0          ;load in dummy data to transmit
    str r0, [r5]
    add r11, r11, #4 
    
    ;set how many we are going to read in this transaction
    sub r8, r9, #(CSPI_MAX_NUMBER_BYTE_EXCHANGE - CMD_READ_LEN)
    cmp r10, r8
    bgt LessThanMaximumTransfer
    add r2, r2, #(CSPI_MAX_NUMBER_BYTE_EXCHANGE - CMD_READ_LEN)    ;we subtract 4 for the dummy bytes we already got
    b continue_transmit
LessThanMaximumTransfer
    sub r8, r9, r10
    add r2, r2, r8
    
continue_transmit
    ldr r8, [r7]
    mov r0, #CSPI_STATREG_TRANS_COMPLETE
    and r8, r8, r0
    cmp r0, r8      ;if the transfer is complete start transmit again
    bne continue_poll
    ldr r0, [r6]        
    orr r0, r0, #(CSPI_CONREG_XCHG) ;start transmit
    str r0, [r6]
    
continue_poll
    ldr r8, [r7]
    mov r0, #(CSPI_STATREG_RXRDY)
    and r8, r8, #(CSPI_STATREG_RXRDY)
    cmp r0, r8
    bne continue_transmit      ; if RX Ready bit is not set then keep polling it
    ldr r0, [r4]        ;read in RX data from RX FIFO
    str r0, [r3]       ;store the RX data
    add r3, r3, #4      ;increment RAM address by 4 bytes
    add r10, r10, #4
    mov r0, #0          ;load in dummy data to transmit
    str r0, [r5]
    add r11, r11, #4
    cmp r11, r2     ;if number of bytes transmitted == bytes this transaction, then stop.
    blt continue_transmit

wait_trans_completion   
    ldr r8, [r7]
    mov r0, #(CSPI_STATREG_TRANS_COMPLETE)
    and r8, r8, r0
    cmp r0, r8
    bne wait_trans_completion       ; wait for transaction to complete
    
    ; Read out remaining RX data (until number of RX data read is IMAGE_BOOT_BOOTIMAGE_SPI_SIZE)
finish_read
    ldr r8, [r7]
    mov r0, #(CSPI_STATREG_RXRDY)
    and r8, r0, r8
    cmp r0, r8
    bne finish_read     ; if RX Ready bit is not set then keep polling it
    ldr r0, [r4]        ;read in RX data from RX FIFO
    str r0, [r3]       ;store the RX data
    add r3, r3, #4      ;increment RAM address by 1 byte (look into auto-indexing later)
    add r10, r10, #4
    cmp r10, r11       ;if number of bytes received < number transmitted. Were done for this transaction
    blt finish_read
    
    ; Increase the address to get data
    add r12, r12, #(CSPI_MAX_NUMBER_BYTE_EXCHANGE - CMD_READ_LEN) 
    cmp r10, r9
    blt FillAddress    ;if the number received is less than the size of the file, start over.
    
    ;Since data from the eeprom is shifted in least significant byte first and stored most
    ;significant byte first (4 bytes at a time), the received data needs to be rearranged
    ;the result should be this
    ;pOut[i] = pIn[i+3];
    ;pOut[i+1] = pIn[i+2];
    ;pOut[i+2] = pIn[i+1];
    ;pOut[i+3] = pIn[i];

    ldr r3,  =(IMAGE_BOOT_BOOTIMAGE_RAM_PA_START)
    ldr r10, =(IMAGE_BOOT_BOOTIMAGE_RAM_PA_START + IMAGE_BOOT_BOOTIMAGE_SPI_SIZE)
    
continue_rearrange
    ldr r0, [r3]
    strb r0, [r3, #3]
    mov r0, r0, lsr #8
    strb r0, [r3, #2]
    mov r0, r0, lsr #8
    strb r0, [r3, #1]
    mov r0, r0, lsr #8
    strb r0, [r3]
    add r3, r3, #4
    cmp r3, r10
    blt continue_rearrange
    
    RETURN    
    
    END

