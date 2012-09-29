//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bufferpool.h
//
//  Implementation of block buffer manager.
//
//-----------------------------------------------------------------------------
#ifndef _BUFFERPOOL_H_
#define _BUFFERPOOL_H_
#include <diskio.h>

#define POOL_BUFFER_COUNT 3
#define POOL_DEFAULT_TRANSFER_THREAD_PRIORITY 101

enum BufferStatus
{
    BUFFER_EMPTY,
    BUFFER_EXECUTING,
    BUFFER_FILLED,
    BUFFER_ERROR
};

struct BufferObj
{
    PVOID         pBuffer;
    DWORD         dwStartAddress;
    DWORD         dwLength;
    DWORD         dwStatus;         // 0 -- empty
                                    // 1 -- filling
                                    // 2 -- filled

    DWORD         dwPrioity;
};

class BufferPool
{
public:
    BufferPool(DWORD len, BOOL LegacyBlockDriver, HANDLE hStore);
    virtual ~BufferPool();
    BOOL               ReAllocBufferObj(DWORD dwLength);
    virtual PVOID      GetBuffer(DWORD dwStartAddress, DWORD dwLength) = 0;
    virtual void       ReturnBuffer(DWORD dwLogicalBlockAddress, DWORD dwTransferLength, PVOID pbData) = 0;
    friend DWORD       RunningThread(BufferPool*);
private:   
    void               ExecuterLoop(UINT32); 
    virtual void       DoExec() = 0;
    
    virtual void       SendNewRequest(DWORD dwStartAddress, DWORD dwLength, PVOID pBuffer) = 0;
    virtual void       WaitforResponseReady() = 0;
    
    virtual BufferObj* GetFilledBufferObj(DWORD dwStartAddress, DWORD dwLength) = 0;
    virtual BufferObj* GetExecutingBufferObj(DWORD dwStartAddress, DWORD dwLength) = 0;
    virtual BufferObj* GetEmptyBufferObj() = 0;
    
    CRITICAL_SECTION   m_CSection;  
    
protected:
    inline void        SetResponseBufferObj(BufferObj* pBuffObj);
    inline void        GetResponseBufferObj(BufferObj** ppBufferObj);
    
    inline void        SetNewRequestFlag();
    inline void        ResetNewRequestFlag();
    inline BOOL        GetNewRequestFlag();
    
    inline void        Lock();
    inline void        Unlock();
    
    HANDLE             m_hRequestEvent;
    HANDLE             m_hResponseEvent;
    HANDLE             m_hThread;
    BOOL               m_bRunning;
    DWORD              m_dwNewRequestFlag;
    DWORD              m_dwBufferLength; 
    BufferObj          m_BufferRequest;
    BufferObj*         m_pBufferObj[POOL_BUFFER_COUNT];
    BufferObj*         m_pResponseBufferObj;
    BOOL               m_fLegacyBlockDriver;
    HANDLE             m_hStore;
    
};

class ReadBufferPool: public BufferPool
{
public:
    ReadBufferPool(DWORD len, BOOL LegacyBlockDriver, HANDLE hStore);
    ~ReadBufferPool();
    virtual void       ReturnBuffer(DWORD dwLogicalBlockAddress, DWORD dwTransferLength, PVOID pbData);
    virtual PVOID      GetBuffer(DWORD dwStartAddress, DWORD dwLength);
protected:
    virtual BufferObj* GetFilledBufferObj(DWORD dwStartAddress, DWORD dwLength);
    virtual BufferObj* GetExecutingBufferObj(DWORD dwStartAddress, DWORD dwLength);
    virtual BufferObj* GetEmptyBufferObj();
private: 
    DWORD              ExecReadBuffer(BufferObj* pBufferObj);
    void               DoExec();
    
    virtual void       SendNewRequest(DWORD dwStartAddress, DWORD dwLength, PVOID pBuffer);
    virtual void       WaitforResponseReady(); 
    
    BufferObj*         GetNewRequestBufferObj();
    BufferObj*         PrepareBufferObj();
};

class WriteBufferPool: public BufferPool
{
public:
    WriteBufferPool(DWORD len, BOOL LegacyBlockDriver, HANDLE hStore);
    ~WriteBufferPool();
    virtual void       ReturnBuffer(DWORD dwLogicalBlockAddress, DWORD dwTransferLength, PVOID pbData);
    virtual PVOID      GetBuffer(DWORD dwStartAddress, DWORD dwLength);
    
protected:
    virtual BufferObj* GetFilledBufferObj(DWORD dwStartAddress, DWORD dwLength);
    virtual BufferObj* GetExecutingBufferObj(DWORD dwStartAddress, DWORD dwLength);
    virtual BufferObj* GetEmptyBufferObj();
private:
    BufferObj*         CheckReturnBuffer(DWORD dwStartAddress, DWORD dwLength, PVOID pBuffer);
    DWORD              ExecWriteBuffer(BufferObj* pBufferObj);
    void               DoExec();
    
    virtual void       SendNewRequest(DWORD dwStartAddress, DWORD dwLength, PVOID pBuffer);
    virtual void       WaitforResponseReady(); 

    BufferObj*         GetNewRequestBufferObj();
};

#endif