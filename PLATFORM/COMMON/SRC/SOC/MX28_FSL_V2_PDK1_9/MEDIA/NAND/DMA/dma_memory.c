//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dma_memory.c
//
//  This file contains memory management for NAND DMA
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#ifdef IN_BOOTLOADER
#include <oal.h>
#include "image_cfg.h"
#endif
#include "nand_dma.h"
#include "nand_hal.h"
#pragma warning(pop)
#include "dma_descriptor.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

NAND_dma_read_t            *DmaReadDescriptor;
NAND_dma_block_erase_t     *DmaEraseBlockDescriptor;
NAND_dma_program_t         *DmaProgramDescriptor;
NAND_dma_generic_struct_t  *DmaGenericDescriptor;

nand_mem_map_t            NandMemMap[MAX_NAND_MEM_MAP];
UINT32                    NandMemMapSize = 0;

static DWORD gNANDBufferPtr = 0;

// OCRAM address taken for NAND device
// Please not that, OCRAM physical address is taken from header file bsp_cfg.h
// which defines address list for OCRAM, in our case _PA_MEM_BASE_NAND_FLASH
//#define PA_NAND_DESCP_BUFFER    (16*1024)
//#define MEM_SIZE_NAND_FLASH     (12*1024)

#ifdef IN_BOOTLOADER
PBYTE pNANDBuffer = (BYTE*)IMAGE_WINCE_NAND_IRAM_PA_START;  //(BYTE*)PA_NAND_DESCP_BUFFER;
#endif

ULONG DMA_MemAlloc(ULONG size)
{
    if (NandMemMapSize >= MAX_NAND_MEM_MAP)
    {
        ERRORMSG(1, (_T("DMA_MemAlloc.. No more room in mapping table!\r\n")));
        return ((ULONG)NULL);
    }
#ifdef IN_BOOTLOADER
    {
        ULONG m_pVir,m_pPhy;
        if(gNANDBufferPtr < IMAGE_WINCE_NAND_IRAM_SIZE)
        {
            if(size%4 < 4)
               size+= 4 - (size%4);

            gNANDBufferPtr += size;        
            pNANDBuffer += gNANDBufferPtr;
        
            m_pPhy = (DWORD)pNANDBuffer;
            m_pVir = (ULONG)OALPAtoUA(m_pPhy);    
            if(m_pVir == (ULONG)NULL)
            {
              ERRORMSG(1, (TEXT("Allocate NAND buffer fail\r\n")));
            }

            NandMemMap[NandMemMapSize].size = size;
            NandMemMap[NandMemMapSize].phys = m_pVir;
            NandMemMap[NandMemMapSize].virt = m_pPhy;
        }
    }
#else //BOOTLOADER
    {
        NandMemMap[NandMemMapSize].size = size;
        NandMemMap[NandMemMapSize].phys = (ULONG)AllocPhysMem(NandMemMap[NandMemMapSize].size,
                                                              PAGE_READWRITE|PAGE_NOCACHE,
                                                              0,
                                                              0,
                                                              (VOID *)&NandMemMap[NandMemMapSize].virt);
    }
#endif 

    if ((ULONG)NULL != NandMemMap[NandMemMapSize].phys)
    {
              RETAILMSG(0, (TEXT(" DMA_MemAlloc: 0x%x(size:0x%x) -> 0x%x\r\n"),

            NandMemMap[NandMemMapSize].phys,
            NandMemMap[NandMemMapSize].size,
            NandMemMap[NandMemMapSize].virt));

               return (NandMemMap[NandMemMapSize++].phys);
    }
    else
    {
        NandMemMap[NandMemMapSize].phys    = (ULONG)NULL;
        NandMemMap[NandMemMapSize].virt    = (ULONG)NULL;
        NandMemMap[NandMemMapSize].size    = 0;

        RETAILMSG(0, (TEXT("-DMA_MemAlloc(NULL)\r\n")));
        return ((ULONG)NULL);
    }
}

VOID
DMA_MemFree(ULONG phys_addr)
{
    UINT idx;

    if (NandMemMapSize == 0) {
        // No mappings
        RETAILMSG(0, (TEXT("-DMA_MemFree() - NoMappings\r\n")));
        return;
    }

    for (idx=0; idx<NandMemMapSize; idx++) {
        if (NandMemMap[idx].phys == phys_addr) {
            if (NandMemMapSize > 1) {
                // Multiple mappings, copy last
                // mapping to this position.
                //FreePhysMem((VOID*)NandMemMap[idx].phys);
                NandMemMap[idx].phys = (ULONG)NULL;
                NandMemMapSize--;
                NandMemMap[idx].phys = NandMemMap[NandMemMapSize].phys;
                NandMemMap[idx].virt = NandMemMap[NandMemMapSize].virt;;
                NandMemMap[idx].size = NandMemMap[NandMemMapSize].size;;
                NandMemMap[NandMemMapSize].phys = (ULONG)NULL;
                NandMemMap[NandMemMapSize].virt = (ULONG)NULL;
                NandMemMap[NandMemMapSize].size = 0;
            } else {
                // only one mapping, null table
                //FreePhysMem((VOID*)NandMemMap[idx].phys);
                NandMemMap[idx].phys = (ULONG)NULL;
                NandMemMap[idx].virt = (ULONG)NULL;
                NandMemMap[idx].size = 0;
                NandMemMapSize = 0;
            }
        }
    }
    RETAILMSG(0, (TEXT("-DMA_MemFree()\r\n")));
}

VOID*
DMA_MemTransfer(VOID* virt_addr_t)
{
    UINT idx;

    ULONG virt_addr = (ULONG)virt_addr_t;

    RETAILMSG(0, (TEXT("+DMA_MemTransfer(%x)\r\n"), virt_addr));

    if (virt_addr == (ULONG)NULL) {
        // NULL - Required no mapping, Its NULL
        RETAILMSG(0, (TEXT("-DMA_MemTransfer(NULL)\r\n")));
        return NULL;
    }

    if (NandMemMapSize == 0) {
        // No mappings
        RETAILMSG(0, (TEXT("-DMA_MemTransfer() - NoMappings\r\n")));
        return NULL;
    }

    for (idx=0; idx<NandMemMapSize; idx++) {
        if ((virt_addr >= NandMemMap[idx].phys) &&
            (virt_addr < (NandMemMap[idx].phys + NandMemMap[idx].size))) {
            
            RETAILMSG(0, (TEXT(" DMA_MemTransfer: 0x%x(0x%x) -> 0x%x = 0x%x\r\n"),
                virt_addr,
                virt_addr - NandMemMap[idx].phys,
                NandMemMap[idx].virt,
                NandMemMap[idx].virt + (virt_addr - NandMemMap[idx].phys)));
                
            RETAILMSG(0, (TEXT("-DMA_MemTransfer(%x)\r\n"),
                NandMemMap[idx].virt + (virt_addr - NandMemMap[idx].phys)));
            return (VOID*)(NandMemMap[idx].virt + (virt_addr - NandMemMap[idx].phys));
        }
    }

    RETAILMSG(0, (TEXT("-DMA_MemTransfer(NULL)\r\n")));
    return (NULL);
}
