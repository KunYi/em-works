//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  asrc_main.cpp
//
//   This module provides a stream interface for the ASRC
//   driver. 
//
//------------------------------------------------------------------------------

#include "asrc_main.h"


#ifndef DEBUG
#define    DEBUG    // always turn on debug output
#endif  // DEBUG

#ifdef DEBUG

#define DEBUGMASK(bit)  (1 << (bit))

#define MASK_ERROR  DEBUGMASK(0)
#define MASK_WARN   DEBUGMASK(1)
#define MASK_INIT   DEBUGMASK(2)
#define MASK_FUNCTION   DEBUGMASK(3)
#define MASK_IOCTL      DEBUGMASK(4)
#define MASK_DEVICE     DEBUGMASK(5)
#define MASK_ACTIVITY   DEBUGMASK(6)

DBGPARAM dpCurSettings = {
    _T("ASRC"), 
    {
        _T("Errors"), _T("Warnings"), _T("Init"), _T("Function"), 
        _T("Ioctl"), _T("Device"), _T("Activity"), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T(""),_T(""),_T(""),_T("") 
    },
    MASK_ERROR | MASK_WARN | MASK_INIT | MASK_IOCTL | MASK_DEVICE
}; 

#define ZONE_ERROR  DEBUGZONE(0)
#define ZONE_WARN   DEBUGZONE(1)
#define ZONE_INIT   DEBUGZONE(2)
#define ZONE_FUNCTION   DEBUGZONE(3)
#define ZONE_IOCTL  DEBUGZONE(4)
#define ZONE_DEVICE DEBUGZONE(5)
#define ZONE_ACTIVITY   DEBUGZONE(6)

#endif


#define ASRC_FILTER_OFFSET 42 //43
#define ASRC_ENABLE_INIT_AT_START   1


static PASRC_OPEN_CONTEXT g_pAsrcOpenContext[3];   

static const WCHAR c_szAsrcKey[]   = L"Drivers\\BuiltIn\\ASRC";
static const WCHAR c_szPairIndex[] = L"PairIndex";
static const WCHAR c_szMaxChnNum[] = L"MaxChnNum";

static ASRC_PAIR_INDEX g_pairIndex = ASRC_PAIR_A;
static DWORD g_dwMaxChnNum = 2;
static DWORD ARCInputIST(PASRC_OPEN_CONTEXT pOpenContext);

static DWORD ARCOutputIST(PASRC_OPEN_CONTEXT pOpenContext);

static DWORD TransferInputBuf(PASRC_OPEN_CONTEXT pOpenContext, DWORD indexNextBuf)
{

    PASRCHDR pHdr;
    EnterCriticalSection(&(pOpenContext->lockInput));
   
    if ((pOpenContext->pInputBufPos == NULL) || (pOpenContext ->pInputListHead == NULL)){
            //RETAILMSG(TRUE, (_T("!!!TransferInputBuf, no bufffer available\r\n")));   
            LeaveCriticalSection(&(pOpenContext->lockInput));
        return 0;     
    }

    pHdr = pOpenContext->pInputListHead;

    CeSafeCopyMemory((LPVOID)(pOpenContext->pVirtInputBuf[indexNextBuf]),
        (LPCVOID)(pOpenContext->pInputBufPos),ASRC_DMA_INPUT_BUF_SIZE);

    if (indexNextBuf == ASRC_DMA_BUF_A)
        pOpenContext->indexNextInputDMABuf =ASRC_DMA_BUF_B;
    else
        pOpenContext->indexNextInputDMABuf =ASRC_DMA_BUF_A;

    

    pOpenContext->pInputBufPos +=  ASRC_DMA_INPUT_BUF_SIZE;

    
    if (pOpenContext->pInputBufPos >= ((PBYTE)(pHdr->pAsyncBuf) + (pHdr->wavHdr).dwBufferLength) ){

        pOpenContext->pInputListHead  =  pHdr->lpNext;
        if (pOpenContext->pInputListHead == NULL){
                //RETAILMSG(TRUE, (_T("TransferInputBuf, no buffer anymore\r\n")));   
            pOpenContext->pInputListTail = NULL;
            pOpenContext->pInputBufPos =NULL;
        }else{
            pOpenContext->pInputBufPos = (PBYTE)(pOpenContext->pInputListHead)->pAsyncBuf; 
            
        }

        pHdr->dwHeadFlags &= ~ASRC_HDR_INPUT_QUEUED;
        pHdr->dwHeadFlags |= ASRC_HDR_INPUT_PROCESSED;

        CeFreeAsynchronousBuffer(pHdr->pAsyncBuf,
                                                pHdr->pMashalledBuf,
                                                pHdr->wavHdr.dwBufferLength,
                                                //ARG_I_PTR|MARSHAL_FORCE_ALIAS);
                                                ARG_I_PTR);

        CeCloseCallerBuffer(pHdr->pMashalledBuf,
                                       pHdr->wavHdr.lpData,
                                       pHdr->wavHdr.dwBufferLength,
                                       ARG_I_PTR);

        CeFreeAsynchronousBuffer(pHdr,
                                                pHdr->pUser,
                                                sizeof(ASRCHDR),
                                                //ARG_I_PTR|MARSHAL_FORCE_ALIAS);
                                                ARG_I_PTR);
        
        SetEvent(pOpenContext->hUserEvent[0]);
    }

    LeaveCriticalSection(&(pOpenContext->lockInput));
    return ASRC_DMA_INPUT_BUF_SIZE;
}

DWORD TransferOutputBuf(PASRC_OPEN_CONTEXT pOpenContext, DWORD indexNextBuf)
{
    PASRCHDR pHdr;
    FLOAT dwSampleProcessed;

    if ((pOpenContext->pOutputBufPos == NULL) || (pOpenContext ->pOutputListHead == NULL)){
        //RETAILMSG(TRUE, (_T("!!!!TransferOutputBuf, no bufffer available\r\n")));   
        return 0;     

    }

    pHdr = pOpenContext->pOutputListHead;

   
    if ((pOpenContext->pOutputBufPos == NULL) || (pOpenContext ->pOutputListHead == NULL)){
        return 0;     
    }

    memcpy( pOpenContext->pOutputBufPos,(const void *)pOpenContext->pVirtOutputBuf[indexNextBuf], 
        ASRC_DMA_OUTPUT_BUF_SIZE);


    
    if (indexNextBuf == ASRC_DMA_BUF_A)
        pOpenContext->indexNextOutputDMABuf =ASRC_DMA_BUF_B;
    else
        pOpenContext->indexNextOutputDMABuf =ASRC_DMA_BUF_A;

    pHdr = pOpenContext->pOutputListHead;
    pHdr->wavHdr.dwBytesRecorded += ASRC_DMA_OUTPUT_BUF_SIZE;

    pOpenContext->pOutputBufPos +=  ASRC_DMA_OUTPUT_BUF_SIZE;
    
    if (pOpenContext->pOutputBufPos >= ((PBYTE)(pHdr->pAsyncBuf) + (pHdr->wavHdr).dwBufferLength) ){
        pOpenContext->pOutputListHead  =  pHdr->lpNext;
        if (pOpenContext->pOutputListHead == NULL){
            pOpenContext->pOutputListTail = NULL;
            pOpenContext->pOutputBufPos =NULL;
        }else{
            pOpenContext->pOutputBufPos = (PBYTE)(pOpenContext->pOutputListHead)->pAsyncBuf; 
            
        }

        
        pHdr->dwHeadFlags &= ~ASRC_HDR_OUTPUT_QUEUED;
        pHdr->dwHeadFlags |= ASRC_HDR_OUTPUT_READY;

        CeFreeAsynchronousBuffer(pHdr->pAsyncBuf,
                                                pHdr->pMashalledBuf,
                                                pHdr->wavHdr.dwBufferLength,
                                                ARG_O_PTR|MARSHAL_FORCE_ALIAS);

        CeCloseCallerBuffer(pHdr->pMashalledBuf,
                                       pHdr->wavHdr.lpData,
                                       pHdr->wavHdr.dwBufferLength,
                                       ARG_O_PTR);

        CeFreeAsynchronousBuffer(pHdr,
                                                pHdr->pUser,
                                                sizeof(ASRCHDR),
                                                ARG_O_PTR|MARSHAL_FORCE_ALIAS);
        SetEvent(pOpenContext->hUserEvent[1]);
    }

    dwSampleProcessed = (FLOAT)(ASRC_DMA_OUTPUT_BUF_SIZE/(4 * pOpenContext->dwOutputChnNum));
    if (pOpenContext->bNeedInit){

        //FLOAT ratio = (FLOAT)pOpenContext->dwOutputSampleRate/(FLOAT)pOpenContext->dwInputSampleRate;

        // fifi length (64)+40(estimated filter affected len) * ratio* channel* 4byte
        //dwSampleProcessed -= (pOpenContext->dwInitLenth)*ratio;
        dwSampleProcessed -= pOpenContext->dwInitLenth;
         
        pHdr->dwValidOffset = (DWORD)(pOpenContext->dwInitLenth  * pOpenContext->dwOutputChnNum *4);
        pHdr->dwValidOffset -= (pHdr->dwValidOffset % 4); //this val must be 32bit aligned.
        pHdr->wavHdr.dwBytesRecorded -= pHdr->dwValidOffset;
        pOpenContext->bNeedInit = FALSE;

    }
    
    pOpenContext->dwOutputSampleNum += dwSampleProcessed;

    dwSampleProcessed = (dwSampleProcessed * pOpenContext->dwInputSampleRate )/ pOpenContext->dwOutputSampleRate;

    EnterCriticalSection(&(pOpenContext->lockContext));
    if(pOpenContext->dwUnProcessedSample > dwSampleProcessed){
        pOpenContext->dwUnProcessedSample -= dwSampleProcessed;
    }else{
        pOpenContext->dwUnProcessedSample = 0;
    }
    LeaveCriticalSection(&(pOpenContext->lockContext));

    return ASRC_DMA_OUTPUT_BUF_SIZE;
}


VOID ReleaseAsrcOpenContext(ASRC_PAIR_INDEX pairIndex)
{
    PASRC_OPEN_CONTEXT pOpenContext;
    PHYSICAL_ADDRESS phyAddr;
    DMA_ADAPTER_OBJECT Adapter;
    
    if (g_pAsrcOpenContext[pairIndex]  != NULL){
        pOpenContext = g_pAsrcOpenContext[pairIndex];

        if (pOpenContext-> hInputIST != NULL){
            pOpenContext->bExitInputIST = TRUE;
            SetEvent(pOpenContext-> hInputEvent);
            SetEvent(pOpenContext-> hTriggerInputEvent );
            CloseHandle(pOpenContext-> hInputIST );
        }

        if (pOpenContext-> hInputEvent != NULL)
            CloseHandle(pOpenContext-> hInputEvent);
        
        if (pOpenContext-> hTriggerInputEvent != NULL)
            CloseHandle(pOpenContext-> hTriggerInputEvent );    

        if (pOpenContext-> hOutputIST != NULL){
            pOpenContext->bExitOutputIST = TRUE;    
            SetEvent(pOpenContext-> hOutputEvent);
            SetEvent(pOpenContext-> hTriggerOutputEvent );
            CloseHandle(pOpenContext-> hOutputIST);
        }

        if (pOpenContext-> hOutputEvent != NULL)
            CloseHandle(pOpenContext-> hInputEvent);
        
        if (pOpenContext-> hTriggerOutputEvent != NULL)
            CloseHandle(pOpenContext-> hTriggerOutputEvent );    

        // Logical address parameter is ignored
        phyAddr.QuadPart = 0;
        if (pOpenContext->pVirtInputBuf[0] != NULL){
            memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
            Adapter.InterfaceType = Internal;
            Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
            
            HalFreeCommonBuffer(&Adapter, 0, phyAddr, pOpenContext->pVirtInputBuf[0], FALSE);
        }

        DeleteCriticalSection(&(pOpenContext->lockInput));
        DeleteCriticalSection(&(pOpenContext->lockOutput));
    
        LocalFree((HLOCAL) g_pAsrcOpenContext[pairIndex]);
        g_pAsrcOpenContext[pairIndex] = NULL;
    }

    return;
}


DWORD InitAsrcOpenContext(ASRC_PAIR_INDEX pairIndex)
{
    PASRC_OPEN_CONTEXT pOpenContext;
    DMA_ADAPTER_OBJECT Adapter;

    if (g_pAsrcOpenContext[pairIndex]  != NULL)
        return (DWORD)g_pAsrcOpenContext[pairIndex];
    
    pOpenContext= (PASRC_OPEN_CONTEXT)LocalAlloc(LPTR, sizeof(ASRC_OPEN_CONTEXT));
    if (pOpenContext == NULL)
        return 0;
    
    pOpenContext->pairIndex = pairIndex; //other val should be zero
    
    pOpenContext->hInputEvent = CreateEvent(0,FALSE,FALSE,0);
    if (pOpenContext->hInputEvent == NULL){
        goto exitinit;
    }
    
    pOpenContext->hOutputEvent = CreateEvent(0,FALSE,FALSE,0);
    if (pOpenContext->hOutputEvent == NULL){
        goto exitinit;
    }
    
    pOpenContext->hTriggerInputEvent = CreateEvent(0,FALSE,FALSE,0);
    if (pOpenContext->hTriggerInputEvent == NULL){
        goto exitinit;
    }
    
    pOpenContext->hTriggerOutputEvent = CreateEvent(0,FALSE,FALSE,0);
    if (pOpenContext->hTriggerOutputEvent == NULL){
        goto exitinit;
    }

    pOpenContext->bExitInputIST = FALSE;
    pOpenContext->bExitOutputIST = FALSE;
  
    pOpenContext->hInputIST = CreateThread(NULL,
                                                0,
                                                (LPTHREAD_START_ROUTINE)ARCInputIST,
                                                (LPVOID)pOpenContext,
                                                 0,
                                                 &(pOpenContext->dwInputThreadID));
    if (pOpenContext->hInputIST == NULL){
        goto exitinit;
    }
    
    pOpenContext->hOutputIST = CreateThread(NULL,
                                                 0,
                                                 (LPTHREAD_START_ROUTINE)ARCOutputIST,
                                                 (LPVOID)pOpenContext,
                                                 0,
                                                 &(pOpenContext->dwOutputThreadID));
    if (pOpenContext->hOutputIST == NULL){
        goto exitinit;
    }    
   
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
    //
    pOpenContext->pVirtInputBuf[0] = (PBYTE)HalAllocateCommonBuffer(&Adapter, 
    (ASRC_DMA_INPUT_BUF_SIZE *2 + ASRC_DMA_OUTPUT_BUF_SIZE*2), 
    &(pOpenContext->phyAddrInputBuf[0]), FALSE);

    if (pOpenContext->pVirtInputBuf[0]  == NULL){
        goto exitinit;
    }    

    memset((void *)pOpenContext->pVirtInputBuf[0],0,
        (ASRC_DMA_INPUT_BUF_SIZE *2 + ASRC_DMA_OUTPUT_BUF_SIZE*2));
     
    pOpenContext->pVirtInputBuf[1]  =  pOpenContext->pVirtInputBuf[0] + ASRC_DMA_INPUT_BUF_SIZE;
    pOpenContext->pVirtOutputBuf[0] = pOpenContext->pVirtInputBuf[0] + 2*ASRC_DMA_INPUT_BUF_SIZE;
    pOpenContext->pVirtOutputBuf[1] = pOpenContext->pVirtOutputBuf[0] + ASRC_DMA_OUTPUT_BUF_SIZE;

    (pOpenContext->phyAddrInputBuf[1]).LowPart = (pOpenContext->phyAddrInputBuf[0]).LowPart +  ASRC_DMA_INPUT_BUF_SIZE ;
    (pOpenContext->phyAddrOutputBuf[0]).LowPart = (pOpenContext->phyAddrInputBuf[0]).LowPart +  2*ASRC_DMA_INPUT_BUF_SIZE ;
    (pOpenContext->phyAddrOutputBuf[1]).LowPart = (pOpenContext->phyAddrOutputBuf[0]).LowPart +  ASRC_DMA_OUTPUT_BUF_SIZE ;
    
    InitializeCriticalSection(&(pOpenContext->lockInput));
    InitializeCriticalSection(&(pOpenContext->lockOutput));

    g_pAsrcOpenContext[pairIndex] = pOpenContext;

    return (DWORD)pOpenContext;

exitinit:
    
    ReleaseAsrcOpenContext(pairIndex);
    return 0;
}

BOOL OpenAsrcPair(ASRC_PAIR_INDEX pairIndex,PASRC_OPEN_PARAM pOpenParam)
{
    PASRC_OPEN_CONTEXT pOpenContext = g_pAsrcOpenContext[pairIndex];

    if (pOpenContext == NULL)
        return FALSE;
    
    if (pOpenContext->bOpened == TRUE){
        return FALSE;
    }
            
    if ( AsrcOpenPair(pairIndex, pOpenParam->inputChnNum, pOpenParam->outputChnNum,
                pOpenContext->hInputEvent, pOpenContext->hOutputEvent,
                ASRC_TRANS_MEM2MEM) != pairIndex ){
        return FALSE;
    }else {
        pOpenContext->bOpened = TRUE;

        pOpenContext->dwInputChnNum = pOpenParam->inputChnNum;
        pOpenContext->dwOutputChnNum = pOpenParam->outputChnNum;
        pOpenContext->dwUnProcessedSample = 0;
        pOpenContext->dwInputSampleNum = 0;
        pOpenContext->dwOutputSampleNum = 0;

         if (!DuplicateHandle(GetOwnerProcess(),
                pOpenParam->hEventInputDone,
                GetCurrentProcess(),
                &(pOpenContext->hUserEvent[0]),  
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS)){
                        RETAILMSG(TRUE, (_T("Duplicate handle1 failed")));

            }

          if (!DuplicateHandle(GetOwnerProcess(),
                pOpenParam->hEventOutputDone,
                GetCurrentProcess(),
                &(pOpenContext->hUserEvent[1]),  
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS)){
                        RETAILMSG(TRUE, (_T("Duplicate handle2 failed")));

            }
        
          
        SetEvent( pOpenContext->hTriggerInputEvent);
        SetEvent( pOpenContext->hTriggerOutputEvent);

        AsrcSetInputBuf(pairIndex, pOpenContext->phyAddrInputBuf[0], 
            ASRC_DMA_INPUT_BUF_SIZE, 0, pOpenContext->pVirtInputBuf[0]);
        AsrcSetInputBuf(pairIndex, pOpenContext->phyAddrInputBuf[1], 
            ASRC_DMA_INPUT_BUF_SIZE, 1,pOpenContext->pVirtInputBuf[1]);
        AsrcSetOutputBuf(pairIndex, pOpenContext->phyAddrOutputBuf[0], 
            ASRC_DMA_OUTPUT_BUF_SIZE, 0,pOpenContext->pVirtOutputBuf[0]);
        AsrcSetOutputBuf(pairIndex, pOpenContext->phyAddrOutputBuf[1], 
            ASRC_DMA_OUTPUT_BUF_SIZE, 1,pOpenContext->pVirtOutputBuf[1]);
    }
    return TRUE;
}

VOID CloseAsrcPair(ASRC_PAIR_INDEX pairIndex)
{
    PASRC_OPEN_CONTEXT pOpenContext = g_pAsrcOpenContext[pairIndex];
    AsrcClosePair(pairIndex);
    pOpenContext->bOpened = FALSE;
    SetEvent( pOpenContext->hTriggerInputEvent);
    SetEvent( pOpenContext->hTriggerOutputEvent);
    return;
}

DWORD ARC_Init(PVOID Context)
{
    UINT32 pAsrcContext;
    int i;
    HKEY hKey = NULL;   

    UNREFERENCED_PARAMETER(Context);

    if (!AsrcInit(&pAsrcContext)){
        RETAILMSG(TRUE, (_T("ASRC Init failed")));
        return 0;
    }

    for(i=0;i<3;i++){
        g_pAsrcOpenContext[i]= NULL;
        }
 
    DWORD dwSize = sizeof(DWORD);
    DWORD dwVal;
    // Allow for registry override of gamma value
    if (RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        c_szAsrcKey,
        0,
        0,
        &hKey
        ) == ERROR_SUCCESS)
    {
        

        RegQueryValueEx(
            hKey,
            c_szPairIndex,
            NULL,
            NULL,
            (LPBYTE)&dwVal,
            &dwSize
        );


        if((dwVal < 0) || (dwVal > 2)){
            g_pairIndex = ASRC_PAIR_A;        
        }else{
            g_pairIndex = (ASRC_PAIR_INDEX)dwVal;
        }

        RegQueryValueEx(
            hKey,
            c_szMaxChnNum,
            NULL,
            NULL,
            (LPBYTE)&dwVal,
            &dwSize
        );

        if((dwVal >= 2) && (dwVal <= 10)){
            g_dwMaxChnNum = dwVal;        
        }else{
            g_dwMaxChnNum = 2;  //default for stereo function
        }
        
        RegCloseKey(hKey);
    }
    return pAsrcContext;
}


BOOL ARC_Deinit(DWORD dwContext )
{
    UNREFERENCED_PARAMETER(dwContext);
    
    AsrcDeinit();
    return TRUE;
}

DWORD ARC_Open(DWORD Context, DWORD Access,  DWORD ShareMode)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Access);
    UNREFERENCED_PARAMETER(ShareMode);
    
    return 1 ;
}


BOOL ARC_Close(DWORD Context ) 
{
   UNREFERENCED_PARAMETER(Context);    
   return TRUE;
}

VOID ARC_PowerDown(DWORD dwContext )
{
    UNREFERENCED_PARAMETER(dwContext);
    return;
}


VOID ARC_PowerUp(DWORD dwContext )
{
    UNREFERENCED_PARAMETER(dwContext);
    return;
}


DWORD ARC_Read(DWORD  dwContext, LPVOID pBuf, DWORD  Len ) 
{
    UNREFERENCED_PARAMETER(dwContext);
    UNREFERENCED_PARAMETER(pBuf);
    UNREFERENCED_PARAMETER(Len);
    
    DEBUGMSG(ZONE_ERROR | ZONE_FUNCTION,(_T("PDX_Read\r\n")));
    SetLastError(ERROR_INVALID_FUNCTION);
    return  0;
}

DWORD ARC_Write( DWORD  dwContext, LPVOID pBuf, DWORD  Len  ) 
{
    UNREFERENCED_PARAMETER(dwContext);
    UNREFERENCED_PARAMETER(pBuf);
    UNREFERENCED_PARAMETER(Len);
    
    DEBUGMSG(ZONE_ERROR | ZONE_FUNCTION,(_T("PDX_Read\r\n")));
    SetLastError(ERROR_INVALID_FUNCTION);
    return  0;
}

ULONG ARC_Seek( PVOID Context, LONG Position, DWORD Type  )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Position);
    UNREFERENCED_PARAMETER(Type);
    
    return (DWORD)-1;
}

BOOL ARC_IOControl( DWORD  dwContext, DWORD  dwCode, PUCHAR pBufIn,
    DWORD inBufLen,  PUCHAR pBufOut, DWORD  outBufLen, 
    PDWORD  pdwBytesTransferred)
{

    UNREFERENCED_PARAMETER(pdwBytesTransferred);
    UNREFERENCED_PARAMETER(dwContext);
    
    
    BOOL bRet = TRUE;

    ASRC_PAIR_INDEX pairIndex = ASRC_PAIR_NONE;// now we only deal with pair a for default;
    PASRC_OPEN_CONTEXT pOpenContxt;
    PASRC_OPEN_PARAM pOpenParam;
    PASRC_CONFIG_PARAM pConfigParam;
    PASRC_CAP_PARAM pCapParam;
    ASRCHDR *pHdr;
    DWORD *pWordArray;
    DWORD *pPairIndex = NULL;

#ifdef ASRC_ENABLE_INIT_AT_START 
    FLOAT ratio = 0;
#endif

    switch (dwCode){
        case ASRC_IOCTL_REQUEST_PAIR:
            if ((pBufOut== NULL) || (outBufLen <sizeof(DWORD)))
                return FALSE;

            pairIndex = g_pairIndex;
            
            
            pWordArray = (DWORD*)pBufOut;

            if( g_pAsrcOpenContext[pairIndex] != NULL){
                *pWordArray = (DWORD)ASRC_PAIR_NONE;
                bRet = TRUE;
                break;

            }
            //NOW WE JUST RETURN PAIR A
            if (InitAsrcOpenContext(pairIndex) ==0){
                *pWordArray = (DWORD)ASRC_PAIR_NONE;
                bRet = TRUE;
                break;
            }

            
            *pWordArray = pairIndex;
                     
            bRet = TRUE;
            break;

       case ASRC_IOCTL_RELEASE_PAIR:  
             if ((pBufIn== NULL) || (inBufLen <sizeof(DWORD)))
                return FALSE; 

             pWordArray = (DWORD*)pBufIn;
             pairIndex = (ASRC_PAIR_INDEX)*pWordArray;

             if ((pairIndex < ASRC_PAIR_A) || (pairIndex > ASRC_PAIR_C))
                return FALSE;

             ReleaseAsrcOpenContext(pairIndex);
             
             bRet = TRUE;
             break;
            
        
        case ASRC_IOCTL_OPEN_PAIR :
            if ((pBufIn == NULL) || (inBufLen < sizeof(PASRC_OPEN_PARAM))){
                bRet =FALSE;
                break;
            }
            pOpenParam = (PASRC_OPEN_PARAM)pBufIn;
            pairIndex = pOpenParam->pairIndex;
            
            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_OPEN_PAIR, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }

            if(((pOpenParam->inputChnNum % 2 != 0) && (!AsrcSupportOddChnNum())) || 
               (pOpenParam->inputChnNum <= 0) || 
               (pOpenParam->inputChnNum > g_dwMaxChnNum) || 
               (pOpenParam->outputChnNum != pOpenParam->inputChnNum)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_OPEN_PAIR, Invalid ChnNum(%d)(%d)\r\n"),
                    pOpenParam->inputChnNum,pOpenParam->outputChnNum));
                bRet = FALSE;
                break;
            }
             

            bRet = OpenAsrcPair(pairIndex, pOpenParam);
            break;

       
        case ASRC_IOCTL_CLOSE_PAIR:
            if ((pBufIn == NULL) || (inBufLen < sizeof(DWORD))){
                bRet =FALSE;
                break;
            }
            pPairIndex = (DWORD*)pBufIn;
            pairIndex = (ASRC_PAIR_INDEX)*pPairIndex;
            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_CLOSE_PAIR, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }
            CloseAsrcPair(pairIndex);

            bRet = TRUE;
            break;

        case ASRC_IOCTL_SET_CONFIG:
            if ((pBufIn == NULL) || (inBufLen < sizeof(ASRC_CONFIG_PARAM ))){
                bRet =FALSE;
                break;
            }
            
            pConfigParam = (PASRC_CONFIG_PARAM)pBufIn;
            //pairIndex = pConfigParam->
            //jeremy : add pair index to config param structure!!!!
            pairIndex = pConfigParam->pairIndex;

            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_SET_CONFIG, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }
            pOpenContxt = g_pAsrcOpenContext[pairIndex];
            pOpenContxt->dwInputSampleRate = pConfigParam->inputSampleRate;
            pOpenContxt->dwOutputSampleRate = pConfigParam->outputSampleRate;

            
            bRet = AsrcConfigPair(pairIndex, pConfigParam);
            break;

        case ASRC_IOCTL_START_CONVERT:
            if ((pBufIn == NULL) || (inBufLen < sizeof(DWORD))){
                bRet =FALSE;
                break;
            }
            pPairIndex = (DWORD*)pBufIn;
            pairIndex = (ASRC_PAIR_INDEX)*pPairIndex;

            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_START_CONVERT, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }

            pOpenContxt = g_pAsrcOpenContext[pairIndex];
            if (pOpenContxt->bAsrcRunning ==  TRUE){
                bRet = TRUE;
                break;
            }

            pOpenContxt->bAsrcRunning = TRUE;
            pOpenContxt->bAsrcSuspend = FALSE;
            pOpenContxt->bInputDMARunning = TRUE;
            pOpenContxt->bOutputDMARunning = TRUE;
            pOpenContxt->bInputBufUnderrun = FALSE;
            pOpenContxt->bOutputBufUnderrun = FALSE;
            pOpenContxt->bNeedInit = FALSE;

#ifdef ASRC_ENABLE_INIT_AT_START          
            ratio = (FLOAT)pOpenContxt->dwOutputSampleRate/(FLOAT)pOpenContxt->dwInputSampleRate;
            pOpenContxt->bNeedInit = TRUE;
            pOpenContxt->dwInitLenth = ASRC_FILTER_OFFSET; //41

            if(ratio>1) 
                pOpenContxt->dwInitLenth += 15;//56

            if(ratio >1.5)
                pOpenContxt->dwInitLenth += 18; //74

            if(ratio >2)
                pOpenContxt->dwInitLenth += 34;//108

            if(ratio >5)
                pOpenContxt->dwInitLenth += 220;//328

            if(ratio < 1)
                pOpenContxt->dwInitLenth += 18;//64

            if(ratio <=0.5)
                pOpenContxt->dwInitLenth += 82;//146    
#endif            
            TransferInputBuf(pOpenContxt, ASRC_DMA_BUF_A);
            TransferInputBuf(pOpenContxt, ASRC_DMA_BUF_B);
            pOpenContxt->indexNextInputDMABuf = ASRC_DMA_BUF_A;

            bRet = AsrcStartConv(pairIndex,TRUE,TRUE,TRUE);
            
            break;

        case ASRC_IOCTL_STOP_CONVERT:
            if ((pBufIn == NULL) || (inBufLen < sizeof(DWORD))){
                bRet =FALSE;
                break;
            }
            pPairIndex = (DWORD*)pBufIn;
            pairIndex = (ASRC_PAIR_INDEX)*pPairIndex;

            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_STOP_CONVERT, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }

            pOpenContxt = g_pAsrcOpenContext[pairIndex];
            pOpenContxt->bAsrcRunning = FALSE;
            pOpenContxt->bInputDMARunning = FALSE;
            pOpenContxt->bOutputDMARunning = FALSE;
            bRet = AsrcStopConv(pairIndex,TRUE,TRUE,TRUE);
            break;
#if 0
        case ASRC_IOCTL_PREPARE_INPUT_BUFFER:
            pHdr = (PASRCHDR) pBufIn;

            if (CeOpenCallerBuffer( &(pHdr->pMashalledBuf),
                (PVOID)((pHdr->wavHdr).lpData),
                pHdr->wavHdr.dwBufferLength,
                ARG_I_PTR,
                FALSE) != S_OK){
                bRet = FALSE;
                break;
            }
            bRet = TRUE;
            
            break;

        case ASRC_IOCTL_UNPREPARE_INPUT_BUFFER:
            pHdr = (PASRCHDR) pBufIn;

            if (CeCloseCallerBuffer((PVOID)pHdr->pMashalledBuf,
                (PVOID)((pHdr->wavHdr).lpData),
                pHdr->wavHdr.dwBufferLength,
                ARG_I_PTR) != S_OK){
                    bRet = FALSE;
            }else{
                    bRet = TRUE;
            }
            break;

        case ASRC_IOCTL_PREPARE_OUTPUT_BUFFER:
            pHdr = (PASRCHDR) pBufIn;
            
            if (CeOpenCallerBuffer( &(pHdr->pMashalledBuf),
                (PVOID)((pHdr->wavHdr).lpData),
                pHdr->wavHdr.dwBufferLength,
                ARG_O_PTR,
                FALSE) != S_OK){
                bRet = FALSE;
                break;
            }
            bRet = TRUE;

            break;

        case ASRC_IOCTL_UNPREPARE_OUTPUT_BUFFER:
            pHdr = (PASRCHDR) pBufIn;

            if (CeCloseCallerBuffer((PVOID)pHdr->pMashalledBuf,
                (PVOID)((pHdr->wavHdr).lpData),
                pHdr->wavHdr.dwBufferLength,
                ARG_O_PTR) != S_OK){
                    bRet = FALSE;
            }else{
                    bRet = TRUE;
            }
            break; 
#endif            

        case ASRC_IOCTL_ADD_INPUT_BUFFER:
            pHdr = (PASRCHDR) pBufIn;

            pairIndex =(ASRC_PAIR_INDEX) pHdr->dwPairIndex;
            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_ADD_INPUT_BUFFER, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }

            if((pHdr->wavHdr.dwBufferLength % (ASRC_DMA_INPUT_BUF_SIZE * 2)) != 0){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_ADD_OUTPUT_BUFFER, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }

            pOpenContxt = g_pAsrcOpenContext[pairIndex];
            CeAllocAsynchronousBuffer( (PVOID*)&pHdr,
                    pBufIn,
                    inBufLen,
                    ARG_IO_PTR);
                    //ARG_IO_PTR|MARSHAL_FORCE_ALIAS);
            pHdr->pUser = (PVOID)pBufIn;
                    

            if (CeOpenCallerBuffer( &(pHdr->pMashalledBuf),
                (PVOID)((pHdr->wavHdr).lpData),
                pHdr->wavHdr.dwBufferLength,
                ARG_I_PTR,
                FALSE) != S_OK){
                bRet = FALSE;
                break;
                }

            if (CeAllocAsynchronousBuffer( &(pHdr->pAsyncBuf),
                    pHdr->pMashalledBuf,
                    pHdr->wavHdr.dwBufferLength,
                    ARG_I_PTR) != S_OK){
                    //ARG_I_PTR|MARSHAL_FORCE_ALIAS) != S_OK){

                CeCloseCallerBuffer(pHdr->pMashalledBuf,
                    (PVOID)((pHdr->wavHdr).lpData),   
                    pHdr->wavHdr.dwBufferLength,
                    ARG_I_PTR);
                    
                bRet = FALSE;
                break;  

                }

            EnterCriticalSection(&(pOpenContxt->lockInput));
            
            if (pOpenContxt->pInputListHead == NULL){
                pOpenContxt->pInputListHead= pHdr;
                pOpenContxt->pInputListTail= pHdr;
                pOpenContxt->pInputBufPos = (PBYTE)pHdr->pAsyncBuf;
                pHdr->lpNext = NULL;
                //RETAILMSG(TRUE, (_T("IOCTL ADD INPUT BUF NEW HEAR: head(%x) tail(%x)poz(%x)mash(%x)\r\n"),
                //    pOpenContxt->pInputListHead,pOpenContxt->pInputListTail,pOpenContxt->pInputBufPos,
                //    pHdr->pAsyncBuf));
                //restart dma is it stops because buffer under-run
                if((pOpenContxt->bAsrcRunning) && (!pOpenContxt->bInputDMARunning)){
                    pOpenContxt->bInputDMARunning = TRUE;
                    AsrcStartConv(pairIndex,FALSE,TRUE,FALSE);

                    }
                
            }else {
                //dwTmp = *((DWORD*)(pOpenContxt->pInputListTail->pMashalledBuf));
                //pTmp = pOpenContxt->pInputListTail;
             
                pHdr->lpNext = NULL;
                pOpenContxt->pInputListTail->lpNext =  pHdr;
                pOpenContxt->pInputListTail = pHdr;
                //RETAILMSG(TRUE, (_T("IOCTL ADD INPUT BUF: cur poz(%x)\r\n"),pOpenContxt->pInputBufPos));
            }
            pHdr->dwHeadFlags |= ASRC_HDR_INPUT_QUEUED;
            pHdr->dwHeadFlags &= ~ASRC_HDR_INPUT_PROCESSED;

            LeaveCriticalSection(&(pOpenContxt->lockInput));

            EnterCriticalSection(&(pOpenContxt->lockContext));

            pOpenContxt->dwUnProcessedSample += 
                (pHdr->wavHdr.dwBufferLength)/(4 * pOpenContxt->dwInputChnNum); 

            pOpenContxt->dwInputSampleNum += (pHdr->wavHdr.dwBufferLength)/(4 * pOpenContxt->dwInputChnNum); 

            LeaveCriticalSection(&(pOpenContxt->lockContext));

            break;

       case ASRC_IOCTL_ADD_OUTPUT_BUFFER:
            CeAllocAsynchronousBuffer( (PVOID*)&pHdr,
                    pBufIn,
                    inBufLen,
                    ARG_IO_PTR|MARSHAL_FORCE_ALIAS);
            
            pHdr->pUser = (PVOID)pBufIn;
            pairIndex = (ASRC_PAIR_INDEX)pHdr->dwPairIndex;
            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_ADD_OUTPUT_BUFFER, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }
            if((pHdr->wavHdr.dwBufferLength % (ASRC_DMA_OUTPUT_BUF_SIZE * 2)) != 0){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_ADD_OUTPUT_BUFFER, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }

            

            pOpenContxt = g_pAsrcOpenContext[pairIndex];
            

           if (CeOpenCallerBuffer( &(pHdr->pMashalledBuf),
                (PVOID)((pHdr->wavHdr).lpData),
                pHdr->wavHdr.dwBufferLength,
                ARG_O_PTR,
                FALSE) != S_OK){
                bRet = FALSE;
                break;
                }

            if (CeAllocAsynchronousBuffer( &(pHdr->pAsyncBuf),
                    pHdr->pMashalledBuf,
                    pHdr->wavHdr.dwBufferLength,
                    ARG_O_PTR|MARSHAL_FORCE_ALIAS) != S_OK){

                CeCloseCallerBuffer(pHdr->pMashalledBuf,
                    (PVOID)((pHdr->wavHdr).lpData),   
                    pHdr->wavHdr.dwBufferLength,
                    ARG_O_PTR);
                    
                bRet = FALSE;
                break;  

                }

            pHdr->wavHdr.dwBytesRecorded = 0;
            pHdr->dwValidOffset = 0;

            EnterCriticalSection(&(pOpenContxt->lockOutput));

            if (pOpenContxt->pOutputListHead == NULL){
                pOpenContxt->pOutputListHead= pHdr;
                pOpenContxt->pOutputListTail= pHdr;   
                pOpenContxt->pOutputBufPos = (PBYTE)pHdr->pAsyncBuf;
                pHdr->lpNext = NULL;
                //RETAILMSG(TRUE, (_T("IOCTL ADD OUTPUT BUF NEW HEAR: head(%x) tail(%x)poz(%x)mash(%x)\r\n"),
                 //   pOpenContxt->pOutputListHead,pOpenContxt->pOutputListTail,pOpenContxt->pOutputBufPos,
                  //  pHdr->pAsyncBuf));  

                //restart dma is it stops because buffer under-run
                if((pOpenContxt->bAsrcRunning) && (!pOpenContxt->bOutputDMARunning)){
                    pOpenContxt->bOutputDMARunning = TRUE;
                    AsrcStartConv(pairIndex,FALSE,FALSE,TRUE);
                    //RETAILMSG(1,(_T("Restart output dma as overrun\r\n")));
                }
            }else {
                pOpenContxt->pOutputListTail->lpNext =  pHdr;
                pOpenContxt->pOutputListTail = pHdr;
                pHdr->lpNext = NULL;
            }
            pHdr->dwHeadFlags |= ASRC_HDR_OUTPUT_QUEUED;
            pHdr->dwHeadFlags &= ~ASRC_HDR_OUTPUT_READY;
            
            LeaveCriticalSection(&(pOpenContxt->lockOutput));

            break;

        case ASRC_IOCTL_RESET_PAIR:

            break;

        case ASRC_IOCTL_GET_CAPALITY:
            if ((pBufOut== NULL) || (outBufLen <sizeof(ASRC_CAP_PARAM)))
                return FALSE;
            pCapParam = (PASRC_CAP_PARAM)pBufOut;

            pCapParam->bSupportMultiPair = FALSE;
            pCapParam->bSupportOddChnNum = AsrcSupportOddChnNum();
            pCapParam->dwInputBlockSize = ASRC_DMA_INPUT_BUF_SIZE * 2;
            pCapParam->dwMaxChnNum = g_dwMaxChnNum;
            pCapParam->dwMaxInputSampleRate = ASRC_MAX_INPUT_SAMPLE_RATE;
            pCapParam->dwMaxOutputSampleRate = ASRC_MAX_OUTPUT_SAMPLE_RATE;
            pCapParam->dwMinInputSampleRate = ASRC_MIN_INPUT_SAMPLE_RATE;
            pCapParam->dwMinOutputSampleRate = ASRC_MIN_OUTPUT_SAMPLE_RATE;
            pCapParam->dwOutputBlockSize = ASRC_DMA_OUTPUT_BUF_SIZE * 2;
            pCapParam->pairIndexDefault = g_pairIndex;
            
            break;

        case ASRC_IOCTL_SUSPEND_CONVERT:
            if ((pBufIn == NULL) || (inBufLen < sizeof(DWORD))){
                bRet =FALSE;
                break;
            }
            pPairIndex = (DWORD*)pBufIn;
            pairIndex = (ASRC_PAIR_INDEX)*pPairIndex;

            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_SUSPEND_CONVERT, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }

            pOpenContxt = g_pAsrcOpenContext[pairIndex];
            if (pOpenContxt->bAsrcSuspend ==  TRUE){
                bRet = TRUE;
                break;
            }

            bRet = AsrcSuspendConvert(pairIndex);
            if (bRet == TRUE)
                pOpenContxt->bAsrcSuspend = TRUE;

            break;

        case ASRC_IOCTL_RESUME_CONVERT:
            if ((pBufIn == NULL) || (inBufLen < sizeof(DWORD))){
                bRet =FALSE;
                break;
            }
            pPairIndex = (DWORD*)pBufIn;
            pairIndex = (ASRC_PAIR_INDEX)*pPairIndex;

            if((pairIndex<ASRC_PAIR_A) || (pairIndex>ASRC_PAIR_C)){
                RETAILMSG(1,(TEXT("ASRC_IOCTL_RESUME_CONVERT, Invalid Pairindex(%d)\r\n"),
                    (DWORD)pairIndex));
                bRet = FALSE;
                break;
            }

            pOpenContxt = g_pAsrcOpenContext[pairIndex];
            if (pOpenContxt->bAsrcSuspend ==  FALSE){
                bRet = TRUE;
                break;
            }

            bRet = AsrcResumeConvert(pairIndex);
            if (bRet == TRUE)
                pOpenContxt->bAsrcSuspend = FALSE;

            break;    
            

        default:
            break;
            
    }

    return bRet;
}


static DWORD ARCInputIST(PASRC_OPEN_CONTEXT pOpenContext)
{
    UINT32 status = 0;
    
    if (!CeSetThreadPriority(GetCurrentThread(), 142)){
        //RETAILMSG(TRUE, (_T("input ist set priority failed \r\n")));
    }

    while (!pOpenContext->bExitInputIST)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(pOpenContext->hTriggerInputEvent, INFINITE);

        if (pOpenContext->bExitInputIST)
        {
            break;
        }




        while (pOpenContext->bOpened){
            WaitForSingleObject(pOpenContext->hInputEvent, INFINITE);
            
            if (pOpenContext->bExitInputIST)
            {
                //RETAILMSG(TRUE, (_T("ARCInputIST , flags to exit \r\n")));  
                break;
            }

            if (!pOpenContext->bOpened)
            {
                //RETAILMSG(TRUE, (_T("ARCInputIST context not open any more \r\n")));  
                break;
            }

            status = AsrcGetInputBufStatus(pOpenContext->pairIndex);
            
            if (status & ASRC_BUF_INPUT_DONE_A){
                if (pOpenContext->indexNextInputDMABuf == ASRC_DMA_BUF_A){
                     if(TransferInputBuf(pOpenContext,ASRC_DMA_BUF_A) == 0){
                     pOpenContext->bInputBufUnderrun = TRUE;

                    }
                }
            }

            if (status & ASRC_BUF_INPUT_DONE_B){

                if ((pOpenContext->bInputBufUnderrun) 
                     && (pOpenContext->indexNextInputDMABuf == ASRC_DMA_BUF_A)){
                   //RETAILMSG(TRUE, (_T("Input buffer a underrun detected \r\n")));
                   pOpenContext->bInputBufUnderrun = FALSE;

                   if (TransferInputBuf(pOpenContext,ASRC_DMA_BUF_A) == 0){
                       //RETAILMSG(TRUE, (_T("still no input buffer ,stop input dma \r\n")));
                       AsrcStopConv(pOpenContext->pairIndex,FALSE,TRUE,FALSE);
                       pOpenContext->bInputDMARunning = FALSE;

                   }
                    

                }

                if ((pOpenContext->indexNextInputDMABuf == ASRC_DMA_BUF_B) ){
                    //If buffer A is transferred, b can surely be transfered , as the buffer size requirement
                    TransferInputBuf(pOpenContext,ASRC_DMA_BUF_B);
                     
                    AsrcStartConv(pOpenContext->pairIndex,FALSE,TRUE,FALSE);   
                        
                    //need to handle the case, the buffer is added by user during buf b is transfering.
                    // in this case need to transfer buffer a, bufferb and, start dma agian.
                }
            }
        }//end of while (pOpenContext->bOpened)

    }



    return 1;

}

static DWORD ARCOutputIST(PASRC_OPEN_CONTEXT pOpenContext)
{
    UINT32 status;
    
    if (!CeSetThreadPriority(GetCurrentThread(), 142)){
        //RETAILMSG(TRUE, (_T("output ist set priority failed \r\n")));
    }

    while (!pOpenContext->bExitOutputIST)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(pOpenContext->hTriggerOutputEvent, INFINITE);

        if (pOpenContext->bExitOutputIST)
        {
            break;
        }

        while (pOpenContext->bOpened){
            
            WaitForSingleObject(pOpenContext->hOutputEvent, INFINITE);

            if (pOpenContext->bExitOutputIST)
            {
                break;
            }

            if (!pOpenContext->bOpened)
            {
                break;
            }

            status = AsrcGetOutputBufStatus(pOpenContext->pairIndex);

            EnterCriticalSection(&(pOpenContext->lockOutput));

            if (status & ASRC_BUF_OUTPUT_DONE_A){
                if (pOpenContext->indexNextOutputDMABuf ==ASRC_DMA_BUF_A){
                    //RETAILMSG(TRUE, (_T("ARCOutputIST OUTBUF A DONE \r\n")));

                    if (TransferOutputBuf(pOpenContext,ASRC_DMA_BUF_A) == 0){
                        //OUTPUT BUFFER IS UNDERRUN,MARK IT BUFFER B IS STILL IN PROCESS
                        pOpenContext->bOutputBufUnderrun = TRUE;

                        }
                }
            }

            if (status & ASRC_BUF_OUTPUT_DONE_B){
                if((pOpenContext->bOutputBufUnderrun) 
                     && (pOpenContext->indexNextOutputDMABuf == ASRC_DMA_BUF_A)){

                     pOpenContext->bOutputBufUnderrun = FALSE;//Un-set the flag

                     //RETAILMSG(TRUE, (_T("Output buffer a underrun detected \r\n")));
                      
                     //we need to transfer buf a first
                     if(TransferOutputBuf(pOpenContext,ASRC_DMA_BUF_A) == 0){
                        // still no data, stop dma now
                        //RETAILMSG(TRUE, (_T("Output still no data, stop dma \r\n")));
                        AsrcStopConv(pOpenContext->pairIndex,FALSE,FALSE,TRUE);
                        pOpenContext->bOutputDMARunning = FALSE;

                     }
                     //else we can let the following code to trans buffer b
                }


                
                if (pOpenContext->indexNextOutputDMABuf == ASRC_DMA_BUF_B){
                    //RETAILMSG(TRUE, (_T("ARCOutputIST OUTBUF B DONE \r\n")));

                   // the user buffer size must be the multiple of BUFA_SIZE+BUFB_SIZE
                   //so as buffer a is transfered, b must can be transfered.
                   TransferOutputBuf(pOpenContext,ASRC_DMA_BUF_B);
                   if(pOpenContext->dwUnProcessedSample> 0){
                      // AsrcStartConv(pOpenContext->pairIndex,FALSE,FALSE,TRUE);
                   }else{

                       if(pOpenContext->pOutputListHead == NULL){
                           //RETAILMSG(TRUE, (_T("no data to process , stop dma \r\n")));
                 
                           AsrcStopConv(pOpenContext->pairIndex,FALSE,FALSE,TRUE);
                           pOpenContext->bOutputDMARunning = 0;
                           //we stop the dma as no avail data to process
                       } 

                   }
                }
            }

            if (pOpenContext->bOutputDMARunning ){
                AsrcStartConv(pOpenContext->pairIndex,FALSE,FALSE,TRUE);
            }

            LeaveCriticalSection(&(pOpenContext->lockOutput));

        }//end of while (pOpenContext->bOpened)

    }



    return 1;

}

BOOL  DllEntry(HANDLE hDllHandle,DWORD dwReason,LPVOID lpreserved) 
{
    BOOL bRc = TRUE;
    
    //UNREFERENCED_PARAMETER(hDllHandle);
    UNREFERENCED_PARAMETER(lpreserved);
    
    switch (dwReason) {
        case DLL_PROCESS_ATTACH: 
            {
                DEBUGREGISTER((HINSTANCE)hDllHandle);
                DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_ATTACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
                DisableThreadLibraryCalls((HMODULE)hDllHandle);
            } 
            break;

        case DLL_PROCESS_DETACH: 
            {
                DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_DETACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
            } 
            break;

        default:
            break;
    }
    
    return bRc;
}
