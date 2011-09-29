//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cspata.cpp
//
//  Provides SoC-specific configuration routines for
//  the ATA common driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"

//-----------------------------------------------------------------------------
// Global Variables
DWORD           DMAMode; // 0 for SDMA, 1 for ADMA
DWORD           dwSysIntr; // SysIntr associated with ATA channel

// (SDMA support)
UINT8           DmaChanATARx, DmaChanATATx; 
DDK_DMA_REQ     DmaReqTx; 
DDK_DMA_REQ     DmaReqRx; 
UINT8           chan;
// (ADMA support)
PHYSICAL_ADDRESS    PhysADMATableAddr;
PVOID               pVirtADMATableAddr;
DWORD*              pADMATable;

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// External Functions
extern UINT8 BSPATASDMAchannelprio();

//-----------------------------------------------------------------------------
// Local Variables
DWORD dwADMAFIFOAlarm = 32;

//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function:  ATAGetBaseRegAddr
//
// This function returns the physical base address for the
// ATA registers.
//
// Parameters:
//      None.
//
// Returns:
//      Physical base address for ATA registers.
//
//-----------------------------------------------------------------------------
UINT32 CSPATAGetBaseRegAddr(void)
{
    UINT32 siRev;

    // Ideally, this code should be put in some init function, and
    // we add it here because all init functions are in common code.
    if (!KernelIoControl(IOCTL_HAL_QUERY_SI_VERSION, NULL, 0, 
        &siRev, sizeof(siRev), NULL))
    {
        ERRORMSG(1, (_T("ATA: query silicon revision failed!")));
    }
    if (siRev == DDK_SI_REV_TO1)
    {
        dwADMAFIFOAlarm = 8;
    }
    
    return CSP_BASE_REG_PA_ATA;
}

//------------------------------------------------------------------------------
//
//     Function: CSPATAEnableClock
//
//            This function enable/disable ATA clock
//
//     Parameters:
//            bEnable
//            [in]    TRUE if ATA Clock is to be enabled. FALSE if ATA Clock is
//                  to be disabled.
//
//     Returns:
//            TRUE if successfully performed the required action.
//
//------------------------------------------------------------------------------
BOOL CSPATAEnableClock(BOOL bEnable)
{
    if (bEnable)
        return DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_ATA, 
            DDK_CLOCK_GATE_MODE_ENABLED_ALL);
    else
        return DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_ATA, 
            DDK_CLOCK_GATE_MODE_DISABLED);
}

int CSPATACLOCKPERIOD()
{
    DEBUGMSG(1, (_T("CLOCK_PERIOD %d\r\n"), CLOCK_PERIOD));
    return CLOCK_PERIOD; // for 133MHz
}


/// Register
// Common
DWORD CSPATARegIntEnable()
{
    DWORD dIntEnable = 0;
    switch(DMAMode)
    {
        case 0: // SDMA
            dIntEnable= 0x80;
        break;
        case 1: // ADMA
            dIntEnable= 0x8c;
        break;
        default:
            DEBUGMSG(1, (_T("Undefined DMA mode\r\n")));
        break;
    }
    return dIntEnable;
}

DWORD CSPATARegControl(BYTE bTransferMode, BOOL fRead)
{
    DWORD dControl = 0;

    switch(bTransferMode)
    {
        case 0x20:  // MDMA_MODE:
        case 0x21:
        case 0x22:           
             switch(DMAMode)
             {
                 case 0: // SDMA
                     dControl= fRead ? 0xd8 : 0xea;
                 break;
                 case 1: // ADMA
                     dControl= fRead ? 0x7d8 : 0x7ea;
                 break;
                 default:
                     DEBUGMSG(1, (_T("Undefined DMA mode\r\n")));
                 break;
             }  
             break;
        case 0x40:  // UDMA_MODE:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:           
             switch(DMAMode)
             {
                 case 0: // SDMA
                     dControl= fRead ? 0xdc : 0xee;
                 break;
                 case 1: // ADMA
                     dControl= fRead ? 0x7dc : 0x7ee;
                 break;
                 default:
                     DEBUGMSG(1, (_T("Undefined DMA mode\r\n")));
                 break;
             }          
             break;
    }
    return dControl;
}

DWORD CSPATARegFIFOAlarm()
{
    DWORD dFIFOAlarm = 0;
    switch(DMAMode)
    {
        case 0: // SDMA
            dFIFOAlarm= ATA_DMA_WATERMARK/2;
        break;
        case 1: // ADMA
            dFIFOAlarm= dwADMAFIFOAlarm;
        break;
        default:
            DEBUGMSG(1, (_T("Undefined DMA mode\r\n")));
        break;
    }
    return dFIFOAlarm;
}

// ADMA
DWORD CSPATARegTablePhy()
{
    DWORD dwtablephy;
    dwtablephy = PhysADMATableAddr.LowPart;
    return dwtablephy;
}
/// End of register


/// Int
// ATA int
//-----------------------------------------------------------------------------
//
// Function:  ATAGetIRQ
//
// This function returns the IRQ number for the ATA.
//
// Parameters:
//      None.
//
// Returns:
//      IUP IRQ number.
//
//-----------------------------------------------------------------------------
DWORD CSPATAGetIRQ(void)
{
    return IRQ_ATA;
}

// DMA int
DWORD CSPATAGetDMACHIRQ(DWORD DmaChanATA)
{
    DWORD dDMAChIRQ = 0;
    switch(DMAMode)
    {
        case 0: // SDMA
            dDMAChIRQ= IRQ_SDMA_CH0 + DmaChanATA;
        break;
        case 1: // ADMA
        break;
        default:
            DEBUGMSG(1, (_T("Undefined DMA mode\r\n")));
        break;
    }
    return dDMAChIRQ;
}

BOOL CSPATAConfigInt(HANDLE hIRQEvent)
{
    BOOL bRet = TRUE;
    DWORD dwIRQ;

    DEBUGMSG(1,(_T("CSPATAConfigInt+\r\n")));
    switch(DMAMode)
    {
        case 0: // SDMA
            DWORD dwIRQ_DMACHRX;
            DWORD dwIRQ_DMACHTX;
            
            dwIRQ = CSPATAGetIRQ();
            dwIRQ_DMACHRX = CSPATAGetDMACHIRQ(DmaChanATARx);
            dwIRQ_DMACHTX = CSPATAGetDMACHIRQ(DmaChanATATx);

            // Register three IRQs with one SYSINTR
            UINT32 aIrqs[5];
            aIrqs[0] = (UINT32) -1;
            aIrqs[1] = 0;
            aIrqs[2] = dwIRQ;
            aIrqs[3] = dwIRQ_DMACHRX;
            aIrqs[4] = dwIRQ_DMACHTX;
            KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(aIrqs),
                &dwSysIntr, 
                sizeof(dwSysIntr), NULL);
            if (!InterruptInitialize(
                dwSysIntr,
                hIRQEvent,
                NULL,
                0)
            ) {
                goto cleanup;
            }
        break;
        case 1: // ADMA
            dwIRQ = CSPATAGetIRQ();
            // Translate IRQ to SYSINTR
            // Request SYSINTR from OAL
            // ATA irq
            if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &(dwIRQ), sizeof(dwIRQ), 
                &(dwSysIntr), sizeof(dwSysIntr), NULL))
            {
                ERRORMSG(1,(TEXT("%s: Request SYSINTR failed\r\n"), __WFUNCTION__));
                goto cleanup;
            }        
            // associate interrupt event with IRQ
            if (!InterruptInitialize(
                dwSysIntr,
                hIRQEvent,
                NULL,
                0)
            ) {
                goto cleanup;
            }
        break;
        default:
            DEBUGMSG(1, (_T("CSPATADMAStart undefined DMA mode\r\n")));
        break;
    }

    bRet = TRUE;
cleanup:        
    DEBUGMSG(1,(_T("CSPATAConfigInt-\r\n")));
    return bRet;
}

void CSPATASYSINTRInit(void)
{
    dwSysIntr = SYSINTR_NOP;
}

void CSPATASYSINTRDisable(void)
{
    if (dwSysIntr != SYSINTR_NOP) {
        InterruptDisable(dwSysIntr);
    }
}

void CSPATASYSINTRDone(void)
{
    InterruptDone(dwSysIntr);
}
/// End of int


/// DMA
// Value
DWORD CSPATAMaxSectDMA()
{
    DWORD dwMaxSectDMA = 0;
    switch(DMAMode)
    {
        case 0: // SDMA
            dwMaxSectDMA = MAX_SECT_PER_SDMA;
        break;
        case 1: // ADMA
            dwMaxSectDMA = MAX_SECT_PER_ADMA;
        break;
        default:
            DEBUGMSG(1, (_T("CSPATADMAStart undefined DMA mode\r\n")));
        break;
    }
    return dwMaxSectDMA;
}

// SDMA init
//------------------------------------------------------------------------------
//
// Function: DeinitChannelDMA
//
//  This function deinitializes the DMA channel for output.
//
// Parameters:
//        pController[in] - hardware context
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CSPATADeinitChannelDMA(void)
{    
   if (DmaChanATARx != 0)
   {
       DDKSdmaCloseChan(DmaChanATARx);
       DmaChanATARx = 0;
   }
   if (DmaChanATATx != 0)
   {
       DDKSdmaCloseChan(DmaChanATATx);
       DmaChanATATx = 0;
   }
   return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: InitChannelDMA
//
//  This function initializes the DMA channel for output.
//
// Parameters:
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CSPATAInitChannelDMA(void)
{
    BOOL rc = FALSE;

    DEBUGMSG(1,(_T("CMXDisk::InitChannelDMA+\r\n")));

    DmaReqTx = DDK_DMA_REQ_ATA_TX ; 
    DmaReqRx = DDK_DMA_REQ_ATA_RX ; 

    // Open virtual DMA channels 
    DmaChanATARx = DDKSdmaOpenChan(DmaReqRx, BSPATASDMAchannelprio());
    DEBUGMSG(1,(_T("Channel Allocated(Rx) : %d\r\n"),DmaChanATARx));
    if (!DmaChanATARx)
    {
        DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Rx): SdmaOpenChan for input failed.\r\n")));
        goto cleanUp;
    }

    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(DmaChanATARx, ATA_MAX_SDMADESC_COUNT))
    {
        DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaAllocChain for input failed.\r\n")));
        goto cleanUp;
    }  

    // Initialize the chain and set the watermark level     
    if (!DDKSdmaInitChain(DmaChanATARx, ATA_DMA_WATERMARK))
    {
        DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    // Open virtual DMA channels 
    DmaChanATATx = DDKSdmaOpenChan(DmaReqTx, BSPATASDMAchannelprio());
    DEBUGMSG(1,(_T("Channel Allocated(Tx) : %d\r\n"),DmaChanATATx));
    if (!DmaChanATATx)
    {
        DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Tx): SdmaOpenChan(Rx) for input failed.\r\n")));
        goto cleanUp;
    }

    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(DmaChanATATx, ATA_MAX_SDMADESC_COUNT))
    {
         DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaAllocChain for input failed.\r\n")));
         goto cleanUp;
    }  

    // Initialize the chain and set the watermark level 
    if (!DDKSdmaInitChain(DmaChanATATx, ATA_DMA_WATERMARK))
    {
        DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
   if (!rc)
   {
      CSPATADeinitChannelDMA();
   }
   DEBUGMSG(1,(_T("CMXDisk::InitChannelDMA-\r\n")));
   return rc;
}

// ADMA init
//------------------------------------------------------------------------------
//
// Function: UnmapADMATable
//
//  This function unmaps the advanced DMA table previously mapped with the
//  MapADMATable function.
//
// Parameters:
//        pController[in] - hardware context
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CSPATAUnmapADMATable(void)
{
    PHYSICAL_ADDRESS phyAddr;
    DMA_ADAPTER_OBJECT Adapter;
    DEBUGMSG(1,(_T("CSPATAUnmapADMATable+\r\n")));

    // PREFAST
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));

    if(pVirtADMATableAddr)
    {
        // Logical address parameter is ignored
        phyAddr.QuadPart = 0;
        HalFreeCommonBuffer(&Adapter, 0, phyAddr, (PVOID)pVirtADMATableAddr, FALSE);
    }

    DEBUGMSG(1,(_T("CSPATAUnmapADMATable-\r\n")));
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: MapADMATable
//
// Allocate and map advanced DMA table 
//
// Parameters:
//        pController[in] - hardware context
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL CSPATAMapADMATable(void)
{
    DMA_ADAPTER_OBJECT Adapter;
    BOOL bRet = FALSE;
    DEBUGMSG(1,(_T("CSPATAMapADMATable+\r\n")));
      
    pVirtADMATableAddr = NULL;
   
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate a block of virtual memory (physically contiguous) for the ADMA table.
    pVirtADMATableAddr = HalAllocateCommonBuffer(&Adapter, (ATA_MAX_ADMADESC_COUNT*4), &(PhysADMATableAddr), FALSE);

    if (pVirtADMATableAddr == NULL)
    {
        DEBUGMSG(1, (TEXT("CSPATAMapADMATable - Failed to allocate DMA buffer.\r\n")));
        goto cleanup;
    }

    bRet = TRUE;
cleanup:        
    DEBUGMSG(1,(_T("CSPATAMapADMATable-\r\n")));
    return bRet;
}

// Init DMA
BOOL CSPATADMADeInit()
{
    BOOL bRet = FALSE;

    DEBUGMSG(1,(_T("CSPATADMADeInit+\r\n")));
    switch(DMAMode)
    {
        case 0: // SDMA
            if(!CSPATADeinitChannelDMA())
            {
                DEBUGMSG(1, (TEXT("CSPATADMADeInit - Failed to deinitialize DMA.\r\n")));
                goto cleanup;
            }
        break;
        case 1: // ADMA
            if(!CSPATAUnmapADMATable())
            {
                DEBUGMSG(1, (TEXT("CSPATADMADeInit - Failed to Unmap DMA buffers\r\n")));
                goto cleanup;
            }
        break;
        default:
            DEBUGMSG(1, (_T("Undefined DMA mode\r\n")));
        break;
    }
    
    bRet = TRUE;
cleanup:        
    DEBUGMSG(1,(_T("CSPATADMADeInit-\r\n")));
    return bRet;
}


//-----------------------------------------------------------------------------
//
// Function:  SPBAGetBaseRegAddr
//
// This function returns the physical base address for the
// SPBA registers to allow access of SDMA to ATA via SPBA.
//
// Parameters:
//      None.
//
// Returns:
//      Physical base address for SPBA registers.
//
//-----------------------------------------------------------------------------
UINT32 CSPSPBAGetBaseRegAddr(void)
{
    return CSP_BASE_REG_PA_SPBA;
}

//------------------------------------------------------------------------------
//
// Function: CSPATAGetModeFromReg
//
// Gets the current panel type from registry.
//
// Parameters:
//      None.
//
// Returns:
//      Panel type number for initial display panel.
//
//------------------------------------------------------------------------------
BOOL CSPATAGetModeFromReg(VOID)
{
    HKEY  hKey;
    DWORD dwSize;
    BOOL bRet = FALSE;
    DEBUGMSG(1, (TEXT("CSPATAGetModeFromReg+\r\n")));
    
    DMAMode = 0;
    // Open registry key for ATA driver
    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, ATA_REG_PATH, 0 , 0, &hKey))
    {
        DEBUGMSG(1,(TEXT("CSPATAGetModeFromReg: Failed to open reg path:%s\r\n"), ATA_REG_PATH));
        goto cleanup;
    }

    dwSize = sizeof(DMAMode);
    if(ERROR_SUCCESS != RegQueryValueEx(hKey, REG_DMA_MODE, NULL, NULL, (LPBYTE)&DMAMode, (LPDWORD)&dwSize))
    {
        DEBUGMSG(1,
                 (TEXT("CSPATAGetModeFromReg: Failed to get DMAMode\r\n")));
        goto cleanup;
    }

    DEBUGMSG(1, (TEXT("CSPATAGetModeFromReg- DMAMode %d\r\n"), DMAMode));
    bRet = TRUE;
cleanup:
    // Close registry key
    RegCloseKey(hKey);
    DEBUGMSG(1, (TEXT("CSPATAGetModeFromReg-\r\n")));

    return bRet;
}


BOOL CSPATADMAInit()
{
    BOOL bRet = FALSE;

    CSPATAGetModeFromReg();
    switch(DMAMode)
    {
        case 0: // SDMA
            volatile PBYTE m_pSPBAReg;        // Base address of device control register set (MmMapped)
            #define SPBA_PRR8 (m_pSPBAReg+0x20) 

            PHYSICAL_ADDRESS phyAddr;

            phyAddr.QuadPart = CSPSPBAGetBaseRegAddr();
            m_pSPBAReg = (PBYTE) MmMapIoSpace(phyAddr, 0x3c, FALSE);
            if (m_pSPBAReg == NULL)
            {
                DEBUGMSG(1, (TEXT("ATA:  MmMapIoSpace failed!\r\n")));
                goto cleanup;
            }

            // Allow access of SDMA to ATA via SPBA
            SETREG32(SPBA_PRR8, 0x07);  // 0x07 for write

            // Initialize the output DMA
            if (!CSPATAInitChannelDMA())
            {
                DEBUGMSG(1, (TEXT("ATA_MX:InitDMA() - Failed to initialize output DMA.\r\n")));
                goto cleanup;
            }
        break;
        case 1: // ADMA
            // Map the ADMA table into driver's virtual address space
            if(!CSPATAMapADMATable())
            {
                DEBUGMSG(1, (TEXT("ATA_MX:InitDMA() - Failed to map DMA buffers.\r\n")));
                goto cleanup;
            }
        break;
        default:
            DEBUGMSG(1, (_T("CSPATADMAStart undefined DMA mode\r\n")));
            goto cleanup;
        break;
    }
    
    bRet = TRUE;
cleanup:
    
    return bRet;
}

// Setup DMA
BOOL CSPATAPageAlign(DWORD dwPhys,DWORD dwSize,BOOL fRead)
{
    BOOL bAlign = FALSE;           

    switch(DMAMode)
    {
        case 0: // SDMA
            // If we have any unaligned SG pages, perform remainder of transfer
            // using a copy into an aligned buffer.  We require the buffers to
            // be cache line aligned to prevent coherency problems that may
            // result from concurrent accesses to cache line data that is
            // "shared" between the DMA and the CPU
            if (fRead && ((dwSize & CACHE_LINE_SIZE_MASK) || (dwPhys & CACHE_LINE_SIZE_MASK)))
                goto notalign;
            else if(!fRead && ((dwSize & 0x3) || (dwPhys & 0x3)))
                goto notalign;
        break;
        case 1: // ADMA
            if((dwSize & 0x1ff) || (dwPhys & 0xfff))
                goto notalign;
        break;
        default:
            DEBUGMSG(1, (_T("CSPATADMAStart undefined DMA mode\r\n")));
        break;
    }

    bAlign = TRUE;           
notalign:            
    return bAlign;
}

void CSPATADMAAddItem(DWORD dwPhys,DWORD dwSize, DWORD dwDescCount, BOOL bBufferEnd,BOOL fRead)
{
    DWORD dwFlags;

    // if we're at the end to enable the intr flag
    switch(DMAMode)
    {
        case 0: // SDMA
            chan = fRead ? DmaChanATARx : DmaChanATATx;
            if(bBufferEnd)
                dwFlags = DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP;
            else
                dwFlags = DDK_DMA_FLAGS_CONT;

            // add new descriptor to chain
            DDKSdmaSetBufDesc(chan, dwDescCount, dwFlags, dwPhys,
                0, DDK_DMA_ACCESS_32BIT, (WORD)dwSize);
        break;
        case 1: // ADMA
            // set size
            dwFlags = ADMA_ACT_SET | DDK_ADMA_FLAGS_VALID;
            pADMATable = (DWORD*)pVirtADMATableAddr + 2*dwDescCount;
            *pADMATable++ = (DWORD)((dwSize<<12) | dwFlags); // dwsize must be 0x200 (sector) aligned
                
            // set address
            if(bBufferEnd)
                dwFlags = ADMA_ACT_TRAN | DDK_ADMA_FLAGS_VALID | DDK_ADMA_FLAGS_END;
            else
                dwFlags = ADMA_ACT_TRAN | DDK_ADMA_FLAGS_VALID;

            *pADMATable = (DWORD)(dwPhys | dwFlags); // dwPhys must be 0x1000 (page) aligned

            DEBUGMSG(1, (_T("CMXDisk::after set(0x%x, 0x%x)(0x%x, 0x%x)\n"), 
              (pADMATable -1), (DWORD)*(pADMATable -1), (pADMATable), (DWORD)*(pADMATable)));
        break;
        default:
            DEBUGMSG(1, (_T("CSPATADMAAddItem undefined DMA mode\r\n")));
        break;
        }
}

// Start DMA
void CSPATADMAStart()
{
    switch(DMAMode)
    {
        case 0: // SDMA
            DDKSdmaStartChan(chan);
        break;
        case 1: // ADMA
        break;
        default:
            DEBUGMSG(1, (_T("CSPATADMAStart undefined DMA mode\r\n")));
        break;
    }
}

void CSPATADMAStop()
{
    switch(DMAMode)
    {
        case 0: // SDMA
            DDKSdmaStopChan(chan, TRUE);
        break;
        case 1: // ADMA
        break;
        default:
            DEBUGMSG(1, (_T("CSPATADMAStart undefined DMA mode\r\n")));
        break;
    }
}
/// End of DMA
