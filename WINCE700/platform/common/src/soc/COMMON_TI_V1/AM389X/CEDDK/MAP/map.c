/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//------------------------------------------------------------------------------
//
//  File:  map.c
//
//  This file contains DDK library implementation for IO space mapping
//  functions. For OMAP we choose to use one common mapping for all
//  on chip devices to reduce number of mapping (usually each device
//  will create its own mapping which cause unnecessary TLB missises).
//  
#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>
#include <am389x.h>
#include "map.h"

#pragma warning(push)
#pragma warning(disable: 4213)

#pragma data_seg(".SHARED")

static MEMORY_REGISTER_ENTRY _pRegEntries[] = {
	{0x8F000000,  0x1000000, NULL }, // 16MB uncached memory for device buffers 
	{0x48000000,  0x0400000, NULL }, // Slow Periferal domain L4
	{0x4A000000,  0x1000000, NULL }, // Fast Periferal domain L4
	{0x40300000,  0x0200000, NULL }, // OCMC SRAM
	{0x08000000,  0x1000000, NULL }, // NAND Flash SC0  
	{0x46000000,  0x1000000, NULL }, // McASP & HDMI  
	{0x47000000,  0x0800000, NULL }, // McBSP & USB/CPPI  
	{0x49000000,  0x0100000, NULL }, // TPCC  
	{0x49800000,  0x0400000, NULL }, // TPTC  
	{0x55000000,  0x1000000, NULL }, // Ducati
	{0x56000000,  0x1000000, NULL }, // SGX530
	{0x58000000,  0x1000000, NULL }, // IVAHD0 host port
	{0x59000000,  0x1000000, NULL }, // IVAHD0 SL2 port
	{0x5A000000,  0x1000000, NULL }, // IVAHD1 host port
	{0x5B000000,  0x1000000, NULL }, // IVAHD1 SL2 port
	{0x53000000,  0x1000000, NULL }, // IVAHD2 host port
	{0x54000000,  0x1000000, NULL }, // IVAHD2 SL2 port
	{0x90000000,  0x9800000, NULL }, // 152MB for Display. Syslink shared mem, M3 code/data
	{0xB8000000,  0x0400000, NULL }, // 4MB for Display. VPSS shared mem, vpdma desc mem
	{0,           0,         NULL }
};

static  BOOL    _bMapRegisters = FALSE;

#pragma data_seg()
#pragma comment(linker,"/section:.SHARED,RWS")

//------------------------------------------------------------------------------
VOID* MmMapIoSpace(PHYSICAL_ADDRESS PhysAddr, ULONG size, BOOLEAN cacheEnable)
{
    MEMORY_REGISTER_ENTRY *pRegEntry = _pRegEntries;

	VOID   *pAddress = NULL;
    UINT32 phSource;
    UINT32 sourceSize, offset;
	BOOL   rc;

    //  Initialize the list of registers above with kernel mappings
    if( _bMapRegisters == FALSE ){
        MEMORY_REGISTER_ENTRY *pRegMap = _pRegEntries;
        while (pRegMap->dwSize > 0){
            //  Have kernel map the memory section
            rc = KernelIoControl( IOCTL_HAL_PHYS_TO_VIRT, &(pRegMap->dwStart),
				                  sizeof(DWORD), &pRegMap->pv, sizeof(DWORD), NULL );
            if( rc == FALSE ){
                DEBUGMSG(TRUE, (L"ERROR: MmMapIoSpace failed to map registers memory\r\n"));
                pRegMap->pv = NULL;
                goto cleanUp;
            }
			++pRegMap;
        }
        _bMapRegisters = TRUE;
    }

    if (cacheEnable == FALSE && PhysAddr.HighPart == 0){
        while (pRegEntry->dwSize > 0){
            if (pRegEntry->dwStart <= PhysAddr.LowPart &&
                (pRegEntry->dwStart + pRegEntry->dwSize) > PhysAddr.LowPart){
                // Calculate offset
				if (pRegEntry->pv == NULL){
					RETAILMSG(1,(L"!!!! MmMapIoSpace PA %08x doesn't have VA\r\n", pRegEntry->dwStart)); 				
					break;
				}	
                offset = PhysAddr.LowPart - pRegEntry->dwStart;
                (UINT32)pAddress = (UINT32)pRegEntry->pv + offset;
				rc = TRUE;
				break;
			}
            ++pRegEntry;
		}
    }

	if (pAddress == NULL){
        phSource = PhysAddr.LowPart & ~(PAGE_SIZE - 1);
        sourceSize = size + (PhysAddr.LowPart & (PAGE_SIZE - 1));

		pAddress = VirtualAlloc(0, sourceSize, MEM_RESERVE, PAGE_NOACCESS);
		if (pAddress == NULL){
			DEBUGMSG(1, (L"ERROR: MmMapIoSpace failed reserve memory\r\n"));
			goto cleanUp;
		}   
	    
		rc = VirtualCopy(pAddress, (PVOID)(phSource >> 8), sourceSize,
				PAGE_PHYSICAL|PAGE_READWRITE|(cacheEnable ? 0 : PAGE_NOCACHE));

		if (!rc){
			DEBUGMSG(1, (L"ERROR: MmMapIoSpace failed allocate memory\r\n"));
			VirtualFree(pAddress, 0, MEM_RELEASE);
			pAddress = NULL;
			goto cleanUp;
		}
		pAddress = (PVOID)((UINT32)pAddress + (PhysAddr.LowPart & (PAGE_SIZE - 1)));
	}

cleanUp:
	return pAddress;
}


//------------------------------------------------------------------------------
//
//  Function:  MmUnmapIoSpace
//
//  Unmap a specified range of physical addresses previously mapped by
//  MmMapIoSpace
//
VOID MmUnmapIoSpace( VOID *pAddress, ULONG size )
{
    BOOL                   bFree = TRUE;
    MEMORY_REGISTER_ENTRY *pRegEntry = _pRegEntries;

    // We unmap only in case that memory wasn't mapped from common mapping.
    while (pRegEntry->dwSize > 0){
        if (pRegEntry->pv && (DWORD)pAddress >= (DWORD)pRegEntry->pv &&
            (DWORD)pAddress < ((DWORD)pRegEntry->pv + pRegEntry->dwSize)) {
            bFree = FALSE;
            break;
        }
        ++pRegEntry;
    }

    if (bFree == TRUE){
        VirtualFree((VOID*)((UINT32)pAddress & ~(PAGE_SIZE - 1)), 0, MEM_RELEASE);
    }
}


//------------------------------------------------------------------------------
//
//  Function:  TransBusAddrToVirtual
//
BOOL
TransBusAddrToVirtual(
    INTERFACE_TYPE ifcType,
    ULONG busNumber,
    PHYSICAL_ADDRESS busAddress,
    ULONG length, 
    ULONG *pAddressSpace,
    VOID **ppMappedAddress
    )
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS PhysAddr; 

    if (!HalTranslateBusAddress(
        ifcType, busNumber, busAddress, pAddressSpace, &PhysAddr
        ))
        {
        goto cleanUp;
        }

    switch (*pAddressSpace)
        {
        case 0:
            // Memory-mapped I/O, get virtual address for translated address
            *ppMappedAddress = MmMapIoSpace(PhysAddr, length, FALSE);
            if (*ppMappedAddress != NULL) rc = TRUE;
            break;
        case 1:        
            // I/O port
            *ppMappedAddress = (VOID*)PhysAddr.LowPart;
            rc = TRUE;
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  TransBusAddrToVirtual
//
BOOL
TransBusAddrToStatic(
    INTERFACE_TYPE ifcType,
    ULONG busNumber,
    PHYSICAL_ADDRESS busAddress,
    ULONG length,
    ULONG *pAddressSpace,
    VOID **ppAddress
    )
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS PhysAddr;
    UINT64 address;
    UINT32 size;

    if (!HalTranslateBusAddress(
        ifcType, busNumber, busAddress, pAddressSpace, &PhysAddr
        ))
        {
        goto cleanUp;
        }
    switch (*pAddressSpace)
        {
        case 0:        
            // Memory-mapped I/O, get statically-mapped virtual address
            // for translated physical address
            address = PhysAddr.QuadPart & ~(PAGE_SIZE - 1);
            size = length + (PhysAddr.LowPart & (PAGE_SIZE - 1));
            *ppAddress = CreateStaticMapping((UINT32)(address >> 8), size);
            if (*ppAddress != NULL)
                {
                rc = TRUE;
                // Adjust with offset from page
                (UINT32)*ppAddress += PhysAddr.LowPart & (PAGE_SIZE - 1);
                }
            break;
        case 1:        
            // I/O port
            *ppAddress = (VOID*)PhysAddr.LowPart;
            rc = TRUE;
            break;
        }

cleanUp:
    return rc;
}

#pragma warning(pop)
//------------------------------------------------------------------------------

