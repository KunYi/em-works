;------------------------------------------------------------------------------
;
;   Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   FILE:   xldr.s
;   
;   Provides support for booting from a SD device connected to the 
;   ESDHC controller.
;
;------------------------------------------------------------------------------
    INCLUDE armmacros.s
    INCLUDE mx25_base_regs.inc
    INCLUDE mx25_base_mem.inc
    INCLUDE mx25_crm.inc
    INCLUDE mx25_esdramc.inc
    INCLUDE image_cfg.inc

    IF :LNOT: :DEF: SECUREXLDR
    GBLL    SECUREXLDR
SECUREXLDR  SETL    {FALSE}
    ENDIF

; ESDHC registers
DSADDR      EQU     CSP_BASE_REG_PA_ESDHC1+0x0000
BLKATTR     EQU     CSP_BASE_REG_PA_ESDHC1+0x0004
CMDARG      EQU     CSP_BASE_REG_PA_ESDHC1+0x0008
XFERTYP     EQU     CSP_BASE_REG_PA_ESDHC1+0x000C
CMDRSP0     EQU     CSP_BASE_REG_PA_ESDHC1+0x0010
CMDRSP1     EQU     CSP_BASE_REG_PA_ESDHC1+0x0014
CMDRSP2     EQU     CSP_BASE_REG_PA_ESDHC1+0x0018
CMDRSP3     EQU     CSP_BASE_REG_PA_ESDHC1+0x001C
DATPORT     EQU     CSP_BASE_REG_PA_ESDHC1+0x0020
PRSSTAT     EQU     CSP_BASE_REG_PA_ESDHC1+0x0024
PROCTL      EQU     CSP_BASE_REG_PA_ESDHC1+0x0028
SYSCTL      EQU     CSP_BASE_REG_PA_ESDHC1+0x002C
IRQSTAT     EQU     CSP_BASE_REG_PA_ESDHC1+0x0030
IRQSTATEN   EQU     CSP_BASE_REG_PA_ESDHC1+0x0034
IRQSIGEN    EQU     CSP_BASE_REG_PA_ESDHC1+0x0038
AUTOC12ERR  EQU     CSP_BASE_REG_PA_ESDHC1+0x003C
HOSTCAPBLT  EQU     CSP_BASE_REG_PA_ESDHC1+0x0040
WML         EQU     CSP_BASE_REG_PA_ESDHC1+0x0044
FEVT        EQU     CSP_BASE_REG_PA_ESDHC1+0x0050
HOSTVER     EQU     CSP_BASE_REG_PA_ESDHC1+0x00FC

; ROM code stores the address mode of the card at this location in IRAM
    IF :DEF: BSP_CPU_TO1
    ; Red board with TO1
IRAM_CARD_ADDR_MODE    EQU     0x780018FC
    ELSE
    ; Blue board with TO1.1
IRAM_CARD_ADDR_MODE    EQU     0x78001908
    ENDIF
    
    IF SECUREXLDR
LS_HEADER_OFFSET       EQU     0x00001800
    ENDIF

    OPT 2                                       ; disable listing
    INCLUDE kxarm.h
    OPT 1                                       ; reenable listing
   
    TEXTAREA

    IF SECUREXLDR
    IMPORT LS_init
    IMPORT LS_exit
    IMPORT LS_hal_valid_range
    IMPORT LS_authenticate_image
    ENDIF

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
    


    ; SD XLDR can only be 2KB for max size since ROM Boot only copies over the first 2KB. No code exists in XLDR to copy the rest of itself
    ; into RAM.
    
    ;----------------------------------------
    ; Disable all clock except the EMI
    ;----------------------------------------   
    ;   The EMI clock used by the XLDR in Clock Gating Control Register 0
    ;   is not turned off. We only need RAM and ESDHC 
    ;       Enable AHB hclk_emi             = 0x00080000
    ;       Enable AHB hclk_esdhc1          = 0x00200000
    ;       Enable AHB ipg_per_esdhc2       = 0x00000008
    ;                                       ------------
    ;                                         0x00280008
    ldr     r0, =0x00280008
    str     r0, [r1, #CRM_CGCR0_OFFSET]

    ;   Clock Gating Control Register 1
    ;       Disable all IPG Clock except for ipg_clk_esdhc1 (1<<13)  = 0x00002000
    ;
    ldr     r0, =0x00002000
    str     r0, [r1, #CRM_CGCR1_OFFSET]

    ;   Clock Gating Control Register 2
    ;       Disable all IPG Clock except for ESDHC1 (0 << 0)  = 0x00000000
    ;
    ldr     r0, =0x00000000
    str     r0, [r1, #CRM_CGCR2_OFFSET]
    
      
    ; Do the DDR initialization sequence
    bl      ESDCTLSetup

    IF SECUREXLDR
    ;
    ; Initialize the Loader Security Library
    ; LS_init Prototype
    ; void LS_init (void);
    ;
    bl      LS_init;
    
    ; Verify whether the specified address range is
    ; within the valid RAM address ranges
    ;
    ; LS_hal_valid_range prototype
    ;
    ; UINT8
    ; LS_hal_valid_range (
    ;    UINT32 start,
    ;    UINT32 length_in_bytes)
    ;
    ; Return Value:
    ; This function returns 1 if success 0 otherwise
    ;
    ;   UINT32 start (0x90040000)
    ldr     r0, =IMAGE_BOOT_BOOTIMAGE_RAM_PA_START
    ;   UINT32 length_in_bytes (256 kbytes)
    ldr     r1,=IMAGE_BOOT_BOOTIMAGE_SD_SIZE
    bl      LS_hal_valid_range
    cmp     r0, #0  
    beq     forever ; Invalid Address Range
    ENDIF

copy_eboot

    ; is this card to be addressed in sector mode or byte mode? Save flag in r12. DO NOT MODIFY r12 AGAIN!
    ldr r0, =IRAM_CARD_ADDR_MODE
    ldr r12,[r0]   
;Issue Cmd to Read Single Block Read   

    ldr     r11,=0x200 ; loop for 512 times
    ldr     r7,=0x0 ; Maintain Offset in RAM

;Set Read Watermark to 512 bytes (128 words = 0x80)
    ldr     r1,=(WML)
    ldr     r0,=0x80
    str     r0,[r1]    

read_loop 

;Reset IRQSTATUS Register
    ldr     r1,=(IRQSTAT)
    ldr     r0,=0xFFFFFFFF ; All w1c
    str     r0,[r1]

;Set Block Length to 512 and number of blocks to 1
    ldr     r1,=(BLKATTR)
    ldr     r0,=0x10200 ; 1 block of 512 bytes
    str     r0,[r1]
; End  Set Block Length   
    
;Set Single Block Read command (CMD17) values into the XFERTYP register
    ldr     r0,=0x113A0010

; command argument in r2
    ldr     r2,=IMAGE_BOOT_BOOTIMAGE_SD_OFFSET
    add     r2,r2,r7

 ; if sector mode, need to shift the address back to sector number (/512 = >> 9)
    cmp r12, #1
    moveq r2, r2, LSR #9

; Issue Cmd
    bl      SD_Issue_Cmd

;Wait for End Command
    bl      SDHCWaitEndCmdRespIntr
    
;Check Status - Branch Forever if Error
    cmp     r2,#1
    bne     forever
    
;Get current offset of EBOOT to be stored in DRAM
    ldr     r3,=IMAGE_BOOT_BOOTIMAGE_RAM_PA_START
    add     r3,r3,r7

;Increment Offset
    add     r7,r7,#0x200

;Call Read Data 
    bl      SDHCDataRead

;Check Status - Branch Forever if Error
    cmp     r2,#1
    bne     forever

    subs    r11,r11,#1
    
;Loop until sector counter is 0
    bne     read_loop
    
    IF SECUREXLDR
    ; Authenticate EBOOT before lauching it
    ;
    ; LS_authenticate_image prototype
    ;
    ; UINT32*
    ;   LS_authenticate_image (
    ;       LS_APP_HEADER_T* next_image_hdr,
    ;       UINT32* anti_replay_addr,
    ;       UINT32* buff_addr,
    ;       UINT32* length);
    ;
    ; Return Value:
    ; This function returns the entry point address if authentication 
    ; is successful, NULL pointer otherwise 
    ;
    ; LS_APP_HEADER_T* next_image_hdr (0x90041800)
    ldr     r0,=(IMAGE_BOOT_BOOTIMAGE_RAM_PA_START + LS_HEADER_OFFSET)

    ; UINT32* anti_replay_addr (Not used)
    ldr     r1, =0x0

    ; UINT32* buff_addr - (secure warm boot buffer address - Not reqd)
    ldr     r2, =0x0

    ; UINT32* length - (secure warm boot buffer length - Not reqd.)
    ldr    r3, =0x0

    bl     LS_authenticate_image

    ; LS_authenticate image returns NULL incase of authentication
    ; failure
    cmp     r0, #0  
    beq     forever ; Authentication fails
    mov     r5, r0  ; Preserve the jump address in r5

    ; De-Initialize the Loader Security Library
    ; LS_exit Prototype
    ; void LS_exit (void);
    ;
    bl  LS_exit

    bx      r5  ; Authentication succes, so launch EBOOT

    ELSE    ; non-secure boot

    ; Jump to EBOOT image copied to DRAM
    ldr     r0, =IMAGE_BOOT_BOOTIMAGE_RAM_PA_START
    bx      r0
    ENDIF
    
    nop
    nop
    nop

forever
    b       forever


;-------------------------------------------------------------------------------
;
;   Function:  SD_Issue_Cmd
;
;   This function issues the specified command to the SD device.
;
;   Parameters:
;       command (r0)
;           [in] - Value of XFERTYP register to issue the CMD to the SD device.
;
;       command Arguement(r2)
;           [in] - Command Argument 
;
;
;   Returns:
;       None.
;-------------------------------------------------------------------------------
    LEAF_ENTRY SD_Issue_Cmd

;Set CMDARG Register
    ldr     r1,=(CMDARG)
    str     r2, [r1]

;Set XFERTYP Register which issues the command on the SDBus
    ldr     r1,=(XFERTYP)
    str     r0, [r1]

    RETURN
    


;-------------------------------------------------------------------------------
;
;   Function:  SDHCWaitEndCmdRespIntr
;
;     Wait for the response after issuing the command.
;
;   Parameters:
;       None.
;
;   Returns:
;       Status (r2)
;           [out] - 0 - Fail , 1 - Pass
;-------------------------------------------------------------------------------
    LEAF_ENTRY SDHCWaitEndCmdRespIntr

;Access STATUS Register
    ldr     r1,=(IRQSTAT)

;Loop for ECR
wait_end_cmd_loop

;CSP_BITFVAL (IRQSTAT_CC)
    ldr     r0,=0x1
    ldr     r9,[r1]
;Check for CC
   ands    r0,r0,r9
    beq      wait_end_cmd_loop

;Return Value - Set FALSE as default
    ldr     r2,=0

;Check for Command Timeout, Command CRC, Command End Bit, and Command Index errors
    ldr     r0,=0xF0000
    ldr     r9,[r1]

;Timeout bit should be 0 for correct operation
    ands    r0,r0,r9

;Set False and jump if timeout is set
    bne     return_value
    
;Set Return Value to TRUE
    ldr     r2,=1


return_value


    RETURN



;-------------------------------------------------------------------------------
;
;   Function:  SDHCReadResponse
;
;   Read the response returned by the card after a command 
;
;   Parameters:
;       Response Format (r0)
;           [in] - Response Format (2 or 4 bytes)
;
;   Returns:
;       Read Values (r7-r10)
;           [out] - Read FIIFO Values
;-------------------------------------------------------------------------------
    LEAF_ENTRY SDHCReadResponse

    ldr     r1,=(IRQSTATEN)

    cmp     r0,#2    
    beq      read_3
    cmp     r0,#7
    beq      read_8
    b return

read_3
    ldrh    r7,[r1]
    ldr     r4,[r1]
    orr     r7,r7,r4
    ldr     r8,[r1]
    b       return
;TODO - use strh
read_8
    ldrh    r7,[r1]
    ldr     r4,[r1]
    orr     r7,r7,r4
    ldrh     r8,[r1]
    ldr     r4,[r1]
    orr    r8,r8,r4

    ldrh    r9,[r1]
    ldr     r4,[r1]
    orr     r9,r9,r4
    ldrh    r10,[r1]
    ldr     r4,[r1]
    orr     r10,r10,r4

    
return   

;TODO - Status Return to be implemented ?

    RETURN


;-------------------------------------------------------------------------------
;
;   Function:  SDHCDataRead
;
;   Read the data from the card. 1 block is read in 1 read because Watermark is set at 512 bytes
;
;   Parameters:
;      Destination Memory Address (r3)
;           [out] - Destination Memory Address.
;
;   Returns:
;       Status (r2)
;           [out] - 0 - Fail , 1 - Pass
;-------------------------------------------------------------------------------
    LEAF_ENTRY SDHCDataRead

;Read STATUS Register
    ldr     r1,=(IRQSTAT)

;Return Value - Set FALSE as Default
    ldr     r2,=0

;Loop for BRR to get Set
wait_brr
    ldr     r9,[r1]
;Check if BRR is set
    ands    r9,r9,#0x20
    beq      wait_brr

;Loop counter for FIFO SIZE: 128 words have to be read to complete the fifo read
    ldr     r5,=0x80

;Access BUFFER_ACCESS Register
    ldr     r0,=(DATPORT)
    
;Loop for FIFO Read - 128*4 bytes
fifo_read

;Get 4-byte FIFO Value
    ldr     r6,[r0]
;Put 4-byte FIFO Value into SDRAM
    str     r6,[r3]
;Increment destination Address
    add     r3,r3,#4
    subs    r5,r5,#1
    bne     fifo_read
;End - Loop FIFO Read    

;Loop for Transfer Complete event
wait_tc
;Get IRQSTATUS Register Value
    ldr     r9,[r1]
;Check for TC
    ands    r9,r9,#0x2
    beq      wait_tc
;End - Loop for Transfer Complete event    

;Check for Data Timeout, Data CRC, and Data End Bit errors
    ldr     r9,[r1]
    ands    r9,r9,#0x700000
    bne     return_read

return_pass
    ldr     r2,=1

return_read   
   

    RETURN

    
;-------------------------------------------------------------------------------
;
;   Function:  ESDCTLSetup
;
;   Configure DDR memory
;
;   NOTE: It is important for this function to be defined (& loaded last in address map) so that 
;            the SD functions are part of initial 2K loaded in by ROM code. That will allow copying   
;            remaining 2K from SD/MMC card.
;
;   Parameters:
;      No parameters
;
;   Returns:
;       No return value
;-------------------------------------------------------------------------------
    LEAF_ENTRY ESDCTLSetup

    ;; now we do the external RAM init. it's located in another file because it's shared among all the XLDRs
    INCLUDE xldr_sdram_init.inc
    

    RETURN

    END


