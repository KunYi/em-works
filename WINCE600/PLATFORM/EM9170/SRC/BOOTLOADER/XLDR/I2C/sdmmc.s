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
;   Provides support for booting from an SD device connected to the 
;   ESDHC controller. This file was meant to come out of I2C EEPROM, so it has to 
;   enumrate and initialize SD.
;
;------------------------------------------------------------------------------
    INCLUDE armmacros.s
    
    INCLUDE mx25_base_mem.inc
    INCLUDE mx25_base_regs.inc
    INCLUDE mx25_crm.inc
    
END_OF_RAM              EQU     0x80200000

MMC_OCR_VALUE           EQU     0x40FF8000
SD_OCR_VALUE            EQU     0x40FF8000

LOG2_SDHC_BLK_LEN               EQU   9
SDHC_BLK_LEN                    EQU   (1<<LOG2_SDHC_BLK_LEN)
ESDHC_MAX_DATA_BUFFER_SIZE      EQU   SDHC_BLK_LEN
ESDHC_INIT_CLOCK_RATE   EQU     250000
ESDHC_STATUS_PASS       EQU     0
ESDHC_STATUS_FAILURE    EQU     1



XFERTYPE_SD_CMD_SEND_IF_COND            EQU 0x081a0000
XFERTYPE_SD_CMD_APP_CMD                 EQU 0x371a0000
XFERTYPE_SD_ACMD_SD_SEND_OP_COND        EQU 0x29020000
XFERTYPE_SD_CMD_ALL_SEND_CID            EQU 0x02090000
XFERTYPE_SD_CMD_SELECT_DESELECT_CARD    EQU 0x071b0000
XFERTYPE_SD_CMD_SET_BLOCKLEN            EQU 0x101a0000
XFERTYPE_SD_CMD_MMC_SET_RCA             EQU 0x031a0000
XFERTYPE_SD_CMD_SEND_STATUS             EQU 0x0d1a0000
XFERTYPE_SD_CMD_MMC_SEND_OPCOND         EQU 0x01020000
XFERTYPE_SD_CMD_READ_MULTIPLE_BLOCK     EQU 0x123a0036

OFFSET_DSADDR      EQU     0x0000
OFFSET_BLKATTR     EQU     0x0004
OFFSET_CMDARG      EQU     0x0008
OFFSET_XFERTYP     EQU     0x000C
OFFSET_CMDRSP0     EQU     0x0010
OFFSET_CMDRSP1     EQU     0x0014
OFFSET_CMDRSP2     EQU     0x0018
OFFSET_CMDRSP3     EQU     0x001C
OFFSET_DATPORT     EQU     0x0020
OFFSET_PRSSTAT     EQU     0x0024
OFFSET_PROCTL      EQU     0x0028
OFFSET_SYSCTL      EQU     0x002C
OFFSET_IRQSTAT     EQU     0x0030
OFFSET_IRQSTATEN   EQU     0x0034
OFFSET_IRQSIGEN    EQU     0x0038
OFFSET_AUTOC12ERR  EQU     0x003C
OFFSET_HOSTCAPBLT  EQU     0x0040
OFFSET_WML         EQU     0x0044
OFFSET_FEVT        EQU     0x0050
OFFSET_HOSTVER     EQU     0x00FC

;; ESDHC registers
DSADDR      EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_DSADDR      
BLKATTR     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_BLKATTR     
CMDARG      EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_CMDARG      
XFERTYP     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_XFERTYP     
CMDRSP0     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_CMDRSP0     
CMDRSP1     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_CMDRSP1     
CMDRSP2     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_CMDRSP2     
CMDRSP3     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_CMDRSP3     
DATPORT     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_DATPORT     
PRSSTAT     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_PRSSTAT     
PROCTL      EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_PROCTL      
SYSCTL      EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_SYSCTL      
IRQSTAT     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_IRQSTAT     
IRQSTATEN   EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_IRQSTATEN   
IRQSIGEN    EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_IRQSIGEN    
AUTOC12ERR  EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_AUTOC12ERR  
HOSTCAPBLT  EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_HOSTCAPBLT  
WML         EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_WML         
FEVT        EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_FEVT        
HOSTVER     EQU     CSP_BASE_REG_PA_ESDHC1+OFFSET_HOSTVER     
    

    OPT 2                                       ; disable listing
    INCLUDE kxarm.h
    OPT 1                                       ; reenable listing
   
    TEXTAREA

    

;------------------------------------------------------------------------------
;- Define the entry point
;------------------------

    EXPORT  SDMMC_go

    
SDMMC_go
  

        
        
    ldr     r1, =CSP_BASE_REG_PA_CRM
    ldr     r0, =0xFFFFFFFF
    str     r0, [r1, #CRM_CGCR0_OFFSET]
   
    ldr     r0, =0xFFFFFFFF
    str     r0, [r1, #CRM_CGCR1_OFFSET]

    ldr     r0, =0xFFFFFFFF
    str     r0, [r1, #CRM_CGCR2_OFFSET]        
                    
        
        
;  Set up a supervisor mode stack.
    ldr sp, =END_OF_RAM
    add sp,sp,#-4

    bl SDConfigPins
    
    bl InitSDController
    
    bl SDMMC_card_software_reset
    
    bl  SDMMC_Init
    
    mov r0,#256         ; start sector of EBOOT (on MMC or SD)
    ldr r1,=0x80040000  ; address of execution of EBOOT
    mov r2,#512         ; number of sectors to read from the card
    bl  ASM_SDMMC_ReadSectors
    
    ldr r0,=0x80040000
    mov pc,r0
    
    
    
Jump
        mov pc, r0      
        
        
;-------------------------------------------------------------------------------
;
;   Function:  InitSDController
;
;   Reset and Initialize the controller
;
;   Parameters:
;      No parameters
;
;
;
;   mov pc,r14s:
;       No mov pc,r14 value
;-------------------------------------------------------------------------------    
InitSDController
    stmfd    r13!,{r0,r1,r14}
    
    ldr     r0,=CSP_BASE_REG_PA_ESDHC1    
    ;; Reset the controller
    ;INSREG32BF(&g_pESDHCReg->SYSCTL,ESDHC_SYSCTL_RSTA, 1);    
    ldr     r1,[r0,#OFFSET_SYSCTL]
    orr     r1,r1, #(1<<24)
    str     r1,[r0,#OFFSET_SYSCTL]
    
    ;; Mask all SDHC interrupts
    ;OUTREG32(&g_pESDHCReg->IRQSIGEN, 0);
    mov     r1,#0x0
    str     r1,[r0,#OFFSET_IRQSIGEN]
    
    ;; Enable all status bits in the IRQSTAT register except CINS and CRM because we do not process changes in card status in eboot
    ;OUTREG32(&g_pESDHCReg->IRQSTATEN, 0xFFFFFF3F);
    mov     r1,#0xFFFFFF3F
    str     r1,[r0,#OFFSET_IRQSTATEN]
    
    ;;set the default bitrate (around 250KHz) and enable the clocks
    ldr     r1,=0xE1087
    str     r1,[r0,#OFFSET_SYSCTL]
           

    ;set watermark level (WML register units are 4-byte words, not bytes        
    ldr     r1,=(ESDHC_MAX_DATA_BUFFER_SIZE / 4) | ((ESDHC_MAX_DATA_BUFFER_SIZE/4)<<16)
    str     r1,[r0,#OFFSET_WML]
    
    ;setup data bus width TO 1 BIT, no DMA etc...    
    ldr     r1,[r0,#OFFSET_PROCTL]
    and     r1,r1,#~(3<<1)
    str     r1,[r0,#OFFSET_PROCTL]


    ;send the initialization clocks (80) before first command is sent (bit INITA)
    
    ldr     r1,[r0,#OFFSET_SYSCTL]
    orr     r1,r1,#(1<<27)
    str     r1,[r0,#OFFSET_SYSCTL]
wait_end_inita_loop    
    ldr     r1,[r0,#OFFSET_SYSCTL]
    ands    r1,r1,#(1<<27)
    beq      wait_end_inita_loop
   
    ldmfd    r13!,{r0,r1,r14}           
    mov pc,r14
                
    
;-------------------------------------------------------------------------------
;
;   Function:  SDConfigPins
;
;   Configure SD/MMC pins
;
;   Parameters:
;      No parameters
;
;
;
;   mov pc,r14s:
;       No mov pc,r14 value
;-------------------------------------------------------------------------------    
SDConfigPins
    stmfd    r13!,{r0,r1,r14}
    
    ldr     r0,=CSP_BASE_REG_PA_IOMUXC
    
    ldr     r1,=0x000001f0
    str     r1,[r0,#0x388]
    ldr     r1,=0x000000f0
    str     r1,[r0,#0x38C]
    ldr     r1,=0x00000103
    str     r1,[r0,#0x390]
    str     r1,[r0,#0x394]
    str     r1,[r0,#0x398]
    str     r1,[r0,#0x39C]

    ldr     r1,=0x00000010
    str     r1,[r0,#0x190]
    ldr     r1,=0x0000000
    str     r1,[r0,#0x194]
    str     r1,[r0,#0x198]
    str     r1,[r0,#0x19C]
    str     r1,[r0,#0x1A0]
    str     r1,[r0,#0x1A4]
        
    ldmfd    r13!,{r0,r1,r14}           
    mov pc,r14
            
            
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
;       command Arguement(r1)
;           [in] - Command Argument 
;
;
;   mov pc,r14s:
;       None.
;-------------------------------------------------------------------------------
ASM_SD_Issue_Cmd
    stmfd    r13!,{r1,r2,r3,r14}
    
    ;clear interrupt status
    mov     r3,#0xFFFFFFFF
    ldr     r2,=(IRQSTAT)
    str     r3, [r2]
    
    ;Set CMDARG Register
    ldr     r2,=(CMDARG)
    str     r1, [r2]

    ;Set XFERTYP Register which issues the command on the SDBus
    ldr     r2,=(XFERTYP)
    str     r0, [r2]
    ldmfd    r13!,{r1,r2,r3,r14}        
    mov pc,r14          

;-------------------------------------------------------------------------------
;
;   Function:  SDHCWaitEndCmdRespIntr
;
;     Wait for the response after issuing the command.
;
;   Parameters:
;       None.
;
;   mov pc,r14s:
;       Status (r0)
;           [out] - 0 - Fail , 1 - Pass
;-------------------------------------------------------------------------------
ASM_SDHCWaitEndCmdRespIntr    
    stmfd    r13!,{r1,r2,r14}
    

    ldr     r0,=(IRQSTAT)
 
    ; r0 : status mask = error bits | Command complete
    ldr     r2,=0x000F0001  
    
wait_end_cmd_loop    
   ldr     r1,[r0]
   ands    r1,r1,r2
   beq      wait_end_cmd_loop

    
    ;Default mov pc,r14 Value
    ldr     r0,=ESDHC_STATUS_PASS    
    ;Check for Command Timeout, Command CRC, Command End Bit, and Command Index errors
    ldr     r2,=0x000F0000
    ands    r1,r1,r2    
    movne   r0,#ESDHC_STATUS_FAILURE
   
    ldmfd   r13!,{r1,r2,r14}
    
    mov pc,r14
            
;-------------------------------------------------------------------------------
;
;   Function:  SDHCDataRead
;
;   Read the data from the card. 
;
;   Parameters:
;      Destination Memory Address                  (r0)
;      Size of the transfer in DWORD (32 bits)     (r1)
;
;   mov pc,r14s:
;       Status (r0)
;           [out] - 0 - Fail , 1 - Pass
;-------------------------------------------------------------------------------
ASM_SDHCDataRead
    
    stmfd    r13!,{r1-r6,r14}

    ldr     r2,=CSP_BASE_REG_PA_ESDHC1
    
    ; get the burst length
    ldr     r3,[r2,#OFFSET_WML]
    and     r3,r3,#0xFF         ;r3 = burstlength in DWORD 

1
    ;Loop for BRR to get Set
wait_brr
    ldr     r4,[r2,#OFFSET_IRQSTAT]
    ;Check if BRR is set
    ands    r4,r4,#0x20
    beq      wait_brr
    
    ; Read burstlength DWORD from fifo and store at address r0
    mov     r5,r3
2   ldr     r6,[r2,#OFFSET_DATPORT]
    str     r6,[r0], #4 ; store the DWORD at address R0 and increment r0 by 4
    sub     r5,r5,#1    
    cmp     r5, #0      ; Done?
    bne     %B2
        
    ;clear the BRR bit.
    
    mov     r4,#(0x20)
    str     r4,[r2,#OFFSET_IRQSTAT]
    
    ; remaining DWORD = remaining DWORD - burstlength
    sub     r1,r1,r3
    cmp     r1,#0
    bne     %B1
        
    mov   r0,#ESDHC_STATUS_PASS    
    ldmfd    r13!,{r1-r6,r14}
    mov pc,r14


;-------------------------------------------------------------------------------
;
;   Function:  SDMMC_card_software_reset
;
;  This function will issue CMD0 to card. This gives software reset to card.
;
;   Parameters:
;      No parameters
;
;
;
;   mov pc,r14s:
;       No mov pc,r14 value
;-------------------------------------------------------------------------------    
SDMMC_card_software_reset
    stmfd    r13!,{r0,r1,r14}
       
    mov r0,#0   ;SD_CMD_GO_IDLE_STATE
    mov r1,#0
    bl ASM_SD_Issue_Cmd
    bl ASM_SDHCWaitEndCmdRespIntr
        
    ldmfd    r13!,{r0,r1,r14}       
    mov pc,r14
    
    

;-------------------------------------------------------------------------------
;
;   Function:  SDCard_sendCmdAndWait
;
;    This function sends a command and wait for the response
;
;   Parameters:
;      r0 : XFERTYPE registers
;      r1 : argument
;
;       Status (r0)
;           [out] - 0 - Fail , 1 - Pass
;-------------------------------------------------------------------------------        
SDCard_sendCmdAndWait
    stmfd    r13!,{r14}       
    
    ;issue the command
    bl ASM_SD_Issue_Cmd
    
    ;Wait for interrupt end_command_resp
    bl ASM_SDHCWaitEndCmdRespIntr
    
    ldmfd    r13!,{r14}       
    mov pc,r14
    
;-------------------------------------------------------------------------------
;
;   Function:  ASM_SDMMC_R1b_busy_wait
;
;    This function will wait during a command with R1b response type till the
;    SD state changes back to normal;
;
;   Parameters:
;      No parameters
;
;       Status (r0)
;           [out] - 0 - Fail , 1 - Pass
;-------------------------------------------------------------------------------    
ASM_SDMMC_R1b_busy_wait
    stmfd    r13!,{r1,r2,r3,r14}       
    
    mov     r3,#0
    ldr     r2,=CSP_BASE_REG_PA_ESDHC1
    ldr     r1,CardAddress
1    
    ldr     r0,=XFERTYPE_SD_CMD_SEND_STATUS    
    bl      SDCard_sendCmdAndWait
    cmp     r0,#ESDHC_STATUS_FAILURE
    ldrne   r3,[r2,#OFFSET_CMDRSP0]
    and     r3,r3,#0x00001E00
    cmp     r3,#(4<<9)
    bne     %B1
    
    mov     r0,#ESDHC_STATUS_PASS
    ldmfd    r13!,{r1,r2,r3,r14}
    mov pc,r14

;------------------------------------------------------------------------------
;
; Function: ASM_SDMMC_set_blocklen
;
;    This function will will set the maximum read/write Block Length
;
; Parameters:
;        r0 : the block length
;
; mov pc,r14s:
;        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
;
;------------------------------------------------------------------------------ 
ASM_SDMMC_set_blocklen
    stmfd    r13!,{r1,r14}
    
    mov     r1,r0
    ldr     r0,=XFERTYPE_SD_CMD_SET_BLOCKLEN
    bl      SDCard_sendCmdAndWait
    
    ldmfd    r13!,{r1,r14}
    mov pc,r14 

;------------------------------------------------------------------------------
;
; Function: ASM_SDMMC_card_send_appcmd
;
;    This function will issue an APP CMD (CMD55) to the card
;
; Parameters:
;        no parameter
;
; mov pc,r14s:
;        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
;
;------------------------------------------------------------------------------ 
ASM_SDMMC_card_send_appcmd
    stmfd    r13!,{r1,r14}
    
    ldr     r1,CardAddress
    ldr     r0,=XFERTYPE_SD_CMD_APP_CMD
    bl      SDCard_sendCmdAndWait
    
    ldmfd    r13!,{r1,r14}
    mov pc,r14 


;-------------------------------------------------------------------------------
;
;   Function:  ASM_DMMC_card_send_appcmd
;
;  This function sends the app cmd
;
;   Parameters:
;      r2: card adress
;
;   mov pc,r14s:
;      r2: status
;-------------------------------------------------------------------------------    
ASM_DMMC_card_send_appcmd
    
    stmfd    r13!,{r0,r14}       
    
    mov r0,#55    
    bl ASM_SD_Issue_Cmd
    bl ASM_SDHCWaitEndCmdRespIntr        
    
    ldmfd    r13!,{r0,r14}
        
    mov pc,r14 
    


;-------------------------------------------------------------------------------
;
;   Function:  ASM_SetClockRate25MHz
;
;  This function sets the bus speed to 25MHz (assumption : the IPG Clock is 66MHz)
;
;-------------------------------------------------------------------------------    
ASM_SetClockRate25MHz
    
    stmfd    r13!,{r0,r1,r14}
    
    ;clear SDCLKEN bit first before changing frequency    
    ldr     r0,=(SYSCTL)
    ldr     r1,[r0]
    and     r1,r1,#~(1<<3)
    str     r1,[r0]
    
    ;Configure the new dividers and turn on the clocks
    ldr     r1,=0xe0027
    str     r1,[r0]
            
    ldmfd    r13!,{r0,r1,r14}
        
    mov pc,r14 


;-----------------------------------------------------------------------------
;
; Function: ASM_SDMMC_ReadSectors
;
; Function Reads the requested sector data and metadata from the flash media. 
;
; Parameters:
;      r0   :   starting sector address.
;
;      r1  :   Ptr to  buffer that contains sector data read from flash.
;
;      r2   :   Number of sectors to read.
;
; mov pc,r14s:  
;        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
;
;-----------------------------------------------------------------------------      

ASM_SDMMC_ReadSectors
    
    stmfd    r13!,{r1-r6,r14}
    
    ldr     r4,=CSP_BASE_REG_PA_ESDHC1
        
    ; If the card is high density then the arument is the sector addr, otherwise it's sectore addr * sector length
    ldr     r3,HighDensity
    cmp     r3,#1
    movne   r0,r0,LSL #LOG2_SDHC_BLK_LEN
    
    ; set the block length and the number of block in BLKATTR
    mov     r3,r2,LSL #16
    orr     r3,r3,#SDHC_BLK_LEN
    str     r3,[r4,#OFFSET_BLKATTR]
    
    ; save the buffer addr
    mov     r5,r1
    ;compute the total length in DWORD
    mov     r6,r2,LSL #(LOG2_SDHC_BLK_LEN - 2)
    
    ; issue the read multiple block command
    mov     r1,r0
    ldr     r0,=XFERTYPE_SD_CMD_READ_MULTIPLE_BLOCK
    bl      SDCard_sendCmdAndWait
    cmp     r0,#ESDHC_STATUS_PASS
    bne     error
        
    ; read the data from the fifo
    mov     r0,r5
    mov     r1,r6
    bl      ASM_SDHCDataRead
    
    ; wait for the transfer to complete (JJH : useles ??)
    bl ASM_SDMMC_R1b_busy_wait
    
    mov     r0,#ESDHC_STATUS_PASS    
    b   %F1
error
    mov     r0,#ESDHC_STATUS_FAILURE
    
1   ldmfd    r13!,{r1-r6,r14}
    
    mov pc,r14      

;-----------------------------------------------------------------------------
;
;  Function: ASM_SDMMC_set_data_transfer_mode
;
;  This function will put card in transfer mode.
;
;  Parameters:
;        None.
;
;  mov pc,r14s:
;        ESDHC_STATUS_PASS or ESDHC_STATUS_FAILURE
;
;-----------------------------------------------------------------------------

ASM_SDMMC_set_data_transfer_mode
    
    stmfd    r13!,{r1,r14}
    
    ldr     r0,=XFERTYPE_SD_CMD_SELECT_DESELECT_CARD
    ldr     r1,CardAddress
    bl      SDCard_sendCmdAndWait
    bl      ASM_SDMMC_R1b_busy_wait
    
    ldmfd    r13!,{r1,r14}
    
    mov pc,r14

;------------------------------------------------------------------------------
;
; Function: ASM_SDMMC_send_rca
;
;    This function will send/set RCA for SD/MMC card..
;
;    no parameter
;
; mov pc,r14s:
;        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
;
;------------------------------------------------------------------------------     

ASM_SDMMC_send_rca
    
    stmfd    r13!,{r1,r2,r3,r14}
    
    mov     r2,#0       ;default card address
    ldr     r1,IsMMC    
    cmp     r1,#1       
    bne     %F1         
    mov     r2,#1       ;if the card is MMC, then RCA = 1 and cardadress is (1<<16)
    str     r2,CardRCA
    mov     r2,r2,LSL #16
    str     r2,CardAddress
        
1   mov     r1,r2
    ldr     r0,=XFERTYPE_SD_CMD_MMC_SET_RCA
    bl      SDCard_sendCmdAndWait
 
    ldr     r1,IsMMC    
    cmp     r1,#1   
    beq     %F2
               
    ldr     r2,=(CMDRSP0)
    ldr     r2,[r2]
    ldr     r3,=0xFFFF0000
    and     r2,r2,r3
    str     r2,CardAddress
    mov     r2,r2,LSR #16
    str     r2,CardRCA
2    
    mov     r0, #ESDHC_STATUS_PASS
    
    ldmfd   r13!,{r1,r2,r3,r14}
    
    mov pc,r14
    
;-----------------------------------------------------------------------------
;
;  Function: SDMMC_get_cid
;
;  This function will mov pc,r14 the CID value of card.
;
;  Parameters:
;        None.
;
;  mov pc,r14s:
;        ESDHC_STATUS_PASS or ESDHC_STATUS_FAILURE
;
;-----------------------------------------------------------------------------    
    
SDMMC_get_cid
    
    stmfd   r13!,{r1,r14}
    
    ldr     r0,=XFERTYPE_SD_CMD_ALL_SEND_CID
    mov     r1,#0
    bl      SDCard_sendCmdAndWait
    
    ldmfd   r13!,{r1,r14}
    
    mov pc,r14
    
;------------------------------------------------------------------------------
;
; Function: MMC_VoltageValidation
;
;    This function will validate the operating voltage range of MMCMemory card.
;
; Parameters:
;        None
;
; mov pc,r14s:
;        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
;
;------------------------------------------------------------------------------
        
MMC_VoltageValidation
    
    stmfd   r13!,{r1,r14}
    
1    
    ldr     r0,=XFERTYPE_SD_CMD_MMC_SEND_OPCOND
    ldr     r1,=MMC_OCR_VALUE    
    bl      SDCard_sendCmdAndWait
    
    cmp     r0,#ESDHC_STATUS_FAILURE   
    beq     %F2
    
    ldr     r0,=(CMDRSP0)
    ldr     r0,[r0]
    ands    r0,r0,#(1<<31)
    beq     %B1
        
    mov     r1,#1
    mov     r0,#ESDHC_STATUS_PASS
    b %F3

2    
    mov     r0,#ESDHC_STATUS_FAILURE
 
3
    str     r1,(IsMMC)
    ldmfd   r13!,{r1,r14}
    
    mov pc,r14
        
;------------------------------------------------------------------------------
;
; Function: SD_VoltageValidation
;
;    This function will validate the operating voltage range of SDMemory card.
;
; Parameters:
;        None
;
; mov pc,r14s:
;        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
;
;------------------------------------------------------------------------------

SD_VoltageValidation
    
    stmfd   r13!,{r1,r2,r14}
    

    ldr     r0,=XFERTYPE_SD_CMD_SEND_IF_COND
    ldr     r1,=0x1AA    
    bl      SDCard_sendCmdAndWait
    
1   bl ASM_SDMMC_card_send_appcmd
    cmp     r0,#ESDHC_STATUS_FAILURE
    beq     %F2
    
    
    ldr     r0,=XFERTYPE_SD_ACMD_SD_SEND_OP_COND
    ldr     r1,=SD_OCR_VALUE
    bl      SDCard_sendCmdAndWait
    cmp     r0,#ESDHC_STATUS_FAILURE
    beq     %F2
    
    ldr     r0,=(CMDRSP0)
    ldr     r0,[r0]
    ands    r1,r0,#(1<<31)
    beq     %B1

    mov     r2,#0    
    and     r1,r0,#0x40000000
    cmp     r1,#0
    movne   r2,#1
    str     r2,(HighDensity)               
        
    
    mov     r0,#ESDHC_STATUS_PASS
    b %F3

2    
    mov     r0,#ESDHC_STATUS_FAILURE
 
3
    
    ldmfd   r13!,{r1,r2,r14}
    
    mov pc,r14

;-----------------------------------------------------------------------------
;
;  Function: SDMMC_Init
;
;  This function initializes the SD interface.
;
;  Parameters:
;        None.
;
;  mov pc,r14s:
;        None.
;
;-----------------------------------------------------------------------------
SDMMC_Init
    
    stmfd   r13!,{r1,r2,r14}
    
    bl      SD_VoltageValidation   
    cmp     r0,#ESDHC_STATUS_PASS    
    beq     %F3
    
    bl      MMC_VoltageValidation
    cmp     r0,#ESDHC_STATUS_PASS
    bne     %F4
3    
    bl      SDMMC_get_cid    
    cmp     r0,#ESDHC_STATUS_PASS
    bne     %F4
    
    bl      ASM_SDMMC_send_rca
    cmp     r0,#ESDHC_STATUS_PASS
    bne     %F4

    bl      ASM_SetClockRate25MHz        
    
    bl      ASM_SDMMC_set_data_transfer_mode
    cmp     r0,#ESDHC_STATUS_PASS
    bne     %F4
    
    mov     r0,#SDHC_BLK_LEN
    bl      ASM_SDMMC_set_blocklen
        
4    
    ldmfd   r13!,{r1,r2,r14}
    
    mov pc,r14

;-------------------------------------------------------------------------------
;   Global variables        
;-------------------------------------------------------------------------------
    EXPORT IsMMC
    EXPORT CardAddress
    EXPORT CardRCA
    EXPORT HighDensity
IsMMC
    DCD 0    
CardRCA
    DCD 0
CardAddress
    DCD 0
HighDensity
    DCD 0       

    END
