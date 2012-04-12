//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

#ifndef _XFERLIST_H_
#define _XFERLIST_H_

#pragma warning(push)
#pragma warning(disable: 4189 4511 4512)
#include <assert.h>
#include <block_allocator.hxx>
#include <sync.hxx>
#include "intrlk_stack.hxx"


// Set of functions that do nothing when called by CFreePool
struct null_funcs
{
    static void create(void *pv)    {}
    static void free(void *pv)      {}
    static void reuse(void *pv)     {}
    static void destroy(void *pv)   {}
};


// Maintains a very fast free cache and slower free list of elements.
template <size_t _CACHE_SIZE = 4, typename _Al = ce::allocator, typename _ElementFuncs = null_funcs>
class CFreePool
{
protected:
    typedef interlocked_stack<void*, _Al> _FreeStack;
    
public:
    // Constructor--Initialize our data structures.
    CFreePool(
        size_t cbElement
        ) : m_alloc(),
            m_stack(),
            m_csPopProtector()
    {
        init(cbElement);
    }

    // Constructor--Initialize our data structures.
    CFreePool(
        size_t cbElement,
        const _Al &alloc
        ) : m_alloc(alloc),
            m_stack(alloc),
            m_csPopProtector()
    {
        init(cbElement);
    }


    // Destructor--Free every element in the list.
    ~CFreePool(
        ) 
    {
        void *pv;

        // Free all elements in the free cache
        for (DWORD dw = 0; dw < _CACHE_SIZE; ++dw) {
            pv = m_rgpvCache[dw];
            if (pv) {
#ifdef DEBUG
                InterlockedDecrement((PLONG) &m_dwInCache);
#endif
                DeleteElement(pv);
            }
        }
        assert(m_dwInCache == 0);
        
        // Free all elements on the free list
        while (m_stack.pop(pv)) {
#ifdef DEBUG
            InterlockedDecrement((PLONG) &m_dwOnStack);
#endif
            DeleteElement(pv);
        }
        assert(m_dwOnStack == 0);
        assert(m_dwTotalOutstanding == 0);
        
#ifdef CHECK_SATISFIED_COUNTER
        // This assertion will fail if m_dwSatisfiedTotal loops to 0 so
        // it should only be enabled to validate the satisfied counters.
        assert( (m_dwSatisfiedTotal == 0) || 
            (m_dwSatisfiedFromCache + m_dwSatisfiedFromStack < m_dwSatisfiedTotal) ); 
#endif
    }


    // Get a new/recycled element structure.
    void*
    allocate(
        size_t cbElement
        )
    {
        assert(cbElement == m_cbElement);
        
        void* pv = NULL;

        // First try to get one of the cached entries
        for (DWORD dw = 0; dw < _CACHE_SIZE; ++dw) {
            if (m_rgpvCache[dw]) {
                pv = (void*) InterlockedExchange((PLONG) &m_rgpvCache[dw], NULL);
                break;
            }
        }

        if (pv == NULL) {
            // Could not get anything from the cache. Try the free list.
            
            // Protect accesses to the element list's pop method since multiple
            // threads could be calling it.
            m_csPopProtector.lock();
            
            BOOL fPoppedOldElement = m_stack.pop(pv);
            if (fPoppedOldElement) {
                assert(pv);
                
#ifdef DEBUG
                InterlockedDecrement((PLONG) &m_dwOnStack);
#endif
                
                // Now that we have the critical section, we might as
                // well populate the cache from the free list.
                PopulateCache();
            }
            
            m_csPopProtector.unlock();

            if (fPoppedOldElement) {
                // Recycle an old element
                assert(pv);
                _ElementFuncs::reuse(pv);
#ifdef DEBUG
                InterlockedIncrement((PLONG) &m_dwSatisfiedFromStack);
#endif
            }
            else {
                // Allocate a new element
                pv = m_alloc.allocate(cbElement);

                if (pv) {
                    _ElementFuncs::create(pv);
#ifdef DEBUG
                    InterlockedIncrement((PLONG) &m_dwTotalOutstanding);
#endif
                }
            }
        }
        else {
            // Recycle an old element
            assert(pv);
            _ElementFuncs::reuse(pv);
#ifdef DEBUG
            InterlockedDecrement((PLONG) &m_dwInCache);
            InterlockedIncrement((PLONG) &m_dwSatisfiedFromCache);
#endif
        }

        if (pv) {
#ifdef DEBUG
            InterlockedIncrement((PLONG) &m_dwSatisfiedTotal);
#endif
        }

        return pv;
    }


    // Frees all elements and compacts allocators.
    //
    // Note that during this operation, there may be other threads creating 
    // elements. This means that this function does not guarantee that 
    // everything will be totally compacted.
    // 
    // This can be an expensive operation.
    void compact()  
    {
        void* pv;

        m_csPopProtector.lock();
        
        // Free all elements in the free cache
        for (DWORD dw = 0; dw < _CACHE_SIZE; ++dw) {
            if (m_rgpvCache[dw]) {
                pv = (void*) InterlockedExchange((PLONG) &m_rgpvCache[dw], NULL);

                if (pv) {
#ifdef DEBUG
                    InterlockedDecrement((PLONG) &m_dwInCache);
#endif
                    DeleteElement(pv);
                }
            }
        }
        
        // Free all elements on the free list
        
        while (m_stack.pop(pv)) {
#ifdef DEBUG
            InterlockedDecrement((PLONG) &m_dwOnStack);
#endif
            DeleteElement(pv);
        }

        m_alloc.compact();
        m_stack.compact();

        m_csPopProtector.unlock();
    }

    // Return a element structure to the free list for later use.
    void 
    deallocate(
        void* pv,
        size_t cbElement
        )
    {
        assert(cbElement == m_cbElement);
        
        if (!pv) {
            return;
        }

        _ElementFuncs::free(pv);

        // First try to put it in one of the cached entries
        for (DWORD dw = 0; dw < _CACHE_SIZE; ++dw) {
            if (m_rgpvCache[dw] == NULL) {
                pv = (void*) InterlockedExchange((PLONG) &m_rgpvCache[dw], (LONG) pv);
                break;
            }
        }

        if (pv) {
            // No room on the cache. Put it on the free list.
            if (!m_stack.push(pv)) {
                // No room on the free list so really free it.
                DeleteElement(pv);
            }
            else {
#ifdef DEBUG
                InterlockedIncrement((PLONG) &m_dwOnStack);
#endif
            }
        }
        else {
#ifdef DEBUG
            InterlockedIncrement((PLONG) &m_dwInCache);
#endif
        }
    }

    
protected:

    void init(size_t cbElement) {
        memset(m_rgpvCache, 0, sizeof(m_rgpvCache));
        m_cbElement = cbElement;

#ifdef DEBUG
        m_dwOnStack = 0;
        m_dwInCache = 0;
        m_dwSatisfiedFromCache = 0;
        m_dwSatisfiedFromStack = 0;
        m_dwSatisfiedTotal = 0;
        m_dwTotalOutstanding = 0;
#endif
    }

    // Really "free" the element
    void
    DeleteElement(
        void* pv
        )
    {
        assert(pv);

        _ElementFuncs::destroy(pv);
        m_alloc.deallocate(pv, m_cbElement);
#ifdef DEBUG
        InterlockedDecrement((PLONG) &m_dwTotalOutstanding);
#endif
    }

    void
    PopulateCache(
        )
    {
        void *pv = NULL;
        
        for (size_t i = 0; i < _CACHE_SIZE; ++i) {
            if (pv == NULL) {
                if (m_stack.pop(pv)) {
                    assert(pv);
#ifdef DEBUG
                    InterlockedDecrement((PLONG) &m_dwOnStack);
#endif
                }
                else {
                    // No more elements on the free list
                    assert(pv == NULL);
                    break;
                }
            }

            if (m_rgpvCache[i] == NULL) {
                pv = (void*) InterlockedExchange((PLONG) &m_rgpvCache[i], (LONG) pv);

#ifdef DEBUG
                if (pv == NULL) {
                    InterlockedIncrement((PLONG) &m_dwInCache);
                }
#endif
            }
        }

        if (pv) {
            // There was no slot for this item. Put it back on the free list.
            if (!m_stack.push(pv)) {
                // No room on the free list so really free it.
                DeleteElement(pv);
            }
            else {
#ifdef DEBUG
                InterlockedIncrement((PLONG) &m_dwOnStack);
#endif
            }
        }
    }

    _Al m_alloc;
    _FreeStack           m_stack;  // Stack of free elements
    ce::critical_section    m_csPopProtector;   // Serializes accesses to "pop"
    
    void* m_rgpvCache[_CACHE_SIZE];

    size_t m_cbElement;

#ifdef DEBUG
    DWORD m_dwOnStack;
    DWORD m_dwInCache;
    DWORD m_dwSatisfiedFromCache;
    DWORD m_dwSatisfiedFromStack;
    DWORD m_dwSatisfiedTotal;
    DWORD m_dwTotalOutstanding;
#endif
};


#include "transfer.h"

// Set of functions which do initialization and checking on CFreePool transfers
struct transfer_funcs
{
    static void create(void *pv) {
        PREFAST_ASSERT(pv);
        PCUfnMddTransfer pTransfer = new (pv) CUfnMddTransfer;
    }

    static void free(void *pv) {
        PREFAST_ASSERT(pv);
        PCUfnMddTransfer pTransfer = (PCUfnMddTransfer) pv;
        pTransfer->MarkAsFree();
    }
    
    static void reuse(void *pv) {
        PREFAST_ASSERT(pv);
        PCUfnMddTransfer pTransfer = (PCUfnMddTransfer) pv;
        pTransfer->MarkAsValid();
    }
    
    static void destroy(void *pv) {
        PREFAST_ASSERT(pv);
        PCUfnMddTransfer pTransfer = (PCUfnMddTransfer) pv;
        pTransfer->~CUfnMddTransfer();
    }
};


#ifdef DEBUG
#define TRANSFER_CACHE_SIZE 1
#else
#define TRANSFER_CACHE_SIZE 4
#endif

#define TRANSFER_LIST_SIZE  8

// Utilizes CFreePool for fast access to pre-initialized transfer structures.
class CFreeTransferList
{
public:
    typedef CFreePool<TRANSFER_CACHE_SIZE, ce::fixed_block_allocator<TRANSFER_LIST_SIZE>, transfer_funcs> _MyFreePool;
    
    CFreeTransferList() : m_FreePool(sizeof(CUfnMddTransfer)) {}
    ~CFreeTransferList() {}

    // 
    PCUfnMddTransfer AllocTransfer() {
        return (PCUfnMddTransfer) m_FreePool.allocate(sizeof(CUfnMddTransfer));
    }

    void FreeTransfer(
        PCUfnMddTransfer pTransfer
        )
    {
        assert(pTransfer);

        m_FreePool.deallocate(pTransfer, sizeof(CUfnMddTransfer));
    }

    // This is a slow operation.
    void Compact(
        )
    {
        m_FreePool.compact();
    }

protected:
    _MyFreePool m_FreePool;
};

#pragma warning(pop)
#endif // _XFERLIST_H_

