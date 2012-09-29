//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  asrc_base.cpp
//
//   This module 
//------------------------------------------------------------------------------

#include "asrc_base.h"
#include "common_ddk.h"

#define ASRC_NUM_DMA_BUFFERS    2

PASRC_CONTROLLER_CONTEXT g_pAsrcContext = NULL;


BOOL BSPConfigClkSrc(PASRC_CONFIG_PARAM pParam);


#ifdef ASRC_DEBUG
static void DumpReg(void)
{
    PCSP_ASRC_REG  pReg = g_pAsrcContext->pAsrcReg;

    RETAILMSG(TRUE, (_T("ASRC ASRCTR:%x\r\n "),INREG32(&pReg->ASRCTR)));
    RETAILMSG(TRUE, (_T("ASRC ASRIER:%x\r\n "),INREG32(&pReg->ASRIER)));
    RETAILMSG(TRUE, (_T("ASRC ASRCNCR:%x\r\n "),INREG32(&pReg->ASRCNCR)));
    RETAILMSG(TRUE, (_T("ASRC RESERV:%x\r\n "),INREG32(&pReg->RESERV)));
    RETAILMSG(TRUE, (_T("ASRC ASRCFG:%x\r\n "),INREG32(&pReg->ASRCFG)));
    RETAILMSG(TRUE, (_T("ASRC ASRCSR:%x\r\n "),INREG32(&pReg->ASRCSR)));
    RETAILMSG(TRUE, (_T("ASRC ASRCDR1:%x\r\n "),INREG32(&pReg->ASRCDR1)));
    RETAILMSG(TRUE, (_T("ASRC ASRCDR2:%x\r\n "),INREG32(&pReg->ASRCDR2)));
    RETAILMSG(TRUE, (_T("ASRC ASRSTR:%x\r\n "),INREG32(&pReg->ASRSTR)));


    RETAILMSG(TRUE, (_T("ASRC ASRPM1:%x\r\n "),INREG32(&pReg->ASRPM1)));
    RETAILMSG(TRUE, (_T("ASRC ASRPM2:%x\r\n "),INREG32(&pReg->ASRPM2)));
    RETAILMSG(TRUE, (_T("ASRC ASRPM3:%x\r\n "),INREG32(&pReg->ASRPM3)));
    RETAILMSG(TRUE, (_T("ASRC ASRPM4:%x\r\n "),INREG32(&pReg->ASRPM5)));
    RETAILMSG(TRUE, (_T("ASRC ASRPM5:%x\r\n "),INREG32(&pReg->ASRPM5)));

    RETAILMSG(TRUE, (_T("ASRC ASRCCR:%x\r\n "),INREG32(&pReg->ASRCCR)));
    RETAILMSG(TRUE, (_T("ASRC ASRDIA:%x\r\n "),INREG32(&pReg->ASRDIA)));
    RETAILMSG(TRUE, (_T("ASRC ASRDOA:%x\r\n "),INREG32(&pReg->ASRDOA)));
    RETAILMSG(TRUE, (_T("ASRC ASRIDRHA:%x\r\n "),INREG32(&pReg->ASRIDRHA)));
    RETAILMSG(TRUE, (_T("ASRC ASRIDRLA:%x\r\n "),INREG32(&pReg->ASRIDRLA)));
      

    RETAILMSG(TRUE, (_T("ASRC ASR76K:%x\r\n "),INREG32(&pReg->ASR76K)));
    RETAILMSG(TRUE, (_T("ASRC ASR56K:%x\r\n "),INREG32(&pReg->ASR56K)));

}
#endif

static BOOL AsrcSetReg(UINT32 * pReg, UINT32 val, UINT32 mask)
{
    LONG oldVal,newVal,count=0;
    for (count = 0; count <1000; count ++){
        oldVal = INREG32(pReg);
        newVal = (oldVal& (~mask)) | val;
        if (InterlockedTestExchange((volatile LONG *)pReg, (LONG)oldVal,(LONG) newVal)
                    == oldVal){
                break;
        }   
    }

    if (count >= 1000){
        return FALSE;
    }else{
         return TRUE;    
    }
}


static void CalClockRate(UINT32 bitClockRate,UINT32 sampleClockRate,
                                  UINT32 *prescaler,  UINT32 *divider, 
                                  BOOL bIdeaMode)
{
    //RETAILMSG(1,(_T("CalClockRate:bitClk(%d)SampleRate(%d)ideamode(%d)\r\n"),
    //bitClockRate,sampleClockRate,bIdeaMode));

    UINT32 ratio,i;

    //bIdeaMode = FALSE;

    if((bitClockRate == 0) || (sampleClockRate == 0))
        return;

    ratio = bitClockRate/sampleClockRate;

    if (bIdeaMode){
        //we use asrc clk as output clk, idea ratio used, in this mode, we use the
        // max clk rate to accelerate process speed.
        i = 7;
        *divider = 7;
        *prescaler = 0;
        return;
    }
    else{
        i= 0;
   
        while(ratio > 8){
            i++;
            ratio = ratio >>1; 
        }

        *prescaler = i;
        /*if ((2<<i)* (ratio-1) >= (bitClockRate/sampleClockRate)){
            *divider = (ratio-1);
        }else {
            *divider = ratio;
        }*/
        if(i>=1){
            *divider = ((bitClockRate/sampleClockRate + (1<<(i-1)) -1)>>i)- 1;
        }else{
            *divider = bitClockRate/sampleClockRate - 1;
        }

        /*RETAILMSG(1,(_T("CalClockRate: prescaler(%d) divider(%d) (%d)(%d)(%d)(%d)\r\n"), 
            *prescaler, *divider,bitClockRate/sampleClockRate,bitClockRate/sampleClockRate + (1<<i),
            bitClockRate/sampleClockRate + (1<<i) -1,i));*/

    }
    return; 
    
}

static BOOL CalProcessOpition(UINT32 inSampleRate,UINT32 outSampleRate,
        UINT32 *preProc, UINT32 *postProc )
{
    FLOAT fsIn = (FLOAT)inSampleRate;
    FLOAT fsOut = (FLOAT)outSampleRate;


    if ( fsIn > (16.125 * fsOut) ){
        //not supported, *proProc = 5
        RETAILMSG(TRUE,(_T("Not supported\r\n")));
        return FALSE;
    }else if ( fsIn > (8.125*fsOut) ){
        //not supported, * preProc = 4
        RETAILMSG(TRUE,(_T("Not supported\r\n")));
        return FALSE;
    }else if (fsIn > (4.125 * fsOut) ){
        *preProc = 2;
    }else if (fsIn > (1.875 * fsOut) ){
        if (fsIn >15200)
            *preProc = 2;
        else
            *preProc = 1;
    }else if (fsIn < 76000 ){
        *preProc =0;
    }else if (fsIn > 152000){
        *preProc = 2;
    }else {
        *preProc = 1;
    }

    if ( ((fsIn > fsOut/(1.0 - (1.0/16))) && (fsOut < 56000))
          ||  ((fsIn> 56000) && (fsOut <56000))  ){
            *postProc = 2;
    }else if ( fsIn < fsOut/(2.0 + 1.0 - (1.0/8)) ){
            *postProc = 0;
    }else{
            *postProc = 1;
    }

    return TRUE;
}

static void CalConvertRatio(UINT32 inSampleRate,UINT32 outSampleRate,
        UINT32 *highPart, UINT32 *lowPart)
{
    UINT32 interPart, fracPart;
    UINT64 temp;

    
    interPart = (inSampleRate/outSampleRate) & 0x3F;

    inSampleRate = inSampleRate % outSampleRate;

    temp = (UINT64)inSampleRate * (1 << 26);

    temp = (temp/outSampleRate); 

    fracPart = (UINT32)(temp & 0x03FFFFFF);

    *highPart =  (interPart << 2) | (fracPart >>24);
    *lowPart = fracPart & 0x00FFFFFF;

    return;
}


static UINT32 GetAsrcRate(void)
{
    return 768000;
}


//-----------------------------------------------------------------------------
//
//  Function:  AsrcInit
//
//  This function initializes the ASRC.  
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL AsrcInit(UINT32  *ppContext)
{
    PHYSICAL_ADDRESS    phyAddr;
    BOOL ret = FALSE;
    PCSP_ASRC_REG  pReg;

    AsrcEnableClock();

    if (g_pAsrcContext!= NULL){
        return TRUE;
    }

    g_pAsrcContext = (PASRC_CONTROLLER_CONTEXT)LocalAlloc(LPTR,
        sizeof(ASRC_CONTROLLER_CONTEXT));

    if (g_pAsrcContext == NULL){
        return FALSE;
    }

    phyAddr.QuadPart = AsrcGetBaseRegAddr();

    pReg = (PCSP_ASRC_REG)MmMapIoSpace(phyAddr, sizeof(PCSP_ASRC_REG),FALSE);
    if (pReg == NULL){
        ret = FALSE;
        goto initexit;
    }
    g_pAsrcContext->pAsrcReg = pReg;

    //Get asrc version
    g_pAsrcContext->dwVersion = AsrcGetVersion();
   
    // set asrc en
    SETREG32(&(pReg->ASRCTR), CSP_BITFVAL(ASRCTR_ASRCEN,1));
    //mask all interrupts and setup isr
    OUTREG32(&(pReg->ASRIER), 0);

    //set the default param
    OUTREG32(&(pReg->ASRPM1), 0x7FFFFF);
    OUTREG32(&(pReg->ASRPM2), 0x255555);
    OUTREG32(&(pReg->ASRPM3), 0xFF7280);
    OUTREG32(&(pReg->ASRPM4), 0xFF7208);
    OUTREG32(&(pReg->ASRPM5), 0xFF7280);

    OUTREG32(&(pReg->ASRCFG), 0); //USE DEFAULT PARAN

    // set ASR76K and ASR56K
    OUTREG32(&(pReg->ASR56K), 0x0947);
    OUTREG32(&(pReg->ASR76K), 0x06D6);
    OUTREG32(&(pReg->ASRTFR1), 0x1f00);

 
   if( ppContext != NULL){
        *ppContext =(UINT32) g_pAsrcContext ;
   }
   return TRUE;

initexit:

    if (g_pAsrcContext->pAsrcReg != NULL){
        MmUnmapIoSpace((void*)g_pAsrcContext->pAsrcReg, sizeof(PCSP_ASRC_REG));
    }

    if (g_pAsrcContext != NULL){
        LocalFree((HLOCAL) g_pAsrcContext);
    }    
    
    return ret;
}

//-----------------------------------------------------------------------------
//
// Function:  AsrcDeinit
//
// This function deinitializes the ASRC.  
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL AsrcDeinit(void)
{
    if (g_pAsrcContext == NULL)
        return TRUE;
    if (g_pAsrcContext->pAsrcReg != NULL){
        MmUnmapIoSpace((void*)g_pAsrcContext->pAsrcReg, sizeof(PCSP_ASRC_REG));
    }

    if (g_pAsrcContext != NULL){
        LocalFree((HLOCAL) g_pAsrcContext);
        g_pAsrcContext = NULL;
    }    
    return TRUE;
}


static ASRC_PAIR_INDEX AsrcRequstPair(UINT chnNum,ASRC_PAIR_INDEX pairIndex)
{
    //check chan num configure reg, find the available one (0,disabled)
    //config NUM OF CHN OF this pair
    //acquie the convert pair and set asrc cncr

    INT32 mask, val, reg;
    PCSP_ASRC_REG pReg =  g_pAsrcContext->pAsrcReg;
   
    if (g_pAsrcContext->dwVersion == ASRC_VERSION_1){
        switch (pairIndex){
            case ASRC_PAIR_A:
               mask = CSP_BITFMASK(ASRCNCR_V1_ANCA);
               val = CSP_BITFVAL(ASRCNCR_V1_ANCA, (chnNum>>1));
               break;

            case ASRC_PAIR_B:
               mask = CSP_BITFMASK(ASRCNCR_V1_ANCB);
               val = CSP_BITFVAL(ASRCNCR_V1_ANCB, (chnNum>>1));
               break;
               
            case ASRC_PAIR_C:
               mask = CSP_BITFMASK(ASRCNCR_V1_ANCC);
               val = CSP_BITFVAL(ASRCNCR_V1_ANCC, (chnNum>>1));
               break;

            default:
               return ASRC_PAIR_NONE;
        }
    }else{
        switch (pairIndex){
            case ASRC_PAIR_A:
               mask = CSP_BITFMASK(ASRCNCR_V2_ANCA);
               val = CSP_BITFVAL(ASRCNCR_V2_ANCA, (chnNum));
               break;

            case ASRC_PAIR_B:
               mask = CSP_BITFMASK(ASRCNCR_V2_ANCB);
               val = CSP_BITFVAL(ASRCNCR_V2_ANCB, (chnNum));
               break;
               
            case ASRC_PAIR_C:
               mask = CSP_BITFMASK(ASRCNCR_V2_ANCC);
               val = CSP_BITFVAL(ASRCNCR_V2_ANCC, (chnNum));
               break;

            default:
               return ASRC_PAIR_NONE;
        }
    }
       
    reg = INREG32(&(pReg->ASRCNCR));
    if ((reg & mask) != 0){
        return ASRC_PAIR_NONE;
    }    

    val |= reg;
    if (InterlockedTestExchange((LPLONG) &(pReg->ASRCNCR), 
                reg, val) 
                != reg)
    {
        return  ASRC_PAIR_NONE;
    }

    return pairIndex;
}



static void  AsrcFreePair(UINT chnNum,ASRC_PAIR_INDEX pairIndex)
{
    //check chan num configure reg, find the available one (0,disabled)
    //config NUM OF CHN OF this pair
    //acquie the convert pair and set asrc cncr

    INT32 mask, val, reg,count;
    PCSP_ASRC_REG pReg =  g_pAsrcContext->pAsrcReg;
   
    if (g_pAsrcContext->dwVersion == ASRC_VERSION_1){  
        switch (pairIndex){
            case ASRC_PAIR_A:
               mask = CSP_BITFMASK(ASRCNCR_V1_ANCA);
               val = CSP_BITFVAL(ASRCNCR_V1_ANCA, (chnNum>>1));
               break;

            case ASRC_PAIR_B:
               mask = CSP_BITFMASK(ASRCNCR_V1_ANCB);
               val = CSP_BITFVAL(ASRCNCR_V1_ANCB, (chnNum>>1));
               break;
               
            case ASRC_PAIR_C:
               mask = CSP_BITFMASK(ASRCNCR_V1_ANCC);
               val = CSP_BITFVAL(ASRCNCR_V1_ANCC, (chnNum>>1));
               break;

            default:
               return ;
        }
    }else{
        switch (pairIndex){
            case ASRC_PAIR_A:
               mask = CSP_BITFMASK(ASRCNCR_V2_ANCA);
               val = CSP_BITFVAL(ASRCNCR_V2_ANCA, (chnNum));
               break;

            case ASRC_PAIR_B:
               mask = CSP_BITFMASK(ASRCNCR_V2_ANCB);
               val = CSP_BITFVAL(ASRCNCR_V2_ANCB, (chnNum));
               break;
               
            case ASRC_PAIR_C:
               mask = CSP_BITFMASK(ASRCNCR_V2_ANCC);
               val = CSP_BITFVAL(ASRCNCR_V2_ANCC, (chnNum));
               break;

            default:
               return ;
        }

    }
       
    reg = INREG32(&(pReg->ASRCNCR));
    if ((reg & mask) != val){
        // error
        return;
    }    

    val = reg &(~mask);
    
    for(count = 0; count < 1000; count ++){
        if (InterlockedTestExchange((LPLONG)&(pReg->ASRCNCR), 
                reg, val) == reg){
                break;
        }
        reg = INREG32(&(pReg->ASRCNCR));
        val = reg &(~mask); 

    } 

    return;
}

static void AsrcReleaseSDMA(PASRC_PAIR_CONTEXT pPairContext)
{
    if (pPairContext->inputDMAChan != 0){
        DDKSdmaCloseChan(pPairContext->inputDMAChan);
        pPairContext->inputDMAChan = 0;
    }

    if (pPairContext->outputDMAChan!= 0){
        DDKSdmaCloseChan((UINT8)pPairContext->outputDMAChan);
        pPairContext->outputDMAChan = 0;
    }

    return;
}

//input: mem->asrc
//output: asrc->mem

static BOOL AsrcAllocateSDMA(PASRC_PAIR_CONTEXT pPairContext)
{
    BOOL res;
    
    pPairContext->inputDMAReq = (DDK_DMA_REQ)AsrcGetInputDMAReq(pPairContext->index);

    if(pPairContext->transMode == ASRC_TRANS_MEM2ESAI){
        pPairContext->outputDMAReq = (DDK_DMA_REQ)AsrcGetOutputP2PDMAReq(pPairContext->index);
        //pPairContext->outputDMAReq = (DDK_DMA_REQ)AsrcGetOutputDMAReq(pPairContext->index);
     
    }else{    
        pPairContext->outputDMAReq = (DDK_DMA_REQ)AsrcGetOutputDMAReq(pPairContext->index);
    }
    pPairContext ->inputDMAPriority = AsrcGetInputDMAPriority(pPairContext ->index);
    pPairContext ->outputDMAPriority = AsrcGetOutputDMAPriority(pPairContext ->index);

    pPairContext->inputDMAChan= DDKSdmaOpenChan(pPairContext ->inputDMAReq, 
                                              pPairContext ->inputDMAPriority);
    
    if(pPairContext->inputDMAChan == 0){
        res =  FALSE;
        goto exit;
    }

    if (!DDKSdmaAllocChain(pPairContext->inputDMAChan, ASRC_NUM_DMA_BUFFERS)){
        res = FALSE;
        goto exit;
    }

    pPairContext->outputDMAChan= DDKSdmaOpenChan(pPairContext ->outputDMAReq, 
                                              pPairContext ->outputDMAPriority);
     
    if(pPairContext->outputDMAChan == 0){
        res = FALSE;
        goto exit;
    }

    if(pPairContext->transMode == ASRC_TRANS_MEM2ESAI){
        // for output, we use p2p, never use more than 1 buffer
        if (!DDKSdmaAllocChain((UINT8)pPairContext->outputDMAChan, 1)){
            res = FALSE;
            goto exit;
        }

    }else{

        if (!DDKSdmaAllocChain((UINT8)pPairContext->outputDMAChan, ASRC_NUM_DMA_BUFFERS)){
            res = FALSE;
            goto exit;
        }
    }

    res = TRUE;
    
exit:
    if(res ==FALSE){
        AsrcReleaseSDMA(pPairContext);   
    }
    
    return res;
}


//-----------------------------------------------------------------------------
//
//  Function:  AsrcOpenPair
//
//  This function request  the pair for convertion 
//
//  Parameters:
//      indexPair:  pair index
//      inputChnNum: input channel number, should be even now
//      outputChnNum: output channel number, should be equal to input channel number now
//      hInputEvent:  event to trigger when input dma interrupt happens
//      hOutputEvent: event to trigger when output dma interrupt happens
//      transMode: mem->mem or mem->esai, etc..
//  Returns:
//      Pair index requested.
//
//-----------------------------------------------------------------------------
ASRC_PAIR_INDEX AsrcOpenPair(ASRC_PAIR_INDEX pairIndex,
                UINT32 inputChnNum,UINT32 outputChnNum, 
                HANDLE hInputEvent, HANDLE hOutputEvent,
                ASRC_TRANS_MODE transMode)
{
    //UINT32 mask,val,reg,
    UINT32 pairChnNum = 0;
    ASRC_PAIR_INDEX pairIndexRet = ASRC_PAIR_NONE;
    PASRC_PAIR_CONTEXT pPairContext;
    //UINT32  aIrqs[4];

    if(!AsrcSupportOddChnNum())
        pairChnNum = (inputChnNum + 1) & 0xfffe;
    else
        pairChnNum = inputChnNum;

   // RETAILMSG(1,(TEXT("To request pair: index(%d) pairChnNum(%d)\r\n"),
   //     pairIndex,pairChnNum));
    
    pairIndexRet = AsrcRequstPair(pairChnNum,pairIndex);
    //RETAILMSG(1,(TEXT("AsrcRequstPair: pairIndexRet(%d)\r\n"),
    //    pairIndexRet));
    if (pairIndexRet == ASRC_PAIR_NONE){
        return pairIndexRet;
    }

    
    //allocate the data structure , and set the default val
    pPairContext = (PASRC_PAIR_CONTEXT)LocalAlloc(LPTR,sizeof(ASRC_PAIR_CONTEXT));
    if ( pPairContext== NULL){
        RETAILMSG(1,(TEXT("Locall Alloc context failed\r\n")));
        pairIndexRet  = ASRC_PAIR_NONE ;
        goto openexit;
    }

    g_pAsrcContext->pPairArray[pairIndexRet] = pPairContext;
    pPairContext->index = pairIndexRet;
    pPairContext->inputChnNum = inputChnNum;
    pPairContext->outputChnNum = outputChnNum;
    pPairContext->pairChnNum = pairChnNum;
    pPairContext ->transMode = transMode;
    pPairContext ->inputWaterMark = 32; //word (24bit)
    pPairContext ->outputWaterMark = 32; //word(24bit) 
    pPairContext ->bOpened = TRUE;
    pPairContext->dwInputSysintr = (DWORD)SYSINTR_UNDEFINED;
    pPairContext->dwOutputSysintr = (DWORD)SYSINTR_UNDEFINED;
    InitializeCriticalSection(&(pPairContext->lockInput) );
    InitializeCriticalSection(&(pPairContext->lockOutput) );

    //allocate sdma resource
    if (!AsrcAllocateSDMA(pPairContext)){
        pairIndexRet  = ASRC_PAIR_NONE ;
        goto openexit;    
    }

    //mask interrupt
    pPairContext->dwInputIRQ = AsrcGetSDMABaseIrq() + pPairContext->inputDMAChan; 
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &(pPairContext->dwInputIRQ ), 
                         sizeof(pPairContext->dwInputIRQ ), &(pPairContext->dwInputSysintr), 
                         sizeof(pPairContext->dwInputSysintr), NULL)){
        pairIndexRet  = ASRC_PAIR_NONE ;
        goto openexit; ;
    }

    pPairContext->inputEvent[0] = hInputEvent;

    if (!InterruptInitialize(pPairContext->dwInputSysintr, 
                            hInputEvent, NULL, 0))
    {
        pairIndexRet  = ASRC_PAIR_NONE ;
        goto openexit; 
    }

    pPairContext->dwOutputIRQ = AsrcGetSDMABaseIrq() + pPairContext->outputDMAChan; 
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &(pPairContext->dwOutputIRQ ), 
                         sizeof(pPairContext->dwOutputIRQ ), &(pPairContext->dwOutputSysintr), 
                         sizeof(pPairContext->dwOutputSysintr), NULL)){
        pairIndexRet  = ASRC_PAIR_NONE ;
        goto openexit; ;
    }


    pPairContext->outputEvent[0] = hOutputEvent;

    if (!InterruptInitialize(pPairContext->dwOutputSysintr, 
                            hOutputEvent, NULL, 0))
    {
        pairIndexRet  = ASRC_PAIR_NONE ;
        goto openexit; 
    }

    
openexit:
    
    if (pairIndexRet == ASRC_PAIR_NONE){
        AsrcClosePair(pairIndex);
    }
    return pairIndexRet;
  
}



//-----------------------------------------------------------------------------
//
//  Function:  AsrcClosePair
//
//  This function releases the pair 
//
//  Parameters:
//      indexPair:  pair index
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
void AsrcClosePair(ASRC_PAIR_INDEX indexPair)
{
    PASRC_PAIR_CONTEXT pPairContext; 
    UINT32 chnNum;

    if ((indexPair < ASRC_PAIR_A) || (indexPair > ASRC_PAIR_C))
        return;
    
    pPairContext = g_pAsrcContext->pPairArray[indexPair];
    
    if (pPairContext == NULL){
        return;
    }
    chnNum = pPairContext->pairChnNum;    
    pPairContext->bOpened = FALSE;

    
    if (pPairContext->dwInputSysintr != (DWORD)SYSINTR_UNDEFINED){
        InterruptDisable(pPairContext->dwInputSysintr);
         // Release SYSINTR
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &(pPairContext->dwInputSysintr), 
                    sizeof(DWORD), NULL, 0, NULL);
         pPairContext->dwInputSysintr = (DWORD)SYSINTR_UNDEFINED;
    }

    if (pPairContext->dwOutputSysintr != (DWORD)SYSINTR_UNDEFINED){
        InterruptDisable(pPairContext->dwOutputSysintr);
         // Release SYSINTR
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &(pPairContext->dwOutputSysintr), 
                    sizeof(DWORD), NULL, 0, NULL);
         pPairContext->dwOutputSysintr = (DWORD)SYSINTR_UNDEFINED;
    }

    DeleteCriticalSection(&(pPairContext->lockInput));
    DeleteCriticalSection(&(pPairContext->lockOutput));

    AsrcReleaseSDMA(pPairContext);
    

    LocalFree((HLOCAL)pPairContext);
    g_pAsrcContext->pPairArray[indexPair] = NULL;

    AsrcFreePair(chnNum,indexPair);
    
    return;

}



//This function does nothing
BOOL AsrcInitPair()
{
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  AsrcConfigPair
//
//  This function configure the pair for conversion 
//
//  Parameters:
//      indexPair:  pair index
//      pParam:   config param      
//  Returns:
//      Returns TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL AsrcConfigPair(ASRC_PAIR_INDEX indexPair,PASRC_CONFIG_PARAM pParam)
{
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    PCSP_ASRC_REG pReg =  g_pAsrcContext->pAsrcReg;
//    UINT32 reg,val,newReg,mask,mask2;
    UINT32 valCSR=0,maskCSR=0, valCTR=0,maskCTR=0;
    UINT32 valCDR=0,maskCDR=0;
    UINT32 prescalerIn=0,dividerIn=0,prescalerOut=0,dividerOut=0;
    UINT32 preProc=0,postProc=0,valCFG=0,maskCFG=0;
    UINT32 ratioLow=0,ratioHigh=0;

    pPairContext->configParam.clkMode = pParam->clkMode;
    pPairContext->configParam.inputSampleRate = pParam->inputSampleRate;
    pPairContext->configParam.outputSampleRate = pParam->outputSampleRate;
    pPairContext->configParam.inClkSrc = pParam->inClkSrc;
    pPairContext->configParam.outClkSrc = pParam->outClkSrc;
    pPairContext->configParam.inputBitClkRate = pParam->inputBitClkRate;
    pPairContext->configParam.outputBitClkRate = pParam->outputBitClkRate; 



    
   /* RETAILMSG(1,(TEXT("AsrcCfg v2.1: clkmode(%d)in sampel(%d)out sample(%d)clk src(%d) out src(%d) in bit rate(%d) out bitrate(%d)\r\n"),
     pParam->clkMode, pParam->inputSampleRate, pParam->outputSampleRate,
    pParam->inClkSrc, pParam->outClkSrc,pParam->inputBitClkRate,
    pParam->outputBitClkRate));*/
     

    if ( pParam->clkMode == ASRC_CLK_ONE_SRC_OUTPUT){
        pPairContext->configParam.inClkSrc = ASRC_ICLK_SRC_ASRCK1;
        pParam->inClkSrc = ASRC_ICLK_SRC_ASRCK1;
        pPairContext->configParam.inputBitClkRate = GetAsrcRate();
        pParam->inputBitClkRate = GetAsrcRate();
        pPairContext->bIdealRatioMode = TRUE; //force to be true, this will ignore the input clk seting
    }else if ( pParam->clkMode == ASRC_CLK_NONE_SRC){
        pPairContext->configParam.outClkSrc = ASRC_OCLK_SRC_ASRCK1;
        pParam->outClkSrc = ASRC_OCLK_SRC_ASRCK1;
        pPairContext->configParam.outputBitClkRate = GetAsrcRate(); 
        //pParam->outputSampleRate = pPairContext->configParam.outputBitClkRate;
        pParam->outputBitClkRate = pPairContext->configParam.outputBitClkRate;

        pPairContext->configParam.inClkSrc = ASRC_ICLK_SRC_RSV1;
        pParam->inClkSrc = ASRC_ICLK_SRC_RSV1; 

        pPairContext->bIdealRatioMode = TRUE;
    }else if ( pParam->clkMode == ASRC_CLK_ONE_SRC_OUTPUT_AUTO_SEL){
        //select the appropriate clock according to output sample rate
        BSPConfigClkSrc(pParam);

        pPairContext->configParam.outClkSrc = pParam->outClkSrc;
        pPairContext->configParam.outputBitClkRate = pParam->outputBitClkRate;
        pPairContext->bIdealRatioMode = TRUE;

        //RETAILMSG(1,(TEXT("Config pair: output clk src(%d) bitrate(%d)\r\n"),
        //    pPairContext->configParam.outClkSrc,pPairContext->configParam.outputBitClkRate));
    }

    //RETAILMSG(1,(TEXT("AsrcCfg 2: clkmode(%d)in sampel(%d)out sample(%d)clk src(%d) out src(%d) in bit rate(%d) out bitrate(%d)\r\n"),
    // pParam->clkMode, pParam->inputSampleRate, pParam->outputSampleRate,
    // pParam->inClkSrc, pParam->outClkSrc,pParam->inputBitClkRate,
    // pParam->outputBitClkRate));


    //if (pParam->clkMode != ASRC_CLK_NONE_SRC){
    //if (pParam->clkMode == ASRC_CLK_ONE_SRC_OUTPUT){
    //if (pParam->clkMode == ASRC_CLK_TWO_SRC){
    if (!pPairContext->bIdealRatioMode){
            // in this mode both input clock and output clock are available from audio device.
            
            CalClockRate(pPairContext->configParam.outputBitClkRate,
                                 pPairContext->configParam.outputSampleRate,
                                 &prescalerOut, &dividerOut, FALSE); 
            CalClockRate(pPairContext->configParam.inputBitClkRate,
                                 pPairContext->configParam.inputSampleRate,
                                 &prescalerIn, &dividerIn, FALSE);
          

            
            switch (indexPair){
            case ASRC_PAIR_A:

                maskCTR = CSP_BITFMASK(ASRCTR_ATSA) 
                                    | CSP_BITFMASK(ASRCTR_IDRA) 
                                    | CSP_BITFMASK(ASRCTR_USRA);
                valCTR = CSP_BITFVAL(ASRCTR_ATSA, 1)
                                    | CSP_BITFVAL(ASRCTR_IDRA, 0)
                                     | CSP_BITFVAL(ASRCTR_USRA, 1);
                maskCSR = CSP_BITFMASK(ASRCSR_AICSA) 
                                    | CSP_BITFMASK(ASRCSR_AOCSA) ;
                valCSR = CSP_BITFVAL(ASRCSR_AICSA, pParam->inClkSrc)
                                    | CSP_BITFVAL(ASRCSR_AOCSA, pParam->outClkSrc);
                maskCDR = CSP_BITFMASK(ASRCDR1_AICPA)
                                    | CSP_BITFMASK(ASRCDR1_AICDA)
                                    | CSP_BITFMASK(ASRCDR1_AOCPA)
                                    | CSP_BITFMASK(ASRCDR1_AOCDA);
                valCDR = CSP_BITFVAL(ASRCDR1_AICPA, prescalerIn)
                                    | CSP_BITFVAL(ASRCDR1_AICDA, dividerIn)
                                    | CSP_BITFVAL(ASRCDR1_AOCPA, prescalerOut)
                                    | CSP_BITFVAL(ASRCDR1_AOCDA, dividerOut);

                AsrcSetReg(&pReg->ASRCDR1, valCDR, maskCDR); 

                break;

             case ASRC_PAIR_B:

           
                maskCTR = CSP_BITFMASK(ASRCTR_ATSB) 
                                    | CSP_BITFMASK(ASRCTR_IDRB) 
                                    | CSP_BITFMASK(ASRCTR_USRB);
                valCTR = CSP_BITFVAL(ASRCTR_ATSB, 1)
                                    | CSP_BITFVAL(ASRCTR_IDRB, 0)
                                     | CSP_BITFVAL(ASRCTR_USRB, 1);
                maskCSR = CSP_BITFMASK(ASRCSR_AICSB) 
                                    | CSP_BITFMASK(ASRCSR_AOCSB) ;
                valCSR = CSP_BITFVAL(ASRCSR_AICSB, pParam->inClkSrc)
                                    | CSP_BITFVAL(ASRCSR_AOCSB, pParam->outClkSrc);
                maskCDR = CSP_BITFMASK(ASRCDR1_AICPB)
                                    | CSP_BITFMASK(ASRCDR1_AICDB)
                                    | CSP_BITFMASK(ASRCDR1_AOCPB)
                                    | CSP_BITFMASK(ASRCDR1_AOCDB);
                valCDR = CSP_BITFVAL(ASRCDR1_AICPB, prescalerIn)
                                    | CSP_BITFVAL(ASRCDR1_AICDB, dividerIn)
                                    | CSP_BITFVAL(ASRCDR1_AOCPB, prescalerOut)
                                    | CSP_BITFVAL(ASRCDR1_AOCDB, dividerOut);
                AsrcSetReg(&pReg->ASRCDR1, valCDR, maskCDR); 
                
                break;      

             case ASRC_PAIR_C:

             
                maskCTR = CSP_BITFMASK(ASRCTR_ATSC) 
                                    | CSP_BITFMASK(ASRCTR_IDRC) 
                                    | CSP_BITFMASK(ASRCTR_USRC);
                valCTR = CSP_BITFVAL(ASRCTR_ATSC, 1)
                                    | CSP_BITFVAL(ASRCTR_IDRC, 0)
                                     | CSP_BITFVAL(ASRCTR_USRC, 1);
                maskCSR = CSP_BITFMASK(ASRCSR_AICSC) 
                                    | CSP_BITFMASK(ASRCSR_AOCSC) ;
                valCSR = CSP_BITFVAL(ASRCSR_AICSC, pParam->inClkSrc)
                                    | CSP_BITFVAL(ASRCSR_AOCSC, pParam->outClkSrc);
                maskCDR = CSP_BITFMASK(ASRCDR2_AICPC)
                                    | CSP_BITFMASK(ASRCDR2_AICDC)
                                    | CSP_BITFMASK(ASRCDR2_AOCPC)
                                    | CSP_BITFMASK(ASRCDR2_AOCDC);
                valCDR = CSP_BITFVAL(ASRCDR2_AICPC, prescalerIn)
                                    | CSP_BITFVAL(ASRCDR2_AICDC, dividerIn)
                                    | CSP_BITFVAL(ASRCDR2_AOCPC, prescalerOut)
                                    | CSP_BITFVAL(ASRCDR2_AOCDC, dividerOut);
                OUTREG32(&pReg->ASRCDR2, valCDR);
                
                break;     
            default:
                break;
        }

        // set clk src 
        AsrcSetReg(&pReg->ASRCSR, valCSR, maskCSR);
        // set control
        AsrcSetReg(&pReg->ASRCTR, valCTR, maskCTR);
        //AsrcSetReg(&pReg->ASRCSR, valCSR, maskCSR); 

        


    }
    //else if (pParam->clkMode == ASRC_CLK_NONE_SRC){
    else{// ideal ratio mode
        //RETAILMSG(1,(TEXT("config asrc with ideal mode\r\n")));
        if(pParam->clkMode == ASRC_CLK_NONE_SRC){
            //the clock is not from codec, will not affect the output
            CalClockRate(pPairContext->configParam.outputBitClkRate,
                                 pPairContext->configParam.outputSampleRate,
                                 &prescalerOut, &dividerOut,
                                 TRUE); 
        }else{
            CalClockRate(pPairContext->configParam.outputBitClkRate,
                                 pPairContext->configParam.outputSampleRate,
                                 &prescalerOut, &dividerOut,
                                 FALSE); 

        }
        
        CalProcessOpition( pPairContext->configParam.inputSampleRate,
                                        pPairContext->configParam.outputSampleRate,
                                        &preProc, &postProc);
        CalConvertRatio( pPairContext->configParam.inputSampleRate,
                                    pPairContext->configParam.outputSampleRate,
                                    &ratioHigh, &ratioLow);
            
        // set asrcsr, clock source
        switch (indexPair){
            case ASRC_PAIR_A:

                maskCSR = CSP_BITFMASK(ASRCSR_AOCSA);
                valCSR = CSP_BITFVAL(ASRCSR_AOCSA, pParam->outClkSrc);
                maskCTR = CSP_BITFMASK(ASRCTR_ATSA) 
                                    | CSP_BITFMASK(ASRCTR_IDRA) 
                                    | CSP_BITFMASK(ASRCTR_USRA);
                valCTR = CSP_BITFVAL(ASRCTR_ATSA, 0)    //diable auto sel for process opition
                                    | CSP_BITFVAL(ASRCTR_IDRA, 1)   //use ideal ratio
                                     | CSP_BITFVAL(ASRCTR_USRA, 1);
                maskCDR = CSP_BITFMASK(ASRCDR1_AOCPA)
                                    | CSP_BITFMASK(ASRCDR1_AOCDA);
                valCDR =  CSP_BITFVAL(ASRCDR1_AOCPA, prescalerOut)
                                    | CSP_BITFVAL(ASRCDR1_AOCDA, dividerOut);

                maskCFG = CSP_BITFMASK(ASRCFG_POSTMODA)
                                    | CSP_BITFMASK(ASRCFG_PREMODA);
                valCFG = CSP_BITFVAL(ASRCFG_POSTMODA, postProc)
                                    | CSP_BITFVAL(ASRCFG_PREMODA, preProc);

                AsrcSetReg(&pReg->ASRCDR1, valCDR, maskCDR); 
                OUTREG32(&pReg->ASRIDRHA, ratioHigh);
                OUTREG32(&pReg->ASRIDRLA, ratioLow);

                break;

             case ASRC_PAIR_B:

                maskCSR = CSP_BITFMASK(ASRCSR_AOCSB);
                valCSR = CSP_BITFVAL(ASRCSR_AOCSB, pParam->outClkSrc);
                maskCTR = CSP_BITFMASK(ASRCTR_ATSB) 
                                    | CSP_BITFMASK(ASRCTR_IDRB) 
                                    | CSP_BITFMASK(ASRCTR_USRB);
                valCTR = CSP_BITFVAL(ASRCTR_ATSB, 0)
                                    | CSP_BITFVAL(ASRCTR_IDRB, 1)
                                     | CSP_BITFVAL(ASRCTR_USRB, 1);
                maskCDR = CSP_BITFMASK(ASRCDR1_AOCPB)
                                    | CSP_BITFMASK(ASRCDR1_AOCDB);
                valCDR =  CSP_BITFVAL(ASRCDR1_AOCPB, prescalerOut)
                                    | CSP_BITFVAL(ASRCDR1_AOCDB, dividerOut);
                maskCFG = CSP_BITFMASK(ASRCFG_POSTMODB)
                                    | CSP_BITFMASK(ASRCFG_PREMODB);
                valCFG = CSP_BITFVAL(ASRCFG_POSTMODB, postProc)
                                    | CSP_BITFVAL(ASRCFG_PREMODB, preProc);

                AsrcSetReg(&pReg->ASRCDR1, valCDR, maskCDR); 
                OUTREG32(&pReg->ASRIDRHB, ratioHigh);
                OUTREG32(&pReg->ASRIDRLB, ratioLow);
                break;      

             case ASRC_PAIR_C:

                maskCSR = CSP_BITFMASK(ASRCSR_AOCSC);
                valCSR = CSP_BITFVAL(ASRCSR_AOCSC, pParam->outClkSrc);
                maskCTR = CSP_BITFMASK(ASRCTR_ATSC) 
                                    | CSP_BITFMASK(ASRCTR_IDRC) 
                                    | CSP_BITFMASK(ASRCTR_USRC);
                valCTR = CSP_BITFVAL(ASRCTR_ATSC, 0)
                                    | CSP_BITFVAL(ASRCTR_IDRC, 1)
                                     | CSP_BITFVAL(ASRCTR_USRC, 1);
                maskCDR = CSP_BITFMASK(ASRCDR2_AOCPC)
                                    | CSP_BITFMASK(ASRCDR2_AOCDC);
                valCDR = CSP_BITFVAL(ASRCDR2_AOCPC, prescalerOut)
                                    | CSP_BITFVAL(ASRCDR2_AOCDC, dividerOut);
                maskCFG = CSP_BITFMASK(ASRCFG_POSTMODC)
                                    | CSP_BITFMASK(ASRCFG_PREMODC);
                valCFG = CSP_BITFVAL(ASRCFG_POSTMODC, postProc)
                                    | CSP_BITFVAL(ASRCFG_PREMODC, preProc);

                AsrcSetReg(&pReg->ASRCDR2, valCDR,maskCDR);
                OUTREG32(&pReg->ASRIDRHC, ratioHigh);
                OUTREG32(&pReg->ASRIDRLC, ratioLow);               
                
                break;     
            default:
                break;
        }
        
        AsrcSetReg(&pReg->ASRCSR, valCSR, maskCSR);
        AsrcSetReg(&pReg->ASRCTR, valCTR, maskCTR);

        //SET ASRCFG
        AsrcSetReg(&pReg->ASRCFG, valCFG, maskCFG); 

        //IDEAL RATIO

    } 

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  AsrcSetInputBuf
//
//  This function sets buffer for input function (mem ->asrc)  
//
//  Parameters:
//      indexPair:  pair index
//      phyAddr:   physical address      
//      numBytes: buffer size
//      index: buffer index, 0 or 1
//      pVirtBuf: virtual addr pointer
//  Returns:
//      Returns TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL AsrcSetInputBuf(ASRC_PAIR_INDEX indexPair, PHYSICAL_ADDRESS phyAddr,
    UINT32 numBytes, UINT32 index,PBYTE pVirtBuf )
{
    UINT32 modeFlags;
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    
    UNREFERENCED_PARAMETER(pVirtBuf);

    if(index >= ASRC_DMA_BUFFER_NUM)
        return FALSE;

    ( pPairContext->phyAddrInputBuf[index]).LowPart = phyAddr.LowPart;
    pPairContext->inputBufSize[index] = numBytes;


    if (index == 1){
        modeFlags = (DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT | DDK_DMA_FLAGS_WRAP);
    }else{
        modeFlags = (DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT );
    }

    if (!DDKSdmaSetBufDesc(pPairContext->inputDMAChan,
                           index,
                           modeFlags,
                           phyAddr.LowPart,
                           0,
                           DDK_DMA_ACCESS_32BIT, //we always use 32bit for asrc dma
                           (UINT16)numBytes)) {
        (pPairContext->phyAddrInputBuf[index]).LowPart = 0;                  
        pPairContext->inputBufSize[index] = 0;      
        return FALSE;
   }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  AsrcSetOutputBuf
//
//  This function sets buffer for output function (asrc->mem)  
//
//  Parameters:
//      indexPair:  pair index
//      phyAddr:   physical address      
//      numBytes: buffer size
//      index: buffer number, 0 or 1
//      pVirtBuf: virtual addr pointer
//  Returns:
//      Returns TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL AsrcSetOutputBuf(ASRC_PAIR_INDEX indexPair, PHYSICAL_ADDRESS phyAddr,
    UINT32 numBytes, UINT32 index,PBYTE pVirtBuf )
{
    UINT32 modeFlags;
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    
    UNREFERENCED_PARAMETER(pVirtBuf);

    if(pPairContext->transMode == ASRC_TRANS_MEM2ESAI){
        modeFlags = (DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT ); //it's not necessay
        if (!DDKSdmaSetBufDesc(pPairContext->outputDMAChan,
                           0,
                           modeFlags,
                           0,
                           0,
                           DDK_DMA_ACCESS_32BIT, //We always use 32bit for asrc p2p
                           60)) { //For p2p we just use some non-zero val
            (pPairContext->phyAddrOutputBuf[index]).LowPart = 0;                  
            pPairContext->outputBufSize[index] = 0;      
            return FALSE;
        }
    }
    else if(pPairContext->transMode == ASRC_TRANS_MEM2MEM){
    
    if(index >= ASRC_DMA_BUFFER_NUM)
        return FALSE;

    ( pPairContext->phyAddrOutputBuf[index]).LowPart = phyAddr.LowPart;
    pPairContext->outputBufSize[index] = numBytes;

   
    memset((void*)pVirtBuf,0,numBytes);

    if (index == 1){
        modeFlags = (DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT | DDK_DMA_FLAGS_WRAP);
    }else{
        modeFlags = (DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT );
    }

    if (!DDKSdmaSetBufDesc(pPairContext->outputDMAChan,
                           index,
                           modeFlags,
                           phyAddr.LowPart,
                           0,
                           DDK_DMA_ACCESS_32BIT,
                           (UINT16)numBytes)) {
        (pPairContext->phyAddrOutputBuf[index]).LowPart = 0;                  
        pPairContext->outputBufSize[index] = 0;      
        return FALSE;
   }
    } 
    return TRUE;
}

//This function is not used now
BOOL AsrcSetInputEvent(ASRC_PAIR_INDEX indexPair,HANDLE inputEvent)
{
    UNREFERENCED_PARAMETER(indexPair);
    UNREFERENCED_PARAMETER(inputEvent);
    return TRUE;
}

//This function is not used now
BOOL AsrcSetOutputEvent(ASRC_PAIR_INDEX indexPair,HANDLE inputEvent)
{
    UNREFERENCED_PARAMETER(indexPair);
    UNREFERENCED_PARAMETER(inputEvent);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  AsrcStartConv
//
//  This function starts asrc convertion  
//
//  Parameters:
//      indexPair:  pair index
//      bEnableChannel: enable the pair convertion operation or not      
//      bEanbleInput: enable input dma or  not
//      bEnableOutput: enable output dma or not
//  Returns:
//      Returns TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL AsrcStartConv(ASRC_PAIR_INDEX indexPair,BOOL bEnableChannel,
    BOOL bEanbleInput,BOOL bEnableOutput)
{
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair]; 
    BOOL res;
    UINT32 val=0,mask=0;
    PCSP_ASRC_REG pReg =  g_pAsrcContext->pAsrcReg;

#if 0
   UINT32 i,j;

   if (bEnableChannel){    
    //for(i=0;i<pPairContext->pairChnNum;i++){ 
            for(j=0,i=0;j<57;j++){
                switch(indexPair){
                    case ASRC_PAIR_A:
                        OUTREG32(&pReg->ASRDIA,0);
                        break;
                    case ASRC_PAIR_B:
                        OUTREG32(&pReg->ASRDIB,0);
                        break;
                    case ASRC_PAIR_C:
                        OUTREG32(&pReg->ASRDIC,0);
                        break;
                    default:
                        break;
                }//end of switch
            }
       
    //} //end of for(i=0;i<3;i++)

    }
#endif
   
    //Enable input DMA
    if(bEanbleInput){
        if (bEnableChannel){ //init dma since it's supposed to be the start of the whol transfer
            if(! DDKSdmaInitChain(pPairContext->inputDMAChan,pPairContext->inputWaterMark)){
                res = FALSE;
                goto exit;    
    
            }
        }    
        DDKSdmaStartChan(pPairContext->inputDMAChan);
    }    

    //Enable Output DMA
    if(bEnableOutput){
        if(bEnableChannel){
            if (pPairContext->transMode == ASRC_TRANS_MEM2ESAI){
                //p2p requires different init function, jeremy
                if(! DDKSdmaInitChain(pPairContext->outputDMAChan,
                    //pPairContext->pairChnNum * pPairContext->outputWaterMark)){
                    AsrcGetP2PInfo(pPairContext))){
                    res = FALSE;
                    goto exit;    
    
                }

            }else{
            
                if(! DDKSdmaInitChain(pPairContext->outputDMAChan,pPairContext->outputWaterMark)){
                    res = FALSE;
                    goto exit;    
    
                }
            }
        }
        DDKSdmaStartChan(pPairContext->outputDMAChan);
    } 


    if (bEnableChannel){
    //enable channel
        switch (indexPair)
        {
            case ASRC_PAIR_A:
              val = CSP_BITFVAL(ASRCTR_ASREA, 1);
              mask = CSP_BITFMASK(ASRCTR_ASREA);
              break;
            case ASRC_PAIR_B:
                val = CSP_BITFVAL(ASRCTR_ASREB, 1);
                mask = CSP_BITFMASK(ASRCTR_ASREB);
                break;            
            case ASRC_PAIR_C:
                val = CSP_BITFVAL(ASRCTR_ASREC, 1);
                mask = CSP_BITFMASK(ASRCTR_ASREC);
                break;
            default:
            break;
        }
        
        AsrcSetReg(&pReg->ASRCTR, val, mask);  


    }

    res = TRUE;
exit:

   return res; 
    
  // enable asrc en in control reg
}

//-----------------------------------------------------------------------------
//
//  Function:  AsrcStopConv
//
//  This function stops asrc convertion  
//
//  Parameters:
//      indexPair:  pair index
//      bStopChannel: stop the pair convertion operation or not      
//      bStopInput: stop input dma or  not
//      bStopOutput: stop output dma or not
//  Returns:
//      Returns TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL AsrcStopConv(ASRC_PAIR_INDEX indexPair,BOOL bStopChannel,
    BOOL bStopInput,BOOL bStopOutput)
{
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair]; 
    BOOL res;
    UINT32 val=0,mask=0;
    PCSP_ASRC_REG pReg =  g_pAsrcContext->pAsrcReg;

    if(bStopChannel){
    //disable channel
    switch (indexPair)
    {
        case ASRC_PAIR_A:
            val = CSP_BITFVAL(ASRCTR_ASREA, 0);
            mask = CSP_BITFMASK(ASRCTR_ASREA);
            break;
        case ASRC_PAIR_B:
            val = CSP_BITFVAL(ASRCTR_ASREB, 0);
            mask = CSP_BITFMASK(ASRCTR_ASREB);
            break;            
        case ASRC_PAIR_C:
            val = CSP_BITFVAL(ASRCTR_ASREC, 0);
            mask = CSP_BITFMASK(ASRCTR_ASREC);
            break;
        default:
            break;
    }
    AsrcSetReg(&pReg->ASRCTR, val, mask);    

    }
    
    if(bStopInput)
        DDKSdmaStopChan(pPairContext->inputDMAChan,TRUE);

    if(bStopOutput)
        DDKSdmaStopChan(pPairContext->outputDMAChan,TRUE);

    res = TRUE;
    
//exit:

   return res; 

}



//-----------------------------------------------------------------------------
//
//  Function:  AsrcGetInputBufStatus
//
//  This function check the buffer status for input function (mem->asrc).  
//
//  Parameters:
//      indexPair:  pair index
//      
//  Returns:
//      Returns status.
//
//-----------------------------------------------------------------------------
UINT32 AsrcGetInputBufStatus(ASRC_PAIR_INDEX indexPair)
{
    UINT32 status = 0;
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    UINT32 bufDescStatus[2]; 
    
    EnterCriticalSection(&(pPairContext->lockInput));   

    if (!DDKSdmaGetChainStatus(pPairContext->inputDMAChan, bufDescStatus))
    {
    //    ERRORMSG(ZONE_ERROR,(_T("Could not retrieve output buffer ")
     //                                  _T("status\r\n")));
    }

    if (!(bufDescStatus[ASRC_BUF_A] & DDK_DMA_FLAGS_BUSY)){
        status |= ASRC_BUF_INPUT_DONE_A;
        DDKSdmaClearBufDescStatus(pPairContext->inputDMAChan, ASRC_BUF_A);
    }

    if (!(bufDescStatus[ASRC_BUF_B] & DDK_DMA_FLAGS_BUSY)){
        status |= ASRC_BUF_INPUT_DONE_B;    
        DDKSdmaClearBufDescStatus(pPairContext->inputDMAChan, ASRC_BUF_B);
    }

    LeaveCriticalSection (&(pPairContext->lockInput));
    return status;
}

//-----------------------------------------------------------------------------
//
//  Function:  AsrcGetOutputBufStatus
//
//  This function check the buffer status for output function (asrc->mem).  
//
//  Parameters:
//      indexPair:  pair index
//      
//  Returns:
//      Returns status.
//
//-----------------------------------------------------------------------------

UINT32 AsrcGetOutputBufStatus(ASRC_PAIR_INDEX indexPair)
{
    UINT32 status = 0;
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    UINT32 bufDescStatus[2]; 
    
    EnterCriticalSection(&(pPairContext->lockOutput));   

    
    if (!DDKSdmaGetChainStatus(pPairContext->outputDMAChan, bufDescStatus))
    {
       // ERRORMSG(ZONE_ERROR,(_T("Could not retrieve output buffer ")
        //                               _T("status\r\n")));
    }

    if (!(bufDescStatus[ASRC_BUF_A] & DDK_DMA_FLAGS_BUSY)){
        status |= ASRC_BUF_OUTPUT_DONE_A;
        DDKSdmaClearBufDescStatus(pPairContext->outputDMAChan, ASRC_BUF_A);
    }

    if(pPairContext->transMode != ASRC_TRANS_MEM2ESAI){
        // in p2p mode , we don't use the 2nd buffer
        if (!(bufDescStatus[ASRC_BUF_B] & DDK_DMA_FLAGS_BUSY)){
            status |= ASRC_BUF_OUTPUT_DONE_B;    
            DDKSdmaClearBufDescStatus(pPairContext->outputDMAChan, ASRC_BUF_B);
        }
    }

    LeaveCriticalSection (&(pPairContext->lockOutput));
    
    return status;
}

//-----------------------------------------------------------------------------
//
//  Function:  AsrcPairUnderrun
//
//  This function check if the asrc pair is underrun.  
//
//  Parameters:
//      indexPair:  pair index
//      
//  Returns:
//      Returns TRUE if underrun happens.
//
//-----------------------------------------------------------------------------

BOOL AsrcPairUnderrun(ASRC_PAIR_INDEX indexPair)
{
    PCSP_ASRC_REG pReg =  g_pAsrcContext->pAsrcReg;
    //PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    UINT32 mask = 0;
    UINT32 status = INREG32(&pReg->ASRSTR);

    switch(indexPair){
        case ASRC_PAIR_A:
            mask = CSP_BITFMASK(ASRSTR_AIDUA);
            break;
        case ASRC_PAIR_B:
            mask = CSP_BITFMASK(ASRSTR_AIDUB);
            break; 
        case ASRC_PAIR_C:
            mask = CSP_BITFMASK(ASRSTR_AIDUC);
            break;    
        default:
            break;
    }

    if(status & mask){
        INSREG32(&pReg->ASRSTR,mask,0);
        //RETAILMSG(TRUE,(TEXT("Asrc status reg:%x\r\n"),INREG32(&pReg->ASRSTR)));
        return TRUE;
    }else{
        return FALSE;        
    }   

}


//-----------------------------------------------------------------------------
//
//  Function:  AsrcSetP2PDeviceWML
//
//  This function set device watermark level for p2p fucntion.  
//
//  Parameters:
//      indexPair:  pair index
//      deviceWML:  watermark level
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

BOOL AsrcSetP2PDeviceWML(ASRC_PAIR_INDEX indexPair,UINT32 deviceWML)
{
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    pPairContext->deviceWML = deviceWML;
    return TRUE;
}



BOOL AsrcSuspendConvert(ASRC_PAIR_INDEX indexPair)
{
    //PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    PCSP_ASRC_REG pReg =  g_pAsrcContext->pAsrcReg;
    UINT32 valCSR=0,maskCSR=0;
    BOOL bRet = FALSE;

    //RETAILMSG(1,(TEXT("AsrcSuspendConvert\r\n")));
    
    switch(indexPair){
            case ASRC_PAIR_A:
                maskCSR = CSP_BITFMASK(ASRCSR_AOCSA);
                valCSR = CSP_BITFVAL(ASRCSR_AOCSA, ASRC_OCLK_SRC_RSV1);
                break;
    
            case ASRC_PAIR_B:
                maskCSR = CSP_BITFMASK(ASRCSR_AOCSB);
                valCSR = CSP_BITFVAL(ASRCSR_AOCSB, ASRC_OCLK_SRC_RSV1);
                break;
    
            case ASRC_PAIR_C:
                maskCSR = CSP_BITFMASK(ASRCSR_AOCSC);
                valCSR = CSP_BITFVAL(ASRCSR_AOCSC, ASRC_OCLK_SRC_RSV1);
                break;
    
            default:
                return FALSE;
                break;
    }            
    
    bRet = AsrcSetReg(&pReg->ASRCSR, valCSR, maskCSR);        
        
    return bRet;

}

BOOL AsrcResumeConvert(ASRC_PAIR_INDEX indexPair)
{
    PASRC_PAIR_CONTEXT pPairContext = g_pAsrcContext->pPairArray[indexPair];
    PCSP_ASRC_REG pReg =  g_pAsrcContext->pAsrcReg;
    UINT32 valCSR=0,maskCSR=0;
    BOOL bRet = FALSE;

    //RETAILMSG(1,(TEXT("AsrcResumeConvert\r\n")));

    switch(indexPair){
        case ASRC_PAIR_A:
            maskCSR = CSP_BITFMASK(ASRCSR_AOCSA);
            valCSR = CSP_BITFVAL(ASRCSR_AOCSA, pPairContext->configParam.outClkSrc);
            break;

        case ASRC_PAIR_B:
            maskCSR = CSP_BITFMASK(ASRCSR_AOCSB);
            valCSR = CSP_BITFVAL(ASRCSR_AOCSB, pPairContext->configParam.outClkSrc);
            break;

        case ASRC_PAIR_C:
            maskCSR = CSP_BITFMASK(ASRCSR_AOCSC);
            valCSR = CSP_BITFVAL(ASRCSR_AOCSC, pPairContext->configParam.outClkSrc);
            break;

        default:
            return FALSE;
            break;
    }            

    bRet = AsrcSetReg(&pReg->ASRCSR, valCSR, maskCSR);   

       
    return bRet;
}

BOOL AsrcSupportOddChnNum(void)
{
    return((g_pAsrcContext->dwVersion >= ASRC_VERSION_2)?TRUE:FALSE);
}
