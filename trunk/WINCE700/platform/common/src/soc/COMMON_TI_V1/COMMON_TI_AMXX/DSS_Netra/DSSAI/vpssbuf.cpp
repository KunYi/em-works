/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
* This file has utility functions to manage memory area shared between ARM
* and VPSS-M3. The shared memory is expected to be in uncached memory.
* The routines rely on the heap implementation. With slight modification
* the function can be used to manage memory in memory area other than the
* shared memory area as well (need addition of a context).
* 
================================================================================
*/


#include "omap.h"
#include "heap.h"
#include "vpss.h"
#include "vpssbuf.h"
#include "am38xx_display_cfg.h"

//
//  Globals
//
HANDLE      hSbufHeap;
DWORD       dwSbufSize;
DWORD       dwSbufPhyAddr;
VOID*       pSbufVirtBuf;

/**
 * \brief vps_sbuf_alloc
 *        This function is called to allocate shared memory buffer
 *
 * \param   pinfo->size   [IN]  Ptr to info structure. Only size is input parameter
 * \param   pinfo->paddr  [OUT] Physical address of allocated buffer
 * \param   pinfo->vaddr  [OUT] Virtual address of allocated buffer
 * \param   pinfo->handle [OUT] Handle of allocated buffer
 *
 * \return  int. 0 if success, 1 otherwise
 */
int vps_sbuf_alloc(struct vps_payload_info *pinfo)
{
    Heap    *pSbufHeap = (Heap*) hSbufHeap;
    Heap    *pHeap = NULL;
    int     result = 1;

    if (pinfo->size == 0)
    {
        pinfo->vaddr = NULL;
        return result;
    }

    // Align size to SBUF_MEM_ALIGN byte boundry 
    pinfo->allocsize = ((pinfo->size + VPSS_SHARED_MEM_ALIGN - 1)/VPSS_SHARED_MEM_ALIGN) * VPSS_SHARED_MEM_ALIGN;

    //  Allocate the memory from the given shared memory heap
    pHeap = pSbufHeap->Allocate( pinfo->allocsize );
    if( pHeap == NULL )
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: Unable to allocate heap memory\r\n"));
        pinfo->vaddr = NULL;
        pinfo->paddr = 0u;
        goto cleanUp;
    }

    pinfo->handle = (DWORD)pHeap;
    pinfo->paddr = dwSbufPhyAddr + (DWORD)(pHeap->Address() - pSbufHeap->Address());
    pinfo->vaddr = (void *)pHeap->Address();

#ifdef DEBUG
    RETAILMSG(1, (L"vps shared buf alloc. Paddr=0x%x, vaddr=0x%x, handle=0x%x, size=0x%x, allocsize=0x%x\r\n", 
                (DWORD)pinfo->paddr, (DWORD)pinfo->vaddr, (DWORD)pinfo->handle, pinfo->size, pinfo->allocsize));
#endif

    result = 0;

cleanUp:
    //  Return
    return result;
}

/**
 * \brief vps_sbuf_free
 *        This function is called to free shared memory buffer
 *
 * \param   pinfo   [IN]  Ptr to info structure.
 *
 * \return  0 if success, 1 otherwise
 */
int vps_sbuf_free(struct vps_payload_info *pinfo)
{
    Heap    *pHeap = (Heap*) pinfo->handle;

    if( pHeap )
        pHeap->Free();

#ifdef DEBUG
    RETAILMSG(1, (L"vps shared buf free. Paddr=0x%x, vaddr=0x%x, handle=0x%x, size=0x%x, allocsize=0x%x\r\n", 
                (DWORD)pinfo->paddr, (DWORD)pinfo->vaddr, (DWORD)pinfo->handle, pinfo->size, pinfo->allocsize));
#endif

    pinfo->paddr = 0;
    pinfo->vaddr = 0;
    pinfo->handle = 0;
    pinfo->allocsize = 0;

    return 0;
}

/**
 * \brief vps_sbuf_init
 *        This function is called to map/initialize shared memory area.
 *
 * \param   None
 *
 * \return  0 if success, 1 otherwise
 */
int vps_sbuf_init()
{
    int     bResult = 1;    // default Fail
    Heap*   pSbufHeap;

    Display_GetVPSSSharedMemory( &dwSbufSize, &dwSbufPhyAddr );
    if( !dwSbufPhyAddr || !dwSbufSize )
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: Unable to get vpss shared memory addr and/or size\r\n"));
        goto cleanUp;
    }

    //  Map physical memory to VM
    pSbufVirtBuf = VirtualAlloc(0, dwSbufSize, MEM_RESERVE, PAGE_NOACCESS);
    if( !pSbufVirtBuf )
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: Unable to allocate sbuf virtual buffer\r\n"));
        goto cleanUp;
    }

    if( !VirtualCopy(pSbufVirtBuf, (void *)(dwSbufPhyAddr >> 8), dwSbufSize, PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: Unable to map display buffer physical memory\r\n"));
        VirtualFree( pSbufVirtBuf, 0, MEM_RELEASE );
        pSbufVirtBuf = NULL;
        goto cleanUp;
    }

    //  Change the attributes of the buffer for cache write combine
    if( !CeSetMemoryAttributes(pSbufVirtBuf, (void *)(dwSbufPhyAddr >> 8), dwSbufSize, PAGE_WRITECOMBINE))
    {
//        DEBUGMSG(ZONE_ERROR, (L"ERROR: Failed CeSetMemoryAttributes for Shared mem buffer\r\n"));
//        VirtualFree( pSbufVirtBuf, 0, MEM_RELEASE );
//        pSbufVirtBuf = NULL;
//        goto cleanUp;
    }

    // memset the allocated memory
    memset(pSbufVirtBuf, 0, dwSbufSize);

    //  Initialize the heap manager for the shared buf memory
    pSbufHeap = new Heap(dwSbufSize, (DWORD)pSbufVirtBuf);
    if( pSbufHeap == NULL )
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: Unable to create heap manager\r\n"));
        VirtualFree( pSbufVirtBuf, 0, MEM_RELEASE );
        pSbufVirtBuf = NULL;
        goto cleanUp;
    }

    hSbufHeap = (HANDLE) pSbufHeap;
    bResult = 0;

#ifdef DEBUG
    RETAILMSG(1, (L"vps_sbuf_init. SharedBufferPhyAddr = 0x%x, SharedBufSize=0x%x, SharedBufVirtAddr= 0x%x\r\n", 
                    dwSbufPhyAddr, dwSbufSize, pSbufVirtBuf));
#endif

cleanUp:
    //  Return result
    return bResult;
}

/**
 * \brief vps_sbuf_deinit
 *        This function is called to unmap/uninitialize shared memory area.
 *
 * \param   None
 *
 * \return  0 if success, 1 otherwise
 */
int vps_sbuf_deinit(void)
{
    Heap*   pSbufHeap = (Heap*) hSbufHeap;

    //  Free the heap manager
    if( pSbufHeap )
        pSbufHeap->Free();

    //  Free memory
    if( pSbufVirtBuf ) 
        VirtualFree( pSbufVirtBuf, 0, MEM_RELEASE );

    return 0;
}
