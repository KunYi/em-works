//------------------------------------------------------------------------------
//
// Copyright (C) 2009 - 2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 

#pragma warning(push)
#pragma warning(disable: 4201)
#include <windows.h>
#pragma warning(pop)

#include <diskio.h>

extern DWORD BytesPerSector();

#include "proxy.h"

#include "bufferpool.h"

#define DEBUG_FLG 0

#define SLEEP_INTERVAL 100

//-----------------------------------------------------------------------------
//
// Function: BufferPool
//
// constrctor of BufferPool class
//
// Parameters:
//      len                ----    buffer length
//      LegacyBlockDriver  ----    if legacyblock driver
//      hStore             ----    storage device handle
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BufferPool::BufferPool(DWORD len, BOOL LegacyBlockDriver, HANDLE hStore):
                      m_dwBufferLength(len), m_fLegacyBlockDriver(LegacyBlockDriver),m_hStore(hStore)
{
    DWORD   dwRet;

    InitializeCriticalSection(&m_CSection);
    
    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        m_pBufferObj[i] = new BufferObj;
        if(m_pBufferObj[i] != NULL)
        {
            m_pBufferObj[i]->pBuffer = (PVOID) LocalAlloc(0, m_dwBufferLength);
            if(m_pBufferObj[i]->pBuffer == NULL)
            {
                dwRet = GetLastError();
                ERRORMSG(1, (_T("BufferPool LocalAlloc failed. Error: %d\r\n"), dwRet));
            }
        }
    }

    m_bRunning = TRUE;

    m_dwNewRequestFlag = 0;

    m_pResponseBufferObj = NULL;

    m_hRequestEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == m_hRequestEvent)
    {
        ERRORMSG(TRUE,
            (TEXT("BufferPool: CreateEvent failed for Request \r\n")));
    }

    m_hResponseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == m_hResponseEvent)
    {
        ERRORMSG(TRUE,
            (TEXT("BufferPool: CreateEvent failed for Response \r\n")));
    }

    m_hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunningThread, this, 0, NULL);
    if (m_hThread == NULL)
    {
        ERRORMSG(TRUE,(TEXT("BufferPool: CreateThread failed!\r\n")));
    }
    else
    {
        RETAILMSG(DEBUG_FLG, (TEXT("BufferPool: create thread success 0x%x\r\n"),m_hThread));
        CeSetThreadPriority(m_hThread, POOL_DEFAULT_TRANSFER_THREAD_PRIORITY);
    }
}


//-----------------------------------------------------------------------------
//
// Function: ~BufferPool
//
// deconstrctor of BufferPool class
//
// Parameters:
//      N/A
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BufferPool::~BufferPool()
{
    // Stop interrupt thread
    if (m_hThread != NULL)
    {
        m_bRunning = FALSE;
        SetEvent(m_hRequestEvent);
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    CloseHandle(m_hResponseEvent);
    m_hResponseEvent = NULL;

    CloseHandle(m_hRequestEvent);
    m_hRequestEvent = NULL;
    
    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        if(m_pBufferObj[i] != NULL)
        {
            if(m_pBufferObj[i]->pBuffer != NULL)
            {
                LocalFree(m_pBufferObj[i]->pBuffer);
                m_pBufferObj[i]->pBuffer = NULL;
            }
        }
        delete m_pBufferObj[i];
        m_pBufferObj[i] = NULL;
    }
    
    DeleteCriticalSection(&m_CSection);  

    RETAILMSG(DEBUG_FLG,(TEXT("buffer pool deconstrutor\r\n")));
}

//-----------------------------------------------------------------------------
//
// Function: ReAllocBufferObj
//
// According to the buffer length to re-alloc bigger buffer, if failed, the buffer address will not be changed
//
// Parameters:
//      dwLength              ----     the lenght which will be used to re-alloc buffer
//
// Returns:
//      TURE                  ----     realloc successfully
//      FALSE                 ----     realloc failed
//
//-----------------------------------------------------------------------------
BOOL BufferPool::ReAllocBufferObj(DWORD dwLength)
{
    BOOL blRet = TRUE;
    Lock();
    
    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        PVOID pTemp;
        if(m_pBufferObj[i] != NULL)
        {
            pTemp = m_pBufferObj[i]->pBuffer;
            m_pBufferObj[i]->pBuffer = (PVOID) LocalReAlloc(m_pBufferObj[i]->pBuffer, dwLength, LMEM_MOVEABLE);

            if(m_pBufferObj[i]->pBuffer == NULL)
            {
                RETAILMSG(1,(TEXT("ReAllocBufferObj failed\r\n")));
                m_pBufferObj[i]->pBuffer = pTemp;
                blRet = FALSE;
                break;
            }
        }
    }
    
    if(blRet)
    {
        m_dwBufferLength = dwLength;
    }
    Unlock();
    return blRet;
}

//-----------------------------------------------------------------------------
//
// Function: SetNewRequestFlag
//
// When we have some new request need buffer pool running thread to handle, we need
// set this flag to indicate a new request incoming
//
//
// Parameters:
//      N/A
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void BufferPool::SetNewRequestFlag()
{
    InterlockedIncrement((LPLONG)&m_dwNewRequestFlag);
}


//-----------------------------------------------------------------------------
//
// Function: ResetNewRequestFlag
//
// buffer pool running thread use this function to reset the new request
//
//
// Parameters:
//      N/A
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void BufferPool::ResetNewRequestFlag()
{
    InterlockedDecrement((LPLONG)&m_dwNewRequestFlag);
}


//-----------------------------------------------------------------------------
//
// Function: Lock
//
// Lock the critical section to sync the access
//
//
// Parameters:
//      N/A
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void BufferPool::Lock()
{
    EnterCriticalSection(&m_CSection);
}


//-----------------------------------------------------------------------------
//
// Function: Unlock
//
// Unlock the critical section to sync the access
//
//
// Parameters:
//      N/A
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void BufferPool::Unlock()
{
    LeaveCriticalSection(&m_CSection);
}

//-----------------------------------------------------------------------------
//
// Function: GetNewRequestFlag
//
// check if there is  a new request sent to buffer pool running thread so that running thread can handle it
//
//
// Parameters:
//      N/A
//
// Returns:
//      TRUE    ----    a new request sent
//      FALSE   ----    no new request
//
//-----------------------------------------------------------------------------
BOOL BufferPool::GetNewRequestFlag()
{
    Lock();
    BOOL blRet = m_dwNewRequestFlag>0?TRUE:FALSE;
    Unlock();
    
    return blRet;
}

//-----------------------------------------------------------------------------
//
// Function: SetResponseBufferObj
//
// set response obj
//
// 
// Parameters:
//      pBuffObj    ----    the buffer obj pointer which will be set as response buffer obj 
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void BufferPool::SetResponseBufferObj(BufferObj * pBuffObj)
{
    Lock();
    m_pResponseBufferObj = pBuffObj;
    //RETAILMSG(DEBUG_FLG,(TEXT("SetResponseBufferObj 0x%x\r\n"),m_pResponseBufferObj));
    Unlock();
}

//-----------------------------------------------------------------------------
//
// Function: GetResponseBufferObj
//
// get response obj
//
// 
// Parameters:
//      ppBuffObj    ----    pointer point to the buffer obj pointer which will have been set as response buffer obj 
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void BufferPool::GetResponseBufferObj(BufferObj** ppBufferObj)
{
    //RETAILMSG(DEBUG_FLG,(TEXT("GetResponseBufferObj 0x%x\r\n"),m_pResponseBufferObj));
    Lock();
    *ppBufferObj = m_pResponseBufferObj;
    Unlock();
    return ;
}

//-----------------------------------------------------------------------------
//
// Function: ExecuterLoop
//
// buffer pool running thread function
//
// 
// Parameters:
//      i        ----    Unused
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void BufferPool::ExecuterLoop(UINT32 i)
{
    UNREFERENCED_PARAMETER(i);
    // loop here
    while(m_bRunning)
    {
        if (WaitForSingleObject(m_hRequestEvent,INFINITE) == WAIT_OBJECT_0)
        {
            DoExec();
        }
        else
        {
            //impossible here
        }
    }
    
    return ;
}

//-----------------------------------------------------------------------------
//
// Function: RunningThread
//
// buffer pool running thread wrapper
//
// 
// Parameters:
//      pBufferPool    ----    pointer point to buffer pool instance 
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
DWORD RunningThread(BufferPool* pBufferPool)
{
    pBufferPool->ExecuterLoop(0);
    ExitThread(0);
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: ReadBufferPool
//
// constrctor of ReadBufferPool class
//
// Parameters:
//      len                ----    buffer length
//      LegacyBlockDriver  ----    if legacyblock driver
//      hStore             ----    storage device handle
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
ReadBufferPool::ReadBufferPool(DWORD len, BOOL LegacyBlockDriver, HANDLE hStore):BufferPool(len, LegacyBlockDriver, hStore)
{
    
}


//-----------------------------------------------------------------------------
//
// Function: ~BufferPool
//
// deconstrctor of BufferPool class
//
// Parameters:
//      N/A
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
ReadBufferPool::~ReadBufferPool()
{
}


//-----------------------------------------------------------------------------
//
// Function: GetFilledBufferObj
//
// Every buffer in the buffer pool has a status, BUFFER_EMPTY mean this buffer is empty
// BUFFER_EXECUTING means this buffer is using be file access, BUFFER_FILLED means this 
// buffer has filled date to handle.
//
// This function try to get a filled buffer from buffer pool
//
// Parameters:
//      ldwStartAddressen  ----    buffer start address
//      dwLength           ----    buffer length
//
// Returns:
//      NULL                  ----     no empty buffer
//      non NULL              ----     empty buffer obj
//
//-----------------------------------------------------------------------------
BufferObj* ReadBufferPool::GetFilledBufferObj(DWORD dwStartAddress, DWORD dwLength)
{
    Lock();

    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        if(m_pBufferObj[i]->dwStatus == BUFFER_FILLED)
        {
            if((m_pBufferObj[i]->dwStartAddress == dwStartAddress) && 
               (dwLength == m_pBufferObj[i]->dwLength))
            {
                Unlock();
                return m_pBufferObj[i];
            }
        }   
    }
    
    Unlock();
    return NULL;
}

//-----------------------------------------------------------------------------
//
// Function: GetExecutingBufferObj
//
// Every buffer in the buffer pool has a status, BUFFER_EMPTY mean this buffer is empty
// BUFFER_EXECUTING means this buffer is using be file access, BUFFER_FILLED means this 
// buffer has filled date to handle.
//
// This function try to get an executing buffer from buffer pool
//
// Parameters:
//      ldwStartAddressen  ----    buffer start address
//      dwLength           ----    buffer length
//
// Returns:
//      NULL                  ----     no empty buffer
//      non NULL              ----     empty buffer obj
//
//-----------------------------------------------------------------------------
BufferObj* ReadBufferPool::GetExecutingBufferObj(DWORD dwStartAddress, DWORD dwLength)
{
    Lock();

    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        if(m_pBufferObj[i]->dwStatus == BUFFER_EXECUTING)
        {
            if((m_pBufferObj[i]->dwStartAddress == dwStartAddress) && 
               (dwLength == m_pBufferObj[i]->dwLength))
            {
                Unlock();
                return m_pBufferObj[i];
            }
        }   
    }
    
    Unlock();
    return NULL;
}

//-----------------------------------------------------------------------------
//
// Function: GetEmptyBufferObj
//
// Every buffer in the buffer pool has a status, BUFFER_EMPTY mean this buffer is empty
// BUFFER_EXECUTING means this buffer is using be file access, BUFFER_FILLED means this 
// buffer has filled date to handle.
//
// This function try to get a empty buffer from buffer pool
//
// Parameters:
//      N/A
//
// Returns:
//      NULL                  ----     no empty buffer
//      non NULL              ----     empty buffer obj
//
//-----------------------------------------------------------------------------
BufferObj* ReadBufferPool::GetEmptyBufferObj()
{
    return NULL;
}


//-----------------------------------------------------------------------------
//
// Function: GetBuffer
//
// Get a buffer from ReadBufferPool class, it will try to get a filled buffer from buffer pool
//
// Parameters:
//      dwStartAddress        ----     the start address which will be used to execute in the returned buffer
//      dwLength              ----     the lenght which will be used to execute in the returned buffer
//
// Returns:
//      the buffer address.
//
//-----------------------------------------------------------------------------
PVOID ReadBufferPool::GetBuffer(DWORD dwStartAddress, DWORD dwLength)
{
    static DWORD dwTotal = 0;
    static DWORD dwHit = 0; 
    BufferObj* pBufferObj = NULL;
    RETAILMSG(DEBUG_FLG,(TEXT("GetBuffer %d, %d\r\n"),dwStartAddress,dwLength));
    dwTotal++;
    if(dwLength >= m_dwBufferLength)
    {
        ReAllocBufferObj(dwLength);

        //because of buffer realloc, all the buffer is empty, we need send a new request
        SendNewRequest(dwStartAddress, dwLength, NULL);

        WaitforResponseReady();

        pBufferObj = GetFilledBufferObj(dwStartAddress, dwLength);
        if(pBufferObj != NULL)
        {
            RETAILMSG(DEBUG_FLG,(TEXT("ReAllocBuffer New request successful!!!\r\n")));
            SetResponseBufferObj(pBufferObj);
            // already get response obj, reset response event
            ResetEvent(m_hResponseEvent);
            return pBufferObj->pBuffer;
        }
        else
        {
            RETAILMSG(1,(TEXT("ReAllocBuffer New request failed!!!\r\n")));
            return NULL;
        }     
    }
    else
    {
        //try to find a filled buffer to match request
        pBufferObj = GetFilledBufferObj(dwStartAddress, dwLength);
        if(pBufferObj != NULL)
        {
            dwHit++;
            RETAILMSG(DEBUG_FLG,(TEXT("in Filled!!!\r\n")));
            SetResponseBufferObj(pBufferObj);
            // already get response obj, reset response event
            ResetEvent(m_hResponseEvent);
            return pBufferObj->pBuffer;
        }
        else
        {
            ResetEvent(m_hResponseEvent);
            pBufferObj = GetExecutingBufferObj(dwStartAddress, dwLength);
            if(pBufferObj != NULL)
            {
                dwHit++;
                RETAILMSG(DEBUG_FLG,(TEXT("in Executing!!!\r\n")));
                WaitforResponseReady();

                pBufferObj = GetFilledBufferObj(dwStartAddress, dwLength);
                if(pBufferObj != NULL)
                {
                    dwHit++;
                    RETAILMSG(DEBUG_FLG,(TEXT("Wait for executing successful!!!\r\n")));
                    SetResponseBufferObj(pBufferObj);
                    // already get response obj, reset response event
                    //ResetEvent(m_hResponseEvent);
                    return pBufferObj->pBuffer;
                }
                else
                {
                    RETAILMSG(1,(TEXT("Wait for executing failed!!!\r\n")));
                    return NULL;
                }    
            }
            else
            {
                RETAILMSG(DEBUG_FLG,(TEXT("Miss\r\n")));
                
                SendNewRequest(dwStartAddress, dwLength, NULL);

                WaitforResponseReady();
        
                pBufferObj = GetFilledBufferObj(dwStartAddress, dwLength);
                if(pBufferObj != NULL)
                {
                    RETAILMSG(DEBUG_FLG,(TEXT("New request successful!!!\r\n")));
                    SetResponseBufferObj(pBufferObj);
                    // already get response obj, reset response event
                    //ResetEvent(m_hResponseEvent);
                    return pBufferObj->pBuffer;
                }
                else
                {
                    RETAILMSG(1,(TEXT("New request failed!!!\r\n")));
                    return NULL;
                }     
            }   
        }
    }
}


//-----------------------------------------------------------------------------
//
// Function: SendNewRequest
//
// send a new request to buffer pool running thread so that running thread can handle it
//
//
// Parameters:
//      dwStartAddress        ----     the start address which will be used to execute in the requested buffer
//      dwLength              ----     the lenght which will be used to execute in the requested buffer
//      pBuffer               ----     the buffer address be requested
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void ReadBufferPool::SendNewRequest(DWORD dwStartAddress, DWORD dwLength, PVOID pBuffer)
{
    UNREFERENCED_PARAMETER(pBuffer);
    // prepare scatter/gather buffer
    Lock();
    
    m_BufferRequest.dwStartAddress = dwStartAddress;
    m_BufferRequest.dwLength       = dwLength;
    
    SetNewRequestFlag();

    // we should reset response event
    ResetEvent(m_hResponseEvent);
    
    SetEvent(m_hRequestEvent);

    Unlock();
    
}


//-----------------------------------------------------------------------------
//
// Function: GetResponse
//
// After we send new request to running thread, we may need to get the response
// for example, if we send a read request to running thread, we need to get the 
// filled buffer as the response.
//
//
// Parameters:
//      N/A
//
// Returns:
//      NULL       ----    some error
//      non NULL   ----    the response buffer address
//
//-----------------------------------------------------------------------------
void ReadBufferPool::WaitforResponseReady()
{
    WaitForSingleObject(m_hResponseEvent,INFINITE);
    return;
}


//-----------------------------------------------------------------------------
//
// Function: ReturnBuffer
//
// turn a buffer to BufferPool class, for read buffer pool, it means this buffer can
// be filled with unread data again.
//
// Parameters:
//      dwStartAddress        ----     the start address which will be used to execute in the returned buffer
//      dwLength              ----     the lenght which will be used to execute in the returned buffer
//      pBuffer               ----     the buffer address will be returned
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void ReadBufferPool::ReturnBuffer(DWORD dwLogicalBlockAddress, DWORD dwTransferLength, PVOID pbData)
{
    UNREFERENCED_PARAMETER(dwLogicalBlockAddress);
    UNREFERENCED_PARAMETER(dwTransferLength);
    UNREFERENCED_PARAMETER(pbData);
    
    BufferObj* pBufferObj = NULL;
    GetResponseBufferObj(&pBufferObj);
    if(pBufferObj == NULL)
    {
        RETAILMSG(DEBUG_FLG,(TEXT("no buffer gotten, no need return\r\n")));
        return ;
    }
    Lock();
    if(pBufferObj->pBuffer != pbData)
    {
        RETAILMSG(DEBUG_FLG,(TEXT("ReadBuffer:ReturnBuffer expect 0x%x, but gotten 0x%x\r\n"),pBufferObj->pBuffer, pbData));
        Unlock();
        return;
    }
    else
    {
        RETAILMSG(DEBUG_FLG,(TEXT("ReadBuffer:ReturnBuffer  0x%x\r\n"),pbData));
    }
    
    if(pBufferObj->dwStatus != BUFFER_FILLED)
    {
        RETAILMSG(1,(TEXT("ReturnBuffer error, buffer(0x%x) is not BUFFER_FILLED status %d\r\n"),pBufferObj->pBuffer, pBufferObj->dwStatus != BUFFER_FILLED));
    }
    
    pBufferObj->dwStatus = BUFFER_EMPTY;

    SetResponseBufferObj(NULL);
    
    Unlock();

}


//-----------------------------------------------------------------------------
//
// Function: DoExec
//
// This function will be used to perform the acctual read file access
// it has two case: 
//    1. new request 
//       in this case, we shold clear all the filled buffer and try to fill them with 
//       the new beginning address.
//    2. no new request
//       in this case, we should try to fill in all the empty buffer
//
// 
// Parameters:
//      N/A
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void ReadBufferPool::DoExec()
{
    BufferObj* pBufferObj = NULL;

    
    if(GetNewRequestFlag())
    {
        // Clear new request flag
        ResetNewRequestFlag();
        
        pBufferObj = GetNewRequestBufferObj();
        
        if(ExecReadBuffer(pBufferObj) == EXECUTE_PASS)
        {
            // response to request
            SetEvent(m_hResponseEvent);         

            // try to filled next empty buffer
            SetEvent(m_hRequestEvent);
        }
        else
        {
            // response to request
            SetEvent(m_hResponseEvent);
        }
    }
    else
    {
        pBufferObj = PrepareBufferObj();
        if(pBufferObj != NULL)
        {
            if(ExecReadBuffer(pBufferObj) == EXECUTE_PASS)
            { 
                if(!GetNewRequestFlag())
                {
                    SetEvent(m_hResponseEvent);
                    
                    SetEvent(m_hRequestEvent);
                }
            }
            else
            {
                // retry 
                RETAILMSG(DEBUG_FLG,(TEXT("ExecReadBuffer failed\r\n")));

                SetEvent(m_hResponseEvent);
            }
        }
        else
        {
            RETAILMSG(DEBUG_FLG,(TEXT("No Empty BufferObj\r\n")));
        }
    }  
}

//-----------------------------------------------------------------------------
//
// Function: GetNewRequestBufferObj
//
// This function get a buffer obj which will be performed the new request.
// actually it will always return m_pBufferObj[0]
//
// 
// Parameters:
//      N/A
//
// Returns:
//      m_pBufferObj[0]
//
//-----------------------------------------------------------------------------
BufferObj* ReadBufferPool::GetNewRequestBufferObj()
{
    BufferObj* pBufferObj;
    Lock();

    // firstly we should reset all the bufferobj
    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        m_pBufferObj[i]->dwStartAddress = 0;
        m_pBufferObj[i]->dwLength       = 0;
        m_pBufferObj[i]->dwStatus       = BUFFER_EMPTY;
    }
    
    // secondly we return m_pBufferObj[0]
    pBufferObj = m_pBufferObj[0];
    pBufferObj->dwStartAddress = m_BufferRequest.dwStartAddress;
    pBufferObj->dwLength       = m_BufferRequest.dwLength;
    pBufferObj->dwStatus       = BUFFER_EXECUTING;
    
    Unlock();
    return pBufferObj;
}


//-----------------------------------------------------------------------------
//
// Function: PrepareBufferObj
//
// when no new request coming, we should decide which buffer obj should be performed
// the next read operation, and this function will reset the dwStartAddress
//
// 
// Parameters:
//      N/A
//
// Returns:
//      m_pBufferObj[0]
//
//-----------------------------------------------------------------------------
BufferObj* ReadBufferPool::PrepareBufferObj()
{
    BufferObj* pBufferObj = NULL;
    Lock();
    
    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        if(m_pBufferObj[i]->dwStatus == BUFFER_EMPTY)
        {
            pBufferObj = m_pBufferObj[i];
            break;
        }   
    }

    if(pBufferObj != NULL)
    {
        pBufferObj->dwStartAddress = m_BufferRequest.dwStartAddress+m_BufferRequest.dwLength;
        pBufferObj->dwLength       = m_BufferRequest.dwLength;
        pBufferObj->dwStatus       = BUFFER_EXECUTING;
        m_BufferRequest.dwStartAddress += m_BufferRequest.dwLength;
    }
    
    RETAILMSG(DEBUG_FLG,(TEXT("PrepareBufferObj %d %d\r\n")));
   
    Unlock();
    return pBufferObj;
}

//-----------------------------------------------------------------------------
//
// Function: ExecReadBuffer
//
// perform the file read access
//
// 
// Parameters:
//      pBufferObj       ----    buffer obj need to be executed
//
// Returns:
//      m_pBufferObj[0]
//
//-----------------------------------------------------------------------------
DWORD ReadBufferPool::ExecReadBuffer(BufferObj* pBufferObj)
{
    DWORD dwResult = EXECUTE_FAIL;

    SG_REQ sgSgReq;
    SG_BUF sgSgBuf;
    DWORD dwBytesReturned;
    BOOL fResult = FALSE;

    Lock();
    //need I memset it ?
    //ZeroMemory(pBufferObj->pBuffer, (pBufferObj->dwLength*BytesPerSector()));

    // prepare scatter/gather buffer   
    sgSgBuf.sb_buf = (PUCHAR)pBufferObj->pBuffer;
    sgSgBuf.sb_len = pBufferObj->dwLength*BytesPerSector();

    // prepare scatter/gather request
    sgSgReq.sr_start = pBufferObj->dwStartAddress;
    sgSgReq.sr_num_sec = pBufferObj->dwLength;
    sgSgReq.sr_status = 0;
    sgSgReq.sr_callback = NULL;
    sgSgReq.sr_num_sg = 1;
    sgSgReq.sr_sglist[0] = sgSgBuf;

    pBufferObj->dwStatus = BUFFER_EXECUTING;
    
    Unlock();
    // read from device
    DWORD dwIoControlCode = (m_fLegacyBlockDriver) ? DISK_IOCTL_READ : IOCTL_DISK_READ;
    fResult = DeviceIoControl(
        m_hStore,
        dwIoControlCode,
        &sgSgReq,
        sizeof(sgSgReq),
        NULL,
        0,
        &dwBytesReturned,
        NULL);
    Lock();
    if (fResult) 
    {
        RETAILMSG(DEBUG_FLG,(TEXT("Read access %d %d to buffer 0x%x\r\n"),pBufferObj->dwStartAddress,pBufferObj->dwLength, pBufferObj->pBuffer));
        pBufferObj->dwStatus = BUFFER_FILLED;
        dwResult = EXECUTE_PASS;
    }
    else 
    {
        RETAILMSG(DEBUG_FLG,(TEXT("Read access failed %d %d\r\n"),pBufferObj->dwStartAddress,pBufferObj->dwLength));
        pBufferObj->dwStatus = BUFFER_ERROR;
    }

    Unlock();
    return dwResult;
}

//-----------------------------------------------------------------------------
//
// Function: WriteBufferPool
//
// constrctor of WriteBufferPool class
//
// Parameters:
//      len                ----    buffer length
//      LegacyBlockDriver  ----    if legacyblock driver
//      hStore             ----    storage device handle
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
WriteBufferPool::WriteBufferPool(DWORD len, BOOL LegacyBlockDriver, HANDLE hStore):BufferPool(len, LegacyBlockDriver, hStore)
{
    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        m_pBufferObj[i]->dwStatus = BUFFER_EMPTY;
    }
}


//-----------------------------------------------------------------------------
//
// Function: ~WriteBufferPool
//
// deconstrctor of WriteBufferPool class
//
// Parameters:
//      N/A
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
WriteBufferPool::~WriteBufferPool()
{
}

//-----------------------------------------------------------------------------
//
// Function: GetFilledBufferObj
//
// Every buffer in the buffer pool has a status, BUFFER_EMPTY mean this buffer is empty
// BUFFER_EXECUTING means this buffer is using be file access, BUFFER_FILLED means this 
// buffer has filled date to handle.
//
// This function try to get a filled buffer from buffer pool
//
// Parameters:
//      ldwStartAddressen     ----    buffer start address
//      dwLength              ----    buffer length
//
// Returns:
//      NULL                  ----     no empty buffer
//      non NULL              ----     empty buffer obj
//
//-----------------------------------------------------------------------------
BufferObj* WriteBufferPool::GetFilledBufferObj(DWORD dwStartAddress, DWORD dwLength)
{
    UNREFERENCED_PARAMETER(dwStartAddress);
    UNREFERENCED_PARAMETER(dwLength);
    return NULL;
}

//-----------------------------------------------------------------------------
//
// Function: GetExecutingBufferObj
//
// Every buffer in the buffer pool has a status, BUFFER_EMPTY mean this buffer is empty
// BUFFER_EXECUTING means this buffer is using be file access, BUFFER_FILLED means this 
// buffer has filled date to handle.
//
// This function try to get an executing buffer from buffer pool
//
// Parameters:
//      ldwStartAddressen  ----    buffer start address
//      dwLength           ----    buffer length
//
// Returns:
//      NULL                  ----     no empty buffer
//      non NULL              ----     empty buffer obj
//
//-----------------------------------------------------------------------------
BufferObj* WriteBufferPool::GetExecutingBufferObj(DWORD dwStartAddress, DWORD dwLength)
{
    UNREFERENCED_PARAMETER(dwStartAddress);
    UNREFERENCED_PARAMETER(dwLength);
    return NULL;
}

//-----------------------------------------------------------------------------
//
// Function: GetEmptyBufferObj
//
// Every buffer in the buffer pool has a status, BUFFER_EMPTY mean this buffer is empty
// BUFFER_EXECUTING means this buffer is using be file access, BUFFER_FILLED means this 
// buffer has filled date to handle.
//
// This function try to get a empty buffer from buffer pool
//
// Parameters:
//      N/A
//
// Returns:
//      NULL                  ----     no empty buffer
//      non NULL              ----     empty buffer obj
//
//-----------------------------------------------------------------------------
BufferObj* WriteBufferPool::GetEmptyBufferObj()
{
    Lock();

    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        if(m_pBufferObj[i]->dwStatus == BUFFER_EMPTY)
        {
            Unlock();
            return m_pBufferObj[i];
        }   
    }

    // if reach here, means all the bufferObj is filled or filling
    Unlock();
    return NULL;
}


//-----------------------------------------------------------------------------
//
// Function: GetBuffer
//
// Get a buffer from ReadBufferPool class, it will try to get a empty buffer from buffer pool
//
// Parameters:
//      dwStartAddress        ----     the start address which will be used to execute in the returned buffer
//      dwLength              ----     the lenght which will be used to execute in the returned buffer
//
// Returns:
//      the buffer address.
//
//-----------------------------------------------------------------------------
PVOID WriteBufferPool::GetBuffer(DWORD dwStartAddress, DWORD dwLength)
{
    static DWORD dwTotal = 0;
    BufferObj* pBufferObj = NULL;

    UNREFERENCED_PARAMETER(dwStartAddress);
    Lock();
    if(dwLength >= m_dwBufferLength)
    {
        ReAllocBufferObj(dwLength);
    }

    pBufferObj = GetEmptyBufferObj();
    
    if(pBufferObj != NULL)
    {
        RETAILMSG(DEBUG_FLG,(TEXT("WriteBufferPool GetBuffer directly 0x%x\r\n"),pBufferObj->pBuffer));
        SetResponseBufferObj(pBufferObj);
        Unlock();
        return pBufferObj->pBuffer;
    }
    else
    {
        RETAILMSG(DEBUG_FLG,(TEXT("no empty buffer\r\n")));
    }
    ResetEvent(m_hResponseEvent);
    Unlock();
    
    // here means all the buffer is filled or filling, we must wait for a empty buffer
    if(WaitForSingleObject(m_hResponseEvent,2000) == WAIT_OBJECT_0)
    {
        Lock();
        pBufferObj = GetEmptyBufferObj();
    
        if(pBufferObj!=NULL)
        {
            RETAILMSG(DEBUG_FLG,(TEXT("WriteBufferPool GetBuffer delay 0x%x\r\n"),pBufferObj->pBuffer));
            SetResponseBufferObj(pBufferObj);
            Unlock();
            return pBufferObj->pBuffer;
        }
        else
        {
            RETAILMSG(DEBUG_FLG,(TEXT("no empty buffer, huh\r\n")));
            SetResponseBufferObj(NULL);
            ResetEvent(m_hRequestEvent);
            for(int i = 0; i < POOL_BUFFER_COUNT; i++)
            {
                m_pBufferObj[i]->dwStatus = BUFFER_EMPTY;
            }
            Unlock();
        }
        
    }
    else
    {
        Lock();
        RETAILMSG(DEBUG_FLG,(TEXT("time out\r\n")));
        SetResponseBufferObj(NULL);
        ResetEvent(m_hRequestEvent);
        for(int i = 0; i < POOL_BUFFER_COUNT; i++)
        {
            m_pBufferObj[i]->dwStatus = BUFFER_EMPTY;
        }
        Unlock();
    }
    
    return NULL;
}


//-----------------------------------------------------------------------------
//
// Function: SendNewRequest
//
// send a new request to buffer pool running thread so that running thread can handle it
// for write buffer pool, we use a priority to indicate which buffer should be handle firstly
//
// Parameters:
//      dwStartAddress        ----     the start address which will be used to execute in the requested buffer
//      dwLength              ----     the lenght which will be used to execute in the requested buffer
//      pBuffer               ----     the buffer address be requested
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void WriteBufferPool::SendNewRequest(DWORD dwStartAddress, DWORD dwLength, PVOID pBuffer)
{
    UNREFERENCED_PARAMETER(dwStartAddress);
    UNREFERENCED_PARAMETER(dwLength);
    Lock();
    // set bufferobj priority
    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        if(m_pBufferObj[i]->dwStatus == BUFFER_FILLED)
        {
            if(m_pBufferObj[i]->pBuffer != pBuffer)
            {
                m_pBufferObj[i]->dwPrioity++;
            }
            else
            {
                m_pBufferObj[i]->dwPrioity = 1;
            }
        }   
    }

    Unlock();
   
    //SetNewRequestFlag();
    RETAILMSG(DEBUG_FLG,(TEXT("WriteBufferPool set request event\r\n")));
    SetEvent(m_hRequestEvent);
}


//-----------------------------------------------------------------------------
//
// Function: CheckReturnBuffer
//
// check the returned buffer is available
//
//
// Parameters:
//      dwStartAddress        ----     the returned start address which will be used to execute in the requested buffer
//      dwLength              ----     the returned lenght which will be used to execute in the requested buffer
//      pBuffer               ----     the returnedbuffer address be requested
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
BufferObj* WriteBufferPool::CheckReturnBuffer(DWORD dwStartAddress, DWORD dwLength, PVOID pBuffer)
{
    //UNREFERENCED_PARAMETER(dwStartAddress);
    //UNREFERENCED_PARAMETER(dwLength);
    Lock();
    BufferObj* pBufferObj = NULL;
    GetResponseBufferObj(&pBufferObj);
    if( pBufferObj == NULL )
    {
        Unlock();
        return NULL;
    }
    if( (pBufferObj->pBuffer != pBuffer)||
        (pBufferObj->dwStatus != BUFFER_EMPTY)
      )
    {
        RETAILMSG(DEBUG_FLG,(TEXT("WriteBufferPool CheckReturnBuffer fail expect 0x%x but 0x%x %d\r\n"),pBufferObj->pBuffer,pBuffer,pBufferObj->dwStatus));
        Unlock();
        return NULL;
    }

    pBufferObj->dwStartAddress = dwStartAddress;
    pBufferObj->dwLength       = dwLength;
    pBufferObj->dwStatus       = BUFFER_FILLED;
    Unlock();
    return pBufferObj;
}


//-----------------------------------------------------------------------------
//
// Function: ReturnBuffer
//
// turn a buffer to BufferPool class, for write buffer pool, it means this buffer have filled data and 
// can be written to file.
//
// Parameters:
//      dwStartAddress        ----     the start address which will be used to execute in the returned buffer
//      dwLength              ----     the lenght which will be used to execute in the returned buffer
//      pBuffer               ----     the buffer address will be returned
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void WriteBufferPool::ReturnBuffer(DWORD dwLogicalBlockAddress, DWORD dwTransferLength, PVOID pbData)
{
    BufferObj* pBufferObj = NULL;

    Lock();
    
    pBufferObj = CheckReturnBuffer(dwLogicalBlockAddress, dwTransferLength, pbData);
    
    if(pBufferObj == NULL)
    {
        RETAILMSG(DEBUG_FLG,(TEXT("WriteBufferPool::ReturnBuffer error\r\n")));
        return ;
    }
    SendNewRequest(pBufferObj->dwStartAddress, pBufferObj->dwLength, pBufferObj->pBuffer);

    RETAILMSG(DEBUG_FLG,(TEXT("WriteBufferPool ReturnBuffer 0x%x \r\n"),pbData));
    Unlock();
}


//-----------------------------------------------------------------------------
//
// Function: DoExec
//
// get the buffer obj which should be performed first and do execution
//
// 
// Parameters:
//      N/A
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
void WriteBufferPool::DoExec()
{
    BufferObj* pBufferObj = NULL;

    static DWORD dwTimes = 0;
    
    if(dwTimes++ >= SLEEP_INTERVAL)
    {
        dwTimes = 0;
        Sleep(1);
    }

    Lock();
    
    {        
        pBufferObj = GetNewRequestBufferObj();

        if(pBufferObj != NULL)
        {
            Unlock();
            if(ExecWriteBuffer(pBufferObj) == EXECUTE_PASS)
            {
                Lock();

                pBufferObj->dwStatus = BUFFER_EMPTY;

                RETAILMSG(DEBUG_FLG,(TEXT("SetResponse 0x%x %d\r\n"),pBufferObj->pBuffer,pBufferObj->dwStatus));

                // response to request
                SetEvent(m_hResponseEvent);

                // try to filled next empty buffer
                SetEvent(m_hRequestEvent);
            }
            else
            {
                // retry 
                Lock();
                RETAILMSG(DEBUG_FLG,(TEXT("ExecWriteBuffer failed\r\n")));
            }
        }
    }

    Unlock();
}

//-----------------------------------------------------------------------------
//
// Function: GetResponse
//
// After we send new request to running thread, we may need to get the response
// for example, if we send a read request to running thread, we need to get the 
// filled buffer as the response.
//
//
// Parameters:
//      N/A
//
// Returns:
//      NULL       ----    some error
//      non NULL   ----    the response buffer address
//
//-----------------------------------------------------------------------------
void WriteBufferPool::WaitforResponseReady()
{
    return ;
}

//-----------------------------------------------------------------------------
//
// Function: GetNewRequestBufferObj
//
// according to the buffer proirity, get the highest priority one
//
// 
// Parameters:
//      N/A
//
// Returns:
//      N/A
//
//-----------------------------------------------------------------------------
BufferObj* WriteBufferPool::GetNewRequestBufferObj()
{
    BufferObj* pBufferObj = NULL;
    Lock();

    // we should get the highest priority bufferobj which dwStatus = BUFFER_FILLED
    for(int i = 0; i < POOL_BUFFER_COUNT; i++)
    {
        if(m_pBufferObj[i]->dwStatus == BUFFER_FILLED)
        {
            if(pBufferObj != NULL)
            {
                if(m_pBufferObj[i]->dwPrioity >= pBufferObj->dwPrioity)
                {
                    pBufferObj = m_pBufferObj[i];
                }
            }
            else
            {
                pBufferObj = m_pBufferObj[i];
            }
        }
    }

    if(pBufferObj != NULL)
    {
        pBufferObj->dwStatus = BUFFER_EXECUTING;
    }
    
    Unlock();
    return pBufferObj;
}


//-----------------------------------------------------------------------------
//
// Function: ExecWriteBuffer
//
// perform the file write access
//
// 
// Parameters:
//      pBufferObj       ----    buffer obj need to be executed
//
// Returns:
//      m_pBufferObj[0]
//
//-----------------------------------------------------------------------------
DWORD WriteBufferPool::ExecWriteBuffer(BufferObj* pBufferObj)
{   
    DWORD dwResult = EXECUTE_FAIL;

    SG_REQ sgSgReq;
    SG_BUF sgSgBuf;
    DWORD dwBytesReturned;
    BOOL fResult = FALSE;

    // prepare scatter/gather buffer   
    sgSgBuf.sb_buf = (PUCHAR)pBufferObj->pBuffer;
    sgSgBuf.sb_len = pBufferObj->dwLength*BytesPerSector();

    // prepare scatter/gather request
    sgSgReq.sr_start = pBufferObj->dwStartAddress;
    sgSgReq.sr_num_sec = pBufferObj->dwLength;
    sgSgReq.sr_status = 0;
    sgSgReq.sr_callback = NULL;
    sgSgReq.sr_num_sg = 1;
    sgSgReq.sr_sglist[0] = sgSgBuf;

    // read from device
    DWORD dwIoControlCode = (m_fLegacyBlockDriver) ? DISK_IOCTL_WRITE : IOCTL_DISK_WRITE;
    fResult = DeviceIoControl(
        m_hStore,
        dwIoControlCode,
        &sgSgReq,
        sizeof(sgSgReq),
        NULL,
        0,
        &dwBytesReturned,
        NULL);
    if (fResult) 
    {
        RETAILMSG(DEBUG_FLG,(TEXT("Write access successfully %d %d 0x%x\r\n"),pBufferObj->dwStartAddress,pBufferObj->dwLength,pBufferObj->pBuffer));
        //pBufferObj->dwStatus = BUFFER_EMPTY;
    }
    else 
    {
        RETAILMSG(DEBUG_FLG,(TEXT("Write access failed %d %d 0x%x\r\n"),pBufferObj->dwStartAddress,pBufferObj->dwLength,pBufferObj->pBuffer));
        //pBufferObj->dwStatus = BUFFER_ERROR;
        goto EXIT;
    }

    dwResult = EXECUTE_PASS;

EXIT:;
    return dwResult;
}
