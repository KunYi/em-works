;PBYTE Render2(           LONG * p_CurrSamp, LONG * p_PrevSamp,
;                 LONG * p_CurrPos,  PBYTE *lpCurrData,
;             LONG fxpGain_0, LONG fxpGain_1, PBYTE pCurrDataEnd,
;             PBYTE pBufferLast, DWORD BaseRate, 
;             PBYTE pBuffer, PBYTE pBufferEnd, 
;                         DWORD ClientRate, DWORD BaseRateInv);

       
        IF __WINCE = 0
         ARM
         PRESERVE8
       ENDIF 
     IF RENDER_ASM_OPT = 1
STACK_SAVE_REG  EQU  (4*14)+4*2
DELTA_OVERFLOW  EQU  (1<<15)
STACK_FXPGAIN   EQU  (4*14)
STACK_pBufferLast EQU (4*14+4*3)
STACK_ClientRate  EQU (4*14+4*7)
STACK_BaseRateInv EQU (4*14+4*8)
STACK_FXPGAIN_0   EQU (4*14+4*0)
STACK_FXPGAIN_1   EQU (4*14+4*1)
STACK_pBufferEnd  EQU (4*14+4*6)

     AREA |.text|, CODE, READONLY, ALIGN=2
       EXPORT Render2_opt
Render2_opt FUNCTION
       STMFD sp!, {r0 - r12, r14}
       ADD   r5, sp, #STACK_SAVE_REG
       LDMIA r0,  {r4, r14}                           ;r4=CurrSamp0   r14=CurrSamp1
       LDMIA r1,  {r8, r9 }                           ;r8=PrevSamp0   r9=PrevSamp1
       LDR   r10,  [r2]                               ;r10=CurrPos = * p_CurrPos;
       LDR   r11,  [r3]                               ;r11=pCurrData = * lpCurrData;
       LDMIA r5,  { r1, r2, r3, r7, r12}               
       
       CMP   r12, r2                                  ;?(pBufferEnd <= pBufferLast)
       MOVGT r12, r2

      LDR     r0, [sp, #STACK_BaseRateInv]            ;r0=BaseRateInv
      LDR     r2, [sp, #STACK_ClientRate]
 
       CMP    r7, r12
       BGE    NEXT_LOOP
LOOP_BUF
      
       CMP   r10,  #0                                 ;? (CurrPos < 0)
       BGE   FILTER
LOOP_CURRT
       CMP   r11, r1                                  ;if (pCurrData>=pCurrDataEnd) r11=pCurrData, r1=pCurrDataEnd
       BGE   EXIT
  
       ADDS   r10, r10, r3                             ; r10 = CurrPos += BaseRate;
       MOV    r8, r4                                   ;r8=PrevSamp0 = CurrSamp0
       LDRSH  r4, [r11], #2                            ;CurrSamp0 = *((short *)pCurrData);
       MOV    r9, r14                                  ;r9=PrevSamp1 = CurrSamp1
       LDRSH  r14,[r11], #2                            ;CurrSamp1 = *((short *)pCurrData+1);
       BMI    LOOP_CURRT 

FILTER
      
       MUL     r0,  r10, r0                            ;r0=Ratio=(int)(CurrPos * BaseRateInv);
       SUB     r5,  r8,  r4                            ;r5=OutSamp0=(CurrSamp0 - PrevSamp0)
       SUB     r6,  r9,  r14                           ;r6=OutSamp1=(CurrSamp1 - PrevSamp1) 
       SUB     r10, r10, r2                            ;r10=CurrPos -= ClientRate;
       MOV     r5,  r5,  ASL #1
       MOV     r0,  r0,  ASR #17
       MOV     r6,  r6,  ASL #1
       
       SMLAWB  r5,  r5,  r0, r4
       SMLAWB  r6,  r6,  r0, r14      

       LDR     r0, [sp, #STACK_FXPGAIN_0]              ; r5=fxpGain
       LDR     r2, [sp, #STACK_FXPGAIN_1]
      
 
       MOV     r5, r5, ASL #2
       MOV     r6, r6, ASL #2

       SMULWB  r5, r5, r0
       SMULWB  r6, r6, r2
  

       LDRSH    r0, [r7]       
       LDRSH    r2, [r7, #2]
       
       QADD16    r5, r0, r5
       QADD16    r6, r2, r6
  
       STRH    r5, [r7], #2
       STRH    r6, [r7], #2
       CMP     r7, r12                                  ;r7=pBuffer
       LDR     r0, [sp, #STACK_BaseRateInv]            ;r0=BaseRateInv
       LDR     r2, [sp, #STACK_ClientRate]
       BLT     LOOP_BUF

NEXT_LOOP

       LDR    r12, [sp, #STACK_pBufferEnd]
       CMP    r7, r12
       BGE    EXIT      
 
LOOP_BUF_NEXT
       CMP   r10,  #0                                 ;? (CurrPos < 0)
       BGE   FILTER_NEXT
LOOP_CURRT_NEXT       
       CMP   r11, r1                                  ;if (pCurrData>=pCurrDataEnd) r11=pCurrData, r1=pCurrDataEnd
       BGE   EXIT
  
       ADDS   r10, r10, r3                             ; r10 = CurrPos += BaseRate;
       MOV    r8, r4                                   ;r8=PrevSamp0 = CurrSamp0
       LDRSH  r4, [r11], #2                            ;CurrSamp0 = *((short *)pCurrData);
       MOV    r9, r14                                  ;r9=PrevSamp1 = CurrSamp1
       LDRSH  r14,[r11], #2                            ;CurrSamp1 = *((short *)pCurrData+1);
       BMI    LOOP_CURRT_NEXT  

FILTER_NEXT
      
       MUL     r0,  r10, r0                            ;r0=Ratio=(int)(CurrPos * BaseRateInv);
       SUB     r5,  r8,  r4                            ;r5=OutSamp0=(CurrSamp0 - PrevSamp0)
       SUB     r6,  r9,  r14                           ;r6=OutSamp1=(CurrSamp1 - PrevSamp1) 
       SUB     r10, r10, r2                            ;r10=CurrPos -= ClientRate;
       MOV     r5,  r5,  ASL #1
       MOV     r0,  r0,  ASR #17
       MOV     r6,  r6,  ASL #1
       
       SMLAWB  r5,  r5,  r0, r4
       SMLAWB  r6,  r6,  r0, r14      

       LDR     r0, [sp, #STACK_FXPGAIN_0]              ; r5=fxpGain
       LDR     r2, [sp, #STACK_FXPGAIN_1]
      
 
       MOV     r5, r5, ASL #2
       MOV     r6, r6, ASL #2

       SMULWB  r5, r5, r0
       SMULWB  r6, r6, r2
  
       STRH    r5, [r7], #2
       STRH    r6, [r7], #2
       CMP     r7, r12                                  ;r7=pBuffer
       LDRLT     r0, [sp, #STACK_BaseRateInv]            ;r0=BaseRateInv
       LDRLT     r2, [sp, #STACK_ClientRate]
       BLT     LOOP_BUF_NEXT 
EXIT
       LDMFD   sp!, {r0-r3}
       STMIA   r0, {r4, r14}
       STMIA   r1, {r8, r9}
       STR     r10,  [r2]                               ;r10=CurrT = * p_CurrT;
       STR     r11,  [r3]                               ;r11=pCurrData = * lpCurrData;
       MOV     r0,   r7                                 ;return the pBuffer
       LDMFD   sp!, {r4-r12, pc}
       ENDFUNC
       ENDIF
       END   
