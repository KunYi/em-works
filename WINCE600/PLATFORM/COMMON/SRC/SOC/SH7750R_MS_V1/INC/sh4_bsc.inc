;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
;
;;------------------------------------------------------------------------------
;;
;;  Copyright(c) 1998,1999 Renesas Technology Corp.
;;
;;------------------------------------------------------------------------------
;;
;;  File:   sh4_bsc.inc
;;
;;------------------------------------------------------------------------------

;
; Bus state controller registers.    
;

BSC_REGBASE                 .equ    h'FF800000  ; Bus Stage Control Register Base Address 
BSC_REGSIZE                 .equ    h'2C        ; Size of all of the BSC regs 

BSC_BCR1_OFFSET             .equ    h'0000      ; Bus Control Register 1 Offset Address 
BSC_BCR2_OFFSET             .equ    h'0004      ; Bus Control Register 2 Offset Address
BSC_BCR3_OFFSET             .equ    h'0050      ; Bus Control Register 3 Offset Address
BSC_WCR1_OFFSET             .equ    h'0008      ; Wait state Control Register 1 Offset Address
BSC_WCR2_OFFSET             .equ    h'000C      ; Wait state Control Register 2 Offset Address
BSC_WCR3_OFFSET             .equ    h'0010      ; Wait state Control Register 3 Offset Address
BSC_MCR_OFFSET              .equ    h'0014      ; Individual Memory Control Register Offset Address
BSC_PCR_OFFSET              .equ    h'0018      ; PCMCIA Control Register Offset Address
BSC_RTCSR_OFFSET            .equ    h'001C      ; Refresh Timer Control/Status Register Offset Address
BSC_RTCNT_OFFSET            .equ    h'0020      ; Refresh Timer Counter Offset Address
BSC_RTCOR_OFFSET            .equ    h'0024      ; Refresh Time Constant Register Offset Address
BSC_RFCR_OFFSET             .equ    h'0028      ; Refresh Count Register Offset Address

BSC_BCR1                    .equ    (BSC_REGBASE + BSC_BCR1_OFFSET) ; Bus Control Register 1 Offset Address 
BSC_BCR2                    .equ    (BSC_REGBASE + BSC_BCR2_OFFSET) ; Bus Control Register 2 Offset Address
BSC_BCR3                    .equ    (BSC_REGBASE + BSC_BCR3_OFFSET) ; Bus Control Register 3 Offset Address
BSC_WCR1                    .equ    (BSC_REGBASE + BSC_WCR1_OFFSET) ; Wait state Control Register 1 Offset Address
BSC_WCR2                    .equ    (BSC_REGBASE + BSC_WCR2_OFFSET) ; Wait state Control Register 2 Offset Address
BSC_WCR3                    .equ    (BSC_REGBASE + BSC_WCR3_OFFSET) ; Wait state Control Register 3 Offset Address
BSC_MCR                     .equ    (BSC_REGBASE + BSC_MCR_OFFSET)  ; Individual Memory Control Register Offset Address
BSC_PCR                     .equ    (BSC_REGBASE + BSC_PCR_OFFSET)  ; PCMCIA Control Register Offset Address
BSC_RTCSR                   .equ    (BSC_REGBASE + BSC_RTCSR_OFFSET); Refresh Timer Control/Status Register Offset Address
BSC_RTCNT                   .equ    (BSC_REGBASE + BSC_RTCNT_OFFSET); Refresh Timer Counter Offset Address
BSC_RTCOR                   .equ    (BSC_REGBASE + BSC_RTCOR_OFFSET); Refresh Time Constant Register Offset Address
BSC_RFCR                    .equ    (BSC_REGBASE + BSC_RFCR_OFFSET) ; Refresh Count Register Offset Address

; BCR1  bus control register 1 fields (function and bus cycle status for each area).    

BSC_BCR1_ENDIAN:            .equ    h'80000000    ; 1 <=> little endian
BSC_BCR1_MASTER:            .equ    h'40000000    ; 0:Master, 1:Slave 
BSC_BCR1_A0MPX:             .equ    h'20000000    ; 0:SRAM, 1:MPX 
BSC_BCR1_IPUP:              .equ    h'02000000    ; 0:pulled up, 1:not pulled up 
BSC_BCR1_OPUP:              .equ    h'01000000    ; 0:pulled up, 1:not pulled up 

BSC_BCR1_A1MBC:             .equ    h'00200000    ; 0:normal, 1:byte control mode 
BSC_BCR1_A4MBC:             .equ    h'00100000    ; 0:normal, 1:byte control mode 
BSC_BCR1_BREQEN:            .equ    h'00080000    ; BREQ Enable Bit 
BSC_BCR1_PSHR:              .equ    h'00040000    ; Partial-Sharing Bit 
BSC_BCR1_MEMMPX:            .equ    h'00020000    ; Area 1 to Area 6 MPX Bus Specification 

BSC_BCR1_HIZMEM:            .equ    h'00008000    ; High-Z Control(HIZMEM) 
BSC_BCR1_HIZCNT:            .equ    h'00004000    ; High-Z Control(HIZCNT) 

BSC_BCR1_A0BST:             .equ    h'00003800    ; Area 0 burst mask
BSC_BCR1_A0BST_N:           .equ    h'00000000    ; Area 0 ordinary memory
BSC_BCR1_A0BST_4:           .equ    h'00000800    ; Area 0 burst (4 consecutive accesses)
BSC_BCR1_A0BST_8:           .equ    h'00001000    ; Area 0 burst (8 consecutive accesses)
BSC_BCR1_A0BST_16:          .equ    h'00001800    ; Area 0 burst (16 consecutive accesses)
BSC_BCR1_A0BST_32:          .equ    h'00002000    ; Area 0 burst (32 consecutive accesses)

BSC_BCR1_A5BST:             .equ    h'00000700    ; Area 5 burst mask
BSC_BCR1_A5BST_N:           .equ    h'00000000    ; Area 5 ordinary memory
BSC_BCR1_A5BST_4:           .equ    h'00000100    ; Area 5 burst (4 consecutive accesses)
BSC_BCR1_A5BST_8:           .equ    h'00000200    ; Area 5 burst (8 consecutive accesses)
BSC_BCR1_A5BST_16:          .equ    h'00000300    ; Area 5 burst (16 consecutive accesses)
BSC_BCR1_A5BST_32:          .equ    h'00000400    ; Area 5 burst (32 consecutive accesses)

BSC_BCR1_A6BST:             .equ    h'000000e0    ; Area 6 burst mask
BSC_BCR1_A6BST_N:           .equ    h'00000000    ; Area 6 ordinary memory
BSC_BCR1_A6BST_4:           .equ    h'00000020    ; Area 6 burst (4 consecutive accesses)
BSC_BCR1_A6BST_8:           .equ    h'00000040    ; Area 6 burst (8 consecutive accesses)
BSC_BCR1_A6BST_16:          .equ    h'00000060    ; Area 6 burst (16 consecutive accesses)
BSC_BCR1_A6BST_32:          .equ    h'00000080    ; Area 6 burst (32 consecutive accesses)

BSC_BCR1_DRAM:              .equ    h'0000001c    ; Areas 2 & 3 mask
BSC_BCR1_DRAM_A2N3N:        .equ    h'00000000    ; Area 2 normal, Area 3 normal
BSC_BCR1_DRAM_A2N3P:        .equ    h'00000004    ; Area 2 normal, Area 3 PSRAM
BSC_BCR1_DRAM_A2N3S:        .equ    h'00000008    ; Area 2 normal, Area 3 SDRAM
BSC_BCR1_DRAM_A2S3S:        .equ    h'0000000c    ; Area 2 SDRAM, Area 3 SDRAM
BSC_BCR1_DRAM_A2N3D:        .equ    h'00000010    ; Area 2 normal, Area 3 DRAM
BSC_BCR1_DRAM_A2D3D:        .equ    h'00000014    ; Area 2 DRAM, Area 3 DRAM

BSC_BCR1_A56PCM:            .equ    h'00000001

; BCR2: bus control register 2 fields. (bus size width of each area)

BSC_BCR2_A0SZ:              .equ    h'c000    ; Area 0 mask 
BSC_BCR2_A0SZ_8:            .equ    h'4000    ; Area 0 is  8-bit memory 
BSC_BCR2_A0SZ_16:           .equ    h'8000    ; Area 0 is 16-bit memory 
BSC_BCR2_A0SZ_32:           .equ    h'c000    ; Area 0 is 32-bit memory 
BSC_BCR2_A0SZ_64:           .equ    h'0000    ; Area 0 is 64-bit memory 

BSC_BCR2_A6SZ:              .equ    h'3000    ; Area 6 mask
BSC_BCR2_A6SZ_8:            .equ    h'1000    ; Area 6 is  8-bit memory
BSC_BCR2_A6SZ_16:           .equ    h'2000    ; Area 6 is 16-bit memory
BSC_BCR2_A6SZ_32:           .equ    h'3000    ; Area 6 is 32-bit memory
BSC_BCR2_A6SZ_64:           .equ    h'0000    ; Area 6 is 64-bit memory

BSC_BCR2_A5SZ:              .equ    h'0c00    ; Area 5 mask
BSC_BCR2_A5SZ_8:            .equ    h'0400    ; Area 5 is  8-bit memory
BSC_BCR2_A5SZ_16:           .equ    h'0800    ; Area 5 is 16-bit memory
BSC_BCR2_A5SZ_32:           .equ    h'0c00    ; Area 5 is 32-bit memory
BSC_BCR2_A5SZ_64:           .equ    h'0000    ; Area 5 is 64-bit memory

BSC_BCR2_A4SZ:              .equ    h'0300    ; Area 4 mask
BSC_BCR2_A4SZ_8:            .equ    h'0100    ; Area 4 is  8-bit memory
BSC_BCR2_A4SZ_16:           .equ    h'0200    ; Area 4 is 16-bit memory
BSC_BCR2_A4SZ_32:           .equ    h'0300    ; Area 4 is 32-bit memory
BSC_BCR2_A4SZ_64:           .equ    h'0000    ; Area 4 is 64-bit memory

BSC_BCR2_A3SZ:              .equ    h'00c0    ; Area 3 mask
BSC_BCR2_A3SZ_8:            .equ    h'0040    ; Area 3 is  8-bit memory
BSC_BCR2_A3SZ_16:           .equ    h'0080    ; Area 3 is 16-bit memory
BSC_BCR2_A3SZ_32:           .equ    h'00c0    ; Area 3 is 32-bit memory
BSC_BCR2_A3SZ_64:           .equ    h'0000    ; Area 3 is 64-bit memory

BSC_BCR2_A2SZ:              .equ    h'0030    ; Area 2 mask
BSC_BCR2_A2SZ_8:            .equ    h'0010    ; Area 2 is  8-bit memory
BSC_BCR2_A2SZ_16:           .equ    h'0020    ; Area 2 is 16-bit memory
BSC_BCR2_A2SZ_32:           .equ    h'0030    ; Area 2 is 32-bit memory
BSC_BCR2_A2SZ_64:           .equ    h'0000    ; Area 2 is 64-bit memory

BSC_BCR2_A1SZ:              .equ    h'000c    ; Area 1 mask
BSC_BCR2_A1SZ_8:            .equ    h'0004    ; Area 1 is  8-bit memory
BSC_BCR2_A1SZ_16:           .equ    h'0008    ; Area 1 is 16-bit memory
BSC_BCR2_A1SZ_32:           .equ    h'000c    ; Area 1 is 32-bit memory
BSC_BCR2_A1SZ_64:           .equ    h'0000    ; Area 1 is 64-bit memory

BSC_BCR2_PORTEN:            .equ    h'0001    ; Port enable

; WCR1: Wait (idle) state control register 1 fields.

BSC_WCR1_DMAIW:             .equ    h'70000000    ; DMA mask 
BSC_WCR1_DMAIW_0:           .equ    h'00000000    ; DMA has 0 idle states 
BSC_WCR1_DMAIW_1:           .equ    h'10000000    ; DMA has 1 idle states 
BSC_WCR1_DMAIW_2:           .equ    h'20000000    ; DMA has 2 idle states 
BSC_WCR1_DMAIW_3:           .equ    h'30000000    ; DMA has 3 idle states 
BSC_WCR1_DMAIW_6:           .equ    h'40000000    ; DMA has 6 idle states 
BSC_WCR1_DMAIW_9:           .equ    h'50000000    ; DMA has 9 idle states 
BSC_WCR1_DMAIW_12:          .equ    h'60000000    ; DMA has 12 idle states 
BSC_WCR1_DMAIW_15:          .equ    h'70000000    ; DMA has 15 idle states 

BSC_WCR1_A6IW:              .equ    h'07000000    ; Area 6 mask
BSC_WCR1_A6IW_0:            .equ    h'00000000    ; Area 6 has 0 idle states
BSC_WCR1_A6IW_1:            .equ    h'01000000    ; Area 6 has 1 idle states
BSC_WCR1_A6IW_2:            .equ    h'02000000    ; Area 6 has 2 idle states
BSC_WCR1_A6IW_3:            .equ    h'03000000    ; Area 6 has 3 idle states
BSC_WCR1_A6IW_6:            .equ    h'04000000    ; Area 6 has 6 idle states
BSC_WCR1_A6IW_9:            .equ    h'05000000    ; Area 6 has 9 idle states
BSC_WCR1_A6IW_12:           .equ    h'06000000    ; Area 6 has 12 idle states
BSC_WCR1_A6IW_15:           .equ    h'07000000    ; Area 6 has 15 idle states

BSC_WCR1_A5IW:              .equ    h'00700000    ; Area 5 mask
BSC_WCR1_A5IW_0:            .equ    h'00000000    ; Area 5 has 0 idle states
BSC_WCR1_A5IW_1:            .equ    h'00100000    ; Area 5 has 1 idle states
BSC_WCR1_A5IW_2:            .equ    h'00200000    ; Area 5 has 2 idle states
BSC_WCR1_A5IW_3:            .equ    h'00300000    ; Area 5 has 3 idle states
BSC_WCR1_A5IW_6:            .equ    h'00400000    ; Area 5 has 6 idle states
BSC_WCR1_A5IW_9:            .equ    h'00500000    ; Area 5 has 9 idle states
BSC_WCR1_A5IW_12:           .equ    h'00600000    ; Area 5 has 12 idle states
BSC_WCR1_A5IW_15:           .equ    h'00700000    ; Area 5 has 15 idle states

BSC_WCR1_A4IW:              .equ    h'00070000    ; Area 4 mask
BSC_WCR1_A4IW_0:            .equ    h'00000000    ; Area 4 has 0 idle states
BSC_WCR1_A4IW_1:            .equ    h'00010000    ; Area 4 has 1 idle states
BSC_WCR1_A4IW_2:            .equ    h'00020000    ; Area 4 has 2 idle states
BSC_WCR1_A4IW_3:            .equ    h'00030000    ; Area 4 has 3 idle states
BSC_WCR1_A4IW_6:            .equ    h'00040000    ; Area 4 has 6 idle states
BSC_WCR1_A4IW_9:            .equ    h'00050000    ; Area 4 has 9 idle states
BSC_WCR1_A4IW_12:           .equ    h'00060000    ; Area 4 has 12 idle states
BSC_WCR1_A4IW_15:           .equ    h'00070000    ; Area 4 has 15 idle states

BSC_WCR1_A3IW:              .equ    h'00007000    ; Area 3 mask
BSC_WCR1_A3IW_0:            .equ    h'00000000    ; Area 3 has 0 idle states
BSC_WCR1_A3IW_1:            .equ    h'00001000    ; Area 3 has 1 idle states
BSC_WCR1_A3IW_2:            .equ    h'00002000    ; Area 3 has 2 idle states
BSC_WCR1_A3IW_3:            .equ    h'00003000    ; Area 3 has 3 idle states
BSC_WCR1_A3IW_6:            .equ    h'00004000    ; Area 3 has 6 idle states
BSC_WCR1_A3IW_9:            .equ    h'00005000    ; Area 3 has 9 idle states
BSC_WCR1_A3IW_12:           .equ    h'00006000    ; Area 3 has 12 idle states
BSC_WCR1_A3IW_15:           .equ    h'00007000    ; Area 3 has 15 idle states

BSC_WCR1_A2IW:              .equ    h'00000700    ; Area 2 mask
BSC_WCR1_A2IW_0:            .equ    h'00000000    ; Area 2 has 0 idle states
BSC_WCR1_A2IW_1:            .equ    h'00000100    ; Area 2 has 1 idle states
BSC_WCR1_A2IW_2:            .equ    h'00000200    ; Area 2 has 2 idle states
BSC_WCR1_A2IW_3:            .equ    h'00000300    ; Area 2 has 3 idle states
BSC_WCR1_A2IW_6:            .equ    h'00000400    ; Area 2 has 6 idle states
BSC_WCR1_A2IW_9:            .equ    h'00000500    ; Area 2 has 9 idle states
BSC_WCR1_A2IW_12:           .equ    h'00000600    ; Area 2 has 12 idle states
BSC_WCR1_A2IW_15:           .equ    h'00000700    ; Area 2 has 15 idle states

BSC_WCR1_A1IW:              .equ    h'00000070    ; Area 1 mask
BSC_WCR1_A1IW_0:            .equ    h'00000000    ; Area 1 has 0 idle states
BSC_WCR1_A1IW_1:            .equ    h'00000010    ; Area 1 has 1 idle states
BSC_WCR1_A1IW_2:            .equ    h'00000020    ; Area 1 has 2 idle states
BSC_WCR1_A1IW_3:            .equ    h'00000030    ; Area 1 has 3 idle states
BSC_WCR1_A1IW_6:            .equ    h'00000040    ; Area 1 has 6 idle states
BSC_WCR1_A1IW_9:            .equ    h'00000050    ; Area 1 has 9 idle states
BSC_WCR1_A1IW_12:           .equ    h'00000060    ; Area 1 has 12 idle states
BSC_WCR1_A1IW_15:           .equ    h'00000070    ; Area 1 has 15 idle states

BSC_WCR1_A0IW:              .equ    h'00000007    ; Area 0 mask
BSC_WCR1_A0IW_0:            .equ    h'00000000    ; Area 0 has 0 idle states
BSC_WCR1_A0IW_1:            .equ    h'00000001    ; Area 0 has 1 idle states
BSC_WCR1_A0IW_2:            .equ    h'00000002    ; Area 0 has 2 idle states
BSC_WCR1_A0IW_3:            .equ    h'00000003    ; Area 0 has 3 idle states
BSC_WCR1_A0IW_6:            .equ    h'00000004    ; Area 0 has 6 idle states
BSC_WCR1_A0IW_9:            .equ    h'00000005    ; Area 0 has 9 idle states
BSC_WCR1_A0IW_12:           .equ    h'00000006    ; Area 0 has 12 idle states
BSC_WCR1_A0IW_15:           .equ    h'00000007    ; Area 0 has 15 idle states

; WCR2: Wait state control register 2 fields.

BSC_WCR2_A6W:               .equ    h'e0000000    ; Area 6 wait state mask
BSC_WCR2_A6W_0:             .equ    h'00000000    ; Area 6 has 0 wait states
BSC_WCR2_A6W_1:             .equ    h'20000000    ; Area 6 has 1 wait states
BSC_WCR2_A6W_2:             .equ    h'40000000    ; Area 6 has 2 wait states
BSC_WCR2_A6W_3:             .equ    h'60000000    ; Area 6 has 3 wait states
BSC_WCR2_A6W_6:             .equ    h'80000000    ; Area 6 has 6 wait states 
BSC_WCR2_A6W_9:             .equ    h'a0000000    ; Area 6 has 9 wait states 
BSC_WCR2_A6W_12:            .equ    h'c0000000    ; Area 6 has 12 wait states 
BSC_WCR2_A6W_15:            .equ    h'e0000000    ; Area 6 has 15 wait states 

BSC_WCR2_A6B:               .equ    h'1c000000    ; Area 6 burst sycle mask 
BSC_WCR2_A6B_0:             .equ    h'00000000    ; Area 6 has 0 states per data transfer 
BSC_WCR2_A6B_1:             .equ    h'04000000    ; Area 6 has 1 states per data transfer  
BSC_WCR2_A6B_2:             .equ    h'08000000    ; Area 6 has 2 states per data transfer 
BSC_WCR2_A6B_3:             .equ    h'0c000000    ; Area 6 has 3 states per data transfer 
BSC_WCR2_A6B_4:             .equ    h'10000000    ; Area 6 has 4 states per data transfer 
BSC_WCR2_A6B_5:             .equ    h'14000000    ; Area 6 has 5 states per data transfer 
BSC_WCR2_A6B_6:             .equ    h'18000000    ; Area 6 has 6 states per data transfer 
BSC_WCR2_A6B_7:             .equ    h'1c000000    ; Area 6 has 7 states per data transfer 

BSC_WCR2_A5W:               .equ    h'03800000    ; Area 5 wait state mask
BSC_WCR2_A5W_0:             .equ    h'00000000    ; Area 5 has 0 wait states
BSC_WCR2_A5W_1:             .equ    h'00800000    ; Area 5 has 1 wait states
BSC_WCR2_A5W_2:             .equ    h'01000000    ; Area 5 has 2 wait states
BSC_WCR2_A5W_3:             .equ    h'01800000    ; Area 5 has 3 wait states
BSC_WCR2_A5W_6:             .equ    h'02000000    ; Area 5 has 6 wait states 
BSC_WCR2_A5W_9:             .equ    h'02800000    ; Area 5 has 9 wait states 
BSC_WCR2_A5W_12:            .equ    h'03000000    ; Area 5 has 12 wait states 
BSC_WCR2_A5W_15:            .equ    h'03800000    ; Area 5 has 15 wait states 

BSC_WCR2_A5B:               .equ    h'00700000    ; Area 5 burst sycle mask 
BSC_WCR2_A5B_0:             .equ    h'00000000    ; Area 5 has 0 states per data transfer 
BSC_WCR2_A5B_1:             .equ    h'00100000    ; Area 5 has 1 states per data transfer  
BSC_WCR2_A5B_2:             .equ    h'00200000    ; Area 5 has 2 states per data transfer 
BSC_WCR2_A5B_3:             .equ    h'00300000    ; Area 5 has 3 states per data transfer 
BSC_WCR2_A5B_4:             .equ    h'00400000    ; Area 5 has 4 states per data transfer 
BSC_WCR2_A5B_5:             .equ    h'00500000    ; Area 5 has 5 states per data transfer 
BSC_WCR2_A5B_6:             .equ    h'00600000    ; Area 5 has 6 states per data transfer 
BSC_WCR2_A5B_7:             .equ    h'00700000    ; Area 5 has 7 states per data transfer 

BSC_WCR2_A4W:               .equ    h'000e0000    ; Area 4 wait state mask
BSC_WCR2_A4W_0:             .equ    h'00000000    ; Area 4 has 0 wait states
BSC_WCR2_A4W_1:             .equ    h'00020000    ; Area 4 has 1 wait states
BSC_WCR2_A4W_2:             .equ    h'00040000    ; Area 4 has 2 wait states
BSC_WCR2_A4W_3:             .equ    h'00060000    ; Area 4 has 3 wait states
BSC_WCR2_A4W_6:             .equ    h'00080000    ; Area 4 has 6 wait states 
BSC_WCR2_A4W_9:             .equ    h'000a0000    ; Area 4 has 9 wait states 
BSC_WCR2_A4W_12:            .equ    h'000c0000    ; Area 4 has 12 wait states 
BSC_WCR2_A4W_15:            .equ    h'000e0000    ; Area 4 has 15 wait states 

;  When Normal Memory is Used

BSC_WCR2_A3W_NORMAL:        .equ    h'0000e000    ; Area 3 wait state mask
BSC_WCR2_A3W_NORMAL_0:      .equ    h'00000000    ; Area 3 has 0 wait states
BSC_WCR2_A3W_NORMAL_1:      .equ    h'00002000    ; Area 3 has 1 wait states
BSC_WCR2_A3W_NORMAL_2:      .equ    h'00004000    ; Area 3 has 2 wait states
BSC_WCR2_A3W_NORMAL_3:      .equ    h'00006000    ; Area 3 has 3 wait states
BSC_WCR2_A3W_NORMAL_6:      .equ    h'00008000    ; Area 3 has 6 wait states 
BSC_WCR2_A3W_NORMAL_9:      .equ    h'0000a000    ; Area 3 has 9 wait states 
BSC_WCR2_A3W_NORMAL_12:     .equ    h'0000c000    ; Area 3 has 12 wait states 
BSC_WCR2_A3W_NORMAL_15:     .equ    h'0000e000    ; Area 3 has 15 wait states    

;  When DRAM, Synchronours DRAM is Used

BSC_WCR2_A3W_DRAM:          .equ    h'0000e000    ; Area 3 assertion width mask 
BSC_WCR2_A3W_DRAM_1:        .equ    h'00000000    ; Area 3 has 1 assertion width 
BSC_WCR2_A3W_DRAM_2:        .equ    h'00002000    ; Area 3 has 2 assertion width  
BSC_WCR2_A3W_DRAM_3:        .equ    h'00004000    ; Area 3 has 3 assertion width 
BSC_WCR2_A3W_DRAM_4:        .equ    h'00006000    ; Area 3 has 4 assertion width 
BSC_WCR2_A3W_DRAM_7:        .equ    h'00008000    ; Area 3 has 7 assertion width 
BSC_WCR2_A3W_DRAM_10:       .equ    h'0000a000    ; Area 3 has 10 assertion width 
BSC_WCR2_A3W_DRAM_13:       .equ    h'0000c000    ; Area 3 has 13 assertion width 
BSC_WCR2_A3W_DRAM_16:       .equ    h'0000e000    ; Area 3 has 16 assertion width 

;  When Normal Memory is Used

BSC_WCR2_A2W_NORMAL:        .equ    h'00000e00    ; Area 2 wait state mask 
BSC_WCR2_A2W_NORMAL_0:      .equ    h'00000000    ; Area 2 has 0 wait states 
BSC_WCR2_A2W_NORMAL_1:      .equ    h'00000200    ; Area 2 has 1 wait states  
BSC_WCR2_A2W_NORMAL_2:      .equ    h'00000400    ; Area 2 has 2 wait states 
BSC_WCR2_A2W_NORMAL_3:      .equ    h'00000600    ; Area 2 has 3 wait states 
BSC_WCR2_A2W_NORMAL_6:      .equ    h'00000800    ; Area 2 has 6 wait states 
BSC_WCR2_A2W_NORMAL_9:      .equ    h'00000a00    ; Area 2 has 9 wait states 
BSC_WCR2_A2W_NORMAL_12:     .equ    h'00000c00    ; Area 2 has 12 wait states 
BSC_WCR2_A2W_NORMAL_15:     .equ    h'00000e00    ; Area 2 has 15 wait states 

;  When DRAM, Synchronours DRAM is Used

BSC_WCR2_A2W_DRAM:          .equ    h'00000e00    ; Area 2 assertion width mask 
BSC_WCR2_A2W_DRAM_1:        .equ    h'00000000    ; Area 2 has 1 assertion width 
BSC_WCR2_A2W_DRAM_2:        .equ    h'00000200    ; Area 2 has 2 assertion width  
BSC_WCR2_A2W_DRAM_3:        .equ    h'00000400    ; Area 2 has 3 assertion width 
BSC_WCR2_A2W_DRAM_4:        .equ    h'00000600    ; Area 2 has 4 assertion width 
BSC_WCR2_A2W_DRAM_7:        .equ    h'00000800    ; Area 2 has 7 assertion width 
BSC_WCR2_A2W_DRAM_10:       .equ    h'00000a00    ; Area 2 has 10 assertion width 
BSC_WCR2_A2W_DRAM_13:       .equ    h'00000c00    ; Area 2 has 13 assertion width 
BSC_WCR2_A2W_DRAM_16:       .equ    h'00000e00    ; Area 2 has 16 assertion width 

BSC_WCR2_A1W:               .equ    h'000001c0    ; Area 1 wait state mask
BSC_WCR2_A1W_0:             .equ    h'00000000    ; Area 1 has 0 wait states 
BSC_WCR2_A1W_1:             .equ    h'00000040    ; Area 1 has 1 wait states  
BSC_WCR2_A1W_2:             .equ    h'00000080    ; Area 1 has 2 wait states 
BSC_WCR2_A1W_3:             .equ    h'000000c0    ; Area 1 has 3 wait states 
BSC_WCR2_A1W_6:             .equ    h'00000100    ; Area 1 has 6 wait states 
BSC_WCR2_A1W_9:             .equ    h'00000140    ; Area 1 has 9 wait states 
BSC_WCR2_A1W_12:            .equ    h'00000180    ; Area 1 has 12 wait states 
BSC_WCR2_A1W_15:            .equ    h'000001c0    ; Area 1 has 15 wait states 

BSC_WCR2_A0W:               .equ    h'00000038    ; Area 0 wait state mask
BSC_WCR2_A0W_0:             .equ    h'00000000    ; Area 0 has 0 wait states
BSC_WCR2_A0W_1:             .equ    h'00000008    ; Area 0 has 1 wait states
BSC_WCR2_A0W_2:             .equ    h'00000010    ; Area 0 has 2 wait states
BSC_WCR2_A0W_3:             .equ    h'00000018    ; Area 0 has 3 wait states
BSC_WCR2_A0W_6:             .equ    h'00000020    ; Area 0 has 3 wait states
BSC_WCR2_A0W_9:             .equ    h'00000028    ; Area 0 has 3 wait states
BSC_WCR2_A0W_12:            .equ    h'00000030    ; Area 0 has 3 wait states
BSC_WCR2_A0W_15:            .equ    h'00000038    ; Area 0 has 3 wait states

BSC_WCR2_A0B:               .equ    h'00000007    ; Area 0 wait state mask 
BSC_WCR2_A0B_0:             .equ    h'00000000    ; Area 0 has 0 wait states 
BSC_WCR2_A0B_1:             .equ    h'00000001    ; Area 0 has 1 wait states  
BSC_WCR2_A0B_2:             .equ    h'00000002    ; Area 0 has 2 wait states 
BSC_WCR2_A0B_3:             .equ    h'00000003    ; Area 0 has 3 wait states 
BSC_WCR2_A0B_4:             .equ    h'00000004    ; Area 0 has 4 wait states 
BSC_WCR2_A0B_5:             .equ    h'00000005    ; Area 0 has 5 wait states 
BSC_WCR2_A0B_6:             .equ    h'00000006    ; Area 0 has 6 wait states 
BSC_WCR2_A0B_7:             .equ    h'00000007    ; Area 0 has 7 wait states 

; WCR3  Wait state control register 3 fields. 

BSC_WCR3_A6S:               .equ    h'04000000    ; Area 6 write strobe setup time mask 
BSC_WCR3_A6S_0:             .equ    h'00000000    ; Area 6 has 0 wait inserted in setup 
BSC_WCR3_A6S_1:             .equ    h'04000000    ; Area 6 has 1 wait inserted in setup    

BSC_WCR3_A6H:               .equ    h'03000000    ; Area 6 data hold time mask 
BSC_WCR3_A6H_0:             .equ    h'00000000    ; Area 6 has 0 wait inserted in hold 
BSC_WCR3_A6H_1:             .equ    h'01000000    ; Area 6 has 1 wait inserted in hold 
BSC_WCR3_A6H_2:             .equ    h'02000000    ; Area 6 has 2 wait inserted in hold 
BSC_WCR3_A6H_3:             .equ    h'03000000    ; Area 6 has 3 wait inserted in hold    

BSC_WCR3_A5S:               .equ    h'00400000    ; Area 5 write strobe setup time mask 
BSC_WCR3_A5S_0:             .equ    h'00000000    ; Area 5 has 0 wait inserted in setup 
BSC_WCR3_A5S_1:             .equ    h'00400000    ; Area 5 has 1 wait inserted in setup 

BSC_WCR3_A5H:               .equ    h'00300000    ; Area 5 data hold time mask 
BSC_WCR3_A5H_0:             .equ    h'00000000    ; Area 5 has 0 wait inserted in hold 
BSC_WCR3_A5H_1:             .equ    h'00100000    ; Area 5 has 1 wait inserted in hold 
BSC_WCR3_A5H_2:             .equ    h'00200000    ; Area 5 has 2 wait inserted in hold 
BSC_WCR3_A5H_3:             .equ    h'00300000    ; Area 5 has 3 wait inserted in hold 

BSC_WCR3_A4S:               .equ    h'00040000    ; Area 4 write strobe setup time mask 
BSC_WCR3_A4S_0:             .equ    h'00000000    ; Area 4 has 0 wait inserted in setup 
BSC_WCR3_A4S_1:             .equ    h'00040000    ; Area 4 has 1 wait inserted in setup

BSC_WCR3_A4H:               .equ    h'00030000    ; Area 4 data hold time mask 
BSC_WCR3_A4H_0:             .equ    h'00000000    ; Area 4 has 0 wait inserted in hold 
BSC_WCR3_A4H_1:             .equ    h'00010000    ; Area 4 has 1 wait inserted in hold 
BSC_WCR3_A4H_2:             .equ    h'00020000    ; Area 4 has 2 wait inserted in hold 
BSC_WCR3_A4H_3:             .equ    h'00030000    ; Area 4 has 3 wait inserted in hold 

BSC_WCR3_A3S:               .equ    h'00004000    ; Area 3 write strobe setup time mask 
BSC_WCR3_A3S_0:             .equ    h'00000000    ; Area 3 has 0 wait inserted in setup 
BSC_WCR3_A3S_1:             .equ    h'00004000    ; Area 3 has 1 wait inserted in setup

BSC_WCR3_A3H:               .equ    h'00003000    ; Area 3 data hold time mask 
BSC_WCR3_A3H_0:             .equ    h'00000000    ; Area 3 has 0 wait inserted in hold 
BSC_WCR3_A3H_1:             .equ    h'00001000    ; Area 3 has 1 wait inserted in hold 
BSC_WCR3_A3H_2:             .equ    h'00002000    ; Area 3 has 2 wait inserted in hold 
BSC_WCR3_A3H_3:             .equ    h'00003000    ; Area 3 has 3 wait inserted in hold

BSC_WCR3_A2S:               .equ    h'00000400    ; Area 2 write strobe setup time mask 
BSC_WCR3_A2S_0:             .equ    h'00000000    ; Area 2 has 0 wait inserted in setup 
BSC_WCR3_A2S_1:             .equ    h'00000400    ; Area 2 has 1 wait inserted in setup 

BSC_WCR3_A2H:               .equ    h'00000300    ; Area 2 data hold time mask 
BSC_WCR3_A2H_0:             .equ    h'00000000    ; Area 2 has 0 wait inserted in hold 
BSC_WCR3_A2H_1:             .equ    h'00000100    ; Area 2 has 1 wait inserted in hold 
BSC_WCR3_A2H_2:             .equ    h'00000200    ; Area 2 has 2 wait inserted in hold 
BSC_WCR3_A2H_3:             .equ    h'00000300    ; Area 2 has 3 wait inserted in hold 

BSC_WCR3_A1S:               .equ    h'00000040    ; Area 1 write strobe setup time mask 
BSC_WCR3_A1S_0:             .equ    h'00000000    ; Area 1 has 0 wait inserted in setup 
BSC_WCR3_A1S_1:             .equ    h'00000040    ; Area 1 has 1 wait inserted in setup 

BSC_WCR3_A1H:               .equ    h'00000030    ; Area 1 data hold time mask 
BSC_WCR3_A1H_0:             .equ    h'00000000    ; Area 1 has 0 wait inserted in hold 
BSC_WCR3_A1H_1:             .equ    h'00000010    ; Area 1 has 1 wait inserted in hold 
BSC_WCR3_A1H_2:             .equ    h'00000020    ; Area 1 has 2 wait inserted in hold 
BSC_WCR3_A1H_3:             .equ    h'00000030    ; Area 1 has 3 wait inserted in hold 

BSC_WCR3_A0S:               .equ    h'00000004    ; Area 0 write strobe setup time mask 
BSC_WCR3_A0S_0:             .equ    h'00000000    ; Area 0 has 0 wait inserted in setup 
BSC_WCR3_A0S_1:             .equ    h'00000004    ; Area 0 has 1 wait inserted in setup 

BSC_WCR3_A0H:               .equ    h'00000003    ; Area 0 data hold time mask 
BSC_WCR3_A0H_0:             .equ    h'00000000    ; Area 0 has 0 wait inserted in hold 
BSC_WCR3_A0H_1:             .equ    h'00000001    ; Area 0 has 1 wait inserted in hold 
BSC_WCR3_A0H_2:             .equ    h'00000002    ; Area 0 has 2 wait inserted in hold 
BSC_WCR3_A0H_3:             .equ    h'00000003    ; Area 0 has 3 wait inserted in hold 

; MCR Individual memory control register (RAS/CAS timing and burst control for
; DRAM, SRAM and PSRAM, address multiplexing, and refresh control.

BSC_MCR_RASD:               .equ    h'80000000    ; RAS down mode 

BSC_MCR_MRSET:              .equ    h'40000000    ; Mode register set 

BSC_MCR_TRC:                .equ    h'38000000    ; RAS precharge time mask 
BSC_MCR_TRC_0:              .equ    h'00000000    ; 0 cycle 
BSC_MCR_TRC_3:              .equ    h'08000000    ; 3 cycle 
BSC_MCR_TRC_6:              .equ    h'10000000    ; 6 cycles  
BSC_MCR_TRC_9:              .equ    h'18000000    ; 9 cycles 
BSC_MCR_TRC_12:             .equ    h'20000000    ; 12 cycles 
BSC_MCR_TRC_15:             .equ    h'28000000    ; 15 cycles 
BSC_MCR_TRC_18:             .equ    h'30000000    ; 18 cycles 
BSC_MCR_TRC_21:             .equ    h'38000000    ; 21 cycles 

BSC_MCR_TCAS:               .equ    h'00800000    ; CAS Assertion Period 0:1, 1:2        

BSC_MCR_TPC:                .equ    h'00380000    ; RAS precharge time mask
BSC_MCR_TPC_0:              .equ    h'00000000    ; 0 cycle
BSC_MCR_TPC_1:              .equ    h'00080000    ; 1 cycles
BSC_MCR_TPC_2:              .equ    h'00100000    ; 2 cycles
BSC_MCR_TPC_3:              .equ    h'00180000    ; 3 cycles
BSC_MCR_TPC_4:              .equ    h'00200000    ; 4 cycles
BSC_MCR_TPC_5:              .equ    h'00280000    ; 5 cycles
BSC_MCR_TPC_6:              .equ    h'00300000    ; 6 cycles
BSC_MCR_TPC_7:              .equ    h'00380000    ; 7 cycles

BSC_MCR_RCD:                .equ    h'00030000    ; RAS-CAS delay mask
BSC_MCR_RCD_2:              .equ    h'00000000    ; 2 cycles
BSC_MCR_RCD_3:              .equ    h'00010000    ; 3 cycles
BSC_MCR_RCD_4:              .equ    h'00020000    ; 4 cycles
BSC_MCR_RCD_5:              .equ    h'00030000    ; 5 cycles

BSC_MCR_TRWL:               .equ    h'0000e000    ; Write-precharge delay
BSC_MCR_TRWL_1:             .equ    h'00000000    ; 1 cycle
BSC_MCR_TRWL_2:             .equ    h'00002000    ; 2 cycles
BSC_MCR_TRWL_3:             .equ    h'00004000    ; 3 cycles
BSC_MCR_TRWL_4:             .equ    h'00006000    ; 4 cycles
BSC_MCR_TRWL_5:             .equ    h'00008000    ; 5 cycles

BSC_MCR_TRAS:               .equ    h'00001c00    ; RAS assertion period mask
BSC_MCR_TRAS_2:             .equ    h'00000000    ; 2 cycles
BSC_MCR_TRAS_3:             .equ    h'00000400    ; 3 cycles
BSC_MCR_TRAS_4:             .equ    h'00000800    ; 4 cycles
BSC_MCR_TRAS_5:             .equ    h'00000c00    ; 5 cycles
BSC_MCR_TRAS_6:             .equ    h'00001000    ; 6 cycles
BSC_MCR_TRAS_7:             .equ    h'00001400    ; 7 cycles
BSC_MCR_TRAS_8:             .equ    h'00001800    ; 8 cycles
BSC_MCR_TRAS_9:             .equ    h'00001c00    ; 9 cycles

BSC_MCR_BE:                 .equ    h'00000200    ; Burst enable
BSC_MCR_SZ:                 .equ    h'00000180    ; Memory data size mask
BSC_MCR_SZ_64:              .equ    h'00000000    ; Memory data size 64
BSC_MCR_SZ_16:              .equ    h'00000100    ; Memory data size 16
BSC_MCR_SZ_32:              .equ    h'00000180    ; Memory data size 32

BSC_MCR_AMXEXT:             .equ    h'00000040    ; Address multiplexing    

BSC_MCR_AMX:                .equ    h'00000038    ; Address multiplex mask
BSC_MCR_AMX_8:              .equ    h'00000000    ; 8-bit column address product
BSC_MCR_AMX_9:              .equ    h'00000008    ; 9-bit column address product
BSC_MCR_AMX_10:             .equ    h'00000010    ; 10-bit column address product
BSC_MCR_AMX_11:             .equ    h'00000018    ; 11-bit column address product
BSC_MCR_AMX_12:             .equ    h'00000020    ; 12-bit column address product    

BSC_MCR_RFSH:               .equ    h'00000004    ; Refresh enable
BSC_MCR_RMODE:              .equ    h'00000002    ; Refresh mode, 0 => ordinary, 1 => self
BSC_MCR_EDOMODE:            .equ    h'00000001    ; EDO mode 1:EDO, 0:SDRAM or SGRAM

; PCMCIA Control Register(PCR)  

BSC_PCR_A5PCW:              .equ    h'c000    ; PCMCIA area 5 wait mask 
BSC_PCR_A5PCW_0:            .equ    h'0000    ; Area 5 has 0 wait inserted 
BSC_PCR_A5PCW_15:           .equ    h'4000    ; Area 5 has 15 wait inserted 
BSC_PCR_A5PCW_30:           .equ    h'8000    ; Area 5 has 30 wait inserted 
BSC_PCR_A5PCW_50:           .equ    h'c000    ; Area 5 has 50 wait inserted 

BSC_PCR_A6PCW:              .equ    h'3000    ; PCMCIA area 6 wait mask 
BSC_PCR_A6PCW_0:            .equ    h'0000    ; Area 6 has 0 wait inserted 
BSC_PCR_A6PCW_15:           .equ    h'1000    ; Area 6 has 15 wait inserted 
BSC_PCR_A6PCW_30:           .equ    h'2000    ; Area 6 has 30 wait inserted 
BSC_PCR_A6PCW_50:           .equ    h'3000    ; Area 6 has 50 wait inserted 

BSC_PCR_A5TED:              .equ    h'0e00    ; PCMCIA area 5 address assertion delay mask 
BSC_PCR_A5TED_0:            .equ    h'0000    ; Area 5 has 0 wait inserted 
BSC_PCR_A5TED_1:            .equ    h'0200    ; Area 5 has 1 wait inserted 
BSC_PCR_A5TED_2:            .equ    h'0400    ; Area 5 has 2 wait inserted 
BSC_PCR_A5TED_3:            .equ    h'0600    ; Area 5 has 3 wait inserted 
BSC_PCR_A5TED_6:            .equ    h'0800    ; Area 5 has 6 wait inserted 
BSC_PCR_A5TED_9:            .equ    h'0a00    ; Area 5 has 9 wait inserted 
BSC_PCR_A5TED_12:           .equ    h'0c00    ; Area 5 has 12 wait inserted 
BSC_PCR_A5TED_15:           .equ    h'0e00    ; Area 5 has 15 wait inserted 

BSC_PCR_A6TED:              .equ    h'01c0    ; PCMCIA area 6 address assertion delay mask 
BSC_PCR_A6TED_0:            .equ    h'0000    ; Area 6 has 0 wait inserted 
BSC_PCR_A6TED_1:            .equ    h'0040    ; Area 6 has 1 wait inserted 
BSC_PCR_A6TED_2:            .equ    h'0080    ; Area 6 has 2 wait inserted 
BSC_PCR_A6TED_3:            .equ    h'00c0    ; Area 6 has 3 wait inserted 
BSC_PCR_A6TED_6:            .equ    h'0100    ; Area 6 has 6 wait inserted 
BSC_PCR_A6TED_9:            .equ    h'0140    ; Area 6 has 9 wait inserted 
BSC_PCR_A6TED_12:           .equ    h'0180    ; Area 6 has 12 wait inserted 
BSC_PCR_A6TED_15:           .equ    h'01c0    ; Area 6 has 15 wait inserted 

BSC_PCR_A5TEH:              .equ    h'0038    ; PCMCIA area 5 negation address delay mask 
BSC_PCR_A5TEH_0:            .equ    h'0000    ; Area 5 has 0 wait inserted 
BSC_PCR_A5TEH_1:            .equ    h'0008    ; Area 5 has 1 wait inserted 
BSC_PCR_A5TEH_2:            .equ    h'0010    ; Area 5 has 2 wait inserted 
BSC_PCR_A5TEH_3:            .equ    h'0018    ; Area 5 has 3 wait inserted 
BSC_PCR_A5TEH_6:            .equ    h'0020    ; Area 5 has 6 wait inserted 
BSC_PCR_A5TEH_9:            .equ    h'0028    ; Area 5 has 9 wait inserted 
BSC_PCR_A5TEH_12:           .equ    h'0030    ; Area 5 has 12 wait inserted 
BSC_PCR_A5TEH_15:           .equ    h'0038    ; Area 5 has 15 wait inserted 

BSC_PCR_A6TEH:              .equ    h'0007    ; PCMCIA area 6 negation address delay mask 
BSC_PCR_A6TEH_0:            .equ    h'0000    ; Area 6 has 0 wait inserted 
BSC_PCR_A6TEH_1:            .equ    h'0001    ; Area 6 has 1 wait inserted 
BSC_PCR_A6TEH_2:            .equ    h'0002    ; Area 6 has 2 wait inserted 
BSC_PCR_A6TEH_3:            .equ    h'0003    ; Area 6 has 3 wait inserted 
BSC_PCR_A6TEH_6:            .equ    h'0004    ; Area 6 has 6 wait inserted 
BSC_PCR_A6TEH_9:            .equ    h'0005    ; Area 6 has 9 wait inserted 
BSC_PCR_A6TEH_12:           .equ    h'0006    ; Area 6 has 12 wait inserted 
BSC_PCR_A6TEH_15:           .equ    h'0007    ; Area 6 has 15 wait inserted 

; Synchronous DRAM Mode Register(SDMR)  

; Values that will be written to many of the following registers must be ORed
; with their corresponding *_*_COOKIE value before writing the register.
; Otherwise, the hardware will reject (ignore) the write.

; RTCSR: Refresh timer control/status register (refresh cycle, interrupt enable,
; and the interrupt's cycle).

BSC_SDMR2_COOKIE:           .equ    h'FF900000    ; OR with new value before writing
BSC_SDMR3_COOKIE:           .equ    h'FF940000    ; OR with new value before writing

BSC_SDMR3_190:              .equ    h'00000190    ; 32bit, LMODE=3, WT=0, BL=4, 
BSC_SDMR3_110:              .equ    h'00000110    ; 32bit, LMODE=3, WT=0, BL=4, 

BSC_RTCSR_COOKIE:           .equ    h'a500    ; OR with new value before writing
BSC_RTCSR_CMF:              .equ    h'0080    ; Predicate, RTCNT == RTCOR
BSC_RTCSR_CMIE:             .equ    h'0040    ; Enable an interrupt via CMF

BSC_RTCSR_CKS:              .equ    h'0038    ; Clock select mask
BSC_RTCSR_CKS_DISABLE:      .equ    h'0000    ; Disable clock input
BSC_RTCSR_CKS_4:            .equ    h'0008    ; CKIO/4
BSC_RTCSR_CKS_16:           .equ    h'0010    ; CKIO/16
BSC_RTCSR_CKS_64:           .equ    h'0018    ; CKIO/64
BSC_RTCSR_CKS_256:          .equ    h'0020    ; CKIO/256
BSC_RTCSR_CKS_1024:         .equ    h'0028    ; CKIO/1024
BSC_RTCSR_CKS_2048:         .equ    h'0030    ; CKIO/2048
BSC_RTCSR_CKS_4096:         .equ    h'0038    ; CKIO/4096

BSC_RTCSR_OVF:              .equ    h'0004    ; RFCR has exceeded count limit in LMTS
BSC_RTCSR_OVIE:             .equ    h'0002    ; Enable an interrupt via OVF
BSC_RTCSR_LMTS:             .equ    h'0001    ; Count limit: 0 => 1024, 1 => 512    

; RTCNT: Refresh timer counter.

BSC_RTCNT_COOKIE:           .equ    h'a500    ; OR with new value before writing
BSC_RTCNT_COUNT_FF:         .equ    h'00ff    ; Refresh timer counter mask
BSC_RTCNT_COUNT_00:         .equ    h'0000    ; Refresh timer counter mask

; RTCOR: Refresh time constant register.

BSC_RTCOR_COOKIE:           .equ    h'a500    ; OR with new value before writing
BSC_RTCOR_COUNT:            .equ    h'00ff    ; Refresh time constant mask

; RFCR: Refresh count register.

BSC_RFCR_COOKIE:            .equ    h'a400    ; OR with new value before writing
BSC_RFCR_COUNT:             .equ    h'03ff    ; Refresh count mask

